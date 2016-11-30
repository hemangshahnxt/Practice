//PinPadUtils.cpp
#include "stdafx.h"
#include "Pinpadutils.h"

#include "Verifone_SC5000_Canada_Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static HWND g_hPinPadWnd = NULL;
static HANDLE g_hPinPadThread = NULL;
static HANDLE g_hPleaseExitThread = NULL;
static HANDLE g_hPinPadPort = NULL;
static CString g_szPinPadComPort;
static HANDLE g_hPinPadPortClosingMutex = NULL;
static CString g_strLastMessage;
static PinPadUtils::MessageType g_mtlastMessageSent;
static BOOL g_bIsPinPadInitialized = FALSE;
static HANDLE g_hWaitForInitialization = NULL;
static HANDLE g_hWaitForCustomerSwipe = NULL;
static HANDLE g_hWaitForClearSwipe = NULL;
static HANDLE g_hWaitForDisplayMessage = NULL;
static HANDLE g_hWaitForSerialInformation = NULL;
static HANDLE g_hWaitForInteracMessage = NULL;
static HANDLE g_hWaitForConfirmAmountTipCashSurcharge = NULL;
static HANDLE g_hWaitForInteracAnalysis = NULL;
static CString g_strLastMessageRead;
static BOOL g_bWaitingOnTrackInfo = FALSE;

#define PIN_PAD_TIMEOUT   300

namespace PinPadUtils {

	// Grab a bowl of serial
	BOOL InitializePinPad(HWND hWnd, const char* szComPort /*=COM1*/)
	{
		DWORD dwThreadId;

 		g_hPinPadWnd = hWnd;
		g_szPinPadComPort = szComPort;
		//if (g_hThread) ClosePinPad();

		g_hPinPadPortClosingMutex = CreateMutex(NULL, FALSE, NULL);
		g_hPleaseExitThread = CreateEvent(NULL, TRUE, FALSE, NULL);

		//initialize things that we will use later
		g_hWaitForInitialization = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(g_hWaitForInitialization);
		g_hWaitForCustomerSwipe = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(g_hWaitForCustomerSwipe);
		g_hWaitForClearSwipe = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(g_hWaitForClearSwipe);
		g_hWaitForDisplayMessage = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(g_hWaitForDisplayMessage);
		g_hWaitForSerialInformation = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(g_hWaitForSerialInformation);
		g_hWaitForInteracMessage = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(g_hWaitForInteracMessage);
		g_hWaitForConfirmAmountTipCashSurcharge = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(g_hWaitForConfirmAmountTipCashSurcharge);
		g_hWaitForInteracAnalysis = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(g_hWaitForInteracAnalysis);
		

		DCB PortDCB;	// Port settings
		DCB DefaultDCB;
		CString strParity, strStopBits;

		// Open the serial port
		g_hPinPadPort = CreateFile (g_szPinPadComPort, // Name of the port
							GENERIC_READ | GENERIC_WRITE,
										// Access (read-write) mode
						  0,            // Share mode
						  NULL,         // Pointer to the security attribute
						  OPEN_EXISTING,// How to open the serial port
						  FILE_FLAG_OVERLAPPED, // Port attributes
						  NULL);        // Handle to port with attribute

		// Make sure the port is open
		if (g_hPinPadPort == INVALID_HANDLE_VALUE)
		{
			AfxMessageBox("Port Open Failed");
			PostMessage(g_hPinPadWnd, WM_PINPAD_OPEN_FAILED, 0, GetLastError());
			return FALSE;
		}

		

		// Initialize the DCBlength member. 
		PortDCB.DCBlength = sizeof (DCB); 

		// Get the default port setting information.
		GetCommState (g_hPinPadPort, &PortDCB);

		// Store it so we can restore it the way it was
		// before we close it
		memcpy(&DefaultDCB, &PortDCB, sizeof(DCB));

		// Set up port
		strParity = "None";
		strStopBits = "1";
		PortDCB.BaudRate = 9600;
		PortDCB.Parity = NOPARITY;
		PortDCB.StopBits = ONESTOPBIT;
		PortDCB.ByteSize = 8;
		
		if (!SetCommState(g_hPinPadPort, &PortDCB))
		{
			//CloseHandle(ol.hEvent); // Close the comm event
			// (b.cardillo 2005-05-02 15:36) - PLID 15568 - Closing the handle atomically.
			WaitForSingleObject(g_hPinPadPortClosingMutex, 5000);
			if (g_hPinPadPort) {
				// Close the port
				if (CloseHandle(g_hPinPadPort)) {
					g_hPinPadPort = NULL;
				}
			}
			ReleaseMutex(g_hPinPadPortClosingMutex);
			PostMessage(g_hPinPadWnd, WM_PINPAD_OPEN_FAILED, 1, GetLastError());
			return FALSE;
		}
		/* Notify us of receive events */
		SetCommMask (g_hPinPadPort, EV_RXCHAR);

		// (b.cardillo 2006-10-02 12:14) - PLID 22798 - Before we start reading from the 
		// port, we need to set the read timeouts to reasonable values.  Otherwise, they 
		// may default to all zeros which would result in our overlapped reads returning 
		// immediately even if no bytes were read (which was the case with this pl item).
		{
			COMMTIMEOUTS ctTimeouts;
			if (!GetCommTimeouts(g_hPinPadPort, &ctTimeouts)) {
				// Couldn't get the current write timeouts, so default to 0; we don't 
				// really care, because we won't be writing anyway.
				ctTimeouts.WriteTotalTimeoutMultiplier = 0;
				ctTimeouts.WriteTotalTimeoutConstant = 0;
			}
			// Set the read timeouts (this is the important part).  We choose MAXWORD for 
			// the total timeout, and an almost arbitrary number of milliseconds for the 
			// interval (how long to wait between bytes).  The exact number doesn't really 
			// matter because we only ever as for 1 byte at a time, and as long as 
			// ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier are not zero, we will 
			// wait indefinitely for that 1 byte, which is exactly what we want.  I chose 
			// 100 ms for the interval timeout because it seems a reasonable amount of time 
			// to wait for subsequent bytes if ever the situation were to arise.
			ctTimeouts.ReadIntervalTimeout = 100;
			ctTimeouts.ReadTotalTimeoutConstant = MAXDWORD;
			ctTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
			if (!SetCommTimeouts(g_hPinPadPort, &ctTimeouts)) {
				// If we failed to set the timeouts, it could be trouble.  It's possible the 
				// pin pad won't read, but it's more likely it will work fine, just 
				// eat a lot of cpu because this thread will loop constantly.  But since it 
				// will still work, we just log it here instead of completely failing.
				TRACE("Could not set port timeouts for Pin Pad!  GetLastError() returned %lu.", GetLastError());
			}
		}

		g_hPinPadThread = CreateThread(NULL, 0, Read_Thread, 0/*(void*)szComPort*/, 0, &dwThreadId);

		return FALSE;
	}


	void PinPadClose()
	{
		if (!g_hPinPadThread) return;

		SetEvent(g_hPleaseExitThread); // ResetEvent is the contrapositive

		// (b.cardillo 2005-02-04 11:16) - PLID 15390 - Incident 48475 - Per c.haag's original solution 
		// we close the handle to the port from here.  This prevents it from locking up in non-terminal 
		// server environments, while the other fix (see below) prevents it in terminal server.

		// (b.cardillo 2005-05-02 15:36) - PLID 15568 - Closing the handle atomically.
		WaitForSingleObject(g_hPinPadPortClosingMutex, 5000);
		if (g_hPinPadPort) {
			// Close the port
			if (CloseHandle(g_hPinPadPort)) {
				g_hPinPadPort = NULL;
			}
		}
		ReleaseMutex(g_hPinPadPortClosingMutex);

		// Wait for the thread to terminate
		WaitForSingleObject(g_hPinPadThread, INFINITE);
		
		// Close the handles that we opened for this thread
		CloseHandle(g_hPleaseExitThread);
		g_hPleaseExitThread = NULL;

		//CloseHandle(g_hThread);
		//g_hThread = NULL;

		CloseHandle(g_hPinPadPortClosingMutex);
		g_hPinPadPortClosingMutex = NULL;
	}

	CString GetDescFromMessageType(MessageType msgType) {

		switch (msgType) {

			case mtNone:
				return "None";
			break;

			case mtInitializeCredDebApp:
				return "Initialize Credit/Debit App";
			break;

			case mtReadTrackInformation:
				return "Get Track2 Info";
			break;

			case mtClearReadTrackInformation:
				return "Clear Screen";
			break;

			case mtInteracDebit:
				return "Process Interac Debit";
			break;

			case mtDisplayMessage:
				return "Display Message";
			break;

			case mtGetSerialNumber:
				return "Get Serial Number";
			break;

			case mtSendInteracRequest:
				return "Interac Request";
			break;

			case mtSendConfirmAmountTipCashSurcharge:
				return "Confirm Amount/Tip/Cashback/Surcharge";
			break;

			case mtSendInteracResponseAnalysis:
				return "Interac Response Analysis";
			break;

			default:
				return "Unknown";
			break;
		}
	}


	BOOL WriteToPinPad(CString strMessage) {


	//	DWORD dwBytesTransferred;
		//HANDLE pdwHandle;
	//	BYTE Byte;
		//OVERLAPPED ol = {0};
		//char szInput[256], *pszInput = szInput;

		
		// Initialize our events
		//ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		//pdwHandles[0] = g_hPleaseExitThread;
		//pdwHandle = ol.hEvent;

		unsigned long lpNumberofBytesWritten;
		OVERLAPPED ol = {0};
	//	HANDLE pdwHandles[2];

		//ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//	pdwHandles[0] = g_hPleaseExitThread;
		//pdwHandles[1] = ol.hEvent;

		//ResetEvent(ol.hEvent);
		//send the message to the COM port
#ifdef _DEBUG		
		CString strDesc = GetDescFromMessageType(g_mtlastMessageSent);
		if (strMessage.GetLength() == 1) {
			//its an ack or Nak
			strDesc = "ACK/NAK";
		}
		LogCCProcess("Writing " + strDesc + " To PinPad: " + strMessage);
		TRACE("Writing To PinPad: " + strMessage + "\r\n");
#endif


		if (!WriteFile(g_hPinPadPort, strMessage, strMessage.GetLength(), &lpNumberofBytesWritten, &ol)) {

				long lErr = GetLastError();
				if (lErr != 997 /* Overlapped I/O operation is in progress. */)
				{
					/* This should never happen */
					CString strErr;
					strErr.Format("WriteFile on Pin Pad failed. Reason: %d", lErr);
					AfxMessageBox(strErr);
					return FALSE;
				}
		}

		//save the message that was sent
		g_strLastMessage = strMessage;
		
		//WaitForSingleObject(pdwHandle, 1000);

	//	GetOverlappedResult(g_hPinPadPort, &ol, &dwBytesTransferred, FALSE /* The overlapped operation is already complete */);


			//now read
	//		CString strResponse = ReadFromPinPad();
		/*}
		else {
			long nError = GetLastError();
		}*/


		return TRUE;


	}




	DWORD WINAPI Read_Thread( LPVOID lpParam )
	{ 
		DWORD dwBytesTransferred;
		HANDLE pdwHandles[2];
		BYTE Byte;
		//DCB PortDCB;	// Port settings
		OVERLAPPED ol = {0};
		//DCB DefaultDCB;
		char szInput[256], *pszInput = szInput;
		CString strInput;
		CString strParity, strStopBits;
		
		// Open the serial port
	/*	g_hPinPadPort = CreateFile (g_szComPort, // Name of the port
							GENERIC_READ | GENERIC_WRITE,
										// Access (read-write) mode
						  0,            // Share mode
						  NULL,         // Pointer to the security attribute
						  OPEN_EXISTING,// How to open the serial port
						  FILE_FLAG_OVERLAPPED, // Port attributes
						  NULL);        // Handle to port with attribute

		// Make sure the port is open
		if (g_hPinPadPort == INVALID_HANDLE_VALUE)
		{
			PostMessage(g_hPinPadWnd, WM_PINPAD_OPEN_FAILED, 0, GetLastError());
			return TRUE;
		}*/

		// Initialize our events
		ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		pdwHandles[0] = g_hPleaseExitThread;
		pdwHandles[1] = ol.hEvent;

		// Initialize the DCBlength member. 
		//PortDCB.DCBlength = sizeof (DCB); 

		// Get the default port setting information.
		//GetCommState (g_hPinPadPort, &PortDCB);

		// Store it so we can restore it the way it was
		// before we close it
		//memcpy(&DefaultDCB, &PortDCB, sizeof(DCB));

		// Set up port
		/*strParity = "None";
		strStopBits = "1";
		PortDCB.BaudRate = 9600;
		PortDCB.Parity = NOPARITY;
		PortDCB.StopBits = ONESTOPBIT;
		
		if (!SetCommState(g_hPinPadPort, &PortDCB))
		{
			CloseHandle(ol.hEvent); // Close the comm event
			// (b.cardillo 2005-05-02 15:36) - PLID 15568 - Closing the handle atomically.
			WaitForSingleObject(g_hPinPadPortClosingMutex, 5000);
			if (g_hPinPadPort) {
				// Close the port
				if (CloseHandle(g_hPinPadPort)) {
					g_hPinPadPort = NULL;
				}
			}
			ReleaseMutex(g_hPinPadPortClosingMutex);
			PostMessage(g_hPinPadWnd, WM_PINPAD_OPEN_FAILED, 1, GetLastError());
			return TRUE;
		}*/
		/* Notify us of receive events */
		//SetCommMask (g_hPort, EV_RXCHAR);

		// (b.cardillo 2006-10-02 12:14) - PLID 22798 - Before we start reading from the 
		// port, we need to set the read timeouts to reasonable values.  Otherwise, they 
		// may default to all zeros which would result in our overlapped reads returning 
		// immediately even if no bytes were read (which was the case with this pl item).
		{
			COMMTIMEOUTS ctTimeouts;
			if (!GetCommTimeouts(g_hPinPadPort, &ctTimeouts)) {
				// Couldn't get the current write timeouts, so default to 0; we don't 
				// really care, because we won't be writing anyway.
				ctTimeouts.WriteTotalTimeoutMultiplier = 0;
				ctTimeouts.WriteTotalTimeoutConstant = 0;
			}
			// Set the read timeouts (this is the important part).  We choose MAXWORD for 
			// the total timeout, and an almost arbitrary number of milliseconds for the 
			// interval (how long to wait between bytes).  The exact number doesn't really 
			// matter because we only ever as for 1 byte at a time, and as long as 
			// ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier are not zero, we will 
			// wait indefinitely for that 1 byte, which is exactly what we want.  I chose 
			// 100 ms for the interval timeout because it seems a reasonable amount of time 
			// to wait for subsequent bytes if ever the situation were to arise.
			ctTimeouts.ReadIntervalTimeout = 100;
			ctTimeouts.ReadTotalTimeoutConstant = MAXDWORD;
			ctTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
			if (!SetCommTimeouts(g_hPinPadPort, &ctTimeouts)) {
				// If we failed to set the timeouts, it could be trouble.  It's possible the 
				// Pin Pad won't read, but it's more likely it will work fine, just 
				// eat a lot of cpu because this thread will loop constantly.  But since it 
				// will still work, we just log it here instead of completely failing.
				TRACE("Could not set port timeouts for Pin Pad!  GetLastError() returned %lu.", GetLastError());
			}
		}

		BOOL bNextToEnd = FALSE;
		BOOL bAtEnd = FALSE;

		/* This is the meat and potatoes; process input. */
		while (1)
		{
			/* Reset the overlapped event. The overlapped event is the
			act of reading one byte. */
			ResetEvent(ol.hEvent);
			if (!ReadFile(g_hPinPadPort, &Byte, 1, NULL, &ol))
			{
				long lErr = GetLastError();
				if (lErr != 997 /* Overlapped I/O operation is in progress. */)
				{
					/* This should never happen */
					TRACE("ReadFile on barcode scanner failed. Reason: %d", lErr);
					break;
				}
			}
			/* Wait for either the read, or an event to close Practice. */
			WaitForMultipleObjects(2, pdwHandles, FALSE, INFINITE);

			/* If an event to close practice was fired, quit. */
			if (WaitForSingleObject(g_hPleaseExitThread, 0) != WAIT_TIMEOUT) break;

			/* Find out how many bytes we read */
			GetOverlappedResult(g_hPinPadPort, &ol, &dwBytesTransferred, FALSE /* The overlapped operation is already complete */);

			if (dwBytesTransferred)
			{		
				if (!bNextToEnd) {
					if (Byte == 0x03) /* End of Text */
					{
						
						bNextToEnd = TRUE;

						// Append the character to our string
						strInput += char(Byte);
						TRACE("Next To End: " + strInput + "\r\n");
						*pszInput++ = Byte;

						// If we have too many bytes in our buffer, dump a copy
						// to the caller and reset our input string.
						if (pszInput - szInput == 250)
						{
							// (a.walling 2007-11-07 09:46) - PLID 27476 - VS2008 - Need a portable solution for allocating the BSTR
							// BSTR bstr = SysAllocString((unsigned short*)szInput);
							BSTR bstr = CString(szInput).AllocSysString();
							PostMessage(g_hPinPadWnd, WM_PINPAD_OVERFLOW, 250, (LPARAM)bstr);
							pszInput = szInput; // Reset string pointer
						}

						
					}
					else if (Byte == 127)
					{
						// I don't know why, but sometimes the parity on
						// the barcode scanner 'magically changes' from
						// one value to another. It deserves investigation,
						// but until then, these bytes are ignored because
						// they are interleaved with the barcode.
					}
					else if (Byte == 0x06) /* ACK */ 
					{
						//we can ignore this character
#ifdef _DEBUG
		TRACE("Recieved ACK: " + strInput + "\r\n");
		LogCCProcess("Recieved ACK: " + strInput);
#endif

						switch (g_mtlastMessageSent) {

							case mtReadTrackInformation:
								//nMessage = WM_PINPAD_MESSAGE_READ_TRACK_DONE;
#ifdef _DEBUG
		TRACE("ACK for Customer Swipe\r\n");
		LogCCProcess("ACK for Customer Swipe");
#endif
								SetEvent(g_hWaitForCustomerSwipe);
							break;

							case mtClearReadTrackInformation:
								//nMessage = WM_PINPAD_MESSAGE_CLEAR_DONE;
#ifdef _DEBUG
		TRACE("ACK for Clear Swipe\r\n");
		LogCCProcess("ACK for Clear Swipe");
#endif
								SetEvent(g_hWaitForClearSwipe);
							break;

							case mtDisplayMessage:
#ifdef _DEBUG
		TRACE("ACK for Display Message\r\n");
		LogCCProcess("ACK for Display Message");
#endif
								SetEvent(g_hWaitForDisplayMessage);
							break;

							default:
								//do nothing
							//	nMessage = WM_PINPAD_MESSAGE_UNKNOWN_READ;
							break;
						}		
					}
					else if (Byte == 0x15) /*NAK*/
					{
						//we have to send the last message again
						WriteToPinPad(g_strLastMessage);
#ifdef _DEBUG
		TRACE("Received NAK: " + strInput + "\r\n");
		LogCCProcess("Received NAK: " + strInput);
#endif
					}
					else /* Actual input from the scanner */
					{

						/*if (bNextToEnd) {
							bAtEnd = TRUE;
						}*/
						// Append the character to our string
						strInput += char(Byte);

						TRACE("Other: " + strInput + "\r\n");
						*pszInput++ = Byte;

						// If we have too many bytes in our buffer, dump a copy
						// to the caller and reset our input string.
						if (pszInput - szInput == 250)
						{
							// (a.walling 2007-11-07 09:46) - PLID 27476 - VS2008 - Need a portable solution for allocating the BSTR
							// BSTR bstr = SysAllocString((unsigned short*)szInput);
							BSTR bstr = CString(szInput).AllocSysString();
							PostMessage(g_hPinPadWnd, WM_PINPAD_OVERFLOW, 250, (LPARAM)bstr);
							pszInput = szInput; // Reset string pointer
						}
					}
				}
				else {

					//we are at the LRC
#ifdef _DEBUG
		LogCCProcess("Read From PinPad: " + strInput);
		TRACE("At End: " + strInput + "\r\n");
#endif

					//CString str = (char*)bstr;
					//we need to take off the last character because its the LRC
					char ch = CalcLRC(strInput);
					CString str;

					if (ch == Byte) {
						//return an ACK
						str = (char)0x06;					
#ifdef _DEBUG
		TRACE("Returning ACK \r\n");
		LogCCProcess("Returning ACK");
#endif
						WriteToPinPad(str);
						TRACE("Done Returning ACK\r\n");

						

						// Trail it with a NULL character
						*pszInput++ = 0;

						BSTR bstr;
						// (a.walling 2007-11-07 09:46) - PLID 27476 - VS2008 - Need a portable solution for allocating the BSTR
						// bstr = SysAllocString((unsigned short*)szInput);
						// (a.walling 2007-11-08 16:28) - PLID 27476 - This is never deallocated! Also it should be allocated immediately
						// before sending, since it is not sent in a message in all logic paths below. Added comment in 26710
						bstr = CString(szInput).AllocSysString();

						//now we can send post the message
						// Create our string and send it. We assume the
						// caller will deallocate it.

						g_strLastMessageRead = strInput;

						long nMessage;
						switch (g_mtlastMessageSent) {

							case mtInitializeCredDebApp :
								//nMessage = WM_PINPAD_MESSAGE_INITIALIZE_DONE;
								SetEvent(g_hWaitForInitialization);
							break;

							case mtReadTrackInformation:
								//nMessage = WM_PINPAD_MESSAGE_READ_TRACK_DONE;
								SetEvent(g_hWaitForCustomerSwipe);

								if (g_bWaitingOnTrackInfo) {
									//post a message with the track information
									PostMessage(g_hPinPadWnd, WM_PINPAD_MESSAGE_READ_TRACK_DONE, pszInput - szInput, (LPARAM)bstr);
									g_bWaitingOnTrackInfo = FALSE;
								}
							break;

							case mtClearReadTrackInformation:
								//nMessage = WM_PINPAD_MESSAGE_CLEAR_DONE;
								SetEvent(g_hWaitForClearSwipe);
							break;

							case mtInteracDebit:
								nMessage = WM_PINPAD_MESSAGE_INTERAC_DEBIT_DONE;
							break;

							case mtDisplayMessage:
								SetEvent(g_hWaitForDisplayMessage);
							break;

							case mtGetSerialNumber:
								SetEvent(g_hWaitForSerialInformation);
							break;

							case mtSendInteracRequest:
								SetEvent(g_hWaitForInteracMessage);
							break;

							case mtSendConfirmAmountTipCashSurcharge:
								SetEvent(g_hWaitForConfirmAmountTipCashSurcharge);
							break;

							case mtSendInteracResponseAnalysis:
								SetEvent(g_hWaitForInteracAnalysis);
							break;

							default:
								nMessage = WM_PINPAD_MESSAGE_UNKNOWN_READ;
							break;
						}								
									
									
						
						//PostMessage(g_hPinPadWnd, nMessage, pszInput - szInput, (LPARAM)bstr);
						bNextToEnd = FALSE;
						bAtEnd = FALSE;
						g_mtlastMessageSent = mtNone;
						//break;

						strInput = ""; 
						pszInput = szInput; // Reset string pointer

					}
					else {
						//return an NAK
						str = (char)0x15;
#ifdef _DEBUG
		TRACE("Returning NAK \r\n");
		LogCCProcess("Returning NAK");
#endif
						bNextToEnd = FALSE;
						strInput = "";
						pszInput = szInput;
						WriteToPinPad(str);
					}
				}


			}

			// We may have broken out of the loop because someone quit
			// the program while something was being scanned.
			if (WaitForSingleObject(g_hPleaseExitThread, 0) != WAIT_TIMEOUT) break;
		}

		// Restore the port settings we entered with
	//	SetCommState (g_hPinPadPort, &DefaultDCB);

		CloseHandle(ol.hEvent); // Close the comm event	

		// (b.cardillo 2005-02-04 11:16) - PLID 15390 - Incident 48475 - Well this solution doesn't work 
		// for all cases, but it's the only one that works for the terminal server case so I'm leaving it 
		// in, but for the other cases I'm putting back c.haag's fix (which was just closing the handle 
		// from outside the thread; see above).

			// (b.cardillo 2005-05-02 15:30) - PLID 15568 - T.schneider found that in some cases (though 
			// maybe only in debug mode) the fact that we close the handle both inside and outside of the 
			// thread can cause exceptions thrown from kernel32.dll.  So I'm making it so both places will 
			// only close it if it's not null, and then immediately set it to null after closing.  But in 
			// order to do this atomically we have to surround the two actions in a mutex.

		// (b.cardillo 2005-01-24 09:29) - PLID 15390 - Incident 46552 - We need to close the handle from 
		// within the thread that owns the handle.  C.haag commented the close out back in October as a way 
		// of trying to fix a problem.  The problem was that when using ThinPATH to forward serial port 
		// communication through a terminal server session Practice would not close because it was waiting 
		// on this thread, and this thread would get stuck.  But then when Microsoft Windows Server 2003 
		// came out, ThinPATH was no longer necessary, but the problem of Practice locking up on close came 
		// back!  It came back because trying to close the port outside the thread was not the right 
		// solution.  It only had side-effects that resulted in a masking of the problem under ThinPATH.  
		// So from my research and testing I've found that the better solution is to avoid the lock 
		// altogether by calling CancelIo() before attempting to close the port (and to still close the 
		// port from within the thread).  So that's what we do now.
		CancelIo(g_hPinPadPort);
		// The old comment was "(c.haag 2004-10-07 17:54) - We already close the port at Barcode_Close" 
		// which is not valid anymore, because we are back to closing the port here (see comment above)

		// (b.cardillo 2005-05-02 15:36) - PLID 15568 - Closing the handle atomically.
		WaitForSingleObject(g_hPinPadPortClosingMutex, 5000);
		if (g_hPinPadPort) {
			// Close the port
			if (CloseHandle(g_hPinPadPort)) {
				g_hPinPadPort = NULL;
			}
		}
		ReleaseMutex(g_hPinPadPortClosingMutex);
		
		return 0;
	} 


	char CalcLRC(CString strMessage) {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:

				return Verifone_SC5000_Canada_Utils::CalcLRC(strMessage);
			break;

			//that's all we got so far

			default:
				ASSERT(FALSE);
				return 'A';
			break;

		}
	}

	CString GetCustomerSwipe() {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:

				g_mtlastMessageSent = mtReadTrackInformation;

				Verifone_SC5000_Canada_Utils::SendTrack2Message();

				if (WaitForSingleObject(g_hWaitForCustomerSwipe, 60000) != WAIT_TIMEOUT) {

					ResetEvent(g_hWaitForCustomerSwipe);

					g_bWaitingOnTrackInfo = TRUE;
					
					//we have the swipe data, let's send it back
					return g_strLastMessageRead;
				}
				else {
					//we timed out
					AfxMessageBox("Timed Out");
				}

			break;

			default:
				return "";				
			break;
		}
		return "";
	} 


	void ClearCustomerSwipe() {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:

				g_mtlastMessageSent = mtClearReadTrackInformation;

				Verifone_SC5000_Canada_Utils::ClearSwipe();

				if (WaitForSingleObject(g_hWaitForClearSwipe, PIN_PAD_TIMEOUT) != WAIT_TIMEOUT) {

					ResetEvent(g_hWaitForClearSwipe);

					g_bWaitingOnTrackInfo = FALSE;
					
					//there really isn't anything to return here either
				}
				else {
					//we timed out
					AfxMessageBox("Timed Out");
				}
			break;

			default:
				
			break;
		}
	} 


	BOOL IsInitialized() {

		return g_bIsPinPadInitialized;
	}

	BOOL SendInitMessage() {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:

				g_mtlastMessageSent = mtInitializeCredDebApp;

				Verifone_SC5000_Canada_Utils::SendInitMessage();

				if (WaitForSingleObject(g_hWaitForInitialization, PIN_PAD_TIMEOUT) != WAIT_TIMEOUT) {

					//there really isn't anything to return here
					g_bIsPinPadInitialized = TRUE;

					ResetEvent(g_hWaitForInitialization);

					return TRUE;
				}
				else {
					//we timed out, return false
					g_bIsPinPadInitialized = FALSE;
					return FALSE;
				}
			break;

			default:
				return FALSE;

			break;
		}
	} 

	void DisplayMessage(CString strTopLine, CString strBottomLine) {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:

				g_mtlastMessageSent = mtDisplayMessage;
				
				Verifone_SC5000_Canada_Utils::SendDisplayMessage(strTopLine, strBottomLine);

				if (WaitForSingleObject(g_hWaitForDisplayMessage, PIN_PAD_TIMEOUT) != WAIT_TIMEOUT) {

					//there isn't anything to know except its displaying now
				}
			break;

			default:

			break;
		}
	}


	BOOL FillSerialInformation(CString &strApplicationName, CString &strCoreLibraryVersion, CString &strSecurityLibraryVersion, 
		CString &strPinPadSerialNumber, CString &strROMVersion, CString &strRAMVersion) {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:

				g_mtlastMessageSent = mtGetSerialNumber;
				
				Verifone_SC5000_Canada_Utils::SendSerialNumber();

				//this seems to take a while to get
				if (WaitForSingleObject(g_hWaitForSerialInformation, PIN_PAD_TIMEOUT + 1000) != WAIT_TIMEOUT) {

					ResetEvent(g_hWaitForSerialInformation);

					//we got it, we can return
					//let's parse the information

					return Verifone_SC5000_Canada_Utils::ParseSerialNumberResponse(g_strLastMessageRead,
						strApplicationName, strCoreLibraryVersion, strSecurityLibraryVersion, 
						strPinPadSerialNumber, strROMVersion, strRAMVersion);
				}
			break;

			default:
				return FALSE;
			break;
		}

		return FALSE;
	}


	BOOL GetTrack2InformationFromWholeSwipe(CString strSwipe, CString &strTrack2, CString &strTrack1, CString &strTrack3) {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:

				g_mtlastMessageSent = mtGetSerialNumber;
				
				return Verifone_SC5000_Canada_Utils::GetTrack2InformationFromWholeSwipe(strSwipe, strTrack2, strTrack1, strTrack3);

			break;

			default:

			break;
		}

		return FALSE;
	}

	void GetCreditCardTypeFromNumber(CString strAccountNumber, CString &strDescription, CString &strCD)
	{
		strDescription = "";
		strCD = "";
		long nBINNumber = atoi(strAccountNumber.Left(8));
		
		// Determine the card type by the BIN number
		if (nBINNumber >= 40000000 && nBINNumber <= 49999999) {
			strDescription = "Visa";
			strCD = "VI";
		}
		else if ((nBINNumber >= 51000000 && nBINNumber <= 55999999) ||
			(nBINNumber >= 36000000 && nBINNumber <= 36999999)) {
			strDescription = "MasterCard";
			strCD = "MC";
		}
		else if ((nBINNumber >= 60110000 && nBINNumber <= 60119999) ||
			(nBINNumber >= 65000000 && nBINNumber <= 65999999) ||
			(nBINNumber >= 62212600 && nBINNumber <= 62292599)) {
			strDescription = "Discover";
			strCD = "DS";
		}
		else if ((nBINNumber >= 34000000 && nBINNumber <= 34999999) ||
			(nBINNumber >= 37000000 && nBINNumber <= 37999999)) {
			strDescription = "American Express";
			strCD = "AE";
		}
		else if ((nBINNumber >= 30830000 && nBINNumber <= 33299999) ||
			(nBINNumber >= 35280000 && nBINNumber <= 35899999)) {
			strDescription = "JCB";
			strCD = "JC";
		}
		else if ((nBINNumber >= 30813600 && nBINNumber <= 30813699) ||
			(nBINNumber >= 30813800 && nBINNumber <= 30819999)) {
			strDescription = "In Good Taste";
			strCD = "IG";
		}
		else if (nBINNumber >= 69000710 && nBINNumber <= 69000710) {
			strDescription = "Retail PL";
			strCD = "PL";
		}
		else if (nBINNumber >= 60302800 && nBINNumber <= 60302899) {
			strDescription = "Club Card";
			strCD = "CC";
		}
		else if (nBINNumber >= 60375700 && nBINNumber <= 60375799) {
			strDescription = "RAI";
			strCD = "RA";
		}
		else if (nBINNumber >= 70717000 && nBINNumber <= 70717099) {
			strDescription = "Irving Oil PL";
			strCD = "IR";
		}
		else if ((nBINNumber >= 76278300 && nBINNumber <= 76278399) ||
			(nBINNumber >= 76278900 && nBINNumber <= 76278999)) {
			strDescription = "Smarte Card";
			strCD = "SC";
		}
		else if (nBINNumber >= 70000500 && nBINNumber <= 70000599) {
			strDescription = "TCH Light Fleet";
			strCD = "TH";
		}
		else if (nBINNumber >= 70714500 && nBINNumber <= 70714599) {
			strDescription = "TCRS Consumer";
			strCD = "TS";
		}
		else if (nBINNumber >= 70828500 && nBINNumber <= 70828599) {
			strDescription = "MotoMart";
			strCD = "MM";
		}
		else if (nBINNumber >= 62742500 && nBINNumber <= 62742599) {
			strDescription = "Disney Rewards";
			strCD = "DY";
		}
		else {
			strDescription = "Unknown";
			strCD = "Unknown";
		}
	}

	BOOL GetStringField(IN CString strString, IN long nFieldStart, IN BOOL bFixedLength, IN CString strFixedLengthOrFieldSeparator, 
					OUT CString &strField, OUT long &nNextFieldStart) 
	{
		if (strString.GetLength() < nFieldStart) {
			// The string isn't even long enough
			return FALSE;
		}

		long nFixedLength = -1;
		long nFieldEnd = -1;
		if (bFixedLength) {
			nFixedLength = atoi(strFixedLengthOrFieldSeparator);
			nFieldEnd = nFieldStart + nFixedLength;
		}
		else {
			if (strFixedLengthOrFieldSeparator.Find(",") != -1) {
				// There are two separators, so let's try them both and use the closest one
				CString strSeparator1 = strFixedLengthOrFieldSeparator.Left(1), strSeparator2 = strFixedLengthOrFieldSeparator.Right(1);
				long nSeparator1 = -1, nSeparator2 = -1;
				nSeparator1 = strString.Find((LPCTSTR)strSeparator1, nFieldStart);
				nSeparator2 = strString.Find((LPCTSTR)strSeparator2, nFieldStart);

				if (nSeparator1 == -1 && nSeparator2 != -1)
					nFieldEnd = nSeparator2;
				else if (nSeparator1 != -1 && nSeparator2 == -1)
					nFieldEnd = nSeparator1;
				else if (nSeparator1 != -1 && nSeparator2 != -1) {
					if (nSeparator1 < nSeparator2)
						nFieldEnd = nSeparator1;
					else
						nFieldEnd = nSeparator2;
				}
			}
			else {
				nFieldEnd = strString.Find((LPCTSTR)strFixedLengthOrFieldSeparator, nFieldStart);
			}
		}

		if (nFieldEnd == -1 || nFieldEnd > strString.GetLength()) {
			// Let's just use the rest of the string
			nFieldEnd = strString.GetLength();
		}

		// Get the field
		strField = strString.Mid(nFieldStart, nFieldEnd-nFieldStart);

		// Find the next field's start
		if (bFixedLength) {
			// The next field starts right away
			nNextFieldStart = nFieldEnd;
		}
		else {
			// The next field starts after the separator
			nNextFieldStart = nFieldEnd + 1;
		}

		return TRUE;
	}

	void ParseTrackInformation(CString strTrack1, CString strTrack2, CString strTrack3, CString &strAccountNumber, CString &strFirstName, CString & strMiddleName,
		CString &strLastName, CString &strSuffix, CString &strTitle, CString &strCardType, CString &strCD, COleDateTime &dtExpireDate) {

		//I took this from the ParseCreditCardInfo function in the OPOSMSRDevice
		// ******************************* TRACK 1 ************************************
		// The first character should be "B" which signifies that this is a card from a financial institution
		CString strIDChar(""), strName(""), strExpDate("");
		long nNextFieldStart = -1;
		
		// Get the inital character on the track
		if (GetStringField(strTrack1, 0, TRUE, "1", strIDChar, nNextFieldStart)) {
			// Check the ID character
			if (strIDChar != "B") {
				ASSERT(FALSE);
				// This doesn't appear to be a credit card, so this function should not be used to parse this card's data
				return;
			}
			// Get the account number
			if (GetStringField(strTrack1, nNextFieldStart, FALSE, "^", strAccountNumber, nNextFieldStart)) {
				// Get the name on the card
				if (GetStringField(strTrack1, nNextFieldStart, FALSE, "^", strName, nNextFieldStart)) {
					// Get the expiration date
					GetStringField(strTrack1, nNextFieldStart, TRUE, "4", strExpDate, nNextFieldStart);
				}
			}
		}

		// ******************************* TRACK 2 ************************************
		// The second track has the same information as the first track, but is there for redundancy
		CString strAccountNumberRedundant(""), strExpDateRedundant("");
		nNextFieldStart = -1;

		// Get the account number
		if (GetStringField(strTrack2, 0, FALSE, "=", strAccountNumberRedundant, nNextFieldStart)) {
			// Get the expiration date
			GetStringField(strTrack2, nNextFieldStart, TRUE, "4", strExpDateRedundant, nNextFieldStart);
		}

		// (d.thompson 2010-10-26) - PLID 41080 - American express now puts spaces in some of their numbers, which throws off the 
		//	"convert to numeric" functionality we use later, and breaks the string comparison here.  It seems like Track 1 contains
		//	the spaces, while track 2 does not.  I've found a reference online confirming this, so it's not a fluke.
		strAccountNumber.Remove(' ');
		strAccountNumberRedundant.Remove(' ');

		////////////////////////////////////////////////////////////
		//// Now process the information

		// First make sure that the redundancy between the first two tracks checks out, if not then there was an error reading the 
		// card and return blank information
		if (strAccountNumber != strAccountNumberRedundant || strExpDate != strExpDateRedundant) {
			// There was trouble reading the card, return blank information
			return;
		}

		// Get the card type
		GetCreditCardTypeFromNumber(strAccountNumber, strCardType, strCD);

		// Parse the expiration date
		dtExpireDate = COleDateTime(1899,12,30,0,0,0);
		long nYear = atoi(strExpDate.Left(2)) + 2000;
		long nMonth = atoi(strExpDate.Right(2));
		if (strExpDate != "") {
			dtExpireDate.SetDate(nYear, nMonth, 1);
		}

		// Parse the name
		long nSpace1Pos = -1, nSpace2Pos = -1;
		nSpace1Pos = strName.Find("/");
		if (nSpace1Pos != -1) {
			nSpace2Pos = strName.Find(" ", nSpace1Pos+1);
			if (nSpace2Pos == -1) {
				// (Last/First)
				strLastName = strName.Left(nSpace1Pos);
				strFirstName = strName.Right(strName.GetLength() - (nSpace1Pos+1));
			}
			else {
				// (Last/First Middle)
				strLastName = strName.Left(nSpace1Pos);
				strFirstName = strName.Mid(nSpace1Pos+1, nSpace2Pos-nSpace1Pos);
				strMiddleName = strName.Right(strName.GetLength() - (nSpace2Pos+1));
			}
		}
		else {
			nSpace1Pos = strName.Find(" ");
			nSpace2Pos = strName.Find(" ", nSpace1Pos+1);
			if (nSpace2Pos == -1) {
				// (First Last)
				strFirstName = strName.Left(nSpace1Pos);
				strLastName = strName.Right(strName.GetLength() - (nSpace1Pos+1));
			}
			else {
				// (First Middle Last)
				strFirstName = strName.Left(nSpace1Pos);
				strMiddleName = strName.Mid(nSpace1Pos+1, nSpace2Pos-nSpace1Pos);
				strLastName = strName.Right(strName.GetLength() - (nSpace2Pos+1));
			}
		}
		strFirstName.TrimLeft(); strFirstName.TrimRight();
		strMiddleName.TrimLeft(); strMiddleName.TrimRight();
		strLastName.TrimLeft(); strLastName.TrimRight();

		// Return all of the credit card information
		return;		

	}


	BOOL SendInteracRequest(IN long nTransID, IN COleCurrency cyTransAmount, IN CString strAccountNumber, 
		//information for MACing
		IN CString strMerchantNumber, IN CString strTerminalNumber, IN CString strTransactionCode, 
		IN CString strSequenceNumber, 		
		//out parameters
		OUT long &nAccountType, OUT CString &strMACBlock, OUT CString& strPINBlock, OUT CString &strPinPadSerialNumber) {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:
				{

					g_mtlastMessageSent = mtSendInteracRequest;

								
					Verifone_SC5000_Canada_Utils::SendInteracRequestMessage(nTransID, cyTransAmount, strAccountNumber, 
						strMerchantNumber, strTerminalNumber, strTransactionCode, strSequenceNumber);

					if (WaitForSingleObject(g_hWaitForInteracMessage, INFINITE) != WAIT_TIMEOUT) {

						ResetEvent(g_hWaitForInteracMessage);

					//we got it, we can return
					//let's parse the information

						return Verifone_SC5000_Canada_Utils::ParseInteracResponse(g_strLastMessageRead,
							nAccountType, strMACBlock, strPINBlock, strPinPadSerialNumber);
					}
				}
				
			break;

			default:

			break;
		}

		return FALSE;



	}


	BOOL SendConfirmAmountTipCashSurcharge(long lcCode, COleCurrency cyTransAmount, BOOL bTipFlag, BOOL bCashBackFlag, BOOL bSurchargeFlag, COleCurrency cySurchargeAmount,
		//out parameters
		OUT COleCurrency &cyTipAmount, OUT COleCurrency &cyCashBackAmount, BOOL &bSurchargeAccepted, CString &strMessage) {


		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:
				{

					g_mtlastMessageSent = mtSendConfirmAmountTipCashSurcharge;

					
					Verifone_SC5000_Canada_Utils::SendConfirmAmountTipCashSurcharge(lcCode, cyTransAmount, bTipFlag, bCashBackFlag, bSurchargeFlag, cySurchargeAmount);

					if (WaitForSingleObject(g_hWaitForConfirmAmountTipCashSurcharge, INFINITE) != WAIT_TIMEOUT) {

						ResetEvent(g_hWaitForConfirmAmountTipCashSurcharge);

						//we got it, we can return
						//let's parse the information

						return Verifone_SC5000_Canada_Utils::ParseConfirmAmountTipCashSurcharge(g_strLastMessageRead,
							bTipFlag, bCashBackFlag,
							cyTipAmount, cyCashBackAmount, bSurchargeAccepted, strMessage);
					}
				}
				break;

				default:

			break;
		}

		return FALSE;
	}

	BOOL PerformInteracResponseAnalysis(long lcLangCode, CString strBalanceEncryption, CString strMessageKey, CString strTPK, CString strTAK, CString strTopLine, CString strBottomLine, CString strMACData, CString &strMessage) {

		DeviceType dtDevice = dtVerifoneSCCanada5000;//eventually when we support more, this will be a configrt value

		switch (dtDevice) {

			case dtVerifoneSCCanada5000:
				{

					g_mtlastMessageSent = mtSendInteracResponseAnalysis;

					
					Verifone_SC5000_Canada_Utils::PerformInteracResponseAnalysis(lcLangCode, strBalanceEncryption, strMessageKey, strTPK, strTAK, strTopLine, strBottomLine, strMACData);

					if (WaitForSingleObject(g_hWaitForInteracAnalysis, INFINITE) != WAIT_TIMEOUT) {

						ResetEvent(g_hWaitForInteracAnalysis);

						//we got it, we can return
						//let's parse the information

						//the pinpad resets itself when this happens, so we need to wait for that
						return Verifone_SC5000_Canada_Utils::ParseInteracResponseAnalysis(g_strLastMessageRead,
							strMessage);

						}
				}
				break;

				default:

			break;
		}

		return FALSE;


	}

	static CFile l_fLogCCTransactions;
	static BOOL l_bLogFileCCTransLocked = FALSE;

	CFile *GetLogFileCCTransactions()
	{
		// See if the file's already open
		if (l_fLogCCTransactions.m_hFile != CFile::hFileNull) {
			return &l_fLogCCTransactions;
		}
		
		// We'll probably need to open the log file; it's at this path.
		// (j.armen 2011-10-25 15:47) - PLID 46139 - Save this in the session path or practice path based on concurrency
		CString strFilePath = GetPracPath(g_pLicense->IsConcurrent()?PracPath::SessionPath:PracPath::PracticePath) ^ "CreditCardProcessing.log";
		
		// If we already know this log file is locked, there's no need to even try
		if (l_bLogFileCCTransLocked) {
			// But, if the locked file was later removed for analysis, we should resume logging in 
			// case we want to try to log the problem happening again.  (This way the user won't 
			// have to close and re-open Practice just to get the logging to start up again.)
			if (DoesExist(strFilePath)) {
				// Yep, the locked file is still there, so don't mess with it
				return NULL;
			} else {
				// The locked file was removed, so proceed normally, recreating a new log file
				l_bLogFileCCTransLocked = FALSE;
			}
		}
		
		// Make sure the parent folder exists
		{
			CString strFolderPath = GetFilePath(strFilePath);
			if (!strFolderPath.IsEmpty() && !DoesExist(strFolderPath)) {
				CreatePath(strFolderPath);
			}
		}
		
		// Try to open the file
		if (l_fLogCCTransactions.Open(strFilePath, CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyWrite, NULL)) {
			// We opened the file, now see if it has the 'locked' indicator.
			char chIsLocked;
			if (l_fLogCCTransactions.Read(&chIsLocked, 1) == 1 && chIsLocked == '1') {
				// File is locked, which means the problem has been reproduced and appropriately 
				// logged so there's no need to keep adding to the log file.
				l_fLogCCTransactions.Close();
				l_bLogFileCCTransLocked = TRUE;
				return NULL;
			} else {
				// We're free to truncate the file and start from scratch
				l_fLogCCTransactions.SeekToBegin();
				l_fLogCCTransactions.SetLength(0);
				// Write the 'unlocked' indicator
				// Followed by the version/timestamp of this executable
				CString strInitLog;
				strInitLog.Format("BeginLog...");
				l_fLogCCTransactions.Write((LPCTSTR)strInitLog, strInitLog.GetLength());
				// Return
				return &l_fLogCCTransactions;
			}
		} else {
			// Couldn't open the file, try again next time around.
			return NULL;
		}
	}

	void LogCCProcess(LPCTSTR strToLog)
	{
		CFile *pLog = GetLogFileCCTransactions();
		if (pLog) {
			// Convert the ... to and argumented string
			/*CString strString;
			va_list argList;
			va_start(argList, strFormat);
			strString.FormatV(strFormat, argList);
			va_end(argList);
			*/
			// Format the string for output
			CString strOutput;
			strOutput.Format("%s:%5li: ", COleDateTime::GetCurrentTime().Format("%c"), GetTickCount());

			strOutput += strToLog;

			strOutput += "\r\n";
			
			// Output the string
			pLog->Write(strOutput, strOutput.GetLength());
		}
	}

	void LogCCProcessLockAndClose()
	{
		CFile *pLog = GetLogFileCCTransactions();
		if (pLog) {
			pLog->SeekToBegin();
			pLog->Write("1", 1);
			pLog->Close();
			// Also rename it so we don't have to manually delete the log file just to get logging to resume
			// (j.armen 2011-10-25 15:53) - PLID 46139 - This is saved to the session or practice path based upon concurrency
			{
				CString strPath = GetPracPath(g_pLicense->IsConcurrent()?PracPath::SessionPath:PracPath::PracticePath) ^ "NexTech";
				CString strNewName;
				strNewName.Format("CreditCardProcessing_%s.log", COleDateTime::GetCurrentTime().Format("%Y%m%d-%H%M%S"));
				MoveFile(strPath ^ "CreditCardProcessing.log", strPath ^ strNewName);
			}
			// And remember we just locked it so next time we try to log we'll create a new log file
			l_bLogFileCCTransLocked = TRUE;
		}
	}
}
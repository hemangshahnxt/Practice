#include <stdafx.h>
#include "barcode.h"

// (c.haag 2008-09-23 12:13) - PLID 20990 - Now using CWinThread
static CWinThread* g_pThread = NULL;

static HWND g_hWnd = NULL;
static HANDLE g_hPleaseExitThread = NULL;
static HANDLE g_hPort = NULL;
static CString g_szComPort;
static HANDLE g_hPortClosingMutex = NULL;

UINT Barcode_Thread(LPVOID pParam);

// Grab a bowl of serial
BOOL Barcode_Open(HWND hWnd, const char* szComPort)
{
	g_hWnd = hWnd;
	g_szComPort = szComPort;
	if (NULL != g_pThread) {
		Barcode_Close();
	}
	g_hPortClosingMutex = CreateMutex(NULL, FALSE, NULL);
	g_hPleaseExitThread = CreateEvent(NULL, TRUE, FALSE, NULL);
	// (c.haag 2008-09-23 12:13) - PLID 20990 - Launch the thread
	g_pThread = AfxBeginThread(Barcode_Thread, NULL, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	g_pThread->m_bAutoDelete = false;
	g_pThread->ResumeThread();

	return FALSE;
}

void Barcode_Close()
{
	// (c.haag 2008-09-23 12:15) - PLID 20990 - Replaced old thread handle with g_pThread
	if (!g_pThread) return;

	SetEvent(g_hPleaseExitThread); // ResetEvent is the contrapositive

	// (b.cardillo 2005-02-04 11:16) - PLID 15390 - Incident 48475 - Per c.haag's original solution 
	// we close the handle to the port from here.  This prevents it from locking up in non-terminal 
	// server environments, while the other fix (see below) prevents it in terminal server.

	// (b.cardillo 2005-05-02 15:36) - PLID 15568 - Closing the handle atomically.
	WaitForSingleObject(g_hPortClosingMutex, 5000);
	if (g_hPort) {
		// Close the port
		if (CloseHandle(g_hPort)) {
			g_hPort = NULL;
		}
	}
	ReleaseMutex(g_hPortClosingMutex);

	// Wait for the thread to terminate
	WaitForSingleObject(g_pThread->m_hThread, INFINITE);
	
	// Close the handles that we opened for this thread
	CloseHandle(g_hPleaseExitThread);
	g_hPleaseExitThread = NULL;

	delete g_pThread;
	g_pThread = NULL;

	CloseHandle(g_hPortClosingMutex);
	g_hPortClosingMutex = NULL;
}

UINT Barcode_Thread(LPVOID pParam)
{ 
	DWORD dwBytesTransferred;
	HANDLE pdwHandles[2];
	BYTE Byte;
	DCB PortDCB;	// Port settings
	OVERLAPPED ol = {0};
	DCB DefaultDCB;
	char szInput[256], *pszInput = szInput;
	CString strParity, strStopBits;
	
	// Open the serial port
	g_hPort = CreateFile (g_szComPort, // Name of the port
						GENERIC_READ | GENERIC_WRITE,
                                    // Access (read-write) mode
                      0,            // Share mode
                      NULL,         // Pointer to the security attribute
                      OPEN_EXISTING,// How to open the serial port
                      FILE_FLAG_OVERLAPPED, // Port attributes
                      NULL);        // Handle to port with attribute

	// Make sure the port is open
	if (g_hPort == INVALID_HANDLE_VALUE)
	{
		PostMessage(g_hWnd, WM_BARCODE_OPEN_FAILED, 0, GetLastError());
		return TRUE;
	}

	// Initialize our events
	ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	pdwHandles[0] = g_hPleaseExitThread;
	pdwHandles[1] = ol.hEvent;

	// Initialize the DCBlength member. 
	PortDCB.DCBlength = sizeof (DCB); 

	// Get the default port setting information.
	GetCommState (g_hPort, &PortDCB);

	// Store it so we can restore it the way it was
	// before we close it
	memcpy(&DefaultDCB, &PortDCB, sizeof(DCB));

	// Set up port
	// (j.jones 2013-05-15 13:42) - PLID 25479 - converted these settings to per user, per workstation,
	// pulling the defaults from their old per-workstation value
	strParity = GetRemotePropertyText("BarcodeScannerParity_UserWS", GetPropertyText("BarcodeScannerParity", "None"), 0, GetCurrentUserComputerName(), true);
	strStopBits = GetRemotePropertyText("BarcodeScannerStopBits_UserWS", GetPropertyText("BarcodeScannerStopBits", "1"), 0, GetCurrentUserComputerName(), true);
	PortDCB.BaudRate = 9600;
	PortDCB.Parity = NOPARITY;//SPACEPARITY;
	PortDCB.StopBits = ONESTOPBIT;//TWOSTOPBITS;

	if (strParity == "None") PortDCB.Parity = NOPARITY;
	else if (strParity == "Odd") PortDCB.Parity = ODDPARITY;
	else if (strParity == "Even") PortDCB.Parity = EVENPARITY;
	else if (strParity == "Space") PortDCB.Parity = SPACEPARITY;

	if (strStopBits == "1") PortDCB.StopBits = ONESTOPBIT;
	else if (strStopBits == "1.5") PortDCB.StopBits = ONE5STOPBITS;
	else if (strStopBits == "2") PortDCB.StopBits = TWOSTOPBITS;

	if (!SetCommState(g_hPort, &PortDCB))
	{
		CloseHandle(ol.hEvent); // Close the comm event
		// (b.cardillo 2005-05-02 15:36) - PLID 15568 - Closing the handle atomically.
		WaitForSingleObject(g_hPortClosingMutex, 5000);
		if (g_hPort) {
			// Close the port
			if (CloseHandle(g_hPort)) {
				g_hPort = NULL;
			}
		}
		ReleaseMutex(g_hPortClosingMutex);
		PostMessage(g_hWnd, WM_BARCODE_OPEN_FAILED, 1, GetLastError());
		return TRUE;
	}
	/* Notify us of receive events */
	SetCommMask (g_hPort, EV_RXCHAR);

	// (b.cardillo 2006-10-02 12:14) - PLID 22798 - Before we start reading from the 
	// port, we need to set the read timeouts to reasonable values.  Otherwise, they 
	// may default to all zeros which would result in our overlapped reads returning 
	// immediately even if no bytes were read (which was the case with this pl item).
	{
		COMMTIMEOUTS ctTimeouts;
		if (!GetCommTimeouts(g_hPort, &ctTimeouts)) {
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
		if (!SetCommTimeouts(g_hPort, &ctTimeouts)) {
			// If we failed to set the timeouts, it could be trouble.  It's possible the 
			// barcode scanner won't read, but it's more likely it will work fine, just 
			// eat a lot of cpu because this thread will loop constantly.  But since it 
			// will still work, we just log it here instead of completely failing.
			LogDetail("Could not set port timeouts for barcode scanner!  GetLastError() returned %lu.", GetLastError());
		}
	}

	// (a.walling 2007-11-27 09:23) - PLID 28167 - Support wireless metrologic VoyagerBT (MS9535) scanner
	// These are defines for the standard start of text, end of text ascii control codes
	const char STX = char(2);
	const char ETX = char(3);
	// The VoyagerBT by default was scanning in this format: [STX]x1234567890[ETX]y where x defines the type
	// of code, y defines the checksum (either mod 10 or a running XOR, depending on code type x).

	bool bReadInProgress = false;

	/* This is the meat and potatoes; process input. */
	while (1)
	{
		/* Reset the overlapped event. The overlapped event is the
		act of reading one byte. */
		// (a.walling 2012-12-24 13:37) - PLID 54321 - The overlapped system in windows owns this event, not us. ReadFile will reset it.
		//ResetEvent(ol.hEvent);

		BOOL bRead = ReadFile(g_hPort, &Byte, 1, NULL, &ol);
		if (bRead) {
			// synchronous read ok, fulfilled by buffer
		} else {
			long lErr = GetLastError();
			if (lErr != 997 /* Overlapped I/O operation is in progress. */)
			{
				/* This should never happen */
				LogDetail("ReadFile on barcode scanner failed. Reason: %d", lErr);
				break;
			}

			// asynchronous read in progress, io pending
			bReadInProgress = true;

			/* Wait for either the read, or an event to close Practice. */
			WaitForMultipleObjects(2, pdwHandles, FALSE, INFINITE);

			/* If an event to close practice was fired, quit. */
			if (WaitForSingleObject(g_hPleaseExitThread, 0) != WAIT_TIMEOUT) break;

			/* Find out how many bytes we read */
			BOOL bSuccess = GetOverlappedResult(g_hPort, &ol, &dwBytesTransferred, FALSE /* The overlapped operation is already complete */);

			bReadInProgress = false;
		}

		if (dwBytesTransferred)
		{
			if ( (Byte == '\r') || (Byte == ETX) ) /* End of barcode */
			{
				BSTR bstr;

				// Trail it with a NULL character
				*pszInput++ = 0;

				// Create our string and send it. We assume the
				// caller will deallocate it.
				// (a.walling 2007-11-06 17:49) - PLID 27476 - VS2008 - Need a portable solution for allocating the BSTR
				//bstr = SysAllocString((unsigned short*)szInput);

				CString strBarcode(szInput);

				// (a.walling 2007-11-27 10:26) - PLID 28167
				if (Byte == ETX) {
					// if we were in an ETX block, then we should have a single character prefixing the barcode that
					// denotes the type of the barcode. Since we don't really care, we can discard it.
					strBarcode = strBarcode.Mid(1);
				}

				bstr = strBarcode.AllocSysString();
				// (j.jones 2008-12-23 13:24) - PLID 32545 - changed to queue the barcode, instead of
				// directly posting it
				PostMessage(g_hWnd, WM_ENQUEUE_BARCODE, strBarcode.GetLength(), (LPARAM)bstr);

				pszInput = szInput; // Reset string pointer
			}
			else if (Byte == '\n')
			{
				// We ignore \n's since they come after \r's and
				// noone should ever have a \n in their barcode.
			}
			else if (Byte == 127)
			{
				// I don't know why, but sometimes the parity on
				// the barcode scanner 'magically changes' from
				// one value to another. It deserves investigation,
				// but until then, these bytes are ignored because
				// they are interleaved with the barcode.
			}
			else if (Byte == STX) {
				// (a.walling 2007-11-27 10:26) - PLID 28167 - start of text! discard anything we may have already.
				pszInput = szInput;
			}
			else /* Actual input from the scanner */
			{
				// Append the character to our string
				*pszInput++ = Byte;

				// If we have too many bytes in our buffer, dump a copy
				// to the caller and reset our input string.
				if (pszInput - szInput == 250)
				{
					// (a.walling 2007-11-06 17:49) - PLID 27476 - VS2008 - Need a portable solution for allocating the BSTR
					//BSTR bstr = SysAllocString((unsigned short*)szInput);
					BSTR bstr = CString(szInput).AllocSysString();
					PostMessage(g_hWnd, WM_BARCODE_OVERFLOW, 250, (LPARAM)bstr);
					pszInput = szInput; // Reset string pointer
				}
			}
		}

		// We may have broken out of the loop because someone quit
		// the program while something was being scanned.
		if (WaitForSingleObject(g_hPleaseExitThread, 0) != WAIT_TIMEOUT) break;
	}

	// Restore the port settings we entered with
	SetCommState (g_hPort, &DefaultDCB);

	// (a.walling 2012-12-24 13:41) - PLID 54321 - Don't do this now -- we may have broken out while an async read still in progress!
	//CloseHandle(ol.hEvent); // Close the comm event	

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
	CancelIo(g_hPort);
	// The old comment was "(c.haag 2004-10-07 17:54) - We already close the port at Barcode_Close" 
	// which is not valid anymore, because we are back to closing the port here (see comment above)

	// (a.walling 2012-12-24 13:41) - PLID 54321 - If we had a read, wait for all cancelled IO operations to complete
	if (bReadInProgress) {
		WaitForSingleObject(ol.hEvent, 5000);
	}

	// (a.walling 2012-12-24 13:41) - PLID 54321 - Now that IO is cancelled, we can close the event
	CloseHandle(ol.hEvent); // Close the comm event	

	// (b.cardillo 2005-05-02 15:36) - PLID 15568 - Closing the handle atomically.
	WaitForSingleObject(g_hPortClosingMutex, 5000);
	if (g_hPort) {
		// Close the port
		if (CloseHandle(g_hPort)) {
			g_hPort = NULL;
		}
	}
	ReleaseMutex(g_hPortClosingMutex);
	
    return 0;
} 
 
// (a.walling 2007-05-10 15:52) - PLID 25171 - Check for barcode collisions. At the moment, the inventory and cpt code dialogs
// do not allow you to have any barcode collisions. However I've decided to allow duplicate barcodes for coupons, if the user so
// desires, as long as we prompt them. I find it unlikely but it's a situation we need to handle.
BOOL Barcode_CheckCollisions(CWnd* pNotifyWnd, CString &strBarcode, BOOL bForceUnique /* = FALSE*/) // check if the barcode string collides with any other barcodes
{
	try {
		if (strBarcode.GetLength() == 0) {
			// It was stupid of me not to check for this initially.
			// Zero length barcodes = No barcode, no problem, so return TRUE.
			return TRUE;
		}

		long nObjectCount = 0;
		CString strMessage = FormatString("The barcode '%s' already exists on the following objects:\r\n\r\n", strBarcode);

		CString strCoupons, strProducts, strServices;

		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT Description, CASE WHEN EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 0 ELSE 1 END AS Expired FROM CouponsT WHERE BarCode = '%s'", _Q(strBarcode));
		if (!prs->eof) {
			strMessage += "Coupons: \r\n";

			while (!prs->eof) {
				BOOL bExpired = AdoFldLong(prs, "Expired", 0);
				CString strExpired = bExpired ? " (Expired)" : "";

				strMessage += FormatString("\t%s%s\r\n", AdoFldString(prs, "Description", "<No Coupon Description>"), strExpired);
				
				nObjectCount++;
				prs->MoveNext();
			}
		}
		prs->Close();

		prs = CreateRecordset("SELECT Code, Name, Active, CASE WHEN ProductT.ID IS NULL THEN 1 ELSE 0 END AS IsService FROM ServiceT LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE BarCode = '%s'", _Q(strBarcode));
		if (!prs->eof) {
			while (!prs->eof) {
				BOOL bActive = AdoFldBool(prs, "Active", 1);
				BOOL bIsService = AdoFldLong(prs, "IsService", 1);
				CString strName = AdoFldString(prs, "Name", "<No Description>");
				CString strCode = AdoFldString(prs, "Code", "<No Code>");
				CString strActive = bActive ? "" : " (Inactive)";

				if (bIsService) {
					// cptcode
					if (strServices.GetLength() == 0) {
						strServices = "Service Codes: \r\n";
					}

					strServices += FormatString("\t(%s) %s%s\r\n", strCode, strName, strActive);
				} else {
					// product
					if (strProducts.GetLength() == 0) {
						strProducts = "Products: \r\n";
					}

					strProducts += FormatString("\t%s%s\r\n", strName, strActive);
				}

				nObjectCount++;
				prs->MoveNext();
			}
		}

		if (strCoupons.GetLength())
			strMessage += strCoupons;
		
		if (strProducts.GetLength())
			strMessage += strProducts;
		
		if (strServices.GetLength())
			strMessage += strServices;

		if (!bForceUnique) {
			strMessage += "\r\nThese conflicts may cause confusion in the billing module. Products and Service Codes have highest priority, followed by Coupons. It is recommended that you resolve these conflicts if possible.\r\n\r\nWould you like to continue?";

			if (nObjectCount > 0) {
				if (pNotifyWnd && pNotifyWnd->GetSafeHwnd() != NULL) {
					return IDYES == pNotifyWnd->MessageBox(strMessage, NULL, MB_YESNO);
				} else {
					return IDYES == AfxMessageBox(strMessage, MB_YESNO);
				}
			}

			return TRUE;
		} else {
			strMessage += "\r\nThese conflicts must be resolved.";

			if (nObjectCount > 0) {
				if (pNotifyWnd && pNotifyWnd->GetSafeHwnd() != NULL) {
					pNotifyWnd->MessageBox(strMessage, NULL, MB_OK);
				} else {
					return IDYES == AfxMessageBox(strMessage, MB_OK);
				}
			}

			return FALSE;
		}
	} NxCatchAll("Error checking barcode conflicts!");

	return FALSE;
}

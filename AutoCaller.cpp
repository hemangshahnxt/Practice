#include "stdafx.h"
#include <tapi.h>
#include <mmsystem.h>
#include "AutoCaller.h"

/******************************
*****  Auto Phone Dialer  *****
******************************/

//variables
//files
CString	m_strNumber;

CString	m_strMsg;
CString	m_strStartMsg;
CString	m_strUser0;
CString	m_strUser1;
CString	m_strUser2;
CString	m_strUser3;
CString	m_strUser4;
CString	m_strUser5;
CString	m_strUser6;
CString	m_strUser7;
CString	m_strUser8;
CString	m_strUser9;
CString m_strRings;
CString m_strResponseValue;
long m_nPreviousPatient = -1;

// TAPI constants; command line settable
DWORD dwDeviceID = 0;
DWORD dwAddressID = 0;

// TAPI global variables.
HINSTANCE hInstance;
HLINEAPP hLineApp = 0;
LINEEXTENSIONID LineExtensionID;
DWORD dwNumDevs;
DWORD dwAPIVersion;
HLINE hLine;
HCALL hCall = 0;
long lRet;
char szPhoneNumber[1024] = "45776";

LPVARSTRING      lpVarString;
LPLINEDEVSTATUS  lpLineDevStatus;
LPLINECALLINFO   lpLineCallInfo;
LPLINECALLPARAMS lpLineCallParams;

// State machine information.
DWORD dwMakeCallAsyncID=0;
DWORD dwLineDropAsyncID=0;
BOOL bDropped = FALSE;
BOOL bConnected = FALSE;
BOOL bAnswered  = FALSE;
BOOL bReadyToEnd = FALSE;
BOOL bPrintedEnd = FALSE;
BOOL bStarted = FALSE;
BOOL bResponse = FALSE;
BOOL bFinished = FALSE;

// Variables so we can ^C to shutdown and clean up properly.
DWORD dwThreadID;
HANDLE hSleep;

// Used to format data into.
char szBuff[4096];

// /// Waveaudio content ///
UINT WaveInID = 0;
UINT WaveOutID = 0;
BOOL bWaveLocal = FALSE;
char szFileName[1024] = "greeting.wav";
DWORD dwWaveBuffers = 50;
DWORD dwWaveBuffSize = 1024;

DWORD dwWaveMapped = WAVE_MAPPED;

HWAVEIN hWaveIn = NULL;
HWAVEOUT hWaveOut = NULL;
LPWAVEHDR   lpWaveHdr  = NULL;
LPSTR    lpData      = NULL;     // waveform data block
LPSTR    lpstrLoadStrBuf   = NULL;
HMMIO          hmmio;
MMCKINFO       mmckinfoParent;
MMCKINFO       mmckinfoSubchunk;
DWORD          dwFmtSize;
WAVEFORMATEX   *pFormat;
DWORD          dwDataSize;
LPSTR          hpch1, hpch2;
WORD           wBlockSize;

//handles the entire process for calling the phone number provided
void StartAutoCaller(CString strPhoneNumber)
{

	bFinished = FALSE;
	bResponse = FALSE;
	bStarted = FALSE;
	m_strResponseValue = "";

	//copy in the phone number and starting message to be called
	strcpy(szPhoneNumber, strPhoneNumber);
	strcpy(szFileName, m_strStartMsg);

	//setup global vars
   lpVarString      = (VARSTRING*)LocalAlloc(LPTR, 1024);
   lpLineDevStatus  = (LINEDEVSTATUS*)LocalAlloc(LPTR, 4096);
   lpLineCallInfo   = (LINECALLINFO*)LocalAlloc(LPTR, 4096);
   lpLineCallParams = (LINECALLPARAMS*)LocalAlloc(LPTR, 4096);

   lpVarString      -> dwTotalSize = 1024;
   lpLineDevStatus  -> dwTotalSize = 4096;
   lpLineCallInfo   -> dwTotalSize = 4096;

   lpLineCallParams -> dwTotalSize = 4096;
   lpLineCallParams -> dwBearerMode       = LINEBEARERMODE_VOICE;
   lpLineCallParams -> dwMinRate          = 300;
   lpLineCallParams -> dwMaxRate          = 64000;
   lpLineCallParams -> dwMediaMode        = LINEMEDIAMODE_AUTOMATEDVOICE,
   lpLineCallParams -> dwCallParamFlags   = 0;
   lpLineCallParams -> dwAddressMode      = LINEADDRESSMODE_ADDRESSID;
   lpLineCallParams -> dwAddressID        = dwAddressID;

   //initialize the line
   if (lRet = lineInitialize(&hLineApp, hInstance, lineCallbackFunc, 
      "NexTech Practice", &dwNumDevs))
   {
      AfxMessageBox("lineInitialize failed");
      return;
   }

   //what version of tapi are we using?
   if (lRet = lineNegotiateAPIVersion(hLineApp, dwDeviceID, 
      0x00010004, 0x00010004, &dwAPIVersion, &LineExtensionID))
   {
      if (lRet = LINEERR_INCOMPATIBLEAPIVERSION)
      {
         if (lRet = lineNegotiateAPIVersion(hLineApp, dwDeviceID, 
            0x00010003, 0x0FFF0FFF, &dwAPIVersion, &LineExtensionID))
         {
            AfxMessageBox("lineNegotiateAPIVersion failed");
            return;
         }
      }
      else
      {
         AfxMessageBox("lineNegotiateAPIVersion failed");
         return;
      }
   }

   if (lRet = lineOpen(hLineApp, dwDeviceID, &hLine, dwAPIVersion, 0, 0,
            LINECALLPRIVILEGE_NONE, 0, NULL))
   {
      AfxMessageBox("lineOpen failed");
      return;
   }
   
   while (TRUE)
   {
      if (lRet = lineGetLineDevStatus(hLine, lpLineDevStatus))
      {
         AfxMessageBox("lineGetLineDevStatus failed");
         return;
      }

      if (lpLineDevStatus->dwNeededSize > lpLineDevStatus->dwTotalSize)
      {
         LocalReAlloc(lpLineDevStatus, lpLineDevStatus->dwNeededSize, LMEM_MOVEABLE);
         lpLineDevStatus->dwTotalSize = lpLineDevStatus->dwNeededSize;
         continue;
      }
      break;
   }

   if (lpLineDevStatus -> dwOpenMediaModes)
      printf("!!!WARNING!!!  Another application is already waiting for calls.\n\n");

   if (!((lpLineDevStatus -> dwLineFeatures) & LINEFEATURE_MAKECALL))
      printf("!!!WARNING!!!  No call appearances available at this time.\n\n");

  lRet = lineMakeCall(hLine, &hCall, szPhoneNumber, 0, lpLineCallParams);
  if (lRet < 0)
  {
     AfxMessageBox("lineMakeCall failed");
     return;
  }
  else
  {
     dwMakeCallAsyncID = lRet;
  }

   //need to wait until lineMakeCall returns a valid hCall before this function will work
   Sleep(3000);
   
   long nDigit = lineMonitorDigits(hCall, LINEDIGITMODE_DTMF );

	switch (nDigit) {
		case LINEERR_INVALCALLHANDLE:
			AfxMessageBox("LINEERR_INVALCALLHANDLE");
		break;
		case LINEERR_OPERATIONUNAVAIL:
			AfxMessageBox("LINEERR_OPERATIONUNAVAIL");
		break;
		case LINEERR_INVALCALLSTATE:
			AfxMessageBox("LINEERR_INVALCALLSTATE");
		break;
		case LINEERR_OPERATIONFAILED:
			AfxMessageBox("LINEERR_OPERATIONFAILED");
		break;
		case  LINEERR_INVALDIGITMODE:
			AfxMessageBox("LINEERR_INVALDIGITMODE");
		break;
		case LINEERR_RESOURCEUNAVAIL:
			AfxMessageBox("LINEERR_RESOURCEUNAVAIL");
		break;
		case LINEERR_NOMEM:
			AfxMessageBox("LINEERR_NOMEM");
		break;
		case LINEERR_UNINITIALIZED:
			AfxMessageBox("LINEERR_UNINITIALIZED");
		break;
	}

}

// Here's the TAPI callback.  Mondo switch statement!
void CALLBACK lineCallbackFunc(
    DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance, 
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
   switch(dwMsg)
   {
   case LINE_MONITORDIGITS:
   {
	   if(!bStarted && char(LOWORD(dwParam1)) == '*'){
		   bStarted = TRUE;		//the user has heard the opening file, now we can play the other file
		   strcpy(szFileName, m_strMsg);
	   }

	   if(bStarted && char(LOWORD(dwParam1)) == '0'){
		   strcpy(szFileName, m_strUser0);
		   bResponse = TRUE;
		   m_strResponseValue = "0";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '1'){
		   strcpy(szFileName, m_strUser1);
		   bResponse = TRUE;
		   m_strResponseValue = "1";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '2'){
		   strcpy(szFileName, m_strUser2);
		   bResponse = TRUE;
		   m_strResponseValue = "2";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '3'){
		   strcpy(szFileName, m_strUser3);
		   bResponse = TRUE;
		   m_strResponseValue = "3";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '4'){
		   strcpy(szFileName, m_strUser4);
		   bResponse = TRUE;
		   m_strResponseValue = "4";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '5'){
		   strcpy(szFileName, m_strUser5);
		   bResponse = TRUE;
		   m_strResponseValue = "5";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '6'){
		   strcpy(szFileName, m_strUser6);
		   bResponse = TRUE;
		   m_strResponseValue = "6";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '7'){
		   strcpy(szFileName, m_strUser7);
		   bResponse = TRUE;
		   m_strResponseValue = "7";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '8'){
		   strcpy(szFileName, m_strUser8);
		   bResponse = TRUE;
		   m_strResponseValue = "8";
	   }
	   if(bStarted && char(LOWORD(dwParam1)) == '9'){
		   strcpy(szFileName, m_strUser9);
		   bResponse = TRUE;
		   m_strResponseValue = "9";
	   }
	   	   
   }   break;

      case LINE_LINEDEVSTATE:
		  AfxMessageBox("LINE_LINEDEVSTATE");
         if (dwParam1 == LINEDEVSTATE_REINIT)
         {
            printf("LINEDEVSTATE_REINIT\n");
            StopEverything();
         }
		 if(dwParam1 == LINEDEVSTATE_RINGING)
		 {	//I've never been able to get this message
		 }
			 
         break;

      case LINE_REPLY:
         if (dwParam2 == dwLineDropAsyncID)
         {
            if (dwParam2 != 0)
            {
               AfxMessageBox("Failure in LINE_REPLY.  Closing this attempt to call");
               StopEverything();
            }
         }
         else if (dwParam2 == dwMakeCallAsyncID)
         {
            if (dwParam2 != 0)
            {
               AfxMessageBox("lineMakeCall LINE_REPLY with failure");
               StopEverything();
            }
         }

         // else ignore it.
         break;

      case LINE_CALLSTATE:
      {
         // Is this a new call?
         if (dwParam3 == LINECALLPRIVILEGE_OWNER)
         {
            // Do we already have a call?
            if (hCall && (hCall != (HCALL) dwDevice))
            {
               if (dwMsg == LINECALLSTATE_IDLE)
	           break;
            }

            if (hCall)
               printf("Given OWNER privs to a call already owned?\n"
                      " - Should only happen if handed a call already owned.\n");
            else
            {
               hCall = (HCALL) dwDevice;
			   // (d.thompson 2015-10-14 10:48) - PLID 67284 - Bad code in AutoCaller giving too many parameters to printf() statement.
               printf("New incoming hCall 0x%lx on TAPI Line Device.\r\n",
                  hCall);
            }
         }

         if (hCall != (HCALL) dwDevice)
         {
            if (dwMsg == LINECALLSTATE_IDLE)
               lineDeallocateCall((HCALL) dwDevice);
            else
            {
               lineDrop((HCALL) dwDevice, NULL, 0);
               printf("LINE_CALLSTATE 0x%lx for non-main hCall: 0x%lx.  Dropping.\n", 
                  dwParam1, dwDevice);
            }
            break;
         }

         switch (dwParam1)
         {
            case LINECALLSTATE_IDLE:
            {
			   lRet = lineDeallocateCall(hCall);

			   // Should make sure lineDeallocateCall succeeded
			   hCall = 0;
			   StopEverything();

			   ShutdownLine();
               break;
            }

            case LINECALLSTATE_BUSY:
//				AfxMessageBox("The line is busy");
				break;

			case LINECALLSTATE_RINGBACK:
//				AfxMessageBox("The line is ringing");
				break;

			case LINECALLSTATE_DIALING:
			{
				//the thread will play the first wav file until a * is pressed on the phone
				CWinThread *pThread = AfxBeginThread(PlayLoop, 0);
			}
				break;

            case LINECALLSTATE_DISCONNECTED:

               if (!bDropped){
				   dwLineDropAsyncID = lineDrop(hCall, NULL, 0);
			   }
               if (dwLineDropAsyncID < 0)
               {
                  printf("lineDrop failed.  Terminating\n");
                  hCall = 0;
                  StopEverything();
               }

			   	//close down the line and free memory
				ShutdownLine();

				bFinished = TRUE;

				bDropped = TRUE;
				break;


            case LINECALLSTATE_CONNECTED:
               if (!bConnected)
               {
                  bConnected = TRUE;
                  printf("CONNECTED.\n");

             }
               break;
         }

         break;
      }
   }
}

//Created this function from pieces of the DoConnectedStuff() function
//plays the wave file to the modem, handling all creation and deletion of handles
bool PlayFile()
{

	MMRESULT mmResult;
	
	//descend into the wav file, setup the wav out handles, up to the point where we prepare the headers
	if(!SetupWaveFile())
		return false;

   // Allocate a waveform data header. The WAVEHDR must be
   // globally allocated and locked.
   lpWaveHdr = (LPWAVEHDR)LocalAlloc(LPTR, (DWORD) sizeof(WAVEHDR));
   if (!lpWaveHdr)
   {
      printf("Memory issues.\n");
      StopConnectedStuff();
      return FALSE;
   }

   // Set up WAVEHDR structure and prepare it to be written to wave device.
   lpWaveHdr->lpData = lpData;
   lpWaveHdr->dwBufferLength = dwDataSize;
   lpWaveHdr->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
   lpWaveHdr->dwLoops = 5;

   CString str;
   str.Format("dwDatasize %lu\n",dwDataSize);
//   AfxMessageBox(str);
   Sleep(0);

   // lpWaveHdr->dwUser = (DWORD) lpYourData; // save instance data ptr
   if(mmResult = waveOutPrepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR)))
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("waveOutPrepareHeader returned %s\r\n", szBuff);
      StopConnectedStuff();
      return FALSE;
   }

   //calculate the time it will take (milliseconds)
   long nLength = long((float(lpWaveHdr->dwBufferLength) / float(pFormat->nAvgBytesPerSec)) * 1000);
   CString temp;
   temp.Format("Bytes: %li\nBytes/Sec:  %li\nLength:  %li", lpWaveHdr->dwBufferLength, pFormat->nAvgBytesPerSec, nLength);
//   AfxMessageBox(temp);

	//allow the header to get setup	- don't need this apparently
//   Sleep(2000);

   // Then the data block can be sent to the output device.
   if (mmResult = waveOutWrite(hWaveOut, lpWaveHdr, sizeof(WAVEHDR)))
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("waveOutWrite returned %s\r\n", szBuff);
   }
   else
   {
      printf("waveOutWrite succeeded!.\n");
	//we have to wait the length of the wav file (I don't know why) ... 
	  Sleep(nLength);
   }

	ShutdownWaveFile();

   return true;
}

//thread to play the wav files in 2 loops until their control values are changed
UINT PlayLoop(LPVOID pParam)
{

/*	while(!bStopped)
	{
		PlayFile();
	}
*/	
	while(!bStarted)
		PlayFile();		//play the opening file to the phone
	

	//play the recorded message
	PlayFile();

	//wait for a response
//	while(!bResponse){
//		Sleep(0);
//	}

	bResponse = false;

	//check every 50 ms to see if they've responded ... if they haven't within 10 seconds, give up
	// (a.walling 2007-11-05 13:21) - PLID 27974 - VS2008 - for() loops
	int i = 0;
	for(i = 0; i < 10000 && !bResponse; i+= 50)
		Sleep(50);

	if(i == 10000)		//no response, no sense playing the rest of these
		return 0;

	//play the response message
	PlayFile();

	//say goodbye
	strcpy(szFileName, "c:\\don\\calltest\\goodbye.wav");
	PlayFile();

	return 0;

}

bool SetupWaveFile()
{

   MMRESULT mmResult;

   if (!bWaveLocal)
   {
	   if (lRet = lineGetID(0, 0, hCall, LINECALLSELECT_CALL, 
         (LPVARSTRING) lpVarString, "wave/out"))
      {
//         AfxMessageBox("lineGetID failed - Make sure your modem is plugged in and no other devices are using the phone line");
         return FALSE;
      }

      WaveOutID = *((LPUINT)((LPBYTE) lpVarString +
                    (lpVarString -> dwStringOffset)));
   }

   // Open the given file for reading using buffered I/O.
   if(!(hmmio = mmioOpen(szFileName, NULL, MMIO_READ | MMIO_ALLOCBUF)))
   {
      printf("mmioOpen failed to open file.\n");
      return FALSE;
   }

   // Locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
   mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
   if (mmResult = mmioDescend(hmmio, &mmckinfoParent, NULL, MMIO_FINDRIFF))
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("mmioDescend RIFF WAVE returned %s\r\n", szBuff);
      mmioClose(hmmio, 0);
      return FALSE;
   }

   //  Now, find the format chunk (form type 'fmt '). It should be
   //  a subchunk of the 'RIFF' parent chunk.
   mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
   if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
      MMIO_FINDCHUNK))
   {
      printf("Wave file corrupt.\n");
      mmioClose(hmmio, 0);
      return FALSE;
   }

   // Get the size of the format chunk, allocate and lock memory for it.
   dwFmtSize = mmckinfoSubchunk.cksize;
   pFormat = (WAVEFORMATEX*)LocalAlloc(LPTR, LOWORD(dwFmtSize));

  if (!pFormat)
   {
      printf("Memory error.\n");
      mmioClose(hmmio, 0);
      return FALSE;
   }

   // Read the format chunk.
   if (mmioRead(hmmio, (HPSTR) pFormat, dwFmtSize) != (LONG) dwFmtSize)
   {
      printf("mmioRead: failed to read FMT chunk.\n");
      LocalFree( pFormat );
      mmioClose(hmmio, 0);
      return FALSE;
   }

   printf("wFormatTag = %lu\n",        (DWORD) pFormat->wFormatTag);
   printf("nChannels = %lu\n",         (DWORD) pFormat->nChannels );
   printf("nSamplesPerSec = %lu\n",    (DWORD) pFormat->nSamplesPerSec);
   printf("nAvgBytesPerSec = %lu\n",   (DWORD) pFormat->nAvgBytesPerSec);
   printf("nBlockAlign = %lu\n",       (DWORD) pFormat->nBlockAlign);
   printf("wBitsPerSample = %lu\n",    (DWORD) pFormat->wBitsPerSample);
   printf("cbSize = %lu\n",            (DWORD) pFormat->cbSize);

   // TESTING THE DEVICE
   // Make sure a waveform output device supports this format.
   if (WAVE_MAPPER == WaveOutID)
      mmResult = waveOutOpen(&hWaveOut, WaveOutID, pFormat, 0, 0L,
               WAVE_FORMAT_QUERY);
   else
      mmResult = waveOutOpen(&hWaveOut, WaveOutID, pFormat, 0, 0L,
               dwWaveMapped  | WAVE_FORMAT_QUERY);
   if (mmResult)
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("waveOutOpen while QUERY returned %s\r\n", szBuff);
      LocalFree( pFormat );
      mmioClose(hmmio, 0);
      return FALSE;
   }

   // Ascend out of the format subchunk.
   mmioAscend(hmmio, &mmckinfoSubchunk, 0);

   // Find the data subchunk.
   mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
   if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
      MMIO_FINDCHUNK))
   {
      printf("mmioDescend: No DATA chunk.\n");
      LocalFree( pFormat );
      mmioClose(hmmio, 0);
      return FALSE;
   }

   //  Get the size of the data subchunk.
   dwDataSize = mmckinfoSubchunk.cksize;
   if (dwDataSize == 0L)
   {
      printf("Data chunk actually has no data.\n");
      LocalFree( pFormat );
      mmioClose(hmmio, 0);
      return FALSE;
   }
   printf("Size of data is %lu\n",dwDataSize);

   // Open a waveform output device.
   if (WaveOutID == WAVE_MAPPER)
      mmResult = waveOutOpen(&hWaveOut, WaveOutID,
         pFormat, 0 /* (UINT)hwndApp */, 0L, 0/*| CALLBACK_WINDOW*/);
   else
      mmResult = waveOutOpen(&hWaveOut, WaveOutID,
         pFormat, 0 /* (UINT)hwndApp */, 0L, 0 /*WAVE_MAPPED*/ /*| CALLBACK_WINDOW*/);

   if (mmResult)
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("waveOutOpen returned %s\r\n", szBuff);
      LocalFree( pFormat );
      mmioClose(hmmio, 0);
      return FALSE;
   }

   // Save block alignment info for later use.
   wBlockSize = pFormat->nBlockAlign;

   // We're done with the format header, free it.
   //*****LocalFree( pFormat );

   // Allocate and lock memory for the waveform data.
   lpData = (char*)LocalAlloc(LPTR, dwDataSize );
   if (!lpData)
   {
      printf("Memory problems.\n");
      mmioClose(hmmio, 0);
      return FALSE;
   }

   // Read the waveform data subchunk.
   if(mmioRead(hmmio, lpData, dwDataSize) != (LONG) dwDataSize)
   {
      printf("Failed to read waveform data subchunk.\n");
      LocalFree( lpData );
      mmioClose(hmmio, 0);
      return FALSE;
   }

   // We're done with the file, close it.
   mmioClose(hmmio, 0);

   /* If you need instance data for a waveform data block, allocate some
    * memory and store the pointer in lpWaveHdr->dwUser, before the call
    * to waveOutPrepareHeader(). The code inside the #if 0 / #endif, and
    * the commented-out lpWaveHdr->dwUser = ... illustrate this.
    * Don't forget to free the instance memory when you're done with it,
    * or on error bailout.
    */
   #if 0
   lpYourData = LocalAlloc(LPTR, sizeof(YOURDATA));
   if (!lpYourData)
   {
      printf("Memory issues.\n");
      StopConnectedStuff();
      return FALSE;
   }
   #endif
	
   return true;

}


void ShutdownWaveFile()
{

	MMRESULT mmResult;

   //unprepare our header
   if (lpWaveHdr)
   {
      if(mmResult = waveOutUnprepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR) ))
      {
         waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
         printf("waveOutUnprepareHeader returned %s\r\n", szBuff);
      }
      LocalFree( lpWaveHdr );
      lpWaveHdr = NULL;
   }

	//reset the waveOut handle
   if (mmResult = waveOutReset(hWaveOut))
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("waveOutReset returned %s\r\n", szBuff);
   }

   Sleep(500);  // Give it 1/2 sec to actually reset.
   
   //close the waveOut handle
   if(mmResult = waveOutClose(hWaveOut))
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("waveOutClose returned %s\r\n", szBuff);
   }

   Sleep(500); // Give it 1/2 sec to clean up.

   //free the data
   LocalFree( lpData );

}

void StopEverything()
{
   bReadyToEnd = TRUE;

   if (bConnected)
       StopConnectedStuff();

   if (hCall && !bDropped)
   {
      dwLineDropAsyncID = lineDrop(hCall, NULL, 0);
      if (dwLineDropAsyncID < 0)
      {
         printf("lineDrop failed.  Terminating\n");
         hCall = 0;
      }
      bDropped = TRUE;
   }

   // lets prime the pump with a message.
   PostThreadMessage(dwThreadID, WM_USER, 0, 0);
}

void StopConnectedStuff()
{
   MMRESULT mmResult;

   if (lpWaveHdr)
   {
      if(mmResult = waveOutUnprepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR) ))
      {
         waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
         printf("waveOutUnprepareHeader returned %s\r\n", szBuff);
      }
      LocalFree( lpWaveHdr );
      lpWaveHdr = NULL;
   }

   if (mmResult = waveOutReset(hWaveOut))
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("waveOutReset returned %s\r\n", szBuff);
   }

   Sleep(500);  // Give it 1/2 sec to actually reset.
   
   if(mmResult = waveOutClose(hWaveOut))
   {
      waveOutGetErrorText(mmResult, szBuff, sizeof(szBuff));
      printf("waveOutClose returned %s\r\n", szBuff);
   }

   Sleep(500); // Give it 1/2 sec to clean up.

   LocalFree( lpData );
}

void ShutdownLine()
{

   lineShutdown(hLineApp);
   hLineApp = 0;

   LocalFree(lpLineDevStatus);
   LocalFree(lpLineCallInfo);
   LocalFree(lpLineCallParams);
   LocalFree(lpVarString);

}


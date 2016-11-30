#include "stdafx.h"
#include "nxaudiocapture.h"
#include <afxtempl.h>
#include <mmsystem.h>

// (c.haag 2006-09-22 17:43) - PLID 21327 - Initial implementation

// This means that we record at 22050 Hz
#define SAMPLES_PER_SEC			22050

// This means a single recording buffer contains 1 second of data
#define RECORD_BUFFER_DURATION	1

#define NXTM_BASE				WM_USER + 0x1000
#define NXTM_START_RECORDING	NXTM_BASE + 0x0001
#define NXTM_STOP_RECORDING		NXTM_BASE + 0x0002
#define NXTM_CANCEL_RECORDING	NXTM_BASE + 0x0003

void NxPrepareOutputFile(CFile& fileCapture, WAVEFORMATEX& wfmt)
{
	//
	// File format description: 
	//
	// 1. char[4] = "RIFF", The characters "RIFF" indicate the start of the RIFF header 
	// 2. Int32 = FileSize – 8, This is the size of the entire file following this data, i.e., the size of the rest of the file 
	// 3. char[4] = "WAVE", The characters "WAVE" indicate the format of the data. 
	// 4. char[4] = "fmt ", The "fmt " characters specify that this is the section of the file describing the format specifically 
	// 5. Int32 = 16, The size of the WAVEFORMATEX data to follow 
	// WAVEFORMATEX (shown below) 
	//		6. UInt16 wFormatTag, only PCM data is supported in this sample 
	//		7. UInt16 nChannels, Number of channels in (1 for mono, 2 for stereo) 
	//		8. UInt32 nSamplesPerSec, Sample rate of the waveform in samples per second 
	//		9. UInt32 nAvgBytesPerSec, Average bytes per second which can be used to determine the time-wise length of the audio 
	//		10. UInt16 nBlockAlign, Specifies how each audio block must be aligned in bytes 
	//		11. UInt16 wBitsPerSample, How many bits represent a single sample (typically 8 or 16) 
	// 12. char[4] = "data", The "data" characters specify that the audio data is next in the file 
	// 13. Int32, The length of the data in bytes 
	// 14. Data, The rest of the file is the actual samples
	//
	DWORD nInt32;

	// 1. riff text
	fileCapture.Write("RIFF", 4);

	// 2. file size (we are writing 0 now; we will change this later)
	nInt32 = 0;
	fileCapture.Write(&nInt32, sizeof(nInt32));

	// 3. wave text
	fileCapture.Write("WAVE", 4);

	// 4. fmt  text
	fileCapture.Write("fmt ", 4);

	// 5. size of WAVEFORMATEX data
	nInt32 = 16;
	fileCapture.Write(&nInt32, sizeof(nInt32));

	// 6. format tag
	fileCapture.Write(&wfmt.wFormatTag, sizeof(wfmt.wFormatTag));

	// 7. number of channels
	fileCapture.Write(&wfmt.nChannels, sizeof(wfmt.nChannels));

	// 8. samples per second
	fileCapture.Write(&wfmt.nSamplesPerSec, sizeof(wfmt.nSamplesPerSec));

	// 9. average bytes per second
	fileCapture.Write(&wfmt.nAvgBytesPerSec, sizeof(wfmt.nAvgBytesPerSec));

	// 10. block alignment
	fileCapture.Write(&wfmt.nBlockAlign, sizeof(wfmt.nBlockAlign));

	// 11. Bits per sample
	fileCapture.Write(&wfmt.wBitsPerSample, sizeof(wfmt.wBitsPerSample));

	// 12. data text
	fileCapture.Write("data", 4);

	// 13. data size (we will write this later)
	nInt32 = 0;
	fileCapture.Write(&nInt32, sizeof(nInt32));
}

void NxCompleteOutputFile(CFile& fileCapture)
{
//TES 11/7/2007 - PLID 27979 - VS2008 - CFiles report their lengths as ULONGLONGs now.
#if _MSC_VER > 1300
	ULONGLONG ullLength = fileCapture.GetLength();
	
	// (a.walling 2008-10-30 14:26) - PLID 31858 - This would have worked, if we passed
	// sizeof(DWORD) to the Write function. However by changing the variable to ULONGLONG
	// instead of DWORD, we double the size. This was effectively overwriting the 'WAVE'
	// string in the header

	if (ullLength > ULONG_MAX) {
		ThrowNxException("NxCompleteOutputFile() - Cannot be called for files over 4 GB in size.");
	}
	DWORD nLength = (DWORD)ullLength;
	DWORD nInt32 = nLength - 8;
#else
	DWORD nLength = fileCapture.GetLength();
	DWORD nInt32 = nLength - 8;
#endif

	// Update the file size block
	fileCapture.Seek(4, CFile::begin);
	fileCapture.Write(&nInt32, sizeof(nInt32));

	// Update the data size block
	fileCapture.Seek(40, CFile::begin);
	nInt32 = nLength - 44;
	fileCapture.Write(&nInt32, sizeof(nInt32));
}

MMRESULT NxAddRecordingBuffer(HWAVEIN hWaveIn, WAVEHDR*& pWaveInHdr)
{
	//
	// This function creates and prepares a buffer for waveform-audio input
	//
	MMRESULT result;
	const int nBufferSize = SAMPLES_PER_SEC * RECORD_BUFFER_DURATION;
	pWaveInHdr = new WAVEHDR; // This structure defines the header used to identify a waveform-audio buffer
	pWaveInHdr->lpData = (LPSTR)new short[nBufferSize]; // This is where the waveform data will go
	pWaveInHdr->dwBufferLength = nBufferSize * sizeof(short);
	pWaveInHdr->dwBytesRecorded = 0;
	pWaveInHdr->dwUser = 0L;
	pWaveInHdr->dwFlags = 0L;
	pWaveInHdr->dwLoops = 0L;

	//
	// Prepare the buffer for recording
	//
	if (MMSYSERR_NOERROR != (result = waveInPrepareHeader(hWaveIn, pWaveInHdr, sizeof(WAVEHDR)))) {
		delete [] (short*)pWaveInHdr->lpData;
		delete pWaveInHdr;
		pWaveInHdr = NULL;
		return result;
	}

	//
	// Now add the buffer to the stream
	//
	if (MMSYSERR_NOERROR != (result = waveInAddBuffer(hWaveIn, pWaveInHdr, sizeof(WAVEHDR)))) {
		waveInUnprepareHeader(hWaveIn, pWaveInHdr, sizeof(WAVEHDR));
		delete [] (short*)pWaveInHdr->lpData;
		delete pWaveInHdr;
		pWaveInHdr = NULL;
		return result;
	}

	//
	// Success
	//
#ifdef _DEBUG
	char szErr[256];
	_snprintf(szErr, 256, "Wavein header 0x%08x created\n", pWaveInHdr);
	TRACE(szErr);
#endif
	return MMSYSERR_NOERROR;
}

void NxReleaseAllWaveHeaders(HWAVEIN hWaveIn, CArray<WAVEHDR*,WAVEHDR*>& apWaveHeaders)
{
	//
	// This function goes through the array of allocated waveform headers and buffers,
	// and releases them all. This must be done before we can cleanly close the input
	// stream
	//
	for (int i=0; i < apWaveHeaders.GetSize(); i++) {
		WAVEHDR* pWaveInHdr = apWaveHeaders[i];
		waveInUnprepareHeader(hWaveIn, pWaveInHdr, sizeof(WAVEHDR));
		delete [] (short*)pWaveInHdr->lpData;
		delete pWaveInHdr;
	}
	apWaveHeaders.RemoveAll();
}

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	DWORD nThreadID = dwInstance;
	PostThreadMessage(nThreadID, uMsg, dwParam1, dwParam2);
}

UINT NxAudioCapture_Thread(LPVOID p)
{
	CNxAudioCapture* pCapture = (CNxAudioCapture*)p; // The source audio capture object
	CString strAudioFileName = pCapture->GetAudioFileName(); // The final output file to write to when the recording is done
	const DWORD nMaxRecordingLength = pCapture->GetMaxRecordingDuration(); // Maximum recording duration in seconds
	CArray<WAVEHDR*,WAVEHDR*> apWaveHeaders; // Allocated recording buffers
	CFile fileCapture; // The file that the recording data is streamed to
	CWnd* pWndNotify = pCapture->GetNotificationWnd(); // The notification window to post NexTech messages to
	WAVEHDR* pWaveInHdr = NULL; // temporary variable
	WAVEFORMATEX wfmt; // The wave file format descriptor
	HWAVEIN hWaveIn = NULL; // Handle to the recording stream
	MMRESULT result; // temporary variable
	BOOL bShuttingDown = FALSE; // True if we are ending a recording
	BOOL bSaveRecording = FALSE; // True if we should save the recording when done
	long nAllocatedBuffers = 0; // The number of times we allocated a recording buffer
	DWORD nStartTime; // The time that the recording started (Resolution is milliseconds)
	DWORD nTotalSize = 0; // The total size, in bytes, of data that was recorded
	char szErr[256]; // temporary variable
	MSG msg; // temporary variable
	int i; // temporary variable

	//
	// Specify recording parameters for the input stream
	//
	wfmt.wFormatTag = WAVE_FORMAT_PCM;     // simple, uncompressed format
	wfmt.nChannels = 1;                    // 1=mono, 2=stereo
	wfmt.nSamplesPerSec = SAMPLES_PER_SEC; // SAMPLES_PER_SEC Hz
	wfmt.nBlockAlign = 2;                  // = n.Channels * wBitsPerSample/8
	wfmt.wBitsPerSample = 16;              // 16 for high quality, 8 for telephone-grade
	wfmt.cbSize = 0;					   // Size, in bytes, of extra format information appended to the end of the WAVEFORMATEX structure
	wfmt.nAvgBytesPerSec = wfmt.nSamplesPerSec * wfmt.nChannels * (wfmt.wBitsPerSample / 8);

	//
	// We need to create the message queue before we initialize the wave input stream because
	// it will post messages to the thread. To guarantee it is created, we post the first message
	//
	PostThreadMessage(GetCurrentThreadId(), NXTM_START_RECORDING, 0, 0);

	//
	// Now run the main thread message pump
	//
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch (msg.message) {
		case NXTM_START_RECORDING:
			try {
				//
				// Fail if there are no recording devices on the computer
				//
				if (0 == waveInGetNumDevs()) {
					MsgBox("NexTech Practice could not detect any recording devices on your computer. The recording session will be terminated.");
					if (pWndNotify) pWndNotify->PostMessage(NXM_AUDIO_RECORDING_TERMINATED);
					return 1;			
				}

				//
				// Try opening the output file for write access. We begin by streaming audio
				// into the user's NxTemp path. When a recording is successfully finished, we
				// will move the file to the final location
				//
				if (!fileCapture.Open(strAudioFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareCompat)) {
					CString strErr;
					FormatLastError(strErr, "Could not open '%s' for writing", strAudioFileName);
					ThrowNxException(strErr);
				}

				//
				// Now write the standard waveform header to the file. This function throws
				// a CFile exception on failure
				//
				NxPrepareOutputFile(fileCapture, wfmt);

				//
				// Open the input stream for recording
				//
				if (MMSYSERR_NOERROR != (result = waveInOpen(&hWaveIn, WAVE_MAPPER, &wfmt, (DWORD)waveInProc, GetCurrentThreadId(), WAVE_FORMAT_DIRECT | CALLBACK_FUNCTION)))
				{
					waveInGetErrorText(result, szErr, 256);
					ThrowNxException(szErr);
				}

				//
				// Now that we have a handle to the input stream, we need to create
				// recording buffers. Here's how it works: We begin by creating two
				// recording buffers. When one buffer is filled, we will be notified
				// in a callback function. When notified, we will create a new recording
				// buffer, and add it to the stream. The process will repeat for as long
				// as we like.
				//
				for (i=0; i < 2; i++) {
					if (MMSYSERR_NOERROR != (result = NxAddRecordingBuffer(hWaveIn, pWaveInHdr))) {
						waveInGetErrorText(result, szErr, 256);
						ThrowNxException(szErr);
					} else {
						apWaveHeaders.Add(pWaveInHdr);
					}
					nAllocatedBuffers++;
				}

				//
				// Now that our input buffers have been added to the stream, begin
				// recording
				//
				if (MMSYSERR_NOERROR != (result = waveInStart(hWaveIn))) {
					waveInGetErrorText(result, szErr, 256);
					ThrowNxException(szErr);
				}
				nStartTime = GetTickCount();
			}
			// (a.walling 2007-07-20 11:05) - PLID 26762 - Changed to use threaded exception handling
			NxCatchAllCallThread("Error in NXTM_START_RECORDING", 
				if (hWaveIn) {
					NxReleaseAllWaveHeaders(hWaveIn, apWaveHeaders);
					waveInClose(hWaveIn);
				}
				if (pWndNotify) pWndNotify->PostMessage(NXM_AUDIO_RECORDING_TERMINATED);
				return 2;
			);
			break;

		case NXTM_STOP_RECORDING:
			//
			// This message is posted when the application wants to end and save
			// the recording
			//
			// Stop the recording and set the shutdown flag. The act of calling waveInReset
			// will invoke the WIM_DATA message
			//
			bShuttingDown = TRUE;
			bSaveRecording = TRUE;
			waveInReset(hWaveIn);
			break;

		case NXTM_CANCEL_RECORDING:
			//
			// This message is posted when the application wants to end and NOT save
			// the recording
			//
			// Stop the recording and set the shutdown flag. The act of calling waveInReset
			// will invoke the WIM_DATA message
			//
			bShuttingDown = TRUE;
			bSaveRecording = FALSE;
			waveInReset(hWaveIn);
			break;

		case WIM_OPEN: // Currently ignored
			break;
		case WIM_CLOSE: // Currently ignored
			break;
		case WIM_DATA:
			try {
#ifdef _DEBUG
				_snprintf(szErr, 256, "Wavein header 0x%08x saturated\n", msg.wParam);
				TRACE(szErr);
#endif
				//
				// The WIM_DATA message is sent to the given waveform-audio input callback
				// function when waveform-audio data is present in the input buffer and the
				// buffer is being returned to the application. The message can be sent when
				// the buffer is full or after the waveInReset function is called.
				//

				//
				// If we are at the maximum time allotment for recording, we must begin shutting down
				//
				if (!bShuttingDown && nAllocatedBuffers >= ((long)nMaxRecordingLength / RECORD_BUFFER_DURATION) + 1) {
					bShuttingDown = TRUE;
					bSaveRecording = TRUE;
					waveInReset(hWaveIn);
				}

				//
				// If we are shutting down, then perform cleanup code
				//
				if (bShuttingDown) {
					//
					// Save the samples that were recorded between the time recording started
					// in the active buffer and now
					//
					pWaveInHdr = (WAVEHDR*)msg.wParam;
					if (pWaveInHdr->dwBytesRecorded > 0) {
						fileCapture.Write(pWaveInHdr->lpData, pWaveInHdr->dwBytesRecorded);
						nTotalSize += pWaveInHdr->dwBytesRecorded;
						if (pWndNotify) {
							pWndNotify->PostMessage(NXM_AUDIO_RECORDING_PROGRESS, GetTickCount() - nStartTime, nTotalSize);
						}
					}

					//
					// Have the thread terminate
					//
					PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
				} 
				//
				// If we are not shutting down and we get this message, we need to create a
				// new buffer and append it to the stream. Then we must unprepare the old buffer,
				// save the data to disk, then delete the buffer. 
				//			
				else {
					//
					// Add a new recording buffer first
					//
					if (MMSYSERR_NOERROR != (result = NxAddRecordingBuffer(hWaveIn, pWaveInHdr))) {
						waveInGetErrorText(result, szErr, 256);
						ThrowNxException(szErr);
					} else {
						apWaveHeaders.Add(pWaveInHdr);
						nAllocatedBuffers++;
					}

					//
					// Now write the contents of the finished buffer to disk
					//
					pWaveInHdr = (WAVEHDR*)msg.wParam;
					if (pWaveInHdr->dwBytesRecorded > 0) {
						fileCapture.Write(pWaveInHdr->lpData, pWaveInHdr->dwBytesRecorded);
						nTotalSize += pWaveInHdr->dwBytesRecorded;
						if (pWndNotify) {
							pWndNotify->PostMessage(NXM_AUDIO_RECORDING_PROGRESS, GetTickCount() - nStartTime, nTotalSize);
						}
					} else {
						ASSERT(FALSE); // This should never happen
					}

					//
					// Now delete the saturated buffer
					//
					if (MMSYSERR_NOERROR != (result = waveInUnprepareHeader(hWaveIn, pWaveInHdr, sizeof(WAVEHDR)))) {
						waveInGetErrorText(result, szErr, 256);
						ThrowNxException(szErr);
					}
					// Remove the wave header from the array. The saturated recording header
					// will always be the first element in the list.
					apWaveHeaders.RemoveAt(0);
					delete [] (short*)pWaveInHdr->lpData;
					delete pWaveInHdr;
					break;
				}
			}
			// (a.walling 2007-07-20 11:05) - PLID 26762 - Changed to use threaded exception handling
			NxCatchAllCallThread("Error in WIM_DATA",
				if (hWaveIn) {
					// we can't call waveInReset because it may cause WIM_DATA to fire again,
					// so we'll use waveInStop instead
					waveInStop(hWaveIn);
					NxReleaseAllWaveHeaders(hWaveIn, apWaveHeaders);
					waveInClose(hWaveIn);
					// Close the output file
					try {
						// (a.walling 2007-11-06 17:35) - PLID 27998 - VS2008 - Use INVALID_HANDLE_VALUE rather than 0xFFFF,FFFF explicitly
						if (fileCapture.m_hFile != NULL && (HANDLE)fileCapture.m_hFile != INVALID_HANDLE_VALUE) {
							fileCapture.Close();
						}
					}
					catch (...)
					{
					}
				}
				if (pWndNotify) pWndNotify->PostMessage(NXM_AUDIO_RECORDING_TERMINATED);
				return 3;
			);
			break;
		}

		if (msg.message == WM_QUIT)
			break;
	}

	//
	// Now finish up with the file. If we are to save the recording, then we need to
	// update the size fields in the file content by calling NxCompleteOutputFile,
	// then close the file. If not, we simply close and delete the file. An exception
	// will be thrown if anything goes wrong.
	// 
	try {
		if (bSaveRecording) {
			NxCompleteOutputFile(fileCapture);
			fileCapture.Close();
		} else {
			fileCapture.Close();
			DeleteFile(strAudioFileName);
		}
	}
	// (a.walling 2007-07-20 11:05) - PLID 26762 - Changed to use threaded exception handling
	NxCatchAllCallThread("Error saving recording",
		try {
			// (a.walling 2007-11-06 17:35) - PLID 27998 - VS2008 - Use INVALID_HANDLE_VALUE rather than 0xFFFF,FFFF explicitly
			if (fileCapture.m_hFile != NULL && (HANDLE)fileCapture.m_hFile != INVALID_HANDLE_VALUE) {
				fileCapture.Close();
			}
			DeleteFile(strAudioFileName);
		}
		catch (...) {}
		);

	//
	// Release all of our recording buffers
	//
	NxReleaseAllWaveHeaders(hWaveIn, apWaveHeaders);

	//
	// Close the recording stream
	//
	waveInClose(hWaveIn);

	//
	// Inform the notification window that we are done
	//
	if (pWndNotify) pWndNotify->PostMessage(NXM_AUDIO_RECORDING_TERMINATED, bSaveRecording);

	//
	// Terminate the thread
	//
	return 0;
}

CNxAudioCapture::CNxAudioCapture()
{
	m_pRecordingThread = NULL;
	m_pWndNotify = NULL;
}

CNxAudioCapture::~CNxAudioCapture()
{
	// (c.haag 2006-09-22 11:25) - The thread must NOT be active at this point
	ASSERT(NULL == m_pRecordingThread);
}

CString CNxAudioCapture::GetAudioFileName() const
{
	return m_strAudioFileName;
}

void CNxAudioCapture::SetAudioFileName(const CString& strAudioFileName)
{
	if (IsRecording()) {
		ThrowNxException("Cannot assign an audio file name while a recording session is in progress");
	}
	m_strAudioFileName = strAudioFileName;
}

DWORD CNxAudioCapture::GetMaxRecordingDuration() const
{
	// (j.jones 2010-05-25 09:17) - PLID 38864 - bumped this up to 30 minutes, up from only 5 minutes
	return 1800; // 1800 seconds, aka. 30 minutes
}

CWnd* CNxAudioCapture::GetNotificationWnd() const
{
	return m_pWndNotify;
}

void CNxAudioCapture::SetNotificationWnd(CWnd* pWndNotify)
{
	if (IsRecording()) {
		ThrowNxException("Cannot assign a notification window while a recording session is in progress");
	}
	m_pWndNotify = pWndNotify;
}

BOOL CNxAudioCapture::IsRecording()
{
	if (m_pRecordingThread) {
		DWORD dwExitCode = STILL_ACTIVE;
		::GetExitCodeThread(m_pRecordingThread->m_hThread, &dwExitCode);
		if (dwExitCode == STILL_ACTIVE) {
			return TRUE;
		} else {
			// If it so happens that the thread has terminated, delete it
			delete m_pRecordingThread;
			m_pRecordingThread = NULL;
		}
	}
	return FALSE;
}

void CNxAudioCapture::Start()
{
	//
	// Fail if we are recording
	//
	if (IsRecording()) {
		ThrowNxException("A recording session is already in progress");
	}

	//
	// Make sure the recording thread is not active. This can be true if
	// a recording was abruptly cancelled.
	//
	if (NULL != m_pRecordingThread) {
		if (WAIT_TIMEOUT == WaitForSingleObject(m_pRecordingThread->m_hThread, 1000)) {
			TerminateThread(m_pRecordingThread->m_hThread, 0);
		}
		delete m_pRecordingThread;
	}

	//
	// Start the recording thread
	//
	m_pRecordingThread = AfxBeginThread(NxAudioCapture_Thread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	m_pRecordingThread->m_bAutoDelete = FALSE;
	m_pRecordingThread->ResumeThread();
}

void CNxAudioCapture::Stop()
{
	//
	// Do nothing if we are not recording
	//
	if (!IsRecording()) {
		return;
	}

	//
	// Post a "stop recording" message to the thread. It will internally post a quit
	// message as a result of the NXTM_STOP_RECORDING message
	//
	PostThreadMessage(m_pRecordingThread->m_nThreadID, NXTM_STOP_RECORDING, 0, 0);

	//
	// Terminate the thread
	//
	if (WAIT_TIMEOUT == WaitForSingleObject(m_pRecordingThread->m_hThread, 10000)) {
		TerminateThread(m_pRecordingThread->m_hThread, 0);
	}

	//
	// Cleanup
	//
	delete m_pRecordingThread;
	m_pRecordingThread = NULL;
}

void CNxAudioCapture::Cancel()
{
	//
	// Do nothing if we are not recording
	//
	if (!IsRecording()) {
		return;
	}

	//
	// Post a "stop recording" message to the thread. It will internally post a quit
	// message as a result of the NXTM_STOP_RECORDING message
	//
	PostThreadMessage(m_pRecordingThread->m_nThreadID, NXTM_CANCEL_RECORDING, 0, 0);

	//
	// Terminate the thread
	//
	if (WAIT_TIMEOUT == WaitForSingleObject(m_pRecordingThread->m_hThread, 8000)) {
		TerminateThread(m_pRecordingThread->m_hThread, 0);
	}

	//
	// Cleanup
	//
	delete m_pRecordingThread;
	m_pRecordingThread = NULL;
}
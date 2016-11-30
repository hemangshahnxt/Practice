#ifndef __NXAUDIOCAPTURE_H__
#define __NXAUDIOCAPTURE_H__

#pragma once



// (c.haag 2006-09-22 17:43) - PLID 21327 - Initial implementation
//
// The audio capture class records data from the Windows default input
// device specified by the Volume Control dialog or the control panel.
// This class uses Windows Multimedia functions for recording.
//

class CNxAudioCapture
{
private:
	// The one and only recording thread
	CWinThread* m_pRecordingThread;

private:
	// The name of the audio file
	CString m_strAudioFileName;

	// The notification window for when a recording is done
	CWnd* m_pWndNotify;

public:
	CNxAudioCapture();
	~CNxAudioCapture();

public:
	CString GetAudioFileName() const;
	void SetAudioFileName(const CString&);

public:
	DWORD GetMaxRecordingDuration() const;

public:
	CWnd* GetNotificationWnd() const;
	void SetNotificationWnd(CWnd*);

public:
	BOOL IsRecording();

public:
	void Start();
	void Stop();
	void Cancel();
};

#endif
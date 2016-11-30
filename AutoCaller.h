

#pragma once



void CALLBACK lineCallbackFunc(
    DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance, 
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
BOOL BreakHandlerRoutine(DWORD dwCtrlType);
BOOL GetCallInfo();
void StopEverything();
BOOL PumpMessages(BOOL bWaitForMessage);
BOOL DoConnectedStuff();
void StopConnectedStuff();
void PlayFromMemory (LPSTR szWavData);
DWORD WINAPI ThreadRecorded (LPVOID lpvThreadParam);
void RecordToMemory (LPSTR szWavData);
void BuildWavData (LPSTR szFilename, LPSTR szWavData, DWORD dwMaxBufferSize);
bool PlayFile();
UINT PlayLoop(LPVOID pParam);
void ShutdownWaveFile();
bool SetupWaveFile();
void StartAutoCaller(CString strPhoneNumber);
void ShutdownLine();

extern CString	m_strMsg;
extern CString	m_strStartMsg;
extern CString	m_strUser0;
extern CString	m_strUser1;
extern CString	m_strUser2;
extern CString	m_strUser3;
extern CString	m_strUser4;
extern CString	m_strUser5;
extern CString	m_strUser6;
extern CString	m_strUser7;
extern CString	m_strUser8;
extern CString	m_strUser9;
extern CString  m_strRings;


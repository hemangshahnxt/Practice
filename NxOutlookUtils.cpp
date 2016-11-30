//NxOutlookUtils.cpp

#include "stdafx.h"
#include "NxOutlookUtils.h"
#include "NxException.h"
#include "RegUtils.h"
#include "PathStringUtils.h"
#include <WtsApi32.h>

using namespace Outlook;


CString NxOutlookUtils::GetNexPDAProfilePath()
{
	CString strPath = GetPracInstallPath();
	if (strPath.GetLength() > 0 && strPath[ strPath.GetLength() - 1 ] != '\\') {
		strPath += '\\';
	}
	return strPath + "NexPDA.ini";
}		   

CString NxOutlookUtils::ReadNexPDAProfileString(const CString& strKey, const CString& strDefault /*= ""*/, const CString& strSection /*= "NxOutlookAddin"*/)
{
	const int nMaxSize = 1024;
	CString strOut;
	char* szOut;
	DWORD nChars;

	// Read in the string from the ini file
	szOut = strOut.GetBuffer(nMaxSize);
	nChars = GetPrivateProfileString(strSection, strKey, strDefault, szOut, nMaxSize, GetNexPDAProfilePath());
	strOut.ReleaseBuffer();

	return strOut;
}

void NxOutlookUtils::WriteNexPDAProfileString(const CString& strKey, const CString& strValue, const CString& strSection /*= "NxOutlookAddin"*/)
{
	WritePrivateProfileString(strSection, strKey, strValue, GetNexPDAProfilePath());
}

int NxOutlookUtils::ReadNexPDAProfileInt(const CString& strKey, int nDefault /*= 0*/, const CString& strSection /*= "NxOutlookAddin"*/)
{
	CString strDefault;
	strDefault.Format("%d", nDefault);
	return atol(ReadNexPDAProfileString(strKey, strDefault, strSection));
}

void NxOutlookUtils::WriteNexPDAProfileInt(const CString& strKey, int nValue, const CString& strSection /*= "NxOutlookAddin"*/)
{
	CString strValue;
	strValue.Format("%d", nValue);
	WriteNexPDAProfileString(strKey, strValue, strSection);
}

CString NxOutlookUtils::GetRegSubkey()
{
	// (c.haag 2007-01-10 11:46) - PLID 24196 - We now read from an ini file for Vista compatibility
	return ReadNexPDAProfileString(
		"Subkey",
		NxRegUtils::ReadString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OutlookAddin\\Subkey"));

	//return NxRegUtils::ReadString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OutlookAddin\\Subkey", "");
}

CString NxOutlookUtils::GetRegBase()
{
	CString strSubkey = NxOutlookUtils::GetRegSubkey();
	if (!strSubkey.GetLength()) {
		return "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\";
	}
	return "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\" + strSubkey + "\\";
}

CString NxOutlookUtils::GetPracInstallPath()
{
	// (c.haag 2007-01-10 12:01) - PLID 24196 - We should only check for the install path in the NexTech folder
	CString strPath = NxRegUtils::ReadString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\InstallPath");
	if (strPath.GetLength() && strPath[ strPath.GetLength() - 1 ] != '\\') {
		strPath += "\\";
	}
	return strPath;
}

CString NxOutlookUtils::GetLinkEnabledRegistryPath()
{	
	//Registry location of the value for whether or not the link is enabled
	return "HKEY_CURRENT_USER\\Software\\Nextech\\NxOutlookMonitor\\LinkEnabled";
}

CString NxOutlookUtils::GetFolderNameFromType(OlDefaultFolders eType)
{
	CString strFolder;
	switch(eType)
	{
		case olFolderCalendar:
			strFolder = "calendar";
			break;
		case olFolderContacts:
			strFolder = "contacts";
			break;
		case olFolderNotes:
			strFolder = "notes";
			break;
		case olFolderTasks:
			strFolder = "tasks";
			break;
		default:
			ASSERT(FALSE);
			strFolder = "<invalid type number>";
			break;
	}

	return strFolder;
}

// (z.manning, 11/09/05, PLID 18282)
// Add the session ID to the named pipes' names so to prevent major problems when more than
// 1 user is using the link on the same terminal server.
CString NxOutlookUtils::GetAddinPipeName()
{
	CString strPipeName;
	strPipeName.Format("\\\\.\\pipe\\nxoutlookaddinpipe%li", GetTSSessionID());
	return strPipeName;
}

CString NxOutlookUtils::GetMonitorPipeName()
{
	CString strPipeName;
	strPipeName.Format("\\\\.\\pipe\\nxpdamonitorpipe%li", GetTSSessionID());
	return strPipeName;
}

long NxOutlookUtils::GetTSSessionID()
{
	ULONG* pulSessionID;
	DWORD dwBytes;
	if(!WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSSessionId, 
		(LPTSTR*)&pulSessionID, &dwBytes)) 
	{
		CString strError;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
		strError.ReleaseBuffer();
		Log("WTSQuerySessionInformation failed due to error: " + strError);
		return 0; // Zero is the console session's ID.
	}
	return *pulSessionID;
}

HANDLE NxOutlookUtils::Connect()
{
	//Based on sample code from MSDN
	//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/ipc/base/named_pipe_client.asp
	HANDLE hPipe;
	BOOL bPipeFound;
	DWORD dwError;
	do {
		bPipeFound = TRUE;
		hPipe = CreateFile(GetAddinPipeName(),
			PIPE_ACCESS_DUPLEX,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);					

		dwError = GetLastError();
		switch(dwError) {
		//Just in case, let's wait a second and check again
		case ERROR_FILE_NOT_FOUND:
			Sleep(1000);
			if(WaitNamedPipe(GetAddinPipeName(), 0)) {
				bPipeFound = FALSE;
			}
			break;
		//If the pipe is busy, let's try waiting up to 5 seconds
		case ERROR_PIPE_BUSY:
			if(WaitNamedPipe(GetAddinPipeName(), 5000)) {
				//Pipe is now available, run the loop again
				bPipeFound = FALSE;
			}
			break;
		}
	}while(!bPipeFound);

	//Make sure we got a valid handle.
	if(hPipe == INVALID_HANDLE_VALUE) {
		CString strError;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0,	strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
		strError.ReleaseBuffer();
		AfxThrowNxException("(NxOutlookUtils::Connect) Connecting to named pipe failed with the following error: " + strError);
	}

	return hPipe;
}

void NxOutlookUtils::Disconnect(HANDLE hAddIn)
{
	// In order to ensure that a message plus all data needed for that message is processed
	// at the same time, we send a finished message to the named pipe server.
	int nPipeFinished = NXOLPM_FINISHED;
	Send(hAddIn, sizeof(nPipeFinished), &nPipeFinished);
	CloseHandle(hAddIn);
}

void NxOutlookUtils::Send(HANDLE hAddIn, int nBytes, void* pPayload)
{
	DWORD dwBytesWritten;
	if(!WriteFile(hAddIn, pPayload, nBytes, &dwBytesWritten, NULL)) {
		AfxThrowNxException("(NxOutlookUtils::Send): Sending data through named pipes failed.");
	}
}

BOOL NxOutlookUtils::IsOutlookLinkActive()
{
	Outlook::_ApplicationPtr spOutlookApp;
	spOutlookApp.GetActiveObject(__uuidof(Outlook::Application));
	if(spOutlookApp == NULL) {
		if(!IsOutlookMonitorRunning()) {
			// If neither Outlook or the Outlook monitor are open, then the link isn't active
			return FALSE;
		}
	}
	else {
		spOutlookApp.Release();
	}
	// If the pipe isn't available, the link isn't active
	if(!WaitNamedPipe(GetAddinPipeName(), 3000)) {
		return FALSE;
	}
	return TRUE;
}

BOOL NxOutlookUtils::IsOutlookLinkConnected()
{
	BOOL bIsActive = FALSE;
	Outlook::_ApplicationPtr spOutlookApp;
	//Try to open an existing Outlook instance
	spOutlookApp.GetActiveObject(__uuidof(Outlook::Application));
	//And if there isn't one, then let's make one.
	if(spOutlookApp == NULL) {
		if(!IsOutlookMonitorRunning()) {
			// If neither Outlook or the Outlook monitor are open, then the link isn't active
			bIsActive = FALSE;
		}
		else {
			spOutlookApp.CreateInstance(__uuidof(Outlook::Application));
		}
	}

	if(spOutlookApp != NULL) {
		Office::COMAddInsPtr spAddIns = spOutlookApp->GetCOMAddIns();
		for(long i = 1; i <= spAddIns->GetCount(); i++) {
			_variant_t v = i;
			Office::COMAddInPtr spAddIn = spAddIns->Item(&v);
			_bstr_t strDesc = spAddIn->Description;
			_bstr_t strGuid = spAddIn->Guid;
		//TODO: this code needs fixed when the value of the NxOutlookAddin is known by this program
			CString strDescription = (LPCTSTR)strDesc;
			if(strDescription.Left(7) == "NexTech") {
				if(spAddIn->GetConnect()) {
					bIsActive =  TRUE;
				}
			}
		}
		spOutlookApp.Release();
	}
	return bIsActive;
}


void NxOutlookUtils::Synchronize(long nUserID, LPCTSTR szFolderName)
{
	HANDLE hPipe = Connect();
	int nSynchronize = NXOLPM_SYNCHRONIZE;
	Send(hPipe, sizeof(nSynchronize), (void*)&nSynchronize);
	Send(hPipe, sizeof(nUserID), (void*)&nUserID);
	Send(hPipe, strlen(szFolderName) + 1, (void*)szFolderName);
	Disconnect(hPipe);
}

void NxOutlookUtils::ShowAddIn(BOOL bShow /* = TRUE */)
{
	HANDLE hPipe = Connect();
	int nShowAddin = NXOLPM_SHOW_INTERFACE;
	Send(hPipe, sizeof(nShowAddin), (void*)&nShowAddin);
	Send(hPipe, sizeof(bShow), (void*)&bShow);
	Disconnect(hPipe);
	//TODO: Figure out a way to get the window name using named pipes and GetWindowText() rather than this nonsense
	HWND hwndNexPDA;
	for(int nIteration = 0; nIteration < 15; nIteration++) {
		hwndNexPDA = FindWindow(NULL, "NexTech NexPDA Link");
		if(hwndNexPDA) {
			break;
		}
		Sleep(200);
	}
	if(hwndNexPDA) {
		ShowWindow(hwndNexPDA, SW_RESTORE);
		SetForegroundWindow(hwndNexPDA);
	}
}

void NxOutlookUtils::ShowAirlock(BOOL bShow /* = TRUE */)
{
	HANDLE hPipe = Connect();
	int nShowAirlock = NXOLPM_SHOW_AIRLOCK;
	Send(hPipe, sizeof(nShowAirlock), (void*)&nShowAirlock);
	Send(hPipe, sizeof(bShow), (void*)&bShow);
	Disconnect(hPipe);
	//TODO: see ShowAddIn
	HWND hwndAirlock;
	for(int nIteration = 0; nIteration < 15; nIteration++) {
		hwndAirlock = FindWindow(NULL, "Pending changes between NexTech Practice and Microsoft Outlook");
		if(hwndAirlock) {
			break;
		}
		Sleep(200);
	}
	if(hwndAirlock) {
		ShowWindow(hwndAirlock, SW_RESTORE);
		SetForegroundWindow(hwndAirlock);
	}
}

void NxOutlookUtils::ShowMonitor()
{
	SendToNexPDAMonitor(NXOLPM_RESTORE_MONITOR_DLG);

	long nLinkEnabled = BST_UNCHECKED;
	if( NxRegUtils::DoesValueExist(GetLinkEnabledRegistryPath()) ) {
		nLinkEnabled = NxRegUtils::ReadLong(GetLinkEnabledRegistryPath());
	}

	if(nLinkEnabled == BST_UNCHECKED) {
		// The above code doesn't bring the monitor to the foreground under all circumstances,
		// so this is added as a fail-safe.
		HWND hwndMonitor = FindWindow(NULL, "NexPDA Link Monitor"); //TODO: see ShowAddIn
		if(hwndMonitor) {
			ShowWindow(hwndMonitor, SW_RESTORE);
			SetForegroundWindow(hwndMonitor);
		}
	}
	else {
		// ZM: If the link enabled button was checked, but we're tyring to show the monitor (which should only be done
		// if the link is disabled), then it likely means Outlook has crashed.  The monitor should refresh the link
		// such that Outlook is reopened, so if this is the case, let's try for a few seconds to show the addin
		// dialog instead of the monitor's dialog.
		for(int nIteration = 0; nIteration < 20; nIteration++) {
			HWND hwndAddin = FindWindow(NULL, "NexTech NexPDA Link");
			if(hwndAddin) {
				ShowWindow(hwndAddin, SW_RESTORE);
				SetForegroundWindow(hwndAddin);
				break;
			}
			Sleep(200);
		}
	}
}

void NxOutlookUtils::AddManagerReference()
{
	HANDLE hPipe = Connect();
	int nAddRef = NXOLPM_ADD_MANAGER_REF;
	Log("Sending message to addin to ADD a monitor reference.");
	Send(hPipe, sizeof(nAddRef), (void*)&nAddRef);
	Disconnect(hPipe);
}

void NxOutlookUtils::ReleaseManagerReference()
{
	HANDLE hPipe = Connect();
	int nReleaseRef = NXOLPM_RELEASE_MANAGER_REF;
	Log("Sending message to addin to RELEASE a monitor reference.");
	Send(hPipe, sizeof(nReleaseRef), (void*)&nReleaseRef);
	Disconnect(hPipe);
}

void NxOutlookUtils::ShutdownNexPDALink()
{
	HANDLE hPipe = Connect();
	int nShutdown = NXOLPM_FORCE_SHUTDOWN;
	Log("Sending message to addin to shut it down.");
	Send(hPipe, sizeof(nShutdown), (void*)&nShutdown);
	Disconnect(hPipe);
}

void NxOutlookUtils::CheckForSynchronization()
{
	HANDLE hPipe = Connect();
	int nSyncCheck = NXOLPM_QUERY_SYNC_STATUS;
	Send(hPipe, sizeof(nSyncCheck), (void*)&nSyncCheck);
	Disconnect(hPipe);
}

BOOL NxOutlookUtils::IsOutlookMonitorRunningGlobal()
{
	BOOL bMonitorIsRunning = FALSE;
	DWORD adwProcesses[1024], dwNeeded;
	HINSTANCE hInstLib = LoadLibrary("PSAPI.DLL");
	TCHAR szProcessName[MAX_PATH] = TEXT("");

	// PSAPI Function Pointers.
	BOOL (WINAPI *lpfEnumProcesses)( DWORD*, DWORD cb, DWORD* );
	BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE*, DWORD, LPDWORD );
	DWORD (WINAPI *lpfGetModuleBaseName)( HANDLE, HMODULE, LPTSTR, DWORD );

	// Load the procedures
	lpfEnumProcesses = (BOOL(WINAPI*)(DWORD*,DWORD,DWORD*))GetProcAddress(hInstLib, "EnumProcesses");
	lpfEnumProcessModules = (BOOL(WINAPI*)(HANDLE, HMODULE*, DWORD, LPDWORD)) GetProcAddress(hInstLib,"EnumProcessModules");
	lpfGetModuleBaseName = (DWORD (WINAPI*)(HANDLE, HMODULE, LPTSTR, DWORD)) GetProcAddress(hInstLib,"GetModuleBaseNameA");

	if(!lpfEnumProcesses(adwProcesses, sizeof(adwProcesses), &dwNeeded)) {
		CString strError;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
		strError.ReleaseBuffer();
		Log("Error in NxOutlookUtils::IsOutlookMonitorRunningGlobal - %s", strError);
		//AfxThrowNxException("NxOutlookUtils::IsOutlookMonitorRunningGlobal: Could not enumerate processes");
		// ZM: Returning false here rather than an exception, because the monitor has code preventing it from
		//     being open more than once, so trying to open it again should be harmless.
		return FALSE;
	}
	DWORD dwTotalProcesses = dwNeeded / sizeof(DWORD);
	// Cycle through all processes to see if the Outlook monitor is running
	for(unsigned int i = 0; i < dwTotalProcesses; i++) {
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, adwProcesses[i]);
		if(hProcess != NULL) {
			HMODULE hMod;
			if(lpfEnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwNeeded)) {
				lpfGetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR));
			}
			CString strProcess = szProcessName;
			CString strMonitor = OUTLOOK_MONITOR_EXE_NAME;
			if(strProcess.Left(15).CompareNoCase(strMonitor.Left(15)) == 0) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL NxOutlookUtils::IsOutlookMonitorRunning()
{
	DWORD dwTotalProcesses;
	PWTS_PROCESS_INFO processes;
	if(!WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &processes, &dwTotalProcesses)) {
		// (z.manning, 12/16/05, PLID 18565)
		// WTSEnumerateProcesses does not work in Windows 2000 non-server editions, so
		// let's check the error and use non-TS process enumeration.
		DWORD dwError = GetLastError();
		switch(dwError) {
		case ERROR_APP_WRONG_OS:
			return IsOutlookMonitorRunningGlobal();
			break;
		default:
			CString strError;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
			strError.ReleaseBuffer();
			Log("Error in NxOutlookUtils::IsOutlookMonitorRunning - %s", strError);
			//AfxThrowNxException("NxOutlookUtils::IsOutlookMonitorRunning: Could not enumerate processes");
			// ZM: Returning false here rather than an exception, because the monitor has code preventing it from
			//     being open more than once, so trying to open it again should be harmless.
			return FALSE;
			break;
		}
	}
	// Cycle through all processes to see if the Outlook monitor is running
	for(unsigned int i = 0; i < dwTotalProcesses; i++) {
		if(processes[i].SessionId == (DWORD)GetTSSessionID()) {
			CString strProcessName;
			strProcessName.Format("%s", processes[i].pProcessName);
			CString strMonitorName = OUTLOOK_MONITOR_EXE_NAME;
			// It seems older versions of Windows only diplay the first 15 characters
			// of the process' name, so that's all we're going to compare.
			if(strProcessName.Left(15).CompareNoCase(strMonitorName.Left(15)) == 0) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

void NxOutlookUtils::EnsureOutlookMonitorIsRunning()
{
	if(!IsOutlookMonitorRunning()) {
		CString strMonitorFullPath = GetPracInstallPath() ^ OUTLOOK_MONITOR_EXE_NAME;
		STARTUPINFO startupinfo;
		PROCESS_INFORMATION processinfo;
		ZeroMemory( &startupinfo, sizeof(startupinfo) );
		startupinfo.cb = sizeof(startupinfo);
		startupinfo.lpReserved = NULL;
		ZeroMemory( &processinfo, sizeof(processinfo) );
		if(CreateProcess(NULL, (LPTSTR)(LPCTSTR)strMonitorFullPath, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, 
			NULL, &startupinfo, &processinfo))
		{
			WaitForInputIdle(processinfo.hProcess, 2500);
			Sleep(750);
			// We want to make sure the link has completed initialization before leaving this function.
			// The way to ensure the NexPDA Link is done initializing is if the named pipes are created and 
			// listening, so this code waits up to 10 seconds for that to happen.
			if( BST_CHECKED == NxRegUtils::ReadLong(GetLinkEnabledRegistryPath()) ) {
				if(IsOutlookLinkConnected()) {
					int nAttempts = 0;
					while(!WaitNamedPipe(GetAddinPipeName(), 0)) {
						Sleep(500);
						if(nAttempts > 8) {
							break;
						}
						nAttempts++;
					}
				}
			}
		}
		else //Report the error
		{
			DWORD dwError = GetLastError();
			CString strError;
			switch(dwError) 
			{
				case ERROR_FILE_NOT_FOUND:
				case ERROR_PATH_NOT_FOUND:
					AfxMessageBox("Could not open the NexPDA Link Monitor. The file \'" + strMonitorFullPath + "\' was not found.");
					break;
				default:
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
					strError.ReleaseBuffer();
					AfxMessageBox("Unable to open the NexPDA Link Monitor due to the following error:\r\n" + strError);
					break;
			}
		}
	}
}

BOOL NxOutlookUtils::SendToNexPDAMonitor(int nMsg)
{
	return SendToNexPDAMonitor((void*)&nMsg);
}

BOOL NxOutlookUtils::SendDataToNexPDAMonitor(int nMsg, void* pDataPayload, int nPayloadBytes)
{
	BOOL bResult;
	char* p = new char[sizeof(int) + nPayloadBytes];
	*((int*)p) = nMsg;
	memcpy(p + sizeof(int), pDataPayload, nPayloadBytes);
	bResult = SendToNexPDAMonitor(p, sizeof(int) + nPayloadBytes);
	delete[] p;
	return bResult;
}

BOOL NxOutlookUtils::SendErrorToNexPDAMonitor(int nOriginalMsg, int nErrorCode)
{
	int anPacket[3] = { NXOLPM_ERROR, nOriginalMsg, nErrorCode };
	return SendToNexPDAMonitor(anPacket, sizeof(anPacket));
}

BOOL NxOutlookUtils::SendToNexPDAMonitor(void* pData, int nBytes /* = sizeof(int) */)
{
	// Since it's very allowable for the monitor to not be open and the NexPDA Link still be active,
	// it's possible that this code will fail legitimately, hence the minimal error checking.
	//BOOL bKeepTrying;
	HANDLE hPipe;
	DWORD dwError;
	BOOL bFailed = TRUE;
	//do {
		//bKeepTrying = FALSE;

		// If the pipe is busy, wait up to a second to see if the pipe becomes available.
		if(WaitNamedPipe(GetMonitorPipeName(), 1000)) { //This will return instantly if the pipe is not found.
			hPipe = CreateFile(GetMonitorPipeName(),
					PIPE_ACCESS_DUPLEX,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_OVERLAPPED,
					NULL);
		}
		else {
			hPipe = INVALID_HANDLE_VALUE;
			// (b.savon 2013-05-08 14:39) - PLID 47952 - Output the pipe error message
			CString strError;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
			strError.ReleaseBuffer();
			Log("The pipe %s was unable to open a connection due to error: %s", GetMonitorPipeName(), strError);
			if(IsOutlookMonitorRunning()) {
				CString strError;
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
				strError.ReleaseBuffer();
				// (b.savon 2013-05-08 14:43) - PLID 47952 - According to Ryan's notes in the pl item, this returns successful as the
				// last error from the call to IsOutlook
				Log("The Outlook Monitor is running: %s", strError);
			}
		}

		dwError = GetLastError();
	//}while(bKeepTrying);

	if(hPipe != INVALID_HANDLE_VALUE) {
		DWORD dwBytesWritten;
		Log("Message %li SENT to the monitor.", *((UINT*)pData));
		WriteFile(hPipe, pData, nBytes, &dwBytesWritten, NULL);
		bFailed = FALSE;
	}
	else {
		if(IsOutlookMonitorRunning()) { //No need to report the error if the monitor's not even running
			CString strError;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0,	strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
			strError.ReleaseBuffer();
			Log("Failed to send a message to the monitor due to the following error: %s", strError);
		}
	}
	CloseHandle(hPipe);
	return (bFailed) ? FALSE : TRUE;
}

// (z.manning 2009-10-19 15:21) - PLID 35997 - Moved here from CWizardCalendarSubjectDlg
void NxOutlookUtils::FillSubjectLineFieldList(LPDISPATCH lpFieldsList)
{
	NXDATALISTLib::_DNxDataListPtr pdlFields(lpFieldsList);
	CNxFolderFieldArray astrFields;
	GetSupportedSubjectLineFields(&astrFields);
	for (long i=0; i < astrFields.GetSize(); i++)
	{
		NXDATALISTLib::IRowSettingsPtr pRow = pdlFields->GetRow(-1);
		pRow->Value[flcCheckbox] = (long)0;
		pRow->Value[flcField] = (LPCTSTR)astrFields[i].GetField();
		pRow->Value[flcAlias] = (LPCTSTR)astrFields[i].GetAlias();
		pdlFields->AddRow(pRow);
	}
	pdlFields->Sort();
}

// (z.manning 2009-10-19 15:21) - PLID 35997 - Moved here from CWizardCalendarSubjectDlg
void NxOutlookUtils::GetSupportedSubjectLineFields(CNxFolderFieldArray* paFields)
{
	paFields->RemoveAll();
	paFields->Add(CNxFolderField("[First]", "First Name"));
	paFields->Add(CNxFolderField("[Last]", "Last Name"));
	paFields->Add(CNxFolderField("[Middle]", "Middle Name"));
	// (z.manning 2009-11-04 12:28) - PLID 35997 - The name initial fields are newly added
	// as of NexSync.
	paFields->Add(CNxFolderField("[FirstInitial]", "First Initial"));
	paFields->Add(CNxFolderField("[LastInitial]", "Last Initial"));
	paFields->Add(CNxFolderField("[MiddleInitial]", "Middle Initial"));
	paFields->Add(CNxFolderField("[HomePhone]", "Home Phone"));
	paFields->Add(CNxFolderField("[WorkPhone]", "Work Phone"));
	paFields->Add(CNxFolderField("[CellPhone]", "Cell Phone"));
	paFields->Add(CNxFolderField("[Email]", "E-Mail Address"));
	paFields->Add(CNxFolderField("[BirthDate]", "Birth Date"));
	paFields->Add(CNxFolderField("[Notes]", "Notes"));
	paFields->Add(CNxFolderField("[MoveUp]", "Appointment Move-Up"));
	paFields->Add(CNxFolderField("[Confirmed]", "Appointment Confirmed"));
	paFields->Add(CNxFolderField("[Type]", "Appointment Type"));
	paFields->Add(CNxFolderField("[Purposes]", "Appointment Purposes"));
	paFields->Add(CNxFolderField("[Location]", "Appointment Location"));
	paFields->Add(CNxFolderField("[Resources]", "Appointment Resources"));
	paFields->Add(CNxFolderField("[Diag1Code]", "Default Diag Code 1"));
	paFields->Add(CNxFolderField("[Diag2Code]", "Default Diag Code 2"));
	paFields->Add(CNxFolderField("[Diag3Code]", "Default Diag Code 3"));
	paFields->Add(CNxFolderField("[Diag4Code]", "Default Diag Code 4"));
	paFields->Add(CNxFolderField("[Diag1Name]", "Default Diag Name 1"));
	paFields->Add(CNxFolderField("[Diag2Name]", "Default Diag Name 2"));
	paFields->Add(CNxFolderField("[Diag3Name]", "Default Diag Name 3"));
	paFields->Add(CNxFolderField("[Diag4Name]", "Default Diag Name 4"));
}

// (z.manning 2009-10-19 15:21) - PLID 35997 - Moved here from CWizardCalendarSubjectDlg
void TryInsertOptionalLiteral(const CString& strPrevFieldValue, const CString& strFieldValue, const CString& strLiteral,
							  CString& strText)
{
	if ((!strPrevFieldValue.IsEmpty() || !strFieldValue.IsEmpty()) && !strLiteral.IsEmpty())
	{
		strText += strLiteral;
	}
}

// (z.manning 2009-10-19 15:21) - PLID 35997 - Moved here from CWizardCalendarSubjectDlg
void NxOutlookUtils::GenerateSubjectLinePreview(CEdit *peditSubject, CEdit *peditSubjectPreview)
{
	//
	// This code updates the preview text contained in the 
	// picture of the Outlook appointment
	//
	CString strScript;
	CString strSubject;
	CString strItem;
	BOOL bInBrackets = FALSE;
	BOOL bInDoubleBrackets = FALSE;
	peditSubject->GetWindowText(strScript);
	long n = 0;

	// Variables used for double-bracket operations
	CString strPrevFieldValue;
	CString strDoubleBracketedItem;
	while (n < strScript.GetLength())
	{
		if (strScript.GetAt(n) == '[') {
			if (bInBrackets)
			{
				strDoubleBracketedItem.Empty();
				bInDoubleBrackets = TRUE;
			}
			else
			{
				strItem.Empty();
				bInBrackets = TRUE;
			}
		}
		else if (strScript.GetAt(n) == ']') {
			if (bInDoubleBrackets)
			{
				bInDoubleBrackets = FALSE;
			}
			else if (bInBrackets)
			{
				if (strItem.GetLength())
				{
					CString strFieldValue;

					if (strItem == "Last")
						strFieldValue = "Doe";
					else if (strItem == "First")
						strFieldValue = "Jane";
					else if (strItem == "Middle")
						strFieldValue = "Q";

					// (z.manning 2009-11-04 12:30) - PLID 35997 - The intial fields are new as of NexSync
					if (strItem == "LastInitial")
						strFieldValue = "D";
					else if (strItem == "FirstInitial")
						strFieldValue = "J";
					else if (strItem == "MiddleInitial")
						strFieldValue = "Q";

					else if (strItem == "HomePhone")
						strFieldValue = "(555) 555-3494";
					else if (strItem == "WorkPhone")
						strFieldValue = "(555) 555-4983";
					else if (strItem == "CellPhone")
						strFieldValue = "(555) 555-2381";
					else if (strItem == "Email")
						strFieldValue = "jdoe@internet.com";
					else if (strItem == "Notes")
						strFieldValue = "";
					else if (strItem == "Location")
						strFieldValue = "Pawtuckett Plastic Surgeons";
					else if (strItem == "Resources")
						strFieldValue = "Dr. Smith, Dr. Penn";
					else if (strItem == "MoveUp")
						strFieldValue = "(m)";
					else if (strItem == "Confirmed")
						strFieldValue = "(c)";
					else if (strItem == "Type")
						strFieldValue = "Surgery";
					else if (strItem == "Purposes")
						strFieldValue = "Breast Augmentation";
					else if (strItem == "BirthDate")
						strFieldValue = "5/3/1964";
					else if (strItem == "Diag1Code")
						strFieldValue = "123.45";
					else if (strItem == "Diag2Code")
						strFieldValue = "123.45";
					else if (strItem == "Diag3Code")
						strFieldValue = "123.45";
					else if (strItem == "Diag4Code")
						strFieldValue = "123.45";
					else if (strItem == "Diag1Name")
						strFieldValue = "Symptom";
					else if (strItem == "Diag2Name")
						strFieldValue = "Symptom";
					else if (strItem == "Diag3Name")
						strFieldValue = "Symptom";
					else if (strItem == "Diag4Name")
						strFieldValue = "Symptom";
					TryInsertOptionalLiteral(strPrevFieldValue, strFieldValue, strDoubleBracketedItem, strSubject);
					strDoubleBracketedItem.Empty();

					strSubject += strFieldValue;
					strPrevFieldValue = strFieldValue;
				}
				bInBrackets = FALSE;
			}				
		}
		else {
			if (bInDoubleBrackets)
				strDoubleBracketedItem.Insert(strDoubleBracketedItem.GetLength(), strScript.GetAt(n));
			else if (bInBrackets)
				strItem.Insert(strItem.GetLength(), strScript.GetAt(n));
			else
			{
				TryInsertOptionalLiteral(strPrevFieldValue, "", strDoubleBracketedItem, strSubject);
				strDoubleBracketedItem.Empty();
				strPrevFieldValue.Empty();
				strSubject.Insert(strSubject.GetLength(), strScript.GetAt(n));
			}
		}
		n++;
	}
	TryInsertOptionalLiteral(strPrevFieldValue, "", strDoubleBracketedItem, strSubject);
	peditSubjectPreview->SetWindowText(strSubject);
}

// (z.manning 2009-10-19 15:21) - PLID 35997 - Moved here from CWizardCalendarSubjectDlg
void NxOutlookUtils::HandleSubjectLineEditChange(CEdit *peditSubject, CEdit *peditSubjectPreview, LPDISPATCH lpFieldsList)
{
	//
	// Update the fields checklist
	//
	NXDATALISTLib::_DNxDataListPtr pdlFields(lpFieldsList);
	CString strSubject;
	peditSubject->GetWindowText(strSubject);

	for (long i=0; i < pdlFields->GetRowCount(); i++)
	{
		CString strField(pdlFields->GetValue(i, flcField).bstrVal);
		COleVariant vCheckbox = pdlFields->GetValue(i, flcCheckbox);

		if (-1 == strSubject.Find(strField, 0))
		{
			if (vCheckbox.boolVal)
			{
				vCheckbox.boolVal = FALSE;
				pdlFields->PutValue(i, flcCheckbox, vCheckbox);
			}
		}
		else
		{
			if (!vCheckbox.boolVal)
			{
				vCheckbox.boolVal = TRUE;
				pdlFields->PutValue(i, flcCheckbox, vCheckbox);
			}
		}
	}

	//
	// Update the preview text contained in the picture of the
	// Outlook appointment
	//
	GenerateSubjectLinePreview(peditSubject, peditSubjectPreview);
}

// (z.manning 2009-10-19 15:21) - PLID 35997 - Moved here from CWizardCalendarSubjectDlg
void NxOutlookUtils::HandleSubjectLineFieldsListEditingFinished(CDialog *pdlg, CEdit *peditSubject, CEdit *peditSubjectPreview, LPDISPATCH lpFieldsList, long nRow, _variant_t varNewValue, IN OUT CString &strSubject)
{
	NXDATALISTLib::_DNxDataListPtr pdlFields(lpFieldsList);
	if (varNewValue.boolVal)
	{
		pdlg->UpdateData();
		strSubject += CString(" ") + CString(pdlFields->GetValue(nRow, flcField).bstrVal);
	}
	else
	{
		strSubject.Replace( CString(pdlFields->GetValue(nRow, flcField).bstrVal), "" );
		strSubject.TrimRight();
	}
	// (z.manning 2009-10-20 09:18) - PLID 35997 - The subject line cannot be more than 255 characters.
	if(strSubject.GetLength() > 255) {
		strSubject = strSubject.Left(255);
	}

	pdlg->UpdateData(FALSE);

	//
	// Update the preview text contained in the picture of the
	// Outlook appointment
	//
	GenerateSubjectLinePreview(peditSubject, peditSubjectPreview);
}
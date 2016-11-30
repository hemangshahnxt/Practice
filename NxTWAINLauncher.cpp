// NxTWAIN.cpp: implementation of the CNxTWAIN class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxTWAIN.h"
#include "NxTWAINLaunchPickerDlg.h"
#include "NxMessageDef.h"
#include "RegUtils.h"
#include <sti.h>
#include <tlhelp32.h>
#include <malloc.h>
#include "NxWIA.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
//
// ToolHelp Function Pointers.
HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD);
BOOL (WINAPI *lpfProcess32First)(HANDLE,LPPROCESSENTRY32);
BOOL (WINAPI *lpfProcess32Next)(HANDLE,LPPROCESSENTRY32);

// PSAPI Function Pointers.
BOOL (WINAPI *lpfEnumProcesses)( DWORD *, DWORD cb, DWORD * );
BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *,
 DWORD, LPDWORD );
DWORD (WINAPI *lpfGetModuleBaseName)( HANDLE, HMODULE,
 LPTSTR, DWORD );

long g_nActiveDeviceID = -1;
BOOL CALLBACK NotifyTWAINAppEnum( HWND hwnd, LPARAM lParam )
{
  DWORD dwID;
  GetWindowThreadProcessId(hwnd, &dwID) ;
  if(dwID == (DWORD)lParam)
  {
     PostMessage(hwnd, NXM_TWAIN_SOURCE_LAUNCH, g_nActiveDeviceID, 0);
  }
  return TRUE;
}


long g_nWIAInfoFileNumber = -1;
BOOL CALLBACK NotifyWIAAppEnum( HWND hwnd, LPARAM lParam )
{
  DWORD dwID;
  GetWindowThreadProcessId(hwnd, &dwID) ;
  if(dwID == (DWORD)lParam)
  {
     PostMessage(hwnd, NxWIA::NXM_ACQUIRE_FROM_WIA, g_nWIAInfoFileNumber, 0);
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNxTWAIN::CNxTWAIN()
{
	m_bPersistentActivationByCamera = FALSE;
	m_bPersistentActivationByWIA = FALSE;
	m_nActiveDeviceID = -1;
	m_nWIAInfoFileNumber = -1;
}

CNxTWAIN::~CNxTWAIN()
{

}

DWORD CNxTWAIN::GetPID(const CString& strAppName)
{
  OSVERSIONINFO  osver;
  HINSTANCE hInstLib;
  CDWordArray adwFoundPIDs;

  // Check to see if were running under Windows95 or
  // Windows NT.
  osver.dwOSVersionInfoSize = sizeof( osver ) ;
  //d.thompson 10/14/2015 - PLID 67348 - GetVersionEx is now deprecated.  
  //	Future PLID 67347
#pragma warning (push)
#pragma warning(disable: 4996)
  if( !GetVersionEx( &osver ) )
  {
     return 0;
  }
#pragma warning(pop)

  // If Windows NT:
  if( osver.dwPlatformId == VER_PLATFORM_WIN32_NT )
  {
	 DWORD dwSize;
     DWORD dwSize2 = 256 * sizeof( DWORD ) ;
	 DWORD dwIndex;
     LPDWORD lpdwPIDs = NULL;
	 HANDLE hProcess;
	 HMODULE hMod = NULL;
	 char szFileName[ MAX_PATH ] ;


     // Load library and get the procedures explicitly. We do
     // this so that we don't have to worry about modules using
     // this code failing to load under Windows 95, because
     // it can't resolve references to the PSAPI.DLL.
     hInstLib = LoadLibraryA( "PSAPI.DLL" ) ;
     if( hInstLib == NULL )
        return 0;

	 // Load the procedures
     lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*))
        GetProcAddress( hInstLib, "EnumProcesses" ) ;
     lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,
        DWORD, LPDWORD)) GetProcAddress( hInstLib,
        "EnumProcessModules" ) ;
     lpfGetModuleBaseName =(DWORD (WINAPI *)(HANDLE, HMODULE,
        LPTSTR, DWORD )) GetProcAddress( hInstLib,
        "GetModuleBaseNameA" ) ;

     do
     {
        if( lpdwPIDs )
        {
           HeapFree( GetProcessHeap(), 0, lpdwPIDs ) ;
           dwSize2 *= 2 ;
        }
        lpdwPIDs = (LPDWORD)HeapAlloc( GetProcessHeap(), 0, dwSize2 );
        if( lpdwPIDs == NULL )
        {
           return 0;
        }
        if( !lpfEnumProcesses( lpdwPIDs, dwSize2, &dwSize ) )
        {
           return 0;
        }
     }while( dwSize == dwSize2 ) ;

     // How many ProcID's did we get?
     dwSize /= sizeof( DWORD ) ;

	// Loop through each ProcID to find the HotSync
     for( dwIndex = 0 ; dwIndex < dwSize ; dwIndex++ )
     {
        szFileName[0] = 0 ;
        // Open the process (if we can... security does not
        // permit every process in the system).
        hProcess = OpenProcess(
           PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
           FALSE, lpdwPIDs[ dwIndex ] ) ;
        if( hProcess != NULL )
        {
           // Here we call EnumProcessModules to get only the
           // first module in the process this is important,
           // because this will be the .EXE module for which we
           // will retrieve the full path name in a second.
           if( lpfEnumProcessModules( hProcess, &hMod,
              sizeof( hMod ), &dwSize2 ) )
           {
              // Get Full pathname:
              //if( !GetModuleFileNameEx( hProcess, hMod,
			  if( !lpfGetModuleBaseName( hProcess, hMod,
                 szFileName, sizeof( szFileName ) ) )
              {
                 szFileName[0] = 0 ;
              }
           }
           CloseHandle( hProcess ) ;
		   strupr(szFileName);
		   if (strstr(szFileName, strAppName))
		   {
			   adwFoundPIDs.Add( lpdwPIDs[ dwIndex ] );
			//	return lpdwPIDs[ dwIndex ];
		   }
        }     
	 }
  }
  else if( osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
  {
	HANDLE         hSnapShot;
	PROCESSENTRY32 procentry;
	BOOL           bFlag;

	hInstLib = LoadLibraryA( "Kernel32.DLL" ) ;
	if( hInstLib == NULL )
		return FALSE ;

	// Get procedure addresses.
	// We are linking to these functions of Kernel32
	// explicitly, because otherwise a module using
	// this code would fail to load under Windows NT,
	// which does not have the Toolhelp32
	// functions in the Kernel 32.
	lpfCreateToolhelp32Snapshot=
	(HANDLE(WINAPI *)(DWORD,DWORD))
	GetProcAddress( hInstLib,
	"CreateToolhelp32Snapshot" ) ;
	lpfProcess32First=
	(BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
	GetProcAddress( hInstLib, "Process32First" ) ;
	lpfProcess32Next=
	(BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
	GetProcAddress( hInstLib, "Process32Next" ) ;
	if( lpfProcess32Next == NULL ||
	lpfProcess32First == NULL ||
	lpfCreateToolhelp32Snapshot == NULL )
	{
	FreeLibrary( hInstLib ) ;
	return FALSE ;
	}


     // Get a handle to a Toolhelp snapshot of the systems
     // processes.
     hSnapShot = lpfCreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS, 0 ) ;
     if( hSnapShot == INVALID_HANDLE_VALUE )
     {
        FreeLibrary( hInstLib ) ;
        return FALSE ;
     }

     // Get the first process' information.
     procentry.dwSize = sizeof(PROCESSENTRY32) ;
     bFlag = lpfProcess32First( hSnapShot, &procentry ) ;

     // While there are processes, keep looping.
     while( bFlag )
     {
		 char sz[MAX_PATH];
		 strcpy(sz, procentry.szExeFile);
		 strupr(sz);
		 if (strstr(sz, strAppName))
		 {
           //procentry.dwSize = sizeof(PROCESSENTRY32) ;
           //bFlag = lpfProcess32Next( hSnapShot, &procentry );
		 }
		 else {
			 adwFoundPIDs.Add( procentry.th32ProcessID );
		   //DWORD dwPID = procentry.th32ProcessID;
		   //FreeLibrary( hInstLib );
		   //return dwPID;

		 }
		procentry.dwSize = sizeof(PROCESSENTRY32) ;
		bFlag = lpfProcess32Next( hSnapShot, &procentry );
     }
  }
  FreeLibrary( hInstLib );
  return GetCorrectProcess(adwFoundPIDs);
}

DWORD CNxTWAIN::GetCorrectProcess(const CDWordArray &adwPIDs)
{
	CDWordArray adwPotentialPIDs;
	CStringArray astrPotentialPNames;
	char szUsername[512];
	DWORD dwUsername = 512;
	GetUserName(szUsername, &dwUsername);	
	for (long i=0; i < adwPIDs.GetSize(); i++)
	{
		if (GetCurrentProcessId() == adwPIDs[i])
			continue;

		// Get the process
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, adwPIDs[i]); 
		if(hProcess == INVALID_HANDLE_VALUE)
		{
			continue;
		}

		// Get the owner of the process
		BOOL bHaveSID = FALSE;
		HANDLE hToken;
		if(::OpenProcessToken(hProcess, TOKEN_QUERY_SOURCE|TOKEN_QUERY, &hToken))
		{
			SID_NAME_USE use;
			DWORD dwLen = NULL;
			char szOwner[512];
			char szDomain[512];
			DWORD dwName = 512, dwDomainName = 512;
			::GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLen);
			if(::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				;
			else
			{
				TOKEN_USER* pWork = static_cast<TOKEN_USER*>(_alloca(dwLen));
				if(::GetTokenInformation(hToken, TokenUser, pWork, dwLen, &dwLen))
				{
					if(LookupAccountSid(NULL, pWork->User.Sid, szOwner, &dwName, szDomain, &dwDomainName, &use))
					{
						// If the logged in user is the owner, go ahead and
						// add it to our potential list
						if (!strcmp(szOwner, szUsername))
						{
							adwPotentialPIDs.Add(adwPIDs[i]);
						}
					}
				}
			}
			::CloseHandle(hToken);
		}
	}

//#ifdef _DEBUG
//	CString str;
//	str.Format("Debug message: %d TWAIN-ready instances found", adwPotentialPIDs.GetSize());
//	AfxMessageBox(str);
//#endif

	// Now that we have a list of potential processes, we can finally
	// do something. If there is only one, then we know we can go ahead
	// and use that PID.
	if (adwPotentialPIDs.GetSize() == 1)
		return adwPotentialPIDs[0];
	// If there are multiple PID's, we have to force the user to choose one.
	else if (adwPotentialPIDs.GetSize() > 1)
	{
		CNxTWAINLaunchPickerDlg dlg(NULL);
		dlg.SetPIDs(adwPotentialPIDs);
		dlg.DoModal();
		return dlg.GetSelectedPID();
	}
	// If there are no PID's, that means there are no active instances of
	// the software other than this one. So, we will go ahead and launch this one.
	// However, we don't want to send a message to ourself; instead we will set
	// a member flag.
	else
	{
		m_bPersistentActivationByCamera = TRUE;
	}
	return 0;
}

void CNxTWAIN::OnRequestImport(const CString& strAppName)
{
	CString str;
	DWORD dwPID;
	char* szFilename = str.GetBuffer(MAX_PATH);

	if (!strAppName.GetLength())
	{
		// Get the filename of the application
		GetModuleFileName( GetModuleHandle(NULL), szFilename, MAX_PATH );
		str.ReleaseBuffer(-1);
		if (-1 != str.ReverseFind('\\'))
		{
			str = str.Right( str.GetLength() - str.ReverseFind('\\') - 1);
		}
	}
	else
	{
		str = strAppName;
	}

	// Get the PID
	str.MakeUpper();
	if (dwPID = GetPID(str))
	{
        HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, dwPID ) ;

        if( hProcess == NULL )
			return;

		g_nActiveDeviceID = m_nActiveDeviceID;
		EnumWindows((WNDENUMPROC)NotifyTWAINAppEnum, (LPARAM) dwPID) ;
		CloseHandle(hProcess) ;
	}
}

BOOL CNxTWAIN::Register(const CString& strAppName)
{
	// First make sure sti is available  on this machine
	if (g_dllSti.IsFunctionExported(StiCreateInstance) == NULL) {
		// It's not so we have to fail right off the bat
		return FALSE;
	}
	
	IStillImage* isi = NULL;
	WCHAR   szDeviceName[65];
	WCHAR   szEventName[65];
	DWORD   dwEventCode;
	CString strCmdLine = GetCommandLine();
	strCmdLine = strCmdLine.Left( strCmdLine.Find('\"', 1) );
	strCmdLine.Remove('\"');

	CString strRegisteredPath = NxRegUtils::ReadString(GetRegistryBase() + "Setup\\TWAINLaunchPath");

	HRESULT hRes = g_dllSti.StiCreateInstance(AfxGetApp()->m_hInstance,
		STI_VERSION,
		&isi,
		NULL);

	if (hRes != S_OK)
		return FALSE;

	// Was this a STI launch?
	// Call STI-API and get the device name, event and event code
	hRes = isi->GetSTILaunchInformation(szDeviceName,
										   &dwEventCode,
										   szEventName);
	if (hRes == S_OK)
	{
		// Get the device ID
		DWORD nDevices = 0;
		STI_DEVICE_INFORMATION* pDevInfo;
		if (S_OK == (hRes = isi->GetDeviceList(NULL, NULL, &nDevices, (void**)&pDevInfo)))
		{
			for (DWORD i=0; i < nDevices; i++)
			{
				CString strDeviceName(szDeviceName);
				CString strEnumDeviceName(pDevInfo[i].szDeviceInternalName);
				if (!stricmp(strDeviceName, strEnumDeviceName))
				{
					break;
				}
			}
			if (i == nDevices)
				m_nActiveDeviceID = -1;
			else
				m_nActiveDeviceID = (long)i;
		}
		else
		{
			m_nActiveDeviceID = -1;
		}

		m_strActiveDeviceName = szDeviceName;
		OnRequestImport(strAppName);
		LocalFree(pDevInfo);
		if (!m_bPersistentActivationByCamera)
		{
			isi->Release();
			return TRUE;
		}
	}

	// Register the app if we aren't registered
	if (strRegisteredPath.IsEmpty())
	{
		strRegisteredPath = strCmdLine;
		hRes = isi->RegisterLaunchApplication(L"NexTech Practice", _bstr_t(strCmdLine));
		if (hRes != S_OK)
			return FALSE;
		NxRegUtils::WriteString(GetRegistryBase() + "Setup\\TWAINLaunchPath", strRegisteredPath);
	}

	isi->Release();
	return FALSE;
}

// (a.walling 2008-10-28 13:05) - PLID 31334 - Launched from WIA auto-start
// same as TWAIN, if we have an open process, send to that one. Otherwise we'll try to open a new session.
BOOL CNxTWAIN::HandleWIAEvent(const CString& strWIADevice)
{
	m_strWIADevice = strWIADevice;
	CString str;
	DWORD dwPID;
	char* szFilename = str.GetBuffer(MAX_PATH);

	// Get the filename of the application
	GetModuleFileName( GetModuleHandle(NULL), szFilename, MAX_PATH );
	str.ReleaseBuffer(-1);
	if (-1 != str.ReverseFind('\\'))
	{
		str = str.Right( str.GetLength() - str.ReverseFind('\\') - 1);
	}

	// Get the PID
	str.MakeUpper();
	if (dwPID = GetPID(str))
	{
        HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, dwPID ) ;

        if( hProcess == NULL )
			return FALSE;

		srand((unsigned int)time(NULL));
		m_nWIAInfoFileNumber = -1;

		// Get the real temp path
		CString strTempPath;
		{
			DWORD dwNeedLen = ::GetTempPath(0, NULL);
			::GetTempPath(dwNeedLen, strTempPath.GetBuffer(dwNeedLen+1));
			strTempPath.ReleaseBuffer();
		}

		CString strFileName;
		BOOL bKeepTrying = TRUE;
		HANDLE hFile = INVALID_HANDLE_VALUE;

		while (bKeepTrying) {
			m_nWIAInfoFileNumber = rand();
			strFileName.Format("NxWIA%lu.tmp", m_nWIAInfoFileNumber);

			hFile = ::CreateFile(strTempPath ^ strFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile != INVALID_HANDLE_VALUE) {
				bKeepTrying = FALSE;
				g_nWIAInfoFileNumber = m_nWIAInfoFileNumber;
			}
		}

		DWORD dwWritten = 0;
		::WriteFile(hFile, strWIADevice, strWIADevice.GetLength(), &dwWritten, NULL);

		ASSERT(dwWritten == strWIADevice.GetLength());

		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;

		EnumWindows((WNDENUMPROC)NotifyWIAAppEnum, (LPARAM) dwPID) ;
		CloseHandle(hProcess);

		// we sent our messages, now close this process
		return FALSE;
	} else {
		m_bPersistentActivationByWIA = TRUE;
		return TRUE;
	}
}

CString CNxTWAIN::GetWIADevice()
{
	return m_strWIADevice;
}

long CNxTWAIN::GetWIAInfoFileNumber()
{
	return m_nWIAInfoFileNumber;
}

void CNxTWAIN::Unregister()
{
	// First make sure sti is available  on this machine
	if (g_dllSti.IsFunctionExported(StiCreateInstance) == NULL) {
		// It's not so we have to fail right off the bat
		return;
	}

	IStillImage* isi = NULL;
	HRESULT hRes = g_dllSti.StiCreateInstance(AfxGetApp()->m_hInstance,
		STI_VERSION,
		&isi,
		NULL);

	if (hRes != S_OK)
		return;

	isi->UnregisterLaunchApplication(L"NexTech Practice");
	isi->Release();
}

BOOL CNxTWAIN::ActivatedByCamera()
{
	return m_bPersistentActivationByCamera;
}

void CNxTWAIN::ResetCameraActivate(BOOL bActivate)
{
	m_bPersistentActivationByCamera = bActivate;
}

BOOL CNxTWAIN::ActivatedByWIA()
{
	return m_bPersistentActivationByWIA;
}

void CNxTWAIN::ResetWIAActivate(BOOL bActivate)
{
	m_bPersistentActivationByWIA = bActivate;
}

long CNxTWAIN::GetActiveDeviceID()
{
	return m_nActiveDeviceID;
}
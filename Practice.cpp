// Practice.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "ChildFrm.h"
#include "PracticeDoc.h"
#include "PatientView.h"
#include "SchedulerView.h"
#include "AdminView.h"
#include "InvView.h"
#include "RegSvr32.h"
//#include "PerformView.h"
#include "MarketView.h"
#include "FinancialView.h"
#include "ReportView.h"
//#include "ReportDocView.h"
#include "ContactView.h"
#include "LetterView.h"
#include "DontShowDlg.h"
#include "LicenseDlg.h"
#include "AboutDlg.h"
#include "LoginDlg.h"
#include "Client.h"
#include "MessagerDlg.h"
#include "Upgrade.h"
#include "GlobalUtils.h"
#include "nxtwain.h"
#include "ChooseAdminDlg.h"
#include "VersionInfo.h"
#include "GlobalDataUtils.h"
#include "NxSecurity.h"
#include "LinksView.h"				// (d.thompson 2009-11-16) - PLID 36134
#include "SurgeryCenterView.h"
#include "SharedPathConflictDlg.h"

#include "NxServer.h"
#include "PPCLink.h"

#include "GlobalFinancialUtils.h"
#include "FinanceChargesDlg.h"

#include "DocBar.h"

#include "GlobalReportUtils.h"
#include "NxGdiPlusUtils.h"

// (r.galicki 2008-10-13 13:36) - PLID 31373 - Labs Needing Attention dialog displayed on login
#include "LabFollowUpDlg.h"

#include "SecurityUtils.h"

#include "RegUtils.h"

#include "InternationalUtils.h"

// (a.walling 2010-06-18 10:46) - PLID 39195 - Need to use our override COccManager. This is in the SDK folder.
#include "NxOccManager.h"
#include "RemoteDataCache.h"
#include <PrintUtils.h>
#include <MiscSystemUtils.h>
#include "FileUtils.h"	// (j.armen 2011-10-26 17:10) - PLID 45795
#include "NxAPIManager.h"
#include "NxAPIUtils.h"
#include "NxWordProcessorLib/GenericWordProcessorManager.h"

#include "EmrRc2.h"
#include "CaseHistoryDlg.h"
#include "DeviceImportMonitor.h"

#include "NxResProtocol.h"
#include "NxProtocolRoot.h"
#include "NxProtocolEmr.h"
#include <NxAdvancedUILib/NxSettingsStore.h>

#include <NxAlgorithm.h>

// (j.gruber 2012-06-19 11:57) - PLID 48690
#include <WindowlessTimer.h>

#include <NxPracticeSharedLib/VersionUtils.h>

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get WordApp[lication] out of stdafx
#include <NxCompressUtils.h>

#include <boost/bind.hpp>

// (a.walling 2012-09-05 14:20) - PLID 52494
#include <NxExceptionLib/NxDragons.h>
#include "AuditTrail.h"

#include "NxCache.h"

#include "NxWIA.h" // (a.walling 2014-03-12 12:31) - PLID 61334 - reduce stdafx dependencies and #imports

#include "ASDDlg.h" // (a.wilson 2014-10-09 11:58) - PLID 63567
#include <NxPracticeSharedLib/SharedFirstDataBankUtils.h>	

//TES 3/2/2015 - PLID 64737 
#include <NxDebugUtils.h>

#include <NxAPILib/NexTech.Accessor.SxS.h>

// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - use CNxAdvancedUIApp

using namespace SecurityUtils;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// (z.manning 2012-08-08 09:28) - PLID 52026 - Thread and bool for preloading accessor
static CWinThread* g_pPreLoginThread;
static bool g_bPreLoginThreadRan = false;

// (z.manning 2012-08-08 10:56) - PLID 52026
static UINT PreLogin_Thread(LPVOID pParam)
{
	OleInitialize(NULL);

	try
	{
		// (z.manning 2012-08-08 10:57) - PLID 52026 - Let's call a simple API method in this thread (that doesn't actually
		// do anything) to cause any managed assemblies (including NexTech.Accessor.dll) to load ahead of time to prevent delays
		// later since loading managed assemblies is so slow.
		CNxAPI api(GetAPIAddress(GetSubRegistryKey()));
		api.GetAPI()->NoOp(0);
	}
	NxCatchAllIgnore();

	OleUninitialize();
	return 0;
}

// (z.manning 2012-08-08 10:56) - PLID 52026
void StartPreLoginThread() 
{
	if (g_bPreLoginThreadRan) {
		return;
	}

	g_bPreLoginThreadRan = true;

	if (NxRegUtils::ReadLong(CString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\") + CString("DisablePreLoginThread"), FALSE)) {
		return;
	}

	g_pPreLoginThread = AfxBeginThread(PreLogin_Thread, NULL, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
	g_pPreLoginThread->m_bAutoDelete = FALSE;
	g_pPreLoginThread->ResumeThread();

	return;
}

// (z.manning 2012-08-08 10:56) - PLID 52026
void WaitForPreLoginThread()
{
	if (!g_bPreLoginThreadRan || !g_pPreLoginThread) {
		return;
	}

	DWORD dwResult = ::WaitForSingleObject(g_pPreLoginThread->m_hThread, 0);

	if (WAIT_TIMEOUT == dwResult) {
		g_pPreLoginThread->SetThreadPriority(THREAD_PRIORITY_NORMAL);

#ifdef _DEBUG
		static const DWORD dwWait = 90000;
#else
		static const DWORD dwWait = 30000;
#endif

		dwResult = ::WaitForSingleObject(g_pPreLoginThread->m_hThread, dwWait);
	}

	CWinThread* pThread = NULL;
	std::swap(pThread, g_pPreLoginThread);

	if (WAIT_OBJECT_0 == dwResult) {
		delete pThread;
	}
	else {
		// well, it's not responding for 30 seconds, the best thing to do probably is just to let it go
		// the crt will terminate it anyway, the same as if we do it here, but with the benefit of doing
		// so closer to the natural termination point of the process.
		pThread->m_bAutoDelete = TRUE;
	}
}


/// CPracticeAtlMfcModule

// (a.walling 2012-04-25 16:21) - PLID 49987 - Instantiate our global ATL module

class CPracticeAtlMfcModule :
	public CAtlMfcModule
{
public:
	CPracticeAtlMfcModule()
	{}

	// (a.walling 2013-06-27 13:15) - PLID 57348 - The AFX_MANAGE_STATE is only implemented
	// in CAtlMfcModule if _USR_DLL is defined; we are an exe, we always have a static module
	// state that needs to be kept consistent, so manually override and manage state for the
	// module reference count functions
	
	virtual LONG Lock() throw() override
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		AfxOleLockApp();
		return AfxGetModuleState()->m_nObjectCount;
	}

	virtual LONG Unlock() throw() override
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		AfxOleUnlockApp();
		return AfxGetModuleState()->m_nObjectCount;
	}

	virtual LONG GetLockCount() throw() override
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		return AfxGetModuleState()->m_nObjectCount;
	}

	// (a.walling 2012-04-25 16:21) - PLID 49987 - If we were to expose a lib or etc we could put some macros here
};

CPracticeAtlMfcModule _AtlModule;

///

/// CPracticeInstanceData

// (a.walling 2012-04-25 16:19) - PLID 49987 - This can be used to construct and destroy objects in InitInstance and ExitInstance
// without polluting the header file here with things that are unnecessary

class CPracticeInstanceData
{
public:
	CPracticeInstanceData()
		: m_bInitialized(false)
	{
		Initialize();
	}

	~CPracticeInstanceData()
	{
		Uninitialize();
	}

protected:
	// (a.walling 2012-09-04 12:10) - PLID 52438 - [un]register nx:// protocol and nx://host handlers

	void Initialize()
	{
		if (m_bInitialized) return;

		m_bInitialized = true;

		try {
			// any InitInstance stuff goes here

			// (a.walling 2012-04-25 17:49) - PLID 49996 - nxres protocol
			m_resProtocol.Register();
			m_nxProtocol.Register();

			Nx::Protocol::RegisterHost("emr", 
				boost::bind(&Nx::Protocol::Emr::OpenStream, _1, _2, _3)
			);

			// (a.walling 2013-07-18 10:24) - PLID 57624 - NxCache init!
			Nx::Cache::InitSystem();

		} NxCatchAll(__FUNCTION__);
		
		::CoGetClassObject(__uuidof(NXINKPICTURELib::NxInkPicture), CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, (void**)&m_pNxInkKeepAlive);
		ASSERT(m_pNxInkKeepAlive);
	}

	void Uninitialize()
	{
		if (!m_bInitialized) return;

		m_bInitialized = false;

		try {

			// any ExitInstance stuff goes here

			Nx::Protocol::UnregisterHost("emr");

			// (a.walling 2012-04-25 17:49) - PLID 49996 - nxres protocol
			m_resProtocol.Unregister();
			m_nxProtocol.Unregister();

			// (a.walling 2013-07-18 10:24) - PLID 57624 - NxCache destroy!
			Nx::Cache::Shutdown();

		} NxCatchAll(__FUNCTION__);
	}

	bool m_bInitialized;

	// (a.walling 2012-04-25 17:49) - PLID 49996 - nxres protocol
	Nx::Protocol::Registration<CNxResProtocol> m_resProtocol;
	Nx::Protocol::Registration<Nx::Protocol::Root> m_nxProtocol;
	
	IUnknownPtr m_pNxInkKeepAlive;
};

// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



void AddToValidModuleNames(const CString &strModuleName);
bool IsSuperUser();

// Yes, this IS supposed to create a new CMultiDocTemplate, but it is NOT a memory leak, despite what 
// bounds-checker tells you.  The CWinApp MFC class keeps a reference count of the document templates 
// that were added to the app, and when the app is "destructed", so are all the CMultiDocumentTemplate 
// objects.
// ALSO
// This certainly uses some very unusual c++ syntax, but it's legal and correct.  The reason all the 
// comma's are in there instead of semi-colons is because in case you want to set something EQUAL to 
// this statement (as if it were a function that returned a value), you can.  It "returns" the 
// CMultiDocTemplate* pDocTemplate.
#define InitModule(class_name, module_id, module_cntr_id, module_name) ( \
													AddDocTemplate(pInternal_Init_Module_Doc_Template = new CMultiDocTemplate(module_id, \
													RUNTIME_CLASS(CPracticeDoc), RUNTIME_CLASS(CChildFrame), \
													RUNTIME_CLASS(class_name))), pInternal_Init_Module_Doc_Template->SetContainerInfo(module_cntr_id), \
													AddToValidModuleNames(module_name), pInternal_Init_Module_Doc_Template)
#define BEGIN_INIT_MODULES() { CMultiDocTemplate *pInternal_Init_Module_Doc_Template;
#define END_INIT_MODULES() }

//#define CREATE_TTX_FILES
/////////////////////////////////////////////////////////////////////////////
// CPracticeApp

// (a.walling 2011-09-16 13:01) - PLID 45531 - Now derives from CWinAppEx
// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - use CNxAdvancedUIApp
BEGIN_MESSAGE_MAP(CPracticeApp, CNxAdvancedUIApp)
	//{{AFX_MSG_MAP(CPracticeApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CNxAdvancedUIApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CNxAdvancedUIApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CNxAdvancedUIApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPracticeApp construction

CPracticeApp::CPracticeApp()
:	m_dlgLogin(new CLoginDlg(NULL))
{
	m_hmutex = NULL;
	m_hFileFolderAvail = INVALID_HANDLE_VALUE;
	m_hNxLoginDatMutex = NULL;
	//TES 3/2/2015 - PLID 64736
	m_bDebugChangedThisSession = FALSE;
}
CPracticeApp::~CPracticeApp()
{
	if (m_dlgLogin) {
		delete m_dlgLogin;
		m_dlgLogin = NULL;
	}

	// (z.manning 2012-08-08 10:16) - PLID 52026 - Wait for the accessor preload thread to finish
	WaitForPreLoginThread();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPracticeApp object

CPracticeApp theApp;

// (a.walling 2008-04-10 08:34) - PLID 29497 - The one and only gdi+ initializer.
// (a.walling 2008-04-14 10:12) - PLID 29630 - Moved to NxGdiPlusUtils.h
extern NxGdi::CGDIPlusHolder g_GDIPlusHolder;


//DRT 7/3/02 - Added the registry base on to the end of the creation, so it will always give you a different mutex
//				depending on the registry setup we're currently looking at
// (a.walling 2006-11-10 10:20) - PLID 23294 - GetMutexName and GetMappedFileName functions now include the path
//				Also shortened base strings and made asame length to avoid hitting the unlikely MAX_PATH of 260
static const TCHAR cstrMutex[] =			_T("NexTech Practice MutexObj 0xf2b94da7");
static const TCHAR cstrMappedFileProcId[] = _T("NexTech Practice MappedFile 0d46b258");
static const TCHAR cstrNxLoginDatMutex[]  = _T("NexTech Practice NxLoginMut bb7e1093");

// This identifier was generated to be statistically unique for your app.
// You may change it if you prefer to choose a specific identifier.

// {F2B94DA7-9A7D-11D1-B2C7-00001B4B970B}
static const CLSID clsid =
{ 0xf2b94da7, 0x9a7d, 0x11d1, { 0xb2, 0xc7, 0x0, 0x0, 0x1b, 0x4b, 0x97, 0xb } };

/////////////////////////////////////////////////////////////////////////////
// CPracticeApp initialization

//WINOLEAPI  CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);
static bool Y2KCheck()//needs to be changed as time passes
{	
	int year = COleDateTime::GetCurrentTime().GetYear();

	if (year >= 2000 && year <= 2030)
	{
		return true;
	}
	else return false;
}

typedef BOOL (WINAPI * GDFSEPROC)(LPCSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);

const int WORKSTATION = 0;
const int SERVER = 1;

static bool DiskSpaceCheck (int size,/*in MB*/ int computer)
{

	GDFSEPROC pGetDiskFreeSpaceEx = (GDFSEPROC)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA");
	
	// On early versions of 95, the EX version of this function doesn't work, so if we can't 
	// load it, don't use it!  The ideal solution here would be to use the GetDiskFreeSpace 
	// (note no 'Ex') but I don't have time right now and it doesn't matter all that much

	//BVB the non-ex version won't work for drives larger than 2GB
	//which is more common that early versions of 95.  Keep using EX

	if (!pGetDiskFreeSpaceEx) return true;

	CString directory;

	ULARGE_INTEGER share,
					total,
					free;

	if (computer == SERVER)
		directory = GetSharedPath();
	// (j.armen 2011-10-24 13:52) - PLID 46139 - Check the disk space located at the practice working dir
	else directory = GetPracPath(PracPath::PracticePath);

	if (!pGetDiskFreeSpaceEx(directory, &share, &total, &free))
	{
#ifdef _DEBUG
		// (r.gonet 08/05/2014) - PLID 63098 - Clarified the warning message because it was misleading when I got it the other day.
		AfxMessageBox (FormatString("Could not obtain the amount of free space available for directory \"%s\". Please confirm your dock settings are correct, and try browsing to this path to confirm that it exists and that you have permission to access it.", directory));
#endif
		//LogDetail (FormatString("Could not obtain the amount of free space available for directory \"%s\". Please confirm your dock settings are correct, and try browsing to this path to confirm that it exists and that you have permission to access it.", directory));
		return true; //give them the benefit of the doubt
	}
	if (free.QuadPart>>20 < size)
		return false;
	else return true;
}

// (j.gruber 2014-01-24 12:51) - PLID 60468 - check if we are allowing this workstation to take up multiple licenses
BOOL GetAllowMultipleInstances()
{
	long nAllow = NxRegUtils::ReadLong(_T(GetRegistryBase() + "AllowMultipleInstances"), 0);
	if (nAllow == 0)
	{
		return FALSE;
	}
	else {
		return TRUE;
	}	
}


// (a.walling 2006-11-10 10:16) - PLID 23294 - Create a mutex name that varies on working path, so multiple
// practice instances can be run within the same window session.
CString GetMutexName()
{
	CString strRegKey = GetSubRegistryKey();
	strRegKey.MakeLower();

	// (j.gruber 2014-01-24 12:08) - PLID 60468 - do we want 2 instances
	BOOL bAllowMultipleInstances = GetAllowMultipleInstances();

	if (!bAllowMultipleInstances)
	{
		return strRegKey + " " + cstrMutex;
	}
	else {

		// (j.armen 2011-10-24 13:22) - PLID 45796 - The mutex should be based on the practice working path
		CString strWorkingPath = GetPracPath(PracPath::PracticePath);
		strWorkingPath.TrimRight('\\');
		strWorkingPath.Remove(':');
		strWorkingPath.Replace('\\', '.');
		strWorkingPath.MakeLower();

		return strRegKey + "." + strWorkingPath + "." + cstrMutex;
	}
}

CString GetNxLoginDatMutexName()
{
	CString strRegKey = GetSubRegistryKey();
	strRegKey.MakeLower();

	// (j.armen 2011-10-24 14:04) - PLID 45796 - Need to create a name for the LoginDatMutex.
	// If we are concurrent, then we will need to use the session path so that multiple mutexes can exist in the same practice working dir.
	// If we are not concurrent, then we want the mutex to limit one workstation to this practice working dir.
	CString strWorkingPath = GetPracPath(g_pLicense->IsConcurrent()?PracPath::SessionPath:PracPath::PracticePath);
	strWorkingPath.TrimRight('\\');
	strWorkingPath.Remove(':');
	strWorkingPath.Replace('\\', '.');
	strWorkingPath.MakeLower();

	// (b.cardillo 2011-01-06 20:34) - PLID 33748 - We include the subkey and working path to ensure uniqueness 
	// while protecting against different sessions trying to reuse a single working path that's mapped to two 
	// different physical paths.
	return "Global\\" + strRegKey + "." + strWorkingPath + "." + cstrNxLoginDatMutex;
}

// (a.walling 2006-11-10 10:16) - PLID 23294 - Create a mapped file name that varies on working path,
// so multiple practice instances can be run within the same window session.
CString GetMappedFileName()
{
	CString strRegKey = GetSubRegistryKey();
	strRegKey.MakeLower();

	// (j.gruber 2014-01-24 12:08) - PLID 60468 - do we want 2 instances
	BOOL bAllowMultipleInstances = GetAllowMultipleInstances();

	if (!bAllowMultipleInstances)
	{
		return strRegKey + " " + cstrMappedFileProcId;
	}
	else {

		// (j.armen 2011-10-24 13:34) - PLID 45796 - the mapped file name should be based on the practice working path
		CString strWorkingPath = GetPracPath(PracPath::PracticePath);
		strWorkingPath.TrimRight('\\');
		strWorkingPath.Remove(':');
		strWorkingPath.Replace('\\', '.');
		strWorkingPath.MakeLower();

		return strRegKey + "." + strWorkingPath + "." + cstrMappedFileProcId;
	}
}

HANDLE CreateExclusiveSecureMutex(const CString &strMutexName)
{
	HANDLE hMut = CreateSecureMutex(strMutexName, TRUE, TRUE, MUTEX_ALL_ACCESS);
	if (hMut != NULL) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			CloseHandle(hMut);
			SetLastError(ERROR_ALREADY_EXISTS);
			return NULL;
		} else {
			return hMut;
		}
	} else {
		return NULL;
	}
}

BOOL CPracticeApp::EnsureWorkingPathAvail(BOOL bDisplayErrors)
{
	// (j.armen 2011-10-26 17:12) - PLID 45796 - If we don't have a license, we can't ensure a working path, so fail
	if(!g_pLicense) return FALSE;

#ifdef _DEBUG
	DWORD dwNxLoginDatError = 0;
	DWORD dwNxLoginDatMutexError = 0;
#endif

	CString strNxLoginDatMutexName = GetNxLoginDatMutexName();
	//DRT 7/3/02 - This will now use NxLogin<key>, where key is the registry subkey that we are currently working in.
	//			For example, working in the internal key, this will use NxLoginInternal.dat.  If we are just using regular
	//			Practice, it will continue to use the regular NxLogin.dat file
	// (j.armen 2011-10-24 14:11) - PLID 45796 - 
	// If we are concurrent, then the dat should be located in the specific session folder {PracticeWorkingDir}/Sessions/{Subkey}/{GUID}/NxLogin.dat
	// If we are not concurrent, then the dat should be located in the {PracticeWorkingDir}/Sessions/{Subkey}/NxLogin.dat
	// (c.haag 2015-03-02) - PLID 49928 - Persist the full NxLogin.dat path here
	CString strDat = (g_pLicense->IsConcurrent()
		? GetPracPath(PracPath::SessionPath) 
		: (GetPracPath(PracPath::PracticePath) ^ "Sessions" ^ (GetSubRegistryKey().IsEmpty() ? "Default" : GetSubRegistryKey()))) ^ "NxLogin.dat";

	// Try to open the file (loop up to 5 times)
	for (int nAttempt = 0; ; nAttempt++) {

		// (b.cardillo 2011-01-06 20:34) - PLID 33748 - We now hold a global shared mutex for our working path too.
		if (m_hNxLoginDatMutex == NULL) {
			m_hNxLoginDatMutex = CreateExclusiveSecureMutex(strNxLoginDatMutexName);
#ifdef _DEBUG
			dwNxLoginDatMutexError = GetLastError();
#endif
		}

		if (m_hFileFolderAvail == INVALID_HANDLE_VALUE) {
			// Try to open the lock file
						
			// (b.cardillo 2009-03-26 16:03) - PLID 14887 - We used to store the username inside the file, to fix this pl 
			// item we had to mark the file with delete_on_close, which means nobody else is allowed to open it (well, we 
			// COULD let others open it, but they would also have to pass the delete_on_close flag, which would mean that 
			// when THEY close it, the file would be marked for deletion and then nobody else would be able to open it no 
			// matter what).  So now instead of storing the username as text inside the file, we create the file with the 
			// security descriptor of the current user so other people can tell who created it.
			m_hFileFolderAvail = CreateSecureFile(strDat, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE|FILE_ATTRIBUTE_NORMAL, NULL, TRUE, FILE_ALL_ACCESS);

#ifdef _DEBUG
			dwNxLoginDatError = GetLastError();
#endif
		}


		if (m_hNxLoginDatMutex != NULL && m_hFileFolderAvail != INVALID_HANDLE_VALUE) {
			// It opened okay
			break;
		} else if (nAttempt < 5) {
			// Either the file or mutex didn't open but we're willing to try again
			// This often happens because the user "triple-click" or in some other 
			// way opened more than one instance of practice simultaneously, so we
			// need to try to show that other instance before we sleep for a second
			if (ShowOtherInstance()) {
				// There was another instance so we're done, close our instance
				return FALSE;
			} else {
				// There was not another instance so sleep for a second and then try again
				Sleep(1000);
			}
		} else {
			// It didn't open but we're sick of trying again and again so just break 
			// out of the loop and deal with the failure
			break;
		}
	}
	
	// Now handle it if it was successful or not
	if (m_hNxLoginDatMutex != NULL && m_hFileFolderAvail != INVALID_HANDLE_VALUE) {
		// Got the file and the mutex so we know no one else is using this 
		// directory
		// (b.cardillo 2009-03-26 19:40) - PLID 14887 - We used to write our info to the file, but now we just open the 
		// file with ourselves as owner, so that others can see who opened it just from that.
		return TRUE;
	} else {

#ifdef _DEBUG
		// By design we are supposed to get here if someone else is using our working directory, so 
		// we'll have a sharing violation.  The only reason we'd want to tell someone debugging the 
		// program anything special here is if we got here for some OTHER reason.
		if (m_hFileFolderAvail == INVALID_HANDLE_VALUE && dwNxLoginDatError != ERROR_SHARING_VIOLATION || 
			m_hNxLoginDatMutex == NULL && dwNxLoginDatMutexError != ERROR_ALREADY_EXISTS)
		{
			CString strDebugMsg;
			if (m_hFileFolderAvail == INVALID_HANDLE_VALUE && dwNxLoginDatError != ERROR_SHARING_VIOLATION) {
				strDebugMsg += "dwNxLoginDatError - " + FormatError(dwNxLoginDatError) + "\r\n";
			}
			if (m_hNxLoginDatMutex == NULL && dwNxLoginDatMutexError != ERROR_ALREADY_EXISTS) {
				strDebugMsg += "dwNxLoginDatMutexError - " + FormatError(dwNxLoginDatMutexError) + "\r\n";
			}
			AfxMessageBox(strDebugMsg);
		}
#endif

		if (bDisplayErrors) {
			CString strExistingOwner;
			if (m_hFileFolderAvail == INVALID_HANDLE_VALUE) {
				//DRT 7/3/02 - This will now use NxLogin<key>, where key is the registry subkey that we are currently working in.
				//			For example, working in the internal key, this will use NxLoginInternal.dat.  If we are just using regular
				//			Practice, it will continue to use the regular NxLogin.dat file
				strExistingOwner = GetFileOwner(strDat);
			} else if (m_hNxLoginDatMutex == NULL) {
				// (b.cardillo 2011-01-06 20:34) - PLID 33748 - The file was created just fine, but the mutex couldn't be created 
				// exclusively, which we assume means someone else must be holding it; we get that owner's name here.
				strExistingOwner = SecurityUtils::GetObjectOwnerName(strNxLoginDatMutexName);
			} else {
				// This should never happen because if we reached this section of code it's because either the dat file couldn't 
				// be created or the mutex couldn't be created.  If we reach this ASSERT() something is wrong with our logic.
				ASSERT(FALSE);
				strExistingOwner = "unknown";
			}

			// (j.armen 2011-10-25 11:16) - PLID 45796 - use the session path if we are concurrent, or the practice path if we are not.
			CString strMsg;
			strMsg.Format(
				"Practice could not be started because \n\n"
				" %s \n\n"
				"is still logged into Practice. \n\n"
				"Your working directory: \n\n"
				" %s \n\n"
				"If you continue to have trouble logging into Practice, \n"
				"please close all programs and then shut down and restart \n"
				"this machine.", 
				strExistingOwner, GetPracPath(g_pLicense->IsConcurrent()?PracPath::SessionPath:PracPath::PracticePath));
			// Report the problem and fail
			AfxMessageBox(strMsg);
		}
		return FALSE;
	}
}

// (c.haag 2013-01-15) - PLID 59257 - This function is called to warn users if another
// subkey has the same shared path we have.
void CPracticeApp::WarnOtherSubkeysWithSameSharedPath()
{
	// Set to true if we should check and present a warning. Note that if the preference is turned off,
	// we do not check for conflicts (meaning we don't log the preference of conflicts).
	BOOL bWarn = TRUE;
#ifdef _DEBUG
	bWarn = FALSE;
#else
	bWarn = (GetRemotePropertyInt("WarnOtherSubkeysWithSameSharedPath", 1) == 0) ? FALSE : TRUE;
#endif

	if (bWarn && OtherSubkeysHaveSameSharedPath())
	{
		CSharedPathConflictDlg dlg(NULL);
		dlg.DoModal();
	}
}

// (c.haag 2013-01-15) - PLID 59257 - This function determines whether this workstation
// has at least one other subkey with the same shared path we have.
BOOL CPracticeApp::OtherSubkeysHaveSameSharedPath()
{
	CiString strMySharedPath;
	CString strUniqueFileName;
	BOOL bTestUniqueFile = FALSE;
	BOOL bSubkeyFound = FALSE;

	try
	{
		const CString strRootRegBase = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech";
		CiString strMySubkey = GetSubRegistryKey();
		strMySharedPath = GetSharedPath();
		strMySharedPath.TrimRight('\\');

		// Get all of the subkeys
		CStringArray astrKeyNames;
		NxRegUtils::GetSubKeyNames(strRootRegBase, astrKeyNames);
		astrKeyNames.Add(""); // Default subkey

		// Create a file in our current shared path. When we test the other paths, we will see if the file exists
		// there; and if so, we have a match.
		GUID guid;
		HRESULT hr = CoCreateGuid(&guid);
		if (hr == S_OK)
		{
			RPC_CSTR guidStr;
			if(UuidToString(&guid, &guidStr) == RPC_S_OK)
			{
				strUniqueFileName = CString("SharedPathConfTest") + (LPTSTR)guidStr;
				RpcStringFree(&guidStr);
				// Now that we have our filename, go create the file
				HANDLE hFile = CreateFile(strMySharedPath ^ strUniqueFileName, GENERIC_WRITE, 0,  NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (INVALID_HANDLE_VALUE != hFile)
				{
					// Success
					bTestUniqueFile = TRUE;
					CloseHandle(hFile);
				}
			}
		}

		// Now iterate through all the subkeys
		int nKeys = astrKeyNames.GetCount();
		for (int i=0; i < nKeys; i++)
		{
			CiString strSubkey = astrKeyNames[i];
			// (j.gruber 2014-02-11 12:58) - PLID 60730 - this should be negated, otherwise it warns every time
			if (strSubkey != strMySubkey)
			{
				CiString strSharedPath = (strSubkey.IsEmpty()) ? 
					NxRegUtils::ReadString(strRootRegBase + "\\SharedPath")
					: NxRegUtils::ReadString(strRootRegBase + "\\" + strSubkey + "\\SharedPath");
				strSharedPath.TrimRight('\\');

				if (strSharedPath == strMySharedPath)
				{
					// We found a shared path that matches ours
					Log("Conflicting subkey was detected: Us = '%s' Them = '%s' SharedPath = '%s'"
						,strMySubkey, strSubkey, strMySharedPath);
					bSubkeyFound = TRUE;
				}
				// If a simple path comparison reveals no issue; try checking the path for the file.
				else if (bTestUniqueFile && FileUtils::DoesFileOrDirExist(strSharedPath ^ strUniqueFileName))
				{
					// We found a shared path that points to the same place
					Log("Conflicting subkey was detected: Us = '%s' Them = '%s' SharedPath = '%s'"
						,strMySubkey, strSubkey, strMySharedPath);
					bSubkeyFound = TRUE;
				}
			}
		}
	}
	catch (CException *e)
	{
		// We failed to do the check. Just log it and move on.
		char errorMessage[4096];
		if (!e->GetErrorMessage(errorMessage, 4096)) {
			strcpy(errorMessage, "Unknown CException!");
		}
		Log("Error while checking for conflicting subkeys: %s", errorMessage);
	}
	catch (...)
	{
		Log("Unknown exception while checking for conflicting subkeys!");
	}
	
	// Clean up our temporary file
	if (bTestUniqueFile && strMySharedPath.GetLength() > 0 && strUniqueFileName.GetLength() > 0)
	{
		if (!DeleteFile(strMySharedPath ^ strUniqueFileName))
		{
			Log("Failed to delete temp file %s!", strMySharedPath ^ strUniqueFileName);
		}
	}

	return bSubkeyFound;
}

// (a.wilson 2014-09-30 15:03) - PLID 63567 - detect and alert user if they need to be aware of the issue.
void CPracticeApp::CheckFDBMedsHistory()
{
	using namespace FirstDataBankUtils;

	try {
		_RecordsetPtr prs = CreateRecordsetStd("SELECT Status, RunDate, LastModifiedDate FROM CheckFDBMedsRunHistoryT WHERE Status IS NOT NULL");
		//If nothing is in here then the test hasn't been run yet. return since we are done.
		if (prs->eof) {
			return;
		}

		//If status is set to a finished state then we do not need to continnue.
		CheckFDBMedsRunHistoryStatus eStatus = (CheckFDBMedsRunHistoryStatus)AdoFldLong(prs, "Status", eNoProblemsFound);
		if (eStatus == eNoProblemsFound || eStatus == eFoundAndAutoResolved || eStatus == eAllProblemsResolved) {
			return;
		}

		//If problems were detected and for whatever reason the SendError() failed then alert the user.
		//If problems were detected and SendError() succeeded but has not been started for atleast 21 days then alert the user.
		//If problems were detected and SendError() succeeded and resolution is in progress but not worked on for atleast 21 days then alert the user.
		COleDateTime dtRunDate = AdoFldDateTime(prs, "RunDate");
		COleDateTime dtLastModifiedDate = AdoFldDateTime(prs, "LastModifiedDate", COleDateTime::GetCurrentTime());
		long nDaysIgnored = COleDateTimeSpan(COleDateTime::GetCurrentTime() - dtRunDate).GetDays();
		long nDaysLastProgress = COleDateTimeSpan(COleDateTime::GetCurrentTime() - dtLastModifiedDate).GetDays();

		if (eStatus == eFoundAndSubmissionFailed || 
			(eStatus == eFoundAndSubmitted && nDaysIgnored >= 21) || 
			(eStatus == eResolutionInProgress && nDaysLastProgress >= 21)) {
			//report to the user that the fdbmed check found bad meds (nicely) and support needs to fix them.
			CASDDlg dlg(NULL);
			// (j.jones 2015-03-03 10:46) - PLID 65106 - reworded this dialog
			dlg.SetParams(
				"To ensure consistency, please call 866-654-4396, option 3, then option 2.",
				"Patient Medication Issue", "We have detected a change in the medication naming.", "I have read the warning above and will call support at the phone number listed above.", false);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

BOOL CPracticeApp::CreateFonts()
{
	// (a.walling 2016-06-01 11:12) - NX-100195 - use Segoe UI instead of Arial for these special fonts

	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	if (!CreateCompatiblePointFont(&m_boldFont, 100, "Segoe UI Bold"))
		return FALSE;

	if (!CreateCompatiblePointFont(&m_notboldFont, 100, "Segoe UI"))
		return FALSE;

	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	if (!CreateCompatiblePointFont(&m_subtitleFont, 120, "Segoe UI Bold"))
		return FALSE;

	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	if (!CreateCompatiblePointFont(&m_titleFont, 130, "Segoe UI Bold"))
		return FALSE;

	// (j.jones 2016-05-12 14:39) - NX-100625 - added a non-bold header font
	if (!CreateCompatiblePointFont(&m_headerFont, 130, "Segoe UI"))
		return FALSE;

	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	if (!CreateCompatiblePointFont(&m_fixedFont, 80, "Courier New"))
		return FALSE;
	
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	if (!CreateCompatiblePointFont(&m_multiresFont, 90, "Segoe UI Bold"))
		return FALSE;

	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	if (!CreateCompatiblePointFont(&m_smallFont, 70, "Segoe UI"))
		return FALSE;

	return TRUE;
}

// (b.cardillo 2005-10-31 17:09) - This function is identical between $/Practice/Practice.cpp 
// and $/Tools/Preferences/EMRPrefsDlg.cpp.  Ultimately it should be verified for modularity, 
// atomicity, etc, and moved to a shared utility so that updates don't have to be ported 
// between these two locations.  In the mean time, please port any changes you make.
BOOL GetFontFromVariant(OUT LOGFONT *plfLogFontOut, IN _variant_t &varVariant, OPTIONAL LOGFONT *plfDefault)
{
	if (varVariant.vt == VT_NULL || varVariant.vt == VT_EMPTY) {
		// Doesn't exist, so set the default if one was given, otherwise clear the out parameter
		if (plfDefault) {
			memcpy(plfLogFontOut, plfDefault, sizeof(LOGFONT));
		} else {
			memset(plfLogFontOut, 0, sizeof(LOGFONT));
		}
		return FALSE;
	} else {
		BOOL bAns;
		COleSafeArray sa;
		sa.Attach(varVariant);
		BYTE *pSafeArrayData = NULL;
		sa.AccessData((LPVOID *)&pSafeArrayData);
		if (pSafeArrayData) {
			if (pSafeArrayData[0] == 1) {
				// Indicates the following sizeof(LOGFONTA) bytes are a LOGFONTA struct
				memcpy(plfLogFontOut, pSafeArrayData + 1, sizeof(LOGFONTA));
				bAns = TRUE;
			} else {
				// We don't support any other formats right now, so load the default
				ASSERT(FALSE);
				if (plfDefault) {
					memcpy(plfLogFontOut, plfDefault, sizeof(LOGFONT));
				} else {
					memset(plfLogFontOut, 0, sizeof(LOGFONT));
				}
				bAns = FALSE;
			}
		}
		sa.UnaccessData();
		sa.Clear();
		return bAns;
	}
}

CFont *CPracticeApp::GetPracticeFont(EPracticeFontType epftFontType)
{
	switch (epftFontType) {
	case pftGeneralBold:
		return &m_boldFont;
		break;
	case pftGeneralNotBold:
		return &m_notboldFont;
		break;
	case pftSubtitle:
		return &m_subtitleFont;
		break;
	case pftTitle:
		return &m_titleFont;
		break;
	// (j.jones 2016-05-12 14:39) - NX-100625 - added a non-bold title font
	case pftHeader:
		return &m_headerFont;
		break;
	case pftFixed:
		return &m_fixedFont;
		break;
	case pftMultiRes:
		return &m_multiresFont;
		break;
	case pftSmall:
		return &m_smallFont;
		break;
	default:
		ASSERT(FALSE);
		return &m_boldFont;
		break;
	}
}

// This code is identical between Practice.cpp (of the Practice project) and PracticeEngine.cpp (of 
// the NxWeb project).  If you change it in one place please make the corresponding change in the 
// other.  Ultimately both apps should use truly shared code.
// (b.cardillo 2007-02-01 13:47) - It is now also shared with NxInternalWeb (connectioninfo.cpp).
// (j.jones 2012-07-20 15:33) - PLID 51672 - moved this to VersionUtils in NxPracticeSharedLib
//CString GetDatabaseMaxVersion(_Connection *lpConn)

BOOL CPracticeApp::PreLogin()
//Do anything here that should be done before the user logs in
{
	
/* control container calls this
	if (!AfxOleInit()) {
		// Could not initialize COM so we're pretty much screwed
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
*/

	// (a.walling 2011-09-16 13:01) - PLID 45531 - CleanState will eliminate saved state in the registry
	//CleanState();

	//NxSocketUtils needs to be initialized before the license.
	NxSocketUtils::Initialize();

	// Try to use new DAO 3.6 dlls if possible

	// CAH note: If this crashes on CHtmlView::GetApplication(), NxStandard needs to be
	// built on this machine.
	if (IsJet40() && IsAccess2k()) {
		//LogDetail("Using 3.6");
		AfxGetModuleState()->m_dwVersion = 0x0601;
	}

	if (!Y2KCheck())// System clock must (should -RAC) be set to a reasonable time -BVB
	{	if (AfxMessageBox (CString(
			"Due to a problem with your system clock, Practice may not operate correctly.\n") +
			"Please set your clock to the correct date.\n\n" +
			"Continue?", MB_YESNO) == IDNO) {
			return FALSE;
		}
	}
	//disk space free
	if (!DiskSpaceCheck(100, SERVER))
	{	AfxMessageBox ("Practice requires at least 100 MB free on your server to run.  Please contact your system administrator about freeing up disk space.");
		return FALSE;
	}
	if (!DiskSpaceCheck(50, WORKSTATION))
	{	AfxMessageBox ("Practice requires at least 50 MB free locally to run.  Please contact your system administrator about freeing up disk space.");
		return FALSE;
	}

	AfxInitRichEdit();//Rich edit support

	if (!CreateFonts()) {
		AfxMessageBox("Practice was unable to generate the fonts necessary to display dialog boxes.  Your computer may be running low on resources.  Please restart and try again.");
		return FALSE;
	}

	// Finally open the database (this should be the first time the database is being opened)
	{
		BOOL bRunTroubleshooter = FALSE;
		while (!EnsureSqlServer(bRunTroubleshooter)) {
			if (bRunTroubleshooter) {
				if (!RunTroubleshooter())
					return FALSE; // This means the troubleshooter could not resolve the problem
				bRunTroubleshooter = FALSE;
			} else {
				return FALSE; // This means there is a problem and the troubleshooter cannot
								// resolve it if it were running
			}
		}
	}
	
#ifdef CREATE_TTX_FILES
//		Define this and give it the index of the report you wish to create the .ttx file for
// (j.armen 2011-12-30 15:05) - PLID 47273 - We want these to go back to our working directory so they will be easier to find.
// (r.gonet 2015-05-20) - No PLID - A warning. DO NOT use the CreateAllTtxFiles(long nID) overload here. Use the CreateAllTtxFiles(long nID, CString strPath, long nCustomReportID = -1) overload.
// The former does not respect our hard coded TTX definitions and instead tries to guess the field lengths, which may be different from the hard coded TTX definitions, causing Verify errors. 
// The latter respects our hard coded TTX definitions.
	CString strPath = GetPracPath(PracPath::PracticePath);
	SetCurrentDirectory(strPath);
	m_dlgLogin->SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	CreateAllTtxFiles(762, strPath);

	AfxMessageBox(FormatString("Report and TTX Files should be located in %s", GetPracPath(PracPath::PracticePath)));
	return FALSE;
#endif

	//DRT 7/8/02 - Check the version of Practice against the version in UpgradeT
	//ditch this in debug mode too
	//TES 8/2/2005 - Moved this to before the license initialization.
#ifndef _DEBUG
	try {
		//now we know what the max version is, so compare that against what we want.  If it's the  same, we're all set.
		// (j.jones 2012-07-20 15:34) - PLID 51672 - GetDatabaseMaxVersion is now in NxPracticeSharedLib
		CString str = VersionUtils::GetDatabaseMaxVersion(GetRemoteData());
		if(str != DATA_VERSION_TEXT) {
			//DRT 6/18/03 - We decided we should check this for internal, because you can run into issues if you accidentally run
			//		out of sourcesafe and have r:Internal in your arguments.  The internal data itself will still be screwy, we're
			//		just going to manually update that table to insert the right value in there to keep it up to date.
//			if(!IsNexTechInternal()) {
			CString strMessage;
			// (z.manning 2008-09-30 11:14) - PLID 31532 - Show the db version in this message;
			strMessage.Format("Your database version (%s) is out of date with this version of Practice (%s).  Please ensure that you have upgraded to the most current version.", str, DATA_VERSION_TEXT);
			AfxMessageBox(strMessage);
			//DRT 4/3/03 - PL ID #7340 - make the program abort if data version does not match
			return false;
//			}
		}
	} NxCatchAll("Error connecting to the database.");
#endif

	// (z.manning, 06/14/2006) - PLID 21021 - Let's check the database size and make sure it's
	// not too big for whatever version of SQL Server they have installed.
	try {
		// (b.savon 2011-10-05 15:25) - PLID 44428 - Change nFileSize and nLimit to MB (long) instead of (__int64)
		long nFileSize = GetCurrentDatabaseSize();
		BOOL bWarn = FALSE;
		// (j.jones 2013-03-29 11:50) - PLID 54281 - Added GetMaximumDatabaseSize(), which just returns the maximum
		// size of a database, in MB, for the current version of SQL. For example, 4GB will return a long of 4096.
		// If there is no limit, we return -1.
		long nLimit = GetMaximumDatabaseSize();

		// If we are within .5 GB, warn them.
		// (b.savon 2011-10-05 15:31) - PLID 44428 - Change to 1GB in MB
		const long nGigabyte = 1024;
		if( (nLimit != -1) && (nFileSize > (nLimit - nGigabyte/2)) ) 
		{
			CString strWarning = FormatString("Your database has reached a size of %.2f GB. "
				"The maximum size of a database for your version of SQL Server is %.0f GB.\r\n\r\n"
				"Please contact NexTech Technical Support for assistance.",
				(double)nFileSize / nGigabyte, (double)nLimit / nGigabyte);
			MsgBox(strWarning);
		}
		
	} NxCatchAll("Failure checking the database's size.");

	// (a.walling 2010-07-29 09:48) - PLID 39871 - Initial pre-login bulk-cache for non-username-dependent properties.
	// (j.armen 2011-10-24 13:53) - PLID 46139 - GetPracPath is using ConfigRT
	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Snapshot always OK, moved to NxAdo
	// (j.jones 2016-03-08 13:58) - PLID 68478 - LastLicenseUpdateAttempt is now an encrypted image
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	g_propManager.BulkCache("PreLoginProps", propbitNumber | propbitDateTime | propbitText | propbitImage,
		"((Username = '<None>' OR Username = '%s') AND Name IN (\r\n"	// None
		"	'LastLicenseUpdateAttemptImage',\r\n"
		"	'LoginBanner',\r\n"
		"	'DisplayLoginUsersInUpperAndLowerCases',\r\n"
		"	'LockAccountsOnFailedLogin',\r\n"
		"	'FailedLoginsAllowed',\r\n"
		"	'NewCrop_ProductionStatusOverride',\r\n"// (e.lally 2011-08-26) PLID 44950
		"	'NexTechBypassCriticalBackup'\r\n"		// (j.armen 2014-01-24 10:15) - PLID 59681
		"))\r\n"
		"OR\r\n"
		"((Username = '<None>' OR Username = '%s') AND Name IN (\r\n"	//  {HOSTNAME}.{PATH}\Practice.mde
		"	'LastLocationID',\r\n"
		"	'LastUser',\r\n"
		"	'LastLocationIDPerWinUser',\r\n"		// (d.thompson 2012-05-29) - PLID 48194
		"	'LastUserPerWinUser'\r\n"				// (d.thompson 2012-05-29) - PLID 48194
		"))\r\n"
		"OR\r\n"
		"((Username = '<None>' OR Username = '%s') AND Name IN (\r\n"	// {HOSTNAME}|{PATH}|win user={USERNAME}
		"	'LastLocationID',\r\n"
		"	'LastUser',\r\n"
		"	'LastLocationIDPerWinUser',\r\n"		// (d.thompson 2012-05-29) - PLID 48194
		"	'LastUserPerWinUser'\r\n"				// (d.thompson 2012-05-29) - PLID 48194
		"))\r\n"
		"OR\r\n"
		"((Username = '%s') AND Name IN (\r\n" //win user={USERNAME}
		"	'%s'\r\n"								// CLicense::GetLicenseAgreementAcceptedPropertyName()
		"))",
		"<None>", 
		_Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)),
		_Q(GetAdvancedUsername()),
		_Q(GetLicenseAgreementUsername()),
		_Q(GetLicenseAgreementAcceptedPropertyName()));

	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Snapshot always OK, moved to NxAdo

	// (a.walling 2010-03-15 11:54) - PLID 37751 - Initialize dynamic encryptions keys
	try {
		NxCryptosaur.m_pCon = GetRemoteData();
		NxCryptosaur.Load();
		NxCryptosaur.EnsureLatestKeyExists();
	} NxCatchAll("Failure initializing encryption");

	//Now that we know the database is good, initialize the license.
	{
		BOOL bRunTroubleshooter = FALSE;
		while(!EnsureLicenseServer(bRunTroubleshooter)) {
			if(bRunTroubleshooter) {
				if(!RunTroubleshooter())
					return FALSE; // This means the troubleshooter could not resolve the problem
				bRunTroubleshooter = FALSE;
			} else {
				return FALSE; // This means there is a problem and the troubleshooter cannot
								// resolve it if it where running
			}
		}
	}

	// Now connect to NxServer (this may reference the license).
	{
		BOOL bRunTroubleshooter = FALSE;
		while (!EnsureNxServer(bRunTroubleshooter)) {
			if (bRunTroubleshooter) {
				if (!RunTroubleshooter())
					return FALSE; // This means the troubleshooter could not resolve the problem
				bRunTroubleshooter = FALSE;
			} else {
				return FALSE; // This means there is a problem and the troubleshooter cannot
								// resolve it if it were running
			}
		}
	}

	// (j.armen 2011-10-26 17:13) - PLID 45796 - Ensure the working path after we have a license 
	// Make sure the current directory is not in use by someone else
	if (!EnsureWorkingPathAvail(TRUE)) {
		return FALSE;
	}

	// (c.haag 2013-01-15) - PLID 59257 - Warn if any other subkey has our shared path.
	WarnOtherSubkeysWithSameSharedPath();

	// (a.wilson 2014-09-30 12:46) - PLID 63567 - warn users if they currently have unresolved medicaton conflicts.
	CheckFDBMedsHistory();

	// (z.manning 2012-08-08 09:42) - PLID 52026 - Preload the accessor assemblies
	StartPreLoginThread();

	return TRUE;
}

BOOL CPracticeApp::PreSplash()
{
	// The VERY first thing needs to be to check for other instances of the program
	// (a.walling 2006-11-10 10:18) - PLID 23294 - Use the MutexName function for mutexes based on working path
	CString strMutexName = GetMutexName();
	m_hmutex = CreateMutex(NULL, TRUE, strMutexName);
	long nErr = (long)GetLastError();
	long nAlreadyExists = (long)ERROR_ALREADY_EXISTS;
	if (nErr == nAlreadyExists) {
		if (ShowOtherInstance()) {
			return FALSE;
		}
	}

	// (b.cardillo 2005-05-06 18:55) - PLID 16471 - Store our pid in shared memory so 
	// that if the user tries to run another instance we know where to send focus (see 
	// comments in ShowOtherInstance()).
	// (a.walling 2006-11-10 10:18) - PLID 23294 - Use the MappedFileName function for mapped files based on working path
	HANDLE hMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4, GetMappedFileName());
	if (hMappedFile != NULL) {
		BYTE *pMem = (BYTE *)MapViewOfFile(hMappedFile, FILE_MAP_WRITE, 0, 0, 4);
		if (pMem != NULL) {
			DWORD dwFindProcessId = GetCurrentProcessId();
			memcpy(pMem, &dwFindProcessId, 4);
			UnmapViewOfFile(pMem);
		}
		// (b.cardillo 2005-05-06 18:55) - Don't close the handle, just like we don't 
		// close the mutex above, because we want it to persist until we close Practice.
	}

	// Get whatever info we can
	LoadErrorInfoStrings();

	// (j.jones 2005-12-13 16:06) - PLID 18566 - If Windows 95, stop everything
	CString strOSVersion;
	if(!GetOsVersion(strOSVersion))
		return FALSE;
	if(strOSVersion == "Win32s/Windows 3.1"
		|| strOSVersion == "Windows 95"
		/*|| strOSVersion == "Windows 98"*/) {

		CString strWarning;
		strWarning.Format("Practice has detected that you are running a %s operating system. Practice is not compatible with this operating system.\n\n"
						  "If you receive this message on a non-%s operating system, right-click on the shortcut you used to open Practice and click on "
						  "the 'Compatibility' tab, and uncheck the box that says 'Run this program in compatibility mode'. Save the shortcut, and try running Practice again.", strOSVersion, strOSVersion);
		AfxMessageBox(strWarning);

		return FALSE;
	}

/*
#if (PRODUCT_VERSION_LONG == 200104200)

#ifndef _DEBUG 
	// Register the datalist just for tech-support convenience
	int nResult = RegSvr32(GetPracPath(true) ^ "NxDataList.ocx");
	if (nResult) {
		MsgBox("Could not register 'NxDataList.ocx'.\n\n" 
			"Error Code: %li", nResult);
	}
#endif // !_DEBUG

#else // PRODUCT_VERSION_LONG
#error Todo: Do we still want to be automatically registering the ocx above?
#endif // PRODUCT_VERSION_LONG
*/

	//Check for, and install the lastest version
#ifndef _DEBUG
	if (CheckUpgrade())
		return FALSE;
#endif

	//create the palette BEFORE the splash screen
	// (a.walling 2009-08-12 16:06) - PLID 35136 - Created in InitInstance
	//HDC hdc = ::GetDC(NULL);
	//m_palette.Attach(::CreateHalftonePalette(hdc));//work around for MFC bug in ASSERT
	//ReleaseDC(NULL, hdc);

	return TRUE;
}

// (a.walling 2010-07-29 11:37) - PLID 39871 - Wrote this to test, keeping it around for posterity.
void CPracticeApp::CacheAllProps()
{
#if defined(CACHE_ALL_PROPS_AT_LOGIN)
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	g_propManager.BulkCache("BigCache", propbitNumber | propbitDateTime | propbitText | propbitFloat,
		"(Username IN ("
			"'%s', "
			"'%s', "
			"'%s' "
		"))",
		"<None>", 
		_Q(GetCurrentUserName()),
		_Q(g_propManager.GetSystemName() + '.' + GetPracPath()) );
#endif
}

// (a.walling 2014-10-10 16:18) - PLID 63796 - ICD Health - If ICD database update issues exist, display warning to administrator upon user login to Practice.

namespace {
	class NxSystemHealthTaskDialog : public NxTaskDialog
	{
	public:
		CString m_strInformation;
		CString m_strInformationDetails;

		NxSystemHealthTaskDialog()
		{}

		enum BackupCommands {
			eContinue = 0x100,
			eContact,
			eContactNotify,
			eContactEmail,
			eContactWebsite,
		};

		// (a.walling 2010-06-28 18:04) - PLID 38442 - Some more advanced handling for sending an error report
		virtual BOOL OnButtonClicked(int nButton)
		{
			if (nButton == NxSystemHealthTaskDialog::eContact) {
				ConfigPushNew()
					.ErrorIcon().CloseOnly()
					.MainInstructionText("Contact Nextech technical support")
					.ContentText("East Coast and Midwest states, call (866) 654-4396\nWest Coast and Central states, call (888) 417-8464")
					.AddCommand(eContactNotify, "Send error report\nRelevant error information will be automatically included and submitted with your notification")
					// (a.walling 2011-08-23 14:15) - PLID 44647 - exception/error emails to allsupport@nextech.com
					.AddCommand(eContactEmail, "Email technical support\nOpens your default email application to send an email to allsupport@nextech.com")
					.AddCommand(eContactWebsite, "More contact options\nOpens the Nextech website for further contact options and information")
					;

				NavigatePage();

				return TRUE;
			}
			else if (nButton == NxSystemHealthTaskDialog::eContactNotify) {
				// (a.walling 2010-06-28 18:04) - PLID 38442 - SendErrorToNexTech
				SendErrorToNexTech(NULL, "Nextech System Health", m_strInformation, m_strInformationDetails);
				return TRUE;
			}
			else if (nButton == eContactEmail) {
				CString str;
				str.Format("%s\r\n\r\n%s", m_strInformation, m_strInformationDetails);
				SendErrorEmail(m_pTaskDialogParent, str, "Nextech System Health", false);
				return TRUE;
			}
			else if (nButton == eContactWebsite) {
				ShellExecute(NULL, NULL, "http://www.nextech.com/Contact/Support.aspx", NULL, NULL, SW_SHOW);
				return TRUE;
			}
			else if (nButton == IDCLOSE) {
				ConfigPop();
				NavigatePage();
				return TRUE;
			}

			return NxTaskDialog::OnButtonClicked(nButton);
		};
	};

	void CheckSystemHealth(CWnd* pWnd)
	{
		try {
			if (!IsCurrentUserAdministrator()) {
				return;
			}

			struct Message
			{
				CString text;
				CString details;
			};

			std::vector<Message> messages;

			{
				auto pHealth = GetAPI()->GetSystemHealth(GetAPISubkey(), GetAPILoginToken());

				if (!pHealth) {
					return;
				}

				Nx::SafeArray<IUnknown*> saMessages = pHealth->Messages;
				if (!saMessages || !saMessages.GetLength()) {
					return;
				}

				for (NexTech_Accessor::_SystemHealthMessagePtr pMessage : saMessages) {
					if (!pMessage) {
						ASSERT(FALSE);
						continue;
					}

					messages.push_back({ (const char*)pMessage->Text, (const char*)pMessage->Info });
					messages.back().text.Trim();
					messages.back().details.Trim();
				}
			}

			if (messages.empty()) {
				return;
			}

			// show UI			

			CString text;
			CString details;

			for (const auto& message : messages) {
				if (text.IsEmpty() && !message.text.IsEmpty()) {
					text = message.text;
					details = message.details;
					details.Append("\r\n\r\n");
				}
				else {
					details.Append(message.text);
					details.Append("\r\n");
					details.Append(message.details);
					details.Append("\r\n\r\n");
				}
			}

			text.Trim();
			details.Trim();

			Log("CheckSystemHealth: %s\r\n%s", text, details);

			NxSystemHealthTaskDialog dlg;
			dlg.m_strInformation = text;
			dlg.m_strInformationDetails = details;

			dlg.Config()
				.WarningIcon()
				.OKOnly()
				.MainInstructionText(text)
				.ContentText("Please contact Nextech Technical Support for assistance.")
				.ExpandedInformationText(details)
				.AddCommand(NxSystemHealthTaskDialog::eContact, "Contact Nextech Support")
			;

			auto nRet = dlg.DoModal();
		} NxCatchAll(__FUNCTION__);
	}
}

// (a.walling 2007-05-04 10:03) - PLID 4850 - Include bInitialLogin var, and a CLoginDlg pointer, for switching users.
// (z.manning 2015-12-03 10:43) - PLID 67637 - Added password param
BOOL CPracticeApp::PostLogin(const CString &strPassword, BOOL bInitialLogin /*= TRUE*/,  CWnd* pLoginDlg /*= NULL*/)
{
	// (j.armen 2011-11-02 17:41) - PLID 45796 - Check the license silently to to make sure we still have a good connection to the license server
	if(!g_pLicense->CheckForLicense(CLicense::lcPractice, CLicense::cflrJustCheckingLicenseNoUI)) {
		return FALSE;
	}
	CWaitCursor wait;
	CMainFrame* pMainFrame = NULL;
	BSTR bstrProgress;

	// (b.savon 2015-12-16 09:29) - PLID 67718 - Split out logic
	if (InitializeAPI(strPassword, pLoginDlg) == FALSE) {
		return FALSE;
	}

	if (bInitialLogin) { // (a.walling 2007-05-04 10:05) - PLID 4850 - The initial login

		// (a.walling 2010-07-29 11:37) - PLID 39871 - Wrote this to test, keeping it around for posterity.
		CacheAllProps();

		pLoginDlg = m_dlgLogin; // our login dlg should be the m_dlgLogin if this is initial

		// Make the login not try to be in front of everything
		if (pLoginDlg->GetSafeHwnd()) {
	//		m_dlgLogin->ShowWindow(SW_HIDE);
	//		m_dlgLogin->PostMessage(WM_DESTROY);
		}

		// Helps for error reporting (in case the security has problems)
		LoadErrorInfoStrings();

		// Open the log file
		// (a.walling 2007-11-07 10:18) - PLID 27476 - VS2008 - Need a portable solution for allocating these bstrs
		// bstrProgress = SysAllocString((LPWORD)"Opening log file...");
		bstrProgress = CString("Opening log file...").AllocSysString();
		pLoginDlg->SendMessage(NXM_LOGIN_PROGRESS, 4, (LPARAM)bstrProgress);
		OpenLogFile();

		// Start the hot keys in a disabled state
		InitHotKeys();

		//TES 11/7/2007 - PLID 27979 - VS2008 - This call is no longer needed in VS 2008
#if _MSC_VER <= 1300
		Enable3dControls();			// Call this when using MFC in a shared DLL
#endif

		// (a.walling 2007-11-07 10:18) - PLID 27476 - VS2008 - Need a portable solution for allocating these bstrs
		// bstrProgress = SysAllocString((LPWORD)"Loading profile settings...");
		bstrProgress = CString("Loading profile settings...").AllocSysString();
		pLoginDlg->SendMessage(NXM_LOGIN_PROGRESS, 14, (LPARAM)bstrProgress);

		LoadStdProfileSettings(6);  // Load standard INI file options (including MRU)

		// Load default printer settings
		DefaultPrintSetup();

		// (b.cardillo 2006-10-24 11:09) - PLID 19344 - Instantiate an unused Ink 
		// Collector object.  We'll release this in our ExitInstance.  By creating 
		// this object and holding it open for as long as Practice.exe is running, we 
		// ensure the ink threads and other ink resources are not freed.  We want 
		// this because of a bug in the Microsoft ink code that causes the freeing to 
		// be incomplete.  So by preventing the attempt altogether we effectively work 
		// around the whole problem.
		// TODO: Remove this, once Microsoft resolves our MSDN incident case #srx061019604066.
		try {
			// This is a non-critical effort, so we just log if it fails and carry on.
			m_pAlwaysOpenCollector = MSINKAUTLib::IInkCollectorPtr(__uuidof(MSINKAUTLib::InkCollector));
		} catch (CException *e) {
			CString strErr;
			if (e->GetErrorMessage(strErr.GetBuffer(4096), 4095, NULL)) {
				strErr.ReleaseBuffer();
			} else {
				strErr.ReleaseBuffer(0);
				strErr = "Unknown CException";
			}
			e->Delete();
			LogFlat("CPracticeApp::PostLogin(): Failed to instantiate m_pAlwaysOpenCollector due to CException: %s", strErr);
		} catch (_com_error e) {
			LogFlat("CPracticeApp::PostLogin(): Failed to instantiate m_pAlwaysOpenCollector due to COM error %li: %s (%s)", e.Error(), (LPCTSTR)e.Description(), e.ErrorMessage());
		} catch (...) {
			LogFlat("CPracticeApp::PostLogin(): Failed to instantiate m_pAlwaysOpenCollector due to a low level exception");
		}

		// (a.walling 2007-11-07 10:18) - PLID 27476 - VS2008 - Need a portable solution for allocating these bstrs
		bstrProgress = CString("Initializing modules...").AllocSysString();
		pLoginDlg->SendMessage(NXM_LOGIN_PROGRESS, 29, (LPARAM)bstrProgress);

		CMultiDocTemplate *pDocTemplate;
		BEGIN_INIT_MODULES() {
			pDocTemplate = InitModule(CPatientView,		IDR_NXPRACTYPE,		IDR_NXPRACTYPE_CNTR_IP,		PATIENT_MODULE_NAME);
			pDocTemplate = InitModule(CInvView,			IDR_NXINVTYPE,		IDR_NXINVTYPE_CNTR_IP,		INVENTORY_MODULE_NAME);
			pDocTemplate = InitModule(CMarketView,		IDR_NXMARKETTYPE,	IDR_NXMARKETTYPE_CNTR_IP,	MARKET_MODULE_NAME);
			pDocTemplate = InitModule(CFinView,			IDR_NXFINTYPE,		IDR_NXFINTYPE_CNTR_IP,		FINANCIAL_MODULE_NAME);
			pDocTemplate = InitModule(CReportView,		IDR_NXREPORTTYPE,	IDR_NXREPORTTYPE_CNTR_IP,	REPORT_MODULE_NAME);
	//		pDocTemplate = InitModule(CReportDocView,	IDR_NXREPORTDOCTYPE,IDR_NXREPORTDOCTYPE_CNTR_IP,REPORTDOC_MODULE_NAME);
			pDocTemplate = InitModule(CLetterView,		IDR_NXLETTERTYPE,	IDR_NXLETTERTYPE_CNTR_IP,	LETTER_MODULE_NAME);
			pDocTemplate = InitModule(CAdminView,		IDR_NXADMINTYPE,	IDR_NXADMINTYPE_CNTR_IP,	ADMIN_MODULE_NAME);
			pDocTemplate = InitModule(CContactView,		IDR_NXCONTACTTYPE,	IDR_NXCONTACTTYPE_CNTR_IP,	CONTACT_MODULE_NAME);
			pDocTemplate = InitModule(CSurgeryCenterView, IDR_NXSURGERYCENTERTYPE, IDR_NXSURGERYCENTERTYPE_CNTR_IP, SURGERY_CENTER_MODULE_NAME);
			// (d.thompson 2009-11-16) - PLID 36134
			pDocTemplate = InitModule(CLinksView,		IDR_NXLINKSTYPE,	IDR_NXLINKSTYPE_CNTR_IP,	LINKS_MODULE_NAME);

			// Leave this one last, so it loads as the default -BVB

			//pDocTemplate = InitModule(CSchedulerView,	IDR_NXSCHEDSHOWTYPE,	IDR_NXSCHEDTYPE_CNTR_IP,	SCHEDULER_MODULE_NAME);

			pDocTemplate = InitModule(CSchedulerView,	IDR_NXSCHEDTYPE,	IDR_NXSCHEDTYPE_CNTR_IP,	SCHEDULER_MODULE_NAME);

		} END_INIT_MODULES();

		/* TODO: Commenting this out as part of a fix of the problem where non-administrator users were getting an error about registry permissions.
		// Connect the COleTemplateServer to the document template.
		//  The COleTemplateServer creates new documents on behalf
		//  of requesting OLE containers by using information
		//  specified in the document template.
		m_server.ConnectTemplate(clsid, pDocTemplate, FALSE);

		// Register all OLE server factories as running.  This enables the
		//  OLE libraries to create objects from other applications.
		COleTemplateServer::RegisterAll();
			// Note: MDI applications register all server objects without regard
			//  to the /Embedding or /Automation on the command line.
		//*/
		
		// In case any errors will be popping up, we want to make the login dialog not be on top now
		if (m_pMainWnd->GetSafeHwnd()) {
			m_pMainWnd->SetWindowPos(&CWnd::wndNoTopMost, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW|SWP_NOACTIVATE);
		}

		// create main MDI Frame window
		// (a.walling 2007-11-07 10:18) - PLID 27476 - VS2008 - Need a portable solution for allocating these bstrs
		bstrProgress = CString("Opening Practice Module Manager...").AllocSysString();
		pLoginDlg->SendMessage(NXM_LOGIN_PROGRESS, 44, (LPARAM)bstrProgress);
		pMainFrame = new CMainFrame;
		if (!pMainFrame->LoadFrame(IDR_MAINFRAME)) {//slow
			AfxMessageBox("Practice could not start.");
			return FALSE;
		}
		m_pMainWnd = pMainFrame;
	
		// (c.haag 2005-01-26 15:51) - PLID 15415 - Initialize ConfigRT caching
		InitializeRemotePropertyCache(pMainFrame);

		// (a.walling 2012-06-04 16:30) - PLID 49829 - Load the current user's data from the db
		LoadCurrentUserSettingsData();

		pMainFrame->PostCreate();


		// Initialize TWAIN
		// (a.walling 2007-11-07 10:18) - PLID 27476 - VS2008 - Need a portable solution for allocating these bstrs
		bstrProgress = CString("Initializing Scanner/Digital Camera connection support...").AllocSysString();
		pLoginDlg->SendMessage(NXM_LOGIN_PROGRESS, 58, (LPARAM)bstrProgress);
		NXTWAINlib::Initialize();

		// Connect with the network code
		// (a.walling 2007-11-07 10:18) - PLID 27476 - VS2008 - Need a portable solution for allocating these bstrs
		bstrProgress = CString("Connecting to the NexTech Network Server...").AllocSysString();
		pLoginDlg->SendMessage(NXM_LOGIN_PROGRESS, 73, (LPARAM)bstrProgress);
		
		// Show the changes.txt file
		pMainFrame->ShowWhatsNew();

		// (a.walling 2013-07-18 10:24) - PLID 57624 - NxCache init database connection!
		Nx::Cache::InitDatabase();

		/* TODO: Commenting this out as part of a fix of the problem where non-administrator users were getting an error about registry permissions.
		m_server.UpdateRegistry(OAT_INPLACE_SERVER);
		COleObjectFactory::UpdateRegistryAll();
		//*/
	} else {


		// (a.walling 2007-05-04 10:05) - PLID 4850 - Our mainframe already exists, so point to it.
		pMainFrame = GetMainFrame();

		// our remote prop cache already exists, so flush anything cached
		FlushRemotePropertyCache();
		
		// (a.walling 2010-07-29 11:37) - PLID 39871 - Wrote this to test, keeping it around for posterity.
		CacheAllProps();

		// (a.walling 2007-07-24 13:04) - PLID 26787 - Cache login properties
		if (pMainFrame)
			pMainFrame->CacheLoginProperties();

		// (a.walling 2012-06-04 16:30) - PLID 49829 - Load the current user's data from the db
		LoadCurrentUserSettingsData();
			
		// (a.walling 2007-05-29 12:31) - PLID 4850 - Update the toolbars here, so when the modules are
		// created in OpenFile they will already have the correct active patient / contact ids.
		// Work with the toolbars
		try {
			if (pMainFrame) {
				pMainFrame->m_patToolBar.OnUserChanged();	// refresh for user prefs, last saved, etc
				pMainFrame->m_contactToolBar.OnUserChanged();	// refresh for last saved and permissions
				if (pMainFrame->m_pDocToolBar != NULL)
					pMainFrame->m_pDocToolBar->OnUserChanged();
				
				pMainFrame->UpdateToolBarButtons(true);
			}
		} NxCatchAllThrow("Error initializing toolbars!");
	}

	// In case there's a problem with loading, we don't 
	// want our toolbars to default to being visible
	pMainFrame->ShowPatientBar(false);
	pMainFrame->ShowContactBar(false);
	pMainFrame->ShowDoctorBar(false);
	pMainFrame->RecalcLayout();

	// (a.walling 2007-06-19 18:06) - PLID 4850 - Ensure groups and available reports before opening the default module
	
	//Check all the reports are accounted for in groups
	EnsureReportGroups();

	//Initialize Available reports for this user
	LoadAvailableReportsMap();

	// (d.thompson 2014-02-04) - PLID 60638 - Initialize the search data settings.  This should happen only 
	//	once in the entire application.
	// (d.thompson 2014-03-03) - PLID 60638 - Moved this from OnCreate to HandleUserLogin.  We do want this preference
	//	to be reset when Switch User happens.
	// (d.thompson 2014-03-07) - PLID 60638 - Moved this from HandleUserLogin to PostLogin.  Surprisingly (to me), HandleUserLogin
	//	is called AFTER the default view & tab are created.  This means you can't do any logic in that function that could need
	//	applied to any tab that could be a default one.  We'll instead do it over here, which is always reached before any
	//	sheets are created.
	pMainFrame->ResetDiagSearchSettings();

	// Open the default module
	// (a.walling 2007-11-07 10:18) - PLID 27476 - VS2008 - Need a portable solution for allocating these bstrs
	bstrProgress = CString("Opening Default Practice Module...").AllocSysString();
	pLoginDlg->SendMessage(NXM_LOGIN_PROGRESS, 87, (LPARAM)bstrProgress);
	CDocument *p = OpenFile();//very slow

	// (a.walling 2007-11-07 10:18) - PLID 27476 - VS2008 - Need a portable solution for allocating these bstrs
	bstrProgress = CString("Opening Default Practice Module...").AllocSysString();
	pLoginDlg->SendMessage(NXM_LOGIN_PROGRESS, 100, (LPARAM)bstrProgress);

	
	if (bInitialLogin) {
		pLoginDlg->DestroyWindow(); // when Re-logging in, we'll take care of the window ourselves.
		delete pLoginDlg;
	} else {
		// (a.walling 2007-05-04 10:07) - PLID 4850 - This is a modal dialog when switching users,
		// so we'll just hide it. The base class PostNcDestroy will free memory, and OnOK will EndDialog
		pLoginDlg->ShowWindow(SW_HIDE);
	}

	if (m_dlgLogin) 
		m_dlgLogin = NULL;

	if (bInitialLogin) {
		// The main window has been initialized, so show and update it.
		WINDOWPLACEMENT wp;
		if (ReadWindowPlacement(&wp)) {
			wp.showCmd = SW_SHOWMAXIMIZED;
			pMainFrame->SetWindowPlacement(&wp);
		} else {
			pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
		}
		pMainFrame->RestoreDocking();
	//	pMainFrame->UpdateWindow();
		pMainFrame->UpdateToolBarButtons(true);
		pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
		pMainFrame->ShowWindow(SW_SHOW);
	//	pMainFrame->UpdateWindow();

	//* Brad, let's discuss if this is really needed - RAC
	//	pMainFrame->OnQueryNewPalette();//BVB - otherwise we lose the palette until they click on something
		pMainFrame->PostMessage(WM_QUERYNEWPALETTE);
	//*/

		// Helps for error reporting (last attempt at ensuring we have useful data for error reporting)
		LoadErrorInfoStrings();
	}
	
	// (a.walling 2007-06-18 09:22) - PLID 4850 - Initialize hotkeys both when switching users and when initially logging in
	StartHotKeys();

	//DRT 10/25/02 - Show release - Ask the user if they wish to start logging their time
	//TES 12/18/2008 - PLID 32520 - This feature is blocked to Scheduler Standard users
	if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {	
		if(GetRemotePropertyInt("LogTimePrompt", 0, 0, GetCurrentUserName(), true)) {	//if we're asking for time logging

			//prompt for time if they are not already in the middle of one
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(!ReturnsRecordsParam("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", GetCurrentUserID())) {
				if(AfxMessageBox("Would you like to begin logging your time for a new session?", MB_YESNO) == IDYES) {
					//create a logged in record
					// (j.gruber 2008-06-25 14:37) - PLID 26136 - added location
					// (a.walling 2010-10-19 09:45) - PLID 40965 - Parameterize
					// (b.eyers 2016-01-18) - PLID 67542 - checkin time is now gotten from the server
					ExecuteParamSql("INSERT INTO UserTimesT (ID, UserID, Checkin, Checkout, LocationID) values ({INT}, {INT}, GETDATE(), NULL, {INT})", NewNumber("UserTimesT", "ID"), GetCurrentUserID(), GetCurrentLocationID());
				}
			}
		}
	}

	//TODO:   not sure the best way to do this, but this will work for now
	//we need to set our global var whether we're currently logging or not - so the menu options can grey out appropriately
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(ReturnsRecordsParam("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", GetCurrentUserID()))
		GetMainFrame()->m_bLoggingTime = true;
	else
		GetMainFrame()->m_bLoggingTime = false;

	//if (bInitialLogin && m_Twain.ActivatedByCamera())
	// (a.walling 2008-10-28 13:10) - PLID 31334 - This can be OK if we are switching users; we just need to handle it
	// within the mainframe.
	if (m_Twain.ActivatedByCamera()) 
	{ 
		// Go to the history tab but do NOT acquire because we don't know what patient
		// to acquire to!
		GetMainFrame()->PostMessage(NXM_TWAIN_SOURCE_LAUNCH, m_Twain.GetActiveDeviceID(), 1);
		m_Twain.ResetCameraActivate();
	}

	// (a.walling 2008-10-28 13:05) - PLID 31334 - launched from WIA auto-start
	if (m_Twain.ActivatedByWIA()) {
		GetMainFrame()->PostMessage(NxWIA::NXM_ACQUIRE_FROM_WIA, m_Twain.GetWIAInfoFileNumber(), 1);
		m_Twain.ResetWIAActivate();
	}

	// (j.anspach 05-24-2005 11:11 PLID 16561) - Audit the login
	try {
		int AuditID;
		AuditID = BeginNewAuditEvent();
		if (AuditID != -1)
			AuditEvent(-1, GetCurrentUserName(), AuditID, aeiSessionLogin, -1, "", GetCurrentUserName(), aepMedium);
	} NxCatchAll("Error in auditing login.");

	// (j.jones 2008-09-11 12:17) - PLID 31335 - update any insured parties that are now inactive
	try {

		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtOld;
		dtOld.SetDate(1800,1,1);
		COleDateTime dtLastProcess = GetRemotePropertyDateTime("LastInactiveInsuredPartyProcess", &dtOld, 0, "<None>", false);
		if(dtNow.Format("%Y-%m-%d") != dtLastProcess.Format("%Y-%m-%d")) {
			//now update the insured parties, which will update the
			//LastInactiveInsuredPartyProcess setting upon successful completion
			//(There is no harm in the rare case where two users run this at the same time.)
			UpdateInactiveInsuredParties();
		}

	}NxCatchAll("Error updating inactive insured parties (PostLogin).");

	// (j.jones 2007-03-29 17:52) - PLID 24819 - reinstated the finance charge calculations on startup

	try {

		//Finance Charges

		// (j.jones 2013-07-31 15:22) - PLID 53317 - if the prompt type is 2 (manual), do nothing here
		long nPromptType = GetRemotePropertyInt("FCPromptApplyFinanceCharges",2,0,"<None>",true);
		if(nPromptType != 2
			&& GetCurrentUserPermissions(bioFinanceCharges) & (sptWrite|sptWriteWithPass)) {

			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			COleDateTime dtOld;
			dtOld.SetDate(1800,1,1);
			// (j.jones 2013-07-31 15:14) - PLID 53317 - the LastFinanceChargeProcess ConfigRT record is obsolete,
			//this is now in FinanceChargeHistoryT
			//COleDateTime dtLastProcess = GetRemotePropertyDateTime("LastFinanceChargeProcess", &dtOld, 0, "<None>", true);
			COleDateTime dtLastProcess = dtOld;
			_RecordsetPtr rsLastProcess = CreateParamRecordset("SELECT Max(InputDate) AS LastProcessed "
				"FROM FinanceChargeHistoryT");
			if(!rsLastProcess->eof) {
				// (r.farnworth 2014-04-11 14:15) - PLID 60070 - Needs a default value.
				dtLastProcess = VarDateTime(rsLastProcess->Fields->Item["LastProcessed"]->Value, dtOld);
			}
			rsLastProcess->Close();

			if(dtNow.Format("%Y-%m-%d") != dtLastProcess.Format("%Y-%m-%d")) {
				
				//it's been a day, so let's attempt to prompt

				bool bCalculateFinanceCharges = false;

				if(nPromptType == 0) {
					//auto-calc finance charges
					bCalculateFinanceCharges = true;
				}
				else if(nPromptType == 1) {
					long nUserID = GetRemotePropertyInt("FCPromptUserID",-1,0,"<None>",true);
					if(nUserID == -1 || nUserID == GetCurrentUserID()) {
						//prompt first
						if(IDYES == MessageBox(GetActiveWindow(), "Would you like to calculate today's finance charges now?","Practice",MB_ICONQUESTION|MB_YESNO)) {
							bCalculateFinanceCharges = true;
						}
					}
				}

				if(bCalculateFinanceCharges) {
					CFinanceChargesDlg dlg(NULL);
					dlg.DoModal();
				}
			}
		}
	}NxCatchAll("Error calculating finance charges (PostLogin).");

	// (z.manning 2009-11-10 14:16) - PLID 36255 - If they have NexSync then we need to do a quick
	// check to see if this user's profile has been syncing since there's no interface for NexSync.
	if(g_pLicense != NULL && g_pLicense->GetNexSyncCountAllowed() > 0)
	{
		// (z.manning 2009-11-10 14:17) - PLID 36255 - They are licensed for NexSync so see if
		// the current user has a NexSync profile.
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT LastSyncTime, GetDate() AS CurrentTime FROM OutlookFolderT \r\n"
			"WHERE UserID = {INT} AND IsNexSync = 1; \r\n"
			"SELECT IntParam FROM ConfigRT WHERE Name = {STRING}; \r\n"
			, GetCurrentUserID(), "NexSyncInterval");
		if(!prs->eof) {
			// (z.manning 2009-11-10 14:18) - PLID 36255 - This user does have a NexSync profile so
			// let's see if it's been syncing.
			_variant_t varLastSyncTime = prs->GetFields()->GetItem("LastSyncTime")->GetValue();
			if(varLastSyncTime.vt == VT_DATE) {
				COleDateTime dtLastSync = VarDateTime(varLastSyncTime);
				COleDateTime dtCurrent = AdoFldDateTime(prs->GetFields(), "CurrentTime");
				// (z.manning 2009-11-10 14:19) - PLID 36255 - We'll set the threshold for failure to
				// 24 hours.
				int nMinutesUntilWarnOfFailedSync = 24 * 60;
				prs = prs->NextRecordset(NULL);
				if(!prs->eof) {
					// (z.manning 2009-11-10 14:25) - PLID 36255 - If for whatever reason they have
					// their sync interval set to more than our threshold then make sure we adjust
					// the threshold.
					int nSyncInterval = AdoFldLong(prs->GetFields(), "IntParam", 0);
					if(nSyncInterval >= nMinutesUntilWarnOfFailedSync / 2) {
						nMinutesUntilWarnOfFailedSync = nSyncInterval * 4;
					}
				}

				// (z.manning 2009-11-10 14:28) - PLID 36255 - Finally check and see if the time since last
				// sync is more than our threshold and if so, warn the user via a message box.
				COleDateTimeSpan dtsTimeSinceLastSync = dtCurrent - dtLastSync;
				if(dtsTimeSinceLastSync.GetStatus() == COleDateTimeSpan::valid && dtCurrent > dtLastSync
					&& dtsTimeSinceLastSync.GetTotalMinutes() > nMinutesUntilWarnOfFailedSync)
				{
					CString strMsg = FormatString("Your NexSync profile has not performed a successful sync since %s.\r\n\r\n"
						"Please contact NexTech support if you need help with this."
						, FormatDateTimeForInterface(dtLastSync));
					MessageBox(GetMainFrame()->GetSafeHwnd(), strMsg, "NexSync", MB_OK|MB_ICONERROR);
				}
			}
		}
	}

	// (r.galicki 2008-10-13 13:30) - PLID 31373 - Display labs needing attention, with correct license/permissions
	// (r.galicki 2008-10-15 14:36) - PLID 31373 - Moved checks to CMainFrm::ShowLabFollowUp()
	/*try {
		if(g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) { //license?
			if(GetCurrentUserPermissions(bioPatientLabs)) {	//permission?
				if(GetRemotePropertyInt("LabDisplayNeedAttn", 1, 0, GetCurrentUserName())) { //preference?
					GetMainFrame()->ShowLabFollowUp();

	}NxCatchAll("Error in displaying 'Labs Needing Attention' dialog (PostLogin).");*/
	GetMainFrame()->ShowLabFollowUp();

	//TES 4/2/2004: PLID 11751 - Now, do they want to pop up the productivity dialog?
	// (j.jones 2010-04-09 08:40) - PLID 14360 - moved the marketing productivity popup from CMainFrame::HandleUserLogin() to here
	if(g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrSilent)
		&& GetRemotePropertyInt("PopupProductivity", 0, 0, GetCurrentUserName(), true)) {
		//Have we popped it up already today?
		COleDateTime dtLastPopup = GetRemotePropertyDateTime("LastProductivityPopup", &COleDateTime(1899,12,30,0,0,0),0, GetCurrentUserName(), true);
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		//We're assuming dtLastPopup isn't in the future.
		if(dtLastPopup.GetYear() != dtNow.GetYear() || dtLastPopup.GetMonth() != dtNow.GetMonth() || dtLastPopup.GetDay() != dtNow.GetDay()) {
			//Let's pop it up now.
			SetRemotePropertyDateTime("LastProductivityPopup", dtNow, 0, GetCurrentUserName());
			GetMainFrame()->PostMessage(WM_COMMAND, MAKEWPARAM(ID_ACTIVITIES_SCHEDULING_PRODUCTIVITY,0), NULL);
		}
	}

	// (j.jones 2010-06-01 09:51) - PLID 37976 - check whether we should
	// be monitoring ophthalmology device folders from available plugins
	// (j.jones 2010-10-22 14:05) - PLID 41069 - moved to PostLogin()
	// (a.walling 2013-04-11 17:05) - PLID 56225 - Move DeviceImport stuff into its own class
	// (r.farnworth 2014-10-02 11:41) - PLID 63378 - Added a default parameter to determine when we were coming from Logging in
	DeviceImport::GetMonitor().TryMonitorDevicePluginFolders(true);

	CheckSystemHealth(GetMainFrame());

	// (z.manning 2016-02-10 16:53) - PLID 68223 - Initialize the word processor manager
	m_pWordProcessorManager.reset(new CGenericWordProcessorManager(GetWordProcessorType()));

	return TRUE;
}

class CPracticeCmdLineInfo : public CCommandLineInfo 
{
public:
	CPracticeCmdLineInfo();

public:
	CString m_strUsername;
	CString m_strPassword;
	CString m_strRegistry;
	long m_nLocationID;
	// (a.walling 2008-10-28 09:39) - PLID 31334 - Launched with WIA device parameter
	CString m_strWIADevice;

	// (a.walling 2008-10-28 14:16) - PLID 31334 - Allow setting working path
	CString m_strWorkingPath;
	// (b.savon 2016-05-11 11:03) - NX-100610
	bool m_bSecurityGroup;
	CString m_strDomain;

public:
	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
};

CPracticeCmdLineInfo::CPracticeCmdLineInfo()	
	: m_bSecurityGroup(false), m_nLocationID(-1) // (a.walling 2011-12-14 10:41) - PLID 40593
{
}

void CPracticeCmdLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	CString strParam(pszParam);
	
	if (!bFlag) {
	if (strnicmp(strParam, "u:", 2) == 0) {
		// This is a username parameter
		m_strUsername = strParam.Mid(2);
	} else if (strnicmp(strParam, "user:", 5) == 0) {
		// This is a username parameter
		m_strUsername = strParam.Mid(5);
	} else if (strnicmp(strParam, "username:", 9) == 0) {
		// This is a username parameter
		m_strUsername = strParam.Mid(9);
	
	
	} else if (strnicmp(strParam, "p:", 2) == 0) {
		// This is a password parameter
		m_strPassword = strParam.Mid(2);
	} else if (strnicmp(strParam, "pass:", 5) == 0) {
		// This is a password parameter
		m_strPassword = strParam.Mid(5);
	} else if (strnicmp(strParam, "password:", 9) == 0) {
		// This is a password parameter
		m_strPassword = strParam.Mid(9);


	}
	else if (strnicmp(strParam, "r:", 2) == 0) {
		//This is a registry parameter
		m_strRegistry = strParam.Mid(2);

	} 
	// (b.savon 2016-05-11 11:03) - NX-100610
	else if (strnicmp(strParam, "d:", 2) == 0) {
		m_strDomain = strParam.Mid(2);
	}
	else if (strnicmp(strParam, "domain:", 7) == 0) {
		m_strDomain = strParam.Mid(7);
	}
	else if (strnicmp(strParam, "l:", 2) == 0) {
		// This is a location parameter
		m_nLocationID = atol(strParam.Mid(2));
	} else if (strnicmp(strParam, "loc:", 4) == 0) {
		// This is a location parameter
		m_nLocationID = atol(strParam.Mid(4));
	} else if (strnicmp(strParam, "location:", 9) == 0) {
		// This is a location parameter
		m_nLocationID = atol(strParam.Mid(9));


	} else if (strnicmp(strParam, "wia:", 4) == 0) {
		// (a.walling 2008-10-28 09:39) - PLID 31334 - Launched with WIA device parameter
		m_strWIADevice = strParam.Mid(4);
	
		// (a.walling 2008-10-28 15:11) - PLID 31334 - Ability to set the working path
	} else if (strnicmp(strParam, "dir:", 4) == 0) {
		m_strWorkingPath = strParam.Mid(4);
		m_strWorkingPath.TrimLeft("\"");
		m_strWorkingPath.TrimRight("\"");
		} else {
			// (a.walling 2011-12-14 10:41) - PLID 40593 - Added assertions for unrecognized command line params and flags for posterity
			TRACE("Unrecognized command line parameter %s\n", pszParam);
			ASSERT(FALSE);
	}
	}
	else {
		// (b.savon 2016-05-11 11:03) - NX-100610
		if (strParam.CompareNoCase("g") == 0 || strParam.CompareNoCase("group") == 0){
			//This is the security group
			m_bSecurityGroup = true;
		}
		else {
			// (a.walling 2011-12-14 10:41) - PLID 40593 - Added assertions for unrecognized command line params and flags for posterity
			TRACE("Unrecognized command line flag %s\n", pszParam);
			ASSERT(FALSE);
		}
	}
}

// (a.walling 2009-08-12 16:07) - PLID 35136 - Create a palette for Practice. Basically, we create a halftone palette like before,
// but we also use 
void CreatePracticePalette(CPalette& pal)
{	
	HDC hdc = ::GetDC(NULL);
	pal.Attach(::CreateHalftonePalette(hdc));//work around for MFC bug in ASSERT
	ReleaseDC(NULL, hdc);

	long nEntries = 18;

	PALETTEENTRY* colors = new PALETTEENTRY[nEntries];


	long nActualEntries = 0;
#define ADD_PALETTE_ENTRY(flags, rgb) 	colors[nActualEntries].peRed = GetRValue(rgb); \
										colors[nActualEntries].peGreen = GetGValue(rgb); \
										colors[nActualEntries].peBlue = GetBValue(rgb); \
										colors[nActualEntries].peFlags = flags; \
										nActualEntries++;

	// (a.walling 2010-06-16 18:11) - PLID 39087
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, CNexTechDialog::GetSolidBackgroundRGBColor());

	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_PATIENT_STATUS, 1));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_PATIENT_STATUS, 2));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_PATIENT_STATUS, 3));

	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, RGB(0xBA, 0xCE, 0xE7)); // toolbar	

	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_ADMIN, 0));

	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_CONTACT, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_FINANCIAL, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_INVENTORY, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_LETTER, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_MARKET, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_REPORT, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_CPT_CODE, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_PRODUCT, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_PERSONNEL, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_EMR_ITEM_BG, 0));

	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_FILTER_USE_OR, 0));
	ADD_PALETTE_ENTRY(PC_NOCOLLAPSE, GetNxColor(GNC_FILTER_USE_OR, 1));

	ASSERT(nEntries == nActualEntries);

	pal.SetPaletteEntries(0, nActualEntries, colors);

	delete[] colors;
}

bool GetModuleFilePathName(IN const HMODULE hModule, OUT CString &strModuleFilePathName);

// (a.walling 2011-12-14 10:41) - PLID 40593 - Now this is global so we can access it anywhere
static CPracticeCmdLineInfo g_cmdInfo;


/// Misc
namespace Nx
{
	// loads a function from a dll that is already loaded into the process
	// and automatically casts to the given fn type template parameter
	template<typename FnType>
	FnType GetLoadedDllProcAddress(const char* module, const char* fn)
	{
		return reinterpret_cast<FnType>(
			::GetProcAddress(
				::GetModuleHandle(module)
				, fn
			)
		);
	}
}

#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDATA 0x0049
#endif

// (a.walling 2013-08-21 13:26) - PLID 58039 - Allow file drag+drop from lower integrity level processes
static void InitWindowMessageFilter()
{
	typedef BOOL (WINAPI* ChangeWindowMessageFilterFnType)(UINT,DWORD);
	static ChangeWindowMessageFilterFnType ChangeWindowMessageFilterFn = Nx::GetLoadedDllProcAddress<ChangeWindowMessageFilterFnType>("user32.dll", "ChangeWindowMessageFilter");
	if (!ChangeWindowMessageFilterFn) {
		return;
	}

	ChangeWindowMessageFilterFn(WM_DROPFILES, 1);
	ChangeWindowMessageFilterFn(WM_COPYDATA, 1);
	ChangeWindowMessageFilterFn(WM_COPYGLOBALDATA, 1);
}

#ifndef PROCESS_DEP_ENABLE
#define PROCESS_DEP_ENABLE                          0x00000001
#endif
#ifndef PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION
#define PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION     0x00000002
#endif

// (a.walling 2014-05-13 14:12) - PLID 62098 - VS2013 - Ensures ATL thunk emulation and DEP is enabled
// returns true unless DEP is enabled AND ATL thunk emulation is disabled
bool EnsureDEPWithOldATLThunkEmulation()
{
	typedef BOOL(WINAPI * fnSetProcessDEPPolicy)(
		DWORD dwFlags
	);

	typedef BOOL(WINAPI * fnGetProcessDEPPolicy)(
		HANDLE hProcess,
		LPDWORD lpFlags,
		PBOOL lpPermanent
	);
	
	auto hKernel = ::GetModuleHandle("kernel32.dll");
	static auto getProcessDEPPolicy = (fnGetProcessDEPPolicy)::GetProcAddress(hKernel, "GetProcessDEPPolicy");
	static auto setProcessDEPPolicy = (fnSetProcessDEPPolicy)::GetProcAddress(hKernel, "SetProcessDEPPolicy");

	if (!getProcessDEPPolicy || !setProcessDEPPolicy) {
		return true;
	}

	DWORD lastError = 0;
	DWORD originalFlags = 0;
	DWORD flags = 0;
	BOOL permanent = FALSE;

	getProcessDEPPolicy(::GetCurrentProcess(), &originalFlags, &permanent);
	
	// do not disable ATL thunk emulation!
	if (!setProcessDEPPolicy(PROCESS_DEP_ENABLE)) {
		lastError = ::GetLastError();
	}

	if (!getProcessDEPPolicy(::GetCurrentProcess(), &flags, &permanent)) {
		lastError = ::GetLastError();
		return true; // let it slide
	}

	if (flags & PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION) {
		return false;
	}

	return true;
}

// (b.savon 2016-05-11 11:03) - NX-100610
/// <summary>
/// Returns true if the command line domain parameter is the same as the supplied strDomainParameter 
/// AND if the supplied strGroupName parameters contains only digits.
/// </summary>
bool IsValidSecurityGroup(const LPCSTR strDomainName, const LPCSTR strGroupName)
{
	return g_cmdInfo.m_strDomain.CompareNoCase(strDomainName) == 0
		&& ((CString)strGroupName).SpanIncluding("0123456789").CompareNoCase(strGroupName) == 0;
}

// (b.savon 2016-05-11 11:03) - NX-100610
/// <summary>
/// On success this method sets the registry base determined by the security group of the given user executing the Practice process.
/// On failure this method throws and exception because we can't continue opening Practice with the given command line parameters.
/// </summary>
void UseSecurityGroupSettingsForRegistryBase()
{
	bool bSuccess = false;

	// Attempt to open the Practice process token to retrieve user information
	HANDLE hTokenHandle;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenHandle)) {

		// We first have to query the token information for how big it is going to be.  If you
		// don't do this you get Error Code 122 (ERROR_INSUFFICIENT_BUFFER).  This first call
		// populates dwcbSz with the proper size to initialize the PTOKEN_GROUPS object for
		// a subsequent call to retrieve the security groups of the user that is running the
		// Practice process.
		LPBYTE lpBytes = NULL;
		PTOKEN_GROUPS pGroups = NULL;
		DWORD dwcbSz = 0;
		BOOL bQueryTokenResult = GetTokenInformation(hTokenHandle, TokenGroups, NULL, 0, &dwcbSz);

		// This is the expected error case.  We now know what the buffer size we'll be (dwcbSz)
		// So initialize our PTOKEN_GROUPS with that size.
		if (!bQueryTokenResult && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			lpBytes = new BYTE[dwcbSz];
			pGroups = (PTOKEN_GROUPS)lpBytes;
		}
		else if (!bQueryTokenResult) {
			// We got an unexpected error when querying GetTokenInformation, close the handles and throw the exception
			// to the user.
			CloseHandle(hTokenHandle);
			ThrowNxException(FormatString("UseSecurityGroupSettings: Unable to GetTokenInformation. Error 0x%08x", GetLastError()));
		}
		else {
			//Continue on we had success with our first call to GetTokenInformation...
		}

		// Now, query the token information with a properly sized object if we didn't succeed the first time
		if (bQueryTokenResult || GetTokenInformation(hTokenHandle, TokenGroups, pGroups, dwcbSz, &dwcbSz)) {

			// Our token query was successful so iterate through the groups to find 
			// this user's security group.
			for (DWORD dwGroup = 0; dwGroup < pGroups->GroupCount; dwGroup++) {

				//Initialize our variables to get the Domain and Group name from the group sid
				// +1 for null terminator
				TCHAR tcGroupName[UNLEN + 1];
				TCHAR tcDomainName[DNLEN + 1];
				DWORD dwGroupName = UNLEN + 1, dwDomainName = DNLEN + 1;
				SID_NAME_USE eUse;

				//Query the Domain and Group name from the sid
				if (LookupAccountSid(NULL, pGroups->Groups[dwGroup].Sid, tcGroupName, (LPDWORD)&dwGroupName, tcDomainName, (LPDWORD)&dwDomainName, &eUse)) {

					if (tcGroupName == NULL) {
						continue;
					}

					if (tcDomainName == NULL) {
						continue;
					}

					// If the queried domain matches that of the command line parameter we're expecting and it's in
					// the proper format [valid integer], then set the registry base with the group name
					if (IsValidSecurityGroup(tcDomainName, tcGroupName)) {
						// Set the registry base to be used for all database/api connections
						::SetRegistryBase(tcGroupName);
						bSuccess = true;
						break;
					}
					else {
						// Ignore and continue.
					}
				}
				else {
					//Log("UseSecurityGroupSettings: Failed LookupAccountSid -- Error code: 0x%08x", GetLastError());
				}
			}//end iterate through groups

		}
		else {
			//This was our failed attempt to GetTokenInformation with a properly sized object. Report error to user.  We can't continue
			//given the current command line parameters
			CloseHandle(hTokenHandle);
			if (lpBytes != NULL) {
				delete[] lpBytes;
			}
			ThrowNxException(FormatString("UseSecurityGroupSettings: Unable to GetTokenInformation. Error 0x%08x", GetLastError()));
		}

		//Cleanup
		CloseHandle(hTokenHandle);
		if (lpBytes != NULL) {
			delete[] lpBytes;
		}
	}
	else {
		ThrowNxException(FormatString("UseSecurityGroupSettings: Unable to OpenProcessToken. Error 0x%08x", GetLastError()));
	}

	if (!bSuccess) {
		ThrowNxException("UseSecurityGroupSettings: Unable to find valid security group settings");
	}
}

BOOL CPracticeApp::InitInstance()
{
	// In case of exceptions, FIRST make NxException handle all low-level exceptions that might arise during this thread
#ifndef _DEBUG
	//TODO: PLID 12027
	CNxException::TakeOverLowLevelExceptionHandling();
#else
	// (a.walling 2009-12-24 13:13) - PLID 36707 - Enable breakpoints on failed assertions
	// (a.walling 2010-01-15 17:39) - PLID 36707 - Only if a debugger is present
	if (IsDebuggerPresent()) {
		CNxException::SetBreakOnAssert(true);
	}
#endif

	// (a.walling 2008-10-13 16:31) - PLID 31676 - Set our invalid_parameter_handler, debug or not.
	CNxException::TakeOverInvalidParameterHandling();

	// (a.walling 2012-07-05 14:53) - PLID 51422
	//Nx::Dragons::TakeOverAfxCallWndProc();

	// (a.walling 2012-09-05 14:20) - PLID 52494 - Override default CWnd::CreateAccessibleProxy to not use the unsafe CMFCComObject which does not properly maintain MFC's module / thread state
	// (a.walling 2014-04-24 17:20) - VS2013 - THANKS FOR FINALLY FIXING THE BUG I REPORTED TO YOU 5 YEARS AGO MICROSOFT
	//Nx::Dragons::TakeOverCreateAccessibleProxy();

	// (a.walling 2014-09-11 10:04) - PLID 63620 - Hook default CWnd::OnTabletQuerySystemGestureStatus
	Nx::Dragons::TakeOverTabletQuerySystemGestureStatus();

	// (a.walling 2011-03-19 16:18) - PLID 42914 - Register exception handling callbacks with NxCatch
	// yes yes I *know* the & operator is unnecessary after VS6
	NxCatch::OnException = &NxCatch::Practice::HandleMFCException;
	NxCatch::OnCOMException = &NxCatch::Practice::HandleCOMException;


	// (a.walling 2009-07-10 17:20) - PLID 33644
	srand((UINT)time(NULL));
	
	// (a.walling 2011-05-25 18:25) - PLID 43848 - Enable the low-fragmentation heap for the process's default heap and the CRT default heap
	// This has no effect on Vista+
	EnableLowFragmentationHeap();

	// (a.walling 2007-05-10 13:18) - PLID 25971 - Set the main practice thread name
	// A bug in EMR was causing Practice to lock up on me; breaking and using the threads dialog was unintuitive, so
	// I named the main thread 'Practice'. Decided I may as well leave it in here. This does nothing in release mode.
	SetThreadName(-1, "Practice");

	// (a.walling 2014-05-13 14:12) - PLID 62098 - VS2013 - Ensure ATL thunking emulation is enabled for DEP stuff
	EnsureDEPWithOldATLThunkEmulation();

	__super::InitInstance(); // (a.walling 2014-09-18 16:34) - PLID 63485 - Must not skip base CWinApp initialization!

	InitWindowMessageFilter();

	// (a.walling 2010-06-18 10:46) - PLID 39195 - Need to use our override COccManager.
	// So we will call AfxEnableNxControlContainer instead of AfxEnableControlContainer.
	AfxEnableNxControlContainer();//Active X support

	AfxOleInit();

	// (a.walling 2008-04-01 12:19) - PLID 29497 - Startup GDI+
	// (a.walling 2008-04-10 08:35) - PLID 29497 - Use the helper class
	g_GDIPlusHolder.Initialize();

	// (a.walling 2009-08-12 16:07) - PLID 35136 - Create the initial palette
	CreatePracticePalette(m_palette);

	SetRegistryKey(_T("NexTech"));

	//DRT 7/2/02 - I moved this ahead of the PreSplash(), because we need to check the command line for a registry setting before we 
	//				connect to the database
	ParseCommandLine(g_cmdInfo);

	CString strPracticeInstanceUUID = NewUUID();

	// (a.walling 2012-03-29 09:38) - PLID 49300 - Ensure all instances of the Practice process use their own taskbar group
	// Multiple, disparate Practice instances for different databases or users etc end up grouped under a single icon in windows 7 which gets confusing
	{
		// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - SetAppUserModelID is a member of CNxAdvancedUIApp now
		CString strAppUserModelID;
		strAppUserModelID.Format("NexTech.Practice.%s.%s", 
			g_cmdInfo.m_strRegistry.IsEmpty() ? "Default" : g_cmdInfo.m_strRegistry,
			strPracticeInstanceUUID
			);

		CStringW wstrAppUserModelID = strAppUserModelID;

		if (!SUCCEEDED(SetAppUserModelID(wstrAppUserModelID))) {
			ASSERT(FALSE);
		}
	}

	// (a.walling 2008-10-28 15:11) - PLID 31334 - Ability to set the working path
	if(!g_cmdInfo.m_strWorkingPath.IsEmpty()) {
		SetCurrentDirectory(g_cmdInfo.m_strWorkingPath);
	}

	if(!g_cmdInfo.m_strRegistry.IsEmpty()) {
		//copy the registry in the command line to a global variable
		// (a.walling 2011-09-16 13:01) - PLID 45531 - Set/GetRegistryBase is actually a function of CWinAppEx now
		::SetRegistryBase(g_cmdInfo.m_strRegistry);
	}

	// (b.savon 2016-05-11 11:03) - NX-100610
	try{
		if (g_cmdInfo.m_bSecurityGroup) {
			UseSecurityGroupSettingsForRegistryBase();
		}
	} NxCatchAllCall("Error setting security group settings!", {
		return FALSE;
	});

	try {

		//TES 3/2/2015 - PLID 64738 - If the DebugMode flag is set, attach NxDebug
		if (NxRegUtils::ReadLong(CString(::GetRegistryBase()) ^ "DebugMode", 0) != 0) {
			AttachNxDebug();
		}

		// (j.armen 2011-10-26 17:14) - PLID 45796 - Start off by setting the Practice Shared Path
		SetPracPath(PracPath::PracticePath, GetCurrentDirectory());	
		SetPracPath(PracPath::ConfigRT, GetPracPath(PracPath::PracticePath) ^ "Practice.mde");

		// (j.armen 2011-10-24 15:06) - PLID 45796 - Get the location of Practice.exe
		CString strExePath;
		GetModuleFileName(NULL, strExePath.GetBuffer(MAX_PATH), MAX_PATH);
		strExePath.ReleaseBuffer();
		SetPracPath(PracPath::PracticeEXEPath, strExePath);

		CString strBaseWorkingPath = GetPracPath(PracPath::PracticePath) ^ "Sessions" ^ (GetSubRegistryKey().IsEmpty()?"Default":GetSubRegistryKey());
		if (!FileUtils::DoesFileOrDirExist(strBaseWorkingPath)) {
			FileUtils::CreatePath(strBaseWorkingPath);
			if (!FileUtils::DoesFileOrDirExist(strBaseWorkingPath)) {
				AfxThrowNxException("Unable to create base Working Directory: %s", strBaseWorkingPath);
			}
		}

		CString strSessionName;
		CString strWorkingPath;
		{
			// (a.walling 2013-01-23 17:47) - PLID 54805 - The Session path using a GUID is not very friendly to us sacks of meat;
			// we can use something with more info while still ensuring uniqueness.
			// previously strPracticeInstanceUUID
			// now using the format 
			//
			// YYYYMMDDThhmmss_username_PID_XX
			//
			// where PID is the process ID and XX is the hex uniqueifier in the (extremely unlikely) case of collision

			SYSTEMTIME st = {0};
			::GetLocalTime(&st);

			CString strUserName = GetWindowsUsername();
			FileUtils::ReplaceInvalidFileChars(strUserName, '-');
			strUserName.Replace('_', '-');

			DWORD dwUnique = 0;

			for(;;) {
				strSessionName.Format(
					"%04u%02u%02u"
					"T%02u%02u%02u"
					"_%s"
					"_%lu"
					"_%02x"
					, st.wYear, st.wMonth, st.wDay
					, st.wHour, st.wMinute, st.wSecond
					, strUserName
					, GetCurrentProcessId()
					, dwUnique
				);

				++dwUnique;

				strWorkingPath = strBaseWorkingPath ^ strSessionName;

				if (!::CreateDirectory(strWorkingPath, NULL)) {
					DWORD dwErr = ::GetLastError();

					if (dwUnique > 0xFF) {
						AfxThrowNxException("Failed to create Working Directory: %s", strWorkingPath);
					}

					if (dwErr == ERROR_ALREADY_EXISTS) {
						// ok, dwUnique will take care of it, though this is extremely unlikely
					} else if (dwErr == ERROR_PATH_NOT_FOUND) {
						// ensure intermediate path is OK
						if(!FileUtils::CreatePath(strBaseWorkingPath)) {
							AfxThrowNxException("Unable to create base Working Directory: %s", strBaseWorkingPath);
						}
					} else {
						AfxThrowNxException("Failed to create Working Directory: %s", strWorkingPath);
					}
				} else {
					break;
				}
			}
		}

		if(!SetCurrentDirectory(strWorkingPath))
			AfxThrowNxException("Unable to set Current Directory: %s", strWorkingPath);

		// Now that we have the session working path, set it.
		SetPracPath(PracPath::SessionPath, strWorkingPath);
	} NxCatchAllCall("Error Setting Practice paths", { 
		return FALSE; 
	});

	// (a.walling 2012-04-25 16:19) - PLID 49987 - Initialize instance data
	m_pPracticeInstanceData.reset(new CPracticeInstanceData);

	// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - Init CNxAdvancedUIApp : CWinAppEx and global MFC Feature Pack options

	// (a.walling 2012-07-12 11:36) - PLID 51395 - Initialize with schema version
	InitializeAdvancedUI(42);
	
	GetContextMenuManager()->AddMenu("EMR topic", IDR_POPUP_EMR_TOPIC);

	//DRT 7/2/02 - I moved this ahead of the PreSplash(), because we need to check the command line for a registry setting before we 
	//				connect to the database
	
	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - XactAbort always OK, moved to NxAdo

	// (a.walling 2008-10-28 09:39) - PLID 31334 - Launched with WIA device parameter
	if(!g_cmdInfo.m_strWIADevice.IsEmpty()) {
		if (!m_Twain.HandleWIAEvent(g_cmdInfo.m_strWIADevice)) {
			return FALSE;
		}
	}

	// (c.haag 2004-05-17 15:29) - Before we do anything, see if we were invoked
	// by a TWAIN source. If we are the instance invoked by the TWAIN device, and
	// Practice is running already, our work is done, and we will quit right now.
	//
	// If an error occurs, Register() will return false and we will just go about
	// our business.
	//
	// We don't call Unregister() at the end because clients should be able to
	// invoke Practice even if it's not running...in which case Register() would
	// return false.
	//
	if (m_Twain.Register("PRACTICE"))
		return FALSE;

	CWaitCursor wait;

	// (d.thompson 2012-02-22) - PLID 47525 - NxMessenger and NxUpdate are no longer part of our installation.  I just removed all the code (and lots of 
	//	no-longer-relevant comments) from this area.

	if (!PreSplash())
		return FALSE;

	// See if the username/password was given on the command line
	// If it wasn't given, then we want to create and display the login dialog, and do so before we call PreLogin
	if (g_cmdInfo.m_strUsername.IsEmpty() || g_cmdInfo.m_strPassword.IsEmpty()) {
		if (!m_dlgLogin->Create(IDD_LOGIN)) {
			//show the splash screen before anything slow happens
			return FALSE;
		}
	}

	// Call PreLogin to prepare some stuff for the login
	if (!PreLogin())
		return FALSE;

	//DRT 7/3/02 - For safety's sake, let's run a check to make sure that if we've loaded a nextech-only license, that the data structure 
	//			is indeed the correct one (contains NxClientsT for example)
	if(IsNexTechInternal()) {

		try {

			if(!DoesTableExist("NxClientsT")) {
				AfxMessageBox("You are currently using a NexTech-Only License.  Your database does not contain the proper tables to handle this license.\n"
								"Please contact NexTech immediately to have the correct license installed.");
			}
		} NxCatchAll("Error determining license status.");
	}

	try {

		// (j.jones 2010-01-08 12:33) - PLID 35778 - cache the excluded audits
		// before any auditing occurs
		LoadExcludedAuditEvents(GetRemoteConnection(), FALSE);

	}NxCatchAll("Error in CPracticeApp::InitInstance when calling LoadExcludedAuditEvents");

	try {

		//JMJ - 6/5/03 - //the BuiltInObject list is hard coded and thereore automatically loaded,
		//but the UserDefinedObject list loads from data, so we must load it at the beginning of the program!
		LoadUserDefinedPermissions();

		//DRT 3/25/03 - This function is contained in NxSecurity.cpp.  See comments inside - but basically it
		//		handles ensuring that any new permissions which have been added are applied to all users, 
		//		templates, groups, etc.
		EnsureDefaultPermissions();
	} NxCatchAll("Error ensuring permissions.");

	// (b.savon 2016-05-17 7:10) - NX-100662 - Move this from PostLogin to InitInstance because we can
	// Initialize the API (as immediate as) in CChooseAdminDlg and that can be a lengthy COM operation.
	//
	// This eliminates the switch to/retry dialog (be careful with any automation 
	// calls because disabling the busy and notresponding dialogs allows 
	// automation deadlocks to happen!)
	COleMessageFilter *pFilter = AfxOleGetMessageFilter();
	if (pFilter) {
		pFilter->EnableBusyDialog(FALSE);
		pFilter->EnableNotRespondingDialog(FALSE);
	}
	else {
		MsgBox(MB_ICONINFORMATION | MB_OK,
			"Practice could not create a message filter for OLE communication.\n\n"
			"Please continue but you may experience a slower response time with "
			"certain operations, such as mail merges and reports.");
	}

	try {

		if(IsRecordsetEmpty("SELECT Administrator FROM UsersT WHERE PersonID > 0 AND Administrator = 1")) {
			CChooseAdminDlg dlg(NULL);
			dlg.DoModal();
			BSTR bstrProgress = CString("").AllocSysString();
			if (m_dlgLogin) {
				m_dlgLogin->SendMessage(NXM_LOGIN_PROGRESS, 0, (LPARAM)bstrProgress);
			}
		}

	}NxCatchAll("Error determining user administrative status.");

	if (!g_cmdInfo.m_strUsername.IsEmpty() && !g_cmdInfo.m_strPassword.IsEmpty()) {
		// Username and password are given on the command line so set them and don't prompt the user

		// Validate the username and password 
		HANDLE hUserHandle = LogInUser(g_cmdInfo.m_strUsername, g_cmdInfo.m_strPassword, g_cmdInfo.m_nLocationID);
		if (hUserHandle) {
			// Username/password combination has been accepted
			// Since the login dialog won't get a chance to call it, we need to call the PostLogin ourselves
			if (!PostLogin(g_cmdInfo.m_strPassword)) {
				return FALSE;
			}
		} else {
			AfxMessageBox("Invalid username, password, or location given on command line.");
			return FALSE;
		}
	} else {
		// Username or password not given on the command line so we need to prompt the user
		
		// Prepare the login dialog
		if (!m_dlgLogin->LoadInitialLoginInfo()) {
			return FALSE;
		}

		// This used to just say "m_pMainWnd = m_dlgLogin" but the login may have terminated itself 
		// and if it did Practice would just hang if we set the mainwnd to a non-existent dialog
		if (m_dlgLogin->GetSafeHwnd()) {
			m_pMainWnd = m_dlgLogin;//just temporary, so we can start the message pump
		} else {
			m_pMainWnd = NULL;
		}
	}

	return TRUE;
}

// App command to run the dialog
void CPracticeApp::OnAppAbout()
{
	CAboutDlg aboutDlg(NULL);
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CPracticeApp commands

void CPracticeApp::OnFileNew() 
{
	OpenDocumentFile(NULL);
}

CDocument *CPracticeApp::OpenFile(LPCTSTR nameFile)
{
	CString fName(nameFile);
	return OpenDocumentFile(fName);
}

CDocument* CPracticeApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	LogIndent();
	
	CString fName(lpszFileName);
	CMainFrame *pMainWnd = GetMainFrame();

	if (pMainWnd && pMainWnd->m_pOpenDoc) {
		pMainWnd->m_pOpenDoc->OnCloseDocument();
		pMainWnd->m_pOpenDoc = NULL;
		pMainWnd->m_nOpenDocCnt = 0;
	}
	CDocTemplate *newTemplate;
	newTemplate = GetTemplate();
	// (a.walling 2014-06-17 17:37) - PLID 53127 - If GetTemplate returns null, then they do not have permission to access any module.
	if (!newTemplate) {
		return nullptr;
	}
	if (fName.GetLength()) {
		CFileFind finder;
		if (finder.FindFile(fName)) {
			pMainWnd->m_pOpenDoc = (CPracticeDoc *)(newTemplate->OpenDocumentFile(fName));
		} else {
			pMainWnd->m_pOpenDoc = (CPracticeDoc *)(newTemplate->OpenDocumentFile(NULL));
		}
	} else {
		pMainWnd->m_pOpenDoc = (CPracticeDoc *)(newTemplate->OpenDocumentFile(NULL));
	}

	if (pMainWnd->m_pOpenDoc) {
		CString name = GetDefaultModuleName();
		if (name == "")
			pMainWnd->GetActiveViewFrame()->DestroyWindow();
		else
		{	pMainWnd->GetActiveViewFrame()->SetType(name);
			pMainWnd->GetActiveViewFrame()->ModifyStyle(FWS_ADDTOTITLE, WS_CAPTION);//BVB
			pMainWnd->GetActiveViewFrame()->SetWindowText(name);

			if(name == "Scheduler") {
				// (j.jones 2006-12-21 17:23) - PLID 22793 - check the preference to open the room manager
				if(GetRemotePropertyInt("OpenRoomMgrWhenOpenScheduler", 0, 0, GetCurrentUserName(), true) == 1) {

					GetMainFrame()->PostMessage(NXM_OPEN_ROOM_MANAGER, (long)-1,
						(BOOL)GetRemotePropertyInt("OpenRoomMgrWhenOpenSchedulerMinimized", 0, 0, GetCurrentUserName(), true) == 1);
				}
			}
		}
	}

	LogUnindent();

	return pMainWnd->m_pOpenDoc;
}

CDocTemplate * CPracticeApp::GetTemplate(LPCTSTR TemplateName)
{
	try {
		if (!this) {
			return nullptr;
		}

		CString tmpName = TemplateName;

		if (!tmpName.GetLength()) tmpName = GetDefaultModuleName();

		if (!m_pDocManager) {
			return nullptr;
		}

		CDocManager *tmpMan = m_pDocManager;
		POSITION iter;
		CString tmpStr;
		iter = tmpMan->GetFirstDocTemplatePosition();

		CDocTemplate* pTemplate = nullptr;

		// (a.walling 2014-06-09 15:19) - PLID 53127 - cleaned up a bit; no longer potential deref of NULL iter
		while (iter) {
			pTemplate = tmpMan->GetNextDocTemplate(iter);
			if (!pTemplate) {
				continue;
			}
			pTemplate->GetDocString(tmpStr, CDocTemplate::docName);
			if (tmpStr == tmpName) {
				return pTemplate;
			}
		}
	} NxCatchAll(__FUNCTION__);

	return nullptr;
}

// This function searches for an existing instance of the program
// If it finds this other instance it will bring it to focus
bool CPracticeApp::ShowOtherInstance()
{
	// (b.cardillo 2005-05-06 18:42) - PLID 16471 - Get the process id of the other Practice 
	// instance, then limit the window search to only windows under that process.  
	DWORD dwFindProcessId;
	{
		// (a.walling 2006-11-10 10:18) - PLID 23294 - Use the MappedFileName function for mapped files based on working path
		HANDLE hMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4, GetMappedFileName());
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			LPVOID pMem = MapViewOfFile(hMappedFile, FILE_MAP_READ, 0, 0, 4);
			memcpy(&dwFindProcessId, pMem, 4);
			UnmapViewOfFile(pMem);
			CloseHandle(hMappedFile);
		} else {
			CloseHandle(hMappedFile);
			dwFindProcessId = 0;
		}
	}

	// (b.cardillo 2005-05-06 18:44) - PLID 16471 - If there is another instance of Practice 
	// running, we now know its pid.  Use that to filter our window search (so we don't 
	// inadvertantly maximize another application, like NxAttach).
	if (dwFindProcessId != 0) {
		// Search the windows
		HWND hwnd = FindWindow(NULL, NULL);
		hwnd = GetWindow(hwnd, GW_HWNDFIRST);
		while (hwnd) {
			// Only work with visible windows, and windows that aren't our own (that check 
			// isn't actually necessary, but just to be safe)
			if (hwnd != m_pMainWnd->GetSafeHwnd() && hwnd != m_dlgLogin->GetSafeHwnd() && IsWindowVisible(hwnd)) {
				// Get the process id of this window
				DWORD dwProcId;
				if (::GetWindowThreadProcessId(hwnd, &dwProcId)) {
					// Only deail with windows under the process that we know to be the other 
					// instance of Practice.
					if (dwProcId == dwFindProcessId) {
						// Found a window owned by the process we're looking for, now find 
						// its highest parent
						HWND hwndParent = GetParent(hwnd);
						while (hwndParent && IsWindowVisible(hwndParent)) {
							hwnd = hwndParent;
							hwndParent = GetParent(hwnd);
						}
						// If the highest parent doesn't belong to us (if our logic is correct, 
						// that should be impossible, but check just in case) and is visible, 
						// then we have the window we want to give focus to.
						if (hwnd != m_pMainWnd->GetSafeHwnd() && hwnd != m_dlgLogin->GetSafeHwnd() && IsWindowVisible(hwnd)) {
							// If it's a practice main window, maximize it
							CString str;
							str.ReleaseBuffer(GetWindowText(hwnd, str.GetBuffer(25), 25));
							if ((str.Left(16).CompareNoCase("NexTech Practice") == 0))
							{
								ShowWindow(hwnd, SW_MAXIMIZE);
							}
							// And then activate it
							SetForegroundWindow(hwnd);
							// And return the fact that we successfully activated the other instance.
							return true;
						}
					}
				}
			}
			// Haven't found it yet, keep searching
			hwnd = GetWindow(hwnd, GW_HWNDNEXT);
		}
		// We couldn't find it for some reason.  We have to return failure, even though we're 
		// CONFIDENT that other instance is out there, if we couldn't give it focus then we 
		// have to just let the user open a new instance.
		return false;
	} else {
		// Either there isn't another instance of Practice running (but then why did this 
		// function get called?) or the other instance is an old version of Practice that 
		// doesn't support the new mechanism described above.
		HWND tmpHwnd;
		tmpHwnd = FindWindow(NULL, NULL);
		tmpHwnd = GetWindow(tmpHwnd, GW_HWNDFIRST);
		if (tmpHwnd) {
			CString tmpStr;
			char *tmpPtr;
			// (b.cardillo 2005-05-06 18:57) - PLID 16472 - We used to have the m_pMainWnd and 
			// m_dlgLogin checks in the while statement itself, which meant if we ran into one 
			// of those we would just quit the search.  What we want to do is skip them, not 
			// just terminate when we hit them.  So I moved it into an if-statement INSIDE the 
			// while rather than a condition of the loop itself.
			while (tmpHwnd) {
				if (tmpHwnd != m_pMainWnd->GetSafeHwnd() && tmpHwnd != m_dlgLogin->GetSafeHwnd() && IsWindowVisible(tmpHwnd)) {
					// See if this window is a child of NexTech Practice mainframe
					{
						HWND hwnd = tmpHwnd;
						HWND hwndParent = GetParent(hwnd);
						while (hwndParent && IsWindowVisible(hwndParent)) {
							hwnd = hwndParent;
							hwndParent = GetParent(hwnd);
						}
						CString str;
						str.ReleaseBuffer(GetWindowText(hwnd, str.GetBuffer(25), 25));
						// (b.cardillo 2005-05-06 18:08) - PLID 16471 - The if-statement below is not 100% reliable, 
						// because someday we could have another utility that's called, say "NexTech Practice 2006 
						// Utility for Doing Things", and then this could once again incorrectly think that utility 
						// is actually Practice.  Also, let's say there are several instances of Practice currently 
						// running (say different registry bases, like "main", "patch", and "internal"), this code 
						// would just arbitrarily pick one of them and bring that one to the foreground.  That's why 
						// we have the first half of the big if-statement of this function (the one whose "else" we 
						// are in right now).  That code will actually seek a window specific to the instance of 
						// Practice being attempted, and it will set THAT application to the foreground.  The only 
						// reason this whole else block is still here is for our staff internally, because we may 
						// be running two different versions of Practice on the same machine, and the old versions 
						// don't know about the file mapping mechanism we now use, so the above approach won't work 
						// for those cases.
						// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
						if ((str.Left(16).CompareNoCase("NexTech Practice") == 0) && str.GetLength() >= 18 && isdigit(unsigned char(str.GetAt(17))))
						{
							ShowWindow(hwnd, SW_MAXIMIZE);
							SetForegroundWindow(hwnd);
							return true;
						}
					}
					// See if it's the login window or the closing window
					tmpPtr = tmpStr.GetBuffer(25);
					GetWindowText(tmpHwnd, tmpPtr, 25);
					tmpStr.ReleaseBuffer(-1);
					if (tmpStr.Left(17) == "Practice - Log In") {
						SetForegroundWindow(tmpHwnd);
						return true;
					}
					if (tmpStr.Left(24) == "Closing NexTech Practice") {
						SetForegroundWindow(tmpHwnd);
						return true;
					}
				}
				tmpHwnd = GetWindow(tmpHwnd, GW_HWNDNEXT);
			}
		}
		return false;
	}
}


void CPracticeApp::LoadPrinterSettingsFromRegistry()
{
	// (z.manning 2010-11-22 12:26) - PLID 40486 - Renamed this function
	LoadDevSettingsFromRegistryHKLM("PrintSettingsGlobal", m_hDevNames, m_hDevMode);
}

BOOL CPracticeApp::SavePrinterSettingsToRegistry()
{
	// (z.manning 2010-11-22 12:26) - PLID 40486 - Renamed this function
	return SaveDevSettingsToRegistryHKLM("PrintSettingsGlobal", m_hDevNames, m_hDevMode);
}

void CPracticeApp::DefaultPrintSetup()
{
	// Try to load the device settings from the registry if they're there (they would have been saved last time by CPracticeApp::ExitInstance())
	LoadPrinterSettingsFromRegistry();
	
	// Now have the CPrintDialog class validate the app's current device settings (if the app doesn't have device settings yet, then the defaults will be loaded)
	{
		// Open common dialog to validate the app's current device settings
		CPrintDialog pd(TRUE);
		pd.m_pd.Flags |= PD_RETURNDEFAULT;
		pd.m_pd.hDevMode = (HANDLE)m_hDevMode;
		pd.m_pd.hDevNames = (HANDLE)m_hDevNames;
		
		// Put the call in a try catch block because on some versions of windows it can throw a low-level exception
		try {
			pd.DoModal();
		} catch (...) {
			AfxThrowNxException(
				"Could not connect to your default printer.  Please review your printer configuration by "
				"clicking Start -> Settings -> Printers.  You must have access to your default printer.");
		}

		// Refresh current cache of printer device information
		m_hDevMode = pd.m_pd.hDevMode;
		m_hDevNames = pd.m_pd.hDevNames;
	}
}

BOOL PreTranslateMessagesForNotificationDlg(CWinApp *pApp, MSG *pMsg)
{
	switch (pMsg->message) {
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_SETCURSOR:
		// Make sure something has capture (because this function is made to simulate mouse 
		// actions on the notification dialog WHEN SOMETHING ELSE HAS CAPTURE).  Also make 
		// sure the app is not null and that the app's main window is a CMainFrame.
		if (GetCapture() != NULL && pApp && pApp->m_pMainWnd && pApp->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CMainFrame))) {
			// Get the current official notification dialog
			CNotificationDlg *pNotificationDlg = ((CMainFrame *)pApp->m_pMainWnd)->GetNotificationDlg();
			if (pNotificationDlg->GetSafeHwnd() && pNotificationDlg->IsWindowVisible()) {
				// Get the rect of the notification dialog (in screen coords)
				CRect rc;
				{
					pNotificationDlg->GetClientRect(&rc);
					::ClientToScreen(pNotificationDlg->GetSafeHwnd(), &(rc.TopLeft()));
					::ClientToScreen(pNotificationDlg->GetSafeHwnd(), &(rc.BottomRight()));
				}
				// Get the mouse position as reported by this message (but in screen coords)
				CPoint pt;
				{
					if (pMsg->message == WM_SETCURSOR) {
						// For the WM_SETCURSOR message we're not given the point, we have to come up with it ourselves
						GetCursorPos(&pt);
					} else {
						pt = CPoint(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
						::ClientToScreen(pMsg->hwnd, &pt);
					}
				}
				// See if the point is within the client area of the notification dialog
				if (rc.PtInRect(pt)) {
					// The point is in the rect so pass this message on to the notification dialog (don't forget to put 
					// the point into coords relative to the notification dialog)
					if (pMsg->message != WM_SETCURSOR) {
						pNotificationDlg->ScreenToClient(&pt);
						pNotificationDlg->SendMessage(pMsg->message, pMsg->wParam, MAKELONG(pt.x, pt.y));
					}
					// Set the cursor to the hand icon
					SetCursor(GetLinkCursor());
					// Return TRUE because we handled the message
					return TRUE;
				} else {
					// The point is NOT in the rect so tell the notification dialog to draw itself back to normal mode (not BOLD mode)
					// This is a little funky, on mousemove here we're sending a WM_SETCURSOR with HTCLIENT because we're relying on the 
					// side-effect that it will change itself back to normal (not BOLD) mode.
					if (pMsg->message == WM_MOUSEMOVE) {
						pNotificationDlg->ScreenToClient(&pt);
						pNotificationDlg->SendMessage(WM_SETCURSOR, (WPARAM)pNotificationDlg->GetSafeHwnd(), HTCLIENT);
						// We also set the curor back to normal since we took responsibility for setting it to the hand 
						// icon up above (when the mouse is over the notification dialog)
						SetCursor(::LoadCursor(NULL, IDC_ARROW));
					}
					// We're not handling it entirely ourselves (we're just getting it like an event) so 
					// return FALSE to let the app do whatever it wants with it
					return FALSE;
				}
			} else {
				// The notification dialog doesn't exist or isn't visible so do nothing
				return FALSE;
			}
		} else {
			// Nothing has capture so we do nothing
			return FALSE;
		}
		break;
	default:
		// Not a message we care about so do nothing
		return FALSE;
		break;
	}
}

// (a.walling 2009-06-05 16:56) - PLID 34512 - Is this an input message?
bool IsInputMsg(const UINT message)
{
	switch(message) {
		// Keyboard messages
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:

		// Mouse / tablet messages
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:

		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:

		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCLBUTTONDBLCLK:

		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:
		case WM_NCMBUTTONDBLCLK:

		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCRBUTTONDBLCLK:

		case WM_NCXBUTTONDOWN:
		case WM_NCXBUTTONUP:
		case WM_NCXBUTTONDBLCLK:

		case WM_MOUSEWHEEL:
		case 0x020E: // WM_MOUSEHWHEEL:

		// Other
		case WM_SYSCOMMAND:
		case WM_CONTEXTMENU:

		// (a.walling 2009-08-13 09:29) - PLID 35205 - The NXM_EXTEND_INACTIVITY_TIMEOUT should not be considered an input message.
		// This was causing infinite loops in the TWAIN window procedure in some circumstances, since it was considered an input
		// message, and therefore posting this message again, which goes to the window procedure, ad infinitum.
		// (a.walling 2009-12-03 10:16) - PLID 35205 - On second thought, we will still consider this an input message, so by simply
		// posting this message to the UI thread we will extend the inactivity timeout.
		case NXM_EXTEND_INACTIVITY_TIMEOUT:
			return true;
		default:
			return false;
	}

	return false;
}

BOOL CPracticeApp::PreTranslateMessage(MSG* pMsg) 
{
	//static long lParamMouse = 0;
//// All this does is prevent us from seeing the call stack, thus making it 
//// annoying to get to the function were the exception was thrown
////#ifdef _DEBUG 
////	try {
////#endif

		// (a.walling 2009-06-08 12:38) - PLID 34512 - Handle extending the inactivity timeout here rather
		// than in the mainframe
		if (IsInputMsg(pMsg->message)) {
			CMainFrame *pMainFrm = GetMainFrame();

			if (pMainFrm) {
				
				if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) {
					if(pMainFrm->HotKey(pMsg->wParam)) {
						//The hotkey was not handled, so do something with
						//	it if you want.
					}
				}

				if (IsWindow(pMainFrm->m_dlgAutoLogoff.GetSafeHwnd())) {
					pMainFrm->m_dlgAutoLogoff.ExtendInactivityTimer();
				}
			}
		}

		// (a.walling 2009-06-08 09:52) - PLID 34512 - See process message filter and above
		/*
		if(pMsg->message == WM_LBUTTONDOWN) {
			HWND hWnd = pMsg->hwnd;
		}
		if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN ||
			pMsg->message == WM_KEYUP || pMsg->message == WM_SYSKEYUP ||
			pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP ||
			pMsg->message == WM_RBUTTONDOWN || pMsg->message == WM_RBUTTONUP ||
			pMsg->message == WM_MBUTTONDOWN || pMsg->message == WM_MBUTTONUP ||
			(pMsg->message == WM_MOUSEMOVE && lParamMouse != pMsg->lParam))
		{
			//Reset the mouse position
			if (pMsg->message == WM_MOUSEMOVE)
				lParamMouse = pMsg->lParam;

			//Reset the inactivity timer
			if (m_pMainWnd)
				((CMainFrame*)m_pMainWnd)->ResetInactivityTimer();
		}
		*/

		// Process TWAIN messages
		// (c.haag 2004-12-29 17:07) We should not be processing TWAIN messages
		// here anymore.
		//NXTWAINlib::ProcessTWMessage(pMsg);

		// Allow the notification window to steal away these three mouse messages
		if (PreTranslateMessagesForNotificationDlg(this, pMsg)) {
			return TRUE;
		}

		// CAH 6/18/2003 - This can happen if Mirror 6.1 is installed and you close
		// Practice. I don't understand why it's happening, but it doesn't cause
		// any apparent problems, and the condition is met once and only once where
		// the message is WM_SIZE; the last message is WM_ENABLE and the next to last
		// message is WM_ACTIVATEAPP. Why would it try to size the MainFrame at that
		// point if Mirror 6.1 is enabled??
		if (AfxGetMainWnd() && FALSE == ::IsWindow(AfxGetMainWnd()->GetSafeHwnd()))
			return FALSE;

		return CNxAdvancedUIApp::PreTranslateMessage(pMsg);
//// All this does is prevent us from seeing the call stack, thus making it 
//// annoying to get to the function were the exception was thrown
////#ifdef _DEBUG 
////	} NxCatchAllCall("CPracticeApp::PreTranslateMessage 1", try {
////		return CNxAdvancedUIApp::PreTranslateMessage(pMsg);
////	} NxCatchAllCall("CPracticeApp::PreTranslateMessage 2", return TRUE));
////#endif
}

// (a.walling 2009-06-08 10:57) - PLID 34512 - Extend the inactivity timer if necessary
BOOL CPracticeApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	// this complements CNxAdvancedUIApp::PreTranslateMessage because this function
	// will be called for messages processed in a modal dialog box

	// unsafe to catch exceptions in here
	if (IsInputMsg(lpMsg->message)) {
		CMainFrame *pMainFrm = GetMainFrame();

		if (pMainFrm) {
			if (IsWindow(pMainFrm->m_dlgAutoLogoff.GetSafeHwnd())) {
				pMainFrm->m_dlgAutoLogoff.ExtendInactivityTimer();
			}
		}
	}

	// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - handles the common messagefilter for dialog modal loops
	return CNxAdvancedUIApp::ProcessMessageFilter(code, lpMsg);
}

extern CArray<int, int> *g_parAllReferralsPlus;
int CPracticeApp::ExitInstance() 
{
	// (a.walling 2012-04-25 16:19) - PLID 49987 - Uninitialize instance data
	m_pPracticeInstanceData.reset();

	// Write the printer settings to the registry so we can recall them next time we load (see CPracticeApp::DefaultPrintSetup())
	SavePrinterSettingsToRegistry();

	//Disconnect TWAIN source
	NXTWAINlib::CloseConnection();

	// (a.walling 2010-06-23 11:55) - PLID 39321
	FreeRemoteDataCache();

	//TES 6/16/2006 - Need to call TCP_Destroy() to clean up its memory.
	TCP_Destroy();

	// (a.walling 2007-05-11 13:53) - PLID 4850 - Moved clearing last payment, last bill/quote, and closing case histories
	// to TryCloseAll for logging out a user.

	// Delete the file that holds this directory for only one user's use
	if (m_hFileFolderAvail != INVALID_HANDLE_VALUE) {
		// (b.cardillo 2009-03-26 16:03) - PLID 14887 - The file is delete_on_close, so to delete it 
		// all we have to do is close our handle.
		CloseHandle(m_hFileFolderAvail);
		m_hFileFolderAvail = INVALID_HANDLE_VALUE;
	}

	// (b.cardillo 2011-01-06 20:34) - PLID 33748 - We now hold a global shared mutex for our working path too.
	if (m_hNxLoginDatMutex != NULL) {
		CloseHandle(m_hNxLoginDatMutex);
		m_hNxLoginDatMutex = NULL;
	}	

	// (b.cardillo 2006-10-24 11:09) - PLID 19344 - Release the unused ink collector.  We 
	// don't really care if this fails, so we just log any exceptions.
	// TODO: Remove this, once Microsoft resolves our MSDN incident case #srx061019604066.
	try {
		if (m_pAlwaysOpenCollector) {
			m_pAlwaysOpenCollector.Release();
		}
	} catch (CException *e) {
		CString strErr;
		if (e->GetErrorMessage(strErr.GetBuffer(4096), 4095, NULL)) {
			strErr.ReleaseBuffer();
		} else {
			strErr.ReleaseBuffer(0);
			strErr = "Unknown CException";
		}
		e->Delete();
		LogFlat("CPracticeApp::ExitInstance(): Failed to release m_pAlwaysOpenCollector due to CException: %s", strErr);
	} catch (_com_error e) {
		LogFlat("CPracticeApp::ExitInstance(): Failed to release m_pAlwaysOpenCollector due to COM error %li: %s (%s)", e.Error(), (LPCTSTR)e.Description(), e.ErrorMessage());
	} catch (...) {
		LogFlat("CPracticeApp::ExitInstance(): Failed to release m_pAlwaysOpenCollector due to a low level exception");
	}

	//DRT 7/3/02 - TODO:  this reports all errors that have been stored up during program execution, mainly NxLog errors
	//			due to the multi database usage.  We need to add better logging to NxStandard (it handles all the log 
	//			stuff now) so that it will log to different files depending on the database we're working on.
	//			Until that point, this is just going to be commented out 

	// Report errors, if any were encountered
	//ReportErrors("Errors were encountered during program execution.", true);

	// Close our session with the log file (this doesn't actually close the file, 
	// it just logs the "closed" line and tells us not to log anything else)
	ClosePracticeLogFile();

	// (z.manning, 07/19/2006) - PLID 21510 - We at least need to clean up any errors that may have been
	// allocated. (Note: PLID 21519 deals with the lack of handling these errors.)
	// Also, these needs to be done after we close the log file since most of the logging stuff is in NxStandard.
	// (a.walling 2008-09-18 17:10) - PLID 28040 - This is no longer used
	//ClearErrors();

	ClearUserDefinedPermissionList();

	// We're done with the license
	if (g_pLicense) {
		delete g_pLicense;
		g_pLicense = NULL;
	}

	// Close the network utilities
	NxSocketUtils::Destroy();

	if (g_parAllReferralsPlus) {
		delete g_parAllReferralsPlus;
		g_parAllReferralsPlus = NULL;
	}

	if (m_dlgLogin) {
		delete m_dlgLogin;
		m_dlgLogin = NULL;
	}

	//close down TAPI
	GetMainFrame()->ShutdownTapi();
	// (z.manning, 05/05/2008) - PLID 29680 - FreeWtDlls is no longer needed
	//void FreeWtDlls();
	//FreeWtDlls();

	// (c.haag 2005-01-26 15:51) - PLID 16445 - Destroy ConfigRT caching
	DestroyRemotePropertyCache();

	// We have to release any existing COM objects or else 
	// Practice won't close properly 
	extern _RecordsetPtr g_rsReferrals;
	if (g_rsReferrals != NULL){
		g_rsReferrals.Release();
	}
	
	extern _RecordsetPtr g_rsReferralsPlus;
	if (g_rsReferralsPlus != NULL) {
		g_rsReferralsPlus.Release();
	}

	extern _RecordsetPtr g_rsCategories;
	if (g_rsCategories != NULL){
		g_rsCategories.Release();
	}
	
	extern _RecordsetPtr g_rsConfigRT;
	if (g_rsConfigRT != NULL){
		g_rsConfigRT.Release();
	}

	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - NxAdo handles this
	//extern _ConnectionPtr g_ptrRemoteData;
	//if (g_ptrRemoteData != NULL) {
	//	g_ptrRemoteData.Release();
	//}
	
	extern _ConnectionPtr g_pConMirror;
	if (g_pConMirror != NULL){
		g_pConMirror.Release();
	}

	// (a.walling 2014-05-19 15:29) - PLID 62183 - VS2013 - Practice can hang on shutdown
	// this frees all unused COM dlls that may still be loaded
	// therefore also minimizing the surface area exposed to loader lock hangs
	::CoFreeUnusedLibraries();

	// (a.walling 2008-04-01 12:19) - PLID 29497 - Shutdown GDI+
	// This is handled by the helper class now.
	g_GDIPlusHolder.UnInitialize();

	// Close the mutex because Practice is now completely finished
	if (m_hmutex) {
		ReleaseMutex(m_hmutex);
	}

	//TES 2/13/04: Please DO NOT add any code here!  The ReleaseMutex() should be the last thing we do, and releasing
	//all our com objects should be the penultimate thing.  Add your code up above,if it needs to be in this function.
	/*//kill the dragon stuff
	DestroyDragon();*/

	return CNxAdvancedUIApp::ExitInstance();
}

void CPracticeApp::WinHelp(DWORD dwData, UINT nCmd)
{
	if (nCmd == HELP_CONTEXT) {
		nCmd = HELP_CONTEXTPOPUP;
	}
	CNxAdvancedUIApp::WinHelp(dwData, nCmd);
}

BOOL CPracticeApp::OnIdle(LONG lCount) 
{
	if (m_dlgLogin->GetSafeHwnd())
		return CNxAdvancedUIApp::OnIdle(lCount);
	//I hate doing this, but the login must be our main frame temporarily
	//since we have to start the message pump 
	//if we want to log in before creating the mainframe

	BOOL more = CNxAdvancedUIApp::OnIdle(lCount);
	if (GetMainFrame())
		if (GetMainFrame()->Idle(lCount))
			more = TRUE;
	return more;
}

HANDLE CPracticeApp::GetDevModeHandle()
{
	return m_hDevMode;
}

void CPracticeApp::SetDevModeHandle(HANDLE hNewValue)
{
	m_hDevMode = hNewValue;
}

HANDLE CPracticeApp::GetDevNamesHandle()
{
	return m_hDevNames;
}

void CPracticeApp::SetDevNamesHandle(HANDLE hNewValue)
{
	m_hDevNames = hNewValue;
}

void CPracticeApp::DeleteAllCaseHistories()
{
	try {
		//delete any tracked case histories
		CPtrArray &aryOfWnds = m_arypCaseHistories;
		long nSize = aryOfWnds.GetSize();
		for (long i=aryOfWnds.GetSize()-1; i>=0; i--) {
			CCaseHistoryDlg *pTrackedWnd = (CCaseHistoryDlg *)aryOfWnds[i];
			if (pTrackedWnd->GetSafeHwnd()) {
				pTrackedWnd->EndDialog(IDCANCEL);
				aryOfWnds.RemoveAt(i);
				delete pTrackedWnd;
			}		
		}
	}NxCatchAll("Error cleaning up case histories.");
}

// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - Called when schema version saved is out of date
void CPracticeApp::OnSchemaOutOfDate(int nCurrentSchemaVersion, int nSavedSchemaVersion)
{	
	// (a.walling 2012-06-04 16:30) - PLID 49829 - Removed for now since this is per user and that is not available when creating the initial instance
#pragma TODO("!!!!!!!!!!Now that this stuff is stored to db, need a better way to handle the settings schema versions")
	//CleanState("EMR");
	//CleanState("EMRTemplate");
	//CleanState("Workspace");
}

// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - Moved a lot of implementations into CNxAdvancedUIApp

// (a.walling 2012-06-04 16:30) - PLID 49829 - Load the current user's data from the db
void CPracticeApp::LoadCurrentUserSettingsData()
{	
	try {
		ResetSettingsData();

		// (a.walling 2012-06-04 16:42) - PLID 50763 - Uncompress the serialized settings data
		m_pSettingsData = PrepareSettingsData(
			GetRemotePropertyImage("NxSettingsData", 0, GetCurrentUserComputerName(), true)
		);

		return;
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-04 16:42) - PLID 50763 - uncompress the serialized settings data
NxTreeSettingsDataPtr CPracticeApp::PrepareSettingsData(VARIANT& varData)
{	
	try {
		NxTreeSettingsDataPtr pData(new NxTreeSettingsData());

		if (varData.vt != (VT_UI1 | VT_ARRAY)) {
			ASSERT(varData.vt == VT_NULL || varData.vt == VT_EMPTY);
			return pData;
		}

		COleSafeArray saData;
		saData.Attach(NxCompressUtils::NxUncompressSafeArrayOfBytesToSafeArrayOfBytes(varData)); // includes NULL
		
		char* szData = NULL;
		saData.AccessData((void**)&szData);
		
		pData->LoadFromString(szData);
		saData.UnaccessData();

		return pData;
	} NxCatchAllThrow(__FUNCTION__);
}
	
// (a.walling 2012-06-04 16:42) - PLID 50763 - Compress the serialized settings data
_variant_t CPracticeApp::PrepareSettingsData(NxTreeSettingsDataPtr pData)
{
	try {
		_variant_t var = g_cvarNull;
		
		if (pData && !pData->GetTree().empty()) {
			COleSafeArray sa;
			{
				std::string str = pData->ToString();
				sa.CreateOneDim(VT_UI1, str.length() + 1, (const void*)str.c_str());
			}
			var.Attach(NxCompressUtils::NxCompressSafeArrayOfBytesToSafeArrayOfBytes(sa)); // includes NULL
		}

		return var;
	} NxCatchAllThrow(__FUNCTION__);
}

NxTreeSettingsDataPtr CPracticeApp::GetSettingsData(BOOL bAdmin, LPCTSTR szPath)
{
	return CNxAdvancedUIApp::GetSettingsData(bAdmin, szPath);
}

// (a.walling 2012-06-04 16:30) - PLID 49829 - Save the current data to the db
void CPracticeApp::SaveSettingsData()
{	
	if (m_pSettingsData && !m_pSettingsData->Changed()) return;

	// (a.walling 2012-06-04 16:42) - PLID 50763 - Compress the serialized settings data
	SetRemotePropertyImage(
		"NxSettingsData"
		, PrepareSettingsData(m_pSettingsData)
		, 0
		, GetCurrentUserComputerName()
	);
}

// (b.savon 2015-12-16 09:29) - PLID 67718 - Split this out into a function
BOOL CPracticeApp::InitializeAPI(const CString &strPassword, CWnd* pParentDlg /*= NULL*/)
{
	// (b.savon 2016-01-13 11:26) - PLID 67718 - Supplemental
	if (m_pAPIManager == NULL || !m_pAPIManager->IsApiInitialized() || m_pAPIManager->GetLoginResult() == NULL || m_pAPIManager->GetLoginResult()->GetLoginFailed() == VARIANT_TRUE) {

		if (m_dlgLogin) {
			// (b.cardillo 2016-03-15 21:56) - PLID 67718 (supplemental) - Corrected potential BSTR leak
			m_dlgLogin->SendMessage(NXM_LOGIN_PROGRESS, 0, (LPARAM)CString("Initializing the API...").AllocSysString());
		}

		// (j.armen 2013-05-09 09:59) - PLID 56612 - We want to save some device info.  The host name and workstation path 
		// are good enough, but check if we are concurrent just in case.  If so, then use the full session path.
		CString strDeviceInfo = CLicense::GetSystemName() + '.' + GetPracPath(g_pLicense->IsConcurrent() ? PracPath::SessionPath : PracPath::PracticePath);

		// (z.manning 2012-08-02 10:34) - PLID 51763 - Initialize the API object
		// (z.manning 2015-12-03 10:43) - PLID 67637 - Don't use the password we save in memory (which we shouldn't be doing anyway!)
		// because it's not valid when logging in as the support user.
		// (b.savon 2016-01-05 11:03) - PLID 67819 - Actually, let's just not use the password in memory IFF this is the support user.
		if (GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			m_pAPIManager.reset(new CNxAPIManager(GetSubRegistryKey(), GetCurrentUserID(), strPassword, GetCurrentLocationID(), strDeviceInfo));
		}
		else {
			m_pAPIManager.reset(new CNxAPIManager(GetSubRegistryKey(), GetCurrentUserID(), GetCurrentUserPassword(), GetCurrentLocationID(), strDeviceInfo));
		}

		// (a.wilson 2014-01-21 12:38) - PLID 58708 - Ensure we can establish a connection to the soap server.
		try {
			m_pAPIManager->GetAPI();
		}
		catch (_com_error &e) {
			if (m_dlgLogin) {
				// (b.cardillo 2016-03-15 21:56) - PLID 67718 (supplemental) - Corrected potential BSTR leak
				CString strLoginBanner = GetRemotePropertyText("LoginBanner", "", 0, "<None>", true);
				m_dlgLogin->SendMessage(NXM_LOGIN_PROGRESS, 0, (LPARAM)strLoginBanner.AllocSysString());
			}
			//catch the error and determine if its a failure connecting to the remote server.
			//if it is then we need to display an appropriate message explaining what they need to do to get it fixed.
			if (e.Error() == 0x80131509) {
				MessageBox(pParentDlg->GetSafeHwnd(),
					"Practice failed to connect to the NexTech SOAP Service.\r\n"
					"Please ensure the NexTech SOAP Service is running on the server.\r\n\r\n"
					"Otherwise, contact NexTech Technical Support for assistance.",
					"NexTech SOAP Service Connection Failure", MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
			else {
				//if its not our remote server issue then throw the error again.
				throw e;
			}
		}
		// (z.manning 2012-08-31 11:54) - PLID 52413 - The dashboard used to have its own API login and a keep alive timer.
		// I moved the keep alive logic to the API manager so we'll set the timer interval to the same value the dashboard
		// had been using.
		// (j.jones 2014-01-16 11:54) - PLID 60354 - Changed from 5400000 to a calculated value. It's still 90 minutes, just written differently.		
		m_pAPIManager->SetKeepAliveTimer(90 * 60 * 1000);
	}
	else {
		// We're already initialized, don't do anything but fall through to return TRUE
	}
	
	return TRUE;
}

// (z.manning 2012-08-02 10:42) - PLID 51763
CNxAPIManager* CPracticeApp::GetAPIObject()
{
	if(!m_pAPIManager) {
		AfxThrowNxException(CString(__FUNCTION__) + " - Trying to access API manager before it's been initialized");
	}
	return m_pAPIManager.get();
}

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib

// (z.manning 2012-08-02 10:29) - PLID 51763 - Global function to access the API
NexTech_Accessor::_PracticeMethods* GetAPI()
{
	extern CPracticeApp theApp;
	return theApp.GetAPIObject()->GetAPI();
}

// (z.manning 2012-08-02 11:28) - PLID 51763
_bstr_t GetAPILoginToken()
{
	extern CPracticeApp theApp;
	return _bstr_t(theApp.GetAPIObject()->GetLoginToken());
}

// (j.jones 2013-04-17 10:47) - PLID 56303 - added GetLoginTokenID, returns UserLoginTokensT.ID, could be null if using an outdated API
// (j.armen 2013-05-15 09:48) - PLID 56680 - No longer returns null
long GetAPIUserLoginTokenID()
{
	extern CPracticeApp theApp;
	return theApp.GetAPIObject()->GetUserLoginTokenID();
}

// (z.manning 2012-08-02 11:29) - PLID 51763
_bstr_t GetAPISubkey()
{
	extern CPracticeApp theApp;
	return _bstr_t(theApp.GetAPIObject()->GetDatabaseSubkey());
}

// (z.manning 2016-02-10 16:35) - PLID 68223
CGenericWordProcessorManager* CPracticeApp::GetWPManager()
{
	return m_pWordProcessorManager.get();
}

// (z.manning 2016-02-10 16:36) - PLID 68223 - Global function to load the word processor manager
CGenericWordProcessorManager* GetWPManager()
{
	extern CPracticeApp theApp;
	return theApp.GetWPManager();
}

//TES 3/2/2015 - PLID 64734 - Is the DebugMode registry setting currently turned on?
BOOL CPracticeApp::GetDebugModeEnabled()
{
	long nMode = NxRegUtils::ReadLong(CString(::GetRegistryBase()) ^ "DebugMode", 0);
	if (nMode == 0)
	{
		return FALSE;
	}
	else {
		return TRUE;
	}
}

//TES 3/2/2015 - PLID 64736 - Is a debugger currently attached to this process?
BOOL CPracticeApp::GetDebugModeActive()
{
	return IsDebuggerPresent();
}

//TES 3/2/2015 - PLID 64736 - Set the DebugMode registry setting, and attach/detach NxDebug accordingly
void CPracticeApp::SetDebugMode(BOOL bDebug)
{
	//TES 3/2/2015 - PLID 64736 - We now have toggled the DebugMode setting this session.
	m_bDebugChangedThisSession = TRUE;

	if (bDebug) {
		//TES 3/2/2015 - PLID 64736 - Update the registry key
		NxRegUtils::WriteLong(CString(::GetRegistryBase()) ^ "DebugMode", 1);
		if (GetDebugModeActive()) {
			return;
		}
		//TES 3/2/2015 - PLID 64736 - Attach NxDebug
		AttachNxDebug();
		//TES 3/2/2015 - PLID 64739 - Update the title bar
		GetMainFrame()->LoadTitleBarText();
	}
	else {
		//TES 3/2/2015 - PLID 64737 - Update the registry key
		NxRegUtils::WriteLong(CString(::GetRegistryBase()) ^ "DebugMode", 0);
		//TES 3/2/2015 - PLID 64737 - Pass in the string to tell NxDebug to deatch
		OutputDebugString(NXDEBUG_TERMINATE_DEBUGGING_STRING);
		//TES 3/2/2015 - PLID 64739 - Update the title bar
		GetMainFrame()->LoadTitleBarText();
	}
}

//TES 3/2/2015 - PLID 64736 - Attach NxDebug to the current process
void CPracticeApp::AttachNxDebug()
{
	SHELLEXECUTEINFO sei;
	ZeroMemory(&sei, sizeof(sei));

	//TES 3/2/2015 - PLID 64736 - NxDebug.exe should be in our registry path
	CString strNxDebug = NxRegUtils::ReadString(CString(::GetRegistryBase()) ^ "InstallPath") ^ "NxDebug.exe";

	sei.cbSize = sizeof(sei);

	sei.fMask = NULL;
	sei.hwnd = NULL;
	//TES 3/2/2015 - PLID 64736 - Make sure we "Run as Administrator"
	sei.lpVerb = "runas";
	sei.lpFile = strNxDebug;
	CString strParams;
	//TES 3/2/2015 - PLID 64736 - Pass in our PID
	strParams.Format("%u", GetProcessIdOfThread(m_hThread));
	sei.lpParameters = strParams;
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = NULL;

	//TES 3/2/2015 - PLID 64736 - Execute
	ShellExecuteEx(&sei);
}
// NexPhotoDlg.cpp : implementation file
// (c.haag 2009-08-19 12:39) - PLID 35231 - This class encapsulates the NexPhoto C# control as
// a tab for the patients module
//

#include "stdafx.h"
#include "Practice.h"
#include "NexPhotoDlg.h"
#include "NxManagedWrapper.h"
#include "RegUtils.h"
#include "MainFrm.h"
#include <NxSystemUtilitiesLib\NxConnectionUtils.h>

// CNexPhotoDlg dialog

IMPLEMENT_DYNAMIC(CNexPhotoDlg, CPatientDialog)

CNexPhotoDlg::CNexPhotoDlg(CWnd* pParent)
	: CPatientDialog(CNexPhotoDlg::IDD, pParent)
{
	m_pNxPhotoTab = NULL;
}

CNexPhotoDlg::~CNexPhotoDlg()
{
}

// (a.walling 2014-04-25 16:00) - VS2013 - We definitely don't support Win2k any longer

void CNexPhotoDlg::DoDataExchange(CDataExchange* pDX)
{
	CPatientDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNexPhotoDlg, CPatientDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_SYSTEM_NAME_UPDATED, OnSystemNameUpdated)
END_MESSAGE_MAP()


// CNexPhotoDlg message handlers

BOOL CNexPhotoDlg::OnInitDialog() 
{
	try {
		// (a.walling 2011-12-15 10:25) - PLID 40593 - Wait for NexPhoto preload thread
		extern void WaitForPreloadNexPhotoLibs();
		WaitForPreloadNexPhotoLibs();

		CPatientDialog::OnInitDialog();
		// Initialize the NexPhoto library
		// (a.walling 2011-12-15 10:12) - PLID 40593 - We've already initialized COM for this thread...
		//CoInitialize(NULL);
		// (c.haag 2015-07-08) - PLID 65912 - Only log verbose NexPhoto managed operations if this is not commented out
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Initializing the NxManagedWrapper library");
#endif
		try {
			m_pNxPhotoTab.CreateInstance(__uuidof(NxManagedWrapperLib::NxPhotoTab));
			if (NULL == m_pNxPhotoTab) 
			{
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
				Log("(NexPhoto) Failed to initialize!");
#endif
				ThrowNxException("Could not create an instance of the NexTech managed wrapper. "
					"Please ensure that Microsoft .NET Framework 3.5 or better is installed, that "
					"NxManagedWrapper.dll is registered, and that the proper NexPhoto libraries are "
					"in the same directory as the NexTech Practice executable.");
			}
			else 
			{
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
				Log("(NexPhoto) Initialization succeeded!");
#endif
			}
		}
		NxCatchAllCall(__FUNCTION__, m_pNxPhotoTab = NULL; return TRUE;);
		
		// Get the database name
		CString db;
		if(GetSubRegistryKey().IsEmpty())
			db = "PracData";
		else 
			db.Format("PracData_%s", GetSubRegistryKey());

		// Build the connection string. Since the NexPhoto tab control uses SqlConnection objects,
		// we do not need to specify network library information
		CString	strConn;
		// (b.savon 2016-05-18 14:23) - NX-100657 - Check the flag to use Windows Auth for SQL connection
		if (NxConnectionUtils::UseSqlIntegratedSecurity(GetSubRegistryKey())) {
			strConn.Format("Data Source=%s;"
				"Initial Catalog=%s;"
				"Persist Security Info=True;"
				"Language=us_english;Integrated Security=SSPI;"
				"connection timeout=%d",
				GetSqlServerName(),
				db,
				GetRemoteData()->GetConnectionTimeout());
		}
		else {
			strConn.Format("Data Source=" + GetSqlServerName() + ";"
				"Initial Catalog=" + db + ";"
				"Persist Security Info=True;"
				"Language=us_english;User ID=sa;Password=" + GetPassword() + ";"
				"connection timeout=%d",
				GetRemoteData()->GetConnectionTimeout());
		}

#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning connection string");
#endif
		m_pNxPhotoTab->ConnectionString = (LPCTSTR)strConn;

		// (s.dhole 2011-10-28 18:34) -  PLID 36107 we have to move this code up due to tab code changes 
		// Instantiate the control
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Instantiating NexPhoto tab control");
#endif
		m_pNxPhotoTab->CreateNexPhotoControl((OLE_HANDLE)GetSafeHwnd());


		// Assign the registry subkey
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning SubRegistryKey");
#endif
		m_pNxPhotoTab->SubRegistryKey = (LPCTSTR)GetSubRegistryKey();

		// (r.gonet 2016-05-24 15:54) - NX-100732 - Set the tab's system name, used in ConfigRT.
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning SystemName");
#endif
		m_pNxPhotoTab->SystemName = (LPCTSTR)g_propManager.GetSystemName();

		// (v.maida 2016-05-19 16:54) - NX-100684 - Set the stored files path for NexPhoto, which can be different from the Practice Path when in Azure RemoteApp.
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning StoredFilesPath");
#endif
		m_pNxPhotoTab->StoredFilesPath = (LPCTSTR)(g_pLicense->GetAzureRemoteApp() ? GetEnvironmentDirectory() + "\\" : GetPracPath(PracPath::ConfigRT));

		// Assign the Practice Path because we do some machine-specific ConfigRT querying
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning PracPath");
#endif
		// (j.armen 2011-10-24 17:22) - PLID 46137 - GetPracPath references ConfigRT
		m_pNxPhotoTab->PracPath = (LPCTSTR) GetPracPath(PracPath::ConfigRT);


#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning network library string");
#endif
		m_pNxPhotoTab->NetworkLibraryString = (LPCTSTR)GetNetworkLibraryString();

#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning User ID");
#endif
		m_pNxPhotoTab->UserID = GetCurrentUserID();

#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigned Username");
#endif
		m_pNxPhotoTab->Username = (LPCTSTR)GetCurrentUserName();

#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning shared path");
#endif
		m_pNxPhotoTab->SharedPath = (LPCTSTR)GetSharedPath();

		// (j.armen 2011-10-24 17:36) - PLID 46137 - GetPracPath should set log to the session directory
		CString strLogFilename = GetPracPath(PracPath::SessionPath) ^ "NexPhotoLog.log";
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Assigning log filename");
#endif
		m_pNxPhotoTab->LogFileName = (LPCTSTR)strLogFilename;


		// (c.haag 2010-03-30 17:13) - PLID 36327 - Listen for NexPhoto events
		m_EventSink.EnsureSink(m_pNxPhotoTab);

		// (c.haag 2009-12-07 15:35) - The act of instantiating the control, at least in Windows Aero theme,
		// causes the CMainFrame menu to go berserk. Every menu item will take up its own row. I don't know
		// why this happens, but this workaround seems to work well: We basically reseat the menubar and toolbars.
		// Reseating the menubar is pretty easy as you can see below. After that, we post a message to reseat the
		// toolbars because they may have moved up or down based on the weirdness of the CMainFrame menu's
		// positioning. This works for me: A. When opening the pts module, then going to NexPhoto B. Opening the
		// patients module defaulting to the NexPhoto tab C. Opening Practice to the NexPhoto tab by default.
		//
		// I'm doing this workaround in the interests of time. I would much prefer to understand what exactly is going on,
		// but the item scope is about to close. It's far superior to SetRedraw(), which only hides the problem (try pressing
		// the Alt key and then pressing the up and down arrow keys without the following code in place; you'll see what I
		// mean)
		//
		GetMainFrame()->SetMenu( GetMainFrame()->GetMenu() );
		GetMainFrame()->PostMessage(WM_COMMAND, ID_VIEW_RESETTOOLBARS);

#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) Dialog initialization complete");
#endif
	}
	NxCatchAllCall(__FUNCTION__, m_pNxPhotoTab = NULL; return TRUE;);

	return TRUE;
}

void CNexPhotoDlg::OnDestroy()
{
	try {
		CPatientDialog::OnDestroy();

		if (NULL != m_pNxPhotoTab) {
			// (c.haag 2010-03-30 17:13) - PLID 36327 - Stop listening for NexPhoto events
			m_EventSink.CleanUp();
			m_pNxPhotoTab->DestroyNexPhotoControl();
			m_pNxPhotoTab.Release();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CNexPhotoDlg::OnSize(UINT nType, int cx, int cy)
{
	try {
		CPatientDialog::OnSize(nType, cx, cy);

		if (NULL != m_pNxPhotoTab) {
			m_pNxPhotoTab->SetSize(cx,cy);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CNexPhotoDlg::SetColor(OLE_COLOR nNewColor)
{
	try {
		if (NULL != m_pNxPhotoTab) {
			// (a.walling 2010-06-16 18:11) - PLID 39087
			m_pNxPhotoTab->SetBackgroundColor(CNexTechDialog::GetSolidBackgroundRGBColor(), nNewColor);
		}
		CPatientDialog::SetColor(nNewColor);
	}
	NxCatchAll(__FUNCTION__);
}

void CNexPhotoDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		if (NULL != m_pNxPhotoTab) {
			CWaitCursor wc;
			m_pNxPhotoTab->PatientID = GetActivePatientID();
			m_pNxPhotoTab->LocationID = GetCurrentLocationID();
			m_pNxPhotoTab->UserID = GetCurrentUserID();
			m_pNxPhotoTab->Username = (LPCTSTR)GetCurrentUserName();
			m_pNxPhotoTab->UpdateView();
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-05-24 15:54) - NX-100732 - Handle when the system name changes. This is used in ConfigRT
// which NexPhoto queries for machine specific preferences.
LRESULT CNexPhotoDlg::OnSystemNameUpdated(WPARAM wParam, LPARAM lParam)
{
	try {
		if (m_pNxPhotoTab != nullptr) {
			CWaitCursor wc;
			m_pNxPhotoTab->SystemName = (LPCTSTR)g_propManager.GetSystemName();
			m_pNxPhotoTab->UpdateView();
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}
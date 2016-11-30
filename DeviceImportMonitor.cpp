#include "StdAfx.h"

#include "DeviceImportMonitor.h"
#include "DevicePlugin.h"
#include "DevicePluginUtils.h"
#include "DevicePluginImportDlg.h"
#include <CameraUtils.h>	// (j.dinatale 2011-09-09 17:43) - PLID 45420
#include "PicContainerDlg.h"
#include "DeviceConfigTabDlg.h"


// (a.walling 2014-01-22 16:06) - PLID 60271 - Device Import scans changed folders on the main thread - move this to a background thread so the UI is not blocked.

// (a.walling 2014-01-22 16:06) - PLID 60271 - DevicePluginInfo now encapsulated within a shared_ptr

// (a.walling 2013-06-10 14:48) - PLID 56225 - Device Import code all moved into DeviceImport namespace, DeviceImportMonitor.cpp/.h

namespace DeviceImport
{

static Monitor g_Monitor;

Monitor& GetMonitor()
{ return g_Monitor; }

using namespace ADODB;

static CThreadsafeLogFile LogFile;
static CLogger Logger(LogFile, CLogger::defaultOptions | CLogger::eLogTicks);

// (a.walling 2014-01-22 16:06) - PLID 60271 - changed files and patient elements are now gathered into a
// structure within the background scan thread, which is then passed to the main thread to be processed

struct FileChanges
{
	std::set<CiString> changedFiles;
	std::set<CiString> deletedFiles;

	void Add(const FileChanges& more) {
		changedFiles.insert(more.changedFiles.begin(), more.changedFiles.end());
		deletedFiles.insert(more.deletedFiles.begin(), more.deletedFiles.end());
	}
};

struct PendingPatientElement
	: private boost::noncopyable
{
	DevicePluginUtils::PatientElement* pPatient;
	CString strBarcode;
	CString strPluginName;

	explicit PendingPatientElement(DevicePluginUtils::PatientElement* pPatient = NULL, const CString& strPluginName = CString())
		: pPatient(pPatient)
		, strPluginName(strPluginName)
	{}

	~PendingPatientElement()
	{
		delete pPatient;
	}
};

struct PendingPluginChanges
{
	typedef std::map<CiString, shared_ptr<PendingPatientElement>> PatientElementMap;
	PatientElementMap patientElements;

	void AddElement(const shared_ptr<PendingPatientElement>& pPatient) {
		patientElements[pPatient->pPatient->strIdentifyingFilePath] = pPatient;
	}
};

// (a.walling 2012-12-04 16:12) - PLID 54037 - Basically this ensures a second 'lazy' timer is always fired after the original timer
namespace Timers
{
	static CMainFrame* Window()
	{
		return GetMainFrame();
	}

	static const UINT_PTR FirstID = 0x44490000; // DI(xx)
	static const UINT_PTR SecondID = 0x44490001;

	static UINT_PTR First = 0;
	static UINT_PTR Second = 0;

	bool IsTimer(UINT_PTR nIDEvent)
	{
		return nIDEvent >= FirstID && nIDEvent <= SecondID;
	}

	void Trigger();

	///

	void KillAll()
	{
		if (First) {
			Window()->KillTimer(FirstID);
			First = 0;
		}
		if (Second) {
			Window()->KillTimer(SecondID);
			Second = 0;
		}
	}

	bool Kill(UINT_PTR nIDEvent)
	{
		switch (nIDEvent)
		{
		case FirstID:
			if (First) {
				Window()->KillTimer(FirstID);
				First = 0;
			}
			return true;
		case SecondID:
			if (Second) {
				Window()->KillTimer(SecondID);
				Second = 0;
			}
			return true;
		}

		return false;
	}

	void Trigger()
	{
		if (!First) {
			KillAll();
			First = Window()->SetTimer(FirstID, 1000, NULL);
		}
	}

	void StartDelayed()
	{
		if (First) {
			return;
		}
		if (!Second) {
			Second = Window()->SetTimer(SecondID, 3000, NULL);
		}
	}

	static UINT_PTR CurrentEvent = 0;

	bool OnTimer(UINT_PTR nIDEvent)
	{
		if (!IsTimer(nIDEvent)) return false;

		// prevent reentrance!!
		if (CurrentEvent) {
			Logger.Log("Reentrant DeviceImport::Timers::OnTimer(%lu) already handling (%lu)", nIDEvent - FirstID, CurrentEvent - FirstID);
			KillAll();
			Trigger();
			return true;
		}

		CurrentEvent = nIDEvent;

		if (FirstID == nIDEvent) {
			KillAll();
		} else {
			Kill(nIDEvent);
		}

		try {
			GetMonitor().ProcessChangedPlugins();
		} catch (...) {
			if (FirstID == nIDEvent) {
				StartDelayed();
			}
			CurrentEvent = 0;
			throw;
		}
		
		if (FirstID == nIDEvent) {
			StartDelayed();
		}
		CurrentEvent = 0;

		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

Monitor::Monitor()
{	
	// (d.lange 2010-06-07 23:45) - PLID 38850 - Make DevicePluginImportDlg modeless
	m_pDevicePluginImportDlg = NULL;

	// (c.haag 2010-06-30 15:27) - PLID 39424 - Device import license checking
	m_bIsDeviceImportLicensed = FALSE;
	m_bWasDeviceImportLicenseChecked = FALSE;
	m_bFromLogin = FALSE;
}

Monitor::~Monitor()
{	
	CleanupPluginProcessThreads();
}

void Monitor::Init()
{	
	LogFile.SetLog(GetPracPath(PracPath::SessionPath) ^ "DeviceImport.log");
}

void Monitor::Close()
{
	// (a.walling 2012-12-04 16:12) - PLID 54037 - Device import notification tray icon
	Timers::KillAll();
	m_trayDeviceImport.Remove();
}

bool Monitor::HandleTimer(UINT nIDEvent)
{
	// (a.walling 2012-12-04 16:12) - PLID 54037 - Check device import timers
	return Timers::IsTimer(nIDEvent) && Timers::OnTimer(nIDEvent);
}

void Monitor::TriggerFolderChanged()
{
	Timers::Trigger();
}

void Monitor::TriggerReadyToImport()
{
	ProcessPending();
}

// (r.farnworth 2014-10-01 09:53) - PLID 63378 - The preference "Notify me when the device import screen has files already in the list when I log in"  does not work
void Monitor::TriggerOnLogin()
{
	ProcessPending(true);
}

// (d.lange 2010-06-07 23:52) - PLID 38850 - We will show the DevicePluginImportDlg when the folder monitoring is triggered
void Monitor::ShowDevicePluginImportDlg()
{
	try {
		// (b.savon 2011-09-23 12:21) - PLID 44954 - Don't allow the user to open the plugin import dlg
		//										     until the plugin threads are completed.
		if( !IsPluginProcessComplete() ){
			GetMainFrame()->MessageBox("Please wait until the device plugin files are done processing.", 
					   "Device Plugin Files Processing...", MB_ICONWARNING);
			return;
		}

		// (c.haag 2010-06-30 15:27) - PLID 39424 - Added license checking
		if (!m_bWasDeviceImportLicenseChecked) {
			m_bIsDeviceImportLicensed = g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrUse);
			m_bWasDeviceImportLicenseChecked = TRUE;
		}
		if (!m_bIsDeviceImportLicensed) {
			// Our license system has no way of determining "used up" versus "don't have the license". 
			// Therefore we have no good option on which status to check when launching folder monitoring.
			// Once expired, we will warn the user once per attempted-file-import, per Practice session. 
			// Mildly annoying, but required without some license-side rewriting.

			// When we know we can't import, shut down the folder monitor each time. Why each time? 
			// Cause various things in Practice can attempt to restart it. Technically since you warn about
			// the license once per session, it causes no visible difference to the user.
			AfxMessageBox("You are not licensed to access the Device Importing feature. Please contact NexTech Systems for assistance.", MB_OK | MB_ICONINFORMATION);
			CloseAllDeviceFolderMonitors();
			return;
		}

		EnsureImportDlg();

		m_pDevicePluginImportDlg->ShowWindow(SW_SHOW);

		// Bring the window up if it's minimized
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (m_pDevicePluginImportDlg->GetWindowPlacement(&wp)) {
			if (m_pDevicePluginImportDlg->IsIconic()) {
				if (wp.flags & WPF_RESTORETOMAXIMIZED) {
					wp.showCmd = SW_MAXIMIZE;
				} else {
					wp.showCmd = SW_RESTORE;
				}
				m_pDevicePluginImportDlg->SetWindowPlacement(&wp);
			}
		}

		// Bring the window to the foreground so the user is sure to see it
		m_pDevicePluginImportDlg->SetForegroundWindow();

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-06-08 09:10) - PLID 38850 - Clean up the DevicePluginImportDlg dialog when finished
void Monitor::CleanUpDevicePluginImportDlg()
{
	try {
		if(m_pDevicePluginImportDlg) {
			if(m_pDevicePluginImportDlg->m_hWnd) {
				m_pDevicePluginImportDlg->DestroyWindow();
			}
		delete m_pDevicePluginImportDlg;
		m_pDevicePluginImportDlg = NULL;
	}
	} NxCatchAll(__FUNCTION__);
}


// (j.jones 2010-06-01 09:51) - PLID 37976 - check whether we should
// be monitoring ophthalmology device folders from available plugins
// (r.farnworth 2014-10-02 11:49) - PLID 63378 - Added bFromLogin
void Monitor::TryMonitorDevicePluginFolders(bool bFromLogin)
{
	try {
		//close all currently scanning plugins
		{
			boost::container::flat_set<shared_ptr<const DevicePluginInfo>> monitoredDevicePlugins = m_monitoredDevicePlugins;
			for each (shared_ptr<const DevicePluginInfo> pPluginInfo in monitoredDevicePlugins) {
				CloseDeviceFolderMonitor(pPluginInfo->nDevicePluginID);
			}
			
			m_monitoredDevicePlugins.clear();
		}

		// (c.haag 2010-06-30 15:32) - PLID 39424 - Do nothing if the license isn't there
		if (!g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) {
			return;
		}
		// Quit if we checked the license and denied usage
		if (m_bWasDeviceImportLicenseChecked && !m_bIsDeviceImportLicensed) {
			return;
		}

		m_bFromLogin = bFromLogin;

		// (b.savon 2012-02-13 11:31) - PLID 46456 - Begin watching folders
		QueryAndBeginWatch();


	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-01 10:24) - PLID 37976 - closes all open folder monitors
void Monitor::CloseAllDeviceFolderMonitors()
{
	try {

		// (j.jones 2011-03-15 14:20) - PLID 42738 - remove all notifications
		GetMainFrame()->UnNotifyUser(NT_DEVICE_IMPORT);

		boost::container::flat_set<shared_ptr<const DevicePluginInfo>> monitoredDevicePlugins = m_monitoredDevicePlugins;
		for each (shared_ptr<const DevicePluginInfo> pPluginInfo in monitoredDevicePlugins) {

			CFolderMonitor *pFolderMonitor = NULL;
			if(m_mapPluginIDsToFolderMonitors.Lookup(pPluginInfo->nDevicePluginID, pFolderMonitor) && pFolderMonitor != NULL) {
				pFolderMonitor->StopWatchingAllDirectories();
				delete pFolderMonitor;
				pFolderMonitor = NULL;
				m_mapPluginIDsToFolderMonitors.RemoveKey(pPluginInfo->nDevicePluginID);
			}

			CFolderMonitorChangeHandler_Array *paryChanges = NULL;
			if(m_mapPluginIDsToFolderMonitorChangedArrays.Lookup(pPluginInfo->nDevicePluginID, paryChanges) && paryChanges != NULL) {
				// (a.walling 2012-12-04 13:12) - PLID 54027 - This is unnecessary; we are deleting the object anyway
				//paryChanges->m_aryChangedFiles.RemoveAll();
				//// (j.jones 2010-10-25 10:50) - PLID 41008 - also empty the deleted list
				//paryChanges->m_aryDeletedFiles.RemoveAll();
				delete paryChanges;
				paryChanges = NULL;

				m_mapPluginIDsToFolderMonitorChangedArrays.RemoveKey(pPluginInfo->nDevicePluginID);
			}
		}

		m_monitoredDevicePlugins.clear();

	}NxCatchAll(__FUNCTION__);
}

// (d.lange 2011-03-11 11:01) - PLID 41010 - Refresh the device import dialog if the current patient filter is enabled
void Monitor::NotifyDeviceImportPatientChanged()
{
	try {
		//ensure the dialog has been created
		if(m_pDevicePluginImportDlg) {
			if(m_pDevicePluginImportDlg->IsWindowVisible()) {
				if(GetRemotePropertyInt("DeviceImport_ChkCurrentPatientFilter", 0, 0, GetCurrentUserName(), true)) {
					m_pDevicePluginImportDlg->Refresh();
				}
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-08-25 15:02) - PLID 41683 - clear all records for a given plugin,
// provided the import dialog exists
void Monitor::TryRemoveRecordsForPlugin(CString strPluginName)
{
	try {

		//only do this if the dialog exists
		if(m_pDevicePluginImportDlg) {
			if(m_pDevicePluginImportDlg->GetSafeHwnd()) {
				m_pDevicePluginImportDlg->RemoveRecordsForPlugin(strPluginName);
			}
		}

	} NxCatchAll(__FUNCTION__);
}


// (j.jones 2010-06-01 11:41) - PLID 37976 - added ability to monitor device plugin folders
// (j.jones 2010-11-02 14:51) - PLID 41189 - added DevicePatientMatchRule
// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
// (j.jones 2011-03-10 15:41) - PLID 41349 - added NotificationRule
// (b.savon 2011-09-26 11:41) - PLID 44954 - added bNewPlugin
// (r.gonet 06/11/2013) - PLID 56370 - Added Image DPI
// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
void Monitor::StartWatchingDeviceFolder(long nDevicePluginID, CString strPluginFileName, CString strPluginOverrideName, CString strDeviceExportPath,
										   EDeviceNotificationRule eNotificationRule, EDevicePatientMatchRule eDevicePatientMatchRule,
										   EConvertPDFToImage eConvertPDFToImage, long nImageDPI, EPDFToImageConverter eConverter, BOOL bNewPlugin /* = FALSE */)
{
	try {

		//stop watching if it is watching anything

		CloseDeviceFolderMonitor(nDevicePluginID);

		//now start watching the folder
		CFolderMonitor *pFolderMonitor = new CFolderMonitor();

		shared_ptr<DevicePluginInfo> pPluginInfo(new DevicePluginInfo);
		pPluginInfo->nDevicePluginID = nDevicePluginID;
		pPluginInfo->strPluginFileName = strPluginFileName;
		pPluginInfo->strPluginOverrideName = strPluginOverrideName;
		pPluginInfo->strDeviceExportPath = strDeviceExportPath;
		// (j.jones 2011-03-10 15:47) - PLID 41349 - added NotificationRule
		pPluginInfo->eNotificationRule = eNotificationRule;
		// (j.jones 2010-11-02 14:51) - PLID 41189 - added DevicePatientMatchRule
		pPluginInfo->eDevicePatientMatchRule = eDevicePatientMatchRule;
		// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
		pPluginInfo->eConvertPDFToImage = eConvertPDFToImage;
		// (r.gonet 06/11/2013) - PLID 56370 - Initialize the Image DPI
		pPluginInfo->nImageDPI = nImageDPI;
		// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
		pPluginInfo->eConverter = eConverter;

		// (b.savon 2011-09-26 11:41) - PLID 44954 - If we process the plugin while Practice is already
		//											 open use method.  If we process the plugin when Practice
		//											 starts, use the thread so we can keep the GUI active.
		if( bNewPlugin ){
			// (j.jones 2010-10-22 10:52) - PLID 41069 - ProcessPluginFolder will import
			// all files in the given plugin's folder, but not pop up the device import dialog.
			// Call this now so that the device import dialog loads up all the files currently
			// in the watched folder, as the folder monitor itself only looks for changed files.
			ProcessPluginFolder(pPluginInfo);
		} else{
			{

				// (a.walling 2014-01-22 16:10) - PLID 60437 - Device Import may have begun a scan on startup, though in a different thread, in a thread-unsafe way that led to crashes and other undefined behavior. This has been easily resolved by just triggering the plugin changed event with the new architecture in 60271.

				CSingleLock lock(&m_csPluginChanges, TRUE);
				m_mapPluginFileChanges[pPluginInfo].changedFiles.insert(FormatString("\\\\?\\InitialScan_%li", ::GetTickCount()));
			}
			::SetEvent(m_pluginChangedEvent);
		}

		m_monitoredDevicePlugins.insert(pPluginInfo);

		m_mapPluginIDsToFolderMonitors.SetAt(nDevicePluginID, pFolderMonitor);
		
		//if paryChanges->Changed() has its contents changed, the
		//NXM_DEVICE_PLUGIN_FOLDER_CHANGED message will be sent back to CMainFrame
		// (j.jones 2010-10-25 10:53) - PLID 41008 - we are also notified if paryChanges->m_aryDeletedFiles changed
		CFolderMonitorChangeHandler_Array *paryChanges = new CFolderMonitorChangeHandler_Array(GetMainFrame()->GetSafeHwnd(), NXM_DEVICE_PLUGIN_FOLDER_CHANGED, (WPARAM)nDevicePluginID);
		m_mapPluginIDsToFolderMonitorChangedArrays.SetAt(nDevicePluginID, paryChanges);

		// (b.savon 2011-8-31) - PLID 45288 - Device plugins should have the ability to search recursively
		//through the monitored directory.  Set the flag.
		CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";
		BOOL bSupportsRecursion = FALSE;
		{
			scoped_ptr<CDevicePlugin> pPlugin(new CDevicePlugin(strPath, strPluginFileName));
			bSupportsRecursion = pPlugin->m_cDevicePluginSettings.SupportsRecursion();
		}
		// (a.walling 2012-12-04 13:57) - PLID 54027 - Also watch for file size changes and last write changes
		// Note that these are not entirely real time; they usually occur when the filesystem periodically flushes a file, or when the last open handle closes.
		DWORD dwChangesToWatchFor = FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_CREATION|FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_SIZE;
		
		if (bSupportsRecursion) {
			dwChangesToWatchFor |= FILE_NOTIFY_CHANGE_DIR_NAME;
		}

		DWORD dwWatch = pFolderMonitor->WatchDirectory(strDeviceExportPath, 
														dwChangesToWatchFor,
														paryChanges,
														bSupportsRecursion);

		if(dwWatch != ERROR_SUCCESS) {
			CString strError;
			strError.Format("Failed to start watching folder %s for the %s plugin. With Error Code %d", strDeviceExportPath, strPluginFileName, dwWatch);
			GetMainFrame()->MessageBox(strError, "Practice", MB_OK|MB_ICONEXCLAMATION);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-01 11:42) - PLID 37976 - added ability to monitor device plugin folders
void Monitor::CloseDeviceFolderMonitor(long nDevicePluginID)
{
	try {

		//do not kill any active timer

		CFolderMonitor *pFolderMonitor = NULL;
		if(m_mapPluginIDsToFolderMonitors.Lookup(nDevicePluginID, pFolderMonitor) && pFolderMonitor != NULL) {
			pFolderMonitor->StopWatchingAllDirectories();
			delete pFolderMonitor;
			pFolderMonitor = NULL;
			m_mapPluginIDsToFolderMonitors.RemoveKey(nDevicePluginID);
		}

		CFolderMonitorChangeHandler_Array *paryChanges = NULL;
		if(m_mapPluginIDsToFolderMonitorChangedArrays.Lookup(nDevicePluginID, paryChanges) && paryChanges != NULL) {
			// (a.walling 2012-12-04 13:12) - PLID 54027 - This is unnecessary; we are deleting the object anyway
			//paryChanges->m_aryChangedFiles.RemoveAll();
			//// (j.jones 2010-10-25 10:50) - PLID 41008 - also empty the deleted list
			//paryChanges->m_aryDeletedFiles.RemoveAll();
			delete paryChanges;
			paryChanges = NULL;

			m_mapPluginIDsToFolderMonitorChangedArrays.RemoveKey(nDevicePluginID);
		}
		
		boost::container::flat_set<shared_ptr<const DevicePluginInfo>> monitoredDevicePlugins = m_monitoredDevicePlugins;
		for each(shared_ptr<const DevicePluginInfo> pPluginInfo in monitoredDevicePlugins) {
			if(pPluginInfo->nDevicePluginID == nDevicePluginID) {
				m_monitoredDevicePlugins.erase(pPluginInfo);
			}
		}

		// (j.jones 2011-03-15 14:41) - PLID 42738 - if there are no files left,
		// remove any notifications that might exist
		if(GetDeviceImportRecordCount() == 0) {
			GetMainFrame()->UnNotifyUser(NT_DEVICE_IMPORT);
		}

	}NxCatchAll(__FUNCTION__);
}

void Monitor::ProcessChangedPlugins()
{
	bool bShouldNotify = false;
	try {
		for each (shared_ptr<const DevicePluginInfo> pPluginInfo in m_monitoredDevicePlugins) {
			CFolderMonitorChangeHandler_Array *paryChanges = NULL;
			if(m_mapPluginIDsToFolderMonitorChangedArrays.Lookup(pPluginInfo->nDevicePluginID, paryChanges) && paryChanges != NULL) {

				// (a.walling 2012-12-04 13:12) - PLID 54027 - Since it is entirely possible a window message (and hence notification) may be pumped within this block
				// (eg via a message box or modal loop or even an RPC / COM call) we are simply swapping out the changed files rather than deleting at the end.
				// (a.walling 2014-01-22 16:12) - PLID 60271 - Gather change info for later processing

				FileChanges changeInfo;

				swap(changeInfo.changedFiles, paryChanges->Changed());
				swap(changeInfo.deletedFiles, paryChanges->Deleted());
				
				//first process changed files
				if(!changeInfo.changedFiles.empty() || !changeInfo.deletedFiles.empty()) {
					
					DeviceImport::Logger.Log("Plugin %s notified re path %s", pPluginInfo->strPluginFileName, pPluginInfo->strDeviceExportPath);
					
					foreach (const CiString& strFile, changeInfo.changedFiles) {
						DeviceImport::Logger.Log("Changed file: \t%s", strFile);
					}
					foreach (const CiString& strFile, changeInfo.deletedFiles) {
						DeviceImport::Logger.Log("Deleted file: \t%s", strFile);
					}

					{
						CSingleLock lock(&m_csPluginChanges, TRUE);

						m_mapPluginFileChanges[pPluginInfo].Add(changeInfo);
					}

					bShouldNotify = true;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);

	if (bShouldNotify) {
		::SetEvent(m_pluginChangedEvent);
	}
}

namespace {
	struct SignalEventOnScopeExit
	{
		SignalEventOnScopeExit(HANDLE hEvent)
			: hEvent(hEvent)
		{}

		~SignalEventOnScopeExit()
		{
			::SetEvent(hEvent);
		}

		HANDLE hEvent;
	};
}

// (r.farnworth 2014-10-02 11:49) - PLID 63378 - Added loginMsg
void Monitor::RunWorker(bool onlyUniquePaths, HWND notifyHwnd, UINT notifyMsg, WPARAM notifyWParam, LPARAM notifyLParam, UINT loginMsg)
{
	::OleInitialize(NULL);

	// (a.walling 2014-03-11 09:23) - PLID 61312 - ensure the idle event is set when leaving this scope
	SignalEventOnScopeExit signalIdleOnScopeExit(m_workerIdleEvent);

	PendingPluginChangesMap incompletePluginChanges;

	// (a.walling 2014-01-22 16:16) - PLID 60438 - DeviceImport - Speed up scan by ignoring already-processed paths
	typedef boost::container::flat_set<CiString> UniquePathSet;
	typedef boost::container::flat_map<weak_ptr<const DevicePluginInfo>, UniquePathSet> UniquePathByPluginMap;

	UniquePathByPluginMap uniquePathsByPlugin;

	DWORD dwWaitTime = INFINITE;
	for (;;) {

		::SetEvent(m_workerIdleEvent);
		DWORD ret = NxThread::This()->MsgWaitForSingleObjectOrInterruptNoThrow(m_pluginChangedEvent, dwWaitTime, false);

		if (ret != WAIT_OBJECT_0 && ret != WAIT_TIMEOUT) {
			return;
		}

		// (a.walling 2014-03-11 09:23) - PLID 61312 - set not-idle after checking the return of the wait
		::ResetEvent(m_workerIdleEvent);

		// (a.walling 2014-03-11 09:23) - PLID 61312 - ensure the idle event is set when leaving this scope
		SignalEventOnScopeExit signalIdleOnLoopExit(m_workerIdleEvent);

		// process
		PendingPluginChangesMap pendingPluginChanges;
		std::set<CiString> allDeletedFiles;

		int skipped = 0;

		// ensure we handle any lingering incomplete changes, since we may have awoken due 
		// to timeout rather than a new change notification
		swap(pendingPluginChanges, incompletePluginChanges);

		PluginFileChangesMap pluginFileChanges;
		{
			CSingleLock lock(&m_csPluginChanges, TRUE);
			swap(pluginFileChanges, m_mapPluginFileChanges);
		}
		
		CString strBasePath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";

		for each (const PluginFileChangesMap::value_type& entry in pluginFileChanges) {
			shared_ptr<const DevicePluginInfo> pPluginInfo = entry.first.lock();
			if (!pPluginInfo) {
				continue;
			}

			const FileChanges& changes = entry.second;
			PendingPluginChanges& pendingChanges = pendingPluginChanges[pPluginInfo];
			UniquePathSet& uniquePaths = uniquePathsByPlugin[pPluginInfo];

			try {
				allDeletedFiles.insert(changes.deletedFiles.begin(), changes.deletedFiles.end());
				
				if(changes.changedFiles.empty()) {
					continue;
				}

				if (pPluginInfo->strPluginFileName.IsEmpty()) {
					continue;
				}

				DeviceImport::Logger.Log("Plugin %s processing path %s", pPluginInfo->strPluginFileName, pPluginInfo->strDeviceExportPath);
				
				CString strPluginPath = strBasePath ^ pPluginInfo->strPluginFileName;
				if(!DoesExist(strPluginPath)) {
					Logger.Log(__FUNCTION__" - Failed to load device plugin %s because the plugin file does not exist.", pPluginInfo->strPluginFileName);
					continue;
				}
				scoped_ptr<CDevicePlugin> pPlugin(new CDevicePlugin(strBasePath, pPluginInfo->strPluginFileName));
	
				MSXML2::IXMLDOMDocument2Ptr Doc(__uuidof(MSXML2::DOMDocument60));
				CString strErrorDesc;
				BOOL bSuccess = pPlugin->Process(pPluginInfo->strDeviceExportPath, Doc, strErrorDesc);

				if (!bSuccess) {					
					CString strMessage;
					strMessage.Format("Failed to process folder %s with the device plugin %s.\n\n"
						"%s", pPluginInfo->strDeviceExportPath, pPluginInfo->strPluginFileName, strErrorDesc);
					Logger.Log(__FUNCTION__" - %s", strMessage);
					continue;
				}
				
				DeviceImport::Logger.Log("Process complete");

				scoped_ptr<DevicePluginUtils::DeviceResult> pDeviceResult(DevicePluginUtils::GenerateDeviceResultsFromXML(Doc));
				
				CString strPluginName = pDeviceResult->strPluginFullDesc;
				if(!pPluginInfo->strPluginOverrideName.IsEmpty()) {
					strPluginName = pPluginInfo->strPluginOverrideName;
				}

				DeviceImport::Logger.Log("Found %li files", pDeviceResult->aryPatients.GetSize());

				for (int i = 0; i < pDeviceResult->aryPatients.GetSize(); ++i) {
					DevicePluginUtils::PatientElement *pPatient = pDeviceResult->aryPatients.GetAt(i);

					if (!pPatient) {
						continue;
					}

					// (a.walling 2014-01-22 16:16) - PLID 60438 - DeviceImport - Speed up scan by ignoring already-processed paths
					if (onlyUniquePaths) {
						if (uniquePaths.count(pPatient->strIdentifyingFilePath)) {
							++skipped;
							continue;
						} else {
							DeviceImport::Logger.Log("New file `%s`", pPatient->strIdentifyingFilePath);
						}
					}

					pendingChanges.AddElement(shared_ptr<PendingPatientElement>(new PendingPatientElement(pPatient, strPluginName)));

					pDeviceResult->aryPatients.SetAt(i, NULL);
				}

			} catch(...) {
				Logger.Log(__FUNCTION__ " - Error processing plugin %s: %s", pPluginInfo->strPluginFileName, NxCatch::GetErrorMessage());
			}
		}

		PendingPluginChangesMap readyPluginChanges;

		for each (const PendingPluginChangesMap::value_type& entry in pendingPluginChanges) {
			shared_ptr<const DevicePluginInfo> pPluginInfo = entry.first.lock();
			if (!pPluginInfo) {
				continue;
			}

			const PendingPluginChanges& pendingChanges = entry.second;
			UniquePathSet& uniquePaths = uniquePathsByPlugin[pPluginInfo];

			for each (const PendingPluginChanges::PatientElementMap::value_type& elem in pendingChanges.patientElements) {
				const shared_ptr<PendingPatientElement>& pPendingPatient = elem.second;
				const DevicePluginUtils::PatientElement* pPatient = pPendingPatient->pPatient;

				if (!pPatient) {
					continue;
				}

				if (!FileUtils::DoesFileOrStreamExist(pPatient->strIdentifyingFilePath)) {
					Logger.Log("Non-existent file `%s`", pPatient->strIdentifyingFilePath);
					continue;
				}
				
				// (a.walling 2012-12-04 16:24) - PLID 54041 - If the file is incomplete, skip it!
				if (!ValidateCompleteFile(pPatient->strIdentifyingFilePath)) {
					Logger.Log("Truncated or invalid file `%s`", pPatient->strIdentifyingFilePath);

					incompletePluginChanges[pPluginInfo].AddElement(pPendingPatient);
					continue;
				}

				// (a.walling 2014-01-22 16:16) - PLID 60438 - DeviceImport - Speed up scan by ignoring already-processed paths
				if (onlyUniquePaths) {
					uniquePaths.insert(pPatient->strIdentifyingFilePath);
				}

				pPendingPatient->strBarcode = GetImageBarcode(pPatient->strIdentifyingFilePath);
				if (!pPendingPatient->strBarcode.IsEmpty()) {
					Logger.Log("Found barcode `%s` for image `%s`", pPendingPatient->strBarcode, pPatient->strIdentifyingFilePath);
				}

				readyPluginChanges[pPluginInfo].AddElement(pPendingPatient);
			}
		}

		bool bNotify = false;
		if (!allDeletedFiles.empty() || !readyPluginChanges.empty()) {
			bNotify = true;
		}

		{
			{
				CSingleLock lock(&m_csPluginChanges, TRUE);

				// broadcast allDeletedFiles to m_setDeletedFiles
				m_setDeletedFiles.insert(allDeletedFiles.begin(), allDeletedFiles.end());

				// broadcast readyPluginChanges to m_mapPendingPluginChanges
				for each (const PendingPluginChangesMap::value_type& entry in readyPluginChanges) {
					shared_ptr<const DevicePluginInfo> pPluginInfo = entry.first.lock();
					if (!pPluginInfo) {
						continue;
					}

					const PendingPluginChanges& readyChanges = entry.second;

					PendingPluginChanges& destChanges = m_mapPendingPluginChanges[pPluginInfo];

					for each (const PendingPluginChanges::PatientElementMap::value_type& elem in readyChanges.patientElements) {
						destChanges.AddElement(elem.second);
					}
				}
			}
		}

		if (!incompletePluginChanges.empty()) {
			dwWaitTime = 5000;
			Logger.Log("Incomplete files -- retrying in at least %lu ms", dwWaitTime);
		} else {
			dwWaitTime = INFINITE;
		}

		if (bNotify) {
			Logger.Log("Notifying!");
			// (r.farnworth 2014-10-02 11:49) - PLID 63378 - use m_bFromLogin to determine what message to post
			if (!m_bFromLogin) {
				::PostMessage(notifyHwnd, notifyMsg, notifyWParam, notifyLParam);
			}
			else {
				::PostMessage(notifyHwnd, loginMsg, notifyWParam, notifyLParam);
				m_bFromLogin = false;
			}
		}

		if (skipped) {
			Logger.Log("Skipped %li duplicate files!", skipped);
		}
	}

	::SetEvent(m_workerIdleEvent);
}

// (j.jones 2010-06-03 09:42) - PLID 37976 - used to actually run the device plugins on the folders that changed
// (r.farnworth 2014-10-02 11:52) - PLID 63378 - added bOnLogin
void Monitor::ProcessPending(bool bOnLogin /* = false */)
{
	try {

		PendingPluginChangesMap pendingPluginChanges;
		std::set<CiString> deletedFiles;
		{
			// (a.walling 2014-01-22 16:12) - PLID 60271 - Grab all change info from the background thread
			CSingleLock lock(&m_csPluginChanges, TRUE);
			swap(m_mapPendingPluginChanges, pendingPluginChanges);
			swap(m_setDeletedFiles, deletedFiles);
		}

		BOOL bAlreadyDisplayedDeviceImportDlg = FALSE;

		// (j.jones 2010-10-25 16:59) - PLID 41068 - supported the preference to not auto-popup the
		// device import dialog when changes have been made
		// (j.jones 2011-03-10 15:50) - PLID 41349 - removed this preference, it is now a setting per plugin
		//BOOL bAutoPopup = GetRemotePropertyInt("DeviceImport_AutoPopupWithChangedFiles", 1, 0, GetCurrentUserName(), true);

		int nProcessed = 0;
		int nDeleted = 0;
		CString strNotifications;
		//process all devices that have changed files, send the results to the import dialog
		for each (const PendingPluginChangesMap::value_type& pluginElem in pendingPluginChanges) {
			shared_ptr<const DevicePluginInfo> pPluginInfo = pluginElem.first.lock();
			if (!pPluginInfo) {
				continue;
			}
			const PendingPluginChanges& pendingChanges = pluginElem.second;

			for each (const PendingPluginChanges::PatientElementMap::value_type& changeElem in pendingChanges.patientElements) {

				shared_ptr<PendingPatientElement> pPendingPatient = changeElem.second;
				DevicePluginUtils::PatientElement* pPatient = pPendingPatient->pPatient;

				if (!pPatient) {
					continue;
				}

				if (ProcessImageBarcode(pPatient->strIdentifyingFilePath, pPendingPatient->strBarcode, pPendingPatient->strPluginName)) {
					// (a.walling 2012-12-04 16:12) - PLID 54037 - Device import notification tray icon
					strNotifications.AppendFormat("Imported barcoded file %s!\r\n", pPatient->strIdentifyingFilePath);
					++nProcessed;

					continue;
				}

				//ensure the dialog is created (don't show it yet)
				EnsureImportDlg();

				// (j.jones 2010-11-02 16:04) - PLID 41189 - pass in eDevicePatientMatchRule
				// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
				//(e.lally 2011-04-21) PLID 43372 added strPluginFileName
				// (r.gonet 06/11/2013) - PLID 56370 - Pass the Image DPI
				// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
				if(m_pDevicePluginImportDlg->AddNewPatientElement(pPatient, pPendingPatient->strPluginName, pPluginInfo->strPluginFileName, pPluginInfo->eDevicePatientMatchRule, pPluginInfo->eConvertPDFToImage, pPluginInfo->nImageDPI, pPluginInfo->eConverter)) {
					++nProcessed;
					// (a.walling 2012-12-04 16:12) - PLID 54037 - Don't notify with the tray icon in this situation
					//strNotifications.AppendFormat("Imported file %s!\r\n", pPatient->strIdentifyingFilePath);

					//if AddNewPatientElement returned TRUE, we added this data to the dialog

					//we now no longer own the pointer
					pPendingPatient->pPatient = NULL;
					pPendingPatient.reset();

					// (j.jones 2010-10-25 16:39) - PLID 41098 - Display the device
					//import dialog if it isn't shown yet. We only neeed to call this
					//function one time per process, but we call it here to show it
					//as soon as possible, incase the processing takes a while.
					// (j.jones 2010-10-25 16:58) - PLID 41068 - follow the preference 
					// as to whether to auto-popup
					// (d.lange 2011-03-10 09:58) - PLID 41010 - Before we show the dialog, we should check whether the 
					// "Show only files for the current patient" checkbox is checked and if so make sure we are looking at the current patient
					// (j.jones 2011-03-10 15:51) - PLID 41329 - the auto-popup ability is now a property of the plugin,
					// instead of a preference
					// (r.farnworth 2014-10-01 10:01) - PLID 63378 - If we are coming from Login, check the preference settings rather than the dropdown menu
					if (!bAlreadyDisplayedDeviceImportDlg && pPluginInfo->eNotificationRule == dpnrAutoPopup && !bOnLogin) {
						if(!m_pDevicePluginImportDlg->IsDlgButtonChecked(IDC_CHK_CURPATIENT_FILTER)) {
							ShowDevicePluginImportDlg();
							bAlreadyDisplayedDeviceImportDlg = TRUE;
						}else if(m_pDevicePluginImportDlg->IsDlgButtonChecked(IDC_CHK_CURPATIENT_FILTER)) {
							CArray<long,long> aryOpenEMRPatientIDs;
							GetMainFrame()->GetOpenEMRPatientIDs(aryOpenEMRPatientIDs);

							long nPatientID = (pPatient->strPatientID.IsEmpty() ? -1 : atoi(pPatient->strPatientID));
							if(m_pDevicePluginImportDlg->IsCurrentPatient(nPatientID, aryOpenEMRPatientIDs)) {
								ShowDevicePluginImportDlg();
								bAlreadyDisplayedDeviceImportDlg = TRUE;
							}
						}
					}
					// (j.jones 2011-03-15 14:35) - PLID 42738 - supported notifying the user,
					// only if the dialog is not displayed
					else if(bOnLogin ? GetRemotePropertyInt("DeviceImport_NotifyOnStartup", 1, 0, GetCurrentUserName(), true) == 1 : pPluginInfo->eNotificationRule == dpnrNotifyUser) {

						if(!m_pDevicePluginImportDlg->IsWindowVisible() && m_pDevicePluginImportDlg->GetDeviceImportRecordCount() > 0) {
							//notify the user that files are ready to import
							GetMainFrame()->NotifyUser(NT_DEVICE_IMPORT, "", false);
						}
					}
				} else {
					Logger.Log("Could not add new patient element for file `%s`", pPatient->strIdentifyingFilePath);
				}
			}
		}
		
		// (j.jones 2010-10-25 10:50) - PLID 41008 - also process deleted files					
		for each (const CiString& deletedFile in deletedFiles) {
			//ensure the dialog is created (don't show it yet)
			EnsureImportDlg();

			if(m_pDevicePluginImportDlg->HandleDeletedFile(deletedFile)) {
				// yay?
				++nProcessed;
				++nDeleted;
			}
		}

		// (a.walling 2012-12-04 16:12) - PLID 54037 - Device import notification tray icon
		if (!strNotifications.IsEmpty()) {
			BOOL bUseTrayIcon = GetPropertyInt("DeviceImport_UseTrayIcon", 1, 0, true);
			if (bUseTrayIcon) {
				m_trayDeviceImport.Notify(strNotifications);
			}
		}

		// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list
		//									 per-device plugin setup.  There has been a file added.  Let's
		//									 make sure it doesn't show up in the import dialog if it isn't
		//									 the current active filter.
		// (a.walling 2012-12-04 16:12) - PLID 54037 - Only refresh if we processed something; otherwise this causes focus issues
		if (nProcessed) {
			m_pDevicePluginImportDlg->Refresh();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-10-22 10:52) - PLID 41069 - ProcessPluginFolder will import
// all files in the given plugin's folder, but not pop up the device import dialog
void Monitor::ProcessPluginFolder(shared_ptr<const DevicePluginInfo> pPluginInfo)
{
	try {
		if(!pPluginInfo) {
			return;
		}

		//first check that the watched path exists
		if(!DoesExist(pPluginInfo->strDeviceExportPath)) {
			return;
		}

		// (j.jones 2011-03-15 16:05) - PLID 42850 - if the device plugin is not displayed,
		// and gains files due to processing this folder, we may notify the user that
		// files are ready to import
		long nPreviousFileCount = 0;
		if(m_pDevicePluginImportDlg && m_pDevicePluginImportDlg->GetSafeHwnd()
			&& !m_pDevicePluginImportDlg->IsWindowVisible()) {

			nPreviousFileCount = m_pDevicePluginImportDlg->GetDeviceImportRecordCount();
		}

		if(m_pDevicePluginImportDlg && m_pDevicePluginImportDlg->GetSafeHwnd() && pPluginInfo != NULL){
			//(e.lally 2011-04-21) PLID 43372 - Show the button to combine pdfs if we have a generic file import .nxdp file in use
			// (b.savon 2011-9-14) PLID 45491 - Give the ability for PDFs in Generic Recursive File Imports to be combined.
			if(pPluginInfo->strPluginFileName.CompareNoCase("GenericFileImport.nxdp") == 0 ||
				pPluginInfo->strPluginFileName.CompareNoCase("GenericRecursiveFileImport.nxdp") == 0){
				m_pDevicePluginImportDlg->ShowCombinePdfsButton();
			}
		}
		//load the plugin
		if(!pPluginInfo->strPluginFileName.IsEmpty()) {
			CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";
			CString strPluginPath = strPath ^ pPluginInfo->strPluginFileName;
			if(DoesExist(strPluginPath)) {
				//try to load the plugin
				CDevicePlugin *pPlugin = new CDevicePlugin(strPath, pPluginInfo->strPluginFileName);
				if(pPlugin) {

					MSXML2::IXMLDOMDocument2Ptr Doc(__uuidof(MSXML2::DOMDocument60));
					CString strErrorDesc;
					BOOL bSuccess = pPlugin->Process(pPluginInfo->strDeviceExportPath, Doc, strErrorDesc);
					if(bSuccess) {
						//success, process the XML
						DevicePluginUtils::DeviceResult *pDeviceResult = DevicePluginUtils::GenerateDeviceResultsFromXML(Doc);

						CString strPluginName = pDeviceResult->strPluginFullDesc;
						if(!pPluginInfo->strPluginOverrideName.IsEmpty()) {
							strPluginName = pPluginInfo->strPluginOverrideName;
						}

						//send each patient to the dialog
						for(int j=pDeviceResult->aryPatients.GetSize()-1; j>=0; j--) {
							DevicePluginUtils::PatientElement *pPatient = (DevicePluginUtils::PatientElement*)pDeviceResult->aryPatients.GetAt(j);

							//ensure the dialog is created
							EnsureImportDlg();

							// (j.jones 2010-11-02 16:04) - PLID 41189 - pass in eDevicePatientMatchRule
							// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
							//(e.lally 2011-04-21) PLID 43372 added strPluginFileName
							// (r.gonet 06/11/2013) - PLID 56370 - Pass the Image DPI.
							// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
							if(m_pDevicePluginImportDlg->AddNewPatientElement(pPatient, strPluginName, pPluginInfo->strPluginFileName, pPluginInfo->eDevicePatientMatchRule, pPluginInfo->eConvertPDFToImage, pPluginInfo->nImageDPI, pPluginInfo->eConverter)) {
								//if AddNewPatientElement returned TRUE, then we no longer own the pointer,
								//so remove it from the result set
								pDeviceResult->aryPatients.RemoveAt(j);

								// (j.jones 2011-03-15 16:05) - PLID 42850 - at the end of this function
								// we may notify the user if files were found; do not do it here
							}
						}

						//this will delete the patient elements that still remain in the result set,
						//ones used by the import dialog are no longer owned by pDeviceResult
						delete pDeviceResult;
					}
					else {
						CString strMessage;
						strMessage.Format("Failed to process folder %s with the device plugin %s.\n\n"
							"%s", pPluginInfo->strDeviceExportPath, pPluginInfo->strPluginFileName, strErrorDesc);
						GetMainFrame()->MessageBox(strMessage, "Practice", MB_OK|MB_ICONEXCLAMATION);
					}

					//free the plugin
					delete pPlugin;
				}
				else {
					Logger.Log("Monitor::ProcessPluginFolder - Failed to load device plugin %s (LoadLibrary Error: %lu).", pPluginInfo->strPluginFileName, GetLastError());
				}
			}
			else {
				Logger.Log("Monitor::ProcessPluginFolder - Failed to load device plugin %s because the plugin file does not exist.", pPluginInfo->strPluginFileName);
			}
		}

		// (j.jones 2011-03-15 16:05) - PLID 42850 - now, if the import dialog
		// is still not shown, and records now exist that did not exist before,
		// notify the user, provided their preference is set to do so
		if(m_pDevicePluginImportDlg && m_pDevicePluginImportDlg->GetSafeHwnd()
			&& !m_pDevicePluginImportDlg->IsWindowVisible()
			&& m_pDevicePluginImportDlg->GetDeviceImportRecordCount() > nPreviousFileCount
			&& GetRemotePropertyInt("DeviceImport_NotifyOnStartup", 1, 0, GetCurrentUserName(), true) == 1) {

			//notify the user that files are ready to import
			GetMainFrame()->NotifyUser(NT_DEVICE_IMPORT, "", false);
		}

	}NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-10-26 13:44) - PLID 41088 - Returns whether the Device Importer has images waiting to be imported
// (j.jones 2011-03-15 13:54) - PLID 42738 - this now returns a count of records
long Monitor::GetDeviceImportRecordCount()
{
	try {

		//ensure the dialog is created
		EnsureImportDlg();

		// (d.lange 2011-03-11 10:00) - PLID 42754 - No longer checking the datalist for status
		return m_pDevicePluginImportDlg->GetDeviceImportRecordCount();

	} NxCatchAll(__FUNCTION__);

	return 0;
}


// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
void Monitor::BuildDeviceImportFilter()
{
	m_pDevicePluginImportDlg->BuildDevicePluginsCombo();
}

// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
void Monitor::FilterImportList()
{
	m_pDevicePluginImportDlg->Refresh();
}

// (b.savon 2011-09-23 13:51) - PLID 44954 - Optimize initial processing of watched folders at Practice start
BOOL Monitor::IsPluginProcessComplete()
{
	if (!m_workerThread || !m_workerIdleEvent || (WAIT_OBJECT_0 == ::WaitForSingleObject(m_workerIdleEvent, 0)) ) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// (b.savon 2011-09-23 13:51) - PLID 44954 - Optimize initial processing of watched folders at Practice start
void Monitor::CleanupPluginProcessThreads()
{
	if (m_workerThread) {
		m_workerThread->Interrupt();
		if (m_workerThread && m_workerThread->Joinable()) {
			m_workerThread->Join();
		}
		m_workerThread.Reset();
	}
}


// (j.dinatale 2011-07-26 17:41) - PLID 45420 - read barcodes from images
CString GetImageBarcode(const CString& strImageFilePath)
{
	CString strBarcode;

	// EXIF data only exists on jpeg and tiff files
	CString strFileExtension = GetFileExtension(strImageFilePath);
	strFileExtension.MakeLower();
	if(strFileExtension != "jpg" && strFileExtension != "jpeg"){
		return strBarcode;
	}

	// (j.dinatale 2011-09-07 15:51) - PLID 45420 - Use the lib functions that were written to get the App5 info
	CMap<CString, LPCTSTR, CString, LPCTSTR> mapMemos;

	CameraUtils::AppDataReader App5Reader;
	App5Reader.GetApp5Tags(strImageFilePath, mapMemos);

	mapMemos.Lookup("barcode", strBarcode);

	return strBarcode;
}

// (j.dinatale 2011-07-26 17:41) - PLID 45420 - read barcodes from images
bool ProcessImageBarcode(const CString& strImageFilePath, const CString& strBarcode, const CString& strDeviceName)
{
	// keep track if the image was imported
	bool bAutoImported = false;

	if(!strBarcode.IsEmpty()){
		Logger.Log("Processing barcode `%s` for image `%s` from device `%s`", strBarcode, strImageFilePath, strDeviceName);
		// our EMR barcodes are in the format of <personID>-<EMRID>, so find the index of the hyphen
		int nHyphenIndex = strBarcode.Find("-");
		if(nHyphenIndex != -1){
			CString strPersonID = strBarcode.Left(nHyphenIndex);
			CString strEMRID = strBarcode.Right(strBarcode.GetLength() - (nHyphenIndex + 1));
			if(!strEMRID.IsEmpty() && !strPersonID.IsEmpty()){
				long nPersonID = atoi(strPersonID);
				long nEMRID = atoi(strEMRID);

				// once we have our personID and EMRID, loop through all open PICs, and find the one with the same EMR ID as the barcode
				if(!GetMainFrame()->GetPicContainers().IsEmpty()) {			
					POSITION pos = GetMainFrame()->GetPicContainers().GetHeadPosition();
					while(pos) {
						CPicContainerDlg *pPicContainer = GetMainFrame()->GetPicContainers().GetNext(pos);
						if(IsWindow(pPicContainer->GetSafeHwnd())) {
							if(pPicContainer->GetCurrentEMRGroupID() == nEMRID && pPicContainer->HasWriteableEMRTopicOpen()) {
									CString strFinalFileName = "";
									long nMailSentID = -1;

									// (j.dinatale 2011-10-18 13:11) - PLID 45420 - Maximize the window first.
									//Bring the window up if it's minimized
									WINDOWPLACEMENT wp;
									wp.length = sizeof(WINDOWPLACEMENT);
									if(pPicContainer->GetWindowPlacement(&wp)) {
										if(pPicContainer->IsIconic()) {
											if(wp.flags & WPF_RESTORETOMAXIMIZED) {
												wp.showCmd = SW_MAXIMIZE;
											}else {
												wp.showCmd = SW_RESTORE;
											}
											pPicContainer->SetWindowPlacement(&wp);
										}
									}

									// (j.dinatale 2011-09-20 11:58) - PLID 45514 - attach the file silently!
									// (j.dinatale 2011-09-12 17:09) - PLID 45420 - Attach it to history first!
									if(!ImportAndAttachFileToHistory(strImageFilePath, nPersonID, GetMainFrame()->GetSafeHwnd(), -1, 
										pPicContainer->GetCurrentPicID(), TRUE, "",&strFinalFileName, COleDateTime::GetCurrentTime(), &nMailSentID, true)){
											break;
									}

									Logger.Log("Attached as name %s", strFinalFileName);

									// if the PIC is visible, has the same EMR ID as our barcode, and has a topic that we can write to, go ahead and fire away.
									pPicContainer->PostMessage(NXM_ADD_IMAGE_TO_EMR, (WPARAM)strFinalFileName.AllocSysString(), (LPARAM)strDeviceName.AllocSysString());
									bAutoImported = true;

									// (j.dinatale 2011-09-12 17:09) - PLID 45420 - now we can delete the file
									DeleteFile(strImageFilePath);

									break;
							}
						}
					}
				}
			}
		}
	}

	// this is our success flag, return if we successfully imported via barcode or not
	return bAutoImported;
}


// (j.dinatale 2011-07-26 17:41) - PLID 45420 - read barcodes from images
bool InterpretImageBarcode(const CString& strImageFilePath, const CString& strDeviceName)
{
	CString strBarcode = GetImageBarcode(strImageFilePath);
	return ProcessImageBarcode(strImageFilePath, strBarcode, strDeviceName);
}


// (b.savon 2012-02-13 11:33) - PLID 46456 - Restructure
void Monitor::QueryAndBeginWatch()
{
	try{

		// (a.walling 2014-03-11 09:23) - PLID 61312 - This is called upon switching users
		// this was causing trouble since the worker thread is already running,
		// and waiting on these handles that have already been created.
		// we just need to create them if they do not already exist 

		if (!m_pluginChangedEvent) {
			m_pluginChangedEvent.Attach(::CreateEvent(NULL, FALSE, FALSE, NULL));
		}
		if (!m_workerIdleEvent) {
			m_workerIdleEvent.Attach(::CreateEvent(NULL, TRUE, TRUE, NULL));
		}

		// (a.walling 2014-01-22 16:16) - PLID 60438 - DeviceImport - Speed up scan by ignoring already-processed paths, if DeviceImport_OnlyUniquePaths is set
		// (a.walling 2014-01-22 15:55) - PLID 60271 - a WPARAM and LPARAM both -1 means we are ready to import; otherwise just a folder changed notification as usual		
		// (r.farnworth 2014-10-02 11:52) - PLID 63378 - Added loginMsg
		if (!m_workerThread || !m_workerThread->Joinable()) {
			m_workerThread = NxThread(boost::bind(&Monitor::RunWorker, this, !!GetRemotePropertyInt("DeviceImport_OnlyUniquePaths", 1, 0, "<None>", true),
				GetMainFrame()->GetSafeHwnd(), NXM_DEVICE_PLUGIN_FOLDER_CHANGED, (WPARAM)-1, (LPARAM)-1, NXM_DEVICE_PLUGIN_FOLDER_BEGIN_WATCH));
		}

		//load up which plugins are enabled for this computer
		// (j.jones 2010-11-02 14:51) - PLID 41189 - added DevicePatientMatchRule
		// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
		// (j.jones 2011-03-10 15:41) - PLID 41349 - added NotificationRule
		// (r.gonet 06/11/2013) - PLID 56370 - Also get the Image DPI
		// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
		// (r.gonet 2016-05-20 3:39) - NX-100691 - Use the client's machine name.
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID, PluginFileName, PluginOverrideName, DeviceExportPath, "
			"NotificationRule, DevicePatientMatchRule, ConvertPDFToImage, ImageDPI, PDFToImageConverter "
			"FROM DevicePluginConfigT "
			"WHERE ComputerName = {STRING} AND Enable = 1", 
			g_propManager.GetSystemName());
		while(!rs->eof) {

			long nID = AdoFldLong(rs, "ID");
			CString strPluginFileName = AdoFldString(rs, "PluginFileName", "");			
			CString strPluginOverrideName = AdoFldString(rs, "PluginOverrideName", "");
			CString strDeviceExportPath = AdoFldString(rs, "DeviceExportPath", "");
			EDeviceNotificationRule eNotificationRule = (EDeviceNotificationRule)AdoFldLong(rs, "NotificationRule", (long)dpnrAutoPopup);
			EDevicePatientMatchRule eDevicePatientMatchRule = (EDevicePatientMatchRule)AdoFldLong(rs, "DevicePatientMatchRule", (long)dpmrMatchByIDOnly);
			EConvertPDFToImage eConvertPDFToImage = (EConvertPDFToImage)AdoFldLong(rs, "ConvertPDFToImage", (long)cptiDoNotConvert);
			// (r.gonet 06/11/2013) - PLID 56370 - Retrieve the default Image DPI value.
			long nImageDPI = AdoFldLong(rs, "ImageDPI", IMAGE_DPI_NULL);
			if(eConvertPDFToImage == cptiConvertToImage && nImageDPI == IMAGE_DPI_NULL) {
				// (r.gonet 06/11/2013) - PLID 56370 - This is an invalid value combination. It should be set to IMAGE_DPI_AUTO, the placeholder for Automatic.
				ASSERT(FALSE);
				nImageDPI = IMAGE_DPI_AUTO;
			} else if(eConvertPDFToImage == cptiDoNotConvert && nImageDPI != IMAGE_DPI_NULL) {
				// (r.gonet 06/11/2013) - PLID 56370 - This is an invalid value combination. It should be set to IMAGE_DPI_NULL, the placeholder for NULL.
				ASSERT(FALSE);
				nImageDPI = IMAGE_DPI_NULL;
			} else {
				// (r.gonet 06/11/2013) - PLID 56370 - Value combination is fine.
			}
			// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
			EPDFToImageConverter eConverter = AdoFldBool(rs, "PDFToImageConverter", FALSE) == FALSE ? pticTwoPilots : pticBCL;

			//does the export path exist?
			if(strDeviceExportPath.IsEmpty() || !DoesExist(strDeviceExportPath)) {
				Logger.Log("Monitor::TryMonitorDevicePluginFolders - Failed to load device plugin %s because the device export path, %s, does not exist.", strPluginFileName, strDeviceExportPath);
				rs->MoveNext();
				continue;
			}

			//first ensure the plugin exists			
			if(!strPluginFileName.IsEmpty()) {
				CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";
				CString strPluginPath = strPath ^ strPluginFileName;
				if(DoesExist(strPluginPath)) {
					//try to load the plugin
					CDevicePlugin *pPlugin = new CDevicePlugin(strPath, strPluginFileName);
					if(pPlugin) {
						//grab the full name if we have no override
						if(strPluginOverrideName.IsEmpty()) {
							strPluginOverrideName = pPlugin->GetPluginDescription();
						}

						//now that we got this far, we know we have a valid plugin and export path,
						//so we can now begin monitoring the folder
						// (j.jones 2010-11-02 14:51) - PLID 41189 - added DevicePatientMatchRule
						// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
						// (j.jones 2011-03-10 15:41) - PLID 41349 - added NotificationRule
						// (r.gonet 06/11/2013) - PLID 56370 - Pass the Image DPI
						// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
						StartWatchingDeviceFolder(nID, strPluginFileName, strPluginOverrideName, strDeviceExportPath,
							eNotificationRule, eDevicePatientMatchRule, eConvertPDFToImage, nImageDPI, eConverter, FALSE);

						//free the plugin, we will reopen it later
						delete pPlugin;
					}
					else {
						Logger.Log("Monitor::QueryAndBeginWatch - Failed to load device plugin %s (LoadLibrary Error: %lu).", strPluginFileName, GetLastError());
					}
				}
				else {
					Logger.Log("Monitor::QueryAndBeginWatch - Failed to load device plugin %s because the plugin file does not exist.", strPluginFileName);
				}
			}

			rs->MoveNext();
		}
		rs->Close();
	}NxCatchAll("Monitor::QueryAndBeginWatch - Failed to watch device plugins");
}

// (a.walling 2014-01-22 15:55) - PLID 60271 - Moved from the DevicePluginImportDlg
void Monitor::RescanDevicePluginFolders()
{
	// ProcessPluginFolder for all plugins
	for each (shared_ptr<const DevicePluginInfo> pPluginInfo in m_monitoredDevicePlugins) {
		ProcessPluginFolder(pPluginInfo);
	}
}

// (a.walling 2014-01-22 15:55) - PLID 60271 - Consolidated here
void Monitor::EnsureImportDlg()
{
	if(m_pDevicePluginImportDlg == NULL) {
		// (a.walling 2012-07-10 13:32) - PLID 46648 - Dialogs must set a parent - in this case, we don't want one, since we always keep this around.
		m_pDevicePluginImportDlg = new CDevicePluginImportDlg(CWnd::GetDesktopWindow());
		m_pDevicePluginImportDlg->Create(IDD_DEVICES_IMPORT_DLG, CWnd::GetDesktopWindow());
	}
}

}

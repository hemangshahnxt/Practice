#pragma once

#include "DevicePluginImportDlg.h"
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <NxSystemUtilitiesLib/NxThread.h>
#include <NxSystemUtilitiesLib/NxHandle.h>

// (a.walling 2014-01-22 16:06) - PLID 60271 - Device Import scans changed folders on the main thread - move this to a background thread so the UI is not blocked.

// (a.walling 2014-01-22 16:06) - PLID 60271 - DevicePluginInfo now encapsulated within a shared_ptr

// (a.walling 2013-06-10 14:48) - PLID 56225 - Device Import code all moved into DeviceImport namespace, DeviceImportMonitor.cpp/.h

// (r.gonet 06/11/2013) - PLID 56370 - Added some forward declarations to avoid including a header file.
enum EDeviceNotificationRule;
enum EDevicePatientMatchRule;

// (b.savon 2014-12-03 13:36) - PLID 64186 - Likewise
enum EPDFToImageConverter;

// (j.jones 2010-06-01 13:05) - PLID 37976 - cache device plugin info,
// without keeping the plugin file open in memory
struct DevicePluginInfo
{
	long nDevicePluginID;
	CString strPluginFileName;
	CString strPluginOverrideName;
	CString strDeviceExportPath;
	// (j.jones 2011-03-10 15:47) - PLID 41349 - added NotificationRule
	EDeviceNotificationRule eNotificationRule;
	// (j.jones 2010-11-02 14:51) - PLID 41189 - track the setting to match patients
	EDevicePatientMatchRule eDevicePatientMatchRule;
	// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
	EConvertPDFToImage eConvertPDFToImage;
	// (r.gonet 06/11/2013) - PLID 56370 - Added Image DPI.
	long nImageDPI;
	// (b.savon 2014-12-03 13:36) - PLID 64186 - Added converter
	EPDFToImageConverter eConverter;
};

namespace DeviceImport
{

static const size_t ID_TRAYICON = 0xb000;

class Monitor;

Monitor& GetMonitor();

// (j.dinatale 2011-07-26 16:21) - PLID 45420 - function that attempts to interpret a barcode from an image
bool InterpretImageBarcode(const CString& strImageFilePath, const CString& strDeviceName);
CString GetImageBarcode(const CString& strImageFilePath);
bool ProcessImageBarcode(const CString& strImageFilePath, const CString& strBarcode, const CString& strDeviceName);

// (a.walling 2014-01-22 16:06) - PLID 60271 - changed files and patient elements are now gathered into a
// structure within the background scan thread, which is then passed to the main thread to be processed

struct FileChanges;

struct PendingPatientElement;

struct PendingPluginChanges;

class Monitor
{
public:
	Monitor();
	~Monitor();

	void Init();
	void Close();

	Nx::TrayIcon& GetTrayIcon()
	{ return m_trayDeviceImport;}

	CDevicePluginImportDlg* GetImportDlg()
	{ return m_pDevicePluginImportDlg; }
	CDevicePluginImportDlg *GetDevicePluginDlg()
	{ return GetImportDlg(); }

	bool HandleTimer(UINT nIDEvent);
	void TriggerFolderChanged();
	void TriggerReadyToImport();
	void TriggerOnLogin(); // (r.farnworth 2014-10-01 09:53) - PLID 63378

	// (d.lange 2010-06-07 23:50) - PLID 38850 - added DevicePluginImportDlg as a modeless dialog
	void ShowDevicePluginImportDlg();
	void CleanUpDevicePluginImportDlg();

	// (j.jones 2010-06-01 09:51) - PLID 37976 - check whether we should
	// be monitoring ophthalmology device folders from available plugins
	// (r.farnworth 2014-10-02 11:49) - PLID 63378 - Added bFromLogin
	void TryMonitorDevicePluginFolders(bool bFromLogin = false);

	// (j.jones 2010-06-01 10:24) - PLID 37976 - closes all open folder monitors
	void CloseAllDeviceFolderMonitors();

	// (d.lange 2011-03-11 11:01) - PLID 41010 - Refresh the device import dialog if the current patient filter is enabled
	void NotifyDeviceImportPatientChanged();	
	
	// (j.jones 2010-06-01 10:18) - PLID 37976 - added ability to monitor device plugin folders
	// (j.jones 2010-11-02 14:51) - PLID 41189 - added DevicePatientMatchRule
	// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
	// (j.jones 2011-03-10 15:41) - PLID 41349 - added NotificationRule
	// (b.savon 2011-09-23 11:48) - PLID 44954 - added bNewPlugin so that we can distiguish between initial plugin processing on login
	// (r.gonet 06/11/2013) - PLID 56370 - Added Image DPI
	// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
	void StartWatchingDeviceFolder(long nDevicePluginID, CString strPluginFileName, CString strPluginOverrideName, CString strDeviceExportPath,
								EDeviceNotificationRule eNotificationRule, EDevicePatientMatchRule eDevicePatientMatchRule,
								EConvertPDFToImage eConvertPDFToImage, long nImageDPI, EPDFToImageConverter eConverter, BOOL bNewPlugin);
	void CloseDeviceFolderMonitor(long nDevicePluginID);

	// (j.jones 2010-06-01 12:27) - PLID 37976 - process plugins
	void ProcessChangedPlugins();

	// (j.jones 2010-10-22 10:52) - PLID 41069 - ProcessPluginFolder will import
	// all files in the given plugin's folder, but not pop up the device import dialog
	void ProcessPluginFolder(shared_ptr<const DevicePluginInfo> pPluginInfo);

	// (d.lange 2010-10-26 13:44) - PLID 41088 - Returns whether the Device Importer has images waiting to be imported
	// (j.jones 2011-03-15 13:54) - PLID 42738 - this now returns a count of records
	long GetDeviceImportRecordCount();

	// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
	void BuildDeviceImportFilter();
	void FilterImportList();

	BOOL IsPluginProcessComplete();

	// (j.jones 2011-08-25 15:02) - PLID 41683 - clear all records for a given plugin,
	// provided the import dialog exists
	void TryRemoveRecordsForPlugin(CString strPluginName);
	// (a.walling 2014-01-22 15:55) - PLID 60271 - Moved from the DevicePluginImportDlg
	void RescanDevicePluginFolders();

	// (b.savon 2011-09-23 13:51) - PLID 44954 - Optimize initial processing of watched folders at Practice start
	void CleanupPluginProcessThreads();

	// (a.walling 2014-01-22 15:55) - PLID 60271 - Consolidated here
	void EnsureImportDlg();

protected:

	// (d.lange 2010-06-07 23:40) - PLID 38850 - Make DevicePluginImportDlg modeless dialog
	CDevicePluginImportDlg *m_pDevicePluginImportDlg;
	// (a.walling 2012-12-04 16:12) - PLID 54037 - Device import notification tray icon
	Nx::TrayIcon m_trayDeviceImport;
	
	void ProcessPending(bool bOnLogin = false); // (r.farnworth 2014-10-01 09:53) - PLID 63378 - If we are calling this function from logging in, we need to look at different settings
	
	// (j.jones 2010-06-01 10:18) - PLID 37976 - added ability to monitor device plugin folders
	boost::container::flat_set<shared_ptr<const DevicePluginInfo>> m_monitoredDevicePlugins;
	CMap<long, long, CFolderMonitor*, CFolderMonitor*> m_mapPluginIDsToFolderMonitors;
	CMap<long, long, CFolderMonitorChangeHandler_Array*, CFolderMonitorChangeHandler_Array*> m_mapPluginIDsToFolderMonitorChangedArrays;	

	typedef boost::container::flat_map<weak_ptr<const DevicePluginInfo>, FileChanges> PluginFileChangesMap;
	PluginFileChangesMap m_mapPluginFileChanges;	

	typedef boost::container::flat_map<weak_ptr<const DevicePluginInfo>, PendingPluginChanges> PendingPluginChangesMap;
	PendingPluginChangesMap m_mapPendingPluginChanges;
	std::set<CiString> m_setDeletedFiles;

	CCriticalSection m_csPluginChanges;

	// (a.walling 2014-01-22 16:06) - PLID 60271 - The actual worker thread for the background scanning

	// (r.farnworth 2014-10-02 11:54) - PLID 63378 - Added loginMsg
	void RunWorker(bool onlyUniquePaths, HWND notifyHwnd, UINT notifyMsg, WPARAM notifyWParam, LPARAM notifyLParam, UINT loginMsg);

	NxThread m_workerThread;
	Nx::Handle m_pluginChangedEvent;
	Nx::Handle m_workerIdleEvent;

	// (b.savon 2012-02-13 11:42) - PLID 46456 - Handle network disconnect - Restructure
	void QueryAndBeginWatch();
	
	// (c.haag 2010-06-30 15:27) - PLID 39424 - Device import license checking
	BOOL m_bIsDeviceImportLicensed;
	BOOL m_bWasDeviceImportLicenseChecked;

	bool m_bFromLogin; // (r.farnworth 2014-10-02 11:54) - PLID 63378
};

}
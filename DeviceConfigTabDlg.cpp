// DeviceConfigTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "DeviceConfigTabDlg.h"
#include "PracticeRc.h"
#include "RegUtils.h"
#include "GlobalUtils.h"
#include "DevicePlugin.h"
#include "DevicePluginSetupDlg.h"
#include "DevicePluginImportDlg.h"
#include "DevicePluginUtils.h"
#include "DeviceImportMonitor.h"

// (a.walling 2013-04-11 17:05) - PLID 56225 - Move DeviceImport stuff into its own class

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (d.lange 2010-05-07 15:22) - PLID 38536 - created
// CDeviceConfigTabDlg dialog

IMPLEMENT_DYNAMIC(CDeviceConfigTabDlg, CNxDialog)

CDeviceConfigTabDlg::CDeviceConfigTabDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDeviceConfigTabDlg::IDD, pParent)
{
	// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
	m_strPluginNameOriginal = "";
	m_strDeviceExportPathOriginal = "";
	m_strDeviceExePathOriginal = "";
	m_strDeviceAddlPathOriginal = "";
	bRetry = FALSE;	
}

CDeviceConfigTabDlg::~CDeviceConfigTabDlg()
{
}

void CDeviceConfigTabDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DELETE_PLUGIN, m_btnDelete);
}


BEGIN_MESSAGE_MAP(CDeviceConfigTabDlg, CNxDialog)
	ON_BN_CLICKED(IDC_DELETE_PLUGIN, &CDeviceConfigTabDlg::OnBnClickedBtnDelete)
END_MESSAGE_MAP()

BOOL CDeviceConfigTabDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Set controls
		m_btnDelete.AutoSet(NXB_DELETE);

		GetDlgItem(IDC_DELETE_PLUGIN)->EnableWindow(FALSE);

		m_pDetectedPluginsList = BindNxDataList2Ctrl(IDC_DETECTED_DEVICES_COMBO, false);
		m_pActivePluginsList = BindNxDataList2Ctrl(IDC_ACTIVE_DEVICE_PLUGINS, false);

		// (b.savon 2013-05-23 15:29) - PLID 42902 - HB said this shouldn't be filtered on permissioned categories.  The reason of permissioning the
		// categories is so that the user can view the documents in the history tab.  They should, however, be able to import it into that category.
		// (r.gonet 06/11/2013) - PLID 57127 - Fixed a bug where we were using the wrong enumeration type.
		IColumnSettingsPtr pCol = m_pActivePluginsList->GetColumn(apcDefaultCategory);
		pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF UNION SELECT NULL, '{No Category}' AS Description ORDER BY Description"));

		m_pActivePluginsList->GetColumn(apcPluginName)->PutMaxTextLen(255);
		// (r.gonet 2016-05-20 3:39) - NX-100691 - Use the client's machine name.
		m_pActivePluginsList->WhereClause = _bstr_t(FormatString("ComputerName = '%s'", 
			g_propManager.GetSystemName()));
		m_pActivePluginsList->Requery();
		
		//Let's detect available plugins and build the combo box
		BuildDevicePluginsCombo();
		
	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// CDeviceConfigTabDlg message handlers
BEGIN_EVENTSINK_MAP(CDeviceConfigTabDlg, CNxDialog)
	ON_EVENT(CDeviceConfigTabDlg, IDC_DETECTED_DEVICES_COMBO, 16, CDeviceConfigTabDlg::SelChosenDetectedDevicesCombo, VTS_DISPATCH)
	ON_EVENT(CDeviceConfigTabDlg, IDC_ACTIVE_DEVICE_PLUGINS, 19, CDeviceConfigTabDlg::LeftClickActiveDevicePlugins, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDeviceConfigTabDlg, IDC_ACTIVE_DEVICE_PLUGINS, 29, CDeviceConfigTabDlg::SelSetActiveDevicePlugins, VTS_DISPATCH)
	ON_EVENT(CDeviceConfigTabDlg, IDC_ACTIVE_DEVICE_PLUGINS, 10, CDeviceConfigTabDlg::EditingFinishedActiveDevicePlugins, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CDeviceConfigTabDlg, IDC_ACTIVE_DEVICE_PLUGINS, 18, CDeviceConfigTabDlg::RequeryFinishedActiveDevicePlugins, VTS_I2)
	ON_EVENT(CDeviceConfigTabDlg, IDC_ACTIVE_DEVICE_PLUGINS, 8, CDeviceConfigTabDlg::EditingStartingActiveDevicePlugins, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CDeviceConfigTabDlg, IDC_ACTIVE_DEVICE_PLUGINS, 21, CDeviceConfigTabDlg::EditingStartedActiveDevicePlugins, VTS_DISPATCH VTS_I2 VTS_I4)
END_EVENTSINK_MAP()

void CDeviceConfigTabDlg::BuildDevicePluginsCombo()
{
	try {
		CFileFind finder;
		CDevicePlugin *pPlugin;
		CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";
		CString strFileName = "";
		CString strPluginName = "";
		long nLaunchSetting = 0;
		BOOL bDone = TRUE;

		//We need to find and populate the "Detected Device Plugins" with the clean plugin name
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		if(finder.FindFile(strPath + "*.nxdp")) {
			while(bDone) {
				if (!finder.FindNextFile())
					bDone = FALSE;
				
				//Get the filename and create a DevicePlugin object so we can retrieve the "clean" plugin name, then unload it
				strFileName = finder.GetFileName();
				pPlugin = new CDevicePlugin(strPath, strFileName);
				//Get the "clean" device plugin
				strPluginName = pPlugin->GetPluginDescription();
				//Get the Launch Device setting to determine
				nLaunchSetting = (int)pPlugin->GetLaunchDeviceSettings();
				delete pPlugin;

				//Let's put the "clean" plugin name in the dropdown combo
				pRow = m_pDetectedPluginsList->GetNewRow();
				pRow->PutValue(dpcLaunchSetting, nLaunchSetting);
				pRow->PutValue(dpcPluginFileName, _bstr_t(strFileName));
				pRow->PutValue(dpcPluginName, _bstr_t(strPluginName));
				
				m_pDetectedPluginsList->AddRowAtEnd(pRow, NULL);
			}
		}
		finder.Close();

		CString strDetectedCount;
		strDetectedCount.Format("< %li Device Plugin(s) Detected >", m_pDetectedPluginsList->GetRowCount());
		
		m_pDetectedPluginsList->PutComboBoxText(_bstr_t(strDetectedCount));

	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
BOOL CDeviceConfigTabDlg::IsValidPluginName( CString strPluginName, BOOL bConfigTab /* = FALSE */ )
{
	//	Reject if the name is empty (this should only be hit when the user edits the active plugin datalist in device config tab)
	if( strPluginName.Trim().IsEmpty() ){
		MessageBox( "Please enter a valid Device Plugin Name.", "Invalid Plugin Name", MB_ICONINFORMATION );
		return FALSE;
	}

	//	Declare vars
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	int nCount = 0;
	pRow = m_pActivePluginsList->GetFirstRow();

	//	If were editing from the config tab active plugins list, the name changes in the list
	//	and then this func is called.  In which case, there will be 1 name in the list with the same
	//	as the one passed (the one we changed).
	//
	//	The other time is when the setup dlg is in the mix, in which case the name isn't added until
	//	we verify, so there is no records with this name.
	while(pRow){
		CString strActivePlugin = VarString(pRow->GetValue(apcPluginName));
		if ( strActivePlugin.CompareNoCase(strPluginName) == 0 && nCount == 1 && bConfigTab ){
				CString strMessage;
				strMessage.Format("There is already a plugin named %s.\n\nPlease select a unique  name.", strPluginName);
				MessageBox( strMessage, "Duplicate Plugin Name", MB_ICONINFORMATION );
				return FALSE;
			} 	
		else if( strActivePlugin.CompareNoCase(strPluginName) == 0 ){
			if( !bConfigTab ){
				CString strMessage;
				strMessage.Format("There is already a plugin named %s.\n\nPlease select a unique  name.", strPluginName);
				MessageBox( strMessage, "Duplicate Plugin Name", MB_ICONINFORMATION );
				return FALSE;
			} else {
				++nCount;
			}
		}
		pRow = pRow->GetNextRow();
	}

	//	We didn't fail anywhere, this is a valid name
	return TRUE;
}//END BOOL CDeviceConfigTabDlg::IsValidPluginName( CString strPluginFileName, CString strPluginName, BOOL bOverride )

void CDeviceConfigTabDlg::SelChosenDetectedDevicesCombo(LPDISPATCH lpRow)
{
	// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
	if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
		MessageBox("Please wait until all the plugin files are finished processing.", "Device Plugin Files Processing...", MB_ICONWARNING|MB_OK);
		CString strDetectedCount;
		strDetectedCount.Format("< %li Device Plugin(s) Detected >", m_pDetectedPluginsList->GetRowCount());
		m_pDetectedPluginsList->PutComboBoxText(_bstr_t(strDetectedCount));		
		return;
	}

	try {
		//Add the plugin that was selected to the Active Plugin list
		NXDATALIST2Lib::IRowSettingsPtr pSelRow(lpRow);
		
		if (pSelRow) {
			CString strPluginFileName = pSelRow->GetValue(dpcPluginFileName);
			CString strPluginName = pSelRow->GetValue(dpcPluginName);
			long nLaunchSetting = pSelRow->GetValue(dpcLaunchSetting);
			CString strDeviceExportPath = "";
			CString strDeviceAddlPath = "";
			CString strDeviceExePath = "";

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pActivePluginsList->GetNewRow();

			CDevicePluginSetupDlg dlg(this);

			//Send the dialog the "clean" Plugin name and its launch setting
			dlg.m_strPluginName = strPluginName;
			dlg.m_nLaunchSetting = nLaunchSetting;

			// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
			if( bRetry ){
				bRetry = FALSE;
				dlg.m_strDeviceExportPath = m_strDeviceExportPathOriginal;
				dlg.m_strDeviceExePath = m_strDeviceExePathOriginal;
				dlg.m_strDeviceAddlPath = m_strDeviceAddlPathOriginal;
			}
			
			if (dlg.DoModal() == IDOK) {
				//Retrieve the paths
				// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
				strDeviceExportPath = m_strDeviceExportPathOriginal = dlg.m_strDeviceExportPath;
				strDeviceExePath = m_strDeviceExePathOriginal = dlg.m_strDeviceExePath;
				strDeviceAddlPath = m_strDeviceAddlPathOriginal = dlg.m_strDeviceAddlPath;

				_RecordsetPtr rs;

				if(strPluginName.Compare(dlg.m_strPluginName) != 0) {	//If we detect a change from the setup dialog, we should store the new plugin name
					//The name has changed so lets set this local variable so we can update the row with it
					strPluginName = dlg.m_strPluginName;

					// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
					if( !IsValidPluginName( strPluginName ) ){
						bRetry = TRUE;
						SelChosenDetectedDevicesCombo(lpRow);
						return;
					}

					//Since we have detected a change in the name, we need to store this so it can be displayed later
					// (j.jones 2010-06-02 08:57) - PLID 37976 - converted to a recordset that returns the new ID
					// (j.jones 2010-11-02 14:19) - PLID 41189 - added DevicePatientMatchRule, always defaults to 0
					// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage, always defaults to 0
					// (j.jones 2011-03-10 15:47) - PLID 41349 - added NotificationRule, always defaults to 1
					// (r.gonet 06/11/2013) - PLID 57127 - Added Image DPI, which defaults to NULL because it shouldn't be set when ConvertPDFToImage is not enabled.
					// (b.savon 2014-12-03 14:16) - PLID 64184 - Add a new embedded drop down column to the Links->Devices configured devices datalist to support choosing a default PDF->Image converter.
					// (r.gonet 2016-05-20 3:39) - NX-100691 - Use the client's machine name.
					rs = CreateParamRecordset("SET NOCOUNT ON \r\n"
						"INSERT INTO DevicePluginConfigT (ComputerName, PluginFileName, DeviceExportPath, DeviceExecutablePath, DeviceAdditionalPath, "
						"NotificationRule, DevicePatientMatchRule, "
						"PluginOverrideName, ConvertPDFToImage, ImageDPI, PDFToImageConverter) VALUES ({STRING}, {STRING}, {STRING}, {STRING}, {STRING}, 1, 0, {STRING}, 0, NULL, 0) \r\n"
						""
						"SET NOCOUNT OFF \r\n"
						""
						"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID",
						g_propManager.GetSystemName(), 
						strPluginFileName, strDeviceExportPath, strDeviceExePath, strDeviceAddlPath, strPluginName);
				}
				else {
					// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
					if( !IsValidPluginName( strPluginName ) ){
						bRetry = TRUE;
						SelChosenDetectedDevicesCombo(lpRow);
						return;
					}
					//The user did not alter the plugin name, its still the "clean" name so we don't need to store this (we know how to retrieve this)
					//Since the user has selected the OK we need to insert this into the table
					// (j.jones 2010-06-02 08:57) - PLID 37976 - converted to a recordset that returns the new ID
					// (j.jones 2010-11-02 14:19) - PLID 41189 - added DevicePatientMatchRule, always defaults to 0
					// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage, always defaults to 0
					// (j.jones 2011-03-10 15:47) - PLID 41349 - added NotificationRule, always defaults to 1
					// (r.gonet 06/11/2013) - PLID 57127 - Added Image DPI, which defaults to NULL because it shouldn't be available when ConvertPDFToImage is unchecked.
					// (b.savon 2014-12-03 14:16) - PLID 64184 - Add a new embedded drop down column to the Links->Devices configured devices datalist to support choosing a default PDF->Image converter.
					// (r.gonet 2016-05-20 3:39) - NX-100691 - Use the client's machine name.
					rs = CreateParamRecordset("SET NOCOUNT ON \r\n"
						""
						"INSERT INTO DevicePluginConfigT (ComputerName, PluginFileName, DeviceExportPath, DeviceExecutablePath, DeviceAdditionalPath, "
						"NotificationRule, DevicePatientMatchRule, ConvertPDFToImage, ImageDPI, PDFToImageConverter) "
						"VALUES ({STRING}, {STRING}, {STRING}, {STRING}, {STRING}, 1, 0, 0, NULL, 0) \r\n"
						""
						"SET NOCOUNT OFF \r\n"
						""
						"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID", 
						g_propManager.GetSystemName(),
						strPluginFileName, strDeviceExportPath, strDeviceExePath, strDeviceAddlPath);
				}

				//Set the columns in the datalist
				long nPluginID = AdoFldLong(rs, "NewID");				
				pRow->PutValue(apcPluginID, (long)nPluginID);
				pRow->PutValue(apcEnabled, g_cvarTrue);
				pRow->PutValue(apcLaunchSetting, nLaunchSetting);
				pRow->PutValue(apcPluginFileName, _bstr_t(strPluginFileName));
				pRow->PutValue(apcPluginName, _bstr_t(strPluginName));
				pRow->PutValue(apcDeviceExportPath, _bstr_t(strDeviceExportPath));
				pRow->PutValue(apcDeviceExePath, _bstr_t(strDeviceExePath));
				pRow->PutValue(apcDeviceAddlPath, _bstr_t(strDeviceAddlPath));
				// (j.jones 2011-03-10 14:10) - PLID 41349 - added NotificationRule
				pRow->PutValue(apcNotificationRule, (long)dpnrAutoPopup);
				// (j.jones 2010-11-02 14:19) - PLID 41189 - added DevicePatientMatchRule, always defaults to 0 = dpmrMatchByIDOnly
				pRow->PutValue(apcDevicePatientMatchRule, (long)dpmrMatchByIDOnly);
				// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage, always defaults to 0 = cptiDoNotConvert,
				// but in the interface it is treated as a boolean, not an int
				pRow->PutValue(apcConvertPDFToImage, g_cvarFalse);
				// (r.gonet 06/11/2013) - PLID 57127 - Initialize the Image DPI to NULL, since ConvertPDFToImage is unchecked.
				pRow->PutValue(apcImageDPI, g_cvarNull);
				// (b.savon 2014-12-03 14:16) - PLID 64184 - Default
				pRow->PutValue(apcPDFToImageConverter, (long)pticTwoPilots);

				NXDATALIST2Lib::IFormatSettingsPtr pFormatColumn(__uuidof(NXDATALIST2Lib::FormatSettings));
				pFormatColumn->Editable = VARIANT_FALSE;
				pRow->CellFormatOverride[apcImageDPI] = pFormatColumn;

				m_pActivePluginsList->AddRowAtEnd(pRow, NULL);

				if(!rs->eof) {
					// (j.jones 2010-06-02 08:55) - PLID 37976 - start watching this folder
					// (j.jones 2010-11-02 14:51) - PLID 41189 - added DevicePatientMatchRule
					// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
					// (j.jones 2011-03-10 15:41) - PLID 41349 - added NotificationRule
					// (b.savon 2011-9-23) - PLID 44954 - added bNewPlugin
					// (r.gonet 06/11/2013) - PLID 57127 - Added Image DPI
					DeviceImport::GetMonitor().StartWatchingDeviceFolder(nPluginID, strPluginFileName, strPluginName, strDeviceExportPath,
						dpnrAutoPopup, dpmrMatchByIDOnly, cptiDoNotConvert, IMAGE_DPI_NULL, pticTwoPilots, TRUE);
				}
				rs->Close();

				// (b.savon 2011-9-9) - PLID 45314 - The active devices changed, reflect it on the import dialog filter
				DeviceImport::GetMonitor().BuildDeviceImportFilter();
				DeviceImport::GetMonitor().FilterImportList();
			}
		}
		CString strDetectedCount;
		strDetectedCount.Format("< %li Device Plugin(s) Detected >", m_pDetectedPluginsList->GetRowCount());

		m_pDetectedPluginsList->PutComboBoxText(_bstr_t(strDetectedCount));

	} NxCatchAll(__FUNCTION__);
}

void CDeviceConfigTabDlg::OnBnClickedBtnDelete()
{
	// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
	if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
		MessageBox("Please wait until all the plugin files are finished processing.", "Device Plugin Files Processing...", MB_ICONWARNING|MB_OK);
		return;
	}

	try {
		//Remove the selected plugin in the active plugin list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pActivePluginsList->CurSel;

		if (pRow) {
			CString strWarningMsg = "";
			CString strPluginName = pRow->GetValue(apcPluginName);
			strWarningMsg.Format("Are you sure you want to DELETE the %s configuration settings?", strPluginName);

			if (IDYES != MessageBox(strWarningMsg, NULL, MB_YESNO|MB_ICONQUESTION)) {
				return;
			}

			//Remove plugin from the datalist
			m_pActivePluginsList->RemoveRow(pRow);
			
			//Remove plugin from DevicePluginConfigT
			CString strPluginFileName = pRow->GetValue(apcPluginFileName);
			long nPluginID = pRow->GetValue(apcPluginID);
			ExecuteParamSql("DELETE FROM DevicePluginConfigT WHERE PluginFileName = {STRING} AND ID = {INT}", strPluginFileName, nPluginID);

			// (j.jones 2010-06-02 09:02) - PLID 37976 - stop watching this folder
			DeviceImport::GetMonitor().CloseDeviceFolderMonitor(nPluginID);

			// (j.jones 2011-08-25 14:55) - PLID 41683 - remove any files currently in the import screen
			DeviceImport::GetMonitor().TryRemoveRecordsForPlugin(strPluginName);

			// (b.savon 2011-9-9) - PLID 45314 - The active devices changed, reflect it on the import dialog filter
			DeviceImport::GetMonitor().BuildDeviceImportFilter();
			DeviceImport::GetMonitor().FilterImportList();
		}

	} NxCatchAll(__FUNCTION__);
}

void CDeviceConfigTabDlg::LeftClickActiveDevicePlugins(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
	if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
		MessageBox("Please wait until all the plugin files are finished processing.", "Device Plugin Files Processing...", MB_ICONWARNING|MB_OK);
		return;
	}

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		CString strDeviceExportPath = "";
		CString strDeviceExePath = "";
		CString strDeviceAddlPath = "";
		CString strPluginName = pRow->GetValue(apcPluginName);
		CString strPluginFileName = pRow->GetValue(apcPluginFileName);
		long nPluginID = pRow->GetValue(apcPluginID);
		long nLaunchSetting = pRow->GetValue(apcLaunchSetting);
		if (pRow == NULL) {
			return;
		}
		
		if(nCol == apcDeviceExportPath || nCol == apcDeviceExePath || nCol == apcDeviceAddlPath) {

			// (j.jones 2011-08-25 14:55) - PLID 41683 - track if the export path or plugin name changed
			CString strOldExportPath = pRow->GetValue(apcDeviceExportPath);
			CString strOldPluginName = strPluginName;

			CDevicePluginSetupDlg dlg(this);
			//Get and set the LaunchSetting
			dlg.m_nLaunchSetting = nLaunchSetting;
			//Get and set the "clean" plugin name
			dlg.m_strPluginName = strOldPluginName;
			dlg.m_strDeviceExportPath = strOldExportPath;
			dlg.m_strDeviceExePath = pRow->GetValue(apcDeviceExePath);
			dlg.m_strDeviceAddlPath = pRow->GetValue(apcDeviceAddlPath);

			if(dlg.DoModal() == IDOK) {

				// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
				if( strOldPluginName.CompareNoCase(dlg.m_strPluginName) != 0 ){
					if( !IsValidPluginName( dlg.m_strPluginName ) ){
						LeftClickActiveDevicePlugins(lpRow, nCol, x, y, nFlags);
						return;
					}
				}

				CString strBatchSql = BeginSqlBatch();
				CNxParamSqlArray aryParams;
				
				//Check to see if the Device plugin name has changed, if so we need to update the table
				if (strPluginName.Compare(dlg.m_strPluginName) != 0) {
					strPluginName = dlg.m_strPluginName;
					//Add to SQL Batch
					AddParamStatementToSqlBatch(strBatchSql, aryParams, "UPDATE DevicePluginConfigT SET PluginOverrideName = {STRING} "
						"WHERE PluginFileName = {STRING} AND ID = {INT};\r\n", strPluginName, strPluginFileName, nPluginID);
				}

				strDeviceExportPath = dlg.m_strDeviceExportPath;
				strDeviceExePath = dlg.m_strDeviceExePath;
				strDeviceAddlPath = dlg.m_strDeviceAddlPath;
				strPluginName = dlg.m_strPluginName;

				pRow->PutValue(apcDeviceExportPath, _bstr_t(strDeviceExportPath));
				pRow->PutValue(apcDeviceExePath, _bstr_t(strDeviceExePath));
				pRow->PutValue(apcDeviceAddlPath, _bstr_t(strDeviceAddlPath));				
				pRow->PutValue(apcPluginName, _bstr_t(strPluginName));

				//Add to SQL Batch				
				AddParamStatementToSqlBatch(strBatchSql, aryParams, "UPDATE DevicePluginConfigT SET DeviceExportPath = {STRING}, DeviceExecutablePath = {STRING}, "
					"DeviceAdditionalPath = {STRING} WHERE PluginFileName = {STRING} AND ID = {INT}", 
						strDeviceExportPath, strDeviceExePath, strDeviceAddlPath, strPluginFileName, nPluginID);

				if (!strBatchSql.IsEmpty()) {
					ExecuteParamSqlBatch(GetRemoteData(), strBatchSql, aryParams);

					// (j.jones 2010-06-02 09:02) - PLID 37976 - restart watching this folder
					BOOL bEnable = VarBool(pRow->GetValue(apcEnabled), FALSE);

					//stop watching
					DeviceImport::GetMonitor().CloseDeviceFolderMonitor(nPluginID);
					
					if(bEnable) {

						// (j.jones 2011-08-25 14:55) - PLID 41683 - if the export path or plugin name changed,
						// remove any files currently in the import screen
						if(strOldExportPath.CompareNoCase(strDeviceExportPath) != 0
							|| strOldPluginName.CompareNoCase(strDeviceExportPath) != 0) {
							
							DeviceImport::GetMonitor().TryRemoveRecordsForPlugin(strOldPluginName);
						}

						//start watching again
						// (j.jones 2010-11-02 14:51) - PLID 41189 - added DevicePatientMatchRule
						// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage (which is a boolean in the interface)
						// (j.jones 2011-03-10 14:10) - PLID 41349 - added NotificationRule
						// (b.savon 2011-9-23) - PLID 44954 - added bNewPlugin
						// (r.gonet 06/11/2013) - PLID 57127 - Added Image DPI
						// (b.savon 2014-12-03 14:20) - PLID 64184 - Add converter
						DeviceImport::GetMonitor().StartWatchingDeviceFolder(nPluginID, strPluginFileName, strPluginName, strDeviceExportPath,							
							(EDeviceNotificationRule)VarLong(pRow->GetValue(apcNotificationRule), (long)dpnrAutoPopup),
							(EDevicePatientMatchRule)VarLong(pRow->GetValue(apcDevicePatientMatchRule), (long)dpmrMatchByIDOnly),
							VarBool(pRow->GetValue(apcConvertPDFToImage), FALSE) ? cptiConvertToImage : cptiDoNotConvert, VarLong(pRow->GetValue(apcImageDPI), IMAGE_DPI_NULL), 
							(EPDFToImageConverter)VarLong(pRow->GetValue(apcPDFToImageConverter), (long)pticTwoPilots), TRUE);
					}

					// (b.savon 2011-9-9) - PLID 45314 - The active devices changed, reflect it on the import dialog filter
					DeviceImport::GetMonitor().BuildDeviceImportFilter();
					DeviceImport::GetMonitor().FilterImportList();
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CDeviceConfigTabDlg::SelSetActiveDevicePlugins(LPDISPATCH lpSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pSelRow(lpSel);

		//If a row is selected enable the delete button
		GetDlgItem(IDC_DELETE_PLUGIN)->EnableWindow(pSelRow != NULL);
		
	} NxCatchAll(__FUNCTION__);
}

void CDeviceConfigTabDlg::EditingFinishedActiveDevicePlugins(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);		

		if(pRow) {

			long nPluginID = VarLong(pRow->GetValue(apcPluginID));
			CString strDeviceExportPath = VarString(pRow->GetValue(apcDeviceExportPath), "");
			CString strPluginName = VarString(pRow->GetValue(apcPluginName), "");
			CString strPluginFileName = VarString(pRow->GetValue(apcPluginFileName), "");

			// (b.savon 2011-9-9) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
			//	Don't allow misc. spaces
			pRow->PutValue(apcPluginName, _bstr_t(strPluginName.Trim()));

			// (j.jones 2010-12-28 14:59) - PLID 41796 - track whether a change was made
			// that requires restarting watching the folder
			BOOL bNeedRestart = FALSE;

			switch (nCol) {

				case apcEnabled:
					// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
					if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
						VarBool(pRow->GetValue(apcEnabled)) ? pRow->PutValue(apcEnabled, VARIANT_FALSE) : pRow->PutValue(apcEnabled, VARIANT_TRUE);
						return;
					}

					//Need to update the table when the user checks or unchecks the enabled checkbox
					//Since its safe to say there will be unique .nxdp files we can update based
					//on the plugin filename
					ExecuteParamSql("UPDATE DevicePluginConfigT SET Enable = {BIT} WHERE PluginFileName = {STRING} AND ID = {INT}",
							VarBool(pRow->GetValue(apcEnabled), FALSE), strPluginFileName, nPluginID);

					// (j.jones 2010-06-02 09:02) - PLID 37976 - start or stop watching this folder
					if(VarBool(pRow->GetValue(apcEnabled), FALSE)) {
						// (j.jones 2010-12-28 15:00) - PLID 41796 - just set bNeedRestart to TRUE, and start later
						bNeedRestart = TRUE;
					}
					else {
						//stop watching
						DeviceImport::GetMonitor().CloseDeviceFolderMonitor(nPluginID);

						// (j.jones 2011-08-25 14:55) - PLID 41683 - remove any files currently in the import screen
						DeviceImport::GetMonitor().TryRemoveRecordsForPlugin(strPluginName);
					}

					// (b.savon 2011-9-9) - PLID 45314 - The active devices changed, reflect it on the import dialog filter
					if( DeviceImport::GetMonitor().GetDevicePluginDlg() ){
						DeviceImport::GetMonitor().BuildDeviceImportFilter();
						DeviceImport::GetMonitor().FilterImportList();
					}
					break;

				case apcPluginName:
					// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
					if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
						pRow->PutValue(apcPluginName, _bstr_t(m_strPluginNameOriginal));
						return;
					}

					// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
					// If the new name is the same as the original
					//	OR
					// If it isn't a valid plugin name (i.e. It is in use already)
					//	THEN
					// Keep the Same Plugin Name
					if( m_strPluginNameOriginal.Trim().CompareNoCase(strPluginName.Trim()) == 0 || 
						!IsValidPluginName( VarString(pRow->GetValue(apcPluginName)), TRUE ) ){
						pRow->PutValue(apcPluginName, _bstr_t(m_strPluginNameOriginal));
						return;
					}

					ExecuteParamSql("UPDATE DevicePluginConfigT SET PluginOverrideName = {STRING} WHERE PluginFileName = {STRING} AND ID = {INT}",
						VarString(pRow->GetValue(apcPluginName)), strPluginFileName, nPluginID);
					// (d.lange 2010-12-13 10:07) - PLID 41819 - Restart the plugin once we've changed the name
					// (j.jones 2010-12-28 15:00) - PLID 41796 - just set bNeedRestart to TRUE, and start later
					bNeedRestart = TRUE;

					// (j.jones 2011-08-25 14:55) - PLID 41683 - remove any files currently in the import screen
					// for the old plugin name
					{
						CString strOldPluginName = VarString(varOldValue);
						DeviceImport::GetMonitor().TryRemoveRecordsForPlugin(strOldPluginName);
					}
					//	The name was changed, let's update the filter in the import dialog
					if( DeviceImport::GetMonitor().GetDevicePluginDlg() ){
						DeviceImport::GetMonitor().BuildDeviceImportFilter();
					}
					break;

				// (j.jones 2011-03-10 14:10) - PLID 41349 - added NotificationRule
				case apcNotificationRule:
					// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
					if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
						return;
					}
					ExecuteParamSql("UPDATE DevicePluginConfigT SET NotificationRule = {INT} WHERE PluginFileName = {STRING} AND ID = {INT}",
							VarLong(pRow->GetValue(apcNotificationRule), (long)dpnrAutoPopup), strPluginFileName, nPluginID);
					bNeedRestart = TRUE;
					break;

				// (j.jones 2010-11-02 14:19) - PLID 41189 - added DevicePatientMatchRule
				case apcDevicePatientMatchRule:
					// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
					if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
						return;
					}
					ExecuteParamSql("UPDATE DevicePluginConfigT SET DevicePatientMatchRule = {INT} WHERE PluginFileName = {STRING} AND ID = {INT}",
							VarLong(pRow->GetValue(apcDevicePatientMatchRule), (long)dpmrMatchByIDOnly), strPluginFileName, nPluginID);
					bNeedRestart = TRUE;
					break;

				// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
				case apcConvertPDFToImage:
					// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
					if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
						VarBool(pRow->GetValue(apcConvertPDFToImage)) ? pRow->PutValue(apcConvertPDFToImage, VARIANT_FALSE) : pRow->PutValue(apcConvertPDFToImage, VARIANT_TRUE);
						return;
					}

					// (r.gonet 06/11/2013) - PLID 57127 - The value of Convert PDF To Image has changed. The editability of Image DPI depends on
					//  whether Convert is checked.
					if(!VarBool(pRow->GetValue(apcConvertPDFToImage))) {
						// (r.gonet 06/11/2013) - PLID 57127 - Clear out the Image DPI and make it non-editable.
						NXDATALIST2Lib::IFormatSettingsPtr pFormatSettings(__uuidof(NXDATALIST2Lib::FormatSettings));
						pFormatSettings->Editable = VARIANT_FALSE;
						pRow->CellFormatOverride[apcImageDPI] = pFormatSettings;
						pRow->PutValue(apcImageDPI, g_cvarNull);
					} else {
						// (r.gonet 06/11/2013) - PLID 57127 - Set the Image DPI to a {Auto} and remove any override preventing the user from editing the Image DPI.
						pRow->CellFormatOverride[apcImageDPI] = NULL;
						pRow->PutValue(apcImageDPI, _variant_t(IMAGE_DPI_AUTO, VT_I4));
					}

					//remember that this is treated as a boolean in the interface, despite being stored as an int
					// (r.gonet 06/11/2013) - PLID 57127 - Also initialize the Image DPI to the proper value.
					ExecuteParamSql("UPDATE DevicePluginConfigT SET ConvertPDFToImage = {INT}, ImageDPI = {VT_I4} WHERE PluginFileName = {STRING} AND ID = {INT}",
						VarBool(pRow->GetValue(apcConvertPDFToImage), FALSE) ? cptiConvertToImage : cptiDoNotConvert, 
						pRow->GetValue(apcImageDPI).vt == VT_I4 ? pRow->GetValue(apcImageDPI) : g_cvarNull,
						strPluginFileName, nPluginID);
					bNeedRestart = TRUE;
					break;

				case apcImageDPI:
					// (r.gonet 06/11/2013) - PLID 57127 - If the plugin isn't done processing, reset the value
					if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ) {
						pRow->PutValue(apcImageDPI, varOldValue);
						return;
					}

					if( !VariantEqual(varOldValue, varNewValue) ){
						_variant_t varImageDPI = VarLong(varNewValue, IMAGE_DPI_NULL) == IMAGE_DPI_NULL ? g_cvarNull : varNewValue;

						// (r.gonet 06/11/2013) - PLID 57127 - Save the selection
						ExecuteParamSql(
							"UPDATE DevicePluginConfigT SET ImageDPI = {VT_I4} WHERE PluginFileName = {STRING} AND ID = {INT}",
							varImageDPI, strPluginFileName, nPluginID
						);
						if( DeviceImport::GetMonitor().GetDevicePluginDlg() ){
							DeviceImport::GetMonitor().BuildDeviceImportFilter();
						}
						// (r.gonet 06/11/2013) - PLID 57127 - Restart the monitor so we can use the correct default DPI.
						bNeedRestart = TRUE;
					}

					break;

				// (b.savon 2014-12-03 15:11) - PLID 64184 - Add a new embedded drop down column to the Links->Devices configured devices datalist to support choosing a default PDF->Image converter.
				case apcPDFToImageConverter:
				{
					if (!DeviceImport::GetMonitor().IsPluginProcessComplete()) {
						pRow->PutValue(apcPDFToImageConverter, varOldValue);
						return;
					}

					if (!VariantEqual(varOldValue, varNewValue)){
						bool bConverter = (EPDFToImageConverter)VarLong(varNewValue, (long)pticTwoPilots) == pticTwoPilots ? false : true; 

						ExecuteParamSql(
							"UPDATE DevicePluginConfigT SET PDFToImageConverter = {BOOL} WHERE PluginFileName = {STRING} AND ID = {INT}",
							bConverter, strPluginFileName, nPluginID
						);

						if (DeviceImport::GetMonitor().GetDevicePluginDlg()){
							DeviceImport::GetMonitor().BuildDeviceImportFilter();
						}

						bNeedRestart = TRUE;
					}
				}
				break;

				// (b.savon 2013-05-23 15:44) - PLID 42902
				case apcDefaultCategory:
					{
						//If the plugin isn't done processing, reset the value
						if(!DeviceImport::GetMonitor().IsPluginProcessComplete()){
							pRow->PutValue(apcDefaultCategory, varOldValue);
							return;
						}

						//Only do this if the new value is different than the old
						if( !VariantEqual(varOldValue, varNewValue) ){
							//Save the selection
							ExecuteParamSql(
								"UPDATE DevicePluginConfigT SET DefaultCategoryID = {VT_I4} WHERE PluginFileName = {STRING} AND ID = {INT}",
								AsLong(varNewValue) == 0 ? g_cvarNull : AsLong(varNewValue), strPluginFileName, nPluginID
							);
							pRow->PutValue(apcDefaultCategory, AsLong(varNewValue) == 0 ? g_cvarNull : varNewValue);

							if( DeviceImport::GetMonitor().GetDevicePluginDlg() ){
								DeviceImport::GetMonitor().BuildDeviceImportFilter();
							}
						}
					}
					break;

				default:
					break;				
			}

			// (j.jones 2010-12-28 15:00) - PLID 41796 - if we changed something that requires a restart of the folder
			// monitor, do so now, provided that the plugin is enabled
			// (j.jones 2011-03-10 14:10) - PLID 41349 - added NotificationRule
			// (b.savon 2011-9-23) - PLID 44954 - added bNewPlugin
			// (r.gonet 06/11/2013) - PLID 57127 - Added Image DPI
			// (b.savon 2014-12-03 15:04) - PLID 64184 - Added converter
			if(VarBool(pRow->GetValue(apcEnabled), FALSE) && bNeedRestart) {
				DeviceImport::GetMonitor().StartWatchingDeviceFolder(nPluginID, strPluginFileName, strPluginName, strDeviceExportPath,
					(EDeviceNotificationRule)VarLong(pRow->GetValue(apcNotificationRule), (long)dpnrAutoPopup),
					(EDevicePatientMatchRule)VarLong(pRow->GetValue(apcDevicePatientMatchRule), (long)dpmrMatchByIDOnly),
					VarBool(pRow->GetValue(apcConvertPDFToImage), FALSE) ? cptiConvertToImage : cptiDoNotConvert, VarLong(pRow->GetValue(apcImageDPI), IMAGE_DPI_NULL), 
					(EPDFToImageConverter)VarLong(pRow->GetValue(apcPDFToImageConverter), (long)pticTwoPilots), TRUE);
				// (b.savon 2011-9-9) - PLID 45314 - The active devices changed, reflect it on the import dialog filter
				DeviceImport::GetMonitor().FilterImportList();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CDeviceConfigTabDlg::RequeryFinishedActiveDevicePlugins(short nFlags)
{
	try {
		CString strPluginFileName = "";
		CString strPluginPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";
		CDevicePlugin *pPlugin;

		IRowSettingsPtr pRow = m_pActivePluginsList->GetFirstRow();

		// (r.gonet 2016-05-20 3:39) - NX-100691 - Use the client's machine name.
		_RecordsetPtr rs = CreateParamRecordset("SELECT PluginFileName, PluginOverrideName FROM DevicePluginConfigT "
									"WHERE ComputerName = {STRING};\r\n", 
			g_propManager.GetSystemName());

		//We need to give all the active plugins their "clean" name
		while(pRow) {
			if (!rs->eof) {
				CString strOverrideName = AdoFldString(rs, "PluginOverrideName", "");
				strPluginFileName = pRow->GetValue(apcPluginFileName);
				
				pPlugin = new CDevicePlugin(strPluginPath, strPluginFileName);
				if (strOverrideName.IsEmpty()) {
					pRow->PutValue(apcPluginName, _bstr_t(pPlugin->GetPluginDescription()));
				}else {
					pRow->PutValue(apcPluginName, _bstr_t(strOverrideName));
				}
				pRow->PutValue(apcLaunchSetting, (int)pPlugin->GetLaunchDeviceSettings());
				delete pPlugin;

				rs->MoveNext();
			}

			// (r.gonet 06/11/2013) - PLID 57127 - After requerying, make the Image DPI column uneditable and cleared for
			//  any row that doesn't have Convert PDF to Image checked.
			if(!VarBool(pRow->GetValue(apcConvertPDFToImage))) {
				NXDATALIST2Lib::IFormatSettingsPtr pFormatSettings(__uuidof(NXDATALIST2Lib::FormatSettings));
				pFormatSettings->Editable = VARIANT_FALSE;
				pRow->CellFormatOverride[apcImageDPI] = pFormatSettings;
				pRow->PutValue(apcImageDPI, g_cvarNull);
			} else {
				// (r.gonet 06/11/2013) - PLID 57127 - Leave the image DPI column alone. It is by default enabled.
			}

			pRow = pRow->GetNextRow();
		}
		rs->Close();

	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
//	SAVE the original plugin name so we can compare it when the user is done editing the cell.
void CDeviceConfigTabDlg::EditingStartingActiveDevicePlugins(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
		MessageBox("Please wait until all the plugin files are finished processing.", "Device Plugin Files Processing...", MB_ICONWARNING|MB_OK);
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);	
		m_strPluginNameOriginal = VarString(pRow->GetValue(apcPluginName));
		return;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);	
	m_strPluginNameOriginal = VarString(pRow->GetValue(apcPluginName));

	// (b.savon 2013-05-29 15:48) - PLID 42902 - Requery just the combo drop down in case something changed. (i.e. you add/remove a category after you
	// load the plugin config dialog)
	// (r.gonet 06/11/2013) - PLID 57127 - Fixed a bug where we were using the wrong enumeration type.
	IColumnSettingsPtr pCol = m_pActivePluginsList->GetColumn(apcDefaultCategory);
	pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF UNION SELECT NULL, '{No Category}' AS Description ORDER BY Description"));
}

// (b.savon 2011-09-23 11:48) - PLID 44954 - Don't allow modifications to the plugins while there are files processing.
void CDeviceConfigTabDlg::EditingStartedActiveDevicePlugins(LPDISPATCH lpRow, short nCol, long nEditType)
{
	if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
		//MessageBox("Please wait until all the plugin files are finished processing.", "Device Plugin Files Processing...", MB_ICONWARNING|MB_OK);
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);	
		pRow->PutSelected(VARIANT_TRUE);
		::SetFocus(NULL);
		return;
	}
}

// DevicePluginImportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "DevicePluginImportDlg.h"
#include "DevicePatientSelectDlg.h"
#include "AdministratorRc.h"
#include "GlobalParamUtils.h"
#include "GlobalUtils.h"
#include "PicContainerDlg.h"
#include "FileUtils.h"
#include "EmrUtils.h"
#include "EMRItemAdvPopupDlg.h"
#include "InternationalUtils.h"
#include "InvVisionWebUtils.h"
#include "DeviceImportCombineFilesDlg.h"
#include "TaskEditDlg.h"
#include "TodoUtils.h"
#include "DeviceImportMonitor.h"
#include "EMRTopic.h"
#include "EMR.h"
#include "EMNDetail.h"
#include "EMN.h"
#include "CCDInterface.h"
#include <NxXMLUtils.h>
#include "ReconciliationDlg.h"

// (a.walling 2013-04-11 17:05) - PLID 56225 - Move DeviceImport stuff into its own class

// (b.savon 2011-9-9) - PLID 45314 - Provide an ability to ilter the device integration list per-device plugin setup.
#include "DeviceConfigTabDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#define IDM_GO_TO_PATIENT		49014
#define IDM_REMOVE				49015 
#define IDM_UNSELECT_PATIENT	49016	//(e.lally 2011-04-06) PLID 42734
#define IDM_SPLIT_PDF			49017	//(e.lally 2011-05-24) PLID 42819

using namespace NXDATALIST2Lib;
using namespace ADODB;
using namespace DevicePluginUtils;
using namespace NxXMLUtils;
using namespace CCD; 

// (d.lange 2010-05-24 15:34) - PLID 38850 - Created
// CDevicePluginImportDlg dialog

// (r.gonet 06/11/2013) - PLID 56370 - Moved the constructor implementation from the header to the cpp file.
ChildRowRecord::ChildRowRecord() {
	bTableChecked = FALSE;
	bFileChecked = FALSE;
	nParentPointer = -1;
	nCategoryID = -1;
	strFullFilePath = "";
	strFileDescriptionOverride = "";
	eDevicePatientMatchRule = dpmrMatchByIDOnly;
	eConvertPDFToImage = cptiDoNotConvert;
	// (r.gonet 06/10/2013) - PLID 56370 - Added Image DPI. Set to NULL by default.
	nImageDPI = IMAGE_DPI_NULL;
	reRecordElement = NULL;
	// (j.jones 2011-03-10 16:13) - PLID 42329 - added dtFileTime
	dtFileTime = COleDateTime::GetCurrentTime();
	// (d.lange 2011-05-20 11:12) - PLID 43253 - Addded ToDo Task ID
	nToDoTaskID = -1;
	// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter - Initialize to TwoPilots
	eConverter = pticTwoPilots;
}

// (r.gonet 06/11/2013) - PLID 56370 - Moved the constructor implementation from the header to the cpp file.
// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
ChildRowRecord::ChildRowRecord(DevicePluginUtils::RecordElement *reRecordElement, EDevicePatientMatchRule eDevicePatientMatchRule, EConvertPDFToImage eConvertPDFToImage, long nImageDPI, long nParentPointer, EPDFToImageConverter eConverter) {
	// (b.savon 2015-07-17 10:23) - PLID 66569 - Initialize the bTableChecked and bFileChecked to FALSE to avoid erroneous defaults
	this->bTableChecked = FALSE;
	this->bFileChecked = FALSE;
	this->strFullFilePath = reRecordElement->strFullFilePath;
	this->strFileDescriptionOverride = reRecordElement->strFileDescriptionOverride;
	this->eDevicePatientMatchRule = eDevicePatientMatchRule;
	this->eConvertPDFToImage = eConvertPDFToImage;
	// (r.gonet 06/10/2013) - PLID 56370 - Added Image DPI
	this->nImageDPI = nImageDPI;
	this->reRecordElement = reRecordElement;
	this->nParentPointer = nParentPointer;
	// (j.jones 2011-03-10 16:13) - PLID 42329 - added dtFileTime
	this->dtFileTime = reRecordElement->dtFileTime;
	this->nCategoryID = -1;
	this->nToDoTaskID = -1;
	// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter - Initialize to TwoPilots
	this->eConverter = eConverter;
}

// (r.gonet 06/11/2013) - PLID 56370 - Moved the constructor implementation from the header to the cpp file.
ParentRowRecord::ParentRowRecord() 
{
	pePatientElement = NULL;
	strPluginName = "";
	strPluginFileName = "";
	eDevicePatientMatchRule = dpmrMatchByIDOnly;
	eConvertPDFToImage = cptiDoNotConvert;
	// (r.gonet 06/10/2013) - PLID 56370 - Added Image DPI. Set to NULL by default.
	nImageDPI = IMAGE_DPI_NULL;
	eRowStatusColor = rscUnselectedPatient;
	bChecked = FALSE;
	nPatientID = -1;
	nPatientUserDefinedID = -1;
	// (j.jones 2011-03-10 16:13) - PLID 42329 - added dtFileTime
	COleDateTime dtFileTime = COleDateTime::GetCurrentTime();
	// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter - Initialize to TwoPilots
	eConverter = pticTwoPilots;
}

// (r.gonet 06/11/2013) - PLID 56370 - Moved the constructor implementation from the header to the cpp file.
// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
ParentRowRecord::ParentRowRecord(DevicePluginUtils::PatientElement *pePatientElement, const CString& strPluginName, const CString& strPluginFileName, EDevicePatientMatchRule eDevicePatientMatchRule, EConvertPDFToImage eConvertPDFToImage, long nImageDPI, EPDFToImageConverter eConverter) 
{
	// (b.savon 2015-07-17 10:23) - PLID 66569 - Initialize bChecked to FALSE to avoid erroneous defaults
	this->bChecked = FALSE;
	this->pePatientElement = pePatientElement;
	this->strPluginName = strPluginName;
	this->strPluginFileName = strPluginFileName;
	this->eDevicePatientMatchRule = eDevicePatientMatchRule;
	this->eConvertPDFToImage = eConvertPDFToImage;
	// (r.gonet 06/10/2013) - PLID 56370 - Added Image DPI.
	this->nImageDPI = nImageDPI;
	this->nPatientUserDefinedID = (pePatientElement->strPatientID.IsEmpty() ? -1 : atoi(pePatientElement->strPatientID));
	this->nPatientID = -1;
	// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
	this->eConverter = eConverter;

	// (j.jones 2011-03-10 16:13) - PLID 42329 - added dtFileTime, which is the earliest file time in the records we have
	COleDateTime dtMin = g_cdtInvalid;

	for(int i = 0; i < pePatientElement->aryRecords.GetSize(); i++) {
		// (b.savon 2014-12-03 14:12) - PLID 64186 - Add the converter
		ChildRowRecord *crChildRowRecord = new ChildRowRecord((DevicePluginUtils::RecordElement*)pePatientElement->aryRecords.GetAt(i), eDevicePatientMatchRule, eConvertPDFToImage, nImageDPI, (long)this, eConverter);
		this->aryChildRowRecords.Add(crChildRowRecord);

		// (j.jones 2011-03-10 16:24) - PLID 42329 - track the earliest time
		COleDateTime dtCur = ((DevicePluginUtils::RecordElement*)pePatientElement->aryRecords.GetAt(i))->dtFileTime;
		if(dtMin.GetStatus() == COleDateTime::invalid) {
			dtMin = dtCur;
		}
		else if(dtCur < dtMin) {
			dtMin = dtCur;
		}
		
		//use the current time if our min is invalid
		if(dtMin.GetStatus() == COleDateTime::invalid) {
			this->dtFileTime = COleDateTime::GetCurrentTime();
		}
		else {
			this->dtFileTime = dtMin;
		}
	}
}

IMPLEMENT_DYNAMIC(CDevicePluginImportDlg, CNxDialog)

CDevicePluginImportDlg::CDevicePluginImportDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CDevicePluginImportDlg::IDD, pParent)
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
	m_pPDF = NULL;
	m_pBCLPDF = NULL;
	m_bHasLoadedPDF = FALSE;
	m_eSortOrder = disoDateAsc;
	m_pBrowser = NULL;
	// (b.savon 2012-02-01 13:18) - PLID 47651 - Save the current row were working on
	m_strSelectedFilePath = "";
	// (b.savon 2012-02-20 14:59) - PLID 46455 - Keep track of column widths for window resize
	m_nOriginalListWidth = -1;
	m_nOriginalColumnWidth = -1;
	// (b.savon 2012-03-26 17:50) - PLID 49143 - Minimize Preference
	m_muipMinimizePreference = muipNever;

	// (j.armen 2012-06-06 12:39) - PLID 50830 - Set min size
	SetMinSize(930, 630);
}

CDevicePluginImportDlg::~CDevicePluginImportDlg()
{
	try{
		// (b.savon 2012-02-16 16:37) - PLID 46456 - Restructure
		Cleanup();

		// (j.jones 2011-04-26 10:12) - PLID 43439 - destroy our cached icons
		if(m_hIconPreview != NULL) {
			DestroyIcon((HICON)m_hIconPreview);
		}
	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_IMPORT_TO_HISTORY, m_btnImportHistory);
	DDX_Control(pDX, IDC_BTN_IMPORT_TO_EMR, m_btnImportEMR);
	DDX_Control(pDX, IDC_DEVICE_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_CHK_REMOVEFILES, m_checkAlwaysDelete);
	DDX_Control(pDX, IDC_CHK_CURPATIENT_FILTER, m_chkCurPatientFilter);
	DDX_Control(pDX, IDC_DEVICE_COMBINE_FILES_BTN, m_btnCombinePdfFiles);
	DDX_Control(pDX, IDC_TOGGLE_FILE_PREVIEW, m_btnTogglePreview);
	DDX_Control(pDX, IDC_BTN_REFRESH_DI, m_btnRefresh);
}

BOOL CDevicePluginImportDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// (b.savon 2012-04-23 11:31) - PLID 49891 - Add the device icon to the title bar
		HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DEVICE_IMPORT));
		SetIcon(hIcon, TRUE);		// Set big icon
		SetIcon(hIcon, FALSE);		// Set small icon

		//Set controls
		m_btnImportHistory.AutoSet(NXB_OK);
		m_btnImportEMR.AutoSet(NXB_OK);
		m_btnClose.AutoSet(NXB_CLOSE);
		// (b.savon 2012-03-12 14:43) - PLID 48816 - Manual refresh button
		m_btnRefresh.AutoSet(NXB_REFRESH);

		//(e.lally 2011-04-06) PLID 42733
		m_btnCombinePdfFiles.AutoSet(NXB_MODIFY);

		m_pList = BindNxDataList2Ctrl(IDC_IMPORT_LIST, false);

		// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
		m_pDetectedDevicePluginCombo = BindNxDataList2Ctrl(IDC_DEVICE_PLUGIN_SELECT, false);
		GetDlgItem(IDC_DEVICE_PLUGIN_SELECT)->SetWindowPos(&CWnd::wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		BuildDevicePluginsCombo();

		//TES 4/21/2011 - PLID 43252 - Initialize our web browser for previewing attached files.
		m_pBrowser = GetDlgItem(IDC_FILE_PREVIEW)->GetControlUnknown();
		//TES 4/21/2011 - PLID 43252 - Now set it to blank, and disable it.
		COleVariant varUrl("about:blank");
		if (m_pBrowser) {
			m_pBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
			m_pBrowser->Navigate2(varUrl, COleVariant((long)navNoHistory), NULL, NULL, NULL);
			GetDlgItem(IDC_FILE_PREVIEW)->EnableWindow(FALSE);
		}
		m_strCurrentPreviewFile = "about:blank";

		// (j.jones 2011-03-11 14:46) - PLID 42328 - bulk cache preferences in one function,
		// as it can be called multiple times
		CachePreferences();
		
		//The icon for previewing EMR tables
		m_hIconPreview = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16,16, 0);

		IColumnSettingsPtr pCol = m_pList->GetColumn(dlcCategory);
		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		// (b.savon 2013-05-23 15:29) - PLID 42902 - HB said this shouldn't be filtered on permissioned categories.  The reason of permissioning the
		// categories is so that the user can view the documents in the history tab.  They should, however, be able to import it into that category.
		pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF UNION SELECT NULL, '{No Category}' AS Description ORDER BY Description"));

		// (b.savon 2012-02-20 16:09) - PLID 46455 - Update the description to match our mutiple patient matching preferences
		CString strDescText =   "An external device has detected the files listed below.  "
								"Practice will match the patient based on the configured plugin matching preference "
								"or allow for selecting a patient manually by clicking in the 'Patient Name' column.  "
								"Please select which files you would like to import into a patient's history tab or an open EMR.";
		SetDlgItemText(IDC_TEXT_DESC, strDescText);

		CString strMoreDesc = "Items flagged in red were not able to be matched with a Practice patient and require manually selecting a patient.  "
								"Items flagged in yellow have been matched with a Practice patient.  Items flagged in green have been manually "
								"selected or verified by the user.";
		SetDlgItemText(IDC_STATIC_DESC, strMoreDesc);

		// (d.lange 2010-10-20 15:37) - PLID 41006 - Load the cached state of the "Always remove files that have not been imported" checkbox
		//By default, we want to set this to be off
		m_checkAlwaysDelete.SetCheck(GetRemotePropertyInt("RemoveNonImportedDeviceFiles", 0, 0, GetCurrentUserName(), true) ? BST_CHECKED : BST_UNCHECKED);

		// (d.lange 2011-03-10 09:25) - PLID 41010 - Load the cached state for the checkbox that filters on the current patient
		m_chkCurPatientFilter.SetCheck((GetRemotePropertyInt("DeviceImport_ChkCurrentPatientFilter", 0, 0, GetCurrentUserName(), true) == 1 ? BST_CHECKED : BST_UNCHECKED));

		// (j.jones 2011-03-11 12:59) - PLID 42328 - load the preference for sort order
		//1 - date ascending (default), 2 - date descending, 3 - patient name
		m_eSortOrder = (EDeviceImportSortOrder)GetRemotePropertyInt("DeviceImport_SortOrder", (long)disoDateAsc, 0, GetCurrentUserName(), true);
		ApplySortOrder();

		//TES 4/21/2011 - PLID 43252 - Recall whether we're showing the preview window
		m_btnTogglePreview.SetCheck(GetRemotePropertyInt("DeviceImport_ShowPreview", 1, 0, GetCurrentUserName()) ? BST_CHECKED : BST_UNCHECKED);
		OnToggleFilePreview();

		// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device 
		//									 integration list per-device plugin setup 
		m_strSelectedPlugin = "< All Enabled Plugins >";
		
		// (b.savon 2011-09-26 14:39) - PLID 42330 - Load preference and set 'Hide EMR Tables' accordingly.
		((CButton*)GetDlgItem(IDC_CHK_HIDE_EMR_TABLES))->SetCheck(GetRemotePropertyInt("DeviceImport_HideEMRTable", BST_CHECKED, 0, GetCurrentUserName()) ? BST_CHECKED : BST_UNCHECKED);

		// (b.savon 2012-02-20 15:00) - PLID 46455 - Store the original column width for window resize
		// Get the width of the window and columns to use for a sizing ratio
		CRect rctDialog;
		::GetClientRect(this->m_hWnd, &rctDialog);
		m_nOriginalListWidth = rctDialog.Width();

		// Sum up the column widths
		long nTotal = 0;
		for (int i = 0; i < m_pList->ColumnCount; i++){
			IColumnSettingsPtr pCol = m_pList->GetColumn(i);
			long nCurWidth = pCol->GetStoredWidth();

			nTotal += nCurWidth;
		}

		//	Store total width
		m_nOriginalColumnWidth = nTotal; 

		// (b.savon 2012-03-26 16:42) - PLID 49143 - Device Import Minimize preference - Get and Set
		m_muipMinimizePreference = (EMinimizeUIPreference)(GetRemotePropertyInt("DeviceImport_MinimizePreference", 0, 0, GetCurrentUserName(), true));
		switch( m_muipMinimizePreference ){
			case muipAlways:
				((CButton*)GetDlgItem(IDC_RADIO_ALWAYS))->SetCheck(BST_CHECKED);
				break;
			case muipEmptyList:
				((CButton*)GetDlgItem(IDC_RADIO_EMPTY_LIST))->SetCheck(BST_CHECKED);
				break;
			case muipNever:
			default:
				((CButton*)GetDlgItem(IDC_RADIO_NEVER))->SetCheck(BST_CHECKED);
				break;
		}

		// Size the window to the last size it was
		{
			// Get the work area to make sure that wherever we put it, it's accessible
			CRect rcWork;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
			// Get the last size and position of the window
			CRect rcDialog;
			CString strBuffer = AfxGetApp()->GetProfileString("Settings", "DeviceImportWindowSize");
			if (strBuffer.IsEmpty() || _stscanf(strBuffer, "%d,%d,%d,%d", &rcDialog.left, &rcDialog.top, &rcDialog.right, &rcDialog.bottom) != 4) {
				// We couldn't get the registry setting for some reason
				CSize ptDlgHalf(1221/2, 801/2);
				CPoint ptScreenCenter(rcWork.CenterPoint());
				rcDialog.SetRect(ptScreenCenter - ptDlgHalf, ptScreenCenter + ptDlgHalf);
			} else {
				// (b.cardillo 2010-03-26 16:31) - PLID 37583
				// Translate from workspace coordinates (which are what we save in the registry) to screen coordinates
				rcDialog.OffsetRect(rcWork.TopLeft());
			}
			// Make sure if we put the dialog at rcDialog it's accessible (we consider 'accessible' 
			// to mean that the dialog title bar is visible vertically, and 1/3 visible horizontally)
			if (rcDialog.top+rcDialog.Height()/8<rcWork.bottom && rcDialog.top>rcWork.top &&
				rcDialog.left<rcWork.right-rcDialog.Width()/3 && rcDialog.right>rcWork.left+rcDialog.Width()/3) {
				// It's accessible so leave it
			} else {
				// It's not accessible so center it
				CSize ptDlgHalf(rcDialog.Width()/2, rcDialog.Height()/2);
				CPoint ptScreenCenter(rcWork.CenterPoint());
				rcDialog.SetRect(ptScreenCenter - ptDlgHalf, ptScreenCenter + ptDlgHalf);
			}
			// Move the window to its new position
			MoveWindow(rcDialog);
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CDevicePluginImportDlg, CNxDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BTN_IMPORT_TO_HISTORY, &CDevicePluginImportDlg::OnBnClickedBtnImportToHistory)
	ON_BN_CLICKED(IDC_BTN_IMPORT_TO_EMR, &CDevicePluginImportDlg::OnBnClickedBtnImportToEmr)
	ON_BN_CLICKED(IDC_DEVICE_CLOSE, &CDevicePluginImportDlg::OnBnClickedDeviceClose)
	ON_COMMAND(IDM_GO_TO_PATIENT, OnGoToPatient)
	ON_COMMAND(IDM_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_CHK_REMOVEFILES, &CDevicePluginImportDlg::OnBnClickedChkRemovefiles)
	ON_BN_CLICKED(IDC_CHK_CURPATIENT_FILTER, &CDevicePluginImportDlg::OnBnClickedChkCurrentPatientFilter)
	ON_BN_CLICKED(IDC_DEVICE_COMBINE_FILES_BTN, &CDevicePluginImportDlg::OnBnClickedDeviceCombineFilesBtn)
	ON_COMMAND(IDM_UNSELECT_PATIENT, OnUnselectPatient)
	ON_BN_CLICKED(IDC_TOGGLE_FILE_PREVIEW, &CDevicePluginImportDlg::OnToggleFilePreview)
	ON_WM_TIMER()
	ON_COMMAND(IDM_SPLIT_PDF, OnSplitPdf)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_CHK_HIDE_EMR_TABLES, &CDevicePluginImportDlg::OnBnClickedChkHideEmrTables)
	ON_BN_CLICKED(IDC_BTN_REFRESH_DI, &CDevicePluginImportDlg::OnBnClickedBtnRefreshDi)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RADIO_ALWAYS, &CDevicePluginImportDlg::OnBnClickedRadioAlways)
	ON_BN_CLICKED(IDC_RADIO_NEVER, &CDevicePluginImportDlg::OnBnClickedRadioNever)
	ON_BN_CLICKED(IDC_RADIO_EMPTY_LIST, &CDevicePluginImportDlg::OnBnClickedRadioEmptyList)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
void CDevicePluginImportDlg::BuildDevicePluginsCombo()
{
	try {
		//	Fresh start
		m_pDetectedDevicePluginCombo->Clear();

		//	Grab the plugins and if they are enabled on this computer
		// (b.savon 2013-05-23 16:52) - PLID 42902 - Added ID and DefaultCategoryID
		// (r.gonet 2016-05-20 3:39) - NX-100691 - Use the client's machine name.
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteData(),"SELECT ID, PluginFileName, PluginOverrideName, Enable, DefaultCategoryID FROM DevicePluginConfigT "
												"WHERE ComputerName = {STRING}", 
			g_propManager.GetSystemName());

		//	We want to create a map of all the plugins and if they are enabled
		//	so that we can reference it throughout this method.
		CMap<CString, LPCSTR, BOOL, BOOL> mEnabledPlugins;
		CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";
		BOOL bEnabled;
		CString strPluginFileName;
		//We need to find and populate the "Detected Device Plugins" with the clean plugin name
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		while( !rs->eof ){
			bEnabled = AdoFldBool(rs, "Enable", FALSE);
			strPluginFileName = AdoFldString(rs, "PluginOverrideName", "");
			if( strPluginFileName == "" ){
				strPluginFileName = AdoFldString(rs, "PluginFileName");
				//	Let's get the plugin description and store that with the enable bit
				strPluginFileName = CDevicePlugin(strPath, strPluginFileName).GetPluginDescription();
			}
			if( bEnabled ){
				mEnabledPlugins.SetAt(strPluginFileName, bEnabled);
				//Let's put the "clean" plugin name in the dropdown combo
				pRow = m_pDetectedDevicePluginCombo->GetNewRow();
				pRow->PutValue(dpcPluginName, _bstr_t(strPluginFileName));
				// (b.savon 2013-05-23 17:00) - PLID 42902 - Add the id and category id
				pRow->PutValue(dpcID, AdoFldLong(rs, "ID"));
				long nCatID = AdoFldLong(rs, "DefaultCategoryID", -1);
				pRow->PutValue(dpcDefaultCategoryID, nCatID == -1 ? g_cvarNull : nCatID);

				m_pDetectedDevicePluginCombo->AddRowAtEnd(pRow, NULL);
			}
			rs->MoveNext();
		}
		rs->Close();
		bEnabled = TRUE;

		//	Add an ALL feature to the top of the list
		pRow = m_pDetectedDevicePluginCombo->GetNewRow();
		pRow->PutValue(dpcPluginName, _bstr_t("< All Enabled Plugins >"));
		// (b.savon 2013-05-23 17:02) - PLID 42902 - Add a -1 ID for the all row
		pRow->PutValue(dpcID, (long)-1);
		m_pDetectedDevicePluginCombo->AddRowBefore(pRow, m_pDetectedDevicePluginCombo->GetFirstRow());

		CString strFilter = "< All Enabled Plugins >";
		try{
			//	Load filter for this User on this Computer
			strFilter = GetRemotePropertyText("DevicePluginImportFilter", "< All Enabled Plugins >", 0, GetCurrentUserComputerName());
		}NxCatchAll("void CDevicePluginImportDlg::BuildDevicePluginsCombo - Unable to Load Filter");

		//	Fail-safe
		if( !mEnabledPlugins.Lookup(strFilter, bEnabled) ){
			strFilter = "< All Enabled Plugins >";
			SetRemotePropertyText("DevicePluginImportFilter", strFilter, 0, GetCurrentUserComputerName());
		}

		//	Select the plugin we are filtering
		m_strSelectedPlugin = strFilter;
		m_pDetectedDevicePluginCombo->SetSelByColumn(dpcPluginName, _bstr_t(strFilter));
		m_pDetectedDevicePluginCombo->PutComboBoxText(_bstr_t(m_pDetectedDevicePluginCombo->GetCurSel()->GetValue(dpcPluginName)));
	} NxCatchAll(__FUNCTION__);
}//	END void CDevicePluginImportDlg::BuildDevicePluginsCombo()

// CDevicePluginImportDlg message handlers
//Given a parent row, this function will delete the files associated with the parent and then remove the rows. This includes the XML file associated as
//an identifier for most plugins
// (d.lange 2011-05-25 12:51) - PLID 43253 - Added a boolean for identifying whether its a manual deletion or the files have been imported
void CDevicePluginImportDlg::RemoveChildRowFiles(NXDATALIST2Lib::IRowSettingsPtr pParentRow, BOOL bManualRemove /*= FALSE*/)
{
	try {
		if(pParentRow) {
			//Need to make sure this is the parent row
			if(pParentRow->GetParentRow() == NULL) {
				// (d.lange 2010-10-25 16:40) - PLID 41006 - Changed the order of deletion, removing the row first, then removing the file				
				CString strParentRowFile = VarString(pParentRow->GetValue(dlcFilePath));

				NXDATALIST2Lib::IRowSettingsPtr pLastChildRow = pParentRow->GetLastChildRow();
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteNextRow(pParentRow, VARIANT_TRUE);

				//At this point we should be at the first child row if this parent has children
				//We want to iterate through the children and remove the associated files
				while(pRow) {
					NXDATALIST2Lib::IRowSettingsPtr pChildRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
					CString strFilePath = VarString(pRow->GetValue(dlcFilePath));
					
					// (d.lange 2011-05-20 10:16) - PLID 43253 - We should remove the ToDo alarms that were created for these files
					// We only want to remove the todo alarm if we've manually removed the rows
					if(bManualRemove) {
						long nToDoTaskID = VarLong(pRow->GetValue(dlcToDoAlarmID), -1);
						if(nToDoTaskID != -1)
							TodoDelete(nToDoTaskID);
					}

					m_pList->RemoveRow(pRow);

					if(!strFilePath.IsEmpty()/* && pePatient->strIdentifyingFilePath.CompareNoCase(strFilePath) != 0*/)
						DeleteFile(strFilePath);
					

					//If we're at the last child row then let's break out of the while loop because we're only removing for a single parent
					if(pLastChildRow != NULL)
						if(pRow->IsSameRow(pLastChildRow))
							break;
					
					pRow = pChildRow;
				}

				// (b.savon 2012-04-06 12:56) - PLID 41861 - Get the pointer from the array now, instead of directly from the DL
				CArray<CString, CString> aryParentRowFilePaths;
				ParseParentRowFilePath(strParentRowFile, aryParentRowFilePaths);
				ArrayParentRowPointers* aryPaRow = (ArrayParentRowPointers*)VarLong(pParentRow->GetValue(dlcPatientPointer));
				for( int i = 0; i < aryPaRow->aryParentRecordPointers.GetSize(); i++ ){
					DeleteParentRowRecord((ParentRowRecord*)(long)aryPaRow->aryParentRecordPointers.GetAt(i));
					strParentRowFile = aryParentRowFilePaths.GetAt(i);
					//Finally, remove the parent's file
					if(!strParentRowFile.IsEmpty())
						DeleteFile(strParentRowFile);
				}

				m_pList->RemoveRow(pParentRow);

				// (d.lange 2010-10-26 16:45) - PLID 41088 - We need to notify all open EMNs and History tab
				SendMessageToUpdateButtons();
			}
		}
		

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-10-20 15:37) - PLID 41006 - This function will only remove the given row and associated file
void CDevicePluginImportDlg::RemoveRowFile(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bLastChildRow)
{
	try {
		if(pRow) {
			//Grab the file path
			CString strFilePath = VarString(pRow->GetValue(dlcFilePath));
			// (d.lange 2011-03-11 11:45) - PLID 42754 - Since a RecordElement* can contain a file and table, we have to check to see which item
			// is being removed and update m_aryParentRowRecord accordingly
			TableContent *pTablePointer = (TableContent*)atoi(VarString(pRow->GetValue(dlcFilePath)));
			ChildRowRecord *crChildRowRecord = (ChildRowRecord*)(VarLong(pRow->GetValue(dlcPatientPointer)));
			if(pTablePointer) {
				//This is a table, we need to update m_aryParentRowRecords
				// (b.savon 2012-07-02 21:15) - PLID 41861 - In some edge cases, the record element's file path is not initialized.
				// We can short circuit it by checking if there is no . in the current row's file path.  EMR Tables, don't have a .
				// in them
				if(strFilePath.Find(".") == -1 || crChildRowRecord->reRecordElement->strFullFilePath.IsEmpty()) {
					DeleteChildRowRecord(crChildRowRecord);

					// (b.savon 2012-07-02 21:17) - PLID 41861 - We also want to remove the row from the list if it isn't the last and return
					// to the caller.  If it is the last child, continue down the logic and function as normal
					if(!bLastChildRow) {
						//Remove the row
						m_pList->RemoveRow(pRow);
						return;
					}
				}else {
					delete pTablePointer;
					crChildRowRecord->reRecordElement->pTableContent = NULL;
				}

			}else {
				//This is a file, we need to update m_aryParentRowRecords
				if(crChildRowRecord->reRecordElement->pTableContent == NULL) {
					DeleteChildRowRecord(crChildRowRecord);
				}else {
					crChildRowRecord->reRecordElement->strFullFilePath = "";
					crChildRowRecord->strFullFilePath = "";
				}
			}

			//If we're importing the last child row of a parent, we also want to remove the file associated with the parent too
			if(bLastChildRow) {
				RemoveChildRowFiles(pRow->GetParentRow());
				
			}else {
				if(!strFilePath.IsEmpty()) {
					//Delete the file from the export path
					DeleteFile(strFilePath);
				}

				// (b.savon 2012-04-05 17:58) - PLID 41861 - Grab the parent row pointer and and the child row number
				NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();				
				ArrayParentRowPointers* aryParentRowPointers = (ArrayParentRowPointers*)VarLong(pParentRow->GetValue(dlcPatientPointer));
				ParentRowRecord* prrParent = (ParentRowRecord*)aryParentRowPointers->aryParentRecordPointers.GetAt(0);
				long nRowNumber = pRow->CalcRowNumber();
				// If were showing the EMR tables, adjust the array index by 2
				if( (prrParent->strPluginFileName.CompareNoCase("ZeissFORUM.nxdp") == 0 ||
					 prrParent->strPluginFileName.CompareNoCase("Nidek.nxdp") == 0) &&
					((CButton*)GetDlgItem(IDC_CHK_HIDE_EMR_TABLES))->GetCheck() == BST_UNCHECKED){
					nRowNumber = nRowNumber / 2;
				}

				//Remove the row
				m_pList->RemoveRow(pRow);

				// (b.savon 2012-04-05 17:58) - PLID 41861 - Then, Delete the ParentRowRecord, and subsequently the file
				CArray<CString, CString> aryParentRowFilePaths;
				//	This now has a | delimeted string with all the matching XML parent records
				CString strParentFile = VarString(pParentRow->GetValue(dlcFilePath));
				ParseParentRowFilePath(strParentFile, aryParentRowFilePaths);

				// (b.savon 2012-07-02 21:21) - PLID 41861 - This is another Quantel case check.  we only want to continue the normal logic
				// If all the children in the group are checked.  If they aren't all checked, then we cleanup ourselves above when the last child
				// is removed.
				BOOL bAllChildrenChecked = TRUE;
				if( pParentRow ){
					NXDATALIST2Lib::IRowSettingsPtr pLastChild = pParentRow->GetLastChildRow();
					while( pLastChild != NULL && !pParentRow->IsSameRow(pLastChild) ){
						if( VarBool(pLastChild->GetValue(dlcChecked)) == FALSE ){
							bAllChildrenChecked = FALSE;
							break;
						}

						pLastChild = pLastChild->GetPreviousRow();
					}
				}

				// (b.savon 2012-07-02 21:22) - PLID 41861 - We only execute this if we aren't Quantel.  Otherwise, we get cleaned
				// up above.  Execute as normal if were zeiss plugin
				if( (prrParent->strPluginFileName.CompareNoCase("ZeissFORUM.nxdp") == 0 || prrParent->strPluginFileName.CompareNoCase("Nidek.nxdp") == 0)  
					&& !bAllChildrenChecked ){
					//	Get the correct file path from the | delimeted string based on the child row index.
					CString strActualParentFile = aryParentRowFilePaths.GetAt(nRowNumber);

					//Recreate Parent Row File Path, and set the parent row record with it (minus the the XML parent we just removed)
					CString strRecreate = "";
					for( int i = 0; i < aryParentRowFilePaths.GetSize(); i++ ){
						if( i == nRowNumber ){
							continue;
						}else{
							strRecreate += aryParentRowFilePaths.GetAt(i) + "|";
						}
					}
					strRecreate = strRecreate.Left(strRecreate.GetLength()-1);
					//Replace the new | delimited file string
					pParentRow->PutValue(dlcFilePath, _bstr_t(strRecreate));

					//Get the array, and then grab the ParentRowRecord based on the row id gotten above.  Delete the parent row record,
					//and then remove it from the array to keep our records consistent with the | delimeted file descriptor.
					ArrayParentRowPointers* aryParentRowPointers = (ArrayParentRowPointers*)VarLong(pParentRow->GetValue(dlcPatientPointer));
					DeleteParentRowRecord((ParentRowRecord*)(long)aryParentRowPointers->aryParentRecordPointers.GetAt(nRowNumber));
					aryParentRowPointers->aryParentRecordPointers.RemoveAt(nRowNumber);
					//Replace the new ParentRow array in the datalist for future removals.
					pParentRow->PutValue(dlcPatientPointer,(long) aryParentRowPointers);
					if(!strParentFile.IsEmpty()) {
						//Delete the file from the export path
						DeleteFile(strActualParentFile);
					}

					// (d.lange 2010-10-26 16:45) - PLID 41088 - We need to notify all open EMNs and History tab
					SendMessageToUpdateButtons();
				}
			}
		}
		

	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::OnBnClickedBtnImportToHistory()
{
	try {
		//Traverse through all the data rows for each patient
		IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);
		long nPatientID = -1;
		BOOL bFileImport = FALSE;
		CArray<ImportFile*, ImportFile*> aryImportToHistory;
		// (d.lange 2010-10-21 09:23) - PLID 41006 - Need to batch selected rows to be removed later
		CArray<IRowSettingsPtr, IRowSettingsPtr> aryImportedRows;
		
		// (d.lange 2010-11-19 13:48) - PLID 41493 - If the 'Always remove' checkbox has recently been checked and the number of checked rows is less than
		// the total number of rows, the user will get a prompt about the removal of unselected files.  Once the user acknowledges the change, the prompt
		// will no longer appear until they reset the checkbox.
		if(CheckChildRowsSelected() && IsDlgButtonChecked(IDC_CHK_REMOVEFILES) &&
				GetRemotePropertyInt("DeviceImport_AlwaysRemoveWarningSetting", 1, 0, GetCurrentUserName(), true) == 1) {
			if(IDNO == MsgBox(MB_YESNO, FormatString("There are some patient files that have not been selected to be imported. Those that have not been "
																"selected will be DELETED. Would you like to continue?\r\n\r\n"
																"Click YES to import the selected files and remove the unselected files.\r\n"
																"Click NO to cancel the import."))) {
				return;						
			}else {
				SetRemotePropertyInt("DeviceImport_AlwaysRemoveWarningSetting", 0, 0, GetCurrentUserName());	
			}
		}

		// (b.savon 2012-02-13 09:42) - PLID 46456 - Make sure the files exists as well.
		if( !IsFileListUpToDate() ){
			return;
		}

		while(pRow) {
			IRowSettingsPtr pChildRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);

			if(VarBool(pRow->GetValue(dlcChecked)) && pRow->GetParentRow() != NULL) {
				//If the user has checked this row then we want to import to history
				nPatientID = pRow->GetParentRow()->GetValue(dlcPatientID);
				//We may need to check the type of file in the future, as it could be TableContent*
				CString strFullPath = VarString(pRow->GetValue(dlcFilePath));
				TableContent *pTablePointer = (TableContent*)atoi(VarString(pRow->GetValue(dlcFilePath)));
				if(nPatientID != -1) {
					
					if(pTablePointer) {
						//The user has tried to import table data into the history tab, this is not supported!
						AfxMessageBox("You are not able to import an EMR table into the history tab.");
						break;
					}else if(!strFullPath.IsEmpty() /*&& IsImageFile(strFullPath)*/) {
						ImportFile *importFile = new ImportFile;

						importFile->nPatientID = nPatientID;
						importFile->strFilePath = strFullPath;
						importFile->nFileCategory = VarLong(pRow->GetValue(dlcCategory), -1);
						// (j.jones 2011-03-10 16:08) - PLID 42329 - added file time
						importFile->dtFileTime = VarDateTime(pRow->GetValue(dlcFileTime), COleDateTime::GetCurrentTime());
						// (j.jones 2010-11-02 10:15) - PLID 41188 - added device name
						// (j.jones 2010-12-15 08:44) - PLID 41598 - renamed this column
						// (j.jones 2011-08-25 15:15) - PLID 41683 - renamed to plugin name
						importFile->strDeviceName = VarString(pRow->GetParentRow()->GetValue(dlcPluginName));
						// (j.jones 2010-12-15 09:24) - PLID 41598 - added file description
						importFile->strFileDescription = VarString(pRow->GetValue(dlcFileDescriptionToUse));
						
						// (j.jones 2010-12-28 17:29) - PLID 41796 - added ConvertPDFToImage, only applies if it is a PDF
						CString strExtension = FileUtils::GetFileExtension(strFullPath);
						if(strExtension.CompareNoCase("pdf") == 0 && VarBool(pRow->GetValue(dlcConvertPDFToImage), FALSE)) {
							importFile->eConvertPDFToImage = cptiConvertToImage;
							// (r.gonet 06/11/2013) - PLID 56370 - A value or default to automatic if NULL (shouldn't be possible but handle if we do have it)
							importFile->nImageDPI = pRow->GetValue(dlcImageDPI).vt == VT_I4 ? VarLong(pRow->GetValue(dlcImageDPI)) : IMAGE_DPI_AUTO;
						}
						else {
							importFile->eConvertPDFToImage = cptiDoNotConvert;
							// (r.gonet 06/11/2013) - PLID 56370 - NULL placeholder.
							importFile->nImageDPI = IMAGE_DPI_NULL;
						}
						
						// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
						importFile->eConverter = (EPDFToImageConverter)VarLong(pRow->GetValue(dlcPDFToImageConverter), (long)pticTwoPilots);

						// (d.lange 2011-05-20 11:28) - PLID 43253 - added ToDo Task ID
						importFile->nToDoTaskID = VarLong(pRow->GetValue(dlcToDoAlarmID), -1);

						//We'll add this to an array for importing later because the user may decide not to import
						aryImportToHistory.Add(importFile);
						// (d.lange 2010-10-21 09:24) - PLID 41006 - Add the checked row to the array
						aryImportedRows.Add(pRow);

						bFileImport = TRUE;
					}
					
				}else {
					//We were not able to match the returned device demographics to a practice patient, so we need to notify the user
					AfxMessageBox("You must first select a patient before importing to the history tab.");
					break;
				}
			}else if(pRow->GetParentRow() == NULL) {
				bFileImport = FALSE;
			}

			if(pRow->GetParentRow() != NULL) {
				NXDATALIST2Lib::IRowSettingsPtr pLastChildRow = pRow->GetParentRow()->GetLastChildRow();
				//If we are at the last child row of a patient and we've imported a file, let's remove the patient tree and remove the files
				//from the monitored directory
				if(pRow->IsSameRow(pLastChildRow) == -1 && bFileImport) {
					//TES 4/22/2011 - PLID 43252 - At this point we know we're probably removing a file, so clear out the preview just in case
					// it's locked the file we're planning to remove.
					ClearPreview();
					// (j.jones 2010-11-02 11:06) - PLID 41189 - Renamed to FindPatient, which may
					// end up matching by demographics instead of ID, based on eDevicePatientMatchRule.
					// nOverrideUserDefinedID is used if the row has already been linked to a patient.
					long nOverrideUserDefinedID = VarLong(pRow->GetParentRow()->GetValue(dlcUserDefinedID), -1);
					// (b.savon 2012-04-06 11:32) - PLID 41861 - Reflect the new Array of Pointers storage
					ArrayParentRowPointers* prArrayParentRecord = (ArrayParentRowPointers*)VarLong(pRow->GetParentRow()->GetValue(dlcPatientPointer));
					ParentRowRecord *peParentRecord = (ParentRowRecord*)prArrayParentRecord->aryParentRecordPointers.GetAt(0);
					if(FindPatient(peParentRecord->pePatientElement, (EDevicePatientMatchRule)VarLong(pRow->GetParentRow()->GetValue(dlcDevicePatientMatchRule)), nOverrideUserDefinedID) != -1) {
						//At this point we are at the last child row of a parent and we've imported at least one file, so at this point in time
						//we are going to prompt the user that we are removing all files associated with the patient

						if(aryImportToHistory.GetSize() > 0) {
							
							for(int i = 0; i < aryImportToHistory.GetSize(); i++) {

								// (j.jones 2010-11-02 09:25) - PLID 41188 - send in a clean description of the attached file,
								// stating the device it was imported from
								// (j.jones 2010-12-15 09:20) - PLID 41598 - we now have optional file descriptions that devices
								// can send to us, use those in history if available
								CString strFileDescriptionForHistory;
								if(!aryImportToHistory[i]->strFileDescription.IsEmpty()) {
									//we have a description, use it
									strFileDescriptionForHistory.Format("%s - Imported from device: %s", aryImportToHistory[i]->strFileDescription, aryImportToHistory[i]->strDeviceName);
								}
								else {
									//no description given, just list the device name
									strFileDescriptionForHistory.Format("Imported from device: %s", aryImportToHistory[i]->strDeviceName);
								}

								// (j.jones 2010-12-29 17:45) - PLID 41796 - if this is a PDF and they want to convert it to an image,
								// now is the time to do so
								CString strExtension = FileUtils::GetFileExtension(aryImportToHistory[i]->strFilePath);
								if(strExtension.CompareNoCase("pdf") == 0
									&& aryImportToHistory[i]->eConvertPDFToImage == cptiConvertToImage
									&& IsPDFConverterLoaded()) {
									
									//convert to image(s)

									//if IsPDFConverterLoaded() failed, the user would be warned one time only (per session)
									//that the PDF conversion will not work

									// (j.jones 2011-03-10 16:26) - PLID 42329 - pass in the file time as the service date
									// (r.gonet 06/11/2013) - PLID 56370 - Pass in the image DPI
									// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
									if(!ConvertAndImportPDF(aryImportToHistory[i]->strFilePath, aryImportToHistory[i]->nImageDPI, aryImportToHistory[i]->nPatientID, aryImportToHistory[i]->nFileCategory,
										strFileDescriptionForHistory, FALSE, aryImportToHistory[i]->strDeviceName, NULL, -1,
										aryImportToHistory[i]->dtFileTime, aryImportToHistory[i]->nToDoTaskID, aryImportToHistory[i]->eConverter)) {
										
										// (j.jones 2010-12-30 15:41) - PLID 41960 - if this failed, abort the entire import,
										// but only after cleaning up our memory objects
										for(int j = 0; j < aryImportToHistory.GetSize(); j++) {
											ImportFile *ifRemoval = aryImportToHistory[j];
											delete ifRemoval;
										}
										aryImportToHistory.RemoveAll();
										return;
									}																	
								}								
								else {
									long nMailID = -1; 
									// (j.jones 2010-12-30 12:18) - PLID 41796 - moved this to its own function
									// (j.jones 2011-03-10 16:26) - PLID 42329 - pass in the file time as the service date
									if(!ImportSingleFile(aryImportToHistory[i]->strFilePath, aryImportToHistory[i]->nPatientID, aryImportToHistory[i]->nFileCategory,
										strFileDescriptionForHistory, FALSE, aryImportToHistory[i]->strDeviceName, NULL, -1,
										aryImportToHistory[i]->dtFileTime, aryImportToHistory[i]->nToDoTaskID, &nMailID)) {
										
										// (j.jones 2010-12-30 15:41) - PLID 41960 - if this failed, abort the entire import,
										// but only after cleaning up our memory objects
										for(int j = 0; j < aryImportToHistory.GetSize(); j++) {
											ImportFile *ifRemoval = aryImportToHistory[j];
											delete ifRemoval;
										}
										aryImportToHistory.RemoveAll();
										return;
									}
									// (b.spivey, October 30, 2015) PLID 67423 - reconcile ccdas here. 
									else if ((aryImportToHistory[i]->strFilePath.GetLength() >= 4 && aryImportToHistory[i]->strFilePath.Right(4).CompareNoCase(".xml") == 0) 
										&& IsCCDAFile(aryImportToHistory[i]->strFilePath))
									{
										CReconciliationDlg::ReconcileAll(aryImportToHistory[i]->nPatientID, nMailID, this);
									}
								}								
							}
							// (d.lange 2010-10-21 09:39) - PLID 41006 - Check the status of the "Always Remove..." checkbox to either remove
							//all the files associated with the patient or just the imported files
							if(IsDlgButtonChecked(IDC_CHK_REMOVEFILES)) {
								RemoveChildRowFiles(pRow->GetParentRow());
							}else {
								for(int i = 0; i < aryImportedRows.GetSize(); i++) {
									IRowSettingsPtr pLastRow = aryImportedRows[i]->GetParentRow()->GetLastChildRow();
									//Check to see if current row is the last child row and the previous row is the parent
									RemoveRowFile(aryImportedRows[i], 
										((aryImportedRows[i]->IsSameRow(pLastRow) == -1) && (aryImportedRows[i]->CalcRowNumber() == 0) ? TRUE : FALSE));
								}
								aryImportedRows.RemoveAll();
							}
						}
					}else {
						AfxMessageBox("The patient you have selected is no longer a valid patient.");
						pRow->GetParentRow()->PutValue(dlcPatientID, (long)-1);
						pRow->GetParentRow()->PutValue(dlcPatientName, _bstr_t("< Select a Valid Patient >"));
						pRow->GetParentRow()->PutBackColor(rscUnselectedPatient);
						
					}
					//Let's remove everything because at this point we've imported what we needed for this patient
					for(int i = 0; i < aryImportToHistory.GetSize(); i++) {
						ImportFile *ifRemoval = aryImportToHistory[i];
						delete ifRemoval;
					}
					aryImportToHistory.RemoveAll();
				}
			}	
			pRow = pChildRow;
		}
		//Let's remove everything whether we used it or not so we don't leak memory
		for(int i = 0; i < aryImportToHistory.GetSize(); i++) {
			ImportFile *ifRemoval = aryImportToHistory[i];
			delete ifRemoval;
		}
		aryImportToHistory.RemoveAll();

		//TES 4/22/2011 - PLID 43252 - There's maybe a new row selected now, so update the preview
		UpdatePreview();

		// (b.savon 2012-03-26 17:03) - PLID 49143 - Handle Minimize Preference
		HandleWindowAfterImport();

	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::OnBnClickedBtnImportToEmr()
{
	try {

		// (d.lange 2010-10-21 09:23) - PLID 41006 - Need to batch selected rows to be removed later
		CArray<IRowSettingsPtr, IRowSettingsPtr> aryImportedRows;
		CArray<ImportFile*, ImportFile*> aryImportToEMR;
		IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);
		
		BOOL bFileImport = FALSE;

		// (d.lange 2010-11-19 13:48) - PLID 41493 - If the 'Always remove' checkbox has recently been checked and the number of checked rows is less than
		// the total number of rows, the user will get a prompt about the removal of unselected files.  Once the user acknowledges the change, the prompt
		// will no longer appear until they reset the checkbox.
		if(CheckChildRowsSelected() && IsDlgButtonChecked(IDC_CHK_REMOVEFILES) &&
				GetRemotePropertyInt("DeviceImport_AlwaysRemoveWarningSetting", 1, 0, GetCurrentUserName(), true) == 1) {
			if(IDNO == MsgBox(MB_YESNO, FormatString("There are some patient files that have not been selected to be imported. Those that have not been "
																"selected will be DELETED. Would you like to continue?\r\n\r\n"
																"Click YES to import the selected files and remove the unselected files.\r\n"
																"Click NO to cancel the import."))) {
				return;						
			}else {
				SetRemotePropertyInt("DeviceImport_AlwaysRemoveWarningSetting", 0, 0, GetCurrentUserName());	
			}
		}

		// (b.savon 2012-02-13 09:42) - PLID 46456 - Make sure the files exists as well.
		if( !IsFileListUpToDate() ){
			return;
		}

		while(pRow != NULL) {
			
			IRowSettingsPtr pChildRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);

			if(VarBool(pRow->GetValue(dlcChecked)) && pRow->GetParentRow() != NULL  ) {
				long nPatientID = pRow->GetParentRow()->GetValue(dlcPatientID);
				//If it's checked, then do the import
				
					CString strFullPath = VarString(pRow->GetValue(dlcFilePath));
					TableContent *pTablePointer = (TableContent*)atoi(VarString(pRow->GetValue(dlcFilePath)));

					if(pTablePointer) {
						ImportFile *importTable = new ImportFile;

						//At this point we know its table data
						importTable->nPatientID = nPatientID;
						importTable->strFilePath = strFullPath;
						importTable->nFileCategory = -1;
						// (j.jones 2011-03-10 16:08) - PLID 42329 - added file time
						importTable->dtFileTime = VarDateTime(pRow->GetValue(dlcFileTime), COleDateTime::GetCurrentTime());
						// (j.jones 2010-11-02 10:15) - PLID 41188 - added device name
						// (j.jones 2010-12-15 08:44) - PLID 41598 - renamed this column
						// (j.jones 2011-08-25 15:15) - PLID 41683 - renamed to plugin name
						importTable->strDeviceName = VarString(pRow->GetParentRow()->GetValue(dlcPluginName));
						// (j.jones 2010-12-15 09:24) - PLID 41598 - added file description, always blank on tables
						importTable->strFileDescription = "";
						// (j.jones 2010-12-28 17:29) - PLID 41796 - added ConvertPDFToImage, never used on tables
						importTable->eConvertPDFToImage = cptiDoNotConvert;
						// (r.gonet 06/11/2013) - PLID 56370 - Initialize the image DPI to the NULL placeholder.
						importTable->nImageDPI = IMAGE_DPI_NULL;
						// (b.savon 2014-12-03 13:44) - PLID 64186 - Initialize to twopilots
						importTable->eConverter = pticTwoPilots;

						aryImportToEMR.Add(importTable);

						aryImportedRows.Add(pRow);
						//We have at least one record to import, so let's set this flag
						bFileImport = TRUE;						
					}
					// (j.jones 2010-10-29 11:10) - PLID 41187 - allow importing any file type
					else if(!strFullPath.IsEmpty() /*&& IsImageFile(strFullPath)*/){
						CString strFileName = FileUtils::GetFileName(strFullPath);
						ImportFile *importFile = new ImportFile;

						importFile->nPatientID = nPatientID;
						importFile->strFilePath = strFullPath;
						importFile->nFileCategory = VarLong(pRow->GetValue(dlcCategory), -1);
						// (j.jones 2011-03-10 16:08) - PLID 42329 - added file time
						importFile->dtFileTime = VarDateTime(pRow->GetValue(dlcFileTime), COleDateTime::GetCurrentTime());
						// (j.jones 2010-11-02 10:15) - PLID 41188 - added device name
						// (j.jones 2010-12-15 08:44) - PLID 41598 - renamed this column
						// (j.jones 2011-08-25 15:15) - PLID 41683 - renamed to plugin name
						importFile->strDeviceName = VarString(pRow->GetParentRow()->GetValue(dlcPluginName));
						// (j.jones 2010-12-15 09:24) - PLID 41598 - added file description
						importFile->strFileDescription = VarString(pRow->GetValue(dlcFileDescriptionToUse));
						// (d.lange 2011-05-25 16:40) - PLID 43253 - added todo alarm ID
						importFile->nToDoTaskID = VarLong(pRow->GetValue(dlcToDoAlarmID));

						// (j.jones 2010-12-28 17:29) - PLID 41796 - added ConvertPDFToImage, only applies if it is a PDF
						CString strExtension = FileUtils::GetFileExtension(strFullPath);
						if(strExtension.CompareNoCase("pdf") == 0 && VarBool(pRow->GetValue(dlcConvertPDFToImage), FALSE)) {
							importFile->eConvertPDFToImage = cptiConvertToImage;
							// (r.gonet 06/11/2013) - PLID 56370 - Set Image DPI to a DPI value or Auto in case for some reason null is set (it shouldn't be).
							importFile->nImageDPI = pRow->GetValue(dlcImageDPI).vt == VT_I4 ? VarLong(pRow->GetValue(dlcImageDPI)) : IMAGE_DPI_NULL;
							// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
							importFile->eConverter = (EPDFToImageConverter)VarLong(pRow->GetValue(dlcPDFToImageConverter), pticTwoPilots);
						}
						else {
							importFile->eConvertPDFToImage = cptiDoNotConvert;
							// (r.gonet 06/11/2013) - PLID 56370 - Set Image DPI to the NULL placeholder.
							importFile->nImageDPI = IMAGE_DPI_NULL;
							// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
							importFile->eConverter = pticTwoPilots;
						}

						//Now lets add it to the array, so we can import to an open EMR later
						aryImportToEMR.Add(importFile);

						aryImportedRows.Add(pRow);

						bFileImport = TRUE;
					}
					else {
						AfxMessageBox(FormatString("The file '%s' cannot be imported into an open EMR!", GetFileName(strFullPath)));
					}
			}

			//If we are at the end of a patient record set and we are importing at least a single record, we should prompt the user about the
			//deletion of unchecked records, giving them the option to continue or interrupt the import.
			if(pRow->GetParentRow() != NULL) {
				IRowSettingsPtr pLastChildRow = pRow->GetParentRow()->GetLastChildRow();
				if(pRow->IsSameRow(pLastChildRow) == -1 && bFileImport) {
					//TES 4/22/2011 - PLID 43252 - At this point we know we're probably removing a file, so clear out the preview just in case
					// it's locked the file we're planning to remove.
					ClearPreview();
					long nPatientID = VarLong(pRow->GetParentRow()->GetValue(dlcPatientID), -1);
					CPicContainerDlg *pPicContainer = NULL;
					if(nPatientID != -1) {
						//Determine that we have an EMR open for this patient
						CMainFrame *pMainFrame = GetMainFrame();
						if(pMainFrame == NULL) {
							//Should not be possible
							ThrowNxException("Could not acquire CMainFrame!");
						}

						//This function will prompt if there are multiple open, writeable EMRs for the patient, but be silent if none are found
						pPicContainer = pMainFrame->GetOpenPatientEMR_WithWriteableTopic(nPatientID, "Select an EMN to import files into:");

						if(pPicContainer == NULL) {
							//Let's remove everything whether we used it or not so we don't leak memory
							for(int i = 0; i < aryImportToEMR.GetSize(); i++) {
								ImportFile *ifRemoval = aryImportToEMR[i];
								delete ifRemoval;
							}
							aryImportToEMR.RemoveAll();

							AfxMessageBox("You must have an unlocked EMR open for the selected patient, with a writeable topic selected, in order to import these files.");
							return;
						}

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

						pPicContainer->SetForegroundWindow();
					}else {
						AfxMessageBox("You must first select a patient before importing to an open EMR.");
						break;
					}

					if(aryImportToEMR.GetSize() > 0) {
						for(int i = 0; i < aryImportToEMR.GetSize(); i++) {
							//is this a table pointer?
							TableContent *pTablePointer = (TableContent*)atoi(aryImportToEMR[i]->strFilePath);
							if(pTablePointer) {
								// (d.lange 2011-04-12 17:16) - PLID 42754 - Using SendMessage instead of PostMessage
								pPicContainer->SendMessage(NXM_ADD_GENERIC_TABLE_TO_EMR, (WPARAM)pTablePointer, (LPARAM)(pTablePointer->strTableName.AllocSysString()));
							}
							// (j.jones 2010-10-29 11:13) - PLID 41187 - we allow importing any file type
							else if(!aryImportToEMR[i]->strFilePath.IsEmpty() /*&& IsImageFile(aryImportToEMR[i]->strFilePath)*/) {

								// (j.jones 2010-11-02 09:25) - PLID 41188 - send in a clean description of the attached file,
								// stating the device it was imported from
								// (j.jones 2010-12-15 09:20) - PLID 41598 - we now have optional file descriptions that devices
								// can send to us, use those in history if available
								CString strFileDescriptionForHistory;
								if(!aryImportToEMR[i]->strFileDescription.IsEmpty()) {
									//we have a description, use it
									strFileDescriptionForHistory.Format("%s - Imported from device: %s", aryImportToEMR[i]->strFileDescription, aryImportToEMR[i]->strDeviceName);
								}
								else {
									//no description given, just list the device name
									strFileDescriptionForHistory.Format("Imported from device: %s", aryImportToEMR[i]->strDeviceName);
								}

								// (j.jones 2010-10-29 12:04) - PLID 41187 - added nPicID
								long nPicID = pPicContainer->GetCurrentPicID();

								// (j.jones 2010-12-29 17:45) - PLID 41796 - if this is a PDF and they want to convert it to an image,
								// now is the time to do so
								CString strExtension = FileUtils::GetFileExtension(aryImportToEMR[i]->strFilePath);
								if(strExtension.CompareNoCase("pdf") == 0
									&& aryImportToEMR[i]->eConvertPDFToImage == cptiConvertToImage
									&& IsPDFConverterLoaded()) {
									
									//convert to image(s)

									//if IsPDFConverterLoaded() failed, the user would be warned one time only (per session)
									//that the PDF conversion will not work

									// (j.jones 2011-03-10 16:26) - PLID 42329 - pass in the file time as the service date
									// (r.gonet 06/11/2013) - PLID 56370 - Pass the image DPI
									// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
									if(!ConvertAndImportPDF(aryImportToEMR[i]->strFilePath, aryImportToEMR[i]->nImageDPI, aryImportToEMR[i]->nPatientID, aryImportToEMR[i]->nFileCategory,
										strFileDescriptionForHistory, TRUE, aryImportToEMR[i]->strDeviceName, pPicContainer, nPicID,
										aryImportToEMR[i]->dtFileTime, aryImportToEMR[i]->nToDoTaskID, aryImportToEMR[i]->eConverter)) {

										// (j.jones 2010-12-30 15:41) - PLID 41960 - if this failed, abort the entire import,
										// but only after cleaning up our memory objects
										for(int j = 0; j < aryImportToEMR.GetSize(); j++) {
											ImportFile *ifRemoval = aryImportToEMR[j];
											delete ifRemoval;
										}
										aryImportToEMR.RemoveAll();
										return;
									}
								}
								else {
									// (j.jones 2010-12-30 12:35) - PLID 41796 - moved this to its own function
									if(!ImportSingleFile(aryImportToEMR[i]->strFilePath, aryImportToEMR[i]->nPatientID, aryImportToEMR[i]->nFileCategory,
										strFileDescriptionForHistory, TRUE, aryImportToEMR[i]->strDeviceName, pPicContainer, nPicID,
										aryImportToEMR[i]->dtFileTime, aryImportToEMR[i]->nToDoTaskID)) {
										
										// (j.jones 2010-12-30 15:41) - PLID 41960 - if this failed, abort the entire import,
										// but only after cleaning up our memory objects
										for(int j = 0; j < aryImportToEMR.GetSize(); j++) {
											ImportFile *ifRemoval = aryImportToEMR[j];
											delete ifRemoval;
										}
										aryImportToEMR.RemoveAll();
										return;
									}
								}
							}
						}

						// (d.lange 2010-10-21 09:56) - PLID 41006 - Check the status of the "Always Remove..." checkbox to either remove
						//all the files associated with the patient or just the imported files
						if(IsDlgButtonChecked(IDC_CHK_REMOVEFILES)) {
							//Let's remove all the files whether they were imported or not per patient
							RemoveChildRowFiles(pRow->GetParentRow());
						}else {
							for(int i = 0; i < aryImportedRows.GetSize(); i++) {
								// (b.savon 2012-03-21 17:24) - PLID 49097 - Unspecified error; Description 'CNxDataListCtrl::FindAbsoluteNextRow: 
								// The given row is not in a list!' when importing 3+ files from the device import into EMR.
								IRowSettingsPtr pLastRow = aryImportedRows[i]->GetParentRow()->GetLastChildRow();
								// (b.savon 2011-10-19 11:33) - PLID 46027 -	Importing multiple images into an EMR that 
								//												have the same name as images currently in
								//												the EMR gives a NULL pointer error while
								//												importing the second image.
								//IRowSettingsPtr pLastRow = pLastChildRow;
								//Check to see if current row is the last child row and the previous row is the parent
								RemoveRowFile(aryImportedRows[i], 
									((aryImportedRows[i]->IsSameRow(pLastRow) == -1) && (aryImportedRows[i]->CalcRowNumber() == 0) ? TRUE : FALSE));
							}
							aryImportedRows.RemoveAll();
						}

						//Let's remove everything whether we used it or not so we don't leak memory
						for(int i = 0; i < aryImportToEMR.GetSize(); i++) {
							ImportFile *ifRemoval = aryImportToEMR[i];
							delete ifRemoval;
						}
						aryImportToEMR.RemoveAll();
					}
					bFileImport = FALSE;
				}
			}
			pRow = pChildRow;
		}		
		
		//Let's remove everything whether we used it or not so we don't leak memory
		for(int i = 0; i < aryImportToEMR.GetSize(); i++) {
			ImportFile *ifRemoval = aryImportToEMR[i];
			delete ifRemoval;
		}
		aryImportToEMR.RemoveAll();

		//TES 4/22/2011 - PLID 43252 - There's maybe a new row selected now, so update the preview
		UpdatePreview();

		// (b.savon 2012-03-24 12:59) - PLID 49143 - Handle Minimize Preference
		HandleWindowAfterImport();

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-11-02 16:02) - PLID 41189 - added DevicePatientMatchRule
// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
// (d.lange 2011-03-09 16:16) - PLID 42754 - All parameters are now contained in the ParentRowRecord struct
// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
//void CDevicePluginImportDlg::AddParentRow(DevicePluginUtils::PatientElement *pePatientResult, CString strPluginName, _variant_t varChecked, EDevicePatientMatchRule eDevicePatientMatchRule, EConvertPDFToImage eConvertPDFToImage)
void CDevicePluginImportDlg::AddParentRow(ParentRowRecord *prPatientRecord,  ADODB::_ConnectionPtr pCon /*= NULL*/)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = m_pList->GetNewRow();

		//The category column of the parent row should not be enabled
		IFormatSettingsPtr pFormatColumn(__uuidof(FormatSettings));
		pFormatColumn->Editable = _variant_t(VARIANT_FALSE, VT_BOOL);

		CString strPatientName = "< Select a Valid Patient >";
		// (j.jones 2010-11-02 11:06) - PLID 41189 - renamed to FindPatient, which may
		// end up matching by demographics instead of ID, based on eDevicePatientMatchRule
		long nMatchedID = VarLong(prPatientRecord->nPatientID);
		if(nMatchedID == -1) {
			// (b.savon 2012-04-06 17:13) - PLID 49506 - Use the FindAndFill utility
			FindAndFillPatient(prPatientRecord, pCon);
		}

		//Need to verify the returned patient ID
		// (b.savon 2012-04-05 17:59) - PLID 41861 - Create a flag to signal the first matched patient (if any)
		BOOL bFirstMatch = FALSE;
		if(nMatchedID != -1) {
			//Then we have a matched patient ID, let's set the column to display this
			pParentRow->PutValue(dlcPatientID, (long)nMatchedID);
			ADODB::_RecordsetPtr rs;
			// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
			if( pCon ){
				rs = CreateParamRecordset(pCon,"SELECT PersonT.Last AS PatientLast, PersonT.First AS PatientFirst, PatientsT.UserDefinedID AS PatientUserDefinedID "
																"FROM PersonT INNER JOIN PatientsT "
																"ON PersonT.ID = PatientsT.PersonID WHERE ID = {INT}", nMatchedID);
			} else{
				rs = CreateParamRecordset("SELECT PersonT.Last AS PatientLast, PersonT.First AS PatientFirst, PatientsT.UserDefinedID AS PatientUserDefinedID "
																"FROM PersonT INNER JOIN PatientsT "
																"ON PersonT.ID = PatientsT.PersonID WHERE ID = {INT}", nMatchedID);
			}
			CString strPatientFirst = AdoFldString(rs, "PatientFirst");
			CString strPatientLast = AdoFldString(rs, "PatientLast");
			long nUserDefinedID = AdoFldLong(rs, "PatientUserDefinedID");

			strPatientName.Format("%s , %s (%li)", strPatientLast, strPatientFirst, nUserDefinedID);

			pParentRow->PutValue(dlcChecked, prPatientRecord->bChecked == TRUE ? g_cvarTrue : g_cvarFalse);
			pParentRow->PutValue(dlcPatientName, _bstr_t(strPatientName));
			pParentRow->PutValue(dlcUserDefinedID, nUserDefinedID);

			// (b.savon 2012-04-05 18:00) - PLID 41861
			//If we have a match and we have no parent associated with it yet and this isn't a manually matched patient, set it
			// (z.manning 2016-04-06 09:51) - NX-100116 - We now do this by simply checking if this patient is already in the list
			NXDATALIST2Lib::IRowSettingsPtr pRowExistingPatient = m_pList->FindByColumn(dlcUserDefinedID, nUserDefinedID, NULL, VARIANT_FALSE);
			if (pRowExistingPatient == nullptr && prPatientRecord->eRowStatusColor == rscMatchedPatient) {
				bFirstMatch = TRUE;
			}

		}
		else {
			//We did not find a patient ID, so we need to let the user know this
			pParentRow->PutValue(dlcChecked, prPatientRecord->bChecked == TRUE ? g_cvarTrue : g_cvarFalse);
			pParentRow->PutValue(dlcPatientID, (long)nMatchedID);
			pParentRow->PutValue(dlcPatientName, _bstr_t(strPatientName));
		}
		// (d.lange 2011-03-11 11:54) - PLID 42754 - Set the parent row color
		pParentRow->PutBackColor(prPatientRecord->eRowStatusColor);
		//We'll secretly store the pointer as a long in the datalist to reference later
		// (b.savon 2012-04-06 13:03) - PLID 41861 - We now store a pointer to an array of pointers as a long in the
		// datalist to reference later
		ArrayParentRowPointers* aryParent = new ArrayParentRowPointers();
		m_aryParentRowPointers.Add(aryParent); //Add it to our member array so we can cleanup after ourselves when were done.
		aryParent->aryParentRecordPointers.Add((long)prPatientRecord); //Add the pointer to the array
		pParentRow->PutValue(dlcPatientPointer, (long)aryParent);//Store the array in the DL

		// (j.jones 2010-10-25 13:35) - PLID 41008 - track the source file that generated this recordrecord, 
		// also track it as lowercase in the searchable column
		CString strFilePath = prPatientRecord->pePatientElement->strIdentifyingFilePath;
		CString strFilePathLower = strFilePath;
		strFilePathLower.MakeLower();
		pParentRow->PutValue(dlcFilePath, _bstr_t(strFilePath));
		pParentRow->PutValue(dlcFilePathSearchable, _bstr_t(strFilePathLower));

		// (j.jones 2011-03-10 16:08) - PLID 42329 - added file time
		pParentRow->PutValue(dlcFileTime, _variant_t(prPatientRecord->dtFileTime, VT_DATE));

		//Include the plugin name from which these record(s) came from so if we're importing to an open EMR we can provide it
		// (j.jones 2010-12-15 08:44) - PLID 41598 - put the plugin name only in the "display" column
		pParentRow->PutValue(dlcFileDescriptionToUse, _bstr_t(""));
		// (j.jones 2011-08-25 15:15) - PLID 41683 - renamed to plugin name
		pParentRow->PutValue(dlcPluginName, _bstr_t(prPatientRecord->strPluginName));

		// (j.jones 2010-11-02 16:06) - PLID 41189 - added DevicePatientMatchRule
		pParentRow->PutValue(dlcDevicePatientMatchRule, (long)prPatientRecord->eDevicePatientMatchRule);

		// (j.jones 2010-12-28 14:51) - PLID 41796 - ConvertPDFToImage is always empty on this row
		pParentRow->PutValue(dlcConvertPDFToImage, g_cvarNull);

		// (r.gonet 06/11/2013) - PLID 56370 - Image DPI is always null on parent rows.
		pParentRow->CellFormatOverride[dlcImageDPI] = pFormatColumn;
		pParentRow->PutValue(dlcImageDPI, g_cvarNull);

		// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
		pParentRow->PutValue(dlcPDFToImageConverter, (long)prPatientRecord->eConverter);

		// (d.lange 2011-05-06 12:44) - PLID 43253 - CreateToDoAlarm is empty on this row
		pParentRow->PutValue(dlcCreateToDoAlarm, g_cvarNull);

		//(e.lally 2011-04-21) PLID 43372 - added PluginFileName
		pParentRow->PutValue(dlcDeviceFileName, _bstr_t(prPatientRecord->strPluginFileName));

		//The category column should be disabled
		pParentRow->CellFormatOverride[dlcCategory] = pFormatColumn;
		
		// (b.savon 2012-04-05 18:02) - PLID 41861
		//If we have a parent row and a matched id, swap the pParentRow with our matched parent row.
		// (z.manning 2016-04-06 09:58) - NX-100116 - We now do this by looking for the patient in the datalist
		_variant_t varUserDefinedID = pParentRow->GetValue(dlcUserDefinedID);
		NXDATALIST2Lib::IRowSettingsPtr pMatchedRow = nullptr;
		if (varUserDefinedID.vt == VT_I4) {
			pMatchedRow = m_pList->FindByColumn(dlcUserDefinedID, VarLong(varUserDefinedID), NULL, VARIANT_FALSE);
		}
		BOOL bAdd = TRUE;
		if (pMatchedRow != nullptr) {
			//Get the parentfile path and the pointer to an array of pointers
			CString strParentFilePath = VarString(pMatchedRow->GetValue(dlcFilePath));
			ArrayParentRowPointers* npParentRowRecordPointer = (ArrayParentRowPointers*)VarLong((pMatchedRow->GetValue(dlcPatientPointer)));
			if (strParentFilePath != strFilePath) {
				//Append our new parentfile path and pointer and save into the DL
				pMatchedRow->PutValue(dlcFilePath, _bstr_t(strParentFilePath + "|" + strFilePath));
				npParentRowRecordPointer->aryParentRecordPointers.Add((long)prPatientRecord);
				pMatchedRow->PutValue(dlcPatientPointer, (long)npParentRowRecordPointer);
			}
			pParentRow = pMatchedRow;
			bAdd = bFirstMatch;
		}

		// (b.savon 2012-04-05 18:02) - PLID 41861 - Only add if the flag is set.  All matched rows, besides the first will clear the bAdd flag
		if (bAdd) {
			// (j.jones 2011-03-11 15:13) - PLID 42328 - changed to add sorted
			m_pList->AddRowSorted(pParentRow, NULL);
		}

		// (d.lange 2011-03-10 14:57) - PLID 42754 - Now we should attach all the records to the parent row
		AddChildFile(pParentRow, prPatientRecord->aryChildRowRecords);

		//After we've added the files to the parent row, let's expand it so the user can easily see the files returned
		pParentRow->PutExpanded(VARIANT_TRUE);
		
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-11-02 16:06) - PLID 41189 - added DevicePatientMatchRule
// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
// (d.lange 2011-03-09 16:16) - PLID 42754 - All parameters are now contained in the ParentRowRecord struct
//void CDevicePluginImportDlg::AddChildFile(NXDATALIST2Lib::IRowSettingsPtr pParentRow, CArray<DevicePluginUtils::RecordElement*, DevicePluginUtils::RecordElement*> &aryRecords, _variant_t varChecked, EDevicePatientMatchRule eDevicePatientMatchRule, EConvertPDFToImage eConvertPDFToImage)
void CDevicePluginImportDlg::AddChildFile(NXDATALIST2Lib::IRowSettingsPtr pParentRow, CArray<ChildRowRecord*, ChildRowRecord*> &aryChildRowRecords)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pChildRow;

		for(int i = 0; i < aryChildRowRecords.GetSize(); i++) {
			pChildRow = m_pList->GetNewRow();
			
			if(pChildRow) {
				CString strFullFilePath = aryChildRowRecords.GetAt(i)->strFullFilePath; //aryRecords.GetAt(i)->strFullFilePath;
				// (j.jones 2010-12-14 16:46) - PLID 41598 - added optional file description,
				// which can be used to give a clean description for the file to be imported,
				// but is not a required field
				CString strFileDescriptionOverride = aryChildRowRecords.GetAt(i)->strFileDescriptionOverride;

				TableContent *pTableContent = (TableContent*)(((RecordElement*)(aryChildRowRecords.GetAt(i)->reRecordElement))->pTableContent); //aryRecords.GetAt(i)->pTableContent;
				
				//There are some columns we want to make uneditable
				IFormatSettingsPtr pFormatColumn(__uuidof(FormatSettings));
				pFormatColumn->Editable = _variant_t(VARIANT_FALSE, VT_BOOL);

				if(!strFullFilePath.IsEmpty() /*&& IsImageFile(strFullFilePath)*/) {
					CString strExtension = FileUtils::GetFileExtension(strFullFilePath);
					strExtension.MakeLower();
					
					if(!strExtension.IsEmpty()) {
						// (a.walling 2013-10-02 09:46) - PLID 58847 - Get cached, small, generic icon
						if (HICON hIcon = FileUtils::GetCachedSmallIcon(strFullFilePath)) {
							pChildRow->PutValue(dlcType, (long)hIcon);
						}
					}
					
					pChildRow->PutValue(dlcChecked, aryChildRowRecords.GetAt(i)->bFileChecked == TRUE ? g_cvarTrue : g_cvarFalse);

					pChildRow->PutValue(dlcPatientPointer, (long)aryChildRowRecords.GetAt(i));
					// (j.jones 2010-10-25 15:20) - PLID 41008 - track the file path in the datalist
					// normally and as lowercase in the searchable column
					CString strFilePathLower = strFullFilePath;
					strFilePathLower.MakeLower();
					pChildRow->PutValue(dlcFilePath, _bstr_t(strFullFilePath));
					pChildRow->PutValue(dlcFilePathSearchable, _bstr_t(strFilePathLower));

					// (j.jones 2011-03-10 16:08) - PLID 42329 - added file time
					pChildRow->PutValue(dlcFileTime, _variant_t(aryChildRowRecords.GetAt(i)->dtFileTime, VT_DATE));

					// (j.jones 2010-12-14 16:46) - PLID 41598 - If given a file description,
					// store it in the dlcFileDescriptionToUse column (blank otherwise),
					// then for the dlcFileDescriptionToDisplay column show the file name if
					// no file description was given, and also append the patient name if
					// we did not match up with a patient.
					CString strFileDescriptionToDisplay;
					strFileDescriptionOverride.TrimLeft(); strFileDescriptionOverride.TrimRight();
					if(strFileDescriptionOverride.IsEmpty()) {
						strFileDescriptionToDisplay = GetFileName(strFullFilePath);
					}
					else {
						strFileDescriptionToDisplay = strFileDescriptionOverride;
					}

					//if there is no patient currently matched up to this row,
					//append the patient's name as stated by the device, if the
					//device provided that name (it will not be removed later when
					//a patient is selected)
					if(VarLong(pParentRow->GetValue(dlcPatientID)) == -1) {
						// (b.savon 2012-04-06 13:05) - PLID 41861 - Respect our new pointer storage
						ArrayParentRowPointers* aryPaRow = (ArrayParentRowPointers*)VarLong(pParentRow->GetValue(dlcPatientPointer));
						ParentRowRecord *peParentRowRecord = (ParentRowRecord*)aryPaRow->aryParentRecordPointers.GetAt(0);
						if(peParentRowRecord->pePatientElement != NULL && 
							(!peParentRowRecord->pePatientElement->strLast.IsEmpty() || !peParentRowRecord->pePatientElement->strFirst.IsEmpty())) {
							//if the last or first name is empty, this will look weird with the comma, but that is intentional

							CString strAppend;
							//include the patient ID and/or birthdate if we have them
							if(!peParentRowRecord->pePatientElement->strPatientID.IsEmpty() && peParentRowRecord->pePatientElement->dtBirthDate.GetStatus() != COleDateTime::invalid) {
								strAppend.Format(" for %s, %s (ID: %s  DOB: %s)", peParentRowRecord->pePatientElement->strLast, 
																				peParentRowRecord->pePatientElement->strFirst, 
																				peParentRowRecord->pePatientElement->strPatientID, 
																				FormatDateTimeForInterface(peParentRowRecord->pePatientElement->dtBirthDate, NULL, dtoDate));
							}
							else if(peParentRowRecord->pePatientElement->dtBirthDate.GetStatus() != COleDateTime::invalid) {
								strAppend.Format(" for %s, %s (DOB: %s)", peParentRowRecord->pePatientElement->strLast, 
																		peParentRowRecord->pePatientElement->strFirst, 
																		FormatDateTimeForInterface(peParentRowRecord->pePatientElement->dtBirthDate, NULL, dtoDate));
							}
							else if(!peParentRowRecord->pePatientElement->strPatientID.IsEmpty()) {
								strAppend.Format(" for %s, %s (ID: %s)", peParentRowRecord->pePatientElement->strLast, 
																		peParentRowRecord->pePatientElement->strFirst, 
																		peParentRowRecord->pePatientElement->strPatientID);
							}
							else {								
								strAppend.Format(" for %s, %s", peParentRowRecord->pePatientElement->strLast, peParentRowRecord->pePatientElement->strFirst);
							}

							strFileDescriptionToDisplay += strAppend;
						}
					}

					pChildRow->PutValue(dlcFileDescriptionToUse, _bstr_t(strFileDescriptionOverride));
					// (j.jones 2011-08-25 15:15) - PLID 41683 - renamed to plugin name
					pChildRow->PutValue(dlcPluginName, _bstr_t(strFileDescriptionToDisplay));
					// (d.lange 2011-03-11 09:34) - PLID 42754 - Load the history category 
					pChildRow->PutValue(dlcCategory, (long)aryChildRowRecords[i]->nCategoryID);

					// (j.jones 2010-11-02 16:06) - PLID 41189 - added DevicePatientMatchRule
					pChildRow->PutValue(dlcDevicePatientMatchRule, (long)aryChildRowRecords.GetAt(i)->eDevicePatientMatchRule);

					// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage, which is a boolean
					// in the datalist interface, but should not be available unless the file is a PDF
					_variant_t varConvertPDFToImage = g_cvarNull;
					if(strExtension.CompareNoCase("pdf") == 0) {
						//this is a PDF, so we do want the checkbox, now set the default value
						if(aryChildRowRecords.GetAt(i)->eConvertPDFToImage == cptiConvertToImage) {
							varConvertPDFToImage = g_cvarTrue;
						}
						else {
							varConvertPDFToImage = g_cvarFalse;
						}
					}
					pChildRow->PutValue(dlcConvertPDFToImage, varConvertPDFToImage);

					// (r.gonet 06/11/2013) - PLID 56370 - If the record is storing -1, it is a placeholder for NULL. Otherwise, it is a valid selection.
					if(VarBool(pChildRow->GetValue(dlcConvertPDFToImage), FALSE) != FALSE) {
						pChildRow->PutValue(dlcImageDPI, _variant_t(aryChildRowRecords.GetAt(i)->nImageDPI, VT_I4));
					} else {
						// (r.gonet 06/11/2013) - PLID 56370 - But if it is NULL, then prevent the user from ediitng this cell.
						pChildRow->CellFormatOverride[dlcImageDPI] = pFormatColumn;
						pChildRow->PutValue(dlcImageDPI, g_cvarNull);
					}

					// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
					pChildRow->PutValue(dlcPDFToImageConverter, (long)aryChildRowRecords.GetAt(i)->eConverter);		

					//Let's make sure the user can't edit the patient name column
					pChildRow->CellFormatOverride[dlcPatientName] = pFormatColumn;

					// (d.lange 2011-05-20 09:07) - PLID 43253 - Set the saved todo task ID
					long nToDoTaskID = aryChildRowRecords.GetAt(i)->nToDoTaskID;
					pChildRow->PutValue(dlcToDoAlarmID, nToDoTaskID);
					_variant_t varToDoTask = g_cvarNull;
					if(nToDoTaskID != -1) {
						varToDoTask = g_cvarTrue;
					}else {
						varToDoTask = g_cvarFalse;
					}
					pChildRow->PutValue(dlcCreateToDoAlarm, varToDoTask);

					// (j.jones 2011-03-11 15:13) - PLID 42328 - changed to add sorted
					// (b.savon 2012-07-02 15:48) - PLID 41861 - changed to add at end of parent because we process the matched (grouped)
					// file in linear order (in the order in which the files are read from the filepath) and create the parent file path
					// string as well as the PatientRecord array on this process.  If we add the row sorted under the parent, this has 
					// the possibility of ruining this linear process, which in turn, would cause us to have to reprocess the list *everytime*
					// to ensure that the file path and array pointers coincide with the list order. The parent row (the file processed first from
					// the filesystem will be the date/time/pt name that respects the file sort order within the list).  The children, will not necessarily
					// be in dated sorted order due to the way in which the files are processed from the file system. 
					m_pList->AddRowAtEnd(pChildRow, pParentRow);
				}

				// (b.savon 2011-09-26 14:39) - PLID 42330 - Only add the EMR tables to the list
				//										     if we have any AND the 'Hide EMR Tables' is NOT checked
				if(pTableContent && !IsDlgButtonChecked(IDC_CHK_HIDE_EMR_TABLES)) {
					//There is valid table data
					pChildRow = m_pList->GetNewRow();

					long nTableContent = (long)pTableContent;
					CString strTableContent = "";
					strTableContent.Format("%li", nTableContent);

					pChildRow->PutValue(dlcChecked, aryChildRowRecords.GetAt(i)->bTableChecked == TRUE ? g_cvarTrue : g_cvarFalse);
					pChildRow->PutValue(dlcType, (long)m_hIconPreview);
					pChildRow->PutValue(dlcFilePath, _bstr_t(strTableContent));
					pChildRow->PutValue(dlcFilePathSearchable, _bstr_t(strTableContent));

					// (j.jones 2011-03-10 16:08) - PLID 42329 - added file time
					pChildRow->PutValue(dlcFileTime, _variant_t(aryChildRowRecords.GetAt(i)->dtFileTime, VT_DATE));

					// (j.jones 2010-12-15 08:49) - PLID 41598 - renamed the description column,
					// no description is tracked internally because tables aren't attached to history
					pChildRow->PutValue(dlcFileDescriptionToUse, _bstr_t(""));
					// (j.jones 2011-08-25 15:15) - PLID 41683 - renamed to plugin name
					pChildRow->PutValue(dlcPluginName, _bstr_t("EMR Table: " + pTableContent->strTableName));

					pChildRow->PutValue(dlcPatientPointer, (long)aryChildRowRecords.GetAt(i));

					// (j.jones 2010-11-02 16:06) - PLID 41189 - added DevicePatientMatchRule
					pChildRow->PutValue(dlcDevicePatientMatchRule, (long)aryChildRowRecords.GetAt(i)->eDevicePatientMatchRule);

					// (j.jones 2010-12-28 14:51) - PLID 41796 - ConvertPDFToImage is always empty for tables
					pChildRow->PutValue(dlcConvertPDFToImage, g_cvarNull);

					// (r.gonet 06/11/2013) - PLID 56370 - Image DPI is always null for table rows.
					pChildRow->CellFormatOverride[dlcImageDPI] = pFormatColumn;
					pChildRow->PutValue(dlcImageDPI, g_cvarNull);

					// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
					pChildRow->PutValue(dlcPDFToImageConverter, (long)aryChildRowRecords.GetAt(i)->eConverter);

					// (d.lange 2011-05-20 09:07) - PLID 43253 - Disable the creation of ToDos for Table Content rows
					pChildRow->PutValue(dlcCreateToDoAlarm, g_cvarNull);
					pChildRow->PutValue(dlcToDoAlarmID, (long)-1);

					//Let's make sure the user can't edit these columns
					pChildRow->CellFormatOverride[dlcCategory] = pFormatColumn;
					pChildRow->CellFormatOverride[dlcPatientName] = pFormatColumn;
					
					// (b.savon 2012-07-02 15:48) - PLID 41861 - changed to add at end of parent because we process the matched (grouped)
					// file in linear order (in the order in which the files are read from the filepath) and create the parent file path
					// string as well as the PatientRecord array on this process.  If we add the row sorted under the parent, this has 
					// the possibility of ruining this linear process, which in turn, would cause us to have to reprocess the list *everytime*
					// to ensure that the file path and array pointers coincide with the list order. The parent row (the file processed first from
					// the filesystem will be the date/time/pt name that respects the file sort order within the list).  The children, will not necessarily
					// be in dated sorted order due to the way in which the files are processed from the file system. 
					m_pList->AddRowAtEnd(pChildRow, pParentRow);
				}
			}			
		}

		// (b.savon 2011-09-26 14:39) - PLID 42330 - If we removed all the children under the parent, remove the parent as well.
		if( pParentRow->GetFirstChildRow() == NULL ){
			m_pList->RemoveRow(pParentRow);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-11-02 11:06) - PLID 41189 - Renamed to FindPatient, which may
// end up matching by demographics instead of ID, based on eDevicePatientMatchRule.
// nOverrideUserDefinedID is used if the row has already been linked to a patient.
// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
long CDevicePluginImportDlg::FindPatient(DevicePluginUtils::PatientElement *pePatientResult, EDevicePatientMatchRule eDevicePatientMatchRule, long nOverrideUserDefinedID, ADODB::_ConnectionPtr pCon /*= NULL*/)
{
	try {

		long nUserDefinedID = atoi(pePatientResult->strPatientID);
		if(AsString(nUserDefinedID) != pePatientResult->strPatientID) {
			nUserDefinedID = -1;
		}

		//nOverrideUserDefinedID is used if the record has already been linked to a patient
		if(nOverrideUserDefinedID != -1) {
			nUserDefinedID = nOverrideUserDefinedID;
		}

		// (j.jones 2010-11-02 11:04) - PLID 41189 - depending on the eDevicePatientMatchRule
		// setting, we either will match by UserDefinedID ONLY, otherwise we will match on
		// UserDefinedID *OR* if that does not match, if every other demographic field
		// matches 100% with the data we received (but remember, not all devices fill all fields)

		long nMatchedID = -1;
		ADODB::_RecordsetPtr rs;

		// (b.savon 2012-02-03 16:46) - PLID 47973 - Rework the Match by ID - Used better design to 
		// condense the pCon logic 
		// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
		if(pCon == NULL) {
			pCon = GetRemoteData();
		}

		// (b.savon 2012-02-03 17:08) - PLID 47930 - Pull this out so we can use it for
		// IDOrDemographics as well as Demographics match.
		CSqlFragment sqlBirthdate;
		CString strSSN;
		if( eDevicePatientMatchRule == dpmrMatchByIDOrDemographics || eDevicePatientMatchRule == dpmrMatchByDemographics ){

			_variant_t varBirthDate = g_cvarNull;
			if(pePatientResult->dtBirthDate.GetStatus() != COleDateTime::invalid) {
				varBirthDate = COleVariant(pePatientResult->dtBirthDate);
				varBirthDate.vt = VT_DATE;
			}
			
			//replace dashes
			strSSN = pePatientResult->strSSN;
			strSSN.Replace("-","");

			//if we know that all the data is invalid, don't even run the recordset
			if((nUserDefinedID == -1 && pePatientResult->strLast.IsEmpty()
				&& pePatientResult->strFirst.IsEmpty()
				&& pePatientResult->strMiddle.IsEmpty()
				&& pePatientResult->strMiddle.IsEmpty()
				&& pePatientResult->nGender != 1 && pePatientResult->nGender != 2
				&& varBirthDate.vt == VT_NULL
				&& strSSN.IsEmpty())
				//if just the first & last name are empty, it's still considered to be invalid
				|| (nUserDefinedID == -1 && pePatientResult->strLast.IsEmpty()
				&& pePatientResult->strFirst.IsEmpty())) {

				return -1;
			}

			if(varBirthDate != g_cvarNull) {
				sqlBirthdate = CSqlFragment("AND (PersonT.BirthDate = {VT_DATE} OR BirthDate IS NULL)", varBirthDate);
			} else{
				//Dont check in query if there isn't a birthdate in Zeiss XML
			}

		}else{
			// No need to process demographics data if were matching by ID
		}

		//if we have an override, don't match by demographics
		if(nOverrideUserDefinedID == -1 && eDevicePatientMatchRule == dpmrMatchByIDOrDemographics) {

			//match by ID or demographics

			// (b.savon 2012-02-03 16:01) - PLID 47973 - Rework the Match by ID or Demographics
			rs = CreateParamRecordset(pCon,"SELECT PatientsT.PersonID AS PatientID "
				"FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"WHERE PersonT.ID != -25 "
				"AND PersonT.Archived = 0 "
				"AND "
				"(PatientsT.UserDefinedID = {INT} "
				"OR (PatientsT.UserDefinedID <> {INT} "
				//ensure we never match on a blank patient name
				"AND PersonT.Last <> '' "
				"AND PersonT.First <> '' "
				"AND PersonT.Last = {STRING} "
				"AND PersonT.First = {STRING} "
				"AND ('' = {STRING} OR PersonT.Middle = {STRING}) "
				"AND (0 = {INT} OR PersonT.Gender = {INT}) "
				"{SQL} "
				//"AND (NULL = {VT_DATE} OR PersonT.BirthDate = {VT_DATE}) "
				"AND ('' = {STRING} OR Replace(PersonT.SocialSecurity, '-','') = {STRING}) "
				"))",
				nUserDefinedID, nUserDefinedID,
				pePatientResult->strLast, pePatientResult->strFirst,
				pePatientResult->strMiddle, pePatientResult->strMiddle,
				pePatientResult->nGender, pePatientResult->nGender,
				sqlBirthdate,
				//varBirthDate, varBirthDate,
				strSSN, strSSN);

		} else if (	nOverrideUserDefinedID == -1 && eDevicePatientMatchRule == dpmrMatchByDemographics) {
			// (b.savon 2012-02-03 17:08) - PLID 47930 - Match by patient demographics

			rs = CreateParamRecordset(pCon,
				"SELECT PatientsT.PersonID AS PatientID "
				"FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"WHERE PersonT.ID != -25 "
				"AND PersonT.Archived = 0 "
				//ensure we never match on a blank patient name
				"AND PersonT.Last <> '' "
				"AND PersonT.First <> '' "
				"AND PersonT.Last = {STRING} "
				"AND PersonT.First = {STRING} "
				"AND ('' = {STRING} OR PersonT.Middle = {STRING}) "
				"AND (0 = {INT} OR PersonT.Gender = {INT}) "
				"{SQL} "
				"AND ('' = {STRING} OR Replace(PersonT.SocialSecurity, '-','') = {STRING}) "
				,
				pePatientResult->strLast, pePatientResult->strFirst,
				pePatientResult->strMiddle, pePatientResult->strMiddle,
				pePatientResult->nGender, pePatientResult->nGender,
				sqlBirthdate,
				strSSN, strSSN);
		}
		else {
			//match by ID only, do this in a different query, because it is much faster

			//if we know the ID is invalid, don't even run the recordset
			if(nUserDefinedID == -1) {
				return -1;
			}

			rs = CreateParamRecordset(pCon, "SELECT PatientsT.PersonID AS PatientID "
				"FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"WHERE PersonT.ID != -25 "
				"AND PersonT.Archived = 0 "
				"AND PatientsT.UserDefinedID = {INT}",
				nUserDefinedID);
		}

		//only return if we have one and only one match
		if(!rs->eof && rs->GetRecordCount() == 1) {
			nMatchedID = AdoFldLong(rs, "PatientID", -1);
			return nMatchedID;
		}

	} NxCatchAll(__FUNCTION__);

	return -1;
}

// (j.jones 2010-06-17 13:55) - PLID 37976 - Called by MainFrame to add new PatientElements
// returned by the plugins into this importer screen. Return TRUE if we took ownership of
// the PatientElement (and plan to delete it), return FALSE if it is a duplicate and MainFrame
// will then be responsible for deleting.
// (j.jones 2010-11-02 16:02) - PLID 41189 - added DevicePatientMatchRule
// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
//(e.lally 2011-04-21) PLID 43372 added strPluginFileName
// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
// (r.gonet 06/11/2013) - PLID 56370 - Added Image DPI
// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
BOOL CDevicePluginImportDlg::AddNewPatientElement(DevicePluginUtils::PatientElement *pNewPatientElement, CString strPluginName, CString strPluginFileName, EDevicePatientMatchRule eDevicePatientMatchRule, EConvertPDFToImage eConvertPDFToImage, long nImageDPI, EPDFToImageConverter eConverter, ADODB::_ConnectionPtr pCon /*= NULL*/)
{
	try {

		//we need to compare on pNewPatientElement->strIdentifyingFilePath,
		//if we keep the element, just add this existing pointer rather than making a copy.
		//Return TRUE to tell the caller that we took ownership, and the caller won't delete the pointer.
		//Return FALSE to tell the caller that we did not take ownership, and the caller will delete the pointer.
		
		//Before we add the PatientElement and its records to the dialog, we should see if its already there
		BOOL bExist = ComparePatientElement(pNewPatientElement);
		if(!bExist) {
			// (d.lange 2011-03-09 15:22) - PLID 42754 - Create a new ParentRowRecord and add it to the global array
			//(e.lally 2011-04-21) PLID 43372 added strPluginFileName
			// (r.gonet 06/11/2013) - PLID 56370 - Added Image DPI
			// (b.savon 2014-12-03 13:44) - PLID 64186 - And the converter
			ParentRowRecord *prPatientRecord = new ParentRowRecord(pNewPatientElement, strPluginName, strPluginFileName, eDevicePatientMatchRule, eConvertPDFToImage, nImageDPI, eConverter);
			m_aryParentRowRecord.Add(prPatientRecord);

			// (d.lange 2011-03-09 15:22) - PLID 41010 - Check to see if the current patient filter is checked, if so then check against the
			// current patient (if TRUE add the row, else don't show it just keep it in memory to display later). If the patient ID is -1 and the
			// current patient filter is checked, we want to display the patient element.
			CArray<long,long> aryOpenEMRPatientIDs;
			GetMainFrame()->GetOpenEMRPatientIDs(aryOpenEMRPatientIDs);
			// (b.savon 2012-04-09 17:57) - PLID 49506 - Find and Fill Patient to populate pt. info on initial load
			FindAndFillPatient(prPatientRecord, pCon);
			// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
			if(!IsDlgButtonChecked(IDC_CHK_CURPATIENT_FILTER) || 
				(IsDlgButtonChecked(IDC_CHK_CURPATIENT_FILTER) && IsCurrentPatient(prPatientRecord->nPatientUserDefinedID, aryOpenEMRPatientIDs, pCon))) {
				
				// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
				AddParentRow(prPatientRecord, pCon);
			}

			// (d.lange 2010-10-26 16:45) - PLID 41088 - We need to notify all open EMNs and History tab, when adding a new record
			SendMessageToUpdateButtons();

			//we now own the pointer, tell the caller so
			return TRUE;
		}
		else {
			//we did not take ownership of the pointer, the caller will free it
			return FALSE;
		}

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

BEGIN_EVENTSINK_MAP(CDevicePluginImportDlg, CNxDialog)
	ON_EVENT(CDevicePluginImportDlg, IDC_IMPORT_LIST, 19, CDevicePluginImportDlg::LeftClickImportList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDevicePluginImportDlg, IDC_IMPORT_LIST, 10, CDevicePluginImportDlg::EditingFinishedImportList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CDevicePluginImportDlg, IDC_IMPORT_LIST, 7, CDevicePluginImportDlg::RButtonUpImportList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDevicePluginImportDlg, IDC_IMPORT_LIST, 2, CDevicePluginImportDlg::OnSelChangedImportList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CDevicePluginImportDlg, IDC_FILE_PREVIEW, 270, CDevicePluginImportDlg::OnFileDownloadFilePreview, VTS_BOOL VTS_PBOOL)
	// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
	ON_EVENT(CDevicePluginImportDlg, IDC_DEVICE_PLUGIN_SELECT, 16, CDevicePluginImportDlg::SelChosenDevicePluginSelect, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CDevicePluginImportDlg::LeftClickImportList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if(pRow) {
			
			// (b.savon 2012-02-01 13:18) - PLID 47651 - Save the current row were working on
			//Regardless of column clicked, save the row
			m_strSelectedFilePath = pRow->GetValue(dlcFilePath);

			switch (nCol) {
				case dlcPatientName:
					{
						//We need to make sure this only applies to the parent row
						if(pRow->GetParentRow() == NULL) {
							//Pull the PatientElement* from the datalist and pass it to the SelectPatientDlg to display the returned
							//patient demographics
							// (b.savon 2012-04-06 13:05) - PLID 41861 - Respect our new pointer storage
							ArrayParentRowPointers* aryPaRow = (ArrayParentRowPointers*)VarLong(pRow->GetValue(dlcPatientPointer));
							ParentRowRecord *prParentRecord = (ParentRowRecord*)aryPaRow->aryParentRecordPointers.GetAt(0);
							PatientElement *pePatient = (PatientElement*)prParentRecord->pePatientElement;
							CDevicePatientSelectDlg dlg(this, pePatient, VarLong(pRow->GetValue(dlcPatientID)));

							if(IDOK == dlg.DoModal()){
								if(dlg.m_nPatientID != -1 && prParentRecord->nPatientID != dlg.m_nPatientID) {	
									pRow->PutValue(dlcPatientID, (long)dlg.m_nPatientID);									
									pRow->PutValue(dlcUserDefinedID, (long)dlg.m_nUserDefinedID);
									pRow->PutValue(dlcPatientName, _bstr_t(FormatString("%s, %s %s (%li)", dlg.m_strPatientLast, dlg.m_strPatientFirst, dlg.m_strPatientMiddle, dlg.m_nUserDefinedID)));
									pRow->PutBackColor(rscSelectedPatient);

									// (d.lange 2011-03-11 11:55) - PLID 42754 - Since the user selected a patient, we need to update the appropriate ParentRowRecord
									prParentRecord->nPatientID = (long)dlg.m_nPatientID;
									prParentRecord->nPatientUserDefinedID = (long)dlg.m_nUserDefinedID;
									prParentRecord->eRowStatusColor = rscSelectedPatient;

									// (j.jones 2011-03-11 15:12) - PLID 42328 - SortList() always calls LoadData(),
									// which will then apply our patient filter, if it is enabled
									// (b.savon 2012-04-09 16:00) - PLID 41861 - Respect the new DL structure, reselect on pointer to array
									SortList(aryPaRow);
								}
							}
						}
					}
					break;
				case dlcType:
					{
						if(pRow->GetParentRow() != NULL) {
							long nPatientID = pRow->GetParentRow()->GetValue(dlcPatientID);
							CString strFilePath = pRow->GetValue(dlcFilePath);
							//Let's check to see if this is a TablePointer, if so lets preview it for the user.
							TableContent *pTableContent = (TableContent*)atoi(VarString(pRow->GetValue(dlcFilePath)));
							if(pTableContent) {
								PreviewTable(pTableContent);
							}
							// (j.jones 2010-10-29 11:15) - PLID 41187 - be able to open any file,
							// not just an image file
							else if(!strFilePath.IsEmpty() /*&& IsImageFile(strFilePath)*/) {
								OpenDocument(strFilePath, nPatientID);
							}
						}
					}
					break;
				
				
				default:
					break;
			}

		}//END if(pRow)
	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::OnBnClickedDeviceClose()
{
	try {
		ShowWindow(SW_HIDE);

	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::EditingFinishedImportList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(pRow) {

			switch (nCol) {
				case dlcChecked:
					{
						BOOL bEnabled = VarBool(pRow->GetValue(dlcChecked));
						//Only if we clicked on the parent row
						if(pRow->GetParentRow() == NULL) {
							//If the parent row is not expanded and the user checks the checkbox to become enabled, let's expand the parent row
							if(!pRow->GetExpanded() && bEnabled)
								pRow->PutExpanded(VARIANT_TRUE);

							NXDATALIST2Lib::IRowSettingsPtr pNextRow = m_pList->FindAbsoluteNextRow(pRow, true);
							while(pNextRow != NULL && pNextRow->GetParentRow() != NULL) {
								pNextRow->PutValue(dlcChecked, (bEnabled == 1 ? g_cvarTrue : g_cvarFalse));
								pNextRow = pNextRow->GetNextRow();
							}
							// (d.lange 2011-03-11 09:46) - PLID 42754 - Update m_aryParentRowRecord with the value of the checkbox
							// (b.savon 2012-04-09 15:08) - PLID 41861 - Respect the new DL parent pointer structure
							ArrayParentRowPointers* npParentRowRecordPointer = (ArrayParentRowPointers*)VarLong(pRow->GetValue(dlcPatientPointer));
							for( int z = 0; z < npParentRowRecordPointer->aryParentRecordPointers.GetSize(); z++ )
							{
								ParentRowRecord *prParentRowRecord = (ParentRowRecord*)npParentRowRecordPointer->aryParentRecordPointers.GetAt(z);
								for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
									if(m_aryParentRowRecord[j] == prParentRowRecord) {
										m_aryParentRowRecord[j]->bChecked = bEnabled;
										for(int i = m_aryParentRowRecord[j]->aryChildRowRecords.GetSize() - 1; i >= 0; i--) {
											m_aryParentRowRecord[j]->aryChildRowRecords[i]->bTableChecked = bEnabled;
											m_aryParentRowRecord[j]->aryChildRowRecords[i]->bFileChecked = bEnabled;
										}
									}
								}
							}
						}else{
							BOOL bParentEnabled = FALSE;
							if(CheckChildRowsSelected(pRow->GetParentRow())) {
								pRow->GetParentRow()->PutValue(dlcChecked, g_cvarTrue);
								bParentEnabled = TRUE;
							}else {
								pRow->GetParentRow()->PutValue(dlcChecked, g_cvarFalse);
							}
							// (d.lange 2011-03-11 09:46) - PLID 42754 - Update m_aryParentRowRecord with the value of the checkbox
							ChildRowRecord *crChildRowRecord = (ChildRowRecord*)VarLong(pRow->GetValue(dlcPatientPointer));
							for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
								if(m_aryParentRowRecord[j] == (ParentRowRecord*)crChildRowRecord->nParentPointer) {
									m_aryParentRowRecord[j]->bChecked = bParentEnabled;
									for(int i = m_aryParentRowRecord[j]->aryChildRowRecords.GetSize() - 1; i >= 0; i--) {
										if(m_aryParentRowRecord[j]->aryChildRowRecords[i] == crChildRowRecord) {
											if((TableContent*)atoi(VarString(pRow->GetValue(dlcFilePath)))) {
												m_aryParentRowRecord[j]->aryChildRowRecords[i]->bTableChecked = bEnabled;
											}else {
												m_aryParentRowRecord[j]->aryChildRowRecords[i]->bFileChecked = bEnabled;
											}
										}
									}
								}
							}
						}
					}
					break;
				// (d.lange 2011-03-11 09:38) - PLID 42754 - Update m_aryParentRowRecord with the selected categoryID
				case dlcCategory:
					{
						long nCategoryID = VarLong(pRow->GetValue(dlcCategory), -1);
						ChildRowRecord *crChildRowRecord = (ChildRowRecord*)VarLong(pRow->GetValue(dlcPatientPointer));
						for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
							if(m_aryParentRowRecord[j] == (ParentRowRecord*)crChildRowRecord->nParentPointer) {
								for(int i = m_aryParentRowRecord[j]->aryChildRowRecords.GetSize() - 1; i >= 0; i--) {
									if(m_aryParentRowRecord[j]->aryChildRowRecords[i] == crChildRowRecord) {
										m_aryParentRowRecord[j]->aryChildRowRecords[i]->nCategoryID = nCategoryID;
									}
								}
							}
						}
					}
					break;
				// (d.lange 2011-03-11 09:42) - PLID 42754 - Update m_aryParentRowRecord with the value of convert PDF to image
				case dlcConvertPDFToImage:
					{
						BOOL bConvertPDF = VarBool(pRow->GetValue(dlcConvertPDFToImage), FALSE);

						// (r.gonet 06/11/2013) - PLID 56370 - If the Convert PDF to Image checkbox is checked, then we allow the user to select an Image DPI.
						NXDATALIST2Lib::IFormatSettingsPtr pFormatColumn(__uuidof(NXDATALIST2Lib::FormatSettings));
						pFormatColumn->Editable = VARIANT_FALSE;
						if(bConvertPDF) {
							// (r.gonet 06/11/2013) - PLID 56370 - Remove any override preventing the user from editing the value.
							pRow->CellFormatOverride[dlcImageDPI] = NULL;
							// (r.gonet 06/11/2013) - PLID 56370 - Set to the {Auto} placeholder.
							pRow->PutValue(dlcImageDPI, _variant_t(IMAGE_DPI_AUTO, VT_I4));
						} else {
							// (r.gonet 06/11/2013) - PLID 56370 - Prevent the user from editing the value.
							pRow->CellFormatOverride[dlcImageDPI] = pFormatColumn;
							pRow->PutValue(dlcImageDPI, g_cvarNull);
						}

						ChildRowRecord *crChildRowRecord = (ChildRowRecord*)VarLong(pRow->GetValue(dlcPatientPointer));
						for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
							if(m_aryParentRowRecord[j] == (ParentRowRecord*)crChildRowRecord->nParentPointer) {
								for(int i = m_aryParentRowRecord[j]->aryChildRowRecords.GetSize() - 1; i >= 0; i--) {
									if(m_aryParentRowRecord[j]->aryChildRowRecords[i] == crChildRowRecord) {
										m_aryParentRowRecord[j]->aryChildRowRecords[i]->eConvertPDFToImage = (bConvertPDF == TRUE ? cptiConvertToImage : cptiDoNotConvert);
										
										// (r.gonet 06/11/2013) - PLID 56370 - Initialize the child record's Image DPI, the value depending on if this box is checked or not.
										if(bConvertPDF) {
											m_aryParentRowRecord[j]->aryChildRowRecords[i]->nImageDPI = IMAGE_DPI_AUTO;
										} else {
											m_aryParentRowRecord[j]->aryChildRowRecords[i]->nImageDPI = IMAGE_DPI_NULL;
										}
									}
								}
							}
						}
					}
					break;

				case dlcImageDPI:
					{
						// (r.gonet 06/11/2013) - PLID 56370 - Sync the child record's value with the row's Image DPI value.
						long nImageDPI = varNewValue.vt == VT_I4 ? VarLong(varNewValue) : IMAGE_DPI_AUTO;
						ChildRowRecord *crChildRowRecord = (ChildRowRecord*)VarLong(pRow->GetValue(dlcPatientPointer));
						for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
							if(m_aryParentRowRecord[j] == (ParentRowRecord*)crChildRowRecord->nParentPointer) {
								for(int i = m_aryParentRowRecord[j]->aryChildRowRecords.GetSize() - 1; i >= 0; i--) {
									if(m_aryParentRowRecord[j]->aryChildRowRecords[i] == crChildRowRecord) {
										// (r.gonet 06/11/2013) - PLID 56370 - Set the child record's Image DPI value to be the row's Image DPI value.
										m_aryParentRowRecord[j]->aryChildRowRecords[i]->nImageDPI = nImageDPI;
									}
								}
							}
						}
					}
					break;
				// (d.lange 2011-05-20 08:54) - PLID 43253 - Handle the creation and editing of a ToDo Alarm per file (excluding tables)
				case dlcCreateToDoAlarm:
					{
						if(pRow->GetParentRow() != NULL) {
							ChildRowRecord *crChildRecord = (ChildRowRecord*)(VarLong(pRow->GetValue(dlcPatientPointer)));
							TableContent *pTableContent = (TableContent*)atoi(VarString(pRow->GetValue(dlcFilePath)));
							//Check to see if we've clicked on a row that contains table content
							if(!pTableContent) {
								CTaskEditDlg dlg(this);
								long nTaskID = VarLong(pRow->GetValue(dlcToDoAlarmID), -1);
								bool bIsPatient = ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT}", VarLong(pRow->GetParentRow()->GetValue(dlcPatientID),-1))?true:false;

								// (b.savon 2015-07-06 10:32) - PLID 60727 - If it is a patient, set the ID
								if (bIsPatient && nTaskID == -1){
									dlg.m_nPersonID = VarLong(pRow->GetParentRow()->GetValue(dlcPatientID), -1);
								}

								if(nTaskID != -1) {
									//Existing ToDo Alarm
									dlg.m_iTaskID = nTaskID;
									pRow->PutValue(dlcCreateToDoAlarm, g_cvarTrue);
									dlg.DoModal(bIsPatient);
									if(dlg.m_bWasDeleted) {
										//Since the Todo was deleted let's remove the cell coloring and update the task ID
										nTaskID = -1;
										pRow->PutValue(dlcToDoAlarmID, nTaskID);
										pRow->PutValue(dlcCreateToDoAlarm, g_cvarFalse);
									}
								}else {
									nTaskID = -1;
									//New ToDo Alarm
									long nCategory = VarLong(pRow->GetValue(dlcCategory), -1);
									if(nCategory != -1) {
										dlg.m_nDefaultCategoryID = nCategory;
									}

									if(IDOK == dlg.DoModal(bIsPatient)) {
										//Retrieve the TaskID and insert it into the dlcToDoAlarmID column
										nTaskID = dlg.m_iTaskID;
										pRow->PutValue(dlcToDoAlarmID, VarLong(nTaskID));
										pRow->PutValue(dlcCreateToDoAlarm, g_cvarTrue);
									}else {
										pRow->PutValue(dlcToDoAlarmID, nTaskID);
										pRow->PutValue(dlcCreateToDoAlarm, g_cvarFalse);
									}
								}
								crChildRecord->nToDoTaskID = nTaskID;
							}
						}
					}
					break;
				default:
					break;
			}
		}

	} NxCatchAll(__FUNCTION__);
}

BOOL CDevicePluginImportDlg::ComparePatientElement(DevicePluginUtils::PatientElement* peParent)
{
	try {
		//Let's grab the incoming PatientElement* identifier so we can determine if you stays or goes
		/*CString strIncomingIdentifier = peParent->strIdentifyingFilePath;

		//This should always be a parent row, having an identifier
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);
		PatientElement *pCurrentPatient = NULL;

		while(pParentRow) {
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pParentRow->GetNextRow();

			if(pParentRow->GetParentRow() == NULL) {
				//At this point, we are looking at a parent row, we don't care about any other rows beside parents

				//Let's create a temp PatientElement so we know the identifier
				pCurrentPatient = (PatientElement*)((ParentRowRecord*)VarLong(pParentRow->GetValue(dlcPatientPointer)))->pePatientElement;

				CString strCurrentIdentifier = pCurrentPatient->strIdentifyingFilePath;

				if(strCurrentIdentifier.CompareNoCase(strIncomingIdentifier) == 0) { return TRUE; }

			}

			pParentRow = pNextRow;
		}*/
		// (d.lange 2011-03-11 18:04) - PLID 42754 - Since we're storing all these objects in m_aryParentRowRecord, its easy to check if one exists
		for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
			if(m_aryParentRowRecord[j]->pePatientElement->strIdentifyingFilePath.CompareNoCase(peParent->strIdentifyingFilePath) == 0) {
				return TRUE;
			}
		}

	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (j.jones 2010-06-22 09:00) - PLID 38903 - supported previewing a table
void CDevicePluginImportDlg::PreviewTable(DevicePluginUtils::TableContent *pTableContent) 
{
	try {

		if(pTableContent == NULL || !pTableContent->IsValid()) {
			return;
		}

		long nEMRInfoID = -1;
		long nEMRInfoMasterID = -1;

		//load both the info ID and master ID
		_RecordsetPtr prs = CreateParamRecordset("SELECT ID, EMRInfoMasterID FROM EmrInfoT WHERE DataSubType = {INT} AND ID IN "
			"(SELECT ActiveEmrInfoID FROM EMRInfoMasterT)",
			eistGenericTable);

		if (prs->eof) {
			ThrowNxException("GetActiveGenericTableInfoID() could not find an EMR info record!");
		} else if (prs->RecordCount > 1) {
			ThrowNxException("GetActiveGenericTableInfoID() found multiple EMR info records!");
		}

		nEMRInfoID = AdoFldLong(prs, "ID");
		nEMRInfoMasterID = AdoFldLong(prs, "EMRInfoMasterID");
		prs->Close();

		CEMRItemAdvPopupDlg dlg(this);
		//Create a temporary EMR.
		CEMR emr;
		emr.CreateNew(-1, TRUE);
		emr.m_bIgnoreActions = TRUE;
		SourceActionInfo saiBlank;
		// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking
		CEMRTopic *pTopic = emr.AddEMNFromTemplate(-1, saiBlank, NULL, -1)->AddTopic("", -1);
		//Create a temporary EMN Detail
		CEMNDetail *pDetail = CEMNDetail::CreateDetail(pTopic, "CDevicePluginImportDlg::PreviewTable detail");
		pTopic->AddDetail(pDetail, FALSE, FALSE);	

		pDetail->m_nEMRInfoID = nEMRInfoID;
		pDetail->m_nEMRInfoMasterID = nEMRInfoMasterID;
		pDetail->m_EMRInfoType = eitTable;
		pDetail->m_bTableRowsAsFields = FALSE;
		pDetail->SetLabelText(pTableContent->strTableName);

		for(int i=0;i<pTableContent->aryRows.GetSize();i++) {

			RowElement *pRow = (RowElement*)pTableContent->aryRows.GetAt(i);
			
			long nRowID = (long)pRow;
			CString strRowName = pRow->strName;
			long nSortOrder = i+1;

			//(e.lally 2011-12-08) PLID 46471
			BOOL bIsCurrentMedOrAllergy = ((pDetail->IsCurrentMedicationsTable() || pDetail->IsAllergiesTable()) ? TRUE : FALSE);

			//add to the detail
			// (j.jones 2011-03-09 09:05) - PLID 42283 - send -1 for nEMCodeCategoryID
			//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
			// (z.manning 2011-05-26 15:51) - PLID 43865 - Set flags to zero
			// (c.haag 2011-05-31) - PLID 43875 - We now specify placement
			//(e.lally 2011-12-08) PLID 46471 - Specify if this is for a current medications or allergies detail. I am not aware of a way for this to be possible here though.
			// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
			pDetail->AddRow(TableRowID(nRowID,-1,NULL,-1,-1), strRowName, "", 2, -1, FALSE, godtInvalid, -1, 0, CEMNDetail::aroSequential, -1, bIsCurrentMedOrAllergy);
		}
		for(int i=0;i<pTableContent->aryColumns.GetSize();i++) {

			ColumnElement *pColumn = (ColumnElement*)pTableContent->aryColumns.GetAt(i);

			long nColumnID = (long)pColumn;
			CString strColumnName = pColumn->strName;
			long nListType = LIST_TYPE_TEXT;
			BOOL bIsGrouped = FALSE;
			long nSortOrder = i+1;

			//(e.lally 2011-12-08) PLID 46471
			BOOL bIsCurrentMedOrAllergyUsageCol = (i==0 && nListType == LIST_TYPE_CHECKBOX && (pDetail->IsCurrentMedicationsTable() || pDetail->IsAllergiesTable()));

			//add to the detail
			// (z.manning 2010-07-29 15:10) - PLID 36150 - Pass in empty string for sentence format
			// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
			// (j.jones 2011-03-09 09:05) - PLID 42283 - send -1 for nEMCodeCategoryID
			// (z.manning 2011-03-14) - PLID 42778 - Pass in false for bHasDropdownElements as that's not supported
			// at this point with these tables.
			//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
			// (z.manning 2011-03-21 12:01) - PLID 30608 - Added param for autofill type
			// (z.manning 2011-05-26 15:51) - PLID 43865 - Set flags to zero
			// (z.manning 2011-09-19 14:52) - PLID 41954 - Set default values for dropdown separators
			// (z.manning 2011-11-07 11:09) - PLID 46309 - Added default value for SpawnedItemsSeparator
			//(e.lally 2011-12-08) PLID 46471 - Added bIsCurrentMedOrAllergyUsageCol. I am not aware of a way for this to be set here.
			// (r.gonet 08/03/2012) - PLID 51948 - Added a default value for Wound care data type
			// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
			pDetail->AddColumn(nColumnID, strColumnName, nListType, bIsGrouped, "", 2, "", lstDefault, FALSE, FALSE, "", etantPerRow, "", FALSE, FALSE, godtInvalid, -1, etatNone, 0, ", ", ", ", ", ", bIsCurrentMedOrAllergyUsageCol, wcdtNone);
		}

		//there is no need to reload content because we just filled it.
		pDetail->SetNeedContentReload(FALSE);

		_variant_t varState = _bstr_t("");
		CString strState = "";

		//generate our state
		for(int i=0;i<pTableContent->aryCells.GetSize();i++) {

			CellElement *pCell = pTableContent->aryCells.GetAt(i);

			long nDataID_X = (long)pCell->pRow;
			long nDataID_Y = (long)pCell->pColumn;

			CString strData = pCell->strValue;
			// (z.manning 2011-03-02 15:29) - PLID 42335 - Added -1 for nStampID
			AppendTableStateWithUnformattedElement(strState, nDataID_X, nDataID_Y, strData, -1, NULL, -1);
		}

		varState = _bstr_t(strState);
		
		pDetail->RequestStateChange(varState);

		//Pass in TRUE, to tell the dialog that this detail is an independent copy.
		//Pass NULL as the real detail
		dlg.SetDetail(pDetail, TRUE, NULL);

		//Give it a blank linked detail
		dlg.m_strLinkedItemList = "; ;";

		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::RButtonUpImportList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		//Show the menu only on the parent row
		if(pRow != NULL) {
			m_pList->CurSel = pRow;
			//TES 4/22/2011 - PLID 43252 - Update the preview with the new selection.
			UpdatePreview();

			if(pRow->GetParentRow() == NULL) {
				
				CMenu mnu;
				mnu.CreatePopupMenu();
				long nIndex = 0;
				
				long nPatientID = VarLong(pRow->GetValue(dlcPatientID));
				mnu.InsertMenu(nIndex++, MF_BYPOSITION|((nPatientID == -1) ? MF_DISABLED : MF_ENABLED), IDM_GO_TO_PATIENT, "Go To &Patient");
				//(e.lally 2011-04-06) PLID 42734 - Option to unselect the patient for this row
				mnu.InsertMenu(nIndex++, MF_BYPOSITION|((nPatientID == -1) ? MF_DISABLED : MF_ENABLED), IDM_UNSELECT_PATIENT, "&Unselect Patient");
				mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
				mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_ENABLED, IDM_REMOVE, "&Remove Files");

				CRect rc;
				CWnd *pWnd = GetDlgItem(IDC_NEW);
				if (pWnd) {
					pWnd->GetWindowRect(&rc);
					mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
				} else {
					CPoint pt;
					GetCursorPos(&pt);
					mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
				}
			}
			else {
				//(e.lally 2011-05-24) PLID 42819 - Is this selection a PDF for a generic file import device
				CString strDeviceFile = VarString(pRow->GetParentRow()->GetValue(dlcDeviceFileName));
				CString strFullPath = VarString(pRow->GetValue(dlcFilePath));
				CString strExtension = FileUtils::GetFileExtension(strFullPath);
				// (b.savon 2011-9-14) PLID 45491 - Give the ability for PDFs in Generic Recursive File Imports to be combined.
				if(strExtension.CompareNoCase("pdf") == 0 && (strDeviceFile.CompareNoCase("GenericFileImport.nxdp") ==0 || strDeviceFile.CompareNoCase("GenericRecursiveFileImport.nxdp") ==0)  ){
					//It is a PDF for the generic file import device, show the split menu option.

					CMenu mnu;
					mnu.CreatePopupMenu();
					long nIndex = 0;
					mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_ENABLED, IDM_SPLIT_PDF, "&Split Into Single Pages");

					CRect rc;
					CWnd *pWnd = GetDlgItem(IDC_NEW);
					if (pWnd) {
						pWnd->GetWindowRect(&rc);
						mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
					} else {
						CPoint pt;
						GetCursorPos(&pt);
						mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
					}

				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::OnGoToPatient()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		if(pRow) {
			//Let's grab the patient ID
			long nPatientID = VarLong(pRow->GetValue(dlcPatientID), -1);

			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();

			if(pMainFrame != NULL) {
				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
			}

			if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

				//Flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView) {
					pView->UpdateView();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::OnRemove()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		if(pRow == NULL){
			return;
		}
		if(!pRow->GetExpanded()) {
			pRow->PutExpanded(VARIANT_TRUE);
		}

		if(pRow) {
			if(pRow->GetParentRow() == NULL) {
				if(!pRow->GetExpanded()) {
					pRow->PutExpanded(VARIANT_TRUE);
				}
				//(e.lally 2011-03-10) PLID 42740 - Warn the user that this is deleting files and can't be undone
				if(IDYES == MessageBox("This will permanently delete the files associated with this record and cannot be undone.\r\n"
					"Are you sure you wish to delete?", "Practice", MB_YESNO|MB_ICONWARNING)){
					//TES 4/22/2011 - PLID 43252 - Clear out the preview just in case it's locked the file we're planning to remove.
					ClearPreview();
					// (d.lange 2011-05-25 12:51) - PLID 43253 - Pass in TRUE because we are manually removing the files
					RemoveChildRowFiles(pRow, TRUE);
				}
			}
		}
		//TES 4/22/2011 - PLID 43252 - There's maybe a new row selected now, so update the preview window
		UpdatePreview();

	}NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-11-19 10:36) - PLID 41493 - modified this function to check if any row has been checked
// to be imported, instead of per patient record.
BOOL CDevicePluginImportDlg::CheckChildRowsSelected(IRowSettingsPtr pParentRow /*= NULL*/)
{
	try{
		if(pParentRow) {
			//Let's just check if any child rows are selected for the given parent row
			if(pParentRow->GetParentRow() == NULL) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = pParentRow->GetFirstChildRow();
				NXDATALIST2Lib::IRowSettingsPtr pLastChild = pParentRow->GetLastChildRow();
				
				while(pRow){
					if(!VarBool(pRow->GetValue(dlcChecked)))
						return FALSE;

					if(pLastChild->IsSameRow(pRow))
						break;

					pRow = pRow->GetNextRow();
				}
			}

		}else {
			// (d.lange 2010-11-19 15:16) - PLID 41493 - We'll check the entire datalist for checked rows
			for(IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE); pRow != NULL; pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE)) {
				if(pRow->GetParentRow() == NULL && VarBool(pRow->GetValue(dlcChecked))) {
					// The parent row is checked, therefore all the child rows are checked so lets skip all the child rows
					pRow = pRow->GetLastChildRow();
				}else {
					// At 
					if(VarBool(pRow->GetValue(dlcChecked))) {
						return TRUE;
					}
				}
				
			}
			return FALSE;
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (d.lange 2010-08-13 11:53) - PLID 38850 - We must check for valid history tab categories. This is called for every "batch" of files per patient
BOOL CDevicePluginImportDlg::ValidateCategories(CArray<ImportFile*, ImportFile*> &aryFilesToImport)
{
	try {
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM NoteCatsF");

		BOOL bValid = FALSE;
		for(int i = 0; i < aryFilesToImport.GetSize(); i++) {
			rs->MoveFirst();
			while(!rs->eof) {
				long nCatID = AdoFldLong(rs, "ID");
				if(aryFilesToImport[i]->nFileCategory == nCatID || aryFilesToImport[i]->nFileCategory == -1) {
					bValid = TRUE;
					break;
				}
				rs->MoveNext();
			}

			if(!bValid) 
				return FALSE;
		}	

	} NxCatchAll(__FUNCTION__);
	
	return TRUE;
}

// (d.lange 2010-10-20 16:53) - PLID 41006 - Update the ConfigRT record with the status of the Always Delete checkbox
void CDevicePluginImportDlg::OnBnClickedChkRemovefiles()
{
	try {
		SetRemotePropertyInt("RemoveNonImportedDeviceFiles", m_checkAlwaysDelete.GetCheck(), 0, GetCurrentUserName());

		if(IsDlgButtonChecked(IDC_CHK_REMOVEFILES)) {
			SetRemotePropertyInt("DeviceImport_AlwaysRemoveWarningSetting", 1, 0, GetCurrentUserName());	
		}else {
			SetRemotePropertyInt("DeviceImport_AlwaysRemoveWarningSetting", 0, 0, GetCurrentUserName());	
		}	

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-10-25 13:28) - PLID 41008 - called by MainFrame to remove rows for files
// we have been told were deleted (returns TRUE if something was removed from the screen)
//	(b.savon 2011-9-2) - PLID 45326 - This function also accepts Folder paths
BOOL CDevicePluginImportDlg::HandleDeletedFile(CString strDeletedFilePath)
{
	BOOL bDidWeRemoveSomething = FALSE;

	try {

		//loop through every row that references this file - it is likely
		//that a parent and one of its children references it, it is
		//far less likely that the same file will be referenced by
		//two parents or two children

		//we tracked all our paths as lowercase in a separate column
		//so we can compare easily, so do the same to this path we search on
		strDeletedFilePath.MakeLower();

		IRowSettingsPtr pRow = m_pList->FindByColumn(dlcFilePathSearchable, (LPCTSTR)strDeletedFilePath, m_pList->GetFirstRow(), FALSE);

		// (b.savon 2011-9-2) - PLID 45326 - Handle support for deleting folders for recursive plugins
		if( pRow == NULL ){
			if( HandleDeletedFolder(strDeletedFilePath) ){
				bDidWeRemoveSomething = TRUE;
			}
		} else {
			while(pRow) {

				// (b.savon 2011-9-2) - PLID 45326 - Handle support for deleting folders for recursive plugins
				if( RemoveRowFromDlg(pRow) ){
					bDidWeRemoveSomething = TRUE;
				}

				//start from the beginning and look for other references to the file
				//(it is not likely we will find any)
				pRow = m_pList->FindByColumn(dlcFilePathSearchable, (LPCTSTR)strDeletedFilePath, m_pList->GetFirstRow(), FALSE);
			}
			// (d.lange 2010-10-26 16:45) - PLID 41088 - We need to notify all open EMNs and History tab
			SendMessageToUpdateButtons();

		}
	}NxCatchAll(__FUNCTION__);

	//even if we had an exception, return what we know
	return bDidWeRemoveSomething;
}

// (b.savon 2011-9-2) - PLID 45326 - Handle support for deleting folders for recursive plugins
BOOL CDevicePluginImportDlg::RemoveRowFromDlg(IRowSettingsPtr pRow)
{
	BOOL bDidWeRemoveSomething = FALSE;

	// (b.savon 2011-11-29 17:21) - PLID 45326 - Although not possible in code now,
	// let's be safe for the future.
	if( pRow == NULL ){
		return bDidWeRemoveSomething;
	}

	// (d.lange 2011-03-10 10:03) - PLID 42754 - Track the ParentRowRecord pointer instead
	//track the patient element pointer
	ParentRowRecord *pePatientRecord = NULL;

	BOOL bRemovedParent = FALSE;

	//find the parent row
	IRowSettingsPtr pParentRow = pRow->GetParentRow();
	
	// (b.savon 2012-07-03 17:06) - PLID 41861 - Get the ParentRowRecord
	pePatientRecord = (ParentRowRecord*)((ChildRowRecord*)VarLong(pRow->GetValue(dlcPatientPointer)))->nParentPointer;

	//if this is the parent, and the option is checked to delete files,
	//delete them now, because we may have children that still have files
	if(pParentRow == NULL && IsDlgButtonChecked(IDC_CHK_REMOVEFILES)) {
		RemoveChildRowFiles(pRow, TRUE);
		bDidWeRemoveSomething = TRUE;
	}else {
		//just remove the row, it's either a child, or a parent
		//and we are not deleting files ourselves
		// (d.lange 2011-05-25 15:31) - PLID 43253 - If a todo has been created and we're removing the associated file, we need to remove the todo
		long nToDoTaskID = -1;
		if(pParentRow != NULL) {
			//The row is a child so check the Todo task ID
			nToDoTaskID = VarLong(pRow->GetValue(dlcToDoAlarmID), -1);
		}else if(pParentRow == NULL && pRow->GetFirstChildRow() != NULL) {
			//The row is a parent so we know that it will have only a single child
			nToDoTaskID = VarLong(pRow->GetFirstChildRow()->GetValue(dlcToDoAlarmID), -1);
		}
		
		if(nToDoTaskID != -1)
			TodoDelete(nToDoTaskID);

		m_pList->RemoveRow(pRow);
		bDidWeRemoveSomething = TRUE;
	}

	//if the parent has no more children, remove it too
	if(pParentRow != NULL && pParentRow->GetFirstChildRow() == NULL) {
		m_pList->RemoveRow(pParentRow);
		bRemovedParent = TRUE;
		bDidWeRemoveSomething = TRUE;
	}

	//if we removed a parent row, delete the patient element pointer
	if(bRemovedParent) {
		// (d.lange 2011-03-10 11:00) - PLID 42754 - Remove the ParentRowRecord from m_aryParentRowRecord
		DeleteParentRowRecord(pePatientRecord);
		pePatientRecord = NULL;
	}

	return bDidWeRemoveSomething;
}

// (b.savon 2011-9-2) - PLID 45326 - Handle support for deleting folders for recursive plugins
BOOL CDevicePluginImportDlg::HandleDeletedFolder(CString strDeletedFolderPath)
{
	BOOL bDidWeRemoveSomething = FALSE;

	try {

		//loop through every row that references this file - it is likely
		//that a parent and one of its children references it, it is
		//far less likely that the same file will be referenced by
		//two parents or two children

		//we tracked all our paths as lowercase in a separate column
		//so we can compare easily, so do the same to this path we search on
		strDeletedFolderPath.MakeLower();

		CArray<IRowSettingsPtr, IRowSettingsPtr> aryRows;
		IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while( pRow )
		{
			CString strFilePath = pRow->GetValue(dlcFilePathSearchable);
			strFilePath.MakeLower();
			if( strFilePath.Find(strDeletedFolderPath) >= 0 ){
				aryRows.Add(pRow);
			}

			pRow = pRow->GetNextRow();
		}

		int i;
		for( i = 0; i < aryRows.GetSize(); i++ ){
			if( RemoveRowFromDlg(aryRows.GetAt(i)) ){
				bDidWeRemoveSomething = TRUE;
			}
		}
		// (d.lange 2010-10-26 16:45) - PLID 41088 - We need to notify all open EMNs and History tab
		SendMessageToUpdateButtons();

	}NxCatchAll(__FUNCTION__);

	//even if we had an exception, return what we know
	return bDidWeRemoveSomething;
}

// (d.lange 2010-10-26 16:28) - PLID 41088 - Returns the status of the datalist (Empty or filled)
// (d.lange 2011-03-11 09:50) - PLID 42754 - Since there could be files to be imported but hidden, we no longer want to check
// the datalist but instead if m_aryParentRowRecord is empty
// (j.jones 2011-03-15 13:54) - PLID 42738 - this now returns a count of records
long CDevicePluginImportDlg::GetDeviceImportRecordCount()
{
	try {

		//this is the count of patient elements, not the count of their child records,
		//which, despite the naming suggesting otherwise, is what we really want
		return m_aryParentRowRecord.GetSize();

	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (d.lange 2010-10-26 16:36) - PLID 41088 - Sends the Device Import status to open PICs and History tab to update the Import button
void CDevicePluginImportDlg::SendMessageToUpdateButtons()
{
	try {
		CMainFrame *pMainFrame = GetMainFrame();
		CNxTabView *pView = (CNxTabView *)pMainFrame->GetOpenView(PATIENT_MODULE_NAME);

		//Let's grab the current status of the datalist, empty (FALSE) or filled (TRUE)
		// (j.jones 2011-03-15 13:54) - PLID 42738 - this now returns a count of records
		BOOL bStatus = (GetDeviceImportRecordCount() > 0);

		if(pMainFrame) {
			//Send the status of the Device Import dialog to all the PICs
			pMainFrame->BroadcastPostMessageToPICs(NXM_DEVICE_IMPORT_STATUS, (WPARAM)bStatus, 0);

			if(pView) {
				if(pView->GetActiveTab() == PatientsModule::HistoryTab) {
					//Send Message to the History tab to update its device import button
					pView->GetActiveSheet()->SendMessage(NXM_DEVICE_IMPORT_STATUS, (WPARAM)bStatus, 0);
				}
			}

			// (j.jones 2011-03-15 14:41) - PLID 42738 - if there are no files left,
			// remove any notifications that might exist
			if(GetDeviceImportRecordCount() == 0) {
				pMainFrame->UnNotifyUser(NT_DEVICE_IMPORT);
			}
			//if this dialog is not shown, and a notification already exists
			//for imported files, re-notify, which updates its count
			else if(!this->IsWindowVisible() && pMainFrame->IsNotificationTypeInQueue(NT_DEVICE_IMPORT)) {
				pMainFrame->NotifyUser(NT_DEVICE_IMPORT, "", false);
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-12-30 09:18) - PLID 41796 - loads the PDF dll if not already loaded,
// warns one time only if it failed, tells future callers if it is loaded
BOOL CDevicePluginImportDlg::IsPDFConverterLoaded()
{
	try {
		//if we tried to load already and failed, silently return
		if(m_bHasLoadedPDF && m_pPDF == NULL) {
			return FALSE;
		}
		//if we have already successfully loaded, return
		else if(m_bHasLoadedPDF && m_pPDF != NULL) {
			return TRUE;
		}
		else if(!m_bHasLoadedPDF) {
			//we have not yet tried to load the PDF dll

			//set this flag to true, even if the load fails
			//we need to know that we tried it
			m_bHasLoadedPDF = TRUE;

			HRESULT hr = m_pPDF.CreateInstance(__uuidof(PDFCreatorPilotLib::PDFDocument4));
			HRESULT hrBCL = m_pBCLPDF.CreateInstance(__uuidof(EasyPDFConverter::PDFConverter));

			if(FAILED(hr)) {
				if(hr == REGDB_E_CLASSNOTREG) {
					//the PDFCreatorPilot.dll is not registered, we need to register it!
					MessageBox("The PDFCreatorPilot.dll file is not properly installed on your system.\n"
						"Please contact NexTech for assistance.\n\n"
						"The ability to to convert PDFs to images will be disabled until this missing file has been properly installed.",
						"Practice", MB_ICONEXCLAMATION|MB_OK);
					m_pPDF = NULL;
					return FALSE;
				}
				else {
					//unknown error, the dll did not load, but not because it isn't registered
					CString strError;
					strError.Format("There was an unknown error (0x%x: %s) when trying to access the PDFCreatorPilot.dll.\n"
						"Please contact NexTech for assistance.\n\n"
						"The ability to to convert PDFs to images will be disabled until this file has been properly installed.", hr, FormatError((int)hr));
					MessageBox(strError, "Practice", MB_ICONEXCLAMATION|MB_OK);
					m_pPDF = NULL;
					return FALSE;
				}
			}

			if (FAILED(hrBCL)) {
				if (hrBCL == REGDB_E_CLASSNOTREG) {
					//the bepconv.dll is not registered, we need to register it!
					MessageBox("The bepconv.dll file is not properly installed on your system.\n"
						"Please contact NexTech for assistance.\n\n"
						"The ability to to convert PDFs to images will be disabled until this missing file has been properly installed.",
						"Practice", MB_ICONEXCLAMATION | MB_OK);
					m_pBCLPDF = NULL;
					return FALSE;
				}
				else {
					//unknown error, the dll did not load, but not because it isn't registered
					CString strError;
					strError.Format("There was an unknown error (0x%x: %s) when trying to access the bepconv.dll.\n"
						"Please contact NexTech for assistance.\n\n"
						"The ability to to convert PDFs to images will be disabled until this file has been properly installed.", hr, FormatError((int)hr));
					MessageBox(strError, "Practice", MB_ICONEXCLAMATION | MB_OK);
					m_pBCLPDF = NULL;
					return FALSE;
				}
			}
			
			//now that is is loaded, license it
			m_pPDF->SetLicenseData("NexTech", "EZ2PS-SRW5S-2WHKX-V8C33");
			m_pBCLPDF->PutLicenseKey(AsBstr("4755-F1FD-B427-C077-A2A0-CA9C")); // (b.savon 2015-02-04 10:41) - PLID 64186 - Add license

			return m_pPDF != NULL;
		}

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (j.jones 2010-12-30 12:06) - PLID 41796 - imports the given file to history and optionally EMR
// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate
// (d.lange 2011-05-23 14:27) - PLID 43253 - added todo task ID
BOOL CDevicePluginImportDlg::ImportSingleFile(CString strFilePath, long nPatientID, long nCategoryID, CString strFileDescriptionForHistory,
											  BOOL bImportToEMR, CString strDeviceName, CPicContainerDlg *pPicContainer, long nPicID,
											  COleDateTime dtServiceDate, long nToDoTaskID, OUT long *pnMailSentID)
{
	// (j.jones 2010-12-29 15:41) - PLID 41943 - importing may rename the file if the same filename
	// already exists, and since we may need to send that filename to the EMR, we need to track it
	CString strFinalFileName = "";
	
	//This whole idea is shaky but we shouldn't even pass an actual value to this, so reset it to -1 here. 
	// (b.spivey, November 3, 2015) PLID 67423 - just make sure this is absolutely set to nothing useable before it gets set properly in the call below it. 
	long nLocalMailSentID = -1;
	if (!pnMailSentID) {
		pnMailSentID = &nLocalMailSentID;
	}

	*pnMailSentID = -1;

	// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate
	// (d.lange 2011-05-25 09:58) - PLID 43253 - added OUT parameter for retrieving the MailSentID
	// (b.savon 2012-03-21 17:08) - PLID 48119 - Added silent flag to handle NULL pointer when inserting multiple files with the current same name in history
	// Basically, what would happen is there would be more than 1 file in the patients history with the same name as the files that were being imported.
	// When this would happen, the Rename dialog would appear and it would ask you to type the name for the new file.  When this box appeared,
	// the timer would fire in mainframe and it would process the plugins again after the box was closed.  Which in turn would refresh the list and give 
	// each row a new pointer while in midloop of importing.  Thus, making the pointer NOT NULL in the import loop, but the pointer would be invalid because 
	// the list was refreshed and new pointers were assigned.
	if(!ImportAndAttachFileToHistory(strFilePath, nPatientID, GetSafeHwnd(), nCategoryID, nPicID, TRUE,
		strFileDescriptionForHistory, &strFinalFileName, dtServiceDate, pnMailSentID, true)) {
		return FALSE;
	}
	//if it succeeded, see if we need to also send the image to the EMR
	else if(bImportToEMR && pPicContainer != NULL) {

		// (j.jones 2010-12-29 15:49) - PLID 41943 - the file name could have been renamed if a duplicate
		// filename existed, so make sure we use the strFinalFileName variable, which is the name of
		// the file that is now attached to the patient's history
		ASSERT(!strFinalFileName.IsEmpty());

		//send the filename & device name to the EMR, it will know that it is in the patient's documents folder
		// (j.jones 2010-10-29 11:13) - PLID 41187 - only send this info. if it is an image
		if(IsImageFile(strFinalFileName)) {
			pPicContainer->PostMessage(NXM_ADD_IMAGE_TO_EMR, (WPARAM)strFinalFileName.AllocSysString(), (LPARAM)strDeviceName.AllocSysString());
		}
	}

	// (d.lange 2011-05-25 09:22) - PLID 43253 - Update the ToDo alarm with the appropriate RegardingID, this links the Todo to the imported document
	UpdateTodoRegardingIDs(nToDoTaskID, *pnMailSentID, ttMailSent, nPatientID);

	return TRUE;
}

// (j.jones 2010-12-30 12:06) - PLID 41796 - converts the PDF file to images, importing those images into history and optionally EMR
// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate
// (d.lange 2011-05-23 14:28) - PLID 43253 - added todo task ID
// (r.gonet 06/11/2013) - PLID 56370 - Added Image DPI
// (b.savon 2014-12-03 10:43) - PLID 64186 - Added converter
BOOL CDevicePluginImportDlg::ConvertAndImportPDF(CString strPDFFilePath, long nImageDPI, long nPatientID, long nCategoryID,
												 CString strFileDescriptionForHistory,
												 BOOL bImportToEMR, CString strDeviceName, CPicContainerDlg *pPicContainer, long nPicID,
												 COleDateTime dtServiceDate, long nToDoTaskID, EPDFToImageConverter converter)
{
	CWaitCursor pWait1;

	// (b.savon 2014-12-03 10:45) - PLID 64186 - Do this if TwoPilots
	if (converter == pticTwoPilots){
		HRESULT hr = m_pPDF->Open((LPCTSTR)strPDFFilePath, (LPCTSTR)"");

		// (j.jones 2010-12-30) - I've never seen this fail, but we should check for it anyways
		if (hr != S_OK) {
			MessageBox(FormatString("An error occured while trying to convert the PDF file %s.\n\n"
				"Error 0x%x: %s", strPDFFilePath, hr, FormatError((int)hr)), "Practice", MB_OK | MB_ICONSTOP);
			return FALSE;
		}
	}
		
	CString strFileName = FileUtils::GetFileName(strPDFFilePath);
	CString strExtension = FileUtils::GetFileExtension(strFileName);
	if(strExtension.CompareNoCase("pdf") != 0) {
		//we should have never called this function
		ThrowNxException("PDF conversion failed on non-PDF file: %s", strPDFFilePath);
	}

	CString strFileNameNoExt = FileUtils::GetFileName(strFileName);
	strFileNameNoExt = strFileNameNoExt.Left(strFileNameNoExt.GetLength() - 4);

	// (r.gonet 06/11/2013) - PLID 56370 - At this point we should have a positive DPI or a value of -2 indicating we need to automatically choose one.
	// -1 is the placeholder for NULL. Should not exist here.
	ASSERT(nImageDPI != IMAGE_DPI_NULL);
	if (nImageDPI == IMAGE_DPI_NULL || nImageDPI == IMAGE_DPI_AUTO) {
		// (r.gonet 06/11/2013) - PLID 56370 - 120 is what we had hardcoded before we had variable DPIs.
		nImageDPI = 120;
	}

	// (b.savon 2014-12-03 13:34) - PLID 64186 - Convert using BCL (Alternative) -- otherwise, use TwoPilots (Default)
	if (converter == pticBCL){
		CString strExt = ".jpg";
		CString strPageSeparator = " - Page ";
		CString strTempFilePath = GetNxTempPath() ^ strFileNameNoExt;
		CString strNewFilePath = strTempFilePath + strExt;
		strTempFilePath += strPageSeparator;

		m_pBCLPDF->PDF2Image->ImageResolution = nImageDPI;
		m_pBCLPDF->PDF2Image->ImageColor = EasyPDFConverter::CNV_IMAGE_CLR_24BIT;
		m_pBCLPDF->PDF2Image->ImageFormat = EasyPDFConverter::CNV_IMAGE_FMT_JPEG;
		m_pBCLPDF->PDF2Image->ImageQuality = 85;

		m_pBCLPDF->PDF2Image->AddPageNumberForSingleFileOutput = VARIANT_TRUE;
		m_pBCLPDF->PDF2Image->MinimumPageNumberDigits = 1;
		m_pBCLPDF->PDF2Image->PageNumberSeparator = AsBstr(strPageSeparator);

		m_pBCLPDF->PDF2Image->PageConversionTimeout = 180 * 1000; // 3 min
		m_pBCLPDF->PDF2Image->FileConversionTimeout = 300 * 1000; // 5 min

		m_pBCLPDF->PDF2Image->Convert(AsBstr(strPDFFilePath), AsBstr(strNewFilePath));

		long nPageNumber = 1;
		bool bDone = false;
		while (!bDone){
			////attach this image to history and potentially EMR
			CString strNewFileName;
			strNewFileName.Format("%s%li", strTempFilePath, nPageNumber);

			// (b.savon 2015-02-09 12:05) - PLID 64186 - Use the friendly name
			CString strFileDescription;
			strFileDescription.Format("%s%li", (strFileDescriptionForHistory + strPageSeparator), nPageNumber++);

			if (FALSE == PathFileExists(strNewFileName + strExt)){
				bDone = true;
				break;
			}

			// (b.savon 2015-02-09 12:05) - PLID 64186 - Use the friendly name
			if (!ImportSingleFile(strNewFileName + strExt, nPatientID, nCategoryID, strFileDescription,
				bImportToEMR, strDeviceName, pPicContainer, nPicID, dtServiceDate, nToDoTaskID)) {

				//we failed to attach the file
				return FALSE;
			}

			//delete the temp image we just imported
			DeleteFile(strNewFileName + strExt);
		}
	}
	else if (converter == pticTwoPilots){

		//convert each page to an image
		long nPageCount = m_pPDF->GetPageCount();

		//sanity check - warn if the file has more than 20 pages
		if (nPageCount > 20) {
			CString strWarning;
			strWarning.Format("The PDF file %s has %li pages. Are you sure you want to convert this PDF to images?\n\n"
				"Selecting 'No' will import as a PDF file only.", strPDFFilePath, nPageCount);
			int nRet = MessageBox(strWarning, "Practice", MB_ICONQUESTION | MB_YESNOCANCEL);
			if (nRet == IDCANCEL) {
				return FALSE;
			}
			else if (nRet == IDNO) {
				//import just the PDF
				return ImportSingleFile(strPDFFilePath, nPatientID, nCategoryID, strFileDescriptionForHistory,
					bImportToEMR, strDeviceName, pPicContainer, nPicID, dtServiceDate, nToDoTaskID);
			}
		}

		for (int i = 0; i < nPageCount; i++) {

			CString strNewFileName = strFileNameNoExt;
			CString strFileDescriptionToUse = strFileDescriptionForHistory;

			//if it is a multi-page PDF, number the images
			if (nPageCount > 1) {
				strNewFileName.Format("%s - Page %li.png", strFileNameNoExt, i + 1);

				//also append this count to our file description, if we have one
				if (!strFileDescriptionForHistory.IsEmpty()) {
					strFileDescriptionToUse.Format("%s - Page %li", strFileDescriptionForHistory, i + 1);
				}
			}
			else {
				strNewFileName.Format("%s.png", strFileNameNoExt);
			}

			//save to the temp folder
			CString strNewFilePath = GetNxTempPath() ^ strNewFileName;

			CWaitCursor pWait2;

			m_pPDF->SavePageAsPNG(i, (LPCTSTR)strNewFilePath, nImageDPI, nImageDPI);

			//attach this image to history and potentially EMR
			if (!ImportSingleFile(strNewFilePath, nPatientID, nCategoryID, strFileDescriptionToUse,
				bImportToEMR, strDeviceName, pPicContainer, nPicID, dtServiceDate, nToDoTaskID)) {

				//we failed to attach the file
				return FALSE;
			}

			//delete the temp image we just imported
			DeleteFile(strNewFilePath);
		}
	}

	//delete the original PDF
	DeleteFile(strPDFFilePath);

	return TRUE;
}

// (d.lange 2011-03-09 15:37) - PLID 41010 - check to see if the current record's patient is the active patient or has an EMN open
// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
BOOL CDevicePluginImportDlg::IsCurrentPatient(long nPatientUserDefinedID, CArray<long,long> &aryOpenEMRPatientIDs /*=NULL*/,  ADODB::_ConnectionPtr pCon /*= NULL*/)
{
	try {
		// (b.savon 2012-04-06 14:49) - PLID 49231 - Pankova doesnt want these files to show up if they are not tied to the current patient
		// when 'Show Files for this patient' is checked.
		// We want to display files that don't have a patient linked to it
		BOOL bShowFilesPref = GetRemotePropertyInt("DeviceImport_ShowFilesCurrentPatient", 1, 0, GetCurrentUserName(), true);
		if(nPatientUserDefinedID <= 0)
			return bShowFilesPref;

		if(aryOpenEMRPatientIDs.GetSize() > 0) {
			for(int i = 0; i < aryOpenEMRPatientIDs.GetSize(); i++) {
				if(nPatientUserDefinedID == aryOpenEMRPatientIDs[i])
					return TRUE;
			}

			// (b.savon 2011-9-12 11:37) - PLID 45433 - If there is a device plugin that defines a user id not in the NexTech patients,
			//	the record never shows in the import dialog on the initial load.
			// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
			if( !DoesPatientExistByUserDefinedID(nPatientUserDefinedID) ){
				return TRUE;
			} else{
				return FALSE;
			}
		}
		
		// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
		long nUserDefinedID = GetExistingPatientUserDefinedID(GetActivePatientID());
		if(nPatientUserDefinedID == nUserDefinedID)
			return TRUE;

		// (b.savon 2011-9-12 11:37) - PLID 45433 - If there is a device plugin that defines a user id not in the NexTech patients,
		//	the record never shows in the import dialog on the initial load.
		// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
		if( !DoesPatientExistByUserDefinedID(nPatientUserDefinedID) ){
			return TRUE;
		}
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (d.lange 2011-03-09 15:40) - PLID 41010 - This will cach the checkbox value and reload the the datalist based on checkbox value
void CDevicePluginImportDlg::OnBnClickedChkCurrentPatientFilter()
{
	try {
		SetRemotePropertyInt("DeviceImport_ChkCurrentPatientFilter", m_chkCurPatientFilter.GetCheck(), 0, GetCurrentUserName());
		
		// (b.savon 2011-9-9) - PLID 45314 - Reload our import list (LoadData() is handled inside SelChosen...)
		//LoadData(GetRemotePropertyText("DevicePluginImportFilter", "< All Enabled Plugins >", 0, GetCurrentUserComputerName()));
		Refresh();

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2011-03-10 09:32) - PLID 41010 - Load the datalist with objects based on the current patient filter
void CDevicePluginImportDlg::LoadData( CString strCurrentPluginFilter, BOOL bHardLoadAll /* = FALSE */ )
{
	try {

		// (b.savon 2011-9-9) - PLID 45314 - Load data based on plugin filter
		CString strPName;
		if(IsDlgButtonChecked(IDC_CHK_CURPATIENT_FILTER) && !bHardLoadAll) {
			m_pList->Clear();
			CArray<long,long> aryOpenEMRPatientIDs;
			GetMainFrame()->GetOpenEMRPatientIDs(aryOpenEMRPatientIDs);
			for(int i = 0; i < m_aryParentRowRecord.GetSize(); i++) {
				// (b.savon 2011-9-9) - PLID 45314 - Add row to list if we the record matches the current filter
				strPName = m_aryParentRowRecord[i]->strPluginName;
				if(IsCurrentPatient(m_aryParentRowRecord[i]->nPatientUserDefinedID, aryOpenEMRPatientIDs) &&
					(strPName == strCurrentPluginFilter || strCurrentPluginFilter == "< All Enabled Plugins >") )
					AddParentRow(m_aryParentRowRecord[i]);
			}
		}else {
			m_pList->Clear();
			for(int i = 0; i < m_aryParentRowRecord.GetSize(); i++) {
				// (b.savon 2011-9-9) - PLID 45314 - Add row to list if we the record matches the current filter	
				strPName = m_aryParentRowRecord[i]->strPluginName;
				if( strPName == strCurrentPluginFilter || strCurrentPluginFilter == "< All Enabled Plugins >" )
					AddParentRow(m_aryParentRowRecord[i]);
			}
		}

		// (b.savon 2012-02-21 09:32) - PLID 46455 - Commented out the section to select the first row
		//TES 4/21/2011 - PLID 43252 - After loading, set the selection to the first row,
		// and show it in the preview
		//m_pList->CurSel = m_pList->GetFirstRow();
		UpdatePreview();

		// (b.savon 2012-02-21 09:32) - PLID 46455 - Moved this here 
		// (b.savon 2012-02-01 13:18) - PLID 47651 - Set the current row were working on
		//Only set the row if the file path is not empty and the current entry was not the
		//one removed from the list.
		if( !m_strSelectedFilePath.IsEmpty() ){
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindByColumn(dlcFilePath, _bstr_t(m_strSelectedFilePath), m_pList->GetFirstRow(), VARIANT_FALSE);
			if( pRow ){
				m_pList->PutCurSel(pRow);
				//Give focus to highlist the row
				GetDlgItem(IDC_IMPORT_LIST)->SetFocus();
				//Let's be safe and ensure the row is in view
				m_pList->EnsureRowInView(pRow);
			}
		}

		// (b.savon 2013-05-23 17:08) - PLID 42902 - Go through and assign the categories
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDetectedDevicePluginCombo->GetCurSel();
		long nPluginID = -1; 
		VARIANT varCategoryID = g_cvarNull;
		if( pRow ){
			nPluginID = VarLong(pRow->GetValue(dpcID));
			varCategoryID = pRow->GetValue(dpcDefaultCategoryID);
		}
		if( nPluginID != -1 ){
			NXDATALIST2Lib::IRowSettingsPtr pIterate = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);	
			while( pIterate != NULL ){

				// (b.spivey, September 05, 2013) - PLID 58370 - Check for parent first. 
				//If it has a parent row, then it MUST be a child row. 
				if( pIterate->GetParentRow() != NULL) {
					// (b.spivey, September 4, 2013) - PLID 58370 - if there are no table contents, this can be set. 
					TableContent *pTC = NULL;
					ChildRowRecord* pCRR = (ChildRowRecord*)VarLong(pIterate->GetValue(dlcPatientPointer), NULL);
					
					if (pCRR && pCRR->reRecordElement) {
						pTC = pCRR->reRecordElement->pTableContent;
					}

					// (b.spivey, September 05, 2013) - PLID 58370 - classic short circuit, 
					//	 if this has table content then it'll not do anything. 
					if (!pTC && VarLong(pIterate->GetValue(dlcCategory), -1) < 0){
						// (b.spivey, August 29, 2013) - PLID 58370 - Only update this when there is nothing already set. 
						 pIterate->PutValue(dlcCategory, varCategoryID); 
					}
				}
				pIterate = m_pList->FindAbsoluteNextRow(pIterate, VARIANT_FALSE);
			}
		}else{
			NXDATALIST2Lib::IRowSettingsPtr pIterate = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);	
			NXDATALIST2Lib::IRowSettingsPtr pParent = NULL;
			while( pIterate != NULL ){
				if( (pParent = pIterate->GetParentRow()) != NULL ){
					//This is a child, get the parent, find the category id from the filter drop down, apply it
					pParent = m_pDetectedDevicePluginCombo->FindByColumn(dpcPluginName, pParent->GetValue(dlcPluginName), NULL, VARIANT_FALSE);
					// (b.spivey, September 4, 2013) - PLID 58370 - if there are no table contents, this can be set. 
					TableContent *pTC = NULL;
					ChildRowRecord* pCRR = (ChildRowRecord*)VarLong(pIterate->GetValue(dlcPatientPointer), NULL);
					if (pCRR && pCRR->reRecordElement) {
						pTC = pCRR->reRecordElement->pTableContent;
					}
					if (!pTC) {
						// (b.spivey, August 29, 2013) - PLID 58370 - Only update this when there is nothing already set. 
						if( pParent && VarLong(pIterate->GetValue(dlcCategory), -1) < 0) {
							pIterate->PutValue(dlcCategory, pParent->GetValue(dpcDefaultCategoryID)); 
						}
					}
				}
				pIterate = m_pList->FindAbsoluteNextRow(pIterate, VARIANT_FALSE);
			}			
		}

	} NxCatchAll("Error in LoadData");
}

// (d.lange 2011-03-09 19:44) - PLID 42754 - Handle the deletion of a patient record by destroying the objects and remove from m_aryParentRowRecord
void CDevicePluginImportDlg::DeleteParentRowRecord(ParentRowRecord *prParentRowRecord)
{
	try {
		for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
			if((long)m_aryParentRowRecord[j] == (long)prParentRowRecord) {
				delete (PatientElement*)m_aryParentRowRecord[j]->pePatientElement;
				for(int i = m_aryParentRowRecord[j]->aryChildRowRecords.GetSize() - 1; i >= 0; i--) {
					delete (ChildRowRecord*)m_aryParentRowRecord[j]->aryChildRowRecords[i];
					m_aryParentRowRecord[j]->aryChildRowRecords.RemoveAt(i);
				}
				delete (ParentRowRecord*)m_aryParentRowRecord[j];
				m_aryParentRowRecord.RemoveAt(j);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::DeleteChildRowRecord(ChildRowRecord *crChildRowRecord)
{
	try {
		for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
			if(m_aryParentRowRecord[j] == (ParentRowRecord*)crChildRowRecord->nParentPointer) {
				for(int i = m_aryParentRowRecord[j]->aryChildRowRecords.GetSize() - 1; i >= 0; i--) {
					if(m_aryParentRowRecord[j]->aryChildRowRecords[i] == crChildRowRecord) {
						delete (ChildRowRecord*)m_aryParentRowRecord[j]->aryChildRowRecords[i];
						m_aryParentRowRecord[j]->aryChildRowRecords.RemoveAt(i);
					}
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}


void CDevicePluginImportDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	try {
		if(bShow) {
			Refresh();

			// (j.jones 2011-03-15 14:41) - PLID 42738 - remove any notifications that might exist,
			// because we are now viewing the dialog
			if(GetMainFrame()) {
				GetMainFrame()->UnNotifyUser(NT_DEVICE_IMPORT);
			}
		}
		else {
			//TES 4/28/2011 - PLID 43252 - Clear out the preview, if the dialog is being hidden there's no need for it to be locking any files.
			ClearPreview();
		}
	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::Refresh()
{
	try {

		LoadData(GetRemotePropertyText("DevicePluginImportFilter", "< All Enabled Plugins >", 0, GetCurrentUserComputerName()));
	
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-11 14:46) - PLID 42328 - bulk cache preferences in one function,
// as it can be called multiple times
void CDevicePluginImportDlg::CachePreferences() {

	// (b.savon 2011-10-11 16:07) - PLID 45314 - Added DevicePluginImportFilter
	g_propManager.CachePropertiesInBulk("DevicePluginImportDlg", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DevicePluginImportFilter' "
			")",
			_Q(GetCurrentUserComputerName()));

	//TES 4/21/2011 - PLID 43252 - Added DeviceImport_ShowPreview
	// (b.savon 2011-09-26 16:07) - PLID 42330 - Added DeviceImport_HideEMRTable
	// (b.savon 2012-03-26 16:18) - PLID 49143 - Added DeviceImport_MinimizePreference
	g_propManager.CachePropertiesInBulk("DevicePluginImportDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DeviceImport_SortOrder' OR "
			"Name = 'RemoveNonImportedDeviceFiles' OR "
			"Name = 'DeviceImport_ChkCurrentPatientFilter' OR "
			"Name = 'DeviceImport_AlwaysRemoveWarningSetting' OR "
			"Name = 'DeviceImport_ShowPreview' OR "
			"Name = 'DeviceImport_HideEMRTable' OR "
			"Name = 'DeviceImport_MinimizePreference' OR "
			"Name = 'DeviceImport_ShowFilesCurrentPatient' "
			")",
			_Q(GetCurrentUserName()));
}
		

// (j.jones 2011-03-11 14:41) - PLID 42328 - called when preferences have changed
void CDevicePluginImportDlg::OnPreferencesChanged()
{
	try {

		//if preferences changed, the cache has been wiped, so reload it
		CachePreferences();

		// (j.jones 2011-03-11 14:45) - PLID 42328 - check to see if the sort order changed
		EDeviceImportSortOrder eNewSortOrder = (EDeviceImportSortOrder)GetRemotePropertyInt("DeviceImport_SortOrder", (long)disoDateAsc, 0, GetCurrentUserName(), true);

		//only change the sort order if the preference changed
		if(m_eSortOrder != eNewSortOrder) {
			m_eSortOrder = eNewSortOrder;
			ApplySortOrder();
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-11 14:48) - PLID 42328 - will apply m_eSortOrder to the datalist,
// maintaining current rows if necessary
void CDevicePluginImportDlg::ApplySortOrder()
{
	try {

		IColumnSettingsPtr pColFileTime = m_pList->GetColumn(dlcFileTime);
		IColumnSettingsPtr pColPatientName = m_pList->GetColumn(dlcPatientName);

		switch(m_eSortOrder) {			
			case discDateDesc:
				//sort by date descending, patient name ascending
				pColFileTime->PutSortPriority(0);
				pColFileTime->PutSortAscending(VARIANT_FALSE);
				pColPatientName->PutSortPriority(1);
				pColPatientName->PutSortAscending(VARIANT_TRUE);				
				break;
			case discPatientNameAsc:
				//sort by patient name ascending, date ascending				
				pColPatientName->PutSortPriority(0);
				pColPatientName->PutSortAscending(VARIANT_TRUE);
				pColFileTime->PutSortPriority(1);
				pColFileTime->PutSortAscending(VARIANT_TRUE);
				break;
			case disoDateAsc:
				//sort by date ascending, patient name ascending
				pColFileTime->PutSortPriority(0);
				pColFileTime->PutSortAscending(VARIANT_TRUE);
				pColPatientName->PutSortPriority(1);
				pColPatientName->PutSortAscending(VARIANT_TRUE);
			default:
				break;
		}

		//re-sort the list, if we have content
		if(m_pList->GetRowCount() > 0) {
			//try to show the same rows we're currently viewing
			//ParentRowRecord *prRecordToReselect = NULL;
			IRowSettingsPtr pRow = m_pList->GetTopRow();
			ArrayParentRowPointers* aryPaRow = NULL;
			if(pRow) {
				// (b.savon 2012-04-09 15:55) - PLID 41861 - Let's try to match the pointer to the array now. 
				////make sure we're on a parent row
				while( pRow->GetParentRow() ){
					pRow = pRow->GetParentRow();
				}
				//prRecordToReselect = (ParentRowRecord*)VarLong(pRow->GetValue(dlcPatientPointer));
				aryPaRow = (ArrayParentRowPointers*)VarLong(pRow->GetValue(dlcPatientPointer));
			}
			//SortList(prRecordToReselect);
			SortList(aryPaRow);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-11 15:21) - PLID 42328 - if we need to force a datalist sort,
// this function will do so while trying to maintain the current row
// (b.savon 2012-04-09 15:57) - PLID 41861 - Respect the new DL structure, and attempt to Reselect based on the pointer to the array.
void CDevicePluginImportDlg::SortList(/*OPTIONAL IN ParentRowRecord *prRecordToReselect*/ OPTIONAL IN ArrayParentRowPointers* aryPaRow /*= NULL*/)
{
	try {

		//You can't call Sort() on a datalist 2 tree, so if the sort needs to change
		//then the content needs to be reloaded - which isn't so bad, because we need
		//to re-select the current row anyways, wherever it may be.

		//disabling the redraw during the load serves two purposes,
		//one is that the user won't see all the flicker from reloading the list,
		//and the other is that the redraw forces the scrollbar to recalculate,
		//without which functions like EnsureRowInView and PutTopRow *do not work*
		//when called from within the same function that reloaded the rows
		m_pList->SetRedraw(VARIANT_FALSE);

		//this re-adds all rows in sorted order
		LoadData(GetRemotePropertyText("DevicePluginImportFilter", "< All Enabled Plugins >", 0, GetCurrentUserComputerName()));

		m_pList->SetRedraw(VARIANT_TRUE);

		//if given a record to Reselect, do so now
		if(aryPaRow) {
			
			IRowSettingsPtr pRow = m_pList->FindByColumn(dlcPatientPointer, (long)aryPaRow, m_pList->GetFirstRow(), VARIANT_FALSE);
			if(pRow) {
				//PutTopRow is better than EnsureRowInView because we can see the expanded children.
				//If the row is the last entry in the list, the datalist still draws it correctly.
				m_pList->PutTopRow(pRow);
			}

		}



	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-04) PLID 42733
void CDevicePluginImportDlg::OnBnClickedDeviceCombineFilesBtn()
{
	try {

		//Traverse through all the data rows for each patient
		IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);
		long nPatientID = -1, nCategoryID =-1;
		BOOL bCanCombine = TRUE;
		CStringArray aryFilesToCombine;
		
		long nFirstPatientID = -1, nFirstCategoryID = -1;
		while(pRow)
		{
			IRowSettingsPtr pChildRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);

			if(VarBool(pRow->GetValue(dlcChecked)) && pRow->GetParentRow() != NULL)
			{
				//If the user has checked this row then we want to import to history
				// (z.manning 2013-10-25 12:42) - PLID 59188 - Added VarLong calls here
				nPatientID = VarLong(pRow->GetParentRow()->GetValue(dlcPatientID), -1);
				nCategoryID = VarLong(pRow->GetValue(dlcCategory), -1);
				//(e.lally 2011-04-21) PLID 43372
				CString strDeviceFile = VarString(pRow->GetParentRow()->GetValue(dlcDeviceFileName));
				CString strFullPath = VarString(pRow->GetValue(dlcFilePath));
				CString strExtension = FileUtils::GetFileExtension(strFullPath);
				if(strFullPath.IsEmpty() || strExtension.CompareNoCase("pdf") != 0){
						bCanCombine = FALSE;
						//The user has a non-pdf file selected
						AfxMessageBox("Only PDF files can be combined. Please unselect all other file types.");
						break;
				}
				//(e.lally 2011-04-21) PLID 43372
				// (b.savon 2011-9-14) PLID 45491 - Give the ability for PDFs in Generic Recursive File Imports to be combined.
				if(strDeviceFile.CompareNoCase("GenericFileImport.nxdp") !=0 &&
					strDeviceFile.CompareNoCase("GenericRecursiveFileImport.nxdp") != 0){
					bCanCombine = FALSE;
					//The user has a non-generic file import device file selected
						AfxMessageBox("Only the PDF files from the 'Generic File Import' device plugin can be combined. Please unselect the files from all other device plugins.");
						break;
				}
				else {

					if(nFirstPatientID == -1){
						nFirstPatientID = nPatientID;
					}
					else if(nPatientID != -1 && nFirstPatientID != nPatientID){
						bCanCombine = FALSE;
						//The user  practice patient, so we need to notify the user
						AfxMessageBox("There are at least two different patients selected for the PDF files to be combined. You must first select a single patient for all files being combined, unselect the file(s) not belonging to this patient, or unselect the patient on the other files before combining these files.");
						break;
					}

					if(nFirstCategoryID == -1){
						nFirstCategoryID = nCategoryID;
					}

					//We'll add this to an array for importing later because the user may decide not to import
					aryFilesToCombine.Add(strFullPath);
				}
					
			}
			pRow = pChildRow;
		}

		if(bCanCombine && aryFilesToCombine.GetCount() > 0){
			if(aryFilesToCombine.GetCount() > 1){
				//TES 4/28/2011 - PLID 43252 - Clear out the preview, so it doesn't lock the file it's showing and prevent the combine
				// dialog from being able to delete it.
				ClearPreview();
				CDeviceImportCombineFilesDlg dlg(this);
				dlg.LoadFiles(nFirstPatientID, nFirstCategoryID, aryFilesToCombine);
				dlg.DoModal();
			}
			else {
				AfxMessageBox("Please select 2 or more PDF files to combine.");
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-06) PLID 42734
void CDevicePluginImportDlg::OnUnselectPatient()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		if(pRow == NULL){
			return;
		}
		if(!pRow->GetExpanded()) {
			pRow->PutExpanded(VARIANT_TRUE);
		}

		if(pRow) {
			if(pRow->GetParentRow() == NULL) {
				if(!pRow->GetExpanded()) {
					pRow->PutExpanded(VARIANT_TRUE);
				}
				//(e.lally 2011-04-06) PLID 42734 - Warn the user
				if(IDYES != MessageBox("Are you sure you want to unselect the patient on this record?", "Practice", MB_YESNO|MB_ICONWARNING)){
					return;
				}

				pRow->PutValue(dlcPatientID, (long)-1);
				pRow->PutValue(dlcUserDefinedID, (long)-1);
				pRow->PutValue(dlcPatientName, _bstr_t("< Select a Valid Patient >"));
				pRow->PutBackColor(rscUnselectedPatient);

				//(e.lally 2011-04-06) PLID 42734 - update the parent row record too
				// (b.savon 2012-04-09 16:03) - PLID 41861 - Respect the new DL structure and update all the children parent records
				ArrayParentRowPointers* aryPaRow = (ArrayParentRowPointers*)VarLong(pRow->GetValue(dlcPatientPointer));
				for( int i = 0; i < aryPaRow->aryParentRecordPointers.GetSize(); i++ )
				{
					ParentRowRecord *prParentRecord = (ParentRowRecord*)aryPaRow->aryParentRecordPointers.GetAt(i);
					prParentRecord->nPatientID = (long)-1;
					prParentRecord->nPatientUserDefinedID = (long)-1;
					prParentRecord->eRowStatusColor = rscUnselectedPatient;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::OnSelChangedImportList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//TES 4/21/2011 - PLID 43252 - Update the preview window to display the currently selected row's file
		UpdatePreview();
	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::OnToggleFilePreview()
{
	try {
		if(IsDlgButtonChecked(IDC_TOGGLE_FILE_PREVIEW)) {
			//TES 4/21/2011 - PLID 43252 - Show the preview window
			m_pBrowser->put_Visible(g_cvarTrue);

			//TES 4/21/2011 - PLID 43252 - Size the file list to 10 pixels above the preview window
			CRect rFileList;
			GetDlgItem(IDC_IMPORT_LIST)->GetWindowRect(&rFileList);

			CRect rPreview;
			GetDlgItem(IDC_FILE_PREVIEW)->GetWindowRect(&rPreview);

			// (b.savon 2012-02-21 09:31) - PLID 46455 - Changed from 10 to 5
			rFileList.SetRect(rFileList.left, rFileList.top, rFileList.right, rPreview.top-5);
			ScreenToClient(&rFileList);
			GetDlgItem(IDC_IMPORT_LIST)->MoveWindow(&rFileList);

			//TES 4/21/2011 - PLID 43252 - Remember the checkbox setting
			SetRemotePropertyInt("DeviceImport_ShowPreview", 1, 0, GetCurrentUserName());
		}
		else {
			//TES 4/21/2011 - PLID 43252 - Size the file list to line up with the bottom of the preview
			CRect rFileList;
			GetDlgItem(IDC_IMPORT_LIST)->GetWindowRect(&rFileList);

			CRect rPreview;
			GetDlgItem(IDC_FILE_PREVIEW)->GetWindowRect(&rPreview);

			rFileList.SetRect(rFileList.left, rFileList.top, rFileList.right, rPreview.bottom);
			ScreenToClient(&rFileList);
			GetDlgItem(IDC_IMPORT_LIST)->MoveWindow(&rFileList);

			//TES 4/21/2011 - PLID 43252 - Hide the preview window
			m_pBrowser->put_Visible(g_cvarFalse);

			//TES 4/21/2011 - PLID 43252 - Remember the checkbox setting
			SetRemotePropertyInt("DeviceImport_ShowPreview", 0, 0, GetCurrentUserName());
		}

		//TES 4/22/2011 - PLID 43252 - Make sure the preview window is displaying the right file
		UpdatePreview();

	}NxCatchAll(__FUNCTION__);
}

//TES 4/21/2011 - PLID 43252 - Reflect the currently selected file in the preview window
void CDevicePluginImportDlg::UpdatePreview()
{
	//TES 4/22/2011 - PLID 43252 - If the window is hidden, clear out the preview (it can only cause problems)
	//TES 4/28/2011 - PLID 43252 - Likewise if the dialog is hidden.
	if(!IsDlgButtonChecked(IDC_TOGGLE_FILE_PREVIEW) || !IsWindowVisible()) {
		ClearPreview();
	}
	else {
		IRowSettingsPtr pRow = m_pList->CurSel;
		CString strFileName;

		// b.spivey, October 14, 2015 - PLID 67356 
		bool bIsCCDAFile = false;
		MSXML2::IXMLDOMDocument2Ptr pDocument;

		//TES 4/21/2011 - PLID 43252 - Does this row, or any of its children, or, if it has no children, any of its siblings,
		// have an attached file?  Filter out .xml files, and any "file" that doesn't have an extension (which is probably 
		// just a pointer to XML data or somethingn anyway).
		if(pRow) {
			strFileName = VarString(pRow->GetValue(dlcFilePath),"");

			// (b.savon 2012-07-02 14:20) - PLID 41861 - Do we have a grouping?  If we do, attempt to get the first file we can preview
			if( strFileName.Find("|") != -1 ){
				strFileName = GetFirstValidPreviewFile(strFileName);
			}

			// b.spivey, October 14, 2015 - PLID 67356 
			if ((strFileName.GetLength() >= 4 && strFileName.Right(4).CompareNoCase(".xml") == 0))
			{
				pDocument = LoadXMLDocument(strFileName);
				if (CCDA_Document == GetXMLDocumentType(pDocument))
				{
					bIsCCDAFile = true;
				}
				else
				{
					strFileName = "";
				}
			}
			else if (strFileName.Find(".") == -1) {
				strFileName = "";
			}

			if(strFileName.IsEmpty()) {
				IRowSettingsPtr pChild = pRow->GetFirstChildRow();
				while(pChild && strFileName.IsEmpty()) {
					strFileName = VarString(pChild->GetValue(dlcFilePath),"");
					if((strFileName.GetLength() >= 4 && strFileName.Right(4).CompareNoCase(".xml") == 0) || strFileName.Find(".") == -1) {
						strFileName = "";
					}
					pChild = pChild->GetNextRow();
				}
			}
			if(strFileName.IsEmpty() && pRow->GetFirstChildRow() == NULL) {
				IRowSettingsPtr pSibling = pRow->GetNextRow();
				while(pSibling && strFileName.IsEmpty()) {
					strFileName = VarString(pSibling->GetValue(dlcFilePath),"");
					if((strFileName.GetLength() >= 4 && strFileName.Right(4).CompareNoCase(".xml") == 0) || strFileName.Find(".") == -1) {
						strFileName = "";
					}
					pSibling = pSibling->GetNextRow();
				}
				//TES 4/26/2011 - PLID 43252 - If that didn't work, try searching backwards among our siblings.
				if(pSibling == NULL) {
					pSibling = pRow->GetPreviousRow();
					while(pSibling && strFileName.IsEmpty()) {
						strFileName = VarString(pSibling->GetValue(dlcFilePath),"");
						if((strFileName.GetLength() >= 4 && strFileName.Right(4).CompareNoCase(".xml") == 0) || strFileName.Find(".") == -1) {
							strFileName = "";
						}
						pSibling = pSibling->GetPreviousRow();
					}
				}
			}
		}

		if(!strFileName.IsEmpty()) {
			//TES 4/21/2011 - PLID 43252 - Yes, so navigate to it.
			//TES 4/21/2011 - PLID 43252 - If this is a .pdf file, we want to append #toolbar=0, to hide the .pdf controls.
			if(strFileName.GetLength() >= 4 && strFileName.Right(4).CompareNoCase(".pdf") == 0) {
				strFileName += "#toolbar=0";
			}
			if(strFileName != m_strCurrentPreviewFile) {

				// b.spivey, October 15, 2015 - PLID 67303 - If we're a CCDA file, lets get the HTMLStream of the document and preview that. 
				//	 otherwise, do what we've always done. 
				if (bIsCCDAFile) {
					IPersistStreamInitPtr pPersistStreamPtr;

					IDispatchPtr pDoc;
					HR(m_pBrowser->get_Document(&pDoc));
					HR(pDoc->QueryInterface(IID_IPersistStreamInit, (void**)&pPersistStreamPtr));
					if (pPersistStreamPtr) {
						HR(pPersistStreamPtr->InitNew());
						HR(pPersistStreamPtr->Load(GetOutputHTMLStream(pDocument)));
						GetDlgItem(IDC_FILE_PREVIEW)->EnableWindow(TRUE);
					}
				}
				else {
					COleVariant varUrl(strFileName);
					m_pBrowser->Navigate2(varUrl, COleVariant((long)navNoHistory), NULL, NULL, NULL);
					GetDlgItem(IDC_FILE_PREVIEW)->EnableWindow(TRUE);
				}
			}
			m_strCurrentPreviewFile = strFileName;
		}
		else {
			//TES 4/21/2011 - PLID 43252 - No, so clear out the preview window
			ClearPreview();
		}
	}
}

#define IDT_DISABLE_FILE_BROWSER	1001
void CDevicePluginImportDlg::OnFileDownloadFilePreview(BOOL ActiveDocument, BOOL* Cancel)
{
	try {
		if(!ActiveDocument) {
			//TES 4/21/2011 - PLID 43252 - If we get here, that means its about to pop up a dialog to download the file.  We don't want 
			// that, so set the Cancel parameter to TRUE to cancel that.  We do want to go ahead and disable the browser, but if we do that
			// here, it won't actually work, we have to post a message to do it, we'll go ahead and just set a timer.
			SetTimer(IDT_DISABLE_FILE_BROWSER, 0, NULL);
			*Cancel = TRUE;
		}
	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::OnTimer(UINT_PTR nIDEvent)
{
	try {
		switch(nIDEvent) {
			case IDT_DISABLE_FILE_BROWSER:
				//TES 4/21/2011 - PLID 43252 - We've been asked to disable the browser, so go ahead and do that.
				KillTimer(IDT_DISABLE_FILE_BROWSER);
				ClearPreview();
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-21) PLID 43372
void CDevicePluginImportDlg::ShowCombinePdfsButton()
{
	try {
		GetDlgItem(IDC_DEVICE_COMBINE_FILES_BTN)->ShowWindow(TRUE);

	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginImportDlg::ClearPreview()
{
	//TES 4/22/2011 - PLID 43252 - Moved this code to its own function
	if(m_strCurrentPreviewFile != "about:blank") {
		m_pBrowser->Navigate2(COleVariant("about:blank"), COleVariant((long)navNoHistory), NULL, NULL, NULL);
		GetDlgItem(IDC_FILE_PREVIEW)->EnableWindow(FALSE);
	}
	m_strCurrentPreviewFile = "about:blank";
}

//TES 4/28/2011 - PLID 43252 - A local implementation, that accounts for the fact that the preview window sometimes holds a lock
// on the file it was previewing for a few seconds after being cleared.
void CDevicePluginImportDlg::DeleteFile(const CString &strFile)
{
	int nAttempts = 0;
	bool bSuccessfullyDeleted = false;
	//TES 4/28/2011 - PLID 43252 - Try to delete it, and if we get a sharing violation, wait a second and try again.  After 10 seconds,
	// give up.
	//TES 10/11/2012 - PLID 53147 - Actually, that's a terrible idea!  If we're willing to give up without successfully deleting, 
	// let's do it right away, not freeze the program for 10 seconds for no reason.  Instead we'll tell MainFrame to delete it whenever
	// it gets the chance.
	//TES 12/12/2012 - PLID 53147 - I was trying to do all kinds of elaborate things, but in fact there's already a function that does exactly
	// what we need.
	DeleteFileWhenPossible(strFile);
}

// (d.lange 2011-05-20 11:33) - PLID 43253 - Update the RegardingID of the given Todo, to link the alarm to the imported document
void CDevicePluginImportDlg::UpdateTodoRegardingIDs(long nTaskID, long nRegardingID, long nRegardingType, long nPersonID)
{
	try {
		ExecuteParamSql("UPDATE ToDoList SET RegardingID = {INT}, RegardingType = {INT}, PersonID = {INT} WHERE TaskID = {INT};",
					nRegardingID, nRegardingType, nPersonID, nTaskID);
	}NxCatchAll(__FUNCTION__);

}

//(e.lally 2011-05-24) PLID 42819
void CDevicePluginImportDlg::OnSplitPdf()
{

	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		if(pRow == NULL){
			return;
		}

		if(pRow->GetParentRow() != NULL) {
			//We're going to clear the preview here, because the message box will actually help pump messages and allow 
			//the file handle to get fully released.
			m_pList->CurSel = NULL;
			ClearPreview();

			if(IDYES != MessageBox("Any information selected for this record will be lost and the file will be permanently deleted.\r\n"
					"Are you sure you wish to split this PDF file into single page files?", "Practice", MB_YESNO)){
				return;
			}

			CString strDeviceFile = VarString(pRow->GetParentRow()->GetValue(dlcDeviceFileName));
			CString strFullFilePath = VarString(pRow->GetValue(dlcFilePath));
			CString strExtension = FileUtils::GetFileExtension(strFullFilePath);
			// (b.savon 2011-9-14) PLID 45491 - Give the ability for PDFs in Generic Recursive File Imports to be combined.
			if(strExtension.CompareNoCase("pdf") == 0 && (strDeviceFile.CompareNoCase("GenericFileImport.nxdp") ==0 || strDeviceFile.CompareNoCase("GenericRecursiveFileImport.nxdp") ==0)){
				CString strOutputDirectory = FileUtils::GetFilePath(strFullFilePath);
				CString strFileNameNoExt = FileUtils::GetFileName(strFullFilePath);
				strFileNameNoExt.Delete(strFileNameNoExt.GetLength() - 4, 4);
				// Open the file 
				{
					NexTech_COM::IPDFDocumentPtr inputDocument;
					inputDocument.CreateInstance("NexTech_COM.PDFDocument");
					inputDocument->OpenPDFDocument(_bstr_t(strFullFilePath), NexTech_COM::PDFDocumentOpenMode_Import);
					long nPageCount = inputDocument->GetPageCount();
					long nTotalDigits = 1;
					if(nPageCount <= 1){
						//We're not going to even attempt the split for single page files.
						MessageBox("This file only has 1 page and cannot be split.", "Practice", MB_OK|MB_ICONWARNING);
						return;
					}
					if(nPageCount >= 10){
						if(nPageCount <= 99){
							nTotalDigits = 2;
						}
						else if(nPageCount >= 100 && nPageCount <= 999){
							nTotalDigits = 3;
						}
						else {
							//else it is a gigantic pdf!
							nTotalDigits = 6;
						}
					}

					for (int idx = 0; idx < nPageCount; idx++) {

						// Create new document 
						NexTech_COM::IPDFDocumentPtr outputDocument;
						outputDocument.CreateInstance("NexTech_COM.PDFDocument");

						outputDocument->SetVersion(inputDocument->GetVersion());
						outputDocument->SetTitle(inputDocument->GetTitle());
						outputDocument->SetCreator(inputDocument->GetCreator());

						// Add the page and save it 
						NexTech_COM::IPDFPagePtr pPage;
						outputDocument->AddPage(inputDocument->GetPage(idx));
						CString strOutFile;
						//We may need to add zero padding to the page number in order to keep the sorting in tact
						CString strPageNumber = FormatString("00000%li", idx+1).Right(nTotalDigits);
						strOutFile.Format("%s - Page %s.pdf", strFileNameNoExt, strPageNumber);
						strOutFile = strOutputDirectory ^ strOutFile;
						outputDocument->SavePDFDocument(_bstr_t(strOutFile));
						outputDocument->ClosePDFDocument();
					}
				}
				
				//Finally, delete the source file we split.
				DeleteFile(strFullFilePath);

			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.lange 2011-06-01 17:50) - PLID 43253 - Listen for a todo table checker and update the datalist if the todo task was removed
// (s.tullis 2014-08-13 14:37) - PLID 63225 - don't run recordset if -1 taskID and only run the recordset if the todo ID is in the list
LRESULT CDevicePluginImportDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		if(wParam == NetUtils::TodoList) {
			long nTaskID = lParam;
			if (nTaskID != -1){
				//Since the todo is no longer available we need to update the datalist
				for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE); pRow != NULL; pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE)) {
					if (pRow->GetParentRow() != NULL && nTaskID == VarLong(pRow->GetValue(dlcToDoAlarmID), -1)) {
						//we only want to run the recordset if the Todo value exists in the list
						if (!ReturnsRecordsParam("SELECT TaskID FROM TodoList WHERE TaskID = {INT}", nTaskID)){
							pRow->PutValue(dlcToDoAlarmID, -1);
							pRow->PutValue(dlcCreateToDoAlarm, g_cvarFalse);
						}
					}
				}
				
			}
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}




// (s.tullis 2014-08-13 14:37) - PLID 63225 - Ex todo Support
LRESULT CDevicePluginImportDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {
		CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
		if (wParam == NetUtils::TodoList) {
			
			ReflectChangedTodo(pDetails);
			
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (s.tullis 2014-08-13 14:37) - PLID 63225 - Ex todo Support
void CDevicePluginImportDlg::ReflectChangedTodo(CTableCheckerDetails* pDetails){

	long nTaskID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTaskID), -1);
	long nPersonID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiPersonID), -1);
	TableCheckerDetailIndex::Todo_Status todoStatus = (TableCheckerDetailIndex::Todo_Status)VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTodoStatus), -1);
	// we only care if a todo was deleted
	if (todoStatus != TableCheckerDetailIndex::Todo_Status::tddisDeleted){
		return;
	}
	
	if (nTaskID != -1){
		//Since the todo is no longer available we need to update the datalist
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE); pRow != NULL; pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE)) {
			if (pRow->GetParentRow() != NULL && nTaskID == VarLong(pRow->GetValue(dlcToDoAlarmID), -1)) {
			
					pRow->PutValue(dlcToDoAlarmID, -1);
					pRow->PutValue(dlcCreateToDoAlarm, g_cvarFalse);
				
			}
		}

	}




}

// (j.jones 2011-08-25 14:59) - PLID 41683 - clear all records for a given plugin
void CDevicePluginImportDlg::RemoveRecordsForPlugin(CString strPluginName)
{
	try {
		// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device 
		//									 integration list per-device plugin setup 
		//
		//	We need to put all the loaded files into the list and then filter
		//		If we don't, we can get in a state where there are old plugin
		//		names in the list if the user changes the name in the config dlg
		if( m_strSelectedPlugin != "< All Enabled Plugins >" || strPluginName != "< All Enabled Plugins >" ){
			// (b.savon 2011-09-26 14:39) - PLID 42330 - Hard load all the records we have so that we don't miss any
			//											 when we want to remove/change/update files
			BOOL bIsCheckedOriginal = IsDlgButtonChecked(IDC_CHK_HIDE_EMR_TABLES);
			((CButton*)GetDlgItem(IDC_CHK_HIDE_EMR_TABLES))->SetCheck(BST_UNCHECKED);

			LoadData("< All Enabled Plugins >", TRUE);

			// (b.savon 2011-09-26 14:39) - PLID 42330 - Re-check filter if we had it originally.
			if( bIsCheckedOriginal ){
				((CButton*)GetDlgItem(IDC_CHK_HIDE_EMR_TABLES))->SetCheck(BST_CHECKED);
			}
		}

		IRowSettingsPtr pRow = m_pList->FindByColumn(dlcPluginName, (LPCTSTR)strPluginName, m_pList->GetFirstRow(), FALSE);
		//	The new strPluginName cannot be found because all the records still reflect the old name.
		while(pRow) {

			ParentRowRecord *pePatientRecord = NULL;

			BOOL bRemovedParent = FALSE;

			//find the parent row
			IRowSettingsPtr pParentRow = pRow->GetParentRow();
			
			// (b.savon 2012-07-03 17:06) - PLID 41861 - Get the ParentRowRecord
			pePatientRecord = (ParentRowRecord*)((ChildRowRecord*)VarLong(pRow->GetValue(dlcPatientPointer)))->nParentPointer;

			//remove the row, it's either a child, or a parent
			//if a todo has been created and we're removing the associated file, we need to remove the todo
			long nToDoTaskID = -1;
			if(pParentRow != NULL) {
				//The row is a child so check the Todo task ID
				nToDoTaskID = VarLong(pRow->GetValue(dlcToDoAlarmID), -1);
			}else if(pParentRow == NULL && pRow->GetFirstChildRow() != NULL) {
				//The row is a parent so we know that it will have only a single child
				nToDoTaskID = VarLong(pRow->GetFirstChildRow()->GetValue(dlcToDoAlarmID), -1);
			}
			
			if(nToDoTaskID != -1) {
				TodoDelete(nToDoTaskID);
			}

			m_pList->RemoveRow(pRow);

			//if the parent has no more children, remove it too
			if(pParentRow != NULL && pParentRow->GetFirstChildRow() == NULL) {
				m_pList->RemoveRow(pParentRow);
				bRemovedParent = TRUE;
			}

			//if we removed a parent row, delete the patient element pointer
			if(bRemovedParent) {
				//remove the ParentRowRecord from m_aryParentRowRecord
				DeleteParentRowRecord(pePatientRecord);
				pePatientRecord = NULL;
			}

			//start from the beginning and look for other references to the plugin
			pRow = m_pList->FindByColumn(dlcPluginName, (LPCTSTR)strPluginName, m_pList->GetFirstRow(), FALSE);
		}

		//we need to notify all open EMNs and History tab
		SendMessageToUpdateButtons();

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
void CDevicePluginImportDlg::SelChosenDevicePluginSelect(LPDISPATCH lpRow)
{
	if( lpRow != NULL ){
		// Set the selected Text
		m_pDetectedDevicePluginCombo->PutComboBoxText(_bstr_t(m_pDetectedDevicePluginCombo->GetCurSel()->GetValue(dpcPluginName)));

		//	Get the selected device
		CString strSelectedDevice(m_pDetectedDevicePluginCombo->GetCurSel()->GetValue(dpcPluginName));
		m_strSelectedPlugin = strSelectedDevice;
		
		//	Load the data into the list
		LoadData(strSelectedDevice);

		//	Save filter for later use
		try{
			SetRemotePropertyText("DevicePluginImportFilter", strSelectedDevice, 0, GetCurrentUserComputerName());
		}NxCatchAll("void CDevicePluginImportDlg::SelChosenDevicePluginSelect - Unable to Save Filter");
	} else{
		m_pDetectedDevicePluginCombo->PutComboBoxText(_bstr_t(GetRemotePropertyText("DevicePluginImportFilter", "< All Enabled Plugins >", 0, GetCurrentUserComputerName())));
	}
}

// (b.savon 2011-09-26 14:39) - PLID 42330 - Apply the Hide EMR Table filter.
void CDevicePluginImportDlg::OnBnClickedChkHideEmrTables()
{
	// (b.savon 2011-09-26 14:39) - PLID 42330 - Save preference
	SetRemotePropertyInt("DeviceImport_HideEMRTable", IsDlgButtonChecked(IDC_CHK_HIDE_EMR_TABLES) ? BST_CHECKED : BST_UNCHECKED, 0, GetCurrentUserName());
	
	//	Load filtered data.
	LoadData(GetRemotePropertyText("DevicePluginImportFilter", "< All Enabled Plugins >", 0, GetCurrentUserComputerName()));
}

// (b.savon 2012-02-14 08:58) - PLID 46456 - Handle network disconnects gracefully.
void CDevicePluginImportDlg::RescanDevicePluginFolders()
{
	try{
		Cleanup();

	// (a.walling 2014-01-22 15:55) - PLID 60271 - Moved to DeviceImport::Monitor
		DeviceImport::GetMonitor().RescanDevicePluginFolders();

		// 3. Update List
		CString strSelectedDevice(m_pDetectedDevicePluginCombo->GetCurSel()->GetValue(dpcPluginName));
		LoadData( strSelectedDevice );
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-02-14 08:58) - PLID 46456 - Handle network disconnects gracefully. Pulled from the destructor
void CDevicePluginImportDlg::Cleanup()
{
	try {

		//When Mainframe sent over a PatientElement, this dialog checked for duplicates. For the duplicated ones,
		//we gave responsibility for deletion back to Mainframe, but for the PatientElement object that weren't matched
		//this dialog placed them in an array for deletion when the dialog is destroyed.
		
		// (d.lange 2011-03-10 14:42) - PLID 42754 - Destroy all objects
		for(int j = m_aryParentRowRecord.GetSize() - 1; j >= 0; j--) {
			delete (PatientElement*)m_aryParentRowRecord[j]->pePatientElement;
			for(int i = m_aryParentRowRecord[j]->aryChildRowRecords.GetSize() - 1; i >= 0; i--) {
				delete (ChildRowRecord*)m_aryParentRowRecord[j]->aryChildRowRecords[i];
				m_aryParentRowRecord[j]->aryChildRowRecords.RemoveAt(i);
			}
			delete (ParentRowRecord*)m_aryParentRowRecord[j];
			m_aryParentRowRecord.RemoveAt(j);
		}

		// (a.walling 2013-10-02 10:02) - PLID 58847 - map of icons no longer necessary, FileUtils maintains a cache by extension

		for( int i = 0; i < m_aryParentRowPointers.GetSize(); i++){
			delete (ArrayParentRowPointers*)m_aryParentRowPointers.GetAt(i);
		}
		m_aryParentRowPointers.RemoveAll();

	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-02-14 08:58) - PLID 46456 - Handle network disconnects gracefully.
BOOL CDevicePluginImportDlg::IsFileListUpToDate()
{
	try{
		//Vars
		CString strInvalidFiles = "";
		BOOL bHasInvalidFiles = FALSE;
		IRowSettingsPtr pRow = m_pList->GetFirstRow();

		//Go through the list and see if any of the files are missing that are checked to be imported
		while( pRow ){
			// (b.savon 2012-04-09 17:58) - PLID 49506 - Respect new DL structure.
			CArray<CString, CString> aryParentRowFilePaths;
			CString strDelimitedFilePath = VarString(pRow->GetValue(dlcFilePath));
			ParseParentRowFilePath(strDelimitedFilePath, aryParentRowFilePaths);
			for( int i = 0; i < aryParentRowFilePaths.GetSize(); i++){
				if( !FileUtils::DoesFileOrStreamExist(aryParentRowFilePaths.GetAt(i)) && VarBool(pRow->GetValue(dlcChecked)) ){

					strInvalidFiles += VarString(pRow->GetValue(dlcFilePath)) + "\n";
					
					bHasInvalidFiles = TRUE;
				}//END if(...)
			}
			pRow = pRow->GetNextRow();
		}//END while( pFiles )

		//If we have invalid files, alert the user of the situation, re-process the plugins, and relay the info back to the caller
		if( bHasInvalidFiles ){
			BOOL bSingleFile = strInvalidFiles.Find('\n') == strInvalidFiles.ReverseFind('\n') ? TRUE : FALSE;

			// 1. Alert the user
			MessageBox( (bSingleFile ? "File: \n\n" : "Files: \n\n") + 
						strInvalidFiles + 
						(bSingleFile ? "\nhas " : "\nhave ") + 
						"already been imported or no longer " +
						(bSingleFile ? "exists " : "exist ") +
						"in the directory.  The device import window "
						"will refresh with the available files.", 
						"Practice - Device Import", 
						MB_ICONINFORMATION);

			// 2. Rescan
			RescanDevicePluginFolders();

			// 3. Not up to date
			return FALSE;
		}

		//We made it, we're fine.
		return TRUE;
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (b.savon 2012-02-16 17:19) - PLID 46455 - Import dialog needs a facelift - Handle window resizing
void CDevicePluginImportDlg::OnSize(UINT nType, int cx, int cy)
{
	CNxDialog::OnSize(nType, cx, cy);

	try{
		// (b.savon 2012-02-16 17:19) - PLID 46455 - OnSize is called before OnInitDialog, so, we need to check if
		// the controls are created before we attempt to get its WindowRect.  Also, don't execute this if we are
		// minimizing the window or we will get an odd control overlap/redraw issue.
		if( GetDlgItem(IDC_IMPORT_LIST) && GetDlgItem(IDC_FILE_PREVIEW) && nType != SIZE_MINIMIZED  ){
			//Pulled from OnTogglePreviewWindow
			if(!IsDlgButtonChecked(IDC_TOGGLE_FILE_PREVIEW)) {
					//TES 4/21/2011 - PLID 43252 - Size the file list to line up with the bottom of the preview
					CRect rFileList;
					GetDlgItem(IDC_IMPORT_LIST)->GetWindowRect(&rFileList);

					CRect rPreview;
					GetDlgItem(IDC_FILE_PREVIEW)->GetWindowRect(&rPreview);

					rFileList.SetRect(rFileList.left, rFileList.top, rFileList.right, rPreview.bottom);
					ScreenToClient(&rFileList);
					GetDlgItem(IDC_IMPORT_LIST)->MoveWindow(&rFileList);

					//TES 4/21/2011 - PLID 43252 - Hide the preview window
					m_pBrowser->put_Visible(g_cvarFalse);
			}else{
					//TES 4/21/2011 - PLID 43252 - Show the preview window
					m_pBrowser->put_Visible(g_cvarTrue);

					//TES 4/21/2011 - PLID 43252 - Size the file list to 10 pixels above the preview window
					CRect rFileList;
					GetDlgItem(IDC_IMPORT_LIST)->GetWindowRect(&rFileList);

					CRect rPreview;
					GetDlgItem(IDC_FILE_PREVIEW)->GetWindowRect(&rPreview);

					rFileList.SetRect(rFileList.left, rFileList.top, rFileList.right, rPreview.top-10);
					ScreenToClient(&rFileList);
					GetDlgItem(IDC_IMPORT_LIST)->MoveWindow(&rFileList);
			}

			// (b.savon 2012-02-20 16:06) - PLID 46455 - Resize the DL columns as well
			UpdateFileListWidths();

			UpdatePreview();
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-02-20 16:06) - PLID 46455 - Keep the column widths at a proportional size to the window
void CDevicePluginImportDlg::UpdateFileListWidths()
{
	//Get Current Width
	CRect rctCurrentWindow;
	::GetClientRect(this->m_hWnd, &rctCurrentWindow);
	long nCurrentWindowWidth = rctCurrentWindow.Width();

	//Take original column width and multiply it by the current window width
	long fLHS = nCurrentWindowWidth*m_nOriginalColumnWidth;

	//Now divide that by the original list width;
	double fStar = (double)fLHS/(double)m_nOriginalListWidth;

	//Take the result and divide it by the original column width
	double fScale = fStar/(double)m_nOriginalColumnWidth;

	//Update the column widths
	double fTotal = 0;
	for (int i = 0; i < m_pList->ColumnCount; i++)
	{
		IColumnSettingsPtr pCol = m_pList->GetColumn(i);
		long nCurWidth = pCol->GetStoredWidth();
		double fPutWidth = ((double)nCurWidth)*fScale;
		long nPutWidth = (long)Round(fPutWidth, 0);

		pCol->PutStoredWidth(nPutWidth);
		fTotal += nPutWidth;
	}

	//Save the new widths as the 'original' to continue the resize process
	m_nOriginalColumnWidth = (long)fTotal;
	m_nOriginalListWidth = nCurrentWindowWidth;
}

// (b.savon 2012-03-12 14:44) - PLID 48816 - Manual Refresh Button
void CDevicePluginImportDlg::OnBnClickedBtnRefreshDi()
{
	try{

		RescanDevicePluginFolders();

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-24 12:55) - PLID 49143 - Handle Minimize Preference
void CDevicePluginImportDlg::HandleWindowAfterImport()
{
	try {
		//Check Preference
		BOOL bMinimize = FALSE;
		switch( m_muipMinimizePreference )
		{
			case muipAlways:
				bMinimize = TRUE;
				break;
			case muipEmptyList:
				{
					if( m_pList->GetRowCount() == 0 ){
						bMinimize = TRUE;
					}
				}
				break;
			case muipNever:
			default:
				bMinimize = FALSE;
				break;
		}

		//Minimize the window
		if( bMinimize ){
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			if (GetWindowPlacement(&wp)) {
				if (!IsIconic()) {
					wp.showCmd = SW_MINIMIZE;
					SetWindowPlacement(&wp);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-26 16:45) - PLID 49143 - Minimize Preference
void CDevicePluginImportDlg::OnBnClickedRadioAlways()
{
	try{

		m_muipMinimizePreference = muipAlways;
		SetRemotePropertyInt("DeviceImport_MinimizePreference", m_muipMinimizePreference, 0, GetCurrentUserName());

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-26 16:50) - PLID 49143 - Minimize Preference
void CDevicePluginImportDlg::OnBnClickedRadioNever()
{
	try{

		m_muipMinimizePreference = muipNever;
		SetRemotePropertyInt("DeviceImport_MinimizePreference", m_muipMinimizePreference, 0, GetCurrentUserName());

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-26 17:30) - PLID 49143 - Minimize Preference
void CDevicePluginImportDlg::OnBnClickedRadioEmptyList()
{
	try{

		m_muipMinimizePreference = muipEmptyList;
		SetRemotePropertyInt("DeviceImport_MinimizePreference", m_muipMinimizePreference, 0, GetCurrentUserName());

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-04-06 13:07) - PLID 41861 - Utility to help split the | delimeted parent file paths
int CDevicePluginImportDlg::ParseParentRowFilePath(CString strParentRowFilePath, CArray<CString, CString>& aryParentRowFilePaths)
{
	CString resToken;
	int curPos = 0;
	int count = 0;

	resToken= strParentRowFilePath.Tokenize(_T("|"),curPos);
	while (resToken != _T(""))
	{
		aryParentRowFilePaths.Add(resToken);
	   resToken = strParentRowFilePath.Tokenize(_T("|"), curPos);
	   ++count;
	};

	return count;
}

// (b.savon 2012-04-06 17:06) - PLID 49506 - The patients do not show up when the checkbox 'Show only files for current patient' 
// is checked when matching by patient demographics and id in the device import on the initial load.
void CDevicePluginImportDlg::FindAndFillPatient(ParentRowRecord* prParentRecord,  ADODB::_ConnectionPtr pCon /* = NULL */)
{
	long nMatchedID;
	
	nMatchedID = FindPatient(prParentRecord->pePatientElement, prParentRecord->eDevicePatientMatchRule, -1, pCon);

	prParentRecord->nPatientID = nMatchedID;

	if( nMatchedID != -1 ){
		prParentRecord->eRowStatusColor = rscMatchedPatient;
		// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
		prParentRecord->nPatientUserDefinedID = ::GetExistingPatientUserDefinedID(nMatchedID);
	}else{
		prParentRecord->nPatientUserDefinedID = -1;
		prParentRecord->eRowStatusColor = rscUnselectedPatient;
	}	
}

void CDevicePluginImportDlg::OnDestroy()
{
	CNxDialog::OnDestroy();

	try{
		WINDOWPLACEMENT wp;
		GetWindowPlacement(&wp);
		CString strBuffer;
		strBuffer.Format("%d,%d,%d,%d", wp.rcNormalPosition.left, wp.rcNormalPosition.top, wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);
		AfxGetApp()->WriteProfileString("Settings", "DeviceImportWindowSize", strBuffer);

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-07-02 14:41) - PLID 41861 - Get the first file in the pipe delimeted parent file path.
CString CDevicePluginImportDlg::GetFirstValidPreviewFile(CString strFileName)
{	
	CString strFirstFile = "";

	try{
		CArray<CString, CString> aryParentRowFilePaths;

		// Split pipe delimeted into array
		ParseParentRowFilePath(strFileName, aryParentRowFilePaths);

		// Run through our array, returning when we find the first valid file	
		for( int i = 0; i < aryParentRowFilePaths.GetCount(); i++ ){
			if( aryParentRowFilePaths.GetAt(i).Right(3).CompareNoCase("xml") != 0 && 
				aryParentRowFilePaths.GetAt(i).Find(".") != -1 && 
				aryParentRowFilePaths.GetAt(i).GetLength() >= 4){
				// We have a valid file
				strFirstFile = aryParentRowFilePaths.GetAt(i);
				return strFirstFile;
			}
		}

	}NxCatchAll(__FUNCTION__);
	
	return strFirstFile;
}
// EMRAnalysisDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRAnalysisDlg.h"
#include "EMRAnalysisConfigDlg.h"
#include "InternationalUtils.h"
#include "EMNDetail.h"
#include "EMN.h"
#include "EMRPreviewPopupDlg.h"
#include "EmrItemAdvTableDlg.h" // (b.savon 2011-11-02 11:03) - PLID 43205 
// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-10-14 16:29) - PLID 14411 - created

// CEMRAnalysisDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum ConfigComboColumns {

	cccID = 0,
	cccName,
	cccUseDateFilter,
	cccDateFrom,
	cccDateTo,
	cccFilterOnAllItems,
	cccRecordFilter,
	cccDisplayGroupBy,
	cccColumnGroupBy, // (c.haag 2009-02-24 12:01) - PLID 33187
	cccFilterByTemplateID,	// (j.jones 2009-03-27 11:03) - PLID 33703
	cccIncludeSpawningInfo, // (j.jones 2009-04-09 12:53) - PLID 33916
};

// (j.jones 2008-10-20 11:12) - this only tracks the initial fixed columns, the rest are dynamic
enum ResultListColumns {
	rlcPatientID = 0,
	// (j.jones 2009-04-08 14:39) - PLID 33915 - added provider & secondary provider
	rlcProviderName,
	rlcSecondaryProviderName,
	rlcUserDefinedID,
	rlcPatientName,
	rlcEMNID,
	rlcPreview, // (a.walling 2010-01-11 16:38) - PLID 31482
	rlcEMNDate,
	rlcEMNDescription,	
	rlcEMRID,
	rlcEMRDescription,
	rlcPicID,

	rlcFirstDynamicColumn,	//must always be the last in the list
};

// (c.haag 2009-02-24 16:30) - PLID 33187 - This is the maximum number of
// columns a datalist can have. We need to factor this in when calculating
// how many columns we need for items and groups. While the limit is technically
// 255 I think, we'll just cap it at 200 dynamic columns. That's a nice, rounder
// number for users. Not to mention it's still a lot of work to horizontally scroll.
// THIS VALUE MUST NOT EXCEED 255.
#define MAX_RESULT_COLUMNS			(MAX_ANALYSIS_DYNAMIC_COLUMNS + (long)rlcFirstDynamicColumn)

CEMRAnalysisDlg::CEMRAnalysisDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRAnalysisDlg::IDD, pParent)
{
	m_nCurrentConfigID = -1;
	m_bIsLoading = FALSE;

	// (a.walling 2010-01-11 12:11) - PLID 31482
	m_hIconPreview = NULL;
	m_pEMRPreviewPopupDlg = NULL;

	// (c.haag 2009-02-24 17:28) - PLID 33187 - Safety check. If this fails, you broke it.
	ASSERT(MAX_RESULT_COLUMNS <= 255);
}

void CEMRAnalysisDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMR_ANALYSIS_BKG, m_bkg);
	DDX_Control(pDX, IDC_BTN_EMR_ANALYSIS_EXPORT, m_btnExport);
	DDX_Control(pDX, IDC_BTN_EMR_ANALYSIS_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADD_EMR_ANALYSIS_CONFIG, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_EDIT_EMR_ANALYSIS_CONFIG, m_btnEdit);
	DDX_Control(pDX, IDC_BTN_DELETE_EMR_ANALYSIS_CONFIG, m_btnDelete);
	DDX_Control(pDX, IDC_RADIO_ALL_EMN_DATES_ANALYSIS, m_radioAllDates);
	DDX_Control(pDX, IDC_RADIO_EMN_DATE_ANALYSIS, m_radioEMNDate);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_PATIENT_ANALYSIS, m_radioGroupByPatient);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_DATE_ANALYSIS, m_radioGroupByDate);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_EMN_ANALYSIS, m_radioGroupByEMN);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_EMR_ANALYSIS, m_radioGroupByEMR);
	DDX_Control(pDX, IDC_EMN_FROM_DATE_ANALYSIS, m_dtFrom);
	DDX_Control(pDX, IDC_EMN_TO_DATE_ANALYSIS, m_dtTo);
	DDX_Control(pDX, IDC_BTN_RELOAD_RESULTS, m_btnReloadResults);
	DDX_Control(pDX, IDC_BTN_CANCEL_SEARCH, m_btnCancelSearch);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_PROGRESS_BAR_STATUS, m_nxeditProgressStatus);
	DDX_Control(pDX, IDC_RADIO_ONE_COL_PER_ITEM2, m_radioColumnGroupByItem);
	DDX_Control(pDX, IDC_RADIO_MINIMAL_COLUMNS2, m_radioColumnGroupByCondensed);
	DDX_Control(pDX, IDC_CHECK_SHOW_PROVIDER_NAME, m_checkShowProviderColumn);
	DDX_Control(pDX, IDC_CHECK_SHOW_SEC_PROVIDER_NAME, m_checkShowSecondaryProviderColumn);
	DDX_Control(pDX, IDC_CHECK_SHOW_EMN_DESCRIPTION, m_checkShowEMNDescColumn);
	DDX_Control(pDX, IDC_CHECK_SHOW_EMR_DESCRIPTION, m_checkShowEMRDescColumn);
}


BEGIN_MESSAGE_MAP(CEMRAnalysisDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_EMR_ANALYSIS_CLOSE, OnBtnEmrAnalysisClose)
	ON_BN_CLICKED(IDC_BTN_ADD_EMR_ANALYSIS_CONFIG, OnBtnAddEmrAnalysisConfig)
	ON_BN_CLICKED(IDC_BTN_EDIT_EMR_ANALYSIS_CONFIG, OnBtnEditEmrAnalysisConfig)
	ON_BN_CLICKED(IDC_BTN_DELETE_EMR_ANALYSIS_CONFIG, OnBtnDeleteEmrAnalysisConfig)
	ON_BN_CLICKED(IDC_RADIO_ALL_EMN_DATES_ANALYSIS, OnBnClickedRadioAllEmnDatesAnalysis)
	ON_BN_CLICKED(IDC_RADIO_EMN_DATE_ANALYSIS, OnBnClickedRadioEmnDateAnalysis)
	ON_BN_CLICKED(IDC_RADIO_GROUP_BY_PATIENT_ANALYSIS, OnBnClickedRadioGroupByPatientAnalysis)
	ON_BN_CLICKED(IDC_RADIO_GROUP_BY_DATE_ANALYSIS, OnBnClickedRadioGroupByDateAnalysis)
	ON_BN_CLICKED(IDC_RADIO_GROUP_BY_EMN_ANALYSIS, OnBnClickedRadioGroupByEmnAnalysis)
	ON_BN_CLICKED(IDC_RADIO_GROUP_BY_EMR_ANALYSIS, OnBnClickedRadioGroupByEmrAnalysis)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EMN_FROM_DATE_ANALYSIS, OnDtnDatetimechangeEmnFromDateAnalysis)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EMN_TO_DATE_ANALYSIS, OnDtnDatetimechangeEmnToDateAnalysis)
	ON_BN_CLICKED(IDC_BTN_RELOAD_RESULTS, OnBnClickedBtnReloadResults)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_CANCEL_SEARCH, OnBnClickedBtnCancelSearch)
	ON_BN_CLICKED(IDC_BTN_EMR_ANALYSIS_EXPORT, OnBtnEmrAnalysisExport)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PROVIDER_NAME, OnCheckShowProviderName)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SEC_PROVIDER_NAME, OnCheckShowSecProviderName)
	ON_BN_CLICKED(IDC_CHECK_SHOW_EMN_DESCRIPTION, OnCheckShowEmnDescription)
	ON_BN_CLICKED(IDC_CHECK_SHOW_EMR_DESCRIPTION, OnShowEmrDescription)
	// (a.walling 2010-01-11 13:21) - PLID 31482
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CEMRAnalysisDlg message handlers

BOOL CEMRAnalysisDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		// (a.walling 2010-01-11 12:11) - PLID 31482
		m_hIconPreview = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16,16, 0);

		// (j.jones 2009-04-08 17:39) - PLID 33915 - added some preferences here
		g_propManager.CachePropertiesInBulk("CEMRAnalysisDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EMRAnalysis_ShowProvider' OR "
			"Name = 'EMRAnalysis_ShowSecondaryProvider' OR "
			"Name = 'EMRAnalysis_ShowEMNDesc' OR "
			"Name = 'EMRAnalysis_ShowEMRDesc' "
			")",
			_Q(GetCurrentUserName()));

		//always set to the patient color
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// (j.jones 2008-10-22 16:17) - PLID 31790 - added ability to export as csv
		m_btnExport.AutoSet(NXB_EXPORT);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnReloadResults.AutoSet(NXB_INSPECT);
		m_btnCancelSearch.AutoSet(NXB_CANCEL);
		
		// (a.walling 2010-06-02 13:31) - PLID 39007 - Automatic loading can disable an office if the initial recordset takes forever.
		//LoadResults();
		m_btnReloadResults.SetWindowText("Load Results");

		//disable this button by default
		m_btnCancelSearch.EnableWindow(FALSE);

		//used to allow being minimized to the taskbar
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connections
		m_ConfigCombo = BindNxDataList2Ctrl(IDC_EMR_ANALYSIS_CONFIG_COMBO, GetRemoteDataSnapshot(), true);
		m_ResultsList = BindNxDataList2Ctrl(IDC_EMR_ANALYSIS_LIST, GetRemoteDataSnapshot(), false);

		//set our default filters
		m_dtFrom.SetValue(COleDateTime::GetCurrentTime());
		m_dtTo.SetValue(COleDateTime::GetCurrentTime());
		m_radioAllDates.SetCheck(TRUE);

		m_radioGroupByPatient.SetCheck(TRUE);

		// (c.haag 2009-02-24 12:27) - PLID 33187 - Default column grouping
		m_radioColumnGroupByCondensed.SetCheck(TRUE);

		//disable the filters
		DisableAllFilters();

		// (j.jones 2009-04-08 17:38) - PLID 33915 - added checkboxes to show/hide columns
		m_checkShowProviderColumn.SetCheck(GetRemotePropertyInt("EMRAnalysis_ShowProvider", 1, 0, GetCurrentUserName()) == 1);
		m_checkShowSecondaryProviderColumn.SetCheck(GetRemotePropertyInt("EMRAnalysis_ShowSecondaryProvider", 1, 0, GetCurrentUserName()) == 1);
		m_checkShowEMNDescColumn.SetCheck(GetRemotePropertyInt("EMRAnalysis_ShowEMNDesc", 1, 0, GetCurrentUserName()) == 1);
		m_checkShowEMRDescColumn.SetCheck(GetRemotePropertyInt("EMRAnalysis_ShowEMRDesc", 1, 0, GetCurrentUserName()) == 1);

		ToggleColumns();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CEMRAnalysisDlg::OnBtnEmrAnalysisClose()
{
	try {

		//try to save any filter changes we made (this will prompt first)
		IRowSettingsPtr pRow = m_ConfigCombo->GetCurSel();
		if(pRow) {
			if(!TrySaveFilterChanges(pRow)) {
				//if it returns FALSE, cancel this process
				return;
			}
		}

		//CNxDialog::OnOK();

		GetMainFrame()->PostMessage(NXM_EMR_ANALYSIS_CLOSED);

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBtnEmrAnalysisClose");
}

void CEMRAnalysisDlg::OnCancel()
{
	try {

		//disallow cancelling while loading, as only clicking the
		//"cancel search" button will safely clean up memory objects
		if(m_bIsLoading) {
			return;
		}

		//try to save any filter changes we made (this will prompt first)
		IRowSettingsPtr pRow = m_ConfigCombo->GetCurSel();
		if(pRow) {
			if(!TrySaveFilterChanges(pRow)) {
				//if it returns FALSE, cancel this process
				return;
			}
		}

		//CNxDialog::OnCancel();
		GetMainFrame()->PostMessage(NXM_EMR_ANALYSIS_CLOSED);

	}NxCatchAll("Error in CEMRAnalysisDlg::OnCancel");
}

void CEMRAnalysisDlg::OnBtnAddEmrAnalysisConfig()
{
	try {

		// (j.jones 2008-10-14 16:31) - PLID 31692 - create a new configuration
		CEMRAnalysisConfigDlg dlg(this);
		if(dlg.DoModal() == IDOK) {

			long nID = dlg.m_nID;

			if(nID != -1) {
				ClearResultsList();
				m_ConfigCombo->Requery();
				IRowSettingsPtr pRow = m_ConfigCombo->SetSelByColumn(cccID, nID);
				if(pRow) {
					OnSelChosenEmrAnalysisConfigCombo(pRow);
				}
			}
		}

	} NxCatchAll("Error in CEMRAnalysisDlg::OnBtnAddEmrAnalysisConfig");
}

void CEMRAnalysisDlg::OnBtnEditEmrAnalysisConfig()
{
	try {

		IRowSettingsPtr pRow = m_ConfigCombo->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must first select a configuration from the list before editing.");
			return;
		}

		long nID = VarLong(pRow->GetValue(cccID));

		// (j.jones 2008-10-14 16:31) - PLID 31692 - edit an existing configuration
		CEMRAnalysisConfigDlg dlg(this);
		dlg.m_nID = nID;

		//if we hit OK, reload the list, and re-select our configuration
		if(dlg.DoModal() == IDOK) {

			ClearResultsList();
			m_ConfigCombo->Requery();
			IRowSettingsPtr pRow = m_ConfigCombo->SetSelByColumn(cccID, nID);
			if(pRow) {
				//reset the current ID to -1, such that re-selecting
				//this configuration will reload the filters & results
				m_nCurrentConfigID = -1;
				OnSelChosenEmrAnalysisConfigCombo(pRow);
			}
		}

	} NxCatchAll("Error in CEMRAnalysisDlg::OnBtnEditEmrAnalysisConfig");
}

void CEMRAnalysisDlg::OnBtnDeleteEmrAnalysisConfig()
{
	try {

		IRowSettingsPtr pRow = m_ConfigCombo->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must first select a configuration from the list before deleting.");
			return;
		}

		if(IDNO == MessageBox("Are you sure you wish to delete the selected configuration?", "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {
			return;
		}

		long nID = VarLong(pRow->GetValue(cccID));
		
		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRAnalysisConfigDetailsT WHERE EMRAnalysisConfigID = %li", nID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRAnalysisConfigT WHERE ID = %li", nID);
		ExecuteSqlBatch(strSqlBatch);

		//remove the row and clear the list
		m_ConfigCombo->RemoveRow(pRow);
		ClearResultsList();

		//disable our filters
		DisableAllFilters();

		m_nCurrentConfigID = -1;
		
		// (a.walling 2010-07-28 17:17) - PLID 39007 - Set to 'Load Results'
		m_btnReloadResults.SetWindowText("Load Results");

		AfxMessageBox("This configuration has been deleted.");

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBtnDeleteEmrAnalysisConfig");
}

void CEMRAnalysisDlg::DisableAllFilters()
{
	try {

		GetDlgItem(IDC_RADIO_ALL_EMN_DATES_ANALYSIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_EMN_DATE_ANALYSIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMN_FROM_DATE_ANALYSIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMN_TO_DATE_ANALYSIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_GROUP_BY_PATIENT_ANALYSIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_GROUP_BY_DATE_ANALYSIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_GROUP_BY_EMN_ANALYSIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_GROUP_BY_EMR_ANALYSIS)->EnableWindow(FALSE);
		// (c.haag 2009-02-24 12:12) - PLID 33187 - Column group buttons
		m_radioColumnGroupByItem.EnableWindow(FALSE);
		m_radioColumnGroupByCondensed.EnableWindow(FALSE);

	}NxCatchAll("Error in CEMRAnalysisDlg::DisableAllFilters");
}

BEGIN_EVENTSINK_MAP(CEMRAnalysisDlg, CNxDialog)
	ON_EVENT(CEMRAnalysisDlg, IDC_EMR_ANALYSIS_CONFIG_COMBO, 16, OnSelChosenEmrAnalysisConfigCombo, VTS_DISPATCH)
	ON_EVENT(CEMRAnalysisDlg, IDC_EMR_ANALYSIS_LIST, 19, OnLeftClickEmrAnalysisList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRAnalysisDlg, IDC_EMR_ANALYSIS_CONFIG_COMBO, 1, OnSelChangingEmrAnalysisConfigCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CEMRAnalysisDlg::OnSelChosenEmrAnalysisConfigCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {

			//try to re-select the old row
			if(m_nCurrentConfigID != -1) {
				pRow = m_ConfigCombo->SetSelByColumn(cccID, m_nCurrentConfigID);
				if(pRow == NULL) {
					//this shouldn't be possible, find out how it happened
					ASSERT(FALSE);
				}
			}
			
			if(pRow == NULL) {
				//disable the filters and clear the list
				DisableAllFilters();
				ClearResultsList();
				return;
			}
		}

		long nID = VarLong(pRow->GetValue(cccID));

		if(m_nCurrentConfigID == nID) {
			//don't do anything if they are reselecting the same row
			return;
		}

		//before we do anything here, we should offer to save the old configuration if it changed.
		//We would like to do this in OnSelChanging, but having a messagebox in there will cause
		//OnSelChosen to never be called, so we can't do that.
		if(m_nCurrentConfigID != -1 && m_nCurrentConfigID != nID) {
			IRowSettingsPtr pOldRow = m_ConfigCombo->FindByColumn(cccID, m_nCurrentConfigID, m_ConfigCombo->GetFirstRow(), FALSE);
			if(pOldRow == NULL) {
				//this shouldn't be possible, find out how it happened
				ASSERT(FALSE);
			}
			else {

				// (j.jones 2008-10-20 10:23) - Christina said we do not need a warning here,
				// just always clear the configuration
				/*
				//if the results list is non-empty, warn that we will be clearing it
				if(m_ResultsList->GetRowCount() > 0) {
					if(IDNO == MessageBox("Selecting a new configuration will clear the current results list. Are you sure you wish to continue?",
						"Practice", MB_ICONQUESTION|MB_YESNO)) {
						//select the old row and return
						m_ConfigCombo->PutCurSel(pOldRow);
						return;
					}
				}
				*/

				//this will try and save changes to the old configuration (this prompts the user)
				if(!TrySaveFilterChanges(pOldRow)) {
					//if this returned false, we want to re-select the old row
					m_ConfigCombo->PutCurSel(pOldRow);
					return;
				}
			}
		}

		//set our current configuration ID
		m_nCurrentConfigID = nID;

		//clear the list
		ClearResultsList();

		//load the filters based on the selected configuration		
		BOOL bUseDateFilter = VarBool(pRow->GetValue(cccUseDateFilter), FALSE);
		_variant_t dtFrom = pRow->GetValue(cccDateFrom);
		_variant_t dtTo = pRow->GetValue(cccDateTo);
		BOOL bFilterOnAllItems = VarBool(pRow->GetValue(cccFilterOnAllItems), TRUE);
		EMRAnalysisRecordFilterType eRecordFilter = (EMRAnalysisRecordFilterType)VarLong(pRow->GetValue(cccRecordFilter), (long)earftPatient);
		EMRAnalysisGroupByType eGroupBy = (EMRAnalysisGroupByType)VarLong(pRow->GetValue(cccDisplayGroupBy), (long)eagbtPatient);
		// (c.haag 2009-02-24 12:01) - PLID 33187 - ColumnGroupBy
		EMRAnalysisColumnGroupByType eColumnGroupBy = (EMRAnalysisColumnGroupByType)VarLong(pRow->GetValue(cccColumnGroupBy), (long)eacgbtCondensed);

		//enable just our radio buttons
		GetDlgItem(IDC_RADIO_ALL_EMN_DATES_ANALYSIS)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_EMN_DATE_ANALYSIS)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_GROUP_BY_PATIENT_ANALYSIS)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_GROUP_BY_DATE_ANALYSIS)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_GROUP_BY_EMN_ANALYSIS)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_GROUP_BY_EMR_ANALYSIS)->EnableWindow(TRUE);
		// (c.haag 2009-02-24 12:12) - PLID 33187 - Column group buttons
		m_radioColumnGroupByItem.EnableWindow(TRUE);
		m_radioColumnGroupByCondensed.EnableWindow(TRUE);

		//set our on-screen filters
		m_radioAllDates.SetCheck(!bUseDateFilter);
		m_radioEMNDate.SetCheck(bUseDateFilter);
		GetDlgItem(IDC_EMN_FROM_DATE_ANALYSIS)->EnableWindow(bUseDateFilter);
		GetDlgItem(IDC_EMN_TO_DATE_ANALYSIS)->EnableWindow(bUseDateFilter);
		if(bUseDateFilter) {
			m_dtFrom.SetValue(dtFrom);
			m_dtTo.SetValue(dtTo);
		}
		else {
			m_dtFrom.SetValue(COleDateTime::GetCurrentTime());
			m_dtTo.SetValue(COleDateTime::GetCurrentTime());
		}

		m_radioGroupByPatient.SetCheck(eGroupBy == eagbtPatient);
		m_radioGroupByDate.SetCheck(eGroupBy == eagbtDate);
		m_radioGroupByEMN.SetCheck(eGroupBy == eagbtEMN);
		m_radioGroupByEMR.SetCheck(eGroupBy == eagbtEMR);
		// (c.haag 2009-02-24 12:02) - PLID 33187 - ColumnGroupBy
		m_radioColumnGroupByItem.SetCheck(eColumnGroupBy == eacgbtOnePerItem);
		m_radioColumnGroupByCondensed.SetCheck(eColumnGroupBy == eacgbtCondensed);

		//auto-reload the list, when selecting a new configuration,
		//if they want to abort, the cancel option is available

		// (a.walling 2010-06-02 13:31) - PLID 39007 - I am commenting this out because it is driving me mad!
		//LoadResults();
		m_btnReloadResults.SetWindowText("Load Results");

	} NxCatchAll("Error in CEMRAnalysisDlg::OnSelChosenEmrAnalysisConfigCombo");
}

void CEMRAnalysisDlg::OnLeftClickEmrAnalysisList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//our hyperlinks should go to the patient when clicking on their name,
		//go to EMN when clicking on the description, or go to the EMR when
		//clicking on its description

		long nPatientID = VarLong(pRow->GetValue(rlcPatientID));
		long nEMNID = VarLong(pRow->GetValue(rlcEMNID));
		long nEMRID = VarLong(pRow->GetValue(rlcEMRID));
		long nPicID = VarLong(pRow->GetValue(rlcPicID), -1);

		if(nCol == rlcPatientName) {

			if (nPatientID != -1) {
				//Set the active patient
				CMainFrame *pMainFrame;
				pMainFrame = GetMainFrame();
				if (pMainFrame != NULL) {

					if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
						if(IDNO == MessageBox("This patient is not in the current lookup. \n"
							"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
							return;
						}
					}
					//TES 1/7/2010 - PLID 36761 - This function may fail now
					if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

						//Now just flip to the patient's module and set the active Patient
						pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
						CNxTabView *pView = pMainFrame->GetActiveView();
						if(pView) {
							pView->UpdateView();
						}

						//we're done, do NOT close this screen, just return
						return;
					}
				}
				else {
					MsgBox(MB_ICONSTOP|MB_OK, "ERROR - EMRAnalysisDlg.cpp: Cannot Open Mainframe");
					return;
				}
			}
		}
		else if(nCol == rlcEMNDescription) {

			//edit the EMN

			//this logic is used in the lock manager, so I used the same phrasing here
			// (c.haag 2010-08-04 16:21) - PLID 39980 - We now accept -1 PicID's. Edit Emr Record will auto-generate the PIC.
			//if (nPicID == -1) {
			//	AfxMessageBox("This EMN record does not have an associated PIC entry. Data is intact, however the record must be opened manually from the Patients module.");
			//	return;
			//}

			//ensure this EMN has not been deleted
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM EMRMasterT WHERE Deleted = 0 AND ID = {INT}", nEMNID);
			if(rs->eof) {
				AfxMessageBox("This EMN has been deleted. Please click 'Reload Results' to update the results list with the current EMR information.");
				return;
			}
			rs->Close();

			//minimize the window first
			MinimizeWindow();

			GetMainFrame()->EditEmrRecord(nPicID, nEMNID);

		}
		else if(nCol == rlcEMRDescription) {

			//minimize the window first
			MinimizeWindow();

			//ensure this EMR has not been deleted
			// (c.haag 2010-08-04 16:21) - PLID 39980 - We now accept -1 PicID's. Edit Emr Record will auto-generate the PIC.			
			if (nPicID > -1) {
				//ensure this EMR has not been deleted
				_RecordsetPtr rs = CreateParamRecordset("SELECT PicT.ID FROM PicT "
					"INNER JOIN EMRGroupsT ON PicT.EMRGroupID = EMRGroupsT.ID "
					"WHERE EMRGroupsT.Deleted = 0 AND PicT.ID = {INT}", nPicID);

				if(rs->eof) {
					AfxMessageBox("This EMR has been deleted. Please click 'Reload Results' to update the results list with the current EMR information.");
					return;
				}
				rs->Close();

				//edit just the EMR
				GetMainFrame()->EditEmrRecord(nPicID);
			}
			else {
				// In the case of a -1 nPicID, Just test if the EMR Group is deleted by looking at its EMN if we have one
				if (nEMNID > -1) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 EMRGroupsT.ID FROM EMRGroupsT "
						"INNER JOIN EMRMasterT ON EMRMasterT.EmrGroupID = EMRGroupsT.ID "
						"WHERE EMRGroupsT.Deleted = 0 AND EmrMasterT.ID = {INT}", nEMNID);
					if(rs->eof) {
						AfxMessageBox("This EMR has been deleted. Please click 'Reload Results' to update the results list with the current EMR information.");
						return;
					}
					rs->Close();
					GetMainFrame()->EditEmrRecord(-1, nEMNID);
				}
				else {
					// If we get here because it means the PicID is -1 (which can happen) and the EmnID is -1 (I don't know
					// if this can happen). Lets try this from the EMR angle.
					if (-1 == nEMRID) {
						// No PIC, no EMR, nothing we can do. 
						ThrowNxException("Attempted to open an EMR without a valid EMR ID, EMN ID or Pic ID!");
					}
					else {
						// We have an EMR. Open to any EMN; it doesn't matter which one, so that
						// we can auto-create a PIC for it.
						_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM EmrMasterT WHERE EmrGroupID = {INT} AND Deleted = 0", nEMRID);
						if (prs->eof) {
							// We have no PIC, and the EMR has no EMN's.
							prs->Close();
							AfxMessageBox("The EMR could not be opened because it is not assigned to a PIC, and does not contain any EMN's.", MB_OK | MB_ICONERROR);
						}
						else {
							// We found an EMN. Launch the PIC (it will auto-create itself in data if necessary).
							long nEMNID = AdoFldLong(prs, "ID");
							prs->Close();
							GetMainFrame()->EditEmrRecord(-1, nEMNID);
						}
					}
				}
			}
		}		
		else if (nCol == rlcPreview) {
			// (a.walling 2010-01-11 16:47) - PLID 31482 - Show the preview
			ShowPreview(nPatientID, nEMNID);
		}

	} NxCatchAll("Error in CEMRAnalysisDlg::OnLeftClickEmrAnalysisList");
}
void CEMRAnalysisDlg::OnBnClickedRadioAllEmnDatesAnalysis()
{
	try {

		OnBnClickedRadioEmnDateAnalysis();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBnClickedRadioAllEmnDatesAnalysis");
}

void CEMRAnalysisDlg::OnBnClickedRadioEmnDateAnalysis()
{
	try {

		BOOL bEnabled = m_radioEMNDate.GetCheck();
		GetDlgItem(IDC_EMN_FROM_DATE_ANALYSIS)->EnableWindow(bEnabled);
		GetDlgItem(IDC_EMN_TO_DATE_ANALYSIS)->EnableWindow(bEnabled);

		//do not clear or refilter the list

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBnClickedRadioEmnDateAnalysis");
}

void CEMRAnalysisDlg::OnBnClickedRadioGroupByPatientAnalysis()
{
	try {
		
		//If we want the 'group by' filter changes to do something,
		//do it in this function. But right now we do not want the
		//'group by' changes to clear nor reload the list, so
		//currently this function does nothing.

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBnClickedRadioGroupByPatientAnalysis");
}

void CEMRAnalysisDlg::OnBnClickedRadioGroupByDateAnalysis()
{
	try {

		OnBnClickedRadioGroupByPatientAnalysis();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBnClickedRadioGroupByDateAnalysis");
}

void CEMRAnalysisDlg::OnBnClickedRadioGroupByEmnAnalysis()
{
	try {

		OnBnClickedRadioGroupByPatientAnalysis();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBnClickedRadioGroupByEmnAnalysis");
}

void CEMRAnalysisDlg::OnBnClickedRadioGroupByEmrAnalysis()
{
	try {

		OnBnClickedRadioGroupByPatientAnalysis();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBnClickedRadioGroupByEmrAnalysis");
}

void CEMRAnalysisDlg::OnDtnDatetimechangeEmnFromDateAnalysis(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		
		//do not clear or refilter the list

		*pResult = 0;

	}NxCatchAll("Error in CEMRAnalysisDlg::OnDtnDatetimechangeEmnFromDateAnalysis");
}

void CEMRAnalysisDlg::OnDtnDatetimechangeEmnToDateAnalysis(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		
		//do not clear or refilter the list

		*pResult = 0;

	}NxCatchAll("Error in CEMRAnalysisDlg::OnDtnDatetimechangeEmnToDateAnalysis");
}

void CEMRAnalysisDlg::OnBnClickedBtnReloadResults()
{
	try {

		LoadResults();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBnClickedBtnReloadResults");
}

//TrySaveFilterChanges will attempt to save our current filters to the selected configuration
//after prompting, if those filters changed. Only returns FALSE if the user cancels or gets an error.
BOOL CEMRAnalysisDlg::TrySaveFilterChanges(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		if(pRow == NULL) {
			//nothing to save to! Silently return TRUE.
			return TRUE;
		}

		long nID = VarLong(pRow->GetValue(cccID));
		CString strName = VarString(pRow->GetValue(cccName));

		//see if our on-screen filters match our stored filters,
		//and if not, prompt to save the changes, then do so, and
		//update the combo to reflect the new changes

		//get the filters from the screen
		BOOL bUseDateFilter = m_radioEMNDate.GetCheck();
		COleDateTime dtFrom, dtTo;

		if(bUseDateFilter) {
			dtFrom = VarDateTime(m_dtFrom.GetValue());
			dtFrom.SetDateTime(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay(), 0, 0, 0);
			dtTo = VarDateTime(m_dtTo.GetValue());
			dtTo.SetDateTime(dtTo.GetYear(), dtTo.GetMonth(), dtTo.GetDay(), 0, 0, 0);
		}

		EMRAnalysisGroupByType eGroupBy = eagbtPatient;
		if(m_radioGroupByDate.GetCheck()) {
			eGroupBy = eagbtDate;
		}
		else if(m_radioGroupByEMN.GetCheck()) {
			eGroupBy = eagbtEMN;
		}
		else if(m_radioGroupByEMR.GetCheck()) {
			eGroupBy = eagbtEMR;
		}

		// (c.haag 2009-02-24 12:04) - PLID 33187 - ColumnGroupBy
		EMRAnalysisColumnGroupByType eColumnGroupBy = eacgbtCondensed;
		if (m_radioColumnGroupByItem.GetCheck()) {
			eColumnGroupBy = eacgbtOnePerItem;
		}
		else if (m_radioColumnGroupByCondensed.GetCheck()) {
			eColumnGroupBy = eacgbtCondensed;
		}

		//get the filters from our saved configuration
		BOOL bExistingUseDateFilter = VarBool(pRow->GetValue(cccUseDateFilter), FALSE);
		COleDateTime dtExistingFrom, dtExistingTo;
		if(bExistingUseDateFilter) {
			dtExistingFrom = VarDateTime(pRow->GetValue(cccDateFrom));
			dtExistingFrom.SetDateTime(dtExistingFrom.GetYear(), dtExistingFrom.GetMonth(), dtExistingFrom.GetDay(), 0, 0, 0);
			dtExistingTo = VarDateTime(pRow->GetValue(cccDateTo));
			dtExistingTo.SetDateTime(dtExistingTo.GetYear(), dtExistingTo.GetMonth(), dtExistingTo.GetDay(), 0, 0, 0);
		}
		EMRAnalysisGroupByType eExistingGroupBy = (EMRAnalysisGroupByType)VarLong(pRow->GetValue(cccDisplayGroupBy), (long)eagbtPatient);
		// (c.haag 2009-02-24 16:22) - PLID 33187 - Added eExistingColumnGroupBy
		EMRAnalysisColumnGroupByType eExistingColumnGroupBy = (EMRAnalysisColumnGroupByType)VarLong(pRow->GetValue(cccColumnGroupBy), (long)eacgbtCondensed);

		// (c.haag 2009-02-24 16:22) - PLID 33187 - Check the column group by as well 
		if(bUseDateFilter != bExistingUseDateFilter
			|| (bUseDateFilter && bUseDateFilter == bExistingUseDateFilter && dtFrom != dtExistingFrom)
			|| (bUseDateFilter && bUseDateFilter == bExistingUseDateFilter && dtTo != dtExistingTo)
			|| eGroupBy != eExistingGroupBy
			|| eColumnGroupBy != eExistingColumnGroupBy) {

			//something changed, so ask the user if they want to save the filter changes?

			//use MB_YESNOCANCEL and return FALSE only if IDCANCEL is chosen,
			//return TRUE if they selected yes or no
			CString strWarn;
			strWarn.Format("You have made changes to the filter for the '%s' configuration.\n"
				"Would you like to save these changes?", strName);
			int nRet = MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNOCANCEL);
			if(nRet == IDCANCEL) {
				//tell the caller to not continue
				return FALSE;
			}
			else if(nRet == IDNO) {
				//they don't wish to save, allow the caller to continue on
				return TRUE;
			}
		}
		else {
			//nothing needs to change, allow the caller to continue on
			return TRUE;
		}

		//if we get here, something changed, and the user does want to save the changes

		CString strDateFrom = "NULL";
		CString strDateTo = "NULL";

		//refuse to save changes if the from & to date aren't valid		
		if(bUseDateFilter) {
			if(dtFrom > dtTo) {
				if(IDNO == MessageBox("Your From date is greater than your To date. These changes will not be saved.\n"
					"Are you sure you wish to continue without saving this filter?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
					//tell the caller to not continue
					return FALSE;
				}
				else {
					//allow the caller to continue on
					return TRUE;
				}
			}

			strDateFrom.Format("'%s'", FormatDateTimeForSql(dtFrom, dtoDate));
			strDateTo.Format("'%s'", FormatDateTimeForSql(dtTo, dtoDate));
		}

		//save to data
		// (c.haag 2009-02-24 11:36) - PLID 33187 - Added ColumnGroupBy
		ExecuteSql("UPDATE EMRAnalysisConfigT "
				"SET UseDateFilter = %li, DateFrom = %s, DateTo = %s, DisplayGroupBy = %li, "
				"ColumnGroupBy = %li "
				"WHERE ID = %li",
				bUseDateFilter ? 1 : 0, strDateFrom, strDateTo, (long)eGroupBy, (long)eColumnGroupBy, nID);

		//update the row
		if(bUseDateFilter) {
			pRow->PutValue(cccUseDateFilter, _variant_t(VARIANT_TRUE, VT_BOOL));
			pRow->PutValue(cccDateFrom, _variant_t(dtFrom, VT_DATE));
			pRow->PutValue(cccDateTo, _variant_t(dtTo, VT_DATE));
		}
		else {
			pRow->PutValue(cccUseDateFilter, _variant_t(VARIANT_FALSE, VT_BOOL));
			pRow->PutValue(cccDateFrom, g_cvarNull);
			pRow->PutValue(cccDateTo, g_cvarNull);
		}
		pRow->PutValue(cccDisplayGroupBy, (long)eGroupBy);
		// (c.haag 2009-02-25 10:21) - PLID 33187 - eColumnGroupBy
		pRow->PutValue(cccColumnGroupBy, (long)eColumnGroupBy);

		//now leave successfully
		return TRUE;

	}NxCatchAll("Error in CEMRAnalysisDlg::TrySaveFilterChanges");

	return FALSE;
}
void CEMRAnalysisDlg::OnSelChangingEmrAnalysisConfigCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		//selecting nothing is meaningless, so disable that ability
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		//ideally we'd reference TrySaveFilterChanges in this function, but if
		//a messagebox is shown, OnSelChosen will never be hit

	}NxCatchAll("Error in CEMRAnalysisDlg::OnSelChangingEmrAnalysisConfigCombo");
}

void CEMRAnalysisDlg::ClearResultsList()
{
	try {

		m_nxeditProgressStatus.SetWindowText("");
		m_progress.SetPos(0);

		//clear all rows
		m_ResultsList->Clear();

		//remove all dynamic columns, which is everything to the right of rlcPicID
		int i = 0;
		for(i = m_ResultsList->GetColumnCount() - 1; i >= rlcFirstDynamicColumn; i--) {
			m_ResultsList->RemoveColumn(i);
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::ClearResultsList");
}

void CEMRAnalysisDlg::UpdateColumnWidths()
{
	try {

		//forcibly resize the data, including by the header text
		for(int i = 0; i < m_ResultsList->GetColumnCount(); i++) {

			//do not resize the hidden ID columns
			// (a.walling 2010-01-11 16:43) - PLID 31482 - Skip preview column as well
			if(i == rlcPatientID || i == rlcEMNID || i == rlcEMRID || i == rlcPicID || i == rlcPreview) {
				continue;
			}

			// (j.jones 2009-04-09 09:12) - PLID 33915 - skip hidden columns
			if(!m_checkShowProviderColumn.GetCheck() && i == rlcProviderName) {
				continue;
			}
			if(!m_checkShowSecondaryProviderColumn.GetCheck() && i == rlcSecondaryProviderName) {
				continue;
			}
			if(!m_checkShowEMNDescColumn.GetCheck() && i == rlcEMNDescription) {
				continue;
			}
			if(!m_checkShowEMRDescColumn.GetCheck() && i == rlcEMRDescription) {
				continue;
			}

			IColumnSettingsPtr pCol = m_ResultsList->GetColumn(i);
			//tell the column to auto-resize itself, counting the header text
			//(this will not work if the column has no data, so we'll occasionally
			//be stuck with headers that are cut off - no way around it)
			long nNewWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_TRUE);
			if(nNewWidth > 250) {
				//if the width is over 250, cut it off as a maximum
				pCol->PutColumnStyle(csVisible);
				pCol->PutStoredWidth(250);
			}
			else {
				pCol->PutStoredWidth(nNewWidth);
			}
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::UpdateColumnWidths");
}

void CEMRAnalysisDlg::ClearCachedConfigDetails(CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails)
{
	try {

		for(int i = aryCachedConfigDetails.GetSize() - 1; i >= 0; i--) {
			CachedConfigDetail *pConfigDetail = (CachedConfigDetail*)aryCachedConfigDetails.GetAt(i);
			delete (CachedConfigDetail*)pConfigDetail;
			aryCachedConfigDetails.RemoveAt(i);
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::ClearCachedConfigDetails");
}

void CEMRAnalysisDlg::ClearMatchingEMRDetails(CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails)
{
	try {

		for(int i = aryMatchingEMRDetails.GetSize() - 1; i >= 0; i--) {
			MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(i);
			delete (MatchingEMRDetails*)pMatchingDetail;
			aryMatchingEMRDetails.RemoveAt(i);
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::ClearMatchingEMRDetails");
}

void CEMRAnalysisDlg::LoadResults()
{
	try {

		IRowSettingsPtr pRow = m_ConfigCombo->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must first select a configuration from the list before displaying results.");
			return;
		}
		
		// (a.walling 2010-06-02 13:31) - PLID 39007 - Set to 'Reload Results'
		m_btnReloadResults.SetWindowText("Reload Results");

		m_bIsLoading = TRUE;
		ToggleInterface();

		CWaitCursor pWait;

		ClearResultsList();

		long nID = VarLong(pRow->GetValue(cccID));

		//get the filters from the screen
		BOOL bUseDateFilter = m_radioEMNDate.GetCheck();
		COleDateTime dtFrom, dtTo;

		if(bUseDateFilter) {
			dtFrom = VarDateTime(m_dtFrom.GetValue());
			dtFrom.SetDateTime(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay(), 0, 0, 0);
			dtTo = VarDateTime(m_dtTo.GetValue());
			dtTo.SetDateTime(dtTo.GetYear(), dtTo.GetMonth(), dtTo.GetDay(), 0, 0, 0);
		}

		//we will update the column names based on our grouping
		IColumnSettingsPtr pDateCol = m_ResultsList->GetColumn(rlcEMNDate);
		IColumnSettingsPtr pEMNDescCol = m_ResultsList->GetColumn(rlcEMNDescription);
		IColumnSettingsPtr pEMRDescCol = m_ResultsList->GetColumn(rlcEMRDescription);

		EMRAnalysisGroupByType eGroupBy = eagbtPatient;
		if(m_radioGroupByPatient.GetCheck()) {
			eGroupBy = eagbtPatient;
			pDateCol->PutColumnTitle("First EMN Date");
			pEMNDescCol->PutColumnTitle("First EMN Description");
			pEMRDescCol->PutColumnTitle("First EMR Description");
		}
		else if(m_radioGroupByDate.GetCheck()) {
			eGroupBy = eagbtDate;
			pDateCol->PutColumnTitle("EMN Date");
			pEMNDescCol->PutColumnTitle("First EMN Description");
			pEMRDescCol->PutColumnTitle("First EMR Description");
		}
		else if(m_radioGroupByEMN.GetCheck()) {
			eGroupBy = eagbtEMN;
			pDateCol->PutColumnTitle("EMN Date");
			pEMNDescCol->PutColumnTitle("EMN Description");
			pEMRDescCol->PutColumnTitle("EMR Description");
		}
		else if(m_radioGroupByEMR.GetCheck()) {
			eGroupBy = eagbtEMR;
			pDateCol->PutColumnTitle("First EMN Date");
			pEMNDescCol->PutColumnTitle("First EMN Description");
			pEMRDescCol->PutColumnTitle("EMR Description");
		}		

		//do not proceed if the from & to date aren't valid		
		if(bUseDateFilter) {
			if(dtFrom > dtTo) {
				AfxMessageBox("Your From date is greater than your To date. You must correct this before displaying results.");

				//re-enable the interface
				m_bIsLoading = FALSE;
				ToggleInterface();

				m_nxeditProgressStatus.SetWindowText("");
				m_progress.SetPos(0);
				return;
			}
		}

		//these two fields are not on screen, but they are in the combo
		BOOL bFilterOnAllItems = VarBool(pRow->GetValue(cccFilterOnAllItems), TRUE);
		EMRAnalysisRecordFilterType eRecordFilter = (EMRAnalysisRecordFilterType)VarLong(pRow->GetValue(cccRecordFilter), (long)earftPatient);

		// (j.jones 2009-03-27 11:02) - PLID 33703 - added FilterByTemplateID
		long nFilterByTemplateID = VarLong(pRow->GetValue(cccFilterByTemplateID), -1);

		// (j.jones 2009-04-09 12:53) - PLID 33916 - added IncludeSpawningInfo
		BOOL bIncludeSpawningInfo = VarBool(pRow->GetValue(cccIncludeSpawningInfo), FALSE);

		//check to see if the user cancelled
		PeekAndPump();
		if(!m_bIsLoading) {
			ResetInterfaceOnCancel();			
			return;
		}

		m_nxeditProgressStatus.SetWindowText("Loading filters...");
		PeekAndPump();

		//load up the EMRAnalysisConfigDetailsT information for this configuration
		CArray<CachedConfigDetail*, CachedConfigDetail*> aryCachedConfigDetails;
		LoadConfigDetailInfo(nID, aryCachedConfigDetails);

		//check to see if the user cancelled
		PeekAndPump();
		if(!m_bIsLoading) {
			ResetInterfaceOnCancel();

			ClearCachedConfigDetails(aryCachedConfigDetails);	
			return;
		}

		if(aryCachedConfigDetails.GetSize() == 0) {
			//our configuration interface should have never allowed this to happen,
			//but perhaps other deletions in the program caused this to occur, so
			//handle this gracefully
			AfxMessageBox("The selected configuration has no EMR Items in which to filter on.\n"
				"Please edit this configuration and add some EMR Items before attempting to use it.");

			//re-enable the interface
			m_bIsLoading = FALSE;
			ToggleInterface();

			m_nxeditProgressStatus.SetWindowText("");
			m_progress.SetPos(0);

			ClearCachedConfigDetails(aryCachedConfigDetails);
			return;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		//now, reload the list

		//rebuild our columns

		//we have some fixed, permanent columns, but the rest are dynamic
		// (c.haag 2009-02-24 16:34) - PLID 33187 - Get the column grouping style. By default, we conform to the
		// checkbox...but if we use the eacgbtOnePerItem with too many items, then we will have to use the condensed
		// version.
		EMRAnalysisColumnGroupByType eColumnGroupBy;
		BOOL bForcedCondensedMode = FALSE;
		if (m_radioColumnGroupByItem.GetCheck()) {
			eColumnGroupBy = eacgbtOnePerItem;
		} else {
			ASSERT(m_radioColumnGroupByCondensed.GetCheck());
			eColumnGroupBy = eacgbtCondensed;
		}

		if (eacgbtOnePerItem == eColumnGroupBy) {
			// (c.haag 2009-02-24 12:48) - PLID 33187 - If we get here, it means one column is allocated per detail. This
			// is the legacy way of presenting the data. Check to see if we have enough room for all the items. If not,
			// change the type to condensed view.
			if (aryCachedConfigDetails.GetSize() + m_ResultsList->GetColumnCount() > MAX_RESULT_COLUMNS) {
				// Too many columns. Change the style to condensed
				eColumnGroupBy = eacgbtCondensed;
				bForcedCondensedMode = TRUE;
			} else {
				short nStartingColIndex = rlcFirstDynamicColumn;
				int i = 0;
				for(i = 0; i<aryCachedConfigDetails.GetSize(); i++) {

					//get the cached info
					CachedConfigDetail *pConfigDetail = (CachedConfigDetail*)aryCachedConfigDetails.GetAt(i);

					//add the column
					CString strColumnName = pConfigDetail->strEMRInfoName;
					short nNewCol = m_ResultsList->InsertColumn(nStartingColIndex + i, "", (LPCTSTR)strColumnName, 100, csVisible|csWidthData);
					ASSERT(nNewCol == nStartingColIndex + i);
					IColumnSettingsPtr pColumn = m_ResultsList->GetColumn(nNewCol);
					pColumn->FieldType = cftTextWordWrap;
				}
			}
		} else {
			// (c.haag 2009-02-24 12:48) - PLID 33187 - If we get here, this is the new way of presenting data where we
			// only add columns when absolutely necessary. We need to load the EMR data before we can do this.
			ASSERT(eacgbtCondensed == eColumnGroupBy);
		}

		m_nxeditProgressStatus.SetWindowText("Loading EMR Data...");
		PeekAndPump();

		//load up all EMR details that match our filters - this first pass won't fully
		//cover narratives, tables, or the bFilterOnAllItems setting, so we will need
		//multiple passes through this resultset later
		CArray<MatchingEMRDetails*, MatchingEMRDetails*> aryMatchingEMRDetails;
		// (j.jones 2009-03-27 11:05) - PLID 33703 - added nFilterByTemplateID
		LoadAllCandidateDetails(bUseDateFilter, dtFrom, dtTo, nFilterByTemplateID,
			aryCachedConfigDetails, aryMatchingEMRDetails);

		//check to see if the user cancelled
		PeekAndPump();
		if(!m_bIsLoading) {
			ResetInterfaceOnCancel();

			ClearCachedConfigDetails(aryCachedConfigDetails);
			ClearMatchingEMRDetails(aryMatchingEMRDetails);
			return;
		}

		if(aryMatchingEMRDetails.GetSize() == 0) {
			//we have no results! Update our column widths, and return.
			UpdateColumnWidths();

			ClearCachedConfigDetails(aryCachedConfigDetails);

			//re-enable the interface
			m_bIsLoading = FALSE;
			ToggleInterface();

			ClearResultsList();

			m_nxeditProgressStatus.SetWindowText("No Results Found.");
			m_progress.SetPos(0);

			ClearCachedConfigDetails(aryCachedConfigDetails);
			ClearMatchingEMRDetails(aryMatchingEMRDetails);
			return;
		}

		m_nxeditProgressStatus.SetWindowText("Filtering EMR Data...");
		PeekAndPump();

		// (b.savon 2011-11-01 16:07) - PLID 43205
		CArray<DynColumns*, DynColumns*> arydcTableColumns;

		// (b.savon 2011-11-01 16:07) - PLID 43205 - Added arydcTableColumns
		//If we do have results in aryMatchingEMRDetails, they aren't the complete results.
		//We don't know what the "sentence format" is for each detail unless we load it in an EMN,
		//plus we do not know if a narrative or table has data until this is done. So do it now.
		// (j.jones 2009-04-09 14:45) - PLID 33916 - passed in bIncludeSpawningInfo
		FillCandidateDetailsWithEMNData(aryMatchingEMRDetails, bIncludeSpawningInfo, arydcTableColumns);

		//check to see if the user cancelled
		PeekAndPump();
		if(!m_bIsLoading) {
			ResetInterfaceOnCancel();

			ClearCachedConfigDetails(aryCachedConfigDetails);
			ClearMatchingEMRDetails(aryMatchingEMRDetails);
			return;
		}

		if (eacgbtCondensed == eColumnGroupBy) {
			// (c.haag 2009-02-24 16:03) - PLID 33187 - If we're trying to minimize the amount of
			// columns that the result list will need, now is the time we determine how many columns
			// we need, and add them.
			m_nxeditProgressStatus.SetWindowText("Building column list...");
			PeekAndPump();

			CMap<long,long,int,int> mapLongCounter;
			CMap<COleDateTime,COleDateTime&,int,int> mapDateCounter;
			int i;
			int nNeededColumns = 0;
			switch (eGroupBy) {
				case eagbtPatient:
				case eagbtEMN:
				case eagbtEMR:
					for(i = 0; i<aryMatchingEMRDetails.GetSize(); i++) {
						MatchingEMRDetails* pDetail = aryMatchingEMRDetails[i];
						int nCount = 0;
						long nID;
						if (eagbtPatient == eGroupBy) { nID = pDetail->nPatientID; }
						else if (eagbtEMN == eGroupBy) { nID = pDetail->nEMNID; }
						else { nID = pDetail->nEMRID; }
						// If this succeeds, nCount will be a positive number. If not, it will just be zero.
						mapLongCounter.Lookup(nID, nCount);
						mapLongCounter.SetAt(nID, ++nCount);
						nNeededColumns = max(nNeededColumns, nCount);
					}
					break;
				case eagbtDate:
					for(i = 0; i<aryMatchingEMRDetails.GetSize(); i++) {
						MatchingEMRDetails* pDetail = aryMatchingEMRDetails[i];
						COleDateTime dt = pDetail->dtEMNDate;
						int nCount = 0;
						// If this succeeds, nCount will be a positive number. If not, it will just be zero.
						mapDateCounter.Lookup(dt, nCount);
						mapDateCounter.SetAt(dt, ++nCount);
						nNeededColumns = max(nNeededColumns, nCount);
					}
					break;
				default:
					ASSERT(FALSE);
					break;
			}
			// Truncate the number of available columns
			nNeededColumns = min(MAX_RESULT_COLUMNS - m_ResultsList->GetColumnCount(), nNeededColumns);
			// Now insert the columns
			for (i=0; i < nNeededColumns; i++) {
				int nNewCol = m_ResultsList->GetColumnCount();
				m_ResultsList->InsertColumn(nNewCol, "", (LPCTSTR)FormatString("Result %d", i+1), 100, csVisible|csWidthData);
				IColumnSettingsPtr pColumn = m_ResultsList->GetColumn(nNewCol);
				pColumn->FieldType = cftTextWordWrap;
			}

			//check to see if the user cancelled
			PeekAndPump();
			if(!m_bIsLoading) {
				ResetInterfaceOnCancel();

				ClearCachedConfigDetails(aryCachedConfigDetails);
				ClearMatchingEMRDetails(aryMatchingEMRDetails);
				return;
			}
		} // if (eacgbtCondensed == eColumnGroupBy) {
		else {
			ASSERT(eacgbtOnePerItem == eColumnGroupBy);
		}

		//when we get here, we now have in memory everything we need to finish filtering the data

		m_nxeditProgressStatus.SetWindowText("Refining EMR Data...");
		PeekAndPump();

		//We need to make two remaining passes, one will check the "Has Data" setting for any
		//details that request it, and one will verify whether a patient has all the details,
		//if specified by the configuration. This function will do both.
		FinalFilterCandidateDetails(bFilterOnAllItems, eRecordFilter, aryCachedConfigDetails, aryMatchingEMRDetails);

		//check to see if the user cancelled
		PeekAndPump();
		if(!m_bIsLoading) {
			ResetInterfaceOnCancel();

			ClearCachedConfigDetails(aryCachedConfigDetails);
			ClearMatchingEMRDetails(aryMatchingEMRDetails);
			return;
		}

		//now we are done - aryMatchingEMRDetails now contains everything we need to display

		//if it is empty, return now
		if(aryMatchingEMRDetails.GetSize() == 0) {
			//we have no remaining results! Update our column widths, and return.
			UpdateColumnWidths();

			ClearCachedConfigDetails(aryCachedConfigDetails);

			//re-enable the interface
			m_bIsLoading = FALSE;
			ToggleInterface();

			ClearResultsList();

			m_nxeditProgressStatus.SetWindowText("No Results Found.");
			m_progress.SetPos(0);

			ClearCachedConfigDetails(aryCachedConfigDetails);
			ClearMatchingEMRDetails(aryMatchingEMRDetails);
			return;
		}

		//check to see if the user cancelled
		PeekAndPump();
		if(!m_bIsLoading) {
			ResetInterfaceOnCancel();

			ClearCachedConfigDetails(aryCachedConfigDetails);
			ClearMatchingEMRDetails(aryMatchingEMRDetails);
			return;
		}

		//display all the data we gathered during our above processing

		m_nxeditProgressStatus.SetWindowText("Displaying Results...");
		PeekAndPump();

		// (b.savon 2011-11-01 08:56) - PLID 43205 - The EMR Analysis should be able to export individual 
		// table rows as their own cells
		CMap<CString, LPCTSTR, short, short> mapHeaders;
		short nDatalistCol;

		for( int nDynTableColumnIndex = 0; nDynTableColumnIndex < arydcTableColumns.GetSize(); nDynTableColumnIndex++ ){		
			//	Insert and Save the column number if it doesn't already exist and we haven't reached
			//	the maximum number of columns
			if( !mapHeaders.Lookup(arydcTableColumns.GetAt(nDynTableColumnIndex)->strHeader, nDatalistCol) && 
				!(m_ResultsList->GetColumnCount() >= MAX_RESULT_COLUMNS) ){

				// Insert into the map
				mapHeaders.SetAt(arydcTableColumns.GetAt(nDynTableColumnIndex)->strHeader, m_ResultsList->GetColumnCount());
				// Save the column number
				arydcTableColumns.GetAt(nDynTableColumnIndex)->nColumn = m_ResultsList->GetColumnCount();

				//	Create the column
				int nNewCol = m_ResultsList->InsertColumn(m_ResultsList->GetColumnCount(), "", (LPCSTR)arydcTableColumns.GetAt(nDynTableColumnIndex)->strHeader, 100, csVisible|csWidthData);
				IColumnSettingsPtr pColumn = m_ResultsList->GetColumn(nNewCol);
				pColumn->FieldType = cftTextWordWrap;
				pColumn->DataType = VT_BSTR;

			} else{	//Already exists, just save.
				arydcTableColumns.GetAt(nDynTableColumnIndex)->nColumn = nDatalistCol;
			}//END add column
		}//END for all matching EMR details

		AddDetailsToResultList(eGroupBy, eColumnGroupBy, aryCachedConfigDetails, aryMatchingEMRDetails);

		// (b.savon 2011-11-01 16:41) - PLID 43205 - Export Individual rows/cols as their own cells
		//	Go through each row in the list and dump the table values in the correct rows 
		pRow = m_ResultsList->GetFirstRow();
		while(pRow && arydcTableColumns.GetSize() > 0){
			//	Get row identifying info.
			long nPatientID = VarLong(pRow->GetValue(rlcPatientID));
			long nEMNID = VarLong(pRow->GetValue(rlcEMNID));
			COleDateTime dtEMNDate = VarDateTime(pRow->GetValue(rlcEMNDate));
			long nEMRID = VarLong(pRow->GetValue(rlcEMRID));

			//	Go through our data and insert the correct values, removing them as we use them so
			//	our search time depreciates as we move ourselves down the list.
			for( int nData = 0; nData < arydcTableColumns.GetSize(); nData++ ){
				if( arydcTableColumns.GetAt(nData)->nPatientID == nPatientID &&
					arydcTableColumns.GetAt(nData)->nEMNID == nEMNID &&
					arydcTableColumns.GetAt(nData)->nEMRID == nEMRID &&
					arydcTableColumns.GetAt(nData)->dtEMNDate == dtEMNDate ) {

						pRow->PutValue(arydcTableColumns.GetAt(nData)->nColumn, _bstr_t(arydcTableColumns.GetAt(nData)->strData));
						delete arydcTableColumns.GetAt(nData);
						arydcTableColumns.RemoveAt(nData);
						--nData;
				}//END if match
			}//END for all single row/col data

			// Next, please
			pRow = pRow->GetNextRow();
		}//END for all rows in the list

		//resize the columns
		UpdateColumnWidths();

		//clear our memory
		ClearCachedConfigDetails(aryCachedConfigDetails);
		ClearMatchingEMRDetails(aryMatchingEMRDetails);

		//re-enable the interface
		m_bIsLoading = FALSE;
		ToggleInterface();

		m_nxeditProgressStatus.SetWindowText("Analysis Complete.");
		m_progress.SetPos(0);

		// (c.haag 2009-02-24 17:06) - PLID 33187 - If we had to force the list into condensed mode, we must
		// tell the user that we did; and why. Do this at the end in case the user walks away and gets a cup
		// of coffee while the analysis is running.
		if (bForcedCondensedMode) {
			CString str = "The analysis has successfully completed. However, there are too many items to display in the list under individual columns. "
				"As a consequence, the list was generated in 'Use As Few Columns As Possible' mode.\n\n"
				"This means that items are displayed without their names (except for those with no data), and are arranged in a condensed left-to-right ordering "
				"rather than under their respectively named columns.";
			AfxMessageBox(str, MB_ICONINFORMATION);
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::LoadResults");
}

//this function will load the information from EMRAnalysisConfigDetailsT into aryCachedConfigDetails
void CEMRAnalysisDlg::LoadConfigDetailInfo(long nEMRAnalysisConfigID, CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails)
{
	try {

		//cache the detail information from data (do not exclude inactive items)
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		// (a.wilson 2013-05-14 15:12) - PLID 55963 - add emrfiltertextdata for contains operator.
		_RecordsetPtr rsConfigDetails = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EMRInfoT.ID AS EMRInfoID, EMRInfoT.Name AS EMRInfoName, "
			"EMRInfoT.DataType, EMRAnalysisConfigDetailsT.EMRInfoMasterID, "
			"EMRAnalysisConfigDetailsT.DataOperator, EMRAnalysisConfigDetailsT.TextFilterData, "
			"EMRAnalysisConfigDetailsT.EMRDataGroupID, EMRDataT.ID AS EMRDataID, EMRDataT.Data AS EMRDataName, EMRDataT.ListType "
			"FROM EMRAnalysisConfigDetailsT "
			"INNER JOIN EMRInfoMasterT ON EMRAnalysisConfigDetailsT.EMRInfoMasterID = EMRInfoMasterT.ID "
			"INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "
			"LEFT JOIN (SELECT * FROM EMRDataT WHERE IsLabel = 0) AS EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID AND EMRDataT.EMRDataGroupID = EMRAnalysisConfigDetailsT.EMRDataGroupID "
			"WHERE EMRAnalysisConfigDetailsT.EMRAnalysisConfigID = {INT} ", nEMRAnalysisConfigID);

		if(rsConfigDetails->eof) {
			//do nothing, the caller will handle it
			return;
		}

		while(!rsConfigDetails->eof) {

			//check to see if the user cancelled
			PeekAndPump();
			if(!m_bIsLoading) {
				ResetInterfaceOnCancel();
				return;
			}

			CachedConfigDetail *pDetail = new CachedConfigDetail;
			pDetail->nActiveEMRInfoID = AdoFldLong(rsConfigDetails, "EMRInfoID");
			pDetail->strEMRInfoName = AdoFldString(rsConfigDetails, "EMRInfoName");
			pDetail->eDataType = (EmrInfoType)AdoFldByte(rsConfigDetails, "DataType");
			pDetail->nEMRInfoMasterID = AdoFldLong(rsConfigDetails, "EMRInfoMasterID");
			pDetail->eOperator = (EMRAnalysisDataOperatorType)AdoFldLong(rsConfigDetails, "DataOperator", (long)eadotExists);
			pDetail->nEMRDataGroupID = AdoFldLong(rsConfigDetails, "EMRDataGroupID", -1);
			pDetail->strTextFilterData = AdoFldString(rsConfigDetails, "TextFilterData", "");
			pDetail->nEMRDataID = AdoFldLong(rsConfigDetails, "EMRDataID", -1);
			pDetail->strEMRDataName = AdoFldString(rsConfigDetails, "EMRDataName", "");
			pDetail->nListType = AdoFldLong(rsConfigDetails, "ListType", -1);
			aryCachedConfigDetails.Add(pDetail);
						
			rsConfigDetails->MoveNext();
		}
		rsConfigDetails->Close();

	}NxCatchAll("Error in CEMRAnalysisDlg::LoadConfigDetailInfo");
}

//this function will load all details from data that potentially match our filter - it will not be able to
//100% match "has data" filters on narratives and tables, nor check the "filter on all items" setting,
//but will load all details that potentially may be in our final resultset
// (j.jones 2009-03-27 11:05) - PLID 33703 - added nFilterByTemplateID
void CEMRAnalysisDlg::LoadAllCandidateDetails(BOOL bUseDateFilter, COleDateTime dtFrom, COleDateTime dtTo, long nFilterByTemplateID,
											  CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails,
											  CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails)
{
	try {

		CString strDetailWhere;
		
		int i = 0;
		for(i = 0; i<aryCachedConfigDetails.GetSize(); i++) {

			//check to see if the user cancelled
			PeekAndPump();
			if(!m_bIsLoading) {
				ResetInterfaceOnCancel();
				return;
			}

			//get the cached info
			CachedConfigDetail *pConfigDetail = (CachedConfigDetail*)aryCachedConfigDetails.GetAt(i);

			//append to our where clause
			if(!strDetailWhere.IsEmpty()) {
				strDetailWhere += " OR ";
			}
			CString str;
			//process our operator, the EMRAnalysisDataOperatorType enum
			// (a.wilson 2013-05-14 15:25) - PLID 55963 - add new operator contains for text details.
			if (pConfigDetail->eOperator == eadotContains) {
				//this functionality only works for text details at this time so check to make sure its a text detail.
				if (pConfigDetail->eDataType == eitText) {
					str.Format("EMRInfoT.EMRInfoMasterID = %li "
						"AND EMRInfoT.DataType = %li "
						"AND EMRDetailsT.Text LIKE '%%%s%%'", 
						pConfigDetail->nEMRInfoMasterID, eitText, FormatForLikeClause(pConfigDetail->strTextFilterData));
				} else {
					//this should not be possible.
					ASSERT(FALSE);
				}
			}
			else if(pConfigDetail->eOperator == eadotExists) {
				str.Format("EMRInfoT.EMRInfoMasterID = %li", pConfigDetail->nEMRInfoMasterID);
			}
			else if(pConfigDetail->eOperator == eadotItemHasData) {
				//check the data by data type
				switch(pConfigDetail->eDataType) {
					case eitText:
						//this assumes text with spaces or newlines is not empty
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li "
							"AND (EMRDetailsT.Text Is Not Null AND EMRDetailsT.Text NOT LIKE '')) ",
							pConfigDetail->nEMRInfoMasterID,
							eitText);
						break;
					case eitSingleList:
					case eitMultiList:
						//check to see if there are any entries in EMRSelectT
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType IN (%li, %li) "
							"AND EMRDetailsT.ID IN (SELECT EMRDetailID FROM EMRSelectT)) ",
							pConfigDetail->nEMRInfoMasterID,
							eitSingleList, eitMultiList);
						break;
					// (j.jones 2010-04-27 10:27) - PLID 37693 - supported images
					case eitImage:
						//check for ink or text
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li "
							"AND (EMRDetailsT.InkData Is Not Null OR EMRDetailsT.ImageTextData Is Not Null)) ",
							pConfigDetail->nEMRInfoMasterID,
							eitImage);
						break;
					case eitSlider:
						//here, a 0 is data, only NULL is no data
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li "
							"AND EMRDetailsT.SliderValue Is Not Null) ",
							pConfigDetail->nEMRInfoMasterID,
							eitSlider);
						break;
					case eitNarrative:
						//this will have to be loaded and checked later, so just check for existence for now
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li) ",
							pConfigDetail->nEMRInfoMasterID,
							eitNarrative);
						break;
					case eitTable:
						//we have to check differently per column type
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li "
							"AND EMRDetailsT.ID IN (SELECT EMRDetailID FROM EMRDetailTableDataT "
							"	INNER JOIN EMRDataT ON EMRDataID_Y = EMRDataT.ID "
							//"	WHERE "
							// (z.manning 2008-12-10 16:00) - PLID 32389 - We now check for non-blank data in all column types
							// (a.walling 2009-04-03 10:44) - PLID 33831 - blank data is no longer saved to the database
							/*
							"	Len(EMRDetailTableDataT.Data) > 0 \r\n"
							"	AND ( \r\n"
							"		(ListType = 4 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // dropdown
							"		OR (ListType = 5 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // checkbox
							"	) \r\n"	
							*/
							"	))",
							pConfigDetail->nEMRInfoMasterID,
							eitTable);
						break;
					default:
						//invalid type
						ASSERT(FALSE);
				}
			}
			else if(pConfigDetail->eOperator == eadotItemHasNoData) {
				//check the data by data type
				switch(pConfigDetail->eDataType) {
					case eitText:
						//this assumes text with spaces or newlines is not empty
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li "
							"AND (EMRDetailsT.Text Is Null OR EMRDetailsT.Text LIKE '')) ",
							pConfigDetail->nEMRInfoMasterID,
							eitText);
						break;
					case eitSingleList:
					case eitMultiList:
						//check to see if there are any entries in EMRSelectT
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType IN (%li, %li) "
							"AND EMRDetailsT.ID NOT IN (SELECT EMRDetailID FROM EMRSelectT)) ",
							pConfigDetail->nEMRInfoMasterID,
							eitSingleList, eitMultiList);
						break;
					// (j.jones 2010-04-27 10:27) - PLID 37693 - supported images
					case eitImage:
						//check for ink or text
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li "
							"AND EMRDetailsT.InkData Is Null AND EMRDetailsT.ImageTextData Is Null) ",
							pConfigDetail->nEMRInfoMasterID,
							eitImage);
						break;
					case eitSlider:
						//here, a 0 is data, only NULL is no data
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li "
							"AND EMRDetailsT.SliderValue Is Null) ",
							pConfigDetail->nEMRInfoMasterID,
							eitSlider);
						break;
					case eitNarrative:
						//this will have to be loaded and checked later, so just check for existence for now
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li) ",
							pConfigDetail->nEMRInfoMasterID,
							eitNarrative);
						break;
					case eitTable:
						//we have to check differently per column type
						str.Format("(EMRInfoT.EMRInfoMasterID = %li "
							"AND EMRInfoT.DataType = %li "
							"AND EMRDetailsT.ID NOT IN (SELECT EMRDetailID FROM EMRDetailTableDataT "
							"	INNER JOIN EMRDataT ON EMRDataID_Y = EMRDataT.ID "
							//"	WHERE "
							// (z.manning 2008-12-10 16:00) - PLID 32389 - We now check for non-blank data in all column types
							// (a.walling 2009-04-03 10:44) - PLID 33831 - blank data is no longer saved to the database
							/*
							"	Len(EMRDetailTableDataT.Data) > 0 \r\n"
							"	AND ( \r\n"
							"		(ListType = 4 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // dropdown
							"		OR (ListType = 5 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // checkbox
							"	) \r\n"	
							*/
							"	))",
							pConfigDetail->nEMRInfoMasterID,
							eitTable);
						break;
					default:
						//invalid type
						ASSERT(FALSE);
				}
			}
			else if(pConfigDetail->eOperator == eadotHasDataIn) {

				if(pConfigDetail->eDataType == eitTable) {

					//verify our data is accurate
					ASSERT(pConfigDetail->nEMRDataGroupID != -1);

					//check that this table has data in the given row/column
					str.Format("(EMRInfoT.EMRInfoMasterID = %li "
						"AND EMRInfoT.DataType = %li "
						"AND EMRDetailsT.ID IN (SELECT EMRDetailID FROM EMRDetailTableDataT "
						"	INNER JOIN EMRDataT ON EMRDataID_Y = EMRDataT.ID "
						"	WHERE (EMRDataID_X IN (SELECT ID FROM EMRDataT WHERE EMRDataGroupID = %li) OR EMRDataID_Y IN (SELECT ID FROM EMRDataT WHERE EMRDataGroupID = %li)) "
						// (z.manning 2008-12-10 16:00) - PLID 32389 - We now check for non-blank data in all column types
						// (a.walling 2009-04-03 10:44) - PLID 33831 - blank data is no longer saved to the database
						/*
						"	AND Len(EMRDetailTableDataT.Data) > 0 \r\n"
						"	AND ( \r\n"
						"		(ListType = 4 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // dropdown
						"		OR (ListType = 5 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // checkbox
						"	) \r\n"	
						*/
						"	))",
						pConfigDetail->nEMRInfoMasterID,
						eitTable,
						pConfigDetail->nEMRDataGroupID, pConfigDetail->nEMRDataGroupID);
				}
				else if(pConfigDetail->eDataType == eitSingleList || pConfigDetail->eDataType == eitMultiList) {
					//verify our data is accurate
					ASSERT(pConfigDetail->nEMRDataGroupID != -1);

					str.Format("(EMRInfoT.EMRInfoMasterID = %li "
						"AND EMRInfoT.DataType IN (%li, %li) "
						"AND EMRDetailsT.ID IN (SELECT EMRDetailID FROM EMRSelectT WHERE EMRDataID IN (SELECT ID FROM EMRDataT WHERE EMRDataGroupID = %li))) ",
						pConfigDetail->nEMRInfoMasterID,
						eitSingleList, eitMultiList,
						pConfigDetail->nEMRDataGroupID);
				}
				else {
					//this type shouldn't have data items
					//this *may* be possible if a data reference was removed
					//but the filter setting wasn't changed
					ASSERT(FALSE);
				}
			}
			else if(pConfigDetail->eOperator == eadotHasNoDataIn) {
				if(pConfigDetail->eDataType == eitTable) {

					//verify our data is accurate
					ASSERT(pConfigDetail->nEMRDataGroupID != -1);

					//check that this table does not data in the given row/column
					str.Format("(EMRInfoT.EMRInfoMasterID = %li "
						"AND EMRInfoT.DataType = %li "
						"AND EMRDetailsT.ID NOT IN (SELECT EMRDetailID FROM EMRDetailTableDataT "
						"	INNER JOIN EMRDataT ON EMRDataID_Y = EMRDataT.ID "
						"	WHERE (EMRDataID_X IN (SELECT ID FROM EMRDataT WHERE EMRDataGroupID = %li) OR EMRDataID_Y IN (SELECT ID FROM EMRDataT WHERE EMRDataGroupID = %li)) "
						// (z.manning 2008-12-10 16:00) - PLID 32389 - We now check for non-blank data in all column types
						// (a.walling 2009-04-03 10:44) - PLID 33831 - blank data is no longer saved to the database
						/*
						"	AND Len(EMRDetailTableDataT.Data) > 0 \r\n"
						"	AND ( \r\n"
						"		(ListType = 4 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // dropdown
						"		OR (ListType = 5 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // checkbox
						"	) \r\n"	
						*/
						"	))",
						pConfigDetail->nEMRInfoMasterID,
						eitTable,
						pConfigDetail->nEMRDataGroupID, pConfigDetail->nEMRDataGroupID);
				}
				else if(pConfigDetail->eDataType == eitSingleList || pConfigDetail->eDataType == eitMultiList) {
					//verify our data is accurate
					ASSERT(pConfigDetail->nEMRDataGroupID != -1);

					str.Format("(EMRInfoT.EMRInfoMasterID = %li "
						"AND EMRInfoT.DataType IN (%li, %li) "
						"AND EMRDetailsT.ID NOT IN (SELECT EMRDetailID FROM EMRSelectT WHERE EMRDataID IN (SELECT ID FROM EMRDataT WHERE EMRDataGroupID = %li))) ",
						pConfigDetail->nEMRInfoMasterID,
						eitSingleList, eitMultiList,
						pConfigDetail->nEMRDataGroupID);
				}
				else {
					//this type shouldn't have data items
					//this *may* be possible if a data reference was removed
					//but the filter setting wasn't changed
					ASSERT(FALSE);
				}
			}
			strDetailWhere += str;
		}

		if(strDetailWhere.IsEmpty()) {
			//should be impossible at this point
			ThrowNxException("No EMR Info data found!");
		}

		//build our date filter
		CString strDateWhere = "";
		if(bUseDateFilter) {
			// (z.manning 2009-01-28 10:36) - PLID 32879 - We need to use FormatDateTimeForSql for the dates.
			strDateWhere.Format("AND dbo.AsDateNoTime(EMRMasterT.Date) >= '%s' AND dbo.AsDateNoTime(EMRMasterT.Date) <= '%s' ",
				FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		}

		// (j.jones 2009-03-27 11:06) - PLID 33703 - added support for nFilterByTemplateID
		CString strTemplateWhere = "";
		if(nFilterByTemplateID != -1) {
			// (j.jones 2009-06-04 13:11) - PLID 34486 - filter by the template that the EMN
			// was created from, not the detail (details could have been created through a
			// series of spawns that do not tie back to the main template)
			strTemplateWhere.Format("AND EMRMasterT.TemplateID = %li", nFilterByTemplateID);
		}

		//next, query the data for the actual results, and load the results in the list

		//start with all matching details
		//can't parameterize due to our dynamic where clauses
		// (j.jones 2008-10-20 17:38) - the code in FinalFilterCandidateDetails depends on this
		// ORDER BY clause ordering by patient, EMN, and EMR - do not change the ORDER BY clause
		// without updating that logic appropriately
		// (c.haag 2009-02-24 16:13) - PLID 33187 - We need EmrInfoT.Name, too
		// (j.jones 2009-04-08 14:32) - PLID 33915 - added provider & secondary provider
		// (j.jones 2009-04-09 17:13) - PLID 33916 - added SourceDetailID
		// (z.manning 2011-05-24 09:55) - PLID 33114 - Filter out EMNs that the user can't access due to charting permissions
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsResults = CreateRecordset(GetRemoteDataSnapshot(), "SELECT PatientsT.PersonID, PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"EMRGroupsT.Description AS EMRDescription, EMRMasterT.Description AS EMNDescription, "
			"EMRGroupsT.ID AS EMRID, EMRMasterT.ID AS EMNID, PicT.ID AS PicID, "
			"EMRMasterT.Date AS EMNDate, EMRDetailsT.ID AS EMRDetailID, "
			"EMRInfoT.EMRInfoMasterID, EMRInfoT.DataType, EmrInfoT.Name AS EmrInfoName, "
			"dbo.GetEmnProviderList(EMRMasterT.ID) AS ProviderNames, "
			"dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) AS SecondaryProviderNames, "
			"EMRDetailsT.SourceDetailID "
			"FROM EMRGroupsT "
			"INNER JOIN EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID "
			"INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"INNER JOIN PatientsT ON EMRMasterT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN PicT ON PicT.EMRGroupID = EMRGroupsT.ID "
			"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
			"WHERE EMRGroupsT.Deleted = 0 AND EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 "
			"AND (PicT.IsCommitted = 1 OR PicT.IsCommitted IS NULL) "
			"AND (%s) "
			"%s %s %s"
			"ORDER BY PatientsT.PersonID, EMRMasterT.ID, EMRGroupsT.ID, EMRDetailsT.ID ",
			strDetailWhere, strDateWhere, strTemplateWhere, GetEmrChartPermissionFilter().Flatten());

		if(rsResults->eof) {
			//do nothing, the caller will handle it
			return;
		}

		while(!rsResults->eof) {

			//check to see if the user cancelled
			PeekAndPump();
			if(!m_bIsLoading) {
				ResetInterfaceOnCancel();
				return;
			}

			MatchingEMRDetails *pDetail = new MatchingEMRDetails;
			pDetail->nPatientID = AdoFldLong(rsResults, "PersonID");
			pDetail->nUserDefinedID = AdoFldLong(rsResults, "UserDefinedID");
			pDetail->strPatientName = AdoFldString(rsResults, "PatientName");
			pDetail->strEMRDesc = AdoFldString(rsResults, "EMRDescription");
			pDetail->strEMNDesc = AdoFldString(rsResults, "EMNDescription");
			pDetail->nEMRID = AdoFldLong(rsResults, "EMRID");
			pDetail->nEMNID = AdoFldLong(rsResults, "EMNID");
			pDetail->nPicID = AdoFldLong(rsResults, "PicID", -1);
			pDetail->dtEMNDate = AdoFldDateTime(rsResults, "EMNDate");
			pDetail->nEMRInfoMasterID = AdoFldLong(rsResults, "EMRInfoMasterID");
			pDetail->eDataType = (EmrInfoType)AdoFldByte(rsResults, "DataType");
			pDetail->nEMRDetailID = AdoFldLong(rsResults, "EMRDetailID");
			pDetail->bHasData = FALSE; //this is filled in later
			pDetail->strSentenceFormat = ""; //this is filled in later
			pDetail->strSpawningInformation = ""; // (j.jones 2009-04-09 14:47) - PLID 33916 - filled in later
			// (c.haag 2009-02-24 16:11) - PLID 33187 - Need the detail name
			pDetail->strName = AdoFldString(rsResults, "EmrInfoName");
			// (j.jones 2009-04-08 14:32) - PLID 33915 - added provider & secondary provider
			pDetail->strProviderNames = AdoFldString(rsResults, "ProviderNames", "");
			pDetail->strSecondaryProviderNames = AdoFldString(rsResults, "SecondaryProviderNames", "");
			// (j.jones 2009-04-09 17:13) - PLID 33916 - added SourceDetailID
			pDetail->nSourceDetailID = AdoFldLong(rsResults, "SourceDetailID", -1);
			aryMatchingEMRDetails.Add(pDetail);
			rsResults->MoveNext();
		}
		rsResults->Close();

	}NxCatchAll("Error in CEMRAnalysisDlg::LoadAllCandidateDetails");
}

// (b.savon 2011-11-02 11:07) - PLID 43205 - Added arydcTableColumns
//this function will take in existing matching details and load an EMN for each one,
//filling in strSentenceFormat and bHasData
// (j.jones 2009-04-09 14:45) - PLID 33916 - passed in bIncludeSpawningInfo
void CEMRAnalysisDlg::FillCandidateDetailsWithEMNData(CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails, BOOL bIncludeSpawningInfo, CArray<DynColumns*, DynColumns*> &arydcTableColumns)
{
	try {

		//force using the main thread's connection
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_ConnectionPtr pCon = GetRemoteDataSnapshot();

		m_progress.SetRange(0, aryMatchingEMRDetails.GetSize());
		m_progress.SetStep(1);
		m_progress.SetPos(0);

		// (a.walling 2010-06-02 13:31) - PLID 39007 - No longer -- too much memory, too many threads.
		//CArray<CEMN*, CEMN*> aryEMNs;
		CMap<long, long&, BYTE, BYTE> mapCheckedEMNs;
		BYTE bDummy = 0;
		mapCheckedEMNs.InitHashTable(211);

		int ixInitialDetail=0;
		for(ixInitialDetail=0; ixInitialDetail < aryMatchingEMRDetails.GetSize(); ixInitialDetail++) {
			
			// (a.walling 2010-06-02 13:31) - PLID 39007 - temporary EMN
			CEMN* pEMN = NULL;
			
			MatchingEMRDetails *pInitialMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(ixInitialDetail);
			long nCurrentEMNID = pInitialMatchingDetail->nEMNID;
			if (mapCheckedEMNs.Lookup(nCurrentEMNID, bDummy)) {
				continue; // skip this part
			}
			mapCheckedEMNs.SetAt(nCurrentEMNID, 1);

			for (int ixCurrentDetail = 0; ixCurrentDetail < aryMatchingEMRDetails.GetSize(); ixCurrentDetail++) {

				MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(ixCurrentDetail);

				// (a.walling 2010-06-02 13:31) - PLID 39007 - Process details one EMN at a time
				if (pMatchingDetail->nEMNID == nCurrentEMNID) {

					//check to see if the user cancelled
					PeekAndPump();
					if(!m_bIsLoading) {
						// (a.walling 2009-02-26 15:09) - PLID 33255 - we _NOW_ have to delete any EMNs we loaded
						// (a.walling 2010-06-02 13:31) - PLID 39007 - No longer -- too much memory, too many threads.
						/*
						for(i=aryEMNs.GetSize() - 1; i>=0; i--) {
							CEMN *pEMN = (CEMN*)aryEMNs.GetAt(i);
							delete pEMN;
						}
						aryEMNs.RemoveAll();
						*/

						// (a.walling 2010-06-02 13:31) - PLID 39007 - Free our temp EMN
						if (pEMN) {
							delete pEMN;
							pEMN = NULL;
						}

						ResetInterfaceOnCancel();
						return;
					}

					long nDetailID = pMatchingDetail->nEMRDetailID;

					if(nDetailID == -1) {
						ASSERT(FALSE);
					}
					else {
						//for each detail, we have to load its sentence format, and whether it has data
						CString strSentence;

						// (j.jones 2008-10-30 14:00) - PLID 31857 - If we have a Narrative or a Table,
						// we need to load the entire EMN, otherwise our results won't be reliable
						// (narratives won't show their linked items, tables won't show linked details).
						
						//first see if we already have this EMN
						//now if we do not have this EMN already, but this is a Narrative or Table,
						//create the EMN
						if(pEMN == NULL && (pMatchingDetail->eDataType == eitNarrative || pMatchingDetail->eDataType == eitTable)) {
							pEMN = new CEMN(NULL);
							pEMN->LoadFromEmnID(pMatchingDetail->nEMNID);
						}

						CEMNDetail *pDetail = NULL;
						// (j.jones 2008-10-30 14:02) - PLID 31857 - if we have an EMN, just find the already-loaded detail in it
						if(pEMN) {
							pDetail = pEMN->GetDetailByID(nDetailID);
						}

						//if we have no detail, load it now, which is way faster
						//than always loading the EMN
						BOOL bIsLocal = FALSE;
						if(pDetail == NULL) {
							// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
							pDetail = CEMNDetail::CreateDetail(NULL, "EMR Analysis local detail");
							// Load the detail
							pDetail->LoadFromDetailID(nDetailID, NULL);
							bIsLocal = TRUE;
						}

						if(pDetail) {
							//TES 9/16/2009 - PLID 35529 - Flag this detail as "windowless", this will cause it to load
							// more efficiently if it is a narrative that therefore needs a CEmrItemNarrativeDlg to parse properly.
							pDetail->SetWindowless();
							// (j.jones 2010-04-27 10:37) - PLID 37963 - for images, we don't have a sentence format
							if (eitImage == pDetail->m_EMRInfoType) {
								strSentence = "<Image>";
							} else {
								// Get the detail value in sentence form
								if(pDetail->GetStateVarType() == VT_NULL || (pDetail->GetStateVarType() == VT_BSTR && VarString(pDetail->GetState()).IsEmpty())) {
									//TES 2/26/2010 - PLID 37463 - The m_strLongForm variable has never been set at this point, and having the sentence
									// be blank for empty details seems like a better idea anyway (certainly nobody has complained).  So let's just
									// explicitly make it blank.
									strSentence = "";//pDetail->m_strLongForm;
								} else {
									CStringArray saDummy;							
									strSentence = ::GetSentence(pDetail, NULL, false, false, saDummy, ecfParagraph, NULL, NULL, NULL, pCon);
								}
							}
							pMatchingDetail->strSentenceFormat = strSentence;

							// (j.jones 2009-04-09 16:59) - PLID 33916 - fill the spawning information
							if(bIncludeSpawningInfo) {
								// (a.walling 2010-06-02 13:31) - PLID 39007 - This may load pEMN for us
								LoadSpawningInformation(pMatchingDetail, pMatchingDetail->nSourceDetailID, pEMN);
							}

							// (a.walling 2009-01-13 14:51) - PLID 32107 - Use the Detail's IsStateSet function
							// This used to call ::IsDetailStateSet(pMatchingDetail->eDataType, pDetail->GetState());
							// I don't believe there are any situations where the following assertion would fail.
							_ASSERTE(pMatchingDetail->eDataType == pDetail->m_EMRInfoType);
							pMatchingDetail->bHasData = pDetail->IsStateSet();

							// (b.savon 2011-11-01 15:39) - PLID 43205 - Save Single row/col data to be used later when it
							//	is inserted into the column
							if( pMatchingDetail->eDataType == eitTable ){
								//	One Column, Many Rows
								if( pDetail->GetColumnCount() == 1 && pDetail->GetRowCount() > 1 ){
									for( int nRow = 0; nRow < pDetail->GetRowCount(); nRow++ ){
										DynColumns* dcLocal = new DynColumns();
										dcLocal->nPatientID = pMatchingDetail->nPatientID;
										dcLocal->nEMNID = pMatchingDetail->nEMNID;
										dcLocal->nEMRID = pMatchingDetail->nEMRID;
										dcLocal->dtEMNDate = pMatchingDetail->dtEMNDate;
											
										dcLocal->strHeader = pMatchingDetail->strName + " - " + pDetail->GetRowPtr(nRow)->strName;

										//	Fill table element data
										TableElement *teData = pDetail->GetTableElementByRowColPtr(pDetail->GetRowPtr(nRow), pDetail->GetColumnPtr(0));
										if( teData && !teData->IsValueEmpty() ){
											CStringArray saTemp;
											dcLocal->strData = teData->GetValueAsOutput(pDetail, false, saTemp);
											arydcTableColumns.Add(dcLocal);
										}//END has element data
									}//END for all rows

									// One Row, Many Columns
								} else if( pDetail->GetRowCount() == 1 && pDetail->GetColumnCount() > 1 ){
									for( int nCol = 0; nCol < pDetail->GetColumnCount(); nCol++ ){
										DynColumns* dcLocal = new DynColumns();
										dcLocal->nPatientID = pMatchingDetail->nPatientID;
										dcLocal->nEMNID = pMatchingDetail->nEMNID;
										dcLocal->nEMRID = pMatchingDetail->nEMRID;
										dcLocal->dtEMNDate = pMatchingDetail->dtEMNDate;
											
										dcLocal->strHeader = pMatchingDetail->strName + " - " + pDetail->GetColumnPtr(nCol)->strName;

										//	Fill table element data
										TableElement *teData = pDetail->GetTableElementByRowColPtr(pDetail->GetRowPtr(0), pDetail->GetColumnPtr(nCol));
										if( teData && !teData->IsValueEmpty() ){
											CStringArray saTemp;
											dcLocal->strData = teData->GetValueAsOutput(pDetail, false, saTemp);
											arydcTableColumns.Add(dcLocal);
										}//END has element data									
									}//END for all columns
								}//END Row/Col check
							}//END is table

							// (j.jones 2008-10-30 14:03) - PLID 31857 - We don't need to delete detail if it was from the EMN,
							// because it would be handled when the EMN is deleted. But we may have created this detail on our
							//own, and if so, we need to delete it now.
							if(bIsLocal) {
								// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
								pDetail->__QuietRelease();
								//delete pDetail;
								pDetail = NULL;
							}
						}
						else {
							//don't throw an exception, but this should be impossible
							ASSERT(FALSE);

							pMatchingDetail->strSentenceFormat = "";
							pMatchingDetail->strSpawningInformation = "";
							pMatchingDetail->bHasData = FALSE;
						}
					}
				}
			}

			
			// (a.walling 2010-06-02 13:31) - PLID 39007 - Free our temp EMN
			if (pEMN) {
				delete pEMN;
				pEMN = NULL;
			}
			m_progress.StepIt();
		}

		m_progress.SetPos(aryMatchingEMRDetails.GetSize());

		// (j.jones 2008-10-30 12:25) - PLID 31857 - we now have to delete any EMNs we loaded
		// (a.walling 2010-06-02 13:31) - PLID 39007 - No longer -- too much memory, too many threads.
		/*
		for(i=aryEMNs.GetSize() - 1; i>=0; i--) {
			CEMN *pEMN = (CEMN*)aryEMNs.GetAt(i);
			delete pEMN;
		}
		aryEMNs.RemoveAll();
		*/

	}NxCatchAll("Error in CEMRAnalysisDlg::FillCandidateDetailsWithEMNData");
}

//this function will take in existing matching details that have already had their EMN data loaded,
//re-check all the "Has Data" statuses when requested, and check the bFilterOnAllItems status
void CEMRAnalysisDlg::FinalFilterCandidateDetails(BOOL bFilterOnAllItems, EMRAnalysisRecordFilterType eRecordFilter,
								 CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails,
								 CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails)
{
	try {

		//loop through each config detail setting, and if any use eadotItemHasData or eadotItemHasNoData,
		//validate that each related detail matches that setting
		int i = 0;
		for(i = 0; i<aryCachedConfigDetails.GetSize(); i++) {

			//check to see if the user cancelled
			PeekAndPump();
			if(!m_bIsLoading) {
				ResetInterfaceOnCancel();
				return;
			}

			//get the cached info
			CachedConfigDetail *pConfigDetail = (CachedConfigDetail*)aryCachedConfigDetails.GetAt(i);

			if(pConfigDetail->eOperator == eadotItemHasData || pConfigDetail->eOperator == eadotItemHasNoData) {
				//we do need to filter, so now loop through each related detail
				//and remove those that do not match
				int j = 0;
				for(j = aryMatchingEMRDetails.GetSize() - 1; j >= 0; j--) {
					MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(j);

					if(pMatchingDetail->nEMRInfoMasterID == pConfigDetail->nEMRInfoMasterID) {
						//it is a matching item, so now remove it if our filter doesn't match
						if((pMatchingDetail->bHasData && pConfigDetail->eOperator == eadotItemHasNoData)
							|| (!pMatchingDetail->bHasData && pConfigDetail->eOperator == eadotItemHasData)) {

							//doesn't match, so remove it
							delete (MatchingEMRDetails*)pMatchingDetail;
							aryMatchingEMRDetails.RemoveAt(j);
						}
					}
				}
			}
		}

		//hopefully we won't have eliminated every matching detail, but if we did,
		//leave now so we don't expend any more processing time
		if(aryMatchingEMRDetails.GetSize() == 0) {
			return;
		}

		//if we are not requiring that all items exist for a patient/EMN/EMR,
		//then we can leave now without any other filtering
		if(!bFilterOnAllItems) {
			return;
		}
		else {

			//we need to search by patient/EMN/EMR, and ensure that each patient/EMN/EMR has all the details,
			//depending on our eRecordFilter

			//get lists of unique patient IDs, EMNIDs, and EMRIDs
			CArray<long, long> aryPatientIDs;
			CArray<long, long> aryEMNIDs;
			CArray<long, long> aryEMRIDs;

			long nLastPatientID = -1;
			long nLastEMNID = -1;
			long nLastEMRID = -1;

			//the aryMatchingEMRDetails array should have been ordered by patient,
			//EMN, and EMR, so we can determine when the next one of these three types
			//changes in one pass
			for(i = 0; i<aryMatchingEMRDetails.GetSize(); i++) {
				MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(i);

				if(pMatchingDetail->nPatientID != nLastPatientID) {
					nLastPatientID = pMatchingDetail->nPatientID;
					aryPatientIDs.Add(pMatchingDetail->nPatientID);
				}

				if(pMatchingDetail->nEMNID != nLastEMNID) {
					nLastEMNID = pMatchingDetail->nEMNID;
					aryEMNIDs.Add(pMatchingDetail->nEMNID);
				}

				if(pMatchingDetail->nEMRID != nLastEMRID) {
					nLastEMRID = pMatchingDetail->nEMRID;
					aryEMRIDs.Add(pMatchingDetail->nEMRID);
				}
			}

			//now we have unique arrays of patients, EMNs, and EMRs, so what we need to do
			//is re-loop through the list and confirm that the patient, EMN, or EMR has
			//all the details this configuration is looking for

			for(i = 0; i<aryCachedConfigDetails.GetSize(); i++) {
				//get the cached info
				CachedConfigDetail *pConfigDetail = (CachedConfigDetail*)aryCachedConfigDetails.GetAt(i);

				//loop through the given record filter object and remove the ID
				//if a config detail doesn't have a matching EMN detail
				if(eRecordFilter == earftPatient) {

					int j = 0;
					for(j = aryPatientIDs.GetSize() - 1; j >= 0; j--) {
						long nPatientID = (long)aryPatientIDs.GetAt(j);

						BOOL bFound = FALSE;

						int k = 0;
						for(k = 0; k < aryMatchingEMRDetails.GetSize() && !bFound; k++) {
							MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(k);

							if(pMatchingDetail->nPatientID == nPatientID
								&& pMatchingDetail->nEMRInfoMasterID == pConfigDetail->nEMRInfoMasterID) {

								//we found a matching detail for this patient, so we're set
								bFound = TRUE;
							}
						}

						if(!bFound) {
							//we couldn't find a match, so this patient does not qualify
							aryPatientIDs.RemoveAt(j);

							//now we need to remove all the items for this patient
							for(k = aryMatchingEMRDetails.GetSize() - 1; k >= 0; k--) {
								MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(k);

								if(pMatchingDetail->nPatientID == nPatientID) {

									//delete this detail
									delete (MatchingEMRDetails*)pMatchingDetail;
									aryMatchingEMRDetails.RemoveAt(k);
								}
							}
						}
					}
				}
				else if(eRecordFilter == earftEMN) {

					int j = 0;
					for(j = aryEMNIDs.GetSize() - 1; j >= 0; j--) {
						long nEMNID = (long)aryEMNIDs.GetAt(j);

						BOOL bFound = FALSE;

						int k = 0;
						for(k = 0; k < aryMatchingEMRDetails.GetSize() && !bFound; k++) {
							MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(k);

							if(pMatchingDetail->nEMNID == nEMNID
								&& pMatchingDetail->nEMRInfoMasterID == pConfigDetail->nEMRInfoMasterID) {

								//we found a matching detail for this EMN, so we're set
								bFound = TRUE;
							}
						}

						if(!bFound) {
							//we couldn't find a match, so this EMN does not qualify
							aryEMNIDs.RemoveAt(j);

							//now we need to remove all the items for this EMN
							for(k = aryMatchingEMRDetails.GetSize() - 1; k >= 0; k--) {
								MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(k);

								if(pMatchingDetail->nEMNID == nEMNID) {

									//delete this detail
									delete (MatchingEMRDetails*)pMatchingDetail;
									aryMatchingEMRDetails.RemoveAt(k);
								}
							}
						}
					}
				}
				else if(eRecordFilter == earftEMR) {

					int j = 0;
					for(j = aryEMRIDs.GetSize() - 1; j >= 0; j--) {
						long nEMRID = (long)aryEMRIDs.GetAt(j);

						BOOL bFound = FALSE;

						int k = 0;
						for(k = 0; k < aryMatchingEMRDetails.GetSize() && !bFound; k++) {
							MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(k);

							if(pMatchingDetail->nEMRID == nEMRID
								&& pMatchingDetail->nEMRInfoMasterID == pConfigDetail->nEMRInfoMasterID) {

								//we found a matching detail for this EMR, so we're set
								bFound = TRUE;
							}
						}

						if(!bFound) {
							//we couldn't find a match, so this EMR does not qualify
							aryEMRIDs.RemoveAt(j);

							//now we need to remove all the items for this EMR
							for(k = aryMatchingEMRDetails.GetSize() - 1; k >= 0; k--) {
								MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(k);

								if(pMatchingDetail->nEMRID == nEMRID) {

									//delete this detail
									delete (MatchingEMRDetails*)pMatchingDetail;
									aryMatchingEMRDetails.RemoveAt(k);
								}
							}
						}
					}
				}
			}
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::FinalFilterCandidateDetails");
}

// (c.haag 2009-02-24 13:19) - PLID 33187 - Returns the first non-populated column of a row in m_ResultsList.
// If none is available, the column will equal the number of columns in the datalist.
short CEMRAnalysisDlg::CalculateFirstAvailableDynamicColumn(IRowSettingsPtr& pRow)
{
	BOOL bFoundAvailableColumn = FALSE;
	short nCol = rlcFirstDynamicColumn;
	while (!bFoundAvailableColumn && nCol < m_ResultsList->GetColumnCount()) {
		_variant_t v = pRow->GetValue(nCol);
		if (v.vt == VT_EMPTY || v.vt == VT_NULL) {
			// This column is not assigned a value, so it's good
			bFoundAvailableColumn = TRUE;
		}
		else {
			// Column in use. Go to the next one.
			nCol++;
		}
	} // while (!bFoundAvailableColumn) {

	// Keep in mind that nCol can actually be equal to the column count of the datalist.
	// This can happen if there's an absurdly large number of columns needed to fit all the
	// data. The caller must detect and handle this case (usually it would just find a new
	// row)
	return nCol;
}

//this function will take in the final detail list and display them on the result list,
//based on the group by setting
// (c.haag 2009-02-24 16:41) - PLID 33187 - Added eColumnGroupBy
void CEMRAnalysisDlg::AddDetailsToResultList(EMRAnalysisGroupByType eGroupBy, EMRAnalysisColumnGroupByType eColumnGroupBy,
											 CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails,
											 CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails)
{
	try {

		//the caller should have cleared the list before calling this function
		if(m_ResultsList->GetRowCount() > 0) {
			ASSERT(FALSE);
			m_ResultsList->Clear();
		}

		int i = 0;
		for(i = 0; i<aryMatchingEMRDetails.GetSize(); i++) {

			//check to see if the user cancelled
			PeekAndPump();
			if(!m_bIsLoading) {
				ResetInterfaceOnCancel();
				return;
			}

			MatchingEMRDetails *pMatchingDetail = (MatchingEMRDetails*)aryMatchingEMRDetails.GetAt(i);

			//which column index will this detail be in?
			short nCol;
			if (eacgbtOnePerItem == eColumnGroupBy) {
				// (c.haag 2009-02-24 12:59) - PLID 33187 - If each item has its own column, use
				// CalcResultColumnIndexForDetail to get the column index.
				nCol = CalcResultColumnIndexForDetail(pMatchingDetail, aryCachedConfigDetails);
			} else {
				// (c.haag 2009-02-24 12:59) - PLID 33187 - If we add columns on a need-to-add basis,
				// then this should always begin as the first dynamic column
				ASSERT(eacgbtCondensed == eColumnGroupBy);
				nCol = rlcFirstDynamicColumn;
			}

			//determine whether we need a new row or update an existing row
			BOOL bFound = FALSE;
			IRowSettingsPtr pSearchRow = m_ResultsList->GetFirstRow();
			while(pSearchRow && !bFound) {
				//if there are entries, see if we have a complete Patient/EMN/EMR match
				long nPatientID = VarLong(pSearchRow->GetValue(rlcPatientID));
				long nEMNID = VarLong(pSearchRow->GetValue(rlcEMNID));
				COleDateTime dtEMNDate = VarDateTime(pSearchRow->GetValue(rlcEMNDate));
				long nEMRID = VarLong(pSearchRow->GetValue(rlcEMRID));

				if((eGroupBy == eagbtPatient && nPatientID == pMatchingDetail->nPatientID)
					|| (eGroupBy == eagbtDate && nPatientID == pMatchingDetail->nPatientID && dtEMNDate == pMatchingDetail->dtEMNDate)
					|| (eGroupBy == eagbtEMN && nPatientID == pMatchingDetail->nPatientID && nEMNID == pMatchingDetail->nEMNID)					
					|| (eGroupBy == eagbtEMR && nPatientID == pMatchingDetail->nPatientID && nEMRID == pMatchingDetail->nEMRID)) {

					//it's a match, so update the column and continue					

					if (eacgbtOnePerItem == eColumnGroupBy) {
						// (c.haag 2009-02-24 12:59) - PLID 33187 - If every detail has its own column, and the column is 
						// in use, then skip this row altogether.
						//If this column already has data, move to the next row.
						//The result will be that we may have duplicate rows for
						//the same IDs, in order to report duplicate data. The
						//bonus is that if there are other duplicates, they will
						//reuse the duplicate rows.
						if(pSearchRow->GetValue(nCol).vt != VT_NULL) {
							//if non-null, we have data already (an empty string is valid data)
							pSearchRow = pSearchRow->GetNextRow();
							continue;
						}
					} else {
						// (c.haag 2009-02-24 13:02) - PLID 33187 - If we get here, then find the first available
						// column to fit this item in.
						ASSERT(eacgbtCondensed == eColumnGroupBy);
						nCol = CalculateFirstAvailableDynamicColumn(pSearchRow);
						if (nCol >= MAX_RESULT_COLUMNS) {
							// If there's no more room in the datalist columnwise, skip this row entirely
							pSearchRow = pSearchRow->GetNextRow();
							continue;
						}
					}

					//if we're still here, we can update the row
					bFound = TRUE;
					// (c.haag 2009-02-25 09:57) - PLID 33187 - If the item data is effectively empty, we need to
					// at least put the name of the item in the cell. Otherwise, the user gains nothing at all from
					// just a blank cell.
					// (j.jones 2009-04-09 14:50) - PLID 33916 - format our result into strResultToDisplay
					CString strResultToDisplay = pMatchingDetail->strSentenceFormat;
					if (eacgbtCondensed == eColumnGroupBy) {
						strResultToDisplay.Trim("\r\n\t ");
						// (j.jones 2010-04-27 10:37) - PLID 37963 - for images, we don't have a sentence format,
						// and we don't want to say "<No Data>", so just show the image name and <Image>
						if(eitImage == pMatchingDetail->eDataType) {
							strResultToDisplay.Format("Item %s: <Image>", pMatchingDetail->strName);
						}
						else if(strResultToDisplay.IsEmpty()) {
							strResultToDisplay.Format("Item %s: <No Data>", pMatchingDetail->strName);
						}
					}

					// (j.jones 2009-04-09 14:53) - PLID 33916 - prepend spawning info if we have it
					if(!pMatchingDetail->strSpawningInformation.IsEmpty()) {
						strResultToDisplay = pMatchingDetail->strSpawningInformation + ", " + strResultToDisplay;
					}

					//only display whatever we configured strResultToDisplay to contain
					pSearchRow->PutValue(nCol, (LPCTSTR)strResultToDisplay);

					//update the date, EMN / EMR / Pic IDs, and EMN & EMR descriptions if this is earlier data
					if(pMatchingDetail->dtEMNDate < dtEMNDate
						|| (pMatchingDetail->dtEMNDate == dtEMNDate && pMatchingDetail->nEMNID < nEMNID)) {

						//it's an earlier date, or same date and earlier EMN ID
						pSearchRow->PutValue(rlcEMNID, (long)pMatchingDetail->nEMNID);
						pSearchRow->PutValue(rlcEMNDate, _variant_t(pMatchingDetail->dtEMNDate, VT_DATE));
						pSearchRow->PutValue(rlcEMNDescription, (LPCTSTR)pMatchingDetail->strEMNDesc);
						pSearchRow->PutValue(rlcEMRID, (long)pMatchingDetail->nEMRID);
						pSearchRow->PutValue(rlcEMRDescription, (LPCTSTR)pMatchingDetail->strEMRDesc);
						pSearchRow->PutValue(rlcPicID, (long)pMatchingDetail->nPicID);			
						// (a.walling 2010-01-11 16:43) - PLID 31482
						if (m_hIconPreview != NULL && pMatchingDetail->nEMNID != -1) {
							pSearchRow->PutValue(rlcPreview, (long)m_hIconPreview);
						}
					}

					pSearchRow = pSearchRow->GetNextRow();
					continue;
				}

				pSearchRow = pSearchRow->GetNextRow();
			} // while(pSearchRow && !bFound) {

			if(!bFound) {
				//no matching entry, so we need to add a new row
				IRowSettingsPtr pNewRow = m_ResultsList->GetNewRow();
				pNewRow->PutValue(rlcPatientID, (long)pMatchingDetail->nPatientID);
				// (j.jones 2009-04-08 14:39) - PLID 33915 - added provider & secondary provider
				pNewRow->PutValue(rlcProviderName, (LPCTSTR)pMatchingDetail->strProviderNames);
				pNewRow->PutValue(rlcSecondaryProviderName, (LPCTSTR)pMatchingDetail->strSecondaryProviderNames);
				pNewRow->PutValue(rlcUserDefinedID, (long)pMatchingDetail->nUserDefinedID);
				pNewRow->PutValue(rlcPatientName, (LPCTSTR)pMatchingDetail->strPatientName);
				pNewRow->PutValue(rlcEMNID, (long)pMatchingDetail->nEMNID);
				pNewRow->PutValue(rlcEMNDate, _variant_t(pMatchingDetail->dtEMNDate, VT_DATE));
				pNewRow->PutValue(rlcEMNDescription, (LPCTSTR)pMatchingDetail->strEMNDesc);
				pNewRow->PutValue(rlcEMRID, (long)pMatchingDetail->nEMRID);
				pNewRow->PutValue(rlcEMRDescription, (LPCTSTR)pMatchingDetail->strEMRDesc);
				pNewRow->PutValue(rlcPicID, (long)pMatchingDetail->nPicID);
				// (a.walling 2010-01-11 16:43) - PLID 31482
				if (m_hIconPreview != NULL && pMatchingDetail->nEMNID != -1) {
					pNewRow->PutValue(rlcPreview, (long)m_hIconPreview);
				}

				if (eacgbtOnePerItem == eColumnGroupBy) {
					// (c.haag 2009-02-24 13:15) - PLID 33187 - If we get here, every item has its own
					// column; and the columns have already been created. Add them to the row.
					//add NULL to each dynamic column, save for the one column we are populating
					int j = 0;
					for(j = rlcFirstDynamicColumn; j<m_ResultsList->GetColumnCount(); j++) {
						if(j == nCol) {
							//add our data

							// (j.jones 2009-04-09 14:50) - PLID 33916 - format our result into strResultToDisplay
							CString strResultToDisplay = pMatchingDetail->strSentenceFormat;

							// (j.jones 2009-04-09 14:53) - PLID 33916 - prepend spawning info if we have it
							if(!pMatchingDetail->strSpawningInformation.IsEmpty()) {
								if(strResultToDisplay.IsEmpty()) {
									//state that we have no data, but don't include the item name,
									//as it is the column header
									strResultToDisplay = "<No Data>";
								}
								strResultToDisplay = pMatchingDetail->strSpawningInformation + ", " + strResultToDisplay;
							}
					
							pNewRow->PutValue(j, (LPCTSTR)strResultToDisplay);
						}
						else {
							//add NULL, do not add an empty string
							pNewRow->PutValue(j, g_cvarNull);
						}
					}
				} else {
					// (c.haag 2009-02-24 13:15) - PLID 33187 - If we get here, then insert
					// this result in the first possible column. Additionally, we need to check
					// for empty sentence formats, and if empty, we must replace it with the
					// name of the detail.					
					ASSERT(eacgbtCondensed == eColumnGroupBy);

					// (j.jones 2009-04-09 14:50) - PLID 33916 - format our result into strResultToDisplay
					CString strResultToDisplay = pMatchingDetail->strSentenceFormat;
					if(eacgbtCondensed == eColumnGroupBy) {
						strResultToDisplay.Trim("\r\n\t ");
						// (j.jones 2010-04-27 10:37) - PLID 37963 - for images, we don't have a sentence format,
						// and we don't want to say "<No Data>", so just show the image name and <Image>
						if(eitImage == pMatchingDetail->eDataType) {
							strResultToDisplay.Format("Item %s: <Image>", pMatchingDetail->strName);
						}
						else if(strResultToDisplay.IsEmpty()) {
							strResultToDisplay.Format("Item %s: <No Data>", pMatchingDetail->strName);
						}
					}

					// (j.jones 2009-04-09 14:53) - PLID 33916 - prepend spawning info if we have it
					if(!pMatchingDetail->strSpawningInformation.IsEmpty()) {
						strResultToDisplay = pMatchingDetail->strSpawningInformation + ", " + strResultToDisplay;
					}

					nCol = CalculateFirstAvailableDynamicColumn(pNewRow);
					if (nCol >= MAX_RESULT_COLUMNS) {
						ASSERT(FALSE); // For new rows, this should NEVER happen!
					} else {
						pNewRow->PutValue(nCol, (LPCTSTR)strResultToDisplay);
					}
				}

				m_ResultsList->AddRowSorted(pNewRow, NULL);
			}
		}		

	}NxCatchAll("Error in CEMRAnalysisDlg::AddDetailsToResultList");
}

//this function will determine which datalist column represents the info item referenced in the passed-in detail
short CEMRAnalysisDlg::CalcResultColumnIndexForDetail(MatchingEMRDetails *pMatchingDetail,
													  CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails)
{
	try {
		int i = 0;
		for(i = 0; i<aryCachedConfigDetails.GetSize(); i++) {
			CachedConfigDetail *pConfigDetail = (CachedConfigDetail*)aryCachedConfigDetails.GetAt(i);

			//which column index will this detail be in?
			if(pMatchingDetail->nEMRInfoMasterID == pConfigDetail->nEMRInfoMasterID) {
				//found it, so return the calculated column index
				return rlcFirstDynamicColumn + i;
			}
		}

		//if we're still here, we've got problems
		CString str;
		str.Format("Could not find column for EMR Info Master ID %li!", pMatchingDetail->nEMRInfoMasterID);
		ThrowNxException(str);

	}NxCatchAll("Error in CEMRAnalysisDlg::CalcResultColumnIndexForDetail");

	return -1;
}

//this function will disable everything but the cancel button when loading,
//and enable everything but the cancel button when not loading
void CEMRAnalysisDlg::ToggleInterface()
{
	try {

		BOOL bHasSelection = m_ConfigCombo->GetCurSel() != NULL;

		//disable all these controls if we are loading
		// (j.jones 2008-10-22 16:17) - PLID 31790 - added ability to export as csv
		m_btnExport.EnableWindow(!m_bIsLoading);		
		m_btnClose.EnableWindow(!m_bIsLoading);
		m_btnAdd.EnableWindow(!m_bIsLoading);
		m_btnEdit.EnableWindow(!m_bIsLoading);
		m_btnDelete.EnableWindow(!m_bIsLoading);
		m_radioAllDates.EnableWindow(!m_bIsLoading && bHasSelection);
		m_radioEMNDate.EnableWindow(!m_bIsLoading && bHasSelection);
		m_radioGroupByPatient.EnableWindow(!m_bIsLoading && bHasSelection);
		m_radioGroupByDate.EnableWindow(!m_bIsLoading && bHasSelection);
		m_radioGroupByEMN.EnableWindow(!m_bIsLoading && bHasSelection);
		m_radioGroupByEMR.EnableWindow(!m_bIsLoading && bHasSelection);
		m_dtTo.EnableWindow(!m_bIsLoading && bHasSelection && m_radioEMNDate.GetCheck());
		m_dtFrom.EnableWindow(!m_bIsLoading && bHasSelection && m_radioEMNDate.GetCheck());
		m_btnReloadResults.EnableWindow(!m_bIsLoading);
		m_ConfigCombo->Enabled = !m_bIsLoading;
		// (c.haag 2009-02-24 12:12) - PLID 33187 - Column group buttons
		m_radioColumnGroupByItem.EnableWindow(!m_bIsLoading && bHasSelection);
		m_radioColumnGroupByCondensed.EnableWindow(!m_bIsLoading && bHasSelection);

		// (j.jones 2009-04-08 14:39) - PLID 33915 - added checkboxes to show/hide columns
		m_checkShowProviderColumn.EnableWindow(!m_bIsLoading);
		m_checkShowSecondaryProviderColumn.EnableWindow(!m_bIsLoading);
		m_checkShowEMNDescColumn.EnableWindow(!m_bIsLoading);
		m_checkShowEMRDescColumn.EnableWindow(!m_bIsLoading);

		//enable the cancel button if we are loading
		m_btnCancelSearch.EnableWindow(m_bIsLoading);
		
		//leave m_ResultsList enabled at all times

	}NxCatchAll("Error in CEMRAnalysisDlg::ToggleInterface");
}
void CEMRAnalysisDlg::OnBnClickedBtnCancelSearch()
{
	try {

		if(!m_bIsLoading) {
			//find out how this button was enabled if m_bIsLoading is FALSE
			
			// (j.jones 2008-11-06 15:31) - Yazi hit this assertion once, I suspect it
			// could somehow due to clicking cancel a whole bunch of times, and one
			// extra OnCancel message made it through. Since that is not conclusive,
			// and this assertion is minor, I'm leaving it in anyways.
			ASSERT(FALSE);
			return;
		}

		m_bIsLoading = FALSE;

		//the loading process will periodically check m_bIsLoading,
		//and will then call ResetInterfaceOnCancel() if
		//m_bIsLoading is FALSE, and end the loading process

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBnClickedBtnCancelSearch");
}

//this function will re-enable the interface if the user cancelled
void CEMRAnalysisDlg::ResetInterfaceOnCancel()
{
	try {

		ToggleInterface();
		m_nxeditProgressStatus.SetWindowText("Load Cancelled.");
		m_progress.SetPos(0);
	
	}NxCatchAll("Error in CEMRAnalysisDlg::ResetInterfaceOnCancel");
}

void CEMRAnalysisDlg::MinimizeWindow()
{
	try {

		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (this->GetWindowPlacement(&wp)) {
			//Check if we are not minimized
			if (!this->IsIconic()) {
				wp.showCmd = SW_MINIMIZE;
				this->SetWindowPlacement(&wp);
				//ensure that the main app. has focus now
				if(GetMainFrame()) {
					GetMainFrame()->SetForegroundWindow();
				}
			}
		}
		
		// (a.walling 2010-01-11 14:59) - PLID 31482
		if (m_pEMRPreviewPopupDlg) {
			m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::MinimizeWindow");
}

// (j.jones 2008-10-22 16:17) - PLID 31790 - added ability to export as csv
void CEMRAnalysisDlg::OnBtnEmrAnalysisExport()
{
	try {

		if(m_ResultsList->GetRowCount() == 0) {
			AfxMessageBox("There are no results in the list to export.");
			return;
		}

		CString strFilename = "EMRAnalysis";

		CFileDialog fd(FALSE, "csv", strFilename, OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY|OFN_EXPLORER|OFN_ENABLESIZING, "CSV Files (*.csv;*.txt)|*.csv;*.txt|All Files (*.*)|*.*||", this);
		if (fd.DoModal() == IDOK) {
			// Open the output file
			strFilename = fd.GetPathName();
			CFile f(strFilename, CFile::modeWrite|CFile::modeCreate|CFile::shareDenyWrite);

			CWaitCursor pWait;
			
			// Get the export text
			CString strExportText = CalcResultsetAsCsv();
			
			// Write the text to the file
			f.Write((LPCTSTR)strExportText, strExportText.GetLength());

			//close the file
			f.Close();

			//the file dialog ought to have always appended .csv, but just incase
			//they did not save a .csv or .txt, do not prompt to open the file
			//unless it has one of these two extensions
			if(strFilename.GetLength() > 4 && (strFilename.Right(4) == ".csv" || strFilename.Right(4) == ".txt")) {
				if(IDYES == MessageBox("The results have been successfully exported. Would you like to open the exported file now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
					if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, GetFilePath(strFilename), SW_MAXIMIZE) < 32) {
						AfxMessageBox("Could not open the file. You will need to try to open it manually.");						
					}
				}
				return;
			}

			AfxMessageBox("The results have been successfully exported.");
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::OnBtnEmrAnalysisExport");
}

// (j.jones 2008-10-22 16:17) - PLID 31790 - added ability to export as csv
CString CEMRAnalysisDlg::CalcExportTextValue(IN const CString &strValue)
{
	CString strOut = strValue;

	if (strOut.Find(',') != -1 || strOut.Find("\r") != -1 || strOut.Find("\n") != -1) {
		// We have to put the string in quotes
		strOut = "\"" + ConvertToQuotableString(strOut, true) + "\"";
		return strOut;
	} else {
		// We're safe so we don't have to put the string in quotes
		return strOut;
	}
}

// (j.jones 2008-10-22 16:17) - PLID 31790 - added ability to export as csv
// This functionality is based on how NxQuery exports datalist results.
CString CEMRAnalysisDlg::CalcResultsetAsCsv()
{
	_variant_t varEmpty;
	varEmpty.vt = VT_EMPTY;
	
	CString strExportText;

	// (j.jones 2010-05-20 10:38) - PLID 38798 - we need to define which columns are exported
	CWordArray aryColumnIndexes;

	// First add the headers, and define the columns that need exported
	{
		short nColCount = m_ResultsList->GetColumnCount();
		for (short i=0; i<nColCount; i++) {

			//definitely skip the hidden ID columns
			// (a.walling 2010-01-11 16:45) - PLID 31482 - Skip preview column as well
			if(i == rlcPatientID || i == rlcEMNID || i == rlcEMRID || i == rlcPicID || i == rlcPreview) {
				continue;
			}

			// (j.jones 2009-04-08 14:39) - PLID 33915 - skip hidden columns
			if(!m_checkShowProviderColumn.GetCheck() && i == rlcProviderName) {
				continue;
			}
			if(!m_checkShowSecondaryProviderColumn.GetCheck() && i == rlcSecondaryProviderName) {
				continue;
			}
			if(!m_checkShowEMNDescColumn.GetCheck() && i == rlcEMNDescription) {
				continue;
			}
			if(!m_checkShowEMRDescColumn.GetCheck() && i == rlcEMRDescription) {
				continue;
			}

			// If the column has the csVisible style and has at least non-zero width
			IColumnSettingsPtr pCol = m_ResultsList->GetColumn(i);
			if (pCol != NULL && (pCol->GetColumnStyle() & csVisible) && pCol->GetStoredWidth() > 0) {
				// Add that cell's text
				strExportText += CalcExportTextValue((LPCTSTR)pCol->GetColumnTitle()) + ",";

				// (j.jones 2010-05-20 10:38) - PLID 38798 - we need to tell the datalist to export this column
				aryColumnIndexes.Add((WORD)i);
			}
		}

		// Remove the trailing delimiter
		long nLen = strExportText.GetLength();
		if (nLen >= 1) {
			ASSERT(strExportText.Right(1) == ",");
			strExportText.Delete(nLen-1, 1);
		}

		strExportText += "\r\n";
	}

	// (j.jones 2010-05-20 11:01) - PLID 38798 - pass in a safe array of our column indices
	COleSafeArray sa;
	sa.CreateOneDim(VT_UI2, aryColumnIndexes.GetSize(), aryColumnIndexes.GetData(), 0);
	_variant_t varSafeArray = _variant_t(sa.Detach(), false);
	
	// Got the headers, now add the data
	strExportText += (LPCTSTR)m_ResultsList->CalcExportText(efCommaDelimited, VARIANT_FALSE, varEmpty, &varSafeArray);

	// And return
	return strExportText;
}

// (j.jones 2009-04-08 16:14) - PLID 33915 - added checkboxes to show/hide columns
void CEMRAnalysisDlg::OnCheckShowProviderName()
{
	try {

		SetRemotePropertyInt("EMRAnalysis_ShowProvider", m_checkShowProviderColumn.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		ToggleColumns();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnCheckShowProviderName");
}

void CEMRAnalysisDlg::OnCheckShowSecProviderName()
{
	try {

		SetRemotePropertyInt("EMRAnalysis_ShowSecondaryProvider", m_checkShowSecondaryProviderColumn.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		ToggleColumns();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnCheckShowSecProviderName");
}

void CEMRAnalysisDlg::OnCheckShowEmnDescription()
{
	try {

		SetRemotePropertyInt("EMRAnalysis_ShowEMNDesc", m_checkShowEMNDescColumn.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		ToggleColumns();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnCheckShowEmnDescription");
}

void CEMRAnalysisDlg::OnShowEmrDescription()
{
	try {

		SetRemotePropertyInt("EMRAnalysis_ShowEMRDesc", m_checkShowEMRDescColumn.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		ToggleColumns();

	}NxCatchAll("Error in CEMRAnalysisDlg::OnShowEmrDescription");
}

// (j.jones 2009-04-08 17:43) - PLID 33915 - added ability to show/hide certain columns
void CEMRAnalysisDlg::ToggleColumns()
{
	try {

		IColumnSettingsPtr pProviderCol = m_ResultsList->GetColumn(rlcProviderName);
		if(m_checkShowProviderColumn.GetCheck()) {
			pProviderCol->ColumnStyle = csWidthData|csVisible;
			pProviderCol->PutStoredWidth(65);
		}
		else {
			pProviderCol->ColumnStyle = csFixedWidth|csVisible;			
			pProviderCol->PutStoredWidth(0);
		}

		IColumnSettingsPtr pSecondaryProviderCol = m_ResultsList->GetColumn(rlcSecondaryProviderName);
		if(m_checkShowSecondaryProviderColumn.GetCheck()) {
			pSecondaryProviderCol->ColumnStyle = csWidthData|csVisible;
			pSecondaryProviderCol->PutStoredWidth(110);
		}
		else {
			pSecondaryProviderCol->ColumnStyle = csFixedWidth|csVisible;
			pSecondaryProviderCol->PutStoredWidth(0);
		}

		IColumnSettingsPtr pEMNDescCol = m_ResultsList->GetColumn(rlcEMNDescription);
		if(m_checkShowEMNDescColumn.GetCheck()) {
			pEMNDescCol->ColumnStyle = csWidthData|csVisible;
			pEMNDescCol->PutStoredWidth(120);
		}
		else {
			pEMNDescCol->ColumnStyle = csFixedWidth|csVisible;
			pEMNDescCol->PutStoredWidth(0);
		}

		IColumnSettingsPtr pEMRDescCol = m_ResultsList->GetColumn(rlcEMRDescription);
		if(m_checkShowEMRDescColumn.GetCheck()) {
			pEMRDescCol->ColumnStyle = csWidthData|csVisible;
			pEMRDescCol->PutStoredWidth(120);
		}
		else {
			pEMRDescCol->ColumnStyle = csFixedWidth|csVisible;
			pEMRDescCol->PutStoredWidth(0);
		}

	}NxCatchAll("Error in CEMRAnalysisDlg::ToggleColumns");
}

// (j.jones 2009-04-09 16:59) - PLID 33916 - fill the spawning information in pDetail->strSpawningInformation,
// by finding the sentence format for the spawning item, and prepending it to strSpawningInformation
// (a.walling 2010-06-02 13:31) - PLID 39007
void CEMRAnalysisDlg::LoadSpawningInformation(MatchingEMRDetails *pMatchingDetail, long nSpawningDetailID, CEMN* &pEMN)
{
	//throw exceptions to the caller

	if(nSpawningDetailID == -1) {
		//no spawning detail
		return;
	}

	//force using the main thread's connection
	//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
	_ConnectionPtr pCon = GetRemoteDataSnapshot();

	//load the sentence format
	CString strSentence;

	//if we have a Narrative or a Table, we need to load the entire EMN, otherwise our results won't be reliable
	//(narratives won't show their linked items, tables won't show linked details)
	
	CEMNDetail *pDetail = NULL;
	//if we have an EMN, just find the already-loaded detail in it
	if(pEMN) {
		pDetail = pEMN->GetDetailByID(nSpawningDetailID);
	}

	//if we have no detail, load it now, which is way faster
	//than always loading the EMN
	BOOL bIsLocal = FALSE;
	if(pDetail == NULL) {
		// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
		pDetail = CEMNDetail::CreateDetail(NULL, "EMR Analysis local spawning detail");
		// Load the detail
		pDetail->LoadFromDetailID(nSpawningDetailID, NULL);
		bIsLocal = TRUE;
	}

	//only now do we know the data type of this spawning detail, and this is a Narrative or Table,
	//we need to create the full EMN and add it to our array
	if(pEMN == NULL && (pDetail->m_EMRInfoType == eitNarrative || pDetail->m_EMRInfoType == eitTable)) {
		pEMN = new CEMN(NULL);
		pEMN->LoadFromEmnID(pMatchingDetail->nEMNID);

		//clear the detail we loaded
		if(bIsLocal) {
			// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
			pDetail->__QuietRelease();
			//delete pDetail;
			pDetail = NULL;
			bIsLocal = FALSE;
		}

		//and reload the detail from the EMN
		if(pEMN) {
			pDetail = pEMN->GetDetailByID(nSpawningDetailID);
		}
	}

	if(pDetail) {
		// Get the detail value in sentence form
		if(pDetail->GetStateVarType() == VT_NULL || (pDetail->GetStateVarType() == VT_BSTR && VarString(pDetail->GetState()).IsEmpty())) {
			//TES 2/26/2010 - PLID 37463 - Check whether to use the "Smart Stamps" long form.
			// (z.manning 2010-07-26 15:06) - PLID 39848 - All tables use the same long form now
			strSentence = pDetail->m_strLongForm;
		} else {
			CStringArray saDummy;
			if (eitImage == pDetail->m_EMRInfoType) {
				//this is possible as images can spawn things
				strSentence = pDetail->GetLabelText();
			} else {
				strSentence = ::GetSentence(pDetail, NULL, false, false, saDummy, ecfParagraph, NULL, NULL, NULL, pCon);
			}
		}

		//now prepend this sentence format into the spawned information
		strSentence.Trim("\r\n\t ");
		if(strSentence.IsEmpty()) {
			//use the info name
			strSentence = pDetail->GetLabelText();

			ASSERT(!strSentence.IsEmpty());
		}
		if(!pMatchingDetail->strSpawningInformation.IsEmpty()) {
			CString strNew = strSentence + ", " + pMatchingDetail->strSpawningInformation;
			pMatchingDetail->strSpawningInformation = strNew;
		}
		else {
			pMatchingDetail->strSpawningInformation = strSentence;
		}

		//now recursively do this again
		LoadSpawningInformation(pMatchingDetail, pDetail->GetSourceDetailID(), pEMN);

		// (j.jones 2008-10-30 14:03) - PLID 31857 - We don't need to delete detail if it was from the EMN,
		// because it would be handled when the EMN is deleted. But we may have created this detail on our
		//own, and if so, we need to delete it now.
		if(bIsLocal) {
			// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
			pDetail->__QuietRelease();
			//delete pDetail;
			pDetail = NULL;
		}
	}
	else {
		//this is possible if the detail was deleted, which is allowed
		//ASSERT(FALSE);
	}
}

void CEMRAnalysisDlg::OnDestroy()
{
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	if (m_pEMRPreviewPopupDlg) {
		m_pEMRPreviewPopupDlg->DestroyWindow();
		delete m_pEMRPreviewPopupDlg;
		m_pEMRPreviewPopupDlg = NULL;
	}

	if (m_hIconPreview) {
		DestroyIcon(m_hIconPreview);
	}

	CNxDialog::OnDestroy();
}

// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
void CEMRAnalysisDlg::ShowPreview(long nPatID, long nEMNID)
{
	if (nPatID == -1 || nEMNID == -1) {
		return;
	}

	if (m_pEMRPreviewPopupDlg == NULL) {
		// create the dialog!

		// (a.walling 2007-04-13 09:49) - PLID 25648 - Load and initialize our preview popup
		m_pEMRPreviewPopupDlg = new CEMRPreviewPopupDlg(this);
		m_pEMRPreviewPopupDlg->Create(IDD_EMR_PREVIEW_POPUP, this);

		// (a.walling 2010-01-11 12:37) - PLID 31482
		m_pEMRPreviewPopupDlg->RestoreSize("EMRAnalysis");
	}

	// (z.manning 2012-09-10 15:40) - PLID 52543 - Get the EMN modified date
	COleDateTime dtModifiedDate = VarDateTime(GetTableField("EmrMasterT", "ModifiedDate", "ID", nEMNID));
	
	// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
	// of all available EMN IDs, but since we haven't opened the dialog yet,
	// we can pass in an empty array.
	// (z.manning 2012-09-10 15:41) - PLID 52543 - Use the new EmnPreviewPopup struct
	EmnPreviewPopup emn(nEMNID, dtModifiedDate);
	m_pEMRPreviewPopupDlg->SetPatientID(nPatID, emn);
	m_pEMRPreviewPopupDlg->PreviewEMN(emn, 0);

	// (a.walling 2010-01-11 16:20) - PLID 27733 - Only show if it is not already
	if (!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
		m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOWNA);
	}
}
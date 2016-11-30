// EMRSearch.cpp : implementation file
//

#include "stdafx.h"
#include "EMRSearch.h"
#include "multiselectdlg.h"
#include "datetimeutils.h"
#include "filter.h"
#include "filtereditdlg.h"
#include "groups.h"
#include "globalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "PatientsRc.h"
#include "EmrUtils.h"
#include "EMRPreviewPopupDlg.h"
#include "foreach.h"
#include "NxAPI.h"
#include "NxAPIUtils.h"	// (j.dinatale 2013-01-31 16:49) - PLID 54911
#include "DiagSearchUtils.h" // (a.wilson 2014-02-28 12:16) - PLID 60780

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.wilson 2014-02-28 12:25) - PLID 60780 - diagnosis list enum
enum DiagnosisListColumns {
	dlcID = 0,
	dlcCode = 1,
	dlcDescription = 2,
};

/////////////////////////////////////////////////////////////////////////////
// CEMRSearch dialog

CEMRSearch::CEMRSearch(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRSearch::IDD, pParent, "CEMRSearch")
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
	m_bAdvanced = true;
	m_bDialogInitialized = false;
	m_bNoStatus = false;
	m_bRefreshOnFilterChange = false;
	m_bRefreshOnWindowRestore = false;

	// (a.walling 2010-01-11 12:11) - PLID 31482
	m_hIconPreview = NULL;
	m_pEMRPreviewPopupDlg = NULL;

	// (b.spivey, January 29, 2013) - PLID 48370 - Assume this is off, which is the default behavior.
	m_bShowCurrentPatientOnly = false; 

	m_nCurEMNID = -1;
	m_nNextEMNID = -1;
	m_nTopRowEMNID = -1;
}


void CEMRSearch::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRSearch)
	DDX_Control(pDX, IDC_REMEMBERCOLUMNS, m_btnRememberColumns);
	DDX_Control(pDX, IDC_REFRESH_WHEN_FILTERS_CHANGE, m_btnRefreshOnFilterChange);
	DDX_Control(pDX, IDC_REFRESH_WHEN_WINDOW_RESTORED, m_btnRefreshOnWindowRestore);	
	DDX_Control(pDX, IDC_EMRSEARCH_REFRESH, m_btnRefresh);
	DDX_Control(pDX, IDC_STATUS_FINISHED, m_btnStatusFinished);
	DDX_Control(pDX, IDC_STATUS_LOCKED, m_btnStatusLocked);
	DDX_Control(pDX, IDC_STATUS_OPEN, m_btnStatusOpen);
	DDX_Control(pDX, IDC_STATUS_OTHER, m_btnStatusOther);
	DDX_Control(pDX, IDC_SHOW_SEENPATIENTS_WITH, m_checkShowSeenWith);
	DDX_Control(pDX, IDC_SHOW_SEENPATIENTS, m_checkShowSeen);
	DDX_Control(pDX, IDC_EXCLUDE_PENDING, m_checkExcludePending); // (a.walling 2011-01-13 09:58) - PLID 41542
	DDX_Control(pDX, IDC_EMNDATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_EMNDATE_TO, m_dtTo);
	DDX_Control(pDX, IDC_STATIC_FROM, m_nxstaticFrom);
	DDX_Control(pDX, IDC_STATIC_TO, m_nxstaticTo);
	DDX_Control(pDX, IDC_STATIC_PROC, m_nxstaticProc);
	DDX_Control(pDX, IDC_STATIC_DIAG, m_nxstaticDiag);
	DDX_Control(pDX, IDC_STATUSLABEL, m_nxstaticStatuslabel);
	DDX_Control(pDX, IDC_STATIC_PROB, m_nxstaticProb);
	DDX_Control(pDX, IDC_STATIC_PROV, m_nxstaticProv);
	DDX_Control(pDX, IDC_STATIC_SEC_PROV, m_nxstaticSecProv);
	DDX_Control(pDX, IDC_STATIC_TECH, m_nxstaticTech);
	DDX_Control(pDX, IDC_STATIC_LOC, m_nxstaticLoc);
	DDX_Control(pDX, IDC_STATIC_FILTER, m_nxstaticFilter);
	DDX_Control(pDX, ID_EMRSEARCH_CLOSE, m_btnEMRSearchClose);
	DDX_Control(pDX, IDC_DATE_GROUP, m_btnDateGroup);
	DDX_Control(pDX, IDC_EMRGROUP_1, m_btnEmrgroup1);
	DDX_Control(pDX, IDC_EMRGROUP_STATUS, m_btnEmrgroupStatus);
	DDX_Control(pDX, IDC_EMRGROUP_2, m_btnEmrgroup2);
	DDX_Control(pDX, ID_EMRSEARCH_PREVIEW, m_btnEMRSearchPreview);
	DDX_Control(pDX, IDC_SHOW_CURRENT_PATIENT, m_btnShowCurrentPatientOnly);
	DDX_Control(pDX, IDC_EMRSEARCH_SIGN_SELECTED, m_btnSignSelected);
	DDX_Control(pDX, IDC_STATIC_CHART, m_nxstaticChart);
	DDX_Control(pDX, IDC_STATIC_CATEGORIES, m_nxstaticCategory);
	DDX_Control(pDX, IDC_EMR_SEARCH_DIAGNOSIS_NONE, m_btnDiagnosisIncludeNone);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CEMRSearch, IDC_EMNDATE_FROM, 2 /* Change */, OnChangeDateFrom, VTS_NONE)
//	ON_EVENT(CEMRSearch, IDC_EMNDATE_TO, 2 /* Change */, OnChangeDateTo, VTS_NONE)
//	ON_EVENT(CEMRSearch, IDC_EMNDATE_FROM, 4 /* DropDown */, OnDropDownDateFrom, VTS_NONE)
//	ON_EVENT(CEMRSearch, IDC_EMNDATE_FROM, 3 /* CloseUp */, OnCloseUpDateFrom, VTS_NONE)
//	ON_EVENT(CEMRSearch, IDC_EMNDATE_TO, 3 /* CloseUp */, OnCloseUpDateTo, VTS_NONE)
//	ON_EVENT(CEMRSearch, IDC_EMNDATE_TO, 4 /* DropDown */, OnDropDownDateTo, VTS_NONE)

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
BEGIN_MESSAGE_MAP(CEMRSearch, CNxDialog)
	//{{AFX_MSG_MAP(CEMRSearch)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EMNDATE_FROM, OnChangeDateFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EMNDATE_TO, OnChangeDateTo)
	ON_NOTIFY(DTN_CLOSEUP, IDC_EMNDATE_FROM, OnCloseUpDateFrom)
	ON_NOTIFY(DTN_CLOSEUP, IDC_EMNDATE_TO, OnCloseUpDateTo)
	ON_NOTIFY(DTN_DROPDOWN, IDC_EMNDATE_FROM, OnDropDownDateFrom)
	ON_NOTIFY(DTN_DROPDOWN, IDC_EMNDATE_TO, OnDropDownDateTo)

	ON_BN_CLICKED(IDC_DATEFILTER_QUICK, OnDatefilterQuick)
	ON_BN_CLICKED(ID_EMRSEARCH_CLOSE, OnClose)
	ON_BN_CLICKED(IDC_EMRSEARCH_EDIT_FILTER, OnEditFilter)
	ON_BN_CLICKED(IDC_REMEMBERCOLUMNS, OnRememberColumns)
	ON_BN_CLICKED(IDC_REFRESH_WHEN_FILTERS_CHANGE, OnCheckRefreshOnFilterChange)
	ON_BN_CLICKED(IDC_REFRESH_WHEN_WINDOW_RESTORED, OnCheckRefreshWhenWindowRestored)
	ON_BN_CLICKED(IDC_STATUS_OPEN, OnStatusOpen)
	ON_BN_CLICKED(IDC_STATUS_FINISHED, OnStatusFinished)
	ON_BN_CLICKED(IDC_STATUS_LOCKED, OnStatusLocked)
	ON_BN_CLICKED(IDC_EMR_SHOW_ADVANCED, OnShowAdvanced)
	ON_BN_CLICKED(IDC_EMRSEARCH_REFRESH, OnEmrSearchRefresh)
	ON_BN_CLICKED(ID_EMRSEARCH_PREVIEW, OnEmrsearchPreview)
	ON_BN_CLICKED(IDC_SHOW_SEENPATIENTS, OnShowSeenPatients)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	//ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_BN_CLICKED(IDC_SHOW_SEENPATIENTS_WITH, OnShowSeenpatientsWith)
	ON_BN_CLICKED(IDC_EXCLUDE_PENDING, OnExcludePending) // (a.walling 2011-01-13 12:00) - PLID 41542	
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SHOW_CURRENT_PATIENT, OnBnClickedShowCurrentPatient)
	// (a.walling 2010-01-11 13:21) - PLID 31482
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_STATUS_OTHER, OnStatusOther)
	ON_BN_CLICKED(IDC_EMRSEARCH_SIGN_SELECTED, &CEMRSearch::OnBnClickedEmrsearchSignSelected)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_EMR_SEARCH_DIAGNOSIS_NONE, &CEMRSearch::OnBnClickedEmrSearchDiagnosisNone)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRSearch message handlers

BOOL CEMRSearch::OnInitDialog() 
{

	try {
		
		CNxDialog::SetMinSize(860, 440); // (r.goldschmidt 2014-10-06 10:05) - PLID 62648 - Enforce minimum opening size (default is 990x614)
		CNxDialog::OnInitDialog();		
		CNxDialog::SetMinSize(0, 0); // PLID 62648 - Allow resizing down to nothing

		// (a.walling 2011-01-26 18:00) - PLID 41542 - Bulk cache properties
		g_propManager.BulkCache("EMRSearch", propbitNumber | propbitText, 
			"(Username = '<None>' OR Username = '%s') AND Name IN ("
			"'EMRSearchSuppressSeenPatients', "
			"'EMRSearchShowSeenPatientsWith', "
			"'EMRSearchExcludePendingAppts', "
			"'EmrSearchRememberColumns', "
			"'EmrSearchAutoRefresh', "				// (j.jones 2014-05-15 16:40) - PLID 61798 - this now ties to m_bRefreshOnFilterChange			
			"'EmrSearchRefreshOnWindowRestore', "	// (j.jones 2014-05-15 16:40) - PLID 61798 - added
			"'EmrSearchDialogWidth', "
			"'EmrSearchDialogHeight', "
			"'EmrSearchColumnWidths' "
			", 'SignatureCheckPasswordEMR' " // (z.manning 2011-10-31 14:19) - PLID 44594
			", 'EmrPostSignatureInsertStatus' " // (z.manning 2011-10-31 14:19) - PLID 44594
			", 'EmrPostSignatureInsertStatus', " // (z.manning 2012-07-03 10:01) - PLID 50958
			// (b.spivey, January 17, 2013) - PLID 48370 - All of these are saved to user names. 
			"'EMRSearchDateTypeFilter', "
			"'EMRSearchProcedureFilter', "
			"'EMRSearchDiagnosisFilterNone', "
			"'EMRSearchAllOrAnyProcedureFilter', " 
			"'EMRSearchProblemsFilter', "
			""
			"'EMRSearchProviderFilter', "
			"'EMRSearchSecondaryProviderFilter', "
			"'EMRSearchAssistTechFilter', "
			"'EMRSearchLocationFilter', "
			"'EMRSearchLetterWritingFilter', "
			""
			"'EMNSearchStatusOpen', "
			"'EMNSearchStatusFinished', "
			"'EMNSearchStatusLocked', "
			"'EMRSearchShowCurrentPatientOnly', "
			"'EMNSearchStatusOther', "
			"'EMNSearchStatusOtherFilter', "
			"'EMRSearchApptRefPhysFilter', "	// (j.dinatale 2013-01-24 15:24) - PLID 54777
			// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart & category filters
			"'EMRSearchChartFilter', "	
			"'EMRSearchCategoryFilter', "	
			""
			// (b.spivey, January 23, 2013) - PLID 48370 - Save the IDs for multiselects.
			"'EMRSearchProcedureIDs', "
			"'EMRSearchDiagnosisIDs', "
			"'EMRSearchProblemsIDs', "
			"'EMRSearchProviderIDs', "
			"'EMRSearchSecondaryProviderIDs', "
			"'EMRSearchAssistTechIDs', "
			"'EMRSearchLocationIDs', "
			"'EMNSearchStatusOtherIDs', "
			"'EMRSearchApptRefPhysIDs', "	// (j.dinatale 2013-01-24 15:24) - PLID 54777
			// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart & category filters
			"'EMRSearchChartIDs', "
			"'EMRSearchCategoryIDs' "
		")", _Q(GetCurrentUserName()));
		
		// (a.walling 2010-01-11 12:11) - PLID 31482
		m_hIconPreview = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16,16, 0);

		// (c.haag 2008-05-19 16:26) - PLID 29806 - NxIconify the buttons
		m_btnEMRSearchClose.AutoSet(NXB_CLOSE);
		m_btnEMRSearchPreview.AutoSet(NXB_PRINT_PREV);
		m_btnSignSelected.AutoSet(NXB_MODIFY);

		// (j.jones 2014-05-15 17:07) - PLID 61798 - made this use the refresh icon
		m_btnRefresh.AutoSet(NXB_REFRESH);

		// (j.jones 2006-12-21 12:39) - PLID 23938 - returning here does not
		// close the dialog, the permission should be checked prior to this point
		/*
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
			return TRUE;
		}
		*/

		// (a.walling 2006-10-05 15:44) - PLID 22875 - Create an icon for the dialog in the taskbar if necessary
		//								  PLID 22877 - and respect the preference to not do so
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connections on the datalists
		m_dlDateType = BindNxDataList2Ctrl(IDC_DATEFILTER_TYPE, GetRemoteDataSnapshot(), false);
		m_dlProcedure = BindNxDataList2Ctrl(IDC_LIST_PROCEDURE, GetRemoteDataSnapshot(), false);
		m_dlProblems = BindNxDataList2Ctrl(IDC_LIST_PROBLEMS, GetRemoteDataSnapshot(), false);
		m_dlProvider = BindNxDataList2Ctrl(IDC_LIST_PROVIDER, GetRemoteDataSnapshot(), false);
		// (z.manning 2008-11-12 14:32) - PLID 26167 - Added secondary provider
		m_dlSecProvider = BindNxDataList2Ctrl(IDC_LIST_SEC_PROVIDER, GetRemoteDataSnapshot(), false);
		// (d.lange 2011-04-22 14:46) - PLID 43381 - Added Assistant/Tech
		m_dlTechnician = BindNxDataList2Ctrl(IDC_LIST_TECH, GetRemoteDataSnapshot(), false);
		m_dlLocation = BindNxDataList2Ctrl(IDC_LIST_LOCATION, GetRemoteDataSnapshot(), false);
		m_dlFilter = BindNxDataList2Ctrl(IDC_LIST_EMRFILTER, GetRemoteDataSnapshot(), false);
		m_dlEMNList = BindNxDataList2Ctrl(IDC_EMNLIST, GetRemoteDataSnapshot(), false);
		// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status dropdown
		m_dlOtherStatusCombo = BindNxDataList2Ctrl(IDC_LIST_OTHER_STATUS, GetRemoteDataSnapshot(), false);
		// (j.dinatale 2013-01-23 15:47) - PLID 54777 - appt ref phys filter
		m_dlApptRefPhys = BindNxDataList2Ctrl(IDC_LIST_APPT_REF_PHYS, GetRemoteDataSnapshot(), false);
		// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart & category filters
		m_dlChartCombo = BindNxDataList2Ctrl(IDC_COMBO_CHART, GetRemoteDataSnapshot(), false);
		m_dlCategoryCombo = BindNxDataList2Ctrl(IDC_COMBO_CATEGORIES, GetRemoteDataSnapshot(), false);	
		// (a.wilson 2014-02-28 12:18) - PLID 60780 - bind diagnosis search and list.
		m_dlDiagnosisSearch = DiagSearchUtils::BindDiagDualSearchListCtrl(this, IDC_EMR_SEARCH_DIAGNOSIS_SEARCH, GetRemoteData());
		m_dlDiagnosisList = BindNxDataList2Ctrl(IDC_EMR_SEARCH_DIAGNOSIS_LIST, false);

		_variant_t vtNull;
		vtNull.vt = VT_NULL;

		m_dtFrom.SetValue((_variant_t)COleDateTime::GetCurrentTime());
		m_dtTo.SetValue((_variant_t)COleDateTime::GetCurrentTime());

		// (a.walling 2006-11-24 10:07) - Set these all to checked by default.
		CheckDlgButton(IDC_STATUS_OPEN, TRUE);
		CheckDlgButton(IDC_STATUS_FINISHED, TRUE);
		CheckDlgButton(IDC_STATUS_LOCKED, TRUE);

		// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status option
		m_btnStatusOther.SetCheck(TRUE);
		m_dlOtherStatusCombo->Enabled = g_cvarTrue;

		m_bNoStatus = false;

		m_nFilterID = FILTER_ID_ALL;
		m_strFilterString = "";
		m_strWhere = "";

		m_bDateFromDown = false;
		m_bDateToDown = false;

		m_bProcedureAny = true;
		m_bSuppressSeenPatients = GetRemotePropertyInt("EMRSearchSuppressSeenPatients", 1, 0, GetCurrentUserName(), true) == 1;
		CheckDlgButton(IDC_SHOW_SEENPATIENTS, !m_bSuppressSeenPatients);

		// (a.walling 2007-06-15 10:37) - PLID 24859
		m_bShowSeenPatientsWith = GetRemotePropertyInt("EMRSearchShowSeenPatientsWith", 1, 0, GetCurrentUserName(), true) == 1;
		CheckDlgButton(IDC_SHOW_SEENPATIENTS_WITH, m_bShowSeenPatientsWith);

		// (a.walling 2011-01-13 12:00) - PLID 41542
		m_bExcludePending = GetRemotePropertyInt("EMRSearchExcludePendingAppts", 1, 0, GetCurrentUserName(), true) == 1;
		CheckDlgButton(IDC_EXCLUDE_PENDING, m_bExcludePending);		

		m_bRememberColumns = GetRemotePropertyInt("EmrSearchRememberColumns", 0, 0, GetCurrentUserName(), true) != 0;
		CheckDlgButton(IDC_REMEMBERCOLUMNS, m_bRememberColumns);
		if (m_bRememberColumns) {
			RestoreColumns();
		}

		if (m_bAdvanced) { // no point requerying if we don't know it's going to be used yet.
			GetDlgItem(IDC_EMR_SHOW_ADVANCED)->ShowWindow(SW_HIDE);
			ShowAdvanced(true);
		}
		else {
			GetDlgItem(IDC_EMR_SHOW_ADVANCED)->ShowWindow(SW_SHOW);
			ShowAdvanced(false);
		}
		
		// (a.walling 2010-01-11 14:23) - PLID 31482 - Set up this column to pull in the HICON handle
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_dlEMNList->GetColumn(eelcPreview);
		if (pCol != NULL && m_hIconPreview != NULL) {
			CString strHICON;
			strHICON.Format("CASE WHEN MasterQ.EMNID IS NOT NULL THEN %li ELSE NULL END", m_hIconPreview);
			pCol->FieldName = (LPCTSTR)strHICON;
		}

		RequeryLists();
		if (!m_bDialogInitialized)
			SetListDefaults();

		//RefreshList();

		// (j.jones 2006-11-02 11:46) - PLID 23321 - tell the mainfrm we want tablecheckers
		// (z.manning 2013-10-17 09:22) - PLID 59061 - No we don't
		//GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());

		// (j.jones 2014-05-15 16:40) - PLID 61798 - Renamed the existing refresh checkbox to clearly
		// state it only applies to filters, and created a new checkbox for refreshing upon window restore.
		// Also moved this to the end of the function such that nothing refreshes before we finish loading.
		m_bRefreshOnFilterChange = GetRemotePropertyInt("EmrSearchAutoRefresh", 1, 0, GetCurrentUserName(), true) != 0;
		CheckDlgButton(IDC_REFRESH_WHEN_FILTERS_CHANGE, m_bRefreshOnFilterChange);
		m_bRefreshOnWindowRestore = GetRemotePropertyInt("EmrSearchRefreshOnWindowRestore", 1, 0, GetCurrentUserName(), true) != 0;
		CheckDlgButton(IDC_REFRESH_WHEN_WINDOW_RESTORED, m_bRefreshOnWindowRestore);

		// (b.savon 2014-02-05 09:22) - PLID 60646 - On Patients Seen, the Refresh button should always be shown.
		/*if (m_bAutoRefresh) {
		GetDlgItem(IDC_EMRSEARCH_REFRESH)->ShowWindow(SW_HIDE);
		}
		else {
		GetDlgItem(IDC_EMRSEARCH_REFRESH)->ShowWindow(SW_SHOW);
		}*/

		m_bDialogInitialized = true;

	} NxCatchAll("Error in CEMRSearch:OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRSearch::OnDatefilterQuick() 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);

		CMenu mnu;
		CMenu *pSubMenu;
		mnu.LoadMenu(IDR_EMR_POPUP);
		pSubMenu = mnu.GetSubMenu(2);

		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	} NxCatchAll("Error in CEMRSearch:OnDatefilterQuick()");
}

BEGIN_EVENTSINK_MAP(CEMRSearch, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRSearch)
	ON_EVENT(CEMRSearch, IDC_LIST_PROCEDURE, 16 /* SelChosen */, OnSelChosenListProcedure, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_LIST_PROBLEMS, 16 /* SelChosen */, OnSelChosenListProblems, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_LIST_PROVIDER, 16 /* SelChosen */, OnSelChosenListProvider, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_LIST_SEC_PROVIDER, 16 /* SelChosen */, OnSelChosenListSecProvider, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_LIST_TECH, 16 /* SelChosen */, OnSelChosenListTechnician, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_LIST_LOCATION, 16 /* SelChosen */, OnSelChosenListLocation, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_LIST_EMRFILTER, 16 /* SelChosen */, OnSelChosenListEmrfilter, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_DATEFILTER_TYPE, 16 /* SelChosen */, OnSelChosenDatefilterType, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_EMNLIST, 6 /* RButtonDown */, OnRButtonDownEmnlist, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRSearch, IDC_EMNLIST, 19 /* LeftClick */, OnLeftClickEmnlist, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRSearch, IDC_EMNLIST, 18 /* RequeryFinished */, OnRequeryFinishedEmnlist, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CEMRSearch, IDC_LIST_OTHER_STATUS, 16, OnSelChosenListOtherStatus, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_LIST_OTHER_STATUS, 18, OnRequeryFinishedListOtherStatus, VTS_I2)
	ON_EVENT(CEMRSearch, IDC_LIST_APPT_REF_PHYS, 16, SelChosenListApptRefPhys, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_COMBO_CHART, 16, OnSelChosenComboChart, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_COMBO_CATEGORIES, 16, OnSelChosenComboCategories, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_EMR_SEARCH_DIAGNOSIS_SEARCH, 16, CEMRSearch::SelChosenEmrSearchDiagnosisSearch, VTS_DISPATCH)
	ON_EVENT(CEMRSearch, IDC_EMR_SEARCH_DIAGNOSIS_LIST, 6, CEMRSearch::RButtonDownEmrSearchDiagnosisList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CEMRSearch::RequeryLists()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	try {
		pRow = m_dlDateType->GetNewRow();
		pRow->PutValue(egcID, _variant_t((long)dtlEMNDate, VT_I4));
		pRow->PutValue(egcName, "EMN Date");
		m_dlDateType->AddRowSorted(pRow, NULL);

		pRow = m_dlDateType->GetNewRow();
		pRow->PutValue(egcID, _variant_t((long)dtlEMNInputDate, VT_I4));
		pRow->PutValue(egcName, "EMN Input Date");
		m_dlDateType->AddRowSorted(pRow, NULL);

		pRow = m_dlDateType->GetNewRow();
		pRow->PutValue(egcID, _variant_t((long)dtlEMNLastModifiedDate, VT_I4));
		pRow->PutValue(egcName, "EMN Last Modified Date");
		m_dlDateType->AddRowSorted(pRow, NULL);

		pRow = m_dlDateType->GetNewRow();
		pRow->PutValue(egcID, _variant_t((long)dtlPatientSeen, VT_I4));
		pRow->PutValue(egcName, "EMN Date / Patient Seen");
		m_dlDateType->AddRowSorted(pRow, NULL);

		////// -- now add All and Multiple to the other datalists

		m_dlProblems->Requery();
		pRow = m_dlProblems->GetNewRow();
		pRow->PutValue(egcID, (long)lsvAllID);
		pRow->PutValue(egcName, " <All> ");
		m_dlProblems->AddRowSorted(pRow, NULL);

		pRow = m_dlProblems->GetNewRow();
		pRow->PutValue(egcID, (long)lsvMultipleID);
		pRow->PutValue(egcName, "<Multiple> ");
		m_dlProblems->AddRowSorted(pRow, NULL);

		pRow = m_dlProblems->GetNewRow();
		pRow->PutValue(egcID, (long)lsvNoneID);
		pRow->PutValue(egcName, " <None> ");
		m_dlProblems->AddRowSorted(pRow, NULL);

		//////

		m_dlProvider->Requery();
		pRow = m_dlProvider->GetNewRow();
		pRow->PutValue(egcID, (long)lsvAllID);
		pRow->PutValue(egcName, " <All> ");
		m_dlProvider->AddRowSorted(pRow, NULL);

		pRow = m_dlProvider->GetNewRow();
		pRow->PutValue(egcID, (long)lsvMultipleID);
		pRow->PutValue(egcName, "<Multiple> ");
		m_dlProvider->AddRowSorted(pRow, NULL);

		pRow = m_dlProvider->GetNewRow();
		pRow->PutValue(egcID, (long)lsvNoneID);
		pRow->PutValue(egcName, " <None> ");
		m_dlProvider->AddRowSorted(pRow, NULL);

		//////

		// (z.manning 2008-11-12 14:37) - PLID 26167 - Added secondary provider
		m_dlSecProvider->Requery();
		pRow = m_dlSecProvider->GetNewRow();
		pRow->PutValue(egcID, (long)lsvAllID);
		pRow->PutValue(egcName, " <All> ");
		m_dlSecProvider->AddRowSorted(pRow, NULL);

		pRow = m_dlSecProvider->GetNewRow();
		pRow->PutValue(egcID, (long)lsvMultipleID);
		pRow->PutValue(egcName, "<Multiple> ");
		m_dlSecProvider->AddRowSorted(pRow, NULL);

		pRow = m_dlSecProvider->GetNewRow();
		pRow->PutValue(egcID, (long)lsvNoneID);
		pRow->PutValue(egcName, " <None> ");
		m_dlSecProvider->AddRowSorted(pRow, NULL);

		//////
		// (d.lange 2011-04-22 15:01) - PLID 43381 - Added Assistant/Technician dropdown
		m_dlTechnician->Requery();
		pRow = m_dlTechnician->GetNewRow();
		pRow->PutValue(egcID, (long)lsvAllID);
		pRow->PutValue(egcName, " <All> ");
		m_dlTechnician->AddRowSorted(pRow, NULL);

		pRow = m_dlTechnician->GetNewRow();
		pRow->PutValue(egcID, (long)lsvMultipleID);
		pRow->PutValue(egcName, "<Multiple> ");
		m_dlTechnician->AddRowSorted(pRow, NULL);

		pRow = m_dlTechnician->GetNewRow();
		pRow->PutValue(egcID, (long)lsvNoneID);
		pRow->PutValue(egcName, " <None> ");
		m_dlTechnician->AddRowSorted(pRow, NULL);

		//////

		m_dlLocation->Requery();
		pRow = m_dlLocation->GetNewRow();
		pRow->PutValue(egcID, (long)lsvAllID);
		pRow->PutValue(egcName, " <All> ");
		m_dlLocation->AddRowSorted(pRow, NULL);

		pRow = m_dlLocation->GetNewRow();
		pRow->PutValue(egcID, (long)lsvMultipleID);
		pRow->PutValue(egcName, "<Multiple> ");
		m_dlLocation->AddRowSorted(pRow, NULL);

		//////

		m_dlFilter->Requery();
		pRow = m_dlFilter->GetNewRow();
		pRow->PutValue(egcID, _variant_t(long(FILTER_ID_ALL), VT_I4));
		pRow->PutValue(egcName, " <None> ");
		m_dlFilter->AddRowSorted(pRow, NULL);
		
		pRow = m_dlFilter->GetNewRow();
		pRow->PutValue(egcID, _variant_t(long(FILTER_ID_NEW), VT_I4));
		pRow->PutValue(egcName, " <New Filter> ");
		m_dlFilter->AddRowSorted(pRow, NULL);

		//////

		m_dlProcedure->Requery();
		pRow = m_dlProcedure->GetNewRow();
		pRow->PutValue(egcID, (long)lsvAllID);
		pRow->PutValue(egcName, " <All> ");
		m_dlProcedure->AddRowSorted(pRow, NULL);

		pRow = m_dlProcedure->GetNewRow();
		pRow->PutValue(egcID, (long)lsvMultipleID);
		pRow->PutValue(egcName, "<Multiple> ");
		m_dlProcedure->AddRowSorted(pRow, NULL);

		pRow = m_dlProcedure->GetNewRow();
		pRow->PutValue(egcID, (long)lsvNoneID);
		pRow->PutValue(egcName, " <None> ");
		m_dlProcedure->AddRowSorted(pRow, NULL);

		//////

		// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status dropdown,
		// we will add All/Multiple options in OnRequeryFinished
		m_dlOtherStatusCombo->Requery();
		// (b.spivey, January 24, 2013) - PLID 48370 - Add these two regardless -- every other drop down does it, there is little 
		//		benefit not to, other than aesthetics in very limited cases. 
		pRow = m_dlOtherStatusCombo->GetNewRow();
		pRow->PutValue(egcID, (long)lsvAllID);
		pRow->PutValue(egcName, " <All> ");
		m_dlOtherStatusCombo->AddRowSorted(pRow, NULL);

		//do not add a Multiple option if we don't have multiple "other" statuses
		pRow = m_dlOtherStatusCombo->GetNewRow();
		pRow->PutValue(egcID, (long)lsvMultipleID);
		pRow->PutValue(egcName, " <Multiple> ");
		m_dlOtherStatusCombo->AddRowSorted(pRow, NULL);
		
		// (j.dinatale 2013-01-23 12:02) - PLID 54777 - added an appt ref phys dropdown
		m_dlApptRefPhys->Requery();
		pRow = m_dlApptRefPhys->GetNewRow();
		pRow->PutValue(egcID, lsvNoneID);
		pRow->PutValue(egcName, _bstr_t(" <None> "));
		m_dlApptRefPhys->AddRowSorted(pRow, NULL);

		pRow = m_dlApptRefPhys->GetNewRow();
		pRow->PutValue(egcID, lsvMultipleID);
		pRow->PutValue(egcName, _bstr_t(" <Multiple> "));
		m_dlApptRefPhys->AddRowSorted(pRow, NULL);

		pRow = m_dlApptRefPhys->GetNewRow();
		pRow->PutValue(egcID, lsvAllID);
		pRow->PutValue(egcName, _bstr_t(" <All> "));
		m_dlApptRefPhys->AddRowSorted(pRow, NULL);

		// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart & category filters
		m_dlChartCombo->Requery();
		pRow = m_dlChartCombo->GetNewRow();
		pRow->PutValue(egcID, lsvNoneID);
		pRow->PutValue(egcName, _bstr_t(" <None> "));
		m_dlChartCombo->AddRowSorted(pRow, NULL);

		pRow = m_dlChartCombo->GetNewRow();
		pRow->PutValue(egcID, lsvMultipleID);
		pRow->PutValue(egcName, _bstr_t(" <Multiple> "));
		m_dlChartCombo->AddRowSorted(pRow, NULL);

		pRow = m_dlChartCombo->GetNewRow();
		pRow->PutValue(egcID, lsvAllID);
		pRow->PutValue(egcName, _bstr_t(" <All> "));
		m_dlChartCombo->AddRowSorted(pRow, NULL);

		m_dlCategoryCombo->Requery();
		pRow = m_dlCategoryCombo->GetNewRow();
		pRow->PutValue(egcID, lsvNoneID);
		pRow->PutValue(egcName, _bstr_t(" <None> "));
		m_dlCategoryCombo->AddRowSorted(pRow, NULL);

		pRow = m_dlCategoryCombo->GetNewRow();
		pRow->PutValue(egcID, lsvMultipleID);
		pRow->PutValue(egcName, _bstr_t(" <Multiple> "));
		m_dlCategoryCombo->AddRowSorted(pRow, NULL);

		pRow = m_dlCategoryCombo->GetNewRow();
		pRow->PutValue(egcID, lsvAllID);
		pRow->PutValue(egcName, _bstr_t(" <All> "));
		m_dlCategoryCombo->AddRowSorted(pRow, NULL);

	} NxCatchAll("Error in CEMRSearch:RequeryLists");
}

void CEMRSearch::SetListDefaults()
{
	try {
		if (m_bAdvanced) {
			if (!m_bDialogInitialized) {
				// don't set the selection if the dialog has already been initialized,
				// it may already be set to Patient Seen, etc.
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_dlDateType->TrySetSelByColumn_Deprecated(0, _variant_t(long(dtlEMNLastModifiedDate), VT_I4));
				GetDlgItem(IDC_SHOW_SEENPATIENTS)->EnableWindow(FALSE);
				// (a.walling 2011-01-13 12:00) - PLID 41542
				GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->EnableWindow(FALSE);

				/*_variant_t vtNull;

				vtNull.vt = VT_NULL;

				m_dtFrom.SetValue(vtNull);
				m_dtTo.SetValue(vtNull);*/ // (a.walling 2007-02-05 13:54) - PLID 24563 - This is a resource hog loading
				// every EMN in the system when opening. Default to today, as in all EMNs last modified today.
			}

			m_nSelProcedure = -3;
			m_nSelProblems = -3;
			m_nSelProvider = -3;
			m_nSelSecProvider = -3; // (z.manning 2008-11-12 14:51) - PLID 26167
			m_nSelTechnician = -3;	// (d.lange 2011-04-25 09:25) - PLID 43381
			m_nSelLocation = -3;
			m_nFilterID = FILTER_ID_ALL;
			m_nSelOtherStatus = -3;	// (j.jones 2011-07-06 10:27) - PLID 44451
			m_nSelApptRefPhys = lsvAllID;	// (j.dinatale 2013-01-23 15:47) - PLID 54777 - appt ref phys filter
			// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart & category filters
			m_nSelChart = lsvAllID;
			m_nSelCategory = lsvAllID;

			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_dlProcedure->TrySetSelByColumn_Deprecated(0, _variant_t(long(-3), VT_I4));
			m_dlProblems->TrySetSelByColumn_Deprecated(0, _variant_t(long(-3), VT_I4));
			m_dlProvider->TrySetSelByColumn_Deprecated(0, _variant_t(long(-3), VT_I4)); 
			m_dlSecProvider->TrySetSelByColumn_Deprecated(0, _variant_t(long(-3), VT_I4)); // (z.manning 2008-11-12 14:38) - PLID 26167
			m_dlTechnician->TrySetSelByColumn_Deprecated(0, _variant_t(long(-3), VT_I4));	// (d.lange 2011-04-25 09:25) - PLID 43381
			m_dlLocation->TrySetSelByColumn_Deprecated(0, _variant_t(long(-3), VT_I4));
			m_dlFilter->TrySetSelByColumn_Deprecated(0, _variant_t(long(FILTER_ID_ALL), VT_I4));
			// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status dropdown
			m_dlOtherStatusCombo->TrySetSelByColumn_Deprecated(0, _variant_t(long(-3), VT_I4));
			m_dlApptRefPhys->TrySetSelByColumn_Deprecated(egcID, _variant_t(long(lsvAllID), VT_I4));	// (j.dinatale 2013-01-23 15:47) - PLID 54777 - appt ref phys filter

			// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart & category filters
			m_dlChartCombo->TrySetSelByColumn_Deprecated(egcID, _variant_t(long(lsvAllID), VT_I4));
			m_dlCategoryCombo->TrySetSelByColumn_Deprecated(egcID, _variant_t(long(lsvAllID), VT_I4));

			GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->EnableWindow(FALSE);
		}
		else { // patients seen today! so set the date type
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_dlDateType->TrySetSelByColumn_Deprecated(0, _variant_t(long(dtlPatientSeen), VT_I4));
			GetDlgItem(IDC_SHOW_SEENPATIENTS)->EnableWindow(TRUE);
			// (a.walling 2011-01-13 12:00) - PLID 41542
			GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->EnableWindow(TRUE);
			GetDlgItem(IDC_SHOW_SEENPATIENTS)->RedrawWindow();
			GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->RedrawWindow();
			GetDlgItem(IDC_EXCLUDE_PENDING)->RedrawWindow();
		}
		// (b.spivey, January 23, 2013) - PLID 48370 - Load filters after requery.
		LoadPreviousFilters();
		// (b.spivey, January 30, 2013) - PLID 48370 - Enable/disable checkboxes accordingly.
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_dlDateType->GetCurSel());
		if (pRow) {
			if(VarLong(pRow->GetValue(0), -1) == (long)dtlPatientSeen) {
				GetDlgItem(IDC_SHOW_SEENPATIENTS)->EnableWindow(TRUE);
				// (a.walling 2011-01-13 12:00) - PLID 41542
				GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->EnableWindow(TRUE);
				GetDlgItem(IDC_SHOW_SEENPATIENTS)->RedrawWindow();
				GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->RedrawWindow();
				GetDlgItem(IDC_EXCLUDE_PENDING)->RedrawWindow();
			}
		}
		GetDlgItem(IDC_EMNLIST)->SetFocus();
	} NxCatchAll("Error in CEMRSearch:SetListDefaults");
}

CString CEMRSearch::GenerateFromClause()
{
	// (z.manning, 02/29/2008) - PLID 29158 - Note: if you make any changes to this, you will likely have
	// to make the same change to the query for report 581 in ReportInfoPreview.
	// (j.jones 2009-12-29 10:35) - PLID 35795 - ensure the -25 patient is filtered out
	// (j.jones 2011-07-07 09:52) - PLID 44451 - supported EMRStatusListT
	// (j.jones 2013-07-15 16:51) - PLID 57477 - added tab categories
	// (b.eyers 2015-05-19) - PLID 49999 - emns with only unbillable charges should show as <no charges> 
	return FormatString("EMRGroupsT \r\n"
		"LEFT JOIN EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID \r\n"
		"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID \r\n"
		"LEFT JOIN EmnTabCategoriesLinkT ON EmrMasterT.ID = EmnTabCategoriesLinkT.EmnID \r\n"
		"LEFT JOIN PicT ON PicT.EMRGroupID = EMRGroupsT.ID \r\n"
		"LEFT JOIN PersonT PatientQ ON EMRMasterT.PatientID = PatientQ.ID \r\n"
		"LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		"LEFT JOIN AppointmentsT ON AppointmentsT.ID = ( \r\n"
		"	SELECT TOP 1 ID FROM AppointmentsT \r\n"
		"	WHERE AppointmentsT.Date = EmrMasterT.Date AND AppointmentsT.PatientID = EmrMasterT.PatientID \r\n"
		"	AND Status <> 4 AND ShowState <> 3 %s\r\n" // (a.walling 2010-12-29 10:14) - PLID 41542 - Also exclude pending appointments
		"	AND AppointmentsT.PatientID <> -25 \r\n"
		"	) \r\n"
		"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
		"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
		"LEFT JOIN ( "
		"	SELECT EmrChargesT.EmrID FROM EmrChargesT "
		"	LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"	WHERE EmrChargesT.Deleted = 0 AND CPTCodeT.Billable = 1 "
		"	GROUP BY EmrChargesT.EmrID "
		"	) AS BillableCPTQ ON EMRMasterT.ID = BillableCPTQ.EmrID "
		"LEFT JOIN ( "
		"	SELECT EmrChargesT.EmrID, ProductLocationInfoT.LocationID FROM EmrChargesT "
		"	LEFT JOIN ProductLocationInfoT ON EmrChargesT.ServiceID = ProductLocationInfoT.ProductID "
		"	WHERE EmrChargesT.Deleted = 0 AND ProductLocationInfoT.Billable = 1 "
		// (j.gruber 2016-01-22 12:08) - PLID 68027 - EMR Search is showing EMN's multiple times - add group by
		"   GROUP BY EMRChargesT.EMRID, ProductLocationInfoT.LocationID "
		"	) AS BillableProductQ ON EMRMasterT.ID = BillableProductQ.EmrID AND BillableProductQ.LocationID = EmrMasterT.LocationID "
		, m_checkExcludePending.GetCheck() == BST_CHECKED ? " AND ShowState <> 0" : "" // (a.walling 2011-01-13 10:24) - PLID 41542
		);
}

// (z.manning 2008-06-12 15:59) - PLID 29380 - Added optional bSkipSeenWithCheck parameter
// (j.gruber 2013-06-10 11:19) - PLID  43442- date filter only
CString CEMRSearch::GenerateWhereClause(OPTIONAL bool bSkipSeenWithCheck /* = false */, OPTIONAL bool bDateFilterOnly /* = false*/)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	CStringArray straryWhere;
	long nID;
	CString strSql;

	m_bNoStatus = false; // this variable is set to true only if no status checkbox is selected, which effectively
							// means 'WHERE FALSE'

	if (m_bAdvanced && (!bDateFilterOnly)) {
		try {
			/////
			CString strProcedure;
			pRow = m_dlProcedure->GetCurSel();
			if (pRow) {
				nID = VarLong(pRow->GetValue(egcID), -3);

				if (nID > -2) {
					if (m_bProcedureAny || (nID > -1) || m_vaProcedure.GetSize() == 1) {
						strProcedure.Format("EMRMasterT.ID IN (SELECT EMRID FROM EmrProcedureT WHERE ProcedureID IN (%s) AND Deleted = 0)", VariantArrayToString(m_vaProcedure));
					}
					else { // ALL of the selected procedures!
						//select * from emrmastert where exists (select count(emrid) from emrproceduret WHERE emrid = emrmastert.id and procedureid in (1187, 1121, 1001, 1180) group by emrid having count(emrid) = 4)
						strProcedure.Format("EXISTS (SELECT COUNT(EMRID) FROM EMRProcedureT WHERE EMRID = EMRMasterT.ID AND ProcedureID IN (%s) GROUP BY EMRID HAVING Count(EMRID) = %li)", VariantArrayToString(m_vaProcedure), (long)m_vaProcedure.GetSize());
					}
				} else if (nID == -2) {
					// none
					strProcedure.Format("NOT EXISTS (SELECT EMRID FROM EMRProcedureT WHERE EMRID = EMRMasterT.ID AND Deleted = 0)");
				}
			}
			straryWhere.Add(strProcedure);
		} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(procedure)");

		try
		{
			// (a.wilson 2014-03-04 16:06) - PLID 60780 - handle icd10 codes as well as new control setup.
			CString strDiagnosis;
			//if set for none
			if (m_btnDiagnosisIncludeNone.GetCheck() == BST_CHECKED) {
				strDiagnosis.Format("NOT EXISTS (SELECT EMRID FROM EMRDiagCodesT WHERE EMRID = EMRMasterT.ID AND Deleted = 0)");
			}
			//if set for specific
			else if (m_vaDiagnosis.GetCount() > 0) {
				strDiagnosis.Format("EMRMasterT.ID IN (SELECT EMRID FROM EMRDiagCodesT WHERE "
					"(DiagCodeID IN (%s) OR DiagCodeID_ICD10 IN (%s)) "
					"AND Deleted = 0)", VariantArrayToString(m_vaDiagnosis), VariantArrayToString(m_vaDiagnosis));
			}
			//ignore if all since we just want all emns.
			//add to where clause.
			straryWhere.Add(strDiagnosis);
		} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(diagnosis)");

		try {
			/////
			CString strProblem;
			pRow = m_dlProblems->GetCurSel();
			if (pRow) {
				nID = VarLong(pRow->GetValue(egcID), -3);
				// (c.haag 2006-10-19 12:06) - PLID 21454 - Problems now have a deleted flag
				// (j.jones 2008-07-15 17:21) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
				// (c.haag 2009-05-13 11:22) - PLID 34234 - Use the new EMR problem linking tables
				if (nID > -2) {
					strProblem.Format("("
						"(EMRMasterT.ID IN (SELECT EMRID FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRDetailsT ON EMRProblemLinkT.EMRRegardingID = EMRDetailsT.ID "
						"WHERE EMRProblemsT.StatusID IN (%s) AND EMRDetailsT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND (EMRProblemLinkT.EMRRegardingType = %li OR EMRProblemLinkT.EMRRegardingType = %li))) "
						""
						"OR (EMRMasterT.ID IN (SELECT EMRID FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRTopicsT ON EMRProblemLinkT.EMRRegardingID = EMRTopicsT.ID "
						"WHERE EMRProblemsT.StatusID IN (%s) AND EMRTopicsT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li)) "
						""
						"OR (EMRMasterT.ID IN (SELECT EMRRegardingID FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"WHERE EMRProblemsT.StatusID IN (%s) "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li)) "
						""
						"OR (EMRMasterT.ID IN (SELECT EMRID FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRDiagCodesT ON EMRProblemLinkT.EMRRegardingID = EMRDiagCodesT.ID "
						"WHERE EMRProblemsT.StatusID IN (%s) AND EMRDiagCodesT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li)) "
						""
						"OR (EMRMasterT.ID IN (SELECT EMRID FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRChargesT ON EMRProblemLinkT.EMRRegardingID = EMRChargesT.ID "
						"WHERE EMRProblemsT.StatusID IN (%s) AND EMRChargesT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li)) "
						""
						"OR (EMRMasterT.ID IN (SELECT EMRID FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRMedicationsT ON EMRProblemLinkT.EMRRegardingID = EMRMedicationsT.MedicationID "
						"WHERE EMRProblemsT.StatusID IN (%s) AND EMRMedicationsT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li)) "
						")",
						VariantArrayToString(m_vaProblems), eprtEmrItem, eprtEmrDataItem,
						VariantArrayToString(m_vaProblems), eprtEmrTopic,
						VariantArrayToString(m_vaProblems), eprtEmrEMN,
						VariantArrayToString(m_vaProblems), eprtEmrDiag,
						VariantArrayToString(m_vaProblems), eprtEmrCharge,
						VariantArrayToString(m_vaProblems), eprtEmrMedication);
				}
				else if (nID == -2) {
					// none
					strProblem.Format("NOT EXISTS ("
						"SELECT EMRID "
						"FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRDetailsT ON EMRProblemLinkT.EMRRegardingID = EMRDetailsT.ID "
						"WHERE EMRDetailsT.EMRID = EMRMasterT.ID AND EMRDetailsT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND (EMRProblemLinkT.EMRRegardingType = %li OR EMRProblemLinkT.EMRRegardingType = %li)) "
						""
						"AND NOT EXISTS ("
						"SELECT EMRID "
						"FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRTopicsT ON EMRProblemLinkT.EMRRegardingID = EMRTopicsT.ID "
						"WHERE EMRTopicsT.EMRID = EMRMasterT.ID AND EMRTopicsT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li) "
						""
						"AND NOT EXISTS ("
						"SELECT EMRMasterQ.ID "
						"FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRMasterT EMRMasterQ ON EMRProblemLinkT.EMRRegardingID = EMRMasterQ.ID "
						"WHERE EMRMasterQ.ID = EMRMasterT.ID AND EMRMasterQ.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li) "
						""
						"AND NOT EXISTS ("
						"SELECT EMRID "
						"FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRDiagCodesT ON EMRProblemLinkT.EMRRegardingID = EMRDiagCodesT.ID "
						"WHERE EMRDiagCodesT.EMRID = EMRMasterT.ID AND EMRDiagCodesT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li) "						
						""
						"AND NOT EXISTS ("
						"SELECT EMRID "
						"FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRChargesT ON EMRProblemLinkT.EMRRegardingID = EMRChargesT.ID "
						"WHERE EMRChargesT.EMRID = EMRMasterT.ID AND EMRChargesT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li) "
						""
						"AND NOT EXISTS ("
						"SELECT EMRID "
						"FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"LEFT JOIN EMRMedicationsT ON EMRProblemLinkT.EMRRegardingID = EMRMedicationsT.MedicationID "
						"WHERE EMRMedicationsT.EMRID = EMRMasterT.ID AND EMRMedicationsT.Deleted = 0 "
						"AND EMRProblemsT.Deleted = 0 AND EMRProblemLinkT.EMRRegardingType = %li) ",
						eprtEmrItem, eprtEmrDataItem,
						eprtEmrTopic,
						eprtEmrEMN,
						eprtEmrDiag,
						eprtEmrCharge,
						eprtEmrMedication);
				}
			}
			straryWhere.Add(strProblem);
		} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(problem)");

		try {
			/////
			CString strProvider;
			pRow = m_dlProvider->GetCurSel();
			if (pRow) {
				nID = VarLong(pRow->GetValue(egcID), -3);
				if (nID > -2) {
					strProvider.Format("EMRMasterT.ID IN (SELECT EmrID FROM EmrProvidersT WHERE ProviderID IN (%s) AND Deleted = 0)", VariantArrayToString(m_vaProvider));
				} else if (nID == -2) {
					// none
					strProvider.Format("EMRMasterT.ID NOT IN (SELECT EmrID FROM EmrProvidersT WHERE Deleted = 0)");
				}
			}
			straryWhere.Add(strProvider);
		} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(provider)");

		try
		{
			// (z.manning 2008-11-12 14:39) - PLID 26167 - Added secondary provider
			CString strSecProvider;
			pRow = m_dlSecProvider->GetCurSel();
			if (pRow != NULL) {
				nID = VarLong(pRow->GetValue(egcID), -3);
				if (nID > -2) {
					strSecProvider.Format("EMRMasterT.ID IN (SELECT EmrID FROM EmrSecondaryProvidersT WHERE ProviderID IN (%s) AND Deleted = 0)", VariantArrayToString(m_vaSecProvider));
				}
				else if (nID == -2) {
					// none
					strSecProvider.Format("EMRMasterT.ID NOT IN (SELECT EmrID FROM EmrSecondaryProvidersT WHERE Deleted = 0)");
				}
			}
			straryWhere.Add(strSecProvider);
		} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(provider)");

		try {
			// (d.lange 2011-04-25 09:31) - PLID 43381 - Added Assistant/Technicians
			CString strTechnician;
			pRow = m_dlTechnician->GetCurSel();
			if(pRow) {
				nID = VarLong(pRow->GetValue(egcID), -3);
				if(nID > -2) {
					strTechnician.Format("EMRMasterT.ID IN (SELECT EmrID FROM EmrTechniciansT WHERE PersonID IN (%s) AND Deleted = 0)", VariantArrayToString(m_vaTechnician));
				}else if(nID == -2) {
					strTechnician.Format("EMRMasterT.ID NOT IN (SELECT EmrID FROM EmrTechniciansT WHERE Deleted = 0)");
				}
			}
			straryWhere.Add(strTechnician);
		} NxCatchAll("Error in CEMRSearch::GenerateWhereClause(technicians)");

		try {
			/////
			CString strLocation;
			pRow = m_dlLocation->GetCurSel();	
			if (pRow) {
				nID = VarLong(pRow->GetValue(egcID), -3);
				if (nID > -2) {
					strLocation.Format("EMRMasterT.LocationID IN (%s)", VariantArrayToString(m_vaLocation));
				}
			}
			straryWhere.Add(strLocation);
		} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(location)");

		try {

			// (j.jones 2011-07-07 09:18) - PLID 44451 - supported "Other" statuses
			CString strIn1;

			if(IsDlgButtonChecked(IDC_STATUS_OPEN)) {
				strIn1 += "0,";
			}
			if(IsDlgButtonChecked(IDC_STATUS_FINISHED)) {
				strIn1 += "1,";
			}
			if(IsDlgButtonChecked(IDC_STATUS_LOCKED)) {
				strIn1 += "2,";
			}

			strIn1.TrimRight(",");

			CString strWhere1;
			if(!strIn1.IsEmpty()) {
				strWhere1.Format("EMRMasterT.Status IN (%s)", strIn1);
			}

			CString strWhere2;
			if(m_btnStatusOther.GetCheck()) {
				pRow = m_dlOtherStatusCombo->GetCurSel();	
				if (pRow) {
					nID = VarLong(pRow->GetValue(egcID), lsvAllID);
					if(nID == lsvAllID) {
						strWhere2.Format("EMRMasterT.Status IN (SELECT ID FROM EMRStatusListT WHERE ID NOT IN (0,1,2))");
					}
					else if(nID > lsvNoneID) {
						strWhere2.Format("EMRMasterT.Status IN (%s)", VariantArrayToString(m_vaOtherStatus));
					}
				}
			}

			CString strWhere;
			if(strWhere1.IsEmpty() && strWhere2.IsEmpty()) {
				//with no statuses selected, we can't filter on anything at all
				strWhere = "1 = 0";
				m_bNoStatus = true;
			}
			else if(!strWhere1.IsEmpty() && strWhere2.IsEmpty()) {
				strWhere = strWhere1;
			}
			else if(strWhere1.IsEmpty() && !strWhere2.IsEmpty()) {
				strWhere = strWhere2;
			}
			else if(!strWhere1.IsEmpty() && !strWhere2.IsEmpty()) {
				strWhere.Format("(%s OR %s)", strWhere1, strWhere2);
			}
			straryWhere.Add(strWhere);
		} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(status)");

		// (j.dinatale 2013-01-23 15:47) - PLID 54777 - appt ref phys filter
		try{
			pRow = m_dlApptRefPhys->GetCurSel();

			if(pRow){
				long nRefPhysID = VarLong(pRow->GetValue(egcID), lsvAllID);

				if(nRefPhysID != lsvAllID){
					CString strWhere;
					if(nRefPhysID == lsvNoneID){
						strWhere = "(EMRMasterT.AppointmentID IS NULL OR EMRMasterT.AppointmentID IN (SELECT ID FROM AppointmentsT WHERE RefPhysID IS NULL))";
					}else{
						strWhere.Format("EMRMasterT.AppointmentID IN (SELECT ID FROM AppointmentsT WHERE RefPhysID IN (%s))", VariantArrayToString(m_vaApptRefPhys));
					}

					straryWhere.Add(strWhere);
				}
			}
		}NxCatchAll("Error in CEMRSearch:GenerateWhereClause(appt ref phys)");

		// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart filter
		try {
			pRow = m_dlChartCombo->GetCurSel();

			if(pRow){
				long nChartID = VarLong(pRow->GetValue(egcID), lsvAllID);

				if(nChartID != lsvAllID){
					CString strWhere;
					//you can't link EMNs to multiple charts, we've already left joined EmnTabChartsLinkT
					if(nChartID == lsvNoneID){
						strWhere = "EmnTabChartsLinkT.EMNTabChartID Is Null";
					}else{
						strWhere.Format("EmnTabChartsLinkT.EMNTabChartID IN (%s)", VariantArrayToString(m_vaChart));
					}

					straryWhere.Add(strWhere);
				}
			}
		}NxCatchAll("Error in CEMRSearch:GenerateWhereClause (chart)");

		// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart filter
		try {
			pRow = m_dlCategoryCombo->GetCurSel();

			if(pRow){
				long nCategoryID = VarLong(pRow->GetValue(egcID), lsvAllID);

				if(nCategoryID != lsvAllID){
					CString strWhere;
					//you can't link EMNs to multiple categories, we've already left joined EmnTabCategoriesLinkT
					if(nCategoryID == lsvNoneID) {
						strWhere = "EmnTabCategoriesLinkT.EMNTabCategoryID Is Null";
					}else{
						strWhere.Format("EmnTabCategoriesLinkT.EMNTabCategoryID IN (%s)", VariantArrayToString(m_vaCategory));
					}

					straryWhere.Add(strWhere);
				}
			}
		}NxCatchAll("Error in CEMRSearch:GenerateWhereClause (category)");

		try {
			/////
			CString strFilter;
			if (m_nFilterID == FILTER_ID_ALL) {
				m_strFilterString = "";
				strFilter = m_strFilterString;
			}
			else {
				CString strFilterFrom, strFilterWhere;
				bool bResult = CFilter::ConvertFilterStringToClause(m_nFilterID, m_strFilterString, fboPerson, &strFilterWhere, &strFilterFrom, NULL, NULL, TRUE);
				if (bResult) {
					strFilter.Format("EMRMasterT.PatientID IN (SELECT PersonT.ID FROM %s WHERE %s)", strFilterFrom, strFilterWhere);
				}
				else {	
					ASSERT(FALSE);
				}
			}
			straryWhere.Add(strFilter);
		} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(filter)");
	}

	///// Date filter (always run)
	try {
	
		pRow = m_dlDateType->GetCurSel();
		if (pRow == NULL) {
			m_dlDateType->SetSelByColumn(0, _variant_t(long(dtlEMNLastModifiedDate), VT_I4));
			pRow = m_dlDateType->GetCurSel();
		}

		long nDateType;
		if (pRow == NULL) {
			nDateType = dtlEMNLastModifiedDate;
		}
		else
		{
			nDateType = VarLong(pRow->GetValue(egcID), dtlEMNLastModifiedDate);
		}

		// (a.walling 2007-06-15 10:42) - PLID 24859 - If we do not want to see patients with EMNs, then we don't need to return any rows at all.
		// (z.manning 2008-06-12 16:54) - PLID 29380 - Removed m_bAdvanced check as this is now visible in
		// advanced as well.  Then added bSkipSeenWithCheck.
		if (!bSkipSeenWithCheck && !IsDlgButtonChecked(IDC_SHOW_SEENPATIENTS_WITH)) {
			straryWhere.Add("1=0");
		}
			
		///// Show seen patients
		// (a.walling 2007-02-05 13:59) - PLID 24563 - Option to include patients seen w/o EMNs
		if (nDateType == dtlPatientSeen) {
			if (IsDlgButtonChecked(IDC_SHOW_SEENPATIENTS)) {
				straryWhere.Add("1=1"); // just to force the data to be different
			}
		}

		CString strDateFilter = GenerateDateRange("EmrMasterT.Date"); // get the range from our function
		if (!strDateFilter.IsEmpty()) {
			if (nDateType == dtlPatientSeen) {// patient seen!
				CString strDateClause = strDateFilter; // filter EMNs on this date as well.
				strDateFilter.Format("%s AND AppointmentsT.ID IS NOT NULL", strDateClause);
			}
			straryWhere.Add(strDateFilter);
		}

	} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(date)");

	// (z.manning 2009-05-19 14:14) - PLID 28512 - Added option to show only current patient
	// (b.spivey, January 30, 2013) - PLID 48370 - Use a member bool now. 
	if(m_bShowCurrentPatientOnly && (!bDateFilterOnly)) {
		straryWhere.Add(FormatString("EmrMasterT.PatientID = %li", GetActivePatientID()));
	}

	// (z.manning 2011-05-20 11:34) - PLID 33114 - Filter on the current user's EMR chart permissions
	CSqlFragment sqlfragmentChartFilter = GetEmrChartPermissionFilter(FALSE);
	if(!sqlfragmentChartFilter.IsEmpty() && (!bDateFilterOnly)) {
		straryWhere.Add(sqlfragmentChartFilter.Flatten());
	}

	try {
		///////////////////////////////// assimilate the individual clauses

		strSql = "(EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND (PicT.IsCommitted = 1 OR PicT.IsCommitted IS NULL) AND PicT.ID IS NOT NULL) AND ";
		for (int i = 0; i < straryWhere.GetSize(); i++) {
			CString strPart = straryWhere.GetAt(i);
			if (!strPart.IsEmpty()) {
				strSql += "(" + strPart + ")";
				strSql += " AND ";
			}
		}

		strSql = strSql.Left(strSql.GetLength() - 5);

	} NxCatchAll("Error in CEMRSearch:GenerateWhereClause(assimilate)");
	
	return strSql;
}

CString CEMRSearch::GenerateDateRange()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	pRow = m_dlDateType->GetCurSel();
	if (pRow == NULL) {
		m_dlDateType->SetSelByColumn(0, _variant_t(long(dtlEMNLastModifiedDate), VT_I4));
		pRow = m_dlDateType->GetCurSel();
	}

	long nDateType;
	if (pRow == NULL) {
		nDateType = dtlEMNLastModifiedDate;
	}
	else
	{
		nDateType = VarLong(pRow->GetValue(egcID), dtlEMNLastModifiedDate);
	}

	CString strField;
	switch(nDateType) {
		case dtlEMNDate:
			strField = "EMRMasterT.Date";
			break;
		case dtlEMNInputDate:
			strField = "EMRMasterT.InputDate";
			break;
		case dtlEMNLastModifiedDate:
			strField = "EMRMasterT.ModifiedDate";
			break;
		case dtlPatientSeen:
			strField = "Date";
			break;
		default:
			ASSERT(FALSE);
			break;
	}

	return GenerateDateRange(strField);
}

CString CEMRSearch::GenerateDateRange(IN const CString strSqlFieldName)
{
	// (a.walling 2007-02-05 09:39) - PLID 24563
	// return a SQL-ready range of dates
	try {
		CString strDateFilter;
		CString strFrom, strTo;
		CString strFromFilter, strToFilter;

		// from January 1, 1753 to December 30, 9999
		COleDateTime dtMin, dtMax;
		dtMin.SetDate(1753, 1, 1);
		dtMax.SetDate(9999, 12, 30);

		_variant_t vardtFrom = m_dtFrom.GetValue();
		_variant_t vardtTo = m_dtTo.GetValue();

		// swap them if from > to
		if ( (vardtFrom.vt != VT_NULL) && (vardtTo.vt != VT_NULL) ) {
			if (VarDateTime(vardtFrom) > VarDateTime(vardtTo)) {
				_variant_t vardtTemp;
				vardtTemp = vardtTo;
				vardtTo = vardtFrom;
				vardtFrom = vardtTemp;
				m_dtFrom.SetValue(vardtFrom);
				m_dtTo.SetValue(vardtTo);
			}
		}

		if (vardtFrom.vt != VT_NULL) {
			COleDateTime dtFrom = VarDateTime(vardtFrom);
			if (dtFrom > dtMax)
			{
				dtFrom = dtMax;
				m_dtFrom.SetValue(_variant_t(dtFrom));
			}
			else if (dtFrom < dtMin)
			{
				dtFrom = dtMin;
				m_dtFrom.SetValue(_variant_t(dtFrom));
			}
			strFrom = FormatDateTimeForSql(dtFrom, dtoDate);
		}
		if (vardtTo.vt != VT_NULL) {
			COleDateTime dtTo = VarDateTime(vardtTo);
			if (dtTo > dtMax)
			{
				dtTo = dtMax;
				m_dtTo.SetValue(_variant_t(dtTo));
			}
			else if (dtTo < dtMin)
			{
				dtTo = dtMin;
				m_dtTo.SetValue(_variant_t(dtTo));
			}
			COleDateTimeSpan dtsOneDay;
			dtsOneDay.SetDateTimeSpan(1, 0, 0, 0);
			dtTo += dtsOneDay;

			strTo = FormatDateTimeForSql(dtTo, dtoDate);
		}

		if ( (!strTo.IsEmpty()) || (!strFrom.IsEmpty())) {

			if (!strFrom.IsEmpty()) {
				strFromFilter.Format("(%s >= '%s')", strSqlFieldName, strFrom);
			}
			if (!strTo.IsEmpty()) {
				strToFilter.Format("(%s < '%s')", strSqlFieldName, strTo);
			}
			
			if (strTo.IsEmpty()) {
				strDateFilter = strFromFilter;
			}
			if (strFrom.IsEmpty()) {
				strDateFilter = strToFilter;
			}

			if (strDateFilter.IsEmpty()) {
				strDateFilter = strFromFilter + " AND " + strToFilter;
			}
		} else {
			strDateFilter.Empty();
		}

		return strDateFilter;
	} NxCatchAll("Error in GenerateDateRange()");

	return "";
}

void CEMRSearch::OnClose() 
{
	SaveColumns();
	CDialog::OnOK();

	// (j.jones 2006-11-02 11:47) - PLID 23321 - tell mainframe that we don't need tablecheckers anymore
	// (z.manning 2013-10-17 09:23) - PLID 59061 - We don't get them at all anymore
	//GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());

	DestroyWindow();
	m_bDialogInitialized = false;
}

CString CEMRSearch::VariantArrayToString(CVariantArray &va)
{
	CString strRet;

	try {
		if (va.GetSize() == 0) {
			ASSERT(FALSE);
			return "";
		}
		for (int i = 0; i < va.GetSize(); i++) {
			_variant_t var = va[i];

			strRet += AsString(var);
			strRet += ", ";
		}
		strRet.TrimRight(", ");
	} NxCatchAll("Error in CEMRSearch:VariantArrayToString");
	return strRet;
}


bool CEMRSearch::OpenMultiSelectList(NXDATALIST2Lib::_DNxDataListPtr &list, IN OUT CVariantArray &va, IN OUT long &nCurSel, const IN CString &strFieldID, const IN CString &strFieldValue, const IN CString &strDescription, IN CStringArray *straryExtraColumnFields /*= NULL*/, IN CStringArray *straryExtraColumnNames /* = NULL*/)
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, VarString(list->GetFromClause()));

		CString strFrom;
		strFrom.Format("%s", (LPCTSTR)list->GetFromClause());
		CString strWhere;
		strWhere.Format("%s", (LPCTSTR)list->GetWhereClause());

		dlg.PreSelect(va);
		long nResult = dlg.Open(strFrom, strWhere, strFieldID, strFieldValue, strDescription, 0, 0xFFFFFFFF, straryExtraColumnFields, straryExtraColumnNames);

		if (nResult == IDOK) {
			dlg.FillArrayWithIDs(va);
			if (va.GetSize() == 1) {
				list->SetSelByColumn(egcID, va.GetAt(0));
				nCurSel = VarLong(va.GetAt(0));
			} else if (va.GetSize() == 0) {
				list->SetSelByColumn(egcID, _variant_t(long(-2), VT_I4)); // set to none
				nCurSel = -2;
			} else if (va.GetSize() == list->GetRowCount()) {
				list->SetSelByColumn(egcID, _variant_t(long(-3), VT_I4)); // set to all
				nCurSel = -3;
				va.RemoveAll();
			} else {
				nCurSel = -1; // multiple
			}
		} else {
			list->SetSelByColumn(egcID, nCurSel);
			return false;
		}
		
	} NxCatchAll("Error in CEMRSearch:OpenMultiSelectList");
	return true;
}

void CEMRSearch::OnSelChosenListProcedure(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(egcID)) == -1) // multiple
			{
				// the OpenMultiSelect function will set m_nSel variable to the appropriate value.
				bool bResult = OpenMultiSelectList(m_dlProcedure, m_vaProcedure, m_nSelProcedure, "ID", "Name", "Procedures");
					
				if (bResult) { // user did not cancel out of the multi select
					if (m_vaProcedure.GetSize() > 1) {
						// ask whether this should be inclusive or exclusive
						CString strMsg;

						// (b.spivey, January 30, 2013) - PLID 48370 - Let them know this will save. 
						strMsg = "Would you like the filter to only include patients with ALL of these procedures on their EMN "
							"(This selection will be saved)? \n\n"
							"Yes\t- I would only like to see patients with ALL of these procedures.\n"
							"No\t- I would like to see patients with ANY of these procedures.\n";

						long nResult = MessageBox(strMsg, NULL, MB_ICONQUESTION|MB_YESNO);

						if (nResult == IDYES) {
							m_bProcedureAny = false;
						}
						else {
							m_bProcedureAny = true;
						}
						// (b.spivey, January 29, 2013) - PLID  - 48370 - Store the any or all selection. 
						SetRemotePropertyInt("EMRSearchAllOrAnyProcedureFilter", (m_bProcedureAny ? 1 : 0), 0, GetCurrentUserName()); 
						// (b.spivey, January 23, 2013) - PLID  - 48370 - Store procedure ids and filter selection. 
						SetRemotePropertyText("EMRSearchProcedureIDs", VariantArrayToString(m_vaProcedure), 0, GetCurrentUserName()); 
					} else { 
						m_bProcedureAny = true;
					}
				}
			} else {
				m_nSelProcedure = VarLong(pRow->GetValue(egcID));
				m_vaProcedure.RemoveAll();
				m_vaProcedure.Add((_variant_t)m_nSelProcedure);
			}
		} else {
			m_dlProcedure->SetSelByColumn(egcID, m_nSelProcedure);
		}
		SetRemotePropertyInt("EMRSearchProcedureFilter", m_nSelProcedure, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

void CEMRSearch::OnSelChosenListProblems(LPDISPATCH lpRow) 
{
		try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(egcID)) == -1) // multiple
			{
				bool bResult = OpenMultiSelectList(m_dlProblems, m_vaProblems, m_nSelProblems, "ID", "Name", "Problems");   
				// (b.spivey, January 23, 2013) - PLID 48370 - Store problems ids/selection
				if (bResult && m_vaProblems.GetSize() > 0) {
					SetRemotePropertyText("EMRSearchProblemsIDs", VariantArrayToString(m_vaProblems), 0, GetCurrentUserName()); 
				}

			} else {
				m_nSelProblems = VarLong(pRow->GetValue(egcID));
				m_vaProblems.RemoveAll();
				m_vaProblems.Add((_variant_t)m_nSelProblems);
			}
		} else {
			m_dlProblems->SetSelByColumn(egcID, m_nSelProblems);
		}
		SetRemotePropertyInt("EMRSearchProblemsFilter", m_nSelProblems, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

void CEMRSearch::OnSelChosenListProvider(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(egcID)) == -1) // multiple
			{
				bool bResult = OpenMultiSelectList(m_dlProvider, m_vaProvider, m_nSelProvider, "ID", "Last + ', ' + First + ' ' + Middle", "Providers");
				// (b.spivey, January 23, 2013) - PLID 48370 - Store providers ids/selection
				if (bResult && m_vaProvider.GetSize() > 0) {
					SetRemotePropertyText("EMRSearchProviderIDs", VariantArrayToString(m_vaProvider), 0, GetCurrentUserName()); 
				}
			} else {
				m_nSelProvider = VarLong(pRow->GetValue(egcID));
				m_vaProvider.RemoveAll();
				m_vaProvider.Add((_variant_t)m_nSelProvider);
			}
		} else {
			m_dlProvider->SetSelByColumn(egcID, m_nSelProvider);
		}
		SetRemotePropertyInt("EMRSearchProviderFilter", m_nSelProvider, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

// (d.lange 2011-04-22 14:48) - PLID 43381
void CEMRSearch::OnSelChosenListTechnician(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			if(VarLong(pRow->GetValue(egcID)) == -1) {
				bool bResult = OpenMultiSelectList(m_dlTechnician, m_vaTechnician, m_nSelTechnician, "ID", "Last + ', ' + First + ' ' + Middle", "Assistant/Technician");
				// (b.spivey, January 23, 2013) - PLID 48370 - Store technician ids/selection
				if (bResult && m_vaTechnician.GetSize() > 0) {
					SetRemotePropertyText("EMRSearchAssistTechIDs", VariantArrayToString(m_vaTechnician), 0, GetCurrentUserName()); 
				}
			}else {
				m_nSelTechnician = VarLong(pRow->GetValue(egcID));
				m_vaTechnician.RemoveAll();
				m_vaTechnician.Add((_variant_t)m_nSelTechnician);
			}
		}else {
			m_dlTechnician->SetSelByColumn(egcID, m_nSelTechnician);
		}
		SetRemotePropertyInt("EMRSearchAssistTechFilter", m_nSelTechnician, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

// (z.manning 2008-11-12 14:50) - PLID 26167
void CEMRSearch::OnSelChosenListSecProvider(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow != NULL) {
			if (VarLong(pRow->GetValue(egcID)) == -1) // multiple
			{
				bool bResult = OpenMultiSelectList(m_dlSecProvider, m_vaSecProvider, m_nSelSecProvider, "ID", "Last + ', ' + First + ' ' + Middle", "Secondary Providers");
				// (b.spivey, January 23, 2013) - PLID 48370 - Store secondary Provider ids/selection
				if (bResult && m_vaSecProvider.GetSize() > 0) {
					SetRemotePropertyText("EMRSearchSecondaryProviderIDs", VariantArrayToString(m_vaSecProvider), 0, GetCurrentUserName()); 
				}
			}
			else
			{
				m_nSelSecProvider = VarLong(pRow->GetValue(egcID));
				m_vaSecProvider.RemoveAll();
				m_vaSecProvider.Add((_variant_t)m_nSelSecProvider);
			}
		}
		else {
			m_dlSecProvider->SetSelByColumn(egcID, m_nSelSecProvider);
		}
		SetRemotePropertyInt("EMRSearchSecondaryProviderFilter", m_nSelSecProvider, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

void CEMRSearch::OnSelChosenListLocation(LPDISPATCH lpRow) 
{
		try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(egcID)) == -1) // multiple
			{
				bool bResult = OpenMultiSelectList(m_dlLocation, m_vaLocation, m_nSelLocation, "ID", "Name", "Locations");
				// (b.spivey, January 23, 2013) - PLID 48370 - Store location ids/selection
				if (bResult && m_vaLocation.GetSize() > 0) {
					SetRemotePropertyText("EMRSearchLocationIDs", VariantArrayToString(m_vaLocation), 0, GetCurrentUserName()); 
				}

			} else {
				m_nSelLocation = VarLong(pRow->GetValue(egcID));
				m_vaLocation.RemoveAll();
				m_vaLocation.Add((_variant_t)m_nSelLocation);
			}
		} else {
			m_dlLocation->SetSelByColumn(egcID, m_nSelLocation);
		}

		SetRemotePropertyInt("EMRSearchLocationFilter", m_nSelLocation, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

void CEMRSearch::OnEditFilter() 
{
//	bool bNewFilter = (m_nFilterID == FILTER_ID_NEW);
	FilterEditorDlg();
}

void CEMRSearch::RefreshList(bool bForce /* = false*/)
{
	try {
		CString strWhere = GenerateWhereClause();
		CString strFrom = GenerateFromClause();

		// (j.jones 2006-11-02 12:04) - PLID 23327 - if bForce = true,
		// refresh no matter what
		if (strFrom == m_strFrom && strWhere == m_strWhere && !bForce) {
			return;
		}

		// (a.walling 2007-06-28 10:41) - PLID 26353 - Create the entire sql statement here. We will be using a subquery so we can optionally
		// union in the seen patients.
		// (z.manning, 02/29/2008) - PLID 29158 - Added appt start time, show state, resource(s), and has been billed fields
		// (z.manning 2008-11-12 15:00) - PLID 26167 - Added secondary provider
		// (d.lange 2011-04-25 09:20) - PLID 43381 - Added technicians
		// (j.jones 2011-07-07 09:52) - PLID 44451 - supported EMRStatusListT
		// (z.manning 2012-07-05 15:50) - PLID 50958 - Added status ID
		// (b.eyers 2015-05-19) - PLID 49999 - emns with only unbillable charges should show as <no charges> 
		CString strSql = FormatString("SELECT "
			"EMRGroupsT.ID AS EMRID, "
			"EMRMasterT.ID AS EMNID, "
			"PicT.ID AS PICID, "
			"EMRMasterT.PatientID AS PatientID, "
			"dbo.GetEmnProviderList(EmrMasterT.ID) AS Provider, "
			"dbo.GetEmnSecondaryProviderList(EmrMasterT.ID) AS SecProvider, "
			"dbo.GetEmnTechniciansT(EmrMasterT.ID) AS Technician, "
			"COALESCE(PatientLast, PatientQ.Last) + ', ' + COALESCE(PatientFirst, PatientQ.First) + ' ' + COALESCE(PatientMiddle, PatientQ.Middle) AS Patient, "
			"EMRMasterT.Description AS Description, "
			"EMRStatusListT.ID AS StatusID, "
			"EMRStatusListT.Name AS Status, "
			"LocationsT.Name AS Location, "
			"EMRMasterT.Date AS Date, "
			"EMRMasterT.InputDate AS Created, "
			"EMRMasterT.ModifiedDate AS Modified, "
			"CASE WHEN EmrMasterT.ID IN (SELECT EmrID FROM EmrChargesT WHERE Deleted = 0) THEN "
			"	CASE WHEN EmrMasterT.ID IN (SELECT EmnID FROM BilledEmnsT WHERE BillID IN "
			"		(SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) THEN 'Yes' "
			"	WHEN BillableCPTQ.EmrID IS NOT NULL THEN 'No' "
			"	WHEN BillableProductQ.EmrID IS NOT NULL THEN 'No' "
			"	ELSE '<No Charges>' END "
			"	ELSE '<No Charges>' END AS HasBeenBilled, "
			"AppointmentsT.ID AS ApptID, "
			"AppointmentsT.StartTime, "
			"dbo.GetResourceString(AppointmentsT.ID) AS Resource, "
			"'(' + AptShowStateT.Symbol + ')' AS ShowState "
			"FROM %s "
			"WHERE %s ",
			strFrom, strWhere);

		extern CPracticeApp theApp;

		// (j.jones 2014-05-15 16:40) - PLID 61798 - Renamed the existing refresh checkbox to clearly
		// state it only applies to filters. Also ensured we do not requery if the dialog is still
		// initializing.
		if (m_bDialogInitialized && (bForce || m_bRefreshOnFilterChange || !m_bAdvanced)) {
			m_strFrom = strFrom;
			m_strWhere = strWhere;

			//m_dlEMNList->PutWhereClause(_bstr_t(strWhere));

#ifdef _DEBUG
//			AfxMessageBox(strWhere);
#endif
			// (a.walling 2007-06-28 10:42) - PLID 26353 - Include the seen patients if necessary
			bool bSeenToday = false;
			NXDATALIST2Lib::IRowSettingsPtr pDateRow = m_dlDateType->GetCurSel();

			if (pDateRow == NULL) {
				bSeenToday = false;
			} else {
				long nDateType = VarLong(pDateRow->GetValue(egcID), dtlEMNLastModifiedDate);

				if (nDateType == dtlPatientSeen)
					bSeenToday = true;
				else
					bSeenToday = false;
			}

			if (bSeenToday && !m_bSuppressSeenPatients) {
				m_dlEMNList->PutFromClause(_bstr_t(FormatString("(%s UNION ALL %s) AS MasterQ", strSql, GenerateSeenSql())));
			} else {
				m_dlEMNList->PutFromClause(_bstr_t(FormatString("(%s) AS MasterQ", strSql)));
			}

			CString strTitle = m_bAdvanced ? "EMR Search and Review" : "Patients Seen Today";

			strTitle += " - Loading...";

			SetWindowText(strTitle);

			// (j.jones 2014-05-15 17:25) - PLID 62171 - cache the current EMN ID, if any is selected,
			// as well as the next EMN ID, and the top row
			m_nCurEMNID = -1;
			m_nNextEMNID = -1;
			m_nTopRowEMNID = -1;
			NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_dlEMNList->GetCurSel();
			if (pCurRow) {
				m_nCurEMNID = VarLong(pCurRow->GetValue(eelcEMNID), -1);
				//now get the next row
				pCurRow = pCurRow->GetNextRow();
				if (pCurRow) {
					m_nNextEMNID = VarLong(pCurRow->GetValue(eelcEMNID), -1);
				}

				NXDATALIST2Lib::IRowSettingsPtr pTopRow = m_dlEMNList->GetTopRow();
				if (pTopRow) {
					m_nTopRowEMNID = VarLong(pTopRow->GetValue(eelcEMNID), -1);
				}
			}			

			m_dlEMNList->Requery();
		}
		else {
			// (j.jones 2014-05-15 16:40) - PLID 61798 - renamed the existing refresh checkbox to clearly
			// state it only applies to filters
			if (!m_bRefreshOnFilterChange) {
				GetDlgItem(IDC_EMRSEARCH_REFRESH)->SetFocus();
			}
		}
	} NxCatchAll("Error in CEMRSearch:RefreshList");
}

void CEMRSearch::OnSelChosenListEmrfilter(LPDISPATCH lpRow) 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	try {
		if (pRow) {
			long nID = VarLong(pRow->GetValue(egcID), FILTER_ID_ALL);

			if (nID == FILTER_ID_ALL) {
				m_nFilterID = FILTER_ID_ALL;
				m_strFilterString.Empty();
				GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->EnableWindow(FALSE);
			}
			else if (nID == FILTER_ID_NEW) {
				// new filter!
				long nOldFilterID = m_nFilterID;
				m_nFilterID = FILTER_ID_NEW;

				long nResult = FilterEditorDlg();

				if (nResult == IDCANCEL) {
					m_nFilterID = nOldFilterID;
					m_dlFilter->SetSelByColumn(egcID, _variant_t(m_nFilterID, VT_I4));
				}

				if (m_nFilterID == FILTER_ID_ALL) {
					GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->EnableWindow(FALSE);
				} else {
					GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->EnableWindow(TRUE);
				}
			}
			else {
				m_nFilterID = nID;
				m_strFilterString = VarString(pRow->GetValue(2), "");
				GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->EnableWindow(TRUE);
			}

			RefreshList();
		} else {
			m_dlFilter->SetSelByColumn(egcID, _variant_t(long(m_nFilterID), VT_I4));
			m_strFilterString.Empty();
			GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->EnableWindow(FALSE);
		}
		// (b.spivey, January 23, 2013) - PLID 48370 - Store filter selection
		SetRemotePropertyInt("EMRSearchLetterWritingFilter", m_nFilterID, 0, GetCurrentUserName()); 
	} NxCatchAll("Error in CEMRSearch:OnSelChosenEmrfilter");
}

void CEMRSearch::OnRememberColumns() 
{
	try {
		m_bRememberColumns = IsDlgButtonChecked(IDC_REMEMBERCOLUMNS) != 0;
		SetRemotePropertyInt("EmrSearchRememberColumns", m_bRememberColumns ? 1 : 0, 0, GetCurrentUserName());

		if (m_bRememberColumns) {
			SaveColumns(); // this was just checked, so save what we have now (overwriting what was already there)
		}
	} NxCatchAll("Error in CEMRSearch:OnRememberColumns");
}

// (j.jones 2014-05-15 16:40) - PLID 61798 - renamed the existing refresh checkbox to clearly
// state it only applies to filters
void CEMRSearch::OnCheckRefreshOnFilterChange()
{
	try {
		m_bRefreshOnFilterChange = IsDlgButtonChecked(IDC_REFRESH_WHEN_FILTERS_CHANGE) ? true : false;
		//keeping the old ConfigRT name, even though this is no longer called 'auto refresh'
		SetRemotePropertyInt("EmrSearchAutoRefresh", m_bRefreshOnFilterChange ? 1 : 0, 0, GetCurrentUserName());

		// (b.savon 2014-02-05 09:22) - PLID 60646 - On Patients Seen, the Refresh button should always be shown.
		/*if (m_bAutoRefresh) {
			GetDlgItem(IDC_EMRSEARCH_REFRESH)->ShowWindow(SW_HIDE);
		}
		else {
			GetDlgItem(IDC_EMRSEARCH_REFRESH)->ShowWindow(SW_SHOW);
		}*/
		
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-05-15 16:40) - PLID 61798 - created a new checkbox for refreshing upon window restore
void CEMRSearch::OnCheckRefreshWhenWindowRestored()
{
	try {
		m_bRefreshOnWindowRestore = IsDlgButtonChecked(IDC_REFRESH_WHEN_WINDOW_RESTORED) ? true : false;
		SetRemotePropertyInt("EmrSearchRefreshOnWindowRestore", m_bRefreshOnWindowRestore ? 1 : 0, 0, GetCurrentUserName());

		//never hide the refresh button

	} NxCatchAll(__FUNCTION__);
}

BOOL CEMRSearch::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		bool bRefresh = true;
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		long nPatID, nEMNID, nPICID;

		_variant_t vtNull;
		vtNull.vt = VT_NULL;
		
		COleDateTimeSpan dtsOneDay;
		dtsOneDay.SetDateTimeSpan(1, 0, 0, 0);

		COleDateTimeSpan dts6days;
		dts6days.SetDateTimeSpan(6, 0, 0, 0);

		COleDateTimeSpan dts7days;
		dts7days.SetDateTimeSpan(7, 0, 0, 0);

		COleDateTimeSpan dts30days;
		dts30days.SetDateTimeSpan(30, 0, 0, 0);

		COleDateTimeSpan dts365days;
		dts365days.SetDateTimeSpan(365, 0, 0, 0);

		COleDateTime dtYesterday = COleDateTime::GetCurrentTime();
		COleDateTime dtLast7 = COleDateTime::GetCurrentTime();
		COleDateTime dtThisSunday = COleDateTime::GetCurrentTime();
		COleDateTime dtLastSunday = COleDateTime::GetCurrentTime();
		COleDateTime dtLast30 = COleDateTime::GetCurrentTime();
		COleDateTime dtThisMonth = COleDateTime::GetCurrentTime();
		COleDateTime dtLastMonth = COleDateTime::GetCurrentTime();
		COleDateTime dtLast365 = COleDateTime::GetCurrentTime();
		COleDateTime dtThisYear = COleDateTime::GetCurrentTime();
		COleDateTime dtLastYear = COleDateTime::GetCurrentTime();

		switch (wParam) {
		case ID_EMRSEARCH_ALLDATES:
			m_dtFrom.SetValue(vtNull);
			m_dtTo.SetValue(vtNull);

			WarnSeenPatients(); break;

		case ID_EMRSEARCH_TODAY:
			m_dtFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
			m_dtTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

			WarnSeenPatients(); break;

		case ID_EMRSEARCH_YESTERDAY:
			dtYesterday -= dtsOneDay;

			m_dtFrom.SetValue((_variant_t)dtYesterday);
			m_dtTo.SetValue((_variant_t)dtYesterday);

			WarnSeenPatients(); break;

		case ID_EMRSEARCH_LAST7:
			dtLast7 -= dts7days;
			dtLast7 += dtsOneDay;

			m_dtFrom.SetValue((_variant_t)dtLast7);
			m_dtTo.SetValue((_variant_t)COleDateTime::GetCurrentTime());

			WarnSeenPatients(); break;

		case ID_EMRSEARCH_THISWEEK:
			while (dtThisSunday.GetDayOfWeek() != 1) {
				dtThisSunday -= dtsOneDay;
			}
			m_dtFrom.SetValue((_variant_t)dtThisSunday);
			
			dtThisSunday += dts6days;
			m_dtTo.SetValue((_variant_t)dtThisSunday);
			WarnSeenPatients(); break;

		case ID_EMRSEARCH_LASTWEEK:
			dtLastSunday -= dts7days;
			while (dtLastSunday.GetDayOfWeek() != 1) {
				dtLastSunday -= dtsOneDay;
			}
			m_dtFrom.SetValue((_variant_t)dtLastSunday);
			
			dtLastSunday += dts6days;
			m_dtTo.SetValue((_variant_t)dtLastSunday);
			WarnSeenPatients(); break;

		case ID_EMRSEARCH_LAST30:
			dtLast30 -= dts30days;
			dtLast30 += dtsOneDay;

			m_dtFrom.SetValue((_variant_t)dtLast30);
			m_dtTo.SetValue((_variant_t)COleDateTime::GetCurrentTime());

			WarnSeenPatients(); break;

		case ID_EMRSEARCH_THISMONTH:
			dtThisMonth.SetDate(dtThisMonth.GetYear(), dtThisMonth.GetMonth(), 1);

			m_dtFrom.SetValue((_variant_t)dtThisMonth);

			dtThisMonth.SetDate(dtThisMonth.GetYear(), dtThisMonth.GetMonth(), GetDaysInMonth(dtThisMonth.GetMonth(), dtThisMonth.GetYear()));
			m_dtTo.SetValue((_variant_t)dtThisMonth);
			WarnSeenPatients(); break;

		case ID_EMRSEARCH_LASTMONTH:
			if (dtLastMonth.GetMonth() > 1) {
				dtThisMonth.SetDate(dtLastMonth.GetYear(), dtLastMonth.GetMonth() - 1, 1);
			}
			else {
				dtThisMonth.SetDate(dtLastMonth.GetYear() - 1, 12, 1); // december 1st!
			}

			m_dtFrom.SetValue((_variant_t)dtThisMonth);

			dtThisMonth.SetDate(dtThisMonth.GetYear(), dtThisMonth.GetMonth(), GetDaysInMonth(dtThisMonth.GetMonth(), dtThisMonth.GetYear()));
			m_dtTo.SetValue((_variant_t)dtThisMonth);
			WarnSeenPatients(); break;

		case ID_EMRSEARCH_LAST365:

			dtLast365 -= dts365days;
			dtLast365 += dtsOneDay;

			m_dtFrom.SetValue((_variant_t)dtLast365);
			m_dtTo.SetValue((_variant_t)COleDateTime::GetCurrentTime());
			WarnSeenPatients(); break;

		case ID_EMRSEARCH_THISYEAR:

			dtThisYear.SetDate(dtThisYear.GetYear(), 1, 1); // jan 1st
			m_dtFrom.SetValue((_variant_t)dtThisYear);
			dtThisYear.SetDate(dtThisYear.GetYear(), 12, 31); // dec 31st
			m_dtTo.SetValue((_variant_t)dtThisYear);
			WarnSeenPatients(); break;

		case ID_EMRSEARCH_LASTYEAR:

			dtLastYear.SetDate(dtLastYear.GetYear() - 1, 1, 1); // jan 1st
			m_dtFrom.SetValue((_variant_t)dtLastYear);
			dtLastYear.SetDate(dtLastYear.GetYear(), 12, 31); // dec 31st
			m_dtTo.SetValue((_variant_t)dtLastYear);
			WarnSeenPatients(); break;

		///////////////////////////////////

		case ID_EMR_GOTOPATIENT:
			try {
				pRow = m_dlEMNList->GetCurSel();

				if (pRow) {
					nPatID = VarLong(pRow->GetValue(eelcPatientID), -1);
					if (nPatID != -1) {
						GotoPatient(nPatID);
					}
				}
			} NxCatchAll("Error in Command GoToPatient");
			break;

		case ID_EMR_GOTOEMN:
			try {
				pRow = m_dlEMNList->GetCurSel();

				if (pRow != NULL) {
					nEMNID = VarLong(pRow->GetValue(eelcEMNID), -1);
					nPICID = VarLong(pRow->GetValue(eelcPICID), -1);

					if (nPICID == -1) {
						MessageBox("This EMN record does not have an associated PIC entry. Data is intact, however the record must be opened manually from the Patients module.");
						break;
					}
					if (nEMNID == -1) {
						ThrowNxException("Error in GotoEMN: there is no EMN ID!");
					}

					// (j.jones 2008-09-30 17:26) - PLID 28011 - minimize this screen first
					MinimizeWindow();

					GetMainFrame()->EditEmrRecord(nPICID, nEMNID);
				}
			} NxCatchAll("Error in Command GoToEMN");
			break;

		default:
			bRefresh = false;
			break;
		}

		if (bRefresh) {
			RefreshList();
		}

	} NxCatchAll("Error in CEMRSearch:OnCommandQuickDate");
	return CNxDialog::OnCommand(wParam, lParam);
}

void CEMRSearch::OnSelChosenDatefilterType(LPDISPATCH lpRow)
{
		try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// (b.spivey, January 23, 2013) - PLID 48370 - default to last mod date. 
		long nDateType = (long)dtlEMNLastModifiedDate; 
		if (pRow == NULL) {
			m_dlDateType->SetSelByColumn(0, _variant_t(long(dtlEMNLastModifiedDate), VT_I4));
			GetDlgItem(IDC_SHOW_SEENPATIENTS)->EnableWindow(FALSE);
			GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->EnableWindow(FALSE);		
		} else {
			// (z.manning 2008-06-12 17:04) - PLID 29380 - We now need to handle the seent with check as well
			// as it's now visible in advanced mode.
			nDateType = VarLong(pRow->GetValue(egcID), dtlEMNLastModifiedDate);
			if (nDateType == dtlPatientSeen) {
				GetDlgItem(IDC_SHOW_SEENPATIENTS)->EnableWindow(TRUE);
				GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->EnableWindow(TRUE);
			} else {
				GetDlgItem(IDC_SHOW_SEENPATIENTS)->EnableWindow(FALSE);
				GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->EnableWindow(FALSE);
				// (b.spivey, January 29, 2013) - PLID 48370 - Don't manually set these setting. 
			}
		}
		GetDlgItem(IDC_SHOW_SEENPATIENTS)->RedrawWindow();
		GetDlgItem(IDC_SHOW_SEENPATIENTS_WITH)->RedrawWindow();
		// (b.spivey, January 18, 2013) - PLID 48370 - store date type
		SetRemotePropertyInt("EMRSearchDateTypeFilter", nDateType, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

void CEMRSearch::WarnSeenPatients()
{
	// (a.walling 2007-06-15 10:38) - PLID 24859 - Warn if neither is selected
	if (!m_bAdvanced && !IsDlgButtonChecked(IDC_SHOW_SEENPATIENTS) && !IsDlgButtonChecked(IDC_SHOW_SEENPATIENTS_WITH)) {
		MessageBox("By choosing to show neither seen patients with EMNs and without EMNs, there is nothing to display!");
		return;
	}

	// (a.walling 2007-02-05 11:25) - PLID 24563 - Prevent the user from inadvertently using huge amounts of resources by
	// loading Seen Patient rows for a huge date range.

	// (z.manning 2009-05-19 12:32) - PLID 28512 - If we're filtered only on the current patient, then
	// no need to warn here.
	if(m_bShowCurrentPatientOnly) {
		return;
	}

	// from January 1, 1753 to December 30, 9999
	COleDateTime dtMin, dtMax;
	dtMin.SetDate(1753, 1, 1);
	dtMax.SetDate(9999, 12, 30);
	
	_variant_t vardtFrom = m_dtFrom.GetValue();
	_variant_t vardtTo = m_dtTo.GetValue();
	COleDateTime dtTo, dtFrom;

	// swap them if from > to
	if ( (vardtFrom.vt != VT_NULL) && (vardtTo.vt != VT_NULL) ) {
		if (VarDateTime(vardtFrom) > VarDateTime(vardtTo)) {
			_variant_t vardtTemp;
			vardtTemp = vardtTo;
			vardtTo = vardtFrom;
			vardtFrom = vardtTemp;
			m_dtFrom.SetValue(vardtFrom);
			m_dtTo.SetValue(vardtTo);
		}
	}

	if (vardtFrom.vt != VT_NULL) {
		dtFrom = VarDateTime(vardtFrom);
		if (dtFrom > dtMax)
		{
			dtFrom = dtMax;
			m_dtFrom.SetValue(_variant_t(dtFrom));
		}
		else if (dtFrom < dtMin)
		{
			dtFrom = dtMin;
			m_dtFrom.SetValue(_variant_t(dtFrom));
		}
	} else {
		dtFrom = dtMin;
	}
	if (vardtTo.vt != VT_NULL) {
		dtTo = VarDateTime(vardtTo);
		if (dtTo > dtMax)
		{
			dtTo = dtMax;
			m_dtTo.SetValue(_variant_t(dtTo));
		}
		else if (dtTo < dtMin)
		{
			dtTo = dtMin;
			m_dtTo.SetValue(_variant_t(dtTo));
		}
		COleDateTimeSpan dtsOneDay;
		dtsOneDay.SetDateTimeSpan(1, 0, 0, 0);
		dtTo += dtsOneDay;
	} else {
		dtTo = dtMax;
	}

	// (a.walling 2007-04-23 16:22) - PLID 25737 - Exit if we are suppressing
	if (m_bSuppressSeenPatients)
		return;

	COleDateTimeSpan dtsLength = dtTo - dtFrom;
	long nDays = (long)dtsLength.GetTotalDays();

	NXDATALIST2Lib::IRowSettingsPtr pDateRow = m_dlDateType->GetCurSel();
	if (pDateRow) {
		long nDateType = VarLong(pDateRow->GetValue(egcID), -1);
		ASSERT(nDateType != -1);

		// (a.walling 2007-04-23 16:22) - PLID 25737 - this message box should only appear for Patient Seen mode when the box is checked.
		if ((nDays > 365) && (nDateType == dtlPatientSeen) && IsDlgButtonChecked(IDC_SHOW_SEENPATIENTS))  // dlg button should be in sync with m_bSuppressSeenPatients
			
		{
			// more than a year?! make sure the user wants to do this

			// to clarify another concern, i consider this a special case, which is why
			// the preference is not added to ConfigRT.
			// (a.walling 2007-06-28 10:52) - PLID 26353 - Softened the wording a bit since this is much less intensive. May even want to remove this entirely at some point.
			if (IDNO == MessageBox("You have selected a wide date range. Adding a row for each patient and appointment seen during this time period may use a large amount of resources. It is recommended that this option be unselected for wide date ranges.\r\n\r\nDo you want to continue adding these rows?", "Practice", MB_YESNO)) {
				CheckDlgButton(IDC_SHOW_SEENPATIENTS, FALSE);
				m_bSuppressSeenPatients = true;
			}
			else {
				CheckDlgButton(IDC_SHOW_SEENPATIENTS, TRUE);
				m_bSuppressSeenPatients = false;
			}
		}
	}
}

void CEMRSearch::OnChangeDateFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!m_bDateFromDown)
	{
		WarnSeenPatients();
		RefreshList();
	}

	*pResult = 0;
}

void CEMRSearch::OnChangeDateTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!m_bDateToDown)
	{
		WarnSeenPatients();
		RefreshList();
	}

	*pResult = 0;
}

void CEMRSearch::OnDropDownDateFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_bDateFromDown = true;	

	*pResult = 0;
}

void CEMRSearch::OnCloseUpDateFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_bDateFromDown = false;
	WarnSeenPatients();
	RefreshList();

	*pResult = 0;
}


void CEMRSearch::OnDropDownDateTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_bDateToDown = true;	

	*pResult = 0;
}

void CEMRSearch::OnCloseUpDateTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_bDateToDown = false;
	WarnSeenPatients();
	RefreshList();

	*pResult = 0;
}

void CEMRSearch::OnStatusOpen() 
{
	try {
		// (b.spivey, January 23, 2013) - PLID 48370 - store status open state. 
		if (m_btnStatusOpen.GetCheck() == BST_CHECKED) {
			SetRemotePropertyInt("EMNSearchStatusOpen", TRUE, 0, GetCurrentUserName()); 
		}
		else {
			SetRemotePropertyInt("EMNSearchStatusOpen", FALSE, 0, GetCurrentUserName()); 
		}
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

void CEMRSearch::OnStatusFinished() 
{
	try {
		// (b.spivey, January 23, 2013) - PLID 48370 - store finished status state
		if (m_btnStatusFinished.GetCheck() == BST_CHECKED) {
			SetRemotePropertyInt("EMNSearchStatusFinished", TRUE, 0, GetCurrentUserName()); 
		}
		else {
			SetRemotePropertyInt("EMNSearchStatusFinished", FALSE, 0, GetCurrentUserName()); 
		}
		
		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

void CEMRSearch::OnStatusLocked() 
{
	// (b.spivey, January 23, 2013) - PLID 48370 - Store status locked state
	if (m_btnStatusLocked.GetCheck() == BST_CHECKED) {
		SetRemotePropertyInt("EMNSearchStatusLocked", TRUE, 0, GetCurrentUserName()); 
	}
	else {
		SetRemotePropertyInt("EMNSearchStatusLocked", FALSE, 0, GetCurrentUserName()); 
	}
	RefreshList();
	
}

void CEMRSearch::SetSeenToday(bool bSeen /* = true*/)
{
	if (bSeen) {
		
		if (m_bAdvanced && m_bDialogInitialized) {
			// dialog was in advanced mode but now the patients seen dialog is called
			m_bAdvanced = false;
			ShowAdvanced(false);
		}
		else {
			m_bAdvanced = false;
		}
	}
	else {
		if (!m_bAdvanced && m_bDialogInitialized) {
			// dialog already exists and is now becoming advanced
			m_bAdvanced = true;
			ShowAdvanced(true);
		}
	}

	// (j.jones 2014-05-16 09:48) - PLID 61798 - this function is only ever called
	// upon creation of the dialog, so we should always force a requery regardless
	// of their refresh settings
	RefreshList(true);
}

void CEMRSearch::ShowAdvanced(bool bShow)
{
	try {
		if (bShow) {
			m_bAdvanced = true;

			// show controls
			GetDlgItem(IDC_DATE_GROUP)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DATEFILTER_TYPE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMNDATE_FROM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMNDATE_TO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DATEFILTER_QUICK)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_FROM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_TO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_PROC)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_DIAG)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_PROB)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_PROV)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_SEC_PROV)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_LOC)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_FILTER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMRGROUP_1)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_PROCEDURE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_SEARCH)->ShowWindow(SW_SHOW);// (a.wilson 2014-03-03 16:37) - PLID 60780
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_NONE)->ShowWindow(SW_SHOW); // (r.goldschmidt 2014-07-14 15:26) - PLID 62883
			GetDlgItem(IDC_LIST_PROBLEMS)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMRGROUP_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_PROVIDER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_SEC_PROVIDER)->ShowWindow(SW_SHOW); // (z.manning 2008-11-12 14:35) - PLID 26167
			GetDlgItem(IDC_LIST_TECH)->ShowWindow(SW_SHOW);		// (d.lange 2011-04-25 09:33) - PLID 43381
			GetDlgItem(IDC_LIST_LOCATION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_EMRFILTER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMRGROUP_STATUS)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATUS_OPEN)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATUS_FINISHED)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATUS_LOCKED)->ShowWindow(SW_SHOW);
			// (j.jones 2014-05-15 16:40) - PLID 61798 - renamed the existing refresh checkbox to clearly
			// state it only applies to filters, and created a new checkbox for refreshing upon window restore
			GetDlgItem(IDC_REFRESH_WHEN_FILTERS_CHANGE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_REFRESH_WHEN_WINDOW_RESTORED)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATUSLABEL)->ShowWindow(SW_SHOW);
			// (j.jones 2011-07-07 08:48) - PLID 44451 - added option for "other" statuses
			GetDlgItem(IDC_STATUS_OTHER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_OTHER_STATUS)->ShowWindow(SW_SHOW);
			// (j.dinatale 2013-01-25 13:19) - PLID 54777 - appt ref phys filter
			GetDlgItem(IDC_STATIC_APPT_REF_PHYS)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_APPT_REF_PHYS)->ShowWindow(SW_SHOW);
			// (j.jones 2013-07-15 14:49) - PLID 57477 - added chart & category filters
			GetDlgItem(IDC_STATIC_CHART)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_COMBO_CHART)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_CATEGORIES)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_COMBO_CATEGORIES)->ShowWindow(SW_SHOW);

			// (b.savon 2014-02-05 09:22) - PLID 60646 - On Patients Seen, the Refresh button should always be shown.
			/*if (m_bAutoRefresh) {
				GetDlgItem(IDC_EMRSEARCH_REFRESH)->ShowWindow(SW_HIDE);
			}
			else {
				GetDlgItem(IDC_EMRSEARCH_REFRESH)->ShowWindow(SW_SHOW);
			}*/

			GetDlgItem(IDC_EMR_SHOW_ADVANCED)->ShowWindow(SW_HIDE);

			CRect rect, dialogRect, topRect, closeRect;
			GetClientRect(&dialogRect);
			// (d.lange 2011-04-25 15:16) - PLID 43381 - Changed to use the position of the provider filter groupbox since it's been resized to be the largest
			GetDlgItem(IDC_EMRGROUP_2)->GetWindowRect(&topRect);
			GetDlgItem(ID_EMRSEARCH_CLOSE)->GetWindowRect(&closeRect);

			this->ScreenToClient(&topRect);
			this->ScreenToClient(&closeRect);

			rect.SetRect(dialogRect.left + 5, topRect.bottom + 5, dialogRect.right - 10, closeRect.top - 5);
			GetDlgItem(IDC_EMNLIST)->MoveWindow(&rect, TRUE); // move and repaint

			SetWindowText("EMR Search and Review"); // change the window title

			// (j.gruber 2013-05-20 12:40) - PLID 43442 - set the window based on our settings
			CRect rcShowWith;
			GetDlgItem(IDC_ST_WITH_PLACEHOLDER)->GetWindowRect(&rcShowWith);
			ScreenToClient(&rcShowWith);

			//now move the window			
			GetDlgItem(IDC_SHOW_SEENPATIENTS)->MoveWindow(&rcShowWith, TRUE);

			// (b.spivey, January 23, 2013) - PLID 48370 - Refresh this list, they may have other filters applied to it. 
			// (j.jones 2014-05-15 16:40) - PLID 61798 - renamed the existing refresh checkbox to clearly
			// state it only applies to filters
			if (m_bRefreshOnFilterChange) {
				RefreshList();
			}
		}
		else {
			// hide controls
			GetDlgItem(IDC_DATE_GROUP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DATEFILTER_TYPE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMNDATE_FROM)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMNDATE_TO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DATEFILTER_QUICK)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_FROM)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_TO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_PROC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_DIAG)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_PROB)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_PROV)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_SEC_PROV)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_LOC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_FILTER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMRGROUP_1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_PROCEDURE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_LIST)->ShowWindow(SW_HIDE); // (a.wilson 2014-03-03 16:37) - PLID 60780
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_SEARCH)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_NONE)->ShowWindow(SW_HIDE); // (r.goldschmidt 2014-07-14 15:26) - PLID 62883
			GetDlgItem(IDC_LIST_PROBLEMS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMRGROUP_2)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_PROVIDER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_SEC_PROVIDER)->ShowWindow(SW_HIDE); // (z.manning 2008-11-12 14:35) - PLID 26167
			GetDlgItem(IDC_LIST_TECH)->ShowWindow(SW_HIDE);		// (d.lange 2011-04-25 09:34) - PLID 43381
			GetDlgItem(IDC_LIST_LOCATION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_EMRFILTER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMRGROUP_STATUS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATUS_OPEN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATUS_FINISHED)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATUS_LOCKED)->ShowWindow(SW_HIDE);
			// (b.savon 2014-02-05 09:22) - PLID 60646 - On Patients Seen, the Refresh button should always be shown.
			//GetDlgItem(IDC_EMRSEARCH_REFRESH)->ShowWindow(SW_HIDE);
			// (j.jones 2014-05-15 16:40) - PLID 61798 - renamed the existing refresh checkbox to clearly
			// state it only applies to filters, and created a new checkbox for refreshing upon window restore
			GetDlgItem(IDC_REFRESH_WHEN_FILTERS_CHANGE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_REFRESH_WHEN_WINDOW_RESTORED)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATUSLABEL)->ShowWindow(SW_HIDE);
			// (j.jones 2011-07-07 08:48) - PLID 44451 - added option for "other" statuses
			GetDlgItem(IDC_STATUS_OTHER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_OTHER_STATUS)->ShowWindow(SW_HIDE);
			// (j.jones 2013-07-15 14:49) - PLID 57477 - added chart & category filters
			GetDlgItem(IDC_STATIC_CHART)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_COMBO_CHART)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_CATEGORIES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_COMBO_CATEGORIES)->ShowWindow(SW_HIDE);

			GetDlgItem(IDC_EMR_SHOW_ADVANCED)->ShowWindow(SW_SHOW);

			// (j.dinatale 2013-01-25 13:19) - PLID 54777 - appt ref phys filter
			GetDlgItem(IDC_STATIC_APPT_REF_PHYS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_APPT_REF_PHYS)->ShowWindow(SW_HIDE);

			CRect rect, dialogRect, topRect, closeRect;

			GetClientRect(&dialogRect);
			GetDlgItem(ID_EMRSEARCH_CLOSE)->GetWindowRect(&closeRect);
			GetDlgItem(IDC_EMR_SHOW_ADVANCED)->GetWindowRect(&topRect);
			
			this->ScreenToClient(&topRect);
			this->ScreenToClient(&closeRect);

			rect.SetRect(dialogRect.left + 5, topRect.bottom + 5, dialogRect.right - 10, closeRect.top - 5);
			GetDlgItem(IDC_EMNLIST)->MoveWindow(&rect, TRUE); // move and repaint

			m_dtFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
			m_dtTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_dlDateType->TrySetSelByColumn_Deprecated(0, _variant_t(long(dtlPatientSeen), VT_I4));

			SetWindowText("Patients Seen Today");

			// (j.gruber 2013-05-20 12:40) - PLID 43442 - set the window based on our settings
			CRect rcShowWithout;
			GetDlgItem(IDC_ST_WITHOUT_PLACEHOLDER)->GetWindowRect(&rcShowWithout);
			ScreenToClient(&rcShowWithout);

			//now move the window
			GetDlgItem(IDC_SHOW_SEENPATIENTS)->MoveWindow(&rcShowWithout);

		}

		// (j.gruber 2013-05-20 12:20) - PLID 43442 - change to "filtered" if advanced
		GetDlgItem(IDC_SHOW_SEENPATIENTS)->SetWindowTextA(FormatString("Show Seen Patients w/o EMNs%s", m_bAdvanced ? " in the date range":""));		

		if (m_bDialogInitialized) {
			SetListDefaults();
		}
		GetControlPositions();
	} NxCatchAll("Error in CEMRSearch:ShowAdvanced");
}

void CEMRSearch::OnShowAdvanced() 
{
	try {
		ShowAdvanced(true);	
	}NxCatchAll(__FUNCTION__); 
}


int CEMRSearch::FilterEditorDlg() 
{	
	try {
		if (m_nFilterID == FILTER_ID_ALL)
		{
			return IDCANCEL;
		}

		CFilterEditDlg dlg(NULL, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction, NULL, "Patient Filter");
		
		int nResult = 0;
		if (m_nFilterID == FILTER_ID_NEW)
			nResult = dlg.NewFilter();
		else 
			nResult = dlg.EditFilter(m_nFilterID, m_strFilterString);

		if (nResult == IDOK) {
			m_nFilterID = dlg.GetFilterId();
			m_strFilterString = dlg.m_strFilterString;

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			m_dlFilter->Requery();

			// ideally this would all be in its own function
			pRow = m_dlFilter->GetNewRow();
			pRow->PutValue(egcID, _variant_t(long(FILTER_ID_ALL), VT_I4));
			pRow->PutValue(egcName, " <None> ");
			m_dlFilter->AddRowSorted(pRow, NULL);
			
			pRow = m_dlFilter->GetNewRow();
			pRow->PutValue(egcID, _variant_t(long(FILTER_ID_NEW), VT_I4));
			pRow->PutValue(egcName, " <New Filter> ");
			m_dlFilter->AddRowSorted(pRow, NULL);
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_dlFilter->TrySetSelByColumn_Deprecated(egcID, _variant_t(m_nFilterID, VT_I4));
			m_dlFilter->PutComboBoxText((_bstr_t)dlg.m_strFilterName);

			RefreshList();
			GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->EnableWindow(TRUE);
		}
		else {
			if (m_nFilterID == FILTER_ID_NEW) {
				// they probably cancelled out of the new filter dialog.
				m_nFilterID = FILTER_ID_ALL;
				return IDCANCEL;
			}
		}

		return nResult;
	} NxCatchAll("Error in CEMRSearch::FilterEditorDlg ");

	return IDCANCEL;
}

void CEMRSearch::OnCancel() {
	SaveColumns();
	CDialog::OnCancel();

	// (j.jones 2006-11-02 11:47) - PLID 23321 - tell mainframe that we don't need tablecheckers anymore
	// (z.manning 2013-10-17 09:23) - PLID 59061 - We don't get them at all anymore
	//GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());

	DestroyWindow();
	m_bDialogInitialized = false;
}

void CEMRSearch::OnEmrSearchRefresh() 
{
	RefreshList(true);
}

void CEMRSearch::SaveColumns()
{
	CString strColumnWidths;
	if (!m_bRememberColumns) 
		return;
	try {
		// Store the columns in a xx,xx,xx,xx format
		for (int i = 0; i < m_dlEMNList->ColumnCount; i++)
		{
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_dlEMNList->GetColumn(i);
			CString str;
			
			
			str.Format("%d", pCol->StoredWidth);
			
			if (i > 0)
				strColumnWidths += ",";

			strColumnWidths += str;
		}

		SetRemotePropertyText("EmrSearchColumnWidths", strColumnWidths, 0, GetCurrentUserName());
	} NxCatchAll("Error in SaveColumns()");
}

void CEMRSearch::RestoreColumns()
{
	CString strColumnWidths = GetRemotePropertyText("EmrSearchColumnWidths", "", 0, GetCurrentUserName(), false);

	CArray<int, int> arWidths;
	
	try {
		int tokIndex = strColumnWidths.Find(',');

		if (tokIndex == -1) {
			// (a.walling 2006-07-11 16:33) - PLID 21073 - It is empty or invalid, so rebuild
			SaveColumns();
			return;
		}

		while(tokIndex != -1) {
			CString str = strColumnWidths.Left(tokIndex);
			arWidths.Add(atoi(str));
			strColumnWidths = strColumnWidths.Right(strColumnWidths.GetLength() - (tokIndex + 1));
			tokIndex = strColumnWidths.Find(',');
		}
		arWidths.Add(atoi(strColumnWidths));

		if (arWidths.GetSize() != m_dlEMNList->ColumnCount) {
			// (z.manning, 02/29/2008) - PLID 29158 - I increased the column count from 13 to 18 so let's not 
			// destroy their previous remembered column values.
			if(arWidths.GetSize() == 13 && m_dlEMNList->GetColumnCount() == 18) {
				arWidths.InsertAt(eelcAppointmentID, 0);
				arWidths.InsertAt(eelcTime, 70);
				arWidths.InsertAt(eelcResource, 120);
				arWidths.InsertAt(eelcShowState, 41);
				arWidths.InsertAt(eelcHasBeenBilled, 84);
			}
			else if(arWidths.GetSize() == 21 && m_dlEMNList->GetColumnCount() == 22) {
				// (z.manning 2011-10-28 15:48) - PLID 44594 - Added signature column
				arWidths.InsertAt(eelcCheckForSignature, 25);
			}
			else {
				// It is inconsistent, so rebuild
				SaveColumns();
				return;
			}
		}

		for (int i = 0; i < m_dlEMNList->ColumnCount; i++)
		{
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_dlEMNList->GetColumn(i);
			pCol->PutStoredWidth(arWidths[i]);
		}
	} NxCatchAll("Error in RestoreColumns()");
}

void CEMRSearch::OnRButtonDownEmnlist(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL)
			return;

		if(pRow->GetSelected() == 0) {
			m_dlEMNList->PutCurSel(pRow); // set only this row to current selection if it was not originally selected
		}

		CMenu mnu;
		CMenu *pSubMenu;
		mnu.LoadMenu(IDR_EMR_POPUP);
		pSubMenu = mnu.GetSubMenu(3);

		long nEMNID = VarLong(pRow->GetValue(eelcEMNID), -1);
		if (nEMNID < 0) {
			mnu.EnableMenuItem(ID_EMR_GOTOEMN, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		}

		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	} NxCatchAll("Error in CEMRSearch: creating popup menu");
}

void CEMRSearch::OnLeftClickEmnlist(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if (nCol == eelcPatient) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow == NULL)
				return;
			m_dlEMNList->PutCurSel(pRow);

			long nPatID = VarLong(pRow->GetValue(eelcPatientID));

			GotoPatient(nPatID);
		}
		else if (nCol == eelcDescription) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow != NULL) {
				m_dlEMNList->PutCurSel(pRow);
				long nEMNID = VarLong(pRow->GetValue(eelcEMNID), -1);
				long nPICID = VarLong(pRow->GetValue(eelcPICID), -1);

				if ((nEMNID == -1) && (nPICID == -1)) {
					return; // silently
				}

				if (nPICID == -1) {
					MessageBox("This EMN record does not have an associated PIC entry. Data is intact, however the record must be opened manually from the Patients module.", "Practice");
					return;
				}
				if (nEMNID == -1) {
					ThrowNxException("Error in GotoEMN: there is no EMN ID!");
				}

				// (j.jones 2008-09-30 17:26) - PLID 28011 - minimize this screen first
				MinimizeWindow();

				GetMainFrame()->EditEmrRecord(nPICID, nEMNID);
			}
		}
		else if (nCol == eelcPreview) {
			// (a.walling 2010-01-11 12:53) - PLID 31482 - Open up the EMN preview

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow == NULL)
				return;

			// (a.walling 2011-11-07 13:38) - PLID 46320 - Ignore if a non-EMN / non-Patient record

			long nPatID = VarLong(pRow->GetValue(eelcPatientID), -1);
			long nEMNID = VarLong(pRow->GetValue(eelcEMNID), -1);

			if (-1 == nPatID || -1 == nEMNID) {
				return;
			}
			COleDateTime dtModified = VarDateTime(pRow->GetValue(eelcModified));

			ShowPreview(nPatID, nEMNID, dtModified);
		}
	} NxCatchAll("Error in CEMRSearch: OnLeftClickEmnList");
}

void CEMRSearch::GotoPatient(long nPatID)
{
	// (j.jones 2008-09-30 17:26) - PLID 28011 - minimize this screen first
	MinimizeWindow();

	// (a.walling 2008-06-26 14:12) - PLID 30531 - Use the shared function in the mainframe
	GetMainFrame()->GotoPatient(nPatID);
}

CString CEMRSearch::GenerateSeenSql(bool bForReport /*= false*/)
{
	try {
		// we should add placeholder rows for all patients that have had appointments during the date range.
		CString strDateRange = GenerateDateRange();
		// (a.walling 2007-06-28 10:43) - PLID 26353 - Ensure the EMRQ.EMRPatient is null, meaning there was no EMN found.
		// (j.jones 2009-12-29 10:35) - PLID 35795 - ensure the -25 patient is filtered out
		// do not include no shows and cancelled appts and inactive patients 
		// (a.walling 2010-12-29 10:14) - PLID 41542 - Also exclude pending appointments		
		CString strDateFilter = FormatString(
			"WHERE EMRQ.EMRPatient IS NULL AND AppointmentsT.Status <> 4 "
			"AND AppointmentsT.ShowState <> 3 "
			"%s "
			"AND PersonT.Archived = 0 "
			"AND PersonT.ID <> -25"
			, m_checkExcludePending.GetCheck() == BST_CHECKED ? " AND AppointmentsT.ShowState <> 0" : "" // (a.walling 2011-01-13 10:24) - PLID 41542
			); 

		if (!strDateRange.IsEmpty()) {
			strDateFilter += " AND " + strDateRange;
		}

		// (a.walling 2007-02-15 09:38) - PLID 24563 - If they are using a patient filter, apply this to the seen patients as well
		CString strFilter;
		try {
			if (m_nFilterID == FILTER_ID_ALL) {
				strFilter = "";
			}
			else {
				CString strFilterFrom, strFilterWhere;
				bool bResult = CFilter::ConvertFilterStringToClause(m_nFilterID, m_strFilterString, fboPerson, &strFilterWhere, &strFilterFrom, NULL, NULL, TRUE);
				if (bResult) {
					// (r.gonet 01/14/2014) - PLID 59504 - Eliminated ambiguity.
					strFilter.Format(" AND PersonT.ID IN (SELECT PersonT.ID FROM %s WHERE %s)", strFilterFrom, strFilterWhere);
				}
				else {	
					ASSERT(FALSE);
				}
			}
		} NxCatchAll("Error loading patient filter in OnRequeryFinishedEmnList");

		// (z.manning 2009-05-19 15:41) - PLID 28512 - Added an option for current patient only
		// (s.dhole 2013-08-21 14:59) - PLID 57391 Set to AppointmentsT.PatientID
		if(m_bShowCurrentPatientOnly) {
			strFilter.Format(" AND AppointmentsT.PatientID = %li ", GetActivePatientID());
		}

		CString strExcludeSql;
		//CString strFrom = (LPCTSTR)m_dlEMNList->GetFromClause();
		CString strFrom = GenerateFromClause();
		// (a.walling 2007-06-28 10:43) - PLID 26353 - This is now another join to be appended to the master query rather than its own seperate query
		// (z.manning 2008-06-12 16:56) - PLID 29380 - When generating a where clause we need to ignore patients
		// seen with EMNs checklist since we are just doing a join. Otherwise, even if a patient does
		// have an EMN, it will have no idea and return that he/she doesn't.
		// (j.gruber 2013-06-10 10:59) - PLID 43442 - only filter on dates here 
		strExcludeSql = FormatString("LEFT JOIN (SELECT EMRMasterT.PatientID AS EMRPatient, EMRMasterT.Date AS EMRDate "
        "FROM %s WHERE %s) AS EMRQ ON EMRQ.EMRPatient = AppointmentsT.PatientID AND EMRQ.EMRDate = AppointmentsT.Date", strFrom, GenerateWhereClause(true, true));

		// (a.walling 2007-06-28 10:43) - PLID 26353 - Prepare all the columns for unioning.
		// (z.manning, 02/29/2008) - PLID 29158 - Added appt start time, show state, resource(s), and has been billed fields
		// (z.manning 2008-11-12 15:01) - PLID 26167 - Added secondary provider
		// (d.lange 2011-04-25 09:34) - PLID 43381 - Added Assistant/Technician		
		if (!bForReport) {
			return FormatString("SELECT "
				"NULL AS EMRID, "
				"NULL AS EMNID, "
				"NULL AS PICID, "
				"AppointmentsT.PatientID AS PatientID, "
				"'' AS Provider, "
				"'' AS SecProvider, "
				"'' AS Technician, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Patient, "
				"'<No EMN on appt. date>' AS Description, "
				"NULL AS StatusID, "
				"'Appt.' AS Status, "
				"NULL AS Location, "
				"AppointmentsT.Date as Date, "
				"NULL AS Created, "
				"NULL AS Modified, "
				"'<No EMN>' AS HasBeenBilled, "
				"AppointmentsT.ID AS ApptID, "
				"StartTime, "
				"dbo.GetResourceString(AppointmentsT.ID) AS Resource, "
				"'(' + AptShowStateT.Symbol + ')' AS ShowState "
				"FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "

				"%s %s %s ",				
				strExcludeSql, strDateFilter, strFilter);
		} else {
			// (z.manning 2009-05-20 17:21) - PLID 34314 - Added age and gender to the EMR search report			
			// (s.dhole 2013-08-21 14:59) - PLID 57391  Added appointment primary and secondary insurance name
			return FormatString("SELECT NULL AS GroupID, NULL AS EMRID, NULL AS PicID, PatientsT.UserDefinedID AS PatientID, "
				"AppointmentsT.PatientID AS PatID, dbo.GetResourceString(AppointmentsT.ID) AS ProvName, PersonT.Last AS PatientLast, PersonT.First AS PatientFirst, "
				"PersonT.Middle AS PatientMiddle, NULL AS PatientAge, PersonT.BirthDate, PersonT.Gender AS PatientGender, "
				"'<No EMN on appt. date>' AS Description, NULL AS StatusID, 'Appt.' AS Status, NULL AS LocName, "
				"AppointmentsT.Date AS Date, NULL AS InputDate, NULL AS ModifiedDate, "
				"'<No EMN>' AS HasBeenBilled, AppointmentsT.ID AS ApptID, StartTime, dbo.GetResourceString(AppointmentsT.ID) AS Resource, "
				"'(' + AptShowStateT.Symbol + ')' AS ShowState, '' AS SecProvName, '' AS TechnicianName, "
				" ISNULL(InsuranceCoPrimQ.Name,'') as ApptPrimaryInsCo, ISNULL(InsuranceCoSecQ.Name,'') as ApptSecondaryInsCo "
				"FROM AppointmentsT "
				"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "

				"LEFT OUTER JOIN AppointmentInsuredPartyT AS AppointmentInsuredPartyPrimQ "
				" ON ((AppointmentInsuredPartyPrimQ.AppointmentID = AppointmentsT.ID) AND  "
                " (AppointmentInsuredPartyPrimQ.Placement = 1)) "
				" LEFT OUTER JOIN InsuredPartyT AS InsuredPartyPrimQ "
				" ON AppointmentInsuredPartyPrimQ.InsuredPartyID = InsuredPartyPrimQ.PersonID "
				"LEFT OUTER JOIN InsuranceCoT AS InsuranceCoPrimQ "
				"ON InsuranceCoPrimQ.PersonID = InsuredPartyPrimQ.InsuranceCoID "
				"LEFT OUTER JOIN AppointmentInsuredPartyT AS AppointmentInsuredPartySecQ "
				"ON ((AppointmentInsuredPartySecQ.AppointmentID = AppointmentsT.ID) AND "
				" (AppointmentInsuredPartySecQ.Placement = 2)) "
				" LEFT OUTER JOIN InsuredPartyT AS InsuredPartySecQ "
				" ON AppointmentInsuredPartySecQ.InsuredPartyID = InsuredPartySecQ.PersonID "
				" LEFT OUTER JOIN InsuranceCoT AS InsuranceCoSecQ "
				" ON InsuranceCoSecQ.PersonID = InsuredPartySecQ.InsuranceCoID "

				"%s %s %s ",				
				strExcludeSql, strDateFilter, strFilter);
		}

	} NxCatchAll("Error generating patients seen statement");
	return "";
}

void CEMRSearch::OnRequeryFinishedEmnlist(short nFlags) 
{
	try {
		if (nFlags != NXDATALIST2Lib::dlRequeryFinishedCompleted) {
			return;
		}
		long nCount = m_dlEMNList->GetRowCount();
		long nAppts = 0;
		bool bSeenToday = false;
		NXDATALIST2Lib::IRowSettingsPtr pDateRow = m_dlDateType->GetCurSel();

		if (pDateRow == NULL) {
			bSeenToday = false;
		} else {
			long nDateType = VarLong(pDateRow->GetValue(egcID), dtlEMNLastModifiedDate);

			if (nDateType == dtlPatientSeen)
				bSeenToday = true;
			else
				bSeenToday = false;
		}
		
		CString strTitle = m_bAdvanced ? "EMR Search and Review" : "Patients Seen Today";

		CWaitCursor cws; 

		// (a.walling 2007-06-28 10:44) - PLID 26353 - But we still need the count, so iterate through and count the number of appointments.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlEMNList->FindByColumn(eelcEMNID, g_cvarNull, NULL, VARIANT_FALSE);
		NXDATALIST2Lib::IRowSettingsPtr pFirstRow = pRow;

		while (pRow) {
			nAppts++;
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = m_dlEMNList->FindByColumn(eelcEMNID, g_cvarNull, pRow->GetNextRow(), VARIANT_FALSE);
			if (pNextRow->IsSameRow(pFirstRow) == VARIANT_TRUE) {
				break;
			}
			pRow = pNextRow;
		}

		long nEMNs = nCount - nAppts;

		if (nEMNs >= 0) {
			CString strNum;
			strNum.Format(" - %li EMNs", nEMNs);
			strTitle += strNum;
		}

		if (bSeenToday && (nAppts > 0) ) {
			// (j.gruber 2013-05-20 12:01) - PLID 43442 - if we are in advanced mode, add "filtered"
			strTitle += FormatString(" - %li Appointments without EMNs%s", nAppts, m_bAdvanced ? " in the date range" : "");
		}

		if ( (nCount == 0) && m_bNoStatus) {
			strTitle += " - Check at least one status box to display results!";
		}

		CString strOldTitle;
		GetWindowText(strOldTitle);

		if (strOldTitle != strTitle)
		{
			SetWindowText(strTitle);
		}

		// (j.jones 2014-05-15 17:29) - PLID 62171 - If we have a cached EMN ID, try to select it.
		// If it doesn't exist, try to select the next EMN ID.
		if (m_nCurEMNID != -1) {

			//briefly disable the redraw while we deal with resetting rows,
			//so it doesn't look like the datalist is bouncing all over the place
			m_dlEMNList->SetRedraw(VARIANT_FALSE);

			NXDATALIST2Lib::IRowSettingsPtr pFoundRow = m_dlEMNList->SetSelByColumn(eelcEMNID, (long)m_nCurEMNID);
			if (pFoundRow == NULL && m_nNextEMNID != -1) {
				pFoundRow = m_dlEMNList->SetSelByColumn(eelcEMNID, (long)m_nNextEMNID);
			}

			//try to get the screen to look similar such that the top row is the same
			if (pFoundRow != NULL && m_nTopRowEMNID != -1) {

				//first jump to the bottom of the list
				m_dlEMNList->EnsureRowInView(m_dlEMNList->GetLastRow());

				NXDATALIST2Lib::IRowSettingsPtr pTopRow = m_dlEMNList->FindByColumn(eelcEMNID, (long)m_nTopRowEMNID, m_dlEMNList->GetFirstRow(), VARIANT_FALSE);
				if (pTopRow) {
					//ensure you can see this row
					m_dlEMNList->EnsureRowInView(pTopRow);
				}

				//now ensure you can see the selected row
				m_dlEMNList->EnsureRowInView(pFoundRow);
			}

			//redraw the list now
			m_dlEMNList->SetRedraw(VARIANT_TRUE);
		}

	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2013-10-17 09:04) - PLID 59061 - This dialog no longer responds to table checkers
//LRESULT CEMRSearch::OnTableChangedEx(WPARAM wParam, LPARAM lParam)

// (j.gruber 2006-12-29 09:53) - PLID 24034 - make a report
void CEMRSearch::OnEmrsearchPreview() 
{
	try {
		// (a.walling 2010-01-11 14:59) - PLID 31482
		if (m_pEMRPreviewPopupDlg) {
			m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
		}

		//get the report ID
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(581)]);

		//get the where clause
		CString strWhere = GenerateWhereClause();

		CRParameterInfo *pFilter = new CRParameterInfo;
		CPtrArray paramList;
		COleDateTime dtTemp;
		COleDateTime dtNULL;
		dtNULL.SetDate(1500,01,01);

		// (a.walling 2007-04-18 11:22) - PLID 25697 - Need to check that pRow is valid before calling its member functions.
		// the same solution needs to apply anywhere in this function where we get a row.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlDateType->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"DateFilter";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		dtTemp = VarDateTime(m_dtFrom.GetValue(), dtNULL);
		if (dtTemp != dtNULL ) {
			pFilter->m_Data = FormatDateTimeForInterface(dtTemp, NULL, dtoDate, false);
		}
		else {
			pFilter->m_Data = "<None>";
		}
		pFilter->m_Name = (CString)"FromDate";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		dtTemp = VarDateTime(m_dtTo.GetValue(), dtNULL);
		if (dtTemp != dtNULL ) {
			pFilter->m_Data = FormatDateTimeForInterface(dtTemp, NULL, dtoDate, false);
		}
		else {
			pFilter->m_Data = "<None>";
		}

		pFilter->m_Name = (CString)"ToDate";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		pRow = m_dlProcedure->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"ProcedureFilter";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		pRow = m_dlProblems->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"ProblemFilter";
		paramList.Add((void *)pFilter);

		// (a.wilson 2014-03-28) PLID 60780 - handle new diagnosis setup for preview.
		pFilter = new CRParameterInfo;
		{
			long nRowCount = m_dlDiagnosisList->GetRowCount();
			if (m_btnDiagnosisIncludeNone.GetCheck() == BST_CHECKED) {
				pFilter->m_Data = ("<None>");
			} else if (nRowCount == 0) {
				pFilter->m_Data = ("<All>");
			} else if (nRowCount > 1) {
				pFilter->m_Data = ("<Multiple>");
			} else if (nRowCount == 1) {
				pRow = m_dlDiagnosisList->GetFirstRow();
				pFilter->m_Data = (pRow ? VarString(pRow->GetValue(dlcCode), "<None>") : "<None>");
			} else {
				pFilter->m_Data = ("<None>");
			}
			pFilter->m_Name = (CString)"DiagFilter";
			paramList.Add((void *)pFilter);
		}

		pFilter = new CRParameterInfo;
		pRow = m_dlProvider->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"ProvFilter";
		paramList.Add((void *)pFilter);

		// (z.manning 2008-11-12 16:04) - PLID 26167 - Secondary provider filter
		pFilter = new CRParameterInfo;
		pRow = m_dlSecProvider->GetCurSel();
		pFilter->m_Data = pRow != NULL ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"SecProvFilter";
		paramList.Add((void*)pFilter);

		// (d.lange 2011-04-25 09:39) - PLID 43381 - Assistant/Technician filter
		pFilter = new CRParameterInfo;
		pRow = m_dlTechnician->GetCurSel();
		pFilter->m_Data = pRow != NULL ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"TechnicianFilter";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		pRow = m_dlLocation->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"LocationFilter";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		pRow = m_dlFilter->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"FilterFilter";
		paramList.Add((void *)pFilter);

		// (j.dinatale 2013-02-05 17:04) - PLID 54777 - appt ref phys filter
		pFilter = new CRParameterInfo;
		pRow = m_dlApptRefPhys->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"AptRefPhyFilter";
		paramList.Add((void *)pFilter);

		// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart & category filters
		pFilter = new CRParameterInfo;
		pRow = m_dlChartCombo->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"ChartFilter";
		paramList.Add((void *)pFilter);
		pFilter = new CRParameterInfo;
		pRow = m_dlCategoryCombo->GetCurSel();
		pFilter->m_Data = pRow ? VarString(pRow->GetValue(1), "<None>") : "<None>";
		pFilter->m_Name = (CString)"CategoryFilter";
		paramList.Add((void *)pFilter);
		
		CString strStatus = "";
		if (IsDlgButtonChecked(IDC_STATUS_OPEN)) {
			if (IsDlgButtonChecked(IDC_STATUS_FINISHED)) {
				if (IsDlgButtonChecked(IDC_STATUS_LOCKED)) {
					strStatus = "Opened, Finished, and Locked";
				}
				else {
					strStatus = "Opened and Finished";
				}
			}
			else {
				if (IsDlgButtonChecked(IDC_STATUS_LOCKED)) {
					strStatus = "Opened and Locked";
				}
				else {
					strStatus = "Opened";
				}
			}
		}
		else {
			if (IsDlgButtonChecked(IDC_STATUS_FINISHED)) {
				if (IsDlgButtonChecked(IDC_STATUS_LOCKED)) {
					strStatus = "Finished and Locked";
				}
				else {
					strStatus = "Finished";
				}
			}
			else {
				if (IsDlgButtonChecked(IDC_STATUS_LOCKED)) {
					strStatus = "Locked";
				}
				else {
					strStatus = "<None>";
				}
			}
		}


		pFilter = new CRParameterInfo;
		pFilter->m_Data = strStatus;
		pFilter->m_Name = (CString)"StatusFilter";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		if (m_bAdvanced) {
			pFilter->m_Data = "true";
		}
		else {
			pFilter->m_Data = "false";
		}
		pFilter->m_Name = "IsAdvanced";
		paramList.Add((void *)pFilter);

		if (!strWhere.IsEmpty()) {
			strWhere = " AND " + strWhere;
		}

		// get the seen today patients
		NXDATALIST2Lib::IRowSettingsPtr pDateRow = m_dlDateType->GetCurSel();
		bool bSeenToday;

		if (pDateRow == NULL) {
			bSeenToday = false;
		} else {
			long nDateType = VarLong(pDateRow->GetValue(egcID), dtlEMNLastModifiedDate);

			if (nDateType == dtlPatientSeen)
				bSeenToday = true;
			else
				bSeenToday = false;
		}

		pFilter = new CRParameterInfo;
		pFilter->m_Name = (CString)"ShowSeenPatients";

		if (bSeenToday && !m_bSuppressSeenPatients) {
			strWhere += " UNION ALL " + GenerateSeenSql(true);
			// (j.gruber 2013-05-20 12:07) - PLID 43442 - add filtered
			pFilter->m_Data = FormatString("- showing patients seen without EMNs%s", m_bAdvanced ? " in the date range": "");
		} else {
			pFilter->m_Data = "";
		}
	
		paramList.Add((void *)pFilter);

		infReport.strExtraText = strWhere;
			
		RunReport(&infReport, &paramList, true, (CWnd *)this, infReport.strReportName);
		ClearRPIParameterList(&paramList);
			
		OnClose();
	}NxCatchAll("Error Previewing List");
}

void CEMRSearch::OnShowSeenPatients() 
{
	try {
		m_bSuppressSeenPatients = IsDlgButtonChecked(IDC_SHOW_SEENPATIENTS) ? false : true;

		SetRemotePropertyInt("EMRSearchSuppressSeenPatients", m_bSuppressSeenPatients ? 1 : 0, 0, GetCurrentUserName());

		WarnSeenPatients();

		RefreshList();
	}NxCatchAll(__FUNCTION__); 
}

void CEMRSearch::OnShowSeenpatientsWith() 
{
	try {
		m_bShowSeenPatientsWith = IsDlgButtonChecked(IDC_SHOW_SEENPATIENTS_WITH) ? true : false;

		SetRemotePropertyInt("EMRSearchShowSeenPatientsWith", m_bShowSeenPatientsWith ? 1 : 0, 0, GetCurrentUserName());

		WarnSeenPatients();

		RefreshList();	
	}NxCatchAll(__FUNCTION__); 
}

// (a.walling 2011-01-13 12:00) - PLID 41542
void CEMRSearch::OnExcludePending() 
{
	m_bExcludePending = IsDlgButtonChecked(IDC_EXCLUDE_PENDING) ? true : false;

	SetRemotePropertyInt("EMRSearchExcludePendingAppts", m_bExcludePending ? 1 : 0, 0, GetCurrentUserName());

	WarnSeenPatients();

	RefreshList();	
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CEMRSearch::OnOK()
{
	//Eat the message
}

// (j.jones 2008-09-30 17:30) - PLID 28011 - added function to minimize this window
void CEMRSearch::MinimizeWindow()
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

	}NxCatchAll("Error in CEMRSearch::MinimizeWindow");
}

// (z.manning 2009-05-19 14:10) - PLID 28512
void CEMRSearch::OnBnClickedShowCurrentPatient()
{
	try
	{
		// (z.manning 2009-05-19 15:30) - PLID 28512 - If we're only showing the current patient
		// then the patient filter doesn't really apply, so let's just disable it.
		if(m_btnShowCurrentPatientOnly.GetCheck() == BST_CHECKED) {
			m_dlFilter->SetSelByColumn(egcID, (long)FILTER_ID_ALL);
			GetDlgItem(IDC_LIST_EMRFILTER)->EnableWindow(FALSE);
			m_bShowCurrentPatientOnly = true; 
		}
		else {
			GetDlgItem(IDC_LIST_EMRFILTER)->EnableWindow(TRUE);
			m_bShowCurrentPatientOnly = false; 
		}
		OnSelChosenListEmrfilter(m_dlFilter->GetCurSel());
		// (b.spivey, January 29, 2013) - PLID 48370 - save this. 
		SetRemotePropertyInt("EMRSearchShowCurrentPatientOnly", (m_bShowCurrentPatientOnly ? 1 : 0), 0, GetCurrentUserName()); 
		RefreshList();

	}NxCatchAll(__FUNCTION__);
}

void CEMRSearch::OnDestroy()
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
// (z.manning 2012-09-10 16:06) - PLID 52543 - Added modified date
void CEMRSearch::ShowPreview(long nPatID, long nEMNID, COleDateTime dtEmnModifiedDate)
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
		m_pEMRPreviewPopupDlg->RestoreSize("EMRSearch");
	}
	
	// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
	// of all available EMN IDs, but since we haven't opened the dialog yet,
	// we can pass in an empty array.
	// (z.manning 2012-09-10 16:06) - PLID 52543 - Use the new EmnPreviewPopup struct
	EmnPreviewPopup emn(nEMNID, dtEmnModifiedDate);
	m_pEMRPreviewPopupDlg->SetPatientID(nPatID, emn);
	m_pEMRPreviewPopupDlg->PreviewEMN(emn, 0);

	// (a.walling 2010-01-11 16:20) - PLID 27733 - Only show if it is not already
	if (!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
		m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOWNA);
	}
}

// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status option
void CEMRSearch::OnStatusOther()
{
	try {

		m_dlOtherStatusCombo->Enabled = m_btnStatusOther.GetCheck();

		// (b.spivey, January 21, 2013) - PLID 48370 - 
		if (m_btnStatusOther.GetCheck() == BST_CHECKED) {
			SetRemotePropertyInt("EMNSearchStatusOther", TRUE, 0, GetCurrentUserName()); 
		}
		else {
			SetRemotePropertyInt("EMNSearchStatusOther", FALSE, 0, GetCurrentUserName()); 
		}

		RefreshList();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status option
void CEMRSearch::OnSelChosenListOtherStatus(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(egcID)) == -1) // multiple
			{
				bool bResult = OpenMultiSelectList(m_dlOtherStatusCombo, m_vaOtherStatus, m_nSelOtherStatus, "ID", "Name", "EMRStatusListT");
				// (b.spivey, January 23, 2013) - PLID 48370 - Store other status ids/selection
				if (bResult) {
					SetRemotePropertyText("EMNSearchStatusOtherIDs", VariantArrayToString(m_vaOtherStatus), 0, GetCurrentUserName()); 
				}
				
			} else {
				m_nSelOtherStatus = VarLong(pRow->GetValue(egcID));
				m_vaOtherStatus.RemoveAll();
				m_vaOtherStatus.Add((_variant_t)m_nSelOtherStatus);
			}
		} else {
			m_dlOtherStatusCombo->SetSelByColumn(egcID, m_nSelOtherStatus);
		}
		SetRemotePropertyInt("EMNSearchStatusOtherFilter", m_nSelOtherStatus, 0, GetCurrentUserName()); 
		RefreshList();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-07-07 10:17) - PLID 44451 - added "Other" status option
void CEMRSearch::OnRequeryFinishedListOtherStatus(short nFlags)
{
	try {

		/*
		long nCount = m_dlOtherStatusCombo->GetRowCount();
	
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlOtherStatusCombo->GetNewRow();
		pRow->PutValue(egcID, (long)lsvAllID);
		pRow->PutValue(egcName, " <All> ");
		m_dlOtherStatusCombo->AddRowSorted(pRow, NULL);

		//do not add a Multiple option if we don't have multiple "other" statuses
		if(nCount > 1) {
			pRow = m_dlOtherStatusCombo->GetNewRow();
			pRow->PutValue(egcID, (long)lsvMultipleID);
			pRow->PutValue(egcName, " <Multiple> ");
			m_dlOtherStatusCombo->AddRowSorted(pRow, NULL);
		}
		*/

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-10-31 10:48) - PLID 44594
void CEMRSearch::OnBnClickedEmrsearchSignSelected()
{
	try
	{
		// (z.manning 2012-07-03 10:07) - PLID 50958 - Ensure the user has EMR write permission
		if(!CheckCurrentUserPermissions(bioPatientEMR, sptWrite)) {
			return;
		}

		if(m_dlEMNList->FindByColumn(eelcCheckForSignature, g_cvarTrue, NULL, VARIANT_FALSE) == NULL) {
			MessageBox("You must select at least one EMN.");
			return;
		}
		
		// (j.dinatale 2012-12-31 12:07) - PLID 52926 - check the require password preference
		if(GetRemotePropertyInt("SignatureCheckPasswordEMR", 1, 0, GetCurrentUserName()) == 1) {
			if(!CheckCurrentUserPassword()) {
				return;
			}
		}

		// (j.dinatale 2013-01-31 16:49) - PLID 54911 - check and see if they have a default signature set up
		ADODB::_RecordsetPtr rsDefSigCheck = CreateParamRecordset(
			"SELECT CAST(CASE WHEN SignatureData IS NULL THEN 0 ELSE 1 END AS BIT) AS HasStampData, "
			"SignatureFile FROM UsersT WHERE PersonID = {INT}", GetCurrentUserID());

		if(!rsDefSigCheck->eof){
			BOOL bHasStampData = AdoFldBool(rsDefSigCheck, "HasStampData", FALSE);
			CString strSigFile = AdoFldString(rsDefSigCheck, "SignatureFile", "");

			if(!bHasStampData || strSigFile.IsEmpty() || !FileUtils::DoesFileOrDirExist(GetSharedPath() ^ "Images" ^ strSigFile)){
				this->MessageBox("Please configure your default signature before attempting to sign EMNs.", "Error!", MB_OK | MB_ICONEXCLAMATION);
				return;
			}
		}
		rsDefSigCheck->Close();

		CArray<long, long> arynEmnIDsToLock;
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlEMNList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			long nEmnID = VarLong(pRow->GetValue(eelcEMNID), -1);
			if (nEmnID != -1 && VarBool(pRow->GetValue(eelcCheckForSignature), FALSE)) {
				arynEmnIDsToLock.Add(nEmnID);
			}
		}
		
		long nStatusPref = GetRemotePropertyInt("EmrPostSignatureInsertStatus", -1, 0, GetCurrentUserName());
		CString strPrompt = FormatString("Are you sure you want to sign the selected EMN(s)?");
		if(nStatusPref == 2) {
			strPrompt += "\r\n\r\nBecause of your preferences this will also attempt to lock the EMN(s).";
		}
		int nResult = MessageBox(strPrompt, NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return;
		}

		if (nStatusPref == 2)
		{
			// (z.manning 2016-01-13 15:53) - PLID 67778 - Prompt if any incomplete to-dos if we'll be locking the EMNs
			if (!PromptIfAnyOutstandingTodos(GetRemoteData(), arynEmnIDsToLock, GetSafeHwnd())) {
				return;
			}
		}
	
		CWaitCursor wc;

		CString strFailures;
		std::map<CString, CString> mapErrorTypeToMessage;	// (j.dinatale 2013-01-31 15:27) - PLID 54911 - keep track of errors per error type, removed the bFailed flag.
		CString strLockedEMNs;	// (j.dinatale 2013-01-31 15:27) - PLID 54911
		CString strNoCPTs; // (d.singleton 2013-07-25 15:33) - PLID 44840
		CString strNoDiags; // (d.singleton 2013-07-25 15:33) - PLID 44840
		CString strUnsettledPrescriptions; // (b.savon 2015-12-29 13:46) - PLID 58470
		CArray<long,long> arynSignedEmnIDs, arynPatientIDs;
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlEMNList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			const long nEmnID = VarLong(pRow->GetValue(eelcEMNID), -1);
			const long nStatusID = VarLong(pRow->GetValue(eelcStatusID), -1);
			CString strDate;
			_variant_t varDate = pRow->GetValue(eelcDate);
			if(varDate.vt == VT_DATE) {
				strDate = FormatDateTimeForInterface(VarDateTime(varDate), 0, dtoDate);
			}
			const CString strEmnDescription = FormatString("  -- %s - %s - %s", strDate
				, VarString(pRow->GetValue(eelcPatient), ""), VarString(pRow->GetValue(eelcDescription), ""));

			if(nEmnID != -1 && VarBool(pRow->GetValue(eelcCheckForSignature), FALSE))
			{
				// (j.jones 2013-08-16 15:19) - PLID 58073 - added a boolean to track whether
				// we aborted signing this EMN for any reason
				bool bSignEMN = true;

				if(nStatusID == 2) {
					// (z.manning 2012-07-05 16:08) - PLID 50958 - Do not try to sign locked EMNs.
					// (j.dinatale 2013-01-31 17:03) - PLID 54911 - need to show an error message that these couldnt be signed.
					if(strLockedEMNs.IsEmpty()){
						strLockedEMNs = "The following EMN(s) were locked and could not be signed:\r\n";
					}

					strLockedEMNs += (strEmnDescription + "\r\n");

					// (j.jones 2013-08-16 15:19) - PLID 58073 - don't try to sign this EMN
					bSignEMN = false;
				}
				// (d.singleton 2013-07-25 15:25) - PLID 44840 - Having a pop up to warn doctor a diagnosis code and/or CPT needs to be selected before locking an EMN
				else if(nStatusPref == 2) {
					if(GetRemotePropertyInt("RequireCPTCodeEMNLocking", 0, 0, "<None>", true) && !ReturnsRecordsParam("SELECT * FROM EMRChargesT WHERE EMRID = {INT} AND Deleted = 0", nEmnID)) {
						if(strNoCPTs.IsEmpty()) {
							strNoCPTs = "The following EMN(s) had no CPT codes selected, and therefore will not be locked:\r\n";
						}
						strNoCPTs += (strEmnDescription + "\r\n");

						// (j.jones 2013-08-16 15:19) - PLID 58073 - don't try to sign this EMN
						bSignEMN = false;
					}
					if(GetRemotePropertyInt("RequireDiagCodeEMNLocking", 0, 0, "<None>", true) && !ReturnsRecordsParam("SELECT * FROM EMRDiagCodesT WHERE EMRID = {INT} AND Deleted = 0", nEmnID)) {
						if(strNoDiags.IsEmpty()) {
							strNoDiags = "The following EMN(s) had no Diagnosis codes selected, and therefore will not be locked:\r\n";
						}
						strNoDiags += (strEmnDescription + "\r\n");

						// (j.jones 2013-08-16 15:19) - PLID 58073 - don't try to sign this EMN
						bSignEMN = false;
					}

					// (b.savon 2015-12-29 10:18) - PLID 58470 - Do not allow an EMN to be locked if it has an incomplete Rx associated with it.
					if (HasUnsettledPrescriptions(nEmnID, false)) {

						if (strUnsettledPrescriptions.IsEmpty()) {
							strUnsettledPrescriptions = "The following EMN(s) had unsettled prescriptions and cannot be locked until they are resolved:\r\n";
						}
						
						strUnsettledPrescriptions += strEmnDescription + "\r\n";

						bSignEMN = false;
					}
					else {
						// No unsettled prescriptions
					}
				}

				// (j.jones 2013-08-16 15:19) - PLID 58073 - only try to sign the EMN if
				// bSignEMN has not been set to false
				if(bSignEMN) {
					try {
						// (z.manning 2012-08-31 09:53) - PLID 51377 - This now uses the API instead of the dashboard/NexTech.COM stuff.
						GetAPI()->SignEMNAndReleaseAccess(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nEmnID));
						arynSignedEmnIDs.Add(nEmnID);
						long nPatientID = VarLong(pRow->GetValue(eelcPatientID), -1);
						if(nPatientID != -1 && !IsIDInArray(nPatientID, arynPatientIDs)) {
							arynPatientIDs.Add(nPatientID);
						}
					}
					catch(_com_error e) {
						CString strExceptionDescription = NxCatch::GetErrorMessage();

						// (z.manning 2012-07-05 14:57) - PLID 50958 - Even before when we did this all in Practice, we never
						// gave details about why this failed. There are just too many things that could go wrong with this
						// that it isn't yet worthing handling them all. That's why we ignore exceptions here.
						// (j.dinatale 2013-01-31 15:27) - PLID 54911 - log for posterity in case for some reason they call us
						Log("%s: %s\n", strEmnDescription, strExceptionDescription);
						
						// (j.dinatale 2013-01-31 15:29) - PLID 54911 - use the new ProcessAPIComError to determine if this is
						//		a NextechSqlException or some other exception
						CString strErrorType, strErrorCode, strWarningMessage;

						CString strMessage;

						if(ProcessAPIComError(e, strErrorType, strErrorCode, strWarningMessage)){
							// details successfully gathered from the exception
						}else{
							// instead of throwing the error, let's just gather the message and show it at the end as well
							strErrorType = FormatString("_com_error(0x%08x, %s)", e.Error(), (LPCTSTR)e.Source());
							strWarningMessage = strExceptionDescription;
						}

						std::map<CString, CString>::const_iterator it = mapErrorTypeToMessage.find(strErrorType);
						if (it == mapErrorTypeToMessage.end()) {
							strMessage = strWarningMessage + "\r\n";
						}
						else {
							strMessage = it->second;
						}

						strMessage += (strEmnDescription + "\r\n");
						mapErrorTypeToMessage[strErrorType] = strMessage;
					}
				}
			}
		}
		
		if(nStatusPref != -1) {
			// (z.manning 2012-07-05 14:59) - PLID 50958 - We may have updated the status of EMNs we just signed so
			// let's send table checkers for them.
			foreach (long nPatientID, arynPatientIDs) {
				CClient::RefreshTable(NetUtils::EMRMasterT, nPatientID);
			}
		}

		RefreshList(true);

		// (j.dinatale 2013-01-31 15:28) - PLID 54911 - construct a message based upon the errors we received.
		CString strMessage;
		if(arynSignedEmnIDs.GetCount()){
			strMessage = AsString(arynSignedEmnIDs.GetCount()) + " EMN(s) were successfully signed.\r\n\r\n";
		}else{
			strMessage = "No EMNs were successfully signed!\r\n\r\n";
		}

		if(!strLockedEMNs.IsEmpty()){
			strMessage += (strLockedEMNs + "\r\n");
		}

		// (d.singleton 2013-07-25 15:25) - PLID 44840 - Having a pop up to warn doctor a diagnosis code and/or CPT needs to be selected before locking an EMN
		if(!strNoCPTs.IsEmpty()) {
			strMessage += (strNoCPTs + "\r\n");
		}
		if(!strNoDiags.IsEmpty()) {
			strMessage += (strNoDiags + "\r\n");
		}

		// (b.savon 2015-12-29 14:18) - PLID 58470 - Do not allow an EMN to be locked if it has an incomplete Rx associated with it. -- Practice side
		if (!strUnsettledPrescriptions.IsEmpty()) {
			strMessage += strUnsettledPrescriptions + "\r\n";
		}
		else {
			//No Unsettled prescriptions
		}

		foreach(CString strErrorMessage, mapErrorTypeToMessage | boost::adaptors::map_values){
			strMessage += (strErrorMessage + "\r\n");
		}

		// (j.dinatale 2013-01-31 15:28) - PLID 54911 - and also, we need to change the icon on the messagebox so that way its noticable to the user
		MessageBox(strMessage.Trim(), NULL, (!mapErrorTypeToMessage.size() && strLockedEMNs.IsEmpty()) ? MB_OK|MB_ICONINFORMATION : MB_OK|MB_ICONERROR);
	}
	NxCatchAll(__FUNCTION__)
}

// (b.spivey, January 18, 2013) - PLID 48370 - Load all previous selections from the filters. Take into account 
//		there may not be selections made previously. 
void CEMRSearch::LoadPreviousFilters() 
{
	long nProperty = GetRemotePropertyInt("EMRSearchDateTypeFilter", -1, 0, GetCurrentUserName(), true);	
	if (nProperty != -1) {
		m_dlDateType->SetSelByColumn(egcID, nProperty); 
	}

	nProperty = GetRemotePropertyInt("EMRSearchProcedureFilter", lsvAllID, 0, GetCurrentUserName(), true); 
	m_nSelProcedure = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchProcedureIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaProcedure, strIDs); 
		m_dlProcedure->SetSelByColumn(egcID, nProperty); 

		// (b.spivey, January 30, 2013) - PLID 48370 - Set the any/all switch. 
		nProperty = GetRemotePropertyInt("EMRSearchAllOrAnyProcedureFilter", 0, 0, GetCurrentUserName(), true); 
		m_bProcedureAny = (nProperty == 1 ? true : false); 
	}
	else {
		m_vaProcedure.Add(_variant_t(nProperty));
		m_dlProcedure->SetSelByColumn(egcID, nProperty); 
	}

	// (a.wilson 2014-02-28 12:47) - PLID 60780 - load defaults for new list.
	{
		long nFilterNone = GetRemotePropertyInt("EMRSearchDiagnosisFilterNone", 0, 0, GetCurrentUserName(), true);
		CString strIDs = GetRemotePropertyText("EMRSearchDiagnosisIDs", "", 0, GetCurrentUserName(), true);
		FillVariantArrayFromString(m_vaDiagnosis, strIDs);
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT ID, CodeNumber, CodeDesc FROM DiagCodes WHERE ID IN ({INTSTRING})", strIDs);
		while (!prs->eof) {
			NXDATALIST2Lib::IRowSettingsPtr pLoadRow = m_dlDiagnosisList->GetNewRow();
			pLoadRow->PutValue(dlcID, AdoFldLong(prs, "ID"));
			pLoadRow->PutValue(dlcCode, _bstr_t(AdoFldString(prs, "CodeNumber", "")));
			pLoadRow->PutValue(dlcDescription, _bstr_t(AdoFldString(prs, "CodeDesc", "")));
			m_dlDiagnosisList->AddRowAtEnd(pLoadRow, NULL);
			prs->MoveNext();
		}
		m_btnDiagnosisIncludeNone.SetCheck(nFilterNone == 1 ? BST_CHECKED : BST_UNCHECKED);
		
		if (m_btnDiagnosisIncludeNone.GetCheck() == BST_CHECKED) {
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_SEARCH)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_SEARCH)->EnableWindow(TRUE);
		}
	}

	nProperty = GetRemotePropertyInt("EMRSearchProblemsFilter", lsvAllID, 0, GetCurrentUserName(), true);
	m_nSelProblems = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchProblemsIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaProblems, strIDs); 
		m_dlProblems->SetSelByColumn(egcID, nProperty); 
	}
	else {
		m_vaProblems.Add(_variant_t(nProperty));
		m_dlProblems->SetSelByColumn(egcID, nProperty); 
	}

	nProperty = GetRemotePropertyInt("EMRSearchProviderFilter", lsvAllID, 0, GetCurrentUserName(), true);
	m_nSelProvider = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchProviderIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaProvider, strIDs); 
		m_dlProvider->SetSelByColumn(egcID, nProperty); 
	}
	else {
		m_vaProvider.Add(_variant_t(nProperty));
		m_dlProvider->SetSelByColumn(egcID, nProperty); 
	}
	
	nProperty = GetRemotePropertyInt("EMRSearchSecondaryProviderFilter", lsvAllID, 0, GetCurrentUserName(), true);
	m_nSelSecProvider = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchSecondaryProviderIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaSecProvider, strIDs); 
		m_dlSecProvider->SetSelByColumn(egcID, nProperty); 
	}
	else {
		m_vaSecProvider.Add(_variant_t(nProperty));
		m_dlSecProvider->SetSelByColumn(egcID, nProperty); 
	}

	nProperty = GetRemotePropertyInt("EMRSearchAssistTechFilter", lsvAllID, 0, GetCurrentUserName(), true);
	m_nSelTechnician = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchAssistTechIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaTechnician, strIDs); 
		m_dlTechnician->SetSelByColumn(egcID, nProperty); 
	}
	else {
		m_vaTechnician.Add(_variant_t(nProperty));
		m_dlTechnician->SetSelByColumn(egcID, nProperty); 
	}

	nProperty = GetRemotePropertyInt("EMRSearchLocationFilter", lsvAllID, 0, GetCurrentUserName(), true);
	m_nSelLocation = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchLocationIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaLocation, strIDs); 
		m_dlLocation->SetSelByColumn(egcID, nProperty); 
	}
	else {
		m_vaLocation.Add(_variant_t(nProperty));
		m_dlLocation->SetSelByColumn(egcID, nProperty); 
	}

		// (b.spivey, January 30, 2013) - PLID 48370 - If this is on, then the filter controls shouldn't be.
	m_bShowCurrentPatientOnly = (GetRemotePropertyInt("EMRSearchShowCurrentPatientOnly", FALSE, 0, GetCurrentUserName(), true) ? true : false);
	if(m_bShowCurrentPatientOnly) {
		m_btnShowCurrentPatientOnly.SetCheck(m_bShowCurrentPatientOnly); 
		GetDlgItem(IDC_LIST_EMRFILTER)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMRSEARCH_EDIT_FILTER)->EnableWindow(FALSE);
	}
	nProperty = GetRemotePropertyInt("EMRSearchLetterWritingFilter", 0, 0, GetCurrentUserName(), true); 
	if(nProperty > 0 && m_bShowCurrentPatientOnly) {
		m_nFilterID = nProperty;
		m_dlFilter->SetSelByColumn(egcID, nProperty);
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_dlFilter->CurSel);
		m_strFilterString = VarString(pRow->GetValue(2), "");
	}
	else {
		m_nFilterID = FILTER_ID_ALL;
		m_strFilterString.Empty();
		m_dlFilter->SetSelByColumn(egcID, nProperty);
	}
	
	nProperty = GetRemotePropertyInt("EMNSearchStatusOtherFilter", lsvAllID, 0, GetCurrentUserName(), true); 
	m_nSelOtherStatus = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMNSearchStatusOtherIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaOtherStatus, strIDs);
		m_dlOtherStatusCombo->SetSelByColumn(egcID, nProperty);
	}
	else {
		m_vaOtherStatus.Add(_variant_t(nProperty));
		m_dlOtherStatusCombo->SetSelByColumn(egcID, nProperty); 
	}

	if (GetRemotePropertyInt("EMNSearchStatusOpen", TRUE, 0, GetCurrentUserName(), true)) {
		m_btnStatusOpen.SetCheck(BST_CHECKED);
	}
	else {
		m_btnStatusOpen.SetCheck(BST_UNCHECKED);
	}
		
	if (GetRemotePropertyInt("EMNSearchStatusFinished", TRUE, 0, GetCurrentUserName(), true)) {
		m_btnStatusFinished.SetCheck(BST_CHECKED);
	}
	else {
		m_btnStatusFinished.SetCheck(BST_UNCHECKED);
	}

	if (GetRemotePropertyInt("EMNSearchStatusLocked", TRUE, 0, GetCurrentUserName(), true)) {
		m_btnStatusLocked.SetCheck(BST_CHECKED);
	}
	else {
		m_btnStatusLocked.SetCheck(BST_UNCHECKED);
	}

	if (GetRemotePropertyInt("EMNSearchStatusOther", TRUE, 0, GetCurrentUserName(), true)) {
		m_btnStatusOther.SetCheck(BST_CHECKED);
	}
	else {
		m_btnStatusOther.SetCheck(BST_UNCHECKED);
		GetDlgItem(IDC_LIST_OTHER_STATUS)->EnableWindow(FALSE); 
	}

	// (j.dinatale 2013-01-23 15:47) - PLID 54777 - appt ref phys filter
	nProperty = GetRemotePropertyInt("EMRSearchApptRefPhysFilter", lsvAllID, 0, GetCurrentUserName(), true);
	m_nSelApptRefPhys = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchApptRefPhysIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaApptRefPhys, strIDs); 
		m_dlApptRefPhys->SetSelByColumn(egcID, nProperty); 
	}
	else {
		m_vaApptRefPhys.Add(_variant_t(nProperty));
		m_dlApptRefPhys->SetSelByColumn(egcID, nProperty); 
	}

	// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart filter
	nProperty = GetRemotePropertyInt("EMRSearchChartFilter", lsvAllID, 0, GetCurrentUserName(), true);
	m_nSelChart = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchChartIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaChart, strIDs); 
		m_dlChartCombo->SetSelByColumn(egcID, nProperty); 
	}
	else {
		m_vaChart.Add(_variant_t(nProperty));
		m_dlChartCombo->SetSelByColumn(egcID, nProperty); 
	}

	// (j.jones 2013-07-15 15:09) - PLID 57477 - added category filter
	nProperty = GetRemotePropertyInt("EMRSearchCategoryFilter", lsvAllID, 0, GetCurrentUserName(), true);
	m_nSelCategory = nProperty; 
	if (nProperty == lsvMultipleID) {
		CString strIDs = GetRemotePropertyText("EMRSearchCategoryIDs", "", 0, GetCurrentUserName(), true); 
		FillVariantArrayFromString(m_vaCategory, strIDs); 
		m_dlCategoryCombo->SetSelByColumn(egcID, nProperty); 
	}
	else {
		m_vaCategory.Add(_variant_t(nProperty));
		m_dlCategoryCombo->SetSelByColumn(egcID, nProperty); 
	}
}

// (b.spivey, January 23, 2013) - PLID 48370 - Fill variant array from a string of IDs. 
void CEMRSearch::FillVariantArrayFromString(CVariantArray &vary, CString strValues) 
{
	if (vary.GetCount() > 0) {
		vary.RemoveAll(); 
	}

	CArray<long, long> caryValues;
	StringAsArray(strValues, caryValues, true);

	for (int i = 0; i < caryValues.GetCount(); i++) {
		vary.Add(_variant_t(caryValues[i])); 
	}
}

// (j.dinatale 2013-01-24 09:07) - PLID 54777 - refresh the list when a selection is made
void CEMRSearch::SelChosenListApptRefPhys(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(egcID)) == lsvMultipleID) // multiple
			{
				bool bResult = OpenMultiSelectList(m_dlApptRefPhys, m_vaApptRefPhys, m_nSelApptRefPhys, "ID", "Last + ', ' + First + ' ' + Middle", "Appt. Ref. Phys.");
				if (bResult && m_vaApptRefPhys.GetCount() > 0) {
					SetRemotePropertyText("EMRSearchApptRefPhysIDs", VariantArrayToString(m_vaApptRefPhys), 0, GetCurrentUserName()); 
				}
			} else {
				m_nSelApptRefPhys = VarLong(pRow->GetValue(egcID));
				m_vaApptRefPhys.RemoveAll();
				m_vaApptRefPhys.Add((_variant_t)m_nSelApptRefPhys);
			}
		} else {
			m_dlApptRefPhys->SetSelByColumn(egcID, m_nSelApptRefPhys);
		}
		SetRemotePropertyInt("EMRSearchApptRefPhysFilter", m_nSelApptRefPhys, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-15 14:49) - PLID 57477 - added chart filter
void CEMRSearch::OnSelChosenComboChart(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(egcID)) == lsvMultipleID) // multiple
			{
				bool bResult = OpenMultiSelectList(m_dlChartCombo, m_vaChart, m_nSelChart, "ID", "Description", "Chart");
				if (bResult && m_vaChart.GetCount() > 0) {
					SetRemotePropertyText("EMRSearchChartIDs", VariantArrayToString(m_vaChart), 0, GetCurrentUserName()); 
				}
			} else {
				m_nSelChart = VarLong(pRow->GetValue(egcID));
				m_vaChart.RemoveAll();
				m_vaChart.Add((_variant_t)m_nSelChart);
			}
		} else {
			m_dlChartCombo->SetSelByColumn(egcID, m_nSelChart);
		}
		SetRemotePropertyInt("EMRSearchChartFilter", m_nSelChart, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-15 14:49) - PLID 57477 - added category filter
void CEMRSearch::OnSelChosenComboCategories(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(egcID)) == lsvMultipleID) // multiple
			{
				bool bResult = OpenMultiSelectList(m_dlCategoryCombo, m_vaCategory, m_nSelCategory, "ID", "Description", "Category");
				if (bResult && m_vaCategory.GetCount() > 0) {
					SetRemotePropertyText("EMRSearchCategoryIDs", VariantArrayToString(m_vaCategory), 0, GetCurrentUserName()); 
				}
			} else {
				m_nSelCategory = VarLong(pRow->GetValue(egcID));
				m_vaCategory.RemoveAll();
				m_vaCategory.Add((_variant_t)m_nSelCategory);
			}
		} else {
			m_dlCategoryCombo->SetSelByColumn(egcID, m_nSelCategory);
		}
		SetRemotePropertyInt("EMRSearchCategoryFilter", m_nSelCategory, 0, GetCurrentUserName()); 
		RefreshList();
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-02-05 09:49) - PLID 60580 - Refresh automatically doesn't work on Patients Seen Today.
void CEMRSearch::OnSize(UINT nType, int cx, int cy)
{
	try{
		CNxDialog::OnSize(nType, cx, cy);

		//Only Refresh if the dialog is initialized and we're restoring
		// (j.jones 2014-05-15 17:01) - PLID 61798 - now there is a checkbox to control
		// whether or not we refresh upon restoring
		// (e.frazier 2016-05-04 17:03) - PLID-66719 - The dialog now refreshes when it is maximized as well
		if (m_bDialogInitialized && (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED) && m_bRefreshOnWindowRestore) {
			RefreshList(true);
		}

	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-02-28 12:21) - PLID 60780 - add diagnosis to list.
void CEMRSearch::SelChosenEmrSearchDiagnosisSearch(LPDISPATCH lpRow)
{
	try {
		if (lpRow == NULL)
			return;

		long nDiagID = -1;
		CString strDiagCode, strDiagDescription;
		CDiagSearchResults results = DiagSearchUtils::ConvertDualSearchResults(lpRow);
		//Get the code that was selected, it could be either a 9 or 10 code not both.
		if (results.m_ICD9.m_nDiagCodesID != -1) {
			nDiagID = results.m_ICD9.m_nDiagCodesID;
			strDiagCode = results.m_ICD9.m_strCode;
			strDiagDescription = results.m_ICD9.m_strDescription;
		} else if (results.m_ICD10.m_nDiagCodesID != -1) {
			nDiagID = results.m_ICD10.m_nDiagCodesID;
			strDiagCode = results.m_ICD10.m_strCode;
			strDiagDescription = results.m_ICD10.m_strDescription;
		} else {
			return;
		}
		//check if the code is already in the list. if it is then don't bother adding it.
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlDiagnosisList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			if (VarLong(pRow->GetValue(dlcID)) == nDiagID) {
				return;
			}
		}
		m_vaDiagnosis.Add(_variant_t(nDiagID));
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlDiagnosisList->GetNewRow();
		pNewRow->PutValue(dlcID, nDiagID);
		pNewRow->PutValue(dlcCode, _bstr_t(strDiagCode));
		pNewRow->PutValue(dlcDescription, _bstr_t(strDiagDescription));
		m_dlDiagnosisList->AddRowAtEnd(pNewRow, NULL);

		SetRemotePropertyText("EMRSearchDiagnosisIDs", (m_vaDiagnosis.GetCount() == 0 ? "" : VariantArrayToString(m_vaDiagnosis)), 
			0, GetCurrentUserName()); 
		RefreshList();
		
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-02-28 15:11) - PLID 60873 - remove a code from the list.
void CEMRSearch::RButtonDownEmrSearchDiagnosisList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		m_dlDiagnosisList->PutCurSel(pRow);
		int nCmdID = 0;
		CNxMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "Remove Diagnosis Code");

		CPoint pt;
		GetCursorPos(&pt);
		nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

		//Remove Selection.
		if (nCmdID == 1) {
			long nDiagID = VarLong(pRow->GetValue(dlcID));
			//remove from member variable.
			for (int i = 0; i < m_vaDiagnosis.GetSize(); i++) {
				if (m_vaDiagnosis[i] == _variant_t(nDiagID)) {
					m_vaDiagnosis.RemoveAt(i);
					break;
				}
			}
			//remove from datalist.
			m_dlDiagnosisList->RemoveRow(pRow);
			//update remote property
			SetRemotePropertyText("EMRSearchDiagnosisIDs", (m_vaDiagnosis.GetCount() == 0 ? "" : VariantArrayToString(m_vaDiagnosis)), 
				0, GetCurrentUserName());
			RefreshList();
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-03-04 15:22) - PLID 60780 - handle checking and unchecking none.
void CEMRSearch::OnBnClickedEmrSearchDiagnosisNone()
{
	try {
		if (m_btnDiagnosisIncludeNone.GetCheck() == BST_CHECKED) {
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_SEARCH)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_EMR_SEARCH_DIAGNOSIS_SEARCH)->EnableWindow(TRUE);
		}
		SetRemotePropertyInt("EMRSearchDiagnosisFilterNone", 
			(m_btnDiagnosisIncludeNone.GetCheck() == BST_CHECKED ? 1 : 0), 0, GetCurrentUserName());
		RefreshList();
	} NxCatchAll(__FUNCTION__);
}

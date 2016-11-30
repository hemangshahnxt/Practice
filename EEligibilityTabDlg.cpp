// EEligibilityTabDlg.cpp : implementation file
//

// (j.jones 2007-05-01 13:51) - PLID 8993 - created E-Eligibility Tab

#include "stdafx.h"
#include "EEligibilityTabDlg.h"
#include "ANSI271Parser.h"
#include "EligibilityRequestDlg.h"
#include "EEligibility.h"
#include "ANSIProperties.h"
#include "EligibilityReviewDlg.h"
#include "EligibilityRequestDetailDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "HistoryDlg.h"
#include "EligibilityCreateForScheduledPatientsDlg.h"
#include "EligibilityResponseFilteringConfigDlg.h"
#include "EligibilityRealTimeSetupDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum EligibilityTabColumn {

	etcID = 0,
	etcPatientID = 1,
	etcUserDefinedID = 2,
	etcPatientName = 3,
	etcInsCoID = 4,
	etcInsGroupID = 5,
	etcInsCoName = 6,
	etcRespName = 7,
	etcProviderID = 8,
	etcProvName = 9,
	etcLastSentDate = 10,
	etcSelected = 11,
};

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define	ID_EDIT_REQUEST			49931
#define ID_REVIEW_REQUEST		49932
#define	ID_REMOVE_REQUEST		49933
#define	ID_SELECT_BY_INSCO		49934
#define	ID_SELECT_BY_INSGROUP	49935
#define	ID_SELECT_BY_PROV		49936
#define	ID_UNSELECT_BY_INSCO	49937
#define	ID_UNSELECT_BY_INSGROUP	49938
#define	ID_UNSELECT_BY_PROV		49939
#define	ID_ELIG_GOTO_PATIENT	49940
#define	ID_SUBMIT_REQUEST		49942	// (j.jones 2010-07-08 09:23) - PLID 39486

/////////////////////////////////////////////////////////////////////////////
// CEEligibilityTabDlg dialog


CEEligibilityTabDlg::CEEligibilityTabDlg(CWnd* pParent)
	: CNxDialog(CEEligibilityTabDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEEligibilityTabDlg)
	m_iCurrList = 0;
	//}}AFX_DATA_INIT

	// (j.jones 2007-10-24 09:28) - PLID 27855 - added help for E-Eligibility
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Billing/Insurance_Billing/Create_Electronic_Eligibility_Requests_and_Import_Responses.htm";
	
}
// (s.dhole 07/23/2012) PLID 48693
CEEligibilityTabDlg::~CEEligibilityTabDlg()
{
	/*GetMainFrame()->IsEligibilityRequestDlgOpen(this); */
}

void CEEligibilityTabDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEEligibilityTabDlg)
	DDX_Control(pDX, IDC_BTN_REVIEW_PAST_REQUESTS, m_btnReviewPastRequests);
	DDX_Control(pDX, IDC_ELIG_EXPORT, m_btnExport);
	DDX_Control(pDX, IDC_IMPORT_RESPONSES, m_btnImportResponses);
	DDX_Control(pDX, IDC_CONFIG, m_btnANSIProperties);
	DDX_Control(pDX, IDC_UNSELECT_ONE_ELIG, m_btnUnselectOne);
	DDX_Control(pDX, IDC_UNSELECT_ALL_ELIG, m_btnUnselectAll);
	DDX_Control(pDX, IDC_SELECT_ONE_ELIG, m_btnSelectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_ELIG, m_btnSelectAll);
	DDX_Control(pDX, IDC_REMOVE_UNSELECTED_ELIG, m_btnUnbatchUnselected);
	DDX_Control(pDX, IDC_REMOVE_ALL_ELIG, m_btnUnbatchAll);
	DDX_Control(pDX, IDC_REMOVE_SELECTED_ELIG, m_btnUnbatchSelected);
	DDX_Control(pDX, IDC_BTN_CREATE_NEW_ELIGIBILITY_REQUEST, m_btnCreateNewRequest);
	DDX_Control(pDX, IDC_ELIG_UNSELECTED_LABEL, m_nxstaticEligUnselectedLabel);
	DDX_Control(pDX, IDC_ELIG_UNSELECTED_TOTAL, m_nxstaticEligUnselectedTotal);
	DDX_Control(pDX, IDC_ELIG_SELECTED_LABEL, m_nxstaticEligSelectedLabel);
	DDX_Control(pDX, IDC_ELIG_SELECTED_TOTAL, m_nxstaticEligSelectedTotal);
	DDX_Control(pDX, IDC_BTN_CREATE_REQUESTS_FOR_SCHEDULED_PATIENTS, m_btnCreateForScheduled);
	DDX_Control(pDX, IDC_BTN_CONFIG_RESPONSE_FILTERING, m_btnConfigResponseFiltering);
	DDX_Control(pDX, IDC_BTN_CONFIG_ELIGIBILITY_REALTIME_SETUP, m_btnRealTimeSetup);
	DDX_Control(pDX, IDC_RADIO_ELIG_PRODUCTION_BATCH, m_radioProduction);
	DDX_Control(pDX, IDC_RADIO_ELIG_TEST_BATCH, m_radioTest);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEEligibilityTabDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEEligibilityTabDlg)
	ON_BN_CLICKED(IDC_IMPORT_RESPONSES, OnImportResponses)
	ON_BN_CLICKED(IDC_ELIG_EXPORT, OnEligExport)
	ON_BN_CLICKED(IDC_BTN_CREATE_NEW_ELIGIBILITY_REQUEST, OnBtnCreateNewEligibilityRequest)
	ON_BN_CLICKED(IDC_CONFIG, OnConfig)
	ON_BN_CLICKED(IDC_REMOVE_ALL_ELIG, OnRemoveAllElig)
	ON_BN_CLICKED(IDC_REMOVE_SELECTED_ELIG, OnRemoveSelectedElig)
	ON_BN_CLICKED(IDC_REMOVE_UNSELECTED_ELIG, OnRemoveUnselectedElig)
	ON_BN_CLICKED(IDC_SELECT_ALL_ELIG, OnSelectAllElig)
	ON_BN_CLICKED(IDC_SELECT_ONE_ELIG, OnSelectOneElig)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_ELIG, OnUnselectAllElig)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_ELIG, OnUnselectOneElig)
	ON_BN_CLICKED(IDC_BTN_REVIEW_PAST_REQUESTS, OnBtnReviewPastRequests)
	ON_BN_CLICKED(IDC_BTN_CREATE_REQUESTS_FOR_SCHEDULED_PATIENTS, OnBtnCreateRequestsForScheduledPatients)
	ON_BN_CLICKED(IDC_BTN_CONFIG_RESPONSE_FILTERING, OnBtnConfigResponseFiltering)	
	ON_BN_CLICKED(IDC_BTN_CONFIG_ELIGIBILITY_REALTIME_SETUP, OnBtnConfigEligibilityRealtimeSetup)
	//}}AFX_MSG_MAP		
	ON_BN_CLICKED(IDC_RADIO_ELIG_TEST_BATCH, OnRadioEligTestBatch)
	ON_BN_CLICKED(IDC_RADIO_ELIG_PRODUCTION_BATCH, OnRadioEligProductionBatch)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEEligibilityTabDlg message handlers

BOOL CEEligibilityTabDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		// (j.jones 2010-04-19 10:25) - PLID 38202 - added preference caching
		g_propManager.CachePropertiesInBulk("CEEligibilityTabDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'Eligibility_ExcludeFromOutputFile', \r\n"
			"	'GEDIEligibilityRealTime_Enabled', \r\n"	// (j.jones 2010-07-06 10:32) - PLID 39486
			"	'GEDIEligibilityRealTime_OpenNotepad', \r\n" // (j.jones 2010-07-06 10:52) - PLID 39499
			"	'GEDIEligibilityRealTime_UnbatchOnReceive', \r\n " // (j.jones 2010-07-06 10:52) - PLID 39499
				// (j.jones 2010-10-21 12:45) - PLID 40914 - added type of clearinghouse, 0 - Trizetto, 1 - ECP
			"	'RealTimeEligibility_Clearinghouse', \r\n"
				// (j.jones 2016-05-16 09:58) - NX-100357 - added PhoneFormatString
			"	'PhoneFormatString', \r\n "
			// (j.jones 2016-05-19 10:27) - NX-100685 - cached FormatStyle
			"	'FormatStyle', \r\n"
			"	'EnvoyProduction', \r\n"
			"	'EligibilityProduction' "
			")"
			, _Q(GetCurrentUserName()));

		// (j.jones 2008-05-07 11:51) - PLID 29854 - set some icons for modernization,
		// but intentionally kept the color scheme from before so as to maintain the
		// button grouping appearance just like the other batch tabs

		m_btnSelectOne.AutoSet(NXB_DOWN);
		m_btnSelectAll.AutoSet(NXB_DDOWN);
		m_btnUnselectOne.AutoSet(NXB_UP);
		m_btnUnselectAll.AutoSet(NXB_UUP);

		m_btnUnbatchUnselected.AutoSet(NXB_DELETE);
		m_btnUnbatchAll.AutoSet(NXB_DELETE);
		m_btnUnbatchSelected.AutoSet(NXB_DELETE);

		m_btnCreateNewRequest.AutoSet(NXB_NEW);
		// (j.jones 2009-09-16 10:28) - PLID 26481 - added ability to mass-create requests
		m_btnCreateForScheduled.AutoSet(NXB_NEW);
		m_btnReviewPastRequests.AutoSet(NXB_MODIFY);
		m_btnANSIProperties.AutoSet(NXB_MODIFY);
		m_btnImportResponses.AutoSet(NXB_MODIFY);
		m_btnExport.AutoSet(NXB_EXPORT);
		// (j.jones 2010-03-25 17:32) - PLID 37905 - added ability to configure filtering how response info. displays
		m_btnConfigResponseFiltering.AutoSet(NXB_MODIFY);
		// (j.jones 2010-07-02 14:37) - PLID 39506 - added setup for the real-time E-Eligibility settings
		m_btnRealTimeSetup.AutoSet(NXB_MODIFY);

		//set colors to more easily separate the three types of options
		m_btnUnbatchUnselected.SetTextColor(RGB(150,0,0));
		m_btnUnbatchAll.SetTextColor(RGB(0,0,150));
		m_btnUnbatchSelected.SetTextColor(RGB(0,100,0));

		//the eligibility test/production setting defaults to the ebilling setting
		long nEbillingProduction = GetRemotePropertyInt("EnvoyProduction", 0, 0, _T("<None>"), true);
		long nEligibilityProduction = GetRemotePropertyInt("EligibilityProduction", nEbillingProduction, 0, _T("<None>"), true);
		if (nEligibilityProduction == 1)
		{
			m_radioProduction.SetCheck(TRUE);
		}
		else
		{
			m_radioTest.SetCheck(TRUE);
		}

		m_UnselectedList = BindNxDataList2Ctrl(IDC_UNSELECTED_ELIG_LIST,false);
		m_SelectedList = BindNxDataList2Ctrl(IDC_SELECTED_ELIG_LIST,false);
		m_FormatCombo = BindNxDataList2Ctrl(IDC_ANSI_FORMAT,true);

		//set up the list queries here, for easier reading
		CString strListFrom = "EligibilityRequestsT "
				"LEFT JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
				"LEFT JOIN ProvidersT ON EligibilityRequestsT.ProviderID = ProvidersT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN PersonT PersonProvidersT ON ProvidersT.PersonID = PersonProvidersT.ID ";

		m_UnselectedList->PutFromClause(_bstr_t(strListFrom));
		m_SelectedList->PutFromClause(_bstr_t(strListFrom));

		m_UnselectedList->PutWhereClause(_bstr_t("Batched = 1 AND Selected = 0"));
		m_SelectedList->PutWhereClause(_bstr_t("Batched = 1 AND Selected = 1"));

		//and now requery
		m_UnselectedList->Requery();
		m_SelectedList->Requery();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEEligibilityTabDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEEligibilityTabDlg)
	ON_EVENT(CEEligibilityTabDlg, IDC_UNSELECTED_ELIG_LIST, 6 /* RButtonDown */, OnRButtonDownUnselectedEligList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEEligibilityTabDlg, IDC_UNSELECTED_ELIG_LIST, 18 /* RequeryFinished */, OnRequeryFinishedUnselectedEligList, VTS_I2)
	ON_EVENT(CEEligibilityTabDlg, IDC_UNSELECTED_ELIG_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedEligList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEEligibilityTabDlg, IDC_SELECTED_ELIG_LIST, 6 /* RButtonDown */, OnRButtonDownSelectedEligList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEEligibilityTabDlg, IDC_SELECTED_ELIG_LIST, 18 /* RequeryFinished */, OnRequeryFinishedSelectedEligList, VTS_I2)
	ON_EVENT(CEEligibilityTabDlg, IDC_SELECTED_ELIG_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedEligList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEEligibilityTabDlg, IDC_ANSI_FORMAT, 18 /* RequeryFinished */, OnRequeryFinishedAnsiFormat, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CEEligibilityTabDlg, IDC_ANSI_FORMAT, 16, OnSelChosenAnsiFormat, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CEEligibilityTabDlg::OnImportResponses() 
{
	try {

		// (j.jones 2007-06-29 09:28) - PLID 23950 - check the license here, with a usage flag
		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility, sptWrite) || !g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrUse)) 
			return;

		// (j.jones 2010-07-07 10:14) - PLID 39499 - aryRequestIDsUpdated will be filled with the IDs we updated
		std::vector<long> aryRequestIDsUpdated;
		//this will be filled with the responses we just imported
		std::vector<long> aryResponseIDsReturned;

		CANSI271Parser parser;
		// (j.jones 2010-07-02 13:52) - PLID 39499 - all actual importing to Practice data is now done inside ParseFile()
		if(parser.ParseFile(aryRequestIDsUpdated, aryResponseIDsReturned)) {

			// (j.jones 2010-07-07 10:22) - PLID 39499 - when we coded the real-time import,
			// we auto-opened the result dialog - so why not just do it for regular imports too?
			// (r.goldschmidt 2014-10-10 12:17) - PLID 62644 - make eligibility request detail dlg modeless
			// (r.goldschmidt 2015-11-12 12:55) - PLID 65363 - account for if parsing opened notepad
			GetMainFrame()->ShowEligibilityRequestDetailDlg(aryRequestIDsUpdated, aryResponseIDsReturned, true);

			//if we ever visually display something on the
			//E-Eligibility Tab that indicates a request has a
			//response, we would need to requery the tab

			// (j.jones 2010-07-07 10:23) - PLID 39499 - not needed anymore since we show the responses immediately
			//AfxMessageBox("The response file has been imported into Practice and linked with the original requests.\n"
			//	"You may review this information later through the 'Review Past Eligibility Requests' screen.");
		}

	}NxCatchAll("Error in CEEligibilityTabDlg::OnImportResponses");
}

void CEEligibilityTabDlg::OnEligExport() 
{
	try {

		// (j.jones 2007-06-29 09:28) - PLID 23950 - check the license here, with a usage flag
		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility,sptRead) || !g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrUse)) 
			return;

		if(m_SelectedList->GetRowCount() == 0) {
			AfxMessageBox("There are no eligibility requests in the selected list. Please select some requests before exporting.");
			return;
		}

		long nFormatID = -1;
		IRowSettingsPtr pRow = m_FormatCombo->GetCurSel();
		if(pRow != NULL)
			nFormatID = VarLong(pRow->GetValue(0), -1);
		else {
			AfxMessageBox("Please select an ANSI Export Style from the list before exporting.");
			return;
		}

		CEEligibility dlg(this);
		dlg.m_FormatID = nFormatID;
		// (j.jones 2010-07-06 09:29) - PLID 39486 - are we sending in real-time?
		// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only
		dlg.m_bUseRealTimeElig = (GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1);

		// (j.jones 2010-07-02 10:58) - PLID 39486 - we do not need to fill m_aryRequestIDsToExport,
		// because if it is empty, we will export all batched, selected requests

		int nResult = dlg.DoModal();

		// (j.jones 2010-07-02 14:10) - PLID 39486 - call UpdateView(), we need to
		// refresh both lists for not just Last Sent Dates, but also we may have auto-unbatched
		// some successfully imported requests
		UpdateView();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnEligExport");
}

void CEEligibilityTabDlg::OnBtnCreateNewEligibilityRequest() 
{
	try {

		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility,sptWrite))
			return;
		
		//CEligibilityRequestDlg dlg(this);

		//// (j.jones 2010-07-08 09:00) - PLID 39515 - pass in the current format, if one is selected
		long nFormatID =-1;
		IRowSettingsPtr pRow = m_FormatCombo->GetCurSel();
		if(pRow != NULL) {
			nFormatID = VarLong(pRow->GetValue(0), -1);
		}
		// (s.dhole 07/23/2012) PLID 48693 
		GetMainFrame()->ShowEligibilityRequestDlg(this,-1,nFormatID ,-1,GetNxColor(GNC_FINANCIAL, 1));
		// (s.dhole 07/23/2012) PLID 48693 Move this code to OnEligibilityRequestDlgClosed
		//if(dlg.DoModal() == IDOK) {

		//	//requery the unselected list, since new requests are auto-batched into this list
		//	m_UnselectedList->Requery();
		//	// (j.jones 2010-07-07 11:31) - PLID 39515 - now new requests could be left as unselected
		//	m_SelectedList->Requery();
		//}

	}NxCatchAll("Error in CEEligibilityTabDlg::OnBtnCreateNewEligibilityRequest");
}


void CEEligibilityTabDlg::OnConfig() 
{
	try {

		long nFormatID = -1;
		IRowSettingsPtr pRow = m_FormatCombo->GetCurSel();
		if(pRow != NULL)
			nFormatID = VarLong(pRow->GetValue(0), -1);

		CANSIProperties dlg(this);
		dlg.m_FormatID = nFormatID;
		dlg.DoModal();

		m_FormatCombo->Requery();

		if(nFormatID != -1)
			m_FormatCombo->SetSelByColumn(0,(long)nFormatID);

	}NxCatchAll("Error in CEEligibilityTabDlg::OnConfig");
}

void CEEligibilityTabDlg::OnRemoveAllElig() 
{
	try {

		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility,sptWrite))
			return;

		if (MessageBox("Are you sure you want to unbatch all Eligibility Requests?","Practice",MB_YESNO|MB_ICONQUESTION) == IDNO)
			return;

		//unbatch everything
		ExecuteSql("UPDATE EligibilityRequestsT SET Batched = 0, Selected = 0 WHERE Batched = 1");

		//and requery both lists
		m_UnselectedList->Requery();
		m_SelectedList->Requery();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnRemoveAllElig");
}

void CEEligibilityTabDlg::OnRemoveSelectedElig() 
{
	try {

		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility,sptWrite))
			return;

		if (MessageBox("Are you sure you want to unbatch the selected Eligibility Requests?","Practice",MB_YESNO|MB_ICONQUESTION) == IDNO)
			return;

		//unbatch all selected requests
		ExecuteSql("UPDATE EligibilityRequestsT SET Batched = 0, Selected = 0 WHERE Batched = 1 AND Selected = 1");

		//and requery the selected list
		m_SelectedList->Requery();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnRemoveSelectedElig");
}

void CEEligibilityTabDlg::OnRemoveUnselectedElig() 
{
	try {

		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility,sptWrite))
			return;

		if (MessageBox("Are you sure you want to unbatch the unselected Eligibility Requests?","Practice",MB_YESNO|MB_ICONQUESTION) == IDNO)
			return;

		//unbatch all unselected requests
		ExecuteSql("UPDATE EligibilityRequestsT SET Batched = 0 WHERE Batched = 1 AND Selected = 0");

		//and requery the unselected list
		m_UnselectedList->Requery();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnRemoveUnselectedElig");
}

void CEEligibilityTabDlg::OnSelectAllElig() 
{
	try {

		m_SelectedList->TakeAllRows(m_UnselectedList);

		UpdateCurrentSelect(1);

		RefreshTotals();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnSelectAllElig");
}

void CEEligibilityTabDlg::OnSelectOneElig() 
{
	try {

		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Changed to use TakeCurrentRowAddSorted() for efficiency
		m_SelectedList->TakeCurrentRowAddSorted(m_UnselectedList, NULL);

		UpdateCurrentSelect(1);

		RefreshTotals();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnSelectOneElig");
}

void CEEligibilityTabDlg::OnUnselectAllElig() 
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

		UpdateCurrentSelect(0);

		RefreshTotals();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnUnselectAllElig");
}

void CEEligibilityTabDlg::OnUnselectOneElig() 
{
	try {

		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Changed to use TakeCurrentRowAddSorted() for efficiency
		m_UnselectedList->TakeCurrentRowAddSorted(m_SelectedList, NULL);

		UpdateCurrentSelect(0);

		RefreshTotals();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnUnselectOneElig");
}

void CEEligibilityTabDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		m_UnselectedList->Requery();
		m_SelectedList->Requery();

		// (j.jones 2016-05-19 10:22) - NX-100685 - load the format type, it may have changed
		long nFormatID = GetDefaultEbillingANSIFormatID();
		m_FormatCombo->SetSelByColumn(0, (long)nFormatID);

	}NxCatchAll(__FUNCTION__);
}

void CEEligibilityTabDlg::OnRButtonDownUnselectedEligList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		// Do nothing if the user didn't actually click in a cell
		if (pRow == NULL) {
			return;
		}

		if(nCol == -1) {
			return;
		}

		CPoint pt;
		GetCursorPos(&pt);

		m_iCurrList = 0;

		//first clear out the multi-selection, if one exists
		m_UnselectedList->PutCurSel(NULL);
		m_UnselectedList->PutCurSel(pRow);

		if (pRow->GetValue(etcID).vt != VT_EMPTY) {

			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			long nIndex = 0;

			// (j.jones 2010-07-08 09:21) - PLID 39486 - if real-time sends are enabled, give the option
			// to submit this request right now
			if(GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SUBMIT_REQUEST, "&Submit Eligibility Request");
			}
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_EDIT_REQUEST, "&Edit Eligibility Request");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_REVIEW_REQUEST, "&Review Eligibility Request");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_REMOVE_REQUEST, "Remove from &Batch");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_BY_INSCO, "Select Requests with this &Insurance Co.");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_BY_INSGROUP, "Select Requests with this Insurance Gr&oup");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_BY_PROV, "Select Requests with this &Provider");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_ELIG_GOTO_PATIENT, "&Go To Patient");

			mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
		}

	}NxCatchAll("Error in CEEligibilityTabDlg::OnRButtonDownUnselectedEligList");
}

void CEEligibilityTabDlg::OnRequeryFinishedUnselectedEligList(short nFlags) 
{
	try {

		CString str;
		str.Format("%li", m_UnselectedList->GetRowCount());
		SetDlgItemText(IDC_ELIG_UNSELECTED_TOTAL,str);

		CRect rc;
		GetDlgItem(IDC_ELIG_UNSELECTED_TOTAL)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

	}NxCatchAll("Error in CEEligibilityTabDlg::OnRequeryFinishedUnselectedEligList");
}

void CEEligibilityTabDlg::OnRButtonDownSelectedEligList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		// Do nothing if the user didn't actually click in a cell
		if (pRow == NULL) {
			return;
		}

		if(nCol == -1) {
			return;
		}

		CPoint pt;
		GetCursorPos(&pt);

		m_iCurrList = 1;

		//first clear out the multi-selection, if one exists
		m_SelectedList->PutCurSel(NULL);
		m_SelectedList->PutCurSel(pRow);

		if (pRow->GetValue(etcID).vt != VT_EMPTY) {

			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			long nIndex = 0;

			// (j.jones 2010-07-08 09:21) - PLID 39486 - if real-time sends are enabled, give the option
			// to submit this request right now
			if(GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SUBMIT_REQUEST, "&Submit Eligibility Request");
			}
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_EDIT_REQUEST, "&Edit Eligibility Request");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_REVIEW_REQUEST, "&Review Eligibility Request");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_REMOVE_REQUEST, "Remove from &Batch");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UNSELECT_BY_INSCO, "Unselect Requests with this &Insurance Co.");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UNSELECT_BY_INSGROUP, "Unselect Requests with this Insurance Gr&oup");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UNSELECT_BY_PROV, "Unselect Requests with this &Provider");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_ELIG_GOTO_PATIENT, "&Go To Patient");

			mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
		}

	}NxCatchAll("Error in CEEligibilityTabDlg::OnRButtonDownSelectedEligList");
}

void CEEligibilityTabDlg::OnRequeryFinishedSelectedEligList(short nFlags) 
{
	try {

		CString str;
		str.Format("%li", m_SelectedList->GetRowCount());
		SetDlgItemText(IDC_ELIG_SELECTED_TOTAL,str);

		CRect rc;
		GetDlgItem(IDC_ELIG_SELECTED_TOTAL)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

	}NxCatchAll("Error in CEEligibilityTabDlg::OnRequeryFinishedSelectedEligList");
}

void CEEligibilityTabDlg::RefreshTotals()
{
	try {

		CString str;
		CRect rc;

		str.Format("%li", m_UnselectedList->GetRowCount());
		SetDlgItemText(IDC_ELIG_UNSELECTED_TOTAL,str);
		
		GetDlgItem(IDC_ELIG_UNSELECTED_TOTAL)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str.Format("%li", m_SelectedList->GetRowCount());
		SetDlgItemText(IDC_ELIG_SELECTED_TOTAL,str);

		GetDlgItem(IDC_ELIG_SELECTED_TOTAL)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

	}NxCatchAll("Error in CEEligibilityTabDlg::RefreshTotals");
}

//UpdateCurrentSelect will take in the ID of one of the lists (0 - Unselected, 1 - Selected)
//and check for any items that were just moved over from the other list. If any
//are marked as being on the other list, we update the data to reflect which list the item belongs in.
void CEEligibilityTabDlg::UpdateCurrentSelect(int iList)
{
	try {

		CString strSqlBatch = BeginSqlBatch();

		if(iList==0) {
			//loop through the list, look for Selected = TRUE, and reverse it
			IRowSettingsPtr pRow = m_UnselectedList->GetFirstRow();
			while(pRow) {				
				BOOL bSelected = VarBool(pRow->GetValue(etcSelected));
				if(bSelected) {
					AddStatementToSqlBatch(strSqlBatch, "UPDATE EligibilityRequestsT SET Selected = 0 WHERE ID = %li",VarLong(pRow->GetValue(etcID)));
				
					_variant_t var;
					var.vt = VT_BOOL;
					var.boolVal = FALSE;

					pRow->PutValue(etcSelected,var);
				}

				pRow = pRow->GetNextRow();
			}
		}
		else {
			//loop through the list, look m_SelectedList Selected = FALSE, and reverse it
			IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
			while(pRow) {				
				BOOL bSelected = VarBool(pRow->GetValue(etcSelected));
				if(!bSelected) {
					AddStatementToSqlBatch(strSqlBatch, "UPDATE EligibilityRequestsT SET Selected = 1 WHERE ID = %li",VarLong(pRow->GetValue(etcID)));
				
					_variant_t var;
					var.vt = VT_BOOL;
					var.boolVal = TRUE;
				
					pRow->PutValue(etcSelected,var);
				}

				pRow = pRow->GetNextRow();
			}
		}

		if(!strSqlBatch.IsEmpty())
			ExecuteSqlBatch(strSqlBatch);

	}NxCatchAll("Error in UpdateCurrentSelect");
}

void CEEligibilityTabDlg::OnDblClickCellUnselectedEligList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		OnSelectOneElig();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnDblClickCellUnselectedEligList");
}


void CEEligibilityTabDlg::OnDblClickCellSelectedEligList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		OnUnselectOneElig();

	}NxCatchAll("Error in CEEligibilityTabDlg::OnDblClickCellSelectedEligList");
}

void CEEligibilityTabDlg::OnRequeryFinishedAnsiFormat(short nFlags) 
{
	try {

		if (m_FormatCombo->GetCurSel() == NULL) {
			// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format
			long nFormatID = GetDefaultEbillingANSIFormatID();
			m_FormatCombo->SetSelByColumn(0, (long)nFormatID);
		}

	}NxCatchAll("Error in CEEligibilityTabDlg::OnRequeryFinishedAnsiFormat");
}

BOOL CEEligibilityTabDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {

		// (j.jones 2010-07-08 09:24) - PLID 39486 - support sending a request immediately
		case ID_SUBMIT_REQUEST:
			try {

				//make sure real-time sends are enabled
				if(GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) != 1) {
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//verify our row
				IRowSettingsPtr pRow = NULL;
				if(m_iCurrList == 0)
					pRow = m_UnselectedList->GetCurSel();
				else
					pRow = m_SelectedList->GetCurSel();

				if(pRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//grab the ID
				long nID = VarLong(pRow->GetValue(etcID), -1);

				CWaitCursor pWait;

				long nFormatID = -1;
				IRowSettingsPtr pFormatRow = m_FormatCombo->GetCurSel();
				if(pFormatRow != NULL)
					nFormatID = VarLong(pFormatRow->GetValue(0), -1);
				else {
					AfxMessageBox("Please select an ANSI Export Style from the list before submitting.");
					return CNxDialog::OnCommand(wParam, lParam);
				}

				CEEligibility dlg(this);
				dlg.m_FormatID = nFormatID;
				// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only
				dlg.m_bUseRealTimeElig = TRUE;
				dlg.m_aryRequestIDsToExport.Add(nID);
				dlg.DoModal();

				UpdateView();

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Submit Request");		
			break;

		case ID_EDIT_REQUEST:

			try {

				//verify our row
				IRowSettingsPtr pRow = NULL;
				if(m_iCurrList == 0)
					pRow = m_UnselectedList->GetCurSel();
				else
					pRow = m_SelectedList->GetCurSel();

				if(pRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//grab the ID
				long nID = VarLong(pRow->GetValue(etcID), -1);

				// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
				if(!CheckCurrentUserPermissions(bioEEligibility,sptWrite)) {
					return CNxDialog::OnCommand(wParam, lParam);
				}

				CWaitCursor pWait;

				//edit the request
				//CEligibilityRequestDlg dlg(this);
				//dlg.m_nID = nID;

				long  nFormatID =-1;
				//// (j.jones 2010-07-08 09:00) - PLID 39515 - pass in the current format, if one is selected
				IRowSettingsPtr pFormatRow = m_FormatCombo->GetCurSel();
				if(pRow != NULL) {
					nFormatID = VarLong(pFormatRow->GetValue(0), -1);
				}
				// (s.dhole 07/23/2012) PLID 48693 
				GetMainFrame()->ShowEligibilityRequestDlg(this,nID,nFormatID ,-1,GetNxColor(GNC_FINANCIAL, 1));

				// (s.dhole 07/23/2012) PLID 48693 this is part of refresh now
				//if(dlg.DoModal() == IDOK) {
				//	// (j.jones 2010-07-07 11:31) - PLID 39515 - now new requests could be left as unselected,
				//	// so we need to requery both lists
				//	m_SelectedList->Requery();
				//	m_UnselectedList->Requery();
				//}
				
			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Edit Request");
			break;

		// (j.jones 2007-06-21 11:30) - PLID 26387 - added ability to review the details of a request
		case ID_REVIEW_REQUEST:

			try {

				//verify our row
				IRowSettingsPtr pRow = NULL;
				if(m_iCurrList == 0)
					pRow = m_UnselectedList->GetCurSel();
				else
					pRow = m_SelectedList->GetCurSel();

				if(pRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//grab the ID
				long nID = VarLong(pRow->GetValue(etcID), -1);

				CWaitCursor pWait;

				//edit the request
				// (j.jones 2010-07-07 09:52) - PLID 39534 - this dialog now takes in an array of request IDs,
				// though right now we are only viewing one request				
				// (r.goldschmidt 2014-10-10 12:51) - PLID 62644 - convert eligibility request detail dialog to modeless
				GetMainFrame()->ShowEligibilityRequestDetailDlg(nID);
				
			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Review Request");
			break;

		case ID_REMOVE_REQUEST:

			try {

				//verify our row
				IRowSettingsPtr pRow = NULL;
				if(m_iCurrList == 0)
					pRow = m_UnselectedList->GetCurSel();
				else
					pRow = m_SelectedList->GetCurSel();

				if(pRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//unbatch the request
				if (MessageBox("Are you sure you want to unbatch the selected Eligibility Request?","Practice",MB_YESNO|MB_ICONQUESTION) == IDNO)
					return CNxDialog::OnCommand(wParam, lParam);

				//grab the ID
				long nID = VarLong(pRow->GetValue(etcID), -1);

				//unbatch the request
				ExecuteSql("UPDATE EligibilityRequestsT SET Batched = 0, Selected = 0 WHERE ID = %li", nID);

				//remove the row and update the total count
				if(m_iCurrList == 0)
					m_UnselectedList->RemoveRow(pRow);
				else
					m_SelectedList->RemoveRow(pRow);

				RefreshTotals();

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Remove Request");
			break;

		case ID_SELECT_BY_INSCO:

			try {

				//verify our row
				IRowSettingsPtr pSelRow = m_UnselectedList->GetCurSel();
				if(pSelRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//move all requests with this insurance company to the selected list
				long nInsCoID = VarLong(pSelRow->GetValue(etcInsCoID),-1);

				IRowSettingsPtr pRow = m_UnselectedList->GetFirstRow();
				while(pRow) {

					//get the next row now, incase we move this row
					IRowSettingsPtr pNextRow = pRow->GetNextRow();

					if(VarLong(pRow->GetValue(etcInsCoID),-1) == nInsCoID) {
						// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
						m_SelectedList->TakeRowAddSorted(pRow);
					}

					pRow = pNextRow;
				}

				UpdateCurrentSelect(1);

				RefreshTotals();

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Select By Insurance Company");
			break;

		case ID_SELECT_BY_INSGROUP:

			try {

				//verify our row
				IRowSettingsPtr pSelRow = m_UnselectedList->GetCurSel();
				if(pSelRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//move all requests with this insurance group to the selected list
				long nInsGroupID = VarLong(pSelRow->GetValue(etcInsGroupID),-1);

				if(nInsGroupID != -1) {

					IRowSettingsPtr pRow = m_UnselectedList->GetFirstRow();
					while(pRow) {

						//get the next row now, incase we move this row
						IRowSettingsPtr pNextRow = pRow->GetNextRow();

						if(VarLong(pRow->GetValue(etcInsGroupID),-1) == nInsGroupID) {
							// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
							m_SelectedList->TakeRowAddSorted(pRow);
						}

						pRow = pNextRow;
					}

					UpdateCurrentSelect(1);

					RefreshTotals();
				}
				else {
					AfxMessageBox("The selected request's insurance company is not in a HCFA Group.");
				}

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Select By Insurance Group");
			break;

		case ID_SELECT_BY_PROV:

			try {

				//verify our row
				IRowSettingsPtr pSelRow = m_UnselectedList->GetCurSel();
				if(pSelRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//move all requests with this provider to the selected list
				long nProvID = VarLong(pSelRow->GetValue(etcProviderID),-1);

				IRowSettingsPtr pRow = m_UnselectedList->GetFirstRow();
				while(pRow) {

					//get the next row now, incase we move this row
					IRowSettingsPtr pNextRow = pRow->GetNextRow();

					if(VarLong(pRow->GetValue(etcProviderID),-1) == nProvID) {
						// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
						m_SelectedList->TakeRowAddSorted(pRow);
					}

					pRow = pNextRow;
				}

				UpdateCurrentSelect(1);

				RefreshTotals();

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Select By Provider");
			break;

		case ID_UNSELECT_BY_INSCO:

			try {

				//verify our row
				IRowSettingsPtr pSelRow = m_SelectedList->GetCurSel();
				if(pSelRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//move all requests with this insurance company to the unselected list
				long nInsCoID = VarLong(pSelRow->GetValue(etcInsCoID),-1);

				IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
				while(pRow) {

					//get the next row now, incase we move this row
					IRowSettingsPtr pNextRow = pRow->GetNextRow();

					if(VarLong(pRow->GetValue(etcInsCoID),-1) == nInsCoID) {
						// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
						m_UnselectedList->TakeRowAddSorted(pRow);
					}

					pRow = pNextRow;
				}

				UpdateCurrentSelect(0);

				RefreshTotals();

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Unselect By Insurance Company");
			break;

		case ID_UNSELECT_BY_INSGROUP:

			try {

				//verify our row
				IRowSettingsPtr pSelRow = m_SelectedList->GetCurSel();
				if(pSelRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//move all requests with this insurance group to the unselected list
				long nInsGroupID = VarLong(pSelRow->GetValue(etcInsGroupID),-1);

				if(nInsGroupID != -1) {

					IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
					while(pRow) {

						//get the next row now, incase we move this row
						IRowSettingsPtr pNextRow = pRow->GetNextRow();

						if(VarLong(pRow->GetValue(etcInsGroupID),-1) == nInsGroupID) {
							// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
							m_UnselectedList->TakeRowAddSorted(pRow);
						}

						pRow = pNextRow;
					}

					UpdateCurrentSelect(0);

					RefreshTotals();
				}
				else {
					AfxMessageBox("The selected request's insurance company is not in a HCFA Group.");
				}

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Unselect By Insurance Group");
			break;

		case ID_UNSELECT_BY_PROV:

			try {

				//verify our row
				IRowSettingsPtr pSelRow = m_SelectedList->GetCurSel();
				if(pSelRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//move all requests with this provider to the unselected list
				long nProvID = VarLong(pSelRow->GetValue(etcProviderID),-1);

				IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
				while(pRow) {

					//get the next row now, incase we move this row
					IRowSettingsPtr pNextRow = pRow->GetNextRow();

					if(VarLong(pRow->GetValue(etcProviderID),-1) == nProvID) {
						// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
						m_UnselectedList->TakeRowAddSorted(pRow);
					}

					pRow = pNextRow;
				}

				UpdateCurrentSelect(0);

				RefreshTotals();

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Unselect By Provider");
			break;

		case ID_ELIG_GOTO_PATIENT:

			try {

				//verify our row
				IRowSettingsPtr pRow = NULL;
				if(m_iCurrList == 0)
					pRow = m_UnselectedList->GetCurSel();
				else
					pRow = m_SelectedList->GetCurSel();

				if(pRow == NULL) {
					//nothing selected! get out of here
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//grab the ID
				long nPatientID = VarLong(pRow->GetValue(etcPatientID), -1);

				if (nPatientID != -1) {
					//Set the active patient
					CMainFrame *pMainFrame;
					pMainFrame = GetMainFrame();
					if (pMainFrame != NULL) {

						if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
							if(IDNO == MessageBox("This patient is not in the current lookup. \n"
								"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								return CNxDialog::OnCommand(wParam, lParam);
							}
						}
						//TES 1/7/2010 - PLID 36761 - This function may fail now
						if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

							//Now just flip to the patient's module and set the active Patient
							pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
							CNxTabView *pView = pMainFrame->GetActiveView();
							if(pView)
								pView->UpdateView();
						}
					}//end if MainFrame
					else {
						MsgBox(MB_ICONSTOP|MB_OK, "ERROR - EEligibilityTabDlg.cpp: Cannot Open Mainframe");
					}//end else pMainFrame
				}//end if nPatientID

			}NxCatchAll("Error in CEEligibilityTabDlg::OnCommand - Go To Patient");
			break;
	}
	
	return CNxDialog::OnCommand(wParam, lParam);
}

// (j.jones 2007-06-19 14:10) - PLID 26269 - added ability to review (and re-batch) past requests
// (r.goldschmidt 2014-10-08 16:59) - PLID 62644 - converted to modeless
void CEEligibilityTabDlg::OnBtnReviewPastRequests() 
{
	try{
		GetMainFrame()->ShowEligibilityReviewDlg();
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-09-16 10:28) - PLID 26481 - added ability to mass-create requests
void CEEligibilityTabDlg::OnBtnCreateRequestsForScheduledPatients()
{
	try {

		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility,sptWrite))
			return;

		CEligibilityCreateForScheduledPatientsDlg dlg(this);

		// (j.jones 2010-07-08 09:00) - PLID 39515 - pass in the current format, if one is selected
		IRowSettingsPtr pRow = m_FormatCombo->GetCurSel();
		if(pRow != NULL) {
			dlg.m_nFormatID = VarLong(pRow->GetValue(0), -1);
		}

		if(dlg.DoModal() == IDOK) {
			//requery the unselected list, since new requests are auto-batched into this list
			m_UnselectedList->Requery();
			// (j.jones 2010-07-07 11:31) - PLID 39515 - now new requests could be left as unselected
			m_SelectedList->Requery();
		}

	}NxCatchAll("Error in CEEligibilityTabDlg::OnBtnCreateRequestsForScheduledPatients");
}

// (j.jones 2010-03-25 17:32) - PLID 37905 - added ability to configure filtering how response info. displays
void CEEligibilityTabDlg::OnBtnConfigResponseFiltering()
{
	try {

		CEligibilityResponseFilteringConfigDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-02 14:37) - PLID 39506 - added setup for the real-time E-Eligibility settings
void CEEligibilityTabDlg::OnBtnConfigEligibilityRealtimeSetup()
{
	try {

		// Check for permission to edit the clearinghouse integration settings before allowing access to the config.
		if (!CheckCurrentUserPermissions(bioClearinghouseIntegrationSettings, sptWrite)) {
			return;
		}

		CEligibilityRealTimeSetupDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-05-19 10:29) - NX-100685 - added OnSelChosen
void CEEligibilityTabDlg::OnSelChosenAnsiFormat(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow) {		
			long nFormatID = VarLong(pRow->GetValue(0));
			// (j.jones 2016-05-19 10:31) - NX-100685 - save this value
			SetRemotePropertyInt(_T("FormatStyle"), nFormatID, 0, "<None>");
		}

	}NxCatchAll(__FUNCTION__);
}

void CEEligibilityTabDlg::OnRadioEligTestBatch()
{
	try {

		UpdateEligibilityProductionStatus();

	}NxCatchAll(__FUNCTION__);
}

void CEEligibilityTabDlg::OnRadioEligProductionBatch()
{
	try {

		UpdateEligibilityProductionStatus();

	}NxCatchAll(__FUNCTION__);
}


void CEEligibilityTabDlg::UpdateEligibilityProductionStatus()
{
	try {

		long nProduction = 0;
		if (m_radioProduction.GetCheck())
		{
			nProduction = 1;
		}

		SetRemotePropertyInt("EligibilityProduction", nProduction, 0, "<None>");

	}NxCatchAll(__FUNCTION__);
}

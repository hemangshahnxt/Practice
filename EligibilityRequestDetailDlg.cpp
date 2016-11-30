// EligibilityRequestDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EligibilityRequestDetailDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "EligibilityRequestDlg.h"
#include "EligibilityResponseFilteringConfigDlg.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "FinancialView.h"
#include "EEligibilityTabDlg.h"
#include "EligibilityReviewDlg.h"
#include "GlobalFinancialUtils.h"
#include "GlobalInsuredPartyUtils.h"
#include "AuditTrail.h"

// (j.jones 2007-06-19 17:37) - PLID 26387 - created the EligibilityRequestDetailDlg

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (j.jones 2010-03-25 09:32) - PLID 37832 - added list of all details
enum EligibilityResponseDetailColumns {

	erdcID = 0,
	erdcServiceTypeRefID,	// (j.jones 2010-03-26 13:29) - PLID 37905
	erdcTypeOfService,
	erdcCoverageLevelRefID,	// (j.jones 2010-03-26 13:29) - PLID 37905
	erdcCoverage,
	erdcBenefitTypeRefID,	// (j.jones 2010-03-26 13:29) - PLID 37905
	erdcBenefitType,
	erdcTimePeriod,
	erdcBenefit,	//calculated by our code
	erdcValue,		//calculated by our code	
	erdcDescription,		// (j.jones 2012-02-08 17:58) - PLID 48038 - moved to be after the value, and now calculated by our code
	erdcInNetwork,
	erdcAuthorized,
	erdcInsType,
	erdcServiceCode,
	erdcModifiers,	
};

// (j.jones 2010-03-26 17:32) - PLID 37619 - the filter combos have the same column structure
enum FilterComboColumns {

	fccID = 0,
	fccDescription,
};

// (j.jones 2016-05-16 10:15) - NX-100357 - added Other Payors list
enum OtherPayorColumns {

	opcInsuranceName = 0,
	opcFullAddress,
	opcContactInfo,
	opcPayerID,
	opcPolicyNumber,
	opcGroupNumber,
};

//columns in the list of deductible/out-of-pocket values
enum DedOOPColumns {
	docID = 0,		//not a database ID, just a row identifier with a DedOOPRow enum value
	docDescription,	//the displayed row name
	docValue,		//the currency value
};

//identifiers for the deductible/out-of-pocket rows
enum DedOOPRow {
	dorDeductibleTotal = 0,
	dorDeductibleRemaining,
	dorOOPTotal,
	dorOOPRemaining,
};

//used to reset the response filter to "all" after a current
//radio button check is complete
#define NXM_RESET_RADIO_TO_ALL	(WM_USER + 1000)

/////////////////////////////////////////////////////////////////////////////
// CEligibilityRequestDetailDlg dialog

CEligibilityRequestDetailDlg::CEligibilityRequestDetailDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEligibilityRequestDetailDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEligibilityRequestDetailDlg)
		m_nCurRequestIndex = -1;
		m_nCurResponseIndex = -1;
		m_nDetailListInitialTop = -1;
		m_nDetailListInitialBottom = -1;
		m_nResponseListInitialTop = -1;
		m_nResponseListInitialBottom = -1;
		m_bCurResponseHasDetails = FALSE;
		m_bHasDefaultFilters = FALSE;
		m_nCurrentRequestPatientID = -1;
		m_nCurrentRequestInsuredPartyID = -1;
	//}}AFX_DATA_INIT
}


void CEligibilityRequestDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEligibilityRequestDetailDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_REBATCH_ELIG_REQUEST, m_btnRebatch);
	DDX_Control(pDX, IDC_BTN_EDIT_ELIG_REQUEST, m_btnEdit);
	DDX_Control(pDX, IDC_ELIGIBILITY_PREV_RESPONSE, m_btnLeftResponse);
	DDX_Control(pDX, IDC_ELIGIBILITY_NEXT_RESPONSE, m_btnRightResponse);
	DDX_Control(pDX, IDC_ELIG_RESPONSE_LABEL, m_nxstaticResponseLabel);
	DDX_Control(pDX, IDC_ELIG_RESPONSE_COUNT_LABEL, m_nxstaticResponseCountLabel);
	DDX_Control(pDX, IDC_CHECK_FILTER_DETAILS, m_checkFilterDetails);
	DDX_Control(pDX, IDC_BTN_CONFIG_RESPONSE_FILTERING_REVIEW, m_btnConfigureFiltering);
	DDX_Control(pDX, IDC_SERVICETYPE_FILTER_LABEL, m_nxstaticServiceTypeFilterLabel);
	DDX_Control(pDX, IDC_COVERAGE_FILTER_LABEL, m_nxstaticCoverageLevelFilterLabel);
	DDX_Control(pDX, IDC_BENEFIT_FILTER_LABEL, m_nxstaticBenefitTypeFilterLabel);
	DDX_Control(pDX, IDC_ELIGIBILITY_PREV_REQ, m_btnLeftRequest);
	DDX_Control(pDX, IDC_ELIGIBILITY_NEXT_REQ, m_btnRightRequest);
	DDX_Control(pDX, IDC_ELIGBTN_CUR_REQ_INFO, m_btnMultiRequestInfo);
	DDX_Control(pDX, IDC_BTN_PRINT_PREVIEW, m_btnPrintPreview); 
	DDX_Control(pDX, IDC_BTN_GO_TO_PATIENT, m_btnGoToPatient); // (b.eyers 2015-04-17) - PLID 44309
	DDX_Control(pDX, IDC_LABEL_EE_PATIENT_NAME, m_nxstaticPatientName);
	DDX_Control(pDX, IDC_LABEL_EE_INSURED_PARTIES, m_nxstaticInsuredParties);
	DDX_Control(pDX, IDC_OTHER_PAYORS_LABEL, m_nxstaticOtherPayorsLabel);
	DDX_Control(pDX, IDC_FILTER_RESPONSE_TYPE_LABEL, m_nxstaticFilterResponseTypeLabel);
	DDX_Control(pDX, IDC_RADIO_RESPONSE_TYPE_ALL, m_radioResponseTypeAll);
	DDX_Control(pDX, IDC_RADIO_RESPONSE_TYPE_ACTIVE, m_radioResponseTypeActive);
	DDX_Control(pDX, IDC_RADIO_RESPONSE_TYPE_INACTIVE, m_radioResponseTypeInactive);
	DDX_Control(pDX, IDC_RADIO_RESPONSE_TYPE_FAILED, m_radioResponseTypeFailed);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CEligibilityRequestDetailDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEligibilityRequestDetailDlg)
	ON_BN_CLICKED(IDC_BTN_EDIT_ELIG_REQUEST, OnBtnEditEligRequest)
	ON_BN_CLICKED(IDC_BTN_REBATCH_ELIG_REQUEST, OnBtnRebatchEligRequest)	
	ON_BN_CLICKED(IDC_ELIGIBILITY_PREV_RESPONSE, OnEligibilityPrevResponse)
	ON_BN_CLICKED(IDC_ELIGIBILITY_NEXT_RESPONSE, OnEligibilityNextResponse)
	ON_BN_CLICKED(IDC_CHECK_FILTER_DETAILS, OnCheckFilterDetails)
	ON_BN_CLICKED(IDC_BTN_CONFIG_RESPONSE_FILTERING_REVIEW, OnBtnConfigResponseFiltering)	
	ON_BN_CLICKED(IDC_ELIGIBILITY_PREV_REQ, OnEligibilityPrevReq)
	ON_BN_CLICKED(IDC_ELIGIBILITY_NEXT_REQ, OnEligibilityNextReq)
	ON_BN_CLICKED(IDC_BTN_GO_TO_PATIENT, OnBtnGoToPatient) // (b.eyers 2015-04-17) - PLID 44309
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_BTN_PRINT_PREVIEW, OnBnClickedBtnPrintPreview)	
	ON_BN_CLICKED(IDC_RADIO_RESPONSE_TYPE_ALL, OnRadioResponseTypeAll)
	ON_BN_CLICKED(IDC_RADIO_RESPONSE_TYPE_ACTIVE, OnRadioResponseTypeActive)
	ON_BN_CLICKED(IDC_RADIO_RESPONSE_TYPE_INACTIVE, OnRadioResponseTypeInactive)
	ON_BN_CLICKED(IDC_RADIO_RESPONSE_TYPE_FAILED, OnRadioResponseTypeFailed)
	ON_MESSAGE(NXM_RESET_RADIO_TO_ALL, OnResetResponseTypeFilterToAll)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEligibilityRequestDetailDlg message handlers

BOOL CEligibilityRequestDetailDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		// (j.jones 2008-05-07 15:34) - PLID 29854 - added nxiconbuttons for modernization
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnRebatch.AutoSet(NXB_MODIFY);
		m_btnEdit.AutoSet(NXB_MODIFY);
		// (j.jones 2010-03-25 10:19) - PLID 37832 - added arrow buttons
		m_btnLeftResponse.AutoSet(NXB_LEFT);
		m_btnRightResponse.AutoSet(NXB_RIGHT);
		// (j.jones 2010-03-26 15:50) - PLID 37619 - added ability to filter the details
		m_btnConfigureFiltering.AutoSet(NXB_MODIFY);
		// (j.jones 2010-07-07 09:39) - PLID 39534 - supported showing multiple requests on this dialog
		m_btnLeftRequest.AutoSet(NXB_LEFT);
		m_btnRightRequest.AutoSet(NXB_RIGHT);
		// (b.spivey, June 04, 2012) - PLID 48696 - Added the print preview icon to the button. 
		m_btnPrintPreview.AutoSet(NXB_PRINT_PREV);

		// (j.jones 2016-05-12 11:16) - NX-100625 - set a larger font for the
		// patient name & payer list
		m_nxstaticPatientName.SetFont(theApp.GetPracticeFont(CPracticeApp::pftHeader));
		m_nxstaticInsuredParties.SetFont(theApp.GetPracticeFont(CPracticeApp::pftHeader));
		m_nxstaticPatientName.SetWindowText("");
		m_nxstaticInsuredParties.SetWindowText("");

		m_RequestInfoList = BindNxDataList2Ctrl(this, IDC_ELIGIBILITY_REQUEST_INFO, GetRemoteData(), false);
		// (j.jones 2010-03-25 09:11) - PLID 37832 - split out a response info list, and added a list of all details
		m_ResponseInfoList = BindNxDataList2Ctrl(this, IDC_ELIGIBILITY_RESPONSE_INFO, GetRemoteData(), false);
		m_DetailList = BindNxDataList2Ctrl(this, IDC_ELIGIBILITY_RESPONSE_DETAILS_LIST, GetRemoteData(), false);
		// (j.jones 2010-03-26 17:32) - PLID 37619 - added ability to filter the details
		m_ServiceTypeFilterCombo = BindNxDataList2Ctrl(this, IDC_ELIG_SERVICETYPE_COMBO, GetRemoteData(), false);
		m_CoverageLevelFilterCombo = BindNxDataList2Ctrl(this, IDC_ELIG_COVERAGELEVEL_COMBO, GetRemoteData(), false);
		m_BenefitTypeFilterCombo = BindNxDataList2Ctrl(this, IDC_ELIG_BENEFITTYPE_COMBO, GetRemoteData(), false);

		// (j.jones 2016-05-16 10:10) - NX-100357 - added Other Payors list
		m_OtherPayorsList = BindNxDataList2Ctrl(this, IDC_OTHER_PAYORS_LIST, GetRemoteData(), false);

		//set up the list of pay groups, copays, coinsurance
		{
			m_PayGroupsList = BindNxDataList2Ctrl(IDC_ELIG_PAY_GROUP_LIST, false);

			//set up the international currency symbol
			IColumnSettingsPtr pCol = m_PayGroupsList->GetColumn(gpgcCopayMoney);
			if (pCol) {
				pCol->ColumnTitle = _bstr_t("Copay " + GetCurrencySymbol());
			}
		}

		//set up the list of deductible/out-of-pocket values
		{			
			m_DedOOPList = BindNxDataList2Ctrl(IDC_ELIG_DEDOOP_LIST, false);
			IRowSettingsPtr pDedOOPRow = m_DedOOPList->GetNewRow();
			pDedOOPRow->PutValue(docID, (long)dorDeductibleTotal);
			pDedOOPRow->PutValue(docDescription, _bstr_t("Total Deductible"));
			pDedOOPRow->PutValue(docValue, g_cvarNull);
			m_DedOOPList->AddRowAtEnd(pDedOOPRow, NULL);
			pDedOOPRow = m_DedOOPList->GetNewRow();
			pDedOOPRow->PutValue(docID, (long)dorDeductibleRemaining);
			pDedOOPRow->PutValue(docDescription, _bstr_t("Deductible Remaining"));
			pDedOOPRow->PutValue(docValue, g_cvarNull);
			m_DedOOPList->AddRowAtEnd(pDedOOPRow, NULL);
			pDedOOPRow = m_DedOOPList->GetNewRow();
			pDedOOPRow->PutValue(docID, (long)dorOOPTotal);
			pDedOOPRow->PutValue(docDescription, _bstr_t("Total Out Of Pocket"));
			pDedOOPRow->PutValue(docValue, g_cvarNull);
			m_DedOOPList->AddRowAtEnd(pDedOOPRow, NULL);
			pDedOOPRow = m_DedOOPList->GetNewRow();
			pDedOOPRow->PutValue(docID, (long)dorOOPRemaining);
			pDedOOPRow->PutValue(docDescription, _bstr_t("Out Of Pocket Remaining"));
			pDedOOPRow->PutValue(docValue, g_cvarNull);
			m_DedOOPList->AddRowAtEnd(pDedOOPRow, NULL);
		}

		//if the user doesn't have permission to edit insured parties,
		//disable editing the pay group list and the deductible/OOP list
		if (!(GetCurrentUserPermissions(bioPatientInsurance) & SPT___W_______)) {
			m_PayGroupsList->PutReadOnly(VARIANT_TRUE);
			m_DedOOPList->PutReadOnly(VARIANT_TRUE);
		}
		else {

			//color the editable cells blue
			COLORREF clrBlue = RGB(240, 240, 255);

			IColumnSettingsPtr pCoInsCol = m_PayGroupsList->GetColumn(gpgcCoIns);
			if(pCoInsCol) {
				pCoInsCol->PutBackColor(clrBlue);
			}
			IColumnSettingsPtr pCopayMoneyCol = m_PayGroupsList->GetColumn(gpgcCopayMoney);
			if (pCopayMoneyCol) {
				pCopayMoneyCol->PutBackColor(clrBlue);
			}
			IColumnSettingsPtr pCopayPctCol = m_PayGroupsList->GetColumn(gpgcCopayPercent);
			if (pCopayPctCol) {
				pCopayPctCol->PutBackColor(clrBlue);
			}
			IColumnSettingsPtr pDedOOPValueCol = m_DedOOPList->GetColumn(docValue);
			if (pDedOOPValueCol) {
				pDedOOPValueCol->PutBackColor(clrBlue);
			}
		}

		//initialize the filters to all
		m_radioResponseTypeAll.SetCheck(TRUE);
				
	}NxCatchAll("Error in CEligibilityRequestDetailDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (r.goldschmidt 2014-10-13 15:36) - PLID 62644 - Moved from OnInitDialog, every time we ask for this dialog in MainFrame, we need to ensure proper loading
void CEligibilityRequestDetailDlg::EnsureControls()
{
	try{
		// (j.jones 2010-07-07 09:59) - PLID 39534 - select the first request,
		// then update the display of the multi-request controls
		m_nCurRequestIndex = 0;

		//the size of m_aryAllRequestIDs never changes during the lifespan of this dialog,
		//so the showing/hiding of controls specific to multiple requests can be done here,
		//instead of UpdateRequestButtons
		if (m_aryAllRequestIDs.size() <= 1) {
			//if we do not have multiple requests, hide the buttons
			GetDlgItem(IDC_ELIGIBILITY_PREV_REQ)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ELIGIBILITY_NEXT_REQ)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ELIGBTN_CUR_REQ_INFO)->ShowWindow(SW_HIDE);

			//also need to hide the filter request options when we're only viewing one request
			m_nxstaticFilterResponseTypeLabel.ShowWindow(SW_HIDE);
			m_radioResponseTypeAll.ShowWindow(SW_HIDE);
			m_radioResponseTypeActive.ShowWindow(SW_HIDE);
			m_radioResponseTypeInactive.ShowWindow(SW_HIDE);
			m_radioResponseTypeFailed.ShowWindow(SW_HIDE);

			//move up the patient name and insurance co. name to use up some of the now-available space
			CRect rcPatName, rcInsuredName;
			GetDlgItem(IDC_LABEL_EE_PATIENT_NAME)->GetWindowRect(&rcPatName);
			GetDlgItem(IDC_LABEL_EE_INSURED_PARTIES)->GetWindowRect(&rcInsuredName);
			ScreenToClient(&rcPatName);
			ScreenToClient(&rcInsuredName);
			GetDlgItem(IDC_LABEL_EE_PATIENT_NAME)->MoveWindow(rcPatName.left, rcPatName.top - (rcPatName.Height() / 2), rcPatName.Width(), rcPatName.Height());
			GetDlgItem(IDC_LABEL_EE_INSURED_PARTIES)->MoveWindow(rcInsuredName.left, rcInsuredName.top - (rcInsuredName.Height() / 2), rcInsuredName.Width(), rcInsuredName.Height());
		}
		else {
			// (r.goldschmidt 2014-10-13 15:41) - PLID 62644 - if we do have multiple requests, show the buttons
			GetDlgItem(IDC_ELIGIBILITY_PREV_REQ)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ELIGIBILITY_NEXT_REQ)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ELIGBTN_CUR_REQ_INFO)->ShowWindow(SW_SHOW);

			//also need to show the filter request options when multiple requests exist
			m_nxstaticFilterResponseTypeLabel.ShowWindow(SW_SHOW);
			m_radioResponseTypeAll.ShowWindow(SW_SHOW);
			m_radioResponseTypeActive.ShowWindow(SW_SHOW);
			m_radioResponseTypeInactive.ShowWindow(SW_SHOW);
			m_radioResponseTypeFailed.ShowWindow(SW_SHOW);
		}

		//initialize the filtered requests to all requests
		m_aryCurFilteredRequestIDs.clear();
		m_aryCurFilteredRequestIDs.insert(m_aryCurFilteredRequestIDs.end(), m_aryAllRequestIDs.begin(), m_aryAllRequestIDs.end());

		//make sure our filter is set to all
		m_radioResponseTypeAll.SetCheck(TRUE);

		UpdateRequestButtons();

		FullReload(0);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-25 10:17) - PLID 37832 - changed the Load() function
// into FullReload() which loads the request & count of responses,
// and LoadResponse() which loads a specific response
void CEligibilityRequestDetailDlg::FullReload(long nInitialResponseIndex /*= 0*/) 
{
	try {

		long nRequestID = GetRequestID();

		m_nxstaticPatientName.SetWindowText("");
		m_nxstaticInsuredParties.SetWindowText("");

		CWaitCursor pWait;

		//this function can possibly be called multiple times

		//load up the eligibility info, and response info. if it exists,
		//and populate the on screen list

		m_nxstaticResponseLabel.SetWindowText("Response Details");
		m_nxstaticResponseCountLabel.SetWindowText("");

		m_RequestInfoList->SetRedraw(FALSE);

		m_RequestInfoList->Clear();
		m_ResponseInfoList->Clear();
		m_DetailList->Clear();
		// (j.jones 2016-05-16 10:15) - NX-100357 - added Other Payors list
		m_OtherPayorsList->Clear();
		m_PayGroupsList->Clear();

		//don't clear the whole deductible/OOP list, just clear the value columns only
		{
			IRowSettingsPtr pDedOOPRow = m_DedOOPList->GetFirstRow();
			while (pDedOOPRow) {
				pDedOOPRow->PutValue(docValue, g_cvarNull);
				pDedOOPRow = pDedOOPRow->GetNextRow();
			}
		}

		m_aryCurResponseIDs.clear();

		// (j.jones 2010-03-26 17:32) - PLID 37619 - clear the filter combos
		m_ServiceTypeFilterCombo->Clear();
		m_CoverageLevelFilterCombo->Clear();
		m_BenefitTypeFilterCombo->Clear();

		//reset the filter combo where clauses, so that the next time we set them, it will force a requery
		//(as we do not requery unless they changed)
		m_ServiceTypeFilterCombo->PutWhereClause("");
		m_CoverageLevelFilterCombo->PutWhereClause("");
		m_BenefitTypeFilterCombo->PutWhereClause("");

		// (j.jones 2010-03-26 16:14) - PLID 37619 - cache whether there are default filters
		// (b.eyers 2015-04-17) - PLID 44309 - Need patient ID for go to patient button
		// (j.jones 2016-05-19 11:41) - NX-100625 - added policy & group number
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM EligibilityDataReferenceT WHERE FilterExcluded = 1 "
			""
			"SELECT "
			"PatientsT.UserDefinedID, PersonT.ID, EligibilityRequestsT.InsuredPartyID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"InsuranceCoT.Name AS InsCoName, InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum, "
			"InsuredPartyT.TotalDeductible, InsuredPartyT.DeductibleRemaining, "
			"InsuredPartyT.TotalOOP, InsuredPartyT.OOPRemaining, "
			"RespTypeT.TypeName AS RespTypeName, "
			"PersonProvidersT.Last + ', ' + PersonProvidersT.First + ' ' + PersonProvidersT.Middle AS ProvName, "
			"EligibilityRequestsT.LastSentDate, "
			"EligibilityRequestsT.CreateDate "
			"FROM EligibilityRequestsT "
			"INNER JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
			"INNER JOIN ProvidersT ON EligibilityRequestsT.ProviderID = ProvidersT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN PersonT PersonProvidersT ON ProvidersT.PersonID = PersonProvidersT.ID "
			"WHERE EligibilityRequestsT.ID = {INT}", nRequestID);

		// (j.jones 2010-03-26 16:14) - PLID 37619 - cache whether there are default filters
		if (!rs->eof) {
			//this recordset just checked to see if any items were excluded,
			//which determines if the filter checkbox is displayed
			m_bHasDefaultFilters = TRUE;
			GetDlgItem(IDC_CHECK_FILTER_DETAILS)->ShowWindow(SW_SHOWNA);

			//always check the filter box if we have filters, and are doing a full reload
			m_checkFilterDetails.SetCheck(TRUE);
		}
		else {
			m_bHasDefaultFilters = FALSE;
			m_checkFilterDetails.SetCheck(FALSE);
			GetDlgItem(IDC_CHECK_FILTER_DETAILS)->ShowWindow(SW_HIDE);
		}

		rs = rs->NextRecordset(NULL);

		if (!rs->eof) {

			long nUserDefinedID = AdoFldLong(rs, "UserDefinedID");
			m_nCurrentRequestInsuredPartyID = AdoFldLong(rs, "InsuredPartyID");
			m_strCurrentRequestPatientName = AdoFldString(rs, "PatName", "");
			m_strCurrentRequestPatientName.Trim();
			CString strInsCoName = AdoFldString(rs, "InsCoName", "");
			// (j.jones 2016-05-19 11:41) - NX-100625 - added policy & group number
			CString strPolicyNumber = AdoFldString(rs, "IDForInsurance", "");
			CString strGroupNumber = AdoFldString(rs, "PolicyGroupNum", "");
			CString strRespType = AdoFldString(rs, "RespTypeName", "");
			// (b.spivey, June 05, 2012) - PLID 48696 - Member variable now. 
			m_strProvName = AdoFldString(rs, "ProvName", "");
			// (b.eyers 2015-04-17) - PLID 44309
			m_nCurrentRequestPatientID = AdoFldLong(rs, "ID");

			//reload this insured party's pay groups - this has to change
			//the from clause, because we want all pay groups, plus
			//any pay group data that may exist for this insured party
			CString strPayGroupFromClause;			
			strPayGroupFromClause.Format("ServicePayGroupsT LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID AND InsuredPartyPayGroupsT.InsuredPartyID = %li ", m_nCurrentRequestInsuredPartyID);
			m_PayGroupsList->FromClause = _bstr_t(strPayGroupFromClause);
			m_PayGroupsList->Requery();

			//update the deductible/out-of-pocket list
			{
				IRowSettingsPtr pDedOOPRow = m_DedOOPList->GetFirstRow();
				while (pDedOOPRow) {
					DedOOPRow eCurRowID = (DedOOPRow)VarLong(pDedOOPRow->GetValue(docID));
					switch (eCurRowID) {
						case dorDeductibleTotal:
							pDedOOPRow->PutValue(docValue, rs->Fields->Item["TotalDeductible"]->Value);
							break;
						case dorDeductibleRemaining:
							pDedOOPRow->PutValue(docValue, rs->Fields->Item["DeductibleRemaining"]->Value);
							break;
						case dorOOPTotal:
							pDedOOPRow->PutValue(docValue, rs->Fields->Item["TotalOOP"]->Value);
							break;
						case dorOOPRemaining:
							pDedOOPRow->PutValue(docValue, rs->Fields->Item["OOPRemaining"]->Value);
							break;
						default:
							ThrowNxException("Invalid deductible row: %li", (long)eCurRowID);
							break;
					}
					pDedOOPRow = pDedOOPRow->GetNextRow();
				}
			}

			CString strDateInfo = "";
			_variant_t varLastSent = rs->Fields->Item["LastSentDate"]->Value;
			if (varLastSent.vt == VT_DATE) {
				COleDateTime dt = VarDateTime(varLastSent);
				strDateInfo.Format("Date Sent: %s", FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoNaturalDatetime));
			}
			else {
				//show the date created if never sent
				CString strCreateDate = "";
				_variant_t varCreated = rs->Fields->Item["CreateDate"]->Value;
				if (varCreated.vt == VT_DATE) {
					COleDateTime dt = VarDateTime(varCreated);
					strDateInfo.Format("Created: %s (Never Sent)", FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoNaturalDatetime));
				}
			}

			//now build the list with this information

			CString strFmt;

			//Patient ID / Patient Name			

			// (j.jones 2016-05-12 11:19) - NX-100625 - this is now a dedicated label
			strFmt.Format("Patient: %s (ID: %li)", m_strCurrentRequestPatientName, nUserDefinedID);
			m_nxstaticPatientName.SetWindowText(strFmt);
			m_nxstaticInsuredParties.SetWindowText(strFmt);

			//Insurance Co / Responsibility
			strFmt.Format("Insurance Co: %s (%s)", strInsCoName, strRespType);
			AddNewRequestRow(strFmt);

			// (j.jones 2016-05-19 11:40) - NX-100625 - add the policy number
			if (!strPolicyNumber.IsEmpty()) {
				strFmt.Format("Insurance ID: %s", strPolicyNumber);
				AddNewRequestRow(strFmt);
			}

			// (j.jones 2016-05-19 11:40) - NX-100625 - add the group number
			if (!strGroupNumber.IsEmpty()) {
				strFmt.Format("Group Number: %s", strGroupNumber);
				AddNewRequestRow(strFmt);
			}

			//Provider:
			// (j.jones 2009-09-17 12:05) - PLID 35576 - fixed bug where we showed the InsCo name here
			// (b.spivey, June 05, 2012) - PLID 48696 - member variable now for print preview report. 
			strFmt.Format("Provider: %s", m_strProvName);
			AddNewRequestRow(strFmt);

			//Create Date / Last Sent Date
			//this was calculated earlier in the code
			AddNewRequestRow(strDateInfo);

			//If we just got a batch of responses back, we are only displaying the responses
			//we just received, not all historical responses for these requests.
			//If so, this will only show responses we have just received.
			_RecordsetPtr rsResponses = CreateParamRecordset("SELECT EligibilityResponsesT.ID "
				"FROM EligibilityResponsesT "
				"WHERE EligibilityResponsesT.RequestID = {INT} "
				"{SQL} "
				"ORDER BY EligibilityResponsesT.DateReceived DESC", nRequestID, GetResponseFilterWhere());
			if (rsResponses->eof) {
				//if no responses, say so
				AddNewResponseRow(" - No Response Received -", RGB(192, 0, 0));
				m_nxstaticResponseLabel.SetWindowText("Response Details");
			}
			else {
				while (!rsResponses->eof) {

					long nResponseID = AdoFldLong(rsResponses, "ID");
					m_aryCurResponseIDs.push_back(nResponseID);

					rsResponses->MoveNext();
				}
			}
			rsResponses->Close();

			if (nInitialResponseIndex < 0) {
				nInitialResponseIndex = 0;
			}
			if ((long)m_aryCurResponseIDs.size() - 1 >= nInitialResponseIndex) {
				m_nCurResponseIndex = nInitialResponseIndex;
			}
			else if ((long)m_aryCurResponseIDs.size() > 0) {
				m_nCurResponseIndex = 0;
			}
			else {
				m_nCurResponseIndex = -1;
			}

			if (m_nCurResponseIndex >= 0) {
				LoadResponse(m_aryCurResponseIDs.at(m_nCurResponseIndex));
			}
			else {
				// (r.goldschmidt 2014-10-14 11:44) - PLID 62644 - There is no response to load
				//  Therefore, there are no details to show
				m_bCurResponseHasDetails = FALSE;
			}
		}
		rs->Close();

		m_RequestInfoList->SetRedraw(TRUE);

		// (j.jones 2016-05-12 13:59) - NX-100625 - added a list of all insured parties the patient has
		if (m_nCurrentRequestPatientID > 0) {
			CString strInsCoList = "";
			_RecordsetPtr rsInsured = CreateParamRecordset("SELECT InsuranceCoT.Name AS InsCoName, "
				"RespTypeT.TypeName "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE InsuredPartyT.PatientID = {INT} "
				"ORDER BY (CASE WHEN RespTypeT.Priority = -1 THEN 1 ELSE 0 END) ASC, RespTypeT.Priority ASC", m_nCurrentRequestPatientID);
			if (rsInsured->eof) {
				//this ought to be impossible
				strInsCoList += "Insurance: <None>";
			}
			else {
				while (!rsInsured->eof) {
					CString strInsCoName = VarString(rsInsured->Fields->Item["InsCoName"]->Value, "");
					strInsCoName.Trim();
					CString strRespType = VarString(rsInsured->Fields->Item["TypeName"]->Value, "");
					strRespType.Trim();

					CString str;
					str.Format("%s (%s)", strInsCoName, strRespType);
					if (strInsCoList.IsEmpty()) {
						strInsCoList = "Insurance: ";
					}
					else {
						strInsCoList += ", ";
					}
					strInsCoList += str;
					rsInsured->MoveNext();
				}
				rsInsured->Close();
			}

			m_nxstaticInsuredParties.SetWindowText(strInsCoList);
		}

		UpdateResponseControls();

	}NxCatchAll("Error in CEligibilityRequestDetailDlg::FullReload");
}

// (j.jones 2010-03-25 10:17) - PLID 37832 - changed the Load() function
// into InitialLoad() which loads the request & count of responses,
// and LoadResponse() which loads a specific response
// (j.jones 2010-03-29 09:37) - PLID 37619 - added bFiltersChanged which will
// determine whether the load is due to manually changing the filters
void CEligibilityRequestDetailDlg::LoadResponse(long nResponseID, BOOL bManualFiltersChanged /*= FALSE*/)
{
	try {

		CWaitCursor pWait;

		//m_nCurResponseIndex should have been updated prior to calling
		//this function, ASSERT if that did not happen
		ASSERT(m_aryCurResponseIDs.at(m_nCurResponseIndex) == nResponseID);

		// (j.jones 2010-03-29 09:33) - PLID 37619 - grab the manual filters
		long nBenefitTypeRefID = -1;
		long nCoverageLevelRefID = -1;
		long nServiceTypeRefID = -1;

		if(bManualFiltersChanged) {
			IRowSettingsPtr pBenefitRow = m_BenefitTypeFilterCombo->GetCurSel();
			if(pBenefitRow) {
				nBenefitTypeRefID = VarLong(pBenefitRow->GetValue(fccID), -1);
			}

			IRowSettingsPtr pCoverageRow = m_CoverageLevelFilterCombo->GetCurSel();
			if(pCoverageRow) {
				nCoverageLevelRefID = VarLong(pCoverageRow->GetValue(fccID), -1);
			}

			IRowSettingsPtr pServiceRow = m_ServiceTypeFilterCombo->GetCurSel();
			if(pServiceRow) {
				nServiceTypeRefID = VarLong(pServiceRow->GetValue(fccID), -1);
			}
		}

		m_DetailList->SetRedraw(FALSE);

		m_ResponseInfoList->Clear();
		m_DetailList->Clear();
		// (j.jones 2016-05-16 10:15) - NX-100357 - added Other Payors list
		m_OtherPayorsList->Clear();

		{
			//reset the widths for columns we dynamically alter
			IColumnSettingsPtr pBenefitCol = m_DetailList->GetColumn(erdcBenefit);
			pBenefitCol->PutColumnStyle(csVisible|csWidthData);
			pBenefitCol->PutStoredWidth(100);

			IColumnSettingsPtr pAuthCol = m_DetailList->GetColumn(erdcAuthorized);
			pAuthCol->PutColumnStyle(csVisible); //not width data
			pAuthCol->PutStoredWidth(70);

			IColumnSettingsPtr pServCol = m_DetailList->GetColumn(erdcServiceCode);
			pServCol->PutColumnStyle(csVisible|csWidthData);
			pServCol->PutStoredWidth(80);

			IColumnSettingsPtr pModCol = m_DetailList->GetColumn(erdcModifiers);
			pModCol->PutColumnStyle(csVisible|csWidthData);
			pModCol->PutStoredWidth(60);


			// (j.jones 2012-02-09 14:57) - PLID 48038 - removed this column, it is now combined with Description
			/*
			IColumnSettingsPtr pMsgCol = m_DetailList->GetColumn(erdcExtraMessage);
			pMsgCol->PutColumnStyle(csVisible|csWidthData);
			pMsgCol->PutStoredWidth(100);
			*/

			// (j.jones 2010-03-26 16:47) - PLID 37619 - with common filtering, some more columns are routinely empty
			IColumnSettingsPtr pDescCol = m_DetailList->GetColumn(erdcDescription);
			pDescCol->PutColumnStyle(csVisible|csWidthData);
			pDescCol->PutStoredWidth(75);

			IColumnSettingsPtr pInsCol = m_DetailList->GetColumn(erdcInsType);
			pInsCol->PutColumnStyle(csVisible|csWidthData);
			pInsCol->PutStoredWidth(95);
		}

		//reset this variable
		m_bCurResponseHasDetails = FALSE;		

		// (j.jones 2010-03-26 13:23) - PLID 37905 - moved the descriptions of
		// BenefitType, CoverageLevel, and ServiceType into EligibilityDataReferenceT
		// (j.jones 2010-03-26 16:00) - PLID 37619 - apply filters, and also track whether or not 
		// details actually exist in HasDetails, as they may just be filtered out
		// (c.haag 2010-10-20 15:07) - PLID 41017 - Added ServiceCodeRangeEnd
		// (j.jones 2016-05-16 10:15) - NX-100357 - added Other Payors list
		_RecordsetPtr rsResponseDetails = CreateParamRecordset("SELECT DateReceived, Response, "
			"Convert(bit, CASE WHEN EXISTS (SELECT TOP 1 EligibilityResponseDetailsT.ID "
			"	FROM EligibilityResponseDetailsT "
			"	WHERE EligibilityResponseDetailsT.ResponseID = EligibilityResponsesT.ID) THEN 1 ELSE 0 END) AS HasDetails "
			"FROM EligibilityResponsesT "
			"WHERE EligibilityResponsesT.ID = {INT} "
			""
			"SELECT EligibilityResponseDetailsT.ID AS DetailID, "
			"BenefitTypeRefID, BenefitTypeQ.Description AS BenefitType, "
			"CoverageLevelRefID, CoverageLevelQ.Description AS CoverageLevel, "
			"ServiceTypeRefID, ServiceTypeQ.Description AS ServiceType, "
			"InsuranceType, CoverageDesc, TimePeriod, "
			"Amount, Percentage, "
			"QuantityType, Quantity, "
			"Authorized, InNetwork, "
			"ServiceCode, Modifiers, "
			"ExtraMessage, ServiceCodeRangeEnd "
			"FROM EligibilityResponseDetailsT "
			"LEFT JOIN (SELECT ID, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 1) BenefitTypeQ ON EligibilityResponseDetailsT.BenefitTypeRefID = BenefitTypeQ.ID "
			"LEFT JOIN (SELECT ID, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 2) CoverageLevelQ ON EligibilityResponseDetailsT.CoverageLevelRefID = CoverageLevelQ.ID "
			"LEFT JOIN (SELECT ID, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 3) ServiceTypeQ ON EligibilityResponseDetailsT.ServiceTypeRefID = ServiceTypeQ.ID "
			"WHERE EligibilityResponseDetailsT.ResponseID = {INT} "
			//the filter details checkbox just disables default filtering, so if it is off, we don't try to use the FilterExcluded value,
			//and if it is on, we hide any detail that has FilterExcluded = 1 (remember it can be NULL)
			"AND ("
			"	{INT} = 0 "
			"	OR ("
			"		(BenefitTypeQ.FilterExcluded Is Null OR BenefitTypeQ.FilterExcluded = 0) "
			"		AND (CoverageLevelQ.FilterExcluded Is Null OR CoverageLevelQ.FilterExcluded = 0) "
			"		AND (ServiceTypeQ.FilterExcluded Is Null OR ServiceTypeQ.FilterExcluded = 0) "
			"		) "
			"	) "
			//these filters are for the manually selected dropdowns
			"AND ({INT} = -1 OR {INT} = BenefitTypeQ.ID) "
			"AND ({INT} = -1 OR {INT} = CoverageLevelQ.ID) "
			"AND ({INT} = -1 OR {INT} = ServiceTypeQ.ID) "
			"ORDER BY EligibilityResponseDetailsT.ID ASC; "
			""
			"SELECT InsuranceCompanyName, Address1, Address2, City, State, Zip, "
			"ContactName, ContactPhone, PayerID, PolicyNumber, GroupNumber "
			"FROM EligibilityResponseOtherPayorsT "
			"WHERE EligibilityResponseOtherPayorsT.ResponseID = {INT} "
			"ORDER BY EligibilityResponseOtherPayorsT.ID ASC",
			nResponseID,
			nResponseID, m_checkFilterDetails.GetCheck() ? 1 : 0,
			nBenefitTypeRefID, nBenefitTypeRefID,
			nCoverageLevelRefID, nCoverageLevelRefID,
			nServiceTypeRefID, nServiceTypeRefID,
			nResponseID);

		if(!rsResponseDetails->eof) {
			//first update the response date received, and the response text

			COleDateTime dtRecv = AdoFldDateTime(rsResponseDetails, "DateReceived");
			CString strResponse = AdoFldString(rsResponseDetails, "Response");

			CString strFmt;
			strFmt.Format("Response received on %s\r\n", FormatDateTimeForInterface(dtRecv, DTF_STRIP_SECONDS, dtoNaturalDatetime));
			m_nxstaticResponseLabel.SetWindowText(strFmt);

			AddNewResponseRow(strResponse);

			// (j.jones 2010-03-26 16:00) - PLID 37619 - with filtering in place,
			// we can't assume an empty list means no details exist, so track that info.
			m_bCurResponseHasDetails = AdoFldBool(rsResponseDetails, "HasDetails");
		}

		rsResponseDetails = rsResponseDetails->NextRecordset(NULL);

		BOOL bHasAuthorizedColumn = FALSE;
		BOOL bHasServiceCodeColumn = FALSE;
		BOOL bHasModifiersColumn = FALSE;
		BOOL bHasDescriptionColumn = FALSE;
		BOOL bHasInsTypeColumn = FALSE;

		// (j.jones 2010-03-26 17:32) - PLID 37619 - requery the filter combos while we manually load the details
		if(m_bCurResponseHasDetails) {
			if(!bManualFiltersChanged) {
				CString strServiceTypeWhere, strCoverageLevelWhere, strBenefitTypeWhere;
				
				strBenefitTypeWhere.Format("ListType = 1 AND ID IN "
					"(SELECT BenefitTypeRefID FROM EligibilityResponseDetailsT WHERE ResponseID = %li AND BenefitTypeRefID Is Not Null)", nResponseID);
				strCoverageLevelWhere.Format("ListType = 2 AND ID IN "
					"(SELECT CoverageLevelRefID FROM EligibilityResponseDetailsT WHERE ResponseID = %li AND CoverageLevelRefID Is Not Null)", nResponseID);
				strServiceTypeWhere.Format("ListType = 3 AND ID IN "
					"(SELECT ServiceTypeRefID FROM EligibilityResponseDetailsT WHERE ResponseID = %li AND ServiceTypeRefID Is Not Null)", nResponseID);

				if(m_checkFilterDetails.GetCheck()) {
					//hide excluded items
					strBenefitTypeWhere += " AND FilterExcluded = 0";
					strCoverageLevelWhere += " AND FilterExcluded = 0";
					strServiceTypeWhere += " AND FilterExcluded = 0";
				}

				//do not requery unless the where clause changed
				if((LPCTSTR)m_ServiceTypeFilterCombo->GetWhereClause() != strServiceTypeWhere) {
					m_ServiceTypeFilterCombo->PutWhereClause(_bstr_t(strServiceTypeWhere));
					m_ServiceTypeFilterCombo->Requery();

					IRowSettingsPtr pServiceRow = m_ServiceTypeFilterCombo->GetNewRow();
					pServiceRow->PutValue(fccID, (long)-1);
					pServiceRow->PutValue(fccDescription, " <All Types Of Service>");
					m_ServiceTypeFilterCombo->AddRowSorted(pServiceRow, NULL);
					m_ServiceTypeFilterCombo->PutCurSel(pServiceRow);
				}

				if((LPCTSTR)m_CoverageLevelFilterCombo->GetWhereClause() != strCoverageLevelWhere) {
					m_CoverageLevelFilterCombo->PutWhereClause(_bstr_t(strCoverageLevelWhere));
					m_CoverageLevelFilterCombo->Requery();

					IRowSettingsPtr pCoverageRow = m_CoverageLevelFilterCombo->GetNewRow();
					pCoverageRow->PutValue(fccID, (long)-1);
					pCoverageRow->PutValue(fccDescription, " <All Coverage Levels>");
					m_CoverageLevelFilterCombo->AddRowSorted(pCoverageRow, NULL);
					m_CoverageLevelFilterCombo->PutCurSel(pCoverageRow);
				}

				if((LPCTSTR)m_BenefitTypeFilterCombo->GetWhereClause() != strBenefitTypeWhere) {
					m_BenefitTypeFilterCombo->PutWhereClause(_bstr_t(strBenefitTypeWhere));
					m_BenefitTypeFilterCombo->Requery();

					IRowSettingsPtr pBenefitRow = m_BenefitTypeFilterCombo->GetNewRow();
					pBenefitRow->PutValue(fccID, (long)-1);
					pBenefitRow->PutValue(fccDescription, " <All Benefits>");
					m_BenefitTypeFilterCombo->AddRowSorted(pBenefitRow, NULL);
					m_BenefitTypeFilterCombo->PutCurSel(pBenefitRow);
				}
			}
		}
		else {
			m_ServiceTypeFilterCombo->Clear();
			m_CoverageLevelFilterCombo->Clear();
			m_BenefitTypeFilterCombo->Clear();

			//reset the filter combo where clauses, so that the next time we set them, it will force a requery
			//(as we do not requery unless they changed)
			m_ServiceTypeFilterCombo->PutWhereClause("");
			m_CoverageLevelFilterCombo->PutWhereClause("");
			m_BenefitTypeFilterCombo->PutWhereClause("");
		}
		
		//now load the response details
		while(!rsResponseDetails->eof) {

			long nDetailID = AdoFldLong(rsResponseDetails, "DetailID", -1);

			if(nDetailID == -1) {
				//there are no records in EligibilityResponseDetailsT, which either means
				//the response was rejected/invalid, or this is an old response prior to
				//the existence of this table
				rsResponseDetails->MoveNext();
				continue;
			}

			// (j.jones 2010-03-26 13:29) - PLID 37905 - added IDs for BenefitType, CoverageLevel, and ServiceType
			long nCoverageLevelRefID = AdoFldLong(rsResponseDetails, "CoverageLevelRefID", -1);
			long nServiceTypeRefID = AdoFldLong(rsResponseDetails, "ServiceTypeRefID", -1);
			long nBenefitTypeRefID = AdoFldLong(rsResponseDetails, "BenefitTypeRefID", -1);

			CString strBenefitType = AdoFldString(rsResponseDetails, "BenefitType", "");
			CString strCoverageLevel = AdoFldString(rsResponseDetails, "CoverageLevel", "");
			CString strServiceType = AdoFldString(rsResponseDetails, "ServiceType", "");
			CString strInsuranceType = AdoFldString(rsResponseDetails, "InsuranceType", "");
			CString strCoverageDesc = AdoFldString(rsResponseDetails, "CoverageDesc", "");
			CString strTimePeriod = AdoFldString(rsResponseDetails, "TimePeriod", "");
			_variant_t varAmount = rsResponseDetails->Fields->Item["Amount"]->Value;
			_variant_t varPercentage = rsResponseDetails->Fields->Item["Percentage"]->Value;
			CString strQuantityType = AdoFldString(rsResponseDetails, "QuantityType", "");
			_variant_t varQuantity = rsResponseDetails->Fields->Item["Quantity"]->Value;
			_variant_t varAuthorized = rsResponseDetails->Fields->Item["Authorized"]->Value;
			_variant_t varInNetwork = rsResponseDetails->Fields->Item["InNetwork"]->Value;
			CString strServiceCode = AdoFldString(rsResponseDetails, "ServiceCode", "");
			CString strModifiers = AdoFldString(rsResponseDetails, "Modifiers", "");
			CString strExtraMessage = AdoFldString(rsResponseDetails, "ExtraMessage", "");
			// (c.haag 2010-10-20 15:07) - PLID 41017
			CString strServiceCodeRangeEnd = AdoFldString(rsResponseDetails, "ServiceCodeRangeEnd", "");
			if (!strServiceCodeRangeEnd.IsEmpty()) {
				strServiceCode += " - " + strServiceCodeRangeEnd;
			}

			CString strBenefit = strBenefitType;
			//We have a benefit type, and then a time period, but they usually make sense
			//to be displayed together. For example there's Deductible with no time period
			//which shows the total deductible, but then Deductible: Year To Date and
			// Deductible: Remaining, which are easier to read when formatted together.
			if(!strTimePeriod.IsEmpty()) {
				strBenefit += ": ";
				strBenefit += strTimePeriod;
			}

			CString strValue = "";
			//Each response detail has an amount or a percentage or a time, just show one of them
			//However, there's nothing in the specs that specifically state only one can exist,
			//so output more than one in the rare/unlikely case it happens
			if(varAmount.vt == VT_CY) {
				CString strAmount;
				strAmount.Format("%s", FormatCurrencyForInterface(VarCurrency(varAmount)));
				strValue = strAmount;
			}
			if(varPercentage.vt == VT_R8) {
				CString strPercentage;
				// (j.jones 2010-06-14 15:12) - PLID 39154 - increased the precision here
				strPercentage.Format("%0.09g%%", VarDouble(varPercentage));
				if(!strValue.IsEmpty()) {
					strValue += ", ";
				}
				strValue += strPercentage;
			}
			if(!strQuantityType.IsEmpty() || varQuantity.vt == VT_R8) {
				//technically neither one of these should exist without the other,
				//but gracefully handle the output if they do
				CString strQuantityText = strQuantityType;
				if(varQuantity.vt == VT_R8) {
					if(!strQuantityText.IsEmpty()) {
						strQuantityText += ": ";
					}

					CString strQuantity;
					// (j.jones 2010-06-14 15:12) - PLID 39154 - increased the precision here
					strQuantity.Format("%0.09g", VarDouble(varQuantity));
					strQuantityText += strQuantity;
				}

				if(!strValue.IsEmpty() && !strQuantityText.IsEmpty()) {
					strValue += ", ";
				}
				strValue += strQuantityText;
			}

			// (j.jones 2012-02-09 14:26) - PLID 48038 - combine strCoverageDesc and strExtraMessage
			CString strDescription;
			strCoverageDesc.TrimLeft(); strCoverageDesc.TrimRight();
			strExtraMessage.TrimLeft(); strExtraMessage.TrimRight();
			strDescription = strCoverageDesc;
			if(!strExtraMessage.IsEmpty()) {
				if(!strDescription.IsEmpty()) {
					strDescription += "\n";
				}
				strDescription += strExtraMessage;
			}			

			IRowSettingsPtr pRow = m_DetailList->GetNewRow();

			pRow->PutValue(erdcID, (long)nDetailID);
			pRow->PutValue(erdcServiceTypeRefID, (long)nServiceTypeRefID); // (j.jones 2010-03-26 13:29) - PLID 37905
			pRow->PutValue(erdcTypeOfService, _bstr_t(strServiceType));
			pRow->PutValue(erdcCoverageLevelRefID, (long)nCoverageLevelRefID); // (j.jones 2010-03-26 13:29) - PLID 37905
			pRow->PutValue(erdcCoverage, _bstr_t(strCoverageLevel));
			pRow->PutValue(erdcBenefitTypeRefID, (long)nBenefitTypeRefID); // (j.jones 2010-03-26 13:29) - PLID 37905
			pRow->PutValue(erdcBenefit, _bstr_t(strBenefitType));
			pRow->PutValue(erdcTimePeriod, _bstr_t(strTimePeriod));						
			pRow->PutValue(erdcBenefit, _bstr_t(strBenefit));
			pRow->PutValue(erdcValue, _bstr_t(strValue));
			pRow->PutValue(erdcDescription, _bstr_t(strDescription));
			pRow->PutValue(erdcInNetwork, varInNetwork);
			pRow->PutValue(erdcAuthorized, varAuthorized);
			pRow->PutValue(erdcInsType, _bstr_t(strInsuranceType));
			pRow->PutValue(erdcServiceCode, _bstr_t(strServiceCode));
			pRow->PutValue(erdcModifiers, _bstr_t(strModifiers));	

			m_DetailList->AddRowSorted(pRow, NULL);

			//track if we actually filled in some rarely-used columns
			if(!bHasAuthorizedColumn && varAuthorized.vt == VT_BOOL) {
				bHasAuthorizedColumn = TRUE;
			}
			if(!bHasServiceCodeColumn && !strServiceCode.IsEmpty()) {
				bHasServiceCodeColumn = TRUE;
			}
			if(!bHasModifiersColumn && !strModifiers.IsEmpty()) {
				bHasModifiersColumn = TRUE;
			}
			// (j.jones 2010-03-26 16:47) - PLID 37619 - with common filtering, some more columns are routinely empty
			// (j.jones 2012-02-09 14:57) - PLID 48038 - strExtraMessage is now combined in this column
			if(!bHasDescriptionColumn && (!strCoverageDesc.IsEmpty() || !strExtraMessage.IsEmpty())) {
				bHasDescriptionColumn = TRUE;
			}
			if(!bHasInsTypeColumn && !strInsuranceType.IsEmpty()) {
				bHasInsTypeColumn = TRUE;
			}

			rsResponseDetails->MoveNext();
		}

		// (j.jones 2016-05-16 10:15) - NX-100357 - added Other Payors list
		rsResponseDetails = rsResponseDetails->NextRecordset(NULL);
		while (!rsResponseDetails->eof) {

			IRowSettingsPtr pRow = m_OtherPayorsList->GetNewRow();
			pRow->PutValue(opcInsuranceName, _bstr_t(AdoFldString(rsResponseDetails, "InsuranceCompanyName", "")));

			CString strAddress1 = AdoFldString(rsResponseDetails, "Address1", "");
			CString strAddress2 = AdoFldString(rsResponseDetails, "Address2", "");
			CString strCity = AdoFldString(rsResponseDetails, "City", "");
			CString strState = AdoFldString(rsResponseDetails, "State", "");
			CString strZip = AdoFldString(rsResponseDetails, "Zip", "");

			CString strAddress = strAddress1 + " " + strAddress2;
			strAddress.TrimRight();
			if (!strAddress.IsEmpty() && !strCity.IsEmpty()) {
				strAddress += ", ";
			}
			strAddress += strCity;
			if (!strAddress.IsEmpty() && !strState.IsEmpty()) {
				strAddress += ", ";
			}
			strAddress += strState;
			if (!strAddress.IsEmpty() && !strZip.IsEmpty()) {
				strAddress += " ";
			}
			strAddress += strZip;

			pRow->PutValue(opcFullAddress, _bstr_t(strAddress));
			
			CString strContactName = AdoFldString(rsResponseDetails, "ContactName", "");;
			CString strContactPhone = AdoFldString(rsResponseDetails, "ContactPhone", "");;
			CString strContactInfo = strContactName;
			if (!strContactName.IsEmpty() && !strContactPhone.IsEmpty()) {
				strContactInfo += ", ";
			}
			strContactInfo += strContactPhone;

			pRow->PutValue(opcContactInfo, _bstr_t(strContactInfo));
			pRow->PutValue(opcPayerID, _bstr_t(AdoFldString(rsResponseDetails, "PayerID", "")));
			pRow->PutValue(opcPolicyNumber, _bstr_t(AdoFldString(rsResponseDetails, "PolicyNumber", "")));
			pRow->PutValue(opcGroupNumber, _bstr_t(AdoFldString(rsResponseDetails, "GroupNumber", "")));

			m_OtherPayorsList->AddRowAtEnd(pRow, NULL);

			rsResponseDetails->MoveNext();
		}
		rsResponseDetails->Close();

		//force a maximum width for the "benefit" column
		IColumnSettingsPtr pBenefitCol = m_DetailList->GetColumn(erdcBenefit);
		if(pBenefitCol->GetStoredWidth() > 220) {
			pBenefitCol->PutColumnStyle(csVisible);
			pBenefitCol->PutStoredWidth(220);
		}

		//some fields are rarely filled, so let's hide those commonly empty columns
		//unless they were filled on any line
		if(!bHasAuthorizedColumn) {
			IColumnSettingsPtr pAuthCol = m_DetailList->GetColumn(erdcAuthorized);
			pAuthCol->PutColumnStyle(csVisible|csFixedWidth);
			pAuthCol->PutStoredWidth(0);
		}

		if(!bHasServiceCodeColumn) {
			IColumnSettingsPtr pServCol = m_DetailList->GetColumn(erdcServiceCode);
			pServCol->PutColumnStyle(csVisible|csFixedWidth);
			pServCol->PutStoredWidth(0);
		}

		if(!bHasModifiersColumn) {
			IColumnSettingsPtr pModCol = m_DetailList->GetColumn(erdcModifiers);
			pModCol->PutColumnStyle(csVisible|csFixedWidth);
			pModCol->PutStoredWidth(0);
		}
		
		// (j.jones 2012-02-09 14:57) - PLID 48038 - removed this column, it is now combined with Description
		/*
		if(!bHasExtraMessageColumn) {
			IColumnSettingsPtr pMsgCol = m_DetailList->GetColumn(erdcExtraMessage);
			pMsgCol->PutColumnStyle(csVisible|csFixedWidth);
			pMsgCol->PutStoredWidth(0);	
		}
		*/

		// (j.jones 2010-03-26 16:47) - PLID 37619 - with common filtering, some more columns are routinely empty
		if(!bHasDescriptionColumn) {
			IColumnSettingsPtr pDescCol = m_DetailList->GetColumn(erdcDescription);
			pDescCol->PutColumnStyle(csVisible|csFixedWidth);
			pDescCol->PutStoredWidth(0);
		}
		
		if(!bHasInsTypeColumn) {
			IColumnSettingsPtr pInsCol = m_DetailList->GetColumn(erdcInsType);
			pInsCol->PutColumnStyle(csVisible|csFixedWidth);
			pInsCol->PutStoredWidth(0);	
		}

		m_DetailList->SetRedraw(TRUE);

		UpdateResponseControls();

	}NxCatchAll("Error in CEligibilityRequestDetailDlg::LoadResponse");
}

void CEligibilityRequestDetailDlg::OnBtnEditEligRequest() 
{
	try {

		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility,sptWrite))
			return;

		long nRequestID = GetRequestID();

		CWaitCursor pWait;
		
		//edit the request
		//GetMainFrame()->ShowEligibilityRequestDlg(this,nRequestID,-1 ,-1,GetNxColor(GNC_FINANCIAL, 1));

		CEligibilityRequestDlg dlg(this);
		dlg.m_nID = nRequestID;
		if(dlg.DoModal() == IDOK) {

			//if we changed something, reload this list
			FullReload(m_nCurResponseIndex);

			//and also tell our parent to requery, since something changed
			// PLID 62644 - new method for refreshing parent
			RefreshParent();
		}

	}NxCatchAll("Error in CEligibilityRequestDetailDlg::OnBtnEditEligRequest");
}

void CEligibilityRequestDetailDlg::OnBtnRebatchEligRequest() 
{
	try {

		long nRequestID = GetRequestID();

		CWaitCursor pWait;

		//ensure the request is batched, and unselected, but don't change the selection
		//if it is already in the batch
		ExecuteParamSql("UPDATE EligibilityRequestsT SET Selected = (CASE WHEN Batched = 0 THEN 0 ELSE Selected END), Batched = 1 WHERE ID = {INT}", nRequestID);

		//tell the user it succeeded
		AfxMessageBox("The eligibility request has been re-batched.");

		//now tell our parent to requery, since the status changed
		// PLID 62644 - new method for refreshing parent
		RefreshParent();

	}NxCatchAll("Error in CEligibilityRequestDetailDlg::OnBtnRebatchEligRequest");
}

// (j.jones 2010-03-25 09:11) - PLID 37832 - split into request & response lists
void CEligibilityRequestDetailDlg::AddNewRequestRow(CString strText, COLORREF cTextColor /*= RGB(0,0,0)*/)
{
	IRowSettingsPtr pRow = m_RequestInfoList->GetNewRow();
	pRow->PutValue(0, _bstr_t(strText));
	pRow->PutForeColor(cTextColor);
	m_RequestInfoList->AddRowAtEnd(pRow,NULL);
}

// (j.jones 2010-03-25 09:11) - PLID 37832 - split into request & response lists
void CEligibilityRequestDetailDlg::AddNewResponseRow(CString strText, COLORREF cTextColor /*= RGB(0,0,0)*/)
{
	IRowSettingsPtr pRow = m_ResponseInfoList->GetNewRow();
	pRow->PutValue(0, _bstr_t(strText));
	pRow->PutForeColor(cTextColor);
	m_ResponseInfoList->AddRowAtEnd(pRow,NULL);
}

// (j.jones 2010-03-25 10:19) - PLID 37832 - added UpdateResponseControls,
// which will hide/move/size/update the response controls based on the current
// response information
void CEligibilityRequestDetailDlg::UpdateResponseControls()
{
	try {

		CRect rcTOSComboRect;
		GetDlgItem(IDC_ELIG_SERVICETYPE_COMBO)->GetWindowRect(&rcTOSComboRect);
		ScreenToClient(&rcTOSComboRect);	

		CRect rcDetailListRect;
		GetDlgItem(IDC_ELIGIBILITY_RESPONSE_DETAILS_LIST)->GetWindowRect(&rcDetailListRect);
		ScreenToClient(&rcDetailListRect);

		CRect rcResponseListRect;
		GetDlgItem(IDC_ELIGIBILITY_RESPONSE_INFO)->GetWindowRect(&rcResponseListRect);
		ScreenToClient(&rcResponseListRect);
				
		//if this is the first time we've called this function,
		//cache the current placement of the detail & response lists
		if(m_nDetailListInitialTop == -1) {
			m_nDetailListInitialTop = rcDetailListRect.top;
			m_nDetailListInitialBottom = rcDetailListRect.bottom;
			m_nResponseListInitialTop = rcResponseListRect.top;
			m_nResponseListInitialBottom = rcResponseListRect.bottom;
		}

		CString strCount = "";

		BOOL bShowArrows = TRUE;

		if(m_nCurResponseIndex == -1 || (long)m_aryCurResponseIDs.size() <= 1) {
			//we have either no responses, or only one,
			//or no response is selected currently,
			//so both arrows need disabled and hidden
			m_btnLeftResponse.EnableWindow(FALSE);
			m_btnRightResponse.EnableWindow(FALSE);

			bShowArrows = FALSE;
		}
		else if(m_nCurResponseIndex == 0) {
			//we are on the first response
			m_btnLeftResponse.EnableWindow(FALSE);
			m_btnRightResponse.EnableWindow(TRUE);
			strCount.Format("Response 1 of %li", (long)m_aryCurResponseIDs.size());
		}
		else if(m_nCurResponseIndex == (long)m_aryCurResponseIDs.size() - 1) {
			//we are on the last response
			m_btnLeftResponse.EnableWindow(TRUE);
			m_btnRightResponse.EnableWindow(FALSE);
			strCount.Format("Response %li of %li", (long)m_aryCurResponseIDs.size(), (long)m_aryCurResponseIDs.size());
		}
		else {
			//we have more than 1 response, one is selected,
			//and it is not the first nor last response
			m_btnLeftResponse.EnableWindow(TRUE);
			m_btnRightResponse.EnableWindow(TRUE);
			strCount.Format("Response %li of %li", m_nCurResponseIndex + 1, (long)m_aryCurResponseIDs.size());
		}

		m_nxstaticResponseCountLabel.SetWindowText(strCount);

		//show or hide the arrows
		m_btnLeftResponse.ShowWindow(bShowArrows ? SW_SHOW : SW_HIDE);
		m_btnRightResponse.ShowWindow(bShowArrows ? SW_SHOW : SW_HIDE);

		// (j.jones 2010-03-26 16:00) - PLID 37619 - with filtering in place,
		// we can't assume an empty list means no details exist
		if(!m_bCurResponseHasDetails) {
			//hide the detail list, and always show the response info. list

			// (j.jones 2016-05-19 16:34) - NX-100357 - now the list only has its right & bottom sides changed
			rcResponseListRect.right = rcDetailListRect.right;
			rcResponseListRect.bottom = rcDetailListRect.bottom;
			
			// (j.jones 2010-03-26 15:56) - PLID 37619 - hide the filter options
			//(e.lally 2011-09-07) PLID 37940 - Use ShowDlgItem for consistency
			ShowDlgItem(IDC_SERVICETYPE_FILTER_LABEL, SW_HIDE);
			ShowDlgItem(IDC_ELIG_SERVICETYPE_COMBO, SW_HIDE);
			ShowDlgItem(IDC_COVERAGE_FILTER_LABEL, SW_HIDE);
			ShowDlgItem(IDC_ELIG_COVERAGELEVEL_COMBO, SW_HIDE);
			ShowDlgItem(IDC_BENEFIT_FILTER_LABEL, SW_HIDE);
			ShowDlgItem(IDC_ELIG_BENEFITTYPE_COMBO, SW_HIDE);
			ShowDlgItem(IDC_CHECK_FILTER_DETAILS, SW_HIDE);
			ShowDlgItem(IDC_BTN_CONFIG_RESPONSE_FILTERING_REVIEW, SW_HIDE);	
			ShowDlgItem(IDC_ELIGIBILITY_RESPONSE_DETAILS_LIST, SW_HIDE);
			ShowDlgItem(IDC_ELIGIBILITY_RESPONSE_INFO, SW_SHOWNA);
			GetDlgItem(IDC_ELIGIBILITY_RESPONSE_INFO)->MoveWindow(rcResponseListRect);
			
			// (j.jones 2016-05-19 16:37) - NX-100357 - hide the Other Payers list
			ShowDlgItem(IDC_OTHER_PAYORS_LABEL, SW_HIDE);
			ShowDlgItem(IDC_OTHER_PAYORS_LIST, SW_HIDE);
		}
		else {
			//show the detail list

			//(e.lally 2011-09-07) PLID 37940 - Use ShowDlgItem for showing Active-X controls with the SW_SHOWNA to force it to be drawn on the screen.
			//	we'll use it for all controls in this section for consistency in case someone tries to copy this code later on.

			rcDetailListRect.top = m_nDetailListInitialTop;
			rcDetailListRect.bottom = m_nDetailListInitialBottom;

			// (j.jones 2016-05-19 16:29) - NX-100357 - we now never hide the response info list

			rcResponseListRect.top = m_nResponseListInitialTop;
			rcResponseListRect.bottom = m_nResponseListInitialBottom;

			ShowDlgItem(IDC_ELIGIBILITY_RESPONSE_INFO, SW_SHOWNA);
			GetDlgItem(IDC_ELIGIBILITY_RESPONSE_INFO)->MoveWindow(rcResponseListRect);

			if(m_bHasDefaultFilters) {
				//only show if we have default filters
				GetDlgItem(IDC_CHECK_FILTER_DETAILS)->ShowWindow(SW_SHOWNA);
			}
			else {
				GetDlgItem(IDC_CHECK_FILTER_DETAILS)->ShowWindow(SW_HIDE);
			}
			ShowDlgItem(IDC_BTN_CONFIG_RESPONSE_FILTERING_REVIEW, SW_SHOWNA);
			ShowDlgItem(IDC_ELIGIBILITY_RESPONSE_DETAILS_LIST, SW_SHOWNA);
			GetDlgItem(IDC_ELIGIBILITY_RESPONSE_DETAILS_LIST)->MoveWindow(rcDetailListRect);

			ShowDlgItem(IDC_SERVICETYPE_FILTER_LABEL, SW_SHOWNA);
			ShowDlgItem(IDC_ELIG_SERVICETYPE_COMBO, SW_SHOWNA);			
			ShowDlgItem(IDC_COVERAGE_FILTER_LABEL, SW_SHOWNA);
			ShowDlgItem(IDC_ELIG_COVERAGELEVEL_COMBO, SW_SHOWNA);			
			ShowDlgItem(IDC_BENEFIT_FILTER_LABEL, SW_SHOWNA);
			ShowDlgItem(IDC_ELIG_BENEFITTYPE_COMBO, SW_SHOWNA);			
			ShowDlgItem(IDC_CHECK_FILTER_DETAILS, SW_SHOWNA);
			// (j.jones 2016-05-19 16:37) - NX-100357 - show the Other Payers list
			ShowDlgItem(IDC_OTHER_PAYORS_LABEL, SW_SHOWNA);
			ShowDlgItem(IDC_OTHER_PAYORS_LIST, SW_SHOWNA);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-25 10:17) - PLID 37832 - added ability to traverse responses when multiple exist
void CEligibilityRequestDetailDlg::OnEligibilityPrevResponse()
{
	try {

		if(m_nCurResponseIndex > 0) {
			m_nCurResponseIndex--;
			long nResponseID = m_aryCurResponseIDs.at(m_nCurResponseIndex);
			LoadResponse(nResponseID);
		}
		else {
			//should not have been allowed to click on this button
			//if we were on the first response
			ASSERT(FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-25 10:17) - PLID 37832 - added ability to traverse responses when multiple exist
void CEligibilityRequestDetailDlg::OnEligibilityNextResponse()
{
	try {

		if(m_nCurResponseIndex < ((long)m_aryCurResponseIDs.size() - 1)) {
			m_nCurResponseIndex++;
			long nResponseID = m_aryCurResponseIDs.at(m_nCurResponseIndex);
			LoadResponse(nResponseID);
		}
		else {
			//should not have been allowed to click on this button
			//if we were on the last response
			ASSERT(FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-26 15:50) - PLID 37619 - added ability to filter the details
void CEligibilityRequestDetailDlg::OnCheckFilterDetails()
{
	try {

		//reload the currently displayed response
		if(m_nCurResponseIndex >= 0) {
			//this is not considered the "manual filter" so do not pass in TRUE here
			LoadResponse(m_aryCurResponseIDs.at(m_nCurResponseIndex), FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-26 15:50) - PLID 37619 - added ability to filter the details
void CEligibilityRequestDetailDlg::OnBtnConfigResponseFiltering()
{
	try {

		CEligibilityResponseFilteringConfigDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			//if they clicked ok, reload, something could have changed
			//(FullReload will also ensure that the filter dropdowns are definitely requeried)
			FullReload(m_nCurResponseIndex);
		}

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CEligibilityRequestDetailDlg, CNxDialog)
	ON_EVENT(CEligibilityRequestDetailDlg, IDC_ELIG_SERVICETYPE_COMBO, 16, OnSelChosenEligServicetypeCombo, VTS_DISPATCH)
	ON_EVENT(CEligibilityRequestDetailDlg, IDC_ELIG_COVERAGELEVEL_COMBO, 16, OnSelChosenEligCoveragelevelCombo, VTS_DISPATCH)
	ON_EVENT(CEligibilityRequestDetailDlg, IDC_ELIG_BENEFITTYPE_COMBO, 16, OnSelChosenEligBenefittypeCombo, VTS_DISPATCH)
	ON_EVENT(CEligibilityRequestDetailDlg, IDC_ELIG_PAY_GROUP_LIST, 8, OnEditingStartingEligPayGroupList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEligibilityRequestDetailDlg, IDC_ELIG_PAY_GROUP_LIST, 9, OnEditingFinishingEligPayGroupList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEligibilityRequestDetailDlg, IDC_ELIG_PAY_GROUP_LIST, 10, OnEditingFinishedEligPayGroupList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)	
	ON_EVENT(CEligibilityRequestDetailDlg, IDC_ELIG_DEDOOP_LIST, 9, OnEditingFinishingEligDedoopList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEligibilityRequestDetailDlg, IDC_ELIG_DEDOOP_LIST, 10, OnEditingFinishedEligDedoopList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

// (j.jones 2010-03-29 08:37) - PLID 37619 - added ability to filter the details
void CEligibilityRequestDetailDlg::OnSelChosenEligServicetypeCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_ServiceTypeFilterCombo->SetSelByColumn(fccID, (long)-1);
		}

		//reload the currently displayed response
		if(m_nCurResponseIndex >= 0) {
			LoadResponse(m_aryCurResponseIDs.at(m_nCurResponseIndex), TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-29 08:37) - PLID 37619 - added ability to filter the details
void CEligibilityRequestDetailDlg::OnSelChosenEligCoveragelevelCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_CoverageLevelFilterCombo->SetSelByColumn(fccID, (long)-1);
		}

		//reload the currently displayed response
		if(m_nCurResponseIndex >= 0) {
			LoadResponse(m_aryCurResponseIDs.at(m_nCurResponseIndex), TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-29 08:37) - PLID 37619 - added ability to filter the details
void CEligibilityRequestDetailDlg::OnSelChosenEligBenefittypeCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_BenefitTypeFilterCombo->SetSelByColumn(fccID, (long)-1);
		}

		//reload the currently displayed response
		if(m_nCurResponseIndex >= 0) {
			LoadResponse(m_aryCurResponseIDs.at(m_nCurResponseIndex), TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-07 09:39) - PLID 39534 - supported showing multiple requests on this dialog
void CEligibilityRequestDetailDlg::OnEligibilityPrevReq()
{
	try {

		m_nCurRequestIndex--;

		if(m_nCurRequestIndex < 0) {
			m_nCurRequestIndex = 0;
		}

		UpdateRequestButtons();

		FullReload(0);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-07 09:39) - PLID 39534 - supported showing multiple requests on this dialog
void CEligibilityRequestDetailDlg::OnEligibilityNextReq()
{
	try {

		m_nCurRequestIndex++;

		if(m_nCurRequestIndex >= (long)m_aryCurFilteredRequestIDs.size()) {
			m_nCurRequestIndex = (long)m_aryCurFilteredRequestIDs.size() - 1;
		}

		UpdateRequestButtons();

		FullReload(0);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-07 09:55) - PLID 39534 - gets the request ID at m_nCurRequestIndex
long CEligibilityRequestDetailDlg::GetRequestID()
{
	try {

		if(m_nCurRequestIndex < 0 || m_nCurRequestIndex >= (long)m_aryCurFilteredRequestIDs.size()) {
			ThrowNxException("Tried to load a request with out of bounds index %li!", m_nCurRequestIndex);
		}

		return (long)m_aryCurFilteredRequestIDs.at(m_nCurRequestIndex);

	}NxCatchAll(__FUNCTION__);

	return -1;
}

// (j.jones 2010-07-07 10:03) - PLID 39534 - updates the request buttons with proper text, enabling/disabling
void CEligibilityRequestDetailDlg::UpdateRequestButtons()
{
	try {

		//usually, these buttons are never shown unless you have multiple requests displayed,
		//but if you started with multiple requests and apply a filter such that only one remains,
		//we do not hide the buttons, and they will still need updated
		if(m_aryCurFilteredRequestIDs.size() > 0) {

			CString strCount;

			if(m_nCurRequestIndex == -1) {
				m_nCurRequestIndex = 0;
			}
			
			//shortcut logic to handle the case of one request
			if (m_aryCurFilteredRequestIDs.size() == 1) {
				m_btnLeftRequest.EnableWindow(FALSE);
				m_btnRightRequest.EnableWindow(FALSE);
				strCount.Format("Request 1 of 1");
			}
			else if(m_nCurRequestIndex == 0) {
				//we are on the first request
				m_btnLeftRequest.EnableWindow(FALSE);
				m_btnRightRequest.EnableWindow(TRUE);
				strCount.Format("Request 1 of %li", m_aryCurFilteredRequestIDs.size());
			}
			else if(m_nCurRequestIndex == m_aryCurFilteredRequestIDs.size() - 1) {
				//we are on the last request
				m_btnLeftRequest.EnableWindow(TRUE);
				m_btnRightRequest.EnableWindow(FALSE);
				strCount.Format("Request %li of %li", m_aryCurFilteredRequestIDs.size(), m_aryCurFilteredRequestIDs.size());
			}
			else {
				//we have more than 1 request, one is selected,
				//and it is not the first nor last request
				m_btnLeftRequest.EnableWindow(TRUE);
				m_btnRightRequest.EnableWindow(TRUE);
				strCount.Format("Request %li of %li", m_nCurRequestIndex + 1, m_aryCurFilteredRequestIDs.size());
			}

			GetDlgItem(IDC_ELIGBTN_CUR_REQ_INFO)->SetWindowText(strCount);
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, March 30, 2012) - PLID 48696 - Get the response ID. 
long CEligibilityRequestDetailDlg::GetResponseID()
{
	try {

		if(m_nCurResponseIndex < 0 || m_nCurResponseIndex >= (long)m_aryCurFilteredRequestIDs.size()) {
			// (b.spivey, March 30, 2012) - PLID 48696 - It is very likely we won't have a response. In this case, don't error out. 
			return -1; 
		}

		return (long)m_aryCurFilteredRequestIDs.at(m_nCurResponseIndex);

	}NxCatchAll(__FUNCTION__);

	return -1;

}

// (b.spivey, March 30, 2012) - PLID 48696 - Event handler for on print preview. 
void CEligibilityRequestDetailDlg::OnBnClickedBtnPrintPreview()
{
	try {
		
		//Get the filters if they exist.
		long nBenefitTypeRefID = -1, nCoverageLevelRefID = -1, nServiceTypeRefID = -1;

		//Benefit ID 
		IRowSettingsPtr pBenefitRow = m_BenefitTypeFilterCombo->GetCurSel();
		if(pBenefitRow) {
			nBenefitTypeRefID = VarLong(pBenefitRow->GetValue(fccID), -1);
		}

		//Coverage ID 
		IRowSettingsPtr pCoverageRow = m_CoverageLevelFilterCombo->GetCurSel();
		if(pCoverageRow) {
			nCoverageLevelRefID = VarLong(pCoverageRow->GetValue(fccID), -1);
		}

		//Service ID
		IRowSettingsPtr pServiceRow = m_ServiceTypeFilterCombo->GetCurSel();
		if(pServiceRow) {
			nServiceTypeRefID = VarLong(pServiceRow->GetValue(fccID), -1);
		}

		// (b.spivey, March 30, 2012) - PLID 48696 - This is for the report query. 
		// Located in ReportInfoFinancial.cpp, Case 723. Search this for quick access: // E-Eligibility Request Report 
		CString strWhere, strResponseFilter; 

		//If we have a responseID, great. Lets filter. If not, then don't even bother. 
		if(GetResponseID() > 0) {
			strResponseFilter.Format("AND ResponseQ.ID = %li", GetResponseID()); 
		}
		else {
			strResponseFilter = ""; 
		}

		//Building the where clause here. 
		strWhere.Format(
			"WHERE EligibilityRequestsT.ID = %li " 
			"%s "
			"AND ( "
			"	%li = 0 "
			"	OR ( "
			"		(DetailsQ.BenefitExcluded Is Null OR DetailsQ.BenefitExcluded  = 0) "
			"		AND (DetailsQ.CoverageExcluded  Is Null OR DetailsQ.CoverageExcluded  = 0) "
			"		AND (DetailsQ.ServiceExcluded Is Null OR DetailsQ.ServiceExcluded = 0) "
			"	) "
			") "
			"AND (%li = -1 OR %li = DetailsQ.BenefitID) "
			"AND (%li = -1 OR %li = DetailsQ.CoverageID) "
			"AND (%li = -1 OR %li = DetailsQ.ServiceID) ", GetRequestID(),
		strResponseFilter, m_checkFilterDetails.GetCheck() ? 1 : 0,
		nBenefitTypeRefID, nBenefitTypeRefID,
		nCoverageLevelRefID, nCoverageLevelRefID,
		nServiceTypeRefID, nServiceTypeRefID);

		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(723)]);
		// (b.spivey, March 30, 2012) - PLID 48696 - Set the custom where clause and then run it. 
		infReport.SetExtraValue(strWhere); 

		//Set up the parameters. 
		CPtrArray paParams;
		CRParameterInfo *paramInfo;
		
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = GetCurrentUserName();
		paramInfo->m_Name = "CurrentUserName";
		paParams.Add(paramInfo);

		// (b.spivey, March 30, 2012) - PLID 48696 - This parameter will not effect the values returned to the report. So what I 
		//	 did was set it to the value that just sets the date range to say "All dates" and just let it say "All dates" at the 
		//	 top of the report. 
		COleDateTime dt = COleDateTime(1000, 1, 1, 0, 0, 0);
		CString strDate = dt.Format("%m/%d/%Y");

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strDate;
		paramInfo->m_Name = "DateFrom";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strDate;
		paramInfo->m_Name = "DateTo";
		paParams.Add((void *)paramInfo); 

		// (b.spivey, June 05, 2012) - PLID 48696 - Provider name param. 
		paramInfo = new CRParameterInfo; 
		paramInfo->m_Data = m_strProvName;
		paramInfo->m_Name = "ProvName"; 
		paParams.Add((void *)paramInfo); 

		RunReport(&infReport, &paParams, TRUE, (CWnd *)this, "E-Eligibility Requests Preview");
		//Clear params
		ClearRPIParameterList(&paParams); 

		//Just close the dialog, it's not like they'll lose work. 
		OnOK(); 
	} NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-10-10 14:34) - PLID 62644 - Refresh EEligibility Tab if it is the active sheet; refresh eligibility review dlg if it is present
void CEligibilityRequestDetailDlg::RefreshParent()
{
	try{

		if (GetMainFrame()) {
			if (GetMainFrame()->IsActiveView(FINANCIAL_MODULE_NAME)) {
				CFinView* pView = (CFinView *)GetMainFrame()->GetOpenView(FINANCIAL_MODULE_NAME);
				if (pView) {
					if (pView->GetActiveTab() == FinancialModule::EEligibilityTab && pView->GetActiveSheet() != NULL) {
						((CEEligibilityTabDlg*)pView->GetActiveSheet())->UpdateView(); // if EEligibility tab is active
					}
				}
			}
			if (GetMainFrame()->m_pEligibilityReviewDlg){
				if (IsWindow(GetMainFrame()->m_pEligibilityReviewDlg->GetSafeHwnd())){
					if (GetMainFrame()->m_pEligibilityReviewDlg->IsWindowVisible()){
						GetMainFrame()->m_pEligibilityReviewDlg->m_EligibilityList->Requery(); // if eligibility review dialog is open
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRequestDetailDlg::OnOK()
{
	try{
		GetMainFrame()->PostMessage(NXM_ELIGIBILITYREQUEST_DETAIL_CLOSED);
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRequestDetailDlg::OnCancel()
{
	try{
		GetMainFrame()->PostMessage(NXM_ELIGIBILITYREQUEST_DETAIL_CLOSED);
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}


// (b.eyers 2015-04-17) - PLID 44309 - Go to patient button, leave dialog open still
void CEligibilityRequestDetailDlg::OnBtnGoToPatient()
{
	try {

		if (m_nCurrentRequestPatientID != -1) {
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if (!pMainFrame->m_patToolBar.DoesPatientExistInList(m_nCurrentRequestPatientID)) {
					if (IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
						return;
					}
				}

				if (pMainFrame->m_patToolBar.TrySetActivePatientID(m_nCurrentRequestPatientID)) {
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if (pView) {
						pView->UpdateView();
					}
				}
			}
			else {
				MsgBox(MB_ICONSTOP | MB_OK, "ERROR - CEligibilityRequestDetailDlg: Cannot Open Mainframe");
			}
		}

	}NxCatchAll("Error in CEligibilityRequestDetailDlg::OnBtnGoToPatient");
}

//handles the response type radio button filter to show all requests/responses
void CEligibilityRequestDetailDlg::OnRadioResponseTypeAll()
{
	try {

		//clear out our current filtered list
		m_aryCurFilteredRequestIDs.clear();

		//reset it to the full list of requests
		m_aryCurFilteredRequestIDs.insert(m_aryCurFilteredRequestIDs.end(), m_aryAllRequestIDs.begin(), m_aryAllRequestIDs.end());

		//jump to the first request in the list
		m_nCurRequestIndex = 0;

		//update the left/right button UI & count
		UpdateRequestButtons();

		//load the first request
		FullReload(0);

	}NxCatchAll(__FUNCTION__);
}

//if m_aryAllResponseIDs is filled, this will return an AND clause filtering
//on EligibilityResponsesT to filter the results only by the desired reponse IDs
CSqlFragment CEligibilityRequestDetailDlg::GetResponseFilterWhere()
{
	if (m_aryAllResponseIDs.size() == 0) {
		return CSqlFragment("");
	}
	else {
		return CSqlFragment(" AND EligibilityResponsesT.ID IN ({INTVECTOR}) ", m_aryAllResponseIDs);
	}
}

//handles the response type radio button filter to show only active requests/responses
void CEligibilityRequestDetailDlg::OnRadioResponseTypeActive()
{
	try {

		//required to prevent weird message looping in modeless dialogs
		if (!IsDlgButtonChecked(IDC_RADIO_RESPONSE_TYPE_ACTIVE)) {
			return;
		}
		
		ApplyRadioButtonResponseTypeFilter(GetEligibilityActiveCoverageInnerJoin(), "active coverage");

	}NxCatchAll(__FUNCTION__);
}

//handles the response type radio button filter to show only inactive requests/responses
void CEligibilityRequestDetailDlg::OnRadioResponseTypeInactive()
{
	try {

		//required to prevent weird message looping in modeless dialogs
		if (!IsDlgButtonChecked(IDC_RADIO_RESPONSE_TYPE_INACTIVE)) {
			return;
		}

		ApplyRadioButtonResponseTypeFilter(GetEligibilityInactiveCoverageInnerJoin(), "inactive coverage");

	}NxCatchAll(__FUNCTION__);
}

//handles the response type radio button filter to show only failed requests/responses
void CEligibilityRequestDetailDlg::OnRadioResponseTypeFailed()
{
	try {

		//required to prevent weird message looping in modeless dialogs
		if (!IsDlgButtonChecked(IDC_RADIO_RESPONSE_TYPE_FAILED)) {
			return;
		}

		ApplyRadioButtonResponseTypeFilter(GetEligibilityFailedResponseInnerJoin(), "failures");

	}NxCatchAll(__FUNCTION__);
}


//called after filtering on Active, Inactive, or Failed responses,	
//takes in a SQL fragment to filter (via joins) on the proper responses,
//and a string of the reponse type for use in messaging
void CEligibilityRequestDetailDlg::ApplyRadioButtonResponseTypeFilter(CSqlFragment sqlCoverageTypeJoin, CString strCoverageTypeName)
{
	//throw exceptions to the caller

	//load only the active requests, sort by patient name and request ID
	_RecordsetPtr rs = CreateParamRecordset("SELECT EligibilityRequestsT.ID "
		"FROM EligibilityRequestsT "
		"INNER JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
		"INNER JOIN PersonT ON InsuredPartyT.PatientID = PersonT.ID "
		"INNER JOIN EligibilityResponsesT ON EligibilityRequestsT.ID = EligibilityResponsesT.RequestID "
		"{SQL} "
		"WHERE EligibilityRequestsT.ID IN ({INTVECTOR}) "
		"{SQL} "
		"GROUP BY EligibilityRequestsT.ID, PersonT.FullName "
		"ORDER BY PersonT.FullName, EligibilityRequestsT.ID",
		sqlCoverageTypeJoin,
		m_aryAllRequestIDs, GetResponseFilterWhere());

	if (rs->eof) {
		//we have to reset the radio buttons in a separate handler because
		//otherwise the modeless dialog has a heart attack
		PostMessage(NXM_RESET_RADIO_TO_ALL, (WPARAM)strCoverageTypeName.AllocSysString(), (LPARAM)strCoverageTypeName.GetLength());
		return;
	}

	//clear out our current filtered list
	m_aryCurFilteredRequestIDs.clear();

	while (!rs->eof) {
		m_aryCurFilteredRequestIDs.push_back(VarLong(rs->Fields->Item["ID"]->Value));
		rs->MoveNext();
	}
	rs->Close();

	//jump to the first request in the list
	m_nCurRequestIndex = 0;

	//update the left/right button UI & count
	UpdateRequestButtons();

	//load the first request
	FullReload(0);
}

//used to reset the response filter to "all" after a current
//radio button check is complete
LRESULT CEligibilityRequestDetailDlg::OnResetResponseTypeFilterToAll(WPARAM wParam, LPARAM lParam)
{
	try {

		BSTR bstrTextData = (BSTR)wParam;
		CString strCoverageTypeName(bstrTextData);
		SysFreeString(bstrTextData);

		MessageBox(FormatString("No current requests have %s.", strCoverageTypeName), "Practice", MB_ICONINFORMATION | MB_OK);

		CheckDlgButton(IDC_RADIO_RESPONSE_TYPE_ALL, TRUE);
		CheckDlgButton(IDC_RADIO_RESPONSE_TYPE_ACTIVE, FALSE);
		CheckDlgButton(IDC_RADIO_RESPONSE_TYPE_INACTIVE, FALSE);
		CheckDlgButton(IDC_RADIO_RESPONSE_TYPE_FAILED, FALSE);

		OnRadioResponseTypeAll();

	}NxCatchAll(__FUNCTION__);

	return 0;
}

void CEligibilityRequestDetailDlg::OnEditingStartingEligPayGroupList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			//this is a global function
			return OnEditingStartingPayGroupList_Global(this, m_PayGroupsList, pRow, nCol, pvarValue, pbContinue);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRequestDetailDlg::OnEditingFinishingEligPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			//this is a global function
			return OnEditingFinishingPayGroupList_Global(this, m_PayGroupsList, pRow, nCol, varOldValue, strUserEntered, pvarNewValue, pbCommit, pbContinue);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRequestDetailDlg::OnEditingFinishedEligPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			//this is a global function
			return OnEditingFinishedPayGroupList_Global(this, m_PayGroupsList, pRow,
				m_nCurrentRequestPatientID, m_strCurrentRequestPatientName, m_nCurrentRequestInsuredPartyID, 
				nCol, varOldValue, varNewValue, bCommit);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRequestDetailDlg::OnEditingFinishingEligDedoopList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL) {
			return;
		}

		if (!*pbContinue || !*pbCommit) {
			return;
		}

		if (nCol != docValue) {
			//how is this possible, there is only one editable column!
			ASSERT(FALSE);
			return;
		}

		CString strEntered = strUserEntered;

		if (strEntered.IsEmpty()) {
			//put a null value in
			*pvarNewValue = g_cvarNull;
			return;
		}

		//check to make sure its a valid currency
		COleCurrency cyAmt = ParseCurrencyFromInterface(strEntered);
		if (cyAmt.GetStatus() != COleCurrency::valid) {
			*pvarNewValue = g_cvarNull;
			MessageBox("Please enter a valid currency.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			*pbContinue = FALSE;
			*pbCommit = FALSE;
			return;
		}

		if (cyAmt < COleCurrency(0, 0)) {
			*pvarNewValue = g_cvarNull;
			MessageBox("Please enter a currency greater than 0.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			*pbContinue = FALSE;
			*pbCommit = FALSE;
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRequestDetailDlg::OnEditingFinishedEligDedoopList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL) {
			return;
		}

		if (!bCommit) {
			return;
		}

		if (nCol != docValue) {
			//how is this possible, there is only one editable column!
			ASSERT(FALSE);
			return;
		}

		//we can re-use this function because all it does is compare whether
		//two currency variants changed, accounting for nulls
		if (!PayGroupValuesChanged(varOldValue, varNewValue)) {
			//nothing changed
			return;
		}

		CString strOldValue, strNewValue;
		if (varOldValue.vt == VT_CY) {
			strOldValue = FormatCurrencyForInterface(varOldValue);
		}
		if (varNewValue.vt == VT_CY) {
			strNewValue = FormatCurrencyForInterface(varNewValue);
		}

		DedOOPRow eCurRowID = (DedOOPRow)VarLong(pRow->GetValue(docID));

		CString strFieldName;
		AuditEventItems aei;

		switch (eCurRowID) {
			case dorDeductibleTotal:
				strFieldName = "TotalDeductible";
				aei = aeiInsPartyDeductibleTotal;
				break;
			case dorDeductibleRemaining:
				strFieldName = "DeductibleRemaining";
				aei = aeiInsPartyDeductibleRemain;
				break;
			case dorOOPTotal:
				strFieldName = "TotalOOP";
				aei = aeiInsPartyOOPTotal;
				break;
			case dorOOPRemaining:
				strFieldName = "OOPRemaining";
				aei = aeiInsPartyOOPRemain;
				break;
			default:
				ThrowNxException("Invalid deductible row: %li", (long)eCurRowID);
				break;
		}

		ExecuteParamSql("UPDATE InsuredPartyT SET {CONST_STR} = {VT_CY}, LastModifiedDate = GetDate() WHERE PersonID = {INT}", strFieldName, varNewValue, m_nCurrentRequestInsuredPartyID);

		AuditEvent(m_nCurrentRequestPatientID, m_strCurrentRequestPatientName, BeginNewAuditEvent(), aei, m_nCurrentRequestInsuredPartyID, strOldValue, strNewValue, aepMedium, aetChanged);

	}NxCatchAll(__FUNCTION__);
}

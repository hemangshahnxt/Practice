// InsuranceBilling.cpp : implementation file
//
// HCFA and UB-92 are opened in the function
//    "OnClickBtnOpenForm"

#include "stdafx.h"
#include "practice.h"
#include "BillingModuleDlg.h"
#include "BillingDlg.h"
#include "InsuranceBilling.h"
#include "GlobalUtils.h"
#include "GlobalFinancialUtils.h"
#include "PracProps.h"
#include "HCFADlg.h"
#include "UB92.h"
#include "UB04.h"
#include "ADADlg.h"
#include "IDPADlg.h"
#include "NYWCDlg.h"
#include "MICRDlg.h"
#include "MICR2007Dlg.h"
#include "HCFASetup.h"
#include "NxStandard.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
#include "InsuranceReferralsDlg.h"
#include "InternationalUtils.h"
#include "DontShowDlg.h"
#include "GlobalDrawingUtils.h"
#include "NYMedicaidDlg.h"
#include "AlbertaHLINKUtils.h"
#include "Ebilling.h"
#include "BillClaimFieldsDlg.h"
#include "UB04AdditionalFieldsDlg.h" //(j.camacho 2016-3-3) PLID 68501
#include "InsuranceClaimDatesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2014-02-25 13:34) - PLID 61024 - Removed ancient Access error 3048 handling and nonexistent datalist Exception event



#define COLUMN_LINE_ID		0
#define COLUMN_CHARGE_ID	1
#define COLUMN_DOCTOR_ID	2
#define COLUMN_DATE			3
#define COLUMN_DR_NAME		4
#define COLUMN_CPT_CODE		5
#define COLUMN_CPT_SUB_CODE	6
#define COLUMN_CPT_TYPE		7
#define COLUMN_CPT_TYPEOFSERVICE 8
#define COLUMN_MODIFIER		9
#define COLUMN_WHICH_CODES	10
#define COLUMN_DESCRIPTION	11
#define COLUMN_QUANTITY		12
#define COLUMN_UNIT_COST	13
#define COLUMN_LINE_TOTAL	14
#define COLUMN_INSURANCE_RESP 15
#define COLUMN_TAX_RATE		16
#define COLUMN_PROC_CODE	17

// (j.jones 2009-03-10 17:52) - PLID 32864 - added enum for the insured combos
// (j.jones 2010-08-17 11:19) - PLID 40135 - added several more columns
enum InsuredPartyComboColumn {

	ipccID = 0,
	ipccName,
	ipccIsInactive,
	ipccRespTypeID,
	ipccRespTypePriority,
	ipccRespTypeName,
	ipccCategoryTypeID,
	ipccCategoryTypeName,
	ipccInactiveDate,
};

// (j.jones 2008-05-14 12:19) - PLID 30044 - added enums for test result combos
enum TestResultIDList {

	trilCode = 0,
	trilDescription,
};

enum TestResultTypeList {

	trtlCode = 0,
	trtlDescription,
};

// (j.jones 2009-09-14 17:33) - PLID 35284 - added claim type combo
// (j.jones 2016-05-24 15:08) - NX-100704 - added a Code column
enum ClaimTypeComboColumns {

	ctccID = 0,
	ctccCode,
	ctccName,
};

// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
enum PriorAuthTypeComboColumns {

	patccID = 0,
	patccDesc = 1,
};


// (j.jones 2010-06-10 09:51) - PLID 38507 - added HCFA Box 13 combo
enum HCFABox13ComboColumns {

	hb13ccID = 0,
	hb13ccDesc = 1,
};

// (j.jones 2012-01-23 15:50) - PLID 47731 - added condition date combo
// (j.jones 2013-08-16 09:28) - PLID 58063 - moved from .h to .cpp, and added a qualifier column
enum ConditionDateTypeComboColumns {

	cdtccID = 0,
	cdtccDesc,
	cdtccQualifier,
};

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;
using namespace NXDATALISTLib;
using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CInsuranceBilling dialog


CInsuranceBilling::CInsuranceBilling(CWnd* pParent)
	: CNxDialog(CInsuranceBilling::IDD, pParent),
	m_PatInsPartyChecker(NetUtils::PatInsParty),
	m_refphyChecker(NetUtils::RefPhys),
	m_cptChecker(NetUtils::CPTCodeT)
{
	//{{AFX_DATA_INIT(CInsuranceBilling)
	m_boInitialized = FALSE;
	// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
	m_eHasAccess = batNoAccess;
	m_HasHCFABeenOpened = FALSE;
	m_nPendingReferringPhyID = -1;
	m_nPendingUB92Box44ID = -1;
	m_nPatientID = -1;
	m_GuarantorID1 = -1;
	m_GuarantorID2 = -1;
	m_eOldHCFABox13Over = hb13_UseDefault;
	m_nOldInsuranceID1 = -1;
	m_nOldInsuranceID2 = -1;
	m_strHCFABox8 = "";
	m_strHCFABox9b = "";
	m_strHCFABox9c = "";
	m_strHCFABox10d = "";
	m_strHCFABox11bQual = "";
	m_strHCFABox11b = "";
	m_strUB92Box79 = "";
	m_bUseConditionDate = false; // (r.goldschmidt 2016-03-07 12:32) - PLID 68541
	//}}AFX_DATA_INIT
}

CInsuranceBilling::~CInsuranceBilling()
{

}

void CInsuranceBilling::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsuranceBilling)
	DDX_Control(pDX, IDC_TEST_RESULT_LABEL, m_nxstaticTestResultLabel);
	DDX_Control(pDX, IDC_TEST_RESULTS_ID_LABEL, m_nxstaticTestResultsIDLabel);
	DDX_Control(pDX, IDC_TEST_RESULTS_TYPE_LABEL, m_nxstaticTestResultsTypeLabel);
	DDX_Control(pDX, IDC_EDIT_TEST_RESULT, m_editTestResults);
	DDX_Control(pDX, IDC_SEND_TEST_RESULT_CHECK, m_checkSendTestResults);	
	DDX_Control(pDX, IDC_SEND_PAPERWORK_CHECK, m_btnSendPaperwork);
	DDX_Control(pDX, IDC_CHECK_SEND_CORRESPONDENCE, m_checkSendCorrespondence);
	DDX_Control(pDX, IDC_SELECT_REF_PHYS, m_SelectRefPhysButton);
	DDX_Control(pDX, IDC_SELECT_PCP, m_SelectPCPButton);
	DDX_Control(pDX, IDC_OUTSIDE_LAB_CHECK, m_outsideLabCheck);
	DDX_Control(pDX, IDC_RADIO_AUTO_ACCIDENT, m_autoAccidentRadio);
	DDX_Control(pDX, IDC_RADIO_NONE, m_noAccidentRadio);
	DDX_Control(pDX, IDC_RADIO_OTHER_ACCIDENT, m_otherAccidentRadio);
	DDX_Control(pDX, IDC_RADIO_UNBATCHED, m_unbatchedRadio);
	DDX_Control(pDX, IDC_RADIO_PAPER_BATCH, m_paperRadio);
	DDX_Control(pDX, IDC_RADIO_ELECTRONIC_BATCH, m_electronicRadio);
	DDX_Control(pDX, IDC_RADIO_EMPLOYMENT, m_employmentRadio);
	DDX_Control(pDX, IDC_PRINT_FORM, m_printFormButton);
	DDX_Control(pDX, IDC_OPEN_FORM, m_openFormButton);
	DDX_Control(pDX, IDC_EDIT_BOX_19, m_editBox19);
	DDX_Control(pDX, IDC_EDIT_STATE, m_nxeditEditState);
	DDX_Control(pDX, IDC_EDIT_MEDICAID_CODE, m_nxeditEditMedicaidCode);
	DDX_Control(pDX, IDC_EDIT_REFERENCE_NUMBER, m_nxeditEditReferenceNumber);
	DDX_Control(pDX, IDC_EDIT_AUTHORIZATION_NUMBER, m_nxeditEditAuthorizationNumber);
	DDX_Control(pDX, IDC_EDIT_CHARGES, m_nxeditEditCharges);
	DDX_Control(pDX, IDC_PWK_TYPE_LABEL, m_nxstaticPwkTypeLabel);
	DDX_Control(pDX, IDC_PWK_TX_LABEL, m_nxstaticPwkTxLabel);
	DDX_Control(pDX, IDC_LABEL_BOX_19, m_nxstaticLabelBox19);
	DDX_Control(pDX, IDC_LABEL_UB92_BOX44_OVERRIDE, m_nxstaticLabelUb92Box44Override);
	DDX_Control(pDX, IDC_CHECK_MANUAL_REVIEW, m_checkManualReview);
	DDX_Control(pDX, IDC_CLAIM_TYPE_LABEL, m_nxstaticClaimTypeLabel);
	DDX_Control(pDX, IDC_LABEL_HCFA_BOX_13, m_nxstaticHCFABox13Label);
	DDX_Control(pDX, IDC_LABEL_UB_BOX_14, m_nxstaticUBBox14Label);
	DDX_Control(pDX, IDC_EDIT_UB_BOX_14, m_nxeditEditUBBox14);
	DDX_Control(pDX, IDC_LABEL_UB_BOX_15, m_nxstaticUBBox15Label);
	DDX_Control(pDX, IDC_EDIT_UB_BOX_15, m_nxeditEditUBBox15);
	DDX_Control(pDX, IDC_RESUBMISSION_CODE_LABEL, m_nxstaticResubmissionCodeLabel);
	DDX_Control(pDX, IDC_BTN_CLAIM_FIELDS, m_btnClaimFields);
	DDX_Control(pDX, IDC_LABEL_ORIGINAL_REFNO, m_nxstaticOriginalRefNoLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsuranceBilling, CNxDialog)
	//{{AFX_MSG_MAP(CInsuranceBilling)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_OPEN_FORM, OnOpenForm)
	ON_BN_CLICKED(IDC_PRINT_FORM, OnPrintForm)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_EDIT_PRIOR_AUTH_NUM, OnBtnEditPriorAuthNum)
	ON_EN_KILLFOCUS(IDC_EDIT_AUTHORIZATION_NUMBER, OnKillfocusEditAuthorizationNumber)
	ON_BN_CLICKED(IDC_RADIO_PAPER_BATCH, OnRadioPaperBatch)
	ON_BN_CLICKED(IDC_RADIO_ELECTRONIC_BATCH, OnRadioElectronicBatch)
	ON_BN_CLICKED(IDC_SELECT_PCP, OnSelectPcp)
	ON_BN_CLICKED(IDC_SELECT_REF_PHYS, OnSelectRefPhys)
	ON_BN_CLICKED(IDC_SEND_PAPERWORK_CHECK, OnSendPaperworkCheck)
	ON_BN_CLICKED(IDC_SEND_TEST_RESULT_CHECK, OnSendTestResultCheck)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RADIO_UNBATCHED, OnBnClickedRadioUnbatched)
	ON_BN_CLICKED(IDC_BTN_CLAIM_FIELDS, OnBtnClaimFields)
	ON_BN_CLICKED(IDC_BTN_EDIT_CLAIM_DATES, OnBtnEditClaimDates) 
END_MESSAGE_MAP()

void CInsuranceBilling::ReadBillData()
{
	_RecordsetPtr rs, rsInsurance;
	FieldsPtr Fields;
	COleDateTime dt;
	_variant_t var;
	CString str;
	COleCurrency cy;

	try {

		// (j.jones 2010-11-08 16:51) - PLID 39620 - if Alberta is in use, always show
		// the 'send correspondence' checkbox, and also rename it
		if(UseAlbertaHLINK()) {
			GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE)->SetWindowText("Supporting Documentation Will Be Sent");
		}
		else {
			//normal case, ANSI only
			GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE)->SetWindowText("Will Be Sending Correspondence (ANSI E-Billing Only)");
		}

		// (j.jones 2010-06-14 10:59) - PLID 38507 - initialize the old HCFA Box 13 value
		m_eOldHCFABox13Over = hb13_UseDefault;

		// (a.walling 2007-05-21 15:01) - PLID 26088 - Don't query if we already know it won't return anything.
		if (GetBillID() == -1) {
			return;
		}

		// (a.walling 2007-08-27 10:55) - PLID 27026 - Support "Send Paperwork" for ANSI
		// (j.jones 2008-05-14 14:52) - PLID 30044 - added test result fields, and parameterized
		// (j.jones 2009-02-11 10:43) - PLID 33035 - added ManualReview
		// (j.jones 2009-09-15 09:12) - PLID 35284 - added ANSI_ClaimTypeCode
		// (j.jones 2010-03-02 13:56) - PLID 37584 - added PriorAuthType
		// (j.jones 2010-06-10 10:36) - PLID 38507 - added HCFABox13Over
		// (j.jones 2012-01-23 17:20) - PLID 47731 - added ConditionDateType
		// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
		// (j.jones 2013-06-07 16:13) - PLID 41479 - added admission & discharge dates
		// (j.jones 2013-08-14 12:45) - PLID 57902 - added HCFA Boxes 8, 9b, 9c, 11bQual, 11b
		// (a.walling 2016-03-07 08:55) - PLID 68496 - added UB04ClaimInfo
		// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36
		// (r.gonet 2016-04-07) - NX-100072 - Split FirstConditionDate into multiple date fields.
		rs = CreateParamRecordset("SELECT FormType, ConditionDate AS [Date Current Condition], "
			"ConditionDateType, "
			"FirstVisitOrConsultationDate, InitialTreatmentDate, LastSeenDate, AcuteManifestationDate, LastXRayDate, HearingAndPrescriptionDate, AssumedCareDate, RelinquishedCareDate, AccidentDate, "
			"NoWorkFrom AS [Not Working From], "
			"NoWorkTo AS [Not Working To], HospFrom AS [Hospitalized From], "
			"HospTo AS [Hospitalized To], RelatedToEmp AS [Condition Related to Employ], "
			"RelatedToAutoAcc AS [Condition Related to Auto Acc], "
			"RelatedToOther AS [Condition Related to Other Acc], OutsideLab, "
			"OutsideLabCharges, State AS [Auto Acc State], MedicaidResubmission, "
			"OriginalRefNo, PriorAuthNum, HCFABlock19, "
			"HCFABox8, HCFABox9b, HCFABox9c, HCFABox10D, HCFABox11bQual, HCFABox11b, "
			"UB92Box79, UB92Box44, "
			"UBBox14, UBBox15, "
			"RefPhyID, InsuredPartyID, OthrInsuredPartyID, SendCorrespondence, "
			"SendPaperwork, PaperworkType, PaperworkTx, "
			"SendTestResults, TestResultID, TestResultType, TestResult, "
			"ManualReview, ANSI_ClaimTypeCode, PriorAuthType, HCFABox13Over, "
			"AdmissionTime, DischargeTime, UB04ClaimInfo "
			"FROM BillsT WHERE ID = {INT}", GetBillID());

		Fields = rs->Fields;
		if (rs->eof) {
			return;
		}

		var = Fields->GetItem("FormType")->Value;
		m_FormTypeCombo->SetSelByColumn(0,var);

		PostFormTypeChanged();

		/* Date values */
		var = Fields->GetItem("Date Current Condition")->Value;
		if (var.vt != VT_NULL) {
			m_pAccident->SetDateTime(VarDateTime(var));
		}
		else {
			m_pAccident->Clear();
		}
		
		// (r.gonet 2016-04-07) - NX-100072 - Load the ClaimDates structure.
		m_claimDates.eConditionDateType = (ConditionDateType)AdoFldLong(Fields, "ConditionDateType", ConditionDateType::cdtFirstVisitOrConsultation444);
		m_claimDates.dtFirstVisitOrConsultationDate = AdoFldDateTime(Fields, "FirstVisitOrConsultationDate", g_cdtNull);
		m_claimDates.dtInitialTreatmentDate = AdoFldDateTime(Fields, "InitialTreatmentDate", g_cdtNull);
		m_claimDates.dtLastSeenDate = AdoFldDateTime(Fields, "LastSeenDate", g_cdtNull);
		m_claimDates.dtAcuteManifestationDate = AdoFldDateTime(Fields, "AcuteManifestationDate", g_cdtNull);
		m_claimDates.dtLastXRayDate = AdoFldDateTime(Fields, "LastXRayDate", g_cdtNull);
		m_claimDates.dtHearingAndPrescriptionDate = AdoFldDateTime(Fields, "HearingAndPrescriptionDate", g_cdtNull);
		m_claimDates.dtAssumedCareDate = AdoFldDateTime(Fields, "AssumedCareDate", g_cdtNull);
		m_claimDates.dtRelinquishedCareDate = AdoFldDateTime(Fields, "RelinquishedCareDate", g_cdtNull);
		m_claimDates.dtAccidentDate = AdoFldDateTime(Fields, "AccidentDate", g_cdtNull);

		if (m_claimDates.GetFirstConditionDate() != g_cdtNull) {
			m_pIllness->SetDateTime(m_claimDates.GetFirstConditionDate());
		}
		else {
			m_pIllness->Clear();
		}

		var = Fields->GetItem("Not Working From")->Value;
		if (var.vt != VT_NULL) {
			m_pWorkFrom->SetDateTime(VarDateTime(var));
		}
		else {
			m_pWorkFrom->Clear();
		}
		var = Fields->GetItem("Not Working To")->Value;
		if (var.vt != VT_NULL) {
			m_pWorkTo->SetDateTime(VarDateTime(var));
		}
		else {
			m_pWorkTo->Clear();
		}
		var = Fields->GetItem("Hospitalized From")->Value;
		if (var.vt != VT_NULL) {
			m_pHospFrom->SetDateTime(VarDateTime(var));
		}
		else {
			m_pHospFrom->Clear();
		}
		var = Fields->GetItem("Hospitalized To")->Value;
		if (var.vt != VT_NULL) {
			m_pHospTo->SetDateTime(VarDateTime(var));
		}
		else {
			m_pHospTo->Clear();
		}

		// (j.jones 2013-06-07 16:13) - PLID 41479 - added admission & discharge dates
		var = Fields->GetItem("AdmissionTime")->Value;
		if (var.vt != VT_NULL) {
			m_pAdmissionTime->SetDateTime(VarDateTime(var));
		}
		else {
			m_pAdmissionTime->Clear();
		}
		var = Fields->GetItem("DischargeTime")->Value;
		if (var.vt != VT_NULL) {
			m_pDischargeTime->SetDateTime(VarDateTime(var));
		}
		else {
			m_pDischargeTime->Clear();
		}

		((CButton*)GetDlgItem(IDC_RADIO_EMPLOYMENT))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_AUTO_ACCIDENT))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_OTHER_ACCIDENT))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_NONE))->SetCheck(1);

		/* Condition-related-to radio */
		var = Fields->GetItem("Condition Related to Employ")->Value;
		if (VarBool(var, FALSE) != FALSE) {
			((CButton*)GetDlgItem(IDC_RADIO_EMPLOYMENT))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_AUTO_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_OTHER_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_NONE))->SetCheck(0);
		}
		var = Fields->GetItem("Condition Related to Auto Acc")->Value;
		if (VarBool(var, FALSE) != FALSE) {
			((CButton*)GetDlgItem(IDC_RADIO_EMPLOYMENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_AUTO_ACCIDENT))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_OTHER_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_NONE))->SetCheck(0);
		}
		var = Fields->GetItem("Condition Related to Other Acc")->Value;
		if (VarBool(var, FALSE) != FALSE) {
			((CButton*)GetDlgItem(IDC_RADIO_EMPLOYMENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_AUTO_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_OTHER_ACCIDENT))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_NONE))->SetCheck(0);
		}

		////////////////////////////////////////////////////
		// Outside-of-lab checkbox
		var = Fields->GetItem("OutsideLab")->Value;
		if (VarBool(var, FALSE) != FALSE) {
			((CButton*)GetDlgItem(IDC_OUTSIDE_LAB_CHECK))->SetCheck(TRUE);
		}
		////////////////////////////////////////////////////
		// Outside of lab charges
		var = Fields->GetItem("OutsideLabCharges")->Value;
		cy = VarCurrency(var);
		str = FormatCurrencyForInterface(cy, FALSE);
		GetDlgItem(IDC_EDIT_CHARGES)->SetWindowText(str);

		////////////////////////////////////////////////////

		////////////////////////////////////////////////////
		// Send Correspondence checkbox
		var = Fields->GetItem("SendCorrespondence")->Value;
		if (VarBool(var, FALSE) != FALSE) {
			((CButton*)GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE))->SetCheck(TRUE);
		}
		////////////////////////////////////////////////////

		// (j.jones 2009-02-11 10:03) - PLID 33035 - added ManualReview
		var = Fields->GetItem("ManualReview")->Value;
		if(VarBool(var, FALSE) != FALSE) {
			m_checkManualReview.SetCheck(TRUE);
		}

		// (j.jones 2009-09-15 09:13) - PLID 35284 - added ANSI_ClaimTypeCode
		var = Fields->GetItem("ANSI_ClaimTypeCode")->Value;
		NXDATALIST2Lib::IRowSettingsPtr pClaimTypeRow = m_ClaimTypeCombo->SetSelByColumn(ctccID, var);
		if(pClaimTypeRow == NULL) {
			m_ClaimTypeCombo->SetSelByColumn(ctccID, (long)ctcOriginal);
		}

		// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
		var = Fields->GetItem("PriorAuthType")->Value;
		NXDATALIST2Lib::IRowSettingsPtr pPriorAuthRow = m_PriorAuthTypeCombo->SetSelByColumn(patccID, var);
		if(pPriorAuthRow == NULL) {
			m_PriorAuthTypeCombo->SetSelByColumn(patccID, (long)patPriorAuthNumber);
		}

		// (j.jones 2012-01-23 15:49) - PLID 47731 - added condition date combo
		var = Fields->GetItem("ConditionDateType")->Value;
		// (r.gonet 2016-04-07) - NX-100072 - Set the condition date type.
		SetConditionDateType((ConditionDateType)VarLong(var, (long)ConditionDateType::cdtFirstVisitOrConsultation444));

		// (j.jones 2010-06-10 10:34) - PLID 38507 - added HCFA Box 13 config
		m_eOldHCFABox13Over = (HCFABox13Over)VarLong(Fields->GetItem("HCFABox13Over")->Value, (long)hb13_UseDefault);
		NXDATALIST2Lib::IRowSettingsPtr pBox13Row = m_HCFABox13Combo->SetSelByColumn(hb13ccID, (long)m_eOldHCFABox13Over);
		if(pBox13Row == NULL) {
			m_HCFABox13Combo->SetSelByColumn(hb13ccID, (long)hb13_UseDefault);
			m_eOldHCFABox13Over = hb13_UseDefault;
		}

		//must call this again to reflect the Box13 value
		PostFormTypeChanged();

		// Edit boxes
		var = Fields->GetItem("Auto Acc State")->Value;
		GetDlgItem(IDC_EDIT_STATE)->SetWindowText(VarString(var, ""));

		var = Fields->GetItem("MedicaidResubmission")->Value;
		GetDlgItem(IDC_EDIT_MEDICAID_CODE)->SetWindowText(VarString(var, ""));
		
		var = Fields->GetItem("OriginalRefNo")->Value;
		GetDlgItem(IDC_EDIT_REFERENCE_NUMBER)->SetWindowText(VarString(var, ""));
		
		var = Fields->GetItem("PriorAuthNum")->Value;
		GetDlgItem(IDC_EDIT_AUTHORIZATION_NUMBER)->SetWindowText(VarString(var, ""));
		
		// (j.jones 2013-08-13 13:49) - PLID 57902 - these are all now stored as member variables,		
		// the fields to edit them are now in the BillClaimFieldsDlg
		var = Fields->GetItem("HCFABox8")->Value;
		m_strHCFABox8 = VarString(var, "");
		var = Fields->GetItem("HCFABox9b")->Value;
		m_strHCFABox9b = VarString(var, "");
		var = Fields->GetItem("HCFABox9c")->Value;
		m_strHCFABox9c = VarString(var, "");
		var = Fields->GetItem("HCFABox10D")->Value;
		m_strHCFABox10d = VarString(var, "");
		var = Fields->GetItem("HCFABox11bQual")->Value;
		m_strHCFABox11bQual = VarString(var, "");
		var = Fields->GetItem("HCFABox11b")->Value;
		m_strHCFABox11b = VarString(var, "");
		var = Fields->GetItem("UB92Box79")->Value;
		m_strUB92Box79 = VarString(var, "");

		var = Fields->GetItem("HCFABlock19")->Value;
		GetDlgItem(IDC_EDIT_BOX_19)->SetWindowText(VarString(var, ""));

		// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
		m_nxeditEditUBBox14.SetWindowText(VarString(Fields->GetItem("UBBox14")->Value, ""));
		m_nxeditEditUBBox15.SetWindowText(VarString(Fields->GetItem("UBBox15")->Value, ""));

		if(Fields->GetItem("RefPhyID")->Value.vt != VT_NULL) {
			//set the pending ID, because if the TrySetSel fails we do want a ComboBoxText set
			m_nPendingReferringPhyID = AdoFldLong(Fields, "RefPhyID");
			if(m_pRefList->TrySetSelByColumn(rpcID, m_nPendingReferringPhyID) == -1) {
				//they may have an inactive referring physician
				// (b.cardillo 2006-07-31 12:06) - PLID 21705 - In fact, we assume this to be the only 
				// explanation for a failure to set the selection, so we need to clear the selection 
				// before we set the combo box text.
				m_pRefList->PutCurSel(sriNoRow);
				// Now go ahead and set the combo box text to the name of the desired selection.
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", m_nPendingReferringPhyID);
				if(!rsProv->eof) {
					m_pRefList->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
				}
			}
		}

		// (j.jones 2006-11-06 16:33) - PLID 22534 - added UB92 Box 44 CPT List
		if(Fields->GetItem("UB92Box44")->Value.vt != VT_NULL) {
			//set the pending ID, because if the TrySetSel fails we do want a ComboBoxText set
			m_nPendingUB92Box44ID = AdoFldLong(Fields, "UB92Box44");
			if(m_pUB92Box44List->TrySetSelByColumn(0, m_nPendingUB92Box44ID) == -1) {
				//they may have an inactive code
				m_pUB92Box44List->PutCurSel(sriNoRow);
				// Now go ahead and set the combo box text to the name of the desired selection.
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rsCPT = CreateParamRecordset("SELECT Code FROM CPTCodeT WHERE ID = {INT}", m_nPendingUB92Box44ID);
				if(!rsCPT->eof) {
					m_pUB92Box44List->PutComboBoxText(_bstr_t(AdoFldString(rsCPT, "Code", "")));
				}
				rsCPT->Close();
			}
		}

		////////////////////////////////////////////////////
		// Insurance parties
		// (j.jones 2010-08-17 12:07) - PLID 40135 - cache this info
		m_nOldInsuranceID1 = AdoFldLong(Fields, "InsuredPartyID", -1);
		m_nOldInsuranceID2 = AdoFldLong(Fields, "OthrInsuredPartyID", -1);
		m_pInsurance1->SetSelByColumn(ipccID, (long)m_nOldInsuranceID1);
		m_pInsurance2->SetSelByColumn(ipccID, (long)m_nOldInsuranceID2);

		// (j.jones 2013-08-07 12:04) - PLID 58042 - if a HCFA, and a company is selected,
		//toggle the fields that need to change for the new/old HCFA form
		UpdateDisplayedHCFAFields(m_nOldInsuranceID1, GetBillID());

		////////////////////////////////////////////////////
		// (a.walling 2007-08-27 10:58) - PLID 27026 - Send Paperwork and etc
		BOOL bSendPaperwork = AdoFldBool(Fields, "SendPaperwork", FALSE);
		CheckDlgButton(IDC_SEND_PAPERWORK_CHECK, bSendPaperwork);
		if (bSendPaperwork) {
			m_pPWKType->SetSelByColumn(0, _variant_t(AdoFldString(Fields, "PaperworkType", "")));
			m_pPWKTx->SetSelByColumn(0, _variant_t(AdoFldString(Fields, "PaperworkTx", "")));
		} else {
			m_pPWKType->SetSelByColumn(0, _variant_t(AdoFldString(Fields, "PaperworkType", "")));
			m_pPWKTx->SetSelByColumn(0, _variant_t(AdoFldString(Fields, "PaperworkTx", "")));
		}
		if (m_eHasAccess != batNoAccess) {
			OnSendPaperworkCheck(); // enable appropriate dialog items
		}

		// (j.jones 2008-05-14 14:51) - PLID 30044 - added test result info
		BOOL bSendTestResults = VarBool(Fields->Item["SendTestResults"]->Value, FALSE);
		m_checkSendTestResults.SetCheck(bSendTestResults);
		// (j.jones 2011-06-17 16:22) - PLID 42179 - changed the default to a blank string
		m_TestResultIDCombo->SetSelByColumn(trilCode, _variant_t(VarString(Fields->Item["TestResultID"]->Value, "")));
		
		m_strTestResultType = VarString(Fields->Item["TestResultType"]->Value, "");
		NXDATALIST2Lib::IRowSettingsPtr pTRTypeRow = m_TestResultTypeCombo->SetSelByColumn(trtlCode, (LPCTSTR)m_strTestResultType);
		if(pTRTypeRow == NULL && !m_strTestResultType.IsEmpty()) {
			// (j.jones 2010-10-22 09:48) - PLID 40962 - Concentration, Gas Test Rate, and Oxygen
			// are obsolete now, but the combo box text is only the qualifier, so just display
			// the qualifier
			m_TestResultTypeCombo->PutComboBoxText((LPCTSTR)m_strTestResultType);
		}
		
		// (a.walling 2016-03-07 08:55) - PLID 68496 - UB04 Enhancements - Load from database
		m_ub04ClaimInfoOld = UB04::ClaimInfo::FromXml(AdoFldString(rs, "UB04ClaimInfo", ""));
		m_ub04ClaimInfo = m_ub04ClaimInfoOld;
		
		SetDlgItemText(IDC_EDIT_TEST_RESULT, VarString(Fields->Item["TestResult"]->Value, ""));
		if(m_eHasAccess != batNoAccess) {
			PostSendTestResultsCheck();
		}

		rs->Close();

		////////////////////////////////////////////////////
		// Load HCFA batch status
		{
			int nBatch = FindHCFABatch(GetBillID());

			if (nBatch == 1 || nBatch == 3) {
				((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(TRUE);
				((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(FALSE);
			}
			else if (nBatch == 2) {
				((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(TRUE);
				((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(FALSE);
			}
			else {
				((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);
			}
		}
	}NxCatchAll("Error 100: CInsuranceBilling::ReadBillData");
}

/////////////////////////////////////////////////////////////////////////////
// CInsuranceBilling message handlers

// (c.haag 2009-09-03 14:11) - PLID 34781 - Ensures that the tab is fully loaded; including
// any defaults there may be for new bills
void CInsuranceBilling::EnsureInitialized()
{
	if (m_boInitialized) {
		return;
	}
	m_nPatientID = ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID;

	CString str;
	str.Format("InsuredPartyT.PatientID = %li", m_nPatientID);
	m_pInsurance1->WhereClause = (LPCTSTR)str;
	m_pInsurance2->WhereClause = (LPCTSTR)str;
	
	m_pInsurance1->Requery();
	m_pInsurance2->Requery();

	IRowSettingsPtr pRow;
	_variant_t var;
	var.vt = VT_NULL;
	pRow = m_pInsurance1->GetRow(-1);
	pRow->PutValue(ipccID, var);
	pRow->PutValue(ipccName, "<No Company Selected>");
	// (j.jones 2010-08-17 11:19) - PLID 40135 - added several more columns
	pRow->PutValue(ipccIsInactive, (long)0);	
	pRow->PutValue(ipccRespTypeID, var);
	pRow->PutValue(ipccRespTypePriority, (long)-1);
	pRow->PutValue(ipccRespTypeName, _bstr_t(""));
	pRow->PutValue(ipccCategoryTypeID, (long)rctInvalidRespCategory);
	pRow->PutValue(ipccCategoryTypeName, _bstr_t(""));
	pRow->PutValue(ipccInactiveDate, var);
	m_pInsurance1->InsertRow(pRow,0);

	pRow = m_pInsurance2->GetRow(-1);
	pRow->PutValue(ipccID, var);
	pRow->PutValue(ipccName, "<No Company Selected>");
	// (j.jones 2010-08-17 11:19) - PLID 40135 - added several more columns
	pRow->PutValue(ipccIsInactive, (long)0);	
	pRow->PutValue(ipccRespTypeID, var);
	pRow->PutValue(ipccRespTypePriority, (long)-1);
	pRow->PutValue(ipccRespTypeName, _bstr_t(""));
	pRow->PutValue(ipccCategoryTypeID, (long)rctInvalidRespCategory);
	pRow->PutValue(ipccCategoryTypeName, _bstr_t(""));
	pRow->PutValue(ipccInactiveDate, var);
	m_pInsurance2->InsertRow(pRow,0);

	if (m_refphyChecker.Changed()) {

		// (s.dhole 2014-06-25 11:19) - PLID 62282 - Save current selection
		long nRefSel = m_pRefList->CurSel;
		_variant_t vtRefPhyID = g_cvarNull;
		if (nRefSel != -1) {
			// (z.manning 2008-12-08 13:24) - PLID 32320 - Now have an enum for m_pRefList
			vtRefPhyID = m_pRefList->GetValue(nRefSel, CInsuranceBilling::rpcID);
		}
		m_pRefList->Requery();
		
		pRow = m_pRefList->GetRow(-1);
		// (j.jones 2011-12-01 14:13) - PLID 46771 - this now uses the enums
		pRow->PutValue(rpcID, g_cvarNull);
		pRow->PutValue(rpcName, "<No Physician Selected>");
		pRow->PutValue(rpcNpi, "");
		pRow->PutValue(rpcRefPhysID, "");
		pRow->PutValue(rpcUpin, "");
		pRow->PutValue(rpcAddress1, "");
		pRow->PutValue(rpcAddress2, "");
		pRow->PutValue(rpcCity, "");
		pRow->PutValue(rpcState, "");
		pRow->PutValue(rpcZip, "");
		m_pRefList->InsertRow(pRow,0);
		// (s.dhole 2014-06-25 11:19) - PLID 62282 - no try assign back  old selection
		if (m_pRefList->TrySetSelByColumn(CInsuranceBilling::rpcID, vtRefPhyID) == -1){
			m_pRefList->CurSel = sriNoRow;
		}		
	}		
	// (z.manning 2008-12-08 12:39) - PLID 32320 - Show/hide columns in the ref phys list based on
	// the preferences to show them.
	if(GetRemotePropertyInt("RefPhysComboShowNPI", 1, 0, "<None>") == 0) {
		m_pRefList->GetColumn(rpcNpi)->PutStoredWidth(0);
	}
	else {
		m_pRefList->GetColumn(rpcNpi)->PutStoredWidth(80);
	}
	if(GetRemotePropertyInt("RefPhysComboShowID", 0, 0, "<None>") == 0) {
		m_pRefList->GetColumn(rpcRefPhysID)->PutStoredWidth(0);
	}
	else {
		m_pRefList->GetColumn(rpcRefPhysID)->PutStoredWidth(70);
	}
	if(GetRemotePropertyInt("RefPhysComboShowUPIN", 0, 0, "<None>") == 0) {
		m_pRefList->GetColumn(rpcUpin)->PutStoredWidth(0);
	}
	else {
		m_pRefList->GetColumn(rpcUpin)->PutStoredWidth(70);
	}

	if (m_cptChecker.Changed()) {

		// (j.jones 2006-11-06 16:33) - PLID 22534 - added UB92 Box 44 CPT List
		m_pUB92Box44List->Requery();

		IRowSettingsPtr pRow;
		pRow = m_pUB92Box44List->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<None>");
		pRow->PutValue(2,"");
		pRow->PutValue(3,"<No Code Selected>");
		m_pUB92Box44List->InsertRow(pRow,0);
	}

	/*TODO: uncomment when this class handles switching patients more efficiently
	// CH 3/20: Network code check
	if (m_PatInsPartyChecker.Changed())
	{
		m_pInsurance1->Requery();
		m_pInsurance2->Requery();
	}
	*/

	//(e.lally 2009-09-10) PLID 23163 - Already loaded in the BillingDlg
	//m_pRefList->CurSel = -1;
	m_pInsurance1->CurSel = -1;
	m_pInsurance2->CurSel = -1;
	m_nOldInsuranceID1 = -1;
	m_nOldInsuranceID2 = -1;
	m_pUB92Box44List->CurSel = -1;
	
	m_pAccident->Clear();
	// (r.gonet 2016-04-07) - NX-100072
	ClearAdditionalClaimDates();
	m_pWorkFrom->Clear();
	m_pWorkTo->Clear();
	m_pHospFrom->Clear();
	m_pHospTo->Clear();
	m_pAdmissionTime->Clear();
	m_pDischargeTime->Clear();

	((CButton*)GetDlgItem(IDC_OUTSIDE_LAB_CHECK))->SetCheck(FALSE);
	((CButton*)GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE))->SetCheck(FALSE);	
	// (j.jones 2009-02-11 10:04) - PLID 33035 - added manual review
	m_checkManualReview.SetCheck(FALSE);

	// (a.walling 2007-09-19 18:03) - PLID 27026 - Need to reset these for new bills!
	((CButton*)GetDlgItem(IDC_SEND_PAPERWORK_CHECK))->SetCheck(FALSE);	
	
	m_pPWKType->PutCurSel(NULL);
	m_pPWKTx->PutCurSel(NULL);
	m_pPWKType->Enabled = VARIANT_FALSE;
	m_pPWKTx->Enabled = VARIANT_FALSE;

	// (j.jones 2008-05-14 14:29) - PLID 30044 - reset the test results
	m_checkSendTestResults.SetCheck(FALSE);
	//default the ID to TR
	// (j.jones 2011-06-17 16:15) - PLID 42179 - default this to nothing
	m_TestResultIDCombo->PutCurSel(NULL);
	m_TestResultTypeCombo->PutCurSel(NULL);
	m_strTestResultType = "";
	SetDlgItemText(IDC_EDIT_TEST_RESULT, "");		
	PostSendTestResultsCheck();

	// (j.jones 2009-09-14 17:32) - PLID 35284 - added claim type combo
	m_ClaimTypeCombo->SetSelByColumn(ctccID, (long)ctcOriginal);

	// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
	m_PriorAuthTypeCombo->SetSelByColumn(patccID, (long)patPriorAuthNumber);

	// (j.jones 2012-01-23 15:49) - PLID 47731 - added condition date combo
	// (r.gonet 2016-04-07) - NX-100072 - Set the condition date type.
	SetConditionDateType(ConditionDateType::cdtFirstVisitOrConsultation444);

	// (j.jones 2010-06-10 10:38) - PLID 38507 - added HCFA Box 13 config
	m_HCFABox13Combo->SetSelByColumn(hb13ccID, (long)hb13_UseDefault);
	
	SetAccidentStatus();

	GetDlgItem(IDC_EDIT_STATE)->SetWindowText("");
	GetDlgItem(IDC_EDIT_MEDICAID_CODE)->SetWindowText("");
	GetDlgItem(IDC_EDIT_REFERENCE_NUMBER)->SetWindowText("");
	GetDlgItem(IDC_EDIT_AUTHORIZATION_NUMBER)->SetWindowText("");
	GetDlgItem(IDC_EDIT_BOX_19)->SetWindowText("");
	m_strHCFABox8 = "";
	m_strHCFABox9b = "";
	m_strHCFABox9c = "";
	m_strHCFABox10d = "";
	m_strHCFABox11bQual = "";
	m_strHCFABox11b = "";
	m_strUB92Box79 = "";
	m_ub04DefaultOccurrenceCodeToApply = "";
	m_ub04DefaultRemarksToApply = "";
	m_ub04ClaimInfo = {};
	m_ub04ClaimInfoOld = {};

	// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
	m_nxeditEditUBBox14.SetWindowText("");
	m_nxeditEditUBBox15.SetWindowText("");

	//Get some variable to not use the patient set //BVB
	CString			tmpSQL;
	_RecordsetPtr	rsInfo;
	int iPatientID = m_nPatientID;
	//(e.lally 2009-09-10) PLID 23163 - Already loaded in the BillingDlg
	//long nDefaultReferringPhyID;
	COleDateTime	dt;
	_variant_t varDefaultInjuryDate;
	
	rsInfo = CreateParamRecordset("SELECT CASE WHEN GuarantorID1 Is Null THEN -1 ELSE GuarantorID1 End AS GuarantorID1, CASE WHEN GuarantorID2 Is Null THEN -1 ELSE GuarantorID2 End AS GuarantorID2, CASE WHEN DefaultReferringPhyID Is Null THEN -1 ELSE DefaultReferringPhyID END AS DefaultReferringPhyID, DefaultInjuryDate FROM "
		"(PatientsT LEFT JOIN (SELECT PersonID AS GuarantorID2, InsuredPartyT.PatientID FROM InsuredPartyT WHERE RespTypeID=2) AS SecondaryQ ON PatientsT.PersonID = SecondaryQ.PatientID) LEFT JOIN (SELECT PersonID AS GuarantorID1, PatientID FROM InsuredPartyT WHERE RespTypeID=1) AS PrimaryQ ON PatientsT.PersonID = PrimaryQ.PatientID "
		"WHERE PatientsT.PersonID = {INT}", iPatientID);

	if(!rsInfo->eof) {
		//we may have pre-filled the guarantor values, so don't overwrite them
		if(m_GuarantorID1 == -1)
			m_GuarantorID1 = AdoFldLong(rsInfo, "GuarantorID1",-1);
		if(m_GuarantorID2 == -1)
			m_GuarantorID2 = AdoFldLong(rsInfo, "GuarantorID2",-1);
		//(e.lally 2009-09-10) PLID 23163 - Already loaded in the BillingDlg
		//nDefaultReferringPhyID = AdoFldLong(rsInfo, "DefaultReferringPhyID",-1);
		varDefaultInjuryDate = rsInfo->Fields->GetItem("DefaultInjuryDate")->Value;
	}
	else {
		//we may have pre-filled the guarantor values, so don't overwrite them
		if(m_GuarantorID1 == -1)
			m_GuarantorID1 = -1;
		if(m_GuarantorID2 == -1)
			m_GuarantorID2 = -1;
		//(e.lally 2009-09-10) PLID 23163 - Already loaded in the BillingDlg
		//nDefaultReferringPhyID = -1;
	}
	rsInfo->Close();
	
	//if a new bill
	if (GetBillID() == -1)
	{
		try {
			// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
			m_eHasAccess = batFullAccess;

			// (j.jones 2010-06-14 10:59) - PLID 38507 - reset the old HCFA Box 13 value
			m_eOldHCFABox13Over = hb13_UseDefault;

			// Set default insurance
			// (j.jones 2010-01-07 14:24) - PLID 36752 - set the selection, we need it to be set immediately
			//(also these lists are going to be very short, so this isn't a big time sink)
			m_pInsurance1->SetSelByColumn(ipccID, m_GuarantorID1);
			m_pInsurance2->SetSelByColumn(ipccID, m_GuarantorID2);

			// (j.jones 2013-08-07 12:04) - PLID 58042 - if a HCFA, and a company is selected,
			//toggle the fields that need to change for the new/old HCFA form
			UpdateDisplayedHCFAFields(m_GuarantorID1, GetBillID());

			// (s.tullis 2016-02-24 16:48) - PLID 68319
			long ClaimForm = GetFormTypeFromLocationInsuranceSetup(m_GuarantorID1, ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_nCurLocationID);
			m_FormTypeCombo->SetSelByColumn(0, (long)ClaimForm);

			PostFormTypeChanged();

			// (z.manning 2010-08-16 10:39) - PLID 40120 - Use the selected insured party ID instead of always using the default
			long nCurrentInsPartyID = ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetCurrentBillToInsuredPartyID();
			if (nCurrentInsPartyID == -1) {
				// (z.manning 2010-08-16 10:39) - PLID 40120 - If an insurance resp is not selected then now we can
				// default to primary.
				nCurrentInsPartyID = m_GuarantorID1;
			}

			//if the preference says to always batch, or we have insurance charges, then check off the default batch
			// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
			if ((GetRemotePropertyInt("AlwaysUseDefaultClaimBatch", 1, 0, "<None>", TRUE) ||
				((CBillingModuleDlg*)m_pBillingModuleWnd)->CheckHasInsuranceCharges())
				&& nCurrentInsPartyID > 0)
			{
				int batch = 0;
				if (ClaimForm == 1) {
					batch = FindDefaultHCFABatch(nCurrentInsPartyID);
				}
				else if (ClaimForm == 2) {
					batch = FindDefaultUB92Batch(nCurrentInsPartyID);
				}
				((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(batch == 1 || batch == 3);
				((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(batch == 2);
				((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(batch == 0);
			}
			else {
				((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);
			}

			// (j.jones 2006-05-30 12:44) - get the UB92 Box 32 default
			// (z.manning 2010-08-16 11:37) - PLID 40120 - No point in doing this if we don't have an insured party
			// (r.goldschmidt 2016-03-07 12:32) - PLID 68541 - UB04 Enhancements - Get date defaults
			// (r.goldschmidt 2016-03-11 16:16) - PLID 68585 - UB04 Enhancements - Get box 80 defaults
			// (j.jones 2016-05-09 8:53) - NX-100515 - added UB04ClaimInfo
			if (nCurrentInsPartyID != -1) {
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT Box32, Box19, Box20, UB04UseBox31Date, ShowInsAdd, UB04Box80UseThree, UB04ClaimInfo "
					"FROM UB92SetupT "
					"WHERE ID IN (SELECT UB92SetupGroupID FROM InsuranceCoT WHERE PersonID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT})); "
					""
					"SELECT InsuranceCoT.Name, PersonT.Address1, PersonT.Address2, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip "
					"FROM InsuranceCoT LEFT JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID "
					"WHERE InsuranceCoT.PersonID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
					, nCurrentInsPartyID, nCurrentInsPartyID);
				bool bSetUB04Box80 = false;
				bool bUB04Box80ThreeLines = false;
				if (!rs->eof) {
					m_bUseConditionDate = (AdoFldLong(rs, "UB04UseBox31Date", 0) == 1);
					bSetUB04Box80 = (AdoFldLong(rs, "ShowInsAdd", 0) == 1);
					bUB04Box80ThreeLines = (AdoFldLong(rs, "UB04Box80UseThree", 0) == 1);
					// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36

					CString str;

					// (a.walling 2016-03-08 13:09) - PLID 68497 - UB04 Enhancements - Apply defaults
					// (a.walling 2016-03-09 10:48) - PLID 68555 - defer calculated default date that might depend on bill information
					CString ub04box31actually = AdoFldString(rs, "Box32", "").TrimRight();
					m_ub04DefaultOccurrenceCodeToApply = ub04box31actually;

					// (j.jones 2016-05-09 8:53) - NX-100515 - added UB04ClaimInfo
					m_ub04ClaimInfo = UB04::ClaimInfo::FromXml(AdoFldString(rs, "UB04ClaimInfo", ""));

					// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15, which are in the setup as 19/20
					str = AdoFldString(rs, "Box19", "");
					str.TrimRight();
					m_nxeditEditUBBox14.SetWindowText(str);
					str = AdoFldString(rs, "Box20", "");
					str.TrimRight();
					m_nxeditEditUBBox15.SetWindowText(str);
				}
				// (r.goldschmidt 2016-03-11 17:00) - PLID 68585 - UB04 Enhancements - format box 80 defaults
				if (bSetUB04Box80) {
					rs = rs->NextRecordset(NULL);
					if (!rs->eof) {
						CString strInsCoName = AdoFldString(rs, "Name", "").Trim();
						CString strInsCoAddr1 = AdoFldString(rs, "Address1", "").Trim();
						CString strInsCoAddr2 = AdoFldString(rs, "Address2", "").Trim();
						CString strInsCoCityStateZip = AdoFldString(rs, "CityStateZip", "").Trim();
						m_ub04DefaultRemarksToApply.Format("%s\r\n%s%s%s\r\n%s", strInsCoName, strInsCoAddr1, bUB04Box80ThreeLines ? " " : "\r\n", strInsCoAddr2, strInsCoCityStateZip);
					}
				}
				rs->Close();
			}

			if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_bPromptForReferral &&
				 ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nInsuranceReferralID == -1)
				((CBillingModuleDlg*)m_pBillingModuleWnd)->ApplyInsuranceReferral();

		}NxCatchAll("Error loading default insurance information.");
	}
	else {
		// (a.walling 2016-03-09 10:48) - PLID 68555 - m_bUseConditionDate needs to be set for existing bills as well
		try {
			// (z.manning 2010-08-16 10:39) - PLID 40120 - Use the selected insured party ID instead of always using the default
			long nCurrentInsPartyID = ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetCurrentBillToInsuredPartyID();
			if (nCurrentInsPartyID == -1) {
				// (z.manning 2010-08-16 10:39) - PLID 40120 - If an insurance resp is not selected then now we can
				// default to primary.
				nCurrentInsPartyID = m_GuarantorID1;
			}

			if (nCurrentInsPartyID != -1) {
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT UB04UseBox31Date "
					"FROM UB92SetupT "
					"WHERE ID IN (SELECT UB92SetupGroupID FROM InsuranceCoT WHERE PersonID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}))", nCurrentInsPartyID);
				if (!rs->eof) {
					m_bUseConditionDate = (AdoFldLong(rs, "UB04UseBox31Date", 0) == 1);
				}
			}
		}NxCatchAll("Error loading default UB insurance information.");
	}

	m_pUB92Box44List->CurSel = -1;

	GetDlgItem(IDC_EDIT_CHARGES)->SetWindowText(FormatCurrencyForInterface(COleCurrency(0,0), FALSE));
	
	ChangeAccess();

	/* Everything else */
	ReadBillData();

	//Set the default accident date
	//This was only for a new bill, but we changed it to update existing bills if
	//and only if no date exists already

	//DRT 4/29/03 - Added a preference so you can choose what field is filled in 
	//		by this info.  0 = Current date (old option), 1 = First date, 2 = both
	long nAccType = GetRemotePropertyInt("DefaultAccidentType", 0, 0, "<None>", true);		
	if((nAccType == 0 || nAccType == 2) && m_pAccident->GetStatus() == 3 /*empty*/) {
		if (varDefaultInjuryDate.vt == VT_DATE) {
			dt = varDefaultInjuryDate.date;
			m_pAccident->SetDateTime(dt);
		}
		else {
			m_pAccident->Clear();
		}
	}

	// (r.gonet 2016-04-07) - NX-100072 - Use the new ClaimDates structure. Also fixed a bug where the wrong preference was
	// taking priority. The General 2 preference should take priority over the last bill preference.
	if ((nAccType == 1 || nAccType == 2) && m_claimDates.GetFirstConditionDate() == g_cdtNull) {
		if (varDefaultInjuryDate.vt == VT_DATE) {
			SetFirstConditionDate(VarDateTime(varDefaultInjuryDate));
		} else {
			// (r.gonet 2016-04-07) - NX-100072 - Clear the First Condition Date time control.
			ClearFirstConditionDate();
		}
	}

	// (b.eyers 2015-07-14) - PLID 47739 - moved checking preferences for insurance dates to it's own function
	UpdateInsuranceDates();

	//DRT 4/29/03 - Added a preference that allows the 'Condition Related To' field to auto
	//		fill in based on what previous bills had.  Only on New bills.
	if (GetBillID() == -1) {
		long nCondType = GetRemotePropertyInt("DefaultConditionType", 0, 0, "<None>", true);
		if(nCondType == 0) {
			//do nothing - this is already selected as None
		}
		else if(nCondType == 1) {
			//find the last bill and this field on it
			// (c.haag 2009-09-08 14:04) - PLID 35455 - Don't include deleted bills
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr rsCond = CreateParamRecordset(
				"SELECT ID, RelatedToEmp, RelatedToAutoAcc, RelatedToOther, State "
				"FROM BillsT "
				"WHERE BillsT.Deleted = 0 AND BillsT.ID = (SELECT Max(ID) FROM BillsT WHERE PatientID = {INT})", iPatientID);
			if(!rsCond->eof) {
				//there is a previous value!  Set this bill to whatever that was
				var = rsCond->Fields->GetItem("RelatedToEmp")->Value;
				if (VarBool(var, FALSE) != FALSE) {
					CheckDlgButton(IDC_RADIO_EMPLOYMENT, TRUE);
					CheckDlgButton(IDC_RADIO_AUTO_ACCIDENT, FALSE);
					CheckDlgButton(IDC_RADIO_OTHER_ACCIDENT, FALSE);
					CheckDlgButton(IDC_RADIO_NONE, FALSE);
				}
				var = rsCond->Fields->GetItem("RelatedToAutoAcc")->Value;
				if (VarBool(var, FALSE) != FALSE) {
					CheckDlgButton(IDC_RADIO_EMPLOYMENT, FALSE);
					CheckDlgButton(IDC_RADIO_AUTO_ACCIDENT, TRUE);
					CheckDlgButton(IDC_RADIO_OTHER_ACCIDENT, FALSE);
					CheckDlgButton(IDC_RADIO_NONE, FALSE);
				}
				var = rsCond->Fields->GetItem("RelatedToOther")->Value;
				if (VarBool(var, FALSE) != FALSE) {
					CheckDlgButton(IDC_RADIO_EMPLOYMENT, FALSE);
					CheckDlgButton(IDC_RADIO_AUTO_ACCIDENT, FALSE);
					CheckDlgButton(IDC_RADIO_OTHER_ACCIDENT, TRUE);
					CheckDlgButton(IDC_RADIO_NONE, FALSE);
				}
				var = rsCond->Fields->GetItem("State")->Value;
				SetDlgItemText(IDC_EDIT_STATE,VarString(var,""));
			}
		}
	}

	SetDlgItemText(IDC_EDIT_AUTHORIZATION_NUMBER,((CBillingModuleDlg*)m_pBillingModuleWnd)->m_strSelectedAuthNum);

	m_boInitialized = TRUE;
}

void CInsuranceBilling::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try{
		CNxDialog::OnShowWindow(bShow, nStatus);
		if (bShow == FALSE) return;

		GetDlgItem(IDC_FORM_TYPE_COMBO)->SetFocus();

		//TES 3/13/2007 - PLID 24993 - If they have changed from UB92 to UB04, and this dialog (which persists throughout
		// program execution) has already been created, then it might lead to some weird results if we don't update the
		// screen right away (like, now) to show that it's been changed.
		PostFormTypeChanged();

		// (c.haag 2009-09-03 14:10) - PLID 34781 - Make sure the tab is initialized before it is made visible
		EnsureInitialized();

	}NxCatchAll("Error initializing insurance tab.");
}

void CInsuranceBilling::OnOK()
{
	CWnd* pNextTabItem = GetNextDlgTabItem(GetFocus());
	pNextTabItem->SetFocus();
}

void CInsuranceBilling::OnCancel()
{
}

// Search keywords: SaveBillData
int CInsuranceBilling::SaveChanges()
{
	//Returns SCR_ABORT_SAVE, SCR_NOT_SAVED, or SCR_SAVED

	// (c.haag 2009-09-03 14:10) - PLID 34781 - This code is deprecated. The tab
	// should always be initialized at this point
	// /* Don't do anything if this tab was not even accessed */
	//if (!m_boInitialized)
	//	return SCR_NOT_SAVED;

	// (a.walling 2016-03-09 10:48) - PLID 68555 - UB04 Enhancements - Insert default occurrence code on save or on dialog creation to defer calculated default date that might depend on bill information
	if (!m_ub04DefaultOccurrenceCodeToApply.IsEmpty()) {
		COleDateTime dtDefaultDate, dtDefaultFrom, dtDefaultThrough;
		CalculateUB04DefaultDates(dtDefaultDate, dtDefaultFrom, dtDefaultThrough);			
		m_ub04ClaimInfo.occurrences.push_back({ m_ub04DefaultOccurrenceCodeToApply, dtDefaultDate });
		m_ub04DefaultOccurrenceCodeToApply = "";
	}

	// (r.goldschmidt 2016-03-11 17:00) - PLID 68585 - UB04 Enhancements - insert default remarks on save
	if (!m_ub04DefaultRemarksToApply.IsEmpty()) {
		m_ub04ClaimInfo.remarks = m_ub04DefaultRemarksToApply;
		m_ub04DefaultRemarksToApply = "";
	}

	_RecordsetPtr rs;
	FieldsPtr Fields;
	COleDateTime dt;
	_variant_t var;
	CString str;
	CString strOutsideLabCharges, strReferral,
		strAutoAccState, strMedicaidResubmission, strOriginalRefNo,
		strPriorAuthNum, strHCFABlock19,
		strUBBox14, strUBBox15;

	// (r.gonet 2016-04-07) - NX-100072 - m_claimDates replaced dtFirstCurrentCondition.
	COleDateTime dtCurrentCondition, dtWorkFrom, dtWorkTo, dtHospFrom, dtHospTo;
	COleDateTime dtAdmissionTime, dtDischargeTime;

	long nActivePatientID = m_nPatientID;

	BOOL bSendPaperwork = FALSE;
	CString strPaperworkType, strPaperworkTx;
	BOOL bSendTestResults = FALSE;
	CString strTestResultID, strTestResult;

	/***********************************
	* Ensure practice is open          *
	***********************************/
	EnsureRemoteData();

	CWaitCursor cuWait;

	try {
		/***********************************
		* Ensure the bill exists           *
		***********************************/
		
		// (j.jones 2008-05-22 09:52) - PLID 30051 - moved paperwork and test result fields out of this recordset
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
		// (j.jones 2013-08-14 12:45) - PLID 57902 - added HCFA Boxes 8, 9b, 9c, 11bQual, 11b
		// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36
		// (r.gonet 2016-04-07) - NX-100072 - Split FirstConditionDate into multiple date fields.
		rs = CreateParamRecordset(GetRemoteData(), adUseClient, adOpenDynamic, adLockOptimistic, "SELECT ConditionDate AS [Date Current Condition], "
			"FirstVisitOrConsultationDate, InitialTreatmentDate, LastSeenDate, AcuteManifestationDate, LastXRayDate, HearingAndPrescriptionDate, AssumedCareDate, RelinquishedCareDate, AccidentDate, "
			"NoWorkFrom AS [Not Working From], "
			"NoWorkTo AS [Not Working To], HospFrom AS [Hospitalized From], "
			"HospTo AS [Hospitalized To], RelatedToEmp AS [Condition Related to Employ], "
			"RelatedToAutoAcc AS [Condition Related to Auto Acc], RelatedToOther AS [Condition Related to Other Acc], "
			"OutsideLab, OutsideLabCharges, State AS [Auto Acc State], MedicaidResubmission, OriginalRefNo, PriorAuthNum, "
			"HCFABlock19, HCFABox8, HCFABox9b, HCFABox9c, HCFABox10D, HCFABox11bQual, HCFABox11b, UB92Box79, UB92Box44, UBBox14, UBBox15, "
			"RefPhyID, InsuredPartyID, OthrInsuredPartyID, SendCorrespondence "
			"FROM BillsT WHERE ID = {INT}", GetBillID());
		Fields = rs->Fields;

		if (rs->eof) {
			rs->Close();
			return SCR_NOT_SAVED;
		}

		long nFormType = 1;
		if(m_FormTypeCombo->GetCurSel() != -1) {
			nFormType = VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0), 1);
		}		
		
		/***********************************
		* Write to the bill                *
		***********************************/
		/* Date values */

		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		COleDateTime dtOldDate;
		dtOldDate.SetDateTime(1800,1,1,1,1,1);

		if(m_pAccident->GetStatus() == 1) {
			dtCurrentCondition = m_pAccident->GetDateTime();

			if(dtCurrentCondition < dtOldDate) {
				AfxMessageBox("The 'Date of Current Illness / Injury' is before the year 1800.\n"
					"Please correct this date before saving.");
				return SCR_ABORT_SAVE;
			}

			if(dtCurrentCondition > COleDateTime::GetCurrentTime()) {
				AfxMessageBox("The 'Date of Current Illness / Injury' is after today.\n"
					"Please correct this date before saving.");
				return SCR_ABORT_SAVE;
			}
		}
		else {
			dtCurrentCondition = dtInvalid;	// Date of current accident
		}
		
		// (r.gonet 2016-04-07) - NX-100072 - Use the ClaimDates structure.
		if(m_claimDates.AnyFilledAndValid()) {
			CString strAdditionalClaimValidationWarning = m_claimDates.GetValidationWarning();
			if (!strAdditionalClaimValidationWarning.IsEmpty()) {
				AfxMessageBox(strAdditionalClaimValidationWarning);
				return SCR_ABORT_SAVE;
			}
		}
		else {
			// (r.gonet 2016-04-07) - NX-100072
			ClearAdditionalClaimDates();
		}		
		
		if(m_pWorkFrom->GetStatus() == 1) {
			dtWorkFrom = m_pWorkFrom->GetDateTime();
			if(dtWorkFrom < dtOldDate) {
				AfxMessageBox("The 'patient unable to work from' date is before the year 1800.\n"
					"Please correct this date before saving.");
				return SCR_ABORT_SAVE;
			}
		}
		else {
			dtWorkFrom = dtInvalid;
		}

		if(m_pWorkTo->GetStatus() == 1) {
			dtWorkTo = m_pWorkTo->GetDateTime();
			if(dtWorkTo < dtOldDate) {
				AfxMessageBox("The 'patient unable to work to' date is before the year 1800.\n"
					"Please correct this date before saving.");
				return SCR_ABORT_SAVE;
			}
		}
		else {
			dtWorkTo = dtInvalid;
		}

		if(m_pWorkFrom->GetStatus() == 1 && m_pWorkTo->GetStatus() == 1 &&
			dtWorkTo < dtWorkFrom) {
			AfxMessageBox("The 'patient unable to work from' date is after the 'patient unable to work to' date.\n"
					"Please correct this date before saving.");
				return SCR_ABORT_SAVE;
		}

		if(m_pHospFrom->GetStatus() == 1) {
			dtHospFrom = m_pHospFrom->GetDateTime();
			if(dtHospFrom < dtOldDate) {
				AfxMessageBox("The 'patient hospitalized from' date is before the year 1800.\n"
					"Please correct this date before saving.");
				return SCR_ABORT_SAVE;
			}
		}
		else {
			dtHospFrom = dtInvalid;
		}

		if(m_pHospTo->GetStatus() == 1) {
			dtHospTo = m_pHospTo->GetDateTime();
			if(dtHospTo < dtOldDate) {
				AfxMessageBox("The 'patient hospitalized to' date is before the year 1800.\n"
					"Please correct this date before saving.");
				return SCR_ABORT_SAVE;
			}
		}
		else {
			dtHospTo = dtInvalid;
		}

		if(m_pHospFrom->GetStatus() == 1 && m_pHospTo->GetStatus() == 1 &&
			dtHospTo < dtHospFrom) {
			AfxMessageBox("The 'patient hospitalized to' date is after the 'patient hospitalized from' date.\n"
					"Please correct this date before saving.");
				return SCR_ABORT_SAVE;
		}

		// (j.jones 2013-06-07 16:23) - PLID 41479 - added admission & discharge times
		if(m_pAdmissionTime->GetStatus() == 1) {
			dtAdmissionTime = m_pAdmissionTime->GetDateTime();
			if(dtAdmissionTime.GetStatus() == COleDateTime::invalid) {
				AfxMessageBox("The Admission Time is invalid.\n"
					"Please correct this time before saving.");
				return SCR_ABORT_SAVE;
			}
		}
		else {
			dtAdmissionTime = dtInvalid;
		}
		if(m_pDischargeTime->GetStatus() == 1) {
			dtDischargeTime = m_pDischargeTime->GetDateTime();
			if(dtDischargeTime.GetStatus() == COleDateTime::invalid) {
				AfxMessageBox("The Discharge Time is invalid.\n"
					"Please correct this time before saving.");
				return SCR_ABORT_SAVE;
			}
		}
		else {
			dtDischargeTime = dtInvalid;
		}
		
		/* Condition-related-to-radio */
		var.vt = VT_BOOL;
		var.boolVal = ((CButton*)GetDlgItem(IDC_RADIO_EMPLOYMENT))->GetCheck();
		Fields->GetItem("Condition Related to Employ")->PutValue(var);
		var.boolVal = ((CButton*)GetDlgItem(IDC_RADIO_AUTO_ACCIDENT))->GetCheck();
		Fields->GetItem("Condition Related to Auto Acc")->Value = var;
		var.boolVal = ((CButton*)GetDlgItem(IDC_RADIO_OTHER_ACCIDENT))->GetCheck();
		Fields->GetItem("Condition Related to Other Acc")->Value = var;

		/* Outside-of-lab checkbox */
		var.boolVal = ((CButton*)GetDlgItem(IDC_OUTSIDE_LAB_CHECK))->GetCheck();
		Fields->GetItem("OutsideLab")->Value = var;		

		/* Outside of lab charges */
		GetDlgItem(IDC_EDIT_CHARGES)->GetWindowText(strOutsideLabCharges);
		COleCurrency tempcur = ParseCurrencyFromInterface(strOutsideLabCharges);
		if(!IsValidCurrencyText(strOutsideLabCharges) || tempcur.GetStatus() == COleCurrency::invalid) {
			strOutsideLabCharges = "";
		}
		else {
			strOutsideLabCharges = FormatCurrencyForSql(tempcur);
		}

		/* Send Correspondence checkbox */
		var.boolVal = ((CButton*)GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE))->GetCheck();
		Fields->GetItem("SendCorrespondence")->Value = var;

		// (a.walling 2007-08-27 11:11) - PLID 27026 - Sending paperwork info		
		bSendPaperwork = m_btnSendPaperwork.GetCheck();

		// get the type and tx
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPWKType->GetCurSel();
			if (pRow) {
				strPaperworkType = VarString(pRow->GetValue(0), "");
			}

			pRow = m_pPWKTx->GetCurSel();
			if (pRow) {
				strPaperworkTx = VarString(pRow->GetValue(0), "");
			}
		}
		
		if(bSendPaperwork && (strPaperworkType.IsEmpty() || strPaperworkTx.IsEmpty())) {
			AfxMessageBox("When sending paperwork, both the Type and Transmission codes must be set!");
			return SCR_ABORT_SAVE;
		}

		// (j.jones 2008-05-14 14:51) - PLID 30044 - added test result info
		bSendTestResults = m_checkSendTestResults.GetCheck();
		
		//get the ID, type, and result

		BOOL bFail = FALSE;

		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_TestResultIDCombo->GetCurSel();
			if (pRow) {
				strTestResultID = VarString(pRow->GetValue(trilCode), "");
			} else {
				bFail = TRUE;
			}

			// (j.jones 2010-10-22 10:04) - PLID 40962 - this can now have a combo box in use
			pRow = m_TestResultTypeCombo->GetCurSel();
			if (pRow) {
				m_strTestResultType = VarString(pRow->GetValue(trtlCode), "");
			}
			else if (m_TestResultTypeCombo->IsComboBoxTextInUse) {
				//do nothing, m_strTestResultType should already be that text
			}
			else {
				//combobox text has been unselected
				m_strTestResultType = "";
				bFail = TRUE;
			}

			GetDlgItemText(IDC_EDIT_TEST_RESULT, strTestResult);
			strTestResult.TrimLeft();
			strTestResult.TrimRight();
			if(strTestResult.IsEmpty()) {
				bFail = TRUE;
			}
		}

		//note: CBillingDlg::ValidateChanges() should have already caught this
		if(bSendTestResults && bFail && nFormType == 1) { //HCFA-only, otherwise we don't care
			AfxMessageBox("The 'Send Test Result' option cannot be enabled without entering information in the Identifier, Type, and Result fields.\n"
				"Please fill in these fields, or uncheck the 'Send Test Result' box.");
			return SCR_ABORT_SAVE;
		}

		/* Edit boxes */
		GetDlgItem(IDC_EDIT_STATE)->GetWindowText(strAutoAccState);
		GetDlgItem(IDC_EDIT_MEDICAID_CODE)->GetWindowText(strMedicaidResubmission);
		GetDlgItem(IDC_EDIT_REFERENCE_NUMBER)->GetWindowText(strOriginalRefNo);
		GetDlgItem(IDC_EDIT_AUTHORIZATION_NUMBER)->GetWindowText(strPriorAuthNum);
		GetDlgItem(IDC_EDIT_BOX_19)->GetWindowText(strHCFABlock19);
		// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
		m_nxeditEditUBBox14.GetWindowText(strUBBox14);
		m_nxeditEditUBBox15.GetWindowText(strUBBox15);
		
		strAutoAccState.TrimLeft();
		strAutoAccState.TrimRight();
		if(strAutoAccState.GetLength() > 2)
			strAutoAccState = strAutoAccState.Left(2);

		long nRefPhyID = -1;
		// Referring physician
		if(m_pRefList->CurSel != -1)
			nRefPhyID = VarLong(m_pRefList->GetValue(m_pRefList->CurSel, rpcID),-1);
		//(e.lally 2005-11-01) PLID 17444 - We need to account for inactive referring physicians
			//This assumes that if the referring physician is inactive, the CurSel will be -1
		else if(m_pRefList->IsComboBoxTextInUse && m_nPendingReferringPhyID != -1)
			nRefPhyID = m_nPendingReferringPhyID;
		Fields->GetItem("RefPhyID")->Value = (long)nRefPhyID;

		// (j.jones 2006-11-06 16:33) - PLID 22534 - added UB92 Box 44 CPT List
		long nUB92Box44 = -1;
		// UB92 Box44
		if(m_pUB92Box44List->CurSel != -1)
			nUB92Box44 = VarLong(m_pUB92Box44List->GetValue(m_pUB92Box44List->CurSel, 0),-1);
		else if(m_pUB92Box44List->IsComboBoxTextInUse && m_nPendingUB92Box44ID != -1)
			nUB92Box44 = m_nPendingUB92Box44ID;

		_variant_t varBox44;
		varBox44.vt = VT_NULL;
		if(nUB92Box44 != -1)
			varBox44 = (long)nUB92Box44;
		Fields->GetItem("UB92Box44")->Value = varBox44;

		// First insurance company
		long nInsuredPartyID = -1;
		CString strInsuredPartyName = "";
		if(m_pInsurance1->CurSel != -1) {
			nInsuredPartyID = VarLong(m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID),-1);
			// (j.jones 2009-03-10 17:45) - PLID 32864 - grab the name as well
			strInsuredPartyName = VarString(m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccName), "");
		}
		// (j.jones 2009-03-10 17:45) - PLID 32864 - moved to the auditing area of this function
		//Fields->GetItem("InsuredPartyID")->Value = (long)nInsuredPartyID;
		// Second insurance company
		long nOthrInsuredPartyID = -1;
		CString strOthrInsuredPartyName = "";
		if(m_pInsurance2->CurSel != -1) {
			nOthrInsuredPartyID = VarLong(m_pInsurance2->GetValue(m_pInsurance2->CurSel, ipccID),-1);
			// (j.jones 2009-03-10 17:45) - PLID 32864 - grab the name as well
			strOthrInsuredPartyName = VarString(m_pInsurance2->GetValue(m_pInsurance2->CurSel, ipccName), "");
		}
		// (j.jones 2009-03-10 17:45) - PLID 32864 - moved to the auditing area of this function
		//Fields->GetItem("OthrInsuredPartyID")->Value = (long)nOthrInsuredPartyID;

		rs->Update();

		// Update batch status
		{
			short nBatch = 0;

			if (((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->GetCheck()) {
				nBatch = 1; //paper batch
			}
			else if (((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->GetCheck())
				nBatch = 2; // electronic batch

			if(nBatch != 0) {

				// (j.jones 2008-02-13 09:57) - PLID 28847 - check whether they are allowed to batch
				// the claim
				// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
				if(!CanCreateInsuranceClaim(GetBillID(), FALSE)) {					
					((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(0);
					((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(0);
					((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(1);
					BatchBill(GetBillID(), 0, TRUE);
				}
				else {
				
					if(nInsuredPartyID == -1) {
						AfxMessageBox("This bill cannot be batched without an insurance company selected.\n"
							"Please select an insurance company on the insurance tab of the bill.");
						return SCR_ABORT_SAVE;
					}

					//don't automatically batch a $0.00 claim
					COleCurrency cyCharges = COleCurrency(0,0);
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT dbo.GetClaimTotal(ID) AS Total FROM BillsT WHERE ID = {INT}",GetBillID());
					if(!rs->eof) {
						cyCharges = AdoFldCurrency(rs, "Total",COleCurrency(0,0)); 
					}
					rs->Close();

					CString strWarn;
					strWarn.Format("This claim is currently %s. Are you sure you wish to send the claim to the %s batch?",
						FormatCurrencyForInterface(cyCharges,TRUE,TRUE), nBatch == 1 ? "paper" : "electronic");

					if(cyCharges > COleCurrency(0,0) || 
						IDYES == MessageBox(strWarn,"Practice",MB_YESNO|MB_ICONQUESTION)) {

						//tell BatchBill to skip the resp check, as we already did so above
						BatchBill(GetBillID(), nBatch, TRUE);					
					}
					else {
						((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(0);
						((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(0);
						((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(1);
						BatchBill(GetBillID(), 0, TRUE);
					}
				}
			}
			else {
				BatchBill(GetBillID(), 0);
			}
		}

		// (j.jones 2009-02-11 10:05) - PLID 33035 - added Manual Review
		BOOL bManualReview = m_checkManualReview.GetCheck();

		// (j.jones 2009-09-15 09:13) - PLID 35284 - added ANSI_ClaimTypeCode
		ANSI_ClaimTypeCode eClaimTypeCode = ctcOriginal;
		NXDATALIST2Lib::IRowSettingsPtr pClaimTypeRow = m_ClaimTypeCombo->GetCurSel();
		if(pClaimTypeRow) {
			eClaimTypeCode = (ANSI_ClaimTypeCode)VarLong(pClaimTypeRow->GetValue(ctccID));
		}

		// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
		PriorAuthType patPriorAuthType = patPriorAuthNumber;
		NXDATALIST2Lib::IRowSettingsPtr pPriorAuthRow = m_PriorAuthTypeCombo->GetCurSel();
		if(pPriorAuthRow) {
			patPriorAuthType = (PriorAuthType)VarLong(pPriorAuthRow->GetValue(patccID));
		}

		// (j.jones 2010-06-10 09:49) - PLID 38507 - added HCFA Box 13 config
		HCFABox13Over hb13Value = hb13_UseDefault;
		NXDATALIST2Lib::IRowSettingsPtr pBox23Row = m_HCFABox13Combo->GetCurSel();
		if(pBox23Row) {
			hb13Value = (HCFABox13Over)VarLong(pBox23Row->GetValue(hb13ccID));
		}

		// (j.jones 2007-01-16 09:59) - PLID 23650 - regardless of whether the claim is
		// actually in a batch, set m_bBatched to TRUE to signify we have made that decision
		// (j.gruber 2009-07-10 17:19) - PLID 34724 - take this out 
		//((CBillingModuleDlg*)m_pBillingModuleWnd)->m_bBatched = TRUE;
		
		rs->Close();

		try {
			long AuditID = -1;
			// (j.jones 2008-05-22 09:44) - PLID 30051 - added audits for paperwork and test results, and parameterized
			// (j.jones 2009-02-11 10:05) - PLID 33035 - added ManualReview
			// (j.jones 2009-03-10 17:46) - PLID 32864 - added InsuredPartyID and OthrInsuredPartyID, and their company names
			// (j.jones 2009-09-15 09:13) - PLID 35284 - added ANSI_ClaimTypeCode
			// (j.jones 2010-03-02 14:00) - PLID 37584 - added PriorAuthType
			// (j.jones 2012-01-23 16:50) - PLID 47731 - added ConditionDateType
			// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
			// (j.jones 2013-06-07 16:23) - PLID 41479 - added admission & discharge times
			// (j.jones 2013-08-14 12:45) - PLID 57902 - added HCFA Boxes 8, 9b, 9c, 11bQual, 11b
			// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36
			// (r.gonet 2016-04-07) - NX-100072 - Split the FirstConditionDate into multiple date fields.
			rs = CreateParamRecordset("SELECT FormType, ConditionDate, "
				"FirstVisitOrConsultationDate, InitialTreatmentDate, LastSeenDate, AcuteManifestationDate, LastXRayDate, HearingAndPrescriptionDate, AssumedCareDate, RelinquishedCareDate, AccidentDate, "
				"NoWorkFrom, NoWorkTo, HospFrom, HospTo, "
				"State, MedicaidResubmission, OriginalRefNo, PriorAuthNum, HCFABlock19, HCFABox8, HCFABox9b, HCFABox9c, HCFABox10D, HCFABox11bQual, HCFABox11b, "
				"UB92Box79, UBBox14, UBBox15, OutsideLabCharges, "
				"SendPaperwork, PaperworkType, PaperworkTx, SendTestResults, TestResultID, TestResultType, TestResult, "
				"ManualReview, InsuredPartyID, OthrInsuredPartyID, InsuranceCoT.Name As InsCoName, OthrInsuranceCoT.Name AS OthrInsCoName, "
				"ANSI_ClaimTypeCode, PriorAuthType, ConditionDateType, BillsT.AdmissionTime, BillsT.DischargeTime "
				"FROM BillsT "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN InsuredPartyT OthrInsuredPartyT ON BillsT.OthrInsuredPartyID = OthrInsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT OthrInsuranceCoT ON OthrInsuredPartyT.InsuranceCoID = OthrInsuranceCoT.PersonID "
				"WHERE ID = {INT}", GetBillID());
			
			if(!rs->eof) {

				// (a.walling 2008-05-05 13:18) - PLID 29897 - Replaced all GetActivePatientName references with GetBillPatientName
				CString strPatientName = GetBillPatientName();

				var = rs->Fields->Item["FormType"]->Value;
				if(var.vt != VT_NULL && var.lVal != nFormType) {
					CString strOld, strNew;
					if(var.lVal == 2)
						//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
						strOld = "UB";
					else
						strOld = "HCFA";
					if(nFormType == 2)
						//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
						strNew = "UB";
					else
						strNew = "HCFA";
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillFormType,GetBillID(),strOld,strNew,aepMedium);
				}

				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);

				COleDateTime dt;

				dt = AdoFldDateTime(rs, "ConditionDate",dtInvalid);
				if(FormatDateTimeForInterface(dt) != FormatDateTimeForInterface(dtCurrentCondition)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					CString strOld = FormatDateTimeForInterface(dt);
					CString strNew = FormatDateTimeForInterface(dtCurrentCondition);
					if(strOld.Find("Invalid") != -1)
						strOld = "";
					if(strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillConditionDate,GetBillID(),strOld,strNew,aepLow);
				}

				// (r.gonet 2016-04-07) - NX-100072 - Set the condition date type.
				ClaimDates oldClaimDates;
				// (j.jones 2012-01-23 16:42) - PLID 47731 - audit the condition date combo
				oldClaimDates.eConditionDateType = (ConditionDateType)AdoFldLong(rs, "ConditionDateType");
				if(oldClaimDates.eConditionDateType != m_claimDates.eConditionDateType) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}

					CString strOld, strNew;

					// (j.jones 2013-08-16 14:06) - PLID 58069 - added more fields, and streamlined this code
					// (r.gonet 2016-04-07) - NX-100072 - Use a function to get the date type description.
					strOld = GetConditionDateTypeDescription(oldClaimDates.eConditionDateType);
					strNew = GetConditionDateTypeDescription(m_claimDates.eConditionDateType);
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillConditionDateType, GetBillID(), strOld, strNew, aepLow);
				}

				// (r.gonet 2016-04-07) - NX-100072 - Use the new structure.
				oldClaimDates.dtFirstVisitOrConsultationDate = AdoFldDateTime(rs, "FirstVisitOrConsultationDate", g_cdtNull);
				oldClaimDates.dtInitialTreatmentDate = AdoFldDateTime(rs, "InitialTreatmentDate", g_cdtNull);
				oldClaimDates.dtLastSeenDate = AdoFldDateTime(rs, "LastSeenDate", g_cdtNull);
				oldClaimDates.dtAcuteManifestationDate = AdoFldDateTime(rs, "AcuteManifestationDate", g_cdtNull);
				oldClaimDates.dtLastXRayDate = AdoFldDateTime(rs, "LastXRayDate", g_cdtNull);
				oldClaimDates.dtHearingAndPrescriptionDate = AdoFldDateTime(rs, "HearingAndPrescriptionDate", g_cdtNull);
				oldClaimDates.dtAssumedCareDate = AdoFldDateTime(rs, "AssumedCareDate", g_cdtNull);
				oldClaimDates.dtRelinquishedCareDate = AdoFldDateTime(rs, "RelinquishedCareDate", g_cdtNull);
				oldClaimDates.dtAccidentDate = AdoFldDateTime(rs, "AccidentDate", g_cdtNull);

				if (FormatDateTimeForInterface(oldClaimDates.dtFirstVisitOrConsultationDate) != FormatDateTimeForInterface(m_claimDates.dtFirstVisitOrConsultationDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtFirstVisitOrConsultationDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtFirstVisitOrConsultationDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillFirstVisitOrConsultationDate, GetBillID(), strOld, strNew, aepLow);
				}

				if (FormatDateTimeForInterface(oldClaimDates.dtInitialTreatmentDate) != FormatDateTimeForInterface(m_claimDates.dtInitialTreatmentDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtInitialTreatmentDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtInitialTreatmentDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillInitialTreatmentDate, GetBillID(), strOld, strNew, aepLow);
				}

				if (FormatDateTimeForInterface(oldClaimDates.dtLastSeenDate) != FormatDateTimeForInterface(m_claimDates.dtLastSeenDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtLastSeenDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtLastSeenDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillLastSeenDate, GetBillID(), strOld, strNew, aepLow);
				}

				if (FormatDateTimeForInterface(oldClaimDates.dtAcuteManifestationDate) != FormatDateTimeForInterface(m_claimDates.dtAcuteManifestationDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtAcuteManifestationDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtAcuteManifestationDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillAcuteManifestationDate, GetBillID(), strOld, strNew, aepLow);
				}

				if (FormatDateTimeForInterface(oldClaimDates.dtLastXRayDate) != FormatDateTimeForInterface(m_claimDates.dtLastXRayDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtLastXRayDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtLastXRayDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillLastXRayDate, GetBillID(), strOld, strNew, aepLow);
				}

				if (FormatDateTimeForInterface(oldClaimDates.dtHearingAndPrescriptionDate) != FormatDateTimeForInterface(m_claimDates.dtHearingAndPrescriptionDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtHearingAndPrescriptionDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtHearingAndPrescriptionDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillHearingAndPrescriptionDate, GetBillID(), strOld, strNew, aepLow);
				}

				if (FormatDateTimeForInterface(oldClaimDates.dtAssumedCareDate) != FormatDateTimeForInterface(m_claimDates.dtAssumedCareDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtAssumedCareDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtAssumedCareDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillAssumedCareDate, GetBillID(), strOld, strNew, aepLow);
				}

				if (FormatDateTimeForInterface(oldClaimDates.dtRelinquishedCareDate) != FormatDateTimeForInterface(m_claimDates.dtRelinquishedCareDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtRelinquishedCareDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtRelinquishedCareDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillRelinquishedCareDate, GetBillID(), strOld, strNew, aepLow);
				}

				if (FormatDateTimeForInterface(oldClaimDates.dtAccidentDate) != FormatDateTimeForInterface(m_claimDates.dtAccidentDate)) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld = FormatDateTimeForInterface(oldClaimDates.dtAccidentDate);
					CString strNew = FormatDateTimeForInterface(m_claimDates.dtAccidentDate);
					if (strOld.Find("Invalid") != -1)
						strOld = "";
					if (strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillAccidentDate, GetBillID(), strOld, strNew, aepLow);
				}

				dt = AdoFldDateTime(rs, "NoWorkFrom",dtInvalid);
				if(FormatDateTimeForInterface(dt) != FormatDateTimeForInterface(dtWorkFrom)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					CString strOld = FormatDateTimeForInterface(dt);
					CString strNew = FormatDateTimeForInterface(dtWorkFrom);
					if(strOld.Find("Invalid") != -1)
						strOld = "";
					if(strNew.Find("Invalid") != -1)
						strNew = "";					
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillNoWorkFrom,GetBillID(),strOld,strNew,aepLow);
				}

				dt = AdoFldDateTime(rs, "NoWorkTo",dtInvalid);
				if(FormatDateTimeForInterface(dt) != FormatDateTimeForInterface(dtWorkTo)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					CString strOld = FormatDateTimeForInterface(dt);
					CString strNew = FormatDateTimeForInterface(dtWorkTo);
					if(strOld.Find("Invalid") != -1)
						strOld = "";
					if(strNew.Find("Invalid") != -1)
						strNew = "";					
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillNoWorkTo,GetBillID(),strOld,strNew,aepLow);
				}

				dt = AdoFldDateTime(rs, "HospFrom",dtInvalid);
				if(FormatDateTimeForInterface(dt) != FormatDateTimeForInterface(dtHospFrom)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					CString strOld = FormatDateTimeForInterface(dt);
					CString strNew = FormatDateTimeForInterface(dtHospFrom);
					if(strOld.Find("Invalid") != -1)
						strOld = "";
					if(strNew.Find("Invalid") != -1)
						strNew = "";					
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillHospFrom,GetBillID(),strOld,strNew,aepLow);
				}

				dt = AdoFldDateTime(rs, "HospTo",dtInvalid);
				if(FormatDateTimeForInterface(dt) != FormatDateTimeForInterface(dtHospTo)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();					
					CString strOld = FormatDateTimeForInterface(dt);
					CString strNew = FormatDateTimeForInterface(dtHospTo);
					if(strOld.Find("Invalid") != -1)
						strOld = "";
					if(strNew.Find("Invalid") != -1)
						strNew = "";
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillHospTo,GetBillID(),strOld,strNew,aepLow);
				}

				// (j.jones 2013-06-07 16:23) - PLID 41479 - added admission & discharge times
				dt = AdoFldDateTime(rs, "AdmissionTime", dtInvalid);
				if(FormatDateTimeForInterface(dt, NULL, dtoTime) != FormatDateTimeForInterface(dtAdmissionTime, NULL, dtoTime)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();					
					CString strOld = FormatDateTimeForInterface(dt, NULL, dtoTime);
					CString strNew = FormatDateTimeForInterface(dtAdmissionTime, NULL, dtoTime);
					if(strOld.Find("Invalid") != -1)
						strOld = "<No Time>";
					if(strNew.Find("Invalid") != -1)
						strNew = "<No Time>";

					CString strOldValue;
					// (r.gonet 07/02/2014) - PLID 62567 - Use the bill description accessor rather than reading it directly.
					CString strBillDescription = GetBillDescriptionWithPrefix();
					strOldValue.Format("%s (Bill: %s, Date: %s)", strOld, strBillDescription, FormatDateTimeForInterface(VarDateTime(m_peditBillDate->GetValue()), NULL, dtoDate));

					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillAdmissionTime, GetBillID(), strOldValue, strNew, aepLow);
				}
				dt = AdoFldDateTime(rs, "DischargeTime", dtInvalid);
				if(FormatDateTimeForInterface(dt, NULL, dtoTime) != FormatDateTimeForInterface(dtDischargeTime, NULL, dtoTime)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();					
					CString strOld = FormatDateTimeForInterface(dt, NULL, dtoTime);
					CString strNew = FormatDateTimeForInterface(dtDischargeTime, NULL, dtoTime);
					if(strOld.Find("Invalid") != -1)
						strOld = "<No Time>";
					if(strNew.Find("Invalid") != -1)
						strNew = "<No Time>";

					CString strOldValue;
					// (r.gonet 07/02/2014) - PLID 62567 - Use the bill description accessor rather than reading it directly.
					CString strBillDescription = GetBillDescriptionWithPrefix();
					strOldValue.Format("%s (Bill: %s, Date: %s)", strOld, strBillDescription, FormatDateTimeForInterface(VarDateTime(m_peditBillDate->GetValue()), NULL, dtoDate));

					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillDischargeTime, GetBillID(), strOldValue, strNew, aepLow);
				}

				var = rs->Fields->Item["State"]->Value;
				CString strOldAccState = VarString(var,"");
				strOldAccState.TrimRight();
				strAutoAccState.TrimRight();
				if(strOldAccState != strAutoAccState) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillState,GetBillID(),strOldAccState,strAutoAccState,aepLow);
				}

				var = rs->Fields->Item["MedicaidResubmission"]->Value;
				if(var.vt != VT_NULL && (CString(var.bstrVal) != strMedicaidResubmission)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillMedicaidResubmission,GetBillID(),CString(var.bstrVal),strMedicaidResubmission,aepLow);
				}

				var = rs->Fields->Item["OriginalRefNo"]->Value;
				if(var.vt != VT_NULL && (CString(var.bstrVal) != strOriginalRefNo)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillOriginalRefNo,GetBillID(),CString(var.bstrVal),strOriginalRefNo,aepLow);
				}

				var = rs->Fields->Item["PriorAuthNum"]->Value;
				if(var.vt != VT_NULL && (CString(var.bstrVal) != strPriorAuthNum)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillPriorAuthNum,GetBillID(),CString(var.bstrVal),strPriorAuthNum,aepLow);
				}

				var = rs->Fields->Item["HCFABlock19"]->Value;
				if(var.vt != VT_NULL && (CString(var.bstrVal) != strHCFABlock19)) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillHCFABlock19,GetBillID(),CString(var.bstrVal),strHCFABlock19,aepLow);
				}

				// (j.jones 2013-08-13 13:49) - PLID 57902 - these fields are now stored in member variables,
				// the fields to edit them are now in the BillClaimFieldsDlg, and if they changed anything,
				// the variables would already be updated
				var = rs->Fields->Item["HCFABox8"]->Value;
				if(VarString(var, "") != m_strHCFABox8) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillHCFABox8, GetBillID(), VarString(var, ""), m_strHCFABox8, aepLow);
				}

				var = rs->Fields->Item["HCFABox9b"]->Value;
				if(VarString(var, "") != m_strHCFABox9b) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillHCFABox9b, GetBillID(), VarString(var, ""), m_strHCFABox9b, aepLow);
				}

				var = rs->Fields->Item["HCFABox9c"]->Value;
				if(VarString(var, "") != m_strHCFABox9c) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillHCFABox9c, GetBillID(), VarString(var, ""), m_strHCFABox9c, aepLow);
				}

				var = rs->Fields->Item["HCFABox10D"]->Value;
				if(VarString(var, "") != m_strHCFABox10d) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillHCFABox10D, GetBillID(), VarString(var, ""), m_strHCFABox10d, aepLow);
				}

				var = rs->Fields->Item["HCFABox11bQual"]->Value;
				if(VarString(var, "") != m_strHCFABox11bQual) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillHCFABox11bQual, GetBillID(), VarString(var, ""), m_strHCFABox11bQual, aepLow);
				}

				var = rs->Fields->Item["HCFABox11b"]->Value;
				if(VarString(var, "") != m_strHCFABox11b) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillHCFABox11b, GetBillID(), VarString(var, ""), m_strHCFABox11b, aepLow);
				}

				var = rs->Fields->Item["UB92Box79"]->Value;
				if(VarString(var, "") != m_strUB92Box79) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillUB92Box79, GetBillID(), VarString(var, ""), m_strUB92Box79, aepLow);
				}

				// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36

				// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
				var = rs->Fields->Item["UBBox14"]->Value;
				if(var.vt == VT_BSTR && VarString(var) != strUBBox14) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();

					CString strOldValue;
					// (r.gonet 07/02/2014) - PLID 62567 - Use the bill description accessor rather than reading it directly.
					CString strBillDescription = GetBillDescriptionWithPrefix();
					strOldValue.Format("%s (Bill: %s, Date: %s)", VarString(var), strBillDescription, FormatDateTimeForInterface(VarDateTime(m_peditBillDate->GetValue()), NULL, dtoDate));

					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillUBBox14,GetBillID(),strOldValue,strUBBox14,aepLow);
				}
				var = rs->Fields->Item["UBBox15"]->Value;
				if(var.vt == VT_BSTR && VarString(var) != strUBBox15) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();

					CString strOldValue;
					CString strBillDescription = GetBillDescriptionWithPrefix();
					strOldValue.Format("%s (Bill: %s, Date: %s)", VarString(var), strBillDescription, FormatDateTimeForInterface(VarDateTime(m_peditBillDate->GetValue()), NULL, dtoDate));

					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillUBBox15,GetBillID(),strOldValue,strUBBox15,aepLow);
				}

				// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36

				var = rs->Fields->Item["OutsideLabCharges"]->Value;
				if(var.vt != VT_NULL && (FormatCurrencyForSql(var.cyVal) != strOutsideLabCharges) ) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();
					COleCurrency cyOldCharges = ParseCurrencyFromSql(strOutsideLabCharges);
					AuditEvent(m_nPatientID, strPatientName,AuditID,aeiBillOutsideLabCharges,GetBillID(),FormatCurrencyForInterface(var.cyVal),FormatCurrencyForInterface(cyOldCharges),aepHigh);
				}

				// (j.jones 2008-05-22 09:44) - PLID 30051 - added audits for paperwork and test results, and parameterized
				BOOL bOldSendPaperwork = AdoFldBool(rs, "SendPaperwork", FALSE);
				if(bOldSendPaperwork != bSendPaperwork) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld, strNew;
					strOld = bOldSendPaperwork ? "Enabled" : "Disabled";
					strNew = bSendPaperwork ? "Enabled" : "Disabled";

					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillSendPaperwork, GetBillID(), strOld, strNew, aepMedium, aetChanged);
				}

				CString strOldPaperworkType = AdoFldString(rs, "PaperworkType", "");
				if(strOldPaperworkType != strPaperworkType) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillPaperworkType, GetBillID(), strOldPaperworkType, strPaperworkType, aepMedium, aetChanged);
				}

				CString strOldPaperworkTx = AdoFldString(rs, "PaperworkTx", "");
				if(strOldPaperworkTx != strPaperworkTx) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillPaperworkTx, GetBillID(), strOldPaperworkTx, strPaperworkTx, aepMedium, aetChanged);
				}

				BOOL bOldSendTestResults = AdoFldBool(rs, "SendTestResults", FALSE);
				if(bOldSendTestResults != bSendTestResults) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					CString strOld, strNew;
					strOld = bOldSendTestResults ? "Enabled" : "Disabled";
					strNew = bSendTestResults ? "Enabled" : "Disabled";

					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillSendTestResults, GetBillID(), strOld, strNew, aepMedium, aetChanged);
				}

				CString strOldTestResultID = AdoFldString(rs, "TestResultID", "");
				if(strOldTestResultID != strTestResultID) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillTestResultID, GetBillID(), strOldTestResultID, strTestResultID, aepMedium, aetChanged);
				}

				CString strOldTestResultType = AdoFldString(rs, "TestResultType", "");
				if(strOldTestResultType != m_strTestResultType) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillTestResultType, GetBillID(), strOldTestResultType, m_strTestResultType, aepMedium, aetChanged);
				}

				CString strOldTestResult = AdoFldString(rs, "TestResult", "");
				if(strOldTestResult != strTestResult) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillTestResult, GetBillID(), strOldTestResult, strTestResult, aepMedium, aetChanged);
				}

				// (j.jones 2009-02-11 10:05) - PLID 33035 - audit the ManualReview flag
				BOOL bOldManualReview = AdoFldBool(rs, "ManualReview", FALSE);
				if(bOldManualReview != bManualReview) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillManualReview, GetBillID(), bOldManualReview ? "Checked" : "Unchecked", bManualReview ? "Checked" : "Unchecked", aepMedium, aetChanged);
				}

				// (j.jones 2009-09-15 09:13) - PLID 35284 - added ANSI_ClaimTypeCode
				ANSI_ClaimTypeCode eOldClaimTypeCode = (ANSI_ClaimTypeCode)AdoFldLong(rs, "ANSI_ClaimTypeCode");;
				if(eClaimTypeCode != eOldClaimTypeCode) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}

					// (j.jones 2011-07-21 10:07) - PLID 44662 - GetClaimTypeName now generates these names
					// (j.jones 2016-05-24 14:37) - NX-100704 - this function is now unique per form type
					CString strOldCodeName, strNewCodeName;
					if (UseAlbertaHLINK()) {
						//Alberta
						strOldCodeName = GetClaimTypeDescription_Alberta(eOldClaimTypeCode);
						strNewCodeName = GetClaimTypeDescription_Alberta(eClaimTypeCode);
					}
					else if (nFormType == 2) {
						//UB
						strOldCodeName = GetClaimTypeDescription_UB(eOldClaimTypeCode);
						strNewCodeName = GetClaimTypeDescription_UB(eClaimTypeCode);
					}
					else {
						//HCFA
						strOldCodeName = GetClaimTypeDescription_HCFA(eOldClaimTypeCode);
						strNewCodeName = GetClaimTypeDescription_HCFA(eClaimTypeCode);
					}

					CString strOldValue;
					// (r.gonet 07/02/2014) - PLID 62567 - Use the bill description accessor rather than reading it directly.
					CString strBillDescription = GetBillDescriptionWithPrefix();

					strOldValue.Format("%s (Bill: %s, Date: %s)", strOldCodeName, strBillDescription, FormatDateTimeForInterface(VarDateTime(m_peditBillDate->GetValue()), NULL, dtoDate));

					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillClaimTypeCode, GetBillID(), strOldValue, strNewCodeName, aepMedium, aetChanged);
				}

				// (j.jones 2010-03-02 15:48) - PLID 37584 - audit PriorAuthType
				PriorAuthType patOldPriorAuthType = (PriorAuthType)AdoFldLong(rs, "PriorAuthType", (long)patPriorAuthNumber);
				if(patPriorAuthType != patOldPriorAuthType) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}

					CString strOldType, strNewType;

					if(patOldPriorAuthType == patReferralNumber) {
						strOldType = "Referral Number";
					}
					else {
						strOldType = "Prior Authorization Number";
					}

					if(patPriorAuthType == patReferralNumber) {
						strNewType = "Referral Number";
					}
					else {
						strNewType = "Prior Authorization Number";
					}

					CString strOldValue;
					// (r.gonet 07/02/2014) - PLID 62567 - Use the bill description accessor rather than reading it directly.
					CString strBillDescription = GetBillDescriptionWithPrefix();

					strOldValue.Format("%s (Bill: %s, Date: %s)", strOldType, strBillDescription, FormatDateTimeForInterface(VarDateTime(m_peditBillDate->GetValue()), NULL, dtoDate));

					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillPriorAuthType, GetBillID(), strOldValue, strNewType, aepMedium, aetChanged);
				}

				// (j.jones 2010-06-10 10:41) - PLID 38507 - audit the HCFA Box 13 config
				// (j.jones 2010-07-27 11:06) - PLID 39858 - reworded to support UB
				if(hb13Value != m_eOldHCFABox13Over) {
					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}

					CString strOldValue, strNewValue;

					if(m_eOldHCFABox13Over == hb13_No) {
						strOldValue = "Do Not Assign Benefits To Provider (Do Not Fill HCFA Box 13 / UB Box 53)";
					}
					else if(m_eOldHCFABox13Over == hb13_Yes) {
						strOldValue = "Assign Benefits To Provider (Fill HCFA Box 13 / UB Box 53)";
					}
					else {
						// (j.jones 2010-07-22 12:09) - PLID 39780 - claims now use the HCFA setup's Box 13
						// setting, which itself follows the Accept Assignment value, so I changed this
						// wording for accuracy
						strOldValue = "<Use Default Assignment Of Benefits Value>";
					}

					if(hb13Value == hb13_No) {
						strNewValue = "Do Not Assign Benefits To Provider (Do Not Fill HCFA Box 13 / UB Box 53)";
					}
					else if(hb13Value == hb13_Yes) {
						strNewValue = "Assign Benefits To Provider (Fill HCFA Box 13 / UB Box 53)";
					}
					else {
						// (j.jones 2010-07-22 12:09) - PLID 39780 - claims now use the HCFA setup's Box 13
						// setting, which itself follows the Accept Assignment value, so I changed this
						// wording for accuracy
						strNewValue = "<Use Default Assignment Of Benefits Value>";
					}

					CString strOldDesc;
					// (r.gonet 07/02/2014) - PLID 62567 - Use the bill description accessor rather than reading it directly.
					CString strBillDescription = GetBillDescriptionWithPrefix();

					strOldDesc.Format("%s (Bill: %s, Date: %s)", strOldValue, strBillDescription, FormatDateTimeForInterface(VarDateTime(m_peditBillDate->GetValue()), NULL, dtoDate));

					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillHCFABox13Over, GetBillID(), strOldDesc, strNewValue, aepMedium, aetChanged);
				}

				// (j.jones 2009-03-10 17:46) - PLID 32864 - audit InsuredPartyID and OthrInsuredPartyID
				// (j.jones 2010-08-17 12:03) - PLID 40135 - these IDs are cached now
				if(m_nOldInsuranceID1 != nInsuredPartyID) {

					CString strOldName = AdoFldString(rs, "InsCoName", "<None Selected>");
					CString strNewName = strInsuredPartyName;
					if(nInsuredPartyID == -1) {
						strNewName = "<None Selected>";
					}

					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillInsurancePlan, GetBillID(), strOldName, strNewName, aepMedium, aetChanged);
				}

				// (j.jones 2010-08-17 12:03) - PLID 40135 - these IDs are cached now
				if(m_nOldInsuranceID2 != nOthrInsuredPartyID) {

					CString strOldName = AdoFldString(rs, "OthrInsCoName", "<None Selected>");
					CString strNewName = strOthrInsuredPartyName;
					if(nOthrInsuredPartyID == -1) {
						strNewName = "<None Selected>";
					}

					if(AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillOtherInsurancePlan, GetBillID(), strOldName, strNewName, aepMedium, aetChanged);
				}

				// (a.walling 2016-03-07 08:53) - PLID 68499 - UB04 Enhancements - Auditing
				if (m_ub04ClaimInfo != m_ub04ClaimInfoOld) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
			}

					AuditEvent(m_nPatientID, strPatientName, AuditID, aeiBillUB04ClaimInfo, GetBillID(), m_ub04ClaimInfoOld.GetAuditDescription(), m_ub04ClaimInfo.GetAuditDescription(), aepMedium, aetChanged);
				}
			}
			rs->Close();
		}NxCatchAll("Error in updating Audit Trail.");

		// (j.jones 2009-09-21 11:37) - PLID 35564 - -1 is no longer allowed for insured party IDs, so set to NULL
		CString strInsuredPartyID = "NULL";
		CString strOthrInsuredPartyID = "NULL";
		if(nInsuredPartyID != -1) {
			strInsuredPartyID.Format("%li", nInsuredPartyID);
		}
		if(nOthrInsuredPartyID != -1) {
			strOthrInsuredPartyID.Format("%li", nOthrInsuredPartyID);
		}
		
		// (a.walling 2007-08-27 11:17) - PLID 27026 - Added send paperwork fields
		// (j.jones 2008-05-14 14:52) - PLID 30044 - added test result fields
		// (j.jones 2009-02-11 10:05) - PLID 33035 - added ManualReview
		// (j.jones 2009-03-10 17:46) - PLID 32864 - added InsuredPartyID and OthrInsuredPartyID
		// (j.jones 2009-09-15 09:13) - PLID 35284 - added ANSI_ClaimTypeCode
		// (j.jones 2010-03-02 13:40) - PLID 37584 - added PriorAuthType
		// (j.jones 2010-06-10 11:17) - PLID 38507 - added HCFABox13Over
		// (j.jones 2012-01-23 16:48) - PLID 47731 - added ConditionDateType
		// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
		// (j.jones 2013-06-07 16:23) - PLID 41479 - added admission & discharge times
		// (j.jones 2013-08-14 12:45) - PLID 57902 - added HCFA Boxes 8, 9b, 9c, 11bQual, 11b
		// (a.walling 2016-03-07 08:56) - PLID 68498 - added UB04ClaimInfo
		// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36
		// (r.gonet 2016-04-07) - NX-100072 - Split the FirstConditionDate into multiple date fields.
		ExecuteSql("UPDATE BillsT SET FormType = %li, ConditionDate = %s, "
			"FirstVisitOrConsultationDate = %s, "
			"InitialTreatmentDate = %s, "
			"LastSeenDate = %s, "
			"AcuteManifestationDate = %s, "
			"LastXRayDate = %s, "
			"HearingAndPrescriptionDate = %s, "
			"AssumedCareDate = %s, "
			"RelinquishedCareDate = %s, "
			"AccidentDate = %s, "
			"NoWorkFrom = %s, NoWorkTo = %s, HospFrom = %s, HospTo = %s, "
			"State = '%s', [MedicaidResubmission] = '%s', [OriginalRefNo] = '%s', [PriorAuthNum] = '%s', [HCFABlock19] = '%s', "
			"HCFABox8 = '%s', HCFABox9b = '%s', HCFABox9c = '%s', "
			"HCFABox10D = '%s', HCFABox11bQual = '%s', HCFABox11b = '%s', UB92Box79 = '%s', "
			"UBBox14 = '%s', UBBox15 = '%s', "
			"OutsideLabCharges = Convert(money, '%s'), SendPaperwork = %li, PaperworkType = '%s', PaperworkTx = '%s', "
			"SendTestResults = %li, TestResultID = '%s', TestResultType = '%s', TestResult = '%s', ManualReview = %li, "
			"InsuredPartyID = %s, OthrInsuredPartyID = %s, ANSI_ClaimTypeCode = %li, PriorAuthType = %li, HCFABox13Over = %li, ConditionDateType = %li, "
			"AdmissionTime = %s, DischargeTime = %s, UB04ClaimInfo = %s "
			"WHERE BillsT.ID = %d",
			nFormType, dtCurrentCondition.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(dtCurrentCondition)) + "'" : "NULL", 
			m_claimDates.dtFirstVisitOrConsultationDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtFirstVisitOrConsultationDate)) + "'" : "NULL",
			m_claimDates.dtInitialTreatmentDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtInitialTreatmentDate)) + "'" : "NULL",
			m_claimDates.dtLastSeenDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtLastSeenDate)) + "'" : "NULL",
			m_claimDates.dtAcuteManifestationDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtAcuteManifestationDate)) + "'" : "NULL",
			m_claimDates.dtLastXRayDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtLastXRayDate)) + "'" : "NULL",
			m_claimDates.dtHearingAndPrescriptionDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtHearingAndPrescriptionDate)) + "'" : "NULL",
			m_claimDates.dtAssumedCareDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtAssumedCareDate)) + "'" : "NULL",
			m_claimDates.dtRelinquishedCareDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtRelinquishedCareDate)) + "'" : "NULL",
			m_claimDates.dtAccidentDate.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(m_claimDates.dtAccidentDate)) + "'" : "NULL",
			dtWorkFrom.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(dtWorkFrom)) + "'" : "NULL", 
			dtWorkTo.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(dtWorkTo)) + "'" : "NULL", 
			dtHospFrom.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(dtHospFrom)) + "'" : "NULL", 
			dtHospTo.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(dtHospTo)) + "'" : "NULL",
			strAutoAccState, _Q(strMedicaidResubmission), _Q(strOriginalRefNo), _Q(strPriorAuthNum), _Q(strHCFABlock19),
			_Q(m_strHCFABox8), _Q(m_strHCFABox9b), _Q(m_strHCFABox9c),
			_Q(m_strHCFABox10d), _Q(m_strHCFABox11bQual), _Q(m_strHCFABox11b), _Q(m_strUB92Box79),
			_Q(strUBBox14), _Q(strUBBox15),
			_Q(strOutsideLabCharges), bSendPaperwork ? 1 : 0, _Q(strPaperworkType), _Q(strPaperworkTx),
			bSendTestResults ? 1 : 0, _Q(strTestResultID), _Q(m_strTestResultType), _Q(strTestResult), bManualReview ? 1 : 0,
			strInsuredPartyID, strOthrInsuredPartyID, (long)eClaimTypeCode, (long)patPriorAuthType, (long)hb13Value, (long)m_claimDates.eConditionDateType,
			dtAdmissionTime.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(dtAdmissionTime)) + "'" : "NULL", 
			dtDischargeTime.GetStatus() == COleDateTime::valid ? "'" + _Q(FormatDateTimeForSql(dtDischargeTime)) + "'" : "NULL",
			m_ub04ClaimInfo.IsEmpty() ? "NULL" : "'" + _Q(m_ub04ClaimInfo.ToXml()) + "'", // (a.walling 2016-03-07 08:56) - PLID 68498 - UB04 Enhancements - Save to database
			GetBillID());

		// (j.jones 2010-08-17 12:03) - PLID 40135 - cache the insured party IDs
		m_nOldInsuranceID1 = nInsuredPartyID;
		m_nOldInsuranceID2 = nOthrInsuredPartyID;

		// (a.walling 2016-03-07 08:53) - PLID 68499 - to detect changes for auditing
		m_ub04ClaimInfoOld = m_ub04ClaimInfo;

		// (j.jones 2013-08-07 12:42) - PLID 58042 - reflect the HCFA fields in the
		// unlikely case that they backdated the charges
		UpdateDisplayedHCFAFields(m_nOldInsuranceID1, GetBillID());

		return SCR_SAVED;

		
	}NxCatchAll("Error 100: CInsuranceBilling::SaveChanges");

	return SCR_ABORT_SAVE;
}

/////////////////////////////////////////////////////////////////////////////
// CInsuranceBilling edit box message handlers


void CInsuranceBilling::ChangeAccess()
{
	// (j.jones 2011-01-21 10:10) - PLID 42156 - the access level is now an enum,
	// but nothing on this screen is blocked by partial access, because partial
	// access only blocks things that could change A/R
	m_pRefList->Enabled = (m_eHasAccess != batNoAccess);
	m_pInsurance1->Enabled = (m_eHasAccess != batNoAccess);
	m_pInsurance2->Enabled = (m_eHasAccess != batNoAccess);
	m_pUB92Box44List->Enabled = (m_eHasAccess != batNoAccess);
	
	GetDlgItem(IDC_EDIT_CURRENT_DATE)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_FIRST_DATE)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_UNABLE_TO_WORK_FROM)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_UNABLE_TO_WORK_TO)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_HOSPITALIZED_FROM)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_HOSPITALIZED_TO)->EnableWindow(m_eHasAccess != batNoAccess);
	// (j.jones 2013-06-07 15:29) - PLID 41479 - added admission & discharge times
	GetDlgItem(IDC_EDIT_ADMISSION_TIME)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_DISCHARGE_TIME)->EnableWindow(m_eHasAccess != batNoAccess);

	GetDlgItem(IDC_OUTSIDE_LAB_CHECK)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_CHARGES)->EnableWindow(m_eHasAccess != batNoAccess);

	GetDlgItem(IDC_RADIO_EMPLOYMENT)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_RADIO_AUTO_ACCIDENT)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_RADIO_OTHER_ACCIDENT)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_RADIO_NONE)->EnableWindow(m_eHasAccess != batNoAccess);

	GetDlgItem(IDC_EDIT_STATE)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_MEDICAID_CODE)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_REFERENCE_NUMBER)->EnableWindow(m_eHasAccess != batNoAccess);
	// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
	m_PriorAuthTypeCombo->Enabled = (m_eHasAccess != batNoAccess) ? VARIANT_TRUE : VARIANT_FALSE;
	// (j.jones 2012-01-23 15:49) - PLID 47731 - added condition date combo
	m_ConditionDateTypeCombo->Enabled = (m_eHasAccess != batNoAccess) ? VARIANT_TRUE : VARIANT_FALSE;
	GetDlgItem(IDC_EDIT_AUTHORIZATION_NUMBER)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_BOX_19)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_BTN_EDIT_PRIOR_AUTH_NUM)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE)->EnableWindow(m_eHasAccess != batNoAccess);
	// (j.jones 2009-02-11 10:06) - PLID 33035 - added ManualReview
	m_checkManualReview.EnableWindow(m_eHasAccess != batNoAccess);

	// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
	m_nxeditEditUBBox14.EnableWindow(m_eHasAccess != batNoAccess);
	m_nxeditEditUBBox15.EnableWindow(m_eHasAccess != batNoAccess);
	
	GetDlgItem(IDC_SELECT_REF_PHYS)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_SELECT_PCP)->EnableWindow(m_eHasAccess != batNoAccess);	

	// (a.walling 2007-08-27 12:14) - PLID 27026
	
	m_pPWKType->Enabled = (m_eHasAccess != batNoAccess) ? VARIANT_TRUE : VARIANT_FALSE;
	m_pPWKTx->Enabled = (m_eHasAccess != batNoAccess) ? VARIANT_TRUE : VARIANT_FALSE;
	EnableDlgItem(IDC_SEND_PAPERWORK_CHECK, (m_eHasAccess != batNoAccess));
	if(m_eHasAccess != batNoAccess) {
		OnSendPaperworkCheck(); // enable appropriate dialog items
	}

	// (j.jones 2008-05-14 14:15) - PLID 30044 - added test result controls
	EnableDlgItem(IDC_SEND_TEST_RESULT_CHECK, (m_eHasAccess != batNoAccess));	
	m_TestResultIDCombo->Enabled = (m_eHasAccess != batNoAccess) ? VARIANT_TRUE : VARIANT_FALSE;
	m_TestResultTypeCombo->Enabled = (m_eHasAccess != batNoAccess) ? VARIANT_TRUE : VARIANT_FALSE;
	EnableDlgItem(IDC_EDIT_TEST_RESULT, (m_eHasAccess != batNoAccess));
	if(m_eHasAccess != batNoAccess) {
		PostSendTestResultsCheck();
	}

	// (j.jones 2009-09-14 17:32) - PLID 35284 - added claim type combo
	m_ClaimTypeCombo->Enabled = (m_eHasAccess != batNoAccess) ? VARIANT_TRUE : VARIANT_FALSE;

	// (j.jones 2010-06-10 10:32) - PLID 38507 - added HCFA Box 13 config
	m_HCFABox13Combo->Enabled = (m_eHasAccess != batNoAccess) ? VARIANT_TRUE : VARIANT_FALSE;
}

long CInsuranceBilling::GetBillID()
{
	return ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetBillID();
}

void CInsuranceBilling::SetBillID(long BillID)
{
	((CBillingModuleDlg*)m_pBillingModuleWnd)->SetBillID(BillID);
}

BOOL CInsuranceBilling::PreTranslateMessage(MSG* pMsg) 
{
	BOOL IsShiftKeyDown = GetAsyncKeyState(VK_SHIFT);

	try {

		if (pMsg->message == WM_SYSKEYDOWN) {
			switch (pMsg->wParam) {
			case 'O': ((CBillingModuleDlg*)m_pBillingModuleWnd)->OnSave(); return TRUE;
			case 'C':
			case 'X': ((CBillingModuleDlg*)m_pBillingModuleWnd)->OnCancel(); return TRUE;
			case 'D': ((CBillingModuleDlg*)m_pBillingModuleWnd)->OnBillDeleteBtn(); return TRUE;
			case 'E': ((CBillingModuleDlg*)m_pBillingModuleWnd)->OnBillEditBtn(); return TRUE;
			case 'P': ((CBillingModuleDlg*)m_pBillingModuleWnd)->OnBillPreviewBtn(); return TRUE;
			case 'B': {
				((CBillingModuleDlg*)m_pBillingModuleWnd)->SetActiveTab(0);
				return TRUE;
			}
			case 'A': {
				//billing 2 dlg
				((CBillingModuleDlg*)m_pBillingModuleWnd)->SetActiveTab(2);
				return TRUE;
			}
			case 'F': OnOpenForm(); return TRUE;
			case 'N': OnPrintForm(); return TRUE;
			case 'M': GetDlgItem(IDC_FORM_TYPE_COMBO)->SetFocus(); return TRUE;
			case 'S': GetDlgItem(IDC_EDIT_MEDICAID_CODE)->SetFocus(); return TRUE;
			case 'H': GetDlgItem(IDC_EDIT_BOX_19)->SetFocus(); return TRUE;
			}
		}
		
	////////////////////////////////////////////////////
	// When a key is first pressed
	////////////////////////////////////////////////////
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
			// Shift is down
			if (IsShiftKeyDown) {
				if (GetFocus()/*->GetParent()*/ == GetDlgItem(IDC_FORM_TYPE_COMBO)) {
					GetParent()->SetFocus();
					m_peditBillDescription->SetFocus();
					return TRUE;
				}
			}
			else {
				if(GetFocus() == &m_editBox19) {
					GetParent()->SetFocus();
					m_peditBillDate->SetFocus();
					return TRUE;
				}
			}
		}
	}NxCatchAll("Error triggering billing hotkey.");

	return CNxDialog::PreTranslateMessage(pMsg);
}

void CInsuranceBilling::OnDestroy() 
{
	m_boInitialized = FALSE;
	m_HasHCFABeenOpened = FALSE;
	// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
	m_eHasAccess = batNoAccess;

	CNxDialog::OnDestroy();
}

BEGIN_EVENTSINK_MAP(CInsuranceBilling, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CInsuranceBilling)
	ON_EVENT(CInsuranceBilling, IDC_INSPLAN1, 18 /* RequeryFinished */, OnRequeryFinishedInsplan1, VTS_I2)
	ON_EVENT(CInsuranceBilling, IDC_INSPLAN2, 18 /* RequeryFinished */, OnRequeryFinishedInsplan2, VTS_I2)
	ON_EVENT(CInsuranceBilling, IDC_FORM_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenFormTypeCombo, VTS_I4)
	ON_EVENT(CInsuranceBilling, IDC_INSPLAN1, 16 /* SelChosen */, OnSelChosenInsplan1, VTS_I4)
	ON_EVENT(CInsuranceBilling, IDC_EDIT_CURRENT_DATE, 1 /* KillFocus */, OnKillFocusEditCurrentDate, VTS_NONE)
	ON_EVENT(CInsuranceBilling, IDC_EDIT_FIRST_DATE, 1 /* KillFocus */, OnKillFocusEditFirstDate, VTS_NONE)
	ON_EVENT(CInsuranceBilling, IDC_EDIT_UNABLE_TO_WORK_FROM, 1 /* KillFocus */, OnKillFocusEditUnableToWorkFrom, VTS_NONE)
	ON_EVENT(CInsuranceBilling, IDC_EDIT_UNABLE_TO_WORK_TO, 1 /* KillFocus */, OnKillFocusEditUnableToWorkTo, VTS_NONE)
	ON_EVENT(CInsuranceBilling, IDC_EDIT_HOSPITALIZED_FROM, 1 /* KillFocus */, OnKillFocusEditHospitalizedFrom, VTS_NONE)
	ON_EVENT(CInsuranceBilling, IDC_EDIT_HOSPITALIZED_TO, 1 /* KillFocus */, OnKillFocusEditHospitalizedTo, VTS_NONE)
	ON_EVENT(CInsuranceBilling, IDC_REF_PHYS, 20 /* TrySetSelFinished */, OnTrySetSelFinishedRefPhys, VTS_I4 VTS_I4)
	ON_EVENT(CInsuranceBilling, IDC_UB92_BOX_44_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedUb92Box44List, VTS_I4 VTS_I4)
	ON_EVENT(CInsuranceBilling, IDC_CLAIM_TYPE_COMBO, 16, OnSelChosenClaimTypeCombo, VTS_DISPATCH)
	ON_EVENT(CInsuranceBilling, IDC_PRIOR_AUTH_TYPE_COMBO, 16, OnSelChosenPriorAuthTypeCombo, VTS_DISPATCH)
	ON_EVENT(CInsuranceBilling, IDC_HCFA_BOX_13_COMBO, 16, OnSelChosenHCFABox13Combo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP	
	ON_EVENT(CInsuranceBilling, IDC_CONDITION_DATE_COMBO, 1, OnSelChangingConditionDateCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInsuranceBilling, IDC_CONDITION_DATE_COMBO, 16, OnSelChosenConditionDateCombo, VTS_DISPATCH)
	ON_EVENT(CInsuranceBilling, IDC_EDIT_ADMISSION_TIME, 1, OnKillFocusEditAdmissionTime, VTS_NONE)
	ON_EVENT(CInsuranceBilling, IDC_EDIT_DISCHARGE_TIME, 1, OnKillFocusEditDischargeTime, VTS_NONE)
END_EVENTSINK_MAP()

void CInsuranceBilling::OnClickBtnFormProperties() 
{
	CHcfaSetup hcfasetup(this);
	//CUB92Setup ub92setup;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) return;

	if(m_FormTypeCombo->GetCurSel() == -1)
		return;
	
	long nFormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

	switch (nFormType) {
	case 1:	// HCFA
		break;

	case 2: // UB-92		
		break;

	case 3: // ADA		
		break;

	case 4: // IDPA
		break;

	case 5: // NYWC
		break;

	case 6: // MICR
		break;

	case 7: //NY Medicaid
		break;
	}
}

void CInsuranceBilling::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	SetControlPositions();
	Invalidate();
}

BOOL CInsuranceBilling::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_openFormButton.SetTextColor(0x00004000);
	m_printFormButton.SetTextColor(0x00800080);

	m_brush.CreateSolidBrush(PaletteColor(0x00FFC0C0));

	try{
		m_pWorkFrom = GetDlgItemUnknown(IDC_EDIT_UNABLE_TO_WORK_FROM);
		m_pWorkTo = GetDlgItemUnknown(IDC_EDIT_UNABLE_TO_WORK_TO);
		m_pHospFrom = GetDlgItemUnknown(IDC_EDIT_HOSPITALIZED_FROM);
		m_pHospTo = GetDlgItemUnknown(IDC_EDIT_HOSPITALIZED_TO);
		m_pAccident = GetDlgItemUnknown(IDC_EDIT_CURRENT_DATE);
		m_pIllness = GetDlgItemUnknown(IDC_EDIT_FIRST_DATE);
		// (j.jones 2013-06-07 15:29) - PLID 41479 - added admission & discharge times
		m_pAdmissionTime = GetDlgItemUnknown(IDC_EDIT_ADMISSION_TIME);
		m_pDischargeTime = GetDlgItemUnknown(IDC_EDIT_DISCHARGE_TIME);

		CString str;
		m_pRefList = BindNxDataListCtrl(IDC_REF_PHYS);
		m_pInsurance1 = BindNxDataListCtrl(IDC_INSPLAN1, false);
		m_pInsurance2 = BindNxDataListCtrl(IDC_INSPLAN2, false);
		m_FormTypeCombo = BindNxDataListCtrl(IDC_FORM_TYPE_COMBO, false);

		m_pUB92Box44List = BindNxDataListCtrl(IDC_UB92_BOX_44_LIST, true);

		IRowSettingsPtr pRow;
		pRow = m_pUB92Box44List->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<None>");
		pRow->PutValue(2,"");
		pRow->PutValue(3,"<No Code Selected>");
		m_pUB92Box44List->InsertRow(pRow,0);

		pRow = m_pRefList->GetRow(-1);
		// (j.jones 2011-12-01 14:13) - PLID 46771 - this now uses the enums
		pRow->PutValue(rpcID, g_cvarNull);
		pRow->PutValue(rpcName, "<No Physician Selected>");
		pRow->PutValue(rpcNpi, "");
		pRow->PutValue(rpcRefPhysID, "");
		pRow->PutValue(rpcUpin, "");
		pRow->PutValue(rpcAddress1, "");
		pRow->PutValue(rpcAddress2, "");
		pRow->PutValue(rpcCity, "");
		pRow->PutValue(rpcState, "");
		pRow->PutValue(rpcZip, "");
		m_pRefList->InsertRow(pRow,0);

		pRow = m_FormTypeCombo->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,"HCFA");
		m_FormTypeCombo->AddRow(pRow);
		pRow = m_FormTypeCombo->GetRow(-1);
		pRow->PutValue(0,(long)2);
		//TES 3/13/2007 - PLID 24993 - Reflect the form type (this specifies the type, rather than just saying "UB", because
		// the user needs to know the type in order to interpret the "Box nn" labels on screen).
		CString strFormType = GetUBFormType() == 1 ? "UB04" : "UB92";
		pRow->PutValue(1,_bstr_t(strFormType));
		m_FormTypeCombo->AddRow(pRow);

		//TODO - If we make a Dental option in the license, have this use it.
		pRow = m_FormTypeCombo->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,"ADA");
		m_FormTypeCombo->AddRow(pRow);

		pRow = m_FormTypeCombo->GetRow(-1);
		pRow->PutValue(0,(long)4);
		pRow->PutValue(1,"IDPA");
		m_FormTypeCombo->AddRow(pRow);

		pRow = m_FormTypeCombo->GetRow(-1);
		pRow->PutValue(0,(long)5);
		pRow->PutValue(1,"NYWC");
		m_FormTypeCombo->AddRow(pRow);

		pRow = m_FormTypeCombo->GetRow(-1);
		pRow->PutValue(0,(long)6);
		pRow->PutValue(1,"MICR");
		m_FormTypeCombo->AddRow(pRow);

		// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
		pRow = m_FormTypeCombo->GetRow(-1);
		pRow->PutValue(0,(long)7);
		pRow->PutValue(1,"NY Medicaid");
		m_FormTypeCombo->AddRow(pRow);					

		// (a.walling 2007-08-27 11:02) - PLID 27026 - Setup the datalist for sending paperwork
		m_pPWKType = BindNxDataList2Ctrl(IDC_PWK_TYPE, GetRemoteData(), false);
		m_pPWKTx = BindNxDataList2Ctrl(IDC_PWK_TX, GetRemoteData(), false);

		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPWKTx->GetNewRow();
			pRow->PutValue(0, "AA");
			pRow->PutValue(1, "Available on Request at Provider Site");
			m_pPWKTx->AddRowSorted(pRow, NULL);

			pRow = m_pPWKTx->GetNewRow();
			pRow->PutValue(0, "BM");
			pRow->PutValue(1, "By Mail");
			m_pPWKTx->AddRowSorted(pRow, NULL);

			pRow = m_pPWKTx->GetNewRow();
			pRow->PutValue(0, "EL");
			pRow->PutValue(1, "Electronically Only");
			m_pPWKTx->AddRowSorted(pRow, NULL);

			pRow = m_pPWKTx->GetNewRow();
			pRow->PutValue(0, "EM");
			pRow->PutValue(1, "E-Mail");
			m_pPWKTx->AddRowSorted(pRow, NULL);

			// (j.jones 2010-10-15 16:28) - PLID 32848 - FT is new for 5010
			//(if they use it on 4010, they may get rejected)
			pRow = m_pPWKTx->GetNewRow();
			pRow->PutValue(0, "FT");
			pRow->PutValue(1, "File Transfer");
			m_pPWKTx->AddRowSorted(pRow, NULL);

			pRow = m_pPWKTx->GetNewRow();
			pRow->PutValue(0, "FX");
			pRow->PutValue(1, "By Fax");
			m_pPWKTx->AddRowSorted(pRow, NULL);
		}

		{
			// (j.jones 2010-10-15 16:28) - PLID 32848 - the list of options expanded in 5010,
			// in the extremely unlikely chance that someone chose to use a new option and
			// export in 4010, if the insurance company doesn't accept it, they will just get rejected
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "03");
			pRow->PutValue(1, "Report Justifying Treatment Beyond Utilization Guidelines");
			m_pPWKType->AddRowSorted(pRow, NULL);				
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "04");
			pRow->PutValue(1, "Drugs Administered");
			m_pPWKType->AddRowSorted(pRow, NULL);			
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "05");
			pRow->PutValue(1, "Treatment Diagnosis");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "06");
			pRow->PutValue(1, "Initial Assessment");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "07");
			pRow->PutValue(1, "Functional Goals");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "08");
			pRow->PutValue(1, "Plan of Treatment");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "09");
			pRow->PutValue(1, "Progress Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "10");
			pRow->PutValue(1, "Continued Treatment");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "11");
			pRow->PutValue(1, "Chemical Analysis");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "13");
			pRow->PutValue(1, "Certified Test Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "15");
			pRow->PutValue(1, "Justification for Admission");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "21");
			pRow->PutValue(1, "Recovery Plan");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "A3");
			pRow->PutValue(1, "Allergies/Sensitivities Document");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "A4");
			pRow->PutValue(1, "Autopsy Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "AM");
			pRow->PutValue(1, "Ambulance Certification");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "AS");
			pRow->PutValue(1, "Admission Summary");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "B2");
			pRow->PutValue(1, "Prescription");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "B3");
			pRow->PutValue(1, "Physician Order");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "B4");
			pRow->PutValue(1, "Referral Form");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "BR");
			pRow->PutValue(1, "Benchmark Testing Results");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "BS");
			pRow->PutValue(1, "Baseline");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "BT");
			pRow->PutValue(1, "Blanket Test Results");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "CB");
			pRow->PutValue(1, "Chiropractic Justification");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "CK");
			pRow->PutValue(1, "Consent Form(s)");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "CT");
			pRow->PutValue(1, "Certification");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "D2");
			pRow->PutValue(1, "Drug Profile Document");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "DA");
			pRow->PutValue(1, "Dental Models");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "DB");
			pRow->PutValue(1, "Durable Medical Equipment Prescription");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "DG");
			pRow->PutValue(1, "Diagnostic Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "DJ");
			pRow->PutValue(1, "Discharge Monitoring Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "DS");
			pRow->PutValue(1, "Discharge Summary");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "EB");
			pRow->PutValue(1, "Explanation of Benefits (Coordination of Benefits or Medicare Secondary Payor)");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "HC");
			pRow->PutValue(1, "Health Certificate");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "HR");
			pRow->PutValue(1, "Health Clinic Records");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "I5");
			pRow->PutValue(1, "Immunization Record");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "IR");
			pRow->PutValue(1, "State School Immunization Records");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "LA");
			pRow->PutValue(1, "Laboratory Results");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "M1");
			pRow->PutValue(1, "Medical Record Attachment");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "MT");
			pRow->PutValue(1, "Models");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "NN");
			pRow->PutValue(1, "Nursing Notes");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "OB");
			pRow->PutValue(1, "Operative Note");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "OC");
			pRow->PutValue(1, "Oxygen Content Averaging Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "OD");
			pRow->PutValue(1, "Orders and Treatments Document");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "OE");
			pRow->PutValue(1, "Objective Physical Examination (including vital signs) Document");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "OX");
			pRow->PutValue(1, "Oxygen Therapy Certification");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "OZ");
			pRow->PutValue(1, "Support Data for Claim");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "P4");
			pRow->PutValue(1, "Pathology Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "P5");
			pRow->PutValue(1, "Patient Medical History Document");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "PE");
			pRow->PutValue(1, "Parenteral or Enteral Certification");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "PN");
			pRow->PutValue(1, "Physical Therapy Notes");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "PO");
			pRow->PutValue(1, "Prosthetics or Orthotic Certification");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "PQ");
			pRow->PutValue(1, "Paramedical Results");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "PY");
			pRow->PutValue(1, "Physician’s Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "PZ");
			pRow->PutValue(1, "Physical Therapy Certification");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "RB");
			pRow->PutValue(1, "Radiology Films");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "RR");
			pRow->PutValue(1, "Radiology Reports");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "RT");
			pRow->PutValue(1, "Report of Tests and Analysis Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "RX");
			pRow->PutValue(1, "Renewable Oxygen Content Averaging Report");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "SG");
			pRow->PutValue(1, "Symptoms Document");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "V5");
			pRow->PutValue(1, "Death Notification");
			m_pPWKType->AddRowSorted(pRow, NULL);
			pRow = m_pPWKType->GetNewRow();
			pRow->PutValue(0, "XP");
			pRow->PutValue(1, "Photographs");
			m_pPWKType->AddRowSorted(pRow, NULL);
		}

		// (j.jones 2008-05-14 12:12) - PLID 30044 - added test results controls
		m_TestResultIDCombo = BindNxDataList2Ctrl(IDC_TEST_RESULT_ID_COMBO, false);
		m_TestResultTypeCombo = BindNxDataList2Ctrl(IDC_TEST_RESULT_TYPE_COMBO, false);

		//add the test result IDs and Types
		m_strTestResultType = "";
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_TestResultIDCombo->GetNewRow();
			pRow->PutValue(trilCode, "OG");
			pRow->PutValue(trilDescription, "Original Starting Dosage");
			m_TestResultIDCombo->AddRowSorted(pRow, NULL);

			pRow = m_TestResultIDCombo->GetNewRow();
			pRow->PutValue(trilCode, "TR");
			pRow->PutValue(trilDescription, "Test Results");
			m_TestResultIDCombo->AddRowSorted(pRow, NULL);
		}

		{
			// (j.jones 2010-10-22 09:50) - PLID 40962 - removed Concentration, Gas Test Rate, and Oxygen,
			// as they are obsolete now
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_TestResultTypeCombo->GetNewRow();
			pRow->PutValue(trtlCode, "HT");
			pRow->PutValue(trtlDescription, "Height");
			m_TestResultTypeCombo->AddRowSorted(pRow, NULL);

			pRow = m_TestResultTypeCombo->GetNewRow();
			pRow->PutValue(trtlCode, "R1");
			pRow->PutValue(trtlDescription, "Hemoglobin");
			m_TestResultTypeCombo->AddRowSorted(pRow, NULL);

			pRow = m_TestResultTypeCombo->GetNewRow();
			pRow->PutValue(trtlCode, "R2");
			pRow->PutValue(trtlDescription, "Hematocrit");
			m_TestResultTypeCombo->AddRowSorted(pRow, NULL);

			pRow = m_TestResultTypeCombo->GetNewRow();
			pRow->PutValue(trtlCode, "R3");
			pRow->PutValue(trtlDescription, "Epoetin Starting Dosage");
			m_TestResultTypeCombo->AddRowSorted(pRow, NULL);

			pRow = m_TestResultTypeCombo->GetNewRow();
			pRow->PutValue(trtlCode, "R4");
			pRow->PutValue(trtlDescription, "Creatinine");	//yes, Creatinine, not a typo
			m_TestResultTypeCombo->AddRowSorted(pRow, NULL);
		}

		//enforce a 20-character limit on the result text
		m_editTestResults.SetLimitText(20);

		// (j.jones 2011-09-16 15:09) - PLID 40530 - this previously had no limit, but was 60 in data,
		// I increased this to 80, and added a limiter
		m_editBox19.SetLimitText(80);

		// (j.jones 2012-05-14 15:10) - PLID 47650 - added UB Box 14/15
		m_nxeditEditUBBox14.SetLimitText(20);
		m_nxeditEditUBBox15.SetLimitText(20);

		// (j.jones 2009-09-14 17:32) - PLID 35284 - added claim type combo
		m_ClaimTypeCombo = BindNxDataList2Ctrl(IDC_CLAIM_TYPE_COMBO, false);

		// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
		m_PriorAuthTypeCombo = BindNxDataList2Ctrl(IDC_PRIOR_AUTH_TYPE_COMBO, false);

		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_PriorAuthTypeCombo->GetNewRow();
			pRow->PutValue(patccID, (long)patPriorAuthNumber);
			pRow->PutValue(patccDesc, "Prior Authorization Number");
			m_PriorAuthTypeCombo->AddRowAtEnd(pRow, NULL);

			pRow = m_PriorAuthTypeCombo->GetNewRow();
			pRow->PutValue(patccID, (long)patReferralNumber);
			pRow->PutValue(patccDesc, "Referral Number");
			m_PriorAuthTypeCombo->AddRowAtEnd(pRow, NULL);
		}

		// (j.jones 2012-01-23 15:49) - PLID 47731 - added condition date combo
		m_ConditionDateTypeCombo = BindNxDataList2Ctrl(IDC_CONDITION_DATE_COMBO, false);

		// (j.jones 2013-08-16 09:29) - PLID 58063 - changed the enums and added a qualifier
		// column, and the first enum now represents First Visit Or Consultation
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtFirstVisitOrConsultation444);
			pRow->PutValue(cdtccDesc, "First Visit or Consultation");
			pRow->PutValue(cdtccQualifier, "444");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);

			pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtInitialTreatmentDate454);
			pRow->PutValue(cdtccDesc, "Initial Treatment Date");
			pRow->PutValue(cdtccQualifier, "454");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);

			// (j.jones 2013-08-06 16:42) - PLID 57895 - supported last seen date
			pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtLastSeenDate304);
			pRow->PutValue(cdtccDesc, "Last Seen Date / Latest Visit");
			pRow->PutValue(cdtccQualifier, "304");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);

			// (j.jones 2013-08-16 13:46) - PLID 58069 - added 453, 455, 471, 090, 091
			pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtAcuteManifestation453);
			pRow->PutValue(cdtccDesc, "Acute Manifestation of a Chronic Condition");
			pRow->PutValue(cdtccQualifier, "453");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);

			pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtLastXray455);
			pRow->PutValue(cdtccDesc, "Last X-Ray Date");
			pRow->PutValue(cdtccQualifier, "455");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);

			pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtHearingAndPrescription471);
			pRow->PutValue(cdtccDesc, "Hearing And Prescription Date");
			pRow->PutValue(cdtccQualifier, "471");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);

			pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtAssumedCare090);
			pRow->PutValue(cdtccDesc, "Assumed Care Date");
			pRow->PutValue(cdtccQualifier, "090");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);

			pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtRelinquishedCare91);
			pRow->PutValue(cdtccDesc, "Relinquished Care Date");
			pRow->PutValue(cdtccQualifier, "091");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);

			// (j.jones 2014-07-17 11:39) - PLID 62713 - added 439 - accident
			pRow = m_ConditionDateTypeCombo->GetNewRow();
			pRow->PutValue(cdtccID, (long)cdtAccident439);
			pRow->PutValue(cdtccDesc, "Accident Date");
			pRow->PutValue(cdtccQualifier, "439");
			m_ConditionDateTypeCombo->AddRowAtEnd(pRow, NULL);
		}

		// (j.jones 2010-06-10 09:49) - PLID 38507 - added HCFA Box 13 config
		m_HCFABox13Combo = BindNxDataList2Ctrl(IDC_HCFA_BOX_13_COMBO, false);

		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_HCFABox13Combo->GetNewRow();
			pRow->PutValue(hb13ccID, (long)hb13_UseDefault);
			pRow->PutValue(hb13ccDesc, "<Use Default Assignment Of Benefits Value>");
			m_HCFABox13Combo->AddRowAtEnd(pRow, NULL);

			// (j.jones 2010-07-27 11:01) - PLID 39858 - reworded to reflect that this also affects UB claims

			pRow = m_HCFABox13Combo->GetNewRow();
			pRow->PutValue(hb13ccID, (long)hb13_No);
			pRow->PutValue(hb13ccDesc, "Do Not Assign Benefits To Provider (Do Not Fill HCFA Box 13 / UB Box 53)");
			m_HCFABox13Combo->AddRowAtEnd(pRow, NULL);

			pRow = m_HCFABox13Combo->GetNewRow();
			pRow->PutValue(hb13ccID, (long)hb13_Yes);
			pRow->PutValue(hb13ccDesc, "Assign Benefits To Provider (Fill HCFA Box 13 / UB Box 53)");
			m_HCFABox13Combo->AddRowAtEnd(pRow, NULL);
		}

	}NxCatchAll("Error 100: CInsuranceBilling::OnInitDialog");

	return TRUE; 
}

void CInsuranceBilling::OnRequeryFinishedInsplan1(short nFlags) 
{
	IRowSettingsPtr pRow;
	int nType;

	for(int i = 0; i < m_pInsurance1->GetRowCount(); i++){
		// (j.jones 2010-08-17 11:29) - PLID 40135 - corrected to use the priority
		nType = VarLong(m_pInsurance1->GetValue(i, ipccRespTypePriority), -1); 
		if(nType == 1) {
			pRow = m_pInsurance1->GetRow(i);
			pRow->PutForeColor(RGB(192,0,0));
		}
		else if (nType == 2) {
			pRow = m_pInsurance1->GetRow(i);
			pRow->PutForeColor(RGB(0,0,128));
		}
	}
}

void CInsuranceBilling::OnRequeryFinishedInsplan2(short nFlags) 
{
	IRowSettingsPtr pRow;
	int nType;

	for(int i = 0; i < m_pInsurance2->GetRowCount(); i++){
		// (j.jones 2010-08-17 11:29) - PLID 40135 - corrected to use the priority
		nType = VarLong(m_pInsurance2->GetValue(i, ipccRespTypePriority), -1);
		if(nType == 1) {
			pRow = m_pInsurance2->GetRow(i);
			pRow->PutForeColor(RGB(192,0,0));
		}
		else if(nType == 2) {
			pRow = m_pInsurance2->GetRow(i);
			pRow->PutForeColor(RGB(0,0,128));
		}
	}
}

void CInsuranceBilling::OnOpenForm() 
{
	if (!g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrUse))
	{	MsgBox("Your license indicates you didn't purchase insurance billing,\n"
		"please call Nextech if you wish to use this feature.");
		return;
	}

	if (!CheckCurrentUserPermissions(bioClaimForms,sptRead))
		return;

	CBillingModuleDlg* pBillingWnd = (CBillingModuleDlg*)m_pBillingModuleWnd;
//	CPatientSet* pPatientSet = ((CMainFrame *)AfxGetMainWnd())->GetPatientSet();
	int iPatientID = m_nPatientID;
	CString str;

	////////////////////////////////////////////////////////////////////////////////
	// Save the bill and insurance changes
	////////////////////////////////////////////////////////////////////////////////
	// TODO: Toggle with the financial admin tab
	//pBillingWnd->m_boAskForNewPayment = FALSE;

	if (pBillingWnd->m_dlgBilling.m_pList->GetRowCount() == 0) {
		MsgBox("The bill must have at least one charge before a claim form can be made");
		return;
	}

	if (m_pInsurance1->CurSel == -1 || m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID).vt == VT_NULL) {
		if(m_GuarantorID1 != -1 && m_pInsurance1->FindByColumn(ipccID, m_GuarantorID1, 0, FALSE) != -1) {
			if(IDNO == MessageBox("You must have an insurance company selected before opening a form.\n"
				"Would you like to auto-select the patient's primary insurance?\n\n"
				"(If 'No', then you will need to select an insurance company before you can open a form.)", "Practice", MB_YESNO|MB_ICONINFORMATION)) {
				return;
			}
			else {
				m_pInsurance1->SetSelByColumn(ipccID, m_GuarantorID1);

				// (j.jones 2013-08-07 12:04) - PLID 58042 - if a HCFA, and a company is selected,
				//toggle the fields that need to change for the new/old HCFA form
				UpdateDisplayedHCFAFields(m_GuarantorID1, GetBillID());

				// (j.jones 2014-05-29 10:56) - PLID 61837 - also update the charge provider
				// columns back on the bill tab
				((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.TryShowAllChargeProviderColumns();

				// (s.tullis 2016-02-24 16:34) - PLID 68319
				((CBillingModuleDlg*)m_pBillingModuleWnd)->UpdateClaimFormSelection();
			}
		}
		else {
			MsgBox("Please select a primary insurance company before opening a form");
			return;
		}		
	}

	if(IDNO == DontShowMeAgain(this, "This action will first save all changes you've made to this bill.\n"
		"Clicking 'Cancel' on the bill later will not undo your changes.\n"
		"Are you sure you wish to continue?", "BillingDlgOpenClaimForm", "Practice", FALSE, TRUE)) {
		return;
	}

	CWaitCursor pWait;

	// (d.thompson 2009-09-02) - PLID 34694 - Saving is required before previewing the form, but in some cases, 
	//	like when the "don't allow past editing" feature is on, there are no changes to save.  Since we have
	//	no reliable way of detecting changes, we can test specifically for the failure to edit to begin with.
	// (j.jones 2010-02-04 17:25) - PLID 37212 - This function inexplicably checked the bill deletion permission,
	// so anytime this function was called, it would return FALSE if you could not delete a bill, which we could
	// care less about. It now checks the ability to write to a bill. So the more correct solution would be to check:
	// - Do they have the ability to save changes to a bill?
	//		- If so, call this function. If it prompts for a password, then so be it. A little confusing, but accurate.
	//		- It not, don't call the historic financial stuff
	//Technically, you cannot get the bill into a modified state without already having write permissions,
	//and many other actions (preview, clicking OK) will call Save() without permission checks. And nor should they.
	//So we're just going to save always, no matter what. (The commented-out code has been modified to not check
	//for historic permission at all if you don't have write access, but none of it is necessary.)
	/*
	if(GetCurrentUserPermissions(bioBill) & (sptWrite|sptWriteWithPass)) {
		if(!pBillingWnd->IsBillEditable_HistoricFinancial(TRUE)) {
			//It is not possible for the user to edit this particular bill, so there's no need to try to save (which will fail)
		}
		else {
			//You can edit (for this reason, may be prevented for others), so try to save any possible changes.
			if(!pBillingWnd->Save())
				return;
		}
	}
	*/

	//always save, they should not have been allowed to make changes without permission
	if(!pBillingWnd->Save())
		return;

	// (j.jones 2008-02-11 14:43) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
	// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
	if(!CanCreateInsuranceClaim(GetBillID(), FALSE)) {
		return;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Open the form
	////////////////////////////////////////////////////////////////////////////////
	if(m_FormTypeCombo->GetCurSel() == -1)
		return;
	
	long nFormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;
	switch(nFormType) {
	case 1: {	// Open HCFA
		
			CHCFADlg hcfa(this);
			_variant_t var;

			long nInsuranceID = VarLong(m_pInsurance1->GetValue(m_pInsurance1->GetCurSel(), ipccID));

			// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening HCFAs without a HCFA group
			long nHCFAGroupID = -1;
			if(nInsuranceID != -1) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.HCFASetupGroupID FROM InsuranceCoT "
					"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"WHERE InsuredPartyT.PersonID = {INT}", nInsuranceID);
				if(!rs->eof) {
					nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);
				}
				rs->Close();
			}

			if(nHCFAGroupID == -1) {
				MsgBox("The selected insurance company is not configured in a HCFA group.\n"
					"You can set up a HCFA group in the HCFA tab of the Administrator module.\n\n"
					"This HCFA cannot be opened until the insurance company has been properly set up.");
				return;
			}

			hcfa.m_PatientID = m_nPatientID;
			hcfa.DoModal(GetBillID());
		}
		m_HasHCFABeenOpened = TRUE;
		break;
	case 2:
		{
			long nInsuranceID = VarLong(m_pInsurance1->GetValue(m_pInsurance1->GetCurSel(), ipccID));

			// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening UBs without a UB group
			long nUBGroupID = -1;
			if(nInsuranceID != -1) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.UB92SetupGroupID FROM InsuranceCoT "
					"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"WHERE InsuredPartyT.PersonID = {INT}", nInsuranceID);
				if(!rs->eof) {
					nUBGroupID = AdoFldLong(rs, "UB92SetupGroupID", -1);
				}
				rs->Close();
			}

			if(nUBGroupID == -1) {
				MsgBox("The selected insurance company is not configured in a UB group.\n"
					"You can set up a UB group in the UB tab of the Administrator module.\n\n"
					"This claim cannot be opened until the insurance company has been properly set up.");
				return;
			}

			//TES 3/13/2007 - PLID 24993 - Open the correct form
			if(GetUBFormType() == 0) {
				CUB92Dlg ub92(this);
				ub92.m_PatientID = m_nPatientID;
				ub92.DoModal(GetBillID());
			}
			else if(GetUBFormType() == 1) {
				CUB04Dlg ub04(this);
				ub04.m_PatientID = m_nPatientID;
				ub04.DoModal(GetBillID());
			}
			else {
				AfxThrowNxException("Invalid UB Form Type %i found!", GetUBFormType());
			}
		}
		break;
	case 3:
		{
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Fill BillID, PatientID in constructor
			CADADlg ada(this, GetBillID(), m_nPatientID);
			ada.DoModal();
		}
		break;
	case 4:
		{
			CIDPADlg idpa(this);
			idpa.m_PatientID = m_nPatientID;
			idpa.DoModal(GetBillID());
		}
		break;
	case 5:
		{
			CNYWCDlg nywc(this);
			nywc.m_PatientID = m_nPatientID;
			nywc.DoModal(GetBillID());
		}
		break;
	case 6:
		{
			// (j.jones 2007-05-09 10:27) - PLID 25550 - check the internal preference
			// for which MICR form to use

			if(GetRemotePropertyInt("Use2007MICR", 1, 0, "<None>", true) == 1) {

				CMICR2007Dlg micr(this);
				micr.m_PatientID = m_nPatientID;
				micr.DoModal(GetBillID());
			}
			else {

				CMICRDlg micr(this);
				micr.m_PatientID = m_nPatientID;
				micr.DoModal(GetBillID());
			}
		}
		break;

	case 7:
		{
			// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
			CNYMedicaidDlg nymcaid(this);
			nymcaid.m_PatientID = m_nPatientID;
			nymcaid.DoModal(GetBillID());
			break;
		}
	}

	////////////////////////////////////////////////////
	// Load batch status
	{
		int nBatch = FindHCFABatch(GetBillID());

		if (nBatch == 1) {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(TRUE);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(FALSE);
		}
		else if (nBatch == 2) {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(TRUE);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(FALSE);
		}
		else {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);
		}
	}
}

// (z.manning, 05/16/2008) - PLID 30050 - There were a bunch of "CDialog dlgWait" declarations in this function, but
// the dialogs were never used, so I removed the declarations.
void CInsuranceBilling::OnPrintForm() 
{
	if (!g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrUse))
	{	MsgBox("Your license indicates you didn't purchase insurance billing,\n"
		"please call Nextech if you wish to use this feature.");
		return;
	}

	if (!CheckCurrentUserPermissions(bioClaimForms,sptRead))
		return;

	CBillingModuleDlg* pBillingWnd = (CBillingModuleDlg*)m_pBillingModuleWnd;
//	CPatientSet* pPatientSet = ((CMainFrame *)AfxGetMainWnd())->GetPatientSet();
	int iPatientID = m_nPatientID;
	CString str;

	////////////////////////////////////////////////////////////////////////////////
	// Save the bill and insurance changes
	////////////////////////////////////////////////////////////////////////////////
	// TODO: Toggle with the financial admin tab
	//pBillingWnd->m_boAskForNewPayment = FALSE;

	if (pBillingWnd->m_dlgBilling.m_pList->GetRowCount() == 0) {
		MsgBox("The bill must have at least one charge before a claim form can be made");
		return;
	}

	if (m_pInsurance1->CurSel == -1 || m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID).vt == VT_NULL) {
		if(m_GuarantorID1 != -1 && m_pInsurance1->FindByColumn(ipccID, m_GuarantorID1, 0, FALSE) != -1) {
			if(IDNO == MessageBox("You must have an insurance company selected before opening a form.\n"
				"Would you like to auto-select the patient's primary insurance?\n\n"
				"(If 'No', then you will need to select an insurance company before you can open a form.)", "Practice", MB_YESNO|MB_ICONINFORMATION)) {
				return;
			}
			else {
				m_pInsurance1->SetSelByColumn(ipccID, m_GuarantorID1);

				// (j.jones 2013-08-07 12:04) - PLID 58042 - if a HCFA, and a company is selected,
				//toggle the fields that need to change for the new/old HCFA form
				UpdateDisplayedHCFAFields(m_GuarantorID1, GetBillID());

				// (j.jones 2014-05-29 10:56) - PLID 61837 - also update the charge provider
				// columns back on the bill tab
				((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.TryShowAllChargeProviderColumns();

				// (s.tullis 2016-02-24 16:34) - PLID 68319
				((CBillingModuleDlg*)m_pBillingModuleWnd)->UpdateClaimFormSelection();
			}
		}
		else {
			MsgBox("Please select a primary insurance company before opening a form");
			return;
		}		
	}

	if(IDNO == DontShowMeAgain(this, "This action will first save all changes you've made to this bill.\n"
		"Clicking 'Cancel' on the bill later will not undo your changes.\n"
		"Are you sure you wish to continue?", "BillingDlgPrintClaimForm", "Practice", FALSE, TRUE)) {
		return;
	}

	CWaitCursor pWait;
	
	if(!pBillingWnd->Save())
		return;


	////////////////////////////////////////////////////////////////////////////////
	// Open the form
	////////////////////////////////////////////////////////////////////////////////
	if(m_FormTypeCombo->GetCurSel() == -1)
		return;

	// (j.jones 2008-02-11 14:43) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
	// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
	if(!CanCreateInsuranceClaim(GetBillID(), FALSE)) {
		return;
	}
	
	long nFormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

	switch(nFormType) {
	case 1: {	// Open HCFA

		CHCFADlg hcfa(this);
			CRect rc;			
			_variant_t var;

			var = m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID);
			if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
				return;
			}

			long nInsuranceID = VarLong(var);

			// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening HCFAs without a HCFA group
			long nHCFAGroupID = -1;
			if(nInsuranceID != -1) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.HCFASetupGroupID FROM InsuranceCoT "
					"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"WHERE InsuredPartyT.PersonID = {INT}", nInsuranceID);
				if(!rs->eof) {
					nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);
				}
				rs->Close();
			}

			if(nHCFAGroupID == -1) {
				MsgBox("The selected insurance company is not configured in a HCFA group.\n"
					"You can set up a HCFA group in the HCFA tab of the Administrator module.\n\n"
					"This HCFA cannot be opened until the insurance company has been properly set up.");
				return;
			}

/*	DRT	6/3/03 - This code is redundant.  It's setting up all the print info manually here, then 
		calling the print function, which has provisions to do all this anyways, but it sets a flag
		to ignore the FormDisplay print setup.  Well now we're going to call that and let it handle, 
		since we now keep registry keys for each form type.

			CPrintDialog *printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
			// Initialize some of the fields in PRINTDLG structure.
			printDlg->m_pd.nMinPage = printDlg->m_pd.nMaxPage = 1;
			printDlg->m_pd.nFromPage = printDlg->m_pd.nToPage = 1;
			printDlg->DoModal();

			dlgWait.Create(IDD_PRINT_WAIT_DLG, this);
			dlgWait.GetWindowRect(rc);
			dlgWait.SetWindowPos(&wndTopMost,
				(GetSystemMetrics(SM_CXFULLSCREEN) - rc.Width())/2, (GetSystemMetrics(SM_CYFULLSCREEN)-rc.Height())/2,
					0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
			dlgWait.RedrawWindow();
*/
			hcfa.m_ShowWindowOnInit = TRUE;		//DRT
			hcfa.m_PatientID = m_nPatientID;
			hcfa.m_ID = GetBillID();
//DRT			hcfa.m_printDlg = printDlg;
			hcfa.Create(IDD_HCFA, this);
			hcfa.OnPrint();
			hcfa.DestroyWindow();

//DRT			dlgWait.DestroyWindow();

		m_HasHCFABeenOpened = TRUE;
		break;
			}
	case 2: {	// Open UB92

			CRect rc;			
			_variant_t var;

			var = m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID);
			if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
				return;
			}

			long nInsuranceID = VarLong(var);

			// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening UBs without a UB group
			long nUBGroupID = -1;
			if(nInsuranceID != -1) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.UB92SetupGroupID FROM InsuranceCoT "
					"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"WHERE InsuredPartyT.PersonID = {INT}", nInsuranceID);
				if(!rs->eof) {
					nUBGroupID = AdoFldLong(rs, "UB92SetupGroupID", -1);
				}
				rs->Close();
			}

			if(nUBGroupID == -1) {
				MsgBox("The selected insurance company is not configured in a UB group.\n"
					"You can set up a UB group in the UB tab of the Administrator module.\n\n"
					"This claim cannot be opened until the insurance company has been properly set up.");
				return;
			}

/*	DRT - See comments in Hcfa case
			CPrintDialog *printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
			// Initialize some of the fields in PRINTDLG structure.
			printDlg->m_pd.nMinPage = printDlg->m_pd.nMaxPage = 1;
			printDlg->m_pd.nFromPage = printDlg->m_pd.nToPage = 1;
			printDlg->DoModal();

			dlgWait.Create(IDD_PRINT_WAIT_DLG, this);
			dlgWait.GetWindowRect(rc);
			dlgWait.SetWindowPos(&wndTopMost,
				(GetSystemMetrics(SM_CXFULLSCREEN) - rc.Width())/2, (GetSystemMetrics(SM_CYFULLSCREEN)-rc.Height())/2,
					0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
			dlgWait.RedrawWindow();
*/

			//TES 3/13/2007 - PLID 24993 - Open the correct form
			if(GetUBFormType() == eUB04) {
				CUB04Dlg ub04(this);
				ub04.m_ShowWindowOnInit = TRUE;		//DRT
				ub04.m_PatientID = m_nPatientID;
				ub04.m_ID = GetBillID();
	//DRT			ub92.m_printDlg = printDlg;
				ub04.Create(IDD_HCFA, this);
				ub04.OnClickPrint();
				ub04.DestroyWindow();
			}
			else {
				CUB92Dlg ub92(this);
				ub92.m_ShowWindowOnInit = TRUE;		//DRT
				ub92.m_PatientID = m_nPatientID;
				ub92.m_ID = GetBillID();
	//DRT			ub92.m_printDlg = printDlg;
				ub92.Create(IDD_HCFA, this);
				ub92.OnClickPrint();
				ub92.DestroyWindow();
			}

//DRT			dlgWait.DestroyWindow();

		m_HasHCFABeenOpened = TRUE;
		break;
			}
	case 3: {	// Open ADA
			_variant_t var = m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID);
			if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
				return;
			}

			// (j.armen 2014-03-05 09:17) - PLID 60784 - Fill BillID, PatientID in constructor
			CADADlg ada(this, GetBillID(), m_nPatientID);
			ada.m_ShowWindowOnInit = TRUE;
			ada.Create(IDD_HCFA, this);
			ada.OnClickPrint();
			ada.DestroyWindow();

			m_HasHCFABeenOpened = TRUE;
		break;
			}
	case 4: {	// Open IDPA

		CIDPADlg idpa(this);
			CRect rc;			
			_variant_t var;

			var = m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID);
			if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
				return;
			}
/*	DRT - See comments in HCFA case
			CPrintDialog *printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
			// Initialize some of the fields in PRINTDLG structure.
			printDlg->m_pd.nMinPage = printDlg->m_pd.nMaxPage = 1;
			printDlg->m_pd.nFromPage = printDlg->m_pd.nToPage = 1;
			printDlg->DoModal();

			dlgWait.Create(IDD_PRINT_WAIT_DLG, this);
			dlgWait.GetWindowRect(rc);
			dlgWait.SetWindowPos(&wndTopMost,
				(GetSystemMetrics(SM_CXFULLSCREEN) - rc.Width())/2, (GetSystemMetrics(SM_CYFULLSCREEN)-rc.Height())/2,
					0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
			dlgWait.RedrawWindow();
*/
			idpa.m_ShowWindowOnInit = TRUE;		//DRT
			idpa.m_PatientID = m_nPatientID;
			idpa.m_ID = GetBillID();
//DRT			idpa.m_printDlg = printDlg;
			idpa.Create(IDD_HCFA, this);
			idpa.OnClickPrint();
			idpa.DestroyWindow();

//DRT			dlgWait.DestroyWindow();

		m_HasHCFABeenOpened = TRUE;
		break;
			}
	case 5: {	// Open NYWC

		CNYWCDlg nywc(this);
			CRect rc;			
			_variant_t var;

			var = m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID);
			if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
				return;
			}
/*	DRT - See comments in HCFA case
			CPrintDialog *printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
			// Initialize some of the fields in PRINTDLG structure.
			printDlg->m_pd.nMinPage = printDlg->m_pd.nMaxPage = 1;
			printDlg->m_pd.nFromPage = printDlg->m_pd.nToPage = 1;
			printDlg->DoModal();

			dlgWait.Create(IDD_PRINT_WAIT_DLG, this);
			dlgWait.GetWindowRect(rc);
			dlgWait.SetWindowPos(&wndTopMost,
				(GetSystemMetrics(SM_CXFULLSCREEN) - rc.Width())/2, (GetSystemMetrics(SM_CYFULLSCREEN)-rc.Height())/2,
					0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
			dlgWait.RedrawWindow();
*/
			nywc.m_ShowWindowOnInit = TRUE;		//DRT
			nywc.m_PatientID = m_nPatientID;
			nywc.m_ID = GetBillID();
//DRT			nywc.m_printDlg = printDlg;
			nywc.Create(IDD_HCFA, this);
			nywc.OnClickPrint();
			nywc.DestroyWindow();

//DRT			dlgWait.DestroyWindow();

		m_HasHCFABeenOpened = TRUE;
		break;
			}

	case 6: {	// Open MICR

			// (j.jones 2007-05-09 10:28) - PLID 25550 - check the internal preference
			// for which MICR form to use

			if(GetRemotePropertyInt("Use2007MICR", 1, 0, "<None>", true) == 1) {

				CMICR2007Dlg micr(this);
				CRect rc;			
				_variant_t var;

				var = m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID);
				if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
					return;
				}
	/*	DRT - See comments in HCFA case
				CPrintDialog *printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
				// Initialize some of the fields in PRINTDLG structure.
				printDlg->m_pd.nMinPage = printDlg->m_pd.nMaxPage = 1;
				printDlg->m_pd.nFromPage = printDlg->m_pd.nToPage = 1;
				printDlg->DoModal();

				dlgWait.Create(IDD_PRINT_WAIT_DLG, this);
				dlgWait.GetWindowRect(rc);
				dlgWait.SetWindowPos(&wndTopMost,
					(GetSystemMetrics(SM_CXFULLSCREEN) - rc.Width())/2, (GetSystemMetrics(SM_CYFULLSCREEN)-rc.Height())/2,
						0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
				dlgWait.RedrawWindow();
	*/
				micr.m_ShowWindowOnInit = TRUE;		//DRT
				micr.m_PatientID = m_nPatientID;
				micr.m_ID = GetBillID();
	//DRT			micr.m_printDlg = printDlg;
				micr.Create(IDD_HCFA, this);
				micr.OnClickPrint();
				micr.DestroyWindow();

	//DRT		dlgWait.DestroyWindow();
			}
			else {

				CMICRDlg micr(this);
				CRect rc;			
				_variant_t var;

				var = m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID);
				if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
					return;
				}
	/*	DRT - See comments in HCFA case
				CPrintDialog *printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
				// Initialize some of the fields in PRINTDLG structure.
				printDlg->m_pd.nMinPage = printDlg->m_pd.nMaxPage = 1;
				printDlg->m_pd.nFromPage = printDlg->m_pd.nToPage = 1;
				printDlg->DoModal();

				dlgWait.Create(IDD_PRINT_WAIT_DLG, this);
				dlgWait.GetWindowRect(rc);
				dlgWait.SetWindowPos(&wndTopMost,
					(GetSystemMetrics(SM_CXFULLSCREEN) - rc.Width())/2, (GetSystemMetrics(SM_CYFULLSCREEN)-rc.Height())/2,
						0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
				dlgWait.RedrawWindow();
	*/
				micr.m_ShowWindowOnInit = TRUE;		//DRT
				micr.m_PatientID = m_nPatientID;
				micr.m_ID = GetBillID();
	//DRT			micr.m_printDlg = printDlg;
				micr.Create(IDD_HCFA, this);
				micr.OnClickPrint();
				micr.DestroyWindow();

	//DRT		dlgWait.DestroyWindow();
			}

		m_HasHCFABeenOpened = TRUE;
		break;
			}

	case 7: {	// Open NY Medicaid

		// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
		CNYMedicaidDlg nymcaid(this);
		CRect rc;			
		_variant_t var;

		var = m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID);
		if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
			return;
		}
		nymcaid.m_ShowWindowOnInit = TRUE;
		nymcaid.m_PatientID = m_nPatientID;
		nymcaid.m_ID = GetBillID();
		nymcaid.Create(IDD_HCFA, this);
		nymcaid.OnClickPrint();
		nymcaid.DestroyWindow();

		m_HasHCFABeenOpened = TRUE;
		break;
			}
	}
}

HBRUSH CInsuranceBilling::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{	
	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CInsuranceBilling::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	CString value;
	int nID;

	switch (HIWORD(wParam))
	{	case EN_KILLFOCUS:
			switch (nID = LOWORD(wParam))
			{
				case IDC_EDIT_STATE: {
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);
					COleDateTime dtOldDate;
					dtOldDate.SetDateTime(1800,1,1,1,1,1);

				}
				break;

				case IDC_EDIT_CHARGES: {
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);
					COleDateTime dtOldDate;
					dtOldDate.SetDateTime(1800,1,1,1,1,1);

				}
				break;

				default:
					return CNxDialog::OnCommand(wParam, lParam);
			}
			break;
	}	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CInsuranceBilling::OnSelChosenFormTypeCombo(long nRow) 
{
	if(nRow == -1)
		return;

	PostFormTypeChanged();

	// (j.jones 2006-09-11 09:45) - PLID 22177 - changed to allow auto-batching even
	// on existing bills

	if(m_pInsurance1->CurSel == -1)
		return;

	long FormID = VarLong(m_FormTypeCombo->GetValue(nRow,0));
	long nInsuredPartyID = VarLong(m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID), -1);

	if(FormID == 2) {
		//UB92
		if(nInsuredPartyID > 0) {
			int batch = FindDefaultUB92Batch(nInsuredPartyID);
			// (j.jones 2006-11-27 12:18) - PLID 23650 - fixed bug where the batch
			// was sometimes not set
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(batch == 1);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(batch == 2);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(batch == 0);
		}
		else {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);
		}
	}
	else if(FormID == 1) {
		//HCFA
		if(nInsuredPartyID > 0) {
			int batch = FindDefaultHCFABatch(nInsuredPartyID);
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(batch == 1);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(batch == 2);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(batch == 0);
		}
		else {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);
		}
	}

	// (j.jones 2013-08-13 11:23) - PLID 57902 - added additional screen for obscure claim fields,
	// this is always enabled even if the bill is disabled (the dialog would also be disabled),
	// but it is hidden if the form type is not HCFA or UB
	long nFormID = -1;
	if(m_FormTypeCombo->GetCurSel() != -1) {
		nFormID = VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0));
	}
	GetDlgItem(IDC_BTN_CLAIM_FIELDS)->ShowWindow((nFormID == 1 || nFormID == 2) ? SW_SHOW : SW_HIDE);
}

void CInsuranceBilling::OnBtnEditPriorAuthNum() 
{
	try {

		// (j.jones 2010-10-21 11:11) - PLID 41050 - converted into a parameterized recordset
		// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.PersonID "
			"FROM InsuredPartyT "
			"INNER JOIN InsuranceCoT ON InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE PatientID = {INT} AND RespTypeID > 0 "
			"AND (UseReferrals = 1 OR EXISTS ("
			"	SELECT ID "
			"	FROM InsuranceReferralsT "
			"	LEFT JOIN (SELECT Count(InsuranceReferralID) AS NumUsed, InsuranceReferralID "
			"		FROM BillsT WHERE Deleted = 0 AND InsuranceReferralID IS NOT NULL AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT) "
			"		GROUP BY InsuranceReferralID) AS NumUsedQ ON InsuranceReferralsT.ID = NumUsedQ.InsuranceReferralID "
			"	WHERE NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) "
			"	AND dbo.AsDateNoTime(StartDate) <= dbo.AsDateNoTime(GetDate()) AND dbo.AsDateNoTime(EndDate) >= dbo.AsDateNoTime(GetDate()) "
			"	AND InsuredPartyID = InsuredPartyT.PersonID"
			")) "
			"ORDER BY Priority", m_nPatientID);
	
		// check to see if the patient has an insurance company which uses the insurance referral, if they don't let the user know
		// the reason the prompt doesn't come up to choose an insurance referral
		if(rs->eof) {
			AfxMessageBox("There are no active insurance companies for this patient which are marked to use insurance referrals.", MB_OK);
			return;
		}
		rs->Close();

		((CBillingModuleDlg*)m_pBillingModuleWnd)->ApplyInsuranceReferral(true, false);

	}NxCatchAll("Error in CInsuranceBilling::OnBtnEditPriorAuthNum()");
}

void CInsuranceBilling::OnKillfocusEditAuthorizationNumber() 
{
	try {

		//if they killfocus, they may have changed the authorization number to something 
		//that was typed in.  If so, we need to 1)  check if that auth number exists for
		//this ins party.  if so, update our variables appropriately.  if not, then we need
		//to reset our variables.
		CString strAuth;
		GetDlgItemText(IDC_EDIT_AUTHORIZATION_NUMBER, strAuth);
		if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_strSelectedAuthNum == strAuth) {
			//no changes, we're good
			return;
		}

		//lookup the new one
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM InsuranceReferralsT WHERE AuthNum = {STRING} AND InsuredPartyID = {INT}", 
			strAuth, ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_GuarantorID1);
		if(rs->eof) {
			//doesn't exist, wipe out our vars
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nInsuranceReferralID = -1;
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_strSelectedAuthNum = "";
		}
		else {
			//it does exist!  update our info
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nInsuranceReferralID = AdoFldLong(rs, "ID");
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_strSelectedAuthNum = strAuth;
		}

	} NxCatchAll("Error updating authorization number.");

}

void CInsuranceBilling::OnSelChosenInsplan1(long nRow) 
{
	try {

		if(nRow == -1 || m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID).vt == VT_NULL) {
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);
			return;
		}

		long nInsuredPartyID = VarLong(m_pInsurance1->GetValue(nRow, ipccID));

		// (j.jones 2013-08-07 12:04) - PLID 58042 - if a HCFA, and a company is selected,
		//toggle the fields that need to change for the new/old HCFA form
		UpdateDisplayedHCFAFields(nInsuredPartyID, GetBillID());

		//if and only if there is a special POS code associated with this insurance company, switch it on the bill
		//otherwise leave it alone

		// (j.jones 2008-05-02 12:49) - PLID 28278 - check to see if the POS is inactive
		// (a.walling 2011-01-28 15:29) - PLID 40965 - Use a fragment
		CSqlFragment fragmentPlaceOfServiceFilter;
		long nPOSSel = ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_PlaceOfServiceCombo->CurSel;
		if(nPOSSel == -1 && ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_PlaceOfServiceCombo->IsComboBoxTextInUse) {
			fragmentPlaceOfServiceFilter.Create("IN (SELECT LocationID FROM BillsT WHERE ID = {INT})", GetBillID());
		}
		else {
			fragmentPlaceOfServiceFilter.Create("= {INT}",
				VarLong(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_PlaceOfServiceCombo->GetValue(nPOSSel, 0)));
		}

		long nPOS = -1;
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT POSID FROM POSLocationLinkT "
			"INNER JOIN InsuranceCoT ON POSLocationLinkT.HCFASetupGroupID = InsuranceCoT.HCFASetupGroupID "
			"WHERE LocationID {SQL} AND InsuranceCoT.PersonID = {INT}"
			, fragmentPlaceOfServiceFilter
			, GetInsuranceCoID(nInsuredPartyID));
		if(!rs->eof) {
			nPOS = AdoFldLong(rs, "POSID",-1);
			if(nPOS != -1) {
				((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_DesignationCombo->SetSelByColumn(0, nPOS);
			}
		}
		rs->Close();
		
		//if the selected insurance is Worker's Comp., check the employment accident button
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(ReturnsRecordsParam("SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
			"WHERE WorkersComp = 1 AND InsuranceCoT.PersonID = {INT}",GetInsuranceCoID(nInsuredPartyID))) {
			
			((CButton*)GetDlgItem(IDC_RADIO_EMPLOYMENT))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_AUTO_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_OTHER_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_NONE))->SetCheck(0);
		}

		// (j.jones 2006-09-11 09:45) - PLID 22177 - changed to allow auto-batching even
		// on existing bills

		if(nInsuredPartyID > 0) {
			int batch = 0;
			long ClaimForm = VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(), 0));
			if(ClaimForm == 1)
				batch = FindDefaultHCFABatch(nInsuredPartyID);
			else if(ClaimForm == 2)
				batch = FindDefaultUB92Batch(nInsuredPartyID);
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(batch == 1 || batch == 3);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(batch == 2);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(batch == 0);
		}
		else {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);
		}

		// (j.jones 2014-05-29 10:56) - PLID 61837 - also update the charge provider
		// columns back on the bill tab
		((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.TryShowAllChargeProviderColumns();
		// (s.tullis 2016-02-24 16:34) - PLID 68319
		((CBillingModuleDlg*)m_pBillingModuleWnd)->UpdateClaimFormSelection();

	}NxCatchAll(__FUNCTION__);
}

void CInsuranceBilling::OnRadioPaperBatch() 
{
	
	// (j.gruber 2009-07-13 08:48) - PLID 34724 - marked manually unbatched as false
	((CBillingModuleDlg*)m_pBillingModuleWnd)->m_bManuallyUnbatched = FALSE;

	if (m_pInsurance1->CurSel == -1 || m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID).vt == VT_NULL) {
		MsgBox("Please select a primary insurance company before batching a form");
		((CButton*)GetDlgItem(IDC_RADIO_PAPER_BATCH))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);		
		// (j.gruber 2009-07-13 08:48) - PLID 34724 - marked manually unbatched as false
		((CBillingModuleDlg*)m_pBillingModuleWnd)->m_bManuallyUnbatched = TRUE;
	}

	
	
}

void CInsuranceBilling::OnRadioElectronicBatch() 
{
	// (j.gruber 2009-07-13 08:48) - PLID 34724 - marked manually unbatched as false
	((CBillingModuleDlg*)m_pBillingModuleWnd)->m_bManuallyUnbatched = FALSE;

	if (m_pInsurance1->CurSel == -1 || m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID).vt == VT_NULL) {
		MsgBox("Please select a primary insurance company before batching a form");
		((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC_BATCH))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_UNBATCHED))->SetCheck(TRUE);		
		
		// (j.gruber 2009-07-13 08:48) - PLID 34724 - marked manually unbatched as false
		((CBillingModuleDlg*)m_pBillingModuleWnd)->m_bManuallyUnbatched = TRUE;
	}

	
}

void CInsuranceBilling::OnSelectPcp() 
{
	try{
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rsPCP = CreateParamRecordset("SELECT CASE WHEN PCP Is Null THEN -1 ELSE PCP END AS PCP "
			" FROM PatientsT WHERE PersonID = {INT}", m_nPatientID);
		long nDefaultReferringPhyID  = AdoFldLong(rsPCP, "PCP",-1);
		//(e.lally 2005-11-02) PLID 17444 - We need to handle inactive primary care physicians like we do providers.
		if (nDefaultReferringPhyID > 0) {
			if(m_pRefList->TrySetSelByColumn(rpcID, (long) nDefaultReferringPhyID)== -1){
				// (b.cardillo 2006-07-31 12:32) - PLID 21705 - This code was already correct, but for 
				// completeness I wanted to comment the fact that we need to clear the selection whenever 
				// m_pRefList->TrySetSelByColumn() returns failure, because we assume that to mean the 
				// desired ref phys is inactive, and since trysetsel itself doesn't clear the selection 
				// on failure, we have to do so otherwise something else might still be selected.
				m_pRefList->PutCurSel(-1);
				
				//they may have an inactive primary care physician
				
				// (j.jones 2006-08-02 14:53) - PLID 21678 - If making a new bill, we do not want to load an
				// inactive referring physician, so we'll default to no referring physician at all
				/*
				_RecordsetPtr rsPCP = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nDefaultReferringPhyID);
				if(!rsPCP->eof) {
					m_pRefList->PutComboBoxText(_bstr_t(AdoFldString(rsPCP, "Name", "")));
				}
				*/
			}
		}
		else{
			MessageBox("There is no Primary Care Physician selected on the General 2 tab.", "Practice", MB_OK);
		}
	}NxCatchAll("Error in CInsuranceBilling::OnSelectPcp()");
	
}

void CInsuranceBilling::OnSelectRefPhys() 
{
	try{
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rsRefPhys = CreateParamRecordset("SELECT CASE WHEN DefaultReferringPhyID Is Null THEN -1 ELSE DefaultReferringPhyID END AS DefaultReferringPhyID "
			"FROM PatientsT WHERE PersonID = {INT}", m_nPatientID);
		long nDefaultReferringPhyID = AdoFldLong(rsRefPhys, "DefaultReferringPhyID",-1);
		//(e.lally 2005-11-02) PLID 17444 - We need to handle inactive referring physicians like we do providers.
		if (nDefaultReferringPhyID > 0) {
			if(m_pRefList->TrySetSelByColumn(rpcID, (long) nDefaultReferringPhyID)== -1){
				// (b.cardillo 2006-07-31 12:32) - PLID 21705 - This code was already correct, but for 
				// completeness I wanted to comment the fact that we need to clear the selection whenever 
				// m_pRefList->TrySetSelByColumn() returns failure, because we assume that to mean the 
				// desired ref phys is inactive, and since trysetsel itself doesn't clear the selection 
				// on failure, we have to do so otherwise something else might still be selected.
				m_pRefList->PutCurSel(-1);
				
				//they may have an inactive referring physician
				
				// (j.jones 2006-08-02 14:53) - PLID 21678 - If making a new bill, we do not want to load an
				// inactive referring physician, so we'll default to no referring physician at all
				/*
				_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nDefaultReferringPhyID);
				if(!rsProv->eof) {
					m_pRefList->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
				}
				*/
			}
		}
		else{
			MessageBox("There is no Referring Physician selected on the General 2 tab.", "Practice", MB_OK);
		}
	}NxCatchAll("Error in CInsuranceBilling::OnSelectRefPhys()");
	
}

void CInsuranceBilling::SetAccidentStatus()
{
	try {

		if(GetBillID() != -1)
			return;

		//if the primary insurance is Worker's Comp., check the employment accident button
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(ReturnsRecordsParam("SELECT InsuranceCoT.PersonID FROM InsuranceCoT "
			"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"WHERE WorkersComp = 1 AND InsuredPartyT.RespTypeID = 1 AND InsuredPartyT.PatientID = {INT}",m_nPatientID)) {
			
			((CButton*)GetDlgItem(IDC_RADIO_EMPLOYMENT))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_AUTO_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_OTHER_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_NONE))->SetCheck(0);
		}
		else {
			((CButton*)GetDlgItem(IDC_RADIO_EMPLOYMENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_AUTO_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_OTHER_ACCIDENT))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_NONE))->SetCheck(1);
		}
	}NxCatchAll("Error loading default accident status.");
}

void CInsuranceBilling::OnKillFocusEditCurrentDate() 
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	COleDateTime dtOldDate;
	dtOldDate.SetDateTime(1800,1,1,1,1,1);

	if(m_pAccident->GetStatus() == 1) {

		if(m_pAccident->GetDateTime() < dtOldDate) {
			AfxMessageBox("The 'Date of Current Illness / Injury' is before the year 1800.\n"
				"Please correct this date before saving.");
		}

		if(m_pAccident->GetDateTime() > COleDateTime::GetCurrentTime()) {
			AfxMessageBox("The 'Date of Current Illness / Injury' is after today.\n"
				"Please correct this date before saving.");
		}
	}
	else if(m_pAccident->GetStatus() == 2) {
		AfxMessageBox("The 'Date of Current Illness / Injury' is invalid.\n"
				"Please correct this date before saving.");
	}
}

void CInsuranceBilling::OnKillFocusEditFirstDate() 
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	COleDateTime dtOldDate;
	dtOldDate.SetDateTime(1800,1,1,1,1,1);

	if(m_pIllness->GetStatus() == 1) {

		// (r.gonet 2016-04-07) - NX-100072 - Use the correct name for the FirstConditionDate time control.
		if(m_pIllness->GetDateTime() < dtOldDate) {
			AfxMessageBox(FormatString("The %s is before the year 1800.\n"
				"Please correct this date before saving.", GetConditionDateTypeDescription(m_claimDates.eConditionDateType)));
			return;
		}

		if(m_pIllness->GetDateTime() > COleDateTime::GetCurrentTime()) {
			// (r.gonet 2016-04-07) - NX-100072 - Use the correct name for the FirstConditionDate time control.
			AfxMessageBox(FormatString("The %s is after today.\n"
				"Please correct this date before saving.", GetConditionDateTypeDescription(m_claimDates.eConditionDateType)));
			return;
		}

		// (r.gonet 2016-04-07) - NX-100072 - Sync up the ClaimDates structure.
		m_claimDates.SetFirstConditionDate(m_pIllness->GetDateTime());
	}
	else if(m_pIllness->GetStatus() == 2) {
		// (r.gonet 2016-04-07) - NX-100072 - Use the correct name for the FirstConditionDate time control.
		AfxMessageBox(FormatString("The %s is invalid.\n"
				"Please correct this date before saving.", GetConditionDateTypeDescription(m_claimDates.eConditionDateType)));
		return;
	}
	else if (m_pIllness->GetStatus() == 3) {
		m_claimDates.SetFirstConditionDate(g_cdtNull);
	}
}

void CInsuranceBilling::OnKillFocusEditUnableToWorkFrom() 
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	COleDateTime dtOldDate;
	dtOldDate.SetDateTime(1800,1,1,1,1,1);

	if(m_pWorkFrom->GetStatus() == 1) {
		if(m_pWorkFrom->GetDateTime() < dtOldDate) {
			AfxMessageBox("The 'patient unable to work from' date is before the year 1800.\n"
				"Please correct this date before saving.");
		}
	}
	else if(m_pWorkFrom->GetStatus() == 2) {
		AfxMessageBox("The 'patient unable to work from' date is invalid.\n"
				"Please correct this date before saving.");
	}
}

void CInsuranceBilling::OnKillFocusEditUnableToWorkTo() 
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	COleDateTime dtOldDate;
	dtOldDate.SetDateTime(1800,1,1,1,1,1);

	if(m_pWorkTo->GetStatus() == 1) {
		if(m_pWorkTo->GetDateTime() < dtOldDate) {
			AfxMessageBox("The 'patient unable to work to' date is before the year 1800.\n"
				"Please correct this date before saving.");
		}
	}
	else if(m_pWorkTo->GetStatus() == 2) {
		AfxMessageBox("The 'patient unable to work to' date is invalid.\n"
				"Please correct this date before saving.");
	}
}

void CInsuranceBilling::OnKillFocusEditHospitalizedFrom() 
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	COleDateTime dtOldDate;
	dtOldDate.SetDateTime(1800,1,1,1,1,1);

	if(m_pHospFrom->GetStatus() == 1) {
		if(m_pHospFrom->GetDateTime() < dtOldDate) {
			AfxMessageBox("The 'patient hospitalized from' date is before the year 1800.\n"
				"Please correct this date before saving.");
		}
	}
	else if(m_pHospFrom->GetStatus() == 2) {
		AfxMessageBox("The 'patient hospitalized from' date is invalid.\n"
				"Please correct this date before saving.");
	}
}

void CInsuranceBilling::OnKillFocusEditHospitalizedTo() 
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	COleDateTime dtOldDate;
	dtOldDate.SetDateTime(1800,1,1,1,1,1);

	if(m_pHospTo->GetStatus() == 1) {
		if(m_pHospTo->GetDateTime() < dtOldDate) {
			AfxMessageBox("The 'patient hospitalized to' date is before the year 1800.\n"
				"Please correct this date before saving.");
		}
	}
	else if(m_pHospTo->GetStatus() == 2) {
		AfxMessageBox("The 'patient hospitalized to' date is invalid.\n"
				"Please correct this date before saving.");
	}
}

void CInsuranceBilling::PostFormTypeChanged()
{
	// (a.walling 2010-01-14 16:29) - PLID 36889 - All SW_SHOW calls here have been changed to SW_SHOWNOACTIVATE to stop messing with the focus
	//don't change the default batch here, only show/hide the necessary fields

	long nFormID = -1;
	if(m_FormTypeCombo->GetCurSel() != -1) {
		nFormID = VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0));
	}

	// (j.jones 2013-08-13 11:23) - PLID 57902 - added additional screen for obscure claim fields,
	// this is always enabled even if the bill is disabled (the dialog would also be disabled),
	// but it is hidden if the form type is not HCFA or UB
	GetDlgItem(IDC_BTN_CLAIM_FIELDS)->ShowWindow((nFormID == 1 || nFormID == 2) ? SW_SHOW : SW_HIDE);

	// (j.jones 2009-02-11 10:06) - PLID 33035 - added ManualReview, which is OHIP only,
	// this is really not determined by form type but this seems an appropriate place
	// to have this code
	long nEbillingFormatType = GetRemotePropertyInt("EbillingFormatType", ANSI, 0, "<None>", TRUE);
	if(nEbillingFormatType == OHIP) {
		m_checkManualReview.ShowWindow(SW_SHOWNOACTIVATE);
	}
	else {
		m_checkManualReview.ShowWindow(SW_HIDE);
	}

	// (j.jones 2016-05-24 14:39) - NX-100704 - added a function to repopulate the claim type
	// combo based on the current form type
	RepopulateClaimTypeCombo();

	//TES 3/13/2007 - PLID 24993 - Update the dropdown to show the correct form type..
	if(GetUBFormType() == 1) {
		int nUBRow = m_FormTypeCombo->FindByColumn(0, (long)2, 0, VARIANT_FALSE);
		ASSERT(nUBRow != -1);
		m_FormTypeCombo->PutValue(nUBRow, 1, _bstr_t("UB04"));
	}
	else {
		int nUBRow = m_FormTypeCombo->FindByColumn(0, (long)2, 0, VARIANT_FALSE);
		ASSERT(nUBRow != -1);
		m_FormTypeCombo->PutValue(nUBRow, 1, _bstr_t("UB92"));
	}

	if(nFormID == 2) {
		//UB92

		//only show the UB92 fields
		GetDlgItem(IDC_LABEL_BOX_19)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_BOX_19)->ShowWindow(SW_HIDE);
		// (j.jones 2010-11-08 16:51) - PLID 39620 - if Alberta is in use, we always show
		// the 'send correspondence' checkbox
		if(!UseAlbertaHLINK()) {
			GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE)->ShowWindow(SW_HIDE);
		}

		// (j.jones 2010-06-10 10:32) - PLID 38507 - added HCFA Box 13 config
		// (j.jones 2010-07-27 11:02) - PLID 39858 - this is now supported in the UB		
		if(GetRemotePropertyInt("ShowBillHCFABox13Setting", 0, 0, "<None>", true) == 1
			|| m_eOldHCFABox13Over != hb13_UseDefault) {
			GetDlgItem(IDC_LABEL_HCFA_BOX_13)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_HCFA_BOX_13_COMBO)->ShowWindow(SW_SHOWNOACTIVATE);
		}
		else {
			GetDlgItem(IDC_LABEL_HCFA_BOX_13)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_HCFA_BOX_13_COMBO)->ShowWindow(SW_HIDE);
		}

		//TES 3/13/2007 - PLID 24993 - Make sure the field labels reflect the correct box number, based on which UB form
		// they're using.
		if(GetUBFormType() == eUB04) {
			// (j.jones 2010-07-27 11:02) - PLID 39858 - change the label for the Assignment of Benefits
			//(same box number on both forms, but we do change the label)
			GetDlgItem(IDC_LABEL_HCFA_BOX_13)->SetWindowText("UB04 Box 53 (Assignment of Benefits)");
			SetDlgItemText(IDC_LABEL_UB92_BOX44_OVERRIDE, "UB04 Box 44 Override");

			// (j.jones 2012-05-14 15:40) - PLID 47650 - supported UB boxes 14 and 15
			GetDlgItem(IDC_LABEL_UB_BOX_14)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_EDIT_UB_BOX_14)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_LABEL_UB_BOX_15)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_EDIT_UB_BOX_15)->ShowWindow(SW_SHOWNOACTIVATE);
		}
		else {
			// (j.jones 2010-07-27 11:02) - PLID 39858 - change the label for the Assignment of Benefits
			//(same box number on both forms, but we do change the label)
			GetDlgItem(IDC_LABEL_HCFA_BOX_13)->SetWindowText("UB92 Box 53 (Assignment of Benefits)");
			SetDlgItemText(IDC_LABEL_UB92_BOX44_OVERRIDE, "UB92 Box 44 Override");

			// (j.jones 2012-05-14 15:40) - PLID 47650 - UB boxes 14 and 15 are not supported in UB92
			GetDlgItem(IDC_LABEL_UB_BOX_14)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_UB_BOX_14)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LABEL_UB_BOX_15)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_UB_BOX_15)->ShowWindow(SW_HIDE);
		}
		GetDlgItem(IDC_LABEL_UB92_BOX44_OVERRIDE)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_UB92_BOX_44_LIST)->ShowWindow(SW_SHOWNOACTIVATE);

		// (a.walling 2007-09-07 11:07) - PLID 27026
		GetDlgItem(IDC_PWK_TYPE)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PWK_TX)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_SEND_PAPERWORK_CHECK)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PWK_TYPE_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PWK_TX_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);

		// (j.jones 2008-05-14 12:35) - PLID 30044 - hide the test result fields
		GetDlgItem(IDC_SEND_TEST_RESULT_CHECK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULTS_ID_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULT_ID_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULTS_TYPE_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULT_TYPE_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_TEST_RESULT)->ShowWindow(SW_HIDE);

		// (j.jones 2016-05-24 14:56) - NX-100704 - we now show the claim type combo for UB claims
		GetDlgItem(IDC_CLAIM_TYPE_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CLAIM_TYPE_COMBO)->ShowWindow(SW_SHOW);

		// (j.jones 2016-05-26 15:36) - NX-100705 - rename the Original Reference Number field
		// and hide resubmission code
		m_nxstaticOriginalRefNoLabel.SetWindowText("Document Control Number");
		m_nxstaticResubmissionCodeLabel.ShowWindow(SW_HIDE);
		m_nxeditEditMedicaidCode.ShowWindow(SW_HIDE);
	}
	else if(nFormID == 1) {
		//HCFA

		//only show the HCFA fields
		GetDlgItem(IDC_LABEL_BOX_19)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_EDIT_BOX_19)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE)->ShowWindow(SW_SHOWNOACTIVATE);

		// (j.jones 2010-06-10 10:32) - PLID 38507 - added HCFA Box 13 config, 
		// it is only shown if the preference is enabled OR the existing bill has
		// an override chosen
		if(GetRemotePropertyInt("ShowBillHCFABox13Setting", 0, 0, "<None>", true) == 1
			|| m_eOldHCFABox13Over != hb13_UseDefault) {
			// (j.jones 2010-07-27 11:02) - PLID 39858 - change the label for the Assignment of Benefits
			// to reflect the HCFA box
			GetDlgItem(IDC_LABEL_HCFA_BOX_13)->SetWindowText("HCFA Box 13 (Assignment of Benefits)");
			GetDlgItem(IDC_LABEL_HCFA_BOX_13)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_HCFA_BOX_13_COMBO)->ShowWindow(SW_SHOWNOACTIVATE);
		}
		else {
			GetDlgItem(IDC_LABEL_HCFA_BOX_13)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_HCFA_BOX_13_COMBO)->ShowWindow(SW_HIDE);
		}

		GetDlgItem(IDC_LABEL_UB92_BOX44_OVERRIDE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UB92_BOX_44_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_UB_BOX_14)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_UB_BOX_14)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_UB_BOX_15)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_UB_BOX_15)->ShowWindow(SW_HIDE);
		
		// (a.walling 2007-09-07 11:07) - PLID 27026
		GetDlgItem(IDC_PWK_TYPE)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PWK_TX)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_SEND_PAPERWORK_CHECK)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PWK_TYPE_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PWK_TX_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);

		// (j.jones 2008-05-14 12:35) - PLID 30044 - show the test result fields
		GetDlgItem(IDC_SEND_TEST_RESULT_CHECK)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_TEST_RESULTS_ID_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_TEST_RESULT_ID_COMBO)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_TEST_RESULTS_TYPE_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_TEST_RESULT_TYPE_COMBO)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_TEST_RESULT_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_EDIT_TEST_RESULT)->ShowWindow(SW_SHOWNOACTIVATE);

		// (j.jones 2009-09-14 17:32) - PLID 35284 - show the claim type combo,
		//unless it is OHIP, in which case it stays hidden
		if(nEbillingFormatType == OHIP) {
			GetDlgItem(IDC_CLAIM_TYPE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CLAIM_TYPE_COMBO)->ShowWindow(SW_HIDE);
		}
		else {
			GetDlgItem(IDC_CLAIM_TYPE_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_CLAIM_TYPE_COMBO)->ShowWindow(SW_SHOWNOACTIVATE);
		}

		// (j.jones 2013-08-07 12:04) - PLID 58042 - if a HCFA, and a company is selected,
		//toggle the fields that need to change for the new/old HCFA form
		long nInsuredPartyID = -1;
		if(m_pInsurance1->CurSel != -1) {
			nInsuredPartyID = VarLong(m_pInsurance1->GetValue(m_pInsurance1->CurSel, ipccID),-1);
		}
		UpdateDisplayedHCFAFields(nInsuredPartyID, GetBillID());

		// (j.jones 2016-05-26 15:36) - NX-100705 - revert the Original Reference Number field
		// and show the resubmission code
		m_nxstaticOriginalRefNoLabel.SetWindowText("Original Reference Number");
		m_nxstaticResubmissionCodeLabel.ShowWindow(SW_SHOWNOACTIVATE);
		m_nxeditEditMedicaidCode.ShowWindow(SW_SHOWNOACTIVATE);
	}	
	else {

		//hide all form-specific fields
		GetDlgItem(IDC_LABEL_BOX_19)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_BOX_19)->ShowWindow(SW_HIDE);
		// (j.jones 2010-11-08 16:51) - PLID 39620 - if Alberta is in use, we always show
		// the 'send correspondence' checkbox
		if(!UseAlbertaHLINK()) {
			GetDlgItem(IDC_CHECK_SEND_CORRESPONDENCE)->ShowWindow(SW_HIDE);
		}

		// (j.jones 2010-06-10 10:32) - PLID 38507 - added HCFA Box 13 config
		GetDlgItem(IDC_LABEL_HCFA_BOX_13)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_HCFA_BOX_13_COMBO)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_LABEL_UB92_BOX44_OVERRIDE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UB92_BOX_44_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_UB_BOX_14)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_UB_BOX_14)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_UB_BOX_15)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_UB_BOX_15)->ShowWindow(SW_HIDE);
		
		// (a.walling 2007-09-07 11:07) - PLID 27026
		GetDlgItem(IDC_PWK_TYPE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PWK_TX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SEND_PAPERWORK_CHECK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PWK_TYPE_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PWK_TX_LABEL)->ShowWindow(SW_HIDE);

		// (j.jones 2008-05-14 12:35) - PLID 30044 - hide the test result fields
		GetDlgItem(IDC_SEND_TEST_RESULT_CHECK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULTS_ID_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULT_ID_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULTS_TYPE_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULT_TYPE_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEST_RESULT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_TEST_RESULT)->ShowWindow(SW_HIDE);

		// (j.jones 2009-09-14 17:32) - PLID 35284 - hide the claim type combo
		GetDlgItem(IDC_CLAIM_TYPE_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CLAIM_TYPE_COMBO)->ShowWindow(SW_HIDE);

		// (j.jones 2016-05-26 15:36) - NX-100705 - revert the Original Reference Number field
		// and show the resubmission code
		m_nxstaticOriginalRefNoLabel.SetWindowText("Original Reference Number");
		m_nxstaticResubmissionCodeLabel.ShowWindow(SW_SHOWNOACTIVATE);
		m_nxeditEditMedicaidCode.ShowWindow(SW_SHOWNOACTIVATE);
	}
}

void CInsuranceBilling::OnTrySetSelFinishedRefPhys(long nRowEnum, long nFlags) 
{
	if(nFlags == dlTrySetSelFinishedFailure) {
		//they may have an inactive referring physician
		// (b.cardillo 2006-07-31 12:06) - PLID 21705 - In fact, we assume this to be the only 
		// explanation for a failure to set the selection, so we need to clear the selection 
		// before we set the combo box text.
		m_pRefList->PutCurSel(sriNoRow);

		// Now go ahead and set the combo box text to the name of the desired selection.
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", m_nPendingReferringPhyID);
		if(!rsProv->eof) {
			m_pRefList->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
		}
	}
}

void CInsuranceBilling::OnTrySetSelFinishedUb92Box44List(long nRowEnum, long nFlags) 
{
	if(nFlags == dlTrySetSelFinishedFailure) {

		//may be inactive
		m_pUB92Box44List->PutCurSel(sriNoRow);

		// Now go ahead and set the combo box text to the name of the desired selection.
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rsCPT = CreateParamRecordset("SELECT Code FROM CPTCodeT WHERE ID = {INT}", m_nPendingUB92Box44ID);
		if(!rsCPT->eof) {
			m_pUB92Box44List->PutComboBoxText(_bstr_t(AdoFldString(rsCPT, "Code", "")));
		}
		rsCPT->Close();
	}
}

// (a.walling 2007-08-27 12:12) - PLID 27026
void CInsuranceBilling::OnSendPaperworkCheck() 
{
	if (((CButton*)GetDlgItem(IDC_SEND_PAPERWORK_CHECK))->GetCheck()) {
		m_pPWKType->Enabled = VARIANT_TRUE;
		m_pPWKTx->Enabled = VARIANT_TRUE;
	} else {
		m_pPWKType->Enabled = VARIANT_FALSE;
		m_pPWKTx->Enabled = VARIANT_FALSE;
	}		
}


// (a.walling 2008-05-05 13:15) - PLID 29897 - Patient name
CString CInsuranceBilling::GetBillPatientName()
{
	ASSERT(m_pBillingModuleWnd);

	if (m_pBillingModuleWnd)
		return ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetBillPatientName(m_nPatientID);
	else return GetExistingPatientName(m_nPatientID);
}

// (j.jones 2008-05-14 14:14) - PLID 30044 - added test result controls
void CInsuranceBilling::OnSendTestResultCheck() 
{
	try {

		if(m_checkSendTestResults.GetCheck()) {

			//give a don't show again warning that it is only used on certain codes
			DontShowMeAgain(this, "When the 'Send Test Result' setting is enabled, test result information is only sent for codes "
				"J0881, J0882, J0885, J0886, and Q4081, and only when a claim is exported using the ANSI format.",
				"BillingDlg_OnSendTestResultCheck", "Practice", FALSE, FALSE);
		}

		//enable/disable the test result controls
		PostSendTestResultsCheck();

	}NxCatchAll("Error in CInsuranceBilling::OnSendTestResultCheck");
}

// (j.jones 2008-05-14 16:26) - PLID 30044 - this will enable/disable the rest result controls,
// and is called after the user or the program checks/unchecks the 'send test results' box
void CInsuranceBilling::PostSendTestResultsCheck()
{
	try {

		//enable/disable the test result controls

		BOOL bEnable = m_checkSendTestResults.GetCheck();

		m_TestResultIDCombo->Enabled = bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		m_TestResultTypeCombo->Enabled = bEnable ? VARIANT_TRUE : VARIANT_FALSE;
		EnableDlgItem(IDC_EDIT_TEST_RESULT, bEnable);

		//if enabling, and the ID selection is empty, force TR
		// (j.jones 2011-06-17 16:18) - PLID 42179 - somehow this old code always called
		// SetSelByColumn even if bEnable was 0
		if(bEnable) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_TestResultIDCombo->GetCurSel();
			if(pRow == NULL) {
				m_TestResultIDCombo->SetSelByColumn(trilCode, "TR");
			}
		}

	}NxCatchAll("Error in CInsuranceBilling::PostSendTestResultsCheck");
}

// (j.jones 2008-05-14 17:09) - PLID 30044 - returns the status of the test result controls,
// so we can properly validate upon saving
void CInsuranceBilling::CheckTestResultFields(BOOL &bEnabled, BOOL &bIsValid)
{
	try {

		//is the test result enabled?
		bEnabled = m_checkSendTestResults.GetCheck();

		//now, do we have valid data?

		if(!bEnabled) {
			bIsValid = TRUE;
		}
		else {

			bIsValid = TRUE;

			//the data is invalid if the test result option is enabled,
			//but any of its fields are empty

			if(m_TestResultIDCombo->GetCurSel() == NULL) {
				bIsValid = FALSE;
			}

			// (j.jones 2010-10-22 10:04) - PLID 40962 - this can now have a combo box in use
			if(m_TestResultTypeCombo->GetCurSel() == NULL
				&& !m_TestResultTypeCombo->IsComboBoxTextInUse) {
				bIsValid = FALSE;
			}

			CString strResult = "";
			GetDlgItemText(IDC_EDIT_TEST_RESULT, strResult);
			strResult.TrimLeft();
			strResult.TrimRight();
			if(strResult.IsEmpty()) {
				bIsValid = FALSE;
			}
		}

	}NxCatchAll("Error in CInsuranceBilling::CheckTestResultFields");
}

// (j.jones 2008-05-14 17:25) - PLID 30044 - returns the value of the FormType setting
long CInsuranceBilling::GetFormTypeID()
{
	long nFormType = 1;

	try {
		
		if(m_FormTypeCombo->GetCurSel() != -1) {
			nFormType = VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0), 1);
		}

	}NxCatchAll("Error in CInsuranceBilling::GetFormTypeID");

	return nFormType;
}

// (j.gruber 2009-07-13 08:43) - PLID 34724 - store that we manually batched it
void CInsuranceBilling::OnBnClickedRadioUnbatched()
{
	try {
		((CBillingModuleDlg*)m_pBillingModuleWnd)->m_bManuallyUnbatched = TRUE;
	}NxCatchAll("Error in CInsuranceBilling::OnBnClickedRadioUnbatched()");

}

// (j.jones 2009-09-15 09:20) - PLID 35284 - added claim type combo
void CInsuranceBilling::OnSelChosenClaimTypeCombo(LPDISPATCH lpRow)
{
	try {

		// (j.jones 2011-09-16 15:40) - PLID 39790 - If they changed the claim type
		// to be non-Original, auto-fill the Original Reference Number with the Bill ID,
		// unless it is non-empty. Do not clear this if they revert to original.
		// Do nothing if this is a new bill.

		ANSI_ClaimTypeCode eCode = ctcOriginal;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_ClaimTypeCombo->SetSelByColumn(ctccID, (long)ctcOriginal);
		}
		else {
			eCode = (ANSI_ClaimTypeCode)VarLong(pRow->GetValue(ctccID), (long)ctcOriginal);
		}

		long nBillID = GetBillID();

		if(nBillID != -1 && eCode != ctcOriginal && !UseAlbertaHLINK()) {
			//they changed the claim type on an existing bill, and they are not using Alberta claims,
			//so copy the bill ID to the Original Reference Number field, if it is empty

			CString strOriginalRefNo = "";
			GetDlgItemText(IDC_EDIT_REFERENCE_NUMBER, strOriginalRefNo);
			strOriginalRefNo.TrimLeft(); strOriginalRefNo.TrimRight();
			
			if(strOriginalRefNo.IsEmpty()) {
				strOriginalRefNo.Format("%li", nBillID);
				SetDlgItemText(IDC_EDIT_REFERENCE_NUMBER, strOriginalRefNo);
			}
		}

	}NxCatchAll("Error in CInsuranceBilling::OnSelChosenClaimTypeCombo()");
}

// (j.jones 2010-03-02 13:40) - PLID 37584 - added prior. auth. type combo
void CInsuranceBilling::OnSelChosenPriorAuthTypeCombo(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_PriorAuthTypeCombo->SetSelByColumn(patccID, (long)patPriorAuthNumber);
		}

	}NxCatchAll("Error in CInsuranceBilling::OnSelChosenPriorAuthTypeCombo()");
}


// (j.jones 2010-06-10 10:38) - PLID 38507 - added HCFA Box 13 config
void CInsuranceBilling::OnSelChosenHCFABox13Combo(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_HCFABox13Combo->SetSelByColumn(hb13ccID, (long)hb13_UseDefault);
		}

	}NxCatchAll("Error in CInsuranceBilling::OnSelChosenHCFABox13Combo()");
}

// (j.jones 2010-06-14 13:39) - PLID 38507 - tells the caller whether we need to warn
// that HCFA Box 13 setting is in use, and provides the value to warn about
BOOL CInsuranceBilling::NeedWarnHCFABox13(HCFABox13Over &hb13_CurValue)
{
	//get the cur value
	hb13_CurValue = hb13_UseDefault;
	NXDATALIST2Lib::IRowSettingsPtr pBox23Row = m_HCFABox13Combo->GetCurSel();
	if(pBox23Row) {
		hb13_CurValue = (HCFABox13Over)VarLong(pBox23Row->GetValue(hb13ccID));
	}

	//we need to warn if the value is not to use the default,
	//and this is a new bill or the previous setting was to use the default
	if(hb13_CurValue != hb13_UseDefault &&
		(GetBillID() == -1 || m_eOldHCFABox13Over != hb13_CurValue)) {

		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (j.jones 2010-08-17 11:40) - PLID 40135 - If there are two insured parties selected and they
// have different categories (ie. one is Medical, one is Vision), warn the user and ask if they
// wish to continue. Return FALSE if they said no.
BOOL CInsuranceBilling::CheckWarnMismatchedInsuranceCategories()
{
	try {
		// (a.walling 2011-06-01 12:30) - PLID 43922 - This function was inadvertently using the list1 cursel as a list2 index, causing
		// errors in some offices. I ended up using a structure to get rid of duplicated code and ensure safety between the two lists,
		// as well as waiting for the list to requery to avoid the nxdatalist1 race condition

		struct InsuranceInfo
		{
			InsuranceInfo(NXDATALISTLib::_DNxDataListPtr pList)
				: nID(-1)
				, eType(rctInvalidRespCategory)
			{
				pList->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);

				long nCurSel = pList->CurSel;
				if (nCurSel == -1) return;

				NXDATALISTLib::IRowSettingsPtr pRow = pList->GetRow(nCurSel);
				if (!pRow) return;

				nID = VarLong(pRow->GetValue(ipccID), -1);
				
				long nPriority = VarLong(pRow->GetValue(ipccRespTypePriority), -1);

				//if inactive or invalid, leave the insurance type as invalid, we always let them match
				//inactive with anything (inactive is currently Medical in data, but treated as having
				//no category in the interface)
				if(nID != -1 && nPriority != -1) {
					eType = (RespCategoryType)VarLong(pRow->GetValue(ipccCategoryTypeID), (long)rctInvalidRespCategory);
					strName = VarString(pRow->GetValue(ipccName), "");
					strTypeName = VarString(pRow->GetValue(ipccCategoryTypeName), "");
				}
			};

			long nID;
			RespCategoryType eType;
			CString strName;
			CString strTypeName;
		};
		
		InsuranceInfo ins1(m_pInsurance1);
		InsuranceInfo ins2(m_pInsurance2);

		//warn if the companies have changed, there are two insured parties selected, and they have mismatched types
		if(ins1.nID != -1 && ins2.nID != -1
			&& (GetBillID() == -1 || ins1.nID != m_nOldInsuranceID1 || ins2.nID != m_nOldInsuranceID2)
			&& ins1.eType != rctInvalidRespCategory && ins2.eType != rctInvalidRespCategory
			&& ins1.eType != ins2.eType)
		{
			CString strWarn;

			//yes, we intentionally use both the word 'categories' and 'types'
			strWarn.Format("You have selected two insured parties with different insurance responsibility categories:\n\n"
				"%s - %s\n"
				"%s - %s\n\n"
				"Are you sure you wish to save this bill with these different insurance types?",
				ins1.strName, ins1.strTypeName, ins2.strName, ins2.strTypeName);

			if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}
			else {
				return TRUE;
			}
		}
		else {
			//either nothing changed, two active companies aren't selected,
			//or simply nothing is wrong, so return TRUE to continue
			return TRUE;
		}

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (r.gonet 2016-04-07) - NX-100072 - Sets the condition date type in the ClaimDates structure and user interface.
void CInsuranceBilling::SetConditionDateType(ConditionDateType eConditionDateType)
{
	m_claimDates.eConditionDateType = eConditionDateType;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_ConditionDateTypeCombo->SetSelByColumn(cdtccID, (long)eConditionDateType);
	if (pRow == nullptr) {
		pRow = m_ConditionDateTypeCombo->SetSelByColumn(cdtccID, (long)ConditionDateType::cdtFirstVisitOrConsultation444);
	}

	if (m_claimDates.GetFirstConditionDate() != g_cdtNull) {
		m_pIllness->SetDateTime(m_claimDates.GetFirstConditionDate());
	} else {
		m_pIllness->Clear();
	}
}

// (r.gonet 2016-04-07) - NX-100072 - Sets the First Condition Date in the ClaimDates structure and user interface.
void CInsuranceBilling::SetFirstConditionDate(COleDateTime dt)
{
	m_claimDates.SetFirstConditionDate(dt);
	m_pIllness->SetDateTime(dt);
}

// (r.gonet 2016-04-07) - NX-100072 - Clears the First Condition Date in the ClaimDates structure and user interface.
void CInsuranceBilling::ClearFirstConditionDate()
{
	m_claimDates.SetFirstConditionDate(g_cdtNull);
	m_pIllness->Clear();
}

// (r.gonet 2016-04-07) - NX-100072 - Clears all additional claim dates.
void CInsuranceBilling::ClearAdditionalClaimDates()
{
	m_claimDates.Clear();
	m_pIllness->Clear();
}

// (j.jones 2012-01-23 15:49) - PLID 47731 - added condition date combo
void CInsuranceBilling::OnSelChangingConditionDateCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-01-23 15:49) - PLID 47731 - added condition date combo
void CInsuranceBilling::OnSelChosenConditionDateCombo(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			pRow = m_ConditionDateTypeCombo->SetSelByColumn(cdtccID, (long)cdtFirstVisitOrConsultation444);
		}

		// (r.gonet 2016-04-07) - NX-100072 - Sync up the ClaimDates structure.
		if (pRow != NULL) {
			m_claimDates.eConditionDateType = (ConditionDateType)VarLong(pRow->GetValue(ConditionDateTypeComboColumns::cdtccID));
		} else {
			m_claimDates.eConditionDateType = ConditionDateType::cdtFirstVisitOrConsultation444;
		}

		// (r.gonet 2016-04-07) - NX-100072 - Load the First Condition Date from the ClaimDates structure.
		if (m_claimDates.GetFirstConditionDate() != g_cdtNull) {
			m_pIllness->SetDateTime(m_claimDates.GetFirstConditionDate());
		} else {
			m_pIllness->Clear();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-07 15:29) - PLID 41479 - added admission time
void CInsuranceBilling::OnKillFocusEditAdmissionTime()
{
	try {

		//ensure it's a valid time

		if(m_pAdmissionTime->GetStatus() == 3) {
			//blank, which is acceptable
			return;
		}

		if(m_pAdmissionTime->GetStatus() == 2) {
			MessageBox("The Admission Time you entered is invalid.");
			m_pAdmissionTime->Clear();
			return;
		}

		COleDateTime dt;
		if(m_pAdmissionTime->GetStatus() == 1) {
			dt = m_pAdmissionTime->GetDateTime();
		}
		else {
			dt.SetStatus(COleDateTime::invalid);
		}

		if(dt.GetStatus() == COleDateTime::invalid) {
			MessageBox("The Admission Time you entered is invalid.");
			m_pAdmissionTime->Clear();
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-07 15:29) - PLID 41479 - added discharge time
void CInsuranceBilling::OnKillFocusEditDischargeTime()
{
	try {

		//ensure it's a valid time

		if(m_pDischargeTime->GetStatus() == 3) {
			//blank, which is acceptable
			return;
		}

		if(m_pDischargeTime->GetStatus() == 2) {
			MessageBox("The Discharge Time you entered is invalid.");
			m_pDischargeTime->Clear();
			return;
		}

		COleDateTime dt;
		if(m_pDischargeTime->GetStatus() == 1) {
			dt = m_pDischargeTime->GetDateTime();
		}
		else {
			dt.SetStatus(COleDateTime::invalid);
		}

		if(dt.GetStatus() == COleDateTime::invalid) {
			MessageBox("The Discharge Time you entered is invalid.");
			m_pDischargeTime->Clear();
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-08-07 12:04) - PLID 58042 - if a HCFA, and a company is selected,
//toggle the fields that need to change for the new/old HCFA form
void CInsuranceBilling::UpdateDisplayedHCFAFields(long nInsuredPartyID, long nBillID)
{
	try {

		if(m_FormTypeCombo->GetCurSel() == -1) {
			return;
		}
		long nFormID = VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0));
		//if a non-HCFA is selected, don't bother updating the controls
		if(nFormID != 1) {
			return;
		}

		// Determine whether we need to use the new HCFA. If the bill is saved,
		// the service dates currently saved might affect the form.
		// If they have unsaved changes to the dates, the unsaved changes won't
		// have an effect on the form (keep it simple).
		bool bUseNewHCFAForm = UseNewHCFAForm(nInsuredPartyID, nBillID);
		if(bUseNewHCFAForm) {
			m_nxstaticResubmissionCodeLabel.SetWindowText("Resubmission Code");
		}
		else {
			m_nxstaticResubmissionCodeLabel.SetWindowText("Medicaid Resubmission Code");
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-08-13 11:23) - PLID 57902 - added additional screen for obscure claim fields
void CInsuranceBilling::OnBtnClaimFields()
{
	try {

		if(m_FormTypeCombo->GetCurSel() == -1) {
			return;
		}
		long nFormID = VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0));
		//if not a HCFA or a UB, don't open this dialog
		if(nFormID != 1 && nFormID != 2) {
			return;
		}

		long nInsuredPartyID = -1;
		long nRow = m_pInsurance1->CurSel;
		if(nRow != -1 && m_pInsurance1->GetValue(nRow, ipccID).vt == VT_I4) {
			nInsuredPartyID = VarLong(m_pInsurance1->GetValue(nRow, ipccID));
		}

		//(j.camacho 2016-3-3) PLID 68501 - Moved logic with changes to UB04
		if (nFormID == 1)
		{
		CBillClaimFieldsDlg dlg(this);
		dlg.m_bReadOnly = (m_eHasAccess == batNoAccess);
		if(nFormID == 1) {
			//set to true if showing fields for the new HCFA
			dlg.m_bUseNewForm = UseNewHCFAForm(nInsuredPartyID, GetBillID());
		}
		else if(nFormID == 2) {
			//set to true if showing UB04 fields
			dlg.m_bUseNewForm = (GetUBFormType() == 1 ? true : false);
		}
		//fill our dialog with the current data
		dlg.m_strHCFABox8 = m_strHCFABox8;
		dlg.m_strHCFABox9b = m_strHCFABox9b;
		dlg.m_strHCFABox9c = m_strHCFABox9c;
		dlg.m_strHCFABox10d = m_strHCFABox10d;
		dlg.m_strHCFABox11bQual = m_strHCFABox11bQual;
		dlg.m_strHCFABox11b = m_strHCFABox11b;

		if(IDOK == dlg.DoModal() && dlg.m_bChanged) {
			//get the data back from the dialog
			m_strHCFABox8 = dlg.m_strHCFABox8;
			m_strHCFABox9b = dlg.m_strHCFABox9b;
			m_strHCFABox9c = dlg.m_strHCFABox9c;
			m_strHCFABox10d = dlg.m_strHCFABox10d;
			m_strHCFABox11bQual = dlg.m_strHCFABox11bQual;
			m_strHCFABox11b = dlg.m_strHCFABox11b;
		}
		}
		else if(nFormID == 2)
		{
			// (r.goldschmidt 2016-03-07 12:32) - PLID 68541 - get default dates
			COleDateTime dtDefaultDate, dtDefaultFrom, dtDefaultThrough;
			CalculateUB04DefaultDates(dtDefaultDate, dtDefaultFrom, dtDefaultThrough);

			// (a.walling 2016-03-09 10:48) - PLID 68555 - UB04 Enhancements - Insert default occurrence code on save or on dialog creation to defer calculated default date that might depend on bill information
			auto claimInfo = m_ub04ClaimInfo;
			if (!m_ub04DefaultOccurrenceCodeToApply.IsEmpty()) {
				claimInfo.occurrences.push_back({ m_ub04DefaultOccurrenceCodeToApply, dtDefaultDate });
			}

			// (r.goldschmidt 2016-03-11 17:00) - PLID 68585 - UB04 Enhancements - insert default remarks on dialog creation
			if (!m_ub04DefaultRemarksToApply.IsEmpty()) {
				claimInfo.remarks = m_ub04DefaultRemarksToApply;
			}

			// (a.walling 2016-03-10 14:51) - PLID 68561 - UB04 Enhancements - all UB handling in CUB04AdditionalFieldsDlg
			auto formType = GetUBFormType();

			//(j.camacho 2016-3-3) PLID 68501
			CUB04AdditionalFieldsDlg dlg(this, claimInfo, formType, m_strUB92Box79);
			dlg.SetReadOnly(m_eHasAccess == batNoAccess); // (r.goldschmidt 2016-03-03 13:08) - PLID 68511
			dlg.SetDefaultDates(dtDefaultDate, dtDefaultFrom, dtDefaultThrough); // (r.goldschmidt 2016-03-08 11:52) - PLID 68497
			
			if (IDOK == dlg.DoModal()) {
				m_ub04ClaimInfo = dlg.GetClaimInfo();
				m_ub04DefaultOccurrenceCodeToApply = "";
				m_ub04DefaultRemarksToApply = "";
				if (formType == eUB92) {
					m_strUB92Box79 = dlg.GetUB92Box79();
				}
			}
		}
		else
		{
			//shouldn't be possible
			ASSERT(FALSE);
		}
			

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 07/02/2014) - PLID 62567 - Gets the bill description edit box value, with the status prefix.
CString CInsuranceBilling::GetBillDescriptionWithPrefix()
{
	return ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetBillDescriptionWithPrefix();
}


// (b.eyers 2015-07-14) - PLID 47739 - both billingdlg and insurancebilling call this code, moved to own function
void CInsuranceBilling::UpdateInsuranceDates()
{
	try {
		//on new bills, check the fill preferences for the dates on the insurance tab
		long nDefAccDate = GetRemotePropertyInt("DefaultCurrentAccidentToLastBillWhenG2Empty", 0, 0, "<None>", TRUE); //m_pAccident
		long nFirstVisDate = GetRemotePropertyInt("DefaultFirstAccidentToLastBillWhenG2Empty", 0, 0, "<None>", TRUE); //m_pIllness
		long nHospDate = GetRemotePropertyInt("DefaultHospDatesToLastBill", 0, 0, "<None>", TRUE); //m_pHospFrom; m_pHospTo; m_pAdmissionTime; m_pDischargeTime
		long nNoWorkDate = GetRemotePropertyInt("DefaultNoWorkDatesToLastBill", 0, 0, "<None>", TRUE); //m_pWorkFrom; m_pWorkTo
		if (GetBillID() == -1 && (nDefAccDate == 1 || nFirstVisDate == 1 || nHospDate == 1 || nNoWorkDate == 1)) {
			// (r.gonet 2016-04-07) - NX-100072 - Split out the FirstConditionDate into multiple date fields.
			_RecordsetPtr rsLastBill = CreateParamRecordset("SELECT TOP 1 ConditionDate, "
				"CASE BillsT.ConditionDateType "
				"	WHEN {CONST_INT} THEN BillsT.FirstVisitOrConsultationDate "
				"	WHEN {CONST_INT} THEN BillsT.InitialTreatmentDate "
				"	WHEN {CONST_INT} THEN BillsT.LastSeenDate "
				"	WHEN {CONST_INT} THEN BillsT.AcuteManifestationDate "
				"	WHEN {CONST_INT} THEN BillsT.LastXRayDate "
				"	WHEN {CONST_INT} THEN BillsT.HearingAndPrescriptionDate "
				"	WHEN {CONST_INT} THEN BillsT.AssumedCareDate "
				"	WHEN {CONST_INT} THEN BillsT.RelinquishedCareDate "
				"	WHEN {CONST_INT} THEN BillsT.AccidentDate "
				"	ELSE NULL "
				"END AS FirstConditionDate, "
				"ConditionDateType, HospFrom, HospTo, "
				"AdmissionTime, DischargeTime, NoWorkFrom, NoWorkTo FROM BillsT "
				"WHERE Deleted = 0 AND PatientID = {INT} AND EntryType = 1"
				"ORDER BY Date DESC, ID DESC", 
				ConditionDateType::cdtFirstVisitOrConsultation444,
				ConditionDateType::cdtInitialTreatmentDate454,
				ConditionDateType::cdtLastSeenDate304,
				ConditionDateType::cdtAcuteManifestation453,
				ConditionDateType::cdtLastXray455,
				ConditionDateType::cdtHearingAndPrescription471,
				ConditionDateType::cdtAssumedCare090,
				ConditionDateType::cdtRelinquishedCare91,
				ConditionDateType::cdtAccident439,
				m_nPatientID);
			if (!rsLastBill->eof) {

				//date of current illness/accident
				if (nDefAccDate == 1 && m_pAccident->GetStatus() == 3) {
					_variant_t var = rsLastBill->Fields->Item["ConditionDate"]->Value;
					if (var.vt == VT_DATE) {
						m_pAccident->SetDateTime(VarDateTime(var));
					}
				}

				//first visit or consultation is default, it's a dropdown that has different options
				// (r.gonet 2016-04-07) - NX-100072 - Load the first condition date into the ClaimDates structure and UI.
				if (nFirstVisDate == 1 && m_claimDates.GetFirstConditionDate() == g_cdtNull) {
					_variant_t varFirstConditionDate = rsLastBill->Fields->Item["FirstConditionDate"]->Value;
					if (varFirstConditionDate.vt == VT_DATE) {
						COleDateTime dtFirstConditionDate = VarDateTime(varFirstConditionDate);

						_variant_t varConditionDateType = rsLastBill->Fields->GetItem("ConditionDateType")->Value;
						SetConditionDateType((ConditionDateType)VarLong(varConditionDateType, (long)ConditionDateType::cdtFirstVisitOrConsultation444));
						
						SetFirstConditionDate(dtFirstConditionDate);
					}
				}

				//hospitalized date/times
				if (nHospDate == 1 && m_pHospFrom->GetStatus() == 3 && m_pHospTo->GetStatus() == 3 && m_pAdmissionTime->GetStatus() == 3 && m_pDischargeTime->GetStatus() == 3) {
					_variant_t var = rsLastBill->Fields->Item["HospFrom"]->Value;
					if (var.vt == VT_DATE)
						m_pHospFrom->SetDateTime(VarDateTime(var));
					var = rsLastBill->Fields->Item["HospTo"]->Value;
					if (var.vt == VT_DATE)
						m_pHospTo->SetDateTime(VarDateTime(var));
					var = rsLastBill->Fields->Item["AdmissionTime"]->Value;
					if (var.vt == VT_DATE)
						m_pAdmissionTime->SetDateTime(VarDateTime(var));
					var = rsLastBill->Fields->Item["DischargeTime"]->Value;
					if (var.vt == VT_DATE)
						m_pDischargeTime->SetDateTime(VarDateTime(var));
				}

				//no work dates
				if (nNoWorkDate == 1 && m_pWorkFrom->GetStatus() == 3 && m_pWorkTo->GetStatus() == 3) {
					_variant_t var = rsLastBill->Fields->Item["NoWorkFrom"]->Value;
					if (var.vt == VT_DATE)
						m_pWorkFrom->SetDateTime(VarDateTime(var));
					var = rsLastBill->Fields->Item["NoWorkTo"]->Value;
					if (var.vt == VT_DATE)
						m_pWorkTo->SetDateTime(VarDateTime(var));
				}
			}
			rsLastBill->Close();
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-03-07 12:32) - PLID 68541 - get default dates for UB04 additional fields dialog
void CInsuranceBilling::CalculateUB04DefaultDates(COleDateTime& dtDefaultDate, COleDateTime& dtDefaultFrom, COleDateTime& dtDefaultThrough) 
{
	// reset all dates to invalid
	dtDefaultDate = g_cdtInvalid;
	dtDefaultFrom = g_cdtInvalid;
	dtDefaultThrough = g_cdtInvalid;

	// get min and max dates from billing dlg
	((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.GetMinAndMaxChargeDates(dtDefaultFrom, dtDefaultThrough);
	dtDefaultDate = dtDefaultFrom;

	// if supposed to use the condition date
	if (m_bUseConditionDate) {
		// and if there is a valid condition date
		if (m_pAccident->GetStatus() == 1) {
			// (r.goldschmidt 2016-03-14 14:47) - PLID 68541 - set all the default dates to the condition date instead
			dtDefaultDate = m_pAccident->GetDateTime();
			dtDefaultFrom = m_pAccident->GetDateTime();
			dtDefaultThrough = m_pAccident->GetDateTime();
		}
	}

	// (r.goldschmidt 2016-03-14 14:47) - PLID 68541 - set all the default dates; default date
	if (dtDefaultDate.GetStatus() != COleDateTime::valid) {
		// use the bill date if nothing else
		dtDefaultDate = ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_date.GetDateTime();

		// and the current date, though this is not supposed to ever be hit
		if (dtDefaultDate.GetStatus() != COleDateTime::valid) {
			ASSERT(FALSE);
			dtDefaultDate = COleDateTime::GetCurrentTime();
		}
	}

	// (r.goldschmidt 2016-03-14 14:47) - PLID 68541 - set all the default dates; from date
	if (dtDefaultFrom.GetStatus() != COleDateTime::valid) {
		// use the bill date if nothing else
		dtDefaultFrom = ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_date.GetDateTime();

		// and the current date, though this is not supposed to ever be hit
		if (dtDefaultFrom.GetStatus() != COleDateTime::valid) {
			ASSERT(FALSE);
			dtDefaultFrom = COleDateTime::GetCurrentTime();
		}
	}

	// (r.goldschmidt 2016-03-14 14:47) - PLID 68541 - set all the default dates; through date
	if (dtDefaultThrough.GetStatus() != COleDateTime::valid) {
		// use the bill date if nothing else
		dtDefaultThrough = ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_date.GetDateTime();

		// and the current date, though this is not supposed to ever be hit
		if (dtDefaultThrough.GetStatus() != COleDateTime::valid) {
			ASSERT(FALSE);
			dtDefaultThrough = COleDateTime::GetCurrentTime();
		}
	}

	// sanity check -- no time components
	dtDefaultDate = AsDateNoTime(dtDefaultDate);
	dtDefaultFrom = AsDateNoTime(dtDefaultFrom);
	dtDefaultThrough = AsDateNoTime(dtDefaultThrough);
}


// (s.tullis 2016-03-14 10:11) - PLID 68319 
long CInsuranceBilling::GetPrimaryInsuredPartyID()
{
	long nID = -1;
	try {
		long nCurSel = m_pInsurance1->CurSel;
		if (nCurSel != -1) {
			NXDATALISTLib::IRowSettingsPtr pRow = m_pInsurance1->GetRow(nCurSel);
			if (pRow) {
				nID = VarLong(pRow->GetValue(ipccID), -1);
			}
		}

	}NxCatchAll(__FUNCTION__)
	return nID;
}

// (b.eyers 2016-04-07) - NX-100071 - Open the additional claim dates dialog
void CInsuranceBilling::OnBtnEditClaimDates()
{
	try {
		//open dialog
		CInsuranceClaimDatesDlg dlg(this, m_claimDates);
		dlg.m_bReadOnly = (m_eHasAccess == batNoAccess);
		if (IDOK == dlg.DoModal()) {
			//if they closed out of the dialog with 'ok' then update the date for the visible date
			if (m_claimDates.GetFirstConditionDate() != g_cdtNull) {
				m_pIllness->SetDateTime(m_claimDates.GetFirstConditionDate());
			}
			else {
				m_pIllness->Clear();
			}
		}

	}NxCatchAll("Error in CInsuranceBilling::OnBtnEditClaimDates()");
}

// (j.jones 2016-05-24 14:39) - NX-100704 - added a function to repopulate the claim type
// combo based on the current form type
void CInsuranceBilling::RepopulateClaimTypeCombo()
{
	try {

		//try to preserve the same selection
		ANSI_ClaimTypeCode eOldClaimTypeCode = ctcOriginal;
		NXDATALIST2Lib::IRowSettingsPtr pClaimTypeRow = m_ClaimTypeCombo->GetCurSel();
		if (pClaimTypeRow) {
			eOldClaimTypeCode = (ANSI_ClaimTypeCode)VarLong(pClaimTypeRow->GetValue(ctccID));
		}

		m_ClaimTypeCombo->Clear();

		long nFormType = 1;
		if (m_FormTypeCombo->GetCurSel() != -1) {
			nFormType = VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(), 0), 1);
		}

		AddRowToClaimTypeCombo(nFormType, ctcOriginal);
		AddRowToClaimTypeCombo(nFormType, ctcCorrected);
		AddRowToClaimTypeCombo(nFormType, ctcReplacement);
		AddRowToClaimTypeCombo(nFormType, ctcVoid);

		//reselect the same selection
		if (m_ClaimTypeCombo->SetSelByColumn(ctccID, (long)eOldClaimTypeCode) == NULL) {

			//Currently the codes are identical in every claim type, so you should never hit this code
			//unless you added a new code to one form that is not available in another form.
			//Remove this assert if you declare this is ok.
			ASSERT(FALSE);

			//reset to Original if the old selection no longer exists,
			//and trigger OnSelChosen since this is now changing the value
			OnSelChosenClaimTypeCombo(m_ClaimTypeCombo->SetSelByColumn(ctccID, (long)ctcOriginal));
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-05-24 14:44) - NX-100704 - helper function to build the claim type combo
void CInsuranceBilling::AddRowToClaimTypeCombo(long nFormType, ANSI_ClaimTypeCode ctcType)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_ClaimTypeCombo->GetNewRow();
	pRow->PutValue(ctccID, (long)ctcType);
	if (UseAlbertaHLINK()) {
		//Alberta
		pRow->PutValue(ctccCode, (LPCTSTR)GetClaimTypeCode_Alberta(ctcType));
		pRow->PutValue(ctccName, (LPCTSTR)GetClaimTypeDescription_Alberta(ctcType));
	}
	else if (nFormType == 2) {
		//UB
		pRow->PutValue(ctccCode, (LPCTSTR)GetClaimTypeCode_UB(ctcType));
		pRow->PutValue(ctccName, (LPCTSTR)GetClaimTypeDescription_UB(ctcType));
	}
	else {
		//HCFA
		pRow->PutValue(ctccCode, (LPCTSTR)GetClaimTypeCode_HCFA(ctcType));
		pRow->PutValue(ctccName, (LPCTSTR)GetClaimTypeDescription_HCFA(ctcType));
	}
	m_ClaimTypeCombo->AddRowAtEnd(pRow, NULL);
}
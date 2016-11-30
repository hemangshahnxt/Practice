// Billing2Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "Billing2Dlg.h"
#include "BillingDlg.h"
#include "GlobalDrawingUtils.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "BillingModuleDlg.h"
#include "AuditTrail.h"
#include "EditDischargeStatusDlg.h"
#include "BillingExtraChargeInfoDlg.h"
#include "MultiSelectDlg.h"
#include "OHIPUtils.h"
#include "BillingRc.h"
#include <NxSystemUtilitiesLib/NxThread.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CBilling2Dlg dialog

// (a.walling 2010-01-14 16:29) - PLID 36889 - All SW_SHOW calls here have been changed to SW_SHOWNOACTIVATE to stop messing with the focus

// (j.jones 2008-12-11 09:57) - PLID 32407 - added supervising provider column enum
enum SupervisingProviderComboColumns {

	spccID = 0,
	spccName,
};

// (j.jones 2009-08-10 16:16) - PLID 35142 - added case history combo
enum CaseHistoryComboColumns {

	chccID = 0,
	chccDescription,
	chccSurgeryDate,
	chccCompletedDate,
	chccTotalPrice,
	chccTotalCost,
};



// (j.gruber 2011-09-28 14:08) - PLID 45358
enum AffilateStatusColumns {
	afscID = 0,
	afscName,
	afscType,
};



CBilling2Dlg::CBilling2Dlg(CWnd* pParent)
	: CNxDialog(CBilling2Dlg::IDD, pParent),
	// (j.jones 2007-05-02 15:24) - PLID 25883 - supported DischargeStatusT tablechecker
	m_DischargeStatusChecker(NetUtils::DischargeStatusT),
	// (j.jones 2008-12-11 09:50) - PLID 32407 - supported ProvidersT tablechecker
	m_ProviderChecker(NetUtils::Providers),
	// (j.gruber 2011-09-29 15:24) - PLID 45356 - referring physicians
	m_RefPhysChecker(NetUtils::RefPhys)	
{
	//{{AFX_DATA_INIT(CBilling2Dlg)
	m_boInitialized = FALSE;
	// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
	m_eHasAccess = batNoAccess;
	m_EntryType = BillEntryType::Bill;
	m_nCurTotalAnesthMinutes = 0;
	m_nCurTotalFacilityMinutes = 0;
	m_nCurTotalAssistingMinutes = 0;
	m_nOldSupervisingProviderID = -1;
	m_bIsSurgeryCenter = FALSE;
	m_bIsCaseHistoryComboHidden = FALSE;


	m_nOldAffiliatePhysID = -1;
	m_strOldAffiliatePhysName = "";
	m_nCurrentAffiliatePhysID = -1;
	m_strCurrentAffiliatePhysName = "";
	m_strOldAffiliateAmount = "";
	m_strOldAffiliateNote = "";
	m_nOldStatusType = -1;
	
	// (j.gruber 2011-09-28 15:50) - PLID 45358
	m_nOldAffiliateStatusID = -1;
	m_strOldAffiliateStatusDescription = "";

	m_nSavedInvoiceNumber = -1;
	m_nSavedInvoiceProviderID = -1;
	m_nCurrentInvoiceNumber = -1;
	m_nCurrentInvoiceProviderID = -1;
	//}}AFX_DATA_INIT
}


void CBilling2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBilling2Dlg)
	DDX_Control(pDX, IDC_SUPERVISING_PROVIDER_LABEL, m_nxstaticSupervisingProviderLabel);
	DDX_Control(pDX, IDC_BTN_EXTRA_CHARGE_INFO, m_btnExtraChargeInfo);
	DDX_Control(pDX, IDC_EDIT_ANESTH_HOURS, m_nxeditEditAnesthHours);
	DDX_Control(pDX, IDC_EDIT_ANESTH_MINUTES, m_nxeditEditAnesthMinutes);
	DDX_Control(pDX, IDC_EDIT_TOTAL_ANESTH_MINUTES, m_nxeditEditTotalAnesthMinutes);
	DDX_Control(pDX, IDC_EDIT_FACILITY_HOURS, m_nxeditEditFacilityHours);
	DDX_Control(pDX, IDC_EDIT_FACILITY_MINUTES, m_nxeditEditFacilityMinutes);
	DDX_Control(pDX, IDC_EDIT_TOTAL_FACILITY_MINUTES, m_nxeditEditTotalFacilityMinutes);
	DDX_Control(pDX, IDC_DEDUCT_LEFT_TO_MEET, m_nxeditDeductLeftToMeet);
	DDX_Control(pDX, IDC_PAT_CO_INSURANCE, m_nxeditPatCoInsurance);
	DDX_Control(pDX, IDC_OOP_LEFT_TO_MEET, m_nxeditOopLeftToMeet);
	DDX_Control(pDX, IDC_REWARD_POINTS_TOTAL_EDIT, m_nxeditRewardPointsTotalEdit);
	DDX_Control(pDX, IDC_DEDUCT_LEFT_TO_MEET_STATIC, m_nxstaticDeductLeftToMeetStatic);
	DDX_Control(pDX, IDC_PAT_CO_INSURANCE_STATIC, m_nxstaticPatCoInsuranceStatic);
	DDX_Control(pDX, IDC_OOP_LEFT_TO_MEET_STATIC, m_nxstaticOopLeftToMeetStatic);
	DDX_Control(pDX, IDC_REWARD_POINTS_TOTAL_STATIC, m_nxstaticRewardPointsTotalStatic);
	DDX_Control(pDX, IDC_CASE_HISTORY_BILL_LABEL, m_nxstaticCaseHistoryLabel);
	DDX_Control(pDX, IDC_CASE_HISTORY_LINK_LABEL, m_nxlCaseHistoryLinkLabel);
	DDX_Control(pDX, IDC_ASSISTING_TIME_LABEL1, m_nxstaticAssistingLabel1);
	DDX_Control(pDX, IDC_ASSISTING_TIME_LABEL2, m_nxstaticAssistingLabel2);
	DDX_Control(pDX, IDC_ASSISTING_TIME_LABEL3, m_nxstaticAssistingLabel3);
	DDX_Control(pDX, IDC_ASSISTING_TIME_LABEL4, m_nxstaticAssistingLabel4);
	DDX_Control(pDX, IDC_ASSISTING_TIME_LABEL5, m_nxstaticAssistingLabel5);
	DDX_Control(pDX, IDC_ASSISTING_TIME_LABEL6, m_nxstaticAssistingLabel6);
	DDX_Control(pDX, IDC_ASSISTING_TIME_LABEL7, m_nxstaticAssistingLabel7);
	DDX_Control(pDX, IDC_EDIT_ASSISTING_HOURS, m_nxeditEditAssistingHours);
	DDX_Control(pDX, IDC_EDIT_ASSISTING_MINUTES, m_nxeditEditAssistingMinutes);
	DDX_Control(pDX, IDC_EDIT_TOTAL_ASSISTING_MINUTES, m_nxeditEditTotalAssistingMinutes);
	DDX_Control(pDX, IDC_AFFIL_NOTES, m_nxeditAffiliatePhysNote); // (j.gruber 2012-10-17 13:05) - PLID 53227
	DDX_Control(pDX, IDC_LABEL_INVOICE_NUMBER, m_nxstaticLabelInvoiceNumber);
	DDX_Control(pDX, IDC_EDIT_INVOICE_NUMBER, m_nxeditInvoiceNumber);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBilling2Dlg, CNxDialog)
	//{{AFX_MSG_MAP(CBilling2Dlg)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_EDIT_ANESTH_HOURS, OnKillfocusEditAnesthHours)
	ON_EN_KILLFOCUS(IDC_EDIT_ANESTH_MINUTES, OnKillfocusEditAnesthMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_FACILITY_HOURS, OnKillfocusEditFacilityHours)
	ON_EN_KILLFOCUS(IDC_EDIT_FACILITY_MINUTES, OnKillfocusEditFacilityMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_TOTAL_ANESTH_MINUTES, OnKillfocusEditTotalAnesthMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_TOTAL_FACILITY_MINUTES, OnKillfocusEditTotalFacilityMinutes)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_EDIT_DISCHARGE_STATUS, OnBtnEditDischargeStatus)
	ON_EN_KILLFOCUS(IDC_DEDUCT_LEFT_TO_MEET, OnKillfocusDeductLeftToMeet)
	ON_EN_KILLFOCUS(IDC_OOP_LEFT_TO_MEET, OnKillfocusOopLeftToMeet)
	ON_EN_KILLFOCUS(IDC_PAT_CO_INSURANCE, OnKillfocusPatCoInsurance)
	ON_BN_CLICKED(IDC_BTN_EXTRA_CHARGE_INFO, OnBtnExtraChargeInfo)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	//}}AFX_MSG_MAP
	ON_EN_KILLFOCUS(IDC_AFFIL_AMT, OnEnKillfocusAffilAmt)
	ON_EN_KILLFOCUS(IDC_EDIT_ASSISTING_HOURS, OnEnKillfocusEditAssistingHours)
	ON_EN_KILLFOCUS(IDC_EDIT_ASSISTING_MINUTES, OnEnKillfocusEditAssistingMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_TOTAL_ASSISTING_MINUTES, OnEnKillfocusEditTotalAssistingMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_INVOICE_NUMBER, OnEnKillfocusEditInvoiceNumber)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBilling2Dlg message handlers

BOOL CBilling2Dlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	if(m_EntryType == 1) {
		m_nColor = 0x00FFC0C0;
	}
	else {
		m_nColor = RGB(255,179,128);
	}
	m_brush.CreateSolidBrush(PaletteColor(m_nColor));

	// (j.jones 2008-05-28 10:35) - PLID 28782 - added m_btnExtraChargeInfo
	m_btnExtraChargeInfo.AutoSet(NXB_MODIFY);

	// (j.jones 2013-07-10 15:14) - PLID 57148 - set the invoice number field
	// to be a maximum of 10 digits, which is how many digits are in the max long
	m_nxeditInvoiceNumber.SetLimitText(10);

	m_nxtAnesthStart = BindNxTimeCtrl(this, IDC_ANESTH_START_TIME);
	m_nxtAnesthEnd = BindNxTimeCtrl(this, IDC_ANESTH_END_TIME);
	m_nxtFacilityStart = BindNxTimeCtrl(this, IDC_FACILITY_START_TIME);
	m_nxtFacilityEnd = BindNxTimeCtrl(this, IDC_FACILITY_END_TIME);
	// (j.jones 2011-11-01 11:11) - PLID 41558 - added Assisting controls (for OHIP)
	m_nxtAssistingStart = BindNxTimeCtrl(this, IDC_ASSISTING_START_TIME);
	m_nxtAssistingEnd = BindNxTimeCtrl(this, IDC_ASSISTING_END_TIME);

	// (j.jones 2011-11-01 09:39) - PLID 41558 - removed the unnecessary anesth/facility hours & minutes lists

	// (j.jones 2006-11-22 10:06) - PLID 23371 - added discharge status
	m_DischargeStatusCombo = BindNxDataList2Ctrl(IDC_DISCHARGE_STATUS_COMBO);
	// (j.jones 2008-12-11 09:50) - PLID 32407 - added m_SupervisingProviderCombo
	m_SupervisingProviderCombo = BindNxDataList2Ctrl(IDC_SUPERVISING_PROV_COMBO);
	// (j.jones 2009-08-10 16:19) - PLID 35142 - added case history combo
	m_CaseHistoryCombo = BindNxDataList2Ctrl(IDC_CASE_HISTORY_BILL_COMBO, false);

	// (j.jones 2009-08-11 15:14) - PLID 35142 - initialize the hyperlink
	m_nxlCaseHistoryLinkLabel.SetText("");
	m_nxlCaseHistoryLinkLabel.SetType(dtsHyperlink);

	//add a "no provider" row
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_SupervisingProviderCombo->GetNewRow();
	pRow->PutValue(spccID, (long)-1);
	pRow->PutValue(spccName, (LPCSTR)" <No Provider Selected>");
	m_SupervisingProviderCombo->AddRowSorted(pRow, NULL);

	m_DischargeStatusCombo->PutCurSel(NULL);

	// (j.gruber 2011-09-28 14:13) - PLID 45358
	m_pAffiliateStatusList = BindNxDataList2Ctrl(IDC_AFFIL_STATUS, true);
	// (j.gruber 2011-09-28 14:14) - PLID 45356
	m_pAffiliatePhysList = BindNxDataList2Ctrl(IDC_BILL_AFFILIATE_LIST, true);
	((CEdit*)GetDlgItem(IDC_AFFIL_NOTES))->SetLimitText(255);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBilling2Dlg::ChangeAccess()
{
	// (j.jones 2011-01-21 10:10) - PLID 42156 - the access level is now an enum,
	// but every control is available even with partial access (you can change
	// anesthesia & facility times, but you will need permission for them to be
	// allowed to change charge prices)
	m_nxtAnesthStart->PutEnabled(m_eHasAccess != batNoAccess);
	m_nxtAnesthEnd->PutEnabled(m_eHasAccess != batNoAccess);
	m_nxtFacilityStart->PutEnabled(m_eHasAccess != batNoAccess);
	m_nxtFacilityEnd->PutEnabled(m_eHasAccess != batNoAccess);

	// (j.jones 2008-12-11 09:50) - PLID 32407 - added m_SupervisingProviderCombo
	m_SupervisingProviderCombo->PutEnabled(m_eHasAccess != batNoAccess);

	m_DischargeStatusCombo->PutEnabled(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_BTN_EDIT_DISCHARGE_STATUS)->EnableWindow(m_eHasAccess != batNoAccess);

	GetDlgItem(IDC_EDIT_ANESTH_HOURS)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_ANESTH_MINUTES)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_TOTAL_ANESTH_MINUTES)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_FACILITY_HOURS)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_FACILITY_MINUTES)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_TOTAL_FACILITY_MINUTES)->EnableWindow(m_eHasAccess != batNoAccess);

	// (j.jones 2011-11-01 11:11) - PLID 41558 - added Assisting controls (for OHIP)
	GetDlgItem(IDC_EDIT_ASSISTING_HOURS)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_ASSISTING_MINUTES)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_EDIT_TOTAL_ASSISTING_MINUTES)->EnableWindow(m_eHasAccess != batNoAccess);
	m_nxtAssistingStart->PutEnabled(m_eHasAccess != batNoAccess);
	m_nxtAssistingEnd->PutEnabled(m_eHasAccess != batNoAccess);

	GetDlgItem(IDC_DEDUCT_LEFT_TO_MEET)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_PAT_CO_INSURANCE)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_OOP_LEFT_TO_MEET)->EnableWindow(m_eHasAccess != batNoAccess);

	// (j.jones 2009-08-10 16:19) - PLID 35142 - added case history combo
	m_CaseHistoryCombo->PutEnabled(m_eHasAccess != batNoAccess);
	m_nxlCaseHistoryLinkLabel.SetType(m_eHasAccess != batNoAccess ? dtsHyperlink : dtsDisabledHyperlink);
	m_nxlCaseHistoryLinkLabel.Invalidate();

	// (j.gruber 2011-09-28 14:15) - PLID 45358
	//TES 10/30/2011 - PLID 46155 - These fields also require bioBillAffiliatePhysician permission
	BOOL bHasAffiliatePerm = (GetCurrentUserPermissions(bioBillAffiliatePhysician) && SPT___W_______);
	m_pAffiliateStatusList->PutEnabled(m_eHasAccess != batNoAccess && bHasAffiliatePerm);
	//this is always disabled
	//changed to be read only
	((CEdit*)(GetDlgItem(IDC_AFFIL_DATE)))->SetReadOnly(TRUE);

	// (j.gruber 2011-09-28 14:15) - PLID 45356
	m_pAffiliatePhysList->PutEnabled(m_eHasAccess != batNoAccess && bHasAffiliatePerm);
	GetDlgItem(IDC_AFFIL_AMT)->EnableWindow(m_eHasAccess != batNoAccess && bHasAffiliatePerm);
	((CEdit*)(GetDlgItem(IDC_AFFIL_NOTES)))->SetReadOnly(!(m_eHasAccess != batNoAccess && bHasAffiliatePerm));

	// (j.jones 2013-07-10 15:14) - PLID 57148 - added the invoice number field,
	// it will be enabled if they have access, and if it's currently editable
	TryEnableInvoiceNumber();
}

void CBilling2Dlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);
	if (bShow == FALSE) return;

	if (m_boInitialized) {

		// (j.jones 2009-08-11 16:54) - PLID 35142 - update the case history display every time we enter the tab
		ToggleCaseHistoryDisplay();
		return;
	}
	
	if(m_EntryType == 2) {
		((CNxColor*)GetDlgItem(IDC_BILLING_2_COLOR_1))->SetColor(m_nColor);

		// (j.gruber 2006-12-29 14:34) - PLID 23972 - show the quote fields that are only for insurance
		GetDlgItem(IDC_DEDUCT_LEFT_TO_MEET)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_DEDUCT_LEFT_TO_MEET_STATIC)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PAT_CO_INSURANCE)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PAT_CO_INSURANCE_STATIC)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_PERCENT_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_OOP_LEFT_TO_MEET)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_OOP_LEFT_TO_MEET_STATIC)->ShowWindow(SW_SHOWNOACTIVATE);

		// (j.jones 2008-05-28 11:00) - PLID 28782 - the "extra charge info" is not used on quotes
		GetDlgItem(IDC_BTN_EXTRA_CHARGE_INFO)->ShowWindow(SW_HIDE);

		// (j.jones 2008-12-11 10:51) - PLID 32407 - the Supervising Provider is not used on quotes
		GetDlgItem(IDC_SUPERVISING_PROVIDER_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SUPERVISING_PROV_COMBO)->ShowWindow(SW_HIDE);

		// (j.gruber 2010-08-04 13:34) - PLID 39949 - pull defaults from primary insurance
		if (GetBillID() == -1) {
			_RecordsetPtr rsDeduct = CreateParamRecordset("SELECT DeductibleRemaining, OOPRemaining FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID = 1", ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID);
			if (!rsDeduct->eof) {
				SetDlgItemText(IDC_DEDUCT_LEFT_TO_MEET, AsString(rsDeduct->Fields->Item["DeductibleRemaining"]->Value));
				SetDlgItemText(IDC_OOP_LEFT_TO_MEET, AsString(rsDeduct->Fields->Item["OOPRemaining"]->Value));
			}
		}

		// (j.gruber 2011-09-28 14:17) - PLID 45358 - only show on bills
		GetDlgItem(IDC_AFFIL_STATUS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFFIL_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFFIL_STATUS_LABEL)->ShowWindow(SW_HIDE);

		// (j.gruber 2011-09-28 14:15) - PLID 45356
		GetDlgItem(IDC_BILL_AFFILIATE_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFFIL_LIST_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFFIL_AMT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFFIL_AMT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFFIL_NOTES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFFIL_NOTE_LABEL)->ShowWindow(SW_HIDE);

	}
	else {
		GetDlgItem(IDC_DEDUCT_LEFT_TO_MEET)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DEDUCT_LEFT_TO_MEET_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PAT_CO_INSURANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PERCENT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PAT_CO_INSURANCE_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OOP_LEFT_TO_MEET)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OOP_LEFT_TO_MEET_STATIC)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_BTN_EXTRA_CHARGE_INFO)->ShowWindow(SW_SHOWNOACTIVATE);

		GetDlgItem(IDC_SUPERVISING_PROVIDER_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_SUPERVISING_PROV_COMBO)->ShowWindow(SW_SHOWNOACTIVATE);

		
		// (j.gruber 2011-09-28 14:17) - PLID 45358 - only show on bills
		GetDlgItem(IDC_AFFIL_STATUS)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_AFFIL_DATE)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_AFFIL_STATUS_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);

		// (j.gruber 2011-09-28 14:15) - PLID 45356
		GetDlgItem(IDC_BILL_AFFILIATE_LIST)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_AFFIL_LIST_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_AFFIL_AMT)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_AFFIL_AMT_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_AFFIL_NOTES)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_AFFIL_NOTE_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
	}

	// (a.walling 2007-05-24 08:55) - PLID 26114;
	if (m_EntryType == 1 && g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
		GetDlgItem(IDC_REWARD_POINTS_TOTAL_STATIC)->SetWindowText("Patient's Total Reward Points");
		GetDlgItem(IDC_REWARD_POINTS_TOTAL_STATIC)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_REWARD_POINTS_TOTAL_EDIT)->ShowWindow(SW_SHOWNOACTIVATE);
	} else {
		GetDlgItem(IDC_REWARD_POINTS_TOTAL_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REWARD_POINTS_TOTAL_EDIT)->ShowWindow(SW_HIDE);
	}

	// (j.jones 2009-08-10 16:19) - PLID 35142 - only show case histories on bills if we have the surgery center license
	m_bIsSurgeryCenter = IsSurgeryCenter(false);
	if (m_EntryType == 1 && m_bIsSurgeryCenter) {		
		GetDlgItem(IDC_CASE_HISTORY_BILL_LABEL)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_CASE_HISTORY_BILL_COMBO)->ShowWindow(SW_SHOWNOACTIVATE);
		//always hide the link label, let the loading handle that part
		ShowDlgItem(IDC_CASE_HISTORY_LINK_LABEL, SW_HIDE);
		m_bIsCaseHistoryComboHidden = FALSE;

		//now load the contents
		long nPatientID = ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID;
		CString strListSQL;
		//show all case histories whether they are completed or not,
		//include the total cost from all line items,
		//and the total price of all billable pay to practice line items
		
		//Do NOT show the total cost if they cannot see contact costs, and those fees exist.
		BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

		//GetBillID() may be -1, and that is ok in this query
		// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
		// (j.jones 2011-03-28 16:38) - PLID 42575 - ensured we referenced CaseHistoryDetailsT.Billable
		strListSQL.Format("(SELECT CaseHistoryT.ID, PersonID, Name, SurgeryDate, CompletedDate, "
			//if the case has a person in it, hide the cost if the user has no permission
			"(CASE WHEN SUM(CASE WHEN ItemType = -3 THEN 1 ELSE 0 END) > 0 AND %li=0 THEN NULL "
			"ELSE "
			"Sum(Round(Convert(money,Cost*Quantity),2)) "
			"END) AS TotalCost, "				
			"Sum(Round(Convert(money, "
				"CASE WHEN CaseHistoryDetailsT.Billable = 1 THEN Amount*Quantity ELSE 0 END "
				"),2)) AS TotalPrice "
			"FROM CaseHistoryT "
			"INNER JOIN CaseHistoryDetailsT ON CaseHistoryT.ID = CaseHistoryDetailsT.CaseHistoryID "
			"WHERE PersonID = %li "
			"GROUP BY CaseHistoryT.ID, PersonID, Name, SurgeryDate, CompletedDate) AS CaseHistoryQ",
			bCanViewPersonCosts ? 1 : 0, nPatientID);

		m_CaseHistoryCombo->FromClause = _bstr_t(strListSQL);
		m_CaseHistoryCombo->Requery();
		m_CaseHistoryCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		//add a "multiple" row
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_CaseHistoryCombo->GetNewRow();
			pRow->PutValue(chccID, (long)-2);
			pRow->PutValue(chccDescription, _bstr_t(" <Multiple Case Histories>"));
			pRow->PutValue(chccSurgeryDate, g_cvarNull);
			pRow->PutValue(chccCompletedDate, g_cvarNull);
			pRow->PutValue(chccTotalPrice, g_cvarNull);
			pRow->PutValue(chccTotalCost, g_cvarNull);
			m_CaseHistoryCombo->AddRowBefore(pRow, m_CaseHistoryCombo->GetFirstRow());
		}

		//add a "none selected" row
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_CaseHistoryCombo->GetNewRow();
			pRow->PutValue(chccID, (long)-1);
			pRow->PutValue(chccDescription, _bstr_t(" <No Case History>"));
			pRow->PutValue(chccSurgeryDate, g_cvarNull);
			pRow->PutValue(chccCompletedDate, g_cvarNull);
			pRow->PutValue(chccTotalPrice, g_cvarNull);
			pRow->PutValue(chccTotalCost, g_cvarNull);
			m_CaseHistoryCombo->AddRowBefore(pRow, m_CaseHistoryCombo->GetFirstRow());
			m_CaseHistoryCombo->PutCurSel(pRow);
		}

		//and now set our selection
		ToggleCaseHistoryDisplay();
	}
	else {
		GetDlgItem(IDC_CASE_HISTORY_BILL_LABEL)->ShowWindow(SW_HIDE);
		ShowDlgItem(IDC_CASE_HISTORY_LINK_LABEL, SW_HIDE);
		GetDlgItem(IDC_CASE_HISTORY_BILL_COMBO)->ShowWindow(SW_HIDE);		
	}

	// (j.jones 2011-11-01 09:56) - PLID 41558 - if not OHIP, hide the Assisting controls
	if(!UseOHIP()) {
		GetDlgItem(IDC_ASSISTING_TIME_LABEL1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSISTING_TIME_LABEL2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSISTING_TIME_LABEL3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSISTING_TIME_LABEL4)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSISTING_TIME_LABEL5)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSISTING_TIME_LABEL6)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSISTING_TIME_LABEL7)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_ASSISTING_HOURS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_ASSISTING_MINUTES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_TOTAL_ASSISTING_MINUTES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSISTING_START_TIME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSISTING_END_TIME)->ShowWindow(SW_HIDE);
	}

	// (j.jones 2007-05-02 15:24) - PLID 25883 - supported DischargeStatusT tablechecker
	if (m_DischargeStatusChecker.Changed()) {

		//save the current value
		long nCurID = -1;
		if(m_DischargeStatusCombo->GetCurSel() != NULL) {
			nCurID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_DischargeStatusCombo->GetCurSel())->GetValue(0), -1);
		}

		m_DischargeStatusCombo->Requery();

		//now set the old value, if it exists
		if(nCurID != -1) {
			m_DischargeStatusCombo->SetSelByColumn(0, nCurID);
		}
	}

	// (j.jones 2008-12-11 09:55) - PLID 32407 - supported ProvidersT tablechecker
	if(m_ProviderChecker.Changed()) {

		//save the current value
		long nCurID = -1;
		CString strComboBoxText = "";
		if(m_SupervisingProviderCombo->GetCurSel() != NULL) {
			nCurID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_SupervisingProviderCombo->GetCurSel())->GetValue(spccID), -1);
			strComboBoxText = VarString(NXDATALIST2Lib::IRowSettingsPtr(m_SupervisingProviderCombo->GetCurSel())->GetValue(spccName), "");
		}
		else if(m_SupervisingProviderCombo->GetIsComboBoxTextInUse()) {
			strComboBoxText = (LPCTSTR)m_SupervisingProviderCombo->GetComboBoxText();
		}

		m_SupervisingProviderCombo->Requery();

		{
			//add a "no provider" row
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_SupervisingProviderCombo->GetNewRow();
			pNewRow->PutValue(spccID, (long)-1);
			pNewRow->PutValue(spccName, (LPCSTR)" <No Provider Selected>");
			m_SupervisingProviderCombo->AddRowSorted(pNewRow, NULL);
		}

		//now set the old value, if it exists
		if(nCurID != -1) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_SupervisingProviderCombo->SetSelByColumn(spccID, (long)nCurID);
			if(pRow == NULL && !strComboBoxText.IsEmpty()) {
				//set the combobox text
				m_SupervisingProviderCombo->PutComboBoxText((LPCTSTR)strComboBoxText);
			}
		}
		else if(!strComboBoxText.IsEmpty()) {
			m_SupervisingProviderCombo->PutComboBoxText((LPCTSTR)strComboBoxText);
		}
		else {
			m_SupervisingProviderCombo->SetSelByColumn(spccID, (long)-1);
		}	
	}

	// (j.gruber 2011-09-29 15:22) - PLID 45356 - refy phys checker
	if(m_RefPhysChecker.Changed()) {
		//everything is handled in onRequeryFinished
		m_pAffiliatePhysList->Requery();
	}

	//do NOT clear data, it may have already been filled from billingdlg!
	//ClearData();

	ChangeAccess();

	//do NOT load data, it will have been loaded from billingmoduledlg
	//ReadBillData();

	m_boInitialized = TRUE;
}

void CBilling2Dlg::ClearData()
{
	m_nCurTotalAnesthMinutes = 0;
	m_nCurTotalFacilityMinutes = 0;
	m_nCurTotalAssistingMinutes = 0;

	m_nxtAnesthStart->Clear();
	m_nxtAnesthEnd->Clear();
	m_nxtFacilityStart->Clear();
	m_nxtFacilityEnd->Clear();
	m_nxtAssistingStart->Clear();
	m_nxtAssistingEnd->Clear();

	m_DischargeStatusCombo->PutCurSel(NULL);

	// (j.jones 2008-12-11 09:50) - PLID 32407 - added m_SupervisingProviderCombo
	m_SupervisingProviderCombo->SetSelByColumn(spccID, (long)-1);
	m_nOldSupervisingProviderID = -1;
	m_strOldSupervisingProviderName = "";

	SetDlgItemText(IDC_EDIT_ANESTH_HOURS, "");
	SetDlgItemText(IDC_EDIT_ANESTH_MINUTES, "");
	SetDlgItemText(IDC_EDIT_TOTAL_ANESTH_MINUTES, "");
	SetDlgItemText(IDC_EDIT_FACILITY_HOURS, "");
	SetDlgItemText(IDC_EDIT_FACILITY_MINUTES, "");
	SetDlgItemText(IDC_EDIT_TOTAL_FACILITY_MINUTES, "");
	SetDlgItemText(IDC_EDIT_ASSISTING_HOURS, "");
	SetDlgItemText(IDC_EDIT_ASSISTING_MINUTES, "");
	SetDlgItemText(IDC_EDIT_TOTAL_ASSISTING_MINUTES, "");

	SetDlgItemText(IDC_DEDUCT_LEFT_TO_MEET, "");
	SetDlgItemText(IDC_PAT_CO_INSURANCE, "");
	SetDlgItemText(IDC_OOP_LEFT_TO_MEET, "");

	// (j.jones 2009-08-11 17:05) - PLID 35142 - clear the case history info
	m_bIsCaseHistoryComboHidden = FALSE;
	m_nxlCaseHistoryLinkLabel.SetText("");

	// (j.gruber 2011-09-28 14:17) - PLID 45358 - only show on bills
	m_pAffiliateStatusList->CurSel = NULL;
	GetDlgItem(IDC_AFFIL_DATE)->SetWindowText("");

	// (j.gruber 2011-09-28 14:15) - PLID 45356
	m_pAffiliatePhysList->CurSel = NULL;
	GetDlgItem(IDC_AFFIL_AMT)->SetWindowText("");
	GetDlgItem(IDC_AFFIL_NOTES)->SetWindowText("");

	// (j.jones 2013-07-10 16:05) - PLID 57148 - clear the invoice number
	m_nxeditInvoiceNumber.SetWindowText("");
	m_nSavedInvoiceNumber = -1;
	m_nSavedInvoiceProviderID = -1;
	m_nCurrentInvoiceNumber = -1;
	m_nCurrentInvoiceProviderID = -1;

	m_nOldAffiliatePhysID = -1;
	m_strOldAffiliatePhysName = "";
	m_nCurrentAffiliatePhysID = -1;
	m_strCurrentAffiliatePhysName = "";
	m_strOldAffiliateAmount = "";
	m_strOldAffiliateNote = "";
	m_nOldStatusType = -1;
	
	// (j.gruber 2011-09-28 15:50) - PLID 45358
	m_nOldAffiliateStatusID = -1;
	m_strOldAffiliateStatusDescription = "";
}

BEGIN_EVENTSINK_MAP(CBilling2Dlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CBilling2Dlg)
	ON_EVENT(CBilling2Dlg, IDC_ANESTH_START_TIME, 1 /* KillFocus */, OnKillFocusAnesthStartTime, VTS_NONE)
	ON_EVENT(CBilling2Dlg, IDC_ANESTH_END_TIME, 1 /* KillFocus */, OnKillFocusAnesthEndTime, VTS_NONE)
	ON_EVENT(CBilling2Dlg, IDC_FACILITY_START_TIME, 1 /* KillFocus */, OnKillFocusFacilityStartTime, VTS_NONE)
	ON_EVENT(CBilling2Dlg, IDC_FACILITY_END_TIME, 1 /* KillFocus */, OnKillFocusFacilityEndTime, VTS_NONE)
	ON_EVENT(CBilling2Dlg, IDC_ASSISTING_START_TIME, 1 /* KillFocus */, OnKillFocusAssistingStartTime, VTS_NONE)
	ON_EVENT(CBilling2Dlg, IDC_ASSISTING_END_TIME, 1 /* KillFocus */, OnKillFocusAssistingEndTime, VTS_NONE)
	ON_EVENT(CBilling2Dlg, IDC_DISCHARGE_STATUS_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedDischargeStatusCombo, VTS_I2)
	ON_EVENT(CBilling2Dlg, IDC_CASE_HISTORY_BILL_COMBO, 16, OnSelChosenCaseHistoryBillCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP	
	ON_EVENT(CBilling2Dlg, IDC_BILL_AFFILIATE_LIST, 1, CBilling2Dlg::SelChangingBillAffiliateList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBilling2Dlg, IDC_BILL_AFFILIATE_LIST, 16, CBilling2Dlg::SelChosenBillAffiliateList, VTS_DISPATCH)
	ON_EVENT(CBilling2Dlg, IDC_BILL_AFFILIATE_LIST, 18, CBilling2Dlg::RequeryFinishedBillAffiliateList, VTS_I2)
	ON_EVENT(CBilling2Dlg, IDC_AFFIL_STATUS, 1, CBilling2Dlg::SelChangingAffilStatus, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBilling2Dlg, IDC_AFFIL_STATUS, 16, CBilling2Dlg::SelChosenAffilStatus, VTS_DISPATCH)
	ON_EVENT(CBilling2Dlg, IDC_BILL_AFFILIATE_LIST, 20, CBilling2Dlg::TrySetSelFinishedBillAffiliateList, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CBilling2Dlg::OnKillfocusEditAnesthHours() 
{
	long nHours = GetDlgItemInt(IDC_EDIT_ANESTH_HOURS);
	long nMinutes = GetDlgItemInt(IDC_EDIT_ANESTH_MINUTES);
	if(nHours > 24) {
		AfxMessageBox("You cannot have more than 24 anesthesia hours.");
		nHours = 0;
		SetDlgItemInt(IDC_EDIT_ANESTH_HOURS, 0);
	}
	else if(nHours == 24 && nMinutes > 0) {
		AfxMessageBox("You cannot have more than 24 anesthesia hours.");
		nHours = 0;
		nMinutes = 0;
		SetDlgItemInt(IDC_EDIT_ANESTH_HOURS, 0);
		SetDlgItemInt(IDC_EDIT_ANESTH_MINUTES, 0);
	}

	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	long nTotalMinutes = (nHours * 60) + nMinutes;
	SetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES, nTotalMinutes);

	if(m_nCurTotalAnesthMinutes != nTotalMinutes) {

		m_nCurTotalAnesthMinutes = nTotalMinutes;

		if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling)
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnAnesthesiaTimeChanged(nTotalMinutes);
	}
}

void CBilling2Dlg::OnKillfocusEditAnesthMinutes() 
{
	long nHours = GetDlgItemInt(IDC_EDIT_ANESTH_HOURS);
	long nMinutes = GetDlgItemInt(IDC_EDIT_ANESTH_MINUTES);
	if(nMinutes > 59) {
		AfxMessageBox("You cannot have more than 59 anesthesia minutes.");
		nMinutes = 0;
		SetDlgItemInt(IDC_EDIT_ANESTH_MINUTES, 0);
	}
	else if(nHours == 24 && nMinutes > 0) {
		AfxMessageBox("You cannot have more than 24 anesthesia hours.");
		nHours = 0;
		nMinutes = 0;
		SetDlgItemInt(IDC_EDIT_ANESTH_HOURS, 0);
		SetDlgItemInt(IDC_EDIT_ANESTH_MINUTES, 0);
	}

	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	long nTotalMinutes = (nHours * 60) + nMinutes;
	SetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES, nTotalMinutes);

	if(m_nCurTotalAnesthMinutes != nTotalMinutes) {

		m_nCurTotalAnesthMinutes = nTotalMinutes;

		if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling)
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnAnesthesiaTimeChanged(nTotalMinutes);
	}
}

void CBilling2Dlg::OnKillfocusEditFacilityHours() 
{
	long nHours = GetDlgItemInt(IDC_EDIT_FACILITY_HOURS);
	long nMinutes = GetDlgItemInt(IDC_EDIT_FACILITY_MINUTES);
	if(nHours > 24) {
		AfxMessageBox("You cannot have more than 24 facility hours.");
		nHours = 0;
		SetDlgItemInt(IDC_EDIT_FACILITY_HOURS, 0);
	}
	else if(nHours == 24 && nMinutes > 0) {
		AfxMessageBox("You cannot have more than 24 facility hours.");
		nHours = 0;
		nMinutes = 0;
		SetDlgItemInt(IDC_EDIT_FACILITY_HOURS, 0);
		SetDlgItemInt(IDC_EDIT_FACILITY_MINUTES, 0);
	}

	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	long nTotalMinutes = (nHours * 60) + nMinutes;
	SetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES, nTotalMinutes);

	if(m_nCurTotalFacilityMinutes != nTotalMinutes) {

		m_nCurTotalFacilityMinutes = nTotalMinutes;
	
		if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling)
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnFacilityTimeChanged(nTotalMinutes);
	}
}

void CBilling2Dlg::OnKillfocusEditFacilityMinutes() 
{
	long nHours = GetDlgItemInt(IDC_EDIT_FACILITY_HOURS);
	long nMinutes = GetDlgItemInt(IDC_EDIT_FACILITY_MINUTES);
	if(nMinutes > 59) {
		AfxMessageBox("You cannot have more than 59 facility minutes.");
		nMinutes = 0;
		SetDlgItemInt(IDC_EDIT_FACILITY_MINUTES, 0);
	}
	else if(nHours == 24 && nMinutes > 0) {
		AfxMessageBox("You cannot have more than 24 facility hours.");
		nHours = 0;
		nMinutes = 0;
		SetDlgItemInt(IDC_EDIT_FACILITY_HOURS, 0);
		SetDlgItemInt(IDC_EDIT_FACILITY_MINUTES, 0);
	}

	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	long nTotalMinutes = (nHours * 60) + nMinutes;
	SetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES, nTotalMinutes);

	if(m_nCurTotalFacilityMinutes != nTotalMinutes) {

		m_nCurTotalFacilityMinutes = nTotalMinutes;
	
		if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling)
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnFacilityTimeChanged(nTotalMinutes);
	}
}

void CBilling2Dlg::OnKillfocusEditTotalAnesthMinutes() 
{
	long nTotalMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES);

	if(nTotalMinutes > 1440) {
		AfxMessageBox("You cannot have more than 1440 anesthesia minutes (24 hours).");
		nTotalMinutes = 1440;
		SetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES, nTotalMinutes);
	}

	long nHours = nTotalMinutes / 60;
	long nMinutes = nTotalMinutes - (nHours * 60);
	SetDlgItemInt(IDC_EDIT_ANESTH_HOURS, nHours);
	SetDlgItemInt(IDC_EDIT_ANESTH_MINUTES, nMinutes);
	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	if(m_nCurTotalAnesthMinutes != nTotalMinutes) {

		m_nCurTotalAnesthMinutes = nTotalMinutes;
	
		if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling)
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnAnesthesiaTimeChanged(nTotalMinutes);
	}
}

void CBilling2Dlg::OnKillfocusEditTotalFacilityMinutes() 
{
	long nTotalMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES);

	if(nTotalMinutes > 1440) {
		AfxMessageBox("You cannot have more than 1440 facility minutes (24 hours).");
		nTotalMinutes = 1440;
		SetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES, nTotalMinutes);
	}

	long nHours = nTotalMinutes / 60;
	long nMinutes = nTotalMinutes - (nHours * 60);
	SetDlgItemInt(IDC_EDIT_FACILITY_HOURS, nHours);
	SetDlgItemInt(IDC_EDIT_FACILITY_MINUTES, nMinutes);
	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	if(m_nCurTotalFacilityMinutes != nTotalMinutes) {

		m_nCurTotalFacilityMinutes = nTotalMinutes;	
		
		if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling)
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnFacilityTimeChanged(nTotalMinutes);
	}
}

void CBilling2Dlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	SetControlPositions();
	Invalidate();
}

void CBilling2Dlg::OnKillFocusAnesthStartTime() 
{
	if(m_nxtAnesthStart->GetStatus() == 3)
		//blank, that's okay
		return;

	if(m_nxtAnesthStart->GetStatus() == 2) {
		MessageBox("You have entered an invalid time");
		m_nxtAnesthStart->Clear();
		return;
	}

	COleDateTime dtStart, dtEnd;

	if(m_nxtAnesthStart->GetStatus() == 1) {
		dtStart = m_nxtAnesthStart->GetDateTime();
	}
	else {
		dtStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtAnesthEnd->GetStatus() == 1) {
		dtEnd = m_nxtAnesthEnd->GetDateTime();
	}
	else {
		dtEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtStart.GetStatus() == COleDateTime::invalid) {
		MessageBox("You have entered an invalid time");
		m_nxtAnesthStart->Clear();
		return;
	}

	//if a valid end time was entered, then update the minutes appropriately
	if(dtEnd.GetStatus() != COleDateTime::invalid) {

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
			"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
			"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd));
		long nTotalMinutes = AdoFldLong(rs, "Minutes",0);

		if(nTotalMinutes < 0) {
			MessageBox("You must enter a valid time range (greater than zero minutes).");
			return;
		}

		//otherwise, set the total minutes and recalculate
		SetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES, nTotalMinutes);
		OnKillfocusEditTotalAnesthMinutes();
	}
}

void CBilling2Dlg::OnKillFocusAnesthEndTime() 
{
	if(m_nxtAnesthEnd->GetStatus() == 3)
		//blank, that's okay
		return;

	if(m_nxtAnesthEnd->GetStatus() == 2) {
		MessageBox("You have entered an invalid time");
		m_nxtAnesthEnd->Clear();
		return;
	}

	COleDateTime dtStart, dtEnd;

	if(m_nxtAnesthStart->GetStatus() == 1) {
		dtStart = m_nxtAnesthStart->GetDateTime();
	}
	else {
		dtStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtAnesthEnd->GetStatus() == 1) {
		dtEnd = m_nxtAnesthEnd->GetDateTime();
	}
	else {
		dtEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtEnd.GetStatus() == COleDateTime::invalid) {
		MessageBox("You have entered an invalid time");
		m_nxtAnesthEnd->Clear();
		return;
	}

	//if a valid start time was entered, then update the minutes appropriately
	if(dtStart.GetStatus() != COleDateTime::invalid) {

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
			"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
			"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd));
		long nTotalMinutes = AdoFldLong(rs, "Minutes",0);

		if(nTotalMinutes < 0) {
			MessageBox("You must enter a valid time range (greater than zero minutes).");
			return;
		}

		//otherwise, set the total minutes and recalculate
		SetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES, nTotalMinutes);
		OnKillfocusEditTotalAnesthMinutes();
	}
}

void CBilling2Dlg::OnKillFocusFacilityStartTime() 
{
	if(m_nxtFacilityStart->GetStatus() == 3)
		//blank, that's okay
		return;

	if(m_nxtFacilityStart->GetStatus() == 2) {
		MessageBox("You have entered an invalid time");
		m_nxtFacilityStart->Clear();
		return;
	}

	COleDateTime dtStart, dtEnd;

	if(m_nxtFacilityStart->GetStatus() == 1) {
		dtStart = m_nxtFacilityStart->GetDateTime();
	}
	else {
		dtStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtFacilityEnd->GetStatus() == 1) {
		dtEnd = m_nxtFacilityEnd->GetDateTime();
	}
	else {
		dtEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtStart.GetStatus() == COleDateTime::invalid) {
		MessageBox("You have entered an invalid time");
		m_nxtFacilityStart->Clear();
		return;
	}

	//if a valid end time was entered, then update the minutes appropriately
	if(dtEnd.GetStatus() != COleDateTime::invalid) {

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
			"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
			"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd));
		long nTotalMinutes = AdoFldLong(rs, "Minutes",0);

		if(nTotalMinutes < 0) {
			MessageBox("You must enter a valid time range (greater than zero minutes).");
			return;
		}

		//otherwise, set the total minutes and recalculate
		SetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES, nTotalMinutes);
		OnKillfocusEditTotalFacilityMinutes();
	}
}

void CBilling2Dlg::OnKillFocusFacilityEndTime() 
{
	if(m_nxtFacilityEnd->GetStatus() == 3)
		//blank, that's okay
		return;

	if(m_nxtFacilityEnd->GetStatus() == 2) {
		MessageBox("You have entered an invalid time");
		m_nxtFacilityEnd->Clear();
		return;
	}

	COleDateTime dtStart, dtEnd;

	if(m_nxtFacilityStart->GetStatus() == 1) {
		dtStart = m_nxtFacilityStart->GetDateTime();
	}
	else {
		dtStart.SetStatus(COleDateTime::invalid);
	}

	if(m_nxtFacilityEnd->GetStatus() == 1) {
		dtEnd = m_nxtFacilityEnd->GetDateTime();
	}
	else {
		dtEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtEnd.GetStatus() == COleDateTime::invalid) {
		MessageBox("You have entered an invalid time");
		m_nxtFacilityEnd->Clear();
		return;
	}

	//if a valid start time was entered, then update the minutes appropriately
	if(dtStart.GetStatus() != COleDateTime::invalid) {

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
			"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
			"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
			AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd));
		long nTotalMinutes = AdoFldLong(rs, "Minutes",0);

		if(nTotalMinutes < 0) {
			MessageBox("You must enter a valid time range (greater than zero minutes).");
			return;
		}

		//otherwise, set the total minutes and recalculate
		SetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES, nTotalMinutes);
		OnKillfocusEditTotalFacilityMinutes();
	}
}

int CBilling2Dlg::SaveChanges()
{
	//Returns SCR_ABORT_SAVE, SCR_NOT_SAVED, or SCR_SAVED

	/* Don't do anything if this tab was not even accessed */
	//if (!m_boInitialized)
	//	return SCR_NOT_SAVED;

	try {

		//verify that the anesthesia and facility times are kosher

		COleDateTime dtAnesthStart, dtAnesthEnd;
		COleDateTime dtFacilityStart, dtFacilityEnd;
		COleDateTime dtAssistingStart, dtAssistingEnd;

		if(m_nxtAnesthStart->GetStatus() == 1) {
			dtAnesthStart = m_nxtAnesthStart->GetDateTime();
		}
		else {
			dtAnesthStart.SetStatus(COleDateTime::invalid);
		}

		if(m_nxtAnesthEnd->GetStatus() == 1) {
			dtAnesthEnd = m_nxtAnesthEnd->GetDateTime();
		}
		else {
			dtAnesthEnd.SetStatus(COleDateTime::invalid);
		}

		if(m_nxtFacilityStart->GetStatus() == 1) {
			dtFacilityStart = m_nxtFacilityStart->GetDateTime();
		}
		else {
			dtFacilityStart.SetStatus(COleDateTime::invalid);
		}

		if(m_nxtFacilityEnd->GetStatus() == 1) {
			dtFacilityEnd = m_nxtFacilityEnd->GetDateTime();
		}
		else {
			dtFacilityEnd.SetStatus(COleDateTime::invalid);
		}

		// (j.jones 2011-11-01 11:17) - PLID 41558 - added assisting controls
		if(m_nxtAssistingStart->GetStatus() == 1) {
			dtAssistingStart = m_nxtAssistingStart->GetDateTime();
		}
		else {
			dtAssistingStart.SetStatus(COleDateTime::invalid);
		}

		if(m_nxtAssistingEnd->GetStatus() == 1) {
			dtAssistingEnd = m_nxtAssistingEnd->GetDateTime();
		}
		else {
			dtAssistingEnd.SetStatus(COleDateTime::invalid);
		}

		//first see if they entered one date and not the other
		
		if(dtAnesthStart.GetStatus() != COleDateTime::invalid
			&& dtAnesthEnd.GetStatus() == COleDateTime::invalid) {

			AfxMessageBox("The 'Anesthesia Start Time' is filled in, but the corresponding 'Anesthesia End Time' is not.\n"
				"Please ensure that both dates are either filled in or are blank.");
			return SCR_ABORT_SAVE;
		}

		if(dtAnesthStart.GetStatus() == COleDateTime::invalid
			&& dtAnesthEnd.GetStatus() != COleDateTime::invalid) {

			AfxMessageBox("The 'Anesthesia End Time' is filled in, but the corresponding 'Anesthesia Start Time' is not.\n"
				"Please ensure that both dates are either filled in or are blank.");
			return SCR_ABORT_SAVE;
		}

		if(dtFacilityStart.GetStatus() != COleDateTime::invalid
			&& dtFacilityEnd.GetStatus() == COleDateTime::invalid) {

			AfxMessageBox("The 'Facility Start Time' is filled in, but the corresponding 'Facility End Time' is not.\n"
				"Please ensure that both dates are either filled in or are blank.");
			return SCR_ABORT_SAVE;
		}

		if(dtFacilityStart.GetStatus() == COleDateTime::invalid
			&& dtFacilityEnd.GetStatus() != COleDateTime::invalid) {

			AfxMessageBox("The 'Facility End Time' is filled in, but the corresponding 'Facility Start Time' is not.\n"
				"Please ensure that both dates are either filled in or are blank.");
			return SCR_ABORT_SAVE;
		}

		// (j.jones 2011-11-01 11:17) - PLID 41558 - added assisting controls
		if(dtAssistingStart.GetStatus() != COleDateTime::invalid
			&& dtAssistingEnd.GetStatus() == COleDateTime::invalid) {

			AfxMessageBox("The 'Assisting Start Time' is filled in, but the corresponding 'Assisting End Time' is not.\n"
				"Please ensure that both dates are either filled in or are blank.");
			return SCR_ABORT_SAVE;
		}

		if(dtAssistingStart.GetStatus() == COleDateTime::invalid
			&& dtAssistingEnd.GetStatus() != COleDateTime::invalid) {

			AfxMessageBox("The 'Assisting End Time' is filled in, but the corresponding 'Assisting Start Time' is not.\n"
				"Please ensure that both dates are either filled in or are blank.");
			return SCR_ABORT_SAVE;
		}

		//now see if the dates match the total minutes

		if(dtAnesthStart.GetStatus() != COleDateTime::invalid
			&& dtAnesthEnd.GetStatus() != COleDateTime::invalid) {

			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT "
				"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
				"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
				"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
				AsTimeNoDate(dtAnesthStart),AsTimeNoDate(dtAnesthEnd),
				AsTimeNoDate(dtAnesthStart),AsTimeNoDate(dtAnesthEnd),
				AsTimeNoDate(dtAnesthStart),AsTimeNoDate(dtAnesthEnd));
			long nTotalCalcMinutes = AdoFldLong(rs, "Minutes",0);
			long nTotalEnteredMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES);

			if(nTotalCalcMinutes != nTotalEnteredMinutes) {
				CString str;
				str.Format("Your Anesthesia Start and End times span %li minutes, but you have manually entered a total Anesthesia time of %li minutes.\n"
					"Please correct this discrepancy before saving.",nTotalCalcMinutes,nTotalEnteredMinutes);
				AfxMessageBox(str);
				return SCR_ABORT_SAVE;
			}
		}

		if(dtFacilityStart.GetStatus() != COleDateTime::invalid
			&& dtFacilityEnd.GetStatus() != COleDateTime::invalid) {

			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT "
				"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
				"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
				"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
				AsTimeNoDate(dtFacilityStart),AsTimeNoDate(dtFacilityEnd),
				AsTimeNoDate(dtFacilityStart),AsTimeNoDate(dtFacilityEnd),
				AsTimeNoDate(dtFacilityStart),AsTimeNoDate(dtFacilityEnd));
			long nTotalCalcMinutes = AdoFldLong(rs, "Minutes",0);
			long nTotalEnteredMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES);

			if(nTotalCalcMinutes != nTotalEnteredMinutes) {
				CString str;
				str.Format("Your Facility Start and End times span %li minutes, but you have manually entered a total Facility time of %li minutes.\n"
					"Please correct this discrepancy before saving.",nTotalCalcMinutes,nTotalEnteredMinutes);
				AfxMessageBox(str);
				return SCR_ABORT_SAVE;
			}
		}

		// (j.jones 2011-11-01 11:17) - PLID 41558 - added assisting controls
		if(dtAssistingStart.GetStatus() != COleDateTime::invalid
			&& dtAssistingEnd.GetStatus() != COleDateTime::invalid) {

			_RecordsetPtr rs = CreateParamRecordset("SELECT "
				"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
				"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
				"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
				AsTimeNoDate(dtAssistingStart),AsTimeNoDate(dtAssistingEnd),
				AsTimeNoDate(dtAssistingStart),AsTimeNoDate(dtAssistingEnd),
				AsTimeNoDate(dtAssistingStart),AsTimeNoDate(dtAssistingEnd));
			long nTotalCalcMinutes = AdoFldLong(rs, "Minutes",0);
			long nTotalEnteredMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES);

			if(nTotalCalcMinutes != nTotalEnteredMinutes) {
				CString str;
				str.Format("Your Assisting Start and End times span %li minutes, but you have manually entered a total Assisting time of %li minutes.\n"
					"Please correct this discrepancy before saving.",nTotalCalcMinutes,nTotalEnteredMinutes);
				AfxMessageBox(str);
				return SCR_ABORT_SAVE;
			}
		}

		// (j.gruber 2011-09-29 15:29) - PLID 45358 - check to see if they set their status to something before the old
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAffiliateStatusList->CurSel;
		long nStatusType = -1;
		if (pRow) {
			 nStatusType = VarLong(pRow->GetValue(afscType));
			if (m_nOldStatusType > nStatusType) {
				CString strMessage = "You have set the Affiliate Status field to a previous status.";
				if (m_nOldStatusType == 3 && nStatusType == 1) {
					strMessage += " The status history for Ready To Be Paid and Paid will be removed.";
				}else if (m_nOldStatusType == 2 && nStatusType == 1){
					strMessage += " The status history for Ready To Be Paid will be removed.";
				}
				else if (m_nOldStatusType == 3 && nStatusType == 2) {
					strMessage += " The status history for Paid will be removed.";
				}
				else {
					//this should never fire unless we add statii
					ASSERT(FALSE);
					ThrowNxException("Invalid Affiliate Status");
				}
				strMessage += "\n\nAre you sure you wish to continue saving this bill?";

				if (MsgBox(MB_YESNO, strMessage) == IDNO) {
					return SCR_ABORT_SAVE;
				}
			}
		}				

		//for auditing
		long nOldDischargeStatusID = -1;
		CString strOldDischargeStatus = "";
		if(GetBillID() != -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT DischargeStatusT.ID, DischargeStatusT.Description "
				"FROM BillsT "
				"INNER JOIN DischargeStatusT ON BillsT.DischargeStatusID = DischargeStatusT.ID "
				"WHERE BillsT.ID = {INT}", GetBillID());
			if(!rs->eof) {
				nOldDischargeStatusID = AdoFldLong(rs, "ID", -1);
				strOldDischargeStatus = AdoFldString(rs, "Description", "");
			}
			rs->Close();
		}

		CString strAnesthStartTime, strAnesthEndTime, strFacilityStartTime, strFacilityEndTime;
		CString strAssistingStartTime, strAssistingEndTime;
		long nTotalAnesthMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES);
		long nTotalFacilityMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES);
		long nTotalAssistingMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES);

		if(dtAnesthStart.GetStatus() != COleDateTime::invalid) {
			strAnesthStartTime = "'" + _Q(FormatDateTimeForSql(dtAnesthStart,dtoTime)) + "'";
		}
		else {
			strAnesthStartTime = "NULL";
		}

		if(dtAnesthEnd.GetStatus() != COleDateTime::invalid) {
			strAnesthEndTime = "'" + _Q(FormatDateTimeForSql(dtAnesthEnd,dtoTime)) + "'";
		}
		else {
			strAnesthEndTime = "NULL";
		}

		if(dtFacilityStart.GetStatus() != COleDateTime::invalid) {
			strFacilityStartTime = "'" + _Q(FormatDateTimeForSql(dtFacilityStart,dtoTime)) + "'";
		}
		else {
			strFacilityStartTime = "NULL";
		}

		if(dtFacilityEnd.GetStatus() != COleDateTime::invalid) {
			strFacilityEndTime = "'" + _Q(FormatDateTimeForSql(dtFacilityEnd,dtoTime)) + "'";
		}
		else {
			strFacilityEndTime = "NULL";
		}

		// (j.jones 2011-11-01 11:17) - PLID 41558 - added assisting controls
		if(dtAssistingStart.GetStatus() != COleDateTime::invalid) {
			strAssistingStartTime = "'" + _Q(FormatDateTimeForSql(dtAssistingStart,dtoTime)) + "'";
		}
		else {
			strAssistingStartTime = "NULL";
		}

		if(dtAssistingEnd.GetStatus() != COleDateTime::invalid) {
			strAssistingEndTime = "'" + _Q(FormatDateTimeForSql(dtAssistingEnd,dtoTime)) + "'";
		}
		else {
			strAssistingEndTime = "NULL";
		}

		// (j.jones 2006-11-22 13:00) - PLID 23371 - added Discharge Status
		CString strDischargeStatusID = "NULL";
		long nDischargeStatusID = -1;
		CString strDischargeStatusDescription = "";
		if(m_DischargeStatusCombo->GetCurSel() != NULL) {
			nDischargeStatusID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_DischargeStatusCombo->GetCurSel())->GetValue(0), -1);
			if(nDischargeStatusID != -1) {
				strDischargeStatusID.Format("%li", nDischargeStatusID);
				strDischargeStatusDescription = VarString(NXDATALIST2Lib::IRowSettingsPtr(m_DischargeStatusCombo->GetCurSel())->GetValue(2), "");
			}
		}

		// (j.jones 2008-12-11 10:06) - PLID 32407 - added Supervising Provider
		long nSupervisingProviderID = -1;
		CString strSupervisingProviderID = "NULL";
		CString strNewSupervisingProviderName = ""; //for auditing
		if(m_SupervisingProviderCombo->GetCurSel() != NULL) {
			nSupervisingProviderID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_SupervisingProviderCombo->GetCurSel())->GetValue(spccID), -1);
			if(nSupervisingProviderID != -1) {
				strSupervisingProviderID.Format("%li", nSupervisingProviderID);
				strNewSupervisingProviderName = VarString(NXDATALIST2Lib::IRowSettingsPtr(m_SupervisingProviderCombo->GetCurSel())->GetValue(spccName), "");
			}
		}
		else if(m_SupervisingProviderCombo->GetIsComboBoxTextInUse() && m_nOldSupervisingProviderID != -1) {
			nSupervisingProviderID = m_nOldSupervisingProviderID;
			strSupervisingProviderID.Format("%li", m_nOldSupervisingProviderID);
			strNewSupervisingProviderName = (LPCTSTR)m_SupervisingProviderCombo->GetComboBoxText();
		}

		// (j.gruber 2006-12-29 14:53) - PLID 23972 - add Deductible, oop, and coinsurnace
		CString strDeductible = "NULL";
		CString strTemp;
		COleCurrency cyTemp;
		COleCurrency cyDeduct;
		GetDlgItemText(IDC_DEDUCT_LEFT_TO_MEET, strTemp);
		strTemp.TrimLeft();
		strTemp.TrimRight();
		if (!strTemp.IsEmpty()) {
			cyTemp = ParseCurrencyFromInterface(strTemp);
			cyDeduct = cyTemp;
			strDeductible.Format("Convert(money, '%s')", _Q(FormatCurrencyForSql(cyTemp)));
		}

		CString strCoIns = "NULL";
		GetDlgItemText(IDC_PAT_CO_INSURANCE, strTemp);
		strTemp.TrimLeft();
		strTemp.TrimRight();
		if (!strTemp.IsEmpty()) {
			strCoIns = strTemp;
		}

		CString strOOP = "NULL";
		COleCurrency cyOOP;
		GetDlgItemText(IDC_OOP_LEFT_TO_MEET, strTemp);
		strTemp.TrimLeft();
		strTemp.TrimRight();
		if (!strTemp.IsEmpty()) {
			cyTemp = ParseCurrencyFromInterface(strTemp);
			cyOOP = cyTemp;
			strOOP.Format("Convert(money, '%s')", _Q(FormatCurrencyForSql(cyTemp)));
		}

		// (j.gruber 2011-09-28 14:26) - PLID 45356 - affiliate phys
		long nAffiliatePhysID = -1;
		CString strAfilliatePhysID = "NULL";
		CString strNewAffiliatePhysName = ""; //for auditing
		if(m_pAffiliatePhysList->GetCurSel() != NULL) {			
			nAffiliatePhysID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_pAffiliatePhysList->GetCurSel())->GetValue(aflcID), -1);
			if(nAffiliatePhysID != -1) {
				strAfilliatePhysID.Format("%li", nAffiliatePhysID);
				strNewAffiliatePhysName = VarString(NXDATALIST2Lib::IRowSettingsPtr(m_pAffiliatePhysList->GetCurSel())->GetValue(aflcName), "");
			}
		}
		else if(m_pAffiliatePhysList->GetIsComboBoxTextInUse() && m_nOldAffiliatePhysID != -1) {
			nAffiliatePhysID = m_nOldAffiliatePhysID;
			strAfilliatePhysID.Format("%li", m_nOldAffiliatePhysID);
			strNewAffiliatePhysName = (LPCTSTR)m_pAffiliatePhysList->GetComboBoxText();
		}

		// (j.gruber 2011-09-28 14:30) - PLID 45356 - Amount
		CString strAffiliateAmt = "NULL";
		COleCurrency cyAffilAmt;
		CString strAffilAuditAmt;
		GetDlgItemText(IDC_AFFIL_AMT, strTemp);
		strTemp.TrimLeft();
		strTemp.TrimRight();		
		if (!strTemp.IsEmpty()) {
			cyTemp = ParseCurrencyFromInterface(strTemp);
			cyAffilAmt = cyTemp;
			strAffilAuditAmt = FormatCurrencyForInterface(cyAffilAmt);
			strAffiliateAmt.Format("Convert(money, '%s')", _Q(FormatCurrencyForSql(cyTemp)));
		}

		// (j.gruber 2011-09-28 14:30) - PLID 45356 - Note
		CString strAffilNote = "";
		GetDlgItemText(IDC_AFFIL_NOTES, strTemp);
		strTemp.TrimLeft();
		strTemp.TrimRight();
		if (!strTemp.IsEmpty()) {
			strAffilNote = strTemp;
		}

		// (j.gruber 2011-09-28 14:34) - PLID 45358 - status
		CString strAffiliateStatusID = "NULL";
		long nAffiliateStatusID = -1;
		CString strAffiliateStatusDescription = "";
		if(m_pAffiliateStatusList->GetCurSel() != NULL) {
			nAffiliateStatusID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_pAffiliateStatusList->GetCurSel())->GetValue(afscID), -1);
			if(nAffiliateStatusID != -1) {
				strAffiliateStatusID.Format("%li", nAffiliateStatusID);
				strAffiliateStatusDescription= VarString(NXDATALIST2Lib::IRowSettingsPtr(m_pAffiliateStatusList->GetCurSel())->GetValue(afscName), "");
			}
		}

		// (j.gruber 2011-09-28 14:30) - PLID 45358 - Date
		/*CString strAffilDate = "NULL";
		GetDlgItemText(IDC_AFFIL_DATE, strTemp);
		strTemp.TrimLeft();
		strTemp.TrimRight();
		COleDateTime dtAffiliateStatus;
		dtAffiliateStatus.SetDate(1800,01,01);
		if (!strTemp.IsEmpty()) {
			dtAffiliateStatus = ParseDateTime(strTemp);
			strAffilDate = FormatDateTimeForSql(dtAffiliateStatus);
		}*/		

		//ok, now we can save
		// (j.jones 2008-12-11 10:06) - PLID 32407 - added SupervisingProviderID
		// (j.gruber 2011-09-28 14:39) - PLID 45356 - affiliate phys fields
		// (j.gruber 2011-09-28 14:39) - PLID 45358 - affiliate status fields
		// (j.jones 2011-11-01 11:17) - PLID 41558 - added assisting controls

		//****REMEMBER: All new fields also need to be supported in FinancialCorrection.cpp
		//and in SplitChargesIntoNewBill() in GlobalFinancialUtils.****//
		ExecuteSql("UPDATE BillsT SET AnesthStartTime = %s, AnesthEndTime = %s, \r\n"
			"FacilityStartTime = %s, FacilityEndTime = %s, \r\n"
			"AnesthesiaMinutes = %li, FacilityMinutes = %li, \r\n"
			"AssistingStartTime = %s, AssistingEndTime = %s, AssistingMinutes = %li, \r\n"
			"DischargeStatusID = %s, \r\n"
			"DeductibleLeftToMeet = %s, CoInsurance = %s, OutOfPocketLeftToMeet = %s, \r\n"
			"SupervisingProviderID = %s, \r\n"
			"AffiliatePhysID = %s, AffiliatePhysAmount = %s, AffiliateNote = '%s', \r\n"
			"AffiliateStatusID = %s \r\n"
			"WHERE ID = %li; \r\n"
			"IF 1 = %li BEGIN \r\n"
			"	 DELETE FROM BillAffiliateStatusHistoryT WHERE BillID = %li AND StatusID IN (SELECT ID FROM BillAffiliateStatusT WHERE Type >= %li); \r\n"
			"	 INSERT INTO BillAffiliateStatusHistoryT (BillID, StatusID) VALUES (%li, %s); \r\n"
			"END \r\n"
			,
			strAnesthStartTime, strAnesthEndTime, strFacilityStartTime, strFacilityEndTime, 
			nTotalAnesthMinutes, nTotalFacilityMinutes,
			strAssistingStartTime, strAssistingEndTime, nTotalAssistingMinutes,
			strDischargeStatusID,
			strDeductible, strCoIns, strOOP, strSupervisingProviderID,
			strAfilliatePhysID, strAffiliateAmt, _Q(strAffilNote),
			strAffiliateStatusID, 
			GetBillID(),
			m_nOldAffiliateStatusID != nAffiliateStatusID ? 1 : 0,
			GetBillID(), nStatusType,
			GetBillID(), strAffiliateStatusID);

		long nAuditID = -1;

		//audit the discharge status
		if(nOldDischargeStatusID != nDischargeStatusID) {
			if(nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}
			// (a.walling 2008-05-05 13:18) - PLID 29897 - Replaced all GetActivePatientName references with GetBillPatientName
			AuditEvent(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID, GetBillPatientName(), nAuditID, aeiBillDischargeStatus, GetBillID(), strOldDischargeStatus, strDischargeStatusDescription, aepMedium, aetChanged);
		}

		// (j.jones 2008-12-11 10:15) - PLID 32407 - audit the SupervisingProviderID
		if(m_nOldSupervisingProviderID != nSupervisingProviderID) {
			if(nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}
			CString strOld = m_strOldSupervisingProviderName;
			if(strOld.IsEmpty()) {
				strOld = "<No Provider Selected>";
			}
			CString strNew = strNewSupervisingProviderName;
			if(strNew.IsEmpty()) {
				strNew = "<No Provider Selected>";
			}
			AuditEvent(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID, GetBillPatientName(), nAuditID, aeiBillSupervisingProvider, GetBillID(), strOld, strNew, aepMedium, aetChanged);			
		}
		//track these values
		m_nOldSupervisingProviderID = nSupervisingProviderID;
		m_strOldSupervisingProviderName = strNewSupervisingProviderName;

		// (j.gruber 2007-02-02 12:47) - PLID 23972 - save the new fields
		m_strDeductLeftToMeet = FormatCurrencyForInterface(cyDeduct);
		m_strOOPLeftToMeet = FormatCurrencyForInterface(cyOOP);


		// (j.gruber 2011-09-28 14:46) - PLID 45356 - Affiliate PhysID
		if(m_nOldAffiliatePhysID != nAffiliatePhysID) {
			if(nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}
			CString strOld = m_strOldAffiliatePhysName;
			if(strOld.IsEmpty()) {
				strOld = "<No Affiliate Physician>";
			}
			CString strNew = strNewAffiliatePhysName;
			if(strNew.IsEmpty()) {
				strNew = "<No Affiliate Physician>";
			}
			AuditEvent(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID, GetBillPatientName(), nAuditID, aeiBillAffiliatePhysician, GetBillID(), strOld, strNew, aepMedium, aetChanged);			
		}
		//track these values
		m_nOldAffiliatePhysID = nAffiliatePhysID;
		m_strOldAffiliatePhysName = strNewAffiliatePhysName;


		// (j.gruber 2011-09-28 15:19) - PLID 45356 - amount
		if (m_strOldAffiliateAmount != strAffilAuditAmt) {
			if (nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}			
			AuditEvent(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID, GetBillPatientName(), nAuditID, aeiBillAffiliatePhysAmount, GetBillID(), m_strOldAffiliateAmount, strAffilAuditAmt, aepMedium, aetChanged);			
		}
		m_strOldAffiliateAmount = strAffilAuditAmt;

		// (j.gruber 2011-09-28 15:19) - PLID 45356 - notes

		if (m_strOldAffiliateNote != strAffilNote) {
			if (nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}			
			AuditEvent(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID, GetBillPatientName(), nAuditID, aeiBillAffiliatePhysNote, GetBillID(), m_strOldAffiliateNote, strAffilNote, aepMedium, aetChanged);
		}
		m_strOldAffiliateNote = strAffilNote;

		// (j.gruber 2011-09-28 15:19) - PLID 45358 - Status
		if (m_nOldAffiliateStatusID != nAffiliateStatusID) {
			if (nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}			
			AuditEvent(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID, GetBillPatientName(), nAuditID, aeiBillAffiliatePhysStatus, GetBillID(), m_strOldAffiliateStatusDescription, strAffiliateStatusDescription, aepMedium, aetChanged);			
		}
		m_nOldAffiliateStatusID = nAffiliateStatusID;
		m_strOldAffiliateStatusDescription = strAffiliateStatusDescription;

		// (j.jones 2013-07-12 08:40) - PLID 57148 - now save the invoice ID, if it changed,
		// or if the current 'first provider' is not our stored one
		long nFirstProviderID = ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetFirstChargeProviderID();
		if(m_nSavedInvoiceNumber != m_nCurrentInvoiceNumber
			|| (m_nCurrentInvoiceNumber != -1 && nFirstProviderID != m_nSavedInvoiceProviderID)) {

			//Before SaveChanges was called, ValidateInvoiceNumber should have been called
			//in CBillingDlg::ValidateChanges. That means this call is redundant, but we are
			//keeping it here to make absolutely sure the number is still valid.
			if(!ValidateInvoiceNumber(true)) {
				return SCR_ABORT_SAVE;
			}
			else {
				//m_nCurrentInvoiceNumber should now be the current invoice number

				//try to save, but have the recordset re-confirm the ID is useable,
				//so we can handle the case where someone stole our ID in the past
				//few milliseconds
				_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON \r\n"
					"BEGIN TRAN \r\n"
					"DECLARE @ProviderID INT, @BillID INT, @InvoiceID INT \r\n"
					"SET @ProviderID = {INT} \r\n"
					"SET @BillID = {INT} \r\n"
					"SET @InvoiceID = {INT} \r\n"
					""
					"DECLARE @BillIDUsed INT \r\n"
					"SET @BillIDUsed = NULL \r\n"
					"DECLARE @ProviderNameUsed nvarchar(max) \r\n"
					"SET @ProviderNameUsed = '' \r\n"
					""
					"SELECT @BillIDUsed = BillID, @ProviderNameUsed = Last + ', ' + First + ' ' + Middle "
					"FROM BillInvoiceNumbersT "
					"INNER JOIN PersonT ON BillInvoiceNumbersT.ProviderID = PersonT.ID "
					"WHERE ProviderID = @ProviderID AND InvoiceID = @InvoiceID AND BillID <> @BillID "
					""
					"IF (@BillIDUsed Is Null) \r\n"
					"BEGIN \r\n"
					"	DELETE FROM BillInvoiceNumbersT WHERE BillID = @BillID \r\n"
					"	IF(@InvoiceID > 0) \r\n"
					"	BEGIN \r\n"
					"		INSERT INTO BillInvoiceNumbersT (BillID, ProviderID, InvoiceID) VALUES (@BillID, @ProviderID, @InvoiceID) \r\n"
					"	END \r\n"
					"END \r\n"
					""
					"COMMIT TRAN \r\n"
					"SET NOCOUNT OFF \r\n"
					""
					"SELECT @BillIDUsed AS BillIDInUse, @ProviderNameUsed AS ProviderNameInUse",
					nFirstProviderID, GetBillID(), m_nCurrentInvoiceNumber);
				if(!rs->eof) {
					long nExistingBillID = VarLong(rs->Fields->Item["BillIDInUse"]->Value, -1);
					if(nExistingBillID != -1) {
						//someone stole our invoice number, we could not save
						CString strExistingProviderName = VarString(rs->Fields->Item["ProviderNameInUse"]->Value, "");
						strExistingProviderName.TrimRight();

						CString strWarn;
						strWarn.Format("The invoice ID of %li is already in use for %s on Bill ID %li. It cannot be used on this bill.\n\n"
							"You must enter in a new invoice ID and save the bill again.",
							m_nCurrentInvoiceNumber, strExistingProviderName, nExistingBillID);
						AfxMessageBox(strWarn);
						return SCR_ABORT_SAVE;
					}
				}
				rs->Close();

				//if we didn't return, the invoice number successfully saved

				if(m_nSavedInvoiceNumber != m_nCurrentInvoiceNumber) {

					if (nAuditID == -1) {
						nAuditID = BeginNewAuditEvent();
					}
					CString strOld, strNew;
					if(m_nSavedInvoiceNumber == -1) {
						strOld.Format("Bill ID: %li, Invoice Number: <No Invoice Number>", GetBillID());
					}
					else {
						strOld.Format("Bill ID: %li, Invoice Number: %li", GetBillID(), m_nSavedInvoiceNumber);
					}
					if(m_nCurrentInvoiceNumber == -1) {
						strNew = "<No Invoice Number>";
					}
					else {
						strNew.Format("%li", m_nCurrentInvoiceNumber);
					}
					AuditEvent(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID, GetBillPatientName(), nAuditID, aeiBillInvoiceNumber, GetBillID(), strOld, strNew, aepHigh, aetChanged);			
				}

				SetSavedInvoiceNumber(m_nCurrentInvoiceNumber, nFirstProviderID);
			}
		}
		
		return SCR_SAVED;

	}NxCatchAll("Error in CBilling2Dlg::SaveChanges()");

	return SCR_ABORT_SAVE;
}

void CBilling2Dlg::OnDestroy() 
{
	m_boInitialized = FALSE;
	// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
	m_eHasAccess = batNoAccess;

	CNxDialog::OnDestroy();
}

long CBilling2Dlg::GetBillID()
{
	return ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetBillID();
}

void CBilling2Dlg::ReadBillData()
{
	try {
		long nBillID = GetBillID();

		// (a.walling 2007-05-21 14:38) - PLID 20838 - Show total rewards points for this patient
		UpdateRewardPointsDisplay();

		// (j.jones 2013-07-10 14:50) - PLID 57148 - if a bill, show the invoice field, if enabled
		if(m_EntryType == 1 && GetRemotePropertyInt("EnableBillInvoiceNumbers", 0, 0, "<None>", true) == 1) {
			//show the invoice number field
			GetDlgItem(IDC_LABEL_INVOICE_NUMBER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_INVOICE_NUMBER)->ShowWindow(SW_SHOW);
		}
		else {
			//hide the invoice number field
			GetDlgItem(IDC_LABEL_INVOICE_NUMBER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_INVOICE_NUMBER)->ShowWindow(SW_HIDE);
		}

		// (a.walling 2007-05-21 14:58) - PLID 26088 - Don't query if we already know it won't return anything.
		if (nBillID == -1) 
			return;

		// (j.gruber 2006-12-29 14:40) - PLID 23972 - Add Deductible, CoInsurance, and OOP
		// (j.jones 2008-12-11 10:06) - PLID 32407 - added SupervisingProviderID and SupervisingProviderName
		// (j.gruber 2011-09-28 15:57) - PLID 45356 - affiliate phys, amount, note
		// (j.gruber 2011-09-28 16:05) - PLID 45358 - status and date
		// (j.jones 2011-11-01 11:17) - PLID 41558 - added assisting controls
		// (j.jones 2013-07-11 11:38) - PLID 57148 - added invoice number
		_RecordsetPtr rs = CreateParamRecordset("SELECT AnesthStartTime, AnesthEndTime, "
			"FacilityStartTime, FacilityEndTime, AnesthesiaMinutes, FacilityMinutes, "
			"AssistingStartTime, AssistingEndTime, AssistingMinutes, "
			"DischargeStatusID, DeductibleLeftToMeet, CoInsurance, OutOfPocketLeftToMeet, "
			"SupervisingProviderID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS SupervisingProviderName, "
			"AffiliatePersonT.ID as AffiliatePhysID, AffiliatePersonT.Last + ', ' + AffiliatePersonT.First + ' ' + AffiliatePersonT.Middle AS AffiliateName, "
			"BillsT.AffiliatePhysAmount, BillsT.AffiliateNote, "
			"BillsT.AffiliateStatusID, BillAffiliateStatusT.Name as AffiliateStatusName,  "
			"BillAffiliateStatusHistoryT.Date as AffiliatePhysStatusDate, BillAffiliateStatusT.Type as AffiliateStatusType, "
			"BillInvoiceNumbersT.InvoiceID, BillInvoiceNumbersT.ProviderID AS InvoiceProviderID "
			"FROM BillsT "
			"LEFT JOIN PersonT ON BillsT.SupervisingProviderID = PersonT.ID "
			"LEFT JOIN PersonT AffiliatePersonT ON BillsT.AffiliatePhysID = AffiliatePersonT.ID "
			"LEFT JOIN BillAffiliateStatusT ON BillsT.AffiliateStatusID = BillAffiliateStatusT.ID "
			"LEFT JOIN BillAffiliateStatusHistoryT ON BillsT.ID = BillAffiliateStatusHistoryT.BillID "
			"	AND BillsT.AffiliateStatusID = BillAffiliateStatusHistoryT.StatusID "
			"LEFT JOIN BillInvoiceNumbersT ON BillsT.ID = BillInvoiceNumbersT.BillID "
			"WHERE BillsT.ID = {INT}", GetBillID());

		if (rs->eof) {
			return;
		}

		_variant_t var;

		var = rs->Fields->Item["AnesthStartTime"]->Value;
		if (var.vt != VT_NULL) {
			m_nxtAnesthStart->SetDateTime(VarDateTime(var));
		}
		else {
			m_nxtAnesthStart->Clear();
		}

		var = rs->Fields->Item["AnesthEndTime"]->Value;
		if (var.vt != VT_NULL) {
			m_nxtAnesthEnd->SetDateTime(VarDateTime(var));
		}
		else {
			m_nxtAnesthEnd->Clear();
		}

		var = rs->Fields->Item["FacilityStartTime"]->Value;
		if (var.vt != VT_NULL) {
			m_nxtFacilityStart->SetDateTime(VarDateTime(var));
		}
		else {
			m_nxtFacilityStart->Clear();
		}

		var = rs->Fields->Item["FacilityEndTime"]->Value;
		if (var.vt != VT_NULL) {
			m_nxtFacilityEnd->SetDateTime(VarDateTime(var));
		}
		else {
			m_nxtFacilityEnd->Clear();
		}

		var = rs->Fields->Item["AnesthesiaMinutes"]->Value;
		if (var.vt != VT_NULL) {			
			long nTotalMinutes = VarLong(var);
			SetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES, nTotalMinutes);

			m_nCurTotalAnesthMinutes = nTotalMinutes;
			
			long nHours = nTotalMinutes / 60;
			long nMinutes = nTotalMinutes - (nHours * 60);
			SetDlgItemInt(IDC_EDIT_ANESTH_HOURS, nHours);
			SetDlgItemInt(IDC_EDIT_ANESTH_MINUTES, nMinutes);
			CString strHours, strMinutes;
			strHours.Format("%li",nHours);
			strMinutes.Format("%li",nMinutes);
			if(strMinutes.GetLength() == 1)
				strMinutes = "0" + strMinutes;
		}
		else {
			SetDlgItemText(IDC_EDIT_TOTAL_ANESTH_MINUTES, "");
		}

		var = rs->Fields->Item["FacilityMinutes"]->Value;
		if (var.vt != VT_NULL) {
			long nTotalMinutes = VarLong(var);
			SetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES, nTotalMinutes);

			m_nCurTotalFacilityMinutes = nTotalMinutes;
			
			long nHours = nTotalMinutes / 60;
			long nMinutes = nTotalMinutes - (nHours * 60);
			SetDlgItemInt(IDC_EDIT_FACILITY_HOURS, nHours);
			SetDlgItemInt(IDC_EDIT_FACILITY_MINUTES, nMinutes);
			CString strHours, strMinutes;
			strHours.Format("%li",nHours);
			strMinutes.Format("%li",nMinutes);
			if(strMinutes.GetLength() == 1)
				strMinutes = "0" + strMinutes;
		}
		else {
			SetDlgItemText(IDC_EDIT_TOTAL_FACILITY_MINUTES, "");
		}

		// (j.jones 2011-11-01 11:17) - PLID 41558 - added assisting controls
		var = rs->Fields->Item["AssistingStartTime"]->Value;
		if (var.vt != VT_NULL) {
			m_nxtAssistingStart->SetDateTime(VarDateTime(var));
		}
		else {
			m_nxtAssistingStart->Clear();
		}

		var = rs->Fields->Item["AssistingEndTime"]->Value;
		if (var.vt != VT_NULL) {
			m_nxtAssistingEnd->SetDateTime(VarDateTime(var));
		}
		else {
			m_nxtAssistingEnd->Clear();
		}

		var = rs->Fields->Item["AssistingMinutes"]->Value;
		if (var.vt != VT_NULL) {
			long nTotalMinutes = VarLong(var);
			SetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES, nTotalMinutes);

			m_nCurTotalAssistingMinutes = nTotalMinutes;
			
			long nHours = nTotalMinutes / 60;
			long nMinutes = nTotalMinutes - (nHours * 60);
			SetDlgItemInt(IDC_EDIT_ASSISTING_HOURS, nHours);
			SetDlgItemInt(IDC_EDIT_ASSISTING_MINUTES, nMinutes);
			CString strHours, strMinutes;
			strHours.Format("%li",nHours);
			strMinutes.Format("%li",nMinutes);
			if(strMinutes.GetLength() == 1)
				strMinutes = "0" + strMinutes;
		}
		else {
			SetDlgItemText(IDC_EDIT_TOTAL_ASSISTING_MINUTES, "");
		}

		// (j.jones 2006-11-22 12:58) - PLID 23371 - supported the Discharge Status
		var = rs->Fields->Item["DischargeStatusID"]->Value;
		if(var.vt != VT_NULL) {
			m_DischargeStatusCombo->SetSelByColumn(0, var);
		}
		else {
			m_DischargeStatusCombo->PutCurSel(NULL);
		}

		// (j.jones 2008-12-11 09:50) - PLID 32407 - added m_SupervisingProviderCombo
		m_nOldSupervisingProviderID = AdoFldLong(rs, "SupervisingProviderID", -1);
		m_strOldSupervisingProviderName = AdoFldString(rs, "SupervisingProviderName", "");
		if(m_nOldSupervisingProviderID != -1) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_SupervisingProviderCombo->SetSelByColumn(spccID, (long)m_nOldSupervisingProviderID);
			if(pRow == NULL) {
				//set the combobox text
				m_SupervisingProviderCombo->PutComboBoxText((LPCTSTR)m_strOldSupervisingProviderName);
			}
		}
		else {
			m_SupervisingProviderCombo->SetSelByColumn(spccID, (long)-1);
		}

		// (j.gruber 2006-12-29 14:41) - PLID 23972 - add deductibe, oop, and coinsurance
		COleCurrency cyAmt, cyZero(0,0);
		var = rs->Fields->Item["DeductibleLeftToMeet"]->Value;
		if (var.vt == VT_CY) {
			SetDlgItemText(IDC_DEDUCT_LEFT_TO_MEET, FormatCurrencyForInterface(VarCurrency(var)));
			m_strDeductLeftToMeet = FormatCurrencyForInterface(VarCurrency(var));
		}
		
		var = rs->Fields->Item["OutOfPocketLeftToMeet"]->Value;
		if (var.vt == VT_CY) {
			SetDlgItemText(IDC_OOP_LEFT_TO_MEET, FormatCurrencyForInterface(VarCurrency(var)));
			m_strOOPLeftToMeet = FormatCurrencyForInterface(VarCurrency(var));
		}

		var = rs->Fields->Item["CoInsurance"]->Value;
		if (var.vt == VT_I4) {
			SetDlgItemInt(IDC_PAT_CO_INSURANCE, VarLong(var));
		}

		// (j.gruber 2011-09-28 16:29) - PLID 45356 - Affiliate physican		
		m_nOldAffiliatePhysID = AdoFldLong(rs, "AffiliatePhysID", -1);
		m_strOldAffiliatePhysName = AdoFldString(rs, "AffiliateName", "");
		m_nCurrentAffiliatePhysID = m_nOldAffiliatePhysID;
		m_strCurrentAffiliatePhysName = m_strOldAffiliatePhysName;
		if(m_nOldAffiliatePhysID != -1) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAffiliatePhysList->SetSelByColumn(aflcID, (long)m_nOldAffiliatePhysID);
			if(pRow == NULL) {
				//set the combobox text
				m_pAffiliatePhysList->PutComboBoxText((LPCTSTR)m_strOldAffiliatePhysName);
				m_nCurrentAffiliatePhysID = -2;
			}
		}
		else {
			m_pAffiliatePhysList->SetSelByColumn(aflcID, (long)-1);
		}

		// (j.gruber 2011-09-28 16:52) - PLID 45356 - Amount
		var = rs->Fields->Item["AffiliatePhysAmount"]->Value;
		if (var.vt == VT_CY) {
			SetDlgItemText(IDC_AFFIL_AMT, FormatCurrencyForInterface(VarCurrency(var)));
			m_strOldAffiliateAmount = FormatCurrencyForInterface(VarCurrency(var));
		}
		else {
			m_strOldAffiliateAmount  = "";
		}


		// (j.gruber 2011-09-28 16:54) - PLID 45656 - notes
		var = rs->Fields->Item["AffiliateNote"]->Value;
		if (var.vt == VT_BSTR) {
			SetDlgItemText(IDC_AFFIL_NOTES, VarString(var));
			m_strOldAffiliateNote = VarString(var);
		}
		else {
			m_strOldAffiliateNote = "";
		}

		// (j.gruber 2011-09-28 16:55) - PLID 45358 - Status
		m_nOldAffiliateStatusID = AdoFldLong(rs, "AffiliateStatusID", -1);
		m_strOldAffiliateStatusDescription = AdoFldString(rs, "AffiliateStatusName", "");
		m_nOldStatusType = AdoFldLong(rs, "AffiliateStatusType", -1);
		if(m_nOldAffiliateStatusID != -1) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAffiliateStatusList->SetSelByColumn(afscID, (long)m_nOldAffiliateStatusID);
			if(pRow == NULL) {
				//set the combobox text
				m_pAffiliateStatusList->PutComboBoxText((LPCTSTR)m_strOldAffiliateStatusDescription);
			}
		}
		else {
			m_pAffiliateStatusList->SetSelByColumn(afscID, (long)-1);
		}

		// (j.gruber 2011-09-28 16:54) - PLID 45358 - status date
		var = rs->Fields->Item["AffiliatePhysStatusDate"]->Value;
		if (var.vt == VT_DATE) {
			SetDlgItemText(IDC_AFFIL_DATE, FormatDateTimeForInterface(VarDateTime(var), DTF_STRIP_SECONDS));
			//weird stuff to save the seconds stripped
		//	CString strTemp;
			//GetDlgItemText(IDC_AFFIL_DATE, strTemp);
			//COleDateTime dtTemp = ParseDateTime(strTemp);
//		m_strOldAffiliateStatusDate = FormatDateTimeForSql(dtTemp);
			//m_dtOldAffiliateStatus = VarDateTime(var);
		}
		/*else {
			m_strOldAffiliateStatusDate = "";
		}*/

		// (j.jones 2013-07-11 11:38) - PLID 57148 - added invoice number
		m_nSavedInvoiceNumber = VarLong(rs->Fields->Item["InvoiceID"]->Value, -1);
		m_nSavedInvoiceProviderID = VarLong(rs->Fields->Item["InvoiceProviderID"]->Value, -1);
		if(m_nSavedInvoiceNumber != -1) {
			//always force the field to show if we have an invoice ID on this bill
			GetDlgItem(IDC_LABEL_INVOICE_NUMBER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_INVOICE_NUMBER)->ShowWindow(SW_SHOW);
			SetCurrentInvoiceNumber(m_nSavedInvoiceNumber, m_nSavedInvoiceProviderID);
		}

		rs->Close();
	}NxCatchAll("Error in CBilling2Dlg::ReadBillData()");
}

BOOL CBilling2Dlg::PreTranslateMessage(MSG* pMsg) 
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
			case 'B':
				//billing dlg
				((CBillingModuleDlg*)m_pBillingModuleWnd)->SetActiveTab(0);
				return TRUE;
			case 'I':
				//insurance dlg
				if (m_EntryType == 1) {
					((CBillingModuleDlg*)m_pBillingModuleWnd)->SetActiveTab(1);
				}
				return TRUE;
			}
		}
		
	////////////////////////////////////////////////////
	// When a key is first pressed
	////////////////////////////////////////////////////
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
			// Shift is down
			if (IsShiftKeyDown) {
				int nIDOfFirstControl = IDC_EDIT_ANESTH_HOURS;
				// (j.jones 2013-07-10 16:05) - PLID 57148 - The invoice number can now
				// optionally be the first control, otherwise it's anesthesia hours.
				// Don't check the invoice number preference, because we show the field
				// if we have an invoice ID, even if the pref. is off.
				if(m_nxeditInvoiceNumber.IsWindowVisible()) {
					nIDOfFirstControl = IDC_EDIT_INVOICE_NUMBER;
				}
				if (GetFocus() == GetDlgItem(nIDOfFirstControl)) {
					GetParent()->SetFocus();
					m_peditBillDescription->SetFocus();
					return TRUE;
				}
			}
			else {
				if (GetFocus() == GetDlgItem(IDC_BTN_EXTRA_CHARGE_INFO)) {
					GetParent()->SetFocus();
					m_peditBillDate->SetFocus();
					return TRUE;
				}
			}
		}
	}NxCatchAll("Error triggering billing hotkey.");

	return CNxDialog::PreTranslateMessage(pMsg);
}

CString CBilling2Dlg::GetAnesthStartTime()
{
	COleDateTime dtAnesthStart;

	if(m_nxtAnesthStart->GetStatus() == 1) {
		dtAnesthStart = m_nxtAnesthStart->GetDateTime();
	}
	else {
		dtAnesthStart.SetStatus(COleDateTime::invalid);
	}

	if(dtAnesthStart.GetStatus() != COleDateTime::invalid) {
		return dtAnesthStart.Format("%H:%M:%S");
	}

	return "";
}

CString CBilling2Dlg::GetAnesthEndTime()
{
	COleDateTime dtAnesthEnd;

	if(m_nxtAnesthEnd->GetStatus() == 1) {
		dtAnesthEnd = m_nxtAnesthEnd->GetDateTime();
	}
	else {
		dtAnesthEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtAnesthEnd.GetStatus() != COleDateTime::invalid) {
		return dtAnesthEnd.Format("%H:%M:%S");
	}
	
	return "";
}

CString CBilling2Dlg::GetFacilityStartTime()
{
	COleDateTime dtFacilityStart;

	if(m_nxtFacilityStart->GetStatus() == 1) {
		dtFacilityStart = m_nxtFacilityStart->GetDateTime();
	}
	else {
		dtFacilityStart.SetStatus(COleDateTime::invalid);
	}

	if(dtFacilityStart.GetStatus() != COleDateTime::invalid) {
		return dtFacilityStart.Format("%H:%M:%S");
	}

	return "";
}

CString CBilling2Dlg::GetFacilityEndTime()
{
	COleDateTime dtFacilityEnd;

	if(m_nxtFacilityEnd->GetStatus() == 1) {
		dtFacilityEnd = m_nxtFacilityEnd->GetDateTime();
	}
	else {
		dtFacilityEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtFacilityEnd.GetStatus() != COleDateTime::invalid) {
		return dtFacilityEnd.Format("%H:%M:%S");
	}

	return "";
}

long CBilling2Dlg::GetAnesthMinutes()
{
	return m_nCurTotalAnesthMinutes;
}

long CBilling2Dlg::GetFacilityMinutes()
{
	return m_nCurTotalFacilityMinutes;
}

void CBilling2Dlg::SetAnesthStartTime(CString strTime, BOOL bUpdateCharges /*= FALSE*/)
{
	COleDateTime dt;
	if(strTime != "" && dt.ParseDateTime("1/1/1900 " + strTime)) {
		m_nxtAnesthStart->SetDateTime(dt);
	}
	else {
		m_nxtAnesthStart->Clear();
	}
}

void CBilling2Dlg::SetAnesthEndTime(CString strTime, BOOL bUpdateCharges /*= FALSE*/)
{
	COleDateTime dt;
	if(strTime != "" && dt.ParseDateTime("1/1/1900 " + strTime)) {
		m_nxtAnesthEnd->SetDateTime(dt);
	}
	else {
		m_nxtAnesthEnd->Clear();
	}
}

void CBilling2Dlg::SetFacilityStartTime(CString strTime, BOOL bUpdateCharges /*= FALSE*/)
{
	COleDateTime dt;
	if(strTime != "" && dt.ParseDateTime("1/1/1900 " + strTime)) {
		m_nxtFacilityStart->SetDateTime(dt);
	}
	else {
		m_nxtFacilityStart->Clear();
	}
}

void CBilling2Dlg::SetFacilityEndTime(CString strTime, BOOL bUpdateCharges /*= FALSE*/)
{
	COleDateTime dt;
	if(strTime != "" && dt.ParseDateTime("1/1/1900 " + strTime)) {
		m_nxtFacilityEnd->SetDateTime(dt);
	}
	else {
		m_nxtFacilityEnd->Clear();
	}
}

void CBilling2Dlg::SetAnesthMinutes(long nMinutes, BOOL bUpdateCharges /*= FALSE*/)
{
	m_nCurTotalAnesthMinutes = nMinutes;
	SetDlgItemInt(IDC_EDIT_TOTAL_ANESTH_MINUTES, nMinutes);
	
	long nHours = nMinutes / 60;
	long nNewMinutes = nMinutes - (nHours * 60);
	SetDlgItemInt(IDC_EDIT_ANESTH_HOURS, nHours);
	SetDlgItemInt(IDC_EDIT_ANESTH_MINUTES, nNewMinutes);
	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nNewMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	if(bUpdateCharges && ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling)
		((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnAnesthesiaTimeChanged(m_nCurTotalAnesthMinutes);
}

void CBilling2Dlg::SetFacilityMinutes(long nMinutes, BOOL bUpdateCharges /*= FALSE*/)
{
	m_nCurTotalFacilityMinutes = nMinutes;
	SetDlgItemInt(IDC_EDIT_TOTAL_FACILITY_MINUTES, nMinutes);

	long nHours = nMinutes / 60;
	long nNewMinutes = nMinutes - (nHours * 60);
	SetDlgItemInt(IDC_EDIT_FACILITY_HOURS, nHours);
	SetDlgItemInt(IDC_EDIT_FACILITY_MINUTES, nNewMinutes);
	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nNewMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	if(bUpdateCharges && ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling)
		((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnFacilityTimeChanged(m_nCurTotalFacilityMinutes);
}

// (j.jones 2011-11-01 08:55) - PLID 41558 - added assisting times
CString CBilling2Dlg::GetAssistingStartTime()
{
	COleDateTime dtAssistingStart;

	if(m_nxtAssistingStart->GetStatus() == 1) {
		dtAssistingStart = m_nxtAssistingStart->GetDateTime();
	}
	else {
		dtAssistingStart.SetStatus(COleDateTime::invalid);
	}

	if(dtAssistingStart.GetStatus() != COleDateTime::invalid) {
		return dtAssistingStart.Format("%H:%M:%S");
	}

	return "";
}

CString CBilling2Dlg::GetAssistingEndTime()
{
	COleDateTime dtAssistingEnd;

	if(m_nxtAssistingEnd->GetStatus() == 1) {
		dtAssistingEnd = m_nxtAssistingEnd->GetDateTime();
	}
	else {
		dtAssistingEnd.SetStatus(COleDateTime::invalid);
	}

	if(dtAssistingEnd.GetStatus() != COleDateTime::invalid) {
		return dtAssistingEnd.Format("%H:%M:%S");
	}

	return "";
}

long CBilling2Dlg::GetAssistingMinutes()
{
	return m_nCurTotalAssistingMinutes;
}

void CBilling2Dlg::SetAssistingStartTime(CString strTime, BOOL bUpdateCharges /*= FALSE*/)
{
	COleDateTime dt;
	if(strTime != "" && dt.ParseDateTime("1/1/1900 " + strTime)) {
		m_nxtAssistingStart->SetDateTime(dt);
	}
	else {
		m_nxtAssistingStart->Clear();
	}
}

void CBilling2Dlg::SetAssistingEndTime(CString strTime, BOOL bUpdateCharges /*= FALSE*/)
{
	COleDateTime dt;
	if(strTime != "" && dt.ParseDateTime("1/1/1900 " + strTime)) {
		m_nxtAssistingEnd->SetDateTime(dt);
	}
	else {
		m_nxtAssistingEnd->Clear();
	}
}

void CBilling2Dlg::SetAssistingMinutes(long nMinutes, BOOL bUpdateCharges /*= FALSE*/)
{
	m_nCurTotalAssistingMinutes = nMinutes;
	SetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES, nMinutes);
	
	long nHours = nMinutes / 60;
	long nNewMinutes = nMinutes - (nHours * 60);
	SetDlgItemInt(IDC_EDIT_ASSISTING_HOURS, nHours);
	SetDlgItemInt(IDC_EDIT_ASSISTING_MINUTES, nNewMinutes);
	CString strHours, strMinutes;
	strHours.Format("%li",nHours);
	strMinutes.Format("%li",nNewMinutes);
	if(strMinutes.GetLength() == 1)
		strMinutes = "0" + strMinutes;

	if(bUpdateCharges && ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling) {
		((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnAssistingTimeChanged(m_nCurTotalAssistingMinutes);
	}
}

void CBilling2Dlg::OnBtnEditDischargeStatus() 
{
	try {

		//save the current value

		long nCurID = -1;
		if(m_DischargeStatusCombo->GetCurSel() != NULL) {
			nCurID = VarLong(NXDATALIST2Lib::IRowSettingsPtr(m_DischargeStatusCombo->GetCurSel())->GetValue(0), -1);
		}

		CEditDischargeStatusDlg dlg(this);
		dlg.DoModal();

		// (j.jones 2007-05-02 15:24) - PLID 25883 - supported DischargeStatusT tablechecker
		if (m_DischargeStatusChecker.Changed()) {

			m_DischargeStatusCombo->Requery();

			//now set the old value, if it exists
			if(nCurID != -1) {
				m_DischargeStatusCombo->SetSelByColumn(0, nCurID);
			}
		}

	}NxCatchAll("CBilling2Dlg::OnBtnEditDischargeStatus");
}

void CBilling2Dlg::OnRequeryFinishedDischargeStatusCombo(short nFlags) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DischargeStatusCombo->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "");
		pRow->PutValue(2, "<No Status Selected>");
		m_DischargeStatusCombo->AddRowBefore(pRow, m_DischargeStatusCombo->GetFirstRow());

	}NxCatchAll("CBilling2Dlg::OnRequeryFinishedDischargeStatusCombo");
}

// (j.gruber 2006-12-29 15:02) - PLID 23972 - check that it is valid
void CBilling2Dlg::OnKillfocusDeductLeftToMeet() 
{
	CString strValue;
	GetDlgItemText(IDC_DEDUCT_LEFT_TO_MEET, strValue);
	strValue.TrimLeft();
	strValue.TrimRight();
	if (!strValue.IsEmpty() ) {
		COleCurrency cyDeductible = ParseCurrencyFromInterface(strValue);
		if (cyDeductible.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount.");
			SetDlgItemText(IDC_DEDUCT_LEFT_TO_MEET, m_strDeductLeftToMeet);
		}
		else {
			SetDlgItemText(IDC_DEDUCT_LEFT_TO_MEET, FormatCurrencyForInterface(cyDeductible));
			m_strDeductLeftToMeet = FormatCurrencyForInterface(cyDeductible);
		}
	}
	
}

// (j.gruber 2006-12-29 15:02) - PLID 23972 - check that it is valid
void CBilling2Dlg::OnKillfocusOopLeftToMeet() 
{
	CString strValue;
	GetDlgItemText(IDC_OOP_LEFT_TO_MEET, strValue);
	strValue.TrimLeft();
	strValue.TrimRight();
	if (!strValue.IsEmpty() ) {
		COleCurrency cyOOP = ParseCurrencyFromInterface(strValue);
		if (cyOOP.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount.");
			SetDlgItemText(IDC_OOP_LEFT_TO_MEET, m_strOOPLeftToMeet);
		}
		else {
			SetDlgItemText(IDC_OOP_LEFT_TO_MEET, FormatCurrencyForInterface(cyOOP));
			m_strOOPLeftToMeet = FormatCurrencyForInterface(cyOOP);
		}
	}
	
}

// (j.gruber 2006-12-29 15:02) - PLID 23972 - check that it is valid 
void CBilling2Dlg::OnKillfocusPatCoInsurance() 
{
	
}

void CBilling2Dlg::UpdateRewardPointsDisplay()
{
	try {
		COleCurrency cyAdjTotalPoints = ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetAdjustedRewardPoints();
		SetDlgItemText(IDC_REWARD_POINTS_TOTAL_EDIT, FormatCurrencyForInterface(cyAdjTotalPoints, FALSE, TRUE));
	} NxCatchAllCall("Error in CBilling2Dlg::UpdateRewardPointsDisplay()", SetDlgItemText(IDC_REWARD_POINTS_TOTAL_EDIT, ""););
}

// (a.walling 2008-05-05 13:20) - PLID 29897 - Patient Name
CString CBilling2Dlg::GetBillPatientName()
{
	ASSERT(m_pBillingModuleWnd);

	if (m_pBillingModuleWnd)
		return ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetBillPatientName();
	else return "";
}

// (j.jones 2008-05-28 10:35) - PLID 28782 - added OnBtnExtraChargeInfo
void CBilling2Dlg::OnBtnExtraChargeInfo() 
{
	try {

		if(m_pBillingModuleWnd == NULL) {
			ThrowNxException("Cannot access the bill. (m_pBillingModuleWnd == NULL)");
		}
		if(!((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.GetSafeHwnd()) {
			ThrowNxException("Cannot access the billing tab. (m_dlgBilling.GetSafeHwnd() returned FALSE)");
		}

	// (a.walling 2014-02-24 11:27) - PLID 61003 - CPtrArray g_aryBillingTabInfoT in CBillingDlg et al should instead be a typed collection: vector<BillingItemPtr> m_billingItems.

		std::vector<BillingItemPtr>& chargeArray = ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.GetBillingItems();

		if(chargeArray.empty()) {
			AfxMessageBox("There are no charges currently on the bill. You must add charges before you can edit their data.");
			return;
		}

		CBillingExtraChargeInfoDlg dlg(this, chargeArray, ((CBillingModuleDlg*)m_pBillingModuleWnd), m_eHasAccess == batNoAccess);
		// (j.jones 2011-01-21 10:10) - PLID 42156 - the access level is now an enum,
		// this screen is accessible even if you have partial access
		dlg.DoModal();

		//don't need to do anything else here, the dialog would have updated the array upon OK

	} NxCatchAll("Error in CBilling2Dlg::OnBtnExtraChargeInfo");
}

// (j.jones 2009-08-11 12:22) - PLID 35142 - supported multiple case histories
LRESULT CBilling2Dlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {

		//do nothing if the screen is read only
		if (m_bIsCaseHistoryComboHidden && m_bIsSurgeryCenter && m_eHasAccess != batNoAccess) {
			
			if((UINT)wParam == IDC_CASE_HISTORY_LINK_LABEL) {
				OnMultipleCaseHistories();
			}
		}

	}NxCatchAll(__FUNCTION__)

	return 0;
}

// (j.jones 2009-08-11 12:22) - PLID 35142 - supported multiple case histories
BOOL CBilling2Dlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {

		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		//do nothing if the screen is read only
		if (m_bIsCaseHistoryComboHidden && m_bIsSurgeryCenter && m_eHasAccess != batNoAccess) {
			
			CRect rc;
			GetDlgItem(IDC_CASE_HISTORY_LINK_LABEL)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

	}NxCatchAll(__FUNCTION__)

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CBilling2Dlg::OnMultipleCaseHistories()
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "CaseHistoryT");

		long nPatientID = ((CBillingModuleDlg*)m_pBillingModuleWnd)->m_nPatientID;

		//see if we have anything already

		// (j.jones 2009-08-12 15:54) - PLID 35179 - since we no longer have an array only of IDs,
		// we will have to build an array of IDs with which to call preselect
		CArray<long, long> aryCurrentCaseHistoryIDs;
		int i=0;
		for(i=0; i<((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.GetSize(); i++) {
			CaseHistoryInfo *pCase = (CaseHistoryInfo*)((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.GetAt(i);
			aryCurrentCaseHistoryIDs.Add(pCase->nCaseHistoryID);
		}

		dlg.PreSelect(aryCurrentCaseHistoryIDs);

		dlg.m_strNameColTitle = "Case History";

		//show all case histories whether they are completed or not

		CString strWhere;
		strWhere.Format("PersonID = %li", nPatientID);

		CStringArray aryOtherFields, aryOtherNames;
		aryOtherFields.Add("SurgeryDate");
		aryOtherFields.Add("dbo.AsDateNoTime(CompletedDate)");
		aryOtherNames.Add("Surgery Date");
		aryOtherNames.Add("Completed Date");

		dlg.m_astrOrderColumns.Add("SurgeryDate");
		dlg.m_abSortAscending.Add(FALSE);

		int res = dlg.Open("CaseHistoryT", strWhere, "CaseHistoryT.ID", "CaseHistoryT.Name", "Select Case Histories", 0, -1, &aryOtherFields, &aryOtherNames);

		BringWindowToTop();

		if(res == IDCANCEL) {
			ToggleCaseHistoryDisplay();
			return;
		}

		//update our tracked IDs
		
		// (j.jones 2009-08-12 15:32) - PLID 35179 - this is now an array of pointers
		((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.ClearCaseHistoryArray();

		// (j.jones 2009-08-12 15:54) - PLID 35179 - since we no longer track an array only of IDs,
		// we will have to rebuild the contents m_arypBilledCaseHistories
		CArray<long, long> aryNewCaseHistoryIDs;
		dlg.FillArrayWithIDs(aryNewCaseHistoryIDs);		

		for(i=0; i<aryNewCaseHistoryIDs.GetSize(); i++) {

			long nCaseHistoryID = (long)aryNewCaseHistoryIDs.GetAt(i);

			CString strCaseHistoryName;
			COleDateTime dtSurgeryDate;

			//the case should be in our dropdown
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_CaseHistoryCombo->FindByColumn(chccID, (long)nCaseHistoryID, m_CaseHistoryCombo->GetFirstRow(), FALSE);
			if(pRow) {
				strCaseHistoryName = VarString(pRow->GetValue(chccDescription));
				dtSurgeryDate = VarDateTime(pRow->GetValue(chccSurgeryDate));
			}
			else {
				//if it's not in the list, we need to load from data, but that should not be possible
				//unless a case history was added to the patient's account since the list initially requeried
				_RecordsetPtr rs = CreateParamRecordset("SELECT Name, SurgeryDate FROM CaseHistoryT WHERE ID = {INT}", nCaseHistoryID);
				if(!rs->eof) {
					strCaseHistoryName = AdoFldString(rs, "Name");
					dtSurgeryDate = AdoFldDateTime(rs, "SurgeryDate");
				}
				else {
					//this should be impossible unless someone *just* deleted the case history
					ASSERT(FALSE);
					continue;
				}
				rs->Close();
			}

			CaseHistoryInfo *pNewCase = new CaseHistoryInfo;
			pNewCase->nCaseHistoryID = nCaseHistoryID;
			pNewCase->strCaseHistoryName = strCaseHistoryName;
			pNewCase->dtSurgeryDate = dtSurgeryDate;
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.Add(pNewCase);
		}

		ToggleCaseHistoryDisplay();

	} NxCatchAll("Error in CBilling2Dlg::OnMultipleCaseHistories");
}

void CBilling2Dlg::ToggleCaseHistoryDisplay()
{
	try {

		if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.GetSize() <= 1) {
			
			m_bIsCaseHistoryComboHidden = FALSE;

			m_nxlCaseHistoryLinkLabel.SetText("");

			((CWnd*)GetDlgItem(IDC_CASE_HISTORY_BILL_COMBO))->ShowWindow(SW_SHOWNOACTIVATE);
			ShowDlgItem(IDC_CASE_HISTORY_LINK_LABEL, SW_HIDE);

			if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.GetSize() == 1) {

				CaseHistoryInfo *pCase = (CaseHistoryInfo*)((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.GetAt(0);

				m_CaseHistoryCombo->SetSelByColumn(chccID, (long)pCase->nCaseHistoryID);
			}
			else {
				m_CaseHistoryCombo->SetSelByColumn(chccID, (long)-1);
			}
			return;
		}

		m_bIsCaseHistoryComboHidden = TRUE;

		//hide the datalist
		((CWnd*)GetDlgItem(IDC_CASE_HISTORY_BILL_COMBO))->ShowWindow(SW_HIDE);
		
		//update the hypertext
		ShowDlgItem(IDC_CASE_HISTORY_LINK_LABEL, SW_SHOWNOACTIVATE);
		m_nxlCaseHistoryLinkLabel.SetText(GetStringOfCaseHistories());
		m_nxlCaseHistoryLinkLabel.SetType(m_eHasAccess != batNoAccess ? dtsHyperlink : dtsDisabledHyperlink);
		m_nxlCaseHistoryLinkLabel.Invalidate();
	
	}NxCatchAll("Error displaying Resource filter information.");
}

CString CBilling2Dlg::GetStringOfCaseHistories() {

	CString strNames;

	if(!m_bIsSurgeryCenter) {
		return "";
	}

	// (j.jones 2009-08-12 15:50) - PLID 35179 - this is now an array of pointers, which means we don't need to
	// query data to get the names of the case histories
	for(int i=0; i<((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.GetSize(); i++) {

		CaseHistoryInfo *pCase = (CaseHistoryInfo*)((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.GetAt(i);
		if(!strNames.IsEmpty()) {
			strNames += ", ";
		}
		strNames += pCase->strCaseHistoryName;
	}	

	return strNames;
}

// (j.jones 2009-08-11 12:22) - PLID 35142 - supported multiple case histories
void CBilling2Dlg::OnSelChosenCaseHistoryBillCombo(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nCaseHistoryID = -1;
		if(pRow) {
			nCaseHistoryID = VarLong(pRow->GetValue(chccID), -1);
		}

		if(nCaseHistoryID == -1) {
			// (j.jones 2009-08-12 15:32) - PLID 35179 - this is now an array of pointers
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.ClearCaseHistoryArray();
		}
		else if(nCaseHistoryID == -2) {
			OnMultipleCaseHistories();
			return;
		}
		else {
			// (j.jones 2009-08-12 15:32) - PLID 35179 - this is now an array of pointers
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.ClearCaseHistoryArray();

			CString strCaseHistoryName = VarString(pRow->GetValue(chccDescription));
			COleDateTime dtSurgeryDate = VarDateTime(pRow->GetValue(chccSurgeryDate));

			CaseHistoryInfo *pNewCase = new CaseHistoryInfo;
			pNewCase->nCaseHistoryID = nCaseHistoryID;
			pNewCase->strCaseHistoryName = strCaseHistoryName;
			pNewCase->dtSurgeryDate = dtSurgeryDate;
			((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.m_arypBilledCaseHistories.Add(pNewCase);
		}

		ToggleCaseHistoryDisplay();
		
	}NxCatchAll(__FUNCTION__);
}

void CBilling2Dlg::SelChangingBillAffiliateList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CBilling2Dlg::SelChosenBillAffiliateList(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			m_nCurrentAffiliatePhysID = VarLong(pRow->GetValue(aflcID));
			m_strCurrentAffiliatePhysName = VarString(pRow->GetValue(aflcName));
		}

	}NxCatchAll(__FUNCTION__);
}

void CBilling2Dlg::RequeryFinishedBillAffiliateList(short nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAffiliatePhysList->GetNewRow();
		if (pRow) {
			pRow->PutValue(aflcID, (long)-1);
			pRow->PutValue(aflcName, _variant_t("<No Affiliate Physician>"));

			m_pAffiliatePhysList->AddRowBefore(pRow, m_pAffiliatePhysList->GetFirstRow());
		}

		if (m_nCurrentAffiliatePhysID == -2) {
			m_pAffiliatePhysList->PutComboBoxText(_bstr_t(m_strCurrentAffiliatePhysName));
		}
		else if (m_nCurrentAffiliatePhysID == -1) {
			m_pAffiliatePhysList->CurSel = pRow;
		}
		else {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAffiliatePhysList->SetSelByColumn(aflcID, m_nCurrentAffiliatePhysID);
			//the provider they have on this bill has now been inactivated, or is not an affiliate anymore
			if (pRow == NULL) {
				m_pAffiliatePhysList->PutComboBoxText(_bstr_t(m_strCurrentAffiliatePhysName));
				m_nCurrentAffiliatePhysID = -2;
			}
		}		
		
	}NxCatchAll(__FUNCTION__);
}

void CBilling2Dlg::SelChangingAffilStatus(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CBilling2Dlg::SelChosenAffilStatus(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			SetDlgItemText(IDC_AFFIL_DATE, FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), DTF_STRIP_SECONDS));
		}			
	}NxCatchAll(__FUNCTION__);
}

void CBilling2Dlg::OnEnKillfocusAffilAmt()
{
	try {
		CString strAmount;
		GetDlgItemText(IDC_AFFIL_AMT, strAmount);
		if (!strAmount.IsEmpty()) {
			SetDlgItemText(IDC_AFFIL_AMT, FormatCurrencyForInterface(ParseCurrencyFromInterface(strAmount)));
		}
	}NxCatchAll(__FUNCTION__);
}

void CBilling2Dlg::TrySetSelFinishedBillAffiliateList(long nRowEnum, long nFlags)
{
	try {
		if (nFlags == NXDATALIST2Lib::dlTrySetSelFinishedSuccess) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAffiliateStatusList->CurSel;
			if (pRow) {
				long nAffiliatePhysID = VarLong(pRow->GetValue(aflcID));
				m_nOldAffiliatePhysID = nAffiliatePhysID;
				m_nCurrentAffiliatePhysID = nAffiliatePhysID;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-11-01 10:02) - PLID 41558 - added Assisting controls (for OHIP)
void CBilling2Dlg::OnEnKillfocusEditAssistingHours()
{
	try {

		long nHours = GetDlgItemInt(IDC_EDIT_ASSISTING_HOURS);
		long nMinutes = GetDlgItemInt(IDC_EDIT_ASSISTING_MINUTES);
		if(nHours > 24) {
			AfxMessageBox("You cannot have more than 24 assisting hours.");
			nHours = 0;
			SetDlgItemInt(IDC_EDIT_ASSISTING_HOURS, 0);
		}
		else if(nHours == 24 && nMinutes > 0) {
			AfxMessageBox("You cannot have more than 24 assisting hours.");
			nHours = 0;
			nMinutes = 0;
			SetDlgItemInt(IDC_EDIT_ASSISTING_HOURS, 0);
			SetDlgItemInt(IDC_EDIT_ASSISTING_MINUTES, 0);
		}

		CString strHours, strMinutes;
		strHours.Format("%li",nHours);
		strMinutes.Format("%li",nMinutes);
		if(strMinutes.GetLength() == 1)
			strMinutes = "0" + strMinutes;

		long nTotalMinutes = (nHours * 60) + nMinutes;
		SetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES, nTotalMinutes);

		if(m_nCurTotalAssistingMinutes != nTotalMinutes) {

			m_nCurTotalAssistingMinutes = nTotalMinutes;

			if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling) {
				((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnAssistingTimeChanged(nTotalMinutes);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-11-01 10:02) - PLID 41558 - added Assisting controls (for OHIP)
void CBilling2Dlg::OnEnKillfocusEditAssistingMinutes()
{
	try {

		long nHours = GetDlgItemInt(IDC_EDIT_ASSISTING_HOURS);
		long nMinutes = GetDlgItemInt(IDC_EDIT_ASSISTING_MINUTES);
		if(nMinutes > 59) {
			AfxMessageBox("You cannot have more than 59 assisting minutes.");
			nMinutes = 0;
			SetDlgItemInt(IDC_EDIT_ASSISTING_MINUTES, 0);
		}
		else if(nHours == 24 && nMinutes > 0) {
			AfxMessageBox("You cannot have more than 24 assisting hours.");
			nHours = 0;
			nMinutes = 0;
			SetDlgItemInt(IDC_EDIT_ASSISTING_HOURS, 0);
			SetDlgItemInt(IDC_EDIT_ASSISTING_MINUTES, 0);
		}

		CString strHours, strMinutes;
		strHours.Format("%li",nHours);
		strMinutes.Format("%li",nMinutes);
		if(strMinutes.GetLength() == 1)
			strMinutes = "0" + strMinutes;

		long nTotalMinutes = (nHours * 60) + nMinutes;
		SetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES, nTotalMinutes);

		if(m_nCurTotalAssistingMinutes != nTotalMinutes) {

			m_nCurTotalAssistingMinutes = nTotalMinutes;

			if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling) {
				((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnAssistingTimeChanged(nTotalMinutes);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-11-01 10:02) - PLID 41558 - added Assisting controls (for OHIP)
void CBilling2Dlg::OnEnKillfocusEditTotalAssistingMinutes()
{
	try {

		long nTotalMinutes = GetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES);

		if(nTotalMinutes > 1440) {
			AfxMessageBox("You cannot have more than 1440 assistin minutes (24 hours).");
			nTotalMinutes = 1440;
			SetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES, nTotalMinutes);
		}

		long nHours = nTotalMinutes / 60;
		long nMinutes = nTotalMinutes - (nHours * 60);
		SetDlgItemInt(IDC_EDIT_ASSISTING_HOURS, nHours);
		SetDlgItemInt(IDC_EDIT_ASSISTING_MINUTES, nMinutes);
		CString strHours, strMinutes;
		strHours.Format("%li",nHours);
		strMinutes.Format("%li",nMinutes);
		if(strMinutes.GetLength() == 1)
			strMinutes = "0" + strMinutes;

		if(m_nCurTotalAssistingMinutes != nTotalMinutes) {

			m_nCurTotalAssistingMinutes = nTotalMinutes;
		
			if(((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling) {
				((CBillingModuleDlg*)m_pBillingModuleWnd)->m_dlgBilling.OnAssistingTimeChanged(nTotalMinutes);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-11-01 10:02) - PLID 41558 - added Assisting controls (for OHIP)
void CBilling2Dlg::OnKillFocusAssistingStartTime() 
{
	try {

		if(m_nxtAssistingStart->GetStatus() == 3)
			//blank, that's okay
			return;

		if(m_nxtAssistingStart->GetStatus() == 2) {
			MessageBox("You have entered an invalid time");
			m_nxtAssistingStart->Clear();
			return;
		}

		COleDateTime dtStart, dtEnd;

		if(m_nxtAssistingStart->GetStatus() == 1) {
			dtStart = m_nxtAssistingStart->GetDateTime();
		}
		else {
			dtStart.SetStatus(COleDateTime::invalid);
		}

		if(m_nxtAssistingEnd->GetStatus() == 1) {
			dtEnd = m_nxtAssistingEnd->GetDateTime();
		}
		else {
			dtEnd.SetStatus(COleDateTime::invalid);
		}

		if(dtStart.GetStatus() == COleDateTime::invalid) {
			MessageBox("You have entered an invalid time");
			m_nxtAssistingStart->Clear();
			return;
		}

		//if a valid end time was entered, then update the minutes appropriately
		if(dtEnd.GetStatus() != COleDateTime::invalid) {

			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT "
				"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
				"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
				"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
				AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
				AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
				AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd));
			long nTotalMinutes = AdoFldLong(rs, "Minutes",0);

			if(nTotalMinutes < 0) {
				MessageBox("You must enter a valid time range (greater than zero minutes).");
				return;
			}

			//otherwise, set the total minutes and recalculate
			SetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES, nTotalMinutes);
			OnEnKillfocusEditTotalAssistingMinutes();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-11-01 10:02) - PLID 41558 - added Assisting controls (for OHIP)
void CBilling2Dlg::OnKillFocusAssistingEndTime() 
{
	try {

		if(m_nxtAssistingEnd->GetStatus() == 3)
			//blank, that's okay
			return;

		if(m_nxtAssistingEnd->GetStatus() == 2) {
			MessageBox("You have entered an invalid time");
			m_nxtAssistingEnd->Clear();
			return;
		}

		COleDateTime dtStart, dtEnd;

		if(m_nxtAssistingStart->GetStatus() == 1) {
			dtStart = m_nxtAssistingStart->GetDateTime();
		}
		else {
			dtStart.SetStatus(COleDateTime::invalid);
		}

		if(m_nxtAssistingEnd->GetStatus() == 1) {
			dtEnd = m_nxtAssistingEnd->GetDateTime();
		}
		else {
			dtEnd.SetStatus(COleDateTime::invalid);
		}

		if(dtEnd.GetStatus() == COleDateTime::invalid) {
			MessageBox("You have entered an invalid time");
			m_nxtAssistingEnd->Clear();
			return;
		}

		//if a valid start time was entered, then update the minutes appropriately
		if(dtStart.GetStatus() != COleDateTime::invalid) {

			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT "
				"CASE WHEN {OLEDATETIME} >= {OLEDATETIME} "
				"THEN DATEDIFF(minute,DATEADD(day,-1,{OLEDATETIME}),{OLEDATETIME}) "
				"ELSE DATEDIFF(minute,{OLEDATETIME},{OLEDATETIME}) END AS Minutes",
				AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
				AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd),
				AsTimeNoDate(dtStart),AsTimeNoDate(dtEnd));
			long nTotalMinutes = AdoFldLong(rs, "Minutes",0);

			if(nTotalMinutes < 0) {
				MessageBox("You must enter a valid time range (greater than zero minutes).");
				return;
			}

			//otherwise, set the total minutes and recalculate
			SetDlgItemInt(IDC_EDIT_TOTAL_ASSISTING_MINUTES, nTotalMinutes);
			OnEnKillfocusEditTotalAssistingMinutes();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-10 15:31) - PLID 57148 - added invoice number
void CBilling2Dlg::OnEnKillfocusEditInvoiceNumber()
{
	try {

		if(!m_boInitialized) {
			//the tab hasn't even been displayed yet, so this kill focus
			//is MFC nonsense, not an actual change by the user
			return;
		}

		if(!ValidateInvoiceNumber(false)) {
			//if it was bad for some reason, replace with the last known good number
			CString strOldInvoiceNumber = "";
			if(m_nCurrentInvoiceNumber != -1) {
				strOldInvoiceNumber.Format("%li", m_nCurrentInvoiceNumber);
			}
			m_nxeditInvoiceNumber.SetWindowText(strOldInvoiceNumber);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-10 15:31) - PLID 57148 - Ensures that the invoice number
// is valid and available for the current provider.
// If bIsSaving is false, it's assumed this was called from OnEnKillfocusEditInvoiceNumber.
bool CBilling2Dlg::ValidateInvoiceNumber(bool bIsSaving)
{
	try {

		//get the invoice number they typed in
		CString strInvoiceNumber = "";
		m_nxeditInvoiceNumber.GetWindowText(strInvoiceNumber);
		strInvoiceNumber.TrimRight();

		if(strInvoiceNumber.IsEmpty()) {
			//empty is permitted, but if there was a number previously
			//saved for this bill, warn first
			if(GetBillID() != -1 && m_nSavedInvoiceNumber != -1 && m_nCurrentInvoiceNumber != -1) {
				CString strWarn;
				strWarn.Format("Are you sure you wish to remove the invoice number %li from this bill? "
					"A new invoice number will not be generated.", m_nSavedInvoiceNumber);
				if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
					return false;
				}
			}
			SetCurrentInvoiceNumber(-1, -1);
			return true;
		}

		//make sure the number isn't too big to store
		{
			std::stringstream sstr((LPCTSTR)strInvoiceNumber);
			__int64 nnTest;
			sstr >> nnTest;
			if(nnTest > LONG_MAX) {
				CString strWarn;
				strWarn.Format("The Invoice Number you entered is too large. The largest number allowed is %li.", LONG_MAX);
				AfxMessageBox(strWarn);
				return false;
			}
		}

		long nNewInvoiceNumber = atoi(strInvoiceNumber);
		if(nNewInvoiceNumber == 0) {
			//somehow this is invalid
			AfxMessageBox("The Invoice Number you entered is invalid.");
			return false;
		}

		//if we get here, we have a valid number, but we need
		//to find out if it is already in use

		//now confirm that this value can be saved for the current provider
		long nFirstProviderID = ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetFirstChargeProviderID();
		if(nFirstProviderID == -1) {
			//without a provider, we have no idea whether this invoice number is valid
			if(bIsSaving) {
				//fail
				AfxMessageBox("You have entered an invoice number, but do not have a provider selected on any charges. "
					"Invoice numbers can not be entered on bills that do not have at least one provider selected.");
				return false;
			}
			else {
				//permit this, for now, but warn that it won't save until they pick a provider
				AfxMessageBox("You currently have no charges with a provider selected. "
					"You will not be able to save this invoice number until at least one charge has a provider selected.");

				//let them continue
			}
		}
		else {
			//we have a provider, now validate whether the invoice number has been used
			CSqlFragment sqlExcludeBill("");
			if(GetBillID() != -1) {
				//exclude the current bill, if we already have it in use for this bill, we don't care
				sqlExcludeBill = CSqlFragment("AND BillID <> {INT}", GetBillID());
			}
			_RecordsetPtr rs = CreateParamRecordset("SELECT BillID, Last + ', ' + First + ' ' + Middle AS ProviderName "
				"FROM BillInvoiceNumbersT "
				"INNER JOIN PersonT ON BillInvoiceNumbersT.ProviderID = PersonT.ID "
				"WHERE ProviderID = {INT} AND InvoiceID = {INT} {SQL}",
				nFirstProviderID, nNewInvoiceNumber, sqlExcludeBill);
			if(!rs->eof) {
				//<price is right failure horns> This invoice number is in use! </price is right failure horns>
				long nExistingBillID = VarLong(rs->Fields->Item["BillID"]->Value);
				CString strExistingProviderName = VarString(rs->Fields->Item["ProviderName"]->Value, "");
				strExistingProviderName.TrimRight();

				CString strWarn;
				strWarn.Format("The invoice ID of %li is already in use for %s on Bill ID %li. It cannot be used on this bill.",
					nNewInvoiceNumber, strExistingProviderName, nExistingBillID);
				AfxMessageBox(strWarn);
				return false;
			}
			rs->Close();
		}

		//if we get here, the value is one that can be saved, but if they are changing
		//this from an existing value, warn them first
		if(GetBillID() != -1 && m_nSavedInvoiceNumber != -1 && nNewInvoiceNumber != -1
			&& m_nSavedInvoiceNumber != nNewInvoiceNumber && m_nCurrentInvoiceNumber != nNewInvoiceNumber) {
			CString strWarn;
			strWarn.Format("Are you sure you wish to change this bill's invoice number from %li to %li?", m_nSavedInvoiceNumber, nNewInvoiceNumber);
			if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return false;
			}
		}

		//Keep this new value.
		//SetCurrentInvoiceNumber will assign it to our variable,
		//remove leading zeros and reset the edit box.
		SetCurrentInvoiceNumber(nNewInvoiceNumber, nFirstProviderID);
		return true;

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.jones 2013-07-10 16:13) - PLID 57148 - invoice number is not
// editable unless the user has access and a valid bill ID exists
void CBilling2Dlg::TryEnableInvoiceNumber()
{
	//if the user has writeable access, and a bill ID exists, then the invoice number is editable
	if(m_EntryType == 1 && GetBillID() != -1 && m_eHasAccess != batNoAccess) {
		m_nxeditInvoiceNumber.SetReadOnly(FALSE);
	}
	else {
		m_nxeditInvoiceNumber.SetReadOnly(TRUE);
	}
}

// (j.jones 2013-07-10 16:39) - PLID 57148 - given an invoice ID and provider,
// update the saved invoice fields, the current invoice fields, and the edit box
void CBilling2Dlg::SetSavedInvoiceNumber(long nInvoiceID, long nProviderID)
{
	m_nSavedInvoiceNumber = nInvoiceID;
	m_nSavedInvoiceProviderID = nProviderID;
	SetCurrentInvoiceNumber(m_nSavedInvoiceNumber, nProviderID);
}

// (j.jones 2013-07-10 16:39) - PLID 57148 - given an invoice ID,
// update the edit box, m_nCurrentInvoiceNumber, and m_nCurrentInvoiceProviderID
void CBilling2Dlg::SetCurrentInvoiceNumber(long nInvoiceID, long nProviderID)
{
	m_nCurrentInvoiceNumber = nInvoiceID;
	m_nCurrentInvoiceProviderID = nProviderID;
	CString strNewInvoiceNumber = "";
	if(m_nCurrentInvoiceNumber != -1) {
		strNewInvoiceNumber.Format("%li", m_nCurrentInvoiceNumber);
	}
	m_nxeditInvoiceNumber.SetWindowText(strNewInvoiceNumber);
}
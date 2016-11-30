// FinancialLineItemPostingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialLineItemPostingDlg.h"
#include "GlobalFinancialUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "EditComboBox.h"
#include "GlobalDrawingUtils.h"
#include "AuditTrail.h"
#include "EditCreditCardsDlg.h"
#include "FinancialLineItemPostingConfigureColumnsDlg.h"
#include "DontShowDlg.h"
#include "NotesDlg.h"
#include "NxModalParentDlg.h"
#include "MultipleAdjustmentEntryDlg.h"
#include "DontShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

extern CPracticeApp theApp;

using namespace NXDATALISTLib;

// (j.jones 2013-08-27 12:17) - PLID 57398 - this is the count of total dynamic columns
#define DYNAMIC_COLUMN_COUNT	13

// (j.jones 2008-07-11 13:52) - PLID 28756 - added enum for the responsibility combo
enum RespComboColumns {
	rccID = 0,
	rccName,
	rccPriority,
	rccDefaultPayDesc,
	rccDefaultPayCategoryID,
	rccDefaultAdjDesc,
	rccDefaultAdjCategoryID
};

enum CreditCardColumns{
	cccCardID = 0,
	cccCardName,
};

// (j.jones 2009-03-11 08:59) - PLID 32864 - added enum for the insured combos
enum InsuredPartyComboColumn {

	ipccID = 0,
	ipccName,
	ipccRespTypeID,
};

// (j.jones 2010-09-02 16:48) - PLID 40392 - added shift combo columns
enum ShiftRespComboColumn {

	srccID = 0,
	srccName,
	srccPriority,
	srccCategoryTypeID,
	srccCategoryName,
};

// (j.jones 2010-09-23 15:02) - PLID 40653 - added enums for group & reason codes
enum GroupCodeComboColumn {

	gcccID = 0,
	gcccCode,
	gcccDescription,
};

enum ReasonCodeComboColumn {

	rcccID = 0,
	rcccCode,
	rcccDescription,
};

// (j.jones 2014-06-30 15:29) - PLID 62642 - added category combo enum
enum CategoryComboColumn {

	cccID = 0,
	cccCategory,
};

/////////////////////////////////////////////////////////////////////////////
// CFinancialLineItemPostingDlg dialog


CFinancialLineItemPostingDlg::CFinancialLineItemPostingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFinancialLineItemPostingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFinancialLineItemPostingDlg)
		m_nBillID = -1;
		m_nChargeID = -1;
		m_nRevCodeID = -1;
		m_nPatientID = -1;
		m_nInsuredPartyID = -1;
		m_bIsPrimaryIns = FALSE;
		m_nBatchPaymentID = -1;		
		m_bUseRevCodes = FALSE;
		m_bIsEditingCharges = FALSE;
		m_nLineItemPostingAutoAdjust = 2;
		m_nLineItemPostingAutoAdjust_PrimaryOnly = 1;
		m_nLineItemPostingAutoAdjust_AllowOnZeroDollarPayments = 0;
		m_nLineItemPostingAutoPayment = 0;
		m_nLineItemPostingAutoPayment_Secondary = 0;
		m_nBillOldInsuredPartyID = -1;
		m_nBillOldOtherInsuredPartyID = -1;
		m_bIsAutoPosting = FALSE;
		// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType
		m_ePayType = eMedicalPayment;
		m_cyDefaultPaymentAmount = g_ccyInvalid;
		m_bIsCapitation = false;
	//}}AFX_DATA_INIT

	m_nChargesColIndex = -1;
	m_nPatRespColIndex = -1;
	m_nInsRespColIndex = -1;
	m_nNewPaysColIndex = -1;
	m_nAllowableColIndex = -1;
	m_nAdjustmentColIndex = -1;
	m_nPatBalanceColIndex = -1;
	m_nInsBalanceColIndex = -1;
	// (j.jones 2010-05-04 16:09) - PLID 25521 - existing pat/ins pays are now visible
	m_nExistingPatPaysColIndex = -1;
	m_nExistingInsPaysColIndex = -1;
	m_nDeductibleAmtColIndex = -1;
	m_nCoinsuranceAmtColIndex = -1;
	m_nCopayAmtColIndex = -1;

	m_cyUnappliedPatAmt = COleCurrency(0,0);
	m_cyUnappliedInsAmt = COleCurrency(0,0);

	m_hNotes = NULL;
	// (r.gonet 2016-01-25 10:35) - PLID 67942 - Use this window to display message boxes by default.
	m_pInterface = this;
}

CFinancialLineItemPostingDlg::~CFinancialLineItemPostingDlg()
{
	try {

		DestroyIcon((HICON)m_hNotes);

		// (j.jones 2012-05-08 09:29) - PLID 37165 - clear our adjustment info
		ClearAdjustmentPointers();

	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFinancialLineItemPostingDlg)
	DDX_Control(pDX, IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE, m_btnGroupByRev);
	DDX_Control(pDX, IDC_BTN_SWAP_INSCOS, m_btnSwapInscos);
	DDX_Control(pDX, IDOK, m_btnPost);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_SHIFT_PAID_AMOUNTS, m_checkShiftPaidAmounts);
	DDX_Control(pDX, IDC_CHECK_AUTO_BATCH, m_checkAutoBatch);
	DDX_Control(pDX, IDC_CHECK_AUTO_SWAP, m_checkAutoSwap);
	DDX_Control(pDX, IDC_RADIO_POST_UNBATCHED, m_radPostUnbatched);
	DDX_Control(pDX, IDC_RADIO_POST_PAPER_BATCH, m_radPostPaper);
	DDX_Control(pDX, IDC_RADIO_POST_ELECTRONIC_BATCH, m_radPostElectronic);
	DDX_Control(pDX, IDC_PAY_DATE, m_dtPayDate);
	DDX_Control(pDX, IDC_ADJ_DATE, m_dtAdjDate);
	DDX_Control(pDX, IDC_PAY_TOTAL, m_nxeditPayTotal);
	DDX_Control(pDX, IDC_CASH_RECEIVED_LIP, m_nxeditCashReceivedLip);
	DDX_Control(pDX, IDC_CHANGE_GIVEN_LIP, m_nxeditChangeGivenLip);
	DDX_Control(pDX, IDC_CC_NUMBER, m_nxeditCCLast4);
	DDX_Control(pDX, IDC_CC_EXP_DATE, m_nxeditCcExpDate);
	DDX_Control(pDX, IDC_CC_NAME_ON_CARD, m_nxeditCcNameOnCard);
	DDX_Control(pDX, IDC_EDIT_AUTHORIZATION, m_nxeditEditAuthorization);
	DDX_Control(pDX, IDC_CHECK_NO, m_nxeditCheckNo);
	DDX_Control(pDX, IDC_ACCOUNT_NUMBER, m_nxeditAccountNumber);
	DDX_Control(pDX, IDC_BANK_NAME, m_nxeditBankName);
	DDX_Control(pDX, IDC_BANK_NUMBER, m_nxeditBankNumber);
	DDX_Control(pDX, IDC_PAY_DESC, m_nxeditPayDesc);
	DDX_Control(pDX, IDC_ADJ_TOTAL, m_nxeditAdjTotal);
	DDX_Control(pDX, IDC_ADJ_DESC, m_nxeditAdjDesc);
	DDX_Control(pDX, IDC_POSTING_PATIENT_NAME_LABEL, m_nxstaticPostingPatientNameLabel);
	DDX_Control(pDX, IDC_POSTING_PATIENT_ID_LABEL, m_nxstaticPostingPatientIdLabel);
	DDX_Control(pDX, IDC_PAYMENT_LABEL, m_nxstaticPaymentLabel);
	DDX_Control(pDX, IDC_PAY_AMT_LABEL, m_nxstaticPayAmtLabel);
	DDX_Control(pDX, IDC_PAY_CURRENCY_SYMBOL, m_nxstaticPayCurrencySymbol);
	DDX_Control(pDX, IDC_PAY_DATE_LABEL, m_nxstaticPayDateLabel);
	DDX_Control(pDX, IDC_PAY_RESP_LABEL, m_nxstaticPayRespLabel);
	DDX_Control(pDX, IDC_PAY_CAT_LABEL, m_nxstaticPayCatLabel);
	DDX_Control(pDX, IDC_PAY_METHOD_LABEL, m_nxstaticPayMethodLabel);
	DDX_Control(pDX, IDC_CASH_RECEIVED_LIP_LABEL, m_nxstaticCashReceivedLipLabel);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_RECEIVED_LIP, m_nxstaticCurrencySymbolReceivedLip);
	DDX_Control(pDX, IDC_CHANGE_GIVEN_LIP_LABEL, m_nxstaticChangeGivenLipLabel);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_GIVEN_LIP, m_nxstaticCurrencySymbolGivenLip);
	DDX_Control(pDX, IDC_CARD_TYPE_LABEL, m_nxstaticCardTypeLabel);
	DDX_Control(pDX, IDC_CC_NUMBER_LABEL, m_nxstaticCcNumberLabel);
	DDX_Control(pDX, IDC_CC_EXP_DATE_LABEL, m_nxstaticCcExpDateLabel);
	DDX_Control(pDX, IDC_CC_NAME_ON_CARD_LABEL, m_nxstaticCcNameOnCardLabel);
	DDX_Control(pDX, IDC_AUTH_NUM_LABEL, m_nxstaticAuthNumLabel);
	DDX_Control(pDX, IDC_CHECK_NO_LABEL, m_nxstaticCheckNoLabel);
	DDX_Control(pDX, IDC_ACCT_NUM_LABEL, m_nxstaticAcctNumLabel);
	DDX_Control(pDX, IDC_BANK_NAME_LABEL, m_nxstaticBankNameLabel);
	DDX_Control(pDX, IDC_BANK_NUMBER_LABEL, m_nxstaticBankNumberLabel);
	DDX_Control(pDX, IDC_PAY_DESC_LABEL, m_nxstaticPayDescLabel);
	DDX_Control(pDX, IDC_ADJUSTMENT_LABEL, m_nxstaticAdjustmentLabel);
	DDX_Control(pDX, IDC_ADJ_AMT_LABEL, m_nxstaticAdjAmtLabel);
	DDX_Control(pDX, IDC_ADJ_CURRENCY_SYMBOL, m_nxstaticAdjCurrencySymbol);
	DDX_Control(pDX, IDC_ADJ_DATE_LABEL, m_nxstaticAdjDateLabel);
	DDX_Control(pDX, IDC_ADJ_CATEGORY_LABEL, m_nxstaticAdjCategoryLabel);
	DDX_Control(pDX, IDC_ADJ_GROUPCODE_LABEL, m_nxstaticAdjGroupcodeLabel);
	DDX_Control(pDX, IDC_ADJ_REASON_LABEL, m_nxstaticAdjReasonLabel);
	DDX_Control(pDX, IDC_ADJ_DESCRIPTION_LABEL, m_nxstaticAdjDescriptionLabel);
	DDX_Control(pDX, IDC_PAYMENT_BAL_LABEL, m_nxstaticPaymentBalLabel);
	DDX_Control(pDX, IDC_PAYMENT_BALANCE, m_nxstaticPaymentBalance);
	DDX_Control(pDX, IDC_ADJUSTMENT_BAL_LABEL, m_nxstaticAdjustmentBalLabel);
	DDX_Control(pDX, IDC_ADJUSTMENT_BALANCE, m_nxstaticAdjustmentBalance);
	DDX_Control(pDX, IDC_BILL_INSCO_LABEL, m_nxstaticBillInscoLabel);
	DDX_Control(pDX, IDC_BILL_OTHER_INSCO_LABEL, m_nxstaticBillOtherInscoLabel);
	DDX_Control(pDX, IDC_CURRENT_BATCH_LABEL, m_nxstaticCurrentBatchLabel);
	DDX_Control(pDX, IDC_SHIFT_BALANCES_LABEL, m_nxstaticShiftBalancesLabel);
	DDX_Control(pDX, IDC_TOTAL_LABEL_0, m_nxstaticTotalLabel0);
	DDX_Control(pDX, IDC_TOTAL_LABEL_1, m_nxstaticTotalLabel1);
	DDX_Control(pDX, IDC_TOTAL_LABEL_2, m_nxstaticTotalLabel2);
	DDX_Control(pDX, IDC_TOTAL_LABEL_3, m_nxstaticTotalLabel3);
	DDX_Control(pDX, IDC_TOTAL_LABEL_4, m_nxstaticTotalLabel4);
	DDX_Control(pDX, IDC_TOTAL_LABEL_5, m_nxstaticTotalLabel5);
	DDX_Control(pDX, IDC_TOTAL_LABEL_6, m_nxstaticTotalLabel6);
	DDX_Control(pDX, IDC_TOTAL_LABEL_7, m_nxstaticTotalLabel7);
	DDX_Control(pDX, IDC_TOTAL_LABEL_8, m_nxstaticTotalLabel8);
	DDX_Control(pDX, IDC_TOTAL_LABEL_9, m_nxstaticTotalLabel9);
	DDX_Control(pDX, IDC_TOTAL_LABEL_10, m_nxstaticTotalLabel10);
	DDX_Control(pDX, IDC_TOTAL_LABEL_11, m_nxstaticTotalLabel11);
	DDX_Control(pDX, IDC_TOTAL_LABEL_12, m_nxstaticTotalLabel12);
	DDX_Control(pDX, IDC_TOTAL_0, m_nxstaticTotal0);
	DDX_Control(pDX, IDC_TOTAL_1, m_nxstaticTotal1);
	DDX_Control(pDX, IDC_TOTAL_2, m_nxstaticTotal2);
	DDX_Control(pDX, IDC_TOTAL_3, m_nxstaticTotal3);
	DDX_Control(pDX, IDC_TOTAL_4, m_nxstaticTotal4);
	DDX_Control(pDX, IDC_TOTAL_5, m_nxstaticTotal5);
	DDX_Control(pDX, IDC_TOTAL_6, m_nxstaticTotal6);
	DDX_Control(pDX, IDC_TOTAL_7, m_nxstaticTotal7);
	DDX_Control(pDX, IDC_TOTAL_8, m_nxstaticTotal8);
	DDX_Control(pDX, IDC_TOTAL_9, m_nxstaticTotal9);
	DDX_Control(pDX, IDC_TOTAL_10, m_nxstaticTotal10);
	DDX_Control(pDX, IDC_TOTAL_11, m_nxstaticTotal11);
	DDX_Control(pDX, IDC_TOTAL_12, m_nxstaticTotal12);
	DDX_Control(pDX, IDC_CHECK_ALLOW_ZERO_DOLLAR_APPLIES, m_checkApplyZeroDollarPays);
	DDX_Control(pDX, IDC_UNAPPLIED_PAT_PAYS_LABEL, m_nxstaticPatAmtToUnapplyLabel);
	DDX_Control(pDX, IDC_UNAPPLIED_PAT_PAYS, m_nxstaticPatAmtToUnapply);
	DDX_Control(pDX, IDC_UNAPPLIED_INS_PAYS_LABEL, m_nxstaticInsAmtToUnapplyLabel);
	DDX_Control(pDX, IDC_UNAPPLIED_INS_PAYS, m_nxstaticInsAmtToUnapply);
	DDX_Control(pDX, IDC_BTN_BILL_NOTES, m_btnBillNotes);
	DDX_Control(pDX, IDC_BTN_FILTER_REASON_CODES, m_btnFilterReasonCodes);
	DDX_Control(pDX, IDC_BTN_EST_PAYMENTS, m_btnEstPayments);	
	DDX_Control(pDX, IDC_CB_CATEGORY_LABEL, m_nxstaticChargebackCategoryLabel);
	DDX_Control(pDX, IDC_CB_DESCRIPTION_LABEL, m_nxstaticChargebackDescriptionLabel);
	DDX_Control(pDX, IDC_EDIT_CB_DESC, m_btnEditChargebackDescriptions);
	DDX_Control(pDX, IDC_CB_DESC, m_nxeditChargebackDescription);
	DDX_Control(pDX, IDC_CHARGEBACK_TOTAL_LABEL, m_nxstaticChargebackTotalLabel);
	DDX_Control(pDX, IDC_CHARGEBACK_TOTAL, m_nxstaticChargebackTotal);
	DDX_Control(pDX, IDC_CHECK_AUTO_ADJUST_BALANCES, m_checkAutoAdjustBalances);
	DDX_Control(pDX, IDC_TOTAL_PAYMENTS_LABEL, m_nxstaticTotalPaymentsLabel);
	DDX_Control(pDX, IDC_TOTAL_PAYMENTS, m_nxstaticTotalPayments);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFinancialLineItemPostingDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_SWAP_INSCOS, OnBtnSwapInscos)
	ON_BN_CLICKED(IDC_EDIT_CARD_NAME, OnEditCardName)
	ON_EN_KILLFOCUS(IDC_CASH_RECEIVED_LIP, OnKillfocusCashReceived)
	ON_EN_KILLFOCUS(IDC_PAY_TOTAL, OnKillfocusPayTotal)
	ON_EN_KILLFOCUS(IDC_ADJ_TOTAL, OnKillfocusAdjTotal)
	ON_BN_CLICKED(IDC_EDIT_PAY_DESC, OnEditPayDesc)
	ON_BN_CLICKED(IDC_EDIT_ADJ_DESC, OnEditAdjDesc)
	ON_BN_CLICKED(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE, OnCheckGroupChargesByRevenueCode)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PAY_DATE, OnDatetimechangePayDate)
	ON_BN_CLICKED(IDC_CONFIGURE_COLUMNS, OnConfigureColumns)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_BILL_NOTES, OnBtnBillNotes)
	ON_BN_CLICKED(IDC_BTN_FILTER_REASON_CODES, OnBtnFilterReasonCodes)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BTN_EST_PAYMENTS, OnBtnEstPayments)
	ON_BN_CLICKED(IDC_EDIT_CB_DESC, OnBtnEditCbDesc)
	ON_BN_CLICKED(IDC_CHECK_AUTO_ADJUST_BALANCES, &CFinancialLineItemPostingDlg::OnCheckAutoAdjustBalances)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinancialLineItemPostingDlg message handlers

//(e.lally 2007-12-11) PLID 28325 - Updated "Privatize" cc number function to "Mask"

// (j.jones 2007-01-02 16:01) - PLID 24030 - altered to support revenue code parameters
// (j.jones 2012-08-16 10:13) - PLID 52162 - Changed the ID parameter so it always requires
// the bill ID, and having a charge ID will filter on that charge only. Also moved nBatchPaymentID in here.
// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType
int CFinancialLineItemPostingDlg::DoModal(EBatchPaymentPayType ePayType, long nBillID, long nOnlyShowChargeID /*= -1*/, long nBatchPaymentID /*= -1*/, long nInsuredPartyID /*= -1*/, BOOL bUseRevCodes /*= FALSE*/, long nRevCodeID /*= -1*/)
{
	try {

		m_bIsAutoPosting = FALSE;

		m_bUseRevCodes = bUseRevCodes;
		m_nRevCodeID = nRevCodeID;
		m_nBillID = nBillID;
		m_nBatchPaymentID = nBatchPaymentID;

		// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType
		m_ePayType = ePayType;

		// (j.jones 2014-07-02 14:57) - PLID 62548 - posting by revenue code is not allowed on vision payments
		if (m_bUseRevCodes && m_ePayType == eVisionPayment) {
			//no code should have permitted this
			ASSERT(FALSE);
			m_bUseRevCodes = FALSE;
		}

		// (j.jones 2014-06-27 11:00) - PLID 62548 - if they don't have the vision payment license,
		// turn vision payments into medical payments
		if (m_ePayType == eVisionPayment && !g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {
			m_ePayType = eMedicalPayment;
		}

		if(nOnlyShowChargeID != -1) {
			//a charge
			m_nChargeID = nOnlyShowChargeID;

			if(m_nBillID == -1) {
				//if the bill ID was not provided, query it

				//there should be no code that fails to provide a bill ID
				ASSERT(FALSE);

				_RecordsetPtr rs = CreateParamRecordset("SELECT BillID FROM ChargesT WHERE ID = {INT}",m_nChargeID);
				if(!rs->eof) {
					m_nBillID = AdoFldLong(rs, "BillID",-1);
				}
				rs->Close();
			}
		}

		// (j.jones 2013-03-25 17:35) - PLID 55686 - Line item posting will default to the highest insurance
		// resp. on this charge or bill, whoever owes the most becomes the default insured party.
		// This calculation has been moved to be inside the dialog itself, and we tell the dialog to do it
		// by passing in -2.
		if(nInsuredPartyID == -2) {
			if(nBatchPaymentID != -1) {
				//should never have been called with a -2 insured party ID when posting from a batch payment
				ThrowNxException("FinancialLineItemPosting called with a -2 insured party ID for a batch payment.");
			}
			if(m_nChargeID != -1) {
				m_nInsuredPartyID = GetChargeInsuredPartyIDWithHighestBalance(m_nChargeID);
			}
			else if(m_nBillID != -1) {
				m_nInsuredPartyID = GetBillInsuredPartyIDWithHighestBalance(m_nBillID);
			}
			else {
				ASSERT(FALSE);
				//we should always be given a bill ID or charge ID, but if not,
				//then just default to patient resp.
				m_nInsuredPartyID = -1;
			}
		}
		else {
			m_nInsuredPartyID = nInsuredPartyID;
		}

		// (j.jones 2012-08-08 11:17) - PLID 47778 - m_bIsPrimaryIns is true if the current insured party
		// is the patient's primary insurance, either for medical or for vision
		m_bIsPrimaryIns = IsInsuredPartySentAsPrimary(m_nInsuredPartyID);

		return CDialog::DoModal();

	}NxCatchAll("Error loading posting screen.");

	return 0;	
}

// (j.jones 2012-08-16 10:06) - PLID 52162 - added override for Create(), using our required parameters
// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType
// (j.jones 2015-10-20 08:48) - PLID 67377 - added default payment amount, used for auto-posting to
// one charge only, when the default amount has already been calculated
// (j.jones 2015-10-23 14:10) - PLID 67377 - added flag to indicate if this is a capitation payment
BOOL CFinancialLineItemPostingDlg::Create(UINT nIDTemplate, CWnd* pParentWnd, EBatchPaymentPayType ePayType, long nBillID,
	long nOnlyShowChargeID /*= -1*/, long nBatchPaymentID /*= -1*/, long nInsuredPartyID /*= -1*/, BOOL bUseRevCodes /*= FALSE*/, long nRevCodeID /*= -1*/,
	COleCurrency cyDefaultPaymentAmount /*= g_ccyInvalid*/, bool bIsCapitation /*= false*/)
{
	try {
		// (r.gonet 2016-01-25 10:35) - PLID 67942 - Use the parent window to display message boxes so they are modal.
		SetInterface(pParentWnd);
		//auto-posting is enabled if we're manually creating the dialog
		m_bIsAutoPosting = TRUE;

		m_bUseRevCodes = bUseRevCodes;
		m_nRevCodeID = nRevCodeID;
		m_nBillID = nBillID;
		m_nBatchPaymentID = nBatchPaymentID;

		// (j.jones 2015-10-23 14:10) - PLID 67377 - added flag to indicate if this is a capitation payment
		m_bIsCapitation = bIsCapitation;

		// (j.jones 2015-10-20 08:53) - PLID 67377 - added default payment amount, only used
		// when posting to one charge
		if (cyDefaultPaymentAmount.GetStatus() != COleCurrency::invalid) {

			// (j.jones 2015-10-20 08:48) - PLID 67377 - A default payment amount cannot be used unless
			// we are posting to one charge only. It can be zero, but can't be negative.
			if (cyDefaultPaymentAmount < COleCurrency(0,0) 
				|| m_bUseRevCodes || nOnlyShowChargeID == -1) {

				//the caller should not have permitted this
				ASSERT(FALSE);
				cyDefaultPaymentAmount = g_ccyInvalid;
			}
			else {
				m_cyDefaultPaymentAmount = cyDefaultPaymentAmount;
			}
		}
		else if(m_bIsCapitation) {
			//capitation can never post without a default payment!
			ASSERT(FALSE);
			ThrowNxException("Line item posting called on a capitation payment with no default payment amount.");
		}

		// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType
		m_ePayType = ePayType;

		// (j.jones 2014-07-02 14:57) - PLID 62548 - posting by revenue code is not allowed on vision payments
		if (m_bUseRevCodes && m_ePayType == eVisionPayment) {
			//no code should have permitted this
			ASSERT(FALSE);
			m_bUseRevCodes = FALSE;
		}

		// (j.jones 2014-06-27 11:00) - PLID 62548 - if they don't have the vision payment license,
		// turn vision payments into medical payments
		if (m_ePayType == eVisionPayment && !g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {
			m_ePayType = eMedicalPayment;
		}

		if(nOnlyShowChargeID != -1) {
			//a charge
			m_nChargeID = nOnlyShowChargeID;

			if(m_nBillID == -1) {
				//if the bill ID was not provided, query it

				//there should be no code that fails to provide a bill ID
				ASSERT(FALSE);

				_RecordsetPtr rs = CreateParamRecordset("SELECT BillID FROM ChargesT WHERE ID = {INT}",m_nChargeID);
				if(!rs->eof) {
					m_nBillID = AdoFldLong(rs, "BillID",-1);
				}
				rs->Close();
			}
		}

		m_nInsuredPartyID = nInsuredPartyID;

		// (j.jones 2012-08-08 11:17) - PLID 47778 - m_bIsPrimaryIns is true if the current insured party
		// is the patient's primary insurance, either for medical or for vision
		m_bIsPrimaryIns = IsInsuredPartySentAsPrimary(m_nInsuredPartyID);

		return CNxDialog::Create(nIDTemplate, pParentWnd);

	}NxCatchAll("Error creating posting screen.");

	return FALSE;
}

// (r.gonet 2016-01-25 09:16) - PLID 67942 - Sets a window to use as the "interface" window, which all message boxes displayed
// by the line item posting dialog will use.
void CFinancialLineItemPostingDlg::SetInterface(CWnd *pWnd)
{
	if (pWnd == nullptr || !::IsWindow(pWnd->GetSafeHwnd())) {
		ThrowNxException("%s : pWnd is not a window.", __FUNCTION__);
	}

	m_pInterface = pWnd;
}

#define cEditable	RGB(0,0,0)
#define cReadOnly	RGB(127,127,127)
BOOL CFinancialLineItemPostingDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		CWaitCursor pWait;

		// (j.jones 2008-05-08 09:23) - PLID 29953 - added nxiconbuttons for modernization
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnPost.AutoSet(NXB_MODIFY);
		m_btnSwapInscos.AutoSet(NXB_MODIFY);
		m_btnConfigureColumns.AutoSet(NXB_MODIFY);
		// (j.jones 2012-07-27 10:22) - PLID 26877 - added ability to filter reason codes
		m_btnFilterReasonCodes.SetIcon(IDI_FILTER);

		// (j.jones 2010-06-11 11:56) - PLID 16704 - added note icon for the datalist
		m_hNotes = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16,16, 0);

		// (j.jones 2007-05-21 10:02) - PLID 23751 - bulk cache our configrt calls
		g_propManager.CachePropertiesInBulk("FinancialLineItemPostingDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'DisablePaymentDate' OR "
		"Name = 'LineItemPostingAutoAdjust' OR "
		"Name = 'GetChargeInfo' OR "
		"Name = 'DefEOBShiftAction' OR "
		"Name = 'DefEOBShiftPartialPay' OR "
		"Name = 'SwapInsuranceCos' OR "
		"Name = 'DefaultPaymentDate' OR "
		"Name = 'DefaultPayCat' OR "
		// (j.jones 2007-06-07 09:27) - PLID 23037 - added AutoBatchOnSwitch
		"Name = 'AutoBatchOnSwitch' OR "
		//(e.lally 2007-12-11) PLID 28325 - Added hidden preference for masking cc numbers
		"Name = 'MaskCreditCardNumbers' OR "
		//TES 5/28/2008 - PLID 26189 - Added preferences for default adjustment category and description.
		"Name = 'LineItemPosting_DefaultAdjCategory' OR "
		"Name = 'LineItemPosting_DefaultAdjDescription' OR "
		"Name = 'IncludeForInPayDescription' OR "	// (j.jones 2008-06-06 17:30) - PLID 24661
		"Name = 'LineItemPostingColumnList' OR " // (z.manning 2008-06-19 09:57) - PLID 26544
		"Name = 'LineItemPosting_WhenNewInsPayTooMuch' OR "	// (j.jones 2010-06-10 15:30) - PLID 38901
		"Name = 'BatchPayUseChargeProviderOnApplies' "	// (j.jones 2011-07-08 14:31) - PLID 18687
		// (j.jones 2011-07-08 16:58) - PLID 44497 - added DefaultPaymentsNoProvider and RequireProviderOnPayments
		"OR Name = 'DefaultPaymentsNoProvider' "
		"OR Name = 'RequireProviderOnPayments' "
		"OR Name = 'BatchPayUseChargeLocationOnApplies' "	// (j.jones 2011-07-14 16:42) - PLID 18686
		"OR Name = 'dontshow LineItemPostingFilterReasonCodes' "	// (j.jones 2012-07-27 10:28) - PLID 26877
		"OR Name = 'LineItemPostingAutoPayment' " // (j.jones 2012-07-30 11:07) - PLID 47778
		// (j.jones 2012-08-14 09:54) - PLID 52076 - added an adjustment sub-preference to only auto-adjust on primary
		"OR Name = 'LineItemPostingAutoAdjust_PrimaryOnly' "
		"OR Name = 'dontshow LineItemPostingEstPayments' "	// (j.jones 2012-08-21 10:01) - PLID 52029
		// (j.jones 2012-08-14 14:14) - PLID 50285 - added ability to add deductibles/coinsurance to billing notes
		"OR Name = 'LineItemPosting_DedBillingNote' "
		"OR Name = 'LineItemPosting_DedBillingNote_ShowOnStatement' "
		"OR Name = 'LineItemPosting_DedBillingNote_DefCategory' "
		"OR Name = 'LineItemPostingAutoPayment_Secondary' " // (j.jones 2012-08-28 17:07) - PLID 52335
		"OR Name = 'dontshow LineItemPostingEstPayments_Secondary' " // (j.jones 2012-08-28 17:47) - PLID 52335
		// (j.jones 2013-07-03 16:13) - PLID 57226 - added preference for allowing adjustments on $0.00 payments
		"OR Name = 'LineItemPostingAutoAdjust_AllowOnZeroDollarPayments' "
		// (j.jones 2013-07-19 15:38) - PLID 57653 - used in CheckUnbatchCrossoverClaim
		"OR Name = 'ERemit_UnbatchMA18orNA89_MarkForwardToSecondary' "
		"OR Name = 'dontshow LineItemPostingCopay' "	// (j.jones 2013-08-27 13:28) - PLID 57398
		"OR Name = 'LineItemPosting_DefaultChargebackCategory' "	// (j.jones 2014-06-30 16:35) - PLID 62642
		"OR Name = 'LineItemPosting_AutoAdjustVisionBalances' "	// (j.jones 2014-07-01 09:28) - PLID 62552
		// (j.jones 2015-03-20 11:57) - PLID 65400 - added a preference to always void and correct when the system auto-unapplies a payment
		"OR Name = 'UnapplyFromCharge_VoidAndCorrect' "
		// (j.jones 2016-05-12 12:14) - NX-100502 - added more dontshow warnings
		"OR Name = 'dontshow LineItemPosting_EditFinishOverpayment' "
		"OR Name = 'dontshow LineItemPosting_EditFinishTotalOverage' "
		"OR Name = 'dontshow LineItemPosting_EditFinishOveradjust' "
		"OR Name = 'dontshow LineItemPosting_PostPatientOverpayment' "
		"OR Name = 'dontshow LineItemPosting_PostInsOverpayment' "
		"OR Name = 'dontshow LineItemPosting_PostInsOveradjust' "
		"OR Name = 'dontshow LineItemPosting_PostTotalOverage' "
		")",
		_Q(GetCurrentUserName()));

		// (j.jones 2014-06-30 14:22) - PLID 62642 - limit chargeback description length,
		// also limited pay & adj. descriptions
		m_nxeditPayDesc.SetLimitText(255);
		m_nxeditAdjDesc.SetLimitText(255);
		m_nxeditChargebackDescription.SetLimitText(255);

		// (j.jones 2015-09-30 11:04) - PLID 67175 - limit the Last 4 CC number to 4
		m_nxeditCCLast4.SetLimitText(4);

		// (j.jones 2006-12-07 12:40) - PLID 19467 - added option to disable payment date changing
		long nDisablePaymentDate = GetRemotePropertyInt("DisablePaymentDate", 0, 0, "<None>", true);
		if(nDisablePaymentDate == 1) {
			//simply disable the ability to change the payment/adjustment date,
			//the code will fill it any number of ways, however
			GetDlgItem(IDC_PAY_DATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADJ_DATE)->EnableWindow(FALSE);
		}

		// (j.jones 2007-01-02 17:14) - PLID 24030 - check the default only if the caller told us to,
		// and there is no insured party
		// (j.jones 2014-07-02 14:57) - PLID 62548 - this is not allowed on vision payments
		if (m_bUseRevCodes && m_nInsuredPartyID != -1 && m_ePayType != eVisionPayment) {
			CheckDlgButton(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE, TRUE);
		}

		// (j.jones 2007-05-21 10:01) - PLID 23751 - supported Line Item Posting auto-adjust options
		m_nLineItemPostingAutoAdjust = GetRemotePropertyInt("LineItemPostingAutoAdjust",2,0,"<None>",true);

		// (j.jones 2012-08-14 09:54) - PLID 52076 - added a sub-preference to only auto-adjust on primary
		m_nLineItemPostingAutoAdjust_PrimaryOnly = GetRemotePropertyInt("LineItemPostingAutoAdjust_PrimaryOnly",1,0,"<None>",true);

		// (j.jones 2013-07-03 16:13) - PLID 57226 - added preference for allowing adjustments on $0.00 payments
		m_nLineItemPostingAutoAdjust_AllowOnZeroDollarPayments = GetRemotePropertyInt("LineItemPostingAutoAdjust_AllowOnZeroDollarPayments", 1, 0, "<None>", true);

		// (j.jones 2012-07-30 10:58) - PLID 47778 - added ability to auto-calculate payment amounts, for primary insurance only
		m_nLineItemPostingAutoPayment = GetRemotePropertyInt("LineItemPostingAutoPayment",0,0,"<None>",true);

		// (j.jones 2012-08-28 17:10) - PLID 52335 - added the same ability for secondary payments, only used when primary is enabled
		m_nLineItemPostingAutoPayment_Secondary = GetRemotePropertyInt("LineItemPostingAutoPayment_Secondary",0,0,"<None>",true);

		// (j.jones 2014-07-01 09:28) - PLID 62552 - vision payments have a checkbox for auto-adjusting
		if (m_ePayType == eVisionPayment) {
			m_checkAutoAdjustBalances.SetCheck(GetRemotePropertyInt("LineItemPosting_AutoAdjustVisionBalances", 1, 0, GetCurrentUserName(), true) == 1);
		}

		// (j.jones 2012-08-17 09:55) - PLID 52162 - if we're auto-posting, force the auto-payment & auto-adjustment to be on
		// (we would not change the PrimaryOnly setting though)
		if(m_bIsAutoPosting) {
			m_nLineItemPostingAutoPayment = 1;
			m_nLineItemPostingAutoAdjust = 1;
			// (j.jones 2012-08-28 17:10) - PLID 52335 - auto-payments for secondaries, if the caller set m_bIsAutoPosting to
			// on for a secondary posting then they would need this to be on as well
			m_nLineItemPostingAutoPayment_Secondary = 1;
		}

		// (j.jones 2010-06-02 11:55) - PLID 37200 - default the "unapply" labels to hidden, with the currency colored red
		m_nxstaticPatAmtToUnapplyLabel.ShowWindow(SW_HIDE);
		m_nxstaticPatAmtToUnapply.ShowWindow(SW_HIDE);
		m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_HIDE);
		m_nxstaticInsAmtToUnapply.ShowWindow(SW_HIDE);
		m_cyUnappliedPatAmt = COleCurrency(0,0);
		m_cyUnappliedInsAmt = COleCurrency(0,0);

		extern CPracticeApp theApp;
		CFont *bold = &theApp.m_boldFont;

		GetDlgItem(IDC_POSTING_PATIENT_NAME_LABEL)->SetFont(bold);

		m_PostingList = BindNxDataListCtrl(this,IDC_LINE_ITEM_POSTING_LIST,GetRemoteData(),false);
		m_ProviderCombo = BindNxDataListCtrl(this,IDC_COMBO_PROVIDER,GetRemoteData(),true);
		m_LocationCombo = BindNxDataListCtrl(this,IDC_COMBO_LOCATION,GetRemoteData(),true);
		m_PayRespCombo = BindNxDataListCtrl(this,IDC_PAY_INS_RESP_COMBO,GetRemoteData(),false);
		m_PayCatCombo = BindNxDataListCtrl(this,IDC_PAY_CATEGORY,GetRemoteData(),true);
		m_PayMethodCombo = BindNxDataListCtrl(this,IDC_PAY_TYPE_COMBO,GetRemoteData(),false);
		m_PayCardTypeCombo = BindNxDataListCtrl(this,IDC_COMBO_CARD_NAME,GetRemoteData(),true);
		m_AdjCatCombo = BindNxDataListCtrl(this,IDC_ADJ_CATEGORY,GetRemoteData(),true);
		m_BillInsuranceCombo = BindNxDataListCtrl(this,IDC_POSTING_INSURANCE_CO_LIST,GetRemoteData(),false);
		m_BillOtherInsuranceCombo = BindNxDataListCtrl(this,IDC_POSTING_OTHER_INSURANCE_CO_LIST,GetRemoteData(),false);		
		m_ShiftRespCombo = BindNxDataListCtrl(this,IDC_SHIFT_RESP_COMBO,GetRemoteData(),false);
		m_PayDescCombo = BindNxDataListCtrl(this,IDC_PAY_DESCRIPTION_COMBO,GetRemoteData(),true);
		m_AdjDescCombo = BindNxDataListCtrl(this,IDC_ADJ_DESCRIPTION_COMBO,GetRemoteData(),true);
		// (j.jones 2014-06-30 14:24) - PLID 62642 - added chargebacks
		m_ChargebackCategoryCombo = BindNxDataList2Ctrl(IDC_CB_CATEGORY_COMBO, true);
		m_ChargebackDescriptionCombo = BindNxDataList2Ctrl(IDC_CB_DESCRIPTION_COMBO, true);

		// (a.walling 2006-11-15 09:42) - PLID 23552 - Bind and init datalists for group and reason codes
		// (j.jones 2010-09-23 15:02) - PLID 40653 - these can now requery from data
		m_pGroupCodeList = BindNxDataList2Ctrl(this, IDC_ADJ_GROUPCODE, GetRemoteData(), true);
		m_pReasonList = BindNxDataList2Ctrl(this, IDC_ADJ_REASON, GetRemoteData(), true);

		// (j.jones 2010-09-23 15:03) - PLID 40653 - add "no code" rows
		NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_pGroupCodeList->GetNewRow();
		pRow2->PutValue(gcccID, (long)-1);
		pRow2->PutValue(gcccCode, "");
		pRow2->PutValue(gcccDescription, "<No Group Code>");
		m_pGroupCodeList->AddRowSorted(pRow2, NULL);
		m_pGroupCodeList->SetSelByColumn(gcccID, (long)-1);

		pRow2 = m_pReasonList->GetNewRow();
		pRow2->PutValue(rcccID, (long)-1);
		pRow2->PutValue(rcccCode, "");
		pRow2->PutValue(rcccDescription, "<No Reason Code>");
		m_pReasonList->AddRowSorted(pRow2, NULL);
		m_pReasonList->SetSelByColumn(rcccID, (long)-1);

		// (z.manning 2008-06-19 09:42) - PLID 26544 - Add the dynamic columns
		ReloadDynamicColumns();

		//if there is a GetChargeInfo field of 0 in ConfigRT, then GetChargeInfo will be false.
		COleVariant var = GetRemotePropertyInt("GetChargeInfo", -1);
		m_bGetChargeInfo = var.lVal;

		//determine the patient ID, and get the insured party IDs while we have the record open
		// (j.jones 2009-03-11 09:02) - PLID 32864 - get the insurance names as well, for auditing
		// (j.jones 2010-06-11 11:36) - PLID 16704 - added button for billing notes, icon chosen from data
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.PatientID, BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, "
			"InsuranceCoT.Name As InsCoName, OthrInsuranceCoT.Name AS OthrInsCoName, "
			"Convert(bit,(CASE WHEN BillsT.ID IN (SELECT BillID FROM Notes WHERE Notes.BillID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes "
			"FROM BillsT "
			"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN InsuredPartyT OthrInsuredPartyT ON BillsT.OthrInsuredPartyID = OthrInsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT OthrInsuranceCoT ON OthrInsuredPartyT.InsuranceCoID = OthrInsuranceCoT.PersonID "
			"WHERE BillsT.ID = {INT}", m_nBillID);
		if(!rs->eof) {
			m_nPatientID = AdoFldLong(rs, "PatientID",-1);
			// (j.jones 2009-03-11 09:02) - PLID 32864 - save this info. in memory, for auditing later
			m_nBillOldInsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
			m_nBillOldOtherInsuredPartyID = AdoFldLong(rs, "OthrInsuredPartyID",-1);
			m_strOldInsuranceCoName = AdoFldString(rs, "InsCoName", "<None Selected>");
			m_strOldOtherInsuranceCoName = AdoFldString(rs, "OthrInsCoName", "<None Selected>");

			// (j.jones 2010-06-11 11:36) - PLID 16704 - added button for billing notes, icon chosen from data
			if(!AdoFldBool(rs, "HasNotes")) {
				m_btnBillNotes.SetIcon(IDI_OTHER);
			}
			else {
				m_btnBillNotes.SetIcon(IDI_BILL_NOTES);
			}
		}
		else {
			m_btnBillNotes.SetIcon(IDI_OTHER);
		}
		rs->Close();

		IRowSettingsPtr pRow;
		pRow = m_ProviderCombo->GetRow(-1);
		pRow->PutValue(0,long(-1));
		pRow->PutValue(1,_bstr_t(" <No Provider Selected>"));
		m_ProviderCombo->InsertRow(pRow,0);

		pRow = m_PayCatCombo->GetRow(-1);
		pRow->PutValue(cccID, long(0));
		pRow->PutValue(cccCategory, _bstr_t(" <No Category Selected>"));
		m_PayCatCombo->AddRow(pRow);
		
		pRow = m_AdjCatCombo->GetRow(-1);
		pRow->PutValue(cccID, long(0));
		pRow->PutValue(cccCategory, _bstr_t(" <No Category Selected>"));
		m_AdjCatCombo->AddRow(pRow);

		// (j.jones 2014-06-30 15:29) - PLID 62642 - added chargebacks
		pRow2 = m_ChargebackCategoryCombo->GetNewRow();
		pRow2->PutValue(cccID, long(0));
		pRow2->PutValue(cccCategory, _bstr_t(" <No Category Selected>"));
		m_ChargebackCategoryCombo->AddRowSorted(pRow2, NULL);
		
		//TES 5/28/2008 - PLID 26189 - Pull their preferred defaults for adjustments.
		// (j.jones 2008-07-11 14:59) - PLID 28756 - even though an insurance-specific category and description
		// may override these later, continue to do this now
		m_AdjCatCombo->TrySetSelByColumn(cccID, GetRemotePropertyInt("LineItemPosting_DefaultAdjCategory", -1, 0, "<None>", true));
		//TES 8/21/2008 - PLID 26189 - Remember whether we set this.
		CString strDefaultAdjDesc = GetRemotePropertyText("LineItemPosting_DefaultAdjDescription", "", 0, "<None>", true);
		SetDlgItemText(IDC_ADJ_DESC, strDefaultAdjDesc);

		if (m_ePayType == eVisionPayment) {
			// (j.jones 2014-06-30 15:54) - PLID 62642 - chargeback descriptions always default to 'chargeback payment'
			SetDlgItemText(IDC_CB_DESC, "Chargeback Payment");

			// (j.jones 2014-06-30 15:56) - PLID 62642 - if the chargeback category does not exist, add it
			// (j.jones 2015-10-26 11:20) - PLID 67451 - moved this to a modular function
			long nChargebackCategoryID = AutoCreatePaymentCategory("Chargeback Payment");			

			//now remember the default category for this user
			long nDefaultChargebackCategoryID = GetRemotePropertyInt("LineItemPosting_DefaultChargebackCategory", nChargebackCategoryID, 0, GetCurrentUserName(), true);
			//select the default category if it exists - it may be the "no category" row, which is fine
			if (m_ChargebackCategoryCombo->SetSelByColumn(cccID, nDefaultChargebackCategoryID) == NULL) {
				//if the row does not exist, select the built-in chargeback category
				m_ChargebackCategoryCombo->SetSelByColumn(cccID, nChargebackCategoryID);
			}
		}

		pRow = m_PayMethodCombo->GetRow(-1);
		pRow->PutValue(0,long(1));
		pRow->PutValue(1,_bstr_t("Cash"));
		m_PayMethodCombo->AddRow(pRow);
		pRow = m_PayMethodCombo->GetRow(-1);
		pRow->PutValue(0,long(2));
		pRow->PutValue(1,_bstr_t("Check"));
		m_PayMethodCombo->AddRow(pRow);
		pRow = m_PayMethodCombo->GetRow(-1);
		pRow->PutValue(0,long(3));
		pRow->PutValue(1,_bstr_t("Charge"));
		m_PayMethodCombo->AddRow(pRow);

		CString str;
		str.Format("PatientID = %li AND RespTypeID <> -1",m_nPatientID);
		// (j.jones 2014-06-27 10:58) - PLID 62548 - if a vision payment, filter only on vision resp
		if (m_ePayType == eVisionPayment) {
			//filter on vision insurances only
			str += FormatString(" AND RespTypeT.CategoryType = %li", rctVision);
		}
		m_ShiftRespCombo->WhereClause = _bstr_t(str);
		m_ShiftRespCombo->Requery();
		m_ShiftRespCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		pRow = m_ShiftRespCombo->GetRow(-1);
		pRow->PutValue(srccID,long(-1));
		pRow->PutValue(srccName,_bstr_t("<Do Not Shift>"));
		pRow->PutValue(srccPriority, (long)-3);
		pRow->PutValue(srccCategoryTypeID, g_cvarNull);
		pRow->PutValue(srccCategoryName, _bstr_t(""));
		m_ShiftRespCombo->InsertRow(pRow,0);
		pRow = m_ShiftRespCombo->GetRow(-1);
		pRow->PutValue(srccID,long(0));
		pRow->PutValue(srccName,_bstr_t("Patient"));
		pRow->PutValue(srccPriority, (long)-2);
		pRow->PutValue(srccCategoryTypeID, g_cvarNull);
		pRow->PutValue(srccCategoryName, _bstr_t(""));
		m_ShiftRespCombo->InsertRow(pRow,1);
		pRow = m_ShiftRespCombo->GetRow(-1);

		// (j.jones 2010-09-02 17:46) - PLID 40395 - moved the selection of the shift resp. combo
		// into OnSelChosenPayInsRespCombo()

		CheckDlgButton(IDC_CHECK_SHIFT_PAID_AMOUNTS,GetRemotePropertyInt("DefEOBShiftPartialPay",0,0,"<None>",true));
		CheckDlgButton(IDC_CHECK_AUTO_SWAP, GetRemotePropertyInt("SwapInsuranceCos",1,0,"<None>",TRUE) == 1);
		// (j.jones 2007-05-22 11:49) - PLID 23037 - added auto-batch ability
		// (d.thompson 2012-08-06) - PLID 51969 - Changed default to On
		CheckDlgButton(IDC_CHECK_AUTO_BATCH, GetRemotePropertyInt("AutoBatchOnSwitch",1,0,"<None>",TRUE) == 1);
		// (j.jones 2010-05-17 16:54) - PLID 16503 - supported $0.00 applies
		m_checkApplyZeroDollarPays.SetCheck(GetRemotePropertyInt("LineItemPosting_PostZeroDollarPayments", 0, 0, GetCurrentUserName(), true) == 1);

		//format internationally
		CString strApplyZeroDollarPays;
		strApplyZeroDollarPays.Format("Post Payments Even When %s", FormatCurrencyForInterface(COleCurrency(0,0)));
		m_checkApplyZeroDollarPays.SetWindowText(strApplyZeroDollarPays);

		//Depending on our locale, we may want to put a space between the currency symbol and the number.
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, strICurr.GetBuffer(2), 2, TRUE);
		strICurr.ReleaseBuffer();
		strICurr.TrimRight();
		if(strICurr == "2") {
			SetDlgItemText(IDC_PAY_CURRENCY_SYMBOL,GetCurrencySymbol() + " ");
			SetDlgItemText(IDC_ADJ_CURRENCY_SYMBOL,GetCurrencySymbol() + " ");
			SetDlgItemText(IDC_CURRENCY_SYMBOL_RECEIVED_LIP,GetCurrencySymbol() + " ");
			SetDlgItemText(IDC_CURRENCY_SYMBOL_GIVEN_LIP,GetCurrencySymbol() + " ");
		}
		else {
			SetDlgItemText(IDC_PAY_CURRENCY_SYMBOL,GetCurrencySymbol());
			SetDlgItemText(IDC_ADJ_CURRENCY_SYMBOL,GetCurrencySymbol());
			SetDlgItemText(IDC_CURRENCY_SYMBOL_RECEIVED_LIP,GetCurrencySymbol());
			SetDlgItemText(IDC_CURRENCY_SYMBOL_GIVEN_LIP,GetCurrencySymbol());
		}		

		//now display the patient's name and user defined ID
		rs = CreateParamRecordset("SELECT UserDefinedID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PersonT.ID = {INT}",m_nPatientID);
		if(!rs->eof) {
			SetDlgItemText(IDC_POSTING_PATIENT_NAME_LABEL,AdoFldString(rs, "Name",""));
			CString str;
			str.Format("%li",AdoFldLong(rs, "UserDefinedID"));
			SetDlgItemText(IDC_POSTING_PATIENT_ID_LABEL,str);
		}
		rs->Close();

		//load the insurance responsibility list
		str.Format("PatientID = %li", m_nPatientID);
		m_PayRespCombo->WhereClause = _bstr_t(str);
		m_PayRespCombo->Requery();

		pRow = m_PayRespCombo->GetRow(-1);
		pRow->PutValue(rccID,(long)-1);
		pRow->PutValue(rccName,"Patient");
		pRow->PutValue(rccPriority,(long)0);
		pRow->PutValue(rccDefaultPayDesc, g_cvarNull);
		pRow->PutValue(rccDefaultPayCategoryID, g_cvarNull);
		pRow->PutValue(rccDefaultAdjDesc, g_cvarNull);
		pRow->PutValue(rccDefaultAdjCategoryID, g_cvarNull);
		m_PayRespCombo->AddRow(pRow);

		if (m_nInsuredPartyID != -1) {
			// (j.jones 2008-07-11 15:35) - PLID 28756 - continue calling OnSelChosen here,
			// because we DO want the descriptions and categories to change
			OnSelChosenPayInsRespCombo(m_PayRespCombo->SetSelByColumn(0,(long)m_nInsuredPartyID));
		}
		else {
			m_PayRespCombo->PutCurSel(0);
			OnSelChosenPayInsRespCombo(0);
		}

		//now load the bill information at the bottom of the screen
		str.Format("InsuredPartyT.PatientID = %li", m_nPatientID);
		m_BillInsuranceCombo->WhereClause = (LPCTSTR)str;
		m_BillOtherInsuranceCombo->WhereClause = (LPCTSTR)str;
		
		m_BillInsuranceCombo->Requery();
		m_BillOtherInsuranceCombo->Requery();

		m_BillInsuranceCombo->SetSelByColumn(ipccID, (long)m_nBillOldInsuredPartyID);
		m_BillOtherInsuranceCombo->SetSelByColumn(ipccID, (long)m_nBillOldOtherInsuredPartyID);

		pRow = m_BillInsuranceCombo->GetRow(-1);
		pRow->PutValue(ipccID, (long)-1);
		pRow->PutValue(ipccName, _bstr_t("<No Company Selected>"));
		pRow->PutValue(ipccRespTypeID, (long)-1);
		m_BillInsuranceCombo->InsertRow(pRow, 0);

		pRow = m_BillOtherInsuranceCombo->GetRow(-1);
		pRow->PutValue(ipccID, (long)-1);
		pRow->PutValue(ipccName, _bstr_t("<No Company Selected>"));
		pRow->PutValue(ipccRespTypeID, (long)-1);
		m_BillOtherInsuranceCombo->InsertRow(pRow, 0);

		int nBatch = FindHCFABatch(m_nBillID);

		if (nBatch == 1 || nBatch == 3) {
			((CButton*)GetDlgItem(IDC_RADIO_POST_PAPER_BATCH))->SetCheck(TRUE);
			((CButton*)GetDlgItem(IDC_RADIO_POST_ELECTRONIC_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_POST_UNBATCHED))->SetCheck(FALSE);
		}
		else if (nBatch == 2) {
			((CButton*)GetDlgItem(IDC_RADIO_POST_PAPER_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_POST_ELECTRONIC_BATCH))->SetCheck(TRUE);
			((CButton*)GetDlgItem(IDC_RADIO_POST_UNBATCHED))->SetCheck(FALSE);
		}
		else {
			((CButton*)GetDlgItem(IDC_RADIO_POST_PAPER_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_POST_ELECTRONIC_BATCH))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_POST_UNBATCHED))->SetCheck(TRUE);
		}

		SetDefaultData();

		if(m_nBatchPaymentID != -1) {
			//we're posting from a batch payment, so set the screen up accordingly
			SetBatchPaymentData();
		}
		else {
			//try to set the provider on the charge, or the bill if all the providers are the same
			//TES 2003-12-31: Also the location and description
			long nProviderID = -1;
			BOOL bDoNotLoadProvider = FALSE;
			long nLocationID = GetCurrentLocationID();
			CString strDescription = "";
			if(m_nChargeID != -1) {
				// (j.jones 2010-04-06 09:51) - PLID 37131 - disallow using an inactive provider,
				// or an inactive or unmanaged location
				rs = CreateParamRecordset("SELECT LineItemT.Description, ProvidersQ.ID AS DoctorsProviders, LocationsQ.ID AS LocationID "
					"FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS ProvidersQ ON ChargesT.DoctorsProviders = ProvidersQ.ID "
					"LEFT JOIN (SELECT ID FROM LocationsT WHERE Active = 1 AND Managed = 1 AND TypeID = 1) AS LocationsQ ON LineItemT.LocationID = LocationsQ.ID "
					"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = {INT}", m_nChargeID);
			}
			else {

				// (j.jones 2010-04-06 09:51) - PLID 37131 - disallow using an inactive provider,
				// or an inactive or unmanaged location
				rs = CreateParamRecordset("SELECT BillsT.Description, ProvidersQ.ID AS DoctorsProviders, LocationsQ.ID AS LocationID "
					"FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS ProvidersQ ON ChargesT.DoctorsProviders = ProvidersQ.ID "
					"LEFT JOIN (SELECT ID FROM LocationsT WHERE Active = 1 AND Managed = 1 AND TypeID = 1) AS LocationsQ ON LineItemT.LocationID = LocationsQ.ID "
					"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT}", m_nBillID);
			
				nProviderID = -2;
				BOOL bContinue = TRUE;

				while(!rs->eof) {
					long nProvID = AdoFldLong(rs, "DoctorsProviders",-1);
					if(nProviderID != -2 && nProvID != nProviderID) {
						//more than one provider on the bill, can't use this function
						bContinue = FALSE;
						nProviderID = -1;
					}
					else {
						nProviderID = nProvID;
					}
					rs->MoveNext();
				}

				rs->MoveFirst();

				bDoNotLoadProvider = !bContinue;
			}

			if(!rs->eof) {
				if(!bDoNotLoadProvider)
					nProviderID = AdoFldLong(rs, "DoctorsProviders",-1);
				nLocationID = AdoFldLong(rs, "LocationID", GetCurrentLocationID());
				strDescription = AdoFldString(rs, "Description", "");
			}

			// (j.jones 2011-07-08 16:58) - PLID 44497 - added preference to default payments to no provider
			if(GetRemotePropertyInt("DefaultPaymentsNoProvider", 0, 0, "<None>", true) == 1) {
				nProviderID = -1;
			}

			m_ProviderCombo->SetSelByColumn(0, nProviderID);
			m_LocationCombo->SetSelByColumn(0, nLocationID);

			if(strDescription != "") {

				// (j.jones 2008-06-06 17:30) - PLID 24661 - added preference to determine if non-copays
				// include the "For" prefix before the description
				CString str;
				if(GetRemotePropertyInt("IncludeForInPayDescription", 1, 0, "<None>", true) == 1) {
					str.Format("For %s", strDescription);
				}
				else {
					str = strDescription;
				}

				SetDlgItemText(IDC_PAY_DESC, str);
				//TES 8/21/2008 - PLID 26189 - Only set the adjustment description if we haven't already set a default, the
				// preference takes precedence over this.
				if(strDefaultAdjDesc.IsEmpty()) {
					SetDlgItemText(IDC_ADJ_DESC, str);
				}
			}
		}

		// (j.jones 2014-06-27 11:28) - PLID 62548 - For Vision payments, this hides many controls and columns.
		// For Medical payments, will only hide vision-specific controls.
		UpdateVisibleControls();
		
		LoadPostingList();

		// (j.jones 2008-07-11 14:58) - PLID 28756 - now try to set our default category and description
		TrySetDefaultInsuranceDescriptions();

		//TES 2003-12-31: Visually indicate the columns which are never editable.
		m_PostingList->GetColumn(plcChargeDate)->PutForeColor(cReadOnly);
		m_PostingList->GetColumn(plcItemCode)->PutForeColor(cReadOnly);
		m_PostingList->GetColumn(plcOldExistingPatPays)->PutForeColor(cReadOnly);
		m_PostingList->GetColumn(plcOldExistingInsPays)->PutForeColor(cReadOnly);

		// (j.jones 2010-06-03 12:17) - PLID 37200 - announce the new unapply & shift abilities
		DontShowMeAgain(this, "Line Item Posting has the ability to unapply previous patient and insurance payments and optionally reapply them to other charges.\n\n"
			"You can reduce the existing applies in the 'Prev Pat Pays' and 'Prev Ins Pays' columns, and reapply elsewhere by increasing the values in these columns for other charges. "
			"Credits that are not applied to other charges will remain unapplied on the patient's account.\n\n"
			"All applies from the new payment you are entering should be entered into the 'New Pays' column.",
			"LineItemPostingUnapplyFeature", "Practice - Line Item Posting");

	}NxCatchAll("Error initializing posting screen.");
	
	return m_bIsAutoPosting ? FALSE : TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFinancialLineItemPostingDlg::LoadPostingList() {

	try {

		// (j.jones 2012-08-21 09:23) - PLID 52029 - disable the est. payments button if not primary
		// (j.jones 2012-08-28 17:45) - PLID 52335 - secondary is now allowed, so instead just disable if
		// not an insurance posting
		m_btnEstPayments.EnableWindow(m_nInsuredPartyID != -1);

		// (j.jones 2010-06-03 08:57) - PLID 37200 - clear the unapplied totals, and hide the labels
		m_nxstaticPatAmtToUnapplyLabel.ShowWindow(SW_HIDE);
		m_nxstaticPatAmtToUnapply.ShowWindow(SW_HIDE);
		m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_HIDE);
		m_nxstaticInsAmtToUnapply.ShowWindow(SW_HIDE);
		m_cyUnappliedPatAmt = COleCurrency(0,0);
		m_cyUnappliedInsAmt = COleCurrency(0,0);
		
		m_PostingList->GetColumn(m_nChargesColIndex)->PutForeColor(cReadOnly);
		m_PostingList->GetColumn(m_nPatBalanceColIndex)->PutForeColor(cReadOnly);
		m_PostingList->GetColumn(m_nInsBalanceColIndex)->PutForeColor(cReadOnly);

		// (j.jones 2007-01-02 17:18) - PLID 24030 - if grouping by RevCode, handle the load differently
		// (j.jones 2014-07-02 14:57) - PLID 62548 - this is not allowed on vision payments
		if (IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && m_nInsuredPartyID != -1 && m_ePayType != eVisionPayment) {

			if(m_nRevCodeID != -1) {
				//load a row for one revenue code
				AddPostingListRevCodeRow(m_nRevCodeID);
			}
			else {
				//load all revenue codes and non-rev.coded charges for the bill
				// (j.jones 2008-04-14 15:22) - PLID 29638 - reworked some of the joins to be left joins,
				// so that we properly show a given insurance responsibility as $0.00 instead of no line at all
				// (j.jones 2011-09-15 15:29) - PLID 44891 - skip original & void charges
				_RecordsetPtr rs = CreateParamRecordset("SELECT "
					"Min(ChargesT.ID) AS ChargeID, "
					"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) AS RevCodeID "
					"FROM "
					"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
					"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
					"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "					
					"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
					"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
					"	INNER JOIN (SELECT * FROM InsuredPartyT WHERE PersonID = {INT}) AS InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID "
					"	) AS ServiceRevCodesT ON ChargeRespT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
					"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
					"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.ID = {INT} "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"GROUP BY BillsT.ID, ServiceT.RevCodeUse, "
					"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE ChargesT.ID END), "
					"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END)",
					m_nInsuredPartyID, m_nBillID);
				while(!rs->eof) {

					long nRevCodeID = AdoFldLong(rs, "RevCodeID",-1);
					if(nRevCodeID != -1) {
						//load a row for this revenue code
						AddPostingListRevCodeRow(nRevCodeID);
					}
					else {
						//if no rev. code, load the unique charge
						AddPostingListChargeRow(AdoFldLong(rs, "ChargeID"));
					}
					rs->MoveNext();
				}
				rs->Close();
			}
		}
		else {

			CheckDlgButton(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE, FALSE);

			//normal charge listing

			if(m_nChargeID != -1) {
				AddPostingListChargeRow(m_nChargeID);
			}
			else {
				//add all charges for the bill
				// (j.jones 2011-09-15 15:29) - PLID 44891 - skip original & void charges
				_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.ID FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
					"WHERE ChargesT.BillID = {INT} AND Deleted = 0 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null ",
					m_nBillID);
				while(!rs->eof) {
					AddPostingListChargeRow(AdoFldLong(rs, "ID"));
					rs->MoveNext();
				}
				rs->Close();
			}
		}

		CalculateTotals();

	}NxCatchAll("Error loading posting list.");
}

void CFinancialLineItemPostingDlg::AddPostingListChargeRow(long nChargeID) {

	try {

		//add a new row with all the charge information pre-filled in
		
		IRowSettingsPtr pRow = m_PostingList->GetRow(-1);

		_variant_t varNull;
		varNull.vt = VT_NULL;

		pRow->PutValue(plcChargeID, (long)nChargeID);
		pRow->PutValue(plcRevCodeID, varNull);

		long nServiceID = -1;
		COleCurrency cyAllowedAmount = COleCurrency(0,0);

		// (j.jones 2010-06-11 12:02) - PLID 16704 - load whether this charge has notes
		// (j.jones 2011-09-15 15:29) - PLID 44891 - skip original & void charges
		// (b.spivey, January 03, 2012) - PLID 47121 - Get Deductible and Coinsurance from data, find out if there is even a row.
		// (b.spivey, January 11, 2012) - PLID 47121 - Decided against even loading the value. 
		// (j.jones 2012-07-30 14:18) - PLID 47778 - loaded the coinsurance and copay info. to cache it per charge,
		// the coinsurance % is NOT the same as the coinsurance $ that they will later enter as a dollar amount
		// (j.jones 2012-07-31 10:16) - PLID 51863 - moved the allowable calculation into this recordset
		// (d.lange 2015-11-30 14:34) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
		_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.ServiceID, LineItemT.Date, ChargesT.ItemCode, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
			"Convert(bit,(CASE WHEN ChargesT.ID IN (SELECT LineItemID FROM Notes WHERE Notes.LineItemID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes, " 
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CoInsurance ELSE DefaultPayGroupInfoQ.CoInsurance END AS DefaultCoInsurancePercent, "
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CopayMoney ELSE DefaultPayGroupInfoQ.CopayMoney END AS DefaultCopayMoney, "
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CopayPercentage ELSE DefaultPayGroupInfoQ.CopayPercentage END AS DefaultCopayPercent, "
			"Round(Convert(money, "
			"	dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, COALESCE(AllowableInsuredQ.InsuranceCoID, -1)) "
			"	* ChargesT.Quantity * "
			"	(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
			"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"	(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
			"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"	), 2) AS AllowableQty "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
			"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
			"	WHERE InsuranceCoID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
			") AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
			"LEFT JOIN ServicePayGroupsT InsServicePayGroupsT ON InsPayGroupLinkQ.PayGroupID = InsServicePayGroupsT.ID "
			"LEFT JOIN (SELECT PayGroupID, CopayMoney, CopayPercentage, CoInsurance, DeductibleRemaining, OOPRemaining "
			"	FROM InsuredPartyPayGroupsT "
			"	WHERE InsuredPartyID = {INT} "
			") AS DefaultPayGroupInfoQ ON ServicePayGroupsT.ID = DefaultPayGroupInfoQ.PayGroupID "
			"LEFT JOIN (SELECT PayGroupID, CopayMoney, CopayPercentage, CoInsurance, DeductibleRemaining, OOPRemaining "
			"	FROM InsuredPartyPayGroupsT "
			"	WHERE InsuredPartyID = {INT} "
			") AS InsurancePayGroupInfoQ ON InsServicePayGroupsT.ID = InsurancePayGroupInfoQ.PayGroupID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"LEFT JOIN ( "
			"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
			"	FROM InsuredPartyT "
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
			"WHERE ChargesT.ID = {INT} "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null",
			m_nInsuredPartyID, m_nInsuredPartyID, m_nInsuredPartyID,
			nChargeID);
		if(!rs->eof) {
			nServiceID = AdoFldLong(rs, "ServiceID",-1);
			pRow->PutValue(plcServiceID, (long)nServiceID);
			// (j.jones 2012-07-30 13:43) - PLID 47778 - Each charge will load the insured party's pay group copay $, copay %,
			// and/or coinsurance % (if any). This coinsurance is not the same field as the user-editable CoInsurance $ amount
			// that later gets saved to data.
			pRow->PutValue(plcCoinsurancePercent, rs->Fields->Item["DefaultCoInsurancePercent"]->Value);
			pRow->PutValue(plcCopayMoney, rs->Fields->Item["DefaultCopayMoney"]->Value);
			pRow->PutValue(plcCopayPercent, rs->Fields->Item["DefaultCopayPercent"]->Value);
			pRow->PutValue(plcChargeDate, rs->Fields->Item["Date"]->Value);
			pRow->PutValue(plcItemCode, _bstr_t(AdoFldString(rs, "ItemCode","")));
			_variant_t varNoteIcon;
			if(!AdoFldBool(rs, "HasNotes")) {
				//load from the datalist's icon for no notes
				varNoteIcon = (LPCTSTR)"BITMAP:FILE";
			}
			else {
				//load our icon for having notes
				varNoteIcon = (long)m_hNotes;
			}
			pRow->PutValue(plcBillNote, varNoteIcon);
			pRow->PutValue(m_nChargesColIndex, rs->Fields->Item["Amount"]->Value);

			if(m_nInsuredPartyID != -1) {
				// (j.jones 2012-07-31 10:16) - PLID 51863 - moved the allowable calculation into the main recordset
				cyAllowedAmount = AdoFldCurrency(rs, "AllowableQty",COleCurrency(0,0));
			}

			// (b.spivey, January 03, 2012) - PLID 47121 - added Deductible, coinsurance, and hasrow flag. 
			// (b.spivey, January 16, 2012) - PLID 47121 -  Just load null.
			pRow->PutValue(m_nDeductibleAmtColIndex, g_cvarNull);
			pRow->PutValue(m_nCoinsuranceAmtColIndex, g_cvarNull);
			// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
			pRow->PutValue(m_nCopayAmtColIndex, g_cvarNull);
		}
		else {
			return;
		}
		rs->Close();

		COleCurrency cyPatResp = GetChargePatientResp(nChargeID);
		COleCurrency cyPatBal = GetChargePatientBalance(nChargeID, m_nPatientID);
		COleCurrency cyPatPays = cyPatResp - cyPatBal;

		pRow->PutValue(m_nPatRespColIndex,_variant_t(cyPatResp));

		COleCurrency cyInsResp = GetChargeInsResp(nChargeID, m_nInsuredPartyID);
		COleCurrency cyInsBal = GetChargeInsBalance(nChargeID, m_nPatientID, m_nInsuredPartyID);
		COleCurrency cyExistingInsPays = cyInsResp - cyInsBal;
		pRow->PutValue(m_nInsRespColIndex,_variant_t(cyInsResp));
		
		// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new
		// patient payments, plus we cache the initial existing total in an "old" column
		pRow->PutValue(plcOldExistingPatPays,_variant_t(cyPatPays));
		pRow->PutValue(m_nExistingPatPaysColIndex,_variant_t(cyPatPays));

		// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new
		// insurance payments, plus we cache the initial existing total in an "old" column
		pRow->PutValue(plcOldExistingInsPays,_variant_t(cyExistingInsPays));
		pRow->PutValue(m_nExistingInsPaysColIndex,_variant_t(cyExistingInsPays));

		// (j.jones 2012-07-30 16:41) - PLID 47778 - added a cache for "edited payment" and "edited adjustment",
		// so we know whether the user has manually edited the payment or adjustment on this datalist row, at
		// which point we will no longer attempt to auto-update those fields
		pRow->PutValue(plcHasEditedPayment, g_cvarFalse);
		pRow->PutValue(plcHasEditedAdjustment, g_cvarFalse);

		pRow->PutValue(m_nAllowableColIndex,_variant_t(cyAllowedAmount));

		// (j.jones 2012-07-30 10:58) - PLID 47778 - added ability to auto-calculate payment amounts, for primary insurance only
		// (j.jones 2012-08-28 17:10) - PLID 52335 - added ability for secondary as well, as a sub-preference
		COleCurrency cyNewPays = COleCurrency(0,0);

		// (j.jones 2015-10-20 09:06) - PLID 67377 - if a default payment was provided, use it now
		if (m_cyDefaultPaymentAmount.GetStatus() != COleCurrency::invalid) {
			cyNewPays = m_cyDefaultPaymentAmount;
		}
		else if(m_nLineItemPostingAutoPayment != 0 && m_nInsuredPartyID != -1
			&& (m_bIsPrimaryIns || m_nLineItemPostingAutoPayment_Secondary != 0)) {

			//don't calculate payments on zero dollar charges
			COleCurrency cyCharges = VarCurrency(pRow->GetValue(m_nChargesColIndex), COleCurrency(0, 0));
			if (cyCharges > COleCurrency(0, 0)) {
				cyNewPays = CalculatePayment(TRUE, VarString(pRow->GetValue(plcItemCode), ""), cyCharges, cyInsBal,
					cyAllowedAmount, pRow->GetValue(plcCoinsurancePercent),
					pRow->GetValue(plcCopayMoney), pRow->GetValue(plcCopayPercent),
					pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
			}
		}

		// (j.jones 2010-05-05 09:48) - PLID 25521 - new payments is just one column now
		// (j.jones 2012-08-14 10:15) - PLID 52117 - if the value is zero, we default to blank instead of $0.00
		// (j.jones 2015-11-30 12:52) - PLID 67377 - a value of zero is ok for capitation payments
		pRow->PutValue(m_nNewPaysColIndex, cyNewPays == COleCurrency(0,0) && !m_bIsCapitation ? g_cvarNull : _variant_t(cyNewPays));

		//JMJ 4/14/2004 - if the allowable is greater than zero, auto-fill-in the adjustment
		// (j.jones 2007-05-21 10:06) - PLID 23751 - use the cached preference, if m_nLineItemPostingAutoAdjust = 1
		// then we will adjust now, otherwise do not adjust
		// (j.jones 2012-07-30 11:39) - PLID 47778 - if we auto-calculated a payment, then we should respect option 2
		// which calculates after a payment was entered
		// (j.jones 2013-07-03 16:13) - PLID 57226 - We have a preference for allowing adjustments on $0.00 payments,
		// but it should only be used if the payment is not typed in as zero, which would be the case here, so the 
		// preference is ignored. However, if the new pays are null, we now must pass in null.		
		COleCurrency cyNewAdj = COleCurrency(0,0);

		// (j.jones 2015-10-23 14:10) - PLID 67377 - Added a capitation-specific version of CalculateAdjustment
		// which simply adjusts off the balance of the insurance resp., ignoring the allowable.
		// This requires a default payment amount.
		if (m_bIsCapitation && m_cyDefaultPaymentAmount.GetStatus() != COleCurrency::invalid) {
			//just adjust the balance
			cyNewAdj = CalculateAdjustment_Capitation(cyInsResp, cyExistingInsPays + cyNewPays);
		}
		// (j.jones 2014-07-01 09:28) - PLID 62552 - vision payments have a checkbox for auto-adjusting, which overrides
		// all the other preferences
		else if (m_ePayType == eVisionPayment) {
			//this should not be capitation
			ASSERT(!m_bIsCapitation);

			if (m_checkAutoAdjustBalances.GetCheck()) {
				//this calculates for chargebacks too, it will only not calculate if
				//the payment column is null
				cyNewAdj = CalculateAdjustment_Vision(cyInsResp, cyPatResp,
					cyNewPays == COleCurrency(0, 0) ? g_cvarNull : _variant_t(cyExistingInsPays + cyNewPays));
			}
		}
		//normal medical adjustment logic
		else if(m_nLineItemPostingAutoAdjust == 1 || (m_nLineItemPostingAutoAdjust == 2 && cyNewPays > COleCurrency(0,0))) {
			// (j.jones 2012-08-14 09:54) - PLID 52076 - added sub-preference to only auto-adjust on primary
			if(m_nLineItemPostingAutoAdjust_PrimaryOnly != 1 || m_bIsPrimaryIns) {
				// (j.jones 2007-08-29 09:31) - PLID 27176 - moved adjustment formula into its own function
				cyNewAdj = CalculateAdjustment_Medical(cyInsResp, cyPatResp,
					cyNewPays == COleCurrency(0,0) ? g_cvarNull : _variant_t(cyExistingInsPays + cyNewPays),
					cyAllowedAmount);
			}
		}

		pRow->PutValue(m_nAdjustmentColIndex,_variant_t(cyNewAdj));

		// (j.jones 2014-07-01 15:42) - PLID 62553 - chargebacks do not affect the balance
		if (cyNewPays < COleCurrency(0, 0)) {
			cyInsBal -= cyNewAdj;
		}
		else {
			cyInsBal -= (cyNewPays + cyNewAdj);
		}
		
		pRow->PutValue(m_nPatBalanceColIndex,_variant_t(cyPatBal));

		pRow->PutValue(m_nInsBalanceColIndex,_variant_t(cyInsBal));

		m_PostingList->AddRow(pRow);

		//if we added an adjustment amount, we want to correctly update the adjustment total
		AutoFillAdjustmentTotal();

		// (j.jones 2012-08-17 09:10) - PLID 47778 - if not a batch payment, fill the payment
		// total with anything we may have auto-entered
		if(m_nBatchPaymentID == -1) {
			AutoFillPaymentTotal();
		}

	}NxCatchAll("Error adding row to the posting list.");
}

// (j.jones 2007-01-02 15:53) - PLID 24030 - added revenue code grouping
// (j.jones 2012-07-31 10:28) - PLID 51863 - removed nBillID, it's always m_nBillID
void CFinancialLineItemPostingDlg::AddPostingListRevCodeRow(long nRevenueCodeID) {

	try {

		//add a new row with all the charge information pre-filled in
		
		IRowSettingsPtr pRow = m_PostingList->GetRow(-1);

		_variant_t varNull;
		varNull.vt = VT_NULL;

		pRow->PutValue(plcChargeID, varNull);
		pRow->PutValue(plcRevCodeID, (long)nRevenueCodeID);

		// (j.jones 2008-04-14 15:22) - PLID 29638 - reworked some of the joins to be left joins,
		// so that we properly show a given insurance responsibility as $0.00 instead of no line at all
		// (j.jones 2010-06-11 12:02) - PLID 16704 - load whether any of the charges or the bill has notes
		// (j.jones 2011-09-15 15:29) - PLID 44891 - skip original & void charges
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) AS RevCodeID, "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE ItemCode END) AS ItemCode, "
			"Min(LineItemT.Date) AS Date, "
			"Convert(bit,(CASE WHEN BillsT.ID IN (SELECT BillID FROM Notes WHERE Notes.BillID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes "
			"FROM "
			"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "			
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "			
			"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
			"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
			"	INNER JOIN (SELECT * FROM InsuredPartyT WHERE PersonID = {INT}) AS InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID "
			"	) AS ServiceRevCodesT ON ChargeRespT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.ID = {INT} AND "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"GROUP BY BillsT.ID, ServiceT.RevCodeUse, "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE ChargesT.ID END), "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END), "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE ItemCode END)",
			m_nInsuredPartyID, m_nBillID, nRevenueCodeID);
		if(!rs->eof) {
			pRow->PutValue(plcServiceID, varNull);
			// (j.jones 2012-07-30 14:13) - PLID 51863 - the copay/coinsurance fields are always null on revenue code lines
			// This coinsurance is is not the same field as the user-editable CoInsurance $ amount that later gets saved to data.
			pRow->PutValue(plcCoinsurancePercent, varNull);
			pRow->PutValue(plcCopayMoney, varNull);
			pRow->PutValue(plcCopayPercent, varNull);
			pRow->PutValue(plcChargeDate, rs->Fields->Item["Date"]->Value);
			pRow->PutValue(plcItemCode, _bstr_t(AdoFldString(rs, "ItemCode","")));
			_variant_t varNoteIcon;
			if(!AdoFldBool(rs, "HasNotes")) {
				//load from the datalist's icon for no notes
				varNoteIcon = (LPCTSTR)"BITMAP:FILE";
			}
			else {
				//load our icon for having notes
				varNoteIcon = (long)m_hNotes;
			}
			pRow->PutValue(plcBillNote, varNoteIcon);
			// (j.jones 2008-04-21 17:22) - PLID 29737 - moved the ChargeAmount calculation into the loop below
			//pRow->PutValue(m_nChargesColIndex, rs->Fields->Item["ChargeAmount"]->Value);
		}
		else {
			return;
		}
		rs->Close();

		// (j.jones 2007-01-03 09:37) - PLID 24030 - for revenue codes, just calculate the charge amounts
		// for each total, and summarize

		COleCurrency cyTotalChargeAmt = COleCurrency(0,0);
		COleCurrency cyTotalPatResp = COleCurrency(0,0);
		COleCurrency cyTotalPatBal = COleCurrency(0,0);
		COleCurrency cyTotalPatPays = COleCurrency(0,0);
		COleCurrency cyTotalInsResp = COleCurrency(0,0);
		COleCurrency cyTotalInsBal = COleCurrency(0,0);
		COleCurrency cyTotalExistingInsPays = COleCurrency(0,0);
		COleCurrency cyTotalAllowedAmount = COleCurrency(0,0);

		// (j.jones 2008-04-14 15:22) - PLID 29638 - reworked some of the joins to be left joins,
		// so that we properly show a given insurance responsibility as $0.00 instead of no line at all
		// (j.jones 2008-04-21 16:08) - PLID 29737 - needs to group by ChargeID, we only want one
		// returned record per charge
		// (j.jones 2012-07-31 10:16) - PLID 51863 - moved the allowable calculation into this recordset
		// (d.lange 2015-11-30 14:38) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
		// (j.jones 2016-03-04 09:52) - PLID 68525 - fixed group by for AllowableInsuredQ.InsuranceCoID
		_RecordsetPtr rsCharges = CreateParamRecordset("SELECT "
			"ChargesT.ID AS ChargeID, dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmt, "
			"Round(Convert(money, "
			"	dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, COALESCE(AllowableInsuredQ.InsuranceCoID, -1)) "
			"	* ChargesT.Quantity * "
			"	(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
			"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"	(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
			"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"	), 2) AS AllowableQty "
			"FROM "
			"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
			"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
			"	INNER JOIN (SELECT * FROM InsuredPartyT WHERE PersonID = {INT}) AS InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ChargeRespT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
			"LEFT JOIN ( "
			"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
			"	FROM InsuredPartyT "
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.ID = {INT} AND "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
			"GROUP BY ChargesT.ID, ChargesT.Quantity, AllowableInsuredQ.InsuranceCoID, ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4",
			m_nInsuredPartyID, m_nBillID, nRevenueCodeID);

		while(!rsCharges->eof) {

			long nChargeID = AdoFldLong(rsCharges, "ChargeID",-1);

			// (j.jones 2008-04-21 17:23) - PLID 29737 - added charge amount calculation here
			COleCurrency cyChargeAmount = AdoFldCurrency(rsCharges, "ChargeAmt", COleCurrency(0,0));
			cyTotalChargeAmt += cyChargeAmount;

			COleCurrency cyPatResp = GetChargePatientResp(nChargeID);
			cyTotalPatResp += cyPatResp;

			COleCurrency cyPatBal = GetChargePatientBalance(nChargeID, m_nPatientID);
			cyTotalPatBal += cyPatBal;

			COleCurrency cyPatPays = (cyPatResp - cyPatBal);
			cyTotalPatPays += cyPatPays;

			COleCurrency cyInsResp = GetChargeInsResp(nChargeID, m_nInsuredPartyID);
			cyTotalInsResp += cyInsResp;

			COleCurrency cyInsBal = GetChargeInsBalance(nChargeID, m_nPatientID, m_nInsuredPartyID);
			cyTotalInsBal += cyInsBal;

			COleCurrency cyExistingInsPays = (cyInsResp - cyInsBal);
			cyTotalExistingInsPays += cyExistingInsPays;
			
			//calculate allowable based on Insurance ID and nServiceID			

			if(m_nInsuredPartyID != -1) {
				// (j.jones 2012-07-31 10:16) - PLID 51863 - moved the allowable calculation into the main recordset
				COleCurrency cyAllowedAmount = AdoFldCurrency(rsCharges, "AllowableQty",COleCurrency(0,0));				
				cyTotalAllowedAmount += cyAllowedAmount;
			}

			rsCharges->MoveNext();
		}
		rsCharges->Close();

		pRow->PutValue(m_nChargesColIndex, _variant_t(cyTotalChargeAmt));
		pRow->PutValue(m_nPatRespColIndex,_variant_t(cyTotalPatResp));
		pRow->PutValue(m_nInsRespColIndex,_variant_t(cyTotalInsResp));			
		
		// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new
		// patient payments, plus we cache the initial existing total in an "old" column
		pRow->PutValue(plcOldExistingPatPays,_variant_t(cyTotalPatPays));
		pRow->PutValue(m_nExistingPatPaysColIndex,_variant_t(cyTotalPatPays));

		// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new
		// patient payments, plus we cache the initial existing total in an "old" column
		pRow->PutValue(plcOldExistingInsPays,_variant_t(cyTotalExistingInsPays));
		pRow->PutValue(m_nExistingInsPaysColIndex,_variant_t(cyTotalExistingInsPays));

		// (j.jones 2012-07-30 16:45) - PLID 51863 - added a cache for "edited payment" and "edited adjustment",
		// so we know whether the user has manually edited the payment or adjustment on this datalist row, at
		// which point we will no longer attempt to auto-update those fields
		pRow->PutValue(plcHasEditedPayment, g_cvarFalse);
		pRow->PutValue(plcHasEditedAdjustment, g_cvarFalse);

		pRow->PutValue(m_nAllowableColIndex,_variant_t(cyTotalAllowedAmount));

		// (j.jones 2012-07-30 11:36) - PLID 51863 - added ability to auto-calculate payment amounts by revenue code, for primary insurance only
		// (j.jones 2012-08-28 17:10) - PLID 52335 - added ability for secondary as well, as a sub-preference
		COleCurrency cyNewPays = COleCurrency(0,0);
		if(m_nLineItemPostingAutoPayment != 0 && m_nInsuredPartyID != -1
			&& (m_bIsPrimaryIns || m_nLineItemPostingAutoPayment_Secondary != 0)) {
			//don't calculate payments on zero dollar charges
			if(cyTotalChargeAmt > COleCurrency(0,0)) {
				cyNewPays = CalculatePaymentForRevenueCode(TRUE, VarString(pRow->GetValue(plcItemCode), ""), nRevenueCodeID, cyTotalChargeAmt, cyTotalInsBal,
					cyTotalAllowedAmount, g_cvarNull, g_cvarNull, g_cvarNull);
			}
		}

		// (j.jones 2010-05-05 09:48) - PLID 25521 - new payments is just one column now
		// (j.jones 2012-08-14 10:15) - PLID 52117 - if the value is zero, we default to blank instead of $0.00
		pRow->PutValue(m_nNewPaysColIndex, cyNewPays == COleCurrency(0,0) ? g_cvarNull : _variant_t(cyNewPays));

		//JMJ 4/14/2004 - if the allowable is greater than zero, auto-fill-in the adjustment
		// (j.jones 2007-05-21 10:06) - PLID 23751 - use the cached preference, if m_nLineItemPostingAutoAdjust = 1
		// then we will adjust now, otherwise do not adjust
		// (j.jones 2012-07-30 11:43) - PLID 51863 - if we auto-calculated a payment, then we should respect option 2
		// which calculates after a payment was entered
		// (j.jones 2013-07-03 16:13) - PLID 57226 - We have a preference for allowing adjustments on $0.00 payments,
		// but it should only be used if the payment is not typed in as zero, which would be the case here, so the 
		// preference is ignored. However, if the new pays are null, we now must pass in null.
		COleCurrency cyNewAdj = COleCurrency(0,0);

		// (j.jones 2014-07-01 09:28) - PLID 62552 - vision payments have a checkbox for auto-adjusting, which overrides
		// all the other preferences
		if (m_ePayType == eVisionPayment) {
			if (m_checkAutoAdjustBalances.GetCheck()) {
				//this calculates for chargebacks too, it will only not calculate if
				//the payment column is null
				cyNewAdj = CalculateAdjustment_Vision(cyTotalInsResp, cyTotalPatResp,
					cyNewPays == COleCurrency(0, 0) ? g_cvarNull : _variant_t(cyTotalExistingInsPays + cyNewPays));
			}
		}
		else if(m_nLineItemPostingAutoAdjust == 1 || (m_nLineItemPostingAutoAdjust == 2 && cyNewPays > COleCurrency(0,0))) {
			// (j.jones 2012-08-14 09:54) - PLID 52076 - added sub-preference to only auto-adjust on primary
			if(m_nLineItemPostingAutoAdjust_PrimaryOnly != 1 || m_bIsPrimaryIns) {
				// (j.jones 2007-08-29 09:31) - PLID 27176 - moved adjustment formula into its own function
				cyNewAdj = CalculateAdjustment_Medical(cyTotalInsResp, cyTotalPatResp,
					cyNewPays == COleCurrency(0,0) ? g_cvarNull : _variant_t(cyTotalExistingInsPays + cyNewPays),
					cyTotalAllowedAmount);
			}
		}

		pRow->PutValue(m_nAdjustmentColIndex,_variant_t(cyNewAdj));

		// (j.jones 2014-07-01 15:42) - PLID 62553 - chargebacks do not affect the balance
		if (cyNewPays < COleCurrency(0, 0)) {
			cyTotalInsBal -= cyNewAdj;
		}
		else {
			cyTotalInsBal -= (cyNewPays + cyNewAdj);
		}

		pRow->PutValue(m_nPatBalanceColIndex,_variant_t(cyTotalPatBal));
		pRow->PutValue(m_nInsBalanceColIndex,_variant_t(cyTotalInsBal));

		// (b.spivey, January 06, 2012) - PLID 47121 - Set deductible/coinsurance/coinshasrow 
		//		to zero/false. We don't save this currently.
		// (j.jones 2012-07-31 09:59) - PLID 51863 - should be null, not zero
		pRow->PutValue(m_nDeductibleAmtColIndex, g_cvarNull); 
		pRow->PutValue(m_nCoinsuranceAmtColIndex, g_cvarNull); 
		// (b.spivey, January 11, 2012) - PLID 47121 - Removed CoinsHasRowCol
		// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
		pRow->PutValue(m_nCopayAmtColIndex, g_cvarNull);

		m_PostingList->AddRow(pRow);

		//if we added an adjustment amount, we want to correctly update the adjustment total
		AutoFillAdjustmentTotal();

		// (j.jones 2012-08-17 09:10) - PLID 51863 - if not a batch payment, fill the payment
		// total with anything we may have auto-entered
		if(m_nBatchPaymentID == -1) {
			AutoFillPaymentTotal();
		}

	}NxCatchAll("Error adding revenue code row to the posting list.");
}

void CFinancialLineItemPostingDlg::OnOK() 
{
	if(!PostPayments())
		return;
	
	CDialog::OnOK();
}

void CFinancialLineItemPostingDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CFinancialLineItemPostingDlg, CNxDialog)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_POSTING_INSURANCE_CO_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPostingInsuranceCoList, VTS_I2)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_POSTING_OTHER_INSURANCE_CO_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPostingOtherInsuranceCoList, VTS_I2)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_PAY_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenPayTypeCombo, VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_COMBO_CARD_NAME, 16 /* SelChosen */, OnSelChosenComboCardName, VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_LINE_ITEM_POSTING_LIST, 9 /* EditingFinishing */, OnEditingFinishingLineItemPostingList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_LINE_ITEM_POSTING_LIST, 10 /* EditingFinished */, OnEditingFinishedLineItemPostingList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_PAY_INS_RESP_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPayInsRespCombo, VTS_I2)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_PAY_INS_RESP_COMBO, 16 /* SelChosen */, OnSelChosenPayInsRespCombo, VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_POSTING_INSURANCE_CO_LIST, 16 /* SelChosen */, OnSelChosenPostingInsuranceCoList, VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_POSTING_OTHER_INSURANCE_CO_LIST, 16 /* SelChosen */, OnSelChosenPostingOtherInsuranceCoList, VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_PAY_DESCRIPTION_COMBO, 16 /* SelChosen */, OnSelChosenPayDescriptionCombo, VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_ADJ_DESCRIPTION_COMBO, 16 /* SelChosen */, OnSelChosenAdjDescriptionCombo, VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_ADJ_REASON, 1 /* SelChanging */, OnSelChangingReason, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_ADJ_GROUPCODE, 1 /* SelChanging */, OnSelChangingGroupCode, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_LINE_ITEM_POSTING_LIST, 25 /* FocusGained */, OnFocusGainedLineItemPostingList, VTS_NONE)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_LINE_ITEM_POSTING_LIST, 8 /* EditingStarting */, OnEditingStartingLineItemPostingList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_LINE_ITEM_POSTING_LIST, 19 /* LeftClick */, OnLeftClickLineItemPostingList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_LINE_ITEM_POSTING_LIST, 6, OnRButtonDownLineItemPostingList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CFinancialLineItemPostingDlg, IDC_CB_DESCRIPTION_COMBO, 16, OnSelChosenCbDescriptionCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CFinancialLineItemPostingDlg::OnRequeryFinishedPostingInsuranceCoList(short nFlags) 
{
	try {
		IRowSettingsPtr pRow;
		int nType;

		for(int i = 0; i < m_BillInsuranceCombo->GetRowCount(); i++){
			nType = VarLong(m_BillInsuranceCombo->GetValue(i, ipccRespTypeID), -1); 
			if( nType == 1){
				pRow = m_BillInsuranceCombo->GetRow(i);
				pRow->PutForeColor(RGB(192,0,0));
			}
			else if (nType == 2){
				pRow = m_BillInsuranceCombo->GetRow(i);
				pRow->PutForeColor(RGB(0,0,128));
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnRequeryFinishedPostingOtherInsuranceCoList(short nFlags) 
{
	try {
		IRowSettingsPtr pRow;
		int nType;

		for(int i = 0; i < m_BillOtherInsuranceCombo->GetRowCount(); i++){
			nType = VarLong(m_BillOtherInsuranceCombo->GetValue(i, ipccRespTypeID), -1); 
			if( nType == 1){
				pRow = m_BillOtherInsuranceCombo->GetRow(i);
				pRow->PutForeColor(RGB(192,0,0));
			}
			else if (nType == 2){
				pRow = m_BillOtherInsuranceCombo->GetRow(i);
				pRow->PutForeColor(RGB(0,0,128));
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnBtnSwapInscos() 
{
	try {

		long nInsuranceID1 = -1;
		long nInsuranceID2 = -1;

		if(m_BillInsuranceCombo->GetCurSel() != -1)
			nInsuranceID1 = VarLong(m_BillInsuranceCombo->GetValue(m_BillInsuranceCombo->GetCurSel(), ipccID),-1);

		if(m_BillOtherInsuranceCombo->GetCurSel() != -1)
			nInsuranceID2 = VarLong(m_BillOtherInsuranceCombo->GetValue(m_BillOtherInsuranceCombo->GetCurSel(), ipccID),-1);

		if(nInsuranceID2 == -1)
			m_BillInsuranceCombo->CurSel = -1;
		else
			m_BillInsuranceCombo->SetSelByColumn(ipccID, (long)nInsuranceID2);
		if(nInsuranceID1 == -1)
			m_BillOtherInsuranceCombo->CurSel = -1;
		else 
			m_BillOtherInsuranceCombo->SetSelByColumn(ipccID, (long)nInsuranceID1);

	}NxCatchAll("Error swapping insurance companies.");
}

void CFinancialLineItemPostingDlg::SetDefaultData() {

	//fill in defaults for a blank, new posting

	//dollar amounts
	CString strZeroAmt;
	strZeroAmt = FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE);
	SetDlgItemText(IDC_PAY_TOTAL, strZeroAmt);
	SetDlgItemText(IDC_ADJ_TOTAL, strZeroAmt);
	SetDlgItemText(IDC_CASH_RECEIVED_LIP, strZeroAmt);
	SetDlgItemText(IDC_CHANGE_GIVEN_LIP, strZeroAmt);

	// (a.walling 2006-11-16 10:20) - PLID 23552 - Set default items for group code/reason list
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	m_pGroupCodeList->SetSelByColumn(gcccID, (long)-1);
	m_pReasonList->SetSelByColumn(rcccID, (long)-1);

	// (j.jones 2007-02-21 16:14) - PLID 23280 - set the default dates based on the payment date preference

	COleDateTime dtDate = COleDateTime::GetCurrentTime();

	// (j.jones 2010-08-26 17:35) - PLID 34016 - changed to be per user, defaults to the old global setting
	long nDefaultPaymentDateGlobal_Old = GetRemotePropertyInt("DefaultPaymentDate",1,0,"<None>",TRUE);

	long nDefaultPaymentDate = GetRemotePropertyInt("DefaultPaymentDate", nDefaultPaymentDateGlobal_Old, 0, GetCurrentUserName(),TRUE);
	if(nDefaultPaymentDate == 3 && m_nBatchPaymentID == -1) {
		// (j.jones 2007-05-04 09:47) - PLID 23280 - pull the last payment date from GlobalUtils
		//NOT USED if posting from a batch payment
		dtDate = GetLastPaymentDate();
	}
	else if(nDefaultPaymentDate == 2) {
		//find the bill date
		long nBillID = -1;
		if(m_nBillID != -1) {
			nBillID = m_nBillID;
		}
		else if(m_nChargeID != -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT BillID FROM ChargesT WHERE ID = {INT}",m_nChargeID);
			if(!rs->eof) {
				nBillID = AdoFldLong(rs, "BillID",-1);
			}
			rs->Close();
		}

		if(nBillID != -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}",nBillID);
			if(!rs->eof) {
				dtDate = AdoFldDateTime(rs, "Date");
			}
			rs->Close();
		}
	}
	// (j.jones 2010-05-25 14:38) - PLID 38876 - supported charge date
	else if(nDefaultPaymentDate == 4) {
		//find the charge date
		if(m_nChargeID != -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM LineItemT WHERE ID = {INT}",m_nChargeID);
			if(!rs->eof) {
				dtDate = AdoFldDateTime(rs, "Date");
			}
			rs->Close();
		}
		else if(m_nBillID != -1) {
			//select the earliest charge date on the bill
			_RecordsetPtr rs = CreateParamRecordset("SELECT Min(Date) AS MinDate FROM LineItemT "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"WHERE ChargesT.BillID = {INT} AND Deleted = 0",m_nBillID);
			if(!rs->eof) {
				dtDate = AdoFldDateTime(rs, "MinDate");
			}
			rs->Close();
		}
	}
	
	m_dtPayDate.SetValue(_variant_t(dtDate));
	m_dtAdjDate.SetValue(_variant_t(dtDate));

	//default to check payment
	OnSelChosenPayTypeCombo(m_PayMethodCombo->SetSelByColumn(0,(long)2));

	//TES 2003-12-31: Fill in the default category.
	// (j.jones 2008-07-11 14:59) - PLID 28756 - even though an insurance-specific category
	// may override these later, continue to do this now
	long nDefaultCat = GetRemotePropertyInt("DefaultPayCat", -1, 0, "<None>", true);
	if(nDefaultCat != -1) {
		m_PayCatCombo->SetSelByColumn(cccID, nDefaultCat);
		m_AdjCatCombo->SetSelByColumn(cccID, nDefaultCat);
	}

	// (j.jones 2006-08-02 16:38) - PLID 18689 - fill in the default payment category
	// (j.jones 2008-07-11 14:59) - PLID 28756 - even though an insurance-specific description
	// may override this later, continue to do this now
	OnSelChosenPayDescriptionCombo(m_PayDescCombo->SetSelByColumn(0,_bstr_t(GetRemotePropertyText("DefaultPayDesc","",0,"<None>",TRUE))));
	m_PayDescCombo->CurSel = -1;
}

void CFinancialLineItemPostingDlg::SetBatchPaymentData() {

	try {

		//set payment information to the batch payment data, and make read only

		_RecordsetPtr rs = CreateRecordset("SELECT Amount, Date, Description, PayCatID, ProviderID, Location, "
			"CheckNo, CheckAcctNo, BankName, BankRoutingNum "
			"FROM BatchPaymentsT WHERE ID = %li",m_nBatchPaymentID);

		if(!rs->eof) {

			//payment amount
			SetDlgItemText(IDC_PAY_TOTAL,FormatCurrencyForInterface(CalculateRemainingBatchPaymentBalance(m_nBatchPaymentID),FALSE,TRUE));

			//payment date
			m_dtPayDate.SetValue(_variant_t(AdoFldDateTime(rs, "Date",COleDateTime::GetCurrentTime())));

			// (j.jones 2011-03-07 13:36) - PLID 42411 - default the adjustment date to the same date
			m_dtAdjDate.SetValue(_variant_t(AdoFldDateTime(rs, "Date",COleDateTime::GetCurrentTime())));

			//payment category
			m_PayCatCombo->SetSelByColumn(cccID, AdoFldLong(rs, "PayCatID", -1));

			//payment method
			OnSelChosenPayTypeCombo(m_PayMethodCombo->SetSelByColumn(0,(long)2));

			//check number
			SetDlgItemText(IDC_CHECK_NO,AdoFldString(rs, "CheckNo",""));

			//account number
			SetDlgItemText(IDC_ACCOUNT_NUMBER,AdoFldString(rs, "CheckAcctNo",""));

			//bank name
			SetDlgItemText(IDC_BANK_NAME,AdoFldString(rs, "BankName",""));

			//bank number
			SetDlgItemText(IDC_BANK_NUMBER,AdoFldString(rs, "BankRoutingNum",""));

			//payment description
			SetDlgItemText(IDC_PAY_DESC,AdoFldString(rs, "Description",""));
			m_PayDescCombo->CurSel = -1;

			//provider combo
			m_ProviderCombo->SetSelByColumn(0,AdoFldLong(rs, "ProviderID",-1));

			//location combo
			m_LocationCombo->SetSelByColumn(0,AdoFldLong(rs, "Location",-1));

			//now disable everything
			m_ProviderCombo->Enabled = FALSE;
			m_LocationCombo->Enabled = FALSE;
			((CNxEdit*)GetDlgItem(IDC_PAY_DESC))->SetReadOnly(TRUE);
			m_PayDescCombo->Enabled = FALSE;
			GetDlgItem(IDC_EDIT_PAY_DESC)->EnableWindow(FALSE);
			((CNxEdit*)GetDlgItem(IDC_PAY_TOTAL))->SetReadOnly(TRUE);
			((CNxEdit*)GetDlgItem(IDC_CHECK_NO))->SetReadOnly(TRUE);
			((CNxEdit*)GetDlgItem(IDC_ACCOUNT_NUMBER))->SetReadOnly(TRUE);
			((CNxEdit*)GetDlgItem(IDC_BANK_NAME))->SetReadOnly(TRUE);
			((CNxEdit*)GetDlgItem(IDC_BANK_NUMBER))->SetReadOnly(TRUE);
			m_dtPayDate.EnableWindow(FALSE);
			m_PayCatCombo->Enabled = FALSE;
			m_PayMethodCombo->Enabled = FALSE;
			m_PayRespCombo->Enabled = FALSE;

		}
		rs->Close();

	}NxCatchAll("Error loading payment information.");
}

void CFinancialLineItemPostingDlg::OnSelChosenPayTypeCombo(long nRow) 
{
	try {

		if(nRow == -1) {
			nRow = 0;
		}

		long nPaymentMethod = VarLong(m_PayMethodCombo->GetValue(nRow,0));

		switch(nPaymentMethod) {

			case 1: //Cash
				GetDlgItem(IDC_CARD_TYPE_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_EXP_DATE_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_NUMBER_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_AUTH_NUM_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_NAME_ON_CARD_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_EXP_DATE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_NUMBER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EDIT_AUTHORIZATION)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_NAME_ON_CARD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_HIDE);

				GetDlgItem(IDC_CHECK_NO_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BANK_NAME_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ACCT_NUM_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BANK_NUMBER_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CHECK_NO)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BANK_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BANK_NUMBER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ACCOUNT_NUMBER)->ShowWindow(SW_HIDE);

				GetDlgItem(IDC_CASH_RECEIVED_LIP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CASH_RECEIVED_LIP_LABEL)->ShowWindow(SW_SHOW);		
				GetDlgItem(IDC_CHANGE_GIVEN_LIP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CHANGE_GIVEN_LIP_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CURRENCY_SYMBOL_RECEIVED_LIP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CURRENCY_SYMBOL_GIVEN_LIP)->ShowWindow(SW_SHOW);
				break;

			case 2: {	//Check
				GetDlgItem(IDC_CARD_TYPE_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_EXP_DATE_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_NUMBER_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_AUTH_NUM_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_NAME_ON_CARD_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_EXP_DATE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_NUMBER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EDIT_AUTHORIZATION)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CC_NAME_ON_CARD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_HIDE);

				GetDlgItem(IDC_CHECK_NO_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_BANK_NAME_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ACCT_NUM_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_BANK_NUMBER_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CHECK_NO)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_BANK_NAME)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_BANK_NUMBER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ACCOUNT_NUMBER)->ShowWindow(SW_SHOW);

				GetDlgItem(IDC_CASH_RECEIVED_LIP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CASH_RECEIVED_LIP_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CHANGE_GIVEN_LIP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CHANGE_GIVEN_LIP_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CURRENCY_SYMBOL_RECEIVED_LIP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CURRENCY_SYMBOL_GIVEN_LIP)->ShowWindow(SW_HIDE);

				//auto-fill the bank info based on the patient's last check payment
				_RecordsetPtr rsCheckPayments(__uuidof(Recordset));
				COleDateTime dt;
				CString strSql;
				strSql.Format("SELECT TOP 1 BankNo, CheckAcctNo, BankRoutingNum FROM PaymentsT INNER JOIN LineItemT "
					"ON PaymentsT.ID = LineItemT.ID INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
					"WHERE (LineItemT.PatientID = %li AND Deleted = 0 AND PayMethod = '2') ORDER BY Date DESC", m_nPatientID);
				rsCheckPayments = CreateRecordset(strSql);
				if(!rsCheckPayments->eof) {
					FieldsPtr pflds = rsCheckPayments->GetFields();
					SetDlgItemText(IDC_BANK_NUMBER, AdoFldString(pflds, "BankNo", ""));
					SetDlgItemText(IDC_ACCOUNT_NUMBER, AdoFldString(pflds, "CheckAcctNo", ""));
					SetDlgItemText(IDC_BANK_NUMBER, AdoFldString(pflds, "BankRoutingNum", ""));
				}
				rsCheckPayments->Close();
				break;
			}

			case 3: {	//Credit Card
				GetDlgItem(IDC_CARD_TYPE_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CC_EXP_DATE_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CC_NUMBER_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_AUTH_NUM_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CC_NAME_ON_CARD_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CC_EXP_DATE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CC_NUMBER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_EDIT_AUTHORIZATION)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CC_NAME_ON_CARD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_SHOW);

				GetDlgItem(IDC_CHECK_NO_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BANK_NAME_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ACCT_NUM_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BANK_NUMBER_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CHECK_NO)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BANK_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BANK_NUMBER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ACCOUNT_NUMBER)->ShowWindow(SW_HIDE);

				GetDlgItem(IDC_CASH_RECEIVED_LIP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CASH_RECEIVED_LIP_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CHANGE_GIVEN_LIP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CHANGE_GIVEN_LIP_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CURRENCY_SYMBOL_RECEIVED_LIP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CURRENCY_SYMBOL_GIVEN_LIP)->ShowWindow(SW_HIDE);

				//if the option is enabled, auto-fill the credit card information based on the patient's last credit payment
				if(m_bGetChargeInfo != 0) {
					_RecordsetPtr rsCreditPayments(__uuidof(Recordset));
					CString str;
					// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
					// (e.lally 2007-07-16) PLID 25993 - Use the credit card ID instead of name, in case of duplicates
					str.Format("SELECT TOP 1 CreditCardID "
						"FROM PaymentsT "
						"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
						"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
						"INNER JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
						"WHERE (LineItemT.PatientID = %li AND Deleted = 0 "
						"AND PayMethod = '3') "
						"ORDER BY Date DESC ", m_nPatientID);
					
					
					rsCreditPayments = CreateRecordset(str);
					if(!rsCreditPayments->eof){
						// if the patient has previous credit card payments, set the info used with with the most recent payment
						// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
						// (e.lally 2007-07-16) PLID 25993 - Use the credit card ID instead of name, in case of duplicates
						long nLastCreditCardUsed = AdoFldLong(rsCreditPayments, "CreditCardID");
						SetCreditCardInfoForType(nLastCreditCardUsed);
					}
					rsCreditPayments->Close();
				}
				break;
			}
		}


	}NxCatchAll("Error selecting payment method.");
}

void CFinancialLineItemPostingDlg::SetCreditCardInfoForType(const IN long nCreditCardID)
{
	// (e.lally 2007-07-16) PLID 25993 - Use the credit card ID instead of name, in case of duplicates
	try{
		CString str;
		COleDateTime dt;	
		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
			//technically it is possible for the CCNumber et al to exist without a credit card ID, so
			//it should probably be a left join in this case, even though we filter out the nulls.
		// (a.walling 2007-10-31 09:09) - PLID 27891 - Include encrypted CC number
		// (j.gruber 2009-12-22 09:14) - PLID 27936 - took out auth number
		// (a.walling 2010-03-15 12:26) - PLID 37751 - Include KeyIndex
		_RecordsetPtr rsCreditPayments = CreateParamRecordset("SELECT TOP 1 CreditCardID, CCNumber, SecurePAN, KeyIndex, CCHoldersName, CCExpDate "
			"FROM PaymentsT "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"WHERE LineItemT.PatientID = {INT} AND Deleted = 0 "
			"AND PayMethod = '3' AND CreditCardID = {INT} "
			"ORDER BY Date DESC, InputDate DESC, LineItemT.ID DESC", m_nPatientID, nCreditCardID);
		if(!rsCreditPayments->eof) {
			// there are payments for this patient with the credit card we are looking for, so set the Credit card fields
			// to the info that was used with the most recent payment date
			_variant_t var;
			var = rsCreditPayments->Fields->Item["CreditCardID"]->Value;
			if(var.vt != VT_NULL) {
				//(e.lally 2007-07-16) PLID 25993 - use the ID instead in case of duplicates
				m_PayCardTypeCombo->TrySetSelByColumn(cccCardID, nCreditCardID);
				
				var = rsCreditPayments->Fields->Item["CCExpDate"]->Value;
				if(var.vt==VT_DATE) {
					dt = var.date;
					GetDlgItem(IDC_CC_EXP_DATE)->SetWindowText(dt.Format("%m/%y"));
				}
				//(e.lally 2007-10-30) PLID 27892 - Cache the full card number, but only display last 4 digits
				// (a.walling 2007-10-30 18:02) - PLID 27891 - CCs are now encrypted
				// (a.walling 2010-03-15 12:26) - PLID 37751 - Use NxCrypto
				CString strCreditCardNumber;
				NxCryptosaur.DecryptStringFromVariant(rsCreditPayments->Fields->Item["SecurePAN"]->Value, AdoFldLong(rsCreditPayments, "KeyIndex", -1), strCreditCardNumber);
				// (j.jones 2015-09-30 11:04) - PLID 67175 - if we have a last 4, but not a CC number, use the last 4
				CString strDisplayedCC = "";
				if (strCreditCardNumber.IsEmpty()) {
					strDisplayedCC = AdoFldString(rsCreditPayments, "CCNumber", "");
				}
				else {
					strDisplayedCC = strCreditCardNumber.Right(4);
				}
				m_nxeditCCLast4.SetWindowText(strDisplayedCC);

				var = rsCreditPayments->Fields->Item["CCHoldersName"]->Value;
				if(var.vt == VT_BSTR)
					str = CString(var.bstrVal);
				else
					str = "";
				GetDlgItem(IDC_CC_NAME_ON_CARD)->SetWindowText(str);

				// (j.gruber 2009-12-22 09:14) - PLID 27936 - took out auth number
				//var = rsCreditPayments->Fields->Item["CCAuthNo"]->Value;
			}
		}
		else{
			// there was no info found for this credit card type, clear the data
			// JJ - we decided NOT to do this, what if they start typing first?
		}

		rsCreditPayments->Close();
	}NxCatchAll("Error setting current credit card info");
}

void CFinancialLineItemPostingDlg::OnSelChosenComboCardName(long nRow) 
{
	try {
		// (e.lally 2007-07-09) PLID 25993 - Update credit card columns
		// if the option is enabled, auto-fill the credit card information based on the patient's last credit payment
		// for the selected credit card type
		// (e.lally 2007-07-16) PLID 25993 - Use the credit card ID instead of name, in case of duplicates
		if(m_bGetChargeInfo != 0) {
			long nCurCardType = m_PayCardTypeCombo->CurSel == -1 ? -1 : VarLong(m_PayCardTypeCombo->GetValue(m_PayCardTypeCombo->GetCurSel(),cccCardID));
			SetCreditCardInfoForType(nCurCardType);
		}
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnEditCardName() 
{
	try {
		// (e.lally 2007-07-09) PLID 25993 - Use the credit card ID instead of name, in case of duplicates
		long nCardID = -1;
		if(m_PayCardTypeCombo->CurSel != -1){
			nCardID = VarLong(m_PayCardTypeCombo->GetValue(m_PayCardTypeCombo->CurSel, cccCardID));
		}
		//(e.lally 2007-07-11) PLID 26590 - Use the new edit dlg for credit cards
			// - requery the cc combo if they OK'd to save (even if no changes were actually made).
		CEditCreditCardsDlg dlg(this);
		if(IDOK==dlg.DoModal())
			m_PayCardTypeCombo->Requery();

		//Use the ID in case of duplicates
		m_PayCardTypeCombo->TrySetSelByColumn(cccCardID, nCardID);
	}NxCatchAll(__FUNCTION__);
}

BOOL CFinancialLineItemPostingDlg::PreTranslateMessage(MSG* pMsg) 
{
	BOOL IsShiftKeyDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);

	if (pMsg->message == WM_SYSKEYDOWN) {
		switch (pMsg->wParam) {
			case 'P':	//Post
				//OnOK();	//commented out because the dialog automatically detects the 'P' hotkey
				break;
		}
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {

	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CFinancialLineItemPostingDlg::OnEditingFinishingLineItemPostingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		BOOL bIsShiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
		BOOL bIsTabDown = (GetAsyncKeyState(VK_TAB) & 0x80000000) || IsMessageInQueue(NULL, WM_KEYUP, VK_TAB, 0, IMIQ_MATCH_WPARAM);

		if(nRow == -1)
			return;

		if(pvarNewValue->vt != VT_CY) {
			AfxMessageBox("Please enter a valid amount.");
			*pbCommit = FALSE;
			*pbContinue = FALSE;
			return;
		}

		if(pvarNewValue->vt == VT_CY) {
			// (j.jones 2014-07-01 08:49) - PLID 62553 - for vision posting, negative payments are allowed			
			// (j.jones 2016-05-11 8:45) - NX-100503 - you can now enter negative adjustments
			if (VarCurrency(pvarNewValue) < COleCurrency(0, 0)
				&& (m_ePayType != eVisionPayment || nCol != m_nNewPaysColIndex)
				&& nCol != m_nAdjustmentColIndex) {

				AfxMessageBox("Please enter a positive amount.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}

		/////////////////////////////////////
		// Round currency
		if (pvarNewValue->vt == VT_CY) {
			COleCurrency cy = pvarNewValue->cyVal;
			RoundCurrency(cy);
			pvarNewValue->cyVal = cy;
		}

		IRowSettingsPtr pRow = m_PostingList->GetRow(nRow);
		COleCurrency cyCharges = VarCurrency(pRow->GetValue(m_nChargesColIndex),COleCurrency(0,0));
		COleCurrency cyPatResp = VarCurrency(pRow->GetValue(m_nPatRespColIndex),COleCurrency(0,0));
		COleCurrency cyInsResp = VarCurrency(pRow->GetValue(m_nInsRespColIndex),COleCurrency(0,0));
		
		// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new payments,
		// plus we cache the initial existing total in an "old" column
		COleCurrency cyOldExistingPatPays = VarCurrency(pRow->GetValue(plcOldExistingPatPays),COleCurrency(0,0));
		COleCurrency cyExistingPatPays = VarCurrency(pRow->GetValue(m_nExistingPatPaysColIndex),COleCurrency(0,0));		
		COleCurrency cyOldExistingInsPays = VarCurrency(pRow->GetValue(plcOldExistingInsPays),COleCurrency(0,0));
		COleCurrency cyExistingInsPays = VarCurrency(pRow->GetValue(m_nExistingInsPaysColIndex),COleCurrency(0,0));

		// (j.jones 2010-05-05 09:49) - PLID 25521 - New Pays is just one column now
		COleCurrency cyNewPaymentAmt = VarCurrency(pRow->GetValue(m_nNewPaysColIndex), COleCurrency(0, 0));

		// (j.jones 2014-07-02 08:56) - PLID 62553 - if a vision payment, and the new pays
		// is negative, it's a chargeback, so don't calculate it like a new payment
		COleCurrency cyChargeback = COleCurrency(0, 0);
		if (m_ePayType == eVisionPayment && cyNewPaymentAmt < COleCurrency(0, 0)) {
			cyChargeback = cyNewPaymentAmt;
			cyNewPaymentAmt = COleCurrency(0, 0);
		}

		COleCurrency cyInsAdjustments = VarCurrency(pRow->GetValue(m_nAdjustmentColIndex),COleCurrency(0,0));
		COleCurrency cyAllowable = VarCurrency(pRow->GetValue(m_nAllowableColIndex),COleCurrency(0,0));
		COleCurrency cyPatBalance = VarCurrency(pRow->GetValue(m_nPatBalanceColIndex),COleCurrency(0,0));
		COleCurrency cyInsBalance = VarCurrency(pRow->GetValue(m_nInsBalanceColIndex),COleCurrency(0,0));

		//the dollar amount entered
		COleCurrency cyNewAmt = COleCurrency(pvarNewValue->cyVal);

		//for all columns, make sure that the user is keeping the posting balanced		
		if(nCol == m_nPatRespColIndex)
		{
			if(*pbCommit) {
				if(cyNewAmt > cyCharges) {
					AfxMessageBox("You cannot make the patient responsibility be greater than the charge amount.");
					*pbCommit = FALSE;
					return;
				}
			}
		}
		else if(nCol == m_nInsRespColIndex)
		{
			if(*pbCommit) {
				if(cyNewAmt > cyCharges) {
					AfxMessageBox("You cannot make the insurance responsibility be greater than the charge amount.");
					*pbCommit = FALSE;
					return;
				}
			}
		}
		// (j.jones 2010-05-05 10:37) - PLID 25521 - existing pays are now their own columns
		else if(nCol == m_nExistingPatPaysColIndex)
		{
			// (j.jones 2010-06-02 15:58) - PLID 37200 - we can now increase only by how much is available in the "unapplied" amount
			COleCurrency cyAmtIncreased = cyNewAmt - VarCurrency(varOldValue, COleCurrency(0,0));
			//we only need to validate this number if they increased this amount
			if(cyAmtIncreased > COleCurrency(0,0)) {								
				if(cyAmtIncreased > m_cyUnappliedPatAmt) {
					AfxMessageBox("You must unapply other patient payments before attempting to increase this apply amount.");
					*pbCommit = FALSE;
					return;
				}
				else if (m_nInsuredPartyID == -1 && cyNewPaymentAmt + cyNewAmt > cyPatResp) {
					AfxMessageBox("You cannot apply a payment that exceeds the current patient responsibility.");
					*pbCommit = FALSE;
					return;
				}
				else if(m_nInsuredPartyID != -1 && cyNewAmt > cyPatResp) {
					AfxMessageBox("You cannot apply a payment that exceeds the current patient responsibility.");
					*pbCommit = FALSE;
					return;
				}
			}
		}
		else if(nCol == m_nExistingInsPaysColIndex)
		{
			// (j.jones 2010-06-02 15:58) - PLID 37200 - we can now increase only by how much is available in the "unapplied" amount
			COleCurrency cyAmtIncreased = cyNewAmt - VarCurrency(varOldValue, COleCurrency(0,0));
			//we only need to validate this number if they increased this amount
			if(cyAmtIncreased > COleCurrency(0,0)) {
				if(cyAmtIncreased > m_cyUnappliedInsAmt) {
					AfxMessageBox("You must unapply other insurance payments before attempting to increase this apply amount.");
					*pbCommit = FALSE;
					return;
				}
				else if (cyNewPaymentAmt + cyNewAmt > cyInsResp) {
					AfxMessageBox("You cannot apply a payment that exceeds the current insurance responsibility.");
					*pbCommit = FALSE;
					return;
				}
			}
		}
		// (j.jones 2010-05-04 16:30) - PLID 25521 - this is now strictly used for posting new payments
		else if(nCol == m_nNewPaysColIndex)
		{			

			if(*pbCommit) {
				// (j.jones 2012-08-14 10:09) - PLID 52117 - new payments are allowed to be blank
				CString strEntered = strUserEntered; 
				strEntered.Trim(" "); 
				if(strEntered.IsEmpty()) {
					*pvarNewValue = g_cvarNull; 
				}
			}

			if(*pbCommit) {

				// (j.jones 2014-07-02 08:56) - PLID 62553 - if a vision payment, and the new pays
				// is negative, it's a chargeback, so don't calculate it like a new payment
				if (m_ePayType == eVisionPayment && cyNewAmt < COleCurrency(0, 0)) {
					//This is a chargeback, so ensure it is not a larger (negative) value than the charge total.
					//This has no effect on the balance. It also doesn't care about existing chargebacks.
					if (-cyNewAmt > cyCharges) {
						AfxMessageBox("You cannot enter a chargeback that is greater than the charge total.");
						*pbCommit = FALSE;
						return;
					}
				}
				else {
					//this is a normal payment					
					if (m_nInsuredPartyID == -1 && cyNewAmt + cyExistingPatPays > cyPatResp) {
						// (j.jones 2016-05-09 15:55) - NX-100502 - overpayments are now allowed						
						if (IDNO == DontShowMeAgain(this, "The payment you entered exceeds the current patient responsibility.\n\n"
							"Are you sure you wish to continue?", "LineItemPosting_EditFinishOverpayment", "Practice", FALSE, TRUE)) {
							*pbCommit = FALSE;
							return;
						}
					}
					// (j.jones 2010-06-10 14:31) - PLID 38901 - for new payments, we now allow them to type in
					// an amount that is more than the current insurance responsibility, it just cannot exceed
					// the current insurance resp + the current patient resp.
					else if (m_nInsuredPartyID != -1 && (cyNewAmt + cyExistingInsPays) > (cyInsResp + cyPatResp)) {
						// (j.jones 2016-05-09 15:55) - NX-100502 - overpayments are now allowed
						if (IDNO == DontShowMeAgain(this, "The total amount of insurance payments entered exceeds the total value of the "
							"current insurance and patient responsibilities.\n\n"
							"Are you sure you wish to continue?", "LineItemPosting_EditFinishTotalOverage", "Practice", FALSE, TRUE)) {
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}
		}
		else if(nCol == m_nAllowableColIndex)
		{
		}
		else if(nCol == m_nAdjustmentColIndex)
		{
			if(*pbCommit) {
				if(cyNewAmt > cyInsResp) {
					// (j.jones 2016-05-09 15:55) - NX-100502 - overadjustments are now allowed
					if (IDNO == DontShowMeAgain(this, "The adjustment you entered is greater than the current insurance responsibility.\n\n"
						"Are you sure you wish to continue?", "LineItemPosting_EditFinishOveradjust", "Practice", FALSE, TRUE)) {
						*pbCommit = FALSE;
						return;
					}
				}
			}
		}
		// (b.spivey, January 05, 2012) - PLID 47121 - If the str is empty, we set to null because null is a valid value for 
		//		deductible and coinsurance. 
		// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
		else if(nCol == m_nDeductibleAmtColIndex || nCol == m_nCoinsuranceAmtColIndex || nCol == m_nCopayAmtColIndex)
		{
			if(*pbCommit) {
				CString strEntered = strUserEntered; 
				strEntered.Trim(" "); 
				if(strEntered.IsEmpty()) {
					*pvarNewValue = g_cvarNull; 
				}
			}
		}

	}NxCatchAll("Error editing amount.");
}

void CFinancialLineItemPostingDlg::OnEditingFinishedLineItemPostingList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	BOOL bIsShiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
	BOOL bIsTabDown = (GetAsyncKeyState(VK_TAB) & 0x80000000) || IsMessageInQueue(NULL, WM_KEYUP, VK_TAB, 0, IMIQ_MATCH_WPARAM);

	try {

		// (j.jones 2007-02-01 17:30) - PLID 24541 - for tracking focus changes
		m_bIsEditingCharges = FALSE;

		if(nRow == -1)
			return;

		IRowSettingsPtr pRow = m_PostingList->GetRow(nRow);
		long nChargeID = VarLong(pRow->GetValue(plcChargeID), -1);
		long nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID), -1);
		COleCurrency cyCharges = VarCurrency(pRow->GetValue(m_nChargesColIndex),COleCurrency(0,0));
		COleCurrency cyPatResp = VarCurrency(pRow->GetValue(m_nPatRespColIndex),COleCurrency(0,0));
		COleCurrency cyInsResp = VarCurrency(pRow->GetValue(m_nInsRespColIndex),COleCurrency(0,0));
		COleCurrency cyExistingPatResp = GetChargePatientResp(nChargeID);
		COleCurrency cyExistingInsResp = GetChargeInsResp(nChargeID, m_nInsuredPartyID);
		COleCurrency cyInsBal = VarCurrency(pRow->GetValue(m_nInsBalanceColIndex),COleCurrency(0,0));

		BOOL bIsRevCode = IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && nRevCodeID != -1;

		// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new payments,
		// plus we cache the initial existing total in an "old" column
		COleCurrency cyOldExistingPatPays = VarCurrency(pRow->GetValue(plcOldExistingPatPays),COleCurrency(0,0));
		COleCurrency cyExistingPatPays = VarCurrency(pRow->GetValue(m_nExistingPatPaysColIndex),COleCurrency(0,0));
		COleCurrency cyOldExistingInsPays = VarCurrency(pRow->GetValue(plcOldExistingInsPays),COleCurrency(0,0));
		COleCurrency cyExistingInsPays = VarCurrency(pRow->GetValue(m_nExistingInsPaysColIndex),COleCurrency(0,0));

		// (j.jones 2010-05-05 09:49) - PLID 25521 - New Pays is just one column now
		COleCurrency cyNewPays = VarCurrency(pRow->GetValue(m_nNewPaysColIndex),COleCurrency(0,0));

		COleCurrency cyNewPatPays = COleCurrency(0,0);
		COleCurrency cyNewInsPays = COleCurrency(0,0);
		if(m_nInsuredPartyID == -1) {
			cyNewPatPays = cyNewPays;
		}
		else {
			cyNewInsPays = cyNewPays;
		}

		// (j.jones 2014-07-01 15:42) - PLID 62553 - chargebacks do not affect the balance
		COleCurrency cyTotalPatPays = cyExistingPatPays + (cyNewPatPays < COleCurrency(0, 0) ? COleCurrency(0, 0) : cyNewPatPays);
		COleCurrency cyTotalInsPays = cyExistingInsPays + (cyNewInsPays < COleCurrency(0, 0) ? COleCurrency(0, 0) : cyNewInsPays);

		COleCurrency cyInsAdjustments = VarCurrency(pRow->GetValue(m_nAdjustmentColIndex),COleCurrency(0,0));
		COleCurrency cyAllowable = VarCurrency(pRow->GetValue(m_nAllowableColIndex),COleCurrency(0,0));
		COleCurrency cyPatBalance = VarCurrency(pRow->GetValue(m_nPatBalanceColIndex),COleCurrency(0,0));
		COleCurrency cyInsBalance = VarCurrency(pRow->GetValue(m_nInsBalanceColIndex),COleCurrency(0,0));

		//if any column causes another to change, assume OnEditingFinishing warned the user, so commit the change		
		if(nCol == m_nExistingPatPaysColIndex)
		{
			// (j.jones 2010-06-02 14:15) - PLID 37200 - changing existing amounts should update the unapplied labels
			COleCurrency cyAmtChanged = VarCurrency(varOldValue, COleCurrency(0,0)) - cyExistingPatPays;
			m_cyUnappliedPatAmt += cyAmtChanged;

			m_nxstaticPatAmtToUnapply.SetWindowText(FormatCurrencyForInterface(m_cyUnappliedPatAmt));

			if(m_cyUnappliedPatAmt == COleCurrency(0,0)) {
				m_nxstaticPatAmtToUnapplyLabel.ShowWindow(SW_HIDE);
				m_nxstaticPatAmtToUnapply.ShowWindow(SW_HIDE);
			}
			else {
				m_nxstaticPatAmtToUnapplyLabel.ShowWindow(SW_SHOW);
				m_nxstaticPatAmtToUnapply.ShowWindow(SW_SHOW);
			}
		}
		else if(nCol == m_nExistingInsPaysColIndex)
		{
			// (j.jones 2010-06-02 14:15) - PLID 37200 - changing existing amounts should update the unapplied labels
			COleCurrency cyAmtChanged = VarCurrency(varOldValue, COleCurrency(0,0)) - cyExistingInsPays;
			m_cyUnappliedInsAmt += cyAmtChanged;

			m_nxstaticInsAmtToUnapply.SetWindowText(FormatCurrencyForInterface(m_cyUnappliedInsAmt));

			if(m_cyUnappliedInsAmt == COleCurrency(0,0)) {
				m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_HIDE);
				m_nxstaticInsAmtToUnapply.ShowWindow(SW_HIDE);
			}
			else {
				m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_SHOW);
				m_nxstaticInsAmtToUnapply.ShowWindow(SW_SHOW);
			}
		}
		else if(nCol == m_nPatRespColIndex)
		{
			if(cyPatResp < COleCurrency(varOldValue.cyVal)) {
				cyInsResp += (COleCurrency(varOldValue.cyVal) - cyPatResp);
				pRow->PutValue(m_nInsRespColIndex,_variant_t(cyInsResp));
			}
			else {
				cyInsResp -= (cyPatResp - COleCurrency(varOldValue.cyVal));
				pRow->PutValue(m_nInsRespColIndex,_variant_t(cyInsResp));
			}

			// (j.jones 2012-05-02 15:19) - PLID 48367 - if the new amount is less than the existing amount applied,
			// offer to unapply the existing patient applies
			if(cyPatResp < cyExistingPatPays) {
				COleCurrency cyDiff = cyExistingPatPays - cyPatResp;
				CString strWarn;
				strWarn.Format("The new patient responsibility of %s is less than the currently applied patient total of %s.\n\n"
					"Would you like to unapply %s in patient credits?\n\n"
					"If not, you will need to reduce the value in the 'Prev Pat Pays' column or increase the patient responsibility before posting.",
					FormatCurrencyForInterface(cyPatResp), FormatCurrencyForInterface(cyExistingPatPays), FormatCurrencyForInterface(cyDiff));
				if(IDYES == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
					//reduce the applied amount, and update labels
					m_cyUnappliedPatAmt += cyDiff;

					cyExistingPatPays = cyPatResp;
					cyTotalPatPays = cyExistingPatPays + cyNewPatPays;
					pRow->PutValue(m_nExistingPatPaysColIndex, _variant_t(cyExistingPatPays));

					m_nxstaticPatAmtToUnapply.SetWindowText(FormatCurrencyForInterface(m_cyUnappliedPatAmt));

					if(m_cyUnappliedPatAmt == COleCurrency(0,0)) {
						m_nxstaticPatAmtToUnapplyLabel.ShowWindow(SW_HIDE);
						m_nxstaticPatAmtToUnapply.ShowWindow(SW_HIDE);
					}
					else {
						m_nxstaticPatAmtToUnapplyLabel.ShowWindow(SW_SHOW);
						m_nxstaticPatAmtToUnapply.ShowWindow(SW_SHOW);
					}
				}
			}

			// (j.jones 2014-07-01 09:28) - PLID 62552 - vision payments have a dedicated adjustment function
			if (m_ePayType == eVisionPayment && m_checkAutoAdjustBalances.GetCheck()) {
				//this calculates for chargebacks too, it will only not calculate if
				//the payment column is null
				COleCurrency cyNewAdj = CalculateAdjustment_Vision(cyInsResp, cyPatResp,
					pRow->GetValue(m_nNewPaysColIndex).vt == VT_CY ? _variant_t(cyTotalInsPays) : g_cvarNull);

				pRow->PutValue(m_nAdjustmentColIndex, _variant_t(cyNewAdj));

				//if we are tracking a pointer for this adjustment, update it
				ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
				if (pAdjInfo) {
					UpdateAdjustmentPointers(cyNewAdj, pAdjInfo);
				}

				cyInsAdjustments = cyNewAdj;
				AutoFillAdjustmentTotal();
			}
		}
		else if(nCol == m_nInsRespColIndex)
		{
			if(cyInsResp < COleCurrency(varOldValue.cyVal)) {
				cyPatResp += (COleCurrency(varOldValue.cyVal) - cyInsResp);
				pRow->PutValue(m_nPatRespColIndex,_variant_t(cyPatResp));
			}
			else {
				cyPatResp -= (cyInsResp - COleCurrency(varOldValue.cyVal));
				pRow->PutValue(m_nPatRespColIndex,_variant_t(cyPatResp));
			}

			// (j.jones 2012-05-02 15:19) - PLID 48367 - if the new amount is less than the existing amount applied,
			// offer to unapply the existing insurance applies
			if(cyInsResp < cyExistingInsPays) {
				COleCurrency cyDiff = cyExistingInsPays - cyInsResp;
				CString strWarn;
				strWarn.Format("The new insurance responsibility of %s is less than the currently applied insurance total of %s.\n\n"
					"Would you like to unapply %s in insurance credits?\n\n"
					"If not, you will need to reduce the value in the 'Prev Ins Pays' column or increase the insurance responsibility before posting.",
					FormatCurrencyForInterface(cyInsResp), FormatCurrencyForInterface(cyExistingInsPays), FormatCurrencyForInterface(cyDiff));
				if(IDYES == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
					//reduce the applied amount, and update labels
					m_cyUnappliedInsAmt += cyDiff;

					cyExistingInsPays = cyInsResp;
					cyTotalInsPays = cyExistingInsPays + cyNewInsPays;
					pRow->PutValue(m_nExistingInsPaysColIndex, _variant_t(cyExistingInsPays));

					m_nxstaticInsAmtToUnapply.SetWindowText(FormatCurrencyForInterface(m_cyUnappliedInsAmt));

					if(m_cyUnappliedInsAmt == COleCurrency(0,0)) {
						m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_HIDE);
						m_nxstaticInsAmtToUnapply.ShowWindow(SW_HIDE);
					}
					else {
						m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_SHOW);
						m_nxstaticInsAmtToUnapply.ShowWindow(SW_SHOW);
					}
				}
			}

			// (j.jones 2014-07-01 09:28) - PLID 62552 - vision payments have a dedicated adjustment function
			if (m_ePayType == eVisionPayment && m_checkAutoAdjustBalances.GetCheck()) {
				//this calculates for chargebacks too, it will only not calculate if
				//the payment column is null
				COleCurrency cyNewAdj = CalculateAdjustment_Vision(cyInsResp, cyPatResp,
					pRow->GetValue(m_nNewPaysColIndex).vt == VT_CY ? _variant_t(cyTotalInsPays) : g_cvarNull);

				pRow->PutValue(m_nAdjustmentColIndex, _variant_t(cyNewAdj));

				//if we are tracking a pointer for this adjustment, update it
				ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
				if (pAdjInfo) {
					UpdateAdjustmentPointers(cyNewAdj, pAdjInfo);
				}

				cyInsAdjustments = cyNewAdj;
				AutoFillAdjustmentTotal();
			}
		}
		else if(m_nInsuredPartyID != -1 && (nCol == m_nNewPaysColIndex || nCol == m_nAllowableColIndex))
		{
			//for now, we do the same thing for Ins. Pays. and the Allowable, 
			//if the allowable is zero. So don't break after we do our code here.

			if(nCol == m_nNewPaysColIndex) {
				//reduce the adjustment if needed
				if(cyInsAdjustments > COleCurrency(0,0) && cyTotalInsPays + cyInsAdjustments > cyInsResp) {
					cyInsAdjustments = cyInsResp - cyTotalInsPays;
					// (j.jones 2012-08-08 11:08) - PLID 47778 - don't let this drop below zero
					// (j.jones 2016-05-11 8:45) - NX-100503 - you can now manually enter negative adjustments,
					// but this code is still accurate - we would never calculate a negative one
					if(cyInsAdjustments < COleCurrency(0,0)) {
						cyInsAdjustments = COleCurrency(0,0);
					}
					pRow->PutValue(m_nAdjustmentColIndex,_variant_t(cyInsAdjustments));

					// (j.jones 2012-05-08 10:21) - PLID 37165 - if we are tracking a pointer for this adjustment, update it
					ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
					if(pAdjInfo) {
						// (j.jones 2012-05-08 10:26) - PLID 37165 - updates adjustment amounts in tracked pointers
						UpdateAdjustmentPointers(cyInsAdjustments, pAdjInfo);
					}

					AutoFillAdjustmentTotal();
				}
			}
			else if(nCol == m_nAllowableColIndex) {
				// (j.jones 2012-07-30 10:58) - PLID 47778 - added ability to auto-calculate payment amounts, for primary insurance only,
				// we will not try to automate it though if they have manually edited the payment value
				// (j.jones 2012-08-28 17:10) - PLID 52335 - secondary payments aren't calculated here, instead they get calculated later in this function
				if(m_nLineItemPostingAutoPayment != 0 && !VarBool(pRow->GetValue(plcHasEditedPayment)) && m_nInsuredPartyID != -1 && m_bIsPrimaryIns) {
					// (j.jones 2012-07-30 11:36) - PLID 51863 - added ability to auto-calculate payment amounts by revenue code
					
					//don't calculate payments on zero dollar charges
					COleCurrency cyCharges = VarCurrency(pRow->GetValue(m_nChargesColIndex),COleCurrency(0,0));
					COleCurrency cyInsBal = VarCurrency(pRow->GetValue(m_nInsBalanceColIndex),COleCurrency(0,0));
					if(cyCharges > COleCurrency(0,0)) {
						if(bIsRevCode) {
							// (j.jones 2012-08-28 17:56) - PLID 52335 - the balance should calculate as whatever the current balance is,
							// with the current new payments added back into it
							cyNewInsPays = CalculatePaymentForRevenueCode(TRUE, VarString(pRow->GetValue(plcItemCode), ""), nRevCodeID, cyCharges, cyInsBal + cyNewInsPays,
								cyAllowable, pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
						}
						else {
							// (j.jones 2012-08-28 17:56) - PLID 52335 - the balance should calculate as whatever the current balance is,
							// with the current new payments added back into it
							cyNewInsPays = CalculatePayment(TRUE, VarString(pRow->GetValue(plcItemCode), ""), cyCharges, cyInsBal + cyNewInsPays,
								cyAllowable, pRow->GetValue(plcCoinsurancePercent),
								pRow->GetValue(plcCopayMoney), pRow->GetValue(plcCopayPercent),
								pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
						}
					}

					//update our total
					cyTotalInsPays = cyExistingInsPays + cyNewInsPays;
					
					//update the row
					
					// (j.jones 2012-08-14 10:15) - PLID 52117 - A payment can be blank now instead of zero,
					// so if it is already blank and we're calculating zero, leave it blank. If it is non-null,
					// don't clear it out if zero.
					if(pRow->GetValue(m_nNewPaysColIndex).vt == VT_CY || cyNewInsPays > COleCurrency(0,0)) {
						pRow->PutValue(m_nNewPaysColIndex, _variant_t(cyNewInsPays));
					}

					// (j.jones 2012-08-17 09:10) - PLID 47778 - if not a batch payment, fill the payment
					// total with anything we may have auto-entered
					if(m_nBatchPaymentID == -1) {
						AutoFillPaymentTotal();
					}
				}
			}

			//if the Adjustment is $0.00 and Allowable is $0.00 make the adjustment be the Ins.Resp. - Ins. Pays.
			//if the Adjustment is $0.00 and Allowable is not $0.00 and the allowable is more than the payment,
			//make the adjustment be the Allowable - Ins. Pays.

			// (j.jones 2007-05-21 10:06) - PLID 23751 - use the cached preference, in this case we're editing
			// so if m_nLineItemPostingAutoAdjust = 1 then go ahead and adjust, otherwise ensure
			// we changed the payment or allowable (the currently edited value) and then adjust

			BOOL bContinue = FALSE;

			// (j.jones 2014-07-01 09:28) - PLID 62552 - vision payments have a checkbox for auto-adjusting, which overrides
			// all the other preferences
			if (m_ePayType == eVisionPayment) {
				if (m_checkAutoAdjustBalances.GetCheck()) {
					bContinue = TRUE;
				}
			}
			else {
				if (m_nLineItemPostingAutoAdjust == 1) {
					//always adjust
					bContinue = TRUE;
				}
				// (j.jones 2013-07-03 17:13) - PLID 57226 - I reworked how option 2 is used here.
				// Option 2 means don't auto-adjust unless you entered a payment and allowable.
				// But this code is hit if you changed the payment or allowable, meaning you could
				// have changed it from a value that previously adjusted, to one that means no adjustment.
				// We should continue anyways in order to potentially reduce the adjustment to zero.
				else if (m_nLineItemPostingAutoAdjust == 2 && (varOldValue.vt != varNewValue.vt || AsString(varOldValue) != AsString(varNewValue))) {
					//only adjust if the value changed
					bContinue = TRUE;
				}

				// (j.jones 2012-08-14 09:54) - PLID 52076 - added sub-preference to only auto-adjust on primary
				if (bContinue && m_nLineItemPostingAutoAdjust_PrimaryOnly == 1 && !m_bIsPrimaryIns) {
					//the preference is on, but the payer is not primary, so do not adjust
					bContinue = FALSE;
				}

				// (j.jones 2012-07-30 16:53) - PLID 47778 - before we would not auto-calculate
				// the adjustment if it was already non-zero, now we will auto-calculate until they
				// manually change the value
				// (j.jones 2014-07-01 09:28) - PLID 62552 - this is only for medical payments/adjustments
				if (VarBool(pRow->GetValue(plcHasEditedAdjustment))) {
					bContinue = FALSE;
				}

				// (j.jones 2012-07-30 17:13) - PLID 47778 - The logic never did anything
				// before if the allowable was zero, I split out into its own if statement
				// for clarity.
				if (cyAllowable <= COleCurrency(0, 0)) {
					bContinue = FALSE;
				}
			}

			if(bContinue) {

				//JMJ - 3/24/2004 - per client request, the adjustment should be
				//the ins. responsibility - the allowable.

				//JMJ - 4/14/2004 - if the allowable was auto-loaded, this calculation should have been already done

				// (j.jones 2007-08-29 09:31) - PLID 27176 - moved adjustment formula into its own function
				// (j.jones 2013-07-03 16:58) - PLID 57226 - if the new pays are null, pass in null, else total payments
				COleCurrency cyNewAdj = COleCurrency(0, 0);
				
				// (j.jones 2014-07-01 09:28) - PLID 62552 - vision payments have a dedicated adjustment function
				if (m_ePayType == eVisionPayment) {
					if (m_checkAutoAdjustBalances.GetCheck()) {
						//this calculates for chargebacks too, it will only not calculate if
						//the payment column is null
						cyNewAdj = CalculateAdjustment_Vision(cyInsResp, cyPatResp,
							pRow->GetValue(m_nNewPaysColIndex).vt == VT_CY ? _variant_t(cyTotalInsPays) : g_cvarNull);
					}
				}
				else {
					cyNewAdj = CalculateAdjustment_Medical(cyInsResp, cyPatResp,
						pRow->GetValue(m_nNewPaysColIndex).vt == VT_CY ? _variant_t(cyTotalInsPays) : g_cvarNull,
						cyAllowable);
				}

				// (j.jones 2012-08-14 09:29) - PLID 52118 - We used to not change the adjustment if it was calculated
				// out to be zero, but now we do want the adjustment to be zero in some cases, such as when the payment is zero.
				// In all cases, we never get to this point if they manually typed in an adjustment.

				pRow->PutValue(m_nAdjustmentColIndex,_variant_t(cyNewAdj));

				// (j.jones 2012-05-08 10:21) - PLID 37165 - if we are tracking a pointer for this adjustment, update it
				ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
				if(pAdjInfo) {
					// (j.jones 2012-05-08 10:26) - PLID 37165 - updates adjustment amounts in tracked pointers
					UpdateAdjustmentPointers(cyNewAdj, pAdjInfo);
				}

				cyInsAdjustments = cyNewAdj;
				AutoFillAdjustmentTotal();
			}
		}
		else if(nCol == m_nAdjustmentColIndex)
		{
			// (j.jones 2012-05-08 10:21) - PLID 37165 - if we are tracking a pointer for this adjustment, update it
			ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
			if(pAdjInfo != NULL && pAdjInfo->aryAdjustmentInfo.GetSize() == 1) {
				pAdjInfo->aryAdjustmentInfo.GetAt(0)->cyAmount = cyInsAdjustments;
			}

			AutoFillAdjustmentTotal();
		}
		// (j.jones 2012-07-30 16:26) - PLID 47778 - deductible and co-insurance can potentially update the payment amount
		// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
		else if(m_nInsuredPartyID != -1 && (nCol == m_nDeductibleAmtColIndex || nCol == m_nCoinsuranceAmtColIndex || nCol == m_nCopayAmtColIndex)) {
			//we will not try to automate the payment value if they have manually edited the payment value

			// (j.jones 2013-08-27 13:36) - PLID 57398 - if they entered a copay, give the dont show me again warning
			if(nCol == m_nCopayAmtColIndex && pRow->GetValue(m_nCopayAmtColIndex).vt == VT_CY) {
				DontShowMeAgain(this, "The Copay field is used when estimating insurance payments or adding billing notes to a charge.\n\n"
					"The Copay field will not create a patient copayment.", "LineItemPostingCopay", "Line Item Posting Copays");
			}

			// (j.jones 2012-08-28 17:10) - PLID 52335 - secondary payments aren't calculated here, instead they get calculated later in this function
			if(m_nLineItemPostingAutoPayment != 0 && !VarBool(pRow->GetValue(plcHasEditedPayment)) && m_nInsuredPartyID != -1 && m_bIsPrimaryIns) {
				// (j.jones 2012-07-30 11:36) - PLID 51863 - added ability to auto-calculate payment amounts by revenue code, for primary insurance only

				//don't calculate payments on zero dollar charges
				if(cyCharges > COleCurrency(0,0)) {
					if(bIsRevCode) {
						// (j.jones 2012-08-28 17:56) - PLID 52335 - the balance should calculate as whatever the current balance is,
						// with the current new payments added back into it
						cyNewInsPays = CalculatePaymentForRevenueCode(TRUE, VarString(pRow->GetValue(plcItemCode), ""), nRevCodeID, cyCharges, cyInsBal + cyNewInsPays,
							cyAllowable, pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
					}
					else {
						// (j.jones 2012-08-28 17:56) - PLID 52335 - the balance should calculate as whatever the current balance is,
						// with the current new payments added back into it
						cyNewInsPays = CalculatePayment(TRUE, VarString(pRow->GetValue(plcItemCode), ""), cyCharges, cyInsBal + cyNewInsPays,
							cyAllowable, pRow->GetValue(plcCoinsurancePercent),
							pRow->GetValue(plcCopayMoney), pRow->GetValue(plcCopayPercent),
							pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
					}
				}

				//update our total
				cyTotalInsPays = cyExistingInsPays + cyNewInsPays;

				//update the row
				// (j.jones 2012-08-14 10:15) - PLID 52117 - A payment can be blank now instead of zero,
				// so if it is already blank and we're calculating zero, leave it blank. If it is non-null,
				// don't clear it out if zero.
				if(pRow->GetValue(m_nNewPaysColIndex).vt == VT_CY || cyNewInsPays > COleCurrency(0,0)) {
					pRow->PutValue(m_nNewPaysColIndex, _variant_t(cyNewInsPays));
				}

				// (j.jones 2012-08-17 09:10) - PLID 47778 - if not a batch payment, fill the payment
				// total with anything we may have auto-entered
				if(m_nBatchPaymentID == -1) {
					AutoFillPaymentTotal();
				}
			}
		}

		// (j.jones 2012-07-30 16:55) - PLID 47778 - if they edited the payment or adjustment, cache that they did so
		// (j.jones 2012-08-28 17:10) - PLID 52335 - moved where this was called so it's after we handle column-specific
		// changes and before we handle global changes
		if(nCol == m_nNewPaysColIndex && AsString(varOldValue) != AsString(varNewValue)) {
			pRow->PutValue(plcHasEditedPayment, g_cvarTrue);
		}
		else if(nCol == m_nAdjustmentColIndex && AsString(varOldValue) != AsString(varNewValue)) {
			pRow->PutValue(plcHasEditedAdjustment, g_cvarTrue);
		}

		// (j.jones 2012-08-28 17:10) - PLID 52335 - added auto-posting for secondary, which has to
		// calculate all the time, at any time the balance is affected, unless of course they were
		// directly editing the payment amount
		if(nCol != m_nNewPaysColIndex
			&& m_nLineItemPostingAutoPayment != 0 && !VarBool(pRow->GetValue(plcHasEditedPayment)) && m_nInsuredPartyID != -1
			&& !m_bIsPrimaryIns && m_nLineItemPostingAutoPayment_Secondary != 0) {
			// (j.jones 2012-07-30 11:36) - PLID 51863 - added ability to auto-calculate payment amounts by revenue code, for primary insurance only

			//don't calculate payments on zero dollar charges
			if(cyCharges > COleCurrency(0,0)) {
				if(bIsRevCode) {
					// (j.jones 2012-08-28 17:56) - PLID 52335 - the balance should calculate as whatever the current balance is,
					// with the current new payments added back into it
					cyNewInsPays = CalculatePaymentForRevenueCode(TRUE, VarString(pRow->GetValue(plcItemCode), ""), nRevCodeID, cyCharges, (cyInsResp - cyTotalInsPays - cyInsAdjustments) + cyNewInsPays,
						cyAllowable, pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
				}
				else {
					// (j.jones 2012-08-28 17:56) - PLID 52335 - the balance should calculate as whatever the current balance is,
					// with the current new payments added back into it
					cyNewInsPays = CalculatePayment(TRUE, VarString(pRow->GetValue(plcItemCode), ""), cyCharges, (cyInsResp - cyTotalInsPays - cyInsAdjustments) + cyNewInsPays,
						cyAllowable, pRow->GetValue(plcCoinsurancePercent),
						pRow->GetValue(plcCopayMoney), pRow->GetValue(plcCopayPercent),
						pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
				}
			}

			//update our total
			cyTotalInsPays = cyExistingInsPays + cyNewInsPays;

			//update the row
			// (j.jones 2012-08-14 10:15) - PLID 52117 - A payment can be blank now instead of zero,
			// so if it is already blank and we're calculating zero, leave it blank. If it is non-null,
			// don't clear it out if zero.
			if(pRow->GetValue(m_nNewPaysColIndex).vt == VT_CY || cyNewInsPays > COleCurrency(0,0)) {
				pRow->PutValue(m_nNewPaysColIndex, _variant_t(cyNewInsPays));
			}

			// (j.jones 2012-08-17 09:10) - PLID 47778 - if not a batch payment, fill the payment
			// total with anything we may have auto-entered
			if(m_nBatchPaymentID == -1) {
				AutoFillPaymentTotal();
			}
		}

		//right now, for all cases, just update the balances
		pRow->PutValue(m_nPatBalanceColIndex, _variant_t(COleCurrency(cyPatResp - cyTotalPatPays)));
		pRow->PutValue(m_nInsBalanceColIndex, _variant_t(COleCurrency(cyInsResp - cyTotalInsPays - cyInsAdjustments)));

		CalculateTotals();

		//for all cases, use this code to choose the next focus point
		if(nCol == ColumnToEdit(FALSE) && bIsTabDown && !bIsShiftDown) {
			//we're on the rightmost editable column, and are tabbing
			if(nRow == m_PostingList->GetRowCount() - 1) {
				//we're on the last row, so tab out
				m_PostingList->StopEditing(TRUE);
				GetDlgItem(IDC_POSTING_INSURANCE_CO_LIST)->SetFocus();
			}
			else {
				short nNewCol = ColumnToEdit(TRUE);
				//start editing the next row
				m_PostingList->StartEditing(nRow+1,nNewCol);
			}
		}
		else if(nCol == ColumnToEdit(TRUE) && bIsTabDown && bIsShiftDown) {
			//we're on the leftmost editable column, and are tabbing backwards
			if(nRow == 0) {
				//we're on the first row, so tab out
				m_PostingList->StopEditing(TRUE);
				GetDlgItem(IDC_ADJ_DESC)->SetFocus();
			}
			else {
				short nNewCol = ColumnToEdit(FALSE);
				//start editing the next row
				m_PostingList->StartEditing(nRow-1,nNewCol);
			}
		}

	}NxCatchAll("Error editing amount.");
}

BOOL CFinancialLineItemPostingDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{

	switch (HIWORD(wParam)) {

		case EN_KILLFOCUS: {

			// (j.jones 2007-02-01 17:38) - PLID 24541 - functionality migrated to
			// the OnFocusGained handler
			/*
			BOOL bIsShiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
			BOOL bIsTabDown = (GetAsyncKeyState(VK_TAB) & 0x80000000) || IsMessageInQueue(NULL, WM_KEYUP, VK_TAB, 0, IMIQ_MATCH_WPARAM);

			int nID;

			switch ((nID = LOWORD(wParam))) {
				case IDC_ADJ_DESC:
					if(bIsTabDown && !bIsShiftDown) {
						//they've tabbed into beginning of the datalist, so start editing
						if(m_PostingList->GetRowCount() > 0) {

							//which column to edit?

							short nCol = ColumnToEdit(TRUE);

							if(m_PostingList->GetCurSel() == -1)
								m_PostingList->StartEditing(0,nCol);
							else
								m_PostingList->StartEditing(m_PostingList->GetCurSel(),nCol);
						}
					}
					break;
			}
			*/
		}
		break;
	}					
	
	return CDialog::OnCommand(wParam, lParam);
}

bool CFinancialLineItemPostingDlg::PostPayments()
{
	try {

		CString str;

		//we don't batch until the end, but we need to give warnings prior to the payment processing

		// (j.jones 2011-07-08 17:13) - PLID 44497 - added pref. to force selection of a provider,
		// which only applies when we are creating new payments, not when applying a batch payment
		if(m_nBatchPaymentID == -1 && GetRemotePropertyInt("RequireProviderOnPayments", 1, 0, "<None>", true) == 1) {

			long nProviderID = -1;
			if(m_ProviderCombo->GetCurSel() != -1) {
				nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(),0));
			}
			if(nProviderID == -1) {
				CString strMessage;
				strMessage = "You have not selected a provider for new payments and adjustments.\n"
						"All new payments and adjustments must have providers selected.";

				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = strMessage;
				}
				else {
					MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
				}
				return false;
			}
		}

		// (j.jones 2010-05-17 16:54) - PLID 16503 - cache the setting for allowing zero dollar payment applies
		BOOL bPostZeroDollarPayments = m_checkApplyZeroDollarPays.GetCheck();

		// (j.jones 2014-07-02 14:57) - PLID 62548 - this is hidden, but always treated as true for vision payments
		if (m_ePayType == eVisionPayment) {
			bPostZeroDollarPayments = TRUE;
		}

		//save this as the last setting they used
		SetRemotePropertyInt("LineItemPosting_PostZeroDollarPayments", bPostZeroDollarPayments ? 1 : 0, 0, GetCurrentUserName());

		// (j.jones 2014-06-30 16:38) - PLID 62642 - save their chargeback category
		if (m_ePayType == eVisionPayment) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_ChargebackCategoryCombo->GetCurSel();
			if (pRow) {
				long nChargebackCategoryID = VarLong(pRow->GetValue(cccID));
				SetRemotePropertyInt("LineItemPosting_DefaultChargebackCategory", nChargebackCategoryID, 0, GetCurrentUserName());
			}
		}

		// (j.jones 2010-06-10 15:30) - PLID 38901 - check their preference for what to do if they overpaid insurance
		// 0 - do nothing, force them to correct the posting
		// 1 - prompt to unapply pat. pays & shift pat. resp
		// 2 - auto-unapply pat. pays & shift pat. resp
		long nLineItemPosting_WhenNewInsPayTooMuch = GetRemotePropertyInt("LineItemPosting_WhenNewInsPayTooMuch", 1, 0, "<None>", true);

		long PrimaryInsID = -1;
		long OtherInsID = -1;
		CString strPrimaryInsName = "";
		CString strOtherInsName = "";
		long nAuditID = -1;

		if(m_BillInsuranceCombo->GetCurSel() != -1) {
			PrimaryInsID = VarLong(m_BillInsuranceCombo->GetValue(m_BillInsuranceCombo->GetCurSel(), ipccID));
			// (j.jones 2009-03-11 08:58) - PLID 32864 - track the name as well, for auditing
			strPrimaryInsName = VarString(m_BillInsuranceCombo->GetValue(m_BillInsuranceCombo->GetCurSel(), ipccName));
		}

		if(m_BillOtherInsuranceCombo->GetCurSel() != -1) {
			OtherInsID = VarLong(m_BillOtherInsuranceCombo->GetValue(m_BillOtherInsuranceCombo->GetCurSel(), ipccID));
			// (j.jones 2009-03-11 08:58) - PLID 32864 - track the name as well, for auditing
			// (j.jones 2009-03-11 08:58) - PLID 32864 - track the name as well, for auditing
			strOtherInsName = VarString(m_BillOtherInsuranceCombo->GetValue(m_BillOtherInsuranceCombo->GetCurSel(), ipccName));
		}

		//batch type
		short nBatch = 0;

		if (((CButton*)GetDlgItem(IDC_RADIO_POST_PAPER_BATCH))->GetCheck()) {
			nBatch = 1; //paper batch
		}
		else if (((CButton*)GetDlgItem(IDC_RADIO_POST_ELECTRONIC_BATCH))->GetCheck())
			nBatch = 2; // electronic batch

		if(nBatch != 0 && PrimaryInsID == -1) {
			CString strMessage = "This claim cannot be batched without an insurance company selected.\n"
				"Please select an 'Insurance Company' on the lower left of this screen.";

			// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
			// silent and just return the reason for failure (I'm pretty sure it should be impossible
			// to get this message when auto-posting)
			if(m_bIsAutoPosting) {
				m_strAutoPostFailure = strMessage;
			}
			else {
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return FALSE;
		}

		//ensure the payment/adjustment amounts entered are valid

		long nInsuredPartyID = -1;
		if(m_PayRespCombo->GetCurSel() != -1)
			nInsuredPartyID = VarLong(m_PayRespCombo->GetValue(m_PayRespCombo->GetCurSel(),0),-1);

		COleCurrency cyTotalPaymentAmountEntered;
		CString strPayment;
		GetDlgItemText(IDC_PAY_TOTAL, strPayment);
		if (strPayment.GetLength() == 0) {
			CString strMessage = "Please fill in the Payment's 'Total Amount' box.";

			// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
			// silent and just return the reason for failure (I'm pretty sure it should be impossible
			// to get this message when auto-posting)
			if(m_bIsAutoPosting) {
				m_strAutoPostFailure = strMessage;
			}
			else {
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return false;
		}

		cyTotalPaymentAmountEntered = ParseCurrencyFromInterface(strPayment);
		if(cyTotalPaymentAmountEntered.GetStatus() == COleCurrency::invalid) {
			CString strMessage = "Please enter a valid amount in the Payment's 'Total Amount' box.";

			// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
			// silent and just return the reason for failure (I'm pretty sure it should be impossible
			// to get this message when auto-posting)
			if(m_bIsAutoPosting) {
				m_strAutoPostFailure = strMessage;
			}
			else {
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return false;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cyTotalPaymentAmountEntered.Format().Find(strDecimal) != -1 && cyTotalPaymentAmountEntered.Format().Find(strDecimal) + (nDigits+1) < cyTotalPaymentAmountEntered.Format().GetLength()) {
			CString strMessage;
			strMessage.Format("Please fill only %li places to the right of the %s in the Payment's 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");

			// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
			// silent and just return the reason for failure (I'm pretty sure it should be impossible
			// to get this message when auto-posting)
			if(m_bIsAutoPosting) {
				m_strAutoPostFailure = strMessage;
			}
			else {
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return false;
		}

		COleCurrency cyTotalAdjustmentAmountEntered;
		CString strAdjustment;
		GetDlgItemText(IDC_ADJ_TOTAL, strAdjustment);
		if (strAdjustment.GetLength() == 0) {
			CString strMessage = "Please fill in the Adjustment's 'Total Amount' box.";

			// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
			// silent and just return the reason for failure (I'm pretty sure it should be impossible
			// to get this message when auto-posting)
			if(m_bIsAutoPosting) {
				m_strAutoPostFailure = strMessage;
			}
			else {
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return false;
		}

		cyTotalAdjustmentAmountEntered = ParseCurrencyFromInterface(strAdjustment);
		if(cyTotalAdjustmentAmountEntered.GetStatus() == COleCurrency::invalid) {
			CString strMessage = "Please enter a valid amount in the Adjustment's 'Total Amount' box.";

			// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
			// silent and just return the reason for failure (I'm pretty sure it should be impossible
			// to get this message when auto-posting)
			if(m_bIsAutoPosting) {
				m_strAutoPostFailure = strMessage;
			}
			else {
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return false;
		}

		//see how much the regional settings allows to the right of the decimal
		if(cyTotalAdjustmentAmountEntered.Format().Find(strDecimal) != -1 && cyTotalAdjustmentAmountEntered.Format().Find(strDecimal) + (nDigits+1) < cyTotalAdjustmentAmountEntered.Format().GetLength()) {
			CString strMessage;
			strMessage.Format("Please fill only %li places to the right of the %s in the Adjustment's 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");

			// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
			// silent and just return the reason for failure (I'm pretty sure it should be impossible
			// to get this message when auto-posting)
			if(m_bIsAutoPosting) {
				m_strAutoPostFailure = strMessage;
			}
			else {
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return false;
		}

		CString strCashReceived;
		COleCurrency cyCashReceived = COleCurrency(0,0);
		if(m_PayMethodCombo->GetCurSel() != -1) {
			long nPayMethod = VarLong(m_PayMethodCombo->GetValue(m_PayMethodCombo->GetCurSel(),0),2);
			if(nPayMethod == 1) {
				//now get the receive amount
				GetDlgItemText(IDC_CASH_RECEIVED_LIP, strCashReceived);

				if (strCashReceived.GetLength() == 0) {			
					CString strMessage = "Please fill in the 'Cash Received' box.";

					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
					// silent and just return the reason for failure (I'm pretty sure it should be impossible
					// to get this message when auto-posting)
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = strMessage;
					}
					else {
						MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
					return false;
				}

				cyCashReceived = ParseCurrencyFromInterface(strCashReceived);
				if(cyCashReceived.GetStatus() == COleCurrency::invalid) {
					SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
					CString strMessage = "Please enter a valid amount in the 'Cash Received' box.";

					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
					// silent and just return the reason for failure (I'm pretty sure it should be impossible
					// to get this message when auto-posting)
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = strMessage;
					}
					else {
						MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
					return false;
				}
				
				if(cyCashReceived.Format().Find(strDecimal) != -1 && cyCashReceived.Format().Find(strDecimal) + (nDigits+1) < cyCashReceived.Format().GetLength()) {
					SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
					CString strMessage;
					strMessage.Format("Please fill only %li places to the right of the %s in the 'Cash Received' box.",nDigits,strDecimal == "," ? "comma" : "decimal");

					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
					// silent and just return the reason for failure (I'm pretty sure it should be impossible
					// to get this message when auto-posting)
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = strMessage;
					}
					else {
						MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
					return false;
				}

				if(cyCashReceived < cyTotalPaymentAmountEntered) {
					//reset it to the payment amount
					SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(cyTotalPaymentAmountEntered,FALSE,TRUE));
					SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
					CString strMessage = "You cannot enter an amount less than the payment amount in the 'Cash Received' box.";

					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
					// silent and just return the reason for failure (I'm pretty sure it should be impossible
					// to get this message when auto-posting)
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = strMessage;
					}
					else {
						MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
					return false;
				}				
			}
		}		

		// (j.jones 2012-08-14 10:58) - PLID 52117 - change the payment amount to a variant
		_variant_t varTotalPaymentAmtNeeded = g_cvarNull;
		COleCurrency cyTotalAdjustmentAmtNeeded = COleCurrency(0,0);

		// (j.jones 2015-10-26 10:47) - PLID 67451 - capitation allows overpayments,
		// the balance of which will be adjusted off		
		bool bIsCapitationOverpayment = false;
		COleCurrency cyCapitationOverpaymentAmtToApply = COleCurrency(0, 0);
		COleCurrency cyCapitationOverpaymentAmtToAdjust = COleCurrency(0, 0);

		// (j.jones 2016-05-09 17:00) - NX-100502 - we now allow overpayments & overadjustments,
		// track them as such so we don't warn more than once
		bool bIsRegularOverpayment = false;
		bool bIsRegularOveradjustment = false;
		
		// (j.jones 2014-07-10 16:28) - PLID 62553 - track if a chargeback exists,
		// so that we will allow posting even if we aren't making new payments
		bool bHasChargebacks = false;

		//save all the data, create payments as needed, create all the applies, shift responsibilities, etc...

		//Step 1. Double-check to make sure the posting is balanced.

		for(int i=0;i<m_PostingList->GetRowCount();i++) {
			//loop through and get totals, and test that the posting is balanced
			long nChargeID, nRevCodeID;
			COleCurrency cyCharges, cyPatResp, cyInsResp;
			COleCurrency cyOldExistingPatPays, cyExistingPatPays;
			COleCurrency cyOldExistingInsPays, cyExistingInsPays;
			COleCurrency cyInsAdjustments;
			IRowSettingsPtr pRow = m_PostingList->GetRow(i);
			nChargeID = VarLong(pRow->GetValue(plcChargeID), -1);
			nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID), -1);
			cyCharges = VarCurrency(pRow->GetValue(m_nChargesColIndex),COleCurrency(0,0));
			cyPatResp = VarCurrency(pRow->GetValue(m_nPatRespColIndex),COleCurrency(0,0));
			cyInsResp = VarCurrency(pRow->GetValue(m_nInsRespColIndex),COleCurrency(0,0));

			// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new payments,
			// plus we cache the initial existing total in an "old" column
			cyOldExistingPatPays = VarCurrency(pRow->GetValue(plcOldExistingPatPays),COleCurrency(0,0));
			cyExistingPatPays = VarCurrency(pRow->GetValue(m_nExistingPatPaysColIndex),COleCurrency(0,0));
			cyOldExistingInsPays = VarCurrency(pRow->GetValue(plcOldExistingInsPays),COleCurrency(0,0));
			cyExistingInsPays = VarCurrency(pRow->GetValue(m_nExistingInsPaysColIndex),COleCurrency(0,0));

			// (j.jones 2010-05-05 09:49) - PLID 25521 - New Pays is just one column now
			// (j.jones 2012-08-14 10:58) - PLID 52117 - payments can now be null
			_variant_t varNewPays = pRow->GetValue(m_nNewPaysColIndex);

			cyInsAdjustments = VarCurrency(pRow->GetValue(m_nAdjustmentColIndex),COleCurrency(0,0));

			COleCurrency cyTotalPatPays = cyExistingPatPays;
			COleCurrency cyTotalInsPays = cyExistingInsPays;

			// (j.jones 2014-07-01 11:26) - PLID 62553 - the total payments should ignore negative amounts
			COleCurrency cyNewPays = VarCurrency(varNewPays, COleCurrency(0, 0));
			if (cyNewPays > COleCurrency(0, 0)) {
				if (m_nInsuredPartyID == -1) {
					cyTotalPatPays += cyNewPays;
				}
				else {
					cyTotalInsPays += cyNewPays;
				}
			}

			// (j.jones 2007-01-04 09:28) - PLID 24030 - confirm we are posting to a revenue code
			BOOL bIsRevCode = IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && nRevCodeID != -1;

			if(bIsRevCode && nInsuredPartyID == -1) {
				//it should be impossible to get this, but if they do, leave gracefully
				ASSERT(FALSE);
				CString strMessage = "You are configured to post to a revenue code, but you are posting a patient payment.\n"
					"This is not supported. You can only post insurance payments to revenue codes.";

				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure (I'm pretty sure it should be impossible
				// to get this message when auto-posting)
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = strMessage;
				}
				else {
					MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
				}
				return false;
			}

			//test balancing

			//TODO: In most of these cases we will throw an error if the payment total is more than the responsibility total.
			//Consider adding a prompt to auto-increase the reponsibility to match the total needed (if possible),
			//rather than making the user manually fix the amounts.

			if(cyCharges < cyPatResp) {
				//we're trying to make the patient responsibility greater than the charge amount
				str.Format("The patient has a %s %s with %s patient responsibility.\n"
					"Please correct this before posting.",FormatCurrencyForInterface(cyCharges,TRUE,TRUE),
					bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyPatResp,TRUE,TRUE));

				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = str;
				}
				else {
					MessageBox(str, "Practice", MB_ICONEXCLAMATION|MB_OK);
				}
				return false;
			}
			if(cyCharges < cyInsResp) {
				//we're trying to make the insurance responsibility greater than the charge amount
				str.Format("The patient has a %s %s with %s insurance responsibility.\n"
					"Please correct this before posting.",FormatCurrencyForInterface(cyCharges,TRUE,TRUE),
					bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyInsResp,TRUE,TRUE));

				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = str;
				}
				else {
					MessageBox(str, "Practice", MB_ICONEXCLAMATION|MB_OK);
				}
				return false;
			}
			if(cyPatResp < cyTotalPatPays && !bIsRegularOverpayment) {
				//we're trying to put in more patient payments than there is responsibility
				// (j.jones 2016-05-10 14:44) - NX-100502 - we now allow overpayments
				if (m_bIsAutoPosting) {
					m_strAutoPostFailure = "At least one charge has an overpayment that needs to be manually reviewed.";
				}
				else {
					str.Format("The patient has a %s %s with %s patient responsibility, but has %s in patient payments applied to it.\n\n"
						"The overage will remain unapplied on the patient's account.\n\n"
						"Are you sure you wish to continue?", FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
						bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyPatResp, TRUE, TRUE),
						FormatCurrencyForInterface(cyTotalPatPays, TRUE, TRUE));

					// (j.jones 2012-05-02 15:39) - PLID 48367 - if there are patient payments applied already,
					// mention that they can unapply patient payments in this screen
					if (cyExistingPatPays > COleCurrency(0, 0)) {
						str += "\n\nYou may unapply existing patient credits by editing the 'Prev Pat Pays' column, if desired.";
					}

					// (j.jones 2016-05-09 15:55) - NX-100502 - overpayments are now allowed
					if (IDNO == DontShowMeAgain(this, str, "LineItemPosting_PostPatientOverpayment", "Practice", FALSE, TRUE)) {
						return false;
					}
				}
			
				//track that they approved this in order to ignore future warnings
				bIsRegularOverpayment = true;
			}
			if(cyInsResp < cyTotalInsPays) {
				//we're trying to put in more insurance payments than there is responsibility

				// (j.jones 2015-10-26 10:28) - PLID 67451 - capitation allows this, the overpayment
				// will be adjusted off
				if (m_bIsCapitation && m_nInsuredPartyID != -1) {
					
					str.Format("The patient has a %s charge with %s insurance responsibility, but has %s in insurance payments applied to it.\n",
						FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
						FormatCurrencyForInterface(cyInsResp, TRUE, TRUE),
						FormatCurrencyForInterface(cyTotalInsPays, TRUE, TRUE));

					//even when auto-posting, this is a yes/no option
					if (IDNO == MessageBox(str + "If you continue, the balance of this payment will be posted to the patient's account and adjusted off.\n\n"
						"Are you sure you wish to post this overpayment?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
						//return the same warning
						if (m_bIsAutoPosting) {
							m_strAutoPostFailure = str + "Please correct this before posting.";
						}
						return false;
					}

					//track that this is an overpayment
					if (varNewPays.vt == VT_CY) {
						bIsCapitationOverpayment = true;

						COleCurrency cyDiff = cyTotalInsPays - cyInsResp;

						//reduce the amt. to apply to be the insurance balance
						cyCapitationOverpaymentAmtToApply = VarCurrency(varNewPays) - cyDiff;
						
						if(cyCapitationOverpaymentAmtToApply < COleCurrency(0,0)) {
							//how is this possible?
							ASSERT(FALSE);
							cyCapitationOverpaymentAmtToApply = COleCurrency(0, 0);
						}

						//set the amt. to adjust to be the overage
						cyCapitationOverpaymentAmtToAdjust = cyDiff;
					}
				}
				// (j.jones 2010-06-10 15:30) - PLID 38901 - check their preference for what to do if they overpaid insurance
				// 0 - do nothing, force them to correct the posting
				// 1 - prompt to unapply pat. pays & shift pat. resp
				// 2 - auto-unapply pat. pays & shift pat. resp
				else if(m_nInsuredPartyID != -1 && cyPatResp > COleCurrency(0,0)
					&& (nLineItemPosting_WhenNewInsPayTooMuch == 1 || nLineItemPosting_WhenNewInsPayTooMuch == 2)) {
					//can we possibly shift enough from patient resp?

					bool bShiftFromPatient = true;

					if((cyPatResp + cyInsResp) < cyTotalInsPays) {
						//nope
						str.Format("The patient has a %s %s with %s insurance responsibility, but has %s in insurance payments applied to it. "
							"There is not enough patient responsibility on this charge to cover the overage.\n\n"
							"Would you like to shift the balance from patient responsibility? (This will unapply patient payments if necessary.)",
							FormatCurrencyForInterface(cyCharges,TRUE,TRUE),
							bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyInsResp,TRUE,TRUE),
							FormatCurrencyForInterface(cyTotalInsPays,TRUE,TRUE));

						if (IDNO == MessageBox(str, "Practice", MB_ICONQUESTION | MB_YESNO)) {

							bShiftFromPatient = false;

							// (j.jones 2016-05-10 14:54) - NX-100502 - add a second warning, see if they want to keep
							// posting as an overpayment
							if (IDNO == MessageBox("Do you wish to continue posting this overpayment? "
								"The overage will remain unapplied on the patient's account.",
								"Practice", MB_ICONQUESTION | MB_YESNO)) {

								if (m_bIsAutoPosting) {
									//give a simpler warning for the caller
									m_strAutoPostFailure = "At least one charge has an overpayment that needs to be manually reviewed.";
								}
								return false;
							}
						}

						//track that they approved this in order to ignore future warnings
						bIsRegularOverpayment = true;
						m_strAutoPostFailure = "At least one charge has an overpayment that needs to be manually reviewed.";
					}
					else {
						//ok, we CAN shift enough, do they want to be warned first?
						if(nLineItemPosting_WhenNewInsPayTooMuch == 1) {
							str.Format("The patient has a %s %s with %s insurance responsibility, but has %s in insurance payments applied to it.\n\n"
								"Would you like to shift the balance from patient responsibility? (This will unapply patient payments if necessary.)",
								FormatCurrencyForInterface(cyCharges,TRUE,TRUE),
								bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyInsResp,TRUE,TRUE),
								FormatCurrencyForInterface(cyTotalInsPays,TRUE,TRUE));

							// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, but if so
							// go ahead and give this message							
							if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
								bShiftFromPatient = false;

								// (j.jones 2016-05-10 14:54) - NX-100502 - add a second warning, see if they want to keep
								// posting as an overpayment
								if (IDNO == MessageBox("Do you wish to continue posting this overpayment? "
									"The overage will remain unapplied on the patient's account.",
									"Practice", MB_ICONQUESTION | MB_YESNO)) {

									if (m_bIsAutoPosting) {
										//give a simpler warning for the caller
										m_strAutoPostFailure = "At least one charge has an overpayment that needs to be manually reviewed.";
									}
									return false;
								}

								//track that they approved this in order to ignore future warnings
								bIsRegularOverpayment = true;
								m_strAutoPostFailure = "At least one charge has an overpayment that needs to be manually reviewed.";
							}
						}
					}

					if (bShiftFromPatient) {
						//take away patient resp., unapplying as needed

						//how much do we need?
						COleCurrency cyAmtNeeded = cyTotalInsPays - cyInsResp;

						//how much is pat. resp. is available, without unapplying?
						COleCurrency cyPatBalance = cyPatResp - cyExistingPatPays;
						if (cyPatBalance >= cyAmtNeeded || cyPatBalance == cyPatResp) {
							//fantastic, we have enough, so shift now
							COleCurrency cyAmtToShift = cyAmtNeeded;
							if (cyAmtToShift > cyPatBalance) {
								cyAmtToShift = cyPatBalance;
							}
							cyInsResp += cyAmtToShift;
							cyPatResp -= cyAmtToShift;
							pRow->PutValue(m_nPatRespColIndex, _variant_t(cyPatResp));
							pRow->PutValue(m_nInsRespColIndex, _variant_t(cyInsResp));
							pRow->PutValue(m_nPatBalanceColIndex, _variant_t(COleCurrency(cyPatResp - cyTotalPatPays)));
							pRow->PutValue(m_nInsBalanceColIndex, _variant_t(COleCurrency(cyInsResp - cyTotalInsPays - cyInsAdjustments)));
							CalculateTotals();
						}
						else {
							//not enough, gotta unapply
							COleCurrency cyAmtToUnapply = cyAmtNeeded - cyPatBalance;

							if (cyAmtToUnapply > (cyPatResp - cyPatBalance)) {
								cyAmtToUnapply = (cyPatResp - cyPatBalance);
							}

							//update the tracked unapplied amount
							m_cyUnappliedPatAmt += cyAmtToUnapply;
							m_nxstaticPatAmtToUnapply.SetWindowText(FormatCurrencyForInterface(m_cyUnappliedPatAmt));
							if (m_cyUnappliedPatAmt == COleCurrency(0, 0)) {
								m_nxstaticPatAmtToUnapplyLabel.ShowWindow(SW_HIDE);
								m_nxstaticPatAmtToUnapply.ShowWindow(SW_HIDE);
							}
							else {
								m_nxstaticPatAmtToUnapplyLabel.ShowWindow(SW_SHOW);
								m_nxstaticPatAmtToUnapply.ShowWindow(SW_SHOW);
							}

							//now actually do it
							cyExistingPatPays -= cyAmtToUnapply;
							cyTotalPatPays -= cyAmtToUnapply;

							COleCurrency cyAmtToShift = cyAmtNeeded;
							if (cyAmtToShift > cyPatResp) {
								cyAmtToShift = cyPatResp;
							}
							cyInsResp += cyAmtToShift;
							cyPatResp -= cyAmtToShift;
							pRow->PutValue(m_nExistingPatPaysColIndex, _variant_t(cyExistingPatPays));
							pRow->PutValue(m_nPatRespColIndex, _variant_t(cyPatResp));
							pRow->PutValue(m_nInsRespColIndex, _variant_t(cyInsResp));
							pRow->PutValue(m_nPatBalanceColIndex, _variant_t(COleCurrency(cyPatResp - cyTotalPatPays)));
							pRow->PutValue(m_nInsBalanceColIndex, _variant_t(COleCurrency(cyInsResp - cyTotalInsPays - cyInsAdjustments)));
							CalculateTotals();
						}
					}
				}
				else if(!bIsRegularOverpayment) {

					// (j.jones 2016-05-10 14:44) - NX-100502 - we now allow overpayments
					if (m_bIsAutoPosting) {
						m_strAutoPostFailure = "At least one charge has an overpayment that needs to be manually reviewed.";
					}
					else {
						str.Format("The patient has a %s %s with %s insurance responsibility, but has %s in insurance payments applied to it.\n\n"
							"The overage will remain unapplied on the patient's account.\n\n"
							"Are you sure you wish to continue?", FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
							bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyInsResp, TRUE, TRUE),
							FormatCurrencyForInterface(cyTotalInsPays, TRUE, TRUE));

						// (j.jones 2016-05-09 15:55) - NX-100502 - overpayments are now allowed
						if (IDNO == DontShowMeAgain(this, str, "LineItemPosting_PostInsOverpayment", "Practice", FALSE, TRUE)) {
							return false;
						}
					}

					//track that they approved this in order to ignore future warnings
					bIsRegularOverpayment = true;
				}
			}
			if(cyInsResp < cyInsAdjustments) {
				//we're trying to put in more insurance adjustments than there is responsibility
				
				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				// (j.jones 2016-05-09 16:32) - NX-100502 - just let auto-posting continue, remind them to review it
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = "At least one charge has an overadjustment that needs to be manually reviewed.";
				}
				// (j.jones 2016-05-09 16:30) - NX-100502 - we now allow overadjustments, don't prompt
				// if they already approved it
				else if(!bIsRegularOveradjustment) {
					str.Format("The patient has a %s %s with %s insurance responsibility, but has %s in insurance adjustments applied to it.\n\n"
						"The overage will remain unapplied on the patient's account.\n\n"
						"Are you sure you wish to continue?",
						FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
						bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyInsResp, TRUE, TRUE),
						FormatCurrencyForInterface(cyInsAdjustments, TRUE, TRUE));
					if (IDNO == DontShowMeAgain(this, str, "LineItemPosting_PostInsOveradjust", "Practice", FALSE, TRUE)) {
						return false;
					}
				}	

				//track that they approved this in order to ignore future warnings
				bIsRegularOveradjustment = true;
			}
			// (j.jones 2015-10-26 10:28) - PLID 67451 - skip this validation if capitation
			if(cyInsResp < (cyTotalInsPays + cyInsAdjustments) && !m_bIsCapitation) {
				//we're trying to put in more insurance credits than there is responsibility

				// (j.jones 2016-05-09 16:32) - NX-100502 - just let auto-posting continue, remind them to review it
				if (m_bIsAutoPosting) {
					m_strAutoPostFailure = "At least one charge has an overpayment/overadjustment that needs to be manually reviewed.";
				}
				// (j.jones 2016-05-09 16:30) - NX-100502 - we now allow overadjustments/adjustments, don't prompt
				// if they already approved any overpayments/adjustments
				else if (!bIsRegularOveradjustment && !bIsRegularOverpayment) {
					// (j.jones 2016-05-09 16:30) - NX-100502 - we now allow overpayments/adjustments
					str.Format("The patient has a %s %s with %s insurance responsibility,\n"
						"but has %s in insurance payments and %s in insurance adjustments applied to it.",
						FormatCurrencyForInterface(cyCharges, TRUE, TRUE),
						bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyInsResp, TRUE, TRUE),
						FormatCurrencyForInterface(cyTotalInsPays, TRUE, TRUE), FormatCurrencyForInterface(cyInsAdjustments, TRUE, TRUE));

					// (j.jones 2012-05-02 15:39) - PLID 48367 - if there are insurance payments applied already,
					// mention that they can unapply insurance payments in this screen
					if (cyExistingInsPays > COleCurrency(0, 0)) {
						str += "\n\nYou may unapply existing insurance credits by editing the 'Prev Ins Pays' column, if desired.";
					}

					// (j.jones 2016-05-09 16:45) - NX-100502 - we now allow overadjustments
					str += "\n\nThe overage will remain unapplied on the patient's account.\n\n"
						"Are you sure you wish to continue?";

					if (IDNO == DontShowMeAgain(this, str, "LineItemPosting_PostTotalOverage", "Practice", FALSE, TRUE)) {
						return false;
					}
				}

				//track that they approved this in order to ignore future warnings
				bIsRegularOverpayment = true;
				bIsRegularOveradjustment = true;
			}
			if((cyPatResp + cyInsResp) > cyCharges) {
				//they're trying to have responsibilities higher than the charge amount
				str.Format("The patient has a %s %s with %s patient responsibility and %s insurance responsibility, which is more than the charge amount.\n"
					"Please correct this before posting.",FormatCurrencyForInterface(cyCharges,TRUE,TRUE),
					bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyPatResp,TRUE,TRUE),
					FormatCurrencyForInterface(cyInsResp,TRUE,TRUE));
				
				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = str;
				}
				else {
					MessageBox(str, "Practice", MB_ICONEXCLAMATION|MB_OK);
				}
				return false;
			}

			if(m_nInsuredPartyID != -1) {

				//include any other responsibilities not listed on this screen, and test
				COleCurrency cyTotalOtherResp = COleCurrency(0,0);
				_RecordsetPtr rs = CreateRecordset("SELECT PersonID FROM InsuredPartyT WHERE PatientID = %li AND PersonID <> %li",m_nPatientID,m_nInsuredPartyID);
				while(!rs->eof) {
					COleCurrency cyInsResp = COleCurrency(0,0);
					if(bIsRevCode) {
						GetRevenueCodeInsuranceTotals(nRevCodeID, m_nBillID, m_nPatientID, AdoFldLong(rs, "PersonID"), &cyInsResp, 0, 0, 0);
					}
					else {
						cyInsResp = GetChargeInsResp(nChargeID, AdoFldLong(rs, "PersonID"));
					}
					cyTotalOtherResp += cyInsResp;
					rs->MoveNext();
				}
				rs->Close();

				if((cyPatResp + cyInsResp + cyTotalOtherResp) > cyCharges) {
					//they're trying to have total responsibilities higher than the charge amount
					str.Format("The patient has a %s %s with %s patient responsibility, %s insurance responsibility, and %s in outside insurance responsibility.\n"
						"This total is more than the charge amount. Please correct this before posting.",FormatCurrencyForInterface(cyCharges,TRUE,TRUE),
						bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyPatResp,TRUE,TRUE), 
						FormatCurrencyForInterface(cyInsResp,TRUE,TRUE), FormatCurrencyForInterface(cyTotalOtherResp,TRUE,TRUE));
					
					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
					// silent and just return the reason for failure
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = str;
					}
					else {
						MessageBox(str, "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
					return false;
				}

				//don't let them make both responsibilities less than the total charge
				COleCurrency cyExistingPatResp = COleCurrency(0,0);
				{
					if(bIsRevCode) {
						COleCurrency cyCharges = COleCurrency(0,0), cyInsResp = COleCurrency(0,0);
						GetRevenueCodeTotals(nRevCodeID, m_nBillID, m_nPatientID, &cyCharges, 0, 0, 0, &cyInsResp);
						cyExistingPatResp = cyCharges - cyInsResp;
					}
					else {
						cyExistingPatResp = GetChargePatientResp(nChargeID);
					}
				}
				
				COleCurrency cyExistingInsResp = COleCurrency(0,0);
				if(bIsRevCode) {
					GetRevenueCodeInsuranceTotals(nRevCodeID, m_nBillID, m_nPatientID, m_nInsuredPartyID, &cyExistingInsResp, 0, 0, 0);
				}
				else {
					cyExistingInsResp = GetChargeInsResp(nChargeID, m_nInsuredPartyID);
				}

				if(cyPatResp < cyExistingPatResp && cyInsResp < cyExistingInsResp) {
					//they've reduced both responsibilities, which is impossible
					str.Format("The patient has a %s %s with %s patient responsibility and %s insurance responsibility.\n"
						"Both responsibilities have been reduced without increasing the other. Please correct this before posting.",FormatCurrencyForInterface(cyCharges,TRUE,TRUE),
						bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyPatResp,TRUE,TRUE),
						FormatCurrencyForInterface(cyInsResp,TRUE,TRUE));
					
					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
					// silent and just return the reason for failure
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = str;
					}
					else {
						MessageBox(str, "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
					return false;
				}

				if((cyPatResp + cyInsResp + cyTotalOtherResp) < cyCharges) {
					//they're trying to have total responsibilities lower than the charge amount
					str.Format("The patient has a %s %s with %s patient responsibility, %s insurance responsibility, and %s in outside insurance responsibility.\n"
						"This total is less than the %s amount. Please correct this before posting.",FormatCurrencyForInterface(cyCharges,TRUE,TRUE),
						bIsRevCode ? "revenue code" : "charge", FormatCurrencyForInterface(cyPatResp,TRUE,TRUE),
						FormatCurrencyForInterface(cyInsResp,TRUE,TRUE), FormatCurrencyForInterface(cyTotalOtherResp,TRUE,TRUE),
						bIsRevCode ? "revenue code" : "charge");
					
					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
					// silent and just return the reason for failure
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = str;
					}
					else {
						MessageBox(str, "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
					return false;
				}
			}
			
			//TODO: think of more possible ways to validate the posting

			//now increase the payment amount needed

			// (j.jones 2012-08-14 10:58) - PLID 52117 - handle null payments			
			if (varNewPays.vt == VT_CY) {
				// (j.jones 2014-07-10 16:11) - PLID 62553 - If a vision chargeback, don't
				// let the negative payment row turn the total payment amount non-null.
				if (m_ePayType == eVisionPayment && VarCurrency(varNewPays) < COleCurrency(0, 0)) {
					// Track if a chargeback exists, so that we will allow posting
					// even if we aren't making new payments.
					bHasChargebacks = true;
				}
				else {
					//if the total payment is null, set it to zero
					if (varTotalPaymentAmtNeeded.vt == VT_NULL) {
						varTotalPaymentAmtNeeded = _variant_t(COleCurrency(0, 0));
					}
					COleCurrency cyTotalPaymentAmtNeeded = VarCurrency(varTotalPaymentAmtNeeded);

					// (j.jones 2014-07-01 11:26) - PLID 62553 - the total payments should ignore negative amounts
					COleCurrency cyNewPays = VarCurrency(varNewPays, COleCurrency(0, 0));
					if (cyNewPays > COleCurrency(0, 0)) {
						cyTotalPaymentAmtNeeded += cyNewPays;
					}
					varTotalPaymentAmtNeeded = _variant_t(cyTotalPaymentAmtNeeded);
				}
			}
			cyTotalAdjustmentAmtNeeded += cyInsAdjustments;
		}

		//see if the Total Pay/Adj needed is greater than the amount entered

		//payment
		if(cyTotalPaymentAmountEntered < VarCurrency(varTotalPaymentAmtNeeded, COleCurrency(0,0))) {

			if (m_nBatchPaymentID == -1) {
				//not a batch payment

				str.Format("The Payment's 'Total Amount' is %s but you are attempting to apply %s in payments.\n"
					"Please correct this before posting.", FormatCurrencyForInterface(cyTotalPaymentAmountEntered, TRUE, TRUE), FormatCurrencyForInterface(VarCurrency(varTotalPaymentAmtNeeded, COleCurrency(0, 0)), TRUE, TRUE));

				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if (m_bIsAutoPosting) {
					m_strAutoPostFailure = str;
				}
				else {
					MessageBox(str, "Practice", MB_ICONEXCLAMATION | MB_OK);
				}
				return FALSE;
			}
			else {
				//batch payment

				// (j.jones 2014-07-28 08:20) - PLID 63065 - Changed such that on batch payments this is
				// just a warning, not an outright failure because we do allow posting such that you can
				// have a negative balance. Also I changed the logic to not even bother warning if the
				// balance is already negative.
				if (cyTotalPaymentAmountEntered >= COleCurrency(0, 0)) {
					str.Format("The Payment's 'Total Amount' is %s but you are attempting to apply %s in payments.\n"
						"Are you sure you wish to post this amount?", FormatCurrencyForInterface(cyTotalPaymentAmountEntered, TRUE, TRUE), FormatCurrencyForInterface(VarCurrency(varTotalPaymentAmtNeeded, COleCurrency(0, 0)), TRUE, TRUE));

					if (IDNO == MessageBox(str, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
						if (m_bIsAutoPosting) {
							m_strAutoPostFailure = "This payment would leave a negative balance on the batch payment.";
						}
						return false;
					}
				}
			}
		}
		
		//do not worry if the payment is larger and it's a batch payment
		if(cyTotalPaymentAmountEntered > VarCurrency(varTotalPaymentAmtNeeded, COleCurrency(0,0)) && m_nBatchPaymentID == -1) {
			str.Format("The Payment's 'Total Amount' is %s but you are only applying %s in payments.\n"
				"Do you still wish to only apply this amount? (The remainder will be a balance on the patient's account.)",FormatCurrencyForInterface(cyTotalPaymentAmountEntered,TRUE,TRUE),FormatCurrencyForInterface(VarCurrency(varTotalPaymentAmtNeeded, COleCurrency(0,0)),TRUE,TRUE));
			int nResult = MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
			if(nResult == IDCANCEL) {
				// (j.jones 2012-08-16 12:25) - PLID 52162 - it's not currently possible to auto-post a non-batch payment,
				// but if we were, we need to return a failure reason
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = "This payment would leave a balance on the patient's account. Please correct the payment amount before posting.";
				}
				return FALSE;
			}
			else if(nResult == IDNO) {
				// (j.jones 2012-08-16 12:25) - PLID 52162 - it's not currently possible to auto-post a non-batch payment,
				// but if we were, we need to return a failure reason
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = "This payment would leave a balance on the patient's account. Please correct the payment amount before posting.";
				}
				else {
					AfxMessageBox("Please correct the payment amount before posting.");
				}
				return false;
			}
			else {
				//do nothing now, continue normally
			}
		}

		//adjustment
		if(cyTotalAdjustmentAmountEntered < cyTotalAdjustmentAmtNeeded) {
			//this is different, rather than stopping them, we can just prompt them
			str.Format("The Adjustment's 'Total Amount' is %s but you are attempting to apply %s in adjustments.\n"
				"Would you like to automatically increase the total adjustment amount to match the apply amount?",FormatCurrencyForInterface(cyTotalAdjustmentAmountEntered,TRUE,TRUE),FormatCurrencyForInterface(cyTotalAdjustmentAmtNeeded,TRUE,TRUE));
			int nResult = MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
			if(nResult == IDCANCEL) {
				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = "Please correct the total adjustment amount before posting.";
				}
				return false;
			}
			else if(nResult == IDNO) {
				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = "Please correct the total adjustment amount before posting.";
				}
				else {
					AfxMessageBox("Please correct the adjustment amount before posting.");
				}
				return false;
			}
			else {
				//increase the amount
				cyTotalAdjustmentAmountEntered = cyTotalAdjustmentAmtNeeded;
				SetDlgItemText(IDC_ADJ_TOTAL,FormatCurrencyForInterface(cyTotalAdjustmentAmountEntered,FALSE,TRUE));
			}
		}
		else if(cyTotalAdjustmentAmountEntered > cyTotalAdjustmentAmtNeeded) {
			//again, rather than stopping them, we can just prompt them
			str.Format("The Adjustment's 'Total Amount' is %s but you are only applying %s in adjustments.\n"
				"Would you like to reduce the total adjustment amount to match the apply amount?",FormatCurrencyForInterface(cyTotalAdjustmentAmountEntered,TRUE,TRUE),FormatCurrencyForInterface(cyTotalAdjustmentAmtNeeded,TRUE,TRUE));
			int nResult = MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
			if(nResult == IDCANCEL) {
				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = "Please correct the total adjustment amount before posting.";
				}
				return false;
			}
			else if(nResult == IDNO) {
				// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
				// silent and just return the reason for failure
				if(m_bIsAutoPosting) {
					m_strAutoPostFailure = "Please correct the total adjustment amount before posting.";
				}
				else {
					AfxMessageBox("Please correct the adjustment amount before posting.");
				}
				return false;
			}
			else {
				//reduce the amount
				cyTotalAdjustmentAmountEntered = cyTotalAdjustmentAmtNeeded;
				SetDlgItemText(IDC_ADJ_TOTAL,FormatCurrencyForInterface(cyTotalAdjustmentAmountEntered,FALSE,TRUE));
			}
		}

		// (j.jones 2010-05-17 16:52) - PLID 16503 - this check is not necessary if they do want $0.00 payments
		// (j.jones 2012-08-14 11:45) - PLID 52117 - handle null payments
		// (j.jones 2014-07-10 16:30) - PLID 62553 - we will not give this warning if they are only posting chargebacks
		if((varTotalPaymentAmtNeeded.vt == VT_NULL || (!bPostZeroDollarPayments && VarCurrency(varTotalPaymentAmtNeeded, COleCurrency(0,0)) == COleCurrency(0,0)))
			&& cyTotalAdjustmentAmtNeeded == COleCurrency(0, 0) && !bHasChargebacks) {
			//hang on a minute, maybe they just clicked "Post" too soon?
			CString strMessage = "You must enter in a payment or adjustment amount to apply to this patient.";

			// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, if so we need to stay
			// silent and just return the reason for failure
			if(m_bIsAutoPosting) {
				m_strAutoPostFailure = strMessage;
			}
			else {
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
			}
			return false;
		}

		COleDateTime dtPayment = VarDateTime(m_dtPayDate.GetValue());

		// (c.haag 2009-05-27 10:47) - PLID 34273 - Fail if the user is trying to backdate the payment or adjustment.
		// Note how we check to see whether we're going to have a payment or adjustment by mirroring the branching
		// after the point of no return.
		// (j.jones 2010-05-17 16:53) - PLID 16503 - if we allow zero dollar payments, we will be posting
		// the payment, so validate the date
		// (j.jones 2012-08-14 11:45) - PLID 52117 - handle null payments
		// (j.jones 2014-07-10 16:30) - PLID 62553 - this permission is also checked if they are posting chargebacks
		if((varTotalPaymentAmtNeeded.vt == VT_CY && (bPostZeroDollarPayments || VarCurrency(varTotalPaymentAmtNeeded) > COleCurrency(0,0)))
			|| (m_nBatchPaymentID == -1 && cyTotalPaymentAmountEntered > COleCurrency(0,0)
			|| bHasChargebacks))
		{
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			dtPayment.SetDate(dtPayment.GetYear(), dtPayment.GetMonth(), dtPayment.GetDay());
			dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
			if (dtPayment < dtToday) {
				if (!CanChangeHistoricFinancial_ByServiceDate("Payment", dtPayment, TRUE)) {
					// The user cannot save the item as is, and they've already been prompted about it.

					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, we would have already warned,
					// but the caller expects a failure message
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = "A prior payment could not be modified.";
					}
					return false;
				} else {
					// The user is allowed to save this item with this date
				}
			} else {
				// The payment is today or in the future
			}
		}
		if(cyTotalAdjustmentAmtNeeded > COleCurrency(0,0)) 
		{
			COleDateTime dtAdjustment = VarDateTime(m_dtAdjDate.GetValue());
			// (j.jones 2014-07-01 09:16) - PLID 62549 - if a vision payment, the adjustment date is
			// always the same date as the payment
			if (m_ePayType == eVisionPayment) {
				dtAdjustment = VarDateTime(m_dtPayDate.GetValue());
			}

			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			dtAdjustment.SetDate(dtAdjustment.GetYear(), dtAdjustment.GetMonth(), dtAdjustment.GetDay());
			dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
			if (dtAdjustment < dtToday) {
				if (!CanChangeHistoricFinancial_ByServiceDate("Adjustment", dtAdjustment, TRUE)) {
					// The user cannot save the item as is, and they've already been prompted about it.

					// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, we would have already warned,
					// but the caller expects a failure message
					if(m_bIsAutoPosting) {
						m_strAutoPostFailure = "A prior adjustment could not be modified.";
					}
					return false;
				} else {
					// The user is allowed to save this item with this date
				}
			} else {
				// The adjustment is today or in the future
			}
		}

		// (j.jones 2013-03-15 16:04) - PLID 49444 - warn if we are going to make a $0.00 payment
		if(m_nBatchPaymentID == -1 && cyTotalPaymentAmountEntered == COleCurrency(0,0) && bPostZeroDollarPayments) {
			//If auto-posting, this really should not have been called with $0.00,
			//and we'll probably never get here because auto-posting is only enabled for
			//batch payments. But if we do get here, don't warn. We assume they really do want this.
			if(!m_bIsAutoPosting) {
				CString strWarn;
				strWarn.Format("Posting this payment will create and apply a %s payment.\n\n"
					"Are you sure you wish to continue?", FormatCurrencyForInterface(cyTotalPaymentAmountEntered));
				if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
					return false;
				}
			}
		}

		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//POINT OF NO RETURN
		//
		//Not to sound all dramatic, I'm just stating that all possible
		//tests to abort posting must happen before this block, and
		//all processing starts after this block.
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////

		//Step 2. Create a payment and/or adjustment (if needed)

		//payment
		long nPaymentID = -1;
		
		//We will make a payment if the amount needed is greater than zero.
		//We will also make a payment regardless of the above, if the user
		//entered in a payment amount, and this is not a batch payment.
		
		// (j.jones 2010-05-17 16:55) - PLID 16503 - create a payment if we allow zero dollar applies
		// (j.jones 2012-08-14 11:45) - PLID 52117 - handle null payments
		// (j.jones 2013-03-15 15:56) - PLID 49444 - allowed $0.00 payments for non-batch payments, if bPostZeroDollarPayments is enabled		
		if((m_nBatchPaymentID == -1 && (cyTotalPaymentAmountEntered > COleCurrency(0,0) || (bPostZeroDollarPayments && cyTotalPaymentAmountEntered == COleCurrency(0,0))))
			|| (m_nBatchPaymentID != -1 && varTotalPaymentAmtNeeded.vt == VT_CY && (bPostZeroDollarPayments || VarCurrency(varTotalPaymentAmtNeeded) > COleCurrency(0,0))) ) {

			COleCurrency cyPaymentAmt = COleCurrency(0,0);
			// (j.jones 2010-09-21 10:45) - PLID 40605 - If this is not a batch payment,
			// create a payment for the total amount entered, which we later apply as
			// needed. If this is a batch payment, only apply the amount needed.
			// If both are zero, we should only be creating a payment if bPostZeroDollarPayments
			// is true.
			if(m_nBatchPaymentID == -1 && cyTotalPaymentAmountEntered > COleCurrency(0,0)) {
				cyPaymentAmt = cyTotalPaymentAmountEntered;
			}
			else if(m_nBatchPaymentID != -1 && VarCurrency(varTotalPaymentAmtNeeded) > COleCurrency(0,0)) {
				cyPaymentAmt = VarCurrency(varTotalPaymentAmtNeeded);
			}

			nPaymentID = CreatePayment(cyPaymentAmt, ePayment, nInsuredPartyID, m_nBatchPaymentID);
		}

		//adjustments

		// (j.jones 2012-05-09 09:00) - PLID 37165 - We can now apply multiple adjustments per line.
		// If no pointers are in memory, only make an adjustment if the amount is > 0.
		// If any pointers are in memory, create all adjustments, even if the total is zero.
		long nMasterAdjustmentID = -1;
		COleCurrency cyMasterAdjustmentAmount = cyTotalAdjustmentAmtNeeded;

		//if we have pointers, make each adjustment, counting 
		for(int i=0;i<m_aryChargeAdjustmentInfo.GetSize();i++) {
			ChargeAdjustmentInfo *pInfo = (ChargeAdjustmentInfo*)m_aryChargeAdjustmentInfo.GetAt(i);
			for(int j=0;j<pInfo->aryAdjustmentInfo.GetSize();j++) {
				AdjustmentInfo* pAdj = (AdjustmentInfo*)pInfo->aryAdjustmentInfo.GetAt(j);
				cyMasterAdjustmentAmount -= pAdj->cyAmount;
				pAdj->nAdjustmentID = CreatePayment(pAdj->cyAmount, eAdjustment, nInsuredPartyID, -1, pAdj->nGroupCodeID, pAdj->nReasonCodeID);
			}
		}

		//If we have any adjustment amounts remaining, make one master adjustment to cover the balance.
		//If we didn't have any adjustment pointers, this would be the full adjustment amount.
		// (j.jones 2016-05-11 9:34) - NX-100503 - we now allow negative adjustments
		if(cyMasterAdjustmentAmount != COleCurrency(0,0)) {
			nMasterAdjustmentID = CreatePayment(cyMasterAdjustmentAmount, eAdjustment, nInsuredPartyID, m_nBatchPaymentID); // (b.eyers 2015-10-16) - PLID 67357 - send batch id for adjustments
		}

		// (j.jones 2010-06-02 16:33) - PLID 37200 - this array will track payments we unapplied in the first loop
		CArray<UnappliedAmount, UnappliedAmount> aryUnappliedPatPaymentIDs;
		CArray<UnappliedAmount, UnappliedAmount> aryUnappliedInsPaymentIDs;

		BOOL bSwappedInsCos = FALSE;
		BOOL bBatchedBill = FALSE;

		// (j.jones 2010-06-02 16:30) - PLID 37200 - we first have to unapply each charge and shift,
		// then post payments

		for(i=0;i<m_PostingList->GetRowCount();i++) {

			IRowSettingsPtr pRow = m_PostingList->GetRow(i);			
			long nChargeID = VarLong(pRow->GetValue(plcChargeID),-1);
			long nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID),-1);

			COleCurrency cyPatResp = VarCurrency(pRow->GetValue(m_nPatRespColIndex),COleCurrency(0,0));
			COleCurrency cyInsResp = VarCurrency(pRow->GetValue(m_nInsRespColIndex),COleCurrency(0,0));

			// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new payments,
			// plus we cache the initial existing total in an "old" column
			COleCurrency cyOldExistingPatPays = VarCurrency(pRow->GetValue(plcOldExistingPatPays),COleCurrency(0,0));
			COleCurrency cyExistingPatPays = VarCurrency(pRow->GetValue(m_nExistingPatPaysColIndex),COleCurrency(0,0));
			COleCurrency cyOldExistingInsPays = VarCurrency(pRow->GetValue(plcOldExistingInsPays),COleCurrency(0,0));
			COleCurrency cyExistingInsPays = VarCurrency(pRow->GetValue(m_nExistingInsPaysColIndex),COleCurrency(0,0));

			// (j.jones 2010-05-05 09:49) - PLID 25521 - New Pays is just one column now
			// (j.jones 2012-08-14 10:58) - PLID 52117 - payments can now be null
			_variant_t varNewPays = pRow->GetValue(m_nNewPaysColIndex);

			COleCurrency cyTotalPatPays = cyExistingPatPays;
			COleCurrency cyTotalInsPays = cyExistingInsPays;

			// (j.jones 2014-07-01 11:26) - PLID 62553 - the total payments should ignore negative amounts
			COleCurrency cyNewPays = VarCurrency(varNewPays, COleCurrency(0, 0));
			if (cyNewPays > COleCurrency(0, 0)) {
				if (m_nInsuredPartyID == -1) {
					cyTotalPatPays += cyNewPays;
				}
				else {
					cyTotalInsPays += cyNewPays;
				}
			}

			// (j.jones 2007-01-04 09:28) - PLID 24030 - confirm we are posting to a revenue code
			BOOL bIsRevCode = IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && nRevCodeID != -1;

			if(bIsRevCode && nInsuredPartyID == -1) {
				ASSERT(FALSE);
				continue;
			}

			//Step 3. Unapply any amounts (if needed) prior to shifting

			// (j.jones 2010-05-05 10:16) - PLID 25521 - this is now only a feature
			// of the "existing pays" columns
			if(cyExistingPatPays < cyOldExistingPatPays) {
				//unapply the difference
				COleCurrency cyAmtToUnapply = cyOldExistingPatPays - cyExistingPatPays;
				if(bIsRevCode) {
					//revenue code
					// (j.jones 2010-06-02 16:33) - PLID 37200 - track the payments we unapply
					UnapplyFromRevenueCode(m_nBillID, nRevCodeID, -1, FALSE, cyAmtToUnapply, &aryUnappliedPatPaymentIDs);
				}
				else {
					//normal charge
					// (j.jones 2010-06-02 16:33) - PLID 37200 - track the payments we unapply
					UnapplyFromCharge(nChargeID,-1,FALSE,cyAmtToUnapply, &aryUnappliedPatPaymentIDs);
				}
			}

			if(nInsuredPartyID != -1) {
				// (j.jones 2010-05-05 10:16) - PLID 25521 - this is now only a feature
				// of the "existing pays" columns
				if(cyExistingInsPays < cyOldExistingInsPays) {
					//unapply the difference
					COleCurrency cyAmtToUnapply = cyOldExistingInsPays - cyExistingInsPays;

					if(bIsRevCode) {
						//revenue code
						// (j.jones 2010-06-02 16:33) - PLID 37200 - track the payments we unapply
						UnapplyFromRevenueCode(m_nBillID, nRevCodeID, nInsuredPartyID, TRUE, cyAmtToUnapply, &aryUnappliedInsPaymentIDs);
					}
					else {
						//normal charge
						// (j.jones 2010-06-02 16:33) - PLID 37200 - track the payments we unapply
						UnapplyFromCharge(nChargeID,nInsuredPartyID,TRUE,cyAmtToUnapply, &aryUnappliedInsPaymentIDs);
					}
				}
			}

			//Step 4. Shift balances properly (if needed)
			if(nInsuredPartyID != -1) {

				// (j.jones 2008-04-15 16:06) - PLID 29668 - we have to check the responsibility
				// by revenue code, if bIsRevCode is true
				COleCurrency cyExistingPatResp = COleCurrency(0,0);
				{
					if(bIsRevCode) {
						COleCurrency cyCharges = COleCurrency(0,0), cyInsResp = COleCurrency(0,0);
						GetRevenueCodeTotals(nRevCodeID, m_nBillID, m_nPatientID, &cyCharges, 0, 0, 0, &cyInsResp);
						cyExistingPatResp = cyCharges - cyInsResp;
					}
					else {
						cyExistingPatResp = GetChargePatientResp(nChargeID);
					}
				}
				
				COleCurrency cyExistingInsResp = COleCurrency(0,0);
				if(bIsRevCode) {
					GetRevenueCodeInsuranceTotals(nRevCodeID, m_nBillID, m_nPatientID, m_nInsuredPartyID, &cyExistingInsResp, 0, 0, 0);
				}
				else {
					cyExistingInsResp = GetChargeInsResp(nChargeID, m_nInsuredPartyID);
				}

				// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string to all calls to ShiftInsuranceResponsibility

				if(cyPatResp < cyExistingPatResp) {
					if(bIsRevCode) {
						//revenue code
						ShiftInsuranceResponsibility(m_nBillID,m_nPatientID,-1,m_nInsuredPartyID,"RevCode",cyExistingPatResp - cyPatResp,
							"from manually changing responsibility in Line Item Posting",
							COleDateTime::GetCurrentTime(),nRevCodeID);
					}
					else {
						//normal charge
						ShiftInsuranceResponsibility(nChargeID,m_nPatientID,-1,m_nInsuredPartyID,"Charge",cyExistingPatResp - cyPatResp,
							"from manually changing responsibility in Line Item Posting",
							COleDateTime::GetCurrentTime());
					}
				}
				else if(cyInsResp < cyExistingInsResp) {
					if(bIsRevCode) {
						//revenue code
						ShiftInsuranceResponsibility(m_nBillID,m_nPatientID,m_nInsuredPartyID,-1,"RevCode",cyExistingInsResp - cyInsResp,
							"from manually changing responsibility in Line Item Posting",
							COleDateTime::GetCurrentTime(),nRevCodeID);
					}
					else {
						//normal charge
						ShiftInsuranceResponsibility(nChargeID,m_nPatientID,m_nInsuredPartyID,-1,"Charge",cyExistingInsResp - cyInsResp,
							"from manually changing responsibility in Line Item Posting",
							COleDateTime::GetCurrentTime());
					}
				}
			}
		}

		// (j.jones 2011-03-23 16:12) - PLID 42936 - all these apply calls now skip checking for allowables,
		// we do it at the end instead for all the payments we applied
		CArray<long, long> aryAppliedPaymentIDs;

		for(i=0;i<m_PostingList->GetRowCount();i++) {

			IRowSettingsPtr pRow = m_PostingList->GetRow(i);
			long nChargeID = VarLong(pRow->GetValue(plcChargeID),-1);
			long nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID),-1);

			COleCurrency cyPatResp = VarCurrency(pRow->GetValue(m_nPatRespColIndex),COleCurrency(0,0));
			COleCurrency cyInsResp = VarCurrency(pRow->GetValue(m_nInsRespColIndex),COleCurrency(0,0));
			// (j.jones 2011-04-27 09:39) - PLID 42705 - get the allowable
			COleCurrency cyAllowable = VarCurrency(pRow->GetValue(m_nAllowableColIndex),COleCurrency(0,0));

			// (j.jones 2010-05-04 16:23) - PLID 25521 - we now separate out existing & new payments,
			// plus we cache the initial existing total in an "old" column
			COleCurrency cyOldExistingPatPays = VarCurrency(pRow->GetValue(plcOldExistingPatPays),COleCurrency(0,0));
			COleCurrency cyExistingPatPays = VarCurrency(pRow->GetValue(m_nExistingPatPaysColIndex),COleCurrency(0,0));
			COleCurrency cyOldExistingInsPays = VarCurrency(pRow->GetValue(plcOldExistingInsPays),COleCurrency(0,0));
			COleCurrency cyExistingInsPays = VarCurrency(pRow->GetValue(m_nExistingInsPaysColIndex),COleCurrency(0,0));

			// (j.jones 2010-05-05 09:49) - PLID 25521 - New Pays is just one column now
			// (j.jones 2012-08-14 10:58) - PLID 52117 - payments can now be null
			_variant_t varNewPays = pRow->GetValue(m_nNewPaysColIndex);

			// (b.spivey, January 04, 2012) - PLID 47121 - Grab the deductible and coinsurance values. 
			COleCurrency cyDeductible = VarCurrency(pRow->GetValue(m_nDeductibleAmtColIndex), COleCurrency(0,0));
			COleCurrency cyCoinsurance = VarCurrency(pRow->GetValue(m_nCoinsuranceAmtColIndex), COleCurrency(0,0));
			// (j.jones 2013-08-27 11:41) - PLID 57398 - added copay
			COleCurrency cyCopay = VarCurrency(pRow->GetValue(m_nCopayAmtColIndex), COleCurrency(0,0));

			COleCurrency cyTotalPatPays = cyExistingPatPays;
			COleCurrency cyTotalInsPays = cyExistingInsPays;

			// (j.jones 2014-07-01 11:26) - PLID 62553 - the total payments should ignore negative amounts
			COleCurrency cyNewPaysTotal = VarCurrency(varNewPays, COleCurrency(0, 0));
			if (cyNewPaysTotal > COleCurrency(0, 0)) {
				if (m_nInsuredPartyID == -1) {
					cyTotalPatPays += cyNewPaysTotal;
				}
				else {
					cyTotalInsPays += cyNewPaysTotal;
				}
			}

			// (j.jones 2014-07-01 11:34) - PLID 62553 - added chargebacks			
			if (m_ePayType == eVisionPayment && m_nInsuredPartyID != -1 && nChargeID != -1 && cyNewPaysTotal < COleCurrency(0, 0)) {
				COleCurrency cyChargeback = cyNewPaysTotal;

				//create and apply the chargeback now
				CreateChargeback(nChargeID, cyNewPaysTotal);
			}

			// (j.jones 2007-01-04 09:28) - PLID 24030 - confirm we are posting to a revenue code
			BOOL bIsRevCode = IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && nRevCodeID != -1;

			if(bIsRevCode && nInsuredPartyID == -1) {
				ASSERT(FALSE);
				continue;
			}

			// (j.jones 2010-06-02 16:36) - PLID 37200 - supported moving existing applies around
			//Step 5. Apply the previously-unapplied credits (if needed)
			if(cyExistingPatPays > cyOldExistingPatPays && aryUnappliedPatPaymentIDs.GetSize() > 0) {
				//reapply from payments we previously unapplied
				COleCurrency cyAmtToApply = cyExistingPatPays - cyOldExistingPatPays;

				for(int j=aryUnappliedPatPaymentIDs.GetSize()-1;j>=0 && cyAmtToApply > COleCurrency(0,0);j--) {
					UnappliedAmount ua = (UnappliedAmount)aryUnappliedPatPaymentIDs.GetAt(j);

					if(ua.cyAmtUnapplied <= COleCurrency(0,0)) {
						aryUnappliedPatPaymentIDs.RemoveAt(j);
						continue;
					}

					COleCurrency cy = COleCurrency(0,0);
					long nReappliedPaymentID = ua.nPaymentID;

					if(ua.cyAmtUnapplied > cyAmtToApply) {
						cy = cyAmtToApply;
						ua.cyAmtUnapplied -= cy;
					}
					else {
						cy = ua.cyAmtUnapplied;
						//this payment is now used up, remove it
						aryUnappliedPatPaymentIDs.RemoveAt(j);
					}

					if(bIsRevCode) {
						//revenue code
						aryAppliedPaymentIDs.Add(nReappliedPaymentID);
						ApplyPayToBillWithRevenueCode(nReappliedPaymentID,m_nPatientID,cy,m_nBillID,nRevCodeID,-1,nInsuredPartyID, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE);
					}
					else {
						//normal charge
						aryAppliedPaymentIDs.Add(nReappliedPaymentID);
						ApplyPayToBill(nReappliedPaymentID,m_nPatientID,cy,"Charge",nChargeID, -1,FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE);
					}
				}
			}

			if(nInsuredPartyID != -1) {
				if(cyExistingInsPays > cyOldExistingInsPays && aryUnappliedInsPaymentIDs.GetSize() > 0) {
					//reapply from payments we previously unapplied
					COleCurrency cyAmtToApply = cyExistingInsPays - cyOldExistingInsPays;
					
					for(int j=aryUnappliedInsPaymentIDs.GetSize()-1;j>=0 && cyAmtToApply > COleCurrency(0,0);j--) {
						UnappliedAmount ua = (UnappliedAmount)aryUnappliedInsPaymentIDs.GetAt(j);

						if(ua.cyAmtUnapplied <= COleCurrency(0,0)) {
							aryUnappliedInsPaymentIDs.RemoveAt(j);
							continue;
						}

						COleCurrency cy = COleCurrency(0,0);
						long nReappliedPaymentID = ua.nPaymentID;

						if(ua.cyAmtUnapplied > cyAmtToApply) {
							cy = cyAmtToApply;
							ua.cyAmtUnapplied -= cy;
						}
						else {
							cy = ua.cyAmtUnapplied;
							//this payment is now used up, remove it
							aryUnappliedInsPaymentIDs.RemoveAt(j);
						}

						if(bIsRevCode) {
							//revenue code
							aryAppliedPaymentIDs.Add(nReappliedPaymentID);
							ApplyPayToBillWithRevenueCode(nReappliedPaymentID,m_nPatientID,cy,m_nBillID,nRevCodeID,nInsuredPartyID, nInsuredPartyID, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE);
						}
						else {
							//normal charge		
							aryAppliedPaymentIDs.Add(nReappliedPaymentID);
							ApplyPayToBill(nReappliedPaymentID,m_nPatientID,cy,"Charge",nChargeID, nInsuredPartyID, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE);
						}
					}
				}
			}

			//Step 6. Apply the new payment and adjustment (if needed)

			// (j.jones 2012-08-14 11:19) - PLID 52117 - payments can now be null
			COleCurrency cyNewPaysToApply = VarCurrency(varNewPays, COleCurrency(0, 0));

			//Step 6a. Apply adjustments.

			// (j.jones 2012-05-09 11:23) - PLID 37165 - if we have adjustment pointers, apply those adjustment IDs,
			// else apply nMasterAdjustmentID
			// (j.jones 2016-05-11 9:17) - NX-100503 - moved adjustments to post prior to payments
			ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
			if (pAdjInfo) {
				for (int a = 0; a<pAdjInfo->aryAdjustmentInfo.GetSize(); a++) {
					AdjustmentInfo *pAdj = (AdjustmentInfo*)pAdjInfo->aryAdjustmentInfo.GetAt(a);
					if (pAdj->nAdjustmentID == -1) {
						ThrowNxException("AdjustmentInfo found with no adjustment ID!");
					}

					// (j.jones 2016-05-11 8:56) - NX-100503 - never apply negative adjustments to a bill,
					// instead apply them to the payment if one exists
					if (pAdj->cyAmount < COleCurrency(0, 0)) {

						if (cyNewPaysToApply > COleCurrency(0, 0)) {
							COleCurrency cyAmtToApply = -pAdj->cyAmount;
							if (cyAmtToApply > cyNewPaysToApply) {
								cyAmtToApply = cyNewPaysToApply;
							}

							//reduce the amt of the payment we will apply
							if (cyAmtToApply > COleCurrency(0, 0)) {
								cyNewPaysToApply -= cyAmtToApply;
								AutoApplyPayToPay(pAdj->nAdjustmentID, m_nPatientID, "Payment", nPaymentID);
							}
						}
						
						//regardless of whether we applied anything, move to the next adjustment
						continue;
					}

					//unlike adjustments that are not memory objects, $0.00 adjustments will apply
					//if they came from memory objects

					if (bIsRevCode) {
						//revenue code
						// (j.jones 2010-06-02 17:44) - PLID 37200 - renamed to not be insurance-specific
						ApplyPayToBillWithRevenueCode(pAdj->nAdjustmentID, m_nPatientID, pAdj->cyAmount, m_nBillID, nRevCodeID, nInsuredPartyID, nInsuredPartyID, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE);
					}
					else {
						//normal charge
						//(e.lally 2007-03-30) PLID 25263 - Switched this to the new general apply function
						ApplyPayToBill(pAdj->nAdjustmentID, m_nPatientID, pAdj->cyAmount, "Charge", nChargeID, nInsuredPartyID, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE);
					}
				}
			}
			else if (nMasterAdjustmentID != -1) {
				COleCurrency cyInsAdjustments = VarCurrency(pRow->GetValue(m_nAdjustmentColIndex), COleCurrency(0, 0));

				// (j.jones 2016-05-11 8:56) - NX-100503 - never apply negative adjustments to a bill,
				// instead apply them to the payment if one exists
				if (cyInsAdjustments < COleCurrency(0, 0)) {
					if (cyNewPaysToApply > COleCurrency(0, 0)) {
						COleCurrency cyAmtToApply = -cyInsAdjustments;
						if (cyAmtToApply > cyNewPaysToApply) {
							cyAmtToApply = cyNewPaysToApply;
						}

						//reduce the amt of the payment we will apply
						if (cyAmtToApply > COleCurrency(0, 0)) {
							cyNewPaysToApply -= cyAmtToApply;
							AutoApplyPayToPay(nMasterAdjustmentID, m_nPatientID, "Payment", nPaymentID);
						}
					}
				}
				else if (cyInsAdjustments > COleCurrency(0, 0)) {
					if (bIsRevCode) {
						//revenue code
						// (j.jones 2010-06-02 17:44) - PLID 37200 - renamed to not be insurance-specific
						ApplyPayToBillWithRevenueCode(nMasterAdjustmentID, m_nPatientID, cyInsAdjustments, m_nBillID, nRevCodeID, nInsuredPartyID, nInsuredPartyID, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE);
					}
					else {
						//normal charge
						//(e.lally 2007-03-30) PLID 25263 - Switched this to the new general apply function
						ApplyPayToBill(nMasterAdjustmentID, m_nPatientID, cyInsAdjustments, "Charge", nChargeID, nInsuredPartyID, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE);
					}
				}
			}

			//Step 6b. Apply payments.

			if(nInsuredPartyID == -1) {					
				// (j.jones 2013-03-15 16:43) - PLID 49444 - permitted $0.00 applies on patients
				if ((cyNewPaysToApply > COleCurrency(0, 0) || (varNewPays.vt == VT_CY && cyNewPaysToApply == COleCurrency(0, 0) && bPostZeroDollarPayments)) && nPaymentID != -1) {

					if(bIsRevCode) {
						ASSERT(FALSE);
						continue;
					}

					aryAppliedPaymentIDs.Add(nPaymentID);
					ApplyPayToBill(nPaymentID,m_nPatientID, cyNewPaysToApply, "Charge",nChargeID, -1 /*Patient Resp*/, FALSE, FALSE, FALSE, FALSE, TRUE, bPostZeroDollarPayments, TRUE, FALSE);
				}
			}
			// (j.jones 2015-10-26 10:47) - PLID 67451 - capitation allows overpayments,
			// the balance of which will be adjusted off
			else if (m_bIsCapitation && bIsCapitationOverpayment) {
				//only apply part of the payment
				if (cyCapitationOverpaymentAmtToApply > COleCurrency(0, 0)) {
					aryAppliedPaymentIDs.Add(nPaymentID);
					ApplyPayToBill(nPaymentID, m_nPatientID, cyCapitationOverpaymentAmtToApply, "Charge", nChargeID, nInsuredPartyID, FALSE, FALSE, FALSE, FALSE, TRUE, bPostZeroDollarPayments, TRUE, FALSE);
				}
				//if there was an overage, adjust the remainder of the payment
				if (cyCapitationOverpaymentAmtToAdjust > COleCurrency(0, 0)) {
					long nOverpayAdjID = CreatePayment(-cyCapitationOverpaymentAmtToAdjust, eCapitationOverpaymentAdjustment, nInsuredPartyID, m_nBatchPaymentID);
					AutoApplyPayToPay(nOverpayAdjID, m_nPatientID, "Payment", nPaymentID);
				}
			}
			else {
				// (j.jones 2010-05-17 16:56) - PLID 16503 - supported $0.00 applies
				// (j.jones 2012-08-14 11:19) - PLID 52117 - payments can now be null
				if (varNewPays.vt == VT_CY && ((varNewPays.vt == VT_CY && cyNewPaysToApply == COleCurrency(0, 0) && bPostZeroDollarPayments) || cyNewPaysToApply > COleCurrency(0, 0)) && nPaymentID != -1) {
										
					if(bIsRevCode) {
						//revenue code
						// (j.jones 2010-06-02 17:44) - PLID 37200 - renamed to not be insurance-specific
						aryAppliedPaymentIDs.Add(nPaymentID);
						ApplyPayToBillWithRevenueCode(nPaymentID,m_nPatientID, cyNewPaysToApply, m_nBillID,nRevCodeID,nInsuredPartyID, nInsuredPartyID, FALSE, FALSE, FALSE, TRUE, bPostZeroDollarPayments, TRUE);
					}
					else {
						//normal charge
						//(e.lally 2007-03-30) PLID 25263 - Switched this to the new general apply function
						aryAppliedPaymentIDs.Add(nPaymentID);
						ApplyPayToBill(nPaymentID,m_nPatientID, cyNewPaysToApply, "Charge",nChargeID, nInsuredPartyID, FALSE, FALSE, FALSE, FALSE, TRUE, bPostZeroDollarPayments, TRUE, FALSE);
					}
				}
			}

			//Step 7. Shift balance (if needed)

			// (j.jones 2008-04-28 10:21) - PLID 29634 - do not shift the balance if we are
			// paying to patient, it shouldn't try to shift away from the patient resp
			if(m_ShiftRespCombo->CurSel != -1 && nInsuredPartyID != -1) {
				long nRespType = VarLong(m_ShiftRespCombo->GetValue(m_ShiftRespCombo->CurSel,srccID),-1);
				if(nRespType != -1) {	//-1 means do not shift
					BOOL bShiftPaidCharges = IsDlgButtonChecked(IDC_CHECK_SHIFT_PAID_AMOUNTS);

					long nDestInsPartyID = GetInsuranceIDFromType(m_nPatientID,nRespType);

						// (j.jones 2010-05-17 16:56) - PLID 16503 - if we applied $0.00, consider it as having
						// been paid, and shift accordingly
						// (j.jones 2012-08-14 11:19) - PLID 52117 - payments can now be null
						// (j.jones 2014-09-15 13:40) - PLID 62553 - payments can now be negative, and we do want this feature
						// to shift if they posted a chargeback
						if(cyInsResp - cyTotalInsPays > COleCurrency(0,0) &&
							(!bShiftPaidCharges || (varNewPays.vt == VT_CY && ((VarCurrency(varNewPays) == COleCurrency(0, 0) && bPostZeroDollarPayments) || VarCurrency(varNewPays) != COleCurrency(0, 0))))) {
							//shift insurance balance

							// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string to ShiftInsuranceResponsibility
							
							if(bIsRevCode) {
								//revenue code
								ShiftInsuranceResponsibility(m_nBillID,m_nPatientID,m_nInsuredPartyID,nDestInsPartyID,"RevCode",cyInsResp - cyTotalInsPays,
									"after posting through Line Item Posting",
									COleDateTime::GetCurrentTime(), nRevCodeID);
							}
							else {
								//normal charge
								ShiftInsuranceResponsibility(nChargeID,m_nPatientID,m_nInsuredPartyID,nDestInsPartyID,"Charge",cyInsResp - cyTotalInsPays,
									"after posting through Line Item Posting",
									COleDateTime::GetCurrentTime());
							}
						}
					//}

					if(nDestInsPartyID != -1) {

						// (j.jones 2006-08-07 09:27) - PLID 21433 - auto-swap placement if the checkbox says to do so
						// (j.jones 2014-07-02 15:20) - PLID 62548 - this is always true for vision payments
						if((IsDlgButtonChecked(IDC_CHECK_AUTO_SWAP) || m_ePayType == eVisionPayment) && !bSwappedInsCos) {
							bSwappedInsCos = TRUE;
							SwapInsuranceCompanies(m_nBillID, nInsuredPartyID, nDestInsPartyID);
						}

						// (j.jones 2007-05-22 11:50) - PLID 23037 - auto-batch the bill if the checkbox days to do so
						// (j.jones 2014-07-02 15:20) - PLID 62548 - this is always true for vision payments
						if ((IsDlgButtonChecked(IDC_CHECK_AUTO_BATCH) || m_ePayType == eVisionPayment) && !bBatchedBill) {

							//find the default batch
							int iBatch = FindDefaultHCFABatch(nDestInsPartyID);
							if(iBatch > 0) {
								BatchBill(m_nBillID, iBatch);
								bBatchedBill = TRUE;											
							}
						}
					}
				}
			}

			// (j.jones 2011-04-27 09:37) - PLID 42705 - save the allowable, even if it is zero
			// (j.jones 2011-06-24 09:33) - PLID 44314 - this should not be called with a -1 charge ID
			if(nInsuredPartyID != -1
				&& cyAllowable.GetStatus() != COleCurrency::invalid
				&& cyAllowable >= COleCurrency(0,0)
				&& nChargeID != -1) {

				SaveChargeAllowable(m_nPatientID, GetExistingPatientName(m_nPatientID), nChargeID, nInsuredPartyID, cyAllowable, caemLineItemPosting);
			}

			// (b.spivey, January 04, 2012) - PLID 47121 - Save the deductible/coinsurance as long as it's not a patient resp.
			// (j.jones 2013-08-27 11:40) - PLID 57398 - added copay
			if(nInsuredPartyID != -1 
				&& cyDeductible.GetStatus() != COleCurrency::invalid
				&& cyCoinsurance.GetStatus() != COleCurrency::invalid 
				&& cyCopay.GetStatus() != COleCurrency::invalid 
				&& nChargeID != -1) {

				// (b.spivey, January 05, 2012) - PLID 47121 - If either of these have a value other than null, we save that value
				//		even if the other is null. Nulls default to zero. 
				// (j.jones 2013-08-27 11:40) - PLID 57398 - added copay
				if(VarCurrency(pRow->GetValue(m_nDeductibleAmtColIndex), g_ccyNull) != g_ccyNull
					|| VarCurrency(pRow->GetValue(m_nCoinsuranceAmtColIndex), g_ccyNull) != g_ccyNull
					|| VarCurrency(pRow->GetValue(m_nCopayAmtColIndex), g_ccyNull) != g_ccyNull) {

					// (j.jones 2012-08-14 14:38) - PLID 50285 - This now returns true if it created the first
					// deductible/coinsurance record for this charge and insured party. If it returned false,
					// it would have updated an existing entry.
					// (j.jones 2013-08-27 11:40) - PLID 57398 - added copay
					if(SaveChargeCoinsurance(m_nPatientID, GetExistingPatientName(m_nPatientID), nChargeID, nInsuredPartyID, cyDeductible, cyCoinsurance, cyCopay, ccemLineItemPosting)) {
						
						// (j.jones 2012-08-14 14:39) - PLID 50285 - try to create a billing note, will check preferences
						// (j.jones 2013-08-27 11:40) - PLID 57398 - added copay
						CreateBillingNoteForDeductibleCoinsCopay(nChargeID, pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
					}
				}
				// (b.spivey, January 05, 2012) - PLID 47121 - If they're both equal to null and there is an existing row, we save 
				//		those nulls to the database. Otherwise we ignore it.
				// (j.jones 2013-08-27 11:40) - PLID 57398 - added copay
				else if(VarCurrency(pRow->GetValue(m_nDeductibleAmtColIndex), g_ccyNull) == g_ccyNull
					&& VarCurrency(pRow->GetValue(m_nCoinsuranceAmtColIndex), g_ccyNull) == g_ccyNull
					&& VarCurrency(pRow->GetValue(m_nCopayAmtColIndex), g_ccyNull) == g_ccyNull) {
						
						// (b.spivey, January 11, 2012) - PLID 47121 - Nothing happens. 
				}
			}
		}

		//Step 8. Batch a claim and swap. ins. co's as needed
			
		//first update the bill with the new ins. co. placements
		
		//PrimaryInsID and OtherInsID are calculated earlier

		// (j.jones 2006-08-07 09:26) - PLID 21433 - don't swap here if we auto-swapped earlier
		if(!bSwappedInsCos) {
			// (j.jones 2009-10-23 15:08) - PLID 35564 - these can be nullable, but not -1
			_variant_t varInsuredID = g_cvarNull;
			_variant_t varOtherInsuredID = g_cvarNull;
			if(PrimaryInsID != -1) {
				varInsuredID = PrimaryInsID;
			}
			if(OtherInsID != -1) {
				varOtherInsuredID = OtherInsID;
			}
			ExecuteParamSql("UPDATE BillsT SET InsuredPartyID = {VT_I4}, OthrInsuredPartyID = {VT_I4} WHERE ID = {INT}", varInsuredID, varOtherInsuredID, m_nBillID);
			// (s.tullis 2016-03-01 12:47) - PLID 68319 - Update claim for the Insurances
			if (PrimaryInsID != -1) {
				UpdateBillClaimForm(m_nBillID);
			}

			// (j.jones 2009-03-11 08:50) - PLID 32864 - audit this, using our tracked variables
			if(m_nBillOldInsuredPartyID != PrimaryInsID) {

				if(m_nBillOldInsuredPartyID == -1) {
					m_strOldInsuranceCoName = "<None Selected>";
				}

				if(PrimaryInsID == -1) {
					strPrimaryInsName = "<None Selected>";
				}

				if(nAuditID == -1) {
					nAuditID = BeginNewAuditEvent();
				}
				AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditID, aeiBillInsurancePlan, m_nBillID, m_strOldInsuranceCoName, strPrimaryInsName, aepMedium, aetChanged);
			}

			if(m_nBillOldOtherInsuredPartyID != OtherInsID) {

				if(m_nBillOldOtherInsuredPartyID == -1) {
					m_strOldOtherInsuranceCoName = "<None Selected>";
				}

				if(OtherInsID == -1) {
					strOtherInsName = "<None Selected>";
				}

				if(nAuditID == -1) {
					nAuditID = BeginNewAuditEvent();
				}
				AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditID, aeiBillOtherInsurancePlan, m_nBillID, m_strOldOtherInsuranceCoName, strOtherInsName, aepMedium, aetChanged);
			}
		}

		//now batch as needed

		//we'll check if it is paid off, and if so, unbatch, otherwise, batch as indicated
		if(!CheckUnbatchClaim(m_nBillID)) {

			// (j.jones 2007-05-22 11:51) - PLID 23037 - don't batch here if we auto-batched earlier
			if(!bBatchedBill)
				//nBatch is calculated earlier			
				BatchBill(m_nBillID, nBatch);
		}

		// (j.jones 2007-02-21 16:19) - PLID 23280 - update the last payment date now, only if not a batch payment
		if(m_nBatchPaymentID == -1) {
			// (j.jones 2007-05-04 09:49) - PLID 23280 - use the GlobalUtils function
			AddLastPaymentDate(nPaymentID, dtPayment);
		}

		// (j.jones 2013-07-22 08:57) - PLID 57653 - If they have configured insurance companies
		// to force unbatching due to primary crossover to secondary, force unbatching now.
		// This needs to be after shifting/batching has occurred in the normal posting flow.
		if(nInsuredPartyID != -1) {
			//This ConfigRT name is misleading, it actually just means that if we do unbatch a crossed over claim,
			//claim history will only include batched charges. If false, then claim history includes all charges.
			bool bBatchedChargesOnlyInClaimHistory = (GetRemotePropertyInt("ERemit_UnbatchMA18orNA89_MarkForwardToSecondary", 1, 0, "<None>", true) == 1);

			//This function assumes that the bill's current insured party ID is now the "secondary" insured party
			//we crossed over to, and the insured party who paid was primary.
			//If the payer really was the patient's Primary, and crossing over is enabled, the bill will be unbatched.
			CheckUnbatchCrossoverClaim(m_nPatientID, m_nBillID, nInsuredPartyID, dtPayment,
				bBatchedChargesOnlyInClaimHistory, aeiClaimBatchStatusChangedByManualCrossover, "Batched", "Unbatched due to manual Primary/Secondary crossover");
		}

		// (j.jones 2011-03-23 16:12) - PLID 42936 - now we have to check the allowables for what we applied,
		// but only for the bill that we applied to (which is still relevant even if we are only applying to one charge)
		if(aryAppliedPaymentIDs.GetSize() > 0) {
			_RecordsetPtr rsAppliedTo = CreateParamRecordset("SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
				"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
				"InsuredPartyT.PersonID, ChargesT.ID, AppliesT.Amount "
				"FROM LineItemT "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
				"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
				"WHERE LineItemT.Deleted = 0 AND AppliesT.SourceID IN ({INTARRAY}) AND BillsT.ID = {INT}", aryAppliedPaymentIDs, m_nBillID);
			while(!rsAppliedTo->eof) {
				//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, ChargeID, and the dollar amount we applied
				WarnAllowedAmount(AdoFldLong(rsAppliedTo, "ServiceID", -1), AdoFldLong(rsAppliedTo, "InsuranceCoID", -1),
					AdoFldLong(rsAppliedTo, "DoctorsProviders", -1), AdoFldLong(rsAppliedTo, "LocationID", -1),
					AdoFldLong(rsAppliedTo, "POSID", -1), AdoFldLong(rsAppliedTo, "PersonID", -1),
					AdoFldLong(rsAppliedTo, "ID"), AdoFldCurrency(rsAppliedTo, "Amount"));

				//if we underpaid multiple charges, this will cause multiple prompts (this is not typical)
				rsAppliedTo->MoveNext();
			}
			rsAppliedTo->Close();
		}

		return true;

	}NxCatchAll("Error posting changes to the patient's account.");

	// (j.jones 2012-08-16 12:25) - PLID 52162 - we might be auto-posting, we would have already
	// shown the exception, but the caller expects a failure message
	if(m_bIsAutoPosting) {
		m_strAutoPostFailure = "An error has occurred during posting. Please contact NexTech Technical Support.";
	}
	return false;
}

void CFinancialLineItemPostingDlg::OnRequeryFinishedPayInsRespCombo(short nFlags) 
{
	try {
		_RecordsetPtr tmpRS = CreateRecordset("SELECT GuarantorID1, GuarantorID2 FROM PatientsT LEFT JOIN (SELECT InsuredPartyT.PersonID AS GuarantorID1, PatientID FROM InsuredPartyT WHERE InsuredPartyT.RespTypeID = 1) AS InsParty1 "
				"ON PatientsT.PersonID = InsParty1.PatientID LEFT JOIN (SELECT InsuredPartyT.PersonID AS GuarantorID2, PatientID FROM InsuredPartyT WHERE InsuredPartyT.RespTypeID = 2) AS InsParty2 ON PatientsT.PersonID = InsParty2.PatientID "
				"WHERE PatientsT.PersonID = %li", m_nPatientID);
		if(!tmpRS->eof) {
			//filter the list of Ins.Co's to be only these two.
			_variant_t var1, var2;
			var1 = tmpRS->Fields->Item["GuarantorID1"]->Value;
			var2 = tmpRS->Fields->Item["GuarantorID2"]->Value;

			IRowSettingsPtr pRow;
			for(int i=0;i<m_PayRespCombo->GetRowCount();i++) {
				pRow = m_PayRespCombo->GetRow(i);
				if(var1.vt == VT_I4 && pRow->GetValue(0).lVal == var1.lVal)
					pRow->PutForeColor(RGB(192,0,0));
				else if(var2.vt == VT_I4 && pRow->GetValue(0).lVal == var2.lVal)
					pRow->PutForeColor(RGB(0,0,128));
				else if(VarLong(pRow->GetValue(2),-1) == 999999) 
					pRow->PutForeColor(RGB(127,127,127));
			}
		}
	}NxCatchAll("Error in setting insurance colors.");
}

void CFinancialLineItemPostingDlg::OnSelChosenPayInsRespCombo(long nRow) 
{
	try {

		if(nRow == -1) {
			m_PayRespCombo->SetSelByColumn(0,(long)m_nInsuredPartyID);
			return;
		}

		long nNewInsuredPartyID = VarLong(m_PayRespCombo->GetValue(nRow,0),-1);
		
		if(nNewInsuredPartyID != m_nInsuredPartyID) {

			if(IDNO == MessageBox("If you change the insurance responsibility, the data in the charge list will be reloaded.\n"
				"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {

				m_PayRespCombo->SetSelByColumn(0,(long)m_nInsuredPartyID);
				return;
			}

			m_nInsuredPartyID = nNewInsuredPartyID;

			// (j.jones 2012-08-08 11:17) - PLID 47778 - m_bIsPrimaryIns is true if the current insured party
			// is the patient's primary insurance, either for medical or for vision
			m_bIsPrimaryIns = IsInsuredPartySentAsPrimary(m_nInsuredPartyID);

			// (j.jones 2012-05-08 09:29) - PLID 37165 - clear our adjustment info
			ClearAdjustmentPointers();

			m_PostingList->Clear();
			LoadPostingList();
		}

		// (j.jones 2008-07-11 15:36) - PLID 28756 - PostSelChosen will alter the screen accordingly
		PostSelChosenPayInsRespCombo();		

		// (j.jones 2008-07-11 14:58) - PLID 28756 - set our default category and description
		TrySetDefaultInsuranceDescriptions();

		// (j.jones 2010-09-02 17:46) - PLID 40395 - moved the selection of the shift resp. combo
		// here from OnInitDialog, so we can re-select the next highest resp (if needed)
		// when the payment resp changes
		long DefEOBShiftAction = GetRemotePropertyInt("DefEOBShiftAction",2,0,"<None>",true);
		if(DefEOBShiftAction == 0) //do not shift
			m_ShiftRespCombo->SetSelByColumn(srccID,(long)-1);
		else if(DefEOBShiftAction == 1) //shift to patient
			m_ShiftRespCombo->SetSelByColumn(srccID,(long)0);
		else {
			//select next highest resp, or patient
			long nCurRespTypeID = GetInsuranceTypeFromID(m_nInsuredPartyID);
			long nCurRow = m_ShiftRespCombo->FindByColumn(srccID, nCurRespTypeID, 0, FALSE);

			// (j.jones 2014-06-27 11:07) - PLID 62548 - nCurRow may be -1
			long nCurCategoryID = -1;
			if (nCurRow != -1) {
				nCurCategoryID = VarLong(m_ShiftRespCombo->GetValue(nCurRow, srccCategoryTypeID), -1);
			}

			// (j.jones 2010-09-02 16:47) - PLID 40392 - we need the next resp that's the same category
			// as our current resp, can't auto-shift across categories!

			//NOTE: This code is completely dependent on the fact that
			//this dropdown is not sortable by the user.

			long nNextRow = nCurRow + 1;
			if(nNextRow < m_ShiftRespCombo->GetRowCount() && nCurCategoryID != -1) {				
				nNextRow = m_ShiftRespCombo->FindByColumn(srccCategoryTypeID, nCurCategoryID, nNextRow, TRUE);
				if(nNextRow <= nCurRow) {
					//find by column loops around to the front of the list again,
					//we don't want that
					nNextRow = -1;
				}
			}
			else {
				nNextRow = -1;
			}
			if(nNextRow == -1) {
				//next resp. does not exist, so shift to patient
				m_ShiftRespCombo->SetSelByColumn(srccID,(long)0);
			}
		}

	}NxCatchAll("Error switching insurance responsibility.");
}

// (j.jones 2015-10-26 11:14) - PLID 67451 - added function to determine if one of the above enums
// is an adjustment, to reduce repetition of enum comparisons
bool CFinancialLineItemPostingDlg::IsPaymentTypeAdjustment(EPaymentType ePaymentType)
{
	switch (ePaymentType) {
		case eAdjustment:
		case eChargebackAdjustment:
		case eCapitationOverpaymentAdjustment:
			return true;
		default:
			return false;
	}
}

// (j.jones 2012-05-09 09:50) - PLID 37165 - added GroupCodeID and ReasonCodeID, if not -1 they will save
// on adjustments instead of the selected group/reason on the dialog
// (j.jones 2014-07-01 11:44) - PLID 62553 - added an enumeration for whether this is a payment, adjustment,
// chargeback payment, or chargeback adjustment
long CFinancialLineItemPostingDlg::CreatePayment(COleCurrency cyPaymentAmt, EPaymentType ePaymentType, long nInsuredPartyID /* = -1*/, long nBatchPaymentID /*= -1*/, long nGroupCodeID /*= -1*/, long nReasonCodeID /*= -1*/) {

	long nLocationID = GetCurrentLocationID();
	long PaymentGroupID = 0;
	long ProviderID = -1;
	// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
	long nPayMethod = 2, nCreditCardID =-1;
	CString strProviderID, strDescription, strAmount, strDate,
		strCheckNo, strBankName, strCheckingAcct, strBankRoutingNumber,
		strCardName, strExpDate = "NULL", strAuthNumber,
		strCCProcessType = "NULL";
	COleDateTime dt;

	if(m_LocationCombo->GetCurSel() != -1)
		nLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0));
	
	if (ePaymentType == ePayment) {
		if(m_PayCatCombo->GetCurSel() != -1) {
			PaymentGroupID = VarLong(m_PayCatCombo->GetValue(m_PayCatCombo->GetCurSel(), cccID));
		}
		GetDlgItemText(IDC_PAY_DESC, strDescription);
	}
	else if (ePaymentType == eAdjustment) {
		if(m_AdjCatCombo->GetCurSel() != -1) {
			PaymentGroupID = VarLong(m_AdjCatCombo->GetValue(m_AdjCatCombo->GetCurSel(), cccID));
		}
		GetDlgItemText(IDC_ADJ_DESC, strDescription);
	}
	// (j.jones 2014-07-01 11:46) - PLID 62553 - chargebacks use the same chargeback category and description
	else if (ePaymentType == eChargebackPayment || ePaymentType == eChargebackAdjustment) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_ChargebackCategoryCombo->GetCurSel();
		if (pRow) {
			PaymentGroupID = VarLong(pRow->GetValue(cccID));
		}
		GetDlgItemText(IDC_CB_DESC, strDescription);
	}
	// (j.jones 2015-10-26 11:11) - PLID 67451 - added version for capitation overpayment adjustment
	else if (ePaymentType == eCapitationOverpaymentAdjustment) {		
		strDescription = "Capitation Overpayment";
		PaymentGroupID = AutoCreatePaymentCategory(strDescription);
	}

	if(m_ProviderCombo->GetCurSel() != -1)
		ProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(),0));

	strProviderID = "NULL";

	if(ProviderID != -1) {
		strProviderID.Format("%li",ProviderID);
	}

	// (j.jones 2005-02-02 11:03) - PLID 14862 - if no provider is selected on the 
	// batch payment, attempt to assign the charge provider to the child payment, but
	// only if we are applying to one charge or all the charges we are applying to have
	// the same provider
	// (j.jones 2011-07-08 14:37) - PLID 18687 - now we have a preference to always use the charge provider
	// if we are applying to a charge, but only do this on payments, not adjustments
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
	// (j.jones 2014-07-01 11:49) - PLID 62553 - supported chargebacks
	// (j.jones 2015-10-26 11:14) - PLID 67451 - added function to determine if one of the above enums
	// is an adjustment, to reduce repetition of enum comparisons
	if(ProviderID == -1 || (!IsPaymentTypeAdjustment(ePaymentType) && GetRemotePropertyInt("BatchPayUseChargeProviderOnApplies", 1, 0, "<None>", true) == 1)) {

		long nProvIDToUse = -2;

		for(int i=0;i<m_PostingList->GetRowCount() && nProvIDToUse < 0; i++) {

			IRowSettingsPtr pRow = m_PostingList->GetRow(i);			
			long nChargeID = VarLong(pRow->GetValue(plcChargeID),-1);
			long nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID),-1);
			
			// (j.jones 2011-12-07 09:01) - PLID 18687 - If we don't have a valid nProvIDToUse,
			// try to use the charge provider. This will use the first valid charge provider we find.
			// This payment can apply to multiple charges, it is not broken down, so it can't handle
			// cases where the bill has multiple providers.
			if(nProvIDToUse < 0) {
				_RecordsetPtr rsDoc = CreateParamRecordset("SELECT DoctorsProviders FROM ChargesT WHERE ID = {INT}",nChargeID);
				if(!rsDoc->eof) {
					nProvIDToUse = AdoFldLong(rsDoc, "DoctorsProviders",-1);
				}
				rsDoc->Close();
			}
		}

		if(nProvIDToUse > -1) {
			strProviderID.Format("%li",nProvIDToUse);
		}
	}

	// (j.jones 2011-07-14 16:43) - PLID 18686 - added a preference to always use the charge location
	// if we are applying to a charge, but only on payments
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
	// (j.jones 2014-07-01 11:49) - PLID 62553 - supported chargebacks
	// (j.jones 2015-10-26 11:14) - PLID 67451 - added function to determine if one of the above enums
	// is an adjustment, to reduce repetition of enum comparisons
	if (!IsPaymentTypeAdjustment(ePaymentType) && GetRemotePropertyInt("BatchPayUseChargeLocationOnApplies", 1, 0, "<None>", true) == 1 && m_PostingList->GetRowCount() > 0) {

		IRowSettingsPtr pRow = m_PostingList->GetRow(0);
		long nChargeID = VarLong(pRow->GetValue(plcChargeID),-1);
			
		_RecordsetPtr rsLoc = CreateParamRecordset("SELECT LocationID FROM LineItemT WHERE ID = {INT}", nChargeID);
		if(!rsLoc->eof) {
			nLocationID = AdoFldLong(rsLoc, "LocationID");
		}
		rsLoc->Close();
	}

	if (ePaymentType == ePayment || ePaymentType == eChargebackPayment) {
		GetDlgItemText(IDC_CHECK_NO,strCheckNo);
		GetDlgItemText(IDC_BANK_NAME,strBankName);
		GetDlgItemText(IDC_BANK_NUMBER,strBankRoutingNumber);
		GetDlgItemText(IDC_ACCOUNT_NUMBER,strCheckingAcct);

		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
		nCreditCardID = m_PayCardTypeCombo->CurSel == -1 ? -1 : VarLong(m_PayCardTypeCombo->GetValue(m_PayCardTypeCombo->GetCurSel(),cccCardID));
		GetDlgItemText(IDC_CC_NAME_ON_CARD, strCardName);
		CString strExpirationDate = "";
		GetDlgItemText(IDC_CC_EXP_DATE, strExpirationDate);
		GetDlgItemText(IDC_EDIT_AUTHORIZATION, strAuthNumber);
		
		//(e.lally 2008-11-19) PLID 32096 - We only need the expiration date if the method of payment is credit card.
		strExpDate = "NULL";
		long nMethodOfPayment = -1;
		if(m_PayMethodCombo->GetCurSel() >=0){
			nMethodOfPayment = VarLong(m_PayMethodCombo->GetValue(m_PayMethodCombo->GetCurSel(),0),2);
		}

		// (j.jones 2015-09-30 11:03) - PLID 67181 - line item posting always sets CCProcessType as Other (Do not process)
		if (nMethodOfPayment == 3 && ePaymentType == ePayment) {
			strCCProcessType = AsString((long)LineItem::CCProcessType::DoNotProcess);
		}

		if(nMethodOfPayment == 3 && !strExpirationDate.IsEmpty())
		{
			strExpDate = strExpirationDate;
			COleDateTime dt;
			//set the date to be the last date of the exp. month
			CString month, year;
			month = strExpDate.Left(strExpDate.Find("/",0));
			year = "20" + strExpDate.Right(strExpDate.Find("/",0));
			if(month=="12") {
				//the method we use to store dates acts funky with December, so
				//we cannot just increase the month by 1. However, we know the last
				//day in December is always 31, so it's an easy fix.
				dt.SetDate(atoi(year),atoi(month),31);
			}
			else {
				//this method works well for all other months. Set the date to be
				//the first day of the NEXT month, then subtract one day.
				//The result will always be the last day of the month entered.
				COleDateTimeSpan dtSpan;
				dtSpan.SetDateTimeSpan(1,0,0,0);
				dt.SetDate(atoi(year),atoi(month)+1,1);
				//(e.lally 2008-11-19) PLID 32096 - Don't attempt the new date if the original is invalid
				if(dt.m_status == COleDateTime::valid){
					dt = dt - dtSpan;
				}
			}
			
			//(e.lally 2008-11-19) PLID 32096 - Compare against not valid
			if (dt.m_status != COleDateTime::valid) {
				strExpDate = "Null";
			}
			else{
				strExpDate = FormatDateTimeForSql(dt, dtoDate);
				strExpDate = CString("'") + _Q(strExpDate) + "'";
			}
		}
	}

	// (j.jones 2014-07-01 09:16) - PLID 62549 - if a vision payment, all adjustments
	// or chargebacks use the payment date, as the adjustment date is hidden
	// (j.jones 2015-10-26 11:18) - PLID 67451 - capitation overpayment adjustments also use the payment date
	if(ePaymentType != eAdjustment || m_ePayType == eVisionPayment)
		dt = m_dtPayDate.GetValue().date;
	else
		dt = m_dtAdjDate.GetValue().date;
	strDate = FormatDateTimeForSql(dt, dtoDate);

	CString strBatchPaymentID = "NULL";
	// (j.jones 2015-10-26 11:18) - PLID 67451 - capitation overpayment adjustments do track the batch payment ID
	if (nBatchPaymentID != -1 && ePaymentType != eChargebackAdjustment) { // (b.eyers 2015-10-16) - PLID 67357 - removed && ePaymentType != eAdjustment
		strBatchPaymentID.Format("%li",nBatchPaymentID);
	}

	strAmount = FormatCurrencyForSql(cyPaymentAmt);

	CString strCashReceived = "NULL";

	// (j.jones 2015-10-26 11:14) - PLID 67451 - added function to determine if one of the above enums
	// is an adjustment, to reduce repetition of enum comparisons
	if (IsPaymentTypeAdjustment(ePaymentType))
		nPayMethod = 0;
	else {
		if(m_PayMethodCombo->GetCurSel() != -1) {
			nPayMethod = VarLong(m_PayMethodCombo->GetValue(m_PayMethodCombo->GetCurSel(),0),2);
			COleCurrency cyCashReceived = COleCurrency(0,0);
			if(nPayMethod == 1) {
				//now get the receive amount
				GetDlgItemText(IDC_CASH_RECEIVED_LIP, strCashReceived);

				if (strCashReceived.GetLength() == 0) {			
					MsgBox("Please fill in the 'Cash Received' box.");
					return FALSE;
				}

				cyCashReceived = ParseCurrencyFromInterface(strCashReceived);
				if(cyCashReceived.GetStatus() == COleCurrency::invalid) {
					SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
					MsgBox("Please enter a valid amount in the 'Cash Received' box.");			
					return FALSE;
				}
				
				//see how much the regional settings allows to the right of the decimal
				CString strICurr;
				NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
				int nDigits = atoi(strICurr);
				CString strDecimal;
				NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
				if(cyCashReceived.Format().Find(strDecimal) != -1 && cyCashReceived.Format().Find(strDecimal) + (nDigits+1) < cyCashReceived.Format().GetLength()) {
					SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
					MsgBox("Please fill only %li places to the right of the %s in the 'Cash Received' box.",nDigits,strDecimal == "," ? "comma" : "decimal");			
					return FALSE;
				}

				if(cyCashReceived < cyPaymentAmt) {
					//reset it to the payment amount
					SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(cyPaymentAmt,FALSE,TRUE));
					SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
					MsgBox("You cannot enter an amount less than the payment amount in the 'Cash Received' box.");			
					return FALSE;
				}

				strCashReceived.Format("Convert(money,'%s')",FormatCurrencyForSql(cyCashReceived));
			}			
		}
	}

	CString strGroupCodeID = "NULL", strReasonCodeID = "NULL";
	// (a.walling 2006-11-15 12:04) - PLID 23552 - Get the reason and group code strings
	// (j.jones 2010-09-23 15:04) - PLID 40653 - these are now IDs
	// (j.jones 2014-07-02 14:58) - PLID 62548 - vision adjustments do not fill group or reason code
	// (j.jones 2015-10-26 11:18) - PLID 67451 - capitation overpayment adjustments don't fill these either
	if (ePaymentType == eAdjustment && m_ePayType != eVisionPayment) {
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		// (j.jones 2012-05-09 09:50) - PLID 37165 - added GroupCodeID and ReasonCodeID as optional parameters,
		// if not -1 they will override the selected group/reason on the dialog
		long nGroupCodeIDToUse = nGroupCodeID;
		long nReasonCodeIDToUse = nReasonCodeID;

		if(nGroupCodeIDToUse == -1) {
			pRow = m_pGroupCodeList->GetCurSel();
			if (pRow) {
				nGroupCodeIDToUse = VarLong(pRow->GetValue(gcccID), -1);
			}
		}
		if(nReasonCodeIDToUse == -1) {
			pRow = m_pReasonList->GetCurSel();
			if (pRow) {
				nReasonCodeIDToUse = VarLong(pRow->GetValue(rcccID), -1);
			}
		}

		if(nGroupCodeIDToUse != -1) {
			strGroupCodeID.Format("%li", nGroupCodeIDToUse);
		}
		if(nReasonCodeIDToUse != -1) {
			strReasonCodeID.Format("%li", nReasonCodeIDToUse);
		}
	}

	//(e.lally 2006-08-10) PLID 21910 - We need to format the check no., and we should add these queries to a batch
	//in case one fails.
	CString strBatchSql = BeginSqlBatch();
	// (j.dinatale 2012-02-07 15:10) - PLID 51181 - batch the new numbers calls in the sql
	// (d.singleton 2012-07-26 15:32) - PLID 51835 switch the coalesce with max
	// (j.dinatale 2012-08-08 11:35) - PLID 52032 - fixed an issue with PaymentUniqueID and the possibility of a warning being thrown due to a NULL value being eliminated by the aggregate function
	AddStatementToSqlBatch(strBatchSql,
		"SET NOCOUNT ON \r\n"
		"DECLARE @nLineItemID INT; \r\n"
		"DECLARE @nPaymentUniqueID INT; \r\n");

	// (j.jones 2011-09-29 14:55) - PLID 45500 - ensured that InputDate uses GetDate(), NOT BatchPaymentsT.InputDate
	// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
	// (j.jones 2015-10-26 11:14) - PLID 67451 - added function to determine if one of the above enums
	// is an adjustment, to reduce repetition of enum comparisons
	AddStatementToSqlBatch(strBatchSql, 
		"INSERT INTO LineItemT (PatientID, LocationID, Type, Date, InputDate, InputName, Amount, Description) VALUES "
		"(%li, %li, %li, '%s', GetDate(), '%s', Convert(money,'%s'), '%s')",
		m_nPatientID, nLocationID, (IsPaymentTypeAdjustment(ePaymentType) ? 2 : 1), _Q(strDate), _Q(GetCurrentUserName()), _Q(strAmount), _Q(strDescription));

	AddStatementToSqlBatch(strBatchSql,
		"SET @nLineItemID = SCOPE_IDENTITY() \r\n");

	// (j.armen 2013-06-29 15:34) - PLID 57375 - PaymentsT.PaymentUniqueID now gets it's ID from an identity seeded table
	// (j.jones 2015-10-26 11:14) - PLID 67451 - added function to determine if one of the above enums
	// is an adjustment, to reduce repetition of enum comparisons
	AddStatementToSqlBatch(strBatchSql, CSqlFragment(
		"IF {BIT} = 1 -- Is Adjustment\r\n"
		"BEGIN\r\n"
		"	SET @nPaymentUniqueID = NULL\r\n"
		"END\r\n"
		"ELSE -- Is Payment\r\n"
		"BEGIN\r\n"
		"	INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
		"	SET @nPaymentUniqueID = SCOPE_IDENTITY()\r\n"
		"END\r\n", IsPaymentTypeAdjustment(ePaymentType)
		).Flatten());

	// (a.walling 2006-11-16 10:32) - PLID 23552 - Insert group code and reason code into the new payment (they will be empty '' if none or not an adjustment)
	// (j.jones 2010-09-23 15:04) - PLID 40653 - reason & group code are now nullable IDs
	// (j.jones 2015-09-30 11:03) - PLID 67181 - supported CCProcessType
	AddStatementToSqlBatch(strBatchSql, "INSERT INTO PaymentsT (ID, InsuredPartyID, ProviderID, PaymentGroupID, PayMethod, BatchPaymentID, CashReceived, "
		"PaymentUniqueID, GroupCodeID, ReasonCodeID, CCProcessType) "
		"VALUES (@nLineItemID,%li,%s,%li,%li,%s,%s,@nPaymentUniqueID, %s, %s, %s)", 
		nInsuredPartyID, strProviderID, PaymentGroupID, nPayMethod, strBatchPaymentID, strCashReceived, strGroupCodeID, strReasonCodeID, strCCProcessType);

	// (j.jones 2015-10-26 11:14) - PLID 67451 - added function to determine if one of the above enums
	// is an adjustment, to reduce repetition of enum comparisons
	if (!IsPaymentTypeAdjustment(ePaymentType)) {
		// (e.lally 2007-07-09) PLID 25993 - Credit cards link on ID now instead of a text name
		CString strCreditCardID = "NULL";
		if(nCreditCardID > 0)
			strCreditCardID.Format("%li", nCreditCardID);

		// (j.jones 2015-09-30 11:04) - PLID 67175 - supported Last 4, we no longer store full CC numbers in this dialog
		CString strLast4;
		m_nxeditCCLast4.GetWindowText(strLast4);

		AddStatementToSqlBatch(strBatchSql, "INSERT INTO PaymentPlansT (ID, CheckNo, BankNo, CheckAcctNo, BankRoutingNum, "
			"CreditCardID, CCNumber, CCHoldersName, CCExpDate, CCAuthNo) "
			"VALUES (@nLineItemID, '%s', '%s', '%s', '%s', "
			"%s, '%s', '%s', %s, '%s') \r\n",
			_Q(strCheckNo), _Q(strBankName), _Q(strCheckingAcct), _Q(strBankRoutingNumber), 
			strCreditCardID, _Q(strLast4), _Q(strCardName), strExpDate, _Q(strAuthNumber));
	}
	else
		AddStatementToSqlBatch(strBatchSql, "INSERT INTO PaymentPlansT (ID) VALUES (@nLineItemID) \r\n");

	// (j.dinatale 2012-02-07 17:29) - PLID 51181 - grab the line item we just inserted for auditing and to return
	AddStatementToSqlBatch(strBatchSql,
		"SELECT @nLineItemID AS LineItemID \r\n"
		"SET NOCOUNT OFF \r\n\r\n");

	_RecordsetPtr rsLineItem = CreateRecordset("%s", strBatchSql);

	long iLineItemID = -1;
	if(!rsLineItem->eof){
		iLineItemID = AdoFldLong(rsLineItem, "LineItemID");
	}

	{
		long nAuditID = BeginNewAuditEvent();
		CString strAuditDesc = FormatCurrencyForInterface(cyPaymentAmt, TRUE, TRUE);
		AuditEventItems aei = aeiPaymentCreated;
		bool bAudit = true;
		// (j.jones 2014-07-01 14:45) - PLID 62553 - added audits for chargebacks, we only need one
		// for the payment/adjustment combination so if this is the chargeback adjustment entry,
		// don't bother auditing twice
		if (ePaymentType == ePayment) {
			aei = aeiPaymentCreated;
			strAuditDesc += " Payment";
		}
		// (j.jones 2015-10-26 11:18) - PLID 67451 - added capitation overpayment adjustments
		else if (ePaymentType == eAdjustment || ePaymentType == eCapitationOverpaymentAdjustment) {
			aei = aeiAdjustmentCreated;
			strAuditDesc += " Adjustment";
		}
		else if (ePaymentType == eChargebackPayment) {
			aei = aeiChargebackCreated;
			ASSERT(cyPaymentAmt < COleCurrency(0, 0));
			strAuditDesc += " Chargeback";
		}
		else {
			//chargeback adjustments won't audit, because the aeiChargebackCreated covers both line items
			bAudit = false;
		}

		if (bAudit) {
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditID, aei, iLineItemID, "", strAuditDesc, aepHigh, aetCreated);

			// (j.jones 2015-09-30 11:03) - PLID 67181 - line item posting always sets CCProcessType as Other (Do not process),
			// but we only audit this if ICCP is enabled, to be consistent with the payment dialog
			if (nPayMethod == 3 && ePaymentType == ePayment && IsICCPEnabled()) {
				CString strAuditDesc;
				strAuditDesc.Format("%s Payment: Other (Do not process)", FormatCurrencyForInterface(cyPaymentAmt));
				AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditID, aeiPaymentCCProcessType, iLineItemID, "", strAuditDesc, aepHigh, aetCreated);
			}
		}
	}

	if(strBatchPaymentID != "NULL")
		AutoUpdateBatchPaymentDepositDates(nBatchPaymentID);

	return iLineItemID;
}

// (j.jones 2015-10-26 11:20) - PLID 67451 - added function to auto-create payment categories
long CFinancialLineItemPostingDlg::AutoCreatePaymentCategory(CString strCategoryName)
{
	//throw exceptions to the caller

	// (j.jones 2015-10-26 11:22) - PLID 67451 - this just moved existing code for vision chargebacks
	// to a modular function, as capitation overpayments also need the same logic

	long nCategoryID = -1;

	_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM PaymentGroupsT WHERE GroupName = {STRING}", strCategoryName);
	if (!rs->eof) {
		nCategoryID = VarLong(rs->Fields->Item["ID"]->Value);
	}
	else {
		//create this category
		rs->Close();
		rs = CreateParamRecordset("SET NOCOUNT ON \r\n"
			"DECLARE @nNewID INT \r\n"
			"SET @nNewID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM PaymentGroupsT) \r\n"
			"INSERT INTO PaymentGroupsT (ID, GroupName, Explanation) VALUES (@nNewID, {STRING}, '') \r\n"
			"SET NOCOUNT OFF \r\n"
			"SELECT @nNewID AS NewID", strCategoryName);

		if (!rs->eof) {
			nCategoryID = VarLong(rs->Fields->Item["NewID"]->Value);
			CClient::RefreshTable(NetUtils::PaymentGroupsT);
		}
		else {
			//should be impossible
			ThrowNxException("Could not create category for %s.", strCategoryName);
		}

		//add it to our three lists
		NXDATALISTLib::IRowSettingsPtr pRow = m_PayCatCombo->GetRow(-1);
		pRow->PutValue(cccID, long(nCategoryID));
		pRow->PutValue(cccCategory, _bstr_t(strCategoryName));
		m_PayCatCombo->AddRow(pRow);

		pRow = m_AdjCatCombo->GetRow(-1);
		pRow->PutValue(cccID, long(nCategoryID));
		pRow->PutValue(cccCategory, _bstr_t(strCategoryName));
		m_AdjCatCombo->AddRow(pRow);

		NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_ChargebackCategoryCombo->GetNewRow();
		pRow2->PutValue(cccID, long(nCategoryID));
		pRow2->PutValue(cccCategory, _bstr_t(strCategoryName));
		m_ChargebackCategoryCombo->AddRowSorted(pRow2, NULL);
	}
	rs->Close();

	return nCategoryID;
}

short CFinancialLineItemPostingDlg::ColumnToEdit(BOOL bStartOnLeft) {

	if(bStartOnLeft) {
		for(short nCol = 0; nCol < m_PostingList->GetColumnCount(); nCol++) {
			IColumnSettingsPtr pCol = m_PostingList->GetColumn(nCol);
			if((pCol->GetColumnStyle() & csEditable) == csEditable) {
				return nCol;
			}
		}
	}
	else {
		for(short nCol = m_PostingList->GetColumnCount() - 1; nCol >= 0; nCol--) {
			IColumnSettingsPtr pCol = m_PostingList->GetColumn(nCol);
			if((pCol->GetColumnStyle() & csEditable) == csEditable) {
				return nCol;
			}
		}
	}

	return -1;
}

void CFinancialLineItemPostingDlg::AutoFillAdjustmentTotal()
{
	try {
		COleCurrency cyTotal = COleCurrency(0,0);
		for(int i = 0; i < m_PostingList->GetRowCount(); i++) {
			cyTotal += VarCurrency(m_PostingList->GetValue(i, m_nAdjustmentColIndex), COleCurrency(0,0));
		}
		SetDlgItemText(IDC_ADJ_TOTAL, FormatCurrencyForInterface(cyTotal, FALSE, TRUE));

		CalculateTotals();

	}NxCatchAll("Error in CFinancialLineItemPostingDlg::AutoFillAdjustmentTotal()");
}

void CFinancialLineItemPostingDlg::OnSelChosenPostingInsuranceCoList(long nRow) 
{
	try {
		if(nRow != -1 && VarLong(m_BillInsuranceCombo->GetValue(nRow, ipccID),-1) == -1) {
			m_BillInsuranceCombo->CurSel = -1;
		}
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnSelChosenPostingOtherInsuranceCoList(long nRow) 
{
	try {
		if(nRow != -1 && VarLong(m_BillOtherInsuranceCombo->GetValue(nRow, ipccID),-1) == -1) {
			m_BillOtherInsuranceCombo->CurSel = -1;
		}
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnKillfocusCashReceived() 
{
	try {

		CString strCashReceived, strPaymentTotal;

		//first get the payment amount
		GetDlgItemText(IDC_PAY_TOTAL, strPaymentTotal);
		if (strPaymentTotal.GetLength() == 0) {
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please enter a valid amount in the Payment's 'Total Amount' box.");			
			return;			
		}

		COleCurrency cyPaymentTotal = ParseCurrencyFromInterface(strPaymentTotal);
		if(cyPaymentTotal.GetStatus() == COleCurrency::invalid) {
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please enter a valid amount in the Payment's 'Total Amount' box.");			
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cyPaymentTotal.Format().Find(strDecimal) != -1 && cyPaymentTotal.Format().Find(strDecimal) + (nDigits+1) < cyPaymentTotal.Format().GetLength()) {
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please fill only %li places to the right of the %s in the Payment's 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");			
			return;
		}

		//now get the receive amount
		GetDlgItemText(IDC_CASH_RECEIVED_LIP, strCashReceived);
		if (strCashReceived.GetLength() == 0) {
			//re-fill with the total, as this cannot be blank
			strCashReceived = FormatCurrencyForInterface(cyPaymentTotal,FALSE,TRUE);
			SetDlgItemText(IDC_CASH_RECEIVED_LIP,strCashReceived);
		}

		COleCurrency cyCashReceived = ParseCurrencyFromInterface(strCashReceived);
		if(cyCashReceived.GetStatus() == COleCurrency::invalid) {
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please enter a valid amount in the 'Cash Received' box.");			
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		if(cyCashReceived.Format().Find(strDecimal) != -1 && cyCashReceived.Format().Find(strDecimal) + (nDigits+1) < cyCashReceived.Format().GetLength()) {
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please fill only %li places to the right of the %s in the 'Cash Received' box.",nDigits,strDecimal == "," ? "comma" : "decimal");			
			return;
		}

		if(cyCashReceived < cyPaymentTotal) {
			//reset it to the payment amount
			SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(cyPaymentTotal,FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("You cannot enter an amount less than the payment amount in the 'Cash Received' box.");			
			return;
		}

		SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(cyCashReceived,FALSE,TRUE));

		//if we get here, we have valid amounts, so calculate away!

		COleCurrency cyChangeGiven = cyCashReceived - cyPaymentTotal;
		CString strChangeGiven = FormatCurrencyForInterface(cyChangeGiven,FALSE,TRUE);
		SetDlgItemText(IDC_CHANGE_GIVEN_LIP,strChangeGiven);

	}NxCatchAll("Error calculating change due.");
}

void CFinancialLineItemPostingDlg::OnKillfocusPayTotal() 
{
	try {

		COleCurrency cyTotalPaymentAmountEntered;
		CString strPayment;
		GetDlgItemText(IDC_PAY_TOTAL, strPayment);
		if (strPayment.GetLength() == 0) {
			SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			//don't warn them yet, this should be OK for now
			//MsgBox("Please fill in the Payment's 'Total Amount' box.");
			return;
		}

		cyTotalPaymentAmountEntered = ParseCurrencyFromInterface(strPayment);
		if(cyTotalPaymentAmountEntered.GetStatus() == COleCurrency::invalid) {
			SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please enter a valid amount in the Payment's 'Total Amount' box.");
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cyTotalPaymentAmountEntered.Format().Find(strDecimal) != -1 && cyTotalPaymentAmountEntered.Format().Find(strDecimal) + (nDigits+1) < cyTotalPaymentAmountEntered.Format().GetLength()) {
			SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN_LIP,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please fill only %li places to the right of the %s in the Payment's 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			return;
		}

		SetDlgItemText(IDC_PAY_TOTAL,FormatCurrencyForInterface(cyTotalPaymentAmountEntered,FALSE,TRUE));

		//If we changed the amount to be greater than the cash received,
		//update the cash received box to be the same
		//(note: this of course would make no sense - but does protect us from altering things
		//unnecessarily during an OnKillFocus)
		CString strCashReceived;
		GetDlgItemText(IDC_CASH_RECEIVED_LIP, strCashReceived);
		if(strCashReceived.GetLength() > 0) {
			COleCurrency cyCashReceived = ParseCurrencyFromInterface(strCashReceived);
			if(cyCashReceived.GetStatus() != COleCurrency::invalid) {
				if(cyCashReceived < cyTotalPaymentAmountEntered)
					SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(cyTotalPaymentAmountEntered,FALSE,TRUE));
			}
		}
		else {
			SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(cyTotalPaymentAmountEntered,FALSE,TRUE));
		}
		
		OnKillfocusCashReceived();

		//if there is one charge, and the payment column is not filled in,
		//then fill it in with this payment amount, up to the available amount to pay
		//also only do this when not a batch payment
		if(m_nBatchPaymentID == -1 && m_PostingList->GetRowCount() == 1) {
			long nInsuredPartyID = -1;
			if(m_PayRespCombo->GetCurSel() != -1)
				nInsuredPartyID = VarLong(m_PayRespCombo->GetValue(m_PayRespCombo->GetCurSel(),0),-1);

			if(nInsuredPartyID == -1) {
				COleCurrency cyPatBal = VarCurrency(m_PostingList->GetValue(0,m_nPatBalanceColIndex),COleCurrency(0,0));
				COleCurrency cyPatPays = VarCurrency(m_PostingList->GetValue(0,m_nNewPaysColIndex),COleCurrency(0,0));
				if(cyPatPays == COleCurrency(0,0)) {
					//it's $0.00, so let's do it!
					if(cyTotalPaymentAmountEntered <= cyPatBal) {
						m_PostingList->PutValue(0,m_nNewPaysColIndex,_variant_t(cyTotalPaymentAmountEntered));
						//make sure the totals get updated
						OnEditingFinishedLineItemPostingList(0, m_nNewPaysColIndex, _variant_t(cyPatPays), _variant_t(cyTotalPaymentAmountEntered), TRUE);
					}
					else {
						m_PostingList->PutValue(0,m_nNewPaysColIndex,_variant_t(cyPatBal));
						//make sure the totals get updated
						OnEditingFinishedLineItemPostingList(0, m_nNewPaysColIndex, _variant_t(cyPatPays), _variant_t(cyPatBal), TRUE);
					}
				}
			}
			else {
				COleCurrency cyInsBal = VarCurrency(m_PostingList->GetValue(0,m_nInsBalanceColIndex),COleCurrency(0,0));
				COleCurrency cyInsPays = VarCurrency(m_PostingList->GetValue(0,m_nNewPaysColIndex),COleCurrency(0,0));
				if(cyInsPays == COleCurrency(0,0)) {
					//it's $0.00, so let's do it!
					if(cyTotalPaymentAmountEntered <= cyInsBal) {
						m_PostingList->PutValue(0,m_nNewPaysColIndex,_variant_t(cyTotalPaymentAmountEntered));
						OnEditingFinishedLineItemPostingList(0, m_nNewPaysColIndex, _variant_t(cyInsPays), _variant_t(cyTotalPaymentAmountEntered), TRUE);
					}
					else {
						m_PostingList->PutValue(0,m_nNewPaysColIndex,_variant_t(cyInsBal));
						OnEditingFinishedLineItemPostingList(0, m_nNewPaysColIndex, _variant_t(cyInsPays), _variant_t(cyInsBal), TRUE);
					}
				}
			}
		}

		CalculateTotals();

	}NxCatchAll("Error in OnKillfocusPayTotal");
}

// (z.manning 2008-06-19 11:51) - PLID 26544 - Sets the total field for the given index to the given value
void CFinancialLineItemPostingDlg::SetTotal(const short nColIndex, const COleCurrency cyTotal)
{	
	short nLabelIndex = nColIndex - plcBeginDynamicCols;
	CWnd *pwndLabel = NULL, *pwndTotal = NULL;

	// (j.jones 2014-06-27 14:50) - PLID 62631 - if a vision payment, the deductible, coinsurance, and allowable
	// columns are hidden, so we need to ensure there are no spaces in the labels
	if (m_ePayType == eVisionPayment) {

		//force the labels for the hidden columns to be the rightmost 3 labels
		if (nColIndex == m_nDeductibleAmtColIndex) {
			nLabelIndex = 10;
		}
		else if (nColIndex == m_nCoinsuranceAmtColIndex) {
			nLabelIndex = 11;
		}
		else if (nColIndex == m_nAllowableColIndex) {
			nLabelIndex = 12;
		}
		else {
			//If our current column is not hidden, but to the right of the hidden columns,
			//bring its label index forward. This may happen up to 3 times.
			if (nColIndex > m_nDeductibleAmtColIndex) {
				nLabelIndex--;
			}
			if (nColIndex > m_nCoinsuranceAmtColIndex) {
				nLabelIndex--;
			}
			if (nColIndex > m_nAllowableColIndex) {
				nLabelIndex--;
			}
		}
	}

	switch (nLabelIndex)
	{
		case 0:
			pwndLabel = &m_nxstaticTotalLabel0;
			pwndTotal = &m_nxstaticTotal0;
			break;
			
		case 1:
			pwndLabel = &m_nxstaticTotalLabel1;
			pwndTotal = &m_nxstaticTotal1;
			break;
			
		case 2:
			pwndLabel = &m_nxstaticTotalLabel2;
			pwndTotal = &m_nxstaticTotal2;
			break;
			
		case 3:
			pwndLabel = &m_nxstaticTotalLabel3;
			pwndTotal = &m_nxstaticTotal3;
			break;
			
		case 4:
			pwndLabel = &m_nxstaticTotalLabel4;
			pwndTotal = &m_nxstaticTotal4;
			break;
			
		case 5:
			pwndLabel = &m_nxstaticTotalLabel5;
			pwndTotal = &m_nxstaticTotal5;
			break;
			
		case 6:
			pwndLabel = &m_nxstaticTotalLabel6;
			pwndTotal = &m_nxstaticTotal6;
			break;
			
		case 7:
			pwndLabel = &m_nxstaticTotalLabel7;
			pwndTotal = &m_nxstaticTotal7;
			break;
			
		case 8:
			pwndLabel = &m_nxstaticTotalLabel8;
			pwndTotal = &m_nxstaticTotal8;
			break;

		// (j.jones 2010-05-05 09:44) - PLID 25521 - added one more total
		case 9:
			pwndLabel = &m_nxstaticTotalLabel9;
			pwndTotal = &m_nxstaticTotal9;
			break;

		// (b.spivey, January 05, 2012) - PLID 47121 - Two more columns for deductible and coinsurance. 
		case 10:
			pwndLabel = &m_nxstaticTotalLabel10;
			pwndTotal = &m_nxstaticTotal10;
			break; 

		case 11:
			pwndLabel = &m_nxstaticTotalLabel11;
			pwndTotal = &m_nxstaticTotal11;
			break;

		// (j.jones 2013-08-27 12:23) - PLID 57398 - added another column for copay
		case 12:
			pwndLabel = &m_nxstaticTotalLabel12;
			pwndTotal = &m_nxstaticTotal12;
			break;

		default:
			ThrowNxException("CFinancialLineItemPostingDlg::SetTotal - Invalid column index %li (dynamic label index %li)", nColIndex, nLabelIndex);
			break;
	}

	// (j.jones 2014-06-27 13:38) - PLID 62631 - If a vision payment, don't fill the label for the
	// deductible, coinsurance, or allowable totals. The code in ReloadDynamicColumns() should
	// have already forced these to be the right-most 3 columns, so we don't have gaps in the labels.
	if (m_ePayType == eVisionPayment &&
		(nColIndex == m_nDeductibleAmtColIndex || nColIndex == m_nCoinsuranceAmtColIndex || nColIndex == m_nAllowableColIndex)) {

		//we should have forced these to the last 3 spots
		ASSERT(nLabelIndex == 10 || nLabelIndex == 11 || nLabelIndex == 12);
		
		pwndLabel->SetWindowText("");
		pwndTotal->SetWindowText("");
	}
	else {
		pwndLabel->SetWindowText(m_PostingList->GetColumn(nColIndex)->GetColumnTitle());
		pwndTotal->SetWindowText(FormatCurrencyForInterface(cyTotal, TRUE, TRUE));
	}
}

void CFinancialLineItemPostingDlg::CalculateTotals() {

	try {

		//simply add up all the amounts and then display them at the bottom

		long nInsuredPartyID = -1;
		if(m_PayRespCombo->GetCurSel() != -1)
			nInsuredPartyID = VarLong(m_PayRespCombo->GetValue(m_PayRespCombo->GetCurSel(),0),-1);

		// (b.spivey, January 05, 2012) - PLID 47121 - Added Deductible/coinsurance. 
		COleCurrency cyChargeAmount = COleCurrency(0,0),
			cyPatResp = COleCurrency(0,0),
			cyInsResp = COleCurrency(0,0),
			cyExistPatPays = COleCurrency(0,0),
			cyExistInsPays = COleCurrency(0,0),
			cyNewPays = COleCurrency(0,0),
			cyChargebacks = COleCurrency(0, 0),
			cyAllowable = COleCurrency(0,0),
			cyInsAdj = COleCurrency(0,0),
			cyPatBal = COleCurrency(0,0),
			cyInsBal = COleCurrency(0,0),
			cyDeductible = COleCurrency(0,0), 
			cyCoinsurance = COleCurrency(0,0),
			cyCopay = COleCurrency(0,0);

		COleCurrency cyTotalPaymentAmtNeeded = COleCurrency(0,0);
		COleCurrency cyTotalAdjustmentAmtNeeded = COleCurrency(0,0);

		for(int i=0; i<m_PostingList->GetRowCount(); i++) {
			cyChargeAmount += VarCurrency(m_PostingList->GetValue(i,m_nChargesColIndex),COleCurrency(0,0));
			cyPatResp += VarCurrency(m_PostingList->GetValue(i,m_nPatRespColIndex),COleCurrency(0,0));
			cyInsResp += VarCurrency(m_PostingList->GetValue(i,m_nInsRespColIndex),COleCurrency(0,0));
			// (j.jones 2010-05-04 16:36) - PLID 25521 - existing pat/ins pays are now visible
			cyExistPatPays += VarCurrency(m_PostingList->GetValue(i,m_nExistingPatPaysColIndex),COleCurrency(0,0));
			cyExistInsPays += VarCurrency(m_PostingList->GetValue(i,m_nExistingInsPaysColIndex),COleCurrency(0,0));
			
			// (j.jones 2014-07-01 08:57) - PLID 62551 - negative payments update the chargeback total, not the new pays total
			COleCurrency cyPaymentAmt = VarCurrency(m_PostingList->GetValue(i, m_nNewPaysColIndex), COleCurrency(0, 0));
			if (cyPaymentAmt < COleCurrency(0, 0)) {
				cyChargebacks += cyPaymentAmt;
			}
			else {
				cyNewPays += cyPaymentAmt;
			}

			cyAllowable += VarCurrency(m_PostingList->GetValue(i,m_nAllowableColIndex),COleCurrency(0,0));
			cyInsAdj += VarCurrency(m_PostingList->GetValue(i,m_nAdjustmentColIndex),COleCurrency(0,0));
			cyPatBal += VarCurrency(m_PostingList->GetValue(i,m_nPatBalanceColIndex),COleCurrency(0,0));
			cyInsBal += VarCurrency(m_PostingList->GetValue(i,m_nInsBalanceColIndex),COleCurrency(0,0));

			//get the information to update the payment balance
			// (a.walling 2010-07-15 15:48) - PLID 39683 - This was incorrectly adding cyNewPays, which is already a running total, rather
			// than the actual new payment value, leading to accumlative errors.
			cyTotalPaymentAmtNeeded += VarCurrency(m_PostingList->GetValue(i,m_nNewPaysColIndex),COleCurrency(0,0));
			cyTotalAdjustmentAmtNeeded += VarCurrency(m_PostingList->GetValue(i,m_nAdjustmentColIndex),COleCurrency(0,0));
			// (b.spivey, January 04, 2012) - PLID 47121 - Added deductible and coinsurance totals. 
			cyDeductible += VarCurrency(m_PostingList->GetValue(i,m_nDeductibleAmtColIndex),COleCurrency(0,0));
			cyCoinsurance += VarCurrency(m_PostingList->GetValue(i,m_nCoinsuranceAmtColIndex),COleCurrency(0,0));
			// (j.jones 2013-08-27 11:41) - PLID 57398 - added copay
			cyCopay += VarCurrency(m_PostingList->GetValue(i,m_nCopayAmtColIndex),COleCurrency(0,0));
		}

		// (z.manning 2008-06-19 11:44) - PLID 26544 - These columns can now be in any order, so we
		// need to use the columns' indices to set the correct total field.
		// (j.jones 2010-05-04 16:36) - PLID 25521 - existing pat/ins pays are now visible
		SetTotal(m_nChargesColIndex, cyChargeAmount);
		SetTotal(m_nExistingPatPaysColIndex, cyExistPatPays);
		SetTotal(m_nExistingInsPaysColIndex, cyExistInsPays);
		SetTotal(m_nPatRespColIndex, cyPatResp);
		SetTotal(m_nInsRespColIndex, cyInsResp);
		SetTotal(m_nNewPaysColIndex, cyNewPays);
		SetTotal(m_nAllowableColIndex, cyAllowable);
		SetTotal(m_nAdjustmentColIndex, cyInsAdj);
		SetTotal(m_nPatBalanceColIndex, cyPatBal);
		SetTotal(m_nInsBalanceColIndex, cyInsBal);
		// (b.spivey, January 04, 2012) - PLID 47121 - Set the deductible and the coinsurance totals. 
		SetTotal(m_nDeductibleAmtColIndex, cyDeductible);
		SetTotal(m_nCoinsuranceAmtColIndex, cyCoinsurance);
		// (j.jones 2013-08-27 11:41) - PLID 57398 - added copay
		SetTotal(m_nCopayAmtColIndex, cyCopay);

		//get the payment amount entered
		COleCurrency cyTotalPaymentAmountEntered;
		CString strPayment;
		GetDlgItemText(IDC_PAY_TOTAL, strPayment);
		if (strPayment.GetLength() == 0) {
			MsgBox("Please fill in the Payment's 'Total Amount' box.");
			return;
		}

		cyTotalPaymentAmountEntered = ParseCurrencyFromInterface(strPayment);
		if(cyTotalPaymentAmountEntered.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the Payment's 'Total Amount' box.");
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cyTotalPaymentAmountEntered.Format().Find(strDecimal) != -1 && cyTotalPaymentAmountEntered.Format().Find(strDecimal) + (nDigits+1) < cyTotalPaymentAmountEntered.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the Payment's 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			return;
		}

		// (j.jones 2014-07-29 09:26) - PLID 63076 - vision payments show a 'total payments' label
		// which is the true sum of the New Pays column, counting chargebacks
		if (m_ePayType == eVisionPayment) {
			COleCurrency cyTotalPayments = cyNewPays + cyChargebacks;
			m_nxstaticTotalPayments.SetWindowText(FormatCurrencyForInterface(cyTotalPayments));
		}
		
		//now update the payment balance
		COleCurrency cyPayBalance = cyTotalPaymentAmountEntered - cyTotalPaymentAmtNeeded;		
		SetDlgItemText(IDC_PAYMENT_BALANCE,FormatCurrencyForInterface(cyPayBalance));

		//get the adjustment amount entered
		COleCurrency cyTotalAdjustmentAmountEntered;
		CString strAdjustment;
		GetDlgItemText(IDC_ADJ_TOTAL, strAdjustment);
		if (strAdjustment.GetLength() == 0) {
			MsgBox("Please fill in the Adjustment's 'Total Amount' box.");
			return;
		}

		cyTotalAdjustmentAmountEntered = ParseCurrencyFromInterface(strAdjustment);
		if(cyTotalAdjustmentAmountEntered.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the Adjustment's 'Total Amount' box.");
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		if(cyTotalAdjustmentAmountEntered.Format().Find(strDecimal) != -1 && cyTotalAdjustmentAmountEntered.Format().Find(strDecimal) + (nDigits+1) < cyTotalAdjustmentAmountEntered.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the Adjustment's 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			return;
		}
		
		//now update the adjustment balance
		// (j.jones 2014-07-24 10:31) - PLID 62550 - if a Vision Payment, this label is the Adjustment Total,
		// for Medical Payments it is the Adjustment Balance
		if (m_ePayType == eVisionPayment) {
			//these should not be different
			ASSERT(cyTotalAdjustmentAmountEntered == cyTotalAdjustmentAmtNeeded);
			
			//show the adjustment total
			SetDlgItemText(IDC_ADJUSTMENT_BALANCE, FormatCurrencyForInterface(cyTotalAdjustmentAmtNeeded));
		}
		else {
			//show the adjustment balance
			COleCurrency cyAdjBalance = cyTotalAdjustmentAmountEntered - cyTotalAdjustmentAmtNeeded;
			SetDlgItemText(IDC_ADJUSTMENT_BALANCE, FormatCurrencyForInterface(cyAdjBalance));
		}

		// (j.jones 2014-07-01 09:00) - PLID 62551 - update the chargeback total
		SetDlgItemText(IDC_CHARGEBACK_TOTAL, FormatCurrencyForInterface(cyChargebacks));

	}NxCatchAll("Error calculating totals.");
}

void CFinancialLineItemPostingDlg::OnKillfocusAdjTotal() 
{
	try {
		COleCurrency cyTotalAdjustmentAmountEntered;
		CString strAdjustment;
		GetDlgItemText(IDC_ADJ_TOTAL, strAdjustment);
		if (strAdjustment.GetLength() == 0) {
			MsgBox("Please fill in the Adjustment's 'Total Amount' box.");
			return;
		}

		cyTotalAdjustmentAmountEntered = ParseCurrencyFromInterface(strAdjustment);
		if(cyTotalAdjustmentAmountEntered.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the Adjustment's 'Total Amount' box.");
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cyTotalAdjustmentAmountEntered.Format().Find(strDecimal) != -1 && cyTotalAdjustmentAmountEntered.Format().Find(strDecimal) + (nDigits+1) < cyTotalAdjustmentAmountEntered.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the Adjustment's 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			return;
		}

		SetDlgItemText(IDC_ADJ_TOTAL,FormatCurrencyForInterface(cyTotalAdjustmentAmountEntered,FALSE,TRUE));

		CalculateTotals();
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnSelChosenPayDescriptionCombo(long nRow) 
{
	try {
		if(nRow==-1)
			return;

		CString str;
		str = CString(m_PayDescCombo->GetValue(nRow,0).bstrVal);
		m_PayDescCombo->PutComboBoxText("");
		GetDlgItem(IDC_PAY_DESC)->SetWindowText(str);
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnSelChosenAdjDescriptionCombo(long nRow) 
{
	try {
		if(nRow==-1)
			return;

		CString str;
		str = CString(m_AdjDescCombo->GetValue(nRow,0).bstrVal);
		m_AdjDescCombo->PutComboBoxText("");
		GetDlgItem(IDC_ADJ_DESC)->SetWindowText(str);
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnEditPayDesc() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 2, m_PayDescCombo, "Edit Combo Box").DoModal();

		//also requery the adjustment combo
		m_AdjDescCombo->Requery();
		// (j.jones 2014-06-30 15:18) - PLID 62642 - and the chargeback combo, if this is a vision payment
		if (m_ePayType == eVisionPayment) {
			m_ChargebackDescriptionCombo->Requery();
		}
	}NxCatchAll(__FUNCTION__);
}

void CFinancialLineItemPostingDlg::OnEditAdjDesc() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 2, m_AdjDescCombo, "Edit Combo Box").DoModal();

		//also requery the payment combo
		m_PayDescCombo->Requery();
		// (j.jones 2014-06-30 15:18) - PLID 62642 - and the chargeback combo, if this is a vision payment
		if (m_ePayType == eVisionPayment) {
			m_ChargebackDescriptionCombo->Requery();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-06-30 14:58) - PLID 62642 - added chargebacks
void CFinancialLineItemPostingDlg::OnBtnEditCbDesc()
{
	try {

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 2, m_ChargebackDescriptionCombo, "Edit Combo Box").DoModal();

		//also requery the payment and adjustment combo
		m_PayDescCombo->Requery();
		m_AdjDescCombo->Requery();

	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2006-11-15 09:42) - PLID 23552 - Enable/Disable the group and reason code controls
void CFinancialLineItemPostingDlg::EnableGroupCodesAndReasons(OPTIONAL IN bool bShow /* = true */)
{
	try {
		GetDlgItem(IDC_ADJ_GROUPCODE)->EnableWindow(bShow);
		GetDlgItem(IDC_ADJ_REASON)->EnableWindow(bShow);
		// (j.jones 2012-07-27 10:19) - PLID 26877 - added ability to filter reason codes
		GetDlgItem(IDC_BTN_FILTER_REASON_CODES)->EnableWindow(bShow);
	} NxCatchAll("Error displaying group and reason code lists");
}

// (a.walling 2006-11-15 09:42) - PLID 23552 - Initialize the group and reason code datalists with hardcoded values
// (j.jones 2010-09-23 15:07) - PLID 40653 - obsolete
//void CFinancialLineItemPostingDlg::InitGroupCodesAndReasons()

// (a.walling 2006-11-16 10:04) - PLID 23552 - Prevent no selection
void CFinancialLineItemPostingDlg::OnSelChangingReason(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			if (lpOldSel != NULL)
				lpOldSel->AddRef(); // this MUST be done for datalist2s!!!
			*lppNewSel = lpOldSel;	
		}
	}NxCatchAll("Error resetting reason code selection");	
}

// (a.walling 2006-11-16 10:04) - PLID 23552 - Prevent no selection
void CFinancialLineItemPostingDlg::OnSelChangingGroupCode(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			if (lpOldSel != NULL)
				lpOldSel->AddRef(); // this MUST be done for datalist2s!!!
			*lppNewSel = lpOldSel;	
		}
	}NxCatchAll("Error resetting group code selection");	
}

// (j.jones 2007-01-02 15:51) - PLID 24030 - added revenue code grouping
void CFinancialLineItemPostingDlg::OnCheckGroupChargesByRevenueCode() 
{
	try {
		
		// (j.jones 2011-06-24 09:35) - PLID 44314 - warn that if you post by revenue code,
		// the allowable will not be saved per charge
		CString strWarn;

		if(IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE)) {
			strWarn = "If you change the revenue code grouping, the data in the charge list will be reloaded.\n\n"
				"Additionally, allowable information will not be saved per charge if you post by revenue code.\n\n"
				"Are you sure you wish to do this?";
		}
		else {
			strWarn = "If you change the revenue code grouping, the data in the charge list will be reloaded.\n\n"
				"Are you sure you wish to do this?";
		}

		if(IDNO == MessageBox(strWarn,"Practice",MB_ICONQUESTION|MB_YESNO)) {

			CheckDlgButton(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE, !IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE));
			return;
		}

		if(IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) &&
			m_nRevCodeID == -1 && m_nChargeID != -1) {
			//if we opened the dialog filtered on one charge, then change to group by revenue code,
			//then let's filter by that charge's revenue code, if one exists

			_RecordsetPtr rs = CreateRecordset("SELECT "
				"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) AS RevCodeID "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
				"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
				"	INNER JOIN (SELECT * FROM InsuredPartyT WHERE PersonID = %li) AS InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID "
				"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = %li", m_nInsuredPartyID, m_nChargeID);
			if(!rs->eof) {
				m_nRevCodeID = AdoFldLong(rs, "RevCodeID",-1);
			}
			rs->Close();
		}

		// (j.jones 2012-05-08 09:29) - PLID 37165 - clear our adjustment info
		ClearAdjustmentPointers();

		m_PostingList->Clear();
		LoadPostingList();

	}NxCatchAll("Error in CFinancialLineItemPostingDlg::OnCheckGroupChargesByRevenueCode");
}

void CFinancialLineItemPostingDlg::OnFocusGainedLineItemPostingList() 
{
	try {

		// (j.jones 2007-02-01 17:30) - PLID 24541 - this should hopefully solve problems tabbing
		// into the charge list. Just check when the list gains focus that we are not already
		// editing it, and we are tabbing at the time. If the shift key is down, it means we
		// tabbed backwards into it, and should start at the bottom.

		BOOL bIsShiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
		BOOL bIsTabDown = (GetAsyncKeyState(VK_TAB) & 0x80000000) || IsMessageInQueue(NULL, WM_KEYUP, VK_TAB, 0, IMIQ_MATCH_WPARAM);

		// (j.jones 2007-02-01 17:37) - PLID 24541 - a sidebar, m_bIsEditingCharges works in tandem
		// with the arguably peculiar way the datalist handles focus changes in conjunction with
		// editingstarting/finished functions. At this time, OnFocusGained is fired before
		// OnEditingFinished is fired, and thus tabbing through the datalist is handled 
		// in OnEditingFinished without this code interfering.
	
		if(!m_bIsEditingCharges && m_PostingList->GetRowCount() > 0 && bIsTabDown) {

			//are they tabbing in from above or below?
			if(!bIsShiftDown) {	//above
				//which column to edit?

				short nCol = ColumnToEdit(TRUE);

				if(m_PostingList->GetCurSel() == -1)
					m_PostingList->StartEditing(0,nCol);
				else
					m_PostingList->StartEditing(m_PostingList->GetCurSel(),nCol);
			}
			else { //below
				//which column to edit?

				short nCol = ColumnToEdit(FALSE);

				if(m_PostingList->GetCurSel() == -1) 
					m_PostingList->StartEditing(m_PostingList->GetRowCount()-1,nCol);
				else
					m_PostingList->StartEditing(m_PostingList->GetCurSel(),nCol);
			}
		}

	}NxCatchAll("CFinancialLineItemPostingDlg::OnFocusGainedLineItemPostingList");
}

void CFinancialLineItemPostingDlg::OnEditingStartingLineItemPostingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		if(nRow == -1) {
			return;
		}

		// (j.jones 2012-05-08 09:38) - PLID 37165 - if a charge has multiple adjustments, then we have to open the
		// dialog to display them (insurance postings only)
		if(nCol == m_nAdjustmentColIndex && m_nInsuredPartyID != -1) {

			IRowSettingsPtr pRow = m_PostingList->GetRow(nRow);
			long nChargeID = VarLong(pRow->GetValue(plcChargeID), -1);
			long nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID), -1);

			ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
			if(pAdjInfo != NULL && pAdjInfo->aryAdjustmentInfo.GetSize() > 1) {
				//we have more than one entry, so disallow editing, and open the dialog
				*pbContinue = FALSE;
				OpenMultipleAdjustmentEntryDlg(nRow);
				return;
			}
		}
		
	
		// (j.jones 2007-02-01 17:30) - PLID 24541 - for tracking focus changes
		m_bIsEditingCharges = TRUE;

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2007-08-29 09:27) - PLID 27176 - added one function to calculate adjustments
// (j.jones 2013-07-03 16:51) - PLID 57226 - changed the payment parameter to be a variant,
// because payments can sometimes be null, and are treated differently than $0.00
// (j.jones 2014-07-01 09:36) - PLID 62552 - renamed to reflect this is the medical adjustment logic
COleCurrency CFinancialLineItemPostingDlg::CalculateAdjustment_Medical(COleCurrency cyInsResp, COleCurrency cyPatResp, _variant_t varInsPays, COleCurrency cyAllowable)
{
	//do nothing if no allowable is given
	if(cyAllowable <= COleCurrency(0,0)) {
		return COleCurrency(0,0);
	}

	// (j.jones 2012-08-14 08:59) - PLID 52118 - return $0.00 if the payment is $0.00,
	// we would never auto-adjust without a payment amount
	// (j.jones 2013-07-03 16:13) - PLID 57226 - apparently we will...
	// added preference for allowing adjustments on $0.00 payments, so first always
	// return if the payment is NULL, then return if it's somehow negative, finally
	// we might stay in this function if it's really $0.00 and this preference is on
	if(varInsPays.vt != VT_CY) {
		return COleCurrency(0,0);
	}
	COleCurrency cyInsPays = VarCurrency(varInsPays);
	if(cyInsPays < COleCurrency(0,0)) {
		return COleCurrency(0,0);
	}
	if(cyInsPays == COleCurrency(0,0) && m_nLineItemPostingAutoAdjust_AllowOnZeroDollarPayments == 0) {
		return COleCurrency(0,0);
	}

	// (j.jones 2007-08-29 09:29) - PLID 27176 - formerly this was cyInsResp - cyAllowable,
	// but really the allowed amount will also cover any patient resp, such as copays, etc.
	COleCurrency cyNewAdj = (cyInsResp + cyPatResp) - cyAllowable;
	if(cyNewAdj > COleCurrency(0,0)) {

		//make sure we don't make the balance less than zero
		if((cyInsResp - cyInsPays - cyNewAdj) < COleCurrency(0,0))
			cyNewAdj = cyInsResp - cyInsPays;
	}

	// (j.jones 2007-10-01 15:11) - PLID 27176 - make sure we never return a negative adjustment
	// (j.jones 2016-05-11 8:45) - NX-100503 - you can now manually enter negative adjustments,
	// but this code is still accurate - we would never calculate a negative one
	if(cyNewAdj < COleCurrency(0,0)) {
		return COleCurrency(0,0);
	}
	else {
		return cyNewAdj;
	}
}

// (j.jones 2014-07-01 09:36) - PLID 62552 - added a vision-specific version of CalculateAdjustment
COleCurrency CFinancialLineItemPostingDlg::CalculateAdjustment_Vision(COleCurrency cyInsResp, COleCurrency cyPatResp, _variant_t varInsPays)
{
	//don't calculate an adjustment if the option to auto-adjust is turned off
	if (!m_checkAutoAdjustBalances.GetCheck()) {
		//no code should have called this if the box is unchecked
		ASSERT(FALSE);
		return COleCurrency(0, 0);
	}

	//don't adjust unpaid charges
	if (varInsPays.vt != VT_CY) {
		return COleCurrency(0, 0);
	}
	
	COleCurrency cyInsPays = VarCurrency(varInsPays);
	
	//we do adjust when there is a chargeback,
	//but it will adjust as though there is a $0.00 payment
	if (cyInsPays < COleCurrency(0, 0)) {
		cyInsPays = COleCurrency(0, 0);
	}

	COleCurrency cyNewAdj = (cyInsResp + cyPatResp) - cyInsPays;
	if (cyNewAdj > COleCurrency(0, 0)) {

		//make sure we don't make the balance less than zero
		if ((cyInsResp - cyInsPays - cyNewAdj) < COleCurrency(0, 0))
			cyNewAdj = cyInsResp - cyInsPays;
	}

	//make sure we never return a negative adjustment
	// (j.jones 2016-05-11 8:45) - NX-100503 - you can now manually enter negative adjustments,
	// but this code is still accurate - we would never calculate a negative one
	if (cyNewAdj < COleCurrency(0, 0)) {
		return COleCurrency(0, 0);
	}
	else {
		return cyNewAdj;
	}
}

// (j.jones 2015-10-23 14:10) - PLID 67377 - added a capitation-specific version of CalculateAdjustment
// which simply adjusts off the balance of the insurance resp., ignoring the allowable
COleCurrency CFinancialLineItemPostingDlg::CalculateAdjustment_Capitation(COleCurrency cyInsResp, COleCurrency cyInsPays)
{
	//if the payment is negative, 
	//adjust as though there is a $0.00 payment
	if (cyInsPays < COleCurrency(0, 0)) {
		//this shouldn't be possible unless it's a vision payment,
		//and this function shouldn't be called during vision posting
		ASSERT(FALSE);
		cyInsPays = COleCurrency(0, 0);
	}

	COleCurrency cyNewAdj = cyInsResp - cyInsPays;
	if (cyNewAdj > COleCurrency(0, 0)) {
		//make sure we don't make the balance less than zero
		if ((cyInsResp - cyInsPays - cyNewAdj) < COleCurrency(0, 0)) {
			//how is this possible?
			ASSERT(FALSE);
			return COleCurrency(0, 0);
		}
	}

	//make sure we never return a negative adjustment
	// (j.jones 2016-05-11 8:45) - NX-100503 - you can now manually enter negative adjustments,
	// but this code is still accurate - we would never calculate a negative one
	if (cyNewAdj < COleCurrency(0, 0)) {
		return COleCurrency(0, 0);
	}
	else {
		return cyNewAdj;
	}
}

void CFinancialLineItemPostingDlg::OnDatetimechangePayDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		// (c.haag 2008-06-11 16:41) - PLID 26543 - If the following preference is true, it means
		// we need to update the adjustment date as well (even if it's disabled)
		if (GetRemotePropertyInt("LineItemPostingAutoUpdateAdjDate", 1, 0, "<None>", true) == 1) {
			m_dtAdjDate.SetValue(m_dtPayDate.GetValue());
		}
	}
	NxCatchAll("Error in CFinancialLineItemPostingDlg::OnDatetimechangePayDate");
	*pResult = 0;
}

void CFinancialLineItemPostingDlg::OnConfigureColumns()
{
	try
	{
		CFinancialLineItemPostingConfigureColumnsDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			ReloadDynamicColumns();

			// (j.jones 2012-05-08 09:29) - PLID 37165 - clear our adjustment info
			ClearAdjustmentPointers();

			m_PostingList->Clear();
			LoadPostingList();
			// (j.jones 2008-07-11 15:35) - PLID 28756 - changed to call PostSelChosenPayInsRespCombo instead of OnSelChosen
			PostSelChosenPayInsRespCombo();
		}

	}NxCatchAll("CFinancialLineItemPostingDlg::OnConfigureColumns");
}

CString CFinancialLineItemPostingDlg::GetDefaultPostingDynamicColumnList()
{
	// (z.manning 2008-06-19 10:56) - PLID 26544 - This was the order of these columns before
	// they could be dynamically ordered.
	// (j.jones 2010-05-04 16:52) - PLID 25521 - existing pat/ins pays are now visible, and New Pays is just one column now
	// (b.spivey, January 05, 2012) - PLID 47121 - Added deductible/coinsurance. 
	// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay

	//if the names or count of columns change, IsColumnListValid() needs to be updated
	return "Charges;Pat. Resp.;Ins. Resp.;Prev Pat Pays;Prev Ins Pays;New Pays;Allowable;Adjust.;Pat. Bal.;Ins. Bal.;Deductible;CoIns.;Copay;";

	//***When adding new columns, don't forget to increase DYNAMIC_COLUMN_COUNT.***//
}

// (z.manning 2008-06-19 09:52) - PLID 26544 - Removes and then reloads all dynamic columns
// based on the global preference.
void CFinancialLineItemPostingDlg::ReloadDynamicColumns()
{
	// (z.manning 2008-06-19 10:53) - PLID 26544 - First remove all dynamic columns.
	while(m_PostingList->GetColumnCount() > plcBeginDynamicCols) {
		m_PostingList->RemoveColumn(m_PostingList->GetColumnCount() - 1);
	}

	// (z.manning 2008-06-19 10:53) - PLID 26544 - Now load the semicolon delimited list of columns.
	CString strDefaultPostingDynamicColumnList = GetDefaultPostingDynamicColumnList();
	CString strColumnList = GetRemotePropertyText("LineItemPostingColumnList", strDefaultPostingDynamicColumnList, 0, "<None>", true);

	// (j.jones 2010-05-05 09:14) - PLID 25521 - the system that was put in place for column ordering
	// does not handle changes to names well, or new columns. So if the names do not match, we have to use the default value
	if(!IsColumnListValid(strColumnList)) {
		strColumnList = strDefaultPostingDynamicColumnList;
		SetRemotePropertyText("LineItemPostingColumnList", strColumnList, 0, "<None>");
	}

	short nColIndex = plcBeginDynamicCols;
	
	// (j.jones 2014-06-27 13:44) - PLID 62631 - reworked this code to convert the string into an array
	CStringArray aryColumns;
	ParseDelimitedStringToStringArray(strColumnList, ";", aryColumns);

	IColumnSettingsPtr pCol;
	// (z.manning 2008-06-19 10:54) - PLID 26544 - Now go throught the list of columns and add them to
	// the posting list in the order in which they appear.
	for (int i = 0; i < aryColumns.GetSize(); i++)
	{		
		CString strColumn = aryColumns.GetAt(i);

		// (j.jones 2013-08-27 13:08) - PLID 57398 - auto width won't work anymore, I changed the columns to width data
		// so we get a horizontal scrollbar
		pCol = m_PostingList->GetColumn(m_PostingList->InsertColumn(nColIndex, _T(""), _bstr_t(strColumn), 65, csVisible|csWidthData));

		// (j.jones 2014-06-27 14:59) - PLID 62631 - vision payments hide the deductible, coinsurance, and allowable columns
		if (m_ePayType == eVisionPayment &&
			(nColIndex == m_nDeductibleAmtColIndex || nColIndex == m_nCoinsuranceAmtColIndex || nColIndex == m_nAllowableColIndex)) {

			pCol->PutColumnStyle(csVisible | csFixedWidth);
			pCol->PutStoredWidth(0);
		}

		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_CY;

		AssignColumnIndex(strColumn, nColIndex);

		nColIndex++;
	}
}

// (j.jones 2010-05-05 09:34) - PLID 25521 - added ability to validate the loaded dynamic columns
// (b.spivey, January 05, 2012) - PLID 47121 - Notes below. 
//If you're here to update this function after adding a new column, you need to add a case for your column name and up 
//the count for however many columns you added to the datalist (e.g., added 3 and the count was 13, new comparison is nCount != 16). 
BOOL CFinancialLineItemPostingDlg::IsColumnListValid(CString strColumnList) {

	long nCount = 0;
	while(!strColumnList.IsEmpty())
	{
		int nSemicolon = strColumnList.Find(';');
		if(nSemicolon == -1) {
			nSemicolon = strColumnList.GetLength();
		}

		CString strColName = strColumnList.Left(nSemicolon);
		strColumnList.Delete(0, nSemicolon);
		strColumnList.TrimLeft(';');

		nCount++;
		
		// (b.spivey, January 03, 2012) - PLID 47121 - Added deductible and coinsurance and HasCoInsRow. 
		//is this a currently valid column name?
		// (b.spivey, January 16, 2012) - PLID 47121 - Shortened some names to remove ellipsis from columns. 
		// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
		if(strColName != "Charges" && strColName != "Pat. Resp." && strColName != "Ins. Resp."
			&& strColName != "Prev Pat Pays" && strColName != "Prev Ins Pays"
			&& strColName != "New Pays" && strColName != "Allowable" && strColName != "Adjust."
			&& strColName != "Pat. Bal." && strColName != "Ins. Bal." 
			&& strColName != "Deductible" && strColName != "CoIns." && strColName != "Copay") {

			//***When adding new columns, don't forget to increase DYNAMIC_COLUMN_COUNT.***//

			//name mismatch, this can't possibly be valid
			return FALSE;
		}
	}

	//the column total has to be 10
	// (b.spivey, January 03, 2012) - PLID 47121 - Upped to thirteen.
	// (b.spivey, January 16, 2012) - PLID 47121 -  back down to 12.
	// (j.jones 2013-08-27 12:17) - PLID 57398 - turned this into a define
	if(nCount != DYNAMIC_COLUMN_COUNT) {
		return FALSE;
	}

	return TRUE;
}

// (z.manning 2008-06-19 10:52) - PLID 26544 - Set the column index variable based on the given column name.
void CFinancialLineItemPostingDlg::AssignColumnIndex(const CString strColName, const short nColIndex)
{
	if(strColName == "Charges") {
		m_nChargesColIndex = nColIndex;
	}
	else if(strColName == "Pat. Resp.") {
		m_nPatRespColIndex = nColIndex;
	}
	else if(strColName == "Ins. Resp.") {
		m_nInsRespColIndex = nColIndex;
	}
	// (j.jones 2010-05-04 16:09) - PLID 25521 - existing pat/ins pays are now visible	
	else if(strColName == "Prev Pat Pays") {
		m_nExistingPatPaysColIndex = nColIndex;
	}
	else if(strColName == "Prev Ins Pays") {
		m_nExistingInsPaysColIndex = nColIndex;
	}
	// (j.jones 2010-05-05 09:49) - PLID 25521 - New Pays is just one column now
	else if(strColName == "New Pays") {
		m_nNewPaysColIndex = nColIndex;
	}
	else if(strColName == "Allowable") {
		m_nAllowableColIndex = nColIndex;
	}
	else if(strColName == "Adjust.") {
		m_nAdjustmentColIndex = nColIndex;
	}
	else if(strColName == "Pat. Bal.") {
		m_nPatBalanceColIndex = nColIndex;
	}
	else if(strColName == "Ins. Bal.") {
		m_nInsBalanceColIndex = nColIndex;
	}
	// (b.spivey, January 05, 2012) - PLID 47121 - Added deductible/coinsurance/hascoinsrow columns. 
	else if(strColName == "Deductible") {
		m_nDeductibleAmtColIndex = nColIndex; 
	}
	else if(strColName == "CoIns.") {
		m_nCoinsuranceAmtColIndex = nColIndex; 
	}
	// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
	else if(strColName == "Copay") {
		m_nCopayAmtColIndex = nColIndex;
	}
	else {
		ThrowNxException("CFinancialLineItemPostingDlg::AssignColumnIndex - Invalid column name '%s'", strColName);
	}
}

// (j.jones 2008-07-11 14:47) - PLID 28756 - this function will check
// which insurance company is selected (if any), and override the 
// payment description and payment category, as well as adjustments
void CFinancialLineItemPostingDlg::TrySetDefaultInsuranceDescriptions()
{
	try {

		//return if no insurance is selected
		if(m_PayRespCombo->CurSel == -1) {
			return;
		}

		//return if patient is selected
		if(VarLong(m_PayRespCombo->GetValue(m_PayRespCombo->CurSel, rccID), -1) == -1) {
			return;
		}

		CString strDefaultPayDesc = VarString(m_PayRespCombo->GetValue(m_PayRespCombo->CurSel, rccDefaultPayDesc), "");
		long nDefaultPayCategoryID = VarLong(m_PayRespCombo->GetValue(m_PayRespCombo->CurSel, rccDefaultPayCategoryID), -1);

		CString strDefaultAdjDesc = VarString(m_PayRespCombo->GetValue(m_PayRespCombo->CurSel, rccDefaultAdjDesc), "");
		long nDefaultAdjCategoryID = VarLong(m_PayRespCombo->GetValue(m_PayRespCombo->CurSel, rccDefaultAdjCategoryID), -1);

		//if from a batch payment, we can't change the category or description of the payment
		if(m_nBatchPaymentID == -1) {

			if(nDefaultPayCategoryID != -1) {
				m_PayCatCombo->TrySetSelByColumn(cccID, (long)nDefaultPayCategoryID);
				//normally when we manually select a category, it fills the description, but we do NOT do this on defaults
			}

			if(!strDefaultPayDesc.IsEmpty()) {
				m_nxeditPayDesc.SetWindowText(strDefaultPayDesc);
			}
		}

		if(nDefaultAdjCategoryID != -1) {
			m_AdjCatCombo->TrySetSelByColumn(cccID, (long)nDefaultAdjCategoryID);
			//normally when we manually select a category, it fills the description, but we do NOT do this on defaults
		}

		if(!strDefaultAdjDesc.IsEmpty()) {
			m_nxeditAdjDesc.SetWindowText(strDefaultAdjDesc);
		}

	}NxCatchAll("Error in CFinancialLineItemPostingDlg::TrySetDefaultInsuranceDescriptions");
}

// (j.jones 2008-07-11 15:33) - PLID 28756 - moved most of the OnSelChosenPayInsRespCombo logic into this function
void CFinancialLineItemPostingDlg::PostSelChosenPayInsRespCombo()
{
	try {

		// (j.jones 2013-08-27 13:08) - PLID 57398 - auto width won't work anymore, I changed the columns to width data
		// so we get a horizontal scrollbar

		//enable/disable editing of payment columns based on the ins. resp. selected
		if(m_nInsuredPartyID == -1) {
			//patient
			m_PostingList->GetColumn(m_nPatRespColIndex)->ColumnStyle = csVisible|csWidthData;
			m_PostingList->GetColumn(m_nPatRespColIndex)->PutForeColor(cReadOnly);
			m_PostingList->GetColumn(m_nInsRespColIndex)->ColumnStyle = csVisible|csWidthData;
			m_PostingList->GetColumn(m_nInsRespColIndex)->PutForeColor(cReadOnly);
			m_PostingList->GetColumn(m_nExistingPatPaysColIndex)->PutStoredWidth(78);
			m_PostingList->GetColumn(m_nExistingPatPaysColIndex)->ColumnStyle = csVisible|csEditable;
			m_PostingList->GetColumn(m_nExistingPatPaysColIndex)->PutForeColor(cEditable);
			m_PostingList->GetColumn(m_nExistingInsPaysColIndex)->PutStoredWidth(78);
			m_PostingList->GetColumn(m_nExistingInsPaysColIndex)->ColumnStyle = csVisible;
			m_PostingList->GetColumn(m_nExistingInsPaysColIndex)->PutForeColor(cReadOnly);
			m_PostingList->GetColumn(m_nNewPaysColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nNewPaysColIndex)->PutForeColor(cEditable);
			m_PostingList->GetColumn(m_nAdjustmentColIndex)->ColumnStyle = csVisible|csWidthData;
			m_PostingList->GetColumn(m_nAdjustmentColIndex)->PutForeColor(cReadOnly);

			// (b.spivey, January 04, 2012) - PLID 47121 - Added editable deductible and coinsurance, we use an 
			//		invisible column to check if there is an existing row. Uneditable if patient. 
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->PutStoredWidth(75);
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->ColumnStyle = csVisible|csWidthData;
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->PutForeColor(cReadOnly);

			m_PostingList->GetColumn(m_nCoinsuranceAmtColIndex)->ColumnStyle = csVisible|csWidthData;
			m_PostingList->GetColumn(m_nCoinsuranceAmtColIndex)->PutForeColor(cReadOnly);

			// (b.spivey, January 11, 2012) - PLID 47121 - Removed ChargeCoinsHasRow Column

			// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
			m_PostingList->GetColumn(m_nCopayAmtColIndex)->ColumnStyle = csVisible|csWidthData;
			m_PostingList->GetColumn(m_nCopayAmtColIndex)->PutForeColor(cReadOnly);

			//Disable the whole adjustment section, since they can't do anything useful there.  And clear the amount out.
			//They've been prompted, they have nobody to blame but themselves.
			GetDlgItem(IDC_ADJ_TOTAL)->EnableWindow(FALSE);
			SetDlgItemText(IDC_ADJ_TOTAL, FormatCurrencyForInterface(COleCurrency(0,0), FALSE, FALSE));
			GetDlgItem(IDC_ADJ_DATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADJ_CATEGORY)->EnableWindow(FALSE);
			m_AdjDescCombo->Enabled = FALSE;			
			GetDlgItem(IDC_EDIT_ADJ_DESC)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADJ_DESC)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADJUSTMENT_BAL_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADJUSTMENT_BALANCE)->EnableWindow(FALSE);
			m_PostingList->GetColumn(m_nAllowableColIndex)->ColumnStyle = csVisible|csWidthData;
			m_PostingList->GetColumn(m_nAllowableColIndex)->PutForeColor(cReadOnly);
			EnableGroupCodesAndReasons(false); // (a.walling 2006-11-16 10:20) - PLID 23552
			// (j.jones 2007-01-02 17:36) - PLID 24030 - allow revenue code filtering only when insurance
			GetDlgItem(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE)->EnableWindow(FALSE);
			CheckDlgButton(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE, FALSE);
			// (j.jones 2008-04-28 10:23) - PLID 29634 - disable the shifting controls
			m_ShiftRespCombo->Enabled = FALSE;
			GetDlgItem(IDC_CHECK_SHIFT_PAID_AMOUNTS)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_AUTO_BATCH)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_AUTO_SWAP)->EnableWindow(FALSE);
			// (j.jones 2010-05-17 16:54) - PLID 16503 - disable the zero-dollar apply setting for patients
			// (j.jones 2013-03-15 16:32) - PLID 49444 - permitted this on patients, just because it's uncommon
			// doesn't mean we have to disable it
			//m_checkApplyZeroDollarPays.EnableWindow(FALSE);

			// (j.jones 2010-06-02 11:55) - PLID 37200 - hide the "ins. amt. to unapply" data
			m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_HIDE);
			m_nxstaticInsAmtToUnapply.ShowWindow(SW_HIDE);
		}
		else {
			//insurance
			m_PostingList->GetColumn(m_nPatRespColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nPatRespColIndex)->PutForeColor(cEditable);
			m_PostingList->GetColumn(m_nInsRespColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nInsRespColIndex)->PutForeColor(cEditable);
			m_PostingList->GetColumn(m_nExistingPatPaysColIndex)->PutStoredWidth(78);
			m_PostingList->GetColumn(m_nExistingPatPaysColIndex)->ColumnStyle = csVisible|csEditable;
			m_PostingList->GetColumn(m_nExistingPatPaysColIndex)->PutForeColor(cEditable);
			m_PostingList->GetColumn(m_nExistingInsPaysColIndex)->PutStoredWidth(78);
			m_PostingList->GetColumn(m_nExistingInsPaysColIndex)->ColumnStyle = csVisible|csEditable;
			m_PostingList->GetColumn(m_nExistingInsPaysColIndex)->PutForeColor(cEditable);
			m_PostingList->GetColumn(m_nNewPaysColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nNewPaysColIndex)->PutForeColor(cEditable);
			m_PostingList->GetColumn(m_nAdjustmentColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nAdjustmentColIndex)->PutForeColor(cEditable);

			// (b.spivey, January 04, 2012) - PLID 47121 - Added editable deductible and coinsurance, we use an 
			//		invisible column to check if there is an existing row. Editable if insurance selected. 
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->PutStoredWidth(75);
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->PutForeColor(cEditable);

			m_PostingList->GetColumn(m_nCoinsuranceAmtColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nCoinsuranceAmtColIndex)->PutForeColor(cEditable);

			// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
			m_PostingList->GetColumn(m_nCopayAmtColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nCopayAmtColIndex)->PutForeColor(cEditable);

			// (b.spivey, January 11, 2012) - PLID 47121 - Removed ChargeCoinsHasRow Column

			//Enable the adjustment section, since they can edit it now.
			GetDlgItem(IDC_ADJ_TOTAL)->EnableWindow(TRUE);

			// (j.jones 2006-12-07 12:40) - PLID 19467 - added option to disable payment date changing
			long nDisablePaymentDate = GetRemotePropertyInt("DisablePaymentDate", 0, 0, "<None>", true);
			if(nDisablePaymentDate == 0) {
				GetDlgItem(IDC_ADJ_DATE)->EnableWindow(TRUE);
			}

			GetDlgItem(IDC_ADJ_CATEGORY)->EnableWindow(TRUE);
			m_AdjDescCombo->Enabled = TRUE;
			GetDlgItem(IDC_EDIT_ADJ_DESC)->EnableWindow(TRUE);
			GetDlgItem(IDC_ADJ_DESC)->EnableWindow(TRUE);
			GetDlgItem(IDC_ADJUSTMENT_BAL_LABEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ADJUSTMENT_BALANCE)->EnableWindow(TRUE);
			m_PostingList->GetColumn(m_nAllowableColIndex)->ColumnStyle = csVisible|csWidthData|csEditable;
			m_PostingList->GetColumn(m_nAllowableColIndex)->PutForeColor(cEditable);
			EnableGroupCodesAndReasons(true); // (a.walling 2006-11-16 10:20) - PLID 23552
			// (j.jones 2007-01-02 17:36) - PLID 24030 - allow revenue code filtering only when insurance
			GetDlgItem(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE)->EnableWindow(TRUE);

			// (j.jones 2008-04-28 10:23) - PLID 29634 - enable the shifting controls
			m_ShiftRespCombo->Enabled = TRUE;
			GetDlgItem(IDC_CHECK_SHIFT_PAID_AMOUNTS)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_AUTO_BATCH)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_AUTO_SWAP)->EnableWindow(TRUE);

			// (j.jones 2010-05-17 16:54) - PLID 16503 - enable the zero-dollar apply setting for insurance
			m_checkApplyZeroDollarPays.EnableWindow(TRUE);

			// (j.jones 2010-06-02 11:55) - PLID 37200 - show/hide the "ins. amt. to unapply" data
			if(m_cyUnappliedInsAmt != COleCurrency(0,0)) {
				m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_SHOW);
				m_nxstaticInsAmtToUnapply.ShowWindow(SW_SHOW);
			}
			else {
				m_nxstaticInsAmtToUnapplyLabel.ShowWindow(SW_HIDE);
				m_nxstaticInsAmtToUnapply.ShowWindow(SW_HIDE);
			}
		}

		// (j.jones 2014-06-27 14:59) - PLID 62631 - vision payments hide the deductible, coinsurance, and allowable columns
		if (m_ePayType == eVisionPayment) {
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->ColumnStyle = csVisible | csFixedWidth;
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->PutStoredWidth(0);
			m_PostingList->GetColumn(m_nCoinsuranceAmtColIndex)->ColumnStyle = csVisible | csFixedWidth;
			m_PostingList->GetColumn(m_nCoinsuranceAmtColIndex)->PutStoredWidth(0);
			m_PostingList->GetColumn(m_nAllowableColIndex)->ColumnStyle = csVisible | csFixedWidth;
			m_PostingList->GetColumn(m_nAllowableColIndex)->PutStoredWidth(0);
		}

	}NxCatchAll("Error in CFinancialLineItemPostingDlg::PostSelChosenPayInsRespCombo");
}

// (j.jones 2010-06-02 15:38) - PLID 37200 - supported coloring text
HBRUSH CFinancialLineItemPostingDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	try {
		
		long nCtrlID = pWnd->GetDlgCtrlID();
		switch (nCtrlID)
		{
			// (j.jones 2014-07-29 09:30) - PLID 63076 - made this code also cover the total payments label,
			// and unified it so all potentially negative totals used the same code
			case IDC_PAYMENT_BALANCE:
			case IDC_ADJUSTMENT_BALANCE:
			case IDC_UNAPPLIED_PAT_PAYS:
			case IDC_UNAPPLIED_INS_PAYS:
			case IDC_TOTAL_PAYMENTS:{
					CString strAmount;
					GetDlgItemText(nCtrlID, strAmount);
					COleCurrency cyPayBalance = ParseCurrencyFromInterface(strAmount);
					if(cyPayBalance.GetStatus() == COleCurrency::valid && cyPayBalance >= COleCurrency(0,0)) {
						//positive, so paint it black
						pDC->SelectPalette(&theApp.m_palette, FALSE);
						pDC->RealizePalette();
						pDC->SetTextColor(PaletteColor(RGB(0,0,0)));
					}
					else {
						//negative (or invalid), make it red
						pDC->SelectPalette(&theApp.m_palette, FALSE);
						pDC->RealizePalette();
						pDC->SetTextColor(PaletteColor(RGB(255,0,0)));
					}
				}
				break;

			default:
			break;
		}

	}NxCatchAll(__FUNCTION__);

	return hbr;
}

// (j.jones 2010-06-11 11:36) - PLID 16704 - added buttons for billing notes
void CFinancialLineItemPostingDlg::OnBtnBillNotes()
{
	try {

		CNotesDlg dlgNotes(this);
		dlgNotes.m_bIsBillingNote = true;
		// (j.jones 2011-09-19 14:46) - PLID 42135 - added m_bntBillingNoteType
		dlgNotes.m_bntBillingNoteType = bntBill;
		dlgNotes.m_nBillID = m_nBillID;
		// (a.walling 2010-09-20 11:41) - PLID 40589 - Set the patient ID
		dlgNotes.SetPersonID(m_nPatientID);
		CNxModalParentDlg dlg(this, &dlgNotes, CString("Bill Notes"));
		dlg.DoModal();

		//refresh the icon
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 BillID FROM Notes WHERE Notes.BillID = {INT}", m_nBillID);
		if(rs->eof) {
			m_btnBillNotes.SetIcon(IDI_OTHER);
		}
		else {
			m_btnBillNotes.SetIcon(IDI_BILL_NOTES);
		}
		rs->Close();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-11 11:36) - PLID 16704 - added ability to click on charge notes
void CFinancialLineItemPostingDlg::OnLeftClickLineItemPostingList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		if(nRow == -1) {
			return;
		}

		if(nCol == plcBillNote) {
			if(IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && m_nInsuredPartyID != -1) {
				//open the billing notes for this bill

				//warn that this is billing notes
				DontShowMeAgain(this, "Editing billing notes when grouping by revenue code will only display and edit the bill level notes.\n"
					"Charge notes are not viewable or editable when grouping by revenue code.",
					"LineItemPosting_RevCodeNotes", "Practice - Line Item Posting");

				CNotesDlg dlgNotes(this);
				dlgNotes.m_bIsBillingNote = true;
				// (j.jones 2011-09-19 14:46) - PLID 42135 - added m_bntBillingNoteType
				dlgNotes.m_bntBillingNoteType = bntBill;
				dlgNotes.m_nBillID = m_nBillID;
				// (a.walling 2010-09-20 11:41) - PLID 40589 - Set the patient ID
				dlgNotes.SetPersonID(m_nPatientID);
				CNxModalParentDlg dlg(this, &dlgNotes, CString("Bill Notes"));
				dlg.DoModal();

				//refresh the icon
				_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 BillID FROM Notes WHERE Notes.BillID = {INT}", m_nBillID);
				if(rs->eof) {
					m_btnBillNotes.SetIcon(IDI_OTHER);
					m_PostingList->PutValue(nRow, plcBillNote, (LPCTSTR)"BITMAP:FILE");			
				}
				else {
					m_btnBillNotes.SetIcon(IDI_BILL_NOTES);
					m_PostingList->PutValue(nRow, plcBillNote, (long)m_hNotes);
				}
				rs->Close();
			}
			else {
				//open the billing notes for this charge
				long nChargeID = VarLong(m_PostingList->GetValue(nRow, plcChargeID), -1);

				CNotesDlg dlgNotes(this);
				dlgNotes.m_bIsBillingNote = true;
				// (j.jones 2011-09-19 14:46) - PLID 42135 - added m_bntBillingNoteType
				dlgNotes.m_bntBillingNoteType = bntCharge;
				dlgNotes.m_nLineItemID = nChargeID;				
				// (a.walling 2010-09-20 11:41) - PLID 40589 - Set the patient ID
				dlgNotes.SetPersonID(m_nPatientID);
				CNxModalParentDlg dlg(this, &dlgNotes, CString("Charge Notes"));
				dlg.DoModal();

				//refresh the icon
				_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 LineItemID FROM Notes WHERE Notes.LineItemID = {INT}", nChargeID);
				if(rs->eof) {
					m_PostingList->PutValue(nRow, plcBillNote, (LPCTSTR)"BITMAP:FILE");
				}
				else {
					m_PostingList->PutValue(nRow, plcBillNote, (long)m_hNotes);
				}
				rs->Close();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-05-07 09:13) - PLID 37165 - added ability to add multiple adjustments per charge
void CFinancialLineItemPostingDlg::OnRButtonDownLineItemPostingList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		if(nRow == -1) {
			return;
		}

		m_PostingList->CurSel = nRow;

		enum {
			eEditAdjustments = 1,
		};

		if(nCol == m_nAdjustmentColIndex && m_nInsuredPartyID != -1) {
			// (j.jones 2012-05-07 09:13) - PLID 37165 - added ability to add multiple adjustments per charge,
			// you can only add adjustments on insurance postings

			// Create the menu
			CMenu mnu;
			mnu.CreatePopupMenu();
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eEditAdjustments, "&Edit Adjustments");

			CPoint pt;
			GetCursorPos(&pt);

			int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

			if(nRet == eEditAdjustments) {

				OpenMultipleAdjustmentEntryDlg(nRow);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-05-08 09:39) - PLID 37165 - Finds a ChargeAdjustmentInfo entry in m_aryChargeAdjustmentInfo
// to match a charge ID or revenue code ID. Returns NULL if not found.
ChargeAdjustmentInfo* CFinancialLineItemPostingDlg::FindChargeAdjustmentInfo(long nChargeID, long nRevenueCodeID)
{
	//throw exceptions to the caller

	BOOL bIsRevCode = IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && nRevenueCodeID != -1;

	for(int i=0; i<m_aryChargeAdjustmentInfo.GetSize(); i++) {
		//get the adjustment for our charge or revenue code
		ChargeAdjustmentInfo *pInfo = (ChargeAdjustmentInfo*)m_aryChargeAdjustmentInfo.GetAt(i);
		if(bIsRevCode) {
			//if a revenue code, match by it if this line has one
			if(nRevenueCodeID != -1 && pInfo->nRevenueCodeID == nRevenueCodeID) {
				return pInfo;
			}
			//otherwise match by charge ID
			else if(nRevenueCodeID == -1 && pInfo->nChargeID == nChargeID) {
				return pInfo;
			}
		}
		//match by charge ID
		else if(pInfo->nChargeID == nChargeID) {
			return pInfo;
		}
	}

	//return NULL if none found
	return NULL;
}

// (j.jones 2012-05-08 09:43) - PLID 37165 - opens the MultipleAdjustmentEntryDlg with a given
// charge's information & adjustments
void CFinancialLineItemPostingDlg::OpenMultipleAdjustmentEntryDlg(long nRow)
{
	try {

		//should never have been called if a patient posting
		if(m_nInsuredPartyID == -1) {
			ASSERT(FALSE);
			return;
		}

		IRowSettingsPtr pRow = m_PostingList->GetRow(nRow);
		long nChargeID = VarLong(pRow->GetValue(plcChargeID), -1);
		long nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID), -1);

		BOOL bIsRevCode = IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && nRevCodeID != -1;

		CMultipleAdjustmentEntryDlg dlg(this);

		COleCurrency cyInsResp = VarCurrency(pRow->GetValue(m_nInsRespColIndex),COleCurrency(0,0));

		if(m_PayRespCombo->GetCurSel() != -1) {
			dlg.m_strInsuranceCoName = VarString(m_PayRespCombo->GetValue(m_PayRespCombo->GetCurSel(), rccName), "");
		}

		if(bIsRevCode) {
			dlg.m_strLineType = "Revenue Code";
		}
		else {
			dlg.m_strLineType = "Charge";
		}
		//If there is no item code, we're at a bit of a loss, this charge has no unique description loaded.
		//Fortunately this is only used on insurance postings and products would never be paid without codes.
		dlg.m_strItemCode = VarString(pRow->GetValue(plcItemCode), "");
		dlg.m_cyChargeTotal = VarCurrency(pRow->GetValue(m_nChargesColIndex), COleCurrency(0,0));
		dlg.m_dtDateOfService = VarDateTime(pRow->GetValue(plcChargeDate));
		dlg.m_cyInsResp = cyInsResp;

		//calculate the balance
		COleCurrency cyExistingInsPays = VarCurrency(pRow->GetValue(m_nExistingInsPaysColIndex),COleCurrency(0,0));
		COleCurrency cyNewInsPays = VarCurrency(pRow->GetValue(m_nNewPaysColIndex),COleCurrency(0,0));
		COleCurrency cyOldAdjustments = VarCurrency(pRow->GetValue(m_nAdjustmentColIndex),COleCurrency(0,0));
		// (j.jones 2014-07-01 15:42) - PLID 62553 - chargebacks do not affect the balance
		COleCurrency cyTotalInsPays = cyExistingInsPays + (cyNewInsPays < COleCurrency(0, 0) ? COleCurrency(0, 0) : cyNewInsPays);
		COleCurrency cyOldBalance = cyInsResp - cyTotalInsPays - cyOldAdjustments;
		dlg.m_cyInsBalance = cyOldBalance;

		//find our adjustment info, if any, and copy to the dialog
		ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
		if(pAdjInfo) {
			dlg.m_aryAdjustmentInfo.Append(pAdjInfo->aryAdjustmentInfo);
		}
		else {
			//create a new pointer for the current adjustment value
			pAdjInfo = new ChargeAdjustmentInfo;
			pAdjInfo->nChargeID = nChargeID;
			pAdjInfo->nRevenueCodeID = nRevCodeID;
			
			AdjustmentInfo *pNewAdj = new AdjustmentInfo;
			pNewAdj->cyAmount = cyOldAdjustments;
			pAdjInfo->aryAdjustmentInfo.Add(pNewAdj);

			m_aryChargeAdjustmentInfo.Add(pAdjInfo);

			dlg.m_aryAdjustmentInfo.Add(pNewAdj);
		}

		if(dlg.DoModal() == IDOK) {

			//Update our adjustment info. with the results of the dialog.
			//The dialog will have freed memory and created new pointers.
			//Just assign the pointers to our array.
			if(pAdjInfo) {
				pAdjInfo->aryAdjustmentInfo.RemoveAll();
				pAdjInfo->aryAdjustmentInfo.Append(dlg.m_aryAdjustmentInfo);
			}
			else {
				//make a new object and add the adjustments
				ChargeAdjustmentInfo *pNewChargeAdjInfo = new ChargeAdjustmentInfo;
				pNewChargeAdjInfo->nChargeID = nChargeID;
				pNewChargeAdjInfo->nRevenueCodeID = nRevCodeID;
				pNewChargeAdjInfo->aryAdjustmentInfo.Append(dlg.m_aryAdjustmentInfo);
				m_aryChargeAdjustmentInfo.Add(pNewChargeAdjInfo);
			}

			//update the datalist
			COleCurrency cyTotalAdj = COleCurrency(0,0);
			for(int i=0; i<dlg.m_aryAdjustmentInfo.GetSize(); i++) {
				AdjustmentInfo *pAdj = (AdjustmentInfo*)dlg.m_aryAdjustmentInfo.GetAt(i);
				cyTotalAdj += pAdj->cyAmount;
			}
			//update the adjustment row and the balance
			pRow->PutValue(m_nAdjustmentColIndex, _variant_t(cyTotalAdj));
			pRow->PutValue(m_nInsBalanceColIndex, _variant_t(COleCurrency(cyInsResp - cyTotalInsPays - cyTotalAdj)));

			//calculate totals
			AutoFillAdjustmentTotal();
			CalculateTotals();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-05-08 10:26) - PLID 37165 - updates adjustment amounts in tracked pointers
void CFinancialLineItemPostingDlg::UpdateAdjustmentPointers(COleCurrency cyNewAdjustmentTotal, ChargeAdjustmentInfo* pChargeAdjInfo)
{
	if(pChargeAdjInfo == NULL) {
		return;
	}

	if(pChargeAdjInfo->aryAdjustmentInfo.GetSize() == 0) {
		//no pointers to update
		return;
	}

	COleCurrency cyOldTotal = COleCurrency(0,0);
	for(int i=0; i<pChargeAdjInfo->aryAdjustmentInfo.GetSize(); i++) {
		AdjustmentInfo *pAdj = (AdjustmentInfo*)pChargeAdjInfo->aryAdjustmentInfo.GetAt(i);
		cyOldTotal += pAdj->cyAmount;
	}


	COleCurrency cyDifference = cyNewAdjustmentTotal -= cyOldTotal;

	//if cyDifference is positive, we're adding money, so just add it to the first adjustment
	if(cyDifference > COleCurrency(0,0)) {
		AdjustmentInfo *pAdj = (AdjustmentInfo*)pChargeAdjInfo->aryAdjustmentInfo.GetAt(0);
		pAdj->cyAmount += cyDifference;
	}
	//if cyDifference is negative, we're removing money, remove from each adjustment until we're out of money
	else if(cyDifference < COleCurrency(0,0)) {
		COleCurrency cyAmtToReduce = -cyDifference;
		for(int i=0; i<pChargeAdjInfo->aryAdjustmentInfo.GetSize() && cyAmtToReduce > COleCurrency(0,0); i++) {
			AdjustmentInfo *pAdj = (AdjustmentInfo*)pChargeAdjInfo->aryAdjustmentInfo.GetAt(i);
			if(pAdj->cyAmount > cyAmtToReduce) {
				pAdj->cyAmount -= cyAmtToReduce;
				cyAmtToReduce = COleCurrency(0,0);
			}
			else {
				cyAmtToReduce -= pAdj->cyAmount;
				pAdj->cyAmount = COleCurrency(0,0);
			}
		}
	}
}

// (j.jones 2012-05-08 16:35) - PLID 37165 - used to clear our memory objects for adjustments
void CFinancialLineItemPostingDlg::ClearAdjustmentPointers()
{
	for(int i=0;i<m_aryChargeAdjustmentInfo.GetSize();i++) {
		ChargeAdjustmentInfo *pInfo = (ChargeAdjustmentInfo*)m_aryChargeAdjustmentInfo.GetAt(i);
		if(pInfo) {
			for(int j=0;j<pInfo->aryAdjustmentInfo.GetSize();j++) {
				AdjustmentInfo* pAdj = (AdjustmentInfo*)pInfo->aryAdjustmentInfo.GetAt(j);
				if(pAdj) {
					delete pAdj;
				}
			}
			pInfo->aryAdjustmentInfo.RemoveAll();

			delete pInfo;
		}
	}
	m_aryChargeAdjustmentInfo.RemoveAll();
}

// (j.jones 2012-07-30 10:58) - PLID 47778 - added ability to auto-calculate payment amounts
// (j.jones 2012-08-21 09:26) - PLID 52029 - added bSilent, if FALSE then we can warn when we can't calculate
// (j.jones 2012-08-28 17:20) - PLID 52335 - now requires the insurance resp. balance
// (j.jones 2013-08-27 11:48) - PLID 57398 - Added copay manual amount, this is the amount they would have manually typed in.
// Also renamed the other two manually typed in columns for clarity.
COleCurrency CFinancialLineItemPostingDlg::CalculatePayment(BOOL bSilent, CString strItemCode, COleCurrency cyChargeTotal, COleCurrency cyInsBalance,
															COleCurrency cyAllowable, _variant_t varCoInsurancePercent,
															_variant_t varCopayMoney, _variant_t varCopayPercent,
															_variant_t varDeductibleManualAmount, _variant_t varCoinsuranceManualAmount,
															_variant_t varCopayManualAmount)
{
	//***If the primary payment logic ever changes, dbo.EstimatedChargePayment and dbo.EstimatedBillPayment also have to change***/

	//throw exceptions to the caller

	//do nothing if not an insurance posting
	if(m_nInsuredPartyID == -1) {
		//should have never been called on an insurance posting
		ASSERT(FALSE);
		return COleCurrency(0,0);
	}

	//do nothing if it's a zero dollar charge
	if(cyChargeTotal <= COleCurrency(0,0)) {
		//no warning needed
		return COleCurrency(0,0);
	}

	CString strChargeDesc;
	if(strItemCode.IsEmpty()) {
		strChargeDesc.Format("%s charge", FormatCurrencyForInterface(cyChargeTotal));
	}
	else {
		strChargeDesc.Format("%s charge for code %s", FormatCurrencyForInterface(cyChargeTotal), strItemCode);
	}

	// (j.jones 2012-08-28 17:20) - PLID 52335 - if not primary, we can skip this function entirely
	// and just return the secondary balance, because that's all non-primary payments are calculated as
	if(!m_bIsPrimaryIns) {
		if(cyInsBalance > COleCurrency(0,0)) {
			COleCurrency cyNewPayment = cyInsBalance;

			//if they typed in a deductible, subtract it
			COleCurrency cyDeductibleManualAmt = VarCurrency(varDeductibleManualAmount, COleCurrency(0,0));
			if(cyDeductibleManualAmt > COleCurrency(0,0)) {
				cyNewPayment -= cyDeductibleManualAmt;
			}

			//if they typed in a coinsurance amount, subtract that
			COleCurrency cyCoinsuranceManualAmt = VarCurrency(varCoinsuranceManualAmount, COleCurrency(0,0));
			if(cyCoinsuranceManualAmt > COleCurrency(0,0)) {
				cyNewPayment -= cyCoinsuranceManualAmt;
			}

			// (j.jones 2013-08-27 11:41) - PLID 57398 - added copay, this is the amount they manually typed in
			COleCurrency cyCopayManualAmt = VarCurrency(varCopayManualAmount, COleCurrency(0,0));
			if(cyCopayManualAmt > COleCurrency(0,0)) {
				cyNewPayment -= cyCopayManualAmt;
			}

			//don't return $0.00, but no need to warn about it
			if(cyNewPayment < COleCurrency(0,0)) {
				return COleCurrency(0,0);
			}
			else {
				return cyNewPayment;
			}
		}
		else {
			/* I concluded that not having a balance is perfectly legitimate, and needs no warning.
			if(!bSilent) {
				CString strMessage;
				strMessage.Format("A payment could not be calculated for the %s because there is no current insurance balance.", strChargeDesc);
				MessageBox(strMessage, "Practice", MB_ICONINFORMATION|MB_OK);
			}
			*/
			return COleCurrency(0,0);
		}

		//should be logically impossible to get here
		ASSERT(FALSE);
		return COleCurrency(0,0);

	} //if(!m_bIsPrimaryIns)

	//the remainder of this function only applies to calculating primary payments

	//do nothing if no allowable is given
	if(cyAllowable <= COleCurrency(0,0)) {
		// (j.jones 2012-08-21 10:05) - PLID 52029 - if not silent, warn
		if(!bSilent) {
			CString strMessage;
			strMessage.Format("A payment could not be calculated for the %s because no allowable has been entered for this service.", strChargeDesc);
			MessageBox(strMessage, "Practice", MB_ICONINFORMATION|MB_OK);
		}
		return COleCurrency(0,0);
	}

	COleCurrency cyNewPayment = COleCurrency(0,0);

	//In all cases, if they typed in a deductible, subtract it from the allowable.
	//Coinsurance would be calculated on the remaining amount.
	COleCurrency cyDeductibleManualAmt = VarCurrency(varDeductibleManualAmount, COleCurrency(0,0));
	if(cyDeductibleManualAmt > COleCurrency(0,0)) {
		cyAllowable -= cyDeductibleManualAmt;
		if(cyAllowable < COleCurrency(0,0)) {
			cyAllowable = COleCurrency(0,0);
		}
	}

	//If co-insurance was typed in, just subtract that from the allowable (which may have
	//already been reduced by the deductible), the new payment is always the balance.
	//In this case, typing in $0.00 does mean that a value was entered, $0.00 is very different than null.
	// (j.jones 2013-08-27 11:51) - PLID 57398 - Added copays, which adhere to the same logic.
	// Type one in, and we won't try to auto-calculate.
	if(varCoinsuranceManualAmount.vt == VT_CY || varCopayManualAmount.vt == VT_CY) {
		COleCurrency cyCoinsuranceManualAmt = VarCurrency(varCoinsuranceManualAmount, COleCurrency(0,0));
		COleCurrency cyCopayManualAmt = VarCurrency(varCopayManualAmount, COleCurrency(0,0));
		
		//the amount paid would be the allowable balance after the deductible, coinsurance, and copay are taken out,
		//we don't need to estimate the co-insurance or copay because they typed in the actual amount the patient owes

		//if they entered a deductible, we would have already subtracted it from the allowable
		cyNewPayment = cyAllowable - cyCoinsuranceManualAmt - cyCopayManualAmt;
	}
	else {
		//We don't have the co-insurance, we must estimate.

		//if no data is known, we can't calculate anything
		if(varCoInsurancePercent.vt != VT_I4 && varCopayMoney.vt != VT_CY && varCopayPercent.vt != VT_I4) {
			//we can't calculate a payment
			// (j.jones 2012-08-21 10:05) - PLID 52029 - if not silent, warn
			if(!bSilent) {
				CString strMessage;
				strMessage.Format("A payment could not be calculated for the %s because no copay or coinsurance information has been entered for its pay group.", strChargeDesc);
				MessageBox(strMessage, "Practice", MB_ICONINFORMATION|MB_OK);
			}
			return COleCurrency(0,0);
		}

		//First handle copays. If the pay group has a copay, subtract it first.
		//No pay group should have a copay $ and copay %, if they do give the
		//money value priority and ignore the percentage value.

		if(varCopayMoney.vt == VT_CY) {
			//they have a copay $, subtract it from the allowable
			COleCurrency cyCopayMoney = VarCurrency(varCopayMoney, COleCurrency(0,0));
			cyAllowable -= cyCopayMoney;
		}
		else if(varCopayPercent.vt == VT_I4 && VarLong(varCopayPercent) >= 0) {
			//They have a copay %, subtract it from the allowable.
			COleCurrency cyCopay = CalculatePercentOfAmount(cyAllowable, VarLong(varCopayPercent));
			cyAllowable -= cyCopay;
		}

		//if a deductible was entered, it would have already been subtracted from the allowable,
		//co-insurance will now be calculated on the remaining allowable amount

		if(varCoInsurancePercent.vt != VT_I4) {
			//this case means they had a copay and not a coinsurance, which is pretty normal,
			//they would never actually have both

			//we would have reduced the allowable by the copay amount, insurance would pay the rest
			cyNewPayment = cyAllowable;
		}
		else {
			//co-insurance is the amount the patient should pay, so the insurance
			//payment amount is the rest. ie. 20% coinsurance means insurance pays 80%
			long nPercentInsPaid = 100 - VarLong(varCoInsurancePercent);
			cyNewPayment = CalculatePercentOfAmount(cyAllowable, nPercentInsPaid);
		}
	}

	//Now cyNewPayment should be the expected amount paid by insurance. In the unlikely
	//case that the insurance already paid once, or for some reason this payment is greater
	//than the insurance balance, we still use our calculated payment. It's the user's
	//responsibility to change it to be a lower amount, not ours.

	//never return a negative balance
	if(cyNewPayment < COleCurrency(0,0)) {
		return COleCurrency(0,0);
	}
	else {
		return cyNewPayment;
	}
}

// (j.jones 2012-07-31 09:26) - PLID 51863 - added ability to auto-calculate payment amounts by revenue code
// (j.jones 2012-08-21 09:26) - PLID 52029 - added bSilent, if FALSE then we can warn when we can't calculate
// (j.jones 2012-08-28 17:20) - PLID 52335 - now requires the insurance resp. balance
// (j.jones 2013-08-27 11:48) - PLID 57398 - Added copay manual amount, this is the amount they would have manually typed in.
// Also renamed the other two manually typed in columns for clarity.
COleCurrency CFinancialLineItemPostingDlg::CalculatePaymentForRevenueCode(BOOL bSilent, CString strItemCode, long nRevCodeID, COleCurrency cyTotalCharges, COleCurrency cyTotalInsBalance,
																		  COleCurrency cyTotalAllowable, _variant_t varDeductibleManualAmount, _variant_t varCoinsuranceManualAmount, _variant_t varCopayManualAmount)
{
	//throw exceptions to the caller

	//do nothing if not an insurance posting
	if(m_nInsuredPartyID == -1) {
		//should have never been called on an insurance posting
		ASSERT(FALSE);
		return COleCurrency(0,0);
	}

	//do nothing if the charge total is zero
	if(cyTotalCharges <= COleCurrency(0,0)) {
		//no warning needed
		return COleCurrency(0,0);
	}

	CString strRevCodeDesc;
	strRevCodeDesc.Format("%s for %s", strItemCode, FormatCurrencyForInterface(cyTotalCharges));

	// (j.jones 2012-08-28 17:38) - PLID 52335 - if not primary, we can skip this function entirely
	// and just return the secondary balance, because that's all non-primary payments are calculated as
	if(!m_bIsPrimaryIns) {
		if(cyTotalInsBalance > COleCurrency(0,0)) {
			COleCurrency cyNewPayment = cyTotalInsBalance;

			//if they typed in a deductible, subtract it
			COleCurrency cyDeductibleManualAmt = VarCurrency(varDeductibleManualAmount, COleCurrency(0,0));
			if(cyDeductibleManualAmt > COleCurrency(0,0)) {
				cyNewPayment -= cyDeductibleManualAmt;
			}

			//if they typed in a coinsurance amount, subtract that
			COleCurrency cyCoinsuranceManualAmt = VarCurrency(varCoinsuranceManualAmount, COleCurrency(0,0));
			if(cyCoinsuranceManualAmt > COleCurrency(0,0)) {
				cyNewPayment -= cyCoinsuranceManualAmt;
			}

			// (j.jones 2013-08-27 11:41) - PLID 57398 - added copay, this is the amount they manually typed in
			COleCurrency cyCopayManualAmt = VarCurrency(varCopayManualAmount, COleCurrency(0,0));
			if(cyCopayManualAmt > COleCurrency(0,0)) {
				cyNewPayment -= cyCopayManualAmt;
			}

			//don't return $0.00, but no need to warn about it
			if(cyNewPayment < COleCurrency(0,0)) {
				return COleCurrency(0,0);
			}
			else {
				return cyNewPayment;
			}
		}
		else {
			/* I concluded that not having a balance is perfectly legitimate, and needs no warning.
			if(!bSilent) {
				CString strMessage;
				strMessage.Format("A payment could not be calculated for the revenue code %s because there is no current insurance balance.", strRevCodeDesc);
				MessageBox(strMessage, "Practice", MB_ICONINFORMATION|MB_OK);
			}
			*/
			return COleCurrency(0,0);
		}

		//should be logically impossible to get here
		ASSERT(FALSE);
		return COleCurrency(0,0);

	} //if(!m_bIsPrimaryIns)

	//the remainder of this function only applies to calculating primary payments

	//do nothing if no allowable is given
	if(cyTotalAllowable <= COleCurrency(0,0)) {
		// (j.jones 2012-08-21 10:05) - PLID 52029 - if not silent, warn
		if(!bSilent) {
			CString strMessage;
			strMessage.Format("A payment could not be calculated for the revenue code %s because no allowable has been entered for this line.", strRevCodeDesc);
			MessageBox(strMessage, "Practice", MB_ICONINFORMATION|MB_OK);
		}
		return COleCurrency(0,0);
	}

	COleCurrency cyNewPayment = COleCurrency(0,0);

	//In all cases, if they typed in a deductible, subtract it from the allowable.
	//Coinsurance would be calculated on the remaining amount.
	COleCurrency cyDeductibleManualAmt = VarCurrency(varDeductibleManualAmount, COleCurrency(0,0));
	if(cyDeductibleManualAmt > COleCurrency(0,0)) {
		cyTotalAllowable -= cyDeductibleManualAmt;
		if(cyTotalAllowable < COleCurrency(0,0)) {
			cyTotalAllowable = COleCurrency(0,0);
		}
	}

	//If co-insurance was typed in, just subtract that from the allowable (which may have
	//already been reduced by the deductible), the new payment is always the balance.
	//In this case, typing in $0.00 does mean that a value was entered, $0.00 is very different than null.
	// (j.jones 2013-08-27 11:51) - PLID 57398 - Added copays, which adhere to the same logic.
	// Type one in, and we won't try to auto-calculate.
	if(varCoinsuranceManualAmount.vt == VT_CY || varCopayManualAmount.vt == VT_CY) {
		COleCurrency cyCoinsuranceManualAmt = VarCurrency(varCoinsuranceManualAmount, COleCurrency(0,0));
		COleCurrency cyCopayManualAmt = VarCurrency(varCopayManualAmount, COleCurrency(0,0));
		
		//the amount paid would be the allowable balance after the deductible, coinsurance, and copay are taken out,
		//we don't need to estimate the co-insurance or copay because they typed in the actual amount the patient owes

		//if they entered a deductible, we would have already subtracted it from the allowable
		cyNewPayment = cyTotalAllowable - cyCoinsuranceManualAmt - cyCopayManualAmt;
	}
	else {
		//We don't have the co-insurance, we must estimate.

		//We need to get the copay info. and co-insurance percent for each charge in this revenue code.
		//If the expected allowable total matches our current allowable total, then we
		//can recalculate for each charge based off their pay group information.
		//If the expected allowable total does not match our current allowable total,
		//then we can only recalculate if all charges have the same pay group information.
		
		//initalize to -2 to reflect no data
		COleCurrency cyTotalCopayMoney = COleCurrency(-2,0);
		long nTotalCopayPercent = -2;
		long nTotalCoinsurancePercent = -2;

		COleCurrency cyTotalExpectedAllowable = COleCurrency(0,0);
		COleCurrency cyTotalExpectedPayment = COleCurrency(0,0);
		
		// (d.lange 2015-11-30 14:40) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
		// (j.jones 2016-03-04 09:52) - PLID 68525 - fixed group by for AllowableInsuredQ.InsuranceCoID
		_RecordsetPtr rsCharges = CreateParamRecordset("SELECT "
			"ChargesT.ID AS ChargeID, dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmt, "
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CoInsurance ELSE DefaultPayGroupInfoQ.CoInsurance END AS DefaultCoInsurancePercent, "
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CopayMoney ELSE DefaultPayGroupInfoQ.CopayMoney END AS DefaultCopayMoney, "
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CopayPercentage ELSE DefaultPayGroupInfoQ.CopayPercentage END AS DefaultCopayPercent, "
			"Round(Convert(money, "
			"	dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, COALESCE(AllowableInsuredQ.InsuranceCoID, -1)) "
			"	* ChargesT.Quantity * "
			"	(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
			"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"	(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
			"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"	), 2) AS AllowableQty "
			"FROM "
			"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
			"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
			"	WHERE InsuranceCoID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT}) "
			") AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
			"LEFT JOIN ServicePayGroupsT InsServicePayGroupsT ON InsPayGroupLinkQ.PayGroupID = InsServicePayGroupsT.ID "
			"LEFT JOIN (SELECT PayGroupID, CopayMoney, CopayPercentage, CoInsurance, DeductibleRemaining, OOPRemaining "
			"	FROM InsuredPartyPayGroupsT "
			"	WHERE InsuredPartyID = {INT} "
			") AS DefaultPayGroupInfoQ ON ServicePayGroupsT.ID = DefaultPayGroupInfoQ.PayGroupID "
			"LEFT JOIN (SELECT PayGroupID, CopayMoney, CopayPercentage, CoInsurance, DeductibleRemaining, OOPRemaining "
			"	FROM InsuredPartyPayGroupsT "
			"	WHERE InsuredPartyID = {INT} "
			") AS InsurancePayGroupInfoQ ON InsServicePayGroupsT.ID = InsurancePayGroupInfoQ.PayGroupID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
			"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
			"	INNER JOIN (SELECT * FROM InsuredPartyT WHERE PersonID = {INT}) AS InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ChargeRespT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
			"LEFT JOIN ( "
			"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
			"	FROM InsuredPartyT "
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.ID = {INT} AND "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) = {INT} "
			"GROUP BY ChargesT.ID, ChargesT.Quantity, AllowableInsuredQ.InsuranceCoID, ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, "
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CoInsurance ELSE DefaultPayGroupInfoQ.CoInsurance END, "
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CopayMoney ELSE DefaultPayGroupInfoQ.CopayMoney END, "
			"CASE WHEN InsServicePayGroupsT.ID Is Not Null THEN InsurancePayGroupInfoQ.CopayPercentage ELSE DefaultPayGroupInfoQ.CopayPercentage END ",
			m_nInsuredPartyID, m_nInsuredPartyID, m_nInsuredPartyID,
			m_nInsuredPartyID, m_nBillID, nRevCodeID);
		while(!rsCharges->eof) {

			//if the charge amount is $0.00, don't calculate anything
			if(AdoFldCurrency(rsCharges, "ChargeAmt", COleCurrency(0,0)) == COleCurrency(0,0)) {
				rsCharges->MoveNext();
				continue;
			}

			//get the copay $
			COleCurrency cyCopayMoney = VarCurrency(rsCharges->Fields->Item["DefaultCopayMoney"]->Value, COleCurrency(-1,0));
			if(cyCopayMoney != COleCurrency(-1,0)) {
				if(cyTotalCopayMoney == COleCurrency(-2,0)) {
					//-2 means uninitialized
					cyTotalCopayMoney = cyCopayMoney;
				}
				else if(cyTotalCopayMoney != cyCopayMoney) {
					//the prior copay percent doesn't match, switch it to -1 to ensure
					//we have no valid copay $
					cyTotalCopayMoney = COleCurrency(-1,0);
				}
			}
			else if(cyTotalCopayMoney != COleCurrency(-2,0)){
				//if the total is -2 (uninitialized), keep it as such,
				//but if it has been filled in, set the total to -1
				//to ensure we have no valid copay %
				cyTotalCopayMoney = COleCurrency(-1,0);
			}

			//get the copay percent
			long nCopayPercent = VarLong(rsCharges->Fields->Item["DefaultCopayPercent"]->Value, -1);
			if(nCopayPercent != -1) {
				if(nTotalCopayPercent == -2) {
					//-2 means uninitialized
					nTotalCopayPercent = nCopayPercent;
				}
				else if(nTotalCopayPercent != nCopayPercent) {
					//the prior copay percent doesn't match, switch it to -1 to ensure
					//we have no valid copay %
					nTotalCopayPercent = -1;
				}
			}
			else if(nTotalCopayPercent != -2){
				//if the total is -2 (uninitialized), keep it as such,
				//but if it has been filled in, set the total to -1
				//to ensure we have no valid copay %
				nTotalCopayPercent = -1;
			}

			//get the coinsurance percent
			long nCoinsurancePercent = VarLong(rsCharges->Fields->Item["DefaultCoInsurancePercent"]->Value, -1);
			if(nCoinsurancePercent != -1) {
				if(nTotalCoinsurancePercent == -2) {
					//-2 means uninitialized
					nTotalCoinsurancePercent = nCoinsurancePercent;
				}
				else if(nTotalCoinsurancePercent != nCoinsurancePercent) {
					//the prior coinsurance percent doesn't match, switch it to -1 to ensure
					//we have no valid coinsurance
					nTotalCoinsurancePercent = -1;
				}
			}
			else if(nTotalCoinsurancePercent != -2){
				//if the total is -2 (uninitialized), keep it as such,
				//but if it has been filled in, set the total to -1
				//to ensure we have no valid coinsurance
				nTotalCoinsurancePercent = -1;
			}

			COleCurrency cyAllowedAmount = AdoFldCurrency(rsCharges, "AllowableQty", COleCurrency(0,0));
			cyTotalExpectedAllowable += cyAllowedAmount;

			//these cannot be accurately calculated when a deductible is entered,
			//and will be ignored later if a deductible is used

			//First handle copays. If the pay group has a copay, subtract it first.
			//No pay group should have a copay $ and copay %, if they do give the
			//money value priority and ignore the percentage value.

			if(cyCopayMoney > COleCurrency(0,0)) {
				//they have a copay $, subtract it from the allowable
				cyAllowedAmount -= cyCopayMoney;
			}
			else if(nCopayPercent >= 0) {
				//they have a copay %, subtract it from the allowable
				COleCurrency cyCopay = CalculatePercentOfAmount(cyAllowedAmount, nCopayPercent);
				cyAllowedAmount -= cyCopay;
			}

			//now calculate coinsurance (it would not be normal for copay and coinsurance to exist at the same time)
			if(nCoinsurancePercent == -1) {
				//this case means they had a copay and not a coinsurance, which is pretty normal,
				//they would never actually have both

				//we would have reduced the allowable by the copay amount, insurance would pay the rest
				cyTotalExpectedPayment += cyAllowedAmount;
			}
			else {
				//co-insurance is the amount the patient should pay, so the insurance
				//payment amount is the rest. ie. 20% coinsurance means insurance pays 80%
				long nPercentInsPaid = 100 - nCoinsurancePercent;
				COleCurrency cyExpectedPayment = CalculatePercentOfAmount(cyAllowedAmount, nPercentInsPaid);
				cyTotalExpectedPayment += cyExpectedPayment;
			}

			rsCharges->MoveNext();
		}
		rsCharges->Close();

		//If the expected allowable total matches our current allowable total,
		//and no deductible was entered, then we can use the calculated payment total for each charge.
		if(cyTotalExpectedAllowable == cyTotalAllowable && cyDeductibleManualAmt == COleCurrency(0,0)) {

			//if both a coinsurance (or copay) and deductible are entered, the code earlier in
			//this function would have caught it, now we only handle the case if deductible
			//was not entered (or was zero)

			//if a co-insurance or copay was entered, we should have already returned earlier
			ASSERT(varCoinsuranceManualAmount.vt != VT_CY);
			ASSERT(varCopayManualAmount.vt != VT_CY);
			
			//we already calculated the expected payment per charge, and since the deductible
			//is zero and no coinsurance was entered, we can return that already-calculated value
			cyNewPayment = cyTotalExpectedPayment;
		}
		//If the expected allowable total does not match our current allowable total,
		//then we can only recalculate if each charge has the same pay group information across the board.
		//We would have set each of these values to -1 if they mismatched (-2 would mean none found at all)
		else if(cyTotalCopayMoney >= COleCurrency(0,0) || nTotalCopayPercent >= 0 || nTotalCoinsurancePercent >= 0) {

			//If we got here, at least one pay group value (copay $, copay %, coinsurance %) is filled in
			//with the same value for all charges in this revenue code.
			//However, if any are -1, then the value is mismatched across the charges, and we can't calculate.
			//-2 is ok, because it would mean that the field is blank for all pay groups, which is to be expected.
			if(cyTotalCopayMoney == COleCurrency(-1,0) || nTotalCopayPercent == -1 || nTotalCoinsurancePercent == -1) {
				//one of these fields has mismatched values across the pay groups, so we cannot calculate an amount
				// (j.jones 2012-08-21 10:05) - PLID 52029 - if not silent, warn
				if(!bSilent) {
					CString strMessage;
					//This message is slightly misleading, because if we do have an allowable and they didn't change it,
					//then we can calculate for multiple pay groups. But they should never have a revenue code spread
					//across multiple pay groups, so this message is still telling them valid information.
					strMessage.Format("A payment could not be calculated for the revenue code %s because "
						"no valid copay or coinsurance information has been entered for each charge's pay group.\n\n"
						"Please ensure that each service linked to this revenue code is in the same pay group, "
						"and that the insured party has copay/coinsurance entered for that pay group.", strRevCodeDesc);
					MessageBox(strMessage, "Practice", MB_ICONINFORMATION|MB_OK);
				}
				return COleCurrency(0,0);
			}

			//if a deductible was entered, it would have already been subtracted from the allowable,
			//co-insurance will now be calculated on the remaining allowable amount

			//if a co-insurance or copay was entered, we should have already returned earlier
			ASSERT(varCoinsuranceManualAmount.vt != VT_CY);
			ASSERT(varCopayManualAmount.vt != VT_CY);

			//First handle copays. If the pay group has a copay, subtract it first.
			//No pay group should have a copay $ and copay %, if they do give the
			//money value priority and ignore the percentage value.

			if(cyTotalCopayMoney > COleCurrency(0,0)) {
				//they have a copay $, subtract it from the allowable
				cyTotalAllowable -= cyTotalCopayMoney;
			}
			else if(nTotalCopayPercent >= 0) {
				//they have a copay %, subtract it from the allowable
				COleCurrency cyCopay = CalculatePercentOfAmount(cyTotalAllowable, nTotalCopayPercent);
				cyTotalAllowable -= cyCopay;
			}

			//now calculate coinsurance (it would not be normal for copay and coinsurance to exist at the same time)
			if(nTotalCoinsurancePercent == -2) {
				//this case means they had a copay and not a coinsurance, which is pretty normal,
				//they would never actually have both

				//we would have reduced the allowable by the copay amount, insurance would pay the rest
				cyNewPayment += cyTotalAllowable;
			}
			else if(nTotalCoinsurancePercent >= 0) {
				//co-insurance is the amount the patient should pay, so the insurance
				//payment amount is the rest. ie. 20% coinsurance means insurance pays 80%
				long nPercentInsPaid = 100 - nTotalCoinsurancePercent;
				COleCurrency cyExpectedPayment = CalculatePercentOfAmount(cyTotalAllowable, nPercentInsPaid);
				cyNewPayment += cyExpectedPayment;
			}
		}
	}

	//Now cyNewPayment should be the expected amount paid by insurance. In the unlikely
	//case that the insurance already paid once, or for some reason this payment is greater
	//than the insurance balance, we still use our calculated payment. It's the user's
	//responsibility to change it to be a lower amount, not ours.

	//never return a negative balance
	if(cyNewPayment < COleCurrency(0,0)) {
		return COleCurrency(0,0);
	}
	else {
		return cyNewPayment;
	}
}

// (j.jones 2012-07-27 10:19) - PLID 26877 - added ability to filter reason codes
void CFinancialLineItemPostingDlg::OnBtnFilterReasonCodes()
{
	try {

		//when not filtering, this is the default where clause
		CString strDefaultWhereClause = "Type = 2 AND Inactive = 0";

		//see if there is currently a reason code
		long nCurReasonCodeID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReasonList->GetCurSel();
		if (pRow) {
			nCurReasonCodeID = VarLong(pRow->GetValue(rcccID), -1);
		}

		BOOL bCurrentlyShowingAllRecords = (CString((LPCTSTR)m_pReasonList->WhereClause) == strDefaultWhereClause);

		if(nCurReasonCodeID != -1 && bCurrentlyShowingAllRecords) {
			if(IDNO == DontShowMeAgain(this, "Filtering the reason code list will clear out your current selection.  Are you sure you wish to do this?", 
				"LineItemPostingFilterReasonCodes", "Warning", FALSE, TRUE))
			{
				//they do not want to filter
				return;
			}
		}

		//If default where clause is applied, a filter should be created and applied
		if(bCurrentlyShowingAllRecords)
		{
			//Apply filter
			if (FilterDatalist2(m_pReasonList, rcccDescription, rcccID))
			{
				m_pReasonList->CurSel = NULL;
				//Set the filter button's icon
				m_btnFilterReasonCodes.SetIcon(IDI_FILTERDN);
				m_btnFilterReasonCodes.RedrawWindow();
			}
			else {
				//Nothing found. This would have requeried, so we need
				//to add the "none" row.
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReasonList->GetNewRow();
				pRow->PutValue(rcccID, (long)-1);
				pRow->PutValue(rcccCode, "");
				pRow->PutValue(rcccDescription, "<No Reason Code>");
				m_pReasonList->AddRowSorted(pRow, NULL);
			}
		}
		else
		{
			//Remove the filter, and retain the currently selected item in the list
			_variant_t selId = g_cvarNull;
			NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pReasonList->CurSel;
			if(pCurSel) {
				selId = pCurSel->GetValue(rcccID);
			}

			//Set the default where clause and requery
			m_pReasonList->WhereClause = _bstr_t(strDefaultWhereClause);
			for (short i=0; i < m_pReasonList->ColumnCount; i++) {
				m_pReasonList->GetColumn(i)->BackColor = RGB(255,255,255);
			}
			m_pReasonList->Requery();

			//Set the filter button's icon
			m_btnFilterReasonCodes.SetIcon(IDI_FILTER);
			m_btnFilterReasonCodes.RedrawWindow();

			//add the "none" row
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReasonList->GetNewRow();
			pRow->PutValue(rcccID, (long)-1);
			pRow->PutValue(rcccCode, "");
			pRow->PutValue(rcccDescription, "<No Reason Code>");
			m_pReasonList->AddRowSorted(pRow, NULL);
		}

		//Restore the selected item in the list
		NXDATALIST2Lib::IRowSettingsPtr pNewSel = NULL;
		if(nCurReasonCodeID != -1) {
			pNewSel = m_pReasonList->SetSelByColumn(rcccID, (long)nCurReasonCodeID);
		}
		if(!pNewSel) {
			m_pReasonList->SetSelByColumn(rcccID, (long)-1);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-16 10:37) - PLID 52162 - Added ability to auto-post,
// which should only be called when the dialog is invisible, not when it is modal.
// If it cannot auto-post, the caller should display m_strAutoPostFailure and
// then try to open again as a modal dialog.
bool CFinancialLineItemPostingDlg::AutoPost()
{
	try {
		//before trying the post process, let's look for data that we know we should not
		//be auto-posting, like missing payments

		BOOL bHasOneValidPayment = FALSE;
	
		for(int i=0;i<m_PostingList->GetRowCount();i++) {
			IRowSettingsPtr pRow = m_PostingList->GetRow(i);
			COleCurrency cyCharges = VarCurrency(pRow->GetValue(m_nChargesColIndex),COleCurrency(0,0));
			_variant_t varNewPays = pRow->GetValue(m_nNewPaysColIndex);
			//Payments can be NULL or $0.00, we default them to NULL if they are calculated as zero.
			//So just check for NULL. That way if we ever intentionally fill $0.00, that would be
			//considered a valid payment amount.
			if(cyCharges > COleCurrency(0,0) && varNewPays.vt == VT_NULL) {
				//no payment, we can't post this if it is primary

				// (j.jones 2012-08-29 11:27) - PLID 52351 - for secondaries only, we allow posting if some
				// charges have no payment, provided that at least one charge does have a payment
				if(m_bIsPrimaryIns) {
					m_strAutoPostFailure = "At least one charge does not have an estimated payment. A payment amount will need to be manually entered.";
					return false;
				}
			}

			if(cyCharges > COleCurrency(0,0) && VarCurrency(varNewPays, COleCurrency(0,0)) > COleCurrency(0,0)) {
				bHasOneValidPayment = TRUE;
			}
		}

		// (j.jones 2012-08-29 11:27) - PLID 52351 - For secondaries only, we allow posting if some
		// charges have no payment, provided that at least one charge does have a payment.
		// Fail if no charge has a payment.
		if(!m_bIsPrimaryIns && !bHasOneValidPayment) {
			m_strAutoPostFailure = "No charges have an estimated payment. A payment amount will need to be manually entered.";
			return false;
		}


		//now try to post, it may yet fail
		bool bResult = PostPayments();
		return bResult;

	}NxCatchAll(__FUNCTION__)

	return false;
}

// (j.jones 2012-08-17 09:00) - PLID 47778 - added payment calculations
//Sums the payments column, puts that total in the edit box in the payments section.
void CFinancialLineItemPostingDlg::AutoFillPaymentTotal()
{
	try {

		//do not do this on a batch payment
		if(m_nBatchPaymentID != -1) {
			//the caller shouldn't have called this on a non-batch payment,
			//review the calling code to make sure it's not depending on this
			//function to actually do something
			ASSERT(FALSE);
			return;
		}

		COleCurrency cyPaymentTotal = COleCurrency(0,0);
		for(int i = 0; i < m_PostingList->GetRowCount(); i++) {
			cyPaymentTotal += VarCurrency(m_PostingList->GetValue(i, m_nNewPaysColIndex), COleCurrency(0,0));
		}
		SetDlgItemText(IDC_PAY_TOTAL, FormatCurrencyForInterface(cyPaymentTotal, FALSE, TRUE));

		//If we changed the amount to be greater than the cash received,
		//update the cash received box to be the same
		//(note: this of course would make no sense - but does protect us from altering things
		//unnecessarily during an OnKillFocus)
		CString strCashReceived;
		COleCurrency cyCashReceived = COleCurrency(0,0);
		GetDlgItemText(IDC_CASH_RECEIVED_LIP, strCashReceived);
		if(strCashReceived.GetLength() > 0) {
			COleCurrency cyCashReceived = ParseCurrencyFromInterface(strCashReceived);
			if(cyCashReceived.GetStatus() == COleCurrency::invalid) {
				cyCashReceived = COleCurrency(0,0);
			}
			if(cyCashReceived < cyPaymentTotal) {
				cyCashReceived = cyPaymentTotal;
				SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(cyCashReceived,FALSE,TRUE));
			}
		}
		else {
			cyCashReceived = cyPaymentTotal;
			SetDlgItemText(IDC_CASH_RECEIVED_LIP,FormatCurrencyForInterface(cyCashReceived,FALSE,TRUE));
		}

		//fill change given
		COleCurrency cyChangeGiven = cyCashReceived - cyPaymentTotal;
		if(cyChangeGiven < COleCurrency(0,0)) {
			cyChangeGiven = COleCurrency(0,0);
		}
		CString strChangeGiven = FormatCurrencyForInterface(cyChangeGiven,FALSE,TRUE);
		SetDlgItemText(IDC_CHANGE_GIVEN_LIP,strChangeGiven);

		CalculateTotals();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-21 09:07) - PLID 52029 - added estimate payments button
void CFinancialLineItemPostingDlg::OnBtnEstPayments()
{
	try {

		// (j.jones 2012-08-28 17:45) - PLID 52335 - this is now allowed for
		// all insurance postings, but never for a patient posting
		if(m_nInsuredPartyID == -1) {
			//this should be impossible, just assert (bad code)
			//and disable the button
			ASSERT(FALSE);
			
			m_btnEstPayments.EnableWindow(FALSE);
		}

		// (j.jones 2014-07-02 14:57) - PLID 62548 - this should not be accessible on vision payments
		if (m_ePayType == eVisionPayment) {
			//this should be impossible, just assert (bad code)
			//and disable the button
			ASSERT(FALSE);

			m_btnEstPayments.EnableWindow(FALSE);
		}

		//Clicking this button will recalculate an estimated payment for each charge
		//in the list, overwriting anything they may have manually typed in.
		//Adjustments will also be recalculated.


		//warn about this
		// (j.jones 2012-08-28 17:47) - PLID 52335 - give a different warning for primary and secondary
		if(m_bIsPrimaryIns) {
			if(DontShowMeAgain(this, "Practice can estimate a primary responsibility's expected payments using the allowable amount, copay and coinsurance values, and the patient's deductible.\n\n"
				"Are you sure you wish to estimate payments for these services?", "LineItemPostingEstPayments", "Estimating Primary Insurance Payments", FALSE, TRUE) == IDNO) {
				return;
			}
		}
		else {
			//secondary uses much simpler logic, but we need to warn that it's different from primary
			if(DontShowMeAgain(this, "Practice can estimate a secondary responsibility's expected payments using the current insurance balance.\n\n"
				"Are you sure you wish to estimate payments for these services?", "LineItemPostingEstPayments_Secondary", "Estimating Secondary Insurance Payments", FALSE, TRUE) == IDNO) {
				return;
			}
		}

		BOOL bFillPaymentTotal = FALSE;

		for(int i=0;i<m_PostingList->GetRowCount();i++) {
			//loop through and get totals, and test that the posting is balanced
			IRowSettingsPtr pRow = m_PostingList->GetRow(i);
			long nChargeID = VarLong(pRow->GetValue(plcChargeID), -1);
			long nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID), -1);
			COleCurrency cyCharges = VarCurrency(pRow->GetValue(m_nChargesColIndex),COleCurrency(0,0));
			COleCurrency cyPatResp = VarCurrency(pRow->GetValue(m_nPatRespColIndex),COleCurrency(0,0));
			COleCurrency cyInsResp = VarCurrency(pRow->GetValue(m_nInsRespColIndex),COleCurrency(0,0));
			COleCurrency cyExistingPatPays = VarCurrency(pRow->GetValue(m_nExistingPatPaysColIndex),COleCurrency(0,0));
			COleCurrency cyExistingInsPays = VarCurrency(pRow->GetValue(m_nExistingInsPaysColIndex),COleCurrency(0,0));
			COleCurrency cyAllowable = VarCurrency(pRow->GetValue(m_nAllowableColIndex),COleCurrency(0,0));
			COleCurrency cyInsBal = VarCurrency(pRow->GetValue(m_nInsBalanceColIndex),COleCurrency(0,0));
			COleCurrency cyCurNewPays = VarCurrency(pRow->GetValue(m_nNewPaysColIndex),COleCurrency(0,0));

			BOOL bIsRevCode = IsDlgButtonChecked(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE) && nRevCodeID != -1;

			COleCurrency cyNewInsPays = COleCurrency(0,0);
			COleCurrency cyNewAdj = COleCurrency(0,0);
			//don't calculate payments on zero dollar charges, they should default to no payment
			if(cyCharges > COleCurrency(0,0)) {
				if(bIsRevCode) {
					// (j.jones 2012-08-28 17:56) - PLID 52335 - the balance should calculate as whatever the current balance is,
					// with the current new payments added back into it
					cyNewInsPays = CalculatePaymentForRevenueCode(FALSE, VarString(pRow->GetValue(plcItemCode), ""), nRevCodeID, cyCharges, cyInsBal + cyCurNewPays,
						cyAllowable, pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
				}
				else {
					// (j.jones 2012-08-28 17:56) - PLID 52335 - the balance should calculate as whatever the current balance is,
					// with the current new payments added back into it
					cyNewInsPays = CalculatePayment(FALSE, VarString(pRow->GetValue(plcItemCode), ""), cyCharges, cyInsBal + cyCurNewPays,
						cyAllowable, pRow->GetValue(plcCoinsurancePercent),
						pRow->GetValue(plcCopayMoney), pRow->GetValue(plcCopayPercent),
						pRow->GetValue(m_nDeductibleAmtColIndex), pRow->GetValue(m_nCoinsuranceAmtColIndex), pRow->GetValue(m_nCopayAmtColIndex));
				}

				// (j.jones 2013-07-03 16:58) - PLID 57226 - if the new pays are <= $0.00, we're going to be placing
				// null in the new payments column, so in turn we must pass in null here, otherwise the total payments
				cyNewAdj = CalculateAdjustment_Medical(cyInsResp, cyPatResp,
					cyNewInsPays <= COleCurrency(0,0) ? g_cvarNull : _variant_t(cyExistingInsPays + cyNewInsPays),
					cyAllowable);
			}

			if(cyNewInsPays <= COleCurrency(0,0)) {
				//if zero, clear out the payment
				cyNewInsPays = COleCurrency(0,0);
				pRow->PutValue(m_nNewPaysColIndex, g_cvarNull);
			}
			else {
				//only if we fill one non-zero payment should we recalculate the payment total
				bFillPaymentTotal = TRUE;

				pRow->PutValue(m_nNewPaysColIndex, _variant_t(cyNewInsPays));
			}

			//always fill the adjustment, even if it is zero
			pRow->PutValue(m_nAdjustmentColIndex,_variant_t(cyNewAdj));

			//update the balance
			COleCurrency cyTotalInsPays = cyExistingInsPays + cyNewInsPays;
			COleCurrency cyBalance = (cyInsResp - cyTotalInsPays - cyNewAdj);
			pRow->PutValue(m_nInsBalanceColIndex, _variant_t(cyBalance));

			//clear the edited flags
			pRow->PutValue(plcHasEditedPayment, g_cvarFalse);
			pRow->PutValue(plcHasEditedAdjustment, g_cvarFalse);

			//if the balance is negative, warn the user
			if(cyBalance < COleCurrency(0,0)) {
				CString strMessage;
				if(bIsRevCode) {
					CString strRevCodeDesc;
					strRevCodeDesc.Format("%s for %s", VarString(pRow->GetValue(plcItemCode), ""), FormatCurrencyForInterface(cyCharges));
					strMessage.Format("The calculated payment for the revenue code %s is greater than the available insurance balance.\n\n"
						"Please double check this line before posting.", strRevCodeDesc);
				}
				else {
					CString strItemCode = VarString(pRow->GetValue(plcItemCode), "");
					CString strChargeDesc;
					if(strItemCode.IsEmpty()) {
						strChargeDesc.Format("%s charge", FormatCurrencyForInterface(cyCharges));
					}
					else {
						strChargeDesc.Format("%s charge for code %s", FormatCurrencyForInterface(cyCharges), strItemCode);
					}
					strMessage.Format("The calculated payment for the %s is greater than the available insurance balance.\n\n"
						"Please double check this line before posting.", strChargeDesc);
				}
				MessageBox(strMessage, "Practice", MB_ICONINFORMATION|MB_OK);
			}
		}

		//if not a batch payment, fill the payment total with anything we may have auto-entered
		if(m_nBatchPaymentID == -1 && bFillPaymentTotal) {
			AutoFillPaymentTotal();
		}

		CalculateTotals();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-14 14:39) - PLID 50285 - added ability to create a billing note with the user-entered
// deductible and/or coinsurance values
// (j.jones 2013-08-26 16:45) - PLID 57398 - also applies to copays now
void CFinancialLineItemPostingDlg::CreateBillingNoteForDeductibleCoinsCopay(long nChargeID, _variant_t varDeductible, _variant_t varCoinsurance, _variant_t varCopay)
{
	try {

		if((varDeductible.vt == VT_NULL && varCoinsurance.vt == VT_NULL && varCopay.vt == VT_NULL)
			|| nChargeID == -1 || m_nInsuredPartyID == -1) {
			//should be impossible, this function should not have been called
			ASSERT(FALSE);
			return;
		}

		if(m_PayRespCombo->GetCurSel() == -1) {
			//should be impossible, how do we have a m_nInsuredPartyID?
			ASSERT(FALSE);
			return;
		}

		//get the insurance name
		CString strInsuranceCoName = VarString(m_PayRespCombo->GetValue(m_PayRespCombo->GetCurSel(), rccName));

		COleCurrency cyDeductible = VarCurrency(varDeductible, COleCurrency(0,0));
		COleCurrency cyCoinsurance = VarCurrency(varCoinsurance, COleCurrency(0,0));
		// (j.jones 2013-08-26 16:47) - PLID 57398 - added copays
		COleCurrency cyCopay = VarCurrency(varCopay, COleCurrency(0,0));

		//we won't make a note if all are zero, so if they are, just leave
		if(cyDeductible == COleCurrency(0,0) && cyCoinsurance == COleCurrency(0,0) && cyCopay == COleCurrency(0,0)) {
			return;
		}

		//first check the preference whether they want a note to be created
		if(GetRemotePropertyInt("LineItemPosting_DedBillingNote", 1, 0, "<None>") == 0) {
			return;
		}

		long nCategoryID = GetRemotePropertyInt("LineItemPosting_DedBillingNote_DefCategory", -1, 0, "<None>", true);
		//don't waste a recordset validating that the category still exists, we will do so in the execute

		BOOL bShowOnStatement = (GetRemotePropertyInt("LineItemPosting_DedBillingNote_ShowOnStatement", 0, 0, "<None>", true) != 0);

		//now build the note such as: "Aetna states a Deductible of $14.29, Coinsurance of $22.83"
		
		CString strNote;
		strNote.Format("%s states a ", strInsuranceCoName);

		if(cyDeductible > COleCurrency(0,0)) {
			CString strDeductible;
			strDeductible.Format("Deductible of %s", FormatCurrencyForInterface(cyDeductible));
			strNote += strDeductible;
		}
		if(cyCoinsurance > COleCurrency(0,0)) {
			CString strCoinsurance;
			strCoinsurance.Format("Coinsurance of %s", FormatCurrencyForInterface(cyCoinsurance));

			if(cyDeductible > COleCurrency(0,0)) {
				strNote += ", ";
			}

			strNote += strCoinsurance;
		}
		if(cyCopay > COleCurrency(0,0)) {
			CString strCopay;
			strCopay.Format("Copay of %s", FormatCurrencyForInterface(cyCopay));

			if(cyDeductible > COleCurrency(0,0) || cyCoinsurance > COleCurrency(0,0)) {
				strNote += ", ";
			}

			strNote += strCopay;
		}

		strNote += ".";

		//now save the note
		CreateBillingNote(m_nPatientID, nChargeID, strNote, nCategoryID, bShowOnStatement);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-06-27 14:57) - PLID 62631 - exposed the PayType
EBatchPaymentPayType CFinancialLineItemPostingDlg::GetPayType()
{
	return m_ePayType;
}

// (j.jones 2014-06-27 11:28) - PLID 62548 - For Vision payments, this hides many controls and columns.
// For Medical payments, will only hide vision-specific controls.
void CFinancialLineItemPostingDlg::UpdateVisibleControls()
{
	try {

		if (m_ePayType == eVisionPayment) {
			//hide the following controls
			
			//The ‘Estimate Payments’ button.
			GetDlgItem(IDC_BTN_EST_PAYMENTS)->ShowWindow(SW_HIDE);

			//Adjustment Total Amount field
			GetDlgItem(IDC_ADJUSTMENT_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADJ_CURRENCY_SYMBOL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADJ_AMT_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADJ_TOTAL)->ShowWindow(SW_HIDE);
			
			//Adjustment Date
			GetDlgItem(IDC_ADJ_DATE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADJ_DATE)->ShowWindow(SW_HIDE);
			
			//Group Code
			GetDlgItem(IDC_ADJ_GROUPCODE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADJ_GROUPCODE)->ShowWindow(SW_HIDE);

			//Reason Code
			GetDlgItem(IDC_ADJ_REASON_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADJ_REASON)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_FILTER_REASON_CODES)->ShowWindow(SW_HIDE);

			//‘Post Payments Even When $0.00’ checkbox
			GetDlgItem(IDC_CHECK_ALLOW_ZERO_DOLLAR_APPLIES)->ShowWindow(SW_HIDE);

			//‘Group Charges by Revenue Code’ checkbox
			GetDlgItem(IDC_CHECK_GROUP_CHARGES_BY_REVENUE_CODE)->ShowWindow(SW_HIDE);

			//Claim Insurance Company drop down and label
			GetDlgItem(IDC_BILL_INSCO_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_POSTING_INSURANCE_CO_LIST)->ShowWindow(SW_HIDE);

			//‘Other Insurance Company’ drop down and label
			GetDlgItem(IDC_BILL_OTHER_INSCO_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_POSTING_OTHER_INSURANCE_CO_LIST)->ShowWindow(SW_HIDE);

			//‘Swap Insurance Co.on the Bill’ button
			GetDlgItem(IDC_BTN_SWAP_INSCOS)->ShowWindow(SW_HIDE);			

			//‘Auto Swap When Shifting’ checkbox
			GetDlgItem(IDC_CHECK_AUTO_SWAP)->ShowWindow(SW_HIDE);

			//‘Current Batch’ radio button selections
			GetDlgItem(IDC_CURRENT_BATCH_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_POST_UNBATCHED)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_POST_PAPER_BATCH)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_POST_ELECTRONIC_BATCH)->ShowWindow(SW_HIDE);
			
			//‘Auto Batch When Shifting’ checkbox
			GetDlgItem(IDC_CHECK_AUTO_BATCH)->ShowWindow(SW_HIDE);

			// (j.jones 2014-06-27 14:59) - PLID 62631 - hide the following columns
			
			//‘Deductible’ column
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->PutColumnStyle(csVisible|csFixedWidth);
			m_PostingList->GetColumn(m_nDeductibleAmtColIndex)->PutStoredWidth(0);

			//‘CoIns’ column
			m_PostingList->GetColumn(m_nCoinsuranceAmtColIndex)->PutColumnStyle(csVisible|csFixedWidth);
			m_PostingList->GetColumn(m_nCoinsuranceAmtColIndex)->PutStoredWidth(0);

			//‘Allowable’ column
			m_PostingList->GetColumn(m_nAllowableColIndex)->PutColumnStyle(csVisible|csFixedWidth);
			m_PostingList->GetColumn(m_nAllowableColIndex)->PutStoredWidth(0);

			// (j.jones 2014-06-30 09:27) - PLID 62642 - resize the adjustment background color
			CRect rcAdjBkg, rcChargebackBkg;
			GetDlgItem(IDC_ADJ_BKG)->GetWindowRect(&rcAdjBkg);
			ScreenToClient(&rcAdjBkg);
			GetDlgItem(IDC_CHARGEBACK_BKG)->GetWindowRect(&rcChargebackBkg);
			ScreenToClient(&rcChargebackBkg);
			rcAdjBkg.left = rcChargebackBkg.right + 5;
			GetDlgItem(IDC_ADJ_BKG)->MoveWindow(rcAdjBkg);

			// (j.jones 2014-07-24 10:31) - PLID 62550 - rename the Adjustment Balance label
			// to Adjustment Total
			SetDlgItemText(IDC_ADJUSTMENT_BAL_LABEL, "Adjustment Total");
		}
		else {
			//hide the following controls

			// (j.jones 2014-06-30 09:27) - PLID 62642 - hide chargebacks
			
			//chargeback background
			GetDlgItem(IDC_CHARGEBACK_BKG)->ShowWindow(SW_HIDE);

			//chargeback category
			GetDlgItem(IDC_CB_CATEGORY_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CB_CATEGORY_COMBO)->ShowWindow(SW_HIDE);

			//chargeback description
			GetDlgItem(IDC_CB_DESCRIPTION_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CB_DESCRIPTION_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_CB_DESC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CB_DESC)->ShowWindow(SW_HIDE);

			// (j.jones 2014-07-01 09:04) - PLID 62551 - hide the chargeback total
			GetDlgItem(IDC_CHARGEBACK_TOTAL_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHARGEBACK_TOTAL)->ShowWindow(SW_HIDE);

			// (j.jones 2014-07-01 09:22) - PLID 62552 - hide the auto-adjust checkbox
			GetDlgItem(IDC_CHECK_AUTO_ADJUST_BALANCES)->ShowWindow(SW_HIDE);

			// (j.jones 2014-07-29 09:14) - PLID 63076 - hide the total payments labels
			GetDlgItem(IDC_TOTAL_PAYMENTS_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_TOTAL_PAYMENTS)->ShowWindow(SW_HIDE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-06-30 14:58) - PLID 62642 - added chargebacks
void CFinancialLineItemPostingDlg::OnSelChosenCbDescriptionCombo(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		CString str = VarString(pRow->GetValue(0), "");
		m_ChargebackDescriptionCombo->PutComboBoxText("");
		GetDlgItem(IDC_CB_DESC)->SetWindowText(str);
		
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-07-01 09:31) - PLID 62552 - this is for vision payments only
void CFinancialLineItemPostingDlg::OnCheckAutoAdjustBalances()
{
	try {

		if (m_ePayType != eVisionPayment) {
			//this checkbox should not have been available in non-vision payments
			ASSERT(FALSE);
			return;
		}

		bool bAutoAdjust = (m_checkAutoAdjustBalances.GetCheck() == 1);
		SetRemotePropertyInt("LineItemPosting_AutoAdjustVisionBalances", bAutoAdjust ? 1 : 0, 0, GetCurrentUserName());

		if (bAutoAdjust) {
			//recalculate all adjustments
			for (int i = 0; i < m_PostingList->GetRowCount(); i++) {
				IRowSettingsPtr pRow = m_PostingList->GetRow(i);
				COleCurrency cyPatResp = VarCurrency(pRow->GetValue(m_nPatRespColIndex), COleCurrency(0, 0));
				COleCurrency cyInsResp = VarCurrency(pRow->GetValue(m_nInsRespColIndex), COleCurrency(0, 0));
				COleCurrency cyExistingInsPays = VarCurrency(pRow->GetValue(m_nExistingInsPaysColIndex), COleCurrency(0, 0));
				COleCurrency cyNewPays = VarCurrency(pRow->GetValue(m_nNewPaysColIndex), COleCurrency(0, 0));
				COleCurrency cyNewInsPays = COleCurrency(0, 0);
				if (m_nInsuredPartyID != -1) {
					cyNewInsPays = cyNewPays;
				}
				COleCurrency cyTotalInsPays = cyExistingInsPays + cyNewInsPays;

				COleCurrency cyNewAdj = COleCurrency(0, 0);
				//this calculates for chargebacks too, it will only not calculate if
				//the payment column is null
				cyNewAdj = CalculateAdjustment_Vision(cyInsResp, cyPatResp,
					pRow->GetValue(m_nNewPaysColIndex).vt == VT_CY ? _variant_t(cyTotalInsPays) : g_cvarNull);

				pRow->PutValue(m_nAdjustmentColIndex, _variant_t(cyNewAdj));

				//if we are tracking a pointer for this adjustment, update it
				long nChargeID = VarLong(pRow->GetValue(plcChargeID), -1);
				long nRevCodeID = VarLong(pRow->GetValue(plcRevCodeID), -1);
				ChargeAdjustmentInfo *pAdjInfo = FindChargeAdjustmentInfo(nChargeID, nRevCodeID);
				if (pAdjInfo) {
					//updates adjustment amounts in tracked pointers
					UpdateAdjustmentPointers(cyNewAdj, pAdjInfo);
				}

				AutoFillAdjustmentTotal();

				CalculateTotals();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-07-01 11:41) - PLID 62553 - supported chargebacks
void CFinancialLineItemPostingDlg::CreateChargeback(long nChargeID, COleCurrency cyChargebackAmt)
{
	//throw exceptions to the caller

	if (nChargeID == -1 || cyChargebackAmt >= COleCurrency(0, 0)) {
		//this function should not have been called
		ASSERT(FALSE);
		return;
	}

	long nInsuredPartyID = -1;
	if (m_PayRespCombo->GetCurSel() != -1) {
		nInsuredPartyID = VarLong(m_PayRespCombo->GetValue(m_PayRespCombo->GetCurSel(), 0), -1);
	}

	//A chargeback is a negative payment, balanced out by a positive adjustment.
	//Both are applied to the charge itself.
	
	long nPaymentID = CreatePayment(cyChargebackAmt, eChargebackPayment, nInsuredPartyID, m_nBatchPaymentID);
	long nAdjustmentID = CreatePayment(-cyChargebackAmt, eChargebackAdjustment, nInsuredPartyID, m_nBatchPaymentID);

	//apply both to the charge, and save this record in data	
	CString strSqlBatch;
	CNxParamSqlArray aryParams;
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeID INT");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespID INT");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespDetailID INT");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @PaymentID INT");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @AdjustmentID INT");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargebackAmt MONEY");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ApplyID INT");
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @ChargeID = {INT}", nChargeID);
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @PaymentID = {INT}", nPaymentID);
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @AdjustmentID = {INT}", nAdjustmentID);
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @ChargebackAmt = {OLECURRENCY}", cyChargebackAmt);
	if (m_nInsuredPartyID == -1) {
		//this line should be impossible to reach, we don't allow chargebacks on patient posting
		ASSERT(FALSE);
		AddStatementToSqlBatch(strSqlBatch, "SET @ChargeRespID = (SELECT ID FROM ChargeRespT WHERE ChargeID = @ChargeID AND InsuredPartyID Is Null)");
	}
	else {
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @ChargeRespID = (SELECT ID FROM ChargeRespT WHERE ChargeID = @ChargeID AND InsuredPartyID = {INT})", nInsuredPartyID);
	}

	//if the charge resp does not exist, create a $0.00 entry for it
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "IF @ChargeRespID Is Null \r\n"
		"BEGIN \r\n"
		"	INSERT INTO ChargeRespT (ChargeID, InsuredPartyID, Amount) VALUES (@ChargeID, {INT}, 0) \r\n"
		"	SET @ChargeRespID = SCOPE_IDENTITY() \r\n"
		"END \r\n", nInsuredPartyID);

	AddStatementToSqlBatch(strSqlBatch, "SET @ChargeRespDetailID = (SELECT TOP 1 ID FROM ChargeRespDetailT WHERE ChargeRespID = @ChargeRespID)");
	
	//if the charge resp detail does not exist, create a $0.00 entry for it
	AddStatementToSqlBatch(strSqlBatch, "IF @ChargeRespDetailID Is Null \r\n"
		"BEGIN \r\n"
		"	INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) VALUES (@ChargeRespID, 0, GetDate()) \r\n"
		"	SET @ChargeRespDetailID = SCOPE_IDENTITY() \r\n"
		"END \r\n");

	//now create the chargeback line
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ChargebacksT (ChargeID, PaymentID, AdjustmentID) VALUES (@ChargeID, @PaymentID, @AdjustmentID)");

	//and now create our applies
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName) "
		"VALUES (@PaymentID, @ChargeID, @ChargeRespID, @ChargebackAmt, 0, GetDate(), {STRING})", GetCurrentUserName());
	AddStatementToSqlBatch(strSqlBatch, "SET @ApplyID = SCOPE_IDENTITY()");
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount) "
		"VALUES (@ApplyID, @ChargeRespDetailID, @ChargebackAmt)");

	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName) "
		"VALUES (@AdjustmentID, @ChargeID, @ChargeRespID, -@ChargebackAmt, 0, GetDate(), {STRING})", GetCurrentUserName());
	AddStatementToSqlBatch(strSqlBatch, "SET @ApplyID = SCOPE_IDENTITY()");
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount) "
		"VALUES (@ApplyID, @ChargeRespDetailID, -@ChargebackAmt)");

	ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
}

// (r.gonet 2016-01-28 21:04) - PLID 67942 - Handle the event where the window is about to be created.
BOOL CFinancialLineItemPostingDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	try {
		if (m_bIsAutoPosting) {
			// (r.gonet 2016-01-28 21:04) - PLID 67942 - If auto-posting, the window is hidden and non-modal.
			// Don't activate it because this dialog class calls global functions that may show message boxes
			// using the active window. If this was the active window, then the message box would be non-modal
			// and they could click controls behind it, causing trouble.
			cs.dwExStyle |= WS_EX_NOACTIVATE;
		} else {
			// (r.gonet 2016-01-28 21:04) - PLID 67942 - Leave as default if posting normally.
		}
		return CNxDialog::PreCreateWindow(cs);
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

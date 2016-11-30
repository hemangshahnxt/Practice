// PaymentDlg.cpp : implementation file
//
/************************************************************************
	This source contains code for the payment dialog.
*************************************************************************/

#include "stdafx.h"
#include "Practice.h"
#include "PaymentDlg.h"
#include "NxTabView.h"
#include "GlobalUtils.h"
#include "GlobalReportUtils.h"
#include "PracProps.h"
#include "MainFrm.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "EditComboBox.h"
#include "GlobalDataUtils.h"
#include "CalculatePercentageDlg.h"
#include "AuditTrail.h"
#include "EditDrawersDlg.h"
#include "NxStandard.h"
#include "NxException.h"
#include "BillingRc.h"
#include "QuickbooksUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "Barcode.h"
#include "SalesReceiptConfigDlg.h"
#include "dontshowdlg.h"
#include "OPOSMSRDevice.h"
#include "OPOSCashDrawerDevice.h"
#include "PinPadUtils.h"
#include "VerifyCardholderInfoDlg.h"
#include "EditCreditCardsDlg.h"
#include "ProcessCreditCardDlg.h"
#include "ApplyToRefundDlg.h"
// (d.thompson 2009-06-22) - PLID 34687 - QBMS replaces IGS processing
//#include "IGS_ProcessCreditCardDlg.h"
//#include "IGSProcessingUtils.h"
//#include "QBMS_ProcessCreditCardDlg.h"	// (d.lange 2010-09-01 14:55) - PLID 40310 - Now using CreditCardProcessing.h
#include "CreditCardProcessingDlg.h"
#include "KeyboardCardSwipeDlg.h" 
#include "PayCatDlg.h"
#include "PaymentechUtils.h"
#include "GiftCertificateSearchUtils.h"
#include "GCEntryDlg.h"
#include "NxPracticeSharedLib\ICCPUtils.h"
#include "NxThread.h"

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get quickbooks out of stdafx
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "QBFC3.tlb" no_namespace

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (j.jones 2015-09-30 09:40) - PLID 67165 - declare a thread message to handle loading CC payment profiles from the API
UINT LoadCCPaymentProfilesThread_NotifyMessage = ::RegisterWindowMessage("Nx::LoadCCPaymentProfilesThread::Notify");

enum TipListColumns {
	tlcID = 0,
	tlcProvID = 1,
	tlcAmount = 2,
	tlcMethod = 3,
};

// (j.jones 2007-04-19 15:26) - PLID 25711 - created to easily get the priority column
// (j.jones 2008-07-10 11:57) - PLID 28756 - added columns for the payment/adjustment defaults
enum RespListColumns {
	rlcID = 0,
	rlcName,
	rlcPriority,
	rlcDefaultPayDesc,
	rlcDefaultPayCategoryID,
	rlcDefaultAdjDesc,
	rlcDefaultAdjCategoryID
};

//(e.lally 2007-07-09) PLID 25993 - Created to easily get the credit card columns
enum CreditCardColumns{
	cccCardID = 0,
	cccCardName,
};

// (j.jones 2010-09-23 11:40) - PLID 27917 - added enums for group & reason codes
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

// (b.spivey, February 26, 2013) - PLID 51186 - enum for multiple columns 
enum PaymentCategoryColumn {
	pccID = 0,
	pccCategoryName,
	pccCategoryDescription, 
};

// (j.jones 2015-09-30 09:15) - PLID 67158 - enum for CC merchant acct
enum CCMerchantAcctComboColumn {
	maccID = 0,
	maccDescription,
	maccMerchantID,
};

// (j.jones 2015-09-30 09:40) - PLID 67165 - enum for the CC payment profiles
enum CCPaymentProfileColumns {
	ppcCardConnectProfileAccountID = 0,	//the CardConnect account ID for this profile
	ppcCreditCardToken, // (z.manning 2015-08-26 08:24) - PLID 67231
	ppcComboText,	//this is a combined field that shows Card - last 4, exp. date in the collapsed combo only
	ppcCardType,	//Visa, Mastercard, etc.
	ppcLastFour,	//Last 4 on card
	ppcExpDate,		//the hidden datetime of the expiration date
	ppcExpDateText,	//the displayed MM/YY of the expiration date
	ppcDefault,		//checkbox to indicate which profile, if any, is the default
};

/////////////////////////////////////////////////////////////////////////////
// CPaymentDlg dialog

//(e.lally 2007-12-11) PLID 28325 - Updated "Privatize" cc number function to "Mask"

CPaymentDlg::CPaymentDlg(CWnd* pParent)
	: CNxDialog(CPaymentDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPaymentDlg)
	m_strCreditCardNumber = _T("");
	m_strAmount = _T("");
	m_strBankNumOrCCAuthNum = _T("");
	m_strExpDate = _T("");
	m_iDefaultPaymentType = 0;
	m_bForceDefaultPaymentType = FALSE;
	m_bIsReturnedProductAdj = FALSE;
	m_bIsReturnedProductRefund = FALSE;
	m_iDefaultInsuranceCo = -1;
	m_boIsNewPayment = TRUE;
	m_varPaymentID = g_cvarNull;
	m_bIsPrePayment = FALSE;
	m_ApplyOnOK = FALSE;
	m_PromptToShift = TRUE;
	m_strNameOnCard = _T("");
	m_PatientID = -1;
	m_DefLocationID = -1;
	m_bAmountChanged = FALSE;
	m_bIsCoPay = FALSE;
	m_cyTotalCoInsuranceAmt = COleCurrency(0,0);
	// (j.jones 2011-11-11 09:44) - PLID 46348 - this is a pointer to an array of pointers,
	// handled by our caller, NOT deleted within this dialog
	m_parypInsuranceCoPayApplyList = NULL;
	m_cyCopayAmount = COleCurrency(0,0);
	m_cyFinalAmount = COleCurrency(0,0);
	m_cyMaxAmount = COleCurrency(9999999,9999999);
	m_bTipExtended = false;
	m_bHasNexSpa = false;
	m_bViewTips = false;
	m_nTipInactiveProv = -1;
	m_nGiftID = -1;
	m_QuoteID = -1;
	m_nProcInfoID = -1;	// (j.jones 2009-08-25 11:57) - PLID 31549
	m_bAdjustmentFollowMaxAmount = FALSE;
	m_bLastCheckInfoLoaded = FALSE;
	m_strLastBankNo = "";
	m_strLastCheckAcctNo = "";
	m_strLastRoutingNo = "";
	m_bLastCreditCardInfoLoaded = FALSE;
	m_nLastCCTypeID = -1;
	m_cyLastAmount = COleCurrency(0,0);
	m_nBatchPaymentID = -1;
	m_eBatchPayType = eNoPayment;
	m_bSwiped = FALSE;
	m_bPinPadSwipeable = FALSE;
	m_bSetCreditCardInfo = FALSE;
	m_bProcessCreditCards_NonICCP = FALSE;
	m_bHasAnyTransaction = false;
	m_bHasPreICCPTransaction = false;
	m_bHasICCPTransaction = false;
	m_pCurPayMethod = NULL;
	m_odrPreviousReasons = odrNone;
	m_bAllowSwipe = TRUE;
	m_bInitializing = FALSE; // (c.haag 2010-10-07 13:48) - PLID 35723
	m_bCanEdit = TRUE; //TES 3/25/2015 - PLID 65436 - We assume they can edit until we learn otherwise
	//}}AFX_DATA_INIT	

	m_strSwipedCardName = "";
	m_nPendingGroupCodeID = -1;
	m_nPendingReasonCodeID = -1;

	// (b.spivey, December 13, 2011) - PLID 40567 - Assume this is off until we get the property cache. 
	m_bKeyboardCardSwiper = false;

	// (b.spivey, March 06, 2013) - PLID 51186 - string for code-set description from category name. 
	m_strLastSetDescription = "";

	//TES 4/20/2015 - PLID 65655 - When applying to an existing payment, track its payment method
	m_nPayToApplyToMethod = -1;

	m_nSelectedGiftID = -1;
	// (r.gonet 2015-05-05 09:53) - PLID 65326 - Initialize the original pay method.
	m_eOriginalPayMethod = EPayMethod::Invalid;

	m_bNeedToMoveICCPCCCombo = true;
	m_bNeedToMoveICCPMerchantAccountCombo_Processed = true;

	m_bNewICCPPaymentSavedButNotYetProcessed = FALSE;
}


void CPaymentDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPaymentDlg)
	DDX_Control(pDX, IDC_GC_INCLUDE_OTHERS, m_btnIncludeOtherGC);
	DDX_Control(pDX, IDC_TIPS_IN_DRAWER, m_btnTipsInDrawer);
	DDX_Control(pDX, IDC_CHECK_PREPAYMENT, m_btnPrePayment);
	DDX_Control(pDX, IDC_LABEL_INFO1, m_nxlInfoLabel1);
	DDX_Control(pDX, IDC_LABEL_INFO2, m_nxlInfoLabel2);
	DDX_Control(pDX, IDC_LABEL_INFO3, m_nxlInfoLabel3);
	DDX_Control(pDX, IDC_RADIO_PAYMENT, m_radioPayment);
	DDX_Control(pDX, IDC_RADIO_ADJUSTMENT, m_radioAdjustment);
	DDX_Control(pDX, IDC_RADIO_REFUND, m_radioRefund);
	DDX_Control(pDX, IDC_RADIO_CASH, m_radioCash);
	DDX_Control(pDX, IDC_RADIO_CHECK, m_radioCheck);
	DDX_Control(pDX, IDC_RADIO_CHARGE, m_radioCharge);
	DDX_Control(pDX, IDC_RADIO_GIFT_CERT, m_radioGift);
	DDX_Control(pDX, IDC_REFUND_TO_EXISTING_GC_RADIO, m_radioRefundToExistingGiftCertificate);
	DDX_Control(pDX, IDC_REFUND_TO_NEW_GC_RADIO, m_radioRefundToNewGiftCertificate);
	DDX_Text(pDX, IDC_EDIT_TOTAL, m_strAmount);
	DDX_Text(pDX, IDC_EDIT_BANKNUM_OR_CCAUTHNUM, m_strBankNumOrCCAuthNum);
	DDX_Text(pDX, IDC_EDIT_CHECKNUM_OR_CCEXPDATE, m_strExpDate);
	DDX_Text(pDX, IDC_EDIT_ACCTNUM_OR_CCNAMEONCARD, m_strNameOnCard);
	DDX_Control(pDX, IDC_PAY_DATE, m_date);
	DDX_Control(pDX, IDC_EDIT_TOTAL, m_nxeditEditTotal);
	DDX_Control(pDX, IDC_EDIT_DESCRIPTION, m_nxeditEditDescription);
	DDX_Control(pDX, IDC_CHANGE_GIVEN, m_nxeditChangeGiven);
	DDX_Control(pDX, IDC_EDIT_CHECKNUM_OR_CCEXPDATE, m_nxeditCheckNumOrCCExpDate);
	DDX_Control(pDX, IDC_EDIT_BANKNAME_OR_CCCARDNUM, m_nxeditBankNameOrCCCardNum);
	DDX_Control(pDX, IDC_EDIT_BANKNUM_OR_CCAUTHNUM, m_nxeditBankNumOrCCAuthNum);
	DDX_Control(pDX, IDC_EDIT_ACCTNUM_OR_CCNAMEONCARD, m_nxeditAcctNumOrCCNameOnCard);
	DDX_Control(pDX, IDC_CASH_RECEIVED, m_nxeditCashReceived);
	DDX_Control(pDX, IDC_TOTAL_TIP_AMT, m_nxsTotalTipAmt); // (a.walling 2010-01-12 15:13) - PLID 28961 - Now a static label
	DDX_Control(pDX, IDC_REPORT_LABEL, m_nxstaticReportLabel);
	DDX_Control(pDX, IDC_LABEL1, m_nxstaticLabel1);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL, m_nxstaticCurrencySymbol);
	DDX_Control(pDX, IDC_LABEL2, m_nxstaticLabel2);
	DDX_Control(pDX, IDC_PAYMENT_CATEGORY_LABEL, m_nxstaticPayCatLabel);
	DDX_Control(pDX, IDC_LABEL3, m_nxstaticLabel3);
	DDX_Control(pDX, IDC_LABEL6, m_nxstaticLabel6);
	DDX_Control(pDX, IDC_LOCATION_LABEL, m_nxstaticLocationLabel);
	DDX_Control(pDX, IDC_LABEL5, m_nxstaticLabel5);
	DDX_Control(pDX, IDC_CASH_DRAWER_LABEL, m_nxstaticCashDrawerLabel);
	DDX_Control(pDX, IDC_LABEL11, m_nxstaticLabel11);
	DDX_Control(pDX, IDC_ADJ_GROUPCODE_LABEL, m_nxstaticAdjGroupcodeLabel);
	DDX_Control(pDX, IDC_ADJ_REASON_LABEL, m_nxstaticAdjReasonLabel);
	DDX_Control(pDX, IDC_CASH_RECEIVED_LABEL, m_nxstaticCashReceivedLabel);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_RECEIVED, m_nxstaticCurrencySymbolReceived);
	DDX_Control(pDX, IDC_CHANGE_GIVEN_LABEL, m_nxstaticChangeGivenLabel);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_GIVEN, m_nxstaticCurrencySymbolGiven);
	DDX_Control(pDX, IDC_LABEL_CHECKNUM_OR_CCEXPDATE, m_nxstaticLabelCheckNumOrCCExpDate);	
	DDX_Control(pDX, IDC_LABEL_BANKNAME_OR_CCCARDNUM, m_nxstaticLabelBankNameOrCCCardNum);
	DDX_Control(pDX, IDC_LABEL_ACCTNUM_OR_CCNAMEONCARD, m_nxstaticLabelAcctNumOrCCNameOnCard);
	DDX_Control(pDX, IDC_LABEL_CCCARDTYPE, m_nxstaticLabelCCCardType);
	DDX_Control(pDX, IDC_LABEL_BANKNUM_OR_CCAUTHNUM, m_nxstaticLabelBankNumOrCCAuthNum);
	DDX_Control(pDX, IDC_BOTTOM_RIGHT_NORMAL, m_nxstaticBottomRightNormal);
	DDX_Control(pDX, IDC_BOTTOM_RIGHT_EXTENDED, m_nxstaticBottomRightExtended);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_PAY_CAT, m_btnEditPayCat);
	DDX_Control(pDX, IDC_EDIT_PAY_DESC, m_btnEditPayDesc);
	DDX_Control(pDX, IDC_EDIT_CASH_DRAWERS, m_btnEditCashDrawers);
	DDX_Control(pDX, IDC_EDIT_CARD_NAME, m_btnEditCardName);
	DDX_Control(pDX, IDC_ADD_TIP_BTN, m_btnAddTip);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_SAVE_AND_ADD_NEW, m_btnSaveAndAddNew);
	DDX_Control(pDX, IDC_BTN_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_BTN_PREVIEW, m_btnPreview);
	DDX_Control(pDX, IDC_CALC_PERCENT, m_btnCalcPercent);
	DDX_Control(pDX, IDC_PROCESS_CC, m_radioProcessCC);
	DDX_Control(pDX, IDC_KEYBOARD_CARD_SWIPE, m_btnKeyboardCardSwipe); 
	DDX_Control(pDX, IDC_BTN_FILTER_REASON_CODES, m_btnFilterReasonCodes);
	DDX_Control(pDX, IDC_LABEL_GIFT_CERT_NUMBER, m_nxstaticGiftCNumber);
	DDX_Control(pDX, IDC_LABEL_GC_VALUE, m_nxstaticGCValue);
	DDX_Control(pDX, IDC_LABEL_GC_BALANCE, m_nxstaticGCBalance);
	DDX_Control(pDX, IDC_LABEL_GC_PURCH_DATE, m_nxstaticGCPurchDate);
	DDX_Control(pDX, IDC_LABEL_GC_EXP_DATE, m_nxstaticGCExpDate);
	DDX_Control(pDX, IDC_EDIT_GIFT_CERT_NUMBER, m_nxeditGiftCNumber);
	DDX_Control(pDX, IDC_EDIT_GC_VALUE, m_nxeditGCValue);
	DDX_Control(pDX, IDC_EDIT_GC_BALANCE, m_nxeditGCBalance);
	DDX_Control(pDX, IDC_EDIT_GC_PURCH_DATE, m_nxeditGCPurchDate);
	DDX_Control(pDX, IDC_EDIT_GC_EXP_DATE, m_nxeditGCExpDate);
	DDX_Control(pDX, IDC_BTN_CLEAR_GC, m_btnClearGC);
	DDX_Check(pDX, IDC_REFUND_TO_EXISTING_GC_RADIO, m_bRefundToExistingGC);
	DDX_Control(pDX, IDC_GC_SEARCH_LABEL, m_nxstaticGCSearchLabel);
	DDX_Control(pDX, IDC_LABEL_CC_TRANSACTION_TYPE, m_nxstaticLabelCCTransactionType);
	DDX_Control(pDX, IDC_RADIO_SWIPE_CARD, m_radioCCSwipe);
	DDX_Control(pDX, IDC_RADIO_CARD_NOT_PRESENT, m_radioCCCardNotPresent);
	DDX_Control(pDX, IDC_RADIO_DO_NOT_PROCESS, m_radioCCDoNotProcess);
	DDX_Control(pDX, IDC_LABEL_CC_MERCHANT_ACCT, m_nxstaticLabelCCMerchantAcct);
	DDX_Control(pDX, IDC_LABEL_CC_ZIPCODE, m_nxstaticLabelCCZipcode);
	DDX_Control(pDX, IDC_EDIT_CC_ZIPCODE, m_nxeditCCZipcode);
	DDX_Control(pDX, IDC_CHECK_ADD_CC_TO_PAYMENT_PROFILE, m_checkAddCCToProfile);
	DDX_Control(pDX, IDC_RADIO_CC_REFUND_TO_ORIGINAL_CARD, m_radioCCRefundToOriginalCard);
	DDX_Control(pDX, IDC_LABEL_CC_CARD_ON_FILE, m_nxstaticLabelCCCardOnFile);
	DDX_Control(pDX, IDC_LABEL_CC_LAST4, m_nxstaticLabelCCLast4);
	DDX_Control(pDX, IDC_EDIT_CC_LAST4, m_nxeditCCLast4);	
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPaymentDlg, CNxDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO_CASH, OnClickRadioCash)
	ON_BN_CLICKED(IDC_RADIO_CHARGE, OnClickRadioCharge)
	ON_BN_CLICKED(IDC_RADIO_CHECK, OnClickRadioCheck)
	ON_BN_CLICKED(IDC_RADIO_PAYMENT, OnClickRadioPayment)
	ON_BN_CLICKED(IDC_RADIO_ADJUSTMENT, OnClickRadioAdjustment)
	ON_BN_CLICKED(IDC_RADIO_REFUND, OnClickRadioRefund)
	ON_EN_UPDATE(IDC_EDIT_CHECKNUM_OR_CCEXPDATE, OnUpdateEditCheckNumOrCCExpDate)
	ON_BN_CLICKED(IDC_BTN_PRINT, OnClickBtnPrint)
	ON_BN_CLICKED(IDC_BTN_PREVIEW, OnClickBtnPreview)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_EDIT_PAY_DESC, OnEditPayDesc)
	ON_BN_CLICKED(IDC_EDIT_PAY_CAT, OnEditPayCat)
	ON_BN_CLICKED(IDC_EDIT_CARD_NAME, OnEditCardName)
	ON_WM_CREATE()
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_EN_KILLFOCUS(IDC_EDIT_CHECKNUM_OR_CCEXPDATE, OnKillfocusEditCheckNumOrCCExpDate)
	ON_BN_CLICKED(IDC_CALC_PERCENT, OnCalcPercent)
	ON_BN_CLICKED(IDC_QUICKBOOKS_BTN, OnExportToQuickBooks)
	ON_EN_CHANGE(IDC_EDIT_TOTAL, OnChangeEditTotal)
	ON_EN_SETFOCUS(IDC_EDIT_ACCTNUM_OR_CCNAMEONCARD, OnSetfocusEditAcctNumOrCCNameOnCard)
	ON_EN_KILLFOCUS(IDC_CASH_RECEIVED, OnKillfocusCashReceived)
	ON_EN_KILLFOCUS(IDC_EDIT_TOTAL, OnKillfocusEditTotal)
	ON_BN_CLICKED(IDC_ADD_TIP_BTN, OnAddTipBtn)
	ON_BN_CLICKED(IDC_SHOW_TIPS_BTN, OnShowTipsBtn)
	ON_BN_CLICKED(IDC_BTN_SAVE_AND_ADD_NEW, OnBtnSaveAndAddNew)
	ON_BN_CLICKED(IDC_RADIO_GIFT_CERT, OnClickRadioGiftCert)
	ON_BN_CLICKED(IDC_GC_INCLUDE_OTHERS, OnGcIncludeOthers)
	ON_BN_CLICKED(IDC_EDIT_CASH_DRAWERS, OnEditCashDrawers)
	ON_BN_CLICKED(IDC_TIPS_IN_DRAWER, OnTipsInDrawer)
	ON_BN_CLICKED(IDC_CHECK_PREPAYMENT, OnCheckPrepayment)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_MESSAGE(WM_MSR_DATA_EVENT, OnMSRDataEvent)
	ON_MESSAGE(WM_PINPAD_MESSAGE_READ_TRACK_DONE, OnPinPadTrackRead)
	ON_MESSAGE(WM_PINPAD_MESSAGE_INTERAC_DEBIT_DONE, OnPinPadInteracDebitDone)
	ON_EN_KILLFOCUS(IDC_EDIT_BANKNAME_OR_CCCARDNUM, OnKillfocusEditBankNameOrCCCardNum)
	ON_BN_CLICKED(IDC_KEYBOARD_CARD_SWIPE, OnBnClickedKeyboardCardSwipe)
	ON_BN_CLICKED(IDC_BTN_FILTER_REASON_CODES, OnBtnFilterReasonCodes)
	ON_BN_CLICKED(IDC_BTN_CLEAR_GC, OnBtnClearGC)
	ON_BN_CLICKED(IDC_REFUND_TO_EXISTING_GC_RADIO, OnBnClickedRefundToExistingGcRadio)
	ON_BN_CLICKED(IDC_REFUND_TO_NEW_GC_RADIO, OnBnClickedRefundToNewGcRadio)
	ON_BN_CLICKED(IDC_RADIO_SWIPE_CARD, OnRadioSwipeCard)
	ON_BN_CLICKED(IDC_RADIO_CARD_NOT_PRESENT, OnRadioCardNotPresent)
	ON_BN_CLICKED(IDC_RADIO_DO_NOT_PROCESS, OnRadioDoNotProcess)
	ON_BN_CLICKED(IDC_RADIO_CC_REFUND_TO_ORIGINAL_CARD, OnRadioCcRefundToOriginalCard)
	ON_REGISTERED_MESSAGE(LoadCCPaymentProfilesThread_NotifyMessage, OnCCPaymentProfilesThreadComplete)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPaymentDlg, CNxDialog)
	ON_EVENT(CPaymentDlg, IDC_RADIO_PAYMENT, 2 /* Change */, OnChangeRadioPayment, VTS_NONE)
	ON_EVENT(CPaymentDlg, IDC_COMBO_INSURANCE, 18 /* RequeryFinished */, OnRequeryFinishedComboInsurance, VTS_I2)	
	ON_EVENT(CPaymentDlg, IDC_COMBO_LOCATION, 20 /* TrySetSelFinished */, OnTrySetSelFinishedComboLocation, VTS_I4 VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_COMBO_CARD_NAME, 16 /* SelChosen */, OnSelChosenComboCardName, VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_COMBO_DESCRIPTION, 16 /* SelChosen */, OnSelChosenComboDescription, VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_COMBO_LOCATION, 16 /* SelChosen */, OnSelChosenComboLocation, VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_COMBO_CATEGORY, 16 /* SelChosen */, OnSelChosenComboCategory, VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_COMBO_INSURANCE, 16 /* SelChosen */, OnSelChosenComboInsurance, VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_TIP_LIST, 6 /* RButtonDown */, OnRButtonDownTipList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_TIP_LIST, 10 /* EditingFinished */, OnEditingFinishedTipList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPaymentDlg, IDC_TIP_LIST, 18 /* RequeryFinished */, OnRequeryFinishedTipList, VTS_I2)
	ON_EVENT(CPaymentDlg, IDC_TIP_LIST, 9 /* EditingFinishing */, OnEditingFinishingTipList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPaymentDlg, IDC_ADJ_GROUPCODE, 1 /* SelChanging */, OnSelChangingGroupCode, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPaymentDlg, IDC_ADJ_REASON, 1 /* SelChanging */, OnSelChangingReason, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPaymentDlg, IDC_COMBO_CARD_NAME, 20 /* TrySetSelFinished */, OnTrySetSelFinishedComboCardName, VTS_I4 VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_COMBO_LOCATION, 1 /* SelChanging */, OnSelChangingComboLocation, VTS_PI4)
	ON_EVENT(CPaymentDlg, IDC_ADJ_GROUPCODE, 20, OnTrySetSelFinishedAdjGroupcode, VTS_I4 VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_ADJ_REASON, 20, OnTrySetSelFinishedAdjReason, VTS_I4 VTS_I4)
	ON_EVENT(CPaymentDlg, IDC_SEARCH_GC_LIST, 16, OnSelChosenSearchGcList, VTS_DISPATCH)
	ON_EVENT(CPaymentDlg, IDC_CC_CARD_ON_FILE_COMBO, 16, OnSelChosenCCCardOnFileCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPaymentDlg message handlers

BOOL CPaymentDlg::OnInitDialog() 
{
	try {
		// (a.walling 2007-08-03 17:08) - PLID 26922 - Initialize the m_pCurPayMethod
		m_pCurPayMethod = &m_radioCheck;

		CWaitCursor pWait;

		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-23 09:58) - PLID 29751 - NxIconify the buttons
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddTip.AutoSet(NXB_NEW);
		m_btnOK.AutoSet(NXB_OK);
		m_btnSaveAndAddNew.AutoSet(NXB_OK);
		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnPreview.AutoSet(NXB_PRINT_PREV);
		m_btnCalcPercent.AutoSet(NXB_MODIFY);
		// (j.jones 2012-07-26 17:48) - PLID 26877 - added ability to filter adjustment reasons
		m_btnFilterReasonCodes.SetIcon(IDI_FILTER);
		// (j.jones 2015-03-26 09:37) - PLID 65285 - added ability to clear the gift certificate
		m_btnClearGC.AutoSet(NXB_CANCEL);

		// (m.hancock 2006-10-26 13:48) - Moved this from the constructor because it was causing errors with the ClassWizard.
		GetMainFrame()->DisableHotKeys();

		// (j.jones 2006-04-26 17:47) - PLID 20313 - Load all common payment dialog properties into the
		// NxPropManager cache
		g_propManager.CachePropertiesInBulk("PaymentDlg-IntParam", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DisableQuickBooks' OR "
			"Name = 'QuickBooksExportStyle' OR "
			"Name = 'DefaultPayCat' OR "
			"Name = 'ShowPaymentTips' OR "
			"Name = 'TipsInDrawer' OR "
			"Name = 'ShowPaymentSaveAndAdd' OR "
			"Name = 'GetChargeInfo' OR "
			"Name = 'PayLocationPref' OR "
			"Name = 'DefaultPaymentDate' OR "
			"Name = 'WarnCashDrawerEmpty' OR "
			"Name = 'BankingIncludeTips' OR "
			"Name = 'ReceiptShowChargeInfo' OR "
			"Name = 'ReceiptShowTax' OR "
			"Name = 'NexSpa_AllowRefundCash' OR "
			"Name = 'NexSpa_AllowRefundCheck' OR "
			"Name = 'NexSpa_AllowRefundCharge' OR "
			"Name = 'NexSpa_AllowRefundGiftCertificate' OR " // (r.gonet 2015-06-10 17:26) - PLID 65748
			"Name = 'PaymentLocationApplyWarning' OR "
			"Name = 'DisablePaymentDate' OR "
			// (j.jones 2007-04-19 10:44) - PLID 25711 - added receipt preferences to the cache
			"Name = 'ReceiptShowChargeInfo' OR "
			"Name = 'ReceiptShowTax' OR "
			//(e.lally 2007-12-11) PLID 28325 - Added hidden preference for CC number masking
			"Name = 'MaskCreditCardNumbers' OR "
			"Name = 'IncludeForInPayDescription' "	// (j.jones 2008-05-27 09:31) - PLID 24661
			"OR Name = 'NewPaymentDefaultMethod' "	// (d.thompson 2009-08-11) - PLID 24079
			"OR Name = 'DefaultCoPaymentDate' "	// (j.jones 2010-05-25 15:08) - PLID 38661
			"OR Name = 'ShiftCoPayAmount' "		//(e.lally 2010-09-03) PLID 39971
			// (j.jones 2011-07-08 16:58) - PLID 44497 - added DefaultPaymentsNoProvider and RequireProviderOnPayments
			"OR Name = 'DefaultPaymentsNoProvider' "
			"OR Name = 'RequireProviderOnPayments' "
			// (a.wilson 2012-06-14 11:26) - PLID 47966 - caching ShowSalesReceiptDialog
			"OR Name = 'ShowSalesReceiptDialog' "
			"OR Name = 'dontshow PaymentFilterReasonCodes' "	// (j.jones 2012-07-27 10:28) - PLID 26877
			// (j.jones 2013-07-19 15:38) - PLID 57653 - used in CheckUnbatchCrossoverClaim
			"OR Name = 'ERemit_UnbatchMA18orNA89_MarkForwardToSecondary' "
			//TES 3/23/2015 - PLID 65172
			"OR Name = 'AllowTipRefunds' "
			// (j.jones 2015-09-30 09:14) - PLID 67159 - added AutoAddPaymentProfiles
			"OR Name = 'AutoAddPaymentProfiles' "
			"OR Name = 'ICCPEnabled' "
			// (c.haag 2015-08-26) - PLID 67201 - Added LastICCPPaymentTransactionType, LastICCPPaymentMerchantAccountID
			"OR Name = 'LastICCPPaymentTransactionType' "
			"OR Name = 'LastICCPPaymentMerchantAccountID' "
			")",
			_Q(GetCurrentUserName()));

		// (a.walling 2007-10-04 14:46) - PLID 26058 - Cache these workstation properties
		// (j.armen 2011-10-25 15:46) - PLID 46139 - GetPracPath references ConfigRT
		// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
		// than the license object.
		g_propManager.CachePropertiesInBulk("PaymentDlg-IntParam-Workstation", propNumber,
			"(Username = '%s') AND ("
			"Name = 'POSCashDrawer_OpenDrawerForChecks' OR " // (a.walling 2007-08-07 12:04) - PLID 26068 - Pref to open cash drawer for checks
			"Name = 'POSCashDrawer_OpenDrawerForRefunds' OR " // (a.walling 2007-09-20 12:32) - PLID 9801 - Pref to open cash drawer for refunds
			"Name = 'POSCashDrawer_OpenDrawerForCharges' OR " // (a.walling 2007-09-21 15:22) - PLID 27468 - Pref to open cash drawer for charges
			"Name = 'POSCashDrawer_OpenDrawerForTips' OR "// (a.walling 2007-09-20 15:32) - PLID 27468 - Pref to open cash drawer for tips
			"Name = 'MSR_UseDevice' " // (b.spivey, December 13, 2011) - PLID 40567 - Setting to use the keyboard MSR.
			")",
			_Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

		// (j.jones 2007-04-19 10:42) - PLID 25711 - bulk cache text properties as well
		g_propManager.CachePropertiesInBulk("PaymentDlg-TextParam", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PayReceiptLastReportRan' OR "
			"Name = 'PaymentLastCashDrawer' OR "
			"Name = 'DefaultPayDesc' OR "
			"Name = 'ReceiptCustomInfo' "			
			")",
			_Q(GetCurrentUserName()));

		m_bg.CreateSolidBrush(PaletteColor(0x9CC294));

		// (j.jones 2007-03-28 09:16) - PLID 24080 - set up the NxLabels
		m_nxlInfoLabel1.SetColor(0x9CC294);
		m_nxlInfoLabel2.SetColor(0x9CC294);
		
		CRect rc;
		GetDlgItem(IDC_LABEL_INFO1)->GetWindowRect(m_rcInfoLabel1);
		ScreenToClient(&m_rcInfoLabel1);

		// (j.jones 2006-12-07 12:40) - PLID 19467 - added option to disable payment date changing
		long nDisablePaymentDate = GetRemotePropertyInt("DisablePaymentDate", 0, 0, "<None>", true);
		if(nDisablePaymentDate == 1) {
			//simply disable the ability to change the payment date,
			//the code will fill it any number of ways, however
			GetDlgItem(IDC_PAY_DATE)->EnableWindow(FALSE);
		}

		// (b.spivey, October 05, 2011) - PLID 40567 - Cache this check. 
		// (b.spivey, November 18, 2011) - PLID 40567 - Corrected bool type. 
		if(GetPropertyInt("MSR_UseDevice", 0, 0, true) == emmKeyboard){
			m_bKeyboardCardSwiper = true;
		}
		else{
			m_bKeyboardCardSwiper = false; 
		}


		// (j.jones 2015-09-30 09:19) - PLID 67161 - default the CC patient profile checkbox based on
		// the preference, unless the user does not have permission to create payment profiles
		if (GetCurrentUserPermissions(bioPaymentProfile) & sptWrite) {
			//has permission, so respect the preference
			m_checkAddCCToProfile.SetCheck(GetRemotePropertyInt("AutoAddPaymentProfiles", 1, 0, "<None>", true) == 1);
		}
		else {
			//no permission, so uncheck and disable this checkbox
			m_checkAddCCToProfile.SetCheck(FALSE);
			m_checkAddCCToProfile.EnableWindow(FALSE);
		}

		if(m_PatientID == -1) {

			//before we try to get the active patient ID, see if we are applying to a bill or charge,
			//and extract the patient ID from there

			if(m_varBillID.vt == VT_I4) {
				_RecordsetPtr rs = CreateRecordset("SELECT PatientID FROM BillsT WHERE ID = %li",VarLong(m_varBillID));
				if(!rs->eof)
					m_PatientID = AdoFldLong(rs, "PatientID",-1);
				rs->Close();
			}
			else if(m_varChargeID.vt == VT_I4) {
				_RecordsetPtr rs = CreateRecordset("SELECT PatientID FROM LineItemT WHERE ID = %li",VarLong(m_varChargeID));
				if(!rs->eof)
					m_PatientID = AdoFldLong(rs, "PatientID",-1);
				rs->Close();
			}
			
			//NOW get the active ID, if we still have -1
			if(m_PatientID == -1) {
				m_PatientID = GetActivePatientID();
			}
		}

		m_strPatientName = GetExistingPatientName(m_PatientID);

		//Depending on our locale, we may want to put a space between the currency symbol and the number.
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, strICurr.GetBuffer(2), 2, TRUE);
		strICurr.ReleaseBuffer();
		strICurr.TrimRight();
		if(strICurr == "2") {
			SetDlgItemText(IDC_CURRENCY_SYMBOL,GetCurrencySymbol() + " ");
			SetDlgItemText(IDC_CURRENCY_SYMBOL_RECEIVED,GetCurrencySymbol() + " ");
			SetDlgItemText(IDC_CURRENCY_SYMBOL_GIVEN,GetCurrencySymbol() + " ");
		}
		else {
			SetDlgItemText(IDC_CURRENCY_SYMBOL,GetCurrencySymbol());
			SetDlgItemText(IDC_CURRENCY_SYMBOL_RECEIVED,GetCurrencySymbol());
			SetDlgItemText(IDC_CURRENCY_SYMBOL_GIVEN,GetCurrencySymbol());
		}

		//see if they have NexSpa enabled, can't use tips if they don't!
		m_bHasNexSpa = IsSpa(TRUE);

		m_ProviderCombo = BindNxDataListCtrl(IDC_COMBO_PROVIDER,true);
		m_DescriptionCombo = BindNxDataListCtrl(IDC_COMBO_DESCRIPTION,true);
		m_InsuranceCombo = BindNxDataListCtrl(IDC_COMBO_INSURANCE,false);
		m_CardNameCombo = BindNxDataListCtrl(IDC_COMBO_CARD_NAME,true);
		m_CategoryCombo = BindNxDataListCtrl(IDC_COMBO_CATEGORY,true);
		m_LocationCombo = BindNxDataListCtrl(IDC_COMBO_LOCATION,true);
		m_pTipList = BindNxDataListCtrl(IDC_TIP_LIST,false);
		m_pDrawerList = BindNxDataListCtrl(IDC_CASH_DRAWER_LIST,true);
		
		// (j.jones 2015-03-23 13:13) - PLID 65281 - Converted the gift certificate dropdown
		// into a searchable list. This defaults to including GCs only for the current patient or unassigned GCs.
		m_GiftSearch = GiftCertificateSearchUtils::BindGiftCertificatePaymentSearchListCtrl(this, IDC_SEARCH_GC_LIST, GetRemoteData(), m_PatientID);

		// (j.jones 2015-09-30 09:15) - PLID 67158 - added CC merchant account
		m_CCMerchantAccountCombo = BindNxDataList2Ctrl(IDC_CC_MERCHANT_ACCT_COMBO, true);

		// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
		m_CCCardOnFileCombo = BindNxDataList2Ctrl(IDC_CC_CARD_ON_FILE_COMBO, false);
		
		// (j.jones 2015-09-30 09:40) - PLID 67165 - this loads from the API, asynchronously
		BeginLoadCCPaymentProfilesThread(false);

		// (a.walling 2006-11-15 09:42) - PLID 23550 - Bind and init datalists for group and reason codes
		// (j.jones 2010-09-23 11:38) - PLID 27917 - these are now queryable tables
		m_pGroupCodeList = BindNxDataList2Ctrl(IDC_ADJ_GROUPCODE, true);
		m_pReasonList = BindNxDataList2Ctrl(IDC_ADJ_REASON, true);

		// (j.jones 2010-09-23 11:39) - PLID 27917 - need to add "no code" options
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

		// (j.gruber 2007-03-13 10:53) - PLID 25019 - adding report drop down
		m_pReportList = BindNxDataList2Ctrl(IDC_COMBO_REPORT,false);

		pRow2 = m_pReportList->GetNewRow();

		pRow2->PutValue(0, (long)-1);
		pRow2->PutValue(1, (long)-1);
		pRow2->PutValue(2, _variant_t("<System Payment Receipt>"));
		pRow2->PutValue(3, _variant_t(""));
		pRow2->PutValue(4, _variant_t("-1:-1"));
		m_pReportList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pReportList->GetNewRow();
		pRow2->PutValue(0, (long)-2);
		pRow2->PutValue(1, (long)-2);
		pRow2->PutValue(2, _variant_t("<System Sales Receipt>"));
		pRow2->PutValue(3, _variant_t(""));
		pRow2->PutValue(4, _variant_t("-2:-2"));
		m_pReportList->AddRowAtEnd(pRow2, NULL);

		// (j.gruber 2007-03-29 14:39) - PLID 9802 - use a recipt printer format
		//we aren't using this anymore, they can configure the items themselves
		/*pRow2 = m_pReportList->GetNewRow();
		pRow2->PutValue(0, (long)-3);
		pRow2->PutValue(1, (long)-3);
		pRow2->PutValue(2, _variant_t("<Sales Receipt (for Receipt Printer)>"));
		pRow2->PutValue(3, _variant_t(""));
		pRow2->PutValue(4, _variant_t("-2:-2"));
//		m_pReportList->AddRowAtEnd(pRow2, NULL);*/

		_RecordsetPtr rsReports = CreateRecordset("SELECT ID, Number, Title, Filename FROM CustomReportsT WHERE ID IN (235, 585) ");
		while (!rsReports->eof) {

			long nRepID = AdoFldLong(rsReports, "ID", -4);
			long nRepNumber = AdoFldLong(rsReports, "Number", -4);

			pRow2 = m_pReportList->GetNewRow();
			pRow2->PutValue(0, nRepID );
			pRow2->PutValue(1, nRepNumber);
			pRow2->PutValue(2, _variant_t(AdoFldString(rsReports,"Title", "")));
			pRow2->PutValue(3, _variant_t(AdoFldString(rsReports,"FileName", "")));

			CString strIDNumCombo;
			strIDNumCombo.Format("%li:%li", nRepID, nRepNumber);
			pRow2->PutValue(4, _variant_t(strIDNumCombo));

			m_pReportList->AddRowAtEnd(pRow2, NULL);

			rsReports->MoveNext();
		}
		rsReports->Close();

		// (j.gruber 2007-05-08 09:48) - PLID 9802 - Add the Receipt Printer Formats
		//TES 12/6/2007 - PLID 28192 - Check that we have a POS Printer.
		if (GetMainFrame()->CheckPOSPrinter()) {
			rsReports = CreateRecordset("SELECT ID, Name FROM POSReceiptsT ");
			while (! rsReports->eof) {
				long nRepID = -3;
				long nRepNumber = AdoFldLong(rsReports, "ID");

				pRow2 = m_pReportList->GetNewRow();
				pRow2->PutValue(0, nRepID);
				pRow2->PutValue(1, nRepNumber);
				pRow2->PutValue(2, _variant_t("<RP> -  " + AdoFldString(rsReports, "Name", "")));
				pRow2->PutValue(3, _variant_t(""));
				
				CString strIDNumCombo;
				strIDNumCombo.Format("%li:%li", nRepID, nRepNumber);
				pRow2->PutValue(4, _variant_t(strIDNumCombo));

				m_pReportList->AddRowAtEnd(pRow2, NULL);
				rsReports->MoveNext();
			}
			rsReports->Close();
		}

		
		CString strCombo = GetRemotePropertyText("PayReceiptLastReportRan", "-2:-2", 0, GetCurrentUserName(), TRUE);
		

		pRow2 = m_pReportList->FindByColumn(4, _variant_t(strCombo), NULL, FALSE);
		if (pRow2) {
			pRow2->PutCellForeColorSel(1, RGB(255,0,0));
			pRow2->PutCellForeColor(1, RGB(255,0,0));
			m_pReportList->CurSel = pRow2;
		}


		IRowSettingsPtr pRow;
		pRow = m_ProviderCombo->GetRow(-1);
		pRow->PutValue(0,long(-1));
		pRow->PutValue(1,_bstr_t("<No Provider Selected>"));
		m_ProviderCombo->InsertRow(pRow,0);

		// (b.spivey, February 26, 2013) - PLID 51186 - Default values for "no category" 
		pRow = m_CategoryCombo->GetRow(-1);
		pRow->PutValue(pccID,long(0));
		pRow->PutValue(pccCategoryName,_bstr_t("<No Category Selected>"));
		pRow->PutValue(pccCategoryDescription, _bstr_t("")); 
		m_CategoryCombo->AddRow(pRow);

		//add a no-row to drawer list
		pRow = m_pDrawerList->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<No Drawer Selected>"));
		m_pDrawerList->AddRow(pRow);
		m_pDrawerList->SetSelByColumn(0, (long)-1);

		//try to load the last drawer they used
		if(m_bHasNexSpa) {
			CString strDrawer = GetRemotePropertyText("PaymentLastCashDrawer", "", 0, GetCurrentUserName(), false);
			//if this is null or anything non-numeric, it will leave our selection at "No selected"
			long nDrawer = atoi(strDrawer);
			m_pDrawerList->TrySetSelByColumn(0, (long)nDrawer);
		}

		// Only show the send-to-quickbooks button if:
		// - the user is licensed for quickbooks
		// - quickbooks is not disabled
		// - they are not using the "Make Deposits" function
		if (g_pLicense->GetHasQuickBooks() && GetRemotePropertyInt("DisableQuickBooks",0,0,"<None>",TRUE) == 0
			&& GetRemotePropertyInt("QuickBooksExportStyle",0,0,"<None>",TRUE) == 1) {
			GetDlgItem(IDC_QUICKBOOKS_BTN)->ShowWindow(SW_SHOW);
		}

		// (j.jones 2008-07-10 12:34) - PLID 28756 - these defaults may be overridden later by
		// insurance company defaults, but for now, use the global defaults
		//set the default payment category
		OnSelChosenComboCategory(m_CategoryCombo->SetSelByColumn(0,(long)GetRemotePropertyInt("DefaultPayCat",-1,0,"<None>",TRUE)));
		//normally when we select a category, it fills the description, but I don't want that to happen for defaults, so set it to be blank
		GetDlgItem(IDC_EDIT_DESCRIPTION)->SetWindowText("");
		// (a.walling 2007-12-27 18:49) - PLID 28429 - Set the text limit
		((CNxEdit*)GetDlgItem(IDC_EDIT_DESCRIPTION))->SetLimitText(200);

		// (j.jones 2015-09-30 09:15) - PLID 67158 - limit the CC zipcode to 20 characters,
		// only because that's the limit in General 1
		m_nxeditCCZipcode.SetLimitText(20);

		// (j.jones 2015-09-30 09:31) - PLID 67167 - limit the CC Last 4 to 4 characters
		m_nxeditCCLast4.SetLimitText(4);

		//set the default payment description
		OnSelChosenComboDescription(m_DescriptionCombo->SetSelByColumn(0,_bstr_t(GetRemotePropertyText("DefaultPayDesc","",0,"<None>",TRUE))));

		//check the permission, if they don't have ability to add tips, disable the button
		if(!(GetCurrentUserPermissions(bioPaymentTips, false, 0) & sptRead)) {
			GetDlgItem(IDC_SHOW_TIPS_BTN)->EnableWindow(FALSE);
			m_bViewTips = false;
		}
		else
			m_bViewTips = true;

		//TES 3/25/2015 - PLID 65175 - Moved some tip-related code to OnShowWindow()
			
		//remember the setting for including tips in drawers
		int nRemember = GetRemotePropertyInt("TipsInDrawer", 1, 0, GetCurrentUserName());
		CheckDlgButton(IDC_TIPS_IN_DRAWER, nRemember);

		//requery the tip list
		long nPayID = -1;
		if(m_varPaymentID.vt == VT_I4) {
			nPayID = VarLong(m_varPaymentID);
		}

		// (d.lange 2016-05-12 16:02) - NX-100597 - Sets the tip datalist where clause
		UpdateTipListWhereClause(nPayID);
		m_pTipList->Requery();

		//set the checkbox for saving in a drawer appropriately.  This is a little odd, but
		//here's the deal:  The checkbox remembers it's state.  In most cases, you'll turn
		//it on and leave it on.  But if for whatever reason you edited a payment where the
		//tips were not entered, but you had previously been remembering, then the box
		//would be checked, and if you hit OK it would set a drawer on those tips, which
		//they were not in before, and the user didn't change anything.
		if(ReturnsRecords("SELECT ID FROM PaymentTipsT WHERE DrawerID IS NOT NULL AND PaymentID = %li", nPayID)) {
			//there is at least 1 tip deposited
			CheckDlgButton(IDC_TIPS_IN_DRAWER, TRUE);
		}
		else {
			m_pTipList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			//there are no tips of this payment ID.  If there are any tips at all (this could be a new payment), 
			//then uncheck it, otherwise leave as is (the remembered state)
			if(m_pTipList->GetRowCount() > 0)
				CheckDlgButton(IDC_TIPS_IN_DRAWER, FALSE);
		}

		// (j.jones 2007-03-05 08:31) - PLID 25018 - default "Save & Add New" to on
		if(GetRemotePropertyInt("ShowPaymentSaveAndAdd", 1, 0, GetCurrentUserName(), true)) {
			//correctly arrange the buttons so we can see the "save and add new" button
			
			//make the print and preview buttons half height and stacked
			SetDlgItemText(IDC_BTN_PRINT, "Print");
			SetDlgItemText(IDC_BTN_PREVIEW, "Preview");

			// (c.haag 2008-05-21 09:38) - PLID 29751 - Don't move the print and print preview
			// buttons on account of "Save & Add New" being visible
			/*
			CRect rcPrint, rcPreview;
			GetDlgItem(IDC_BTN_PREVIEW)->GetWindowRect(rcPreview);
			ScreenToClient(rcPreview);
			rcPreview.bottom = rcPreview.top + ((long)(rcPreview.Height()) / 2) - 1;
			GetDlgItem(IDC_BTN_PREVIEW)->MoveWindow(rcPreview);

			GetDlgItem(IDC_BTN_PRINT)->GetWindowRect(rcPrint);
			ScreenToClient(rcPrint);
			rcPrint.top = rcPrint.bottom - ((long)(rcPrint.Height()) / 2) + 1;
			rcPrint.left = rcPreview.left;
			rcPrint.right = rcPreview.right;
			GetDlgItem(IDC_BTN_PRINT)->MoveWindow(rcPrint);*/
		}
		else {
			// (c.haag 2008-05-21 09:39) - PLID 29751 - If there is no "Save & Add New",
			// then move the OK, Save & Process, and Cancel buttons up a notch
			// (d.thompson 2009-06-30) - PLID 34687 - Removed the process buttons.
			CRect rcTop, rcMiddle;
			GetDlgItem(IDC_BTN_SAVE_AND_ADD_NEW)->GetWindowRect(rcTop);
			GetDlgItem(IDOK)->GetWindowRect(rcMiddle);
			ScreenToClient(rcTop);
			ScreenToClient(rcMiddle);

			GetDlgItem(IDOK)->MoveWindow(rcTop);
			GetDlgItem(IDCANCEL)->MoveWindow(rcMiddle);

			//hide the "save and add" button
			GetDlgItem(IDC_BTN_SAVE_AND_ADD_NEW)->ShowWindow(SW_HIDE);
		}

		// (j.gruber 2007-07-03 12:21) - PLID 15416 - CC Processing
		// (a.walling 2007-08-03 09:17) - PLID 26899 - Check for license
		m_bSwiped = FALSE;

		// (a.walling 2007-08-03 15:53) - PLID 26922 - Disable if they do not have permission to process
		// (d.thompson 2010-09-02) - PLID 40371 - Any license type satisfies
		// (j.jones 2015-09-30 08:58) - PLID 67157 - this is now non-ICCP
		if (!g_pLicense || !g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent) || IsICCPEnabled())
		{			
			m_bProcessCreditCards_NonICCP = FALSE;
		}
		else
		{
			//they have the old Chase/Intuit processing, and not ICCP
			m_bProcessCreditCards_NonICCP = TRUE;

			//if we are using a pin pad, allow the customer to swipe whenever they want
			if (GetPropertyInt("CCProcessingUsingPinPad", 0, 0, TRUE)) {
				if (PinPadUtils::IsInitialized()) {
					PinPadUtils::GetCustomerSwipe();
					m_bPinPadSwipeable = TRUE;
				}
				else {
					//itialize first
					PinPadUtils::SendInitMessage();
					PinPadUtils::GetCustomerSwipe();
					m_bPinPadSwipeable = TRUE;
				}
			}

			// (d.thompson 2009-06-30) - PLID 34687 - If we have permission and license to
			//	credit card processing, let's default the option to on.
			//Note that the m_boIsNewPayment is not actually valid at this point, so we have to see what the
			//	ID is.  If it's an int, we're loading an existing payment, and should always leave this unchecked.
			if (m_varPaymentID.vt == VT_I4) {
				//Loading existing, leave unchecked
				CheckDlgButton(IDC_PROCESS_CC, FALSE);
			}
			else {
				CheckDlgButton(IDC_PROCESS_CC, TRUE);
			}
		}

		// (d.thompson 2009-06-30) - PLID 34687 - Hide the "process cc transactions" button
		GetDlgItem(IDC_PROCESS_CC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PROCESS_CC)->EnableWindow(FALSE);

	}NxCatchAll("Error in OnInitDialog");

	//register for barcode messages
	if(GetMainFrame()) {
		if(!GetMainFrame()->RegisterForBarcodeScan(this))
			MsgBox("Error registering for barcode scans.  You may not be able to scan.");
	}

	// (a.wetta 2007-07-05 10:16) - PLID 26547 - Determine if the MSR device exists and let the user know
	if (GetMainFrame()->DoesOPOSMSRDeviceExist()) {
		// A MSR device is on and working, let's let the user know they can use it to input credit card information
		CString strMsg = "Practice has detected a magnetic strip reader (card swiper) attached to this computer.  You can swipe a credit card at\n"
						"anytime and the information from that card will fill the credit card information fields.";
		DontShowMeAgain(this, strMsg, "MSR_CreditCard_Payments", "Magnetic Strip Reader Detected");
	}

	return TRUE;
}

//(e.lally 2007-10-30) PLID 27892 - Public functions added to replace the m_strNumber variable for clarity.
void CPaymentDlg::SetCreditCardNumber(CString strNewCardNumber)
{
	m_strCreditCardNumber = strNewCardNumber;
}

CString CPaymentDlg::GetCreditCardNumber()
{
	return m_strCreditCardNumber;
}

// (d.lange 2016-05-12 16:02) - NX-100597 - Sets the tip datalist where clause
void CPaymentDlg::UpdateTipListWhereClause(long nPaymentID)
{
	CString strWhere;
	strWhere.Format("PaymentTipsT.PaymentID = %li", nPaymentID);
	m_pTipList->PutWhereClause(_bstr_t(strWhere));
}

void CPaymentDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWaitCursor pWait;

	int iPatientID = m_PatientID;
	CString str;

	CNxDialog::OnShowWindow(bShow, nStatus);
	if (!bShow)
		return;

	// (c.haag 2010-10-07 13:49) - PLID 35723 - Set a flag that indicates this window is being initialized
	CInitializingFlag initializingFlag(this);

	//if there is a GetChargeInfo field of 0 in ConfigRT, then GetChargeInfo will be false.
	COleVariant var = GetRemotePropertyInt("GetChargeInfo", -1);
	GetChargeInfo = var.lVal;

	m_bSaved = false;
	
	try {
		str.Format("PatientID = %li", m_PatientID);
		m_InsuranceCombo->WhereClause = _bstr_t(str);
		m_InsuranceCombo->Requery();

		IRowSettingsPtr pRow;
		pRow = m_InsuranceCombo->GetRow(-1);
		pRow->PutValue(rlcID,(long)-1);
		pRow->PutValue(rlcName,"Patient");
		pRow->PutValue(rlcPriority,(long)0);
		// (j.jones 2008-07-10 12:41) - PLID 28756 - added more columns
		pRow->PutValue(rlcDefaultPayDesc, g_cvarNull);
		pRow->PutValue(rlcDefaultPayCategoryID, g_cvarNull);
		pRow->PutValue(rlcDefaultAdjDesc, g_cvarNull);
		pRow->PutValue(rlcDefaultAdjCategoryID, g_cvarNull);
		m_InsuranceCombo->AddRow(pRow);

		//show the patient name
		// (b.spivey, October 01, 2012) - PLID 50949 - Deprecated the old window text loading


	} NxCatchAll("Error in PaymentDlg::OnShowWindow");

	/////////////////////////////////////////////
	// Set default physician
	//str.Format("SELECT [Last Name] & ', ' & [First Name] & ' ' & [Middle Name] AS Name, [Provider ID] FROM [Contact Info] INNER JOIN [Doctors and Providers] ON ([Contact Info].ID = [Doctors and Providers].[Contact ID]) AND ([Contact Info].ID = [Doctors and Providers].[Contact ID]) WHERE ((([Doctors and Providers].[Provider ID])=%d));",
	//	tmpRS["Main Physician"].lVal);

	try {
		COleVariant var;
		COleCurrency cy;
		COleDateTime dt;

		if (m_bIsPrePayment)
			((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->SetCheck(1);
		else
			((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->SetCheck(0);

		//TES 3/25/2015 - PLID 65436 - Let's assume they can edit until we see otherwise.
		m_bCanEdit = TRUE;

		//TES 3/25/2015 - PLID 65175 - Track whether there are any tips
		BOOL bTipExists = FALSE;

		//if we're not loading an existing payment, load the provider and location from the bill or charge

		//////////////////////////////////////////////////////////
		// Charge/non-existing payment
		if(m_varPaymentID.vt != VT_I4) {

			long nProviderID = -1;
			long nLocationID = -1;
			CString strDescription = "";

			//m_DefLocationID may be set right now only if we are auto-applying to a payment
			nLocationID = m_DefLocationID;			

			//pull the ProviderID and LocationID from the Bill or Charge, if we have IDs for them
			if(m_varBillID.vt == VT_I4) {
				//a bill

				// (j.jones 2010-04-06 09:51) - PLID 37131 - disallow using an inactive provider,
				// or an inactive or unmanaged location
				_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.Description, ProvidersQ.ID AS DoctorsProviders, LocationsQ.ID AS LocationID "
					"FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS ProvidersQ ON ChargesT.DoctorsProviders = ProvidersQ.ID "
					"LEFT JOIN (SELECT ID FROM LocationsT WHERE Active = 1 AND Managed = 1 AND TypeID = 1) AS LocationsQ ON LineItemT.LocationID = LocationsQ.ID "
					"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT}", VarLong(m_varBillID));

				long nProviderIDToUse = -2;

				while(!rs->eof) {

					strDescription = AdoFldString(rs, "Description","");

					if(nLocationID == -1)
						nLocationID = AdoFldLong(rs, "LocationID",GetCurrentLocationID());

					long nProvID = AdoFldLong(rs, "DoctorsProviders",-1);

					if(nProviderIDToUse != -2 && nProvID != nProviderIDToUse) {
						//more than one provider on the bill, can't use this function
						nProviderIDToUse = -1;
					}
					else {
						nProviderIDToUse = nProvID;
					}

					rs->MoveNext();
				}
				rs->Close();

				if(nProviderIDToUse != -2)
					nProviderID = nProviderIDToUse;
			}
			else if(m_varChargeID.vt == VT_I4) {
				//a charge

				// (j.jones 2010-04-06 09:51) - PLID 37131 - disallow using an inactive provider,
				// or an inactive or unmanaged location
				_RecordsetPtr rs = CreateParamRecordset("SELECT LineItemT.Description, ProvidersQ.ID AS DoctorsProviders, LocationsQ.ID AS LocationID "
					"FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS ProvidersQ ON ChargesT.DoctorsProviders = ProvidersQ.ID "
					"LEFT JOIN (SELECT ID FROM LocationsT WHERE Active = 1 AND Managed = 1 AND TypeID = 1) AS LocationsQ ON LineItemT.LocationID = LocationsQ.ID "
					"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = {INT}", VarLong(m_varChargeID));
				if(!rs->eof) {
					nProviderID = AdoFldLong(rs, "DoctorsProviders",-1);
					if(nLocationID == -1)
						nLocationID = AdoFldLong(rs, "LocationID",GetCurrentLocationID());
					strDescription = AdoFldString(rs, "Description","");
				}
				rs->Close();
			}
			else {
				//new payment

				long nPatientLocationID = -1;

				// (j.jones 2010-04-06 09:51) - PLID 37131 - disallow using an inactive provider,
				// or an inactive or unmanaged location
				_RecordsetPtr rs = CreateParamRecordset("SELECT ProvidersQ.ID AS MainPhysician, LocationsQ.ID AS Location "
					"FROM PatientsT "
					"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS ProvidersQ ON PatientsT.MainPhysician = ProvidersQ.ID "
					"LEFT JOIN (SELECT ID FROM LocationsT WHERE Active = 1 AND Managed = 1 AND TypeID = 1) AS LocationsQ ON PersonT.Location = LocationsQ.ID "
					"WHERE PatientsT.PersonID = {INT}", m_PatientID);
				if(!rs->eof) {
					nProviderID = AdoFldLong(rs, "MainPhysician",-1);
					nPatientLocationID = AdoFldLong(rs, "Location",-1);
				}
				rs->Close();

				// (j.jones 2010-09-24 13:57) - PLID 34518 - if a copay, but this is a standalone payment,
				// make the description be the insurance company name, such that the payment will say
				// "Copay for Medicare", for example
				// (j.jones 2011-11-09 16:29) - PLID 46348 - this copay could now potentially be for multiple
				// insurance companies, if so concatenate the names
				if(m_bIsCoPay && m_parypInsuranceCoPayApplyList != NULL && m_parypInsuranceCoPayApplyList->GetSize() > 0) {

					//build an array of just the insured party IDs
					CArray<long, long> aryInsuredPartyIDs;
					
					for(int i=0; i<m_parypInsuranceCoPayApplyList->GetSize(); i++) {
						InsuranceCoPayApplyList *icpaiApplyList = (InsuranceCoPayApplyList*)m_parypInsuranceCoPayApplyList->GetAt(i);
						aryInsuredPartyIDs.Add(icpaiApplyList->nInsuredPartyID);
					}

					rs = CreateParamRecordset("SELECT InsuranceCoT.Name "
						"FROM InsuredPartyT "
						"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
						"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
						"WHERE InsuredPartyT.PersonID IN ({INTARRAY}) "
						"ORDER BY (CASE WHEN RespTypeT.Priority = -1 THEN 1 ELSE 0 END) ASC, RespTypeT.Priority ASC", aryInsuredPartyIDs);
					if(!rs->eof) {
						strDescription = "";
					}
					while(!rs->eof) {
						if(!strDescription.IsEmpty()) {
							strDescription += ", ";
						}
						strDescription += AdoFldString(rs, "Name", "");

						rs->MoveNext();
					}
					rs->Close();
				}

				if(nLocationID == -1) {

					//no default, so get the preference for which location to use

					//1 - set the location to the currently logged in location
					//2 - set the location to the patient's location
					long nPayLoc = GetRemotePropertyInt("PayLocationPref", 1, 0, "<None>", true);
					if(nPayLoc == 1) {
						// Set the location to the currently logged in location
						nLocationID = GetCurrentLocationID();
					}
					else {
						// Set the location to the patient's location
						if(nPatientLocationID != -1) {
							nLocationID = nPatientLocationID;
						}
						else {
							// Set the location to the currently logged in location
							nLocationID = GetCurrentLocationID();
						}
					}
				}
			}

			if(nLocationID == -1)
				nLocationID = GetCurrentLocationID();

			m_DefLocationID = nLocationID;

			// (j.gruber 2007-09-07 11:53) - PLID 25191 - check the permission and make it default to the currently logged in location if they don't have permission
			EBuiltInObjectIDs bioObject;
			switch (m_iDefaultPaymentType) {
				case 1:
					bioObject = bioAdjLocation;
				break;
				
				case 2:
					bioObject = bioRefundLocation;
				break;

				default:
					bioObject = bioPaymentLocation;
				break;
			}

				
			if (!(GetCurrentUserPermissions(bioObject) & (SPT___W_______))) {
			
				if (!(GetCurrentUserPermissions(bioObject) & (SPT___W________ANDPASS))) {

					//they don't have write permission, so don't let them change it
					//make sure they are changing it
					if (m_DefLocationID != GetCurrentLocationID()) {
						m_DefLocationID = GetCurrentLocationID();
						nLocationID = GetCurrentLocationID();
					}
												
				}
				else {

					//they have with password, so we'll check when they change it
				}
			}		
		
			if(m_LocationCombo->TrySetSelByColumn(0,(long)nLocationID) == -1) {
				//Probably an inactive location.
				_RecordsetPtr rsLocName = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", nLocationID);
				if(!rsLocName->eof) {
					m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name", "")));
				}
			}

			// (j.jones 2011-07-08 16:58) - PLID 44497 - added preference to default payments to no provider
			if(GetRemotePropertyInt("DefaultPaymentsNoProvider", 0, 0, "<None>", true) == 1) {
				nProviderID = -1;
			}

			if(nProviderID != -1) {
				if(m_ProviderCombo->SetSelByColumn(0,(long)nProviderID) == -1) {
					//they may have an inactive provider
					_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nProviderID);
					if(!rsProv->eof) {
						m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
						m_nTipInactiveProv = nProviderID;
					}
					else 
						m_ProviderCombo->PutCurSel(-1);
				}
			}

			str = FormatCurrencyForInterface(COleCurrency(0,0),FALSE);
			
			GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);

			// (r.gonet 2015-05-13 17:15) - PLID 65326 - If this is being applied to a gift certificate payment, then set the existing checkbox.
			if (m_nOriginalGiftID != -1 || m_nPayToApplyToGiftID != -1) {
				// (r.gonet 2015-04-29 11:09) - PLID 65326 - Default the Refund->Gift Cert->Refund to Existing Gift Certificate radio button to be checked.
				m_radioRefundToExistingGiftCertificate.SetCheck(BST_CHECKED);
				m_radioRefundToNewGiftCertificate.SetCheck(BST_UNCHECKED);
				// (r.gonet 2015-04-29 11:09) - PLID 65326 - Variable is necessary as well because the radio button will not be
				// set until after this function returns, but this function calls other functions that need to which mode are refunding in.
				m_bRefundToExistingGC = TRUE;
			} else {
				// (r.gonet 2015-05-13 17:15) - PLID 65326 - If there is no gift certificate in the payment we are refunding, then select to create a new gift certificate.
				m_radioRefundToExistingGiftCertificate.SetCheck(BST_UNCHECKED);
				m_radioRefundToNewGiftCertificate.SetCheck(BST_CHECKED);
				m_bRefundToExistingGC = FALSE;
			}
			// (r.gonet 2015-04-29 11:09) - PLID 65326 - If there is a gift certificate referenced by this payment, re-select it.
			// I think this shouldn't ever be called.
			if (m_nOriginalGiftID != -1) {
				m_nSelectedGiftID = m_nOriginalGiftID;
				UpdateGiftCertificateControls_FromID(m_nSelectedGiftID);
			}

			// Set default payment style
			switch (m_iDefaultPaymentType) {
			case 1:
				m_radioAdjustment.SetCheck(1);
				PostClickRadioAdjustment();
				//We want "-0"
				str = FormatCurrencyForInterface(COleCurrency(-1,0),FALSE);
				str.Replace("1","0");
				GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);
				int nStart, nFinish;
				GetNonNegativeAmountExtent(nStart, nFinish);
				((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(nStart, nFinish);
				GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
				break;
			case 2:
				m_radioRefund.SetCheck(1);
				PostClickRadioRefund();
				break;
			default:
				m_radioPayment.SetCheck(1);

				// (d.thompson 2009-08-11) - PLID 24079 - We now have a preference for default method for payments only
				// (d.thompson 2012-08-01) - PLID 51898 - Changed default to charge (3)
				// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
				long nDefaultMethodInt = GetRemotePropertyInt("NewPaymentDefaultMethod", (long)EPayMethod::ChargePayment, 0, GetCurrentUserName(), true);
				EPayMethod eDefaultMethod = AsPayMethod(nDefaultMethodInt, EPayMethod::ChargePayment);
				if(!m_bHasNexSpa && eDefaultMethod == EPayMethod::GiftCertificatePayment) {
					//revert to check if they default to GC and don't have nexspa
					// (d.thompson 2012-08-01) - PLID 51898 - Changed default to charge (3)
					eDefaultMethod = EPayMethod::ChargePayment;
				}
				//Just in case of some odd value, default back to check
				// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
				switch(eDefaultMethod) {
					case EPayMethod::CashPayment:	//cash
						m_radioCash.SetCheck(1);
						break;
					case EPayMethod::ChargePayment:	//credit
						m_radioCharge.SetCheck(1);
						break;
					case EPayMethod::GiftCertificatePayment:	//gift cert
						m_radioGift.SetCheck(1);
						break;
					case EPayMethod::CheckPayment:	//check
					default:
						m_radioCheck.SetCheck(1);
						break;
				}

				PostClickRadioPayment();

				// (j.jones 2007-04-19 15:14) - PLID 25711 - if a new payment, we want OnRadioCheckUpdated
				// (d.thompson 2009-08-11) - PLID 24079 - Depends what default we picked
				// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
				switch(eDefaultMethod) {
					case EPayMethod::CashPayment:	//cash
						OnRadioCashUpdated();
						break;
					case EPayMethod::ChargePayment:	//credit
						OnRadioChargeUpdated();
						break;
					case EPayMethod::GiftCertificatePayment:	//gift cert
						OnRadioGiftCertUpdated();
						break;
					case EPayMethod::CheckPayment:	//check
					default:
						OnRadioCheckUpdated();
						break;
				}
				
				break;
			}

			//new payment, follow the user preferences on what date to use
			//default to the current date
			COleDateTime dt = COleDateTime::GetCurrentTime();

			// (j.jones 2010-05-25 15:09) - PLID 38661 - split into a separate copay preference

			// (j.jones 2010-08-26 17:35) - PLID 34016 - changed each preference to be per user, defaults to the old global setting
			long nDefaultPaymentDateGlobal_Old = GetRemotePropertyInt("DefaultPaymentDate",1,0,"<None>",TRUE);

			long DefaultPaymentDate = 1;
			if(m_bIsCoPay) {
				//this defaults to whatever the DefaultPaymentDate preference was when they first load the preference
				long nDefaultPaymentDateUser = GetRemotePropertyInt("DefaultPaymentDate", nDefaultPaymentDateGlobal_Old, 0, GetCurrentUserName(), true);
				// (s.dhole 2011-11-07 10:58) - PLID 45330 Read DefaultCoPaymentDate preference  same as we setup in preferences (DefaultCoPaymentDate)   
				long nDefaultCoPaymentDateGlobal_Old = GetRemotePropertyInt("DefaultCoPaymentDate", nDefaultPaymentDateUser, 0, "<None>", true);
				DefaultPaymentDate = GetRemotePropertyInt("DefaultCoPaymentDate",GetRemotePropertyInt("DefaultPaymentDate",nDefaultCoPaymentDateGlobal_Old,0,GetCurrentUserName(),TRUE),0,GetCurrentUserName(),TRUE);
			}
			else {
				DefaultPaymentDate = GetRemotePropertyInt("DefaultPaymentDate",nDefaultPaymentDateGlobal_Old,0,GetCurrentUserName(),TRUE);
			}

			if(DefaultPaymentDate == 3)
				// (j.jones 2007-05-04 09:47) - PLID 23280 - pull the last payment date from GlobalUtils
				dt = GetLastPaymentDate();
			else if(DefaultPaymentDate == 2) {
				//now find the bill date
				long BillID = -1;
				if(m_varBillID.vt == VT_I4 && VarLong(m_varBillID) != -1) {
					BillID = VarLong(m_varBillID);
				}
				else if(m_varChargeID.vt == VT_I4 && VarLong(m_varChargeID) != -1) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT BillID FROM ChargesT WHERE ID = {INT}", VarLong(m_varChargeID));
					if(!rs->eof) {
						BillID = AdoFldLong(rs, "BillID",-1);
					}
					rs->Close();
				}

				if(BillID != -1) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}",BillID);
					if(!rs->eof) {
						dt = AdoFldDateTime(rs, "Date");
					}
					rs->Close();
				}
			}
			// (j.jones 2010-05-25 14:38) - PLID 38876 - supported charge date
			else if(DefaultPaymentDate == 4) {
				//find the charge date
				if(m_varChargeID.vt == VT_I4 && VarLong(m_varChargeID) != -1) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT Date FROM LineItemT WHERE ID = {INT}", VarLong(m_varChargeID));
					if(!rs->eof) {
						dt = AdoFldDateTime(rs, "Date");
					}
					rs->Close();
				}
				else if(m_varBillID.vt == VT_I4 && VarLong(m_varBillID) != -1) {
					//select the earliest charge date on the bill
					_RecordsetPtr rs = CreateParamRecordset("SELECT Min(Date) AS MinDate FROM LineItemT "
						"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
						"WHERE ChargesT.BillID = {INT} AND Deleted = 0", VarLong(m_varBillID));
					if(!rs->eof) {
						dt = AdoFldDateTime(rs, "MinDate");
					}
					rs->Close();
				}
			}

			//now dt has the value we want, so lets set the date
			m_date.SetValue(COleVariant(dt));			

			//////////////////////////////////////////////////////////////
			// Fill in amount due given the bill or charge values

			if (strDescription.GetLength() > 0) {
				if(!m_bIsCoPay) {
					// (j.jones 2008-05-27 09:22) - PLID 24661 - added preference to determine if non-copays
					// include the "For" prefix before the description
					if(GetRemotePropertyInt("IncludeForInPayDescription", 1, 0, "<None>", true) == 1) {
						str.Format("For %s", strDescription);
					}
					else {
						str = strDescription;
					}
				}
				else {
					str.Format("CoPay for %s", strDescription);
				}
				m_DescriptionCombo->CurSel = -1;
				GetDlgItem(IDC_EDIT_DESCRIPTION)->SetWindowText(str);
			}

			if (m_cyFinalAmount != COleCurrency(0,0)) {
				str = FormatCurrencyForInterface(m_cyFinalAmount, FALSE);
				if (m_cyFinalAmount >= COleCurrency(0,0)) {
					GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);
					((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(0, -1);
				}
				else {
					GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);
					int nStart, nFinish;
					GetNonNegativeAmountExtent(nStart, nFinish);				
					((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(nStart, nFinish);
				}
			}

			// (j.gruber 2007-08-02 12:31) - PLID 26916 - fillin the credit card information if this is a refund
			if (m_bSetCreditCardInfo && m_PaymentToApplyTo.nPaymentID != -1 && m_iDefaultPaymentType == 2) {
				// (a.walling 2007-08-03 15:55) - PLID 26922 - If they have CC Processing, but the user lacks permission,
				// then we prevent them from creating a charge/card payment/refund.

				BOOL bContinue = TRUE;
				// (d.thompson 2010-09-02) - PLID 40371 - Any license type satisfies
				// (j.jones 2015-09-30 08:58) - PLID 67157 - if you have ICCP, you are allowed to make a credit
				// card payment, you just won't be able to process it
				if (g_pLicense && g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent) && !IsICCPEnabled()) {
					// they are licensed, check permission silently (will prompt for PW when saving)
					if (!CheckCurrentUserPermissions(bioCCProcess, sptWrite, FALSE, 0, TRUE, TRUE)) {
						// they don't have permission! Warn them
						MessageBox("Payment type cannot be set to 'Charge'; you do not have permission to process credit cards. Please check with your office manager.");
						bContinue = FALSE;
						// use the defaults now, just don't set as a charge/card.
					}
				}

				if (bContinue) {
					//we have a payment and a refundID
					//first, make is a charge
					PostClickRadioCharge();

					//now fillin the values
					//(e.lally 2007-10-30) PLID 27892 - Only display the last 4 digits and buffer with X's
					//The card number may or may not be filled in already (filled for new refunds)
					{
						CString strDisplayedCCNumber = MaskCCNumber(m_strCreditCardNumber);
						// (j.jones 2015-09-30 09:31) - PLID 67167 - ICCP uses a different control to show this
						if (!IsICCPEnabled()) {
							m_nxeditBankNameOrCCCardNum.SetWindowText(strDisplayedCCNumber);
						}
						else {
							m_nxeditCCLast4.SetWindowText(strDisplayedCCNumber.Right(4));
						}
					}
					m_nxeditAcctNumOrCCNameOnCard.SetWindowText(m_strNameOnCard);
					m_nxeditCheckNumOrCCExpDate.SetWindowText(m_strExpDate);

					m_CardNameCombo->TrySetSelByColumn(cccCardID, m_nCreditCardID);
				}
			}					
			else if (m_PaymentToApplyTo.nPaymentID != -1 && m_iDefaultPaymentType == 2) {
				//TES 4/20/2015 - PLID 65655 - We're refunding an existing payment, default to its payment method (if possible, note that if the payment
				// was a gift certificate, we leave the method unchanged, because gift certificate refunds are not allowed).
				switch (m_nPayToApplyToMethod) {
				case 1:
					PostClickRadioCash();
					break;
				case 2:
					PostClickRadioCheck();
					break;
				case 3:
					PostClickRadioCharge();
					break;
				case 4: // (r.gonet 2015-04-29 11:09) - PLID 65326 -  If the payment was with a gift certificate, refund to a gift certiifcate.
					PostClickRadioGC();
					// (r.gonet 2015-04-29 11:09) - PLID 65326 - Pre-select the gift certificate that made the payment, if there was one. It is 
					// thought that the common case would be that the user would want to put the money back on the same gift card.
					if (m_nPayToApplyToGiftID != -1) {
						m_nSelectedGiftID = m_nPayToApplyToGiftID;
						UpdateGiftCertificateControls_FromID(m_nSelectedGiftID);
					}
					break;
				}
			}

			//TES 4/2/2015 - PLID 65171 - If this is a refund of a known payment, and if we have both the preference and permission to refund tips, prompt to do so.
			if (m_PaymentToApplyTo.nPaymentID != -1 && m_iDefaultPaymentType == 2 && GetRemotePropertyInt("AllowTipRefunds", 0, 0, "<None>") && (GetCurrentUserPermissions(bioRefundTips) & sptWrite)) {
				//TES 4/2/2015 - PLID 65171 - Does the source payment have any tips?
				//TES 4/20/2015 - PLID 65171 - Added PayMethod
				_RecordsetPtr rsTips = CreateParamRecordset("SELECT ID, ProvID, Amount, PayMethod FROM PaymentTipsT WHERE PaymentID = {INT}", m_PaymentToApplyTo.nPaymentID);
				if (!rsTips->eof) {
					//TES 4/2/2015 - PLID 65171 - It does, so prompt to add them.
					if (IDYES == MsgBox(MB_YESNO, "The selected payment to be refunded has a tip associated with it. Would you like to refund the tip?")) {
						//TES 4/2/2015 - PLID 65171 - Assign the tip the same paymethod as is currently selected for this refund
						//TES 4/20/2015 - PLID 65171 - We'll use the same method as the original tip, but we'll still use this calculation as the
						// default just in case the original tip was made with a gift certificate, which isn't allowed on a refund
						long nTipMethod = 8;
						if (m_radioCash.GetCheck()) {
							nTipMethod = 7;
						} else if (m_radioCharge.GetCheck()) {
							nTipMethod = 9;
						} else if (m_radioGift.GetCheck()) {
							// (r.gonet 2015-04-29 11:09) - PLID 65326 - Added paymethod = 10
							nTipMethod = 10;
						} else {
							ASSERT(m_radioCheck.GetCheck());
						}
						while (!rsTips->eof) {
							IRowSettingsPtr pTipRow = m_pTipList->GetRow(-1);
							pTipRow->PutValue(tlcID, (long)-1);
							pTipRow->PutValue(tlcProvID, rsTips->Fields->GetItem("ProvID")->Value);
							//TES 4/2/2015 - PLID 65171 - Refund amounts are negative
							COleCurrency cyTipAmount = -1*AdoFldCurrency(rsTips, "Amount");
							pTipRow->PutValue(tlcAmount, _variant_t(cyTipAmount.m_cur));
							long nPayTipMethod = AdoFldLong(rsTips, "PayMethod");
							//TES 4/20/2015 - PLID 65171 - Use the method from the original tip.
							switch (nPayTipMethod) {
							case 1:
								pTipRow->PutValue(tlcMethod, 7);
								break;
							case 2:
								pTipRow->PutValue(tlcMethod, 8);
								break;
							case 3:
								pTipRow->PutValue(tlcMethod, 9);
								break;
							case 4:
								// (r.gonet 2015-04-29 11:09) - PLID 65326 - Added paymethod = 10
								pTipRow->PutValue(tlcMethod, 10);
								break;
							default:
								pTipRow->PutValue(tlcMethod, nTipMethod);
								break;
							}
							m_pTipList->AddRow(pTipRow);
							rsTips->MoveNext();
						}
						bTipExists = true;
						UpdateTotalTipAmt();
					}
				}
			}
		}
		else {

			//we're loading an existing payment

			//////////////////////////////////////////////////////////
			// Payment/Adjustment/Refund
			m_boIsNewPayment = FALSE;

			// (a.walling 2006-11-15 12:45) - PLID 23550 - Retrieve the group and reason code from the data
			// (j.jones 2007-03-27 17:31) - PLID 24080 - added Batch Payment information
			// (j.jones 2007-04-19 14:36) - PLID 25711 - included ReturnedProductInformation, and apply information
			// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.
				// - Formatted part of the query so I could read it, taking out some unneeded parenthesis
				// - Changed varPaymentID to use a varLong instead of .lval
			// (e.lally 2007-07-12) PLID 26590 - Added in the CardName field and split one long line of fields into 3 lines.
			// (j.gruber 2007-07-16 14:15) - PLID 15416 - added CC transaction fields
			// (a.walling 2007-10-30 18:05) - PLID 27891 - Included encrypted credit card number
			// (j.jones 2008-06-03 14:28) - PLID 29928 - only look at ReturnedProductsT records that are not for deleted charges
			// (j.jones 2009-06-09 15:50) - PLID 34549 - ignore batch payments if an adjustment
			// (d.thompson 2009-07-01) - PLID 34687 - Changed CreditTransactionsT data to QBMS
			// (a.walling 2010-03-15 12:27) - PLID 37751 - Include KeyIndex
			// (j.jones 2010-09-22 11:27) - PLID 40631 - added support for Chase_CreditTransactionsT, QBMS takes priority (also parameterized this query)
			// (j.jones 2014-07-02 09:18) - PLID 62562 - track the batch payment type
			// (b.spivey, September 25, 2014) - PLID 63422 - Added the deleted status, which should only ever be false if a valid lockbox payment is linked here.
			// (r.gonet 2015-04-20) - PLID 65326 - Converted from using a magic number to using the EPayMethod enum.
			// (r.gonet 2015-05-05 10:03) - PLID 65657 - Select the ID of the gift certificate that paid the payment this refund is refunding.
			// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType
			// (j.jones 2015-09-30 10:12) - PLID 67168 - split HasTransaction into HasAnyCCTransaction, HasPreICCPTransaction, HasICCPTransaction
			// (j.jones 2015-09-30 09:18) - PLID 67163 - added ICCP transaction account number
			_RecordsetPtr rs = CreateParamRecordset("SELECT [PatientPaymentsQ].Date, PaymentsT.ProviderID, PaymentGroupID, PaymentsT.InsuredPartyID, (CASE WHEN ([PatientPaymentsQ].[Type]=1) THEN PaymentsT.PayMethod ELSE (CASE WHEN([PatientPaymentsQ].[Type]=2) THEN {CONST_INT} ELSE PaymentsT.PayMethod END) END) AS Method, "
					"PaymentsT.GroupCodeID, PaymentsT.ReasonCodeID, PaymentsT.CCProcessType, "
					"[PatientPaymentsQ].Type, PaymentPlansT.CheckNo AS CheckNumber, PaymentPlansT.BankNo AS [Bank Number], "
					"PaymentPlansT.CheckAcctNo AS [Checking Acct Number], PaymentPlansT.BankRoutingNum, "
					"PaymentPlansT.CreditCardID, CreditCardNamesT.CardName, PaymentPlansT.CCNumber AS CreditCardNumber, PaymentPlansT.SecurePAN, PaymentPlansT.KeyIndex, PaymentPlansT.CCHoldersName AS CardholdersName, PaymentPlansT.CCExpDate AS CreditCardExpDate, "
					"PaymentPlansT.CCAuthNo AS CreditCardAuthorizationNumber, [PatientPaymentsQ].Amount, PaymentsT.CashReceived, "
					"[PatientPaymentsQ].Description AS Notes, PaymentsT.PrePayment, PatientPaymentsQ.LocationID, QuoteID, GiftID, DrawerID, "
					"BatchPaymentsT.ID AS BatchPaymentID, BatchPaymentsT.Amount AS BatchPayAmount, BatchPaymentsT.Date AS BatchPayDate, BatchPaymentsT.PayType AS BatchPaymentPayType, "
					"CASE WHEN PaymentsT.ID IN (SELECT FinAdjID FROM ReturnedProductsT WHERE ChargeID NOT IN (SELECT ID FROM LineItemT WHERE Deleted = 1)) THEN 1 ELSE 0 END AS IsReturnedProductAdj, "
					"CASE WHEN PaymentsT.ID IN (SELECT FinRefundID FROM ReturnedProductsT WHERE ChargeID NOT IN (SELECT ID FROM LineItemT WHERE Deleted = 1)) THEN 1 ELSE 0 END AS IsReturnedProductRefund, "
					"CASE WHEN PaymentsT.ID IN (SELECT SourceID FROM AppliesT) THEN 1 ELSE 0 END AS IsSourceApply, "
					"CASE WHEN PaymentsT.ID IN (SELECT DestID FROM AppliesT) THEN 1 ELSE 0 END AS IsDestApply, "
					"COALESCE(LBT.Deleted, 1) AS LockboxBatchDeletedStatus, PaymentsT.RefundedFromGiftID, "
					"Convert(bit, CASE WHEN QBMS_CreditTransactionsT.ID IS NULL AND Chase_CreditTransactionsT.ID IS NULL AND CardConnect_CreditTransactionT.ID IS NULL THEN 0 ELSE 1 END) AS HasAnyCCTransaction, "
					"Convert(bit, CASE WHEN QBMS_CreditTransactionsT.ID IS NULL AND Chase_CreditTransactionsT.ID IS NULL THEN 0 ELSE 1 END) AS HasPreICCPTransaction, "
					"Convert(bit, CASE WHEN CardConnect_CreditTransactionT.ID IS NULL THEN 0 ELSE 1 END) AS HasICCPTransaction, "					
					//Chase/Intuit have IsApproved fields, but the presence of CardConnect transaction inherently means approval
					"CASE WHEN QBMS_CreditTransactionsT.ID Is Not Null THEN QBMS_CreditTransactionsT.IsApproved "
					"	WHEN Chase_CreditTransactionsT.ID Is Not Null THEN Chase_CreditTransactionsT.IsApproved "
					"	WHEN CardConnect_CreditTransactionT.ID Is Not Null THEN Convert(bit, 1) "
					"	ELSE Convert(bit, 0) "
					"END AS IsApproved, "
					"COALESCE(LBT.Deleted, 1) AS LockboxBatchDeletedStatus, "
					"CardConnect_CreditTransactionT.AccountID AS ICCPAccountID "
					"FROM ((SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
					"		FROM LineItemT "
					"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "					
					"		WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted=0 AND "
					"			LineItemT.Type>=1 And LineItemT.Type<=3) AS PatientPaymentsQ "
					"		INNER JOIN PaymentsT ON [PatientPaymentsQ].ID = PaymentsT.ID) "
					"LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
					"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
					"LEFT JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID AND PatientPaymentsQ.Type <> 2 "
					"LEFT JOIN QBMS_CreditTransactionsT ON PaymentsT.ID = QBMS_CreditTransactionsT.ID "
					"LEFT JOIN Chase_CreditTransactionsT ON PaymentsT.ID = Chase_CreditTransactionsT.ID "
					"LEFT JOIN CardConnect_CreditTransactionT ON PaymentsT.ID = CardConnect_CreditTransactionT.ID "
					"LEFT JOIN LockboxPaymentMapT LBPMT ON PaymentsT.ID = LBPMT.PaymentID " 
					"LEFT JOIN LockboxPaymentT LBPT ON LBPMT.LockboxPaymentID = LBPT.ID " 
					"LEFT JOIN LockboxBatchT LBT ON LBPT.LockboxBatchID = LBT.ID "
					"WHERE [PatientPaymentsQ].ID = {INT}", (long)EPayMethod::LegacyAdjustment, m_PatientID, VarLong(m_varPaymentID));
			
			if (rs->eof) {
				MsgBox("This payment has been deleted (possibly by another user); please click on the Cancel button of the Payment dialog to abort this operation.");
				rs->Close();
				return;
			}

			// (j.jones 2015-09-30 10:12) - PLID 67168 - split HasTransaction into HasAnyCCTransaction, HasPreICCPTransaction, HasICCPTransaction
			m_bHasAnyTransaction = AdoFldBool(rs, "HasAnyCCTransaction", FALSE) ? true : false;
			m_bHasPreICCPTransaction = AdoFldBool(rs, "HasPreICCPTransaction", FALSE) ? true : false;
			m_bHasICCPTransaction = AdoFldBool(rs, "HasICCPTransaction", FALSE) ? true : false;

			// (b.spivey, September 25, 2014) - PLID 63422 - store these. 
			m_bIsLinkedLockboxPayment = !AdoFldLong(rs->Fields, "LockboxBatchDeletedStatus", 1);
			m_cyOriginalAmount = AdoFldCurrency(rs->Fields, "Amount", COleCurrency(0, 0));
			// (r.gonet 2015-05-05 10:03) - PLID 65657 - Remember the ID of the gift certificate that paid the payment this refund is refunding.
			m_nRefundedFromGiftID = AdoFldLong(rs->Fields, "RefundedFromGiftID", -1);

			var = rs->Fields->Item["Date"]->Value;	// Payment date
			m_date.SetValue(var);
			var = rs->Fields->Item["ProviderID"]->Value; // Payment Provider
			if (var.vt != VT_NULL)
			{
				if(m_ProviderCombo->SetSelByColumn(0,var) == -1) {
					//they may have an inactive provider
					_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", VarLong(var, -1));
					if(!rsProv->eof) {
						m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
						m_nTipInactiveProv = VarLong(var);

					}
					else 
						m_ProviderCombo->PutCurSel(-1);
				}
			}
			else 
				m_ProviderCombo->PutCurSel(-1);

			var = rs->Fields->Item["LocationID"]->Value; // Payment Location
			if(m_LocationCombo->TrySetSelByColumn(0,var) == -1) {
				//Probably an inactive location.
				_RecordsetPtr rsLocName = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", VarLong(var, -1));
				if(!rsLocName->eof) {
					m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name", "")));
					m_nTipInactiveProv = VarLong(var);
				}
			}
			var = rs->Fields->Item["PaymentGroupID"]->Value;
			long nCategoryRow = m_CategoryCombo->SetSelByColumn(0,var); // Payment category

			// (b.spivey, March 11, 2013) - PLID 51186 - Base this value off the category description. 
			if(nCategoryRow > sriNoRow) {
				var = m_CategoryCombo->GetValue(nCategoryRow, pccCategoryDescription);
				// (b.spivey, March 12, 2013) - PLID 51186 - Forgot payment name case. 
				if (VarString(var, "").GetLength() <= 0) {
					var = m_CategoryCombo->GetValue(nCategoryRow, pccCategoryName); 
				}
			}
			else {
				var = ""; 
			}

			// (b.spivey, March 06, 2013) - PLID 51186 - load this value. 
			m_strLastSetDescription = VarString(var, ""); 

			var = rs->Fields->Item["InsuredPartyID"]->Value;
			m_InsuranceCombo->SetSelByColumn(0,var); // Insurance company

			var = rs->Fields->Item["DrawerID"]->Value;
			if(var.vt == VT_I4) {
				long nSel = m_pDrawerList->SetSelByColumn(0, var);

				if(nSel == sriNoRow) {
					//they saved a drawer ID, but we couldn't select it ... they must have 
					//closed that drawer.  Disable this whole thing and don't let them make 
					//any changes to the drawer.
					m_pDrawerList->PutComboBoxText("Drawer Closed");
					GetDlgItem(IDC_CASH_DRAWER_LIST)->EnableWindow(FALSE);
				}
			}
			else { 
				//DRT 2/9/2005 - PLID 15574 - The OnInitDialog function selects the last drawer used, so if 
				//	we don't have a selection, we need to force that.  I definitely remember testing this
				//	before and it working fine, so we must have changed some of the methods here that is 
				//	causing this bug.
				// (a.walling 2007-09-21 18:01) - PLID 27484 - Not sure if this was meant to set the selection
				// to NULL (0), or if it actually intended to set it to the first item in the list (index 0)
				// since this is not a datalist2. The problem is that, although the <No Drawer> item is usually
				// first in the list, you run into problems if people name their drawers 1, 2, 3, etc.
				// m_pDrawerList->PutCurSel(0);
				m_pDrawerList->SetSelByColumn(0, (long)-1);
			}

			// (j.jones 2007-04-19 14:40) - PLID 25711 - store the ReturnedProduct info, if any
			if(!m_bIsReturnedProductAdj && AdoFldLong(rs, "IsReturnedProductAdj",0) != 0) { //don't set to false, only set to true, incase it is passed in
				m_bIsReturnedProductAdj = TRUE;
			}
			if(!m_bIsReturnedProductRefund && AdoFldLong(rs, "IsReturnedProductRefund",0) != 0) { //don't set to false, only set to true, incase it is passed in
				m_bIsReturnedProductRefund = TRUE;
			}

			// (j.jones 2005-04-19 10:20) - PLID 16063 - remember if this is a prepay linked to a quote
			m_QuoteID = AdoFldLong(rs, "QuoteID",-1);

			// (j.jones 2007-03-27 17:32) - PLID 24080 - if a batch payment, grab its information
			m_nBatchPaymentID = AdoFldLong(rs, "BatchPaymentID",-1);
			if(m_nBatchPaymentID != -1) {				
				COleCurrency cyAmount = AdoFldCurrency(rs, "BatchPayAmount", COleCurrency(0,0));
				COleDateTime dtDate = AdoFldDateTime(rs, "BatchPayDate", COleDateTime::GetCurrentTime());

				//format the hyperlink text, to be drawn later
				m_strBatchPaymentInfo.Format("Part of a %s batch payment from %s.",
					FormatCurrencyForInterface(cyAmount), FormatDateTimeForInterface(dtDate, NULL, dtoDate));

				// (j.jones 2014-07-02 09:18) - PLID 62562 - track the batch payment type
				m_eBatchPayType = (EBatchPaymentPayType)AdoFldLong(rs, "BatchPaymentPayType", (long)eMedicalPayment);
				//if a vision payment, and the user no longer has the vision payment license,
				//treat this like a normal medical payment
				if (m_eBatchPayType == eVisionPayment && !g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {
					m_eBatchPayType = eMedicalPayment;
				}
			}

			

			var = rs->Fields->Item["Method"]->Value;	// Payment method
			if (var.vt == VT_NULL) {
				MessageBox("This is an invalid payment. Please delete it from the Financial Tab.");
				rs->Close();
				return;
			}
			if (var.lVal == 0) { 
				// (r.gonet 2015-04-20) - PLID 65326 - Above in the load recordset, we did a conditional where if the line item type was Adjustment, then select the LegacyAdjustment(6) method rather Adjustment(0). 
				// This is because there may be very old data that is set to 0 but is not an adjustment. Thus the only case where we have 0 here is if the payment is not actually an adjustment. We'll figure it
				// out in the default case in the switch using the line item type.
				var.lVal = (long)EPayMethod::Invalid;
			}
			// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
			EPayMethod eMethod = AsPayMethod(var.lVal);
			// (r.gonet 2015-04-20) - PLID 65326 - Remember the paymethod.
			m_eOriginalPayMethod = eMethod;
			switch (eMethod) {
			case EPayMethod::CashPayment:	// Cash
				GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(TRUE);
				m_radioPayment.SetCheck(1);
				PostClickRadioPayment();
				PostClickRadioCash();
				m_radioCheck.SetCheck(0);
				EnsureLabelArea();
				m_nxstaticLabelCheckNumOrCCExpDate.SetWindowText("Check Number");
				//PLID 15566 - Moved 'Cash Received' below the switch, it's now done for all methods 1-4 (Cash Payment, Check Payment, Credit Card Payment, and Gift Certificate Payment)
				break;
			case EPayMethod::CheckPayment:	// Check
				GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(TRUE);
				m_radioPayment.SetCheck(1);
				PostClickRadioPayment();
				PostClickRadioCheck();
				EnsureLabelArea();
				var = rs->Fields->Item["CheckNumber"]->Value; // Check number
				if (var.vt != VT_NULL) {
					m_nxeditCheckNumOrCCExpDate.SetWindowText(CString(var.bstrVal));
				}
				var = rs->Fields->Item["Bank Number"]->Value; // Bank number
				if (var.vt != VT_NULL) {
					m_nxeditBankNameOrCCCardNum.SetWindowText(CString(var.bstrVal));
				}
				var = rs->Fields->Item["Checking Acct Number"]->Value; // Account number
				if (var.vt != VT_NULL) {
					m_nxeditAcctNumOrCCNameOnCard.SetWindowText(CString(var.bstrVal));
				}
				var = rs->Fields->Item["BankRoutingNum"]->Value; // Account number
				m_nxeditBankNumOrCCAuthNum.SetWindowText(VarString(var, ""));
				break;
			case EPayMethod::ChargePayment:	// Credit charge
			case EPayMethod::ChargeRefund: // Credit Refund
			{
				// (j.jones 2015-09-30 08:58) - PLID 67157 - merged the credit card payment/refund code
				// since so much of it was duplicated

				// (a.walling 2007-08-03 16:29) - PLID 26922 - If a user who lacks CC Process permissions
				// edits a saved payment/refund, we don't need to do anything special. j.gruber says that
				// once they are processed then they should not be able to be changed anyway.

				bool bIsRefund = (eMethod == EPayMethod::ChargeRefund);

				if (!bIsRefund) {
					//payment
					GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(TRUE);
					
					m_radioPayment.SetCheck(1);
					PostClickRadioPayment();
				}
				else {
					//refund
					GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
					GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
					
					m_radioRefund.SetCheck(1);
					PostClickRadioRefund();
				}
				
				PostClickRadioCharge();
				EnsureLabelArea();

				// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.
				// (e.lally 2007-07-17) PLID 25993 - Made all selections use the ID instead of name in case of duplicates
				var = rs->Fields->Item["CreditCardID"]->Value;	// Credit card type
				// (e.lally 2007-07-12) PLID 26590 - The member card ID needs to be set so the trysetsel
				//can add an inactive card
				m_nCreditCardID = VarLong(var, -1);
				if (var.vt != VT_NULL){
					int nResult = m_CardNameCombo->TrySetSelByColumn(cccCardID, m_nCreditCardID);

					//Check if we found the card in the list
					if (nResult == NXDATALISTLib::sriNoRow && m_nCreditCardID != -1) {
						//The list is loaded and the card name is not empty, it must be inactive
						_RecordsetPtr rs = CreateParamRecordset("SELECT CardName "
							"FROM CreditCardNamesT "
							"WHERE Inactive = 1 AND ID = {INT} ", m_nCreditCardID);
						if (!rs->eof){
							IRowSettingsPtr pRow = m_CardNameCombo->GetRow(sriGetNewRow);
							pRow->PutValue(cccCardID, m_nCreditCardID);
							pRow->PutValue(cccCardName, _bstr_t(AdoFldString(rs, "CardName")));

							m_CardNameCombo->AddRow(pRow);
							m_CardNameCombo->SetSelByColumn(cccCardID, m_nCreditCardID);
						}
					}
				}

				// (j.jones 2015-09-30 08:58) - PLID 67157 - load CCProcessType, for ICCP only
				if (IsICCPEnabled()) {
					// (j.jones 2015-09-30 10:12) - PLID 67168 - if the refund was processed using Chase/Intuit,
					// force the process type to be other and disable the swipe/card on file controls
					if (m_bHasPreICCPTransaction) {

						//force 'Other (Do Not Process)'
						m_radioCCDoNotProcess.SetCheck(TRUE);

						//disable all other controls
						m_radioCCSwipe.SetCheck(FALSE);
						m_radioCCSwipe.EnableWindow(FALSE);
						m_radioCCCardNotPresent.SetCheck(FALSE);
						m_radioCCCardNotPresent.EnableWindow(FALSE);
						m_radioCCRefundToOriginalCard.SetCheck(FALSE);
						m_radioCCRefundToOriginalCard.EnableWindow(FALSE);
						PostClickRadioCCDoNotProcess();
					}
					else {
						//not processed by Chase/Intuit

						LineItem::CCProcessType eCCProcessType = (LineItem::CCProcessType)AdoFldLong(rs, "CCProcessType", (long)LineItem::CCProcessType::None);
						switch (eCCProcessType)
						{
						case LineItem::CCProcessType::Swipe:
							m_radioCCSwipe.SetCheck(TRUE);

							m_radioCCCardNotPresent.SetCheck(FALSE);
							m_radioCCDoNotProcess.SetCheck(FALSE);
							m_radioCCRefundToOriginalCard.SetCheck(FALSE);
							PostClickRadioCCSwipe();
							break;
						case LineItem::CCProcessType::CardNotPresent:
							m_radioCCCardNotPresent.SetCheck(TRUE);

							m_radioCCSwipe.SetCheck(FALSE);
							m_radioCCDoNotProcess.SetCheck(FALSE);
							m_radioCCRefundToOriginalCard.SetCheck(FALSE);
							PostClickRadioCCCardNotPresent();
							break;
						case LineItem::CCProcessType::RefundToOriginal:
							//this better be a refund
							ASSERT(bIsRefund);
							m_radioCCRefundToOriginalCard.SetCheck(TRUE);

							m_radioCCSwipe.SetCheck(FALSE);
							m_radioCCCardNotPresent.SetCheck(FALSE);
							m_radioCCDoNotProcess.SetCheck(FALSE);
							PostClickRadioCCRefundToOriginalCard();
							//if this isn't a refund, something is wrong,
							//so skip to the Do Not Process case
							if (bIsRefund) {
								break;
							}
						case LineItem::CCProcessType::DoNotProcess:
						default:
							m_radioCCDoNotProcess.SetCheck(TRUE);

							m_radioCCSwipe.SetCheck(FALSE);
							m_radioCCCardNotPresent.SetCheck(FALSE);
							m_radioCCRefundToOriginalCard.SetCheck(FALSE);
							PostClickRadioCCDoNotProcess();
							break;
						}
					}
				}

				// (j.jones 2015-09-30 09:18) - PLID 67163 - if we have a merchant ID from
				// a card connect transaction, select it
				if (IsICCPEnabled() && m_bHasICCPTransaction) {					
					long nMerchantID = VarLong(rs->Fields->Item["ICCPAccountID"]->Value, -1);
					if (nMerchantID != -1) {
						NXDATALIST2Lib::IRowSettingsPtr pMerchantRow = m_CCMerchantAccountCombo->SetSelByColumn(maccID, nMerchantID);
						if (pMerchantRow == NULL) {
							//is the merchant account just inactive?
							_RecordsetPtr rs = CreateParamRecordset("SELECT Description, MerchantID "
								"FROM CardConnect_SetupDataT "
								"WHERE ID = {INT} ", nMerchantID);
							if (!rs->eof){
								NXDATALIST2Lib::IRowSettingsPtr pNewMerchantRow = m_CCMerchantAccountCombo->GetNewRow();
								pNewMerchantRow->PutValue(maccID, nMerchantID);
								pNewMerchantRow->PutValue(maccDescription, rs->Fields->Item["Description"]->Value);
								pNewMerchantRow->PutValue(maccMerchantID, rs->Fields->Item["MerchantID"]->Value);
								//it's fine to just add this as a legit row, as opposed to combobox text,
								//because the combo is about to become read only
								m_CCMerchantAccountCombo->AddRowSorted(pNewMerchantRow, NULL);
								m_CCMerchantAccountCombo->SetSelByColumn(maccID, nMerchantID);
							}
						}
					}
					//make the merchant dropdown read only
					m_CCMerchantAccountCombo->PutReadOnly(VARIANT_TRUE);
				}

				CString strDisplayedCCNumber = "";
				{
					//(e.lally 2007-10-30) PLID 27892 - cache full credit card number, but only display the last 4 digits
					// (a.walling 2007-10-30 18:02) - PLID 27891 - CCs are now encrypted
					// (a.walling 2010-03-15 12:27) - PLID 37751 - Use NxCrypto
					NxCryptosaur.DecryptStringFromVariant(rs->Fields->Item["SecurePAN"]->Value, AdoFldLong(rs, "KeyIndex", -1), m_strCreditCardNumber);
					// (j.jones 2015-09-30 11:04) - PLID 67175 - if we have a last 4, but not a CC number, use the last 4				
					if (m_strCreditCardNumber.IsEmpty()) {
						strDisplayedCCNumber = AdoFldString(rs, "CreditCardNumber", "");
					}
					else {
						strDisplayedCCNumber = MaskCCNumber(m_strCreditCardNumber);
					}
				}

				// (j.jones 2015-09-30 09:31) - PLID 67167 - ICCP uses a different control to show this
				if (!IsICCPEnabled()) {
					m_nxeditBankNameOrCCCardNum.SetWindowText(strDisplayedCCNumber);
				}
				else {
					m_nxeditCCLast4.SetWindowText(strDisplayedCCNumber.Right(4));
				}

				var = rs->Fields->Item["CardholdersName"]->Value;	// Credit card name
				if (var.vt != VT_NULL)
					m_nxeditAcctNumOrCCNameOnCard.SetWindowText(CString(var.bstrVal));

				var = rs->Fields->Item["CreditCardExpDate"]->Value;	// Credit card exp. date
				if (var.vt != VT_NULL) {
					dt = var.date;
					// (j.jones 2007-03-13 14:47) - PLID 25183 - ensure we do not set it
					// to an invalid date
					COleDateTime dtMin;
					dtMin.ParseDateTime("12/31/1899");
					if (dt.m_status != COleDateTime::invalid && dt >= dtMin) {
						m_nxeditCheckNumOrCCExpDate.SetWindowText(dt.Format("%m/%y"));
					}
				}

				var = rs->Fields->Item["CreditCardAuthorizationNumber"]->Value;	// Credit card auth. #
				m_nxeditBankNumOrCCAuthNum.SetWindowText(VarString(var, ""));

				// (j.jones 2015-09-30 09:18) - PLID 67163 - if this is an ICCP transaction,
				// make the fields read only
				if (IsICCPEnabled() && m_bHasICCPTransaction) {
					//card type
					m_CardNameCombo->PutReadOnly(VARIANT_TRUE);
					//Last 4
					m_nxeditCCLast4.SetReadOnly(TRUE);
					//auth num
					m_nxeditBankNumOrCCAuthNum.SetReadOnly(TRUE);
					//merchant account
					m_CCMerchantAccountCombo->PutReadOnly(VARIANT_TRUE);
				}

			}
			break;

			case EPayMethod::GiftCertificatePayment :	//Gift Certificate
				GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
				m_radioPayment.SetCheck(1);
				PostClickRadioPayment();
				m_radioGift.SetCheck(1);
				PostClickRadioGC();
				EnsureLabelArea();
				// (r.gonet 2015-05-05 10:03) - PLID 35326 - Now store the original Gift ID associated with this payment so we can compare it later
				// during saving.
				m_nOriginalGiftID = AdoFldLong(rs, "GiftID", -1);
				if (m_nOriginalGiftID != -1) {
					// (j.jones 2015-03-26 08:48) - PLID 65281 - no dropdown anymore, just store this GC ID
					m_nSelectedGiftID = m_nOriginalGiftID;

					// (j.jones 2015-03-26 08:48) - PLID 65283 - this function updates the fields on
					// the screen from the ID
					UpdateGiftCertificateControls_FromID(m_nSelectedGiftID);
				}
				break;
			case EPayMethod::Adjustment: // Adjustment
				GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
				m_radioAdjustment.SetCheck(1);
				PostClickRadioAdjustment();
				EnsureLabelArea();
				break;
			case EPayMethod::CashRefund: // Cash Refund
				GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
				m_radioRefund.SetCheck(1);
				PostClickRadioRefund();
				PostClickRadioCash();
				EnsureLabelArea();
				m_nxstaticLabelCheckNumOrCCExpDate.SetWindowText("Check Number");
				break;
			case EPayMethod::CheckRefund: // Check Refund
				GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
				m_radioRefund.SetCheck(1);
				PostClickRadioRefund();
				PostClickRadioCheck();
				EnsureLabelArea();

				var = rs->Fields->Item["CheckNumber"]->Value; // Check number
				if (var.vt != VT_NULL) {
					m_nxeditCheckNumOrCCExpDate.SetWindowText(CString(var.bstrVal));
				}
				var = rs->Fields->Item["Bank Number"]->Value; // Bank number
				if (var.vt != VT_NULL) {
					m_nxeditBankNameOrCCCardNum.SetWindowText(CString(var.bstrVal));
				}
				var = rs->Fields->Item["Checking Acct Number"]->Value; // Account number
				if (var.vt != VT_NULL) {
					m_nxeditAcctNumOrCCNameOnCard.SetWindowText(CString(var.bstrVal));
				}
				var = rs->Fields->Item["CreditCardAuthorizationNumber"]->Value;	// Credit card auth. #
				GetDlgItem(IDC_EDIT_AUTHORIZATION)->SetWindowText(VarString(var,""));
				break;
			case EPayMethod::GiftCertificateRefund: // Gift Certificate Refund
				// (r.gonet 2015-04-29 11:09) - PLID 65326 - Refunds can now be to gift certificates.
				GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
				m_radioRefund.SetCheck(1);
				PostClickRadioRefund();
				PostClickRadioGC();
				EnsureLabelArea();
				// (r.gonet 2015-05-05 10:03) - PLID 35326 - Now store the original Gift ID associated with this payment so we can compare it later
				// during saving.
				m_nOriginalGiftID = AdoFldLong(rs, "GiftID", -1);
				if (m_nOriginalGiftID != -1) {
					// (j.jones 2015-03-26 08:48) - PLID 65281 - no dropdown anymore, just store this GC ID
					m_nSelectedGiftID = m_nOriginalGiftID;

					// (j.jones 2015-03-26 08:48) - PLID 65283 - this function updates the fields on
					// the screen from the ID
					UpdateGiftCertificateControls_FromID(m_nSelectedGiftID);
				} else {
					ClearGiftCertificate();
				}
				break;
			default:
				//for old data, PayMethod will be a 0 or blank. So look at the type field.
				var = rs->Fields->Item["Type"]->Value;	// Payment method
				if (var.vt == VT_NULL) {
					MessageBox("This is an invalid payment. Please delete it from the Financial Tab.");
					rs->Close();
					return;
				}
				switch(var.lVal) {
				case 1: //payment
					m_radioPayment.SetCheck(1);
					EnablePaymentTypes();
					EnsureLabelArea();
					break;
				case 2: //adj.
					GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
					GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
					m_radioAdjustment.SetCheck(1);
					EnsureLabelArea();
					DisablePaymentTypes();
					PostClickRadioCash();
					break;
				case 3: //refund
					GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
					GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
					m_radioRefund.SetCheck(1);
					EnablePaymentTypes();
					EnsureLabelArea();
					break;
				}
			}

			//DRT 2/9/2005 - PLID 15566 - Pulled this out of the Cash section of the switch above, it's now
			//	done for all payments of any method.
			// (r.gonet 2015-04-20) - PLID 65326 - New function to get the line item type from the paymethod.
			if(GetLineItemTypeFromPayMethod(eMethod) == LineItem::Type::Payment) {
				var = rs->Fields->Item["CashReceived"]->Value;
				if(var.vt == VT_CY) {
					COleCurrency cyCashReceived = var.cyVal;
					SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(cyCashReceived,FALSE,TRUE));
					var = rs->Fields->Item["Amount"]->Value;	// Payment amount
					COleCurrency cyPaymentAmount = var.cyVal;
					COleCurrency cyChangeGiven = cyCashReceived - cyPaymentAmount;
					SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(cyChangeGiven,FALSE,TRUE));
				}
			}

			var = rs->Fields->Item["Amount"]->Value;	// Payment amount
			COleCurrency cy = var.cyVal;
			/////////////////////////////////////////////
			// Negatize adjustments and refunds
			if (IsAdjustment() || IsRefund()) {
				// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - Need to explicitly define the multiplier as a long
				cy = cy * long(-1);
				var.cyVal = cy;
			}
			str = FormatCurrencyForInterface(cy,FALSE);
			
			if (cy >= COleCurrency(0,0)) {
				GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);
				((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(0, -1);
			}
			else {
				GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);
				int nStart, nFinish;
				GetNonNegativeAmountExtent(nStart, nFinish);
				((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(nStart, nFinish);
			}

			var = rs->Fields->Item["Notes"]->Value;
			// Set description box
			GetDlgItem(IDC_EDIT_DESCRIPTION)->SetWindowText(CString(var.bstrVal));
			OnSelChosenComboDescription(m_DescriptionCombo->SetSelByColumn(0,var));
			

			// Prepayment
			var = rs->Fields->Item["PrePayment"]->Value;
			if (var.boolVal != FALSE)
				((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->SetCheck(1);
			else
				((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->SetCheck(0);

			// (a.walling 2006-11-15 12:58) - PLID 23550 - Retrieve and set the proper selection for the group and reason code
			var = rs->Fields->Item["ReasonCodeID"]->Value;
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			long nResult = m_pReasonList->TrySetSelByColumn_Deprecated(rcccID, var);
			m_nPendingReasonCodeID = VarLong(var, -1);
			if(nResult == NXDATALIST2Lib::sriNoRow && m_nPendingReasonCodeID != -1) {
				// (j.jones 2010-09-23 11:54) - PLID 27917 - handle inactive codes
				_RecordsetPtr rsAdjCode = CreateParamRecordset("SELECT Description FROM AdjustmentCodesT WHERE ID = {INT}", m_nPendingReasonCodeID);
				if(!rsAdjCode->eof) {
					m_pReasonList->PutComboBoxText(_bstr_t(AdoFldString(rsAdjCode, "Description", "")));
				}
				rsAdjCode->Close();
			}

			var = rs->Fields->Item["GroupCodeID"]->Value;
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			nResult = m_pGroupCodeList->TrySetSelByColumn_Deprecated(gcccID, var);
			m_nPendingGroupCodeID = VarLong(var, -1);
			if (nResult == NXDATALIST2Lib::sriNoRow) {
				// (j.jones 2010-09-23 11:54) - PLID 27917 - handle inactive codes
				_RecordsetPtr rsAdjCode = CreateParamRecordset("SELECT Description FROM AdjustmentCodesT WHERE ID = {INT}", m_nPendingGroupCodeID);
				if(!rsAdjCode->eof) {
					m_pGroupCodeList->PutComboBoxText(_bstr_t(AdoFldString(rsAdjCode, "Description", "")));
				}
				rsAdjCode->Close();
			}

			//////////////////////////////////////////////////////////////
			// See if this payment has been applied to anything. If so,
			// warn the user that the type and amount of the payment will
			// not be changeable.

			// (j.jones 2011-09-13 15:37) - PLID 44887 - We currently have no way to make the dialog read-only,
			// but they cannot edit original & void line items, so at least warn when they open.
			
			CString strMessage;
			BOOL bAllowChangeType = TRUE;
			CString strWarnType = "payment";
			if(IsAdjustment()) {
				strWarnType = "adjustment";
			}
			else if(IsRefund()) {
				strWarnType = "refund";
			}
			else if(IsPrePayment()) {
				strWarnType = "prepayment";
			}
			else {
				strWarnType = "payment";
			}

			if(m_varPaymentID.vt == VT_I4) {
				LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(VarLong(m_varPaymentID));
				if(licsStatus == licsOriginal) {
					strMessage.Format("This %s has been corrected, and can no longer be modified.", strWarnType);
					//TES 3/25/2015 - PLID 65436 - This is now a member variable
					m_bCanEdit = FALSE;
				}
				else if(licsStatus == licsVoid) {
					strMessage.Format("This %s is a voiding line item for an existing correction, and cannot be modified.", strWarnType);
					//TES 3/25/2015 - PLID 65436 - This is now a member variable
					m_bCanEdit = FALSE;
				}
				// (j.jones 2011-09-14 09:55) - PLID 44802 - corrected items have to stay the same type as the original, the type can never be changed
				else if(licsStatus == licsCorrected) {
					bAllowChangeType = FALSE;
				}
			}

			// (j.jones 2007-04-19 15:38) - PLID 25711 - moved these queries to be inside the load

			// (j.gruber 2007-07-11 10:20) - PLID 15416 - check to see if this payment is already authorized
			// (d.thompson 2009-07-01) - PLID 34687 - Cleaned up for QBMS
			// (j.jones 2015-09-30 10:12) - PLID 67168 - split HasTransaction into HasAnyCCTransaction, HasPreICCPTransaction, HasICCPTransaction
			if (m_bHasAnyTransaction) {

				m_bAllowSwipe = FALSE;

				// (j.jones 2011-09-13 15:37) - PLID 44887 - we might not be able to edit as-is,
				// so don't overwrite the message if this is already false
				if (m_bCanEdit) {
					if (AdoFldBool(rs, "IsApproved")) {					
						//Tips are disabled too, if they have nexspa.
						CString strSpa;
						if(m_bHasNexSpa) {
							strSpa = "tips, ";
						}
						strMessage.Format("This %s has been processed.\nThe type, amount, date, %sand credit card information of the %s cannot be changed.",
							IsRefund() ? "refund" : "payment", strSpa, IsRefund() ? "refund" : "payment");
						
					}
					else {
						//the transaction is there, but hasn't been processed, the only way for this to happen is if there was an error and they said they wanted to process it later.
						strMessage = "This payment is pending approval, but hasn't been processed yet.  Please process this from the Financial module's Credit Cards tab.\nThe type, amount, date and credit card information of the payment cannot be changed.";
					}
				}

				//TES 3/25/2015 - PLID 65436 - This is now a member variable
				m_bCanEdit = FALSE;
			}
			else {
				//we don't care about this because it doesn't have anything to do with CC processing
			}

			// (j.jones 2011-09-14 09:55) - PLID 44802 - if the type cannot be changed, disable only the radio buttons
			if(!bAllowChangeType) {
				m_radioPayment.EnableWindow(FALSE);
				m_radioAdjustment.EnableWindow(FALSE);
				m_radioRefund.EnableWindow(FALSE);
			}

			if (!m_bCanEdit) {
				MsgBox(strMessage);
				//first the payment boxes
				m_radioPayment.EnableWindow(FALSE);
				m_radioAdjustment.EnableWindow(FALSE);
				m_radioRefund.EnableWindow(FALSE);
				//now the type
				m_radioCash.EnableWindow(FALSE);
				m_radioCheck.EnableWindow(FALSE);
				m_radioCharge.EnableWindow(FALSE);
				m_radioGift.EnableWindow(FALSE);

				//now the amount
				GetDlgItem(IDC_EDIT_TOTAL)->EnableWindow(FALSE);
				//don't forget the percent button!!
				GetDlgItem(IDC_CALC_PERCENT)->EnableWindow(FALSE);

				//date
				GetDlgItem(IDC_PAY_DATE)->EnableWindow(FALSE);

				// (d.thompson 2009-07-01) - PLID 34687 - The tips need disabled as well, they are part of the transacted amount.
				GetDlgItem(IDC_ADD_TIP_BTN)->EnableWindow(FALSE);
				m_pTipList->ReadOnly = VARIANT_TRUE;

				//credit card information
				m_nxeditCheckNumOrCCExpDate.SetReadOnly(TRUE);
				m_nxeditBankNameOrCCCardNum.SetReadOnly(TRUE);
				m_nxeditAcctNumOrCCNameOnCard.SetReadOnly(TRUE);
				m_nxeditBankNumOrCCAuthNum.SetReadOnly(TRUE);
				GetDlgItem(IDC_COMBO_CARD_NAME)->EnableWindow(FALSE);

				// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
				m_radioCCSwipe.EnableWindow(FALSE);
				m_radioCCCardNotPresent.EnableWindow(FALSE);
				m_radioCCDoNotProcess.EnableWindow(FALSE);
				// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
				m_radioCCRefundToOriginalCard.EnableWindow(FALSE);

				// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct. and zipcode
				m_CCMerchantAccountCombo->PutEnabled(VARIANT_FALSE);
				m_nxeditCCZipcode.SetReadOnly(TRUE);

				// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
				m_CCCardOnFileCombo->PutEnabled(VARIANT_FALSE);

				// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
				m_checkAddCCToProfile.EnableWindow(FALSE);

				// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
				m_nxeditCCLast4.SetReadOnly(TRUE);

				// (d.thompson 2009-06-30) - PLID 34687 - Hide the process cc transactions button
				GetDlgItem(IDC_PROCESS_CC)->ShowWindow(SW_HIDE);

				GetDlgItem(IDC_BTN_PREVIEW)->EnableWindow(TRUE);
				GetDlgItem(IDC_BTN_PRINT)->EnableWindow(TRUE);
			}
			else {

				//we don't want to warn them twice, the CC stuff also greys out the applied stuff
				if(AdoFldLong(rs, "IsSourceApply",0) != 0) {
					m_radioPayment.EnableWindow(FALSE);//.SetEnabled(FALSE);
					m_radioAdjustment.EnableWindow(FALSE);//.SetEnabled(FALSE);
					m_radioRefund.EnableWindow(FALSE);//.SetEnabled(FALSE);
					GetDlgItem(IDC_CHECK_PREPAYMENT)->EnableWindow(FALSE);//bad cast
					GetDlgItem(IDC_COMBO_INSURANCE)->EnableWindow(FALSE);
					GetDlgItem(IDC_EDIT_TOTAL)->EnableWindow(FALSE);
					GetDlgItem(IDC_CALC_PERCENT)->EnableWindow(FALSE);
					// (j.jones 2015-04-23 08:59) - PLID 65281 - payment methods are not disabled
					// when editing applied payments, so neither should the gift certificate selection
					/*
					if (m_radioGift.GetCheck()) {
						GetDlgItem(IDC_SEARCH_GC_LIST)->EnableWindow(FALSE);
						// (j.jones 2015-06-17 15:37) - PLID 65285 - disable the GC X button
						GetDlgItem(IDC_BTN_CLEAR_GC)->EnableWindow(FALSE);
					}
					*/
					
					// (j.jones 2008-11-12 10:02) - PLID 28818 - reword this text to be context sensitive
					CString strWarn = "This payment has been applied to at least one line item.\nThe type and amount of this payment may not be changed.";
					if(IsAdjustment()) {
						strWarn.Replace("payment", "adjustment");
					}
					else if(IsRefund()) {
						strWarn.Replace("payment", "refund");
					}
					else if(IsPrePayment()) {
						strWarn.Replace("payment", "prepayment");
					}
					AfxMessageBox(strWarn);
				}
				//TES 3/1/2004: Also, let's behave the same way if this payment has been applied to.
				else if(AdoFldLong(rs, "IsDestApply",0) != 0) {
					m_radioPayment.EnableWindow(FALSE);//.SetEnabled(FALSE);
					m_radioAdjustment.EnableWindow(FALSE);//.SetEnabled(FALSE);
					m_radioRefund.EnableWindow(FALSE);//.SetEnabled(FALSE);
					GetDlgItem(IDC_CHECK_PREPAYMENT)->EnableWindow(FALSE);//bad cast
					GetDlgItem(IDC_COMBO_INSURANCE)->EnableWindow(FALSE);
					GetDlgItem(IDC_EDIT_TOTAL)->EnableWindow(FALSE);
					GetDlgItem(IDC_CALC_PERCENT)->EnableWindow(FALSE);
					if (m_radioGift.GetCheck()) {
						// (j.jones 2015-03-23 13:13) - PLID 65281 - converted the gift certificate dropdown
						// into a searchable list
						GetDlgItem(IDC_SEARCH_GC_LIST)->EnableWindow(FALSE);
						// (j.jones 2015-06-17 15:37) - PLID 65285 - disable the GC X button
						GetDlgItem(IDC_BTN_CLEAR_GC)->EnableWindow(FALSE);
					}

					// (j.jones 2008-11-12 10:02) - PLID 28818 - reword this text to be context sensitive
					CString strWarn = "This payment has at least one line item applied to it.\nThe type and amount of this payment may not be changed.";
					if(IsAdjustment()) {
						strWarn.Replace("payment", "adjustment");
					}
					else if(IsRefund()) {
						strWarn.Replace("payment", "refund");
					}
					else if(IsPrePayment()) {
						strWarn.Replace("payment", "prepayment");
					}
					AfxMessageBox(strWarn);
				}
			}
			
			rs->Close();

			//TES 3/25/2015 - PLID 65175 - Moved from OnInitDialog(), check whether there are any tips
			CString strTipCheckSQL;
			strTipCheckSQL.Format("SELECT * FROM PaymentTipsT WHERE PaymentID = %li", VarLong(m_varPaymentID));
			// (z.manning 2008-12-10 15:18) - PLID 32397 - Fixed IsRecordsetEmpty call to prevent text formatting errors
			bTipExists = !IsRecordsetEmpty("%s", strTipCheckSQL);
		}

		//TES 3/25/2015 - PLID 65175 - Moved the code for initializing tips here from OnInitDialog()
		if (m_bHasNexSpa) {
			//preference to always have tips enabled
			if (bTipExists ||
				(GetRemotePropertyInt("ShowPaymentTips", 0, 0, GetCurrentUserName(), true) &&
				(IsPayment() || (IsRefund() && GetRemotePropertyInt("AllowTipRefunds", 0, 0, "<None>") && (GetCurrentUserPermissions(bioRefundTips, 0) & sptWrite))))) {

				m_bTipExtended = false;	//this forces it to draw
			}
			else {
				m_bTipExtended = true;	//this forces it to shrink
			}

			//handles requerying the GC datalist
			OnGcIncludeOthers();
		}
		else {
			//they don't have NexSpa, make sure they can't use it.
			m_bTipExtended = true;
			GetDlgItem(IDC_SHOW_TIPS_BTN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SHOW_TIPS_BTN)->EnableWindow(FALSE);

			//also the "gift" radio button must be hidden
			GetDlgItem(IDC_RADIO_GIFT_CERT)->ShowWindow(SW_HIDE);
		}

		//handle the sizing for whatever we just did above
		OnShowTipsBtn();

	// (b.spivey, October 01, 2012) - PLID 50949 - Loading the window name.
	UpdateWindowText(); 

	}
	NxCatchAll("Error in OnShowWindow");

	/* Highlight payment amount */
	if (GetDlgItem(IDC_EDIT_TOTAL)->IsWindowEnabled()) {
		GetDlgItem(IDC_EDIT_TOTAL)->SetFocus();
		int nStart, nFinish;
		GetNonNegativeAmountExtent(nStart, nFinish);
		((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(nStart, nFinish);
	}
	else
		GetDlgItem(IDC_COMBO_DESCRIPTION)->SetFocus();

	//store the current amount
	GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(str);
	m_cyLastAmount = ParseCurrencyFromInterface(str);

	m_bAmountChanged = FALSE;
}


void CPaymentDlg::OnChangeRadioPayment() 
{
	try {

		if (!m_radioPayment.GetCheck()) {
			GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);
			((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->SetCheck(0);//bad cast
		}
		else {
			GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_SHOW);
		}

	}NxCatchAll("Error in CPaymentDlg::OnChangeRadioPayment");
}

void CPaymentDlg::OnClickRadioCash()
{
	try {
		// (c.haag 2010-09-20 15:34) - PLID 35723 - Warn the user if we're switching from a CC
		// method of payment and a transaction took place
		if (!WarnOfExistingCCTransaction()) {
			return;
		}
		OnRadioCashUpdated();
	}
	NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnRadioCashUpdated() 
{
	try {
		// (j.jones 2007-04-19 15:07) - PLID PLID 25711 - all window changes (show/hide, etc.)
		// have been moved to PostClickRadioCash()
		PostClickRadioCash();
	}
	NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnClickRadioCharge()
{
	try {
		OnRadioChargeUpdated();
	}
	NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnRadioChargeUpdated() 
{
	
	try{
		// (a.walling 2007-08-03 15:55) - PLID 26922 - If they have CC Processing, but the user lacks permission,
		// then we prevent them from creating a charge/card payment/refund.
		// (d.thompson 2010-09-02) - PLID 40371 - Any license type satisfies
		// (j.jones 2015-09-30 08:58) - PLID 67157 - if you have ICCP, you are allowed to make a credit
		// card payment, you just won't be able to process it
		if (g_pLicense && g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent) && !IsICCPEnabled()) {
			// they are licensed, check permission silently (will prompt for PW when saving)
			if (!CheckCurrentUserPermissions(bioCCProcess, sptWrite, FALSE, 0, TRUE, TRUE)) {
				// they don't have permission!
				MessageBox("You do not have permission to process credit cards. Please check with your office manager.");

				// need to set the focus and selection back to the previous, curses.
				if (m_pCurPayMethod) {
					m_radioCharge.SetCheck(FALSE);
					m_pCurPayMethod->SetCheck(TRUE);
					m_pCurPayMethod->SetFocus();
				} else {
					ThrowNxException("Error resetting pay method in CPaymentDlg::OnRadioChargeUpdated");
				}

				return;
			}
		}

		// (j.gruber 2007-07-30 14:51) - PLID 26704 - pop up the apply to dialog
		// (a.walling 2007-08-03 15:35) - PLID 26899 - This does not pop up if m_bProcessCreditCards is false
		// by design, even though the ApplyToRefund dialog will popup if you are initially creating a refund
		// (j.jones 2015-09-30 10:33) - PLID 67173 - do not do this if ICCP is enabled
		// (j.jones 2015-09-30 10:37) - PLID 67172 - the payment to apply to is now a struct
		if (m_bProcessCreditCards_NonICCP && m_PaymentToApplyTo.nPaymentID == -1 && m_radioRefund.GetCheck() && !IsICCPEnabled()) {
			CApplyToRefundDlg dlg(this);
			dlg.m_nPatientID = m_PatientID; // (a.walling 2008-07-07 17:14) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
			dlg.DoModal();
			// (j.jones 2015-09-30 10:37) - PLID 67172 - renamed this function
			// (j.jones 2015-09-30 10:34) - PLID 67171 - ApplyToRefund would already have warned about
			// mismatched credit cards, so we do not need to
			SetPaymentToApplyTo(dlg.m_nPayID, false);
		}

		// (j.jones 2007-04-19 15:07) - PLID PLID 25711 - all window changes (show/hide, etc.)
		// have been moved to PostClickRadioCharge()
		PostClickRadioCharge();
	
		// (a.walling 2007-08-03 16:12) - PLID 26922 - moved the try block to the top of the function

		//if the option is enabled, auto-fill the credit card information based on the patient's last credit payment
		if(GetChargeInfo != 0) {

			// (j.jones 2006-04-27 09:07) - PLID 20314 - we cache the last-charge info incase this
			// function is called multiple times			
			
			if(!m_bLastCreditCardInfoLoaded) {
			
				// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.
				// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
				_RecordsetPtr rsCreditPayments = CreateParamRecordset("SELECT TOP 1 CreditCardID, CardName "
				"FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID INNER JOIN PaymentPlansT "
				"ON PaymentsT.ID = PaymentPlansT.ID "
				"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				"WHERE (LineItemT.PatientID = {INT} AND Deleted = 0 "
				"AND PayMethod = {CONST_INT}) ORDER BY Date DESC, PaymentsT.ID DESC", m_PatientID, (long)EPayMethod::ChargePayment);
				if(!rsCreditPayments->eof){
					// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.
					// if the patient has previous credit card payments, set the info used with with the most recent payment
					m_nLastCCTypeID = AdoFldLong(rsCreditPayments, "CreditCardID", -1);					
				}
				rsCreditPayments->Close();

				m_bLastCreditCardInfoLoaded = TRUE;	
			}

			//now set the fields based on the cached values
			SetCreditCardInfoForType(m_nLastCCTypeID);
		}
	}NxCatchAll("Error in CPaymentDlg::OnRadioChargeUpdated()");
}

void CPaymentDlg::OnClickRadioCheck()
{
	try {
		// (c.haag 2010-09-20 15:34) - PLID 35723 - Warn the user if we're switching from a CC
		// method of payment and a transaction took place
		if (!WarnOfExistingCCTransaction()) {
			return;
		}
		OnRadioCheckUpdated();
	}
	NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnRadioCheckUpdated() 
{
	try {
		// (j.jones 2007-04-19 15:07) - PLID PLID 25711 - all window changes (show/hide, etc.)
		// have been moved to PostClickRadioCheck()
		PostClickRadioCheck();
	}NxCatchAll("Error inCPaymentDlg::OnRadioCheckUpdated()");

	try {

		//auto-fill the bank info based on the patient's last check payment

		// (j.jones 2006-04-27 09:04) - PLID 20314 - we cache the last-check info incase this
		// function is called multiple times
		if(!m_bLastCheckInfoLoaded) {
			// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
			_RecordsetPtr rsCheckPayments = CreateParamRecordset("SELECT TOP 1 BankNo, CheckAcctNo, BankRoutingNum FROM PaymentsT INNER JOIN LineItemT "
				"ON PaymentsT.ID = LineItemT.ID INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				"WHERE (LineItemT.PatientID = {INT} AND Deleted = 0 AND PayMethod = {CONST_INT}) ORDER BY Date DESC, PaymentsT.ID DESC", m_PatientID, (long)EPayMethod::CheckPayment);
			if(!rsCheckPayments->eof) {
				FieldsPtr pflds = rsCheckPayments->GetFields();
				m_strLastBankNo = AdoFldString(pflds, "BankNo", "");
				m_strLastCheckAcctNo = AdoFldString(pflds, "CheckAcctNo", "");
				m_strLastRoutingNo = AdoFldString(pflds, "BankRoutingNum", "");
			}
			rsCheckPayments->Close();

			m_bLastCheckInfoLoaded = TRUE;
		}

		//now set the fields based on the cached values
		m_nxeditBankNameOrCCCardNum.SetWindowText(m_strLastBankNo);
		m_nxeditAcctNumOrCCNameOnCard.SetWindowText(m_strLastCheckAcctNo);
		m_nxeditBankNumOrCCAuthNum.SetWindowText(m_strLastRoutingNo);

	}NxCatchAll("Error inCPaymentDlg::OnRadioCheckUpdated()");
}

void CPaymentDlg::OnClickRadioPayment() 
{
	try {
		// (c.haag 2010-10-12 11:05) - PLID 35723 - We could get here by pressing tab; the button
		// may not actually be pressed.
		if (!IsDlgButtonChecked(IDC_RADIO_PAYMENT))
			return;
	
		// (j.jones 2007-02-26 11:46) - PLID 24927 - if we specified that the payment type cannot
		// change, then tell the user they cannot do so
		if(m_boIsNewPayment && m_bForceDefaultPaymentType && m_iDefaultPaymentType != 0) {

			CString strWarning = "The line item type cannot be changed for this transation.";

			if(m_bIsReturnedProductAdj || m_bIsReturnedProductRefund) {
				//if we're stopping because it's going to be a Returned Product record, say so
				strWarning = "Because this is part of a Returned Product transaction, the line item type cannot be changed.";
			}

			AfxMessageBox(strWarning);

			CheckDlgButton(IDC_RADIO_PAYMENT,FALSE);

			// Set default payment style
			switch (m_iDefaultPaymentType) {
			case 1:
				CheckDlgButton(IDC_RADIO_ADJUSTMENT,TRUE);
				break;
			case 2:
				CheckDlgButton(IDC_RADIO_REFUND,TRUE);
				break;
			}
			return;
		}

		// (j.jones 2007-02-26 10:18) - PLID 24927 - if a Returned Product adjustment or refund, do not allow the type to change
		if(m_varPaymentID.vt == VT_I4 && VarLong(m_varPaymentID,-1) != -1) {
			// (j.jones 2007-04-19 14:41) - PLID 25711 - converted to use loaded booleans, instead of recordsets
			if(m_bIsReturnedProductRefund) {
				AfxMessageBox("This refund is linked to a Returned Product record. The type of line item cannot be changed.");
				CheckDlgButton(IDC_RADIO_PAYMENT,FALSE);
				CheckDlgButton(IDC_RADIO_REFUND,TRUE);
				return;
			}
			if(m_bIsReturnedProductAdj) {
				AfxMessageBox("This adjustment is linked to a Returned Product record. The type of line item cannot be changed.");
				CheckDlgButton(IDC_RADIO_PAYMENT,FALSE);		
				CheckDlgButton(IDC_RADIO_ADJUSTMENT,TRUE);
				return;
			}
		}

		// (j.jones 2007-04-19 14:13) - PLID 25711 - all window changes (show/hide, etc.)
		// have been moved to PostClickRadioPayment()
		PostClickRadioPayment();

		// (j.jones 2008-07-10 12:43) - PLID 28756 - set our default category and description
		TrySetDefaultInsuranceDescriptions();
	
		// (b.spivey, October 01, 2012) - PLID 50949 - payment
		UpdateWindowText(); 
	}NxCatchAll("Error in CPaymentDlg::OnClickRadioPayment()");
}

void CPaymentDlg::OnClickRadioAdjustment() 
{
	try {

		// (j.jones 2007-02-26 11:46) - PLID 24927 - if we specified that the payment type cannot
		// change, then tell the user they cannot do so
		if(m_boIsNewPayment && m_bForceDefaultPaymentType && m_iDefaultPaymentType != 1) {

			CString strWarning = "The line item type cannot be changed for this transation.";

			if(m_bIsReturnedProductAdj || m_bIsReturnedProductRefund) {
				//if we're stopping because it's going to be a Returned Product record, say so
				strWarning = "Because this is part of a Returned Product transaction, the line item type cannot be changed.";
			}

			AfxMessageBox(strWarning);

			CheckDlgButton(IDC_RADIO_ADJUSTMENT,FALSE);

			// Set default payment style
			switch (m_iDefaultPaymentType) {
			case 2:
				CheckDlgButton(IDC_RADIO_REFUND,TRUE);
				break;
			default:
				CheckDlgButton(IDC_RADIO_PAYMENT,TRUE);
				break;
			}
			return;
		}

		// (j.jones 2007-02-26 10:18) - PLID 24927 - if a Returned Product refund, do not allow the type to change
		// (j.jones 2007-04-19 14:41) - PLID 25711 - converted to use loaded booleans, instead of recordsets
		if(m_varPaymentID.vt == VT_I4 && VarLong(m_varPaymentID,-1) != -1 && m_bIsReturnedProductRefund) {
			AfxMessageBox("This refund is linked to a Returned Product record. The type of line item cannot be changed.");
			CheckDlgButton(IDC_RADIO_ADJUSTMENT,FALSE);
			CheckDlgButton(IDC_RADIO_REFUND,TRUE);		
			return;
		}

		if(m_QuoteID != -1 && !((CButton *)GetDlgItem(IDC_RADIO_PAYMENT))->GetCheck()) {

			//they are changing a linked prepayment to no longer be a payment at all
			if(IDNO == MessageBox("This PrePayment is linked to a quote.\n"
				"If you change the payment type to be an Adjustment, this payment will not remain linked to the quote.\n"
				"Are you sure you wish to make this change?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {

				CheckDlgButton(IDC_RADIO_ADJUSTMENT,FALSE);
				CheckDlgButton(IDC_RADIO_PAYMENT,TRUE);
				CheckDlgButton(IDC_CHECK_PREPAYMENT,TRUE);
				return;
			}
		}

		//If they have tips and are trying to change to an adjustment, we should warn them
		if(m_pTipList->GetRowCount() > 0) {
			MsgBox("You have tips assigned to this payment.  If you continue to change to an adjustment, these tips will be deleted.");
		}

		// (j.jones 2007-04-19 14:13) - PLID 25711 - all window changes (show/hide, etc.)
		// have been moved to PostClickRadioAdjustment()
		PostClickRadioAdjustment();

		// (j.jones 2008-07-10 12:43) - PLID 28756 - set our default category and description
		TrySetDefaultInsuranceDescriptions();

		// (b.spivey, October 01, 2012) - PLID 50949 - adjustment
		UpdateWindowText(); 
	}NxCatchAll("Error in CPaymentDlg::OnClickRadioAdjustment()");
}

void CPaymentDlg::OnClickRadioRefund() 
{
	try {

		// (j.jones 2007-02-26 11:46) - PLID 24927 - if we specified that the payment type cannot
		// change, then tell the user they cannot do so
		if(m_boIsNewPayment && m_bForceDefaultPaymentType && m_iDefaultPaymentType != 2) {

			CString strWarning = "The line item type cannot be changed for this transation.";

			if(m_bIsReturnedProductAdj || m_bIsReturnedProductRefund) {
				//if we're stopping because it's going to be a Returned Product record, say so
				strWarning = "Because this is part of a Returned Product transaction, the line item type cannot be changed.";
			}

			AfxMessageBox(strWarning);

			CheckDlgButton(IDC_RADIO_REFUND,FALSE);

			// Set default payment style
			switch (m_iDefaultPaymentType) {
			case 1:
				CheckDlgButton(IDC_RADIO_ADJUSTMENT,TRUE);
				break;
			default:
				CheckDlgButton(IDC_RADIO_PAYMENT,TRUE);
				break;
			}
			return;
		}

		// (j.gruber 2007-07-30 14:51) - PLID 26704 - pop up the apply to dialog
		// (a.walling 2007-08-03 15:35) - PLID 26899 - This does not pop up if m_bProcessCreditCards is false
		// by design, even though the ApplyToRefund dialog will popup if you are initially creating a refund
		// (j.jones 2015-09-30 10:33) - PLID 67173 - do not do this if ICCP is enabled
		// (j.jones 2015-09-30 10:37) - PLID 67172 - the payment to apply to is now a struct
		if (m_bProcessCreditCards_NonICCP && m_PaymentToApplyTo.nPaymentID == -1 && m_radioCharge.GetCheck() && !IsICCPEnabled()) {
			CApplyToRefundDlg dlg(this);
			dlg.m_nPatientID = m_PatientID; // (a.walling 2008-07-07 17:14) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
			dlg.DoModal();
			// (j.jones 2015-09-30 10:37) - PLID 67172 - renamed this function
			// (j.jones 2015-09-30 10:34) - PLID 67171 - ApplyToRefund would already have warned about
			// mismatched credit cards, so we do not need to
			SetPaymentToApplyTo(dlg.m_nPayID, false);
		}


		// (j.jones 2007-02-26 10:18) - PLID 24927 - if a Returned Product adjustment, do not allow the type to change
		// (j.jones 2007-04-19 14:41) - PLID 25711 - converted to use loaded booleans, instead of recordsets
		if(m_varPaymentID.vt == VT_I4 && VarLong(m_varPaymentID,-1) != -1 && m_bIsReturnedProductAdj) {
			AfxMessageBox("This adjustment is linked to a Returned Product record. The type of line item cannot be changed.");
			CheckDlgButton(IDC_RADIO_REFUND,FALSE);			
			CheckDlgButton(IDC_RADIO_ADJUSTMENT,TRUE);
			return;
		}

		if(m_QuoteID != -1 && !((CButton *)GetDlgItem(IDC_RADIO_PAYMENT))->GetCheck()) {

			//they are changing a linked prepayment to no longer be a payment at all
			if(IDNO == MessageBox("This PrePayment is linked to a quote.\n"
				"If you change the payment type to be a Refund, this payment will not remain linked to the quote.\n"
				"Are you sure you wish to make this change?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {

				CheckDlgButton(IDC_RADIO_REFUND,FALSE);			
				CheckDlgButton(IDC_RADIO_PAYMENT,TRUE);
				CheckDlgButton(IDC_CHECK_PREPAYMENT,TRUE);
				return;
			}
		}

		//If they have tips and are trying to change to a refund, we should warn them
		//TES 4/2/2015 - PLID 65174 - If they have the permission and preference to allow refunds on tips, then don't warn them
		if (m_pTipList->GetRowCount() > 0 && (!GetRemotePropertyInt("AllowTipRefunds", 0, 0, "<None>") || !(GetCurrentUserPermissions(bioRefundTips) & sptWrite))) {
			MsgBox("You have tips assigned to this payment.  If you continue to change to a refund, these tips will be deleted.");
		}

		// (j.jones 2007-04-19 14:13) - PLID 25711 - all window changes (show/hide, etc.)
		// have been moved to PostClickRadioRefund()
		PostClickRadioRefund();

		// (b.spivey, October 01, 2012) - PLID 50949 - refund
		UpdateWindowText(); 

	}NxCatchAll("Error in CPaymentDlg::OnClickRadioRefund()");
}

void CPaymentDlg::EnablePaymentTypes()
{
	//////////////////////////////////////////////////////////////
	// Enable the ability to select the payment type
	m_radioCash.EnableWindow(TRUE);
	m_radioCheck.EnableWindow(TRUE);
	m_radioCharge.EnableWindow(TRUE);
	// (r.gonet 2015-04-20) - PLID 65326 - Allow gift certificates to be used as refund payment types.
	if(m_bHasNexSpa)
		m_radioGift.EnableWindow(TRUE);
}

void CPaymentDlg::DisablePaymentTypes()
{
	// Disable the ability to select the payment type
	m_radioCash.EnableWindow(FALSE);
	m_radioCheck.EnableWindow(FALSE);
	m_radioCharge.EnableWindow(FALSE);
	if(m_bHasNexSpa)
		m_radioGift.EnableWindow(FALSE);
}


BOOL CPaymentDlg::IsPayment()
{
	if (m_radioPayment.GetCheck())
		return TRUE;
	return FALSE;
}

BOOL CPaymentDlg::IsAdjustment()
{
	if (m_radioAdjustment.GetCheck())
		return TRUE;
	return FALSE;
}

BOOL CPaymentDlg::IsRefund()
{
	if (m_radioRefund.GetCheck())
		return TRUE;
	return FALSE;
}

BOOL CPaymentDlg::IsPrePayment()
{
	if (m_radioPayment.GetCheck() &&
		((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->GetCheck())
		return TRUE;
	return FALSE;
}

void CPaymentDlg::OnUpdateEditCheckNumOrCCExpDate()
{
	try {
		if (m_radioCharge.GetCheck() != FALSE) {
			CString str;
			m_nxeditCheckNumOrCCExpDate.GetWindowText(str);
			FormatItemText(GetDlgItem(IDC_EDIT_CHECKNUM_OR_CCEXPDATE), str, "##/##nn");
		}
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2016-02-15 11:32) - PLID 68258
LineItem::CCProcessType CPaymentDlg::GetCCProcessType()
{
	CString strAuditNewValue;
	return GetCCProcessType(strAuditNewValue);
}

// (z.manning 2016-02-15 11:32) - PLID 68258 - Moved this code to its own function
LineItem::CCProcessType CPaymentDlg::GetCCProcessType(OUT CString &strCCProcessTypeNewAuditValue)
{
	LineItem::CCProcessType eCCProcessType = LineItem::CCProcessType::None;
	strCCProcessTypeNewAuditValue = "";

	if ((IsPayment() || IsRefund()) && m_radioCharge.GetCheck())
	{
		//if not ICCP, just default to Other (Do not process)
		if (!IsICCPEnabled()) {
			eCCProcessType = LineItem::CCProcessType::DoNotProcess;
		}
		else {
			if (m_radioCCSwipe.GetCheck()) {
				eCCProcessType = LineItem::CCProcessType::Swipe;
				strCCProcessTypeNewAuditValue = "Swipe / Dip Card";
			}
			else if (m_radioCCCardNotPresent.GetCheck()) {
				eCCProcessType = LineItem::CCProcessType::CardNotPresent;
				strCCProcessTypeNewAuditValue = "Card Not Present";
			}
			else if (m_radioCCDoNotProcess.GetCheck()) {
				eCCProcessType = LineItem::CCProcessType::DoNotProcess;
				strCCProcessTypeNewAuditValue = "Other (Do not process)";
			}
			else if (IsRefund() && m_radioCCRefundToOriginalCard.GetCheck()) {
				eCCProcessType = LineItem::CCProcessType::RefundToOriginal;
				strCCProcessTypeNewAuditValue = "Refund To Original Card";
			}
		}
	}

	return eCCProcessType;
}

// (d.thompson 2009-07-01) - PLID 34687 - Be aware that during credit card transactions, failures can happen
//	that force the entire dialog to shut down.  All callers of this function need to be aware of that possibility.
BOOL CPaymentDlg::SaveChanges(IN OUT CPaymentSaveInfo* pSaveInfo /*=NULL*/)
{
	CWaitCursor pWait;

	// (j.jones 2011-09-13 15:37) - PLID 44887 - Disallow editing original & void line items.
	// As with other permissions (which are errantly further down in this function than they should be),
	// if you cannot edit a payment, we currently have no way to make the dialog read-only, so we are
	// forced to perform the check here. It's not ideal, but it has always been this way.

	CString strWarnType = "payment";
	if(IsAdjustment()) {
		strWarnType = "adjustment";
	}
	else if(IsRefund()) {
		strWarnType = "refund";
	}
	else if(IsPrePayment()) {
		strWarnType = "prepayment";
	}
	else {
		strWarnType = "payment";
	}

	if(m_varPaymentID.vt == VT_I4) {
		LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(VarLong(m_varPaymentID));
		if(licsStatus == licsOriginal) {
			CString strWarn;
			strWarn.Format("This %s has been corrected, and can no longer be modified.", strWarnType);
			AfxMessageBox(strWarn);
			return FALSE;
		}
		else if(licsStatus == licsVoid) {
			CString strWarn;
			strWarn.Format("This %s is a voiding line item for an existing correction, and cannot be modified.", strWarnType);
			AfxMessageBox(strWarn);
			return FALSE;
		}
	}


	//make sure they selected a valid method
	if(IsPayment() || IsRefund()) {
		if (!m_radioCash.GetCheck() && !m_radioCheck.GetCheck() && !m_radioCharge.GetCheck() && !m_radioGift.GetCheck()) {
			AfxMessageBox("Please select a valid payment/refund method.");
			return FALSE;
		}
	}

	// (a.walling 2007-09-20 16:00) - PLID 27468 - Structure to hold information about saved payments / tips.
	CPaymentSaveInfo Info;

	// (c.haag 2010-10-12 11:25) - PLID 35723 - Final warning if they try to save a non-charge item applied
	// to another item with a charge transaction.
	// (j.jones 2015-09-30 10:37) - PLID 67172 - the payment to apply to is now a struct
	if (m_pCurPayMethod != &m_radioCharge && m_PaymentToApplyTo.bHasAnyCCTransaction) 
	{
		CString strWarn;
		strWarn.Format("This %s is associated with another item that has undergone credit card processing; but "
			"the method for this %s is not 'Charge'.\n\n"
			"Do you wish to CANCEL saving this %s now?", strWarnType, strWarnType, strWarnType);
		if (IDYES == AfxMessageBox(strWarn, MB_YESNO | MB_ICONWARNING))
		{
			return FALSE;
		}
	}

	BOOL bIsNew = FALSE;

	COleVariant var;
	CString strAmount, strDate, strDescription, strProviderID, strPrePayment, strGiftID = "NULL";

	// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
	EPayMethod eMethod = EPayMethod::Invalid;
	int iPaymentGroupID = 0;
	long LocationID;
	//TES 4/9/2008 - PLID 29585 - Track the location name for auditing.
	CString strLocationName;
	
	long ProviderID = -1;
	//TES 7/10/2008 - PLID 30676 - Track the provider name for auditing.
	CString strProviderName;

	// Ensure that, if linked to a quote, that this is a PrePayment
	if(m_QuoteID != -1) {

		if(!((CButton *)GetDlgItem(IDC_RADIO_PAYMENT))->GetCheck() ||
			!((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->GetCheck()) {

			//they are changing a linked prepayment to no longer be a payment at all
			if(IDNO == MessageBox("This PrePayment is linked to a quote.\n"
				"If you change the payment type or PrePayment status, this payment will not remain linked to the quote.\n"
				"Are you sure you wish save these changes?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}
			else {
				m_QuoteID = -1;
			}
		}
	}

	// Find provider ID
	if(m_ProviderCombo->GetCurSel()!=-1) {
		var = m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(),0);
		if(var.vt != VT_NULL) {
			ProviderID = var.lVal;
			strProviderName = VarString(m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,1));
		}
	}

	if (ProviderID != -1) {
		strProviderID.Format("%li", ProviderID);
	}
	else if(!m_ProviderCombo->IsComboBoxTextInUse) {
		//warn them before saving
		if(m_boIsNewPayment) {

			CString strType = "payment";
			if (m_radioPayment.GetCheck()) { strType = "payment"; }
			else if (m_radioAdjustment.GetCheck()) { strType = "adjustment"; }
			else if (m_radioRefund.GetCheck()) { strType = "refund"; }

			// (j.jones 2011-07-08 17:13) - PLID 44497 - added pref. to force selection of a provider
			if(GetRemotePropertyInt("RequireProviderOnPayments", 1, 0, "<None>", true) == 1) {
				CString strWarn;
				strWarn.Format("You have not selected a provider for this %s.\n"
					"All new %ss must have providers selected.", strType, strType);

				MessageBox(strWarn,"Practice",MB_ICONEXCLAMATION|MB_OK);
				return FALSE;
			}
			else {
				CString strWarn;
				strWarn.Format("You have not selected a provider for this %s.\n"
					"Are you sure you wish to save this %s without a provider?", strType, strType);

				if(IDNO == MessageBox(strWarn,"Practice",MB_ICONQUESTION|MB_YESNO)) {
					return FALSE;
				}
			}
		}

		strProviderID = "Null";
		strProviderName = "{No Provider}";
	}

	//DRT 2/15/2005 - PLID 15641 - Preference to warn if no cash drawer is selected.
	//NexSpa licensed + nothing is selected + preference is set
	if(m_bHasNexSpa && GetRemotePropertyInt("WarnCashDrawerEmpty", 0, 0, "<None>", true)) {
		//They want to be warned if no drawer selected.
		long nCurSel = m_pDrawerList->GetCurSel();
		long nValue = -1;
		if(nCurSel != sriNoRow) {
			//Nothing is selected if there is no row selected in the datalist, or if the id of the row selected is -1
			nValue = VarLong(m_pDrawerList->GetValue(nCurSel, 0), -1);
		}

		//if nValue is -1, either nothing was selected or the "No row selected" row was selected.  In either case we warn
		if(nValue == -1 && (IsPayment() || (IsRefund() && m_radioCash.GetCheck()))) {
			if(MessageBox("You have not selected a cash drawer for this payment.  Are you sure you want to save this payment?\r\n\r\n"
				" - If you select YES, the payment will be saved with no cash drawer selected.\r\n"
				" - If you select NO,  you will be returned to the payment dialog and nothing will be saved.", "Practice", MB_ICONQUESTION|MB_YESNO) != IDYES)
				return FALSE;
		}
	}


	//tips!  if they don't have a method chosen for any, we can't save
	//Also, if they are negative we won't allow a save.
	//TES 3/19/2015 - PLID 65070 - Track the total amount being tipped using the gift certificate
	COleCurrency cyGiftTipTotal = COleCurrency(0, 0);
	// (r.gonet 2015-04-29 11:09) - PLID 65326 - Get the total tip amount.
	COleCurrency cyTipTotal(0, 0);
	for(int i = 0; i < m_pTipList->GetRowCount(); i++) {
		_variant_t var = m_pTipList->GetValue(i, tlcID);
		if(var.vt != VT_I4) {
			MsgBox("You must choose a method for all tips.");
			return FALSE;
		}

		//TES 3/26/2015 - PLID 65174 - For refunds, tips are allowed, indeed required, to be negative
		COleCurrency cyTip = VarCurrency(m_pTipList->GetValue(i, tlcAmount));
		if(cyTip < COleCurrency(0, 0) && IsPayment()) {
			MsgBox("You may not save a payment with a negative tip amount.");
			return FALSE;
		}
		//TES 4/2/2015 - PLID 65174 - If they don't have the preference or permission, then this won't get saved, so don't warn them
		if (cyTip > COleCurrency(0, 0) && IsRefund() && GetRemotePropertyInt("AllowTipRefunds", 0, 0, "<None>") && (GetCurrentUserPermissions(bioRefundTips, 0) & sptWrite)) {
			MsgBox("You may not save a refund with a positive tip amount.");
			return FALSE;
		}

		//TES 3/19/2015 - PLID 65069 - Don't allow them to save any gift certificate tips if the payment itself is not for a gift certificate
		// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
		if (VarLong(m_pTipList->GetValue(i, tlcMethod), -1) == (long)EPayMethod::GiftCertificatePayment) {
			if (!m_radioGift.GetCheck()) {
				MsgBox("Tips can only be paid by a gift certificate if the payment method on the payment is gift certificate. "
					"Please correct the tip method or payment method in order to continue saving this payment.");
				return FALSE;
			}
			else {
				cyGiftTipTotal += cyTip;
			}
		}
		// (r.gonet 2015-04-29 11:09) - PLID 65326 - Sum up the tip total.
		cyTipTotal += cyTip;
	}

	// Failure due to database being un-openable
	long iPatientID = m_PatientID;
	if (iPatientID < 0) {
		AfxMessageBox("You have attempted to make a payment for an invalid patient.\nThis payment will not be saved");
		return FALSE;
	}

	COleDateTime dt = COleDateTime(m_date.GetValue());
	COleDateTime dtOld;
	dtOld.SetDateTime(1753,1,1,1,1,1);
	if(dt.GetStatus() == COleDateTime::invalid || dtOld > dt) {
		AfxMessageBox("Please enter a valid date greater than 1/1/1753.");
		return FALSE;
	}

	// (c.haag 2009-03-10 15:41) - PLID 32433 - Fail if the user is trying to backdate the new payment more days than is
	// acceptable. Because users are allowed to backdate existing items they entered in the same day, there's no point in checking
	// this for new items, or items entered on the same day.
	//
	// (c.haag 2009-05-18 11:46) - PLID 34273 - We used to not check for backdating authorization for new payments, or payments.
	// created today. Now we always check.
	//
	{
		COleDateTime dtPayment = dt;
		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		dtPayment.SetDate(dtPayment.GetYear(), dtPayment.GetMonth(), dtPayment.GetDay());
		dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
		if (dtPayment < dtToday) {
			CString strType;
			if (m_radioPayment.GetCheck()) { strType = "Payment"; }
			else if (m_radioAdjustment.GetCheck()) { strType = "Adjustment"; }
			else if (m_radioRefund.GetCheck()) { strType = "Refund"; }
			else { ASSERT(FALSE); } // This should never happen; would be a legacy bug

			if (!strType.IsEmpty()) {
				if (!CanChangeHistoricFinancial_ByServiceDate(strType, dt, TRUE)) {
					// The user cannot save the item as is, and they've already been prompted about it.
					return FALSE;
				} else {
					// The user is allowed to save this item with this date
				}
			} else {
				ASSERT(FALSE); // This should never happen; would be a legacy bug
			}
		} else {
			// The payment is today or in the future
		}
	}

	// JJ - I found no easy way to check this prior to saving changes, because the "you do not have permission"
	// box pops up every time you call UserPermission. As for Edit Pay/Adj/Ref, I am taking care of it elsewhere
	// but it will remain here as a failsafe.
	if(m_boIsNewPayment == TRUE) {
		if(m_radioPayment.GetCheck() && !UserPermission(NewPayment)) return FALSE;
		if(m_radioAdjustment.GetCheck() && !UserPermission(NewAdjustment)) return FALSE;
		if(m_radioRefund.GetCheck() && !UserPermission(NewRefund)) return FALSE;
	}
	else {
		// (j.jones 2007-04-19 11:18) - PLID 25721 - In most cases CanEdit is not needed, so first
		// silently see if they have permission, and if they do, then move ahead normally. But if
		// they don't, or they need a password, then check CanEdit prior to the permission check that
		// would stop or password-prompt the user.
		if(m_radioPayment.GetCheck()) {
			// (j.jones 2014-07-02 09:15) - PLID 62562 - if this payment is part of a vision batch payment,
			// nobody can edit the payment
			// (b.eyers 2015-10-16) - PLID 67357 - let new payments with batch ids pass
			if (m_nBatchPaymentID != -1 && m_eBatchPayType == eVisionPayment && !m_boIsNewPayment) {
				MessageBox("This payment is part of a Vision Batch Payment, and cannot be edited. If you need to make changes, "
					"you must delete this payment and post a new one from the Batch Payments tab.", "Practice", MB_ICONHAND|MB_OK);
				return FALSE;
			}

			// (c.haag 2009-03-10 10:50) - PLID 32433 - We now use CanChangeHistoricFinancial. This way, if the
			// payment has a past date (say 1/1/07), and the user is not allowed to edit historic financials, then
			// this is the last line of defense to stop them.
			if (!CanChangeHistoricFinancial("Payment", VarLong(m_varPaymentID), bioPayment, sptWrite)) {
			//if(!(GetCurrentUserPermissions(bioPayment) & sptWrite) && !CanEdit("Payment", VarLong(m_varPaymentID,-1)) && !UserPermission(EditPayment)) {
				return FALSE;
			}
		}
		if(m_radioAdjustment.GetCheck()) {
			// (c.haag 2009-03-10 10:50) - PLID 32433 - We now use CanChangeHistoricFinancial. This way, if the
			// payment has a past date (say 1/1/07), and the user is not allowed to edit historic financials, then
			// this is the last line of defense to stop them.
			if (!CanChangeHistoricFinancial("Adjustment", VarLong(m_varPaymentID), bioAdjustment, sptWrite)) {
			//if(!(GetCurrentUserPermissions(bioAdjustment) & sptWrite) && !CanEdit("Adjustment", VarLong(m_varPaymentID,-1)) && !UserPermission(EditAdjustment)) {
				return FALSE;
			}
		}
		if(m_radioRefund.GetCheck()) {
			// (c.haag 2009-03-10 10:50) - PLID 32433 - We now use CanChangeHistoricFinancial. This way, if the
			// payment has a past date (say 1/1/07), and the user is not allowed to edit historic financials, then
			// this is the last line of defense to stop them.
			if (!CanChangeHistoricFinancial("Payment", VarLong(m_varPaymentID), bioRefund, sptWrite)) {
			//if(!(GetCurrentUserPermissions(bioRefund) & sptWrite) && !CanEdit("Refund", VarLong(m_varPaymentID,-1)) && !UserPermission(EditRefund)) {
				return FALSE;
			}
		}
	}

	//TES 4/9/2008 - PLID 29585 - Made this code also calculate the location name, for auditing.
	if(m_LocationCombo->GetCurSel()==-1)
		if(CString((LPCTSTR)m_LocationCombo->ComboBoxText) == "") {
			LocationID = GetCurrentLocationID();
			strLocationName = GetCurrentLocationName();
		}
		else {
			//must be their current inactive location.
			if(m_varPaymentID.vt == VT_EMPTY) {
				if(m_varBillID.vt == VT_I4) {
					_RecordsetPtr rsLoc = CreateRecordset("SELECT LocationID, LocationsT.Name FROM LineItemT INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID WHERE LineItemT.ID IN (SELECT ID FROM ChargesT WHERE BillID = %li)", VarLong(m_varBillID));
					if(!rsLoc->eof) {
						LocationID = AdoFldLong(rsLoc, "LocationID", GetCurrentLocationID());
						strLocationName = AdoFldString(rsLoc, "Name", GetCurrentLocationName());
					}
					else {
						LocationID = GetCurrentLocationID();
						strLocationName = GetCurrentLocationName();
					}
				}
				else {
					LocationID = GetCurrentLocationID();
				}
			}
			else{
				_RecordsetPtr rsLoc = CreateRecordset("SELECT LocationID, LocationsT.Name FROM LineItemT INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID WHERE LineItemT.ID = %li", VarLong(m_varPaymentID));
				if(!rsLoc->eof) {
					LocationID = AdoFldLong(rsLoc, "LocationID", GetCurrentLocationID());
					strLocationName = AdoFldString(rsLoc, "Name", GetCurrentLocationName());
				}
				else {
					LocationID = GetCurrentLocationID();
					strLocationName = GetCurrentLocationName();
				}
			}
		}
	else {
		var = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0);
		if(var.vt == VT_I4) {
			LocationID = var.lVal;
			strLocationName = VarString(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),1));
		}
		else {
			LocationID = GetCurrentLocationID();
			strLocationName = GetCurrentLocationName();
		}
	}

	// (j.gruber 2007-08-09 10:42) - PLID 25119 - make sure they chose a drawer at the correct location
	if(m_bHasNexSpa) {
		long nCurSel = m_pDrawerList->GetCurSel();
		long nValue = -1;
		if (nCurSel != sriNoRow) {

			
			nValue = VarLong(m_pDrawerList->GetValue(nCurSel, 0), -1);
			// (j.luckoski 2013-05-07 15:27) - PLID 54418 - Don't warn about the cash drawer location on adjustments, ain't nobody got time fo' that!
			if (nValue != -1 && !(m_radioAdjustment.GetCheck())) {

				_RecordsetPtr rsDrawerLoc = CreateParamRecordset("SELECT LocationID FROM CashDrawersT WHERE ID = {INT}", nValue);
				if (! rsDrawerLoc->eof) {
					long nDrawerLocID = AdoFldLong(rsDrawerLoc, "LocationID", -1);

					if (nDrawerLocID != -1) {
						if (nDrawerLocID != LocationID) {
							MsgBox("The location of the selected drawer does not match the location of this payment.  Please correct this and save the payment again.");
							return FALSE;
						}
					}
				}
				else {
					//this should never happen
					ASSERT(FALSE);
				}

			}	

		}
		//well they've already been warned above and chosen to proceed, so we don't need to bother with our message
	}

	// (d.thompson 2009-06-30) - PLID 34687 - These are all credit-card related data checks.  If they aren't 
	//	processing, we don't worry about them.  I tried to maintain PLIDs from previous notes where possible.
	// (j.jones 2015-09-30 08:58) - PLID 67157 - this does not apply to ICCP
	if(m_radioCharge.GetCheck() && !m_radioAdjustment.GetCheck() && !IsICCPEnabled() && IsDlgButtonChecked(IDC_PROCESS_CC))	{
		// (j.gruber 2007-07-30 14:55) - PLID 26704 - make sure that if this is a refund, they have an applyID
		if (m_radioRefund.GetCheck()) {
			// (j.jones 2015-09-30 09:46) - PLID 67170 - this is ok if they have ICCP
			// (j.jones 2015-09-30 10:37) - PLID 67172 - the payment to apply to is now a struct
			if (m_PaymentToApplyTo.nPaymentID == -1 && !IsICCPEnabled()) {
				MsgBox("You cannot process a refund without first choosing the payment this refund is refunding.");
				return FALSE;
			}
		}

		//Ensure permissions
		if(m_radioRefund.GetCheck()) {
			if(!CheckCurrentUserPermissions(bioCCRefundAndVoid, sptWrite)) {
				//Failure
				return FALSE;
			}
		}
		else if(m_radioPayment.GetCheck()) {
			if(!CheckCurrentUserPermissions(bioCCProcess, sptWrite)) {
				//Failure
				return FALSE;
			}
		}

		if (!CheckCCFields()) {
			MsgBox("Please fill in all applicable credit card information before attempting to process this payment.");
			return FALSE;
		}

		//make sure there is a report to print at the end
		//get the currently selected report
		NXDATALIST2Lib::IRowSettingsPtr pReportRow;
		pReportRow = m_pReportList->CurSel;
		if (pReportRow == NULL) {
			AfxMessageBox("Please select a report to run from the reports drop down");
			return FALSE;
		}
	}




	/******************************************************
	* First make sure all the edit boxes have information *
	******************************************************/
	
	CString str;
	CString month, year;

	////////////////////////////////////////////////////////
	// Make sure total amount is not blank, is less than
	// the maximum allowable amount, has no bad characters,
	// and has no more than two places to the right of the
	// decimal point.
	////////////////////////////////////////////////////////
	GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(str);
	if (str.GetLength() == 0) {
		MsgBox("Please fill in the 'Total Amount' box.");
		return FALSE;
	}


	COleCurrency cy = ParseCurrencyFromInterface(str);
	if(cy.GetStatus() == COleCurrency::invalid) {
		MsgBox("Please enter a valid amount in the 'Total Amount' box.");
		return FALSE;
	}

	if (m_radioPayment.GetCheck() == 1 && cy < COleCurrency::COleCurrency(0,0))
	{
		MsgBox("Payments cannot be negative. Please enter a positive number.");
		return FALSE;
	}

	//see how much the regional settings allows to the right of the decimal
	CString strICurr;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
	int nDigits = atoi(strICurr);
	CString strDecimal;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
	if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
		MsgBox("Please fill only %li places to the right of the %s in the 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
		return FALSE;
	}

	CString strCashReceived;
	COleCurrency cyCashReceived = COleCurrency(0,0);
/*	DRT 2/9/2005 - PLID 15566 - Used to only be enabled for cash, but now for all payment methods (not adj/ref/gift cert)
	if (m_radioPayment.GetCheck() && m_radioCash.GetCheck()) {
*/
	// (j.gruber 2007-07-31 12:58) - PLID 26889 - took this out for cc processing if its a charge
	//if((m_bProcessCreditCards && m_radioPayment.GetCheck() && (m_radioCash.GetCheck() || m_radioCheck.GetCheck() )) || 
	//	(!m_bProcessCreditCards && m_radioPayment.GetCheck() && (m_radioCash.GetCheck() || m_radioCheck.GetCheck() || m_radioCharge.GetCheck()))) {

	// (d.thompson 2009-07-01) - PLID 34779 - Undid this part of the change -- we still need to save this value, even if
	//	they can't see it.  It's automatically kept up to date with the "Total Amount" information.
	if(m_radioPayment.GetCheck() && (m_radioCash.GetCheck() || m_radioCheck.GetCheck() || m_radioCharge.GetCheck())) {
		//now get the receive amount
		GetDlgItem(IDC_CASH_RECEIVED)->GetWindowText(strCashReceived);

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
		
		if(cyCashReceived.Format().Find(strDecimal) != -1 && cyCashReceived.Format().Find(strDecimal) + (nDigits+1) < cyCashReceived.Format().GetLength()) {
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please fill only %li places to the right of the %s in the 'Cash Received' box.",nDigits,strDecimal == "," ? "comma" : "decimal");			
			return FALSE;
		}

		if(cyCashReceived < cy) {
			//reset it to the payment amount
			SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(cy,FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("You cannot enter an amount less than the payment amount in the 'Cash Received' box.");			
			return FALSE;
		}
	}

/*	DRT 2/9/2005 - PLID 15566 - Previously only enabled for cash, now enabled for all payment methods (not adj/ref)
	if (m_radioPayment.GetCheck() && m_radioCash.GetCheck()) {
*/
	if(m_radioPayment.GetCheck()) {
		strCashReceived.Format("Convert(money,'%s')",FormatCurrencyForSql(cyCashReceived));
	}
	else {
		strCashReceived = "NULL";
	}

	// (j.jones 2007-02-26 16:26) - PLID 24927 - warn if they try to change the amount of an item
	// if it is linked to the ReturnedProductsT table
	if(m_varPaymentID.vt == VT_I4 && VarLong(m_varPaymentID,-1) != -1) {

		COleCurrency cyCurAmount = cy;
		if (IsAdjustment() || IsRefund()) {
			// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - Need to explicitly define the multiplier as a long
			cyCurAmount = cyCurAmount * long(-1);
		}

		//compare to the old amount
		if(ReturnsRecords("SELECT Amount FROM LineItemT WHERE ID = %li AND Amount <> Convert(money,'%s')",m_varPaymentID.lVal, FormatCurrencyForSql(cyCurAmount))) {
			//it is different, so now check ReturnedProductsT

			// (j.jones 2007-04-19 14:41) - PLID 25711 - converted to use loaded booleans, instead of recordsets
			if(m_bIsReturnedProductRefund) {
				if(IDNO == MessageBox("This refund is linked to a Returned Product record. It is not recommended that you change the amount of this line item.\n"
								"Are you sure you wish to continue saving with the new amount?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
								return FALSE;
				}
			}
			else if(m_bIsReturnedProductAdj) {
				if(IDNO == MessageBox("This adjustment is linked to a Returned Product record. If you change the amount of this line item,\n"
								"reports such as Tax Totals and Provider Commissions (Charges) will not calculate the value of all\n"
								"Returned Products properly, if you choose to show Returned Products in those reports.\n\n"
								"Are you sure you wish to continue saving with the new amount?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
								return FALSE;
				}
			}
		}
	}

	if(m_radioGift.GetCheck()) {
		//make sure they have chosen a gift cert
		// (j.jones 2015-03-25 17:00) - PLID 65281 - just check m_nSelectedGiftID
		// (r.gonet 2015-04-20) - PLID 65326 - This doesn't apply if we are refunding to a new gift certificate.
		if (m_nSelectedGiftID == -1 &&
			(m_radioPayment.GetCheck() == BST_CHECKED 
			|| (m_radioRefund.GetCheck() == BST_CHECKED && m_radioRefundToExistingGiftCertificate.GetCheck() == BST_CHECKED))
		) {
			MsgBox("You must select a gift certificate before continuing.");
			return FALSE;
		}

		//final check, just in case they somehow snuck by here, they must be using NexSpa to save
		if(!m_bHasNexSpa) {
			MsgBox("Gift certificates are a NexSpa only license.  You must purchase it before saving these payment types.");
			return FALSE;
		}

		// (j.jones 2015-03-25 17:00) - PLID 65281 - use m_nSelectedGiftID
		// (r.gonet 2015-04-20) - PLID 65327 - If they aren't paying with or refunding to a gift certificate, then clear out the GiftID in the database.
		if ((IsPayment() || IsRefund()) && m_radioGift.GetCheck() == BST_CHECKED) {
			strGiftID.Format("%li", m_nSelectedGiftID);
		} else { 
			strGiftID = "NULL";
		}

		// (r.gonet 2015-04-20) - PLID 65326 - Both payments and refunds can now be to gift certificates. Check which one we have.
		if (m_radioPayment.GetCheck() == BST_CHECKED) {
			// (r.gonet 2015-04-20) - PLID 65326 - Since we pull the balance from the database and do not do anything with the balance other than show it, the
			// balance field can be trusted now regardless if the payment is new or existing.

			//DRT 4/23/2004 - If they have entered a GC, the amount of the payment cannot be greater than 
			//	the amount of balance on the GC.
			//TES 3/19/2015 - PLID 65070 - We need to compare against the total of the payment (which is stored in the informatively named variable "cy") plus any gift certificate tips
			COleCurrency cyGiftTotal = cy + cyGiftTipTotal;
			
			// (j.jones 2015-03-26 09:09) - PLID 65283 - pull this from the dialog field
			CString strCurrentBalance = m_nxeditGCBalance.GetText();
			COleCurrency cyCurrentBalance = COleCurrency(0, 0);
			if (!cyCurrentBalance.ParseCurrency(strCurrentBalance)) {
				//this shouldn't be possible
				ASSERT(FALSE);
				cyCurrentBalance = COleCurrency(0, 0);
			}

			if (!m_boIsNewPayment) {
				//we're editing a payment.  Therefore, the balance field is not a valid reference, because it already
				//includes this payment in its calculation.  To be valid, we have to sum all non-deleted payments 
				//(that are not this one) and add this amount, and make sure it is not higher than the purchased
				//amount for this GC (col 2 in this list)
				// (r.gonet 2015-04-20) - PLID 65326 - Just get this payment's saved total + tip amount, subtract that from the saved balance, then add the new cyGiftTotal.
				_RecordsetPtr prsAmountUsedByThisPayment = CreateParamRecordset(R"(
SELECT Sum(Amount) AS AmountUsed
FROM 
(
    SELECT LineItemT.Amount
    FROM LineItemT 		
    LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID 
    LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
    WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1
    AND LineItemT.GiftID = {STRING}
    AND OrigLineItemsT.OriginalLineItemID IS NULL
    AND VoidingLineItemsT.VoidingLineItemID IS NULL
    AND LineItemT.ID = {INT}

    UNION ALL 
    
	SELECT PaymentTipsT.Amount 
    FROM LineItemT 
    INNER JOIN PaymentTipsT ON LineItemT.ID = PaymentTipsT.PaymentID 
    LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID 
    LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
    WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 
    AND LineItemT.GiftID = {STRING}
    AND PaymentTipsT.PayMethod = 4 
    AND OrigLineItemsT.OriginalLineItemID IS NULL
    AND VoidingLineItemsT.VoidingLineItemID IS NULL
    AND LineItemT.ID = {INT}
) AS UsedAmountQ
)", strGiftID, VarLong(m_varPaymentID), strGiftID, VarLong(m_varPaymentID));
				COleCurrency cyAmountUsedByThisPayment(0, 0);
				if (!prsAmountUsedByThisPayment->eof) {
					cyAmountUsedByThisPayment = AdoFldCurrency(prsAmountUsedByThisPayment, "AmountUsed", COleCurrency(0, 0));
					// Add back the amount this payment subtracted from the saved balance so that when we compare below, it will reflect the balance without this payment factored in.
					cyCurrentBalance += cyAmountUsedByThisPayment;
				}
			} else {
				//new payment - we can trust the balance field
			}
			
			if (cyGiftTotal > cyCurrentBalance) {
				//TES 3/19/2015 - PLID 65070 - Updated this message
				MsgBox("You are attempting to deduct a total payment of %s from a gift certificate that has a balance of %s which cannot be done. "
					"Please make the necessary adjustments to continue saving this payment.", FormatCurrencyForInterface(cyGiftTotal), FormatCurrencyForInterface(cyCurrentBalance));
				return FALSE;
			}
		} else if (m_radioRefund.GetCheck() == BST_CHECKED) {
			// (r.gonet 2015-04-20) - PLID 65326 - Don't allow them to refund less than zero.
			if (cy < g_ccyZero) {
				MsgBox("You are attempting to refund %s which cannot be done when refunding to a gift certificate. Please enter a non-negative amount to continue saving.",
					FormatCurrencyForInterface(cy));
				return FALSE;
			}
		}
	}

	//fill in the drawer ID
	CString strDrawerID = "NULL";
	m_nDrawerID = -1;
	if(!m_pDrawerList->GetEnabled()) {
		//drawer list disabled, must not be allowed to change, so keep the same ID
		_RecordsetPtr prs = CreateRecordset("SELECT DrawerID FROM LineItemT WHERE ID = %li", VarLong(m_varPaymentID));
		if(!prs->eof) {
			_variant_t var = prs->Fields->Item["DrawerID"]->Value;
			if(var.vt == VT_I4)
				strDrawerID.Format("%li", VarLong(var));
				m_nDrawerID = VarLong(var);
		}
	}
	else if(m_bHasNexSpa && GetDlgItem(IDC_CASH_DRAWER_LIST)->IsWindowEnabled() && m_pDrawerList->GetCurSel() != sriNoRow) {
		//DRT 2/15/2005 - PLID 15644 - Only do this if it's a payment or cash-refund
		//DRT 6/29/2005 - PLID 16010 - Ignore the above, we've further modified this to allow the
		//	user to specify which are allowed by preference.
		bool bSaveDrawer = false;
		if(IsPayment())
			bSaveDrawer = true;
		else if(IsRefund() && GetDlgItem(IDC_CASH_DRAWER_LIST)->IsWindowVisible()) {
			//However, if they're editing a previous refund that did allow a drawer, we'll have shown that, so
			//	we also need to save that -- we can check by seeing if the window is visible.
			bSaveDrawer = true;
		}

		if(bSaveDrawer) {
			long nDrawerID = VarLong(m_pDrawerList->GetValue(m_pDrawerList->GetCurSel(), 0));
			if(nDrawerID != -1) {	//-1 is the "no selected" case
				strDrawerID.Format("%li", nDrawerID);
				m_nDrawerID = nDrawerID;
			}
		}
	}

//Adjustments should be allowed to be ANY amount, unless m_bAdjustmentFollowMaxAmount is TRUE
//Payments must be positive and less than or equal to the max amount
//Refunds are not allowed
	if (m_radioPayment.GetCheck()) 
	{	if (m_cyMaxAmount != COleCurrency(9999999,9999999) && cy > m_cyMaxAmount) {

			BOOL bAllowOverage = FALSE;

			//JMJ - 7/31/2003 - if the responsibility is insurance and you are trying to apply more
			//than the balance, AND it is a charge line, ask to increase the balance. Else yell at them.
			var = m_InsuranceCombo->GetValue(m_InsuranceCombo->GetCurSel(),0);	// Insurance company
			if (var.vt != VT_EMPTY && var.vt != VT_NULL && var.lVal > 0 && m_varChargeID.vt == VT_I4) {
				str.Format("The maximum amount you can apply to this insurance charge is %s,\n"
					"would you like to increase the insurance balance?\n",FormatCurrencyForInterface(m_cyMaxAmount));
				if(IDYES == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
					if(!IncreaseInsBalance(m_varChargeID.lVal,iPatientID,var.lVal,cy - m_cyMaxAmount))
						return FALSE;
				}
				else {
					bAllowOverage = TRUE;
				}
			}
			else {
				bAllowOverage = TRUE;
			}

			if(bAllowOverage) {
				CString str;
				str.Format("Your payment is greater than the expected amount of %s.\n"
					"If you continue, the payment will be created, but the remainder will be left unapplied.\n\n"
					"Are you sure you wish to save this payment?",FormatCurrencyForInterface(m_cyMaxAmount));
				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return FALSE;
				}
				
				//MsgBox("Please enter an amount no greater than %s", FormatCurrencyForInterface(m_cyMaxAmount));
				//return FALSE;
			}			
		}
	}
	else if(m_radioAdjustment.GetCheck() && m_bAdjustmentFollowMaxAmount) {
		if (m_cyMaxAmount != COleCurrency(9999999,9999999) && cy > -m_cyMaxAmount) {

			CString str;
			str.Format("Your adjustment is greater than the expected amount of %s.\n"
				"If you continue, the adjustment will be created, but the remainder will be left unapplied.\n\n"
				"Are you sure you wish to save this adjustment?",FormatCurrencyForInterface(-m_cyMaxAmount));
			if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}
			
			//MsgBox("Please enter an amount no greater than %s", FormatCurrencyForInterface(-m_cyMaxAmount));
			//return FALSE;		
		}
	}
	else if(m_radioRefund.GetCheck())
	{	
		if (m_cyMaxAmount != COleCurrency(9999999,9999999) && cy > -m_cyMaxAmount) {

			CString str;
			str.Format("Your refund is greater than the expected amount of %s.\n"
				"If you continue, the refund will be created, but the remainder will be left unapplied.\n\n"
				"Are you sure you wish to save this refund?",FormatCurrencyForInterface(-m_cyMaxAmount));
			if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}
			
			//MsgBox("Please enter an amount no greater than %s", FormatCurrencyForInterface(-m_cyMaxAmount));
			//return FALSE;		
		}
	}

	CString strReasonCodeID = "NULL", strGroupCodeID = "NULL";
	long nReasonCodeID = -1, nGroupCodeID = -1;
	CString strReasonCodeNum_ForAudit = "", strGroupCodeNum_ForAudit = "";
	// (a.walling 2006-11-15 12:04) - PLID 23550 - Get the reason and group code strings
	// (j.jones 2010-09-23 11:42) - PLID 27917 - these are now nullable IDs
	if (m_radioAdjustment.GetCheck()) {
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		if(m_pGroupCodeList->IsComboBoxTextInUse) {
			//keep the existing ID, don't need to fill
			nGroupCodeID = m_nPendingGroupCodeID;
		}
		else {
			pRow = m_pGroupCodeList->GetCurSel();
			if(pRow) {
				nGroupCodeID = VarLong(pRow->GetValue(gcccID), -1);			
				strGroupCodeNum_ForAudit = VarString(pRow->GetValue(gcccCode));
			}
		}

		if(nGroupCodeID != -1) {
			strGroupCodeID.Format("%li", nGroupCodeID);			
		}

		if(m_pReasonList->IsComboBoxTextInUse) {
			//keep the existing ID
			nReasonCodeID = m_nPendingReasonCodeID;
		}
		else {
			pRow = m_pReasonList->GetCurSel();
			if (pRow) {
				nReasonCodeID = VarLong(pRow->GetValue(rcccID), -1);			
				strReasonCodeNum_ForAudit = VarString(pRow->GetValue(rcccCode));
			}
		}

		if(nReasonCodeID != -1) {
			strReasonCodeID.Format("%li", nReasonCodeID);	
		}
	}

	CString strCCProcessTypeNewAuditValue;
	// (z.manning 2016-02-15 11:38) - PLID 68258 - Moved this logic to its own function
	LineItem::CCProcessType eCCProcessType = GetCCProcessType(strCCProcessTypeNewAuditValue);

	// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType
	CString strCCProcessTypeForSql = "NULL";
	if (eCCProcessType != LineItem::CCProcessType::None) {
		strCCProcessTypeForSql = AsString((long)eCCProcessType);
	}

	CString strCreditCardToken;
	COleDateTime dtCardExpiration = g_cdtInvalid;
	if ((IsPayment() || IsRefund()) && m_radioCharge.GetCheck())
	{
		if (eCCProcessType == LineItem::CCProcessType::CardNotPresent) {
			// (z.manning 2015-08-27 15:02) - PLID 67232 - Get the token and exp date as they 
			// may have selected to use a saved card.
			NXDATALIST2Lib::IRowSettingsPtr pCardOnFileRow = m_CCCardOnFileCombo->CurSel;
			if (pCardOnFileRow != NULL) {
				strCreditCardToken = VarString(pCardOnFileRow->GetValue(ppcCreditCardToken), "");
				dtCardExpiration = VarDateTime(pCardOnFileRow->GetValue(ppcExpDate), g_cdtInvalid);
			}
		}

		if ((m_varPaymentID.vt == VT_NULL || m_varPaymentID.vt == VT_EMPTY) && eCCProcessType != LineItem::CCProcessType::None) {
			// We now process payments after saving the payment. So for now, flag this payment as "do not process"
			// until we've actually done the processing. We only do this for new payments as this field should not
			// be touched for existing ones.
			strCCProcessTypeForSql = AsString((long)LineItem::CCProcessType::DoNotProcess);
		}
	}

	// (b.spivey, September 25, 2014) - PLID 63422 - Lets warn them if it's linked to a lockbox payment.
	str = ""; 
	GetDlgItemText(IDC_EDIT_TOTAL, str);
	COleCurrency cyPaymentTotal = ParseCurrencyFromInterface(str);

	//If it's a payment, and if the value is valid.  
	if (IsPayment() && m_cyOriginalAmount != cyPaymentTotal && m_bIsLinkedLockboxPayment) {

		int nResult = MessageBox("This payment is associated with a lockbox deposit. If you edit the payment amount, your lockbox deposit will no longer reconcile. \r\n\r\n"
			"Are you sure you wish to do this?", "Lockbox Payment", MB_YESNO | MB_ICONEXCLAMATION);
		//if not, return and let them fix it. 
		if (nResult == IDNO) {
			return FALSE;
		}
	}
	
	// (r.gonet 2015-04-20) - PLID 65326 - Check for some gift certificate related issues and warn the user if they occur.
	if (m_varPaymentID.vt == VT_I4) 
	{
		// (r.gonet 2015-04-20) - PLID 65326 - Warn.
		if (m_eOriginalPayMethod == EPayMethod::GiftCertificateRefund && GetPayMethod() == EPayMethod::GiftCertificateRefund && -1 * m_cyOriginalAmount != cyPaymentTotal) {
			if (IDNO == MessageBox("This refund is crediting an existing gift certificate. If you change the refund amount, the credited gift certificate's value and balance will be changed and may become negative. \r\n\r\n"
				"Are you sure you wish to do this?", "Gift Certificate Refund", MB_YESNO | MB_ICONEXCLAMATION)) {
				return FALSE;
			}
		}

		// (r.gonet 2015-04-20) - PLID 65326 - Warn.
		if (m_eOriginalPayMethod == EPayMethod::GiftCertificateRefund && GetLineItemTypeFromPayMethod(GetPayMethod()) != LineItem::Type::Refund) {
			if (IDNO == MessageBox(FormatString("You are changing the type of this line item from Gift Certificate Refund to %s, but the refund was crediting an existing gift certificate. If you continue, the credited gift certificate's value and balance will be reduced and may become negative. \r\n\r\n"
				"Are you sure you wish to do this?",
				GetPayMethodName(GetPayMethod(), "something different", true, true)), 
				"Gift Certificate Refund", MB_YESNO | MB_ICONEXCLAMATION)) {
				return FALSE;
			}
		}

		// (r.gonet 2015-04-20) - PLID 65326 - Warn.
		if (m_eOriginalPayMethod == EPayMethod::GiftCertificateRefund && GetLineItemTypeFromPayMethod(GetPayMethod()) == LineItem::Type::Refund && GetPayMethod() != EPayMethod::GiftCertificateRefund) {
			if (IDNO == MessageBox("This refund is crediting an existing gift certificate. If you change the refund type to cash, check or charge, the credited gift certificate's value and balance will be reduced and may become negative. \r\n\r\n"
				"Are you sure you wish to do this?", "Gift Certificate Refund", MB_YESNO | MB_ICONEXCLAMATION)) {
				return FALSE;
			}
		}

		// (r.gonet 2015-04-20) - PLID 65326 - Warn.
		if (m_eOriginalPayMethod == EPayMethod::GiftCertificateRefund && GetPayMethod() == EPayMethod::GiftCertificateRefund && m_nOriginalGiftID != -1 && m_nOriginalGiftID != m_nSelectedGiftID) {
			if (IDNO == MessageBox("This refund is crediting an existing gift certificate. If you change which gift certificate to credit with the refund, the old gift certificate's value and balance will be reduced and may become negative. \r\n\r\n"
				"Are you sure you wish to do this?", "Gift Certificate Refund", MB_YESNO | MB_ICONEXCLAMATION)) {
				return FALSE;
			}
		}
	} else {
		// (r.gonet 2015-04-20) - PLID 65327 - Warn.
		if (GetPayMethod() == EPayMethod::GiftCertificateRefund && m_PaymentToApplyTo.nPaymentID == -1 && m_nPayToApplyToGiftID == -1) {
			if (IDNO == MessageBox("This refund is crediting a gift certificate but is being created unapplied to a payment. The gift certificate's Value and Balance will be increased by the refunded amount. \r\n\r\n"
				"Are you sure you wish to do this?", "Gift Certificate Refund", MB_YESNO | MB_ICONEXCLAMATION)) {
				return FALSE;
			}
		}
	}

	// (r.gonet 2015-04-20) - PLID 65327 - If this is a gift certificate refund, and we're refunding to a new gift certificate,
	// then we now have to have the user create the gift certificate.
	long nNewGiftID; CString strNewGiftCertNumber;
	if (IsRefund() && m_radioGift.GetCheck() && m_radioRefundToNewGiftCertificate.GetCheck() == BST_CHECKED) {
		// (r.gonet 2015-04-29 11:09) - PLID 65327 - Open the gift certificate entry dialog displaying the Value as the 
		// refund amount plus the tip total and the Price as 0.00. 
		// (j.jones 2015-05-11 16:57) - PLID 65714 - added GCCreationStyle
		bool bGCSuccess = CreateNewGiftCertificate(this, GCCreationStyle::eRefundToNewGC, nNewGiftID, strNewGiftCertNumber,
			"", -1, g_ccyZero, cy + -1 * cyTipTotal, g_ccyZero, ProviderID, LocationID, m_date.GetDateTime(), false, g_cdtNull, m_PatientID, true, m_PatientID);
		if (bGCSuccess) {
			strGiftID = FormatString("%li", nNewGiftID);
			// (r.gonet 2015-07-06 18:02) - PLID 65327 - A bill was created. Let any callers know through a public variable.
			m_bBillCreated = true;
		} else {
			//abort the saving
			return FALSE;
		}
	}	

	// (c.haag 2015-08-18) - PLID 67203 - We can't check m_boIsNewPayment because this may be an existing
	// "Other (Do not process)" payment that the user decided to change into a Swipe / DIP or possibly a cash payment
	// that the user wants to change to a charge payment and do a transaction for. So we'll instead check m_bHasAnyTransaction
	// to verify that no credit transaction has been done.
	if (!m_bHasAnyTransaction
		&& LineItem::CCProcessType::None != eCCProcessType
		&& LineItem::CCProcessType::DoNotProcess != eCCProcessType
		&& m_radioCharge.GetCheck())
	{
		try
		{
			// (c.haag 2015-08-25) - PLID 67203 - Abort if ICCP is not enabled
			if (!IsICCPEnabled())
			{
				MessageBox("Integrated Credit Card Processing is not enabled. To perform a transaction, please enable this feature before continuing.", NULL, MB_OK | MB_ICONSTOP);
				return FALSE;
			}
			// (c.haag 2015-08-25) - PLID 67203 - Abort if this is not a new payment
			else if (IsAdjustment())
			{
				MessageBox("Integrated Credit Card Processing transactions may only take place on new payments or new refunds.", NULL, MB_OK | MB_ICONSTOP);
				return FALSE;
			}

			// (c.haag 2015-08-18) - PLID 67203 - Handle authorizing "Swipe / DIP Card" credit card payments
			// (c.haag 2015-08-24) - PLID 67202 - Handle authorizing "Card Not Present" credit card payments
			if (LineItem::CCProcessType::Swipe == eCCProcessType
				|| LineItem::CCProcessType::CardNotPresent == eCCProcessType)
			{
				// (z.manning 2015-08-26 11:31) - PLID 67227 - We can't process this payment/refund without a device
				// so let's check that first.
				// (z.manning 2015-10-08 15:58) - PLID 67227 - If we have the token already (because we're using
				// an existing payment profile) then no device is necessary.
				if (strCreditCardToken.IsEmpty() && (GetICCPDevice() == NULL || !GetICCPDevice()->IsDeviceConnected()))
				{
					MessageBox("There is not a supported card reader attached to this workstation. You may only use the transaction type of 'Other (Do Not Process).'"
						, "No Device", MB_ICONERROR);
					return FALSE;
				}

				// (c.haag 2015-08-24) - PLID 67202 - We can't process this payment/refund without a merchant
				if (GetCurrentlySelectedMerchantAccountID() <= 0)
				{
					MessageBox("You must select an entry from the Merchant Acct dropdown.", "No Merchant", MB_ICONERROR);
					GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->SetFocus();
					return FALSE;
				}
			}
			// (c.haag 2015-08-25) - PLID 67197 - Handle refunds to original payments
			else if (LineItem::CCProcessType::RefundToOriginal == eCCProcessType)
			{
				if (!m_PaymentToApplyTo.bHasICCPTransaction)
				{
					MessageBox("The original payment was not authorized by Integrated Credit Card Processing and cannot be refunded.", NULL, MB_OK | MB_ICONSTOP);
					return FALSE;
				}
				else if (GetCurrentlySelectedMerchantAccountID() <= 0)
				{
					MessageBox("You must select an entry from the Merchant Acct dropdown.", "No Merchant", MB_ICONERROR);
					GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->SetFocus();
					return FALSE;
				}
				else if (m_PaymentToApplyTo.nICCPMerchantID != GetCurrentlySelectedMerchantAccountID())
				{
					MessageBox("You may only refund the original card using the same merchant it was charged with.", "Incorrect Merchant", MB_ICONERROR);
					m_CCMerchantAccountCombo->FindByColumn(maccID, m_PaymentToApplyTo.nICCPMerchantID, nullptr, VARIANT_TRUE);
					GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->SetFocus();
					return FALSE;
				}
			}
			else
			{
				ThrowNxException(FormatString("Unhandled eCCProcessType: %d", eCCProcessType));
			}
		}
		NxCatchAllCall("Error performing Integrated Credit Card Processing transaction", return FALSE;);
	}

	long nInsuredPartyID = -1;
	
	try {
		_RecordsetPtr rs(__uuidof(Recordset));
		_RecordsetPtr rsDoctors(__uuidof(Recordset));
		COleVariant var;
		
//////////////////////////////////////////////////////////
// Write to payments table
//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// Prepare to edit existing payment
		//////////////////////////////////////////////////////////		
		if (m_varPaymentID.vt == VT_NULL || m_varPaymentID.vt == VT_EMPTY) {

			bIsNew = TRUE;

			CString strQuoteID = "NULL";
			if(m_QuoteID != -1)
				strQuoteID.Format("%li",m_QuoteID);
			// (r.gonet 2015-04-29 15:38) - PLID 65326 - We need to track where the gift certificate refund came from if it came from a gift certificate payment.
			CString strRefundedFromGiftID = "NULL";
			if (IsRefund() && m_nPayToApplyToGiftID != -1) {
				strRefundedFromGiftID.Format("%li", m_nPayToApplyToGiftID);
			}
			
			//(e.lally 2007-03-21) PLID 25258 - Optimize the save code to use less trips to the SQL server.
			CString strSqlBatch1 = BeginSqlBatch();
			AddDeclarationToSqlBatch(strSqlBatch1,
				"DECLARE @iLineItemID INT \r\n ");

			// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
			AddStatementToSqlBatch(strSqlBatch1,"INSERT INTO LineItemT (PatientID, LocationID, Type, Date, InputName, InputDate, GiftID, DrawerID) VALUES "
				"(%li, %li, %li, GetDate(), '%s', GetDate(), %s, %s)", m_PatientID, LocationID, -1, _Q(GetCurrentUserName()), strGiftID, 
				strDrawerID);

			AddStatementToSqlBatch(strSqlBatch1, 
				"SET @iLineItemID = SCOPE_IDENTITY() \r\n");

			// (j.armen 2013-06-29 15:34) - PLID 57375 - PaymentsT.PaymentUniqueID now gets it's ID from an identity seeded table
			AddStatementToSqlBatch(strSqlBatch1, CSqlFragment(
				"DECLARE @nPaymentUniqueID INT\r\n"
				"IF {BIT} = 1 -- Is Adjustment\r\n"
				"BEGIN\r\n"
				"	SET @nPaymentUniqueID = NULL\r\n"
				"END\r\n"
				"ELSE -- Is Payment\r\n"
				"BEGIN\r\n"
				"	INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
				"	SET @nPaymentUniqueID = SCOPE_IDENTITY()\r\n"
				"END\r\n", m_radioPayment.GetCheck() ? false : true).Flatten());
			
			// (a.walling 2006-11-15 12:07) - PLID 23550 - Add reason and group codes
			// (j.jones 2010-09-23 11:42) - PLID 27917 - these are now nullable IDs
			// (r.gonet 2015-04-29 15:38) - PLID 65326 - Added RefundedFromGiftID
			// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType
			//****REMEMBER: All new fields also need to be supported in FinancialCorrection.cpp****//

			// (b.eyers 2015-10-16) - PLID 67357 - save batch id if there is one
			CString strBatchPaymentID = "NULL";
			if (m_nBatchPaymentID != -1) { 
				strBatchPaymentID.Format("%li", m_nBatchPaymentID);
			}

			AddStatementToSqlBatch(strSqlBatch1,
				"INSERT INTO PaymentsT (ID, InsuredPartyID, PaymentUniqueID, QuoteID, ReasonCodeID, GroupCodeID, RefundedFromGiftID, CCProcessType, BatchPaymentID)\r\n"
				"	VALUES (@iLineItemID,%li,@nPaymentUniqueID,%s, %s, %s, %s, %s, %s)",
				-1, strQuoteID, strReasonCodeID, strGroupCodeID, strRefundedFromGiftID, strCCProcessTypeForSql, strBatchPaymentID);

			AddStatementToSqlBatch(strSqlBatch1, "INSERT INTO PaymentPlansT (ID) VALUES (@iLineItemID)");
			//We are creating a recordset to return our new ID, surround the inserts with
			strSqlBatch1 ="SET NOCOUNT ON \r\n"
				"BEGIN TRAN \r\n" + strSqlBatch1 + 
				"SELECT @iLineItemID AS NewLineItemID \r\n"
				"COMMIT TRAN \r\n"
				"SET NOCOUNT OFF \r\n";

			//(e.lally 2007-03-21) PLID - We have to commit here so we can use the new payment ID
			_RecordsetPtr rsLineItem = CreateRecordset("%s", strSqlBatch1);

			// (z.manning 2016-02-15 15:02) - PLID 68258 - We just created a new payment, so flag it as not yet processed.
			if (ShouldPromptForICCPAuthorization()) {
				m_bNewICCPPaymentSavedButNotYetProcessed = TRUE;
			}

			if(rsLineItem->GetState() == adStateClosed) {
				_variant_t varNull;
				rsLineItem = rsLineItem->NextRecordset(&varNull);
			}
			if(!rsLineItem->eof){
				m_varPaymentID = AdoFldLong(rsLineItem, "NewLineItemID");
			}
			rsLineItem->Close();
		}

		if(m_CategoryCombo->GetCurSel() != -1) {
			var = m_CategoryCombo->GetValue(m_CategoryCombo->GetCurSel(),0);
			if (var.vt != VT_NULL)
				iPaymentGroupID = var.lVal;
			else
				iPaymentGroupID = 0;
		}

		if(m_InsuranceCombo->GetCurSel() != -1) {
			var = m_InsuranceCombo->GetValue(m_InsuranceCombo->GetCurSel(),0);	// Insurance company
			if (var.vt == VT_I4) {
				nInsuredPartyID = VarLong(var);
				if(nInsuredPartyID == 0) {
					nInsuredPartyID = -1;
				}
			}
		}

		GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(str);
		cy = ParseCurrencyFromInterface(str);

		//////////////////////////////////////////////////////////////
		// Negatize adjustments and refunds
		if (IsAdjustment() || IsRefund()) {
			// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - Need to explicitly define the multiplier as a long
			cy = cy * long(-1);
		}

	}NxCatchAllCall("Error in SavePayment:1", 
		//(e.lally 2007-03-21) PLID 25258 - Don't continue if we throw an error
		return FALSE;);

	//(e.lally 2007-03-21) PLID 25258 - execute sql statements from the rest of the sections in one batch
	CString strSqlBatch2 = BeginSqlBatch();
	long nAuditTransactionID = -1;
	CArray<long, long> aryUpdatedProcInfoIDs;

	try {

		//////////////////////////////////////////////////////////////
		// Let caller see new amount without having to open the payments table
		m_cyFinalAmount = cy;
		strAmount = FormatCurrencyForInterface(cy);
		//used in the SQL update
		CString strSQLAmount = FormatCurrencyForSql(cy);

		//always update the last payment date
		// (j.jones 2007-05-04 09:49) - PLID 23280 - use the GlobalUtils function
		// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
		AddLastPaymentDate(VarLong(m_varPaymentID), (m_date.GetValue()));

		strDate = FormatDateTimeForSql((COleDateTime)m_date.GetValue(), dtoDate);

		// Payment category	//Switched the order around to fix some bugs -BVB
		// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
		if (m_radioAdjustment.GetCheck()) {
			eMethod = EPayMethod::Adjustment;
		}
		// Payment/refund method, all specifics related to payment/refund methods
		else if (m_radioCash.GetCheck()) {
			if (m_radioRefund.GetCheck())
				eMethod = EPayMethod::CashRefund;
			else
				eMethod = EPayMethod::CashPayment;
			var.SetString("1", VT_BSTRT);
		}
		else if (m_radioCheck.GetCheck()) {
			if (m_radioRefund.GetCheck())
				eMethod = EPayMethod::CheckRefund;
			else
				eMethod = EPayMethod::CheckPayment;

			CString strCheck, strBankName, strCheckingAcct, strBankRoutingNumber;
			m_nxeditCheckNumOrCCExpDate.GetWindowText(strCheck);
			m_nxeditBankNameOrCCCardNum.GetWindowText(strBankName);
			m_nxeditAcctNumOrCCNameOnCard.GetWindowText(strCheckingAcct);
			m_nxeditBankNumOrCCAuthNum.GetWindowText(strBankRoutingNumber);

			//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
			AddStatementToSqlBatch(strSqlBatch2, "UPDATE PaymentPlansT SET CheckNo = '%s', [BankNo] = '%s', [CheckAcctNo] = '%s', [BankRoutingNum] = '%s' WHERE PaymentPlansT.ID = %li",
				_Q(strCheck), _Q(strBankName), _Q(strCheckingAcct), _Q(strBankRoutingNumber), m_varPaymentID.lVal);//incase the data is funky
		}
		else if (m_radioCharge.GetCheck()) {
			if (m_radioRefund.GetCheck())
				eMethod = EPayMethod::ChargeRefund;
			else
				eMethod = EPayMethod::ChargePayment;

			// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.
			CString strCardTypeID = "NULL", strCardNumber, strCardName, strExpDate = "NULL", strAuthNumber;
			CString strEncryptedCCNumber = "NULL", strKeyIndex = "NULL", strLast4;

			// (c.haag 2015-08-26) - PLID 67201 - Get fields from the previous ICCP transaction
			if (m_bHasICCPTransaction)
			{
				// 1. Card type
				long nCreditCardID = -1;
				nCreditCardID = m_CardNameCombo->CurSel == -1 ? -1 : VarLong(m_CardNameCombo->GetValue(m_CardNameCombo->GetCurSel(), cccCardID));
				if (nCreditCardID != -1)
					strCardTypeID.Format("%li", nCreditCardID);

				// 2. Last four digits
				m_nxeditCCLast4.GetWindowText(strLast4);

				// 3. Authorization
				m_nxeditBankNumOrCCAuthNum.GetWindowText(strAuthNumber);

				// 4. Token is already saved and is read-only

				// 5. Don't need to remember the transaction type

				// 6. Don't need to remember the merchant account
			}
			// (j.jones 2015-09-30 09:31) - PLID 67167 - these fields are available for non-ICCP
			// CC payments and "Other (Do Not Process)" payments								
			// (c.haag 2015-08-26) - PLID 67201 - Defer to this check if the first two cases are false
			else if (!IsICCPEnabled() || m_radioCCDoNotProcess.GetCheck())
			{
				long nCreditCardID = -1;
				nCreditCardID = m_CardNameCombo->CurSel == -1 ? -1 : VarLong(m_CardNameCombo->GetValue(m_CardNameCombo->GetCurSel(), cccCardID));
				if (nCreditCardID != -1)
					strCardTypeID.Format("%li", nCreditCardID);

				//(e.lally 2007-10-30) PLID 27892 - cache full credit card number, but only display the last 4 digits
				//we need to use the member variable card number since it is not privatized like the control text is.
				//m_nxeditBankNameOrCCCardNum.GetWindowText(strCardNumber);
				m_nxeditAcctNumOrCCNameOnCard.GetWindowText(strCardName);
				m_nxeditCheckNumOrCCExpDate.GetWindowText(strExpDate);
				{
					COleDateTime dt;
					//set the date to be the last date of the exp. month
					m_nxeditCheckNumOrCCExpDate.GetWindowText(strExpDate);
					month = strExpDate.Left(strExpDate.Find("/", 0));
					year = "20" + strExpDate.Right(strExpDate.Find("/", 0));
					if (month == "12") {
						//the method we use to store dates acts funky with December, so
						//we cannot just increase the month by 1. However, we know the last
						//day in December is always 31, so it's an easy fix.
						dt.SetDate(atoi(year), atoi(month), 31);
					}
					else {
						//this method works well for all other months. Set the date to be
						//the first day of the NEXT month, then subtract one day.
						//The result will always be the last day of the month entered.
						COleDateTimeSpan dtSpan;
						dtSpan.SetDateTimeSpan(1, 0, 0, 0);
						dt.SetDate(atoi(year), atoi(month) + 1, 1);
						// (d.thompson 2008-12-16) - PLID 32477 - Only do the subtraction if we have a valid date.  If
						//	the user entered '##/##', this will not make sense.
						if (dt.GetStatus() != COleDateTime::invalid) {
							dt = dt - dtSpan;
						}
					}
					strExpDate = FormatDateTimeForSql(dt, dtoDate);
					// (j.jones 2007-03-13 14:45) - PLID 25183 - ensured we won't save an invalid date
					COleDateTime dtMin;
					dtMin.ParseDateTime("12/31/1899");
					if (dt.m_status == COleDateTime::invalid || dt < dtMin) {
						strExpDate = "Null";
					}
					else
						strExpDate = CString("'") + _Q(strExpDate) + "'";
				}
				m_nxeditBankNumOrCCAuthNum.GetWindowText(strAuthNumber);

				// (j.jones 2015-09-30 09:31) - PLID 67167 - if not using ICCP, the last 4
				// on the card comes from the to-be-encrypted CC number.
				// Otherwise, the Last 4 is a field on "Other (Do Not Process)" payments.					
				if (IsICCPEnabled()) {
					if (m_radioCCDoNotProcess.GetCheck()) {
						m_nxeditCCLast4.GetWindowText(strLast4);
					}
					strEncryptedCCNumber = "NULL";
					strKeyIndex = "NULL";
				}
				//not ICCP
				else {
					strLast4 = m_strCreditCardNumber.Right(4);

					//(e.lally 2007-10-30) PLID 27892 - Changed the card number to use the member variable that holds the full number
					// (a.walling 2007-10-30 18:00) - PLID 27891 - Save the truncated CCNumber and the encrypted SecurePAN
					// (a.walling 2010-03-15 12:35) - PLID 37751 - Use NxCrypto					
					NxCryptosaur.EncryptStringForSql(m_strCreditCardNumber, strKeyIndex, strEncryptedCCNumber);
				}
			}

			//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
			// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.				
			AddStatementToSqlBatch(strSqlBatch2, "UPDATE PaymentPlansT SET CreditCardID = %s, CCNumber = '%s', CCHoldersName = '%s', CCExpDate = %s, CCAuthNo = '%s', SecurePAN = %s, KeyIndex = %s WHERE PaymentPlansT.ID = %li",
				strCardTypeID, _Q(strLast4), _Q(strCardName), strExpDate, _Q(strAuthNumber), strEncryptedCCNumber, strKeyIndex, m_varPaymentID.lVal);//incase the data is funky
		}
		else if (m_radioGift.GetCheck()) 
		{
			// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
			if (m_radioRefund.GetCheck()) 
			{
				eMethod = EPayMethod::GiftCertificateRefund;

				// We're refunding to a gift certificate. Cool. If we're also refunding a gift certificate, we have to transfer its Value balance.

			} 
			else 
			{
				eMethod = EPayMethod::GiftCertificatePayment;

				//we've made a payment for a gift certificate, so this is now "redeemed" by this person.  We 
				//need to update GiftCertificatesT to show the ReceivedBy
				//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
				// (a.walling 2008-05-05 13:34) - PLID 29898 - Use the patient ID
				AddStatementToSqlBatch(strSqlBatch2, "UPDATE GiftCertificatesT SET ReceivedBy = %li WHERE ID = %s AND ReceivedBy IS NULL", m_PatientID, strGiftID);
			}
		}
		//////////////////////////////////////////////////////////////
		// See if it's an adjustment or refund
		else {
			AfxMessageBox("Please verify that this is a payment");
			return FALSE;
		}

		GetDlgItem(IDC_EDIT_DESCRIPTION)->GetWindowText(strDescription);
		if (strDescription.GetLength() == 0)
			strDescription = "(No description)";
		else if (strDescription.GetLength()>200)
			strDescription = strDescription.Left(200);

		// (r.gonet 2015-04-20) - PLID 65326 - Simplified to use a function to get the line item type from the method.
		if (((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->GetCheck() && GetLineItemTypeFromPayMethod(eMethod) == LineItem::Type::Payment)
			strPrePayment = "1";
		else
			strPrePayment = "0";

		////(e.lally 2007-03-21) PLID 25258 - Using an audit transaction
		//long AuditID = -1;

		//set up our audit types
		AuditEventItems aeiItemCreated = aeiPaymentCreated;
		AuditEventItems aeiItemDescription = aeiPaymentDescription;
		AuditEventItems aeiItemDate = aeiPaymentDate;
		AuditEventItems aeiItemAmount = aeiPaymentAmount;
		//TES 4/9/2008 - PLID 29585 - Added Location auditing.
		AuditEventItems aeiItemLocation = aeiPaymentLocation;
		//TES 7/10/2008 - PLID 30676 - Added Provider auditing
		AuditEventItems aeiItemProvider = aeiPaymentProvider;

		if(m_radioAdjustment.GetCheck()) {
			aeiItemCreated = aeiAdjustmentCreated;
			aeiItemDescription = aeiAdjustmentDescription;
			aeiItemDate = aeiAdjustmentDate;
			aeiItemAmount = aeiAdjustmentAmount;
			aeiItemLocation = aeiAdjustmentLocation;
			aeiItemProvider = aeiAdjustmentProvider;
		}
		else if(m_radioRefund.GetCheck()) {
			aeiItemCreated = aeiRefundCreated;
			aeiItemDescription = aeiRefundDescription;
			aeiItemDate = aeiRefundDate;
			aeiItemAmount = aeiRefundAmount;
			aeiItemLocation = aeiRefundLocation;
			aeiItemProvider = aeiRefundProvider;
		}

		// (j.jones 2006-11-03 11:16) - PLID 23176 - cleaned up this rather insane calculation for audit usage
		// (r.gonet 2015-04-20) - PLID 65326 - Use a function to get the line item type from the pay method rather than doing some complex logic each time.
		long nLineItemType = GetLineItemTypeFromPayMethod(eMethod);
		if (nLineItemType == LineItem::Type::Invalid) {
			// By default, we're a payment for auditing purposes.
			nLineItemType = LineItem::Type::Payment;
		}

		if(!bIsNew) 
		{
			// (a.walling 2006-11-15 15:56) - PLID 23562 - Return the previous group and reason codes for auditing purposes
			//TES 4/9/2008 - PLID 29585 - Also the location name.
			//TES 7/10/2008 - PLID 30676 - And the provider name
			// (j.jones 2010-09-23 11:42) - PLID 27917 - group code and reason code are now IDs
			// (c.haag 2014-08-15) - PLID 63379 - In addition to auditing, grab the target line item's patient ID
			// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType
			_RecordsetPtr rsAudit = CreateParamRecordset(R"(
SELECT LineItemT.PatientID AS LineItemPatientID,
LineItemT.Date, LineItemT.Description, LineItemT.Amount, LineItemT.Type, 
PaymentsT.GroupCodeID, PaymentsT.ReasonCodeID, 
AdjustmentGroupCodesT.Code AS GroupCode, AdjustmentReasonCodesT.Code AS ReasonCode, 
LocationID, LocationsT.Name AS LocationName, PaymentsT.ProviderID, 
PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProviderName,
PaymentsT.CCProcessType
FROM LineItemT INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID LEFT JOIN PaymentsT ON 
LineItemT.ID = PaymentsT.ID LEFT JOIN PersonT ON PaymentsT.ProviderID = PersonT.ID 
LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID 
LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID 
WHERE LineItemT.ID = {INT})"
, VarLong(m_varPaymentID)
			);

			// (c.haag 2014-08-15) - PLID 63379 - Safety checks
			if (rsAudit->eof)
			{
				// Line item not found in data
				ThrowNxException("Line item %d not found in data! Called from %s:%d", VarLong(m_varPaymentID), m_strCallingFuncName, m_nCallingLineNumber);
			}
			
			const int nLineItemPatientID = AdoFldLong(rsAudit->Fields, "LineItemPatientID");
			if (nLineItemPatientID != m_PatientID)
			{
				// Patient mismatch
				ThrowNxException("Line item %d patient ID %d doesn't match with this payment's patient ID %d! ActivePatientID = %d. Called from %s:%d",
					VarLong(m_varPaymentID), // Line item
					nLineItemPatientID, // Line item patient ID
					m_PatientID, // Current patient ID
					GetActivePatientID(), // Active patient id
					m_strCallingFuncName, m_nCallingLineNumber);
			}

			//now begin all the audits
			{
				CString oldVal;
				_variant_t var;

				var = rsAudit->Fields->Item["Description"]->Value;
				oldVal = CString(var.bstrVal);
				if(oldVal != strDescription) {
					//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(m_PatientID, m_strPatientName,nAuditTransactionID,aeiItemDescription,m_varPaymentID.lVal,oldVal,strDescription,aepLow);
				}

				var = rsAudit->Fields->Item["Date"]->Value;
				COleDateTime dt = var.date;
				oldVal = FormatDateTimeForSql(dt, dtoDate);
				if(oldVal != strDate) {
					//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(m_PatientID, m_strPatientName,nAuditTransactionID,aeiItemDate,m_varPaymentID.lVal,oldVal,strDate,aepMedium);
				}

				var = rsAudit->Fields->Item["Amount"]->Value;
				oldVal = FormatCurrencyForInterface(var.cyVal);
				if(oldVal != strAmount) {
					//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(m_PatientID, m_strPatientName,nAuditTransactionID,aeiItemAmount,m_varPaymentID.lVal,oldVal,strAmount,aepHigh);
				}

				var = rsAudit->Fields->Item["LocationID"]->Value;
				if(VarLong(var) != LocationID) {
					//TES 4/9/2008 - PLID 29585 - Audit.
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(m_PatientID, m_strPatientName,nAuditTransactionID,aeiItemLocation,m_varPaymentID.lVal,AdoFldString(rsAudit, "LocationName"),strLocationName,aepMedium);
				}

				var = rsAudit->Fields->Item["ProviderID"]->Value;
				if(VarLong(var, -1) != ProviderID) {
					//TES 7/10/2008 - PLID 30676 - Audit
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(m_PatientID, m_strPatientName,nAuditTransactionID,aeiItemProvider,m_varPaymentID.lVal,AdoFldString(rsAudit, "ProviderName","{No Provider}"),strProviderName,aepMedium);
				}

				// (j.jones 2006-11-03 11:16) - PLID 23176 - added audit for type
				long nOldType = AdoFldLong(rsAudit, "Type",1);
				if(nOldType != nLineItemType) {
					//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();

					CString strOldType, strNewType;
					switch(nOldType) {
						case 3:
							strOldType = "Refund";
							break;
						case 2:
						case 0:
							strOldType = "Adjustment";
							break;
						case 1:
						default:
							strOldType = "Payment";
							break;
					}
					switch(nLineItemType) {
						case 3:
							strNewType = "Refund";
							break;
						case 2:
						case 0:
							strNewType = "Adjustment";
							break;
						case 1:
						default:
							strNewType = "Payment";
							break;
					}
					AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aeiPaymentType, m_varPaymentID.lVal, strOldType, strNewType, aepHigh, aetChanged);
				}

				// (a.walling 2006-11-15 15:58) - PLID 23562 - Now audit the change in codes if necessary
				if (m_radioAdjustment.GetCheck()) {
					// these do not apply when not an adjustment!

					// first we will handle the reason code
					// (j.jones 2010-09-23 11:42) - PLID 27917 - these are now nullable IDs
					long nOldReasonCodeID = AdoFldLong(rsAudit, "ReasonCodeID", -1);
					CString strOldReasonCodeNum = AdoFldString(rsAudit, "ReasonCode", "");

					if (nReasonCodeID != nOldReasonCodeID) {
						// the code has changed, so audit this fact
						//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
						if(nAuditTransactionID == -1)
							nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aeiAdjustmentReason, m_varPaymentID.lVal, strOldReasonCodeNum, strReasonCodeNum_ForAudit, aepMedium, aetChanged);
					}

					// now the group code
					// (j.jones 2010-09-23 11:42) - PLID 27917 - these are now nullable IDs
					long nOldGroupCodeID = AdoFldLong(rsAudit, "GroupCodeID", -1);
					CString strOldGroupCodeNum = AdoFldString(rsAudit, "GroupCode", "");

					if (nGroupCodeID != nOldGroupCodeID) {
						// the code has changed, so audit this fact
						//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
						if(nAuditTransactionID == -1)
							nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aeiAdjustmentGroup, m_varPaymentID.lVal, strOldGroupCodeNum, strGroupCodeNum_ForAudit, aepMedium, aetChanged);
					}
				}

				// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType, only audited on ICCP
				if ((IsPayment() || IsRefund()) && m_radioCharge.GetCheck()
					&& eCCProcessType != LineItem::CCProcessType::None
					&& IsICCPEnabled())
				{
					AuditEventItems aei = aeiPaymentCCProcessType;
					if (IsRefund()) {
						aei = aeiRefundCCProcessType;
					}

					LineItem::CCProcessType eOldCCProcessType = (LineItem::CCProcessType)AdoFldLong(rsAudit, "CCProcessType", (long)LineItem::CCProcessType::None);
					if (eCCProcessType != eOldCCProcessType)
					{
						CString strCCProcessTypeOldAuditValue = "None";
						switch (eOldCCProcessType)
						{
						case LineItem::CCProcessType::Swipe:
							strCCProcessTypeOldAuditValue = "Swipe / Dip Card";
							break;
						case LineItem::CCProcessType::CardNotPresent:
							strCCProcessTypeOldAuditValue = "Card Not Present";
							break;
						case LineItem::CCProcessType::DoNotProcess:
							strCCProcessTypeOldAuditValue = "Other (Do not process)";
							break;
						case LineItem::CCProcessType::RefundToOriginal:
							strCCProcessTypeOldAuditValue = "Refund To Original Card";
							break;
						default:
							"None";
							break;
						}

						if (nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						CString strOldAuditDesc;
						strOldAuditDesc.Format("%s %s: %s", strAmount, (eMethod == EPayMethod::ChargeRefund ? "Refund" : "Payment"), strCCProcessTypeOldAuditValue);
						AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aei, VarLong(m_varPaymentID), strOldAuditDesc, strCCProcessTypeNewAuditValue, aepHigh, aetChanged);
					}
				}
			}
			rsAudit->Close();
		}
		else {
			//new pay/adj/ref
			//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
			if(nAuditTransactionID == -1)
				nAuditTransactionID = BeginAuditTransaction();
			CString strAuditDesc;
			// (r.gonet 2015-04-20) - PLID 65326 - Use a function to get the line item type from the pay method rather than in-place logic.
			CString strLineItemType;
			LineItem::Type eLineItemType = GetLineItemTypeFromPayMethod(eMethod);
			if (eLineItemType == LineItem::Type::Payment) {
				strLineItemType = "Payment";
			}if (eLineItemType == LineItem::Type::Adjustment) {
				strLineItemType = "Adjustment";
			}if (eLineItemType == LineItem::Type::Refund) {
				strLineItemType = "Refund";
			} else {
				// Default is a payment.
				strLineItemType == "Payment";

				// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType, audited even on new payments, but only on ICCP
				if ((IsPayment() || IsRefund()) && m_radioCharge.GetCheck()
					&& eCCProcessType != LineItem::CCProcessType::None
					&& IsICCPEnabled()) {

					AuditEventItems aei = aeiPaymentCCProcessType;
					if (IsRefund()) {
						aei = aeiRefundCCProcessType;
					}

					CString strNewAuditDesc;
					strNewAuditDesc.Format("%s %s: %s", strAmount, (eMethod == EPayMethod::ChargeRefund ? "Refund" : "Payment"), strCCProcessTypeNewAuditValue);
					AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aei, VarLong(m_varPaymentID), "", strNewAuditDesc, aepHigh, aetCreated);
				}
			}
			strAuditDesc.Format("%s %s",strAmount,strLineItemType);
			AuditEvent(m_PatientID, m_strPatientName,nAuditTransactionID,aeiItemCreated,m_varPaymentID.lVal,"",strAuditDesc,aepHigh,aetCreated);
		}

		//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
		AddStatementToSqlBatch(strSqlBatch2, "UPDATE LineItemT SET LocationID = %li, Amount = Convert(money,'%s'), [Date] = '%s', Type = %d, Description = '%s', "
			"GiftID = %s, DrawerID = %s WHERE LineItemT.ID = %d",
			LocationID, _Q(strSQLAmount), strDate, nLineItemType, _Q(strDescription), strGiftID, strDrawerID, m_varPaymentID.lVal);//incase the description is funky

		//DRT 4/20/2004 - If NexSpa, save the last cash drawer they used.  We'll want to keep pulling it back up.
		if(m_bHasNexSpa) {
			//only save this if they're in a valid drawer state, otherwise every time they made an adjustment we'd lose our drawer
			if(IsPayment() || (IsRefund() && m_radioCash.GetCheck()))
				SetRemotePropertyText("PaymentLastCashDrawer", strDrawerID, 0, GetCurrentUserName());
		}

		//in case provider is inactive, we don't want to save over it
		CString strProviderUpdate = "";
		if(!m_ProviderCombo->IsComboBoxTextInUse)
			strProviderUpdate.Format("ProviderID = %s,", strProviderID);
		else if(m_ProviderCombo->IsComboBoxTextInUse && bIsNew) {
			strProviderUpdate.Format("ProviderID = %li,", m_nTipInactiveProv);
		}

		//if in the rare case they change a refund or adjustment into a payment, we will need
		//to generate a new PaymentUniqueID. Vice-versa, if they change a payment into
		//an adjustment or refund, we will need to remove the PaymentUniqueID
		CString strPaymentUniqueIDUpdate = "";
		if(m_radioPayment.GetCheck() && IsRecordsetEmpty("SELECT ID FROM PaymentsT WHERE PaymentUniqueID IS NOT NULL AND ID = %li",m_varPaymentID.lVal)) {
			//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
			AddDeclarationToSqlBatch(strSqlBatch2, "DECLARE @UpdatePaymentUniqueID INT\r\n"
				// (j.dinatale 2012-08-08 11:35) - PLID 52032 - fixed an issue with PaymentUniqueID and the possibility of a warning being thrown due to a NULL value being eliminated by the aggregate function
				// (j.armen 2013-06-29 15:34) - PLID 57375 - PaymentsT.PaymentUniqueID now gets it's ID from an identity seeded table
				"INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
				"SET @UpdatePaymentUniqueID = SCOPE_IDENTITY() ");
			strPaymentUniqueIDUpdate = "PaymentUniqueID = @UpdatePaymentUniqueID,";
		}
		//Rather than check if the PaymentUniqueID is already null, we will just always set it for non-payments.
		// (d.thompson 2012-08-15) - PLID 52141 - Logic fail - we must check for non-payment before changing the uniqueID.  This code
		//	was causing the PaymentUniqueID value to get wiped out on all new payments for the past 5 or so years (since 3/21/2007).
		else if(!m_radioPayment.GetCheck()) {
			strPaymentUniqueIDUpdate.Format("PaymentUniqueID = NULL,");
		}
		else {
			//It's a payment, but it already has a unique ID.  Don't do a thing.
		}

		// (j.jones 2005-04-19 10:32) - PLID 16063 - we save the QuoteID again because now it is possible
		// (though unlikely) for the payment to become unlinked due to our changes
		CString strQuoteID = "NULL";
		if(m_QuoteID != -1)
			strQuoteID.Format("%li",m_QuoteID);

		// (a.walling 2006-11-15 12:42) - PLID 23550 - Update the group and reason codes
		//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
		// (j.jones 2010-09-23 11:42) - PLID 27917 - group and reason codes are now nullable IDs
		// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
		// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType
		//****REMEMBER: All new fields also need to be supported in FinancialCorrection.cpp****//
		AddStatementToSqlBatch(strSqlBatch2, "UPDATE PaymentsT SET %s %s PaymentGroupID = %li, InsuredPartyID = %li, PayMethod = %li, "
			"PrePayment = %s, CashReceived = %s, QuoteID = %s, ReasonCodeID = %s, GroupCodeID = %s, CCProcessType = %s "
			"WHERE PaymentsT.ID = %li",
			strProviderUpdate, strPaymentUniqueIDUpdate, iPaymentGroupID, nInsuredPartyID,
			((eMethod == EPayMethod::LegacyAdjustment) ? EPayMethod::Adjustment : eMethod), strPrePayment, strCashReceived, strQuoteID, strReasonCodeID, strGroupCodeID, strCCProcessTypeForSql,
			VarLong(m_varPaymentID));
		
		//TES 8/6/2007 - PLID 20720 - If this is a new prepayment that is linked to a quote, and that quote is the 
		// active quote for any PICs, then attach this prepayment to those PICs.
		// (j.jones 2009-08-25 11:55) - PLID 31549 - we will also link to the given m_nProcInfoID, if we have one		
		if(bIsNew && strPrePayment == "1" && (m_QuoteID != -1 || m_nProcInfoID != -1)) {

			// (j.jones 2009-08-25 11:16) - PLID 31549 - find whether the given quote
			// is on a ProcInfo record, or the given ProcInfoID doesn't have this payment already
			// (it is ok if one of these IDs is -1)
			_RecordsetPtr rsProcInfo = CreateParamRecordset("SELECT ID FROM ProcInfoT "
				"WHERE (ActiveQuoteID = {INT} OR ID = {INT}) "
				"AND ID NOT IN (SELECT ProcInfoID FROM ProcInfoPaymentsT WHERE PayID = {INT})",
				m_QuoteID, m_nProcInfoID, VarLong(m_varPaymentID));
			while(!rsProcInfo->eof) {

				long nProcInfoID = AdoFldLong(rsProcInfo, "ID");
				aryUpdatedProcInfoIDs.Add(nProcInfoID);

				AddStatementToSqlBatch(strSqlBatch2, "INSERT INTO ProcInfoPaymentsT (ProcInfoID, PayID) "
					"VALUES (%li, %li)", nProcInfoID, VarLong(m_varPaymentID));

				rsProcInfo->MoveNext();
			}
			rsProcInfo->Close();
		}

			
		if(m_nProcInfoID != -1) {
			//are we already linking to 
		}

		//Batch continues into next section

	}NxCatchAllCall("Error in SavePayment:2",
		//(e.lally 2007-03-21) PLID 25258 - Don't continue if we throw an error
		if(nAuditTransactionID != -1)
			RollbackAuditTransaction(nAuditTransactionID);
		return FALSE;);

	//(e.lally 2007-03-23) PLID 25258 - With batch execution, we need to expand the scope of some variables for the
	//delayed execution.
	// (j.jones 2010-08-04 17:23) - PLID 38613 - replaced a boolean with the amount to shift
	COleCurrency cyTotalAmtToShiftForCopay = COleCurrency(0,0);

	//(e.lally 2010-09-03) PLID 39971 - Renamed this for better clarity that is is the patient responsibility that was already on the bill.
	COleCurrency cyExistingPatientResp;
	// (j.jones 2009-08-25 15:03) - PLID 35337 - we need to find the insured party to shift from
	try{
		//(e.lally 2010-09-03) PLID 39971 - Check if the preference for setting aside co-pays is on
		BOOL bShiftCopayAmount = (GetRemotePropertyInt("ShiftCoPayAmount",0,0,"<None>",TRUE) == 1);

		// DT, 1/6: if this is from a bill, go ahead and auto apply for Receipts purposes
		if (m_varBillID.vt == VT_I4 && m_ApplyOnOK) {
			
			if(m_bIsCoPay && nInsuredPartyID == -1) {
				COleCurrency cyCharges, cyInsurance;
				GetBillTotals(m_varBillID.lVal, m_PatientID, &cyCharges, 0,0,0, &cyInsurance);
				cyExistingPatientResp = cyCharges - cyInsurance;

				// (j.jones 2010-08-04 17:03) - PLID 38613 - we were given what the bill's co-insurance was,
				// and it would have already been shifted to patient, it needs to be excluded from this
				// calculation such that we acquire the copay amount in addition to the co-insurance, with
				// no overlapping resp.

				// (j.jones 2011-11-09 16:17) - PLID 46348 - renamed to m_cyTotalCoInsuranceAmt to clearly reflect that this
				// represents the total co-insurance for the entire bill, it could be for multiple insurance resps.
				if(m_cyTotalCoInsuranceAmt > COleCurrency(0,0)) {

					//***this is not normal***
					//this code is only hit if a bill has both co-insurance and a co-payment,
					//which is not typical (but we fully support it)
					
					cyExistingPatientResp -= m_cyTotalCoInsuranceAmt;

					//cyPatient was the current patient responsibility, as we have already
					//shifted the co-insurance prior to opening the payment dialog.
					//The expected result here is that cyPatient was most likely equal to
					//m_cyCoInsuranceAmt, and we've now reduced it to zero. We now need to shift
					//the co-payment amount from insurance to patient.
					
					if(cyExistingPatientResp < COleCurrency(0,0)) {
						//should be impossible
						ASSERT(FALSE);
						cyExistingPatientResp = COleCurrency(0,0);
					}

					//Now, the amount we need to shift to patient is actually the copay amount,
					//but it really depends on what they entered. If they entered less than the
					//default copay amount, we only shift what they entered, but if they entered more
					//money than the default copay amount, we can allow the overage to apply to the
					//co-insurance resp. before acquiring more resp. from insurance.

					if(m_cyFinalAmount <= m_cyCopayAmount) {
						//normal case, they entered a payment that is the same amount
						//as the copay amount (or less)

						//(e.lally 2010-09-03) Take the expected co-pay amount and subtract the patient's charge resp to find
						//the amount that may still need to be set aside (shifted resp) to cover the entire co-pay.
						const COleCurrency ccyAddlToSetAsideToCoverCoPay = m_cyCopayAmount - cyExistingPatientResp;
						//(e.lally 2010-09-03) Also take the actual payment (finalAmount) and subtract the patient's charge resp to find
						//the amount that may still need to be set aside (shifted resp) to cover the entire payment.
						const COleCurrency ccyAddlToSetAsideToCoverActualPayment = m_cyFinalAmount - cyExistingPatientResp;

						//(e.lally 2010-09-03) PLID 39971 - If the preference is on and the payment was less than the expected co-pay,
						//prompt to see if the user wants just the payment amount as the patient responsibility or the full co-pay amount.
						if(bShiftCopayAmount && m_cyFinalAmount < m_cyCopayAmount){
							if(ccyAddlToSetAsideToCoverCoPay > COleCurrency(0,0)){
								if(IDYES == MsgBox(MB_YESNO, "This %s payment is less than the %s co-pay amount for this patient. "
									"Would you like to leave the remaining %s as the patient's responsibility?", 
									FormatCurrencyForInterface(m_cyFinalAmount),
									FormatCurrencyForInterface(m_cyCopayAmount),
									FormatCurrencyForInterface(m_cyCopayAmount - m_cyFinalAmount) )){
										//(e.lally 2010-09-03) PLID 39971 - Make sure we have enough Patient Responsibility
										//to equal the full copay
										cyTotalAmtToShiftForCopay = ccyAddlToSetAsideToCoverCoPay;
								}
								else{
									//(e.lally 2010-09-03) PLID 39971 - They said No, so make sure we have enough Patient Responsibility
									//to equal the actual payment
									cyTotalAmtToShiftForCopay = ccyAddlToSetAsideToCoverActualPayment;
								}
							}
							else{
								//(e.lally 2010-09-03) PLID 39971 - Make sure we have enough Patient Responsibility
								//to equal the actual payment. The result will likely be negative (meaning we already have enough resp set aside)
								cyTotalAmtToShiftForCopay = ccyAddlToSetAsideToCoverActualPayment;
							}
						}
						else{
							//(e.lally 2010-09-03) PLID 39971 - we only need to shift the payment total to patient
							//because the preference for shifting copays was off or the payment was equal to the co-pay anyways.
							cyTotalAmtToShiftForCopay = ccyAddlToSetAsideToCoverActualPayment;
						}
					}
					else if(m_cyCopayAmount > cyExistingPatientResp) {
						//they entered a payment higher than the copay amount,
						//so we need to shift only the copay amount, provided
						//that there isn't enough balance left after we
						//deducted the co-insurance from the available resp
						cyTotalAmtToShiftForCopay = m_cyCopayAmount - cyExistingPatientResp;
					}
				}
				//***this is the normal case***
				//no co-insurance exists, so set our amount to shift as the true amount needed
				else if(m_cyFinalAmount > cyExistingPatientResp) {
					//the payment is higher than the current patient responsibility,
					//we need to shift enough money to accommodate this payment

					//(e.lally 2010-09-03) Take the expected co-pay amount and subtract the patient's charge resp to find
					//the amount that may still need to be set aside (shifted resp) to cover the entire co-pay.
					const COleCurrency ccyAddlToSetAsideToCoverCoPay = m_cyCopayAmount - cyExistingPatientResp;
					//(e.lally 2010-09-03) Also take the actual payment (finalAmount) and subtract the patient's charge resp to find
					//the amount that may still need to be set aside (shifted resp) to cover the entire payment.
					const COleCurrency ccyAddlToSetAsideToCoverActualPayment = m_cyFinalAmount - cyExistingPatientResp;

					//(e.lally 2010-09-03) PLID 39971 - If the preference is on and the payment was less than the expected co-pay,
					//prompt to see if the user wants just the payment amount as the patient responsibility or the full co-pay amount.
					if(bShiftCopayAmount && m_cyFinalAmount < m_cyCopayAmount){
						//Take the expected co-pay amount and subtract the patient's charge resp to find
						//the amount they may still need to be set aside to cover the entire co-pay.
						COleCurrency cyPatRespStillNeedToSetAsideToCoverCoPay = m_cyCopayAmount - cyExistingPatientResp;
						COleCurrency cyPatRespStillNeedToSetAsideToCoverActualPayment = m_cyFinalAmount - cyExistingPatientResp;
						if(IDYES == MsgBox(MB_YESNO, "This %s payment is less than the %s co-pay amount for this patient. "
							"Would you like to leave the remaining %s as the patient's responsibility?", 
							FormatCurrencyForInterface(m_cyFinalAmount),
							FormatCurrencyForInterface(m_cyCopayAmount),
							FormatCurrencyForInterface(m_cyCopayAmount - m_cyFinalAmount) )){
								//(e.lally 2010-09-03) PLID 39971 - Make sure we have enough Patient Responsibility
								//to equal the full copay
								cyTotalAmtToShiftForCopay = ccyAddlToSetAsideToCoverCoPay;
						}
						else{
							//(e.lally 2010-09-03) PLID 39971 - They said No, so make sure we have enough Patient Responsibility
							//to equal the actual payment
							cyTotalAmtToShiftForCopay = ccyAddlToSetAsideToCoverActualPayment;
						}
					}
					else{
						//(e.lally 2010-09-03) PLID 39971 - we only need to shift the payment total to patient
						//because the preference for shifting copays was off or the payment was greater than or equal to the co-pay anyways.
						cyTotalAmtToShiftForCopay = ccyAddlToSetAsideToCoverActualPayment;
					}
				}

				if(cyTotalAmtToShiftForCopay > COleCurrency(0,0)) {

					if(m_parypInsuranceCoPayApplyList == NULL || m_parypInsuranceCoPayApplyList->GetSize() == 0) {
						
						// (j.jones 2011-11-09 16:33) - PLID 46348 - this means we were asked for a copay,
						// but were not given any apply information, and should only be possible if the preference is
						// enabled to always prompt for a copay, when the charge in question doesn't really need one

						//only thing we can do is shift all possible resp. to patient

						long nBillID = VarLong(m_varBillID);
						COleCurrency cyRemainingToShift = cyTotalAmtToShiftForCopay;

						_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyID, Sum(ChargeRespT.Amount) AS TotalResp "
							"FROM InsuredPartyT "
							"INNER JOIN ChargeRespT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID "
							"INNER JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID "
							"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
							"WHERE PatientID = {INT} AND RespTypeID <> -1 "
							"AND ChargesT.BillID = {INT} "
							"GROUP BY ChargeRespT.InsuredPartyID, RespTypeT.Priority "
							"ORDER BY (CASE WHEN RespTypeT.Priority = -1 THEN 1 ELSE 0 END) ASC, RespTypeT.Priority ASC",
							m_PatientID, nBillID);
						while(!rs->eof && cyRemainingToShift > COleCurrency(0,0)) {

							long nInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value);
							COleCurrency cyAmtToShift = VarCurrency(rs->Fields->Item["TotalResp"]->Value, COleCurrency(0,0));
							if(cyAmtToShift > cyRemainingToShift) {
								cyAmtToShift = cyRemainingToShift;
								cyRemainingToShift = COleCurrency(0,0);
							}
							else {
								cyRemainingToShift -= cyAmtToShift;
							}
							
							//this will try to grab all the resp it can from all charges on the bill
							// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
							// (j.jones 2013-08-21 13:04) - PLID 39987 - changed to call EnsureInsuranceResponsibility,
							// which will do nothing if the desired resp. already exists
							EnsureInsuranceResponsibility(nBillID, m_PatientID, nInsuredPartyID, -1, "Bill", cyAmtToShift, "while saving a copayment", COleDateTime::GetCurrentTime());						
							
							rs->MoveNext();
						}
						rs->Close();
					}
				}
			}
		}

		//batch continues into next section

	}NxCatchAllCall("Error in SavePayment:3",
		//(e.lally 2007-03-21) PLID 25258 - Don't continue if we throw an error
		if(nAuditTransactionID != -1)
			RollbackAuditTransaction(nAuditTransactionID);
		return FALSE;);

	// (a.walling 2008-05-05 13:35) - PLID 29898 - All references to GetActivePatientName replaced with m_strPatientName
	try {

		//TES 3/25/2015 - PLID 65174 - Refunds can be tipped now
		//TES 4/2/2015 - PLID 65174 - Make sure they have the preference and permission (the tip might have been added to a payment, and the payment changed to a refund)
		if (IsPayment() || (IsRefund() && GetRemotePropertyInt("AllowTipRefunds", 0, 0, "<None>") && (GetCurrentUserPermissions(bioRefundTips) & sptWrite))) {	//adjustments and refunds don't get tipped
			//loop through all tips

			//drawer setup
			CString strDrawer = "NULL";
			if(IsDlgButtonChecked(IDC_TIPS_IN_DRAWER)) {
				long nSel = m_pDrawerList->GetCurSel();
				if(nSel != sriNoRow) {
					long nID = VarLong(m_pDrawerList->GetValue(nSel, 0));
					if(nID != -1) {
						strDrawer.Format("%li", nID);
					}
				}
			}

			//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
			AddDeclarationToSqlBatch(strSqlBatch2, "DECLARE @PaymentTipID INT \r\n");
			for(int i = 0; i < m_pTipList->GetRowCount(); i++) {
				long nID = VarLong(m_pTipList->GetValue(i, tlcID));
				long nProvID = VarLong(m_pTipList->GetValue(i, tlcProvID));
				COleCurrency cyAmt = VarCurrency(m_pTipList->GetValue(i, tlcAmount));
				// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
				long nPayMethodInt = VarLong(m_pTipList->GetValue(i, tlcMethod));
				EPayMethod ePayMethod = AsPayMethod(nPayMethodInt);

				//-1 id is a new one, any other is existing
				long nPaymentID = VarLong(m_varPaymentID);	//this will throw an exception if the payment isn't filled for any reason

				// (a.walling 2007-09-20 16:03) - PLID 27468 - Add to count of total tips
				Info.nTotalTips++;
				// the pay method MUST be valid.
				// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
				if (ePayMethod == EPayMethod::Invalid) {
					ThrowNxException("Invalid pay method %li", nPayMethodInt);
				}

				if(nID == -1) {
					// (a.walling 2007-09-20 16:01) - PLID 27468 - Add to count of new tips with this pay method
					// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
					Info.arNewTips[(long)ePayMethod-1]++;
					CString str;
					//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
					AddStatementToSqlBatch(strSqlBatch2, "SET @PaymentTipID = (SELECT (COALESCE(MAX(ID),0) + 1) FROM PaymentTipsT) \r\n"
						"INSERT INTO PaymentTipsT (ID, PaymentID, ProvID, Amount, PayMethod, DrawerID) "
						"values (@PaymentTipID, %li, %li, convert(money, '%s'), %li, %s);\r\n", 
						nPaymentID, nProvID, FormatCurrencyForSql(cyAmt), (long)ePayMethod, strDrawer);

					//for auditing
					//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
					str.Format("Tip added for %s.", FormatCurrencyForInterface(cyAmt));
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aeiTipCreated, nPaymentID, "", str, aepMedium, aetCreated);
				}
				else {
					// (a.walling 2007-09-21 16:27) - PLID 27468 - Add the tip ID to the existing IDs array
					Info.AddExistingTipID(nID);

					_RecordsetPtr prs = CreateRecordset("SELECT ProvID, Amount, PayMethod, DrawerID FROM PaymentTipsT WHERE ID = %li", nID);
					if(!prs->eof) {
						long nOldProv = AdoFldLong(prs, "ProvID");
						// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
						long nOldMethodInt = AdoFldLong(prs, "PayMethod");
						EPayMethod eOldMethod = AsPayMethod(nOldMethodInt);
						COleCurrency cyOldAmt = AdoFldCurrency(prs, "Amount");
						long nOldDrawerID = AdoFldLong(prs, "DrawerID", -1);
						CString strOldDrawer = "NULL";
						if(nOldDrawerID != -1)
							strOldDrawer.Format("%li", nOldDrawerID);

						bool bUser = false, bMethod = false, bAmt = false, bDrawer = false;	//have we changed?
						CString str = "UPDATE PaymentTipsT SET ";
						CString temp;
						//we only need to update what has changed
						if(nOldProv != nProvID) {
							//need to update the provider
							temp.Format("ProvID = %li,", nProvID);
							str += temp;
							bUser = true;

							//lookup the old & new providers
							//(e.lally 2007-03-21) PLID 25258 - Move these into one query
							_RecordsetPtr prsAudit = CreateRecordset("SELECT (SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = %li) AS OldProvider, "
								"(SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li) AS NewProvider", nOldProv, nProvID);
							CString strProv1, strProv2;
							if(!prsAudit->eof){
								strProv1 = AdoFldString(prsAudit, "OldProvider");
								strProv2 = AdoFldString(prsAudit, "NewProvider");
							}
							prsAudit->Close();
							//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
							if(nAuditTransactionID == -1)
								nAuditTransactionID = BeginAuditTransaction();
							AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aeiTipProvider, nPaymentID, strProv1, strProv2, aepMedium, aetChanged);
						}
						// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
						if(eOldMethod != ePayMethod) {
							//need to update the method
							temp.Format(" PayMethod = %li,", (long)ePayMethod);
							str += temp;
							bMethod = true;

							//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
							if(nAuditTransactionID == -1)
								nAuditTransactionID = BeginAuditTransaction();
							// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
							// (r.gonet 2015-04-20) - PLID 65326 - Use a function rather to get the paymethod name rather than doing a switch.
							CString strOldPaymentType = GetPayMethodName(eOldMethod, "Charge", false, true);
							CString strPaymentType = GetPayMethodName(ePayMethod, "Charge", false, true);
							AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aeiTipMethod, nPaymentID, strOldPaymentType, strPaymentType, aepMedium, aetChanged);
						}
						if(cyOldAmt != cyAmt) {
							// (a.walling 2007-09-20 16:02) - PLID 27468 - Add to count of changed tips with this pay method
							// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
							Info.arChangedTips[(long)ePayMethod-1]++;
							Info.arModifiedTipIDs.Add(nID);
							//need to update the amt
							temp.Format(" Amount = convert(money, '%s'),", FormatCurrencyForSql(cyAmt));
							str += temp;
							bAmt = true;

							//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
							if(nAuditTransactionID == -1)
								nAuditTransactionID = BeginAuditTransaction();
							AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aeiTipAmount, nPaymentID, FormatCurrencyForInterface(cyOldAmt), FormatCurrencyForInterface(cyAmt), aepMedium, aetChanged);
						}

						if(GetDlgItem(IDC_CASH_DRAWER_LIST)->IsWindowEnabled()) {
							//If a drawer is closed and we're editing it, we disable the drawer
							//In that case, never change this.
							if(strOldDrawer != strDrawer) {
								//need to update the drawer
								temp.Format(" DrawerID = %s ", strDrawer);
								str += temp;
								bDrawer = true;
							}
						}

						if(bAmt || bMethod || bUser || bDrawer) {
							//something changed, we must update & audit
							str.TrimRight(",");
							temp.Format("WHERE ID = %li;\r\n", nID);
							str += temp;
							//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
							AddStatementToSqlBatch(strSqlBatch2, str);
						}
					}
				}
			}

			if(nAuditTransactionID == -1)
				nAuditTransactionID = BeginAuditTransaction();

			//and now we have to take care of any tips which might have been deleted while they were editing this payment
			for(i = 0; i < m_aryDeleted.GetSize(); i++) {
				long nTipID = m_aryDeleted.GetAt(i);
				// (a.walling 2007-09-28 10:57) - PLID 27468 - Update our save changes info
				Info.arDeletedTipIDs.Add(nTipID);

				// (a.walling 2007-09-28 10:54) - PLID 27561 - Don't do all this if the ID is -1
				if (nTipID != -1) {
					//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
					AddStatementToSqlBatch(strSqlBatch2, "DELETE FROM PaymentTipsT WHERE ID = %li;\r\n", nTipID);

					//for auditing
					_RecordsetPtr prs = CreateRecordset("SELECT Amount FROM PaymentTipsT WHERE ID = %li", nTipID);
					if(!prs->eof) {
						str.Format("Tip deleted for %s.", FormatCurrencyForInterface(AdoFldCurrency(prs, "Amount")));
						//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
						AuditEvent(m_PatientID, m_strPatientName, nAuditTransactionID, aeiTipDeleted, nTipID, "", str, aepMedium, aetDeleted);
					}
				}
			}

		}
		//TES 4/2/2015 - PLID 65174 - Delete the tips if this is a refund, and they don't have the permission or the preference
		else if (IsAdjustment() || (IsRefund() && (!GetRemotePropertyInt("AllowTipRefunds", 0, 0, "<None>") || !(GetCurrentUserPermissions(bioRefundTips) & sptWrite)))) {
			//if there are any tips, we need to delete them.  They have been appropriately warned.
			//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
			AddStatementToSqlBatch(strSqlBatch2, "DELETE FROM PaymentTipsT WHERE PaymentID = %li", VarLong(m_varPaymentID));
		}
		else {
			//not a payment, not a refund, not an adjustment ... what in the world is this?
			ASSERT(FALSE);
		}

		//(e.lally 2007-03-21) PLID 25258 - execute sql statements from the rest of the sections in a 2nd batch
		//now update all of these things
		if(!strSqlBatch2.IsEmpty()){
			ExecuteSqlBatch(strSqlBatch2);
			if(m_varBillID.vt == VT_I4 && m_ApplyOnOK){				
				//now shift the difference from the insurance to the Patient

				// (j.jones 2007-08-03 10:38) - PLID 25844 - Added flag to follow the copay				
				// (j.jones 2009-08-25 14:53) - PLID 35337 - fixed to use the nSourceInsuredPartyID on this payment
				// (j.jones 2010-08-03 15:48) - PLID 39938 - replaced bFollowCoPay with the pay group ID we made the copay for,
				// only sent if this payment is in fact a copay
				// (j.jones 2010-08-04 17:23) - PLID 38613 - replaced a boolean with the amount to shift
				// (j.jones 2011-11-09 16:36) - PLID 46348 - this now needs to shift a different amount per charge
				if(m_bIsCoPay && cyTotalAmtToShiftForCopay > COleCurrency(0,0)
					&& m_parypInsuranceCoPayApplyList != NULL && m_parypInsuranceCoPayApplyList->GetSize() > 0) {
					
					for(int indexInsCoPayApplyList=0; indexInsCoPayApplyList < m_parypInsuranceCoPayApplyList->GetSize() && cyTotalAmtToShiftForCopay > COleCurrency(0,0); indexInsCoPayApplyList++) {

						//Definition of InsuranceCoPayApplyList:

						//This object will represent each insured party's copay
						//responsibilities on a bill. The array of CoPayApplyInformation
						//will track each unique amount to apply to either one charge,
						//or an array of charges.
						
						//Example A: Medicare requires a $20.00 copay, and two charges require
						//a copay, so aryCoPayApplyInformation will have one entry for $20.00,
						//and an array of two charge IDs that the $20.00 can apply to.

						//Example B: Medicare requires a 20% copay, and two charges require
						//a copay, so aryCoPayApplyInformation will have two entries,
						//one for each charge and the 20% amount required for that charge.

						InsuranceCoPayApplyList *icpaiApplyList = (InsuranceCoPayApplyList*)m_parypInsuranceCoPayApplyList->GetAt(indexInsCoPayApplyList);
						long nCurInsuredPartyID = icpaiApplyList->nInsuredPartyID;

						for(int indexCoPayApplies=0; indexCoPayApplies < icpaiApplyList->aryCoPayApplyInformation.GetSize() && cyTotalAmtToShiftForCopay > COleCurrency(0,0); indexCoPayApplies++) {

							//Definition of CoPayApplyInformation:

							//This object will either represent a specific amount
							//to apply to one charge, or an amount that can be applied
							//across multiple charges. The latter will happen if the 
							//copay amount is a fixed $ amount (typical) instead of % (rare),
							//and the amount to apply can go towards any of these charges.

							//This is tracked per InsuranceCoPayApplyList, which
							//is itself tracked per insured party on the bill.
							
							//Example A: Medicare requires a $20.00 copay, and two charges require
							//a copay, so this object will have be for $20.00, and the array will
							//have two charge IDs that the $20.00 can apply to.

							//Example B: Medicare requires a 20% copay, and two charges require
							//a copay, so this object will exist twice in InsuranceCoPayApplyList.
							//It will exist once per charge, with the copay amount being 20% of that
							//charge, and only one charge ID per object.
						
							CoPayApplyInformation *cpaiApplyInfo = (CoPayApplyInformation*)icpaiApplyList->aryCoPayApplyInformation.GetAt(indexCoPayApplies);

							if(cpaiApplyInfo->cyCoPayAmount > COleCurrency(0,0)) {

								COleCurrency cyRemAmountToApply = cpaiApplyInfo->cyCoPayAmount;
								if(cyRemAmountToApply > cyTotalAmtToShiftForCopay) {
									//the intended amount to apply is greater than the amount
									//we have left to apply (likely the payment is less than
									//initially requested), so we can only apply the rest of
									//cyAmtToShift
									cyRemAmountToApply = cyTotalAmtToShiftForCopay;
									cyTotalAmtToShiftForCopay = COleCurrency(0,0);
								}
								else {
									//normal case, we'll apply the existing value for cyRemAmountToApply
									cyTotalAmtToShiftForCopay -= cyRemAmountToApply;
								}

								//now we need to shift, but only for specific charges
								for(int indexChargeIDs=0; indexChargeIDs < cpaiApplyInfo->aryChargeIDs.GetSize() && cyRemAmountToApply > COleCurrency(0,0); indexChargeIDs++) {
									
									long nChargeID = cpaiApplyInfo->aryChargeIDs.GetAt(indexChargeIDs);
									COleCurrency cyInsResp = GetChargeInsResp(nChargeID, nCurInsuredPartyID);
									
									COleCurrency cyAmtToShiftForCharge = COleCurrency(0,0);
									if(cyInsResp < cyRemAmountToApply) {
										//cannot shift more than there is in insurance resp
										cyAmtToShiftForCharge = cyInsResp;
										cyRemAmountToApply -= cyInsResp;
									}
									else {
										//we can shift the entire amount needed
										cyAmtToShiftForCharge = cyRemAmountToApply;
										cyRemAmountToApply = COleCurrency(0,0);
									}

									// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
									// (j.jones 2013-08-21 13:04) - PLID 39987 - changed to call EnsureInsuranceResponsibility,
									// which will do nothing if the desired resp. already exists
									EnsureInsuranceResponsibility(nChargeID, m_PatientID, nCurInsuredPartyID, -1, "Charge", cyAmtToShiftForCharge, "while saving a copayment", COleDateTime::GetCurrentTime());
								}
							}
						}
					}
				}

				// (j.jones 2011-03-23 17:23) - PLID 42936 - do not warn for allowables if this function is not shifting resps
				AutoApplyPayToBill(m_varPaymentID.lVal,	iPatientID, "Bill", m_varBillID.lVal,FALSE,FALSE, m_PromptToShift, TRUE, !m_PromptToShift);				
			}
			else if (m_varChargeID.vt == VT_I4 && m_ApplyOnOK) {
				// (j.jones 2011-03-23 17:23) - PLID 42936 - do not warn for allowables if this function is not shifting resps
				AutoApplyPayToBill(m_varPaymentID.lVal,	iPatientID, "Charge", m_varChargeID.lVal,FALSE,FALSE, m_PromptToShift, TRUE, !m_PromptToShift);
			}
			
			// (j.gruber 2007-07-30 15:46) - PLID 26704 - added ability to apply
			// (j.jones 2015-09-30 10:37) - PLID 67172 - the payment to apply to is now a struct			
			if (m_PaymentToApplyTo.nPaymentID != -1 && m_ApplyOnOK) {
				//apply the refund to the payment
				AutoApplyPayToPay(VarLong(m_varPaymentID), iPatientID, "Refund", m_PaymentToApplyTo.nPaymentID);
			}
		}
		m_aryDeleted.RemoveAll();

		// (r.gonet 2015-04-29 18:46) - PLID 65717 - Saving the payment and the tips has been successful. Now see if there is balance on the gift certificate we are refunding. If so, offer to transfer it.
		if (GetPayMethod() == EPayMethod::GiftCertificateRefund && m_radioRefundToNewGiftCertificate.GetCheck() == BST_CHECKED
			&& (m_nPayToApplyToGiftID != -1 || m_nOriginalGiftID != -1) && nNewGiftID != -1		
			&& CheckCurrentUserPermissions(bioTransferGiftCertificateBalances, sptWrite)) 
		{
			// So there are two cases that can happen here. One is if they are creating the gift certificate refund for the first time
			// and the other is if they are changing the gift certificate that gets credited.
			long nGCTransferSourceID = (m_nOriginalGiftID != -1 ? m_nOriginalGiftID : m_nPayToApplyToGiftID);
			NexTech_Accessor::_GetGiftCertificatePaymentListIDLookupInputPtr pLookup(__uuidof(NexTech_Accessor::GetGiftCertificatePaymentListIDLookupInput));
			pLookup->ID = _bstr_t(FormatString("%li", nGCTransferSourceID));
			pLookup->IncludeExpiredAndVoidedGCs = VARIANT_TRUE;
			NexTech_Accessor::_FindGiftCertificatePaymentListResultPtr pResults =
				GetAPI()->GetGiftCertificateInfo_PaymentList(GetAPISubkey(), GetAPILoginToken(), pLookup);
			if (pResults != NULL && pResults->Result != NULL) {
				COleCurrency cyBalance = AsCurrency(pResults->Result->Balance);
				if (cyBalance > g_ccyZero) {
					// (r.gonet 2015-04-29 18:46) - PLID 65717 - There's a balance. Let's see if they want to put it towards the new gift certificate.
					if (IDYES == MessageBox(FormatString("This refund is being applied to Gift Certificate %s, which has a balance of %s. Would you like to transfer the balance to the new Gift Certificate %s?"
						, (LPCTSTR)pResults->Result->GiftID, FormatCurrencyForInterface(cyBalance), strNewGiftCertNumber), "Balance Transfer", MB_YESNO | MB_ICONQUESTION)) {

						{
							CWaitCursor pWait;
							TransferGiftCertificateAmount(nGCTransferSourceID, nNewGiftID, cyBalance);
						}

						// (r.gonet 2015-05-11 10:46) - PLID 65392 - Void the original if they desire.
						if (GetCurrentUserPermissions(bioVoidGiftCertificates, sptDynamic0) &&
							IDYES == MessageBox("Would you like to void the original gift certificate?", "Void Original", MB_YESNO | MB_ICONQUESTION)) {
							// flag as void
							ExecuteParamSql("UPDATE GiftCertificatesT SET Voided = 1 WHERE ID = {INT}", nGCTransferSourceID);

							_RecordsetPtr prsSourceGiftInfo = CreateParamRecordset(
								"SELECT GiftCertificatesT.GiftID, GiftCertificatesT.PurchasedBy, PersonT.FullName "
								"FROM GiftCertificatesT "
								"INNER JOIN PersonT ON GiftCertificatesT.PurchasedBy = PersonT.ID "
								"WHERE GiftCertificatesT.ID = {INT}", 
								nGCTransferSourceID);
							if (!prsSourceGiftInfo->eof) {
								CString strSourceGiftID = AdoFldString(prsSourceGiftInfo->Fields, "GiftID");
								long nPurchasedByPersonID = AdoFldLong(prsSourceGiftInfo->Fields, "PurchasedBy");
								CString strPurchasedByFullName = AdoFldString(prsSourceGiftInfo->Fields, "FullName");
								// add voided action to audit trail
								CString newVal = FormatString("Voided (%s)", strSourceGiftID);
								CString oldVal = "Active";

								AuditEvent(nPurchasedByPersonID, strPurchasedByFullName, nAuditTransactionID, aeiGiftCStatus, nGCTransferSourceID, oldVal, newVal, aepHigh, aetChanged);
							}
						}
					} else {
						// (r.gonet 2015-04-29 18:46) - PLID 65717 - Don't transfer anything then.
					}
				} else {
					// (r.gonet 2015-04-29 18:46) - PLID 65717 - Nothing left to transfer.
				}
			} else {
				// (r.gonet 2015-04-29 18:46) - PLID 65717 - Why weren't we able to find the gift certificate?
			}
		} else {
			// (r.gonet 2015-04-29 18:46) - PLID 65717 - Not refunding a gift certificate or not refunding to a new gift certificate.
		}
		
		
		//Run our auditing transaction
		if(nAuditTransactionID != -1)
			CommitAuditTransaction(nAuditTransactionID);

	} NxCatchAllCall("Error saving payment tips", 
		//(e.lally 2007-03-21) PLID 25258 - Don't continue if we throw an error
		if(nAuditTransactionID != -1)
			RollbackAuditTransaction(nAuditTransactionID);
		return FALSE;);

	try {
		if(bIsNew) {

			// (j.jones 2009-08-25 11:16) - PLID 31549 - update each open PIC we have already added to, if any are open
			for(int i=0; i<aryUpdatedProcInfoIDs.GetSize(); i++) {
				long nProcInfoID = aryUpdatedProcInfoIDs.GetAt(i);
				GetMainFrame()->BroadcastPostMessageToPICs(NXM_PIC_RELOAD_PAYS, (WPARAM)nProcInfoID, 0, NULL);
			}
			
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, iPatientID, COleDateTime::GetCurrentTime(), VarLong(m_varPaymentID), true, -1);
		}

		// (a.walling 2007-09-20 16:31) - PLID 27468 - Copy the info structure if they want it.
		if (pSaveInfo) {
			*pSaveInfo = Info; // default copy constructor
		}
	}NxCatchAll("Error in SavePayment:4");

	CClient::RefreshTable(NetUtils::PatBal, iPatientID);
	LogDetail("Payment %d updated", m_varPaymentID.lVal);

	// (d.thompson 2009-06-30) - PLID 34687 - Attempt to process credit card payments, if necessary.  This is 
	//	done after the entire payment is saved and all the data assured.  I tried to maintain past PLID 
	//	comments where possible.  This was moved from the previous "Save & Process" button for CC setups.
	try {
		//Ensure this is going to be a live CC processing action.
		// (j.jones 2015-09-30 08:58) - PLID 67157 - this does not apply to ICCP
		if(m_radioCharge.GetCheck() && !m_radioAdjustment.GetCheck() && !IsICCPEnabled() && IsDlgButtonChecked(IDC_PROCESS_CC) 
			&& m_bProcessCreditCards_NonICCP && m_boIsNewPayment)
		{
			//Now Authorize it
			// (a.walling 2007-09-21 16:00) - PLID 27468 - Send the payment info pointer
			if (!QBMS_ProcessTransaction(&Info)) {
				//The transaction has failed.  But this payment was already saved.  We need the give the user some options.
				if(AfxMessageBox("The credit card transaction has failed, but this payment was already saved.  Would you like to "
					"delete this record from your system?\r\n\r\n"
					" - Click YES if you wish to remove this record entirely.\r\n"
					" - Click NO if you wish to leave this record in the system.\r\n", MB_YESNO) == IDYES)
				{
					//Delete it
					DeletePayment(VarLong(m_varPaymentID));

					//Now that it's gone, we have to quit.  This is a little sketchy to be closing up shop here, 
					//	so all callers need to be aware of it.
					//unregister for barcode messages
					if(GetMainFrame()) {
						if(!GetMainFrame()->UnregisterForBarcodeScan(this))
							MsgBox("Error unregistering for barcode scans.");
					}
					CDialog::OnOK();
					return FALSE;
				}
				else {
					//Leave the payment as-is.
					return TRUE;
				}
			}

			NXDATALIST2Lib::IRowSettingsPtr pReportRow;
			pReportRow = m_pReportList->CurSel;
			//we tested above that this was valid, so the last check is just to avoid confusing errors.
			if (pReportRow == NULL) {
				AfxThrowNxException("Unable to find report row for receipt after processing credit card.");
			}
			long nReportID = VarLong(pReportRow->GetValue(0), -2);
			long nReportNumber = VarLong(pReportRow->GetValue(1), -2);

			if (nReportID == -2 || nReportID == -3 || nReportID == 585) {
				RunSalesReceipt((COleDateTime)m_date.GetValue(), nReportNumber, VarLong(m_varPaymentID), FALSE, nReportID, m_PatientID, this); 
			}
			else {
				//see if they have a receipt printer setip
				if (GetMainFrame()->CheckPOSPrinter() && GetPropertyInt("POSPrinter_UseDevice", 0, 0, TRUE)) {
					RunSalesReceipt((COleDateTime)m_date.GetValue(), -3, VarLong(m_varPaymentID), FALSE, -3, m_PatientID, this); 
				}
				else {
					//run the default sales report
					RunSalesReceipt((COleDateTime)m_date.GetValue(), 0, VarLong(m_varPaymentID), FALSE, -2, m_PatientID, this); 
				}
			}
		}
		else {
			//No need to process, they lack the license or the checkbox.
		}
	}
	NxCatchAll("Error in SavePayment:6");

	// (z.manning 2016-02-15 14:01) - PLID 68258 - We now attempt to process an ICCP transaction
	// after the payment has already been saved. This way we prevent situations where (due to some
	// sort of unfortuantely timed connection issue or something) a payment gets processed but
	// no Nextech payment exists.
	try
	{
		if (ShouldPromptForICCPAuthorization())
		{
			if (!ICCPAuthorize())
			{
				return FALSE;
			}
		}
	}
	NxCatchAllCall(__FUNCTION__ + CString(" - ICCP processing error"),
		return FALSE;
	);

	return TRUE;
}

// (z.manning 2016-02-15 11:13) - PLID 68258 - Moved this logic to its own function
BOOL CPaymentDlg::ShouldPromptForICCPAuthorization()
{
	LineItem::CCProcessType eCCProcessType = GetCCProcessType();
	if (!m_bHasAnyTransaction
		&& LineItem::CCProcessType::None != eCCProcessType
		&& LineItem::CCProcessType::DoNotProcess != eCCProcessType
		&& m_radioCharge.GetCheck() == BST_CHECKED)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// (z.manning 2016-02-15 11:23) - PLID 68258 - Moved all the ICCP processing logic to a single function
// now that we do it all after saving the payment.
BOOL CPaymentDlg::ICCPAuthorize()
{
	if (m_varPaymentID.vt != VT_I4)
	{
		ThrowNxException("Trying to authorize ICCP payment that has not yet been saved");
		return FALSE;
	}
	
	LineItem::CCProcessType eCCProcessType = GetCCProcessType();
	_variant_t varCCProcessTypeForSql = (eCCProcessType == LineItem::CCProcessType::None ? g_cvarNull : (long)eCCProcessType);

	long nCCPaymentIDToRefund = -1;
	BOOL bWasSignedElectronically = FALSE;

	CString strPaymentTotal;
	GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(strPaymentTotal);
	COleCurrency cyCCTransactionAmount = ParseCurrencyFromInterface(strPaymentTotal);

	CString strCreditCardToken;
	COleDateTime dtCardExpiration = g_cdtInvalid;
	if ((IsPayment() || IsRefund()) && m_radioCharge.GetCheck())
	{
		if (eCCProcessType == LineItem::CCProcessType::CardNotPresent) {
			// (z.manning 2015-08-27 15:02) - PLID 67232 - Get the token and exp date as they 
			// may have selected to use a saved card.
			NXDATALIST2Lib::IRowSettingsPtr pCardOnFileRow = m_CCCardOnFileCombo->CurSel;
			if (pCardOnFileRow != NULL) {
				strCreditCardToken = VarString(pCardOnFileRow->GetValue(ppcCreditCardToken), "");
				dtCardExpiration = VarDateTime(pCardOnFileRow->GetValue(ppcExpDate), g_cdtInvalid);
			}
		}
	}

	// (c.haag 2015-08-18) - PLID 67203 - We can't check m_boIsNewPayment because this may be an existing
	// "Other (Do not process)" payment that the user decided to change into a Swipe / DIP or possibly a cash payment
	// that the user wants to change to a charge payment and do a transaction for. So we'll instead check m_bHasAnyTransaction
	// to verify that no credit transaction has been done.
	NexTech_Accessor::_AuthorizeCreditCardPaymentResultPtr pAuthorizationResult;
	if (ShouldPromptForICCPAuthorization())
	{
		// (c.haag 2015-08-18) - PLID 67203 - Include tips in the transaction amount
		// (j.jones 2015-09-30 14:50) - PLID 67203 - supported refunded tips
		for (int i = 0; i < m_pTipList->GetRowCount(); i++)
		{
			COleCurrency cyAmt = VarCurrency(m_pTipList->GetValue(i, tlcAmount));
			long nPayMethod = VarLong(m_pTipList->GetValue(i, tlcMethod));
			// (j.jones 2015-09-30 14:50) - PLID 67203 - supported refunded tips
			if ((int)EPayMethod::ChargePayment == nPayMethod || (int)EPayMethod::ChargeRefund == nPayMethod)
			{
				//refunded tips are negative
				if ((int)EPayMethod::ChargeRefund == nPayMethod && cyAmt < COleCurrency(0, 0)) {
					cyAmt = -cyAmt;
				}
				cyCCTransactionAmount += cyAmt;
			}
		}

		// CardConnect takes in a 32-bit int including the decimal places so do a sanity check here to prevent exceptions.
		unsigned long nAmount = abs((int)(OleCurrencyToDouble(cyCCTransactionAmount) * 100));
		if (nAmount >= INT_MAX) {
			MessageBox("Amount is too large for credit card processing.", NULL, MB_ICONERROR);
			return FALSE;
		}

		//   1. We're doing a refund without being associated with any payment
		//   2. We're doing a refund of a non-ICCP payment
		//   3. We're doing a refund of an ICCP payment using another credit card
		//
		//   We handle these cases as so:
		//
		//   1. m_PaymentToApplyTo.nPaymentID is <= 0. We will do an authorization for a negative amount.
		//   2. m_PaymentToApplyTo.nPaymentID is > 0 and m_PaymentToApplyTo.bHasICCPTransaction is false.
		//		We will do an authorization for a negative amount and record it in CardConnect_CreditTransactionRefundT.
		//		This works because CardConnect_CreditTransactionRefundT.ID actually references PaymentsT.
		//   3. m_PaymentToApplyTo.nPaymentID is > 0 and m_PaymentToApplyTo.bHasICCPTransaction is true.
		//      We will do an authorization for a negative amount and record it in CardConnect_CreditTransactionRefundT.
		if (IsRefund())
		{
			if (m_PaymentToApplyTo.nPaymentID <= 0)
			{
				// Handle case 1: Do an authorization for a negative amount. 
				cyCCTransactionAmount = -cyCCTransactionAmount;
			}
			else if (!m_PaymentToApplyTo.bHasICCPTransaction)
			{
				// Handle case 2: Do an authorization for a negative amount. 
				cyCCTransactionAmount = -cyCCTransactionAmount;
			}
			else
			{
				// Handle case 3: Do an authorization for a negative amount. 
				cyCCTransactionAmount = -cyCCTransactionAmount;
			}
			nCCPaymentIDToRefund = m_PaymentToApplyTo.nPaymentID;
		}

		// (c.haag 2015-08-18) - PLID 67203 - Handle authorizing "Swipe / DIP Card" credit card payments
		// (c.haag 2015-08-24) - PLID 67202 - Handle authorizing "Card Not Present" credit card payments
		if (LineItem::CCProcessType::Swipe == eCCProcessType
			|| LineItem::CCProcessType::CardNotPresent == eCCProcessType)
		{

			// (z.manning 2015-08-25 16:30) - PLID 67231 - Do we want to create a payment profile
			// (z.manning 2015-08-27 15:52) - PLID 67232 - Don't create a payment profile if we're
			// using one to pay.
			BOOL bCreatePaymentProfile = (strCreditCardToken.IsEmpty() &&
				m_checkAddCCToProfile.IsWindowVisible() &&
				m_checkAddCCToProfile.GetCheck() == BST_CHECKED &&
				CheckCurrentUserPermissions(bioPaymentProfile, sptWrite, FALSE, 0, TRUE)
				);

			// (z.manning 2015-08-31 09:49) - PLID 67230 - If this is a payment and the card is present
			// then prompt for a signature.
			BOOL bGetSignature = (!IsRefund() && eCCProcessType == LineItem::CCProcessType::Swipe);

			// (c.haag 2015-08-18) - PLID 67203 - Perform the swipe
			std::set<CString> setExistingTokens;
			GetNonExpiredCreditCardTokens(setExistingTokens);
			LPDISPATCH lpdispResult = nullptr;
			if (!AuthorizeCreditCardTransaction(
				this, VarLong(m_varPaymentID), nCCPaymentIDToRefund, LineItem::CCProcessType::Swipe == eCCProcessType, bGetSignature, &setExistingTokens,
				m_PatientID, GetCurrentlySelectedMerchantAccountID(), cyCCTransactionAmount, dtCardExpiration, bCreatePaymentProfile,
				strCreditCardToken, OUT lpdispResult, OUT bWasSignedElectronically))
			{
				// Swipe failed or was cancelled
				return FALSE;
			}
			pAuthorizationResult = lpdispResult;
		}
		// (c.haag 2015-08-25) - PLID 67197 - Handle refunds to original payments
		else if (LineItem::CCProcessType::RefundToOriginal == eCCProcessType)
		{
			LPDISPATCH lpdispResult = nullptr;
			if (!RefundAuthorizedPayment(m_PaymentToApplyTo.nPaymentID, cyCCTransactionAmount, lpdispResult))
			{
				// Refund failed or was cancelled
				return FALSE;
			}
			pAuthorizationResult = lpdispResult;
			nCCPaymentIDToRefund = m_PaymentToApplyTo.nPaymentID;
		}
		else
		{
			ThrowNxException(FormatString("Unhandled eCCProcessType: %d", eCCProcessType));
		}
	}

	if (pAuthorizationResult == nullptr) {
		return FALSE;
	}

	// (z.manning 2016-02-15 15:04) - PLID 68258 - We just processed the ICCP payment so ensure this flag is reset.
	m_bNewICCPPaymentSavedButNotYetProcessed = FALSE;

	// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.
	CString strAuthNumber, strLast4;

	// (c.haag 2015-08-26) - PLID 67201 - Get fields from the ICCP transaction that just took place
	{
		// (c.haag 2015-08-26) - PLID 67201 - From the token response, update the:
		//		1. Card Type
		//		2. Last 4 on Card
		//		3. Authorization # fields (PaymentPlansT.CCAuthNo)
		//		4. The token (already saved in another database table)
		//		5. Remember selected Transaction Type on payment per user
		//		6. Remember selected merchant account on payment per user

		// 1. Card type. Here we just get the card name from the result since we don't track card type codes in data
		CString strCardName = (LPCTSTR)pAuthorizationResult->CCName;

		// Select the card name in the card name combo. If it doesn't exist, just assign the combo text
		if (-1 == m_CardNameCombo->SetSelByColumn(cccCardName, (LPCTSTR)strCardName))
		{
			m_CardNameCombo->PutComboBoxText(_bstr_t(strCardName));
		}

		// 2. Assign the last four digits
		strLast4 = (LPCTSTR)pAuthorizationResult->CCLastFour;

		// 3. Assign the authorization number.
		strAuthNumber = (LPCTSTR)pAuthorizationResult->authorizationCode;

		// 4. The token (already saved in another database table by the API)

		// Ensure the card name is in data and assign it to a local variable that will be used
		// in writing to PaymentPlansT
		// (s.tullis 2016-02-12 15:07) - PLID 68130 - It was possible to have duplicate cardnames causing an error here
		// handle that scenario
		ExecuteParamSql(R"(
DECLARE @creditCardID INT
SET @creditCardID = (SELECT Min(ID) FROM CreditCardNamesT WHERE CardName = {STRING})
IF (@creditCardID IS NULL) BEGIN
	INSERT INTO CreditCardNamesT (CardName, CardType, Inactive)
		VALUES ({STRING}, {CONST_INT}, CONVERT(BIT,0))
	SET @creditCardID = @@IDENTITY
END

UPDATE PaymentsT SET CCProcessType = {VT_I4}
WHERE PaymentsT.ID = {INT}

UPDATE PaymentPlansT SET CreditCardID = @creditCardID, CCNumber = {STRING}, CCAuthNo = {STRING}
WHERE PaymentPlansT.ID = {INT}
)"
, strCardName
, strCardName, PAYMENTECH_ProcessingCardTypes::pctCredit
, varCCProcessTypeForSql, VarLong(m_varPaymentID)
, strLast4, strAuthNumber, VarLong(m_varPaymentID)
);

		// 5. Remember selected Transaction Type on payment per user
		SetRemotePropertyInt("LastICCPPaymentTransactionType", (int)eCCProcessType, 0, GetCurrentUserName());

		// 6. Remember selected merchant account on payment per user
		SetRemotePropertyInt("LastICCPPaymentMerchantAccountID", (int)GetCurrentlySelectedMerchantAccountID(), 0, GetCurrentUserName());
	}

	// (c.haag 2015-08-18) - PLID 67203 - If we authorized a credit for the payment, record it to data
	if (LineItem::CCProcessType::RefundToOriginal == eCCProcessType)
	{
		// (c.haag 2015-08-25) - PLID 67197 - If we refunded the payment, record it to data
		RecordAuthorizedPaymentRefund(pAuthorizationResult,
			GetCurrentlySelectedMerchantAccountID(), cyCCTransactionAmount, _bstr_t(strCreditCardToken), VarLong(m_varPaymentID), nCCPaymentIDToRefund, bWasSignedElectronically);
	}
	else
	{
		RecordAuthorizedTransaction(pAuthorizationResult,
			GetCurrentlySelectedMerchantAccountID(), cyCCTransactionAmount, _bstr_t(strCreditCardToken), VarLong(m_varPaymentID), nCCPaymentIDToRefund, bWasSignedElectronically);

		// (z.manning 2015-09-09 10:19) - PLID 67224 - Need to prompt to print receipts
		PromptToPrintICCPReceipts(this, VarLong(m_varPaymentID));
	}

	return TRUE;
}

void CPaymentDlg::OnExportToQuickBooks()
{
	if(!CheckCurrentUserPermissions(bioQuickbooksLink,sptView))
		return;

	try {

		COleCurrency cyAmount;
		CString str;
		GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(str);
		if (str.GetLength() == 0) {
			MsgBox("Please fill in the 'Total Amount' box.");
			return;
		}

		if(!IsValidCurrencyText(str)) {
			MsgBox("Please fill correct information in the 'Total Amount' box.");
			return;
		}

		cyAmount = ParseCurrencyFromInterface(str);

		if(cyAmount == COleCurrency(0,0)) {
			MsgBox("Zero dollar payments are not allowed in Quickbooks.");
			return;
		}

		if(m_radioPayment.GetCheck() == 1 && m_radioGift.GetCheck() == 1) {
			MsgBox("Gift Certificate payments are not allowed in QuickBooks.\n"
				"However, the payment that purchased this Gift Certificate is allowed in QuickBooks.");
			return;
		}

		if(m_radioPayment.GetCheck() == 1 && m_radioCheck.GetCheck() == 1) {
			CString strCheckNo = "";
			GetDlgItemText(IDC_EDIT_CHECKNUM_OR_CCEXPDATE, strCheckNo);
			if(strCheckNo.GetLength() > 11) {
				MsgBox("The check number on this payment is too long for QuickBooks.\n\n"
					"QuickBooks only supports check numbers up to 11 characters long.");
				return;
			}
		}

		CWaitCursor pWait1;

		// Open the qb connection
		IQBSessionManagerPtr qb = QB_OpenSession();

		if(qb == NULL)
			return;

		str = "Before exporting this payment to Quickbooks, it will be saved and closed.\n"
				"Do you want to send this payment to QuickBooks?";

		CString strQuickbooksID = "";

		if(m_varPaymentID.vt == VT_I4) {
			//it is a saved payment
			_RecordsetPtr rs = CreateRecordset("SELECT QuickbooksID FROM PaymentsT WHERE ID = %li AND QuickbooksID Is Not Null",m_varPaymentID.lVal);
			if(!rs->eof) {
				strQuickbooksID = AdoFldString(rs, "QuickbooksID","");
				if(strQuickbooksID != "") {
					//we do have a quickbooks ID
					if(QB_VerifyPaymentTxnID(qb,strQuickbooksID)) {
						//it does exist in quickbooks
						//right now, we cannot modify payments in Quickbooks
						AfxMessageBox("This payment already exists in Quickbooks, and cannot be re-sent or modified.");
						qb->EndSession();
						return;
					}
					else
						//it doesn't exist - hmmmmm...
						str = "This payment has previously been sent to Quickbooks, but no longer exists in its database.\n"
							"Do you want to re-send this payment to QuickBooks?";
				}
			}
			rs->Close();
		}

		int nResult = MessageBox(str, NULL, MB_YESNO|MB_ICONQUESTION);
		if (nResult != IDYES) {
			qb->EndSession();
			return;
		}

		//because, for now, we can't modify existing ones, we can only get this far if the payment doesn't exist in QB
		//take this out once we can modify existing ones
		strQuickbooksID = "";

		if (!m_bSaved) {
			// (a.walling 2007-10-04 14:35) - PLID 9801 - Gather info from SaveChanges
			CPaymentSaveInfo Info;
			if (FALSE == SaveChanges(&Info)) {
				qb->EndSession();
				return;
			}

			// (a.walling 2007-10-04 14:35) - PLID 9801 - Open the cash drawer if desired
			OpenCashDrawerIfNeeded(&Info, TRUE);
		}

		CWaitCursor pWait2;

		// Make sure the customer exists
		CString strCustomerListID = "", strEditSequence = "";
		if (!QB_GetCustomerListID(qb, m_PatientID, strCustomerListID, strEditSequence)) {
			// Customer doesn't exist, shall we auto-create it?
			int nResult = MessageBox(
				"The customer '" + m_strPatientName + "' could not be found in QuickBooks.  "
				"If you proceed with the payment export, this customer will be created automatically first.", NULL, MB_OKCANCEL);
			if (nResult == IDOK) {
				// Good, go ahead and create the customer				
				if(!QB_CreateCustomer(qb, m_PatientID, strCustomerListID)) {
					qb->EndSession();
					return;
				}
				else
					ExecuteSql("UPDATE PatientsT SET SentToQuickbooks = 1 WHERE PersonID = %li",m_PatientID);
			} else {
				// The user chose to cancel
				qb->EndSession();
				return;
			}
		}

		//we have a link now between providers and deposit accounts, so if all providers from all payments have
		//been linked to a deposit account, we won't need to prompt for one

		CString strPaymentAccount = "";

		_RecordsetPtr rs = CreateRecordset("SELECT QBooksAcctID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID LEFT JOIN QBooksAcctsToProvidersT ON PaymentsT.ProviderID = QBooksAcctsToProvidersT.ProviderID WHERE QBooksAcctID Is NOT NULL AND PaymentsT.ID = %li",m_varPaymentID.lVal);
		if(!rs->eof) {
			strPaymentAccount = AdoFldString(rs, "QBooksAcctID","");			
		}
		rs->Close();

		if(strPaymentAccount == "" || !QB_VerifyPaymentAccountID(qb, strPaymentAccount)) {
			// Choose the payment account			
			if (!QB_ChooseIndividualAccount(qb, strPaymentAccount, this)) {
				// The user canceled, so don't proceed
				qb->EndSession();
				return;
			}
		}

		CWaitCursor pWait3;

		// Finally, send the payment
		CString strPaymentTxnID = strQuickbooksID;
		if(strPaymentTxnID == "") {
			//create new payment
			if(QB_CreatePayment(qb, strCustomerListID, strPaymentTxnID, m_varPaymentID.lVal, TRUE, strPaymentAccount, FALSE, COleDateTime::GetCurrentTime()))
				ExecuteSql("UPDATE PaymentsT SET QuickbooksID = '%s', SentToQB = 1 WHERE ID = %li",_Q(strPaymentTxnID),m_varPaymentID.lVal);
		}
		else {
			//if we ever have the ability to modify existing payments, we would do it here
		}

		BOOL bWarnedCCTip = FALSE;

		if(GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true) == 1) {
			//now send the tips
			// (r.gonet 2015-04-20) - PLID 65326 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT PaymentTipsT.ID, PatientID, PaymentTipsT.QuickbooksID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID INNER JOIN PaymentTipsT ON PaymentsT.ID = PaymentTipsT.PaymentID WHERE PaymentTipsT.PaymentID = {INT} AND PaymentTipsT.PayMethod <> PaymentsT.PayMethod ",m_varPaymentID.lVal);
			while(!rs->eof) {
				// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
				if(ReturnsRecords("SELECT PaymentTipsT.ID FROM PaymentTipsT INNER JOIN PaymentsT ON PaymentTipsT.PaymentID = PaymentsT.ID "
					"WHERE PaymentTipsT.ID = {INT} AND PaymentTipsT.PayMethod = {CONST_INT} AND PaymentsT.PayMethod <> {CONST_INT}",
					AdoFldLong(rs, "ID"), (long)EPayMethod::ChargePayment, (long)EPayMethod::ChargePayment)) {
					if(!bWarnedCCTip) {
						AfxMessageBox("The tip payment could not be exported, because the tip is identified as being from a credit card, \n"
							"and the payment is identified as being a cash or check. Quickbooks will not accept credit card line items \n"
							"without a credit card type associated with them.");
						bWarnedCCTip = TRUE;
					}
					rs->MoveNext();
					continue;
				}

				CString strPaymentTxnID = AdoFldString(rs, "QuickBooksID","");

				if(strPaymentTxnID == "") {
					if(QB_CreatePayment(qb, strCustomerListID, strPaymentTxnID, AdoFldLong(rs, "ID"), TRUE, strPaymentAccount, TRUE, COleDateTime::GetCurrentTime()))
						ExecuteSql("UPDATE PaymentTipsT SET QuickbooksID = '%s', SentToQB = 1 WHERE ID = %li",_Q(strPaymentTxnID),AdoFldLong(rs, "ID"));
				}

				rs->MoveNext();
			}
		}

		qb->EndSession();

	} NxCatchAllCall("CPaymentDlg::OnExportToQuickBooks", {
		SAFE_SET_PROGRESS(pProgress, "ERROR encountered while trying to create a payment.");
		(void)0;
	});

	//unregister for barcode messages
	if(GetMainFrame()) {
		if(!GetMainFrame()->UnregisterForBarcodeScan(this))
			MsgBox("Error unregistering for barcode scans.");
	}

	CDialog::OnOK();
}

BOOL CPaymentDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_SYSKEYDOWN) {
		switch (pMsg->wParam) {
		case 'O': OnOK(); return TRUE;
		case 'C':
		case 'X': OnCancel(); return TRUE;
		case 'R': OnClickBtnPrint(); return TRUE;
		case 'P': 
			if (GetDlgItem(IDC_RADIO_PAYMENT)->IsWindowEnabled()) {
				GetDlgItem(IDC_RADIO_PAYMENT)->SetFocus();
			}return TRUE;
		case 'T':
			if (GetDlgItem(IDC_EDIT_TOTAL)->IsWindowEnabled()) {
				GetDlgItem(IDC_EDIT_TOTAL)->SetFocus();
			}return TRUE;
		case 'D': GetDlgItem(IDC_COMBO_DESCRIPTION)->SetFocus(); return TRUE;
		case 'E': GetDlgItem(IDC_COMBO_INSURANCE)->SetFocus(); return TRUE;
		case 'M': if (GetDlgItem(IDC_RADIO_CASH)->IsWindowEnabled()) {
			GetDlgItem(IDC_RADIO_CASH)->SetFocus();
				  } return TRUE;
		case 'Q':
			OnExportToQuickBooks();
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CPaymentDlg::OnDestroy() 
{
	try {
		GetMainFrame()->EnableHotKeys();
	}NxCatchAll(__FUNCTION__);
	CNxDialog::OnDestroy();
}

void CPaymentDlg::OnClickBtnPreview() 
{
	try {
		PrintReceipt(true);	
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnClickBtnPrint() 
{
	try {
		PrintReceipt(false);
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2007-03-13 12:27) - PLID 25019 - the the report print according to the drop down
void CPaymentDlg::PrintReceipt(bool bPreview)
{
	CString filter, param;

	// Save any changes to the payment, or make a new one
	// if it does not exist.

	AfxMessageBox("This payment will be saved and closed.");

	if (!m_bSaved) {
		// (a.walling 2007-10-04 14:35) - PLID 9801 - Gather info from SaveChanges
		CPaymentSaveInfo Info;
		if (FALSE == SaveChanges(&Info))	return;
		// (a.walling 2007-10-04 14:35) - PLID 9801 - Open the cash drawer if desired
		OpenCashDrawerIfNeeded(&Info, TRUE);
	}

	
	//save the last selection
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pReportList->CurSel;

	if (pRow == NULL) {
		AfxMessageBox("Please select a report to run from the reports drop down");
		return;
	}

	// (a.walling 2008-02-21 10:43) - PLID 29043 - unregister for barcode messages
	if(GetMainFrame()) {
		if(!GetMainFrame()->UnregisterForBarcodeScan(this))
			MsgBox("Error unregistering for barcode scans.");
	}

	long nID = VarLong(pRow->GetValue(0), -2);
	long nNumber = VarLong(pRow->GetValue(1), -2);
	
	CString strCombo;
	strCombo.Format("%li:%li", nID, nNumber);
	SetRemotePropertyText("PayReceiptLastReportRan", strCombo, 0, GetCurrentUserName());

	//check to see what the ID is
	// (j.gruber 2007-03-29 14:41) - PLID 9802 - sales receipt printer report
	// (j.gruber 2007-05-08 09:55) - PLID 9802 - take out 587 since we are doing it all POS 
	if (nID == -2 || nID == 585 || nID == -3 /*|| nID == 587*/) {
		//system sales receipt

		//popup the sales dialog
			
		//this dialog will run the report
		RunSalesReceipt(COleDateTime(m_date.GetValue()), nNumber, VarLong(m_varPaymentID), bPreview, nID, m_PatientID, (CWnd*)this);

		// (a.walling 2007-10-08 16:24) - PLID 9801 - Still see if we need to open the drawer
		// because of a previous payment (when using save and add new)

		OpenCashDrawerIfNeeded(NULL, TRUE);
		
		// (j.gruber 2007-07-17 15:57) - PLID 26686 - moved the logic to globalreportutils
		EndDialog(bPreview ? RETURN_PREVIEW : RETURN_PRINT);
		return;

	}


	
	CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(235)];
	infReport.nExtraID = m_varPaymentID.lVal;

	//setup the params
	//crystal report
	CPtrArray params;
	CRParameterInfo *tmpParam;

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "ReceiptShowChargeInfo";
	tmpParam->m_Data = GetRemotePropertyInt("ReceiptShowChargeInfo", 0, 0, "<None>", true) ? "true" : "false";
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "ReceiptShowTax";
	tmpParam->m_Data = GetRemotePropertyInt("ReceiptShowTax", 1, 0, "<None>", true) ? "true" : "false";
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "ReceiptCustomInfo";
	tmpParam->m_Data = GetRemotePropertyText("ReceiptCustomInfo", "", 0, "<None>", true);
	params.Add((void *)tmpParam);
	
	CPrintInfo prInfo;
	if (!bPreview) {

		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		prInfo.m_bPreview = false;
		prInfo.m_bDirect = false;
		prInfo.m_bDocObject = false;
		if (prInfo.m_pPD) delete prInfo.m_pPD;
		prInfo.m_pPD = dlg;
	}

	// (j.gruber 2007-03-15 13:43) - PLID 25019 - we aren't just running the default report anymore
	if (nNumber > 0) {

		//its a custom report, look up the filename
		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Filename from CustomReportsT WHERE ID = 235 AND Number = %li", nNumber);
		CString strFileName;
		if (! rs->eof) {
			strFileName = AdoFldString(rs, "FileName", "");
			infReport.nDefaultCustomReport = nNumber;
		}
		else {
			strFileName = "PayReceipts";
		}
		rs->Close();

		if (bPreview) {
			infReport.ViewReport("Payment Dialog", strFileName, &params, TRUE, this);
		}
		else {
			infReport.ViewReport("Payment Dialog", strFileName, &params, FALSE, this, &prInfo);
		}
	}
	else {

		//it's a system report!
		if (bPreview) {
			infReport.ViewReport("Payment Dialog", "PayReceipts", &params, TRUE, this);
		}
		else {
			infReport.ViewReport("Payment Dialog", "PayReceipts", &params, FALSE, this, &prInfo);
		}
	}

	// (a.walling 2007-10-08 16:24) - PLID 9801 - Still see if we need to open the drawer
	// because of a previous payment (when using save and add new)

	OpenCashDrawerIfNeeded(NULL, TRUE);

	EndDialog(bPreview ? RETURN_PREVIEW : RETURN_PRINT);
	ClearRPIParameterList(&params);	//DRT - PLID 18085 - Cleanup after ourselves
}

void CPaymentDlg::OnEditPayDesc() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 2, m_DescriptionCombo, "Edit Combo Box").DoModal();
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnEditPayCat() 
{
	try {
		long nCurSel = m_CategoryCombo->GetCurSel();
		long nCurCatID;
		// if there is something selected, then save it so we can set it back after editing the categories
		if(nCurSel != sriNoRow){
			nCurCatID= m_CategoryCombo->GetValue(nCurSel, tlcID);			
		}
		else{
			nCurCatID = -1;
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		// (j.gruber 2012-11-15 13:37) - PLID 53752 - use the paycateogire dialog we have
		CPayCatDlg dlg(this);
		if (dlg.DoModal()) {
			//we need to requery the list
			m_CategoryCombo->Requery();			
			m_CategoryCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}
		//CEditComboBox(this, 3, m_CategoryCombo, "Edit Combo Box").DoModal();

		IRowSettingsPtr pRow;
		pRow = m_CategoryCombo->GetRow(-1);
		pRow->PutValue(0,long(0));
		pRow->PutValue(1,_bstr_t("<No Category Selected>"));
		m_CategoryCombo->AddRow(pRow);

		// set the selection back to what it was before
		m_CategoryCombo->TrySetSelByColumn(tlcID, nCurCatID);
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnEditCardName() 
{
	try {
		if(m_CardNameCombo->CurSel != -1){
			m_nCreditCardID = VarLong(m_CardNameCombo->GetValue(m_CardNameCombo->CurSel, cccCardID));
		}
		// (e.lally 2007-07-12) PLID 26590 - The long member card ID needs to be reset so the trysetsel
			//doesn't accidently add an inactive card somehow
		else{
			m_nCreditCardID = -1;
		}
		// (e.lally 2007-07-12) PLID 26590 - Switched this to the new credit card editor dlg
		CEditCreditCardsDlg dlg(this);
		if(IDOK == dlg.DoModal()){
			// - requery the cc combo if they OK'd to save (even if no changes were actually made).
			m_CardNameCombo->Requery();
		}
		
		m_CardNameCombo->TrySetSelByColumn(cccCardID, m_nCreditCardID);

	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnRequeryFinishedComboInsurance(short nFlags) 
{
	try {

		// (j.jones 2007-04-19 15:27) - PLID 25711 - The priority is in the datalist,
		//so we do not need to query the database, simply loop through and color accordingly.

		IRowSettingsPtr pRow;
		for(int i=0;i<m_InsuranceCombo->GetRowCount();i++) {
			pRow = m_InsuranceCombo->GetRow(i);
			long nPriority = VarLong(pRow->GetValue(rlcPriority), -1);
			if(nPriority == 1) //primary
				pRow->PutForeColor(RGB(192,0,0));
			else if(nPriority == 2) //secondary
				pRow->PutForeColor(RGB(0,0,128));
			else if(nPriority == -1 || nPriority == 999999) //inactive
				pRow->PutForeColor(RGB(127,127,127));
		}

		if(m_InsuranceCombo->CurSel == -1) {
			if (m_iDefaultInsuranceCo != -1) {
				m_InsuranceCombo->SetSelByColumn(0,(long)m_iDefaultInsuranceCo);
			}
			else {
				m_InsuranceCombo->PutCurSel(0);
			}
		}

		// (j.jones 2008-07-10 12:43) - PLID 28756 - set our default category and description
		if(m_boIsNewPayment) {
			TrySetDefaultInsuranceDescriptions();
		}

	}NxCatchAll("Error in setting insurance colors.");
}

void CPaymentDlg::OnSelChosenComboCategory(long nRow) 
{
	try {
		if(nRow == -1)
			return;

		CString str;
		GetDlgItemText(IDC_EDIT_DESCRIPTION, str);
		// (b.spivey, March 06, 2013) - PLID 51186 - Compare against old value. If it was manually 
		//		entered (or matches what was manually entered), then we load over it. 
		if(str == "" || m_strLastSetDescription == str) {
			// (b.spivey, February 26, 2013) - PLID 51186 - Check for a description value. 
			//		If that's blank, use the category name. 
			str = VarString(m_CategoryCombo->GetValue(nRow, pccCategoryDescription), "");
			if (str.IsEmpty()) {
				str = VarString(m_CategoryCombo->GetValue(nRow, pccCategoryName), "");
			}
			m_strLastSetDescription = str; 
			//Set value. 
			GetDlgItem(IDC_EDIT_DESCRIPTION)->SetWindowText(str);
		}
	}NxCatchAll(__FUNCTION__);
}

CPaymentDlg::~CPaymentDlg()
{
	// (j.jones 2012-10-30 16:41) - PLID 53444 - this is in OnDestroy, should not also be in the destructor
	//GetMainFrame()->EnableHotKeys();
}

int CPaymentDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// (a.walling 2008-05-23 12:54) - PLID 30099
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;	
	
	return 0;
}

LRESULT CPaymentDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
		CString strNumber = (LPCTSTR)_bstr_t((BSTR)lParam);

		if (strNumber.IsEmpty()) {
			MessageBox("No Gift Certificate number scanned.", "Practice", MB_ICONINFORMATION | MB_OK);
			return 0;
		}

		// (j.jones 2005-11-10 11:01) - PLID 17710 - GiftCertificateIDs are now strings, albeit numeric only

		//(j.jones 2015-03-25 17:27) - PLID 65245 - now just try to show this GC on the screen,
		//if it doesn't exist the function will return false
			
		//use the API to lookup by GiftID
		NexTech_Accessor::_GetGiftCertificatePaymentListIDLookupInputPtr pInput(__uuidof(NexTech_Accessor::GetGiftCertificatePaymentListIDLookupInput));
		pInput->SearchByGiftID = VARIANT_TRUE;
		pInput->GiftID = _bstr_t(strNumber);

		//since this is searching for a useable GC, we will not include expired/voided GCs
		//$0.00 balance GCs, however, can be returned
		pInput->IncludeExpiredAndVoidedGCs = VARIANT_FALSE;

		NexTech_Accessor::_FindGiftCertificatePaymentListResultPtr pResult =
			GetAPI()->GetGiftCertificateInfo_PaymentList(GetAPISubkey(), GetAPILoginToken(), pInput);

		if (pResult != NULL && pResult->Result != NULL) {
			m_nSelectedGiftID = (long)atoi(VarString(pResult->Result->ID));
			CString strGiftID = (LPCTSTR)pResult->Result->GiftID;
			COleCurrency cyTotalValue = AsCurrency(pResult->Result->TotalValue);
			COleCurrency cyBalance = AsCurrency(pResult->Result->Balance);
			COleDateTime dtPurchasedDate = pResult->Result->PurchaseDate;
			//Exp. Date can be null
			_variant_t varExpDate = pResult->Result->ExpDate->GetValueOrDefault(g_cvarNull);
			COleDateTime dtExpDate = VarDateTime(varExpDate, g_cdtNull);
			UpdateGiftCertificateControls_FromFields(strGiftID, cyTotalValue, cyBalance, dtPurchasedDate, dtExpDate);

			if (!m_radioGift.GetCheck()) {
				m_radioGift.SetCheck(TRUE);
				OnRadioGiftCertUpdated();
			}

			// the remaining logic from the old DL1 dropdown is now handled here
			NewGiftCertificateSelected(m_nSelectedGiftID);
		}
		else {
			//no GC matched this ID
			MessageBox("Gift Certificate not found!", "Practice", MB_ICONINFORMATION | MB_OK);
			return 0;
		}

	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CPaymentDlg::OnKillfocusEditCheckNumOrCCExpDate() 
{
	try {
		CString str;

		//JJ - Check to see if Credit Card is expired
		COleDateTime dt1,dt2;
		CString strMonth, strYear;
		int iMonth, iYear;
		if(m_radioCharge.GetCheck()) {
			m_nxeditCheckNumOrCCExpDate.GetWindowText(str);
			if(str != "" && str != "##/##"){ //TS 6/6/2001: Don't go through this if they haven't entered an exp. date.
				//set the date to test, to be the first day of the next month
				int iIndex = str.Find("/",0);
				strMonth = str.Left(iIndex);
				strYear = str.Right(str.GetLength()-(iIndex+1));
				if(strYear.GetLength() == 2) {
					strYear = "20" + strYear;
				}

				iMonth = atoi(strMonth);
				iYear = atoi(strYear);

				if(iMonth < 1 || iMonth > 12 || str.Find("#")!=-1) {
					MsgBox("The date you entered was invalid.");
					m_nxeditCheckNumOrCCExpDate.SetWindowText("##/##");
					return;
				}
				else if(iMonth==12) {
					//the method we use to store dates acts funky with December, so
					//we cannot just increase the month by 1. Just make it 1/1 of the
					//following year.
					if(dt2.SetDate(iYear+1,1,1)==0) {
						dt1 = COleDateTime::GetCurrentTime();
						if(dt1 >= dt2) {
							MsgBox("This credit card has expired. The date cannot be accepted.");
							m_nxeditCheckNumOrCCExpDate.SetWindowText("##/##");
							return;
						}
					}
				}
				else {
					//this method works well for all other months. Set the date to be
					//the first day of the NEXT month.
					COleDateTimeSpan dtSpan;
					dtSpan.SetDateTimeSpan(1,0,0,0);
					if(dt2.SetDate(iYear,iMonth+1,1)==0) {
						dt1 = COleDateTime::GetCurrentTime();
						if(dt1 >= dt2) {
							MsgBox("This credit card has expired. The date cannot be accepted.");
							m_nxeditCheckNumOrCCExpDate.SetWindowText("##/##");
							return;
						}
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnKillfocusEditBankNameOrCCCardNum() 
{
	try{
		//(e.lally 2007-10-30) PLID 27892 - We need to set our member variable for credit card number
		//	-If we are viewing charges.
		//	-If the card number actually changed (this could be tricky)
		if(m_radioCharge.GetCheck()){
			CString strDisplayedCardNumber;
			m_nxeditBankNameOrCCCardNum.GetWindowText(strDisplayedCardNumber);
			if(MaskCCNumber(m_strCreditCardNumber) != strDisplayedCardNumber){
				//Check if someone manually entered an 'X' in the card number, or
				//edited the card number, but left one or more 'X' in the box.
				if(strDisplayedCardNumber.Find("X") >= 0){
					CString strMessage = "When changing the credit card number, you must re-enter the entire number.\n"
						"The card number will be set back to the previous value.";
					MessageBox(strMessage,"Practice",MB_OK|MB_ICONINFORMATION);
					//Set the display back to what it was
					strDisplayedCardNumber = MaskCCNumber(m_strCreditCardNumber);
					m_nxeditBankNameOrCCCardNum.SetWindowText(strDisplayedCardNumber);
					SetFocus();
					//go no further
					return;
				}

				m_strCreditCardNumber = strDisplayedCardNumber;
				strDisplayedCardNumber = MaskCCNumber(m_strCreditCardNumber);
				m_nxeditBankNameOrCCCardNum.SetWindowText(strDisplayedCardNumber);
			}

		}


	}NxCatchAll("Error in OnKillfocusEditBankNameOrCCCardNum");
}

void CPaymentDlg::OnOK() 
{
	try {
		CString str;

		GetDlgItem(IDOK)->SetFocus();

		/*
		try {
		GetDlgItem(IDC_EDIT_DESCRIPTION)->GetWindowText(str);
		str.TrimLeft(" ");
		str.TrimRight(" ");
		if(str=="")
		return;
		BOOL IsTextInList = FALSE;
		for(int i=0;i<m_DescriptionCombo->GetRowCount();i++) {
		if(CString(m_DescriptionCombo->GetValue(i,0).bstrVal) == str) {
		IsTextInList = TRUE;
		break;
		}
		}
		if (!IsTextInList) {
		if (IDYES == MessageBox("Your description does not appear in the dropdown list.\nWould you like to add it for future use?", "NexTech", MB_YESNO)) {
		ExecuteSql("INSERT INTO PaymentExtraDescT (ExtraDescription) VALUES ('%s')",str);
		IRowSettingsPtr pRow = m_DescriptionCombo->GetRow(-1);
		pRow->PutValue(0,_bstr_t(str));
		int q = m_DescriptionCombo->AddRow(pRow);
		m_DescriptionCombo->PutCurSel(q);
		}
		}
		}NxCatchAll("Error in adding description.");
		*/
		CPaymentSaveInfo Info;
		if (FALSE == SaveChanges(&Info)) {
			// (d.lange 2016-05-12 16:02) - NX-100597 - Requery the tips datalist after updating the where
			// clause to include the newly created payment ID
			long nPaymentID = VarLong(m_varPaymentID, -1);
			if (nPaymentID != -1) {
				UpdateTipListWhereClause(nPaymentID);
				m_pTipList->Requery();
			}
			
			return;
		}

		//unregister for barcode messages
		if (GetMainFrame()) {
			if (!GetMainFrame()->UnregisterForBarcodeScan(this))
				MsgBox("Error unregistering for barcode scans.");
		}

		// (a.walling 2007-09-21 17:47) - PLID 27468 - Open the drawer if needed
		OpenCashDrawerIfNeeded(&Info, TRUE);

		CDialog::OnOK();
	} NxCatchAll(__FUNCTION__); 
}

void CPaymentDlg::OnCancel() 
{
	try
	{
		// (z.manning 2016-02-15 15:05) - PLID 68258 - Check and see if we created a new ICCP payment but failed to
		// process it. If so let's try and delete the payment as it is incomplete and never should have existed in
		// the first place (but there's only so much we can do since the processing happens outside of our control).
		if (m_bNewICCPPaymentSavedButNotYetProcessed && m_varPaymentID.vt == VT_I4)
		{
			// (z.manning 2016-02-15 16:37) - PLID 68258 - Remember, we're deleting a payment that was just created
			// that shouldn't have existed, so no need for a permission check here. Yeah, it will audit the creation
			// then deletion but that's just something we'll live with.
			if (!DeletePayment(VarLong(m_varPaymentID)))
			{
				// (z.manning 2016-02-15 16:38) - PLID 68258 - Okay, so the payment that was just created failed to 
				// delete. This is probably because something funky happened where this workstation lost connection
				// to the server or something along those lines. We do not want this payment to exist so display 
				// a message and return here. If the workstation can't do anything than keeping them stuck in the 
				// payment dialog won't hurt them anyway. And if deletion failed for some other bizarre reason then
				// they still have the option of saving the payment without processing it.
				MessageBox(FormatString("You may not cancel out of this %s because it has been partially created but the credit card "
					"has not yet been processed. Please try and process the card again or select the 'Do not process' option "
					"and press OK."
					, IsRefund() ? "refund" : "payment")
					, "Not Processed", MB_ICONERROR | MB_OK);
				return;
			}
		}

		//////////////////////////////////////////////////////////////
		// Set final amount to whatever is in the edit box
		CString str;
		GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(str);
		m_cyFinalAmount = ParseCurrencyFromInterface(str);

		//////////////////////////////////////////////////////////////
		// Negatize adjustments and refunds
		if (IsAdjustment() || IsRefund()) {
			// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - Need to explicitly define the multiplier as a long
			m_cyFinalAmount = m_cyFinalAmount * long(-1);
		}

		if (m_nGiftID != -1) {
			//we're a gift certificate, and they want to cancel... we need
			//to tell them that we have already saved a bill and the GC for this.
			if (MsgBox(MB_YESNO, "This payment is for a gift certificate.  The certificate has already been saved and a bill "
				"has been created for it.  If you cancel, the patient will have an outstanding balance.  Are you "
				"sure you wish to cancel this payment?") != IDYES)
				return;
		}

		//unregister for barcode messages
		if (GetMainFrame()) {
			if (!GetMainFrame()->UnregisterForBarcodeScan(this))
				MsgBox("Error unregistering for barcode scans.");
		}

		// (a.walling 2007-10-08 16:24) - PLID 9801 - Still see if we need to open the drawer
		// because of a previous payment (when using save and add new)

		OpenCashDrawerIfNeeded(NULL, TRUE);

		CDialog::OnCancel();
	}
	NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnCalcPercent() 
{
	try {
		////////////////////////////////////////////////////////
		// Make sure total amount is not blank, is less than
		// the maximum allowable amount, has no bad characters,
		// and has no more than two places to the right of the
		// decimal point.
		////////////////////////////////////////////////////////
		CString str;
		GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(str);
		if (str.GetLength() == 0) {
			MsgBox("Please fill in the 'Total Amount' box.");
			return;
		}

		if(!IsValidCurrencyText(str)) {
			MsgBox("Please fill correct information in the 'Total Amount' box.");
			return;
		}

		CCalculatePercentageDlg dlg(this);
		dlg.m_color = 0x9CC294;
		//dlg.m_cyOriginalAmt = ParseCurrencyFromInterface(str);
		dlg.m_strOriginalAmt = str;
		if(dlg.DoModal() == IDOK) {
			str = FormatCurrencyForInterface(dlg.m_cyFinalAmt,FALSE);
			SetDlgItemText(IDC_EDIT_TOTAL,str);
			// (z.manning, 04/29/2008) - PLID 29836 - The total changed so let's update the cash received.
			UpdateCashReceived(dlg.m_cyFinalAmt);
			((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(0, -1);

			m_cyLastAmount = dlg.m_cyFinalAmt;
		}
	}NxCatchAll(__FUNCTION__);
}


void CPaymentDlg::OnChangeEditTotal() 
{
	m_bAmountChanged = TRUE;
}

void CPaymentDlg::OnTrySetSelFinishedComboLocation(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//Probably an inactive location.
			_RecordsetPtr rsLocName;
			if(m_varPaymentID.vt == VT_I4) {
				rsLocName = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT LocationID FROM LineItemT WHERE ID = %li)", VarLong(m_varPaymentID, -1));
			}
			else if(m_varBillID.vt == VT_I4) {
				rsLocName = CreateRecordset("SELECT Name FROM LocationsT WHERE ID IN (SELECT LocationID FROM LineItemT WHERE ID IN (SELECT ID FROM ChargesT WHERE BillID = %li))", VarLong(m_varBillID));
			}
			else {
				rsLocName = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", m_DefLocationID);
			}
			if(!rsLocName->eof) {
				m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name", "")));
			}
		}
	}NxCatchAll("Error in CPaymentDlg::OnTrySetSelFinishedComboLocation()");

}


void CPaymentDlg::GetNonNegativeAmountExtent(int &nStart, int &nFinish) {
	nStart = -1;
	nFinish = -1;
	int nCurrentChar = 0;
	CString strAmount;
	GetDlgItemText(IDC_EDIT_TOTAL, strAmount);
	CString strValidChars = "1234567890";
	strValidChars += GetThousandsSeparator();
	strValidChars += GetDecimalSeparator();

	//Find the first character that is a number, thousands separator, or decimal separator.
	while(nCurrentChar < strAmount.GetLength() && nStart == -1) {
		if(strValidChars.Find(strAmount.Mid(nCurrentChar,1)) != -1) {
			nStart = nCurrentChar;
		}
		nCurrentChar++;
	}

	//Now, find the next character that is NOT a number, thousands, separator, or decimal separator.
	while(nCurrentChar < strAmount.GetLength() && nFinish == -1) {
		if(strValidChars.Find(strAmount.Mid(nCurrentChar,1)) == -1) {
			nFinish = nCurrentChar;
		}
		nCurrentChar++;
	}

	//Both are now filled in correctly (-1 is what we want if we couldn't find an appropriate position otherwise).
}

void CPaymentDlg::SetCreditCardInfoForType(const IN long nCreditCardID)
{
	try{
		//no point in querying the database if we don't have a CC type
		if(nCreditCardID == -1)
			return;

		COleDateTime dt;
		CString str;
		
		// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.
		// (a.walling 2007-10-31 10:19) - PLID 27891 - Include the encrypted CC number, SecurePAN
		// (a.walling 2010-03-15 12:30) - PLID 37751 - Include KeyIndex
		// (r.gonet 2015-04-20) - PLID 65326 - Parameterized. Converted from a magic number to the EPayMethod enum.
		_RecordsetPtr rsCreditPayments = CreateParamRecordset("SELECT TOP 1 CreditCardID, CardName, CCNumber, SecurePAN, KeyIndex, CCHoldersName, "
			"CCExpDate, CCAuthNo FROM PaymentsT "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"WHERE (LineItemT.PatientID = {INT} AND Deleted = 0 AND PayMethod = {CONST_INT} "
			"AND CreditCardNamesT.ID = {INT}) "
			"ORDER BY Date DESC, PaymentsT.ID DESC", m_PatientID, (long)EPayMethod::ChargePayment, nCreditCardID);

		if(!rsCreditPayments->eof) {
			// there are payments for this patient with the credit card we are looking for, so set the Credit card fields
			// to the info that was used with the most recent payment date
			// (e.lally 2007-07-09) PLID 25993 - Switched Credit Card type to use the new ID field from the CC table.
			_variant_t var = rsCreditPayments->Fields->Item["CreditCardID"]->Value;
			if(var.vt != VT_NULL) {
				// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
				// (e.lally 2007-07-12) PLID 26590 - The long member card ID needs to be reset so the trysetsel
					//doesn't accidently add an inactive card somehow
				m_nCreditCardID = -1;
				m_CardNameCombo->TrySetSelByColumn(cccCardID, VarLong(var, -1));
				var = rsCreditPayments->Fields->Item["CCExpDate"]->Value;
				if(var.vt==VT_DATE) {
					dt = var.date;
					// (j.jones 2007-03-13 14:47) - PLID 25183 - ensure we do not set it
					// to an invalid date
					COleDateTime dtMin;
					dtMin.ParseDateTime("12/31/1899");
					if(dt.m_status != COleDateTime::invalid && dt >= dtMin) {
						m_nxeditCheckNumOrCCExpDate.SetWindowText(dt.Format("%m/%y"));
					}
				}
				//(e.lally 2007-10-30) PLID 27892 - cache full credit card number, but only display the last 4 digits
				{
					// (a.walling 2007-10-30 18:02) - PLID 27891 - CCs are now encrypted
					// (a.walling 2010-03-15 12:27) - PLID 37751 - Use NxCrypto
					NxCryptosaur.DecryptStringFromVariant(rsCreditPayments->Fields->Item["SecurePAN"]->Value, AdoFldLong(rsCreditPayments, "KeyIndex", -1), m_strCreditCardNumber);
					// (j.jones 2015-09-30 11:04) - PLID 67175 - if we have a last 4, but not a CC number, use the last 4
					CString strDisplayedCCNumber = "";
					if (m_strCreditCardNumber.IsEmpty()) {
						strDisplayedCCNumber = AdoFldString(rsCreditPayments, "CCNumber", "");
					}
					else {
						strDisplayedCCNumber = MaskCCNumber(m_strCreditCardNumber);
					}
					// (j.jones 2015-09-30 09:31) - PLID 67167 - ICCP uses a different control to show this
					if (!IsICCPEnabled()) {
						m_nxeditBankNameOrCCCardNum.SetWindowText(strDisplayedCCNumber);
					}
					else {
						m_nxeditCCLast4.SetWindowText(strDisplayedCCNumber.Right(4));
					}
				}

				var = rsCreditPayments->Fields->Item["CCHoldersName"]->Value;
				if(var.vt == VT_BSTR)
					str = CString(var.bstrVal);
				else
					str = "";
				m_nxeditAcctNumOrCCNameOnCard.SetWindowText(str);

				// JMJ 5/20/2004 - do not fill in the previous auth. no., that always changes!
			}
		}
		else {
			// there was no info found for this credit card type, clear the data
			// JJ - we decided NOT to do this, what if they start typing first?
		}

		rsCreditPayments->Close();
	}NxCatchAll("CPaymentDlg::SetCreditCardInfoForType    Error setting current credit card info");
}

void CPaymentDlg::OnSetfocusEditAcctNumOrCCNameOnCard() 
{
	if (m_radioCharge.GetCheck() && m_boIsNewPayment)
	{
		CString str;
		m_nxeditAcctNumOrCCNameOnCard.GetWindowText(str);
		if (!str.GetLength())
		{
			try {
				// Fill in the patient's name
				// (z.manning 2009-04-22 10:04) - PLID 33976 - Parameterized
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT [Last], [First], [Middle] \r\n"
					"FROM PersonT \r\n"
					"WHERE ID = {INT} \r\n"
					, m_PatientID);
				if (!prs->eof)
				{
					// (z.manning 2009-04-22 10:19) - PLID 33976 - Change the name format to "First Last"
					// instead of "Last, First"
					str = AdoFldString(prs->GetFields(), "First", "") + ' ';
					CString strMiddle = AdoFldString(prs->GetFields(), "Middle", "");
					if(!strMiddle.IsEmpty()) {
						// (z.manning 2009-04-22 10:11) - PLID 33976 - We used to include the whole middle
						// name, but I've never seen a card with a full middle name. And though my research
						// suggests it is possible to do so, middle initial seems to be far more common.
						str += strMiddle.Left(1) + ' ';
					}
					str += AdoFldString(prs->GetFields(), "Last", "");

					m_nxeditAcctNumOrCCNameOnCard.SetWindowText(str);
					m_nxeditAcctNumOrCCNameOnCard.SetSel(0, -1);
				}
			}
			NxCatchAll("Error auto-setting patient's name");
		}
	}
}

void CPaymentDlg::OnSelChosenComboCardName(long nRow) 
{
	try {
		// if the option is enabled, auto-fill the credit card information based on the patient's last credit payment
		// for the selected credit card type
		if(GetChargeInfo != 0) {
			// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
				//-Changed to use a long member variable for the ID in case of duplicate names
			m_nLastCCTypeID = m_CardNameCombo->CurSel == -1 ? -1 : VarLong(m_CardNameCombo->GetValue(m_CardNameCombo->GetCurSel(),cccCardID));
			SetCreditCardInfoForType(m_nLastCCTypeID);
		}

		//see whether we are doing interac
		// (j.gruber 2007-07-03 16:09) - PLID 15416 - CC Processing
		//TODO: j.gruber - fix this when we are fix the credit card types
		// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
		//	- Put in a check for no curSel
		//we are doing this right when they open thepaymentdlg now
		//if (nRow > -1 && VarString(m_CardNameCombo->GetValue(nRow, cccCardType)) == "Interac") {

			//if we are using a pin pad (which they must be, let's send the message to start the pin pad going
			//PinPadUtils::GetCustomerSwipe();
		//}
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnSelChosenComboDescription(long nRow) 
{
	try {
		if(nRow==-1)
			return;

		CString str;
		str = CString(m_DescriptionCombo->GetValue(nRow,0).bstrVal);
		m_DescriptionCombo->PutComboBoxText("");
		GetDlgItem(IDC_EDIT_DESCRIPTION)->SetWindowText(str);
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnSelChosenComboLocation(long nRow) 
{
	try {
		if(nRow == -1)
			nRow = m_LocationCombo->SetSelByColumn(0,m_DefLocationID);	

		// (j.gruber 2007-08-15 17:30) - PLID 25191 - check to see if they have permission to change the location
		if (m_varPaymentID.vt != VT_I4) {

			long nID = VarLong(m_LocationCombo->GetValue(nRow, 0));

			if (nID != GetCurrentLocationID()) {

				EBuiltInObjectIDs bioObject;
				if (m_radioPayment.GetCheck() ) {
					bioObject = bioPaymentLocation;
				}
				else if (m_radioAdjustment.GetCheck() ) {
					bioObject = bioAdjLocation;
				}
				else if (m_radioRefund.GetCheck() ) {
					bioObject = bioRefundLocation;
				}
				else {
					ASSERT(FALSE);
				}

				if (!(CheckCurrentUserPermissions(bioObject, sptWrite))) {
					
					if (m_DefLocationID == -1) {
						m_LocationCombo->SetSelByColumn(0, GetCurrentLocationID());
					}
					else {
						m_LocationCombo->SetSelByColumn(0,m_DefLocationID);	
					}
					return;
				}	
			}		
		}	
	}NxCatchAll("Error in CPaymentDlg::OnSelChosenComboLocation");
	
}

void CPaymentDlg::OnKillfocusCashReceived() 
{
	try {

		CString strCashReceived, strPaymentTotal;

		//first get the payment amount
		GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(strPaymentTotal);
		if (strPaymentTotal.GetLength() == 0) {
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please enter a valid amount in the 'Total Amount' box.");			
			return;			
		}

		COleCurrency cyPaymentTotal = ParseCurrencyFromInterface(strPaymentTotal);
		if(cyPaymentTotal.GetStatus() == COleCurrency::invalid) {
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please enter a valid amount in the 'Total Amount' box.");			
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cyPaymentTotal.Format().Find(strDecimal) != -1 && cyPaymentTotal.Format().Find(strDecimal) + (nDigits+1) < cyPaymentTotal.Format().GetLength()) {
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please fill only %li places to the right of the %s in the 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");			
			return;
		}

		//now get the receive amount
		GetDlgItem(IDC_CASH_RECEIVED)->GetWindowText(strCashReceived);
		if (strCashReceived.GetLength() == 0) {
			//re-fill with the total, as this cannot be blank
			strCashReceived = FormatCurrencyForInterface(cyPaymentTotal,FALSE,TRUE);
			SetDlgItemText(IDC_CASH_RECEIVED,strCashReceived);
		}

		COleCurrency cyCashReceived = ParseCurrencyFromInterface(strCashReceived);
		if(cyCashReceived.GetStatus() == COleCurrency::invalid) {
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please enter a valid amount in the 'Cash Received' box.");			
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		if(cyCashReceived.Format().Find(strDecimal) != -1 && cyCashReceived.Format().Find(strDecimal) + (nDigits+1) < cyCashReceived.Format().GetLength()) {
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please fill only %li places to the right of the %s in the 'Cash Received' box.",nDigits,strDecimal == "," ? "comma" : "decimal");			
			return;
		}

		if(cyCashReceived < cyPaymentTotal) {
			//reset it to the payment amount
			SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(cyPaymentTotal,FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("You cannot enter an amount less than the payment amount in the 'Cash Received' box.");			
			return;
		}

		SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(cyCashReceived,FALSE,TRUE));

		//if we get here, we have valid amounts, so calculate away!

		COleCurrency cyChangeGiven = cyCashReceived - cyPaymentTotal;
		CString strChangeGiven = FormatCurrencyForInterface(cyChangeGiven,FALSE,TRUE);
		SetDlgItemText(IDC_CHANGE_GIVEN,strChangeGiven);

	}NxCatchAll("Error calculating change due.");
}

void CPaymentDlg::OnKillfocusEditTotal() 
{
	//needs to be saved because we format the text, but if they really didn't change the text
	//we want to remember that it wasn't really changed by the user
	BOOL bWasChanged = m_bAmountChanged;

	try {		

		CString str;
		GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(str);
		if (str.GetLength() == 0) {			
			SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			//don't warn them yet, this should be OK for now
			//MsgBox("Please fill in the 'Total Amount' box.");
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(str);
		if(cy.GetStatus() == COleCurrency::invalid) {
			SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please enter a valid amount in the 'Total Amount' box.");			
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
			SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			SetDlgItemText(IDC_CHANGE_GIVEN,FormatCurrencyForInterface(COleCurrency(0,0),FALSE,TRUE));
			MsgBox("Please fill only %li places to the right of the %s in the 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");			
			return;
		}

		//JMJ 3/8/2004 - PLID 11324 - this is a simple fix so if they are making an adjustment and
		//the amount is (0.00) they will still have (0.00) if they kill focus
		BOOL bNegativeZero = FALSE;
		if(str.Find("(") != -1 && ParseCurrencyFromInterface(str) == COleCurrency(0,0)) {
			bNegativeZero = TRUE;
		}

		if(bNegativeZero) {
			//keep international settings
			str = FormatCurrencyForInterface(COleCurrency(-1,0),FALSE);
			str.Replace("1","0");
			SetDlgItemText(IDC_EDIT_TOTAL,str);
		}
		else {
			SetDlgItemText(IDC_EDIT_TOTAL,FormatCurrencyForInterface(cy,FALSE,TRUE));
		}

		// (z.manning, 04/29/2008) - PLID 29836 - Move the logic to chage the cash received field to its own function.
		UpdateCashReceived(cy);

		m_cyLastAmount = cy;

	}NxCatchAll("Error in OnKillfocusEditTotal");

	m_bAmountChanged = bWasChanged;
}

void CPaymentDlg::OnSelChosenComboInsurance(long nRow) 
{
	try {

		if(nRow == -1) {
			m_InsuranceCombo->CurSel = 0;
			nRow = 0;
		}

		// (j.jones 2008-07-10 12:43) - PLID 28756 - set our default category and description
		TrySetDefaultInsuranceDescriptions();

		COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
		CString strAmount;

		// Change the maximum amount of the payment depending
		// on the responsibility

		if (m_QuoteID == -1 && m_varBillID.vt != VT_EMPTY && m_varBillID.vt != VT_NULL) {
			if (nRow <= 0 /*patient, or unselected*/ || m_InsuranceCombo->GetValue(nRow,0).lVal == 0) {
				GetBillTotals(m_varBillID.lVal, m_PatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance);
				if(m_bIsCoPay) {
					//if a copay, we can make the maximum amount the entire charge balance
					m_cyMaxAmount = cyCharges - cyPayments - cyAdjustments - cyRefunds;
					strAmount = FormatCurrencyForInterface(COleCurrency(m_cyCopayAmount),FALSE);
				}
				else {
					//otherwise it's just the patient balance
					m_cyMaxAmount = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;
					strAmount = FormatCurrencyForInterface(m_cyMaxAmount,FALSE);
				}
			}
			else {
				int InsType = GetInsuranceTypeFromID(m_InsuranceCombo->GetValue(nRow,0).lVal);
				if (InsType == 0)
					return;
				if(InsType != -1)
					// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
					GetBillInsuranceTotals(m_varBillID.lVal, m_PatientID, m_InsuranceCombo->GetValue(nRow,0).lVal, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds);
				else 
					GetInactiveInsTotals(m_InsuranceCombo->GetValue(nRow,0).lVal, m_varBillID.lVal, -1, m_PatientID, cyCharges, cyPayments);
				m_cyMaxAmount = cyCharges - cyPayments - cyAdjustments - cyRefunds;
				strAmount = FormatCurrencyForInterface(m_cyMaxAmount,FALSE);
			}
		}
		else if (m_varChargeID.vt != VT_EMPTY && m_varChargeID.vt != VT_NULL) {
			if (nRow <= 0 /*patient, or unselected*/ || m_InsuranceCombo->GetValue(nRow,0).lVal == 0) {
				GetChargeTotals(m_varChargeID.lVal, m_PatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance);
				m_cyMaxAmount = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;
				strAmount = FormatCurrencyForInterface(m_cyMaxAmount,FALSE);
			}
			else {
				int InsType = GetInsuranceTypeFromID(m_InsuranceCombo->GetValue(nRow,0).lVal);
				if (InsType == 0)
					return;
				GetChargeInsuranceTotals(m_varChargeID.lVal, m_PatientID, m_InsuranceCombo->GetValue(nRow,0).lVal, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds);
				m_cyMaxAmount = cyCharges - cyPayments - cyAdjustments - cyRefunds;
				strAmount = FormatCurrencyForInterface(m_cyMaxAmount,FALSE);
			}
		}
		else
			return;

		//If they have edited the payment amount prior to changing the responsibility,
		//then we don't want to change their payment amount,
		//but if they haven't edited it then it's okay
		if(!m_bAmountChanged) {

			if (IsAdjustment() && ParseCurrencyFromInterface(strAmount) != COleCurrency(0,0) ){
				// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - Need to explicitly define the multiplier as a long
				COleCurrency cyAmt = ParseCurrencyFromInterface(strAmount) * long(-1);
				strAmount = FormatCurrencyForInterface(cyAmt,FALSE);
			}
			GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(strAmount);
			// v.arth 06/24/2009 PLID 34699 - Have the IDC_CASH_RECEIVED text box update at the same
			//                                time as the IDC_EDIT_TOTAL
			GetDlgItem(IDC_CASH_RECEIVED)->SetWindowTextA(strAmount);

			m_bAmountChanged = FALSE;
		}

	}NxCatchAll("Error in CPaymentDlg::OnSelChosenComboInsurance");
}

void CPaymentDlg::OnAddTipBtn() 
{
	//ensure they can use nex spa
	if(!m_bHasNexSpa) {
		MsgBox("You must purchase the NexSpa module to use this feature.");
		return;
	}

	//ensure they have permission
	//TES 3/25/2015 - PLID 65175 - Refunds have a separate permission
	if (IsRefund()) {
		if (!CheckCurrentUserPermissions(bioRefundTips, sptWrite, false))
		{
		return;
		}
	}
	else {
		ASSERT(IsPayment());
		if (!CheckCurrentUserPermissions(bioPaymentTips, sptWrite, false)) {
			return;
		}
	}


	try {
		//we want to set the method to whatever they have currently chosen to pay with
		//TES 4/16/2015 - PLID 65171 - If this is a refund, use the refund paymethod
		// (r.gonet 2015-04-20) - PLID 65326 - Changed the magic number for paymethod into an enum.
		EPayMethod eMethod = EPayMethod::CheckPayment;	//default to check if something goes wrong
		if(m_radioCash.GetCheck())
			eMethod = m_radioRefund.GetCheck() ? EPayMethod::CashRefund : EPayMethod::CashPayment;
		else if(m_radioCheck.GetCheck())
			eMethod = m_radioRefund.GetCheck() ? EPayMethod::CheckRefund : EPayMethod::CheckPayment;
		else if(m_radioCharge.GetCheck())
			eMethod = m_radioRefund.GetCheck() ? EPayMethod::ChargeRefund : EPayMethod::ChargePayment;
		//DRT 4/23/2004 - Don't allow tips of GC amounts (they don't total in  with normal payments), so make them cash by default.
		//TES 4/1/2015 - PLID 65069 - GC tips are now allowed
		else if(m_radioGift.GetCheck()) // (r.gonet 2015-04-20) - PLID 65326 - Added gift cert refund.
			eMethod = m_radioRefund.GetCheck() ? EPayMethod::GiftCertificateRefund : EPayMethod::GiftCertificatePayment;

		IRowSettingsPtr pRow = m_pTipList->GetRow(sriNoRow);

		long nProvID = -1;
		//see if anything is already selected for the provider, we can just copy that
		if(m_ProviderCombo->GetCurSel() != sriNoRow)
			nProvID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), 0));
		else {
			//if there's nothing selected, there is the possibility of an inactive provider being there.
			if(m_ProviderCombo->IsComboBoxTextInUse) {
				//aha!  there is an inactive.
				nProvID = m_nTipInactiveProv;
			}
		}

		if(nProvID == -1) {
			//still have no provider!  maybe they unselected the combo there.  Regardless, we need *something* chosen
			//here, so just pick the first provider we can find.
			_RecordsetPtr prs = CreateRecordset("SELECT TOP 1 ID FROM ProvidersT INNER JOIN PersonT ON PersonID = ID WHERE Archived = 0");
			if(prs->eof) {
				//they don't have any providers
				MsgBox("You have no active providers in your database.  You cannot create a tip.");
				return;
			}
			else {
				//ok, stick this one in
				nProvID = AdoFldLong(prs, "ID");
			}
		}

		pRow->PutValue(tlcID, (long)-1);
		pRow->PutValue(tlcProvID, (long)nProvID);
		pRow->PutValue(tlcAmount, _variant_t(COleCurrency(0, 0)));
		// (r.gonet 2015-04-20) - PLID 65326 - Converted from a magic number to the EPayMethod enum.
		pRow->PutValue(tlcMethod, (long)eMethod);

		long nRow = m_pTipList->AddRow(pRow);

		//and we want to start editing the amount field
		if(nRow != sriNoRow)
			m_pTipList->StartEditing(nRow, tlcAmount);
	} NxCatchAll("Error in AddTipBtn()");
}

void CPaymentDlg::OnShowTipsBtn() 
{
	try {
		CRect rcStatic, rcDialog;
		GetWindowRect(&rcDialog);

		//setup for sizing
		int left, right, top, bottom;
		int bottomoffset = 5; //used to add extra space past the NxColor

		//left, top, and right are going to stay the same no matter what, we just expand the bottom
		left = rcDialog.left;
		right = rcDialog.right;
		top = rcDialog.top;

		BOOL bEnableTips = FALSE;
		//if we don't have permission or don't have nexspa, always shrink
		if(m_bTipExtended || !m_bViewTips || !m_bHasNexSpa) {
			//we need to shrink
			GetDlgItem(IDC_BOTTOM_RIGHT_NORMAL)->GetWindowRect(&rcStatic);
			bEnableTips = FALSE;
			bottomoffset = 0;
		}
		else {
			//we need to expand
			GetDlgItem(IDC_BOTTOM_RIGHT_EXTENDED)->GetWindowRect(&rcStatic);
			//TES 3/25/2015 - PLID 65175 - Call the function to check if we can edit
			bEnableTips = CanEditTips();
		}

		//TES 3/25/2015 - PLID 65175 - Enable/disable the tip button, the datalist, and the "Include Tips In Same Cash Drawer" checkbox
		GetDlgItem(IDC_ADD_TIP_BTN)->EnableWindow(bEnableTips);
		m_pTipList->ReadOnly = bEnableTips ? g_cvarFalse : g_cvarTrue;
		m_btnTipsInDrawer.EnableWindow(bEnableTips);

		//we've moved, now update our boolean
		m_bTipExtended = !m_bTipExtended;

		//and the bottom is set to whatever we found above
		bottom = rcStatic.bottom;


		//Now actually move the window
		SetWindowPos(NULL, left, top, right-left, (bottom-top) + bottomoffset, SWP_NOZORDER);

		//and finally, rename the button appropriately
		if(m_bTipExtended) 
			GetDlgItem(IDC_SHOW_TIPS_BTN)->SetWindowText("Hide Tips <<");
		else
			GetDlgItem(IDC_SHOW_TIPS_BTN)->SetWindowText("Show Tips >>");
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnRButtonDownTipList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		//set the selection
		m_pTipList->PutCurSel(nRow);

		//pop up a menu with the ability to add or delete
	
		CMenu mnu;
		mnu.LoadMenu(IDR_PAYMENT_TIPS);
		CMenu *pmnuSub = mnu.GetSubMenu(0);
		if (pmnuSub) {
			CPoint pt;
			pt.x = x;	pt.y = y;
			GetDlgItem(IDC_TIP_LIST)->ClientToScreen(&pt);

			// Hide certain items if we're not on a row
			if (m_pTipList->CurSel == sriNoRow) {
				pmnuSub->EnableMenuItem(ID_PAYMENTTIPS_DELETE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
			} else {
				pmnuSub->SetDefaultItem(ID_PAYMENTTIPS_ADD);
			}
			// Show the popup
			pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		}
	} NxCatchAll("Error in OnRButtonDownTipList()");

}

BOOL CPaymentDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {
	case ID_PAYMENTTIPS_ADD:	//popup menu - Add new tip
		OnAddTipBtn();
		return TRUE;

	case ID_PAYMENTTIPS_DELETE:	//popup menu - Delete current tip
		OnDeleteTip();
		return TRUE;
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CPaymentDlg::OnDeleteTip()
{
	//ensure they have permission
	if(!CheckCurrentUserPermissions(bioPaymentTips, sptWrite, false))
		return;

	//delete the currently selected tip
	try {
		long nCurSel = m_pTipList->GetCurSel();
		if(nCurSel == sriNoRow) {
			//this shouldn't be possible
			ASSERT(FALSE);
			return;
		}

		if(MsgBox(MB_YESNO, "Are you sure you wish to delete this tip?") != IDYES)
			return;

		//we don't really delete it, we just remove the row and set the ID in an array to delete later
		long nID = VarLong(m_pTipList->GetValue(nCurSel, tlcID));

		//flag it in our delete list
		m_aryDeleted.Add(nID);

		//remove the row
		m_pTipList->RemoveRow(nCurSel);

		//update the total
		UpdateTotalTipAmt();

	} NxCatchAll("Error in OnDeleteTip()");
}

//Calculate and update the total amount of tips in the edit box provided
void CPaymentDlg::UpdateTotalTipAmt()
{
	COleCurrency cy(0, 0);

	try {

		for(int i = 0; i < m_pTipList->GetRowCount(); i++) {
			COleCurrency cyAmt = VarCurrency(m_pTipList->GetValue(i, tlcAmount));

			cy += cyAmt;
		}

		//now update the box
		SetDlgItemText(IDC_TOTAL_TIP_AMT, FormatCurrencyForInterface(cy));
	} NxCatchAll("Error updating total tip amount.");
}

void CPaymentDlg::OnEditingFinishedTipList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	switch(nCol) {
	case tlcAmount:	//Amount
		UpdateTotalTipAmt();	//update the total
		break;
	case tlcMethod:
		//TES 3/19/2015 - PLID 65069 - We might want to remove Gift Cert from the list now
		EnsureTipMethodCombo();
		break;
	}
}

void CPaymentDlg::OnRequeryFinishedTipList(short nFlags) 
{
	//update the tip amount
	UpdateTotalTipAmt();
}

void CPaymentDlg::OnEditingFinishingTipList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		//check permission on editing, if they fail we can't commit
		//TES 3/25/2015 - PLID 65175 - Refunds have separate permissions
		if (IsRefund()) {
			if (!CheckCurrentUserPermissions(bioRefundTips, sptWrite, false)) {
			*pbCommit = FALSE;
			}
			else {
				if (pvarNewValue && pvarNewValue->vt == VT_CY) {
					//TES 3/25/2015 - PLID 65174 - If they entered a positive value, make it negative
					COleCurrency cy = VarCurrency(*pvarNewValue);
					if (cy > COleCurrency(0, 0)) {
						cy = cy*-1;
						pvarNewValue->cyVal = cy.m_cur;
					}
				}
			}
		}
		else {
			ASSERT(IsPayment());
			if (!CheckCurrentUserPermissions(bioPaymentTips, sptWrite, false)) {
				*pbCommit = FALSE;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnBtnSaveAndAddNew() 
{
	try {

		//first save as normal
		// (a.walling 2007-10-04 14:35) - PLID 9801 - Gather info from SaveChanges
		CPaymentSaveInfo Info;
		if (FALSE == SaveChanges(&Info))
			return;

		long nID1 = -1;
		long nID2 = -1;
		WORD Type = 0; //1 - Bill, 2 - Charge, 3 - Quote/PIC, 4 - PatientID, 0 - Nothing

		// (j.jones 2009-08-25 12:35) - PLID 31549 - send the QuoteID, ProcInfoID, or both
		if(m_QuoteID > 0 || m_nProcInfoID > 0) {
			nID1 = m_QuoteID;
			nID2 = m_nProcInfoID;
			Type = 3;
		}
		else if(m_varBillID.vt == VT_I4 && VarLong(m_varBillID,-1) > 0) {
			nID1 = VarLong(m_varBillID,-1);
			Type = 1;
		}
		else if(m_varChargeID.vt == VT_I4 && VarLong(m_varChargeID,-1) > 0) {
			nID1 = VarLong(m_varChargeID,-1);
			Type = 2;
		}
		else {
			//if we are not applying to a bill, charge, or quote, send the patient ID
			nID1 = m_PatientID;
			Type = 4;
		}

		// (a.walling 2007-10-04 14:35) - PLID 9801 - Open the cash drawer if desired
		// (a.walling 2007-10-08 15:12) - PLID 9801 - Don't open if adding new, just send our info
		// in the NXM_NEW_PAYMENT message
		WORD odrNew = OpenCashDrawerIfNeeded(&Info, FALSE);

		//unregister for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this))
				MsgBox("Error unregistering for barcode scans.");
		}

		CDialog::OnOK();

		//we will need to post a message that a new payment would need to be made
		// (a.walling 2007-10-08 15:59) - PLID 9801 - Send type and previous drawer info
		// (j.jones 2009-08-25 12:42) - PLID 31549 - we now can send up to two IDs, but they have to be non-negative
		if(nID1 == -1) {
			nID1 = 0;
		}
		if(nID2 == -1) {
			nID2 = 0;
		}

		// (z.manning 2010-02-22 16:15) - PLID 37479 - We used to pass all 4 of these variables in as part
		// of the WPARAM and LPARAM, including nID1 and nID2 as the WPARAM. However, MAKEWPARAM takes in 2
		// unsigned shorts and we were passing in to longs so if either of those ID values was greater than
		// USHRT_MAX (65,635) than it losing its value and leading to unpredictable results.
		NewPaymentMessageInfo *pPaymentMessageInfo = new NewPaymentMessageInfo;
		pPaymentMessageInfo->nID1 = nID1;
		pPaymentMessageInfo->nID2 = nID2;
		pPaymentMessageInfo->nType = Type;
		pPaymentMessageInfo->odr = odrNew;
		GetMainFrame()->PostMessage(NXM_NEW_PAYMENT, (WPARAM)pPaymentMessageInfo);

	}NxCatchAll("Error in OnSaveAndAddNew");
}

void CPaymentDlg::OnClickRadioGiftCert()
{
	try {
		// (c.haag 2010-09-20 15:34) - PLID 35723 - Warn the user if we're switching from a CC
		// method of payment and a transaction took place
		if (!WarnOfExistingCCTransaction()) {
			return;
		}
		OnRadioGiftCertUpdated();
	}
	NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnRadioGiftCertUpdated() 
{
	try {
		// (j.jones 2007-04-19 15:07) - PLID PLID 25711 - all window changes (show/hide, etc.)
		// have been moved to PostClickRadioGC()
		PostClickRadioGC();
	}
	NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnGcIncludeOthers() 
{
	try {
		
		// (j.jones 2015-03-23 15:34) - PLID 65281 - Now we only need to toggle our search logic
		// to only search for GCs for this patient - or all patients.
		// The search will always include GCs not assigned to any patient.
		long nPatientIDToFilter = -1;
		if (!IsDlgButtonChecked(IDC_GC_INCLUDE_OTHERS)) {
			//only can bill GC's which have no receiver (unclaimed) or the receiver is our current patient
			nPatientIDToFilter = m_PatientID;
		}
		GiftCertificateSearchUtils::UpdatePaymentResultList(m_GiftSearch, GetRemoteData(), nPatientIDToFilter);

		//This does not clear the currently selected gift certificate, if there is one.
		//The checkbox only toggles the nature of the search results.

	} NxCatchAll("Error requerying gift certificate list.");
}

void CPaymentDlg::OnEditCashDrawers() 
{
	try {
		if(!m_bHasNexSpa) {
			MsgBox("You must purchase the NexSpa license to use this feature.");
			return;
		}

		CEditDrawersDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			//save sel and requery
			long nCurSel = m_pDrawerList->GetCurSel();
			if(nCurSel != sriNoRow) {
				long nID = VarLong(m_pDrawerList->GetValue(nCurSel, 0));
				m_pDrawerList->Requery();
				IRowSettingsPtr pRow = m_pDrawerList->GetRow(-1);
				pRow->PutValue(0, (long)-1);
				pRow->PutValue(1, _bstr_t("<No Drawer Selected>"));
				m_pDrawerList->AddRow(pRow);
				m_pDrawerList->SetSelByColumn(0, (long)nID);
			}
		}
	} NxCatchAll("Error editing cash drawer list.");
}

//DRT 2/9/2005 - PLID 15566 - This code was in a dozen different places, so consolidated here.
void CPaymentDlg::EnsureReceivedArea()
{
	//These fields are enabled for some payment methods (cash, check, charge - NOT gift cert.), 
	//	but not for any other types (adj, ref, pay GC).
	// (j.gruber 2007-07-31 12:37) - PLID 26889 - take out the cash received for CC processing
	// (j.jones 2015-10-07 15:47) - PLID 67157 - this code was completely insane, this is only for cash/check payments, period
	if (m_radioPayment.GetCheck() && (m_radioCash.GetCheck() || m_radioCheck.GetCheck())) {
		GetDlgItem(IDC_CASH_RECEIVED)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CASH_RECEIVED_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CHANGE_GIVEN)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CHANGE_GIVEN_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CURRENCY_SYMBOL_RECEIVED)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CURRENCY_SYMBOL_GIVEN)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_CASH_RECEIVED)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CASH_RECEIVED_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHANGE_GIVEN)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHANGE_GIVEN_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CURRENCY_SYMBOL_RECEIVED)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CURRENCY_SYMBOL_GIVEN)->ShowWindow(SW_HIDE);
	}
}

//DRT 4/20/2004 - Replaces heavily duplicated code in the ShowWindow function.  Also, we now hide
//	the labels if they are not in use, instead of setting text to an empty string.  We also handle the 
//	datalist for cash drawers.
void CPaymentDlg::EnsureLabelArea()
{
	//handle the static text labels first
	if(IsAdjustment()) {
		//adjustment, show them and display text
		ShowDlgItem(IDC_LABEL_INFO1, SW_SHOW);
		ShowDlgItem(IDC_LABEL_INFO2, SW_SHOW);

		m_nxlInfoLabel1.SetType(dtsText);
		m_nxlInfoLabel2.SetType(dtsText);

		m_nxlInfoLabel1.SetText("A negative adjustment amount will reduce the patient balance");
		m_nxlInfoLabel2.SetText("A positive adjustment amount will increase the patient balance");

		m_nxlInfoLabel1.Invalidate();
		m_nxlInfoLabel2.Invalidate();
	}
	else if(IsRefund()) {
		//refund, only the first is shown
		ShowDlgItem(IDC_LABEL_INFO1, SW_SHOW);
		ShowDlgItem(IDC_LABEL_INFO2, SW_HIDE);

		m_nxlInfoLabel1.SetType(dtsText);

		m_nxlInfoLabel1.SetText("Please enter a positive refund amount");

		m_nxlInfoLabel1.Invalidate();
	}
	else {
		//payment or something bad going down, hide all
		ShowDlgItem(IDC_LABEL_INFO1, SW_HIDE);
		ShowDlgItem(IDC_LABEL_INFO2, SW_HIDE);
	}

	// (j.jones 2007-03-27 17:40) - PLID 24080 - show batch payment hyperlink
	// (c.haag 2008-05-29 12:04) - PLID 29751 - Show it in label info 3 instead of 1.
	// Also hide the other labels; unfortunately there just isn't enough real estate
	// in the dialog to show them.
	if(m_nBatchPaymentID != -1) {
		ShowDlgItem(IDC_LABEL_INFO1, SW_HIDE);
		ShowDlgItem(IDC_LABEL_INFO2, SW_HIDE);
		ShowDlgItem(IDC_LABEL_INFO3, SW_SHOW);
		m_nxlInfoLabel3.SetText(m_strBatchPaymentInfo);
		m_nxlInfoLabel3.SetType(dtsHyperlink);
		m_nxlInfoLabel3.Invalidate();
	}
	else {
		ShowDlgItem(IDC_LABEL_INFO3, SW_HIDE);
	}

	//now handle the showing or hiding for the cash drawers list & edit button
	//DRT 6/29/2005 - PLID 16010 - We've now modified this to allow preferences to determine if refunds can
	//	go in cash drawers.  We also will show the cash drawer if we are editing and a value exists.  This can
	//	happen if they had the preference on, saved a few refunds in drawers, then went back and turned the pref.
	//	off.  We obviously don't want to hide that data, or just throw it away w/o telling them.
	bool bShowDrawer = false;
	if(m_bHasNexSpa) {
		if(IsPayment()){
			//always available for payments
			bShowDrawer = true;
		}
		//(e.lally 2006-10-17) PLID 22725 - Adjustments should never show the drawer
		else if(IsAdjustment()){
			bShowDrawer = false;
		}
		else {
			//Check preferences for refunds
			// (r.gonet 2015-06-10 17:17) - PLID 65748 - Added a preference for gift certificate refunds.
			if( ( IsRefund() && m_radioCash.GetCheck() && GetRemotePropertyInt("NexSpa_AllowRefundCash", 1, 0, "<None>", true) == 1 ) ||
				( IsRefund() && m_radioCheck.GetCheck() && GetRemotePropertyInt("NexSpa_AllowRefundCheck", 1, 0, "<None>", true) == 1 ) ||
				( IsRefund() && m_radioCharge.GetCheck() && GetRemotePropertyInt("NexSpa_AllowRefundCharge", 1, 0, "<None>", true) == 1 ) ||
				( IsRefund() && m_radioGift.GetCheck() && GetRemotePropertyInt("NexSpa_AllowRefundGiftCertificate", 1, 0, "<None>", true) == 1)
			  ) {
				bShowDrawer = true;
			}
			else {
				//only show if it's an edited payment and the drawer isn't "no drawer selected"
				if(!GetDlgItem(IDC_CASH_DRAWER_LIST)->IsWindowEnabled())
					//If the window is disabled this means it was a saved cash drawer, but that drawer is since closed.  We will
					//	always show in this case.
					bShowDrawer = true;
				else {
					long nDrawerID = VarLong(m_pDrawerList->GetValue(m_pDrawerList->GetCurSel(), 0));
					if(!m_boIsNewPayment && nDrawerID != -1) {	//-1 is the "no selected" case
						bShowDrawer = true;
					}
				}
			}
		}
	}

	if(bShowDrawer) {
		//all payments, cash refunds
		GetDlgItem(IDC_CASH_DRAWER_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_CASH_DRAWERS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_CASH_DRAWERS)->EnableWindow(TRUE);
		GetDlgItem(IDC_CASH_DRAWER_LABEL)->ShowWindow(SW_SHOW);
	}
	else {
		//non-spa, adjustments, non-cash refunds, wierd cases
		GetDlgItem(IDC_CASH_DRAWER_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_CASH_DRAWERS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_CASH_DRAWERS)->EnableWindow(FALSE);
		GetDlgItem(IDC_CASH_DRAWER_LABEL)->ShowWindow(SW_HIDE);
	}
}

void CPaymentDlg::OnTipsInDrawer() 
{
	try {
		//we want to remember this setting
		SetRemotePropertyInt("TipsInDrawer", (IsDlgButtonChecked(IDC_TIPS_IN_DRAWER) ? 1 : 0), 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnCheckPrepayment() 
{
	try {

		if(m_QuoteID != -1 && !((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->GetCheck()) {

			//they are changing a linked prepayment to no longer be a prepayment
			if(IDNO == MessageBox("This PrePayment is linked to a quote.\n"
				"If you uncheck the 'PrePayment' box, this payment will not remain linked to the quote.\n"
				"Are you sure you wish to make this change?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {

				((CButton *)GetDlgItem(IDC_CHECK_PREPAYMENT))->SetCheck(1);
			}
		}		

		// (b.spivey, October 01, 2012) - PLID 50949 - prepay
		UpdateWindowText(); 

	}NxCatchAll("Error checking prepayment.");
}

// (a.walling 2006-11-15 09:42) - PLID 23550 - Show/hide the group and reason code controls
void CPaymentDlg::ShowGroupCodesAndReasons(OPTIONAL IN bool bShow /* = true */)
{
	try {
		GetDlgItem(IDC_ADJ_GROUPCODE)->ShowWindow(bShow);
		GetDlgItem(IDC_ADJ_GROUPCODE_LABEL)->ShowWindow(bShow);
		GetDlgItem(IDC_ADJ_REASON)->ShowWindow(bShow);
		GetDlgItem(IDC_ADJ_REASON_LABEL)->ShowWindow(bShow);
		// (j.jones 2012-07-26 17:48) - PLID 26877 - added ability to filter adjustment reasons
		GetDlgItem(IDC_BTN_FILTER_REASON_CODES)->ShowWindow(bShow);
	} NxCatchAll("Error displaying group and reason code lists");
}

// (a.walling 2006-11-15 09:42) - PLID 23550 - Prevent no selection
void CPaymentDlg::OnSelChangingGroupCode(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			if (lpOldSel != NULL)
				lpOldSel->AddRef(); // this MUST be done for datalist2s!!!
			*lppNewSel = lpOldSel;	
		}
	}NxCatchAll("Error resetting group code selection");
}

// (a.walling 2006-11-15 09:42) - PLID 23550 - Prevent no selection
void CPaymentDlg::OnSelChangingReason(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			if (lpOldSel != NULL)
				lpOldSel->AddRef(); // this MUST be done for datalist2s!!!
			*lppNewSel = lpOldSel;	
		}
	}NxCatchAll("Error resetting reason code selection");
}

// (j.jones 2007-03-27 17:24) - PLID 24080 - added batch payment info. hyperlink
BOOL CPaymentDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (m_nBatchPaymentID != -1) {
			
			if (m_rcInfoLabel1.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (j.jones 2007-03-27 17:24) - PLID 24080 - added batch payment info. hyperlink
void CPaymentDlg::OnBatchPaymentInfo()
{
	try {

		if(m_nBatchPaymentID != -1) {

			//display a message with the batch payment's information,
			//and then give the option to preview the report for that batch payment

			// (j.jones 2012-04-24 10:56) - PLID 49933 - ensured we only filter on child payments, not adjustments
			// (j.jones 2012-04-25 10:53) - PLID 48032 - Supported line item corrections that were takebacks,
			// returning the value of the original payment to the batch payment. Also fixed to ignore
			// payments that were voided (unless part of another batch payment's takeback).
			_RecordsetPtr rs = CreateParamRecordset("SELECT BatchPaymentsT.Amount, BatchPaymentsT.Description, "
				"BatchPaymentsT.CheckNo, BatchPaymentsT.Date, InsuranceCoT.Name AS InsCoName, "
				"Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) AS AppliedAmount, "
				"BatchPaymentsT.Amount "
				" - Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) "
				" + Coalesce(LineItemsReversedQ.TotalReversed, Convert(money,0)) "
				" - Coalesce(AppliedPaymentsT.TotalApplied, Convert(money,0)) "
				" AS RemainingAmount, "
				"BatchPaymentsT.InputDate "
				"FROM BatchPaymentsT "
				""
				//find child payments that are not voided, but include them if they are part of a takeback
				"LEFT JOIN (SELECT PaymentsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalApplied "
				"	FROM LineItemT "
				"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
				"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
				"	AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
				"	AND PaymentsT.BatchPaymentID Is Not Null "
				"	GROUP BY PaymentsT.BatchPaymentID "
				") AS LineItemsInUseQ ON BatchPaymentsT.ID = LineItemsInUseQ.BatchPaymentID "
				""
				//find payments that were part of takebacks, crediting this batch payment
				"LEFT JOIN (SELECT LineItemCorrectionsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalReversed "
				"	FROM LineItemT "
				"	INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
				"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
				"	AND LineItemCorrectionsT.BatchPaymentID Is Not Null "
				"	GROUP BY LineItemCorrectionsT.BatchPaymentID "
				") AS LineItemsReversedQ ON BatchPaymentsT.ID = LineItemsReversedQ.BatchPaymentID "
				""
				"LEFT JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID "
				""
				//find the batch payment's adjustments or refunds
				"LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, AppliedBatchPayID "
				"	FROM BatchPaymentsT "
				"	WHERE Type <> 1 AND Deleted = 0 "
				"	GROUP BY AppliedBatchPayID, Deleted "
				") AS AppliedPaymentsT ON BatchPaymentsT.ID = AppliedPaymentsT.AppliedBatchPayID "
				""
				"GROUP BY BatchPaymentsT.ID, InsuranceCoT.PersonID, BatchPaymentsT.Amount, "
				"LineItemsInUseQ.TotalApplied,LineItemsReversedQ.TotalReversed, AppliedPaymentsT.TotalApplied, BatchPaymentsT.Description, BatchPaymentsT.CheckNo, BatchPaymentsT.Date, "
				"InsuranceCoT.Name, BatchPaymentsT.Deleted, BatchPaymentsT.InputDate "
				"HAVING BatchPaymentsT.Deleted = 0 AND BatchPaymentsT.ID = {INT}", m_nBatchPaymentID);
			if(!rs->eof) {

				COleCurrency cyAmount = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
				COleCurrency cyAppliedAmount = AdoFldCurrency(rs, "AppliedAmount",COleCurrency(0,0));
				COleCurrency cyRemainingAmount = AdoFldCurrency(rs, "RemainingAmount",COleCurrency(0,0));
				CString strInsCoName = AdoFldString(rs, "InsCoName","");
				CString strDesc = AdoFldString(rs, "Description","");
				strDesc.TrimLeft();
				strDesc.TrimRight();
				if(strDesc.IsEmpty())
					strDesc = "<None>";
				CString strCheckNo = AdoFldString(rs, "CheckNo","");
				strCheckNo.TrimLeft();
				strCheckNo.TrimRight();
				if(strCheckNo.IsEmpty())
					strCheckNo = "<None>";
				COleDateTime dtDate = AdoFldDateTime(rs, "Date", COleDateTime::GetCurrentTime());
				COleDateTime dtInputDate = AdoFldDateTime(rs, "InputDate", COleDateTime::GetCurrentTime());

				CString strMessage;
				strMessage.Format("This payment is part of a %s batch payment for %s.\n\n"
					"Amount Applied: %s\n"
					"Amount Remaining: %s\n\n"
					"Description: %s\n"
					"Check Number: %s\n\n"
					"Payment Date: %s\n"
					"Input Date: %s\n\n"
					"Would you like to preview the Batch Payment report?\n"
					"(Doing so will save and close this payment.)",
					FormatCurrencyForInterface(cyAmount,TRUE,TRUE), strInsCoName,
					FormatCurrencyForInterface(cyAppliedAmount,TRUE,TRUE), FormatCurrencyForInterface(cyRemainingAmount,TRUE,TRUE),
					strDesc, strCheckNo,
					FormatDateTimeForInterface(dtDate, NULL, dtoDate), FormatDateTimeForInterface(dtInputDate, NULL, dtoDate));

				if(IDYES == MessageBox(strMessage, "Practice", MB_ICONINFORMATION|MB_YESNO)) {

					if (!m_bSaved) {
						// (a.walling 2007-10-04 14:35) - PLID 9801 - Gather info from SaveChanges
						CPaymentSaveInfo Info;
						if (FALSE == SaveChanges(&Info)) {
							return;
						}
						
						// (a.walling 2007-10-04 14:35) - PLID 9801 - Open the cash drawer if desired
						OpenCashDrawerIfNeeded(&Info, TRUE);
					}

					// Create a copy of the report object
					CReportInfo rep(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(322)]);

					rep.nDateFilter = 1;
					rep.nDateRange = -1;

					rep.SetExtraValue(AsString(m_nBatchPaymentID));

					//Set up the parameters.
					CPtrArray paParams;
					CRParameterInfo *paramInfo;
					
					paramInfo = new CRParameterInfo;
					paramInfo->m_Data = GetCurrentUserName();
					paramInfo->m_Name = "CurrentUserName";
					paParams.Add(paramInfo);

					paramInfo = new CRParameterInfo;
					paramInfo->m_Data = "01/01/1000";
					paramInfo->m_Name = "DateFrom";
					paParams.Add((void *)paramInfo);

					paramInfo = new CRParameterInfo;
					paramInfo->m_Data = "12/31/5000";
					paramInfo->m_Name = "DateTo";
					paParams.Add((void *)paramInfo);

					//check to see if there is a default report
					_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 322");
					CString strFileName;

					if (rsDefault->eof) {

						strFileName = "BatchPaymentsService";
					}
					else {
						
						long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

						if (nDefaultCustomReport > 0) {

							_RecordsetPtr rsFileName = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 322 AND Number = %li", nDefaultCustomReport);

							if (rsFileName->eof) {

								//this should never happen
								MessageBox("Practice could not find the custom report.  Please contact NexTech for assistance");
							}
							else {
								
								//set the default
								rep.nDefaultCustomReport = nDefaultCustomReport;
								strFileName =  AdoFldString(rsFileName, "FileName");
							}
						}
						else {
							//if this occurs it just means they want the default, which in this case, there is only one
							strFileName = "BatchPaymentsService";
							
						}
					}

					rep.strReportFile = strFileName;

					//Made new function for running reports - JMM 5-28-04
					RunReport(&rep, &paParams, TRUE, (CWnd *)this, "Batch Payments", NULL);
					ClearRPIParameterList(&paParams);

					// (a.walling 2008-03-17 09:21) - PLID 29043 - unregister for barcode messages
					if(GetMainFrame()) {
						if(!GetMainFrame()->UnregisterForBarcodeScan(this))
							MsgBox("Error unregistering for barcode scans.");
					}

					EndDialog(RETURN_PREVIEW);
				}
			}
			rs->Close();
		}

	}NxCatchAll("Error in CPaymentDlg::OnBatchPaymentInfo");
}

// (j.jones 2007-03-28 09:06) - PLID 24080 - added batch payment info. hyperlink
LRESULT CPaymentDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		// (c.haag 2008-05-29 12:17) - PLID 29751 - Batch payment data now goes
		// in label info 3.
		case IDC_LABEL_INFO3:
			if(m_nBatchPaymentID != -1) {
				OnBatchPaymentInfo();
			}
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

LRESULT CPaymentDlg::OnMSRDataEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.wetta 2007-04-17 10:34) - PLID 25672 - Process the credit card information
		// Can't have credit cards on adjustments
		if (IsAdjustment()) {
			return 0;
		}

		if (!m_bAllowSwipe) {
			return 0;
		}

		// (a.walling 2007-08-03 16:47) - PLID 26922 - Check for licensing and permission
		// (d.thompson 2010-09-02) - PLID 40371 - Any license type satisfies
		// (j.jones 2015-09-30 08:58) - PLID 67157 - if you have ICCP, you are allowed to make a credit
		// card payment, you just won't be able to process it
		if (g_pLicense && g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent) && !IsICCPEnabled()) {
			// they are licensed, check permission silently (will prompt for PW when saving)
			if (!CheckCurrentUserPermissions(bioCCProcess, sptWrite, FALSE, 0, TRUE, TRUE)) {
				// they don't have permission!
				MessageBox("You do not have permission to process credit cards. Please check with your office manager.");

				return 0;
			}
		}

		// (a.wetta 2007-07-05 08:56) - PLID 26547 - Get the credit card information
		// (b.cardillo 2007-09-17 17:21) - I found this problem when investigating the same problem for plid 24983, but it 
		// turns out in that case too it goes back to the mistake made here for plid 26547.  It should be checking that it's 
		// a credit card being swiped BEFORE it gives the message to the user saying it's a credit card!  So I've made the 
		// correction here as I have for those other two places.  There are marked it under plid 24983 for consistency, but 
		// here I'll just reference both pl items so some sense can be made of the situation.
		MSRTrackInfo *mtiInfo = (MSRTrackInfo*)wParam;
		// (j.jones 2009-06-19 11:03) - PLID 33650 - support the msrCardType enum
		if(mtiInfo->msrCardType == msrCreditCard) {
			// Warn the user that the information from the credit card will overwrite the information already in the fields
			CString strMsg;
			strMsg = "A credit card has just been swiped.  The information from the card will fill the credit card information fields for this\n"
					"payment, and any information already in those fields will be overwritten.  Are you sure you want to get the credit card information?";
			HRESULT hr = DontShowMeAgain(this, strMsg, "MSR_GetCreditCardInfoForPayment", "Get Credit Card Information", FALSE, TRUE);
			if (hr == IDYES || hr == IDOK) {

				// (j.gruber 2007-07-17 14:43) - PLID 26710 - if the pin pad is enabled, make sure it knows to stop waiting for a card swipe
				if (m_bPinPadSwipeable) {
					m_bPinPadSwipeable = FALSE;
					PinPadUtils::ClearCustomerSwipe();
				}
				// Make sure that charge is selected for payment type
				if (!m_radioCharge.GetCheck())
					PostClickRadioCharge();

				CreditCardInfo *cciCardInfo = (CreditCardInfo*)lParam;
				
				// Fill the information from the credit card to the screen
				// Set the card type
				m_strSwipedCardName = cciCardInfo->m_strCardType;
				// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
				// (e.lally 2007-07-12) PLID 26590 - The long member card ID needs to be reset so the trysetsel
					//doesn't accidently add an inactive card somehow
				m_nCreditCardID = -1;
				long nResult = m_CardNameCombo->TrySetSelByColumn(cccCardName,_variant_t(m_strSwipedCardName));
				if (nResult == NXDATALISTLib::sriNoRow) {
					// The credit card type could not be selected
					CString strMsg;
					strMsg.Format("The credit card type \'%s\' could not be selected for the card just swiped because is does not\n"
								"exist in the card type list.  Please add this card type to the list.", cciCardInfo->m_strCardType);
					MessageBox(strMsg);
					// Set the card type to be nothing
					// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
					m_CardNameCombo->SetSelByColumn(cccCardName,_variant_t(""));
					// Set m_strSwipedCardName to blank because the card swipe has been handled
					m_strSwipedCardName = "";
				}
				else if (nResult == sriNoRowYet_WillFireEvent) {
					// OnTrySetSelFinishedComboCardName will handle it the selection and it will use m_strSwipedCardName to 
					// determine that it was called because a card was swiped and to get the name of the card
				}
				else {
					ASSERT(nResult >= 0);
					// Set m_strSwipedCardName to blank because the card swipe has been handled
					m_strSwipedCardName = "";
				}

				// Set the expiration date
				// Ensure we do not set it to an invalid date
				COleDateTime dtMin, dt = cciCardInfo->m_dtExpDate;
				dtMin.ParseDateTime("12/31/1899");
				if(dt.m_status != COleDateTime::invalid && dt >= dtMin) {
					m_nxeditCheckNumOrCCExpDate.SetWindowText(dt.Format("%m/%y"));
				}

				// Set the credit card number
				//(e.lally 2007-10-30) PLID 27892 - cache full credit card number, but only display the last 4 digits
				{
					m_strCreditCardNumber = cciCardInfo->m_strCardNum;
					CString strDisplayedCCNumber = MaskCCNumber(m_strCreditCardNumber);
					// (j.jones 2015-09-30 09:31) - PLID 67167 - ICCP uses a different control to show this
					if (!IsICCPEnabled()) {
						m_nxeditBankNameOrCCCardNum.SetWindowText(strDisplayedCCNumber);
					}
					else {
						m_nxeditCCLast4.SetWindowText(strDisplayedCCNumber.Right(4));
					}
				}

				// Set the name on the credit card
				// Format the name
				CString strName = "";
				if (cciCardInfo->m_strMiddleInitial == "" || cciCardInfo->m_strMiddleInitial == " ") {
					strName.Format("%s %s", cciCardInfo->m_strFirstName, cciCardInfo->m_strLastName);
				}
				else {
					strName.Format("%s %s %s", cciCardInfo->m_strFirstName, cciCardInfo->m_strMiddleInitial, cciCardInfo->m_strLastName);
				}

				// Add the Title if there is one
				if (cciCardInfo->m_strTitle != "") {
					CString strTemp;
					strTemp.Format("%s %s", cciCardInfo->m_strTitle, strName);
					strName = strTemp;
				}

				// Add the Suffix if there is one
				if (cciCardInfo->m_strSuffix != "") {
					CString strTemp;
					strTemp.Format("%s, %s", strName, cciCardInfo->m_strSuffix);
					strName = strTemp;
				}

				m_nxeditAcctNumOrCCNameOnCard.SetWindowText(FixCapitalization(strName));

				// (j.gruber 2007-07-03 16:01) - PLID 15416 - update our boolean that we have swiped the card
				m_bSwiped = TRUE;
				m_swtType = swtMSRDevice;
				m_strTrack1 = mtiInfo->strTrack1;
				m_strTrack2 = mtiInfo->strTrack2;
				m_strTrack3 = mtiInfo->strTrack3;
			

			}
		} else {
			MessageBox(
				"A card other than a credit card has just been swiped when a credit card was expected.\r\n\r\n"
				"Please swipe a credit card to retrieve payment information.", 
				"Magnetic Strip Reader", MB_OK|MB_ICONEXCLAMATION);
		}

	
		
	}NxCatchAll("Error in CPaymentDlg::OnMSRDataEvent");

	return 0;
}

// (j.jones 2007-04-19 14:12) - PLID 25711 - added PostClick functions
// so the OnClick functions are only called when actually changed
void CPaymentDlg::PostClickRadioPayment()
{
	try {

		// (j.jones 2008-09-08 13:20) - PLID 26689 - update the payment category description
		m_nxstaticPayCatLabel.SetWindowText("Payment Category");

		GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(TRUE);

		GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_SHOW);

		// (a.walling 2006-11-15 11:39) - PLID 23550 - Hide the group and reason code controls
		ShowGroupCodesAndReasons(false);

		// Allow user to selecting a payment method
		EnablePaymentTypes();

		//////////////////////////////////////////////////////////////
		// Assume the user will put in a new check

		// (j.jones 2007-04-19 15:14) - PLID 25711 - if a new payment, we want OnRadioCheckUpdated
		if(m_boIsNewPayment)
			OnRadioCheckUpdated();
		else
			PostClickRadioCheck();
		EnsureLabelArea();

		//////////////////////////////////////////////////////////////
		// Reset the amount
		CString str = FormatCurrencyForInterface(COleCurrency(0,0), FALSE);
		GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);
		GetDlgItem(IDC_EDIT_TOTAL)->SetFocus();
		((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(0, -1);

		//we need to show the tip button (if they have permission
		if (m_bViewTips) {
			GetDlgItem(IDC_SHOW_TIPS_BTN)->EnableWindow(TRUE);
		}

		//TES 3/23/2015 - PLID 65173 - Update text in the tips section.
		SetDlgItemText(IDC_ADD_TIP_BTN, "Add Tip");
		m_pTipList->GetColumn(tlcMethod)->ColumnTitle = _bstr_t("Payment Method");
		//TES 5/29/2015 - PLID 65175 - Check if we're allowed to edit tips
		if (m_bTipExtended) {
			BOOL bEditTips = CanEditTips();
			m_btnAddTip.EnableWindow(bEditTips);
			m_pTipList->ReadOnly = bEditTips ? g_cvarFalse : g_cvarTrue;
			m_btnTipsInDrawer.EnableWindow(bEditTips);
		}

		// (j.gruber 2007-08-15 17:30) - PLID 25191 - check to see if they have permission to change the location
		if (m_radioPayment.GetCheck() && m_varPaymentID.vt != VT_I4) {
	
			if (!(GetCurrentUserPermissions(bioPaymentLocation) & (SPT___W_______))) {
			
				if (!(GetCurrentUserPermissions(bioPaymentLocation) & (SPT___W________ANDPASS))) {

					//they don't have write permission, set the location to be the currently logged in one
					m_LocationCombo->SetSelByColumn(0, GetCurrentLocationID());
					m_DefLocationID = GetCurrentLocationID();
				}
				else {

					//they have with password, so we'll check when they change it
					
				}
			}
					
		}
	
	}NxCatchAll("Error in CPaymentDlg::PostClickRadioPayment()");
}

// (j.jones 2007-04-19 14:12) - PLID 25711 - added PostClick functions
// so the OnClick functions are only called when actually changed
void CPaymentDlg::PostClickRadioAdjustment()
{
	try {

		// (j.jones 2008-09-08 13:20) - PLID 26689 - update the payment category description
		m_nxstaticPayCatLabel.SetWindowText("Adjustment Category");

		GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);

		GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);

		// Eliminate the checks from the cash/check/charge radios
		// Prevent user from selecting a payment method
		DisablePaymentTypes();

		// Hide all the edit boxes and labels
		PostClickRadioCash();
		EnsureLabelArea();

		// (a.walling 2006-11-15 10:15) - PLID 23550
		ShowGroupCodesAndReasons(true); // show the group codes and reasons dropdowns.

		// Reset the amount
		//TES 6/11: We want to format, using regional settings, the non-existent amount of "-0".  So, we'll get creative.
		CString str = FormatCurrencyForInterface(COleCurrency(-1,0),FALSE);
		str.Replace("1","0");
		GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);
		GetDlgItem(IDC_EDIT_TOTAL)->SetFocus();
		int nStart, nFinish;
		GetNonNegativeAmountExtent(nStart, nFinish);
		((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(nStart, nFinish);

		//we need to hide the tip window and disable the button
		m_bTipExtended = true;
		OnShowTipsBtn();
		GetDlgItem(IDC_SHOW_TIPS_BTN)->EnableWindow(FALSE);

		// (j.gruber 2007-08-15 17:30) - PLID 25191 - check to see if they have permission to change the location
		if (m_radioAdjustment.GetCheck() && m_varPaymentID.vt != VT_I4) {
	
			if (!(GetCurrentUserPermissions(bioAdjLocation) & (SPT___W_______))) {
			
				if (!(GetCurrentUserPermissions(bioAdjLocation) & (SPT___W________ANDPASS))) {

					//they don't have write permission, set the location to be the currently logged in one
					m_LocationCombo->SetSelByColumn(0, GetCurrentLocationID());
					m_DefLocationID = GetCurrentLocationID();
				}
				else {

					//they have with password, so we'll check when they change it
					
				}
			}
		}	

	}NxCatchAll("Error in CPaymentDlg::PostClickRadioAdjustment()");
}

// (j.jones 2007-04-19 14:12) - PLID 25711 - added PostClick functions
// so the OnClick functions are only called when actually changed
void CPaymentDlg::PostClickRadioRefund()
{
	try {

		// (j.jones 2008-09-08 13:20) - PLID 26689 - update the payment category description
		m_nxstaticPayCatLabel.SetWindowText("Refund Category");

		GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);

		GetDlgItem(IDC_CHECK_PREPAYMENT)->ShowWindow(SW_HIDE);

		// (a.walling 2006-11-15 11:39) - PLID 23550 - Hide the group and reason code controls
		ShowGroupCodesAndReasons(false);

		//////////////////////////////////////////////////////////////
		// Eliminate the checks from the cash/check/charge radios
		EnablePaymentTypes();
		// (j.jones 2007-04-19 15:14) - PLID 25711 - if a new payment, we want OnRadioCheckUpdated
		if(m_boIsNewPayment)
			OnRadioCheckUpdated();
		else
			PostClickRadioCheck();

		EnsureLabelArea();

		// Reset the amount
		CString str = FormatCurrencyForInterface(COleCurrency(0,0),FALSE);
		GetDlgItem(IDC_EDIT_TOTAL)->SetWindowText(str);
		GetDlgItem(IDC_EDIT_TOTAL)->SetFocus();
		((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(0, -1);

		//we need to hide the tip window and disable the button
		//TES 3/23/2015 - PLID 65172 - This might be available, if the preference is enabled
		if (GetRemotePropertyInt("AllowTipRefunds", 0, 0, "<None>"))
		{
			//TES 3/23/2015 - PLID 65172 - They do have it, so use the same behavior as PostClickRadioPayment()
			if (m_bViewTips) {
				GetDlgItem(IDC_SHOW_TIPS_BTN)->EnableWindow(TRUE);
			}
		}
		else {
			m_bTipExtended = true;
			OnShowTipsBtn();
			GetDlgItem(IDC_SHOW_TIPS_BTN)->EnableWindow(FALSE);
		}
		//TES 3/23/2015 - PLID 65173 - Update text in the tips section.
		SetDlgItemText(IDC_ADD_TIP_BTN, "Refund Tip");
		m_pTipList->GetColumn(tlcMethod)->ColumnTitle = _bstr_t("Refund Method");
		//TES 5/29/2015 - PLID 65175 - Check if we're allowed to edit tips
		if (m_bTipExtended) {
			BOOL bEditTips = CanEditTips();
			m_btnAddTip.EnableWindow(bEditTips);
			m_pTipList->ReadOnly = bEditTips ? g_cvarFalse : g_cvarTrue;
			m_btnTipsInDrawer.EnableWindow(bEditTips);
		}

		// (j.gruber 2007-08-15 17:30) - PLID 25191 - check to see if they have permission to change the location
		if (m_radioRefund.GetCheck() && m_varPaymentID.vt != VT_I4) {
	
			if (!(GetCurrentUserPermissions(bioRefundLocation) & (SPT___W_______))) {
			
				if (!(GetCurrentUserPermissions(bioRefundLocation) & (SPT___W________ANDPASS))) {

					//they don't have write permission, set the location to be the currently logged in one
					m_LocationCombo->SetSelByColumn(0, GetCurrentLocationID());
					m_DefLocationID = GetCurrentLocationID();
				}
				else {

					//they have with password, so we'll check when they change it
					
				}
			}
					
		}
		
		

	}NxCatchAll("Error in CPaymentDlg::PostClickRadioRefund()");
}

// (j.jones 2007-04-19 14:12) - PLID 25711 - added PostClick functions
// so the OnClick functions are only called when actually changed
void CPaymentDlg::PostClickRadioCash()
{
	try {
		if (m_radioPayment.GetCheck()) {
			GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(TRUE);
		}

		m_radioCash.SetCheck(1);
		m_radioCheck.SetCheck(0);
		m_radioCharge.SetCheck(0);
		m_radioGift.SetCheck(0);

		// (a.walling 2007-08-03 16:01) - PLID 26922 - update m_pCurPayMethod
		UpdateCurPayMethod();

		m_nxstaticLabelCheckNumOrCCExpDate.SetWindowText("");
		m_nxstaticLabelBankNameOrCCCardNum.SetWindowText("");
		m_nxstaticLabelAcctNumOrCCNameOnCard.SetWindowText("");
		m_nxstaticLabelCCCardType.SetWindowText("");
		m_nxstaticLabelBankNumOrCCAuthNum.SetWindowText("");
		m_nxstaticLabelCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
		m_nxstaticLabelBankNameOrCCCardNum.ShowWindow(SW_HIDE);
		m_nxstaticLabelAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);
		m_nxstaticLabelCCCardType.ShowWindow(SW_HIDE);
		m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_HIDE);
		m_nxeditCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
		m_nxeditBankNameOrCCCardNum.ShowWindow(SW_HIDE);
		m_nxeditAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_HIDE);
		m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_HIDE);
		// (r.gonet 2015-04-20) - PLID 65326 - Hide the gift certificate mode radio buttons when in charges.
		GetDlgItem(IDC_REFUND_TO_EXISTING_GC_RADIO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REFUND_TO_NEW_GC_RADIO)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-23 13:13) - PLID 65281 - converted the gift certificate dropdown
		// into a searchable list
		GetDlgItem(IDC_SEARCH_GC_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GC_SEARCH_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GC_INCLUDE_OTHERS)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-26 08:56) - PLID 65283 - hide all GC controls
		GetDlgItem(IDC_LABEL_GIFT_CERT_NUMBER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_BALANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_PURCH_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_EXP_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GIFT_CERT_NUMBER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_BALANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_PURCH_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_EXP_DATE)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-26 09:37) - PLID 65285 - hide the GC X button
		GetDlgItem(IDC_BTN_CLEAR_GC)->ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
		m_nxstaticLabelCCTransactionType.ShowWindow(SW_HIDE);
		m_radioCCSwipe.ShowWindow(SW_HIDE);
		m_radioCCCardNotPresent.ShowWindow(SW_HIDE);
		m_radioCCDoNotProcess.ShowWindow(SW_HIDE);
		// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
		m_radioCCRefundToOriginalCard.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct. and zipcode
		GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_HIDE);
		m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_HIDE);
		m_nxstaticLabelCCZipcode.ShowWindow(SW_HIDE);
		m_nxeditCCZipcode.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
		GetDlgItem(IDC_CC_CARD_ON_FILE_COMBO)->ShowWindow(SW_HIDE);
		m_nxstaticLabelCCCardOnFile.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
		m_nxstaticLabelCCLast4.ShowWindow(SW_HIDE);
		m_nxeditCCLast4.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
		m_checkAddCCToProfile.ShowWindow(SW_HIDE);

		//DRT 2/9/2005 - PLID 15566 - Show the amount received/change given boxes appropriately
		EnsureReceivedArea();
		//

		EnsureLabelArea();

		// (j.gruber 2007-07-03 15:01) - PLID 15416 - CC Processing
		//change the buttons to not show Authorizing
		if (m_bProcessCreditCards_NonICCP) {

			// (d.thompson 2009-06-30) - PLID 34687 - Hide the process cc transactions button
			GetDlgItem(IDC_PROCESS_CC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROCESS_CC)->EnableWindow(FALSE);

			GetDlgItem(IDC_BTN_PREVIEW)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_PRINT)->EnableWindow(TRUE);
		}

		// (b.spivey, September 29, 2011) - PLID 40567 - Hide Card Swipe
		GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->EnableWindow(FALSE); 

		//TES 3/19/2015 - PLID 65069 - Update whether "Gift Cert" shows for tips
		EnsureTipMethodCombo();

		//for some reason the focus is set to the Cash Drawer dropdown, so focus needs to 
		//be set back to the radio button
		m_radioCash.SetFocus();

	}NxCatchAll("Error in CPaymentDlg::PostClickRadioCash()");
}

// (j.jones 2007-04-19 14:12) - PLID 25711 - added PostClick functions
// so the OnClick functions are only called when actually changed
void CPaymentDlg::PostClickRadioCheck()
{
	try {
		if (m_radioPayment.GetCheck()) {
			GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(TRUE);
		}

		m_radioCash.SetCheck(0);
		m_radioCheck.SetCheck(1);
		m_radioCharge.SetCheck(0);
		m_radioGift.SetCheck(0);

		// (a.walling 2007-08-03 16:01) - PLID 26922 - update m_pCurPayMethod
		UpdateCurPayMethod();

		m_nxstaticLabelCheckNumOrCCExpDate.SetWindowText("Check Number");
		m_nxstaticLabelBankNameOrCCCardNum.SetWindowText("Bank Name");
		m_nxstaticLabelAcctNumOrCCNameOnCard.SetWindowText("Account Number");
		m_nxstaticLabelCCCardType.SetWindowText("");
		m_nxstaticLabelBankNumOrCCAuthNum.SetWindowText("Bank Number");
		m_nxstaticLabelCheckNumOrCCExpDate.ShowWindow(SW_SHOW);
		m_nxstaticLabelBankNameOrCCCardNum.ShowWindow(SW_SHOW);
		m_nxstaticLabelAcctNumOrCCNameOnCard.ShowWindow(SW_SHOW);
		m_nxstaticLabelCCCardType.ShowWindow(SW_HIDE);
		m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_SHOW);
		m_nxeditCheckNumOrCCExpDate.ShowWindow(SW_SHOW);
		m_nxeditCheckNumOrCCExpDate.SetWindowText("");
		m_nxeditBankNameOrCCCardNum.ShowWindow(SW_SHOW);
		m_nxeditBankNameOrCCCardNum.SetWindowText("");
		m_nxeditAcctNumOrCCNameOnCard.ShowWindow(SW_SHOW);
		m_nxeditAcctNumOrCCNameOnCard.SetWindowText("");
		GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_HIDE);
		m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_SHOW);
		m_nxeditBankNumOrCCAuthNum.SetWindowText("");
		GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
		m_nxstaticLabelCCTransactionType.ShowWindow(SW_HIDE);
		m_radioCCSwipe.ShowWindow(SW_HIDE);
		m_radioCCCardNotPresent.ShowWindow(SW_HIDE);
		m_radioCCDoNotProcess.ShowWindow(SW_HIDE);
		// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
		m_radioCCRefundToOriginalCard.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct. and zipcode
		GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_HIDE);
		m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_HIDE);
		m_nxstaticLabelCCZipcode.ShowWindow(SW_HIDE);
		m_nxeditCCZipcode.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
		GetDlgItem(IDC_CC_CARD_ON_FILE_COMBO)->ShowWindow(SW_HIDE);
		m_nxstaticLabelCCCardOnFile.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
		m_nxstaticLabelCCLast4.ShowWindow(SW_HIDE);
		m_nxeditCCLast4.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
		m_checkAddCCToProfile.ShowWindow(SW_HIDE);

		//DRT 2/9/2005 - PLID 15566 - Call new Ensure function to make sure the received/change amount is setup.
		EnsureReceivedArea();

		// (r.gonet 2015-04-20) - PLID 65326 - Hide the gift certificate mode radio buttons when in charges.
		GetDlgItem(IDC_REFUND_TO_EXISTING_GC_RADIO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REFUND_TO_NEW_GC_RADIO)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-23 13:13) - PLID 65281 - converted the gift certificate dropdown
		// into a searchable list
		GetDlgItem(IDC_SEARCH_GC_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GC_SEARCH_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GC_INCLUDE_OTHERS)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-26 08:56) - PLID 65283 - hide all GC controls
		GetDlgItem(IDC_LABEL_GIFT_CERT_NUMBER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_BALANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_PURCH_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_EXP_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GIFT_CERT_NUMBER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_BALANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_PURCH_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_EXP_DATE)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-26 09:37) - PLID 65285 - hide the GC X button
		GetDlgItem(IDC_BTN_CLEAR_GC)->ShowWindow(SW_HIDE);

		EnsureLabelArea();

		
		// (j.gruber 2007-07-03 15:01) - PLID 15416 - CC Processing
		//change the buttons to not show Authorizing
		if (m_bProcessCreditCards_NonICCP) {

			// (d.thompson 2009-06-30) - PLID 34687 - hide cc processing
			GetDlgItem(IDC_PROCESS_CC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROCESS_CC)->EnableWindow(FALSE);

			GetDlgItem(IDC_BTN_PREVIEW)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_PRINT)->EnableWindow(TRUE);
		}

		// (b.spivey, September 29, 2011) - PLID 40567 - Hide Card Swipe
		GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->EnableWindow(FALSE); 

		//TES 3/19/2015 - PLID 65069 - Update whether "Gift Cert" shows for tips
		EnsureTipMethodCombo();

		//for some reason the focus is set to the Cash Drawer dropdown, so focus needs to 
		//be set back to the radio button
		m_radioCheck.SetFocus();

	}NxCatchAll("Error in CPaymentDlg::PostClickRadioCheck()");
}

// (j.jones 2007-04-19 14:12) - PLID 25711 - added PostClick functions
// so the OnClick functions are only called when actually changed
void CPaymentDlg::PostClickRadioCharge()
{
	try {

		if (m_radioPayment.GetCheck()) {
			GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(TRUE);
		}

		m_radioCash.SetCheck(0);
		m_radioCheck.SetCheck(0);
		m_radioCharge.SetCheck(1);
		m_radioGift.SetCheck(0);

		// (a.walling 2007-08-03 16:01) - PLID 26922 - update m_pCurPayMethod
		UpdateCurPayMethod();

		m_nxstaticLabelCheckNumOrCCExpDate.SetWindowText("Expiration Date");
		m_nxstaticLabelBankNameOrCCCardNum.SetWindowText("Card Number");
		m_nxstaticLabelAcctNumOrCCNameOnCard.SetWindowText("Name on Card");
		m_nxstaticLabelCCCardType.SetWindowText("Card Type");
		m_nxstaticLabelBankNumOrCCAuthNum.SetWindowText("Authorization #");

		// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
		if (IsICCPEnabled()) 
		{
			// (c.haag 2015-08-26) - PLID 67201 - Try to recall the last authorized merchant for this user for new payments
			if (m_boIsNewPayment)
			{
				int nLastAccountID = GetRemotePropertyInt("LastICCPPaymentMerchantAccountID", -1, 0, GetCurrentUserName());
				if (nLastAccountID > -1)
				{
					// Force the current selection to be the row with the merchant that was used in the most recent authorization.
					// If it was inactivated, select the first possible merchant.
					m_CCMerchantAccountCombo->PutCurSel(m_CCMerchantAccountCombo->FindByColumn(maccID, nLastAccountID, nullptr, VARIANT_FALSE));
					if (m_CCMerchantAccountCombo->GetCurSel() == nullptr && m_CCMerchantAccountCombo->GetRowCount() > 0) 
					{
						m_CCMerchantAccountCombo->PutCurSel(m_CCMerchantAccountCombo->GetFirstRow());
					}
				}				
			}

		GetDlgItem(IDC_REFUND_TO_EXISTING_GC_RADIO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REFUND_TO_NEW_GC_RADIO)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-23 13:13) - PLID 65281 - converted the gift certificate dropdown
		// into a searchable list
		GetDlgItem(IDC_SEARCH_GC_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GC_SEARCH_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GC_INCLUDE_OTHERS)->ShowWindow(SW_HIDE);
		//hide all non-ICCP CC controls
		GetDlgItem(IDC_PROCESS_CC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-26 08:56) - PLID 65283 - hide all GC controls
		GetDlgItem(IDC_LABEL_GIFT_CERT_NUMBER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_BALANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_PURCH_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_EXP_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GIFT_CERT_NUMBER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_BALANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_PURCH_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_EXP_DATE)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-26 09:37) - PLID 65285 - hide the GC X button
		GetDlgItem(IDC_BTN_CLEAR_GC)->ShowWindow(SW_HIDE);

			m_nxstaticLabelCCTransactionType.ShowWindow(SW_SHOW);
			m_radioCCSwipe.ShowWindow(SW_SHOW);
			m_radioCCCardNotPresent.ShowWindow(SW_SHOW);
			m_radioCCDoNotProcess.ShowWindow(SW_SHOW);
			// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
			m_radioCCRefundToOriginalCard.ShowWindow(IsRefund() ? SW_SHOW : SW_HIDE);

			//if the user does not have the Process Transactions permission,
			//the Swipe and Card Not Present options are disabled
			if (!CheckCurrentUserPermissions(bioCCProcess, sptWrite, FALSE, 0, TRUE, TRUE)) {
				m_radioCCSwipe.EnableWindow(FALSE);
				m_radioCCCardNotPresent.EnableWindow(FALSE);
				// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
				m_radioCCRefundToOriginalCard.EnableWindow(FALSE);

				//select "Other / Do Not Process"
				m_radioCCSwipe.SetCheck(FALSE);
				m_radioCCCardNotPresent.SetCheck(FALSE);
				m_radioCCDoNotProcess.SetCheck(TRUE);
				// (j.jones 2015-09-30 10:37) - PLID 67172 - also disable refund to original card
				m_radioCCRefundToOriginalCard.SetCheck(FALSE);
				PostClickRadioCCDoNotProcess();
			}
			else {

				// (j.jones 2015-09-30 10:37) - PLID 67172 - if this is a refund, do we have a valid
				// payment token to refund back to?
				bool bHasValidRefundToken = false;
				if (m_PaymentToApplyTo.nPaymentID != -1 && m_PaymentToApplyTo.bHasICCPTransaction && !m_PaymentToApplyTo.strICCPAuthRetRef.IsEmpty()) {
					//we're applying to an ICCP credit card payment that does have an AuthRetRef
					bHasValidRefundToken = true;
				}

				//if a payment, default to swipe
				// (j.jones 2015-09-30 10:37) - PLID 67172 - also default to swipe for refunds
				// when we do not have a payment token to refund to
				if (IsPayment() || (IsRefund() && !bHasValidRefundToken)) 
				{
					m_radioCCSwipe.SetCheck(FALSE);
					m_radioCCCardNotPresent.SetCheck(FALSE);
					m_radioCCDoNotProcess.SetCheck(FALSE);
					m_radioCCRefundToOriginalCard.SetCheck(FALSE);

					// (c.haag 2015-08-26) - PLID 67201 - Try to recall the last authorized transaction type for this user
					// for new payments. The only defaults we support are Swipe and Card Not Present, and since Swipe
					// is the fallback anyway lets just check for Card Not Present
					int nLastTransactionType = GetRemotePropertyInt("LastICCPPaymentTransactionType", -1, 0, GetCurrentUserName());
					if (m_boIsNewPayment && (int)LineItem::CCProcessType::CardNotPresent == nLastTransactionType)
					{
						m_radioCCCardNotPresent.SetCheck(TRUE);
						PostClickRadioCCCardNotPresent();
					}
					else
					{
						m_radioCCSwipe.SetCheck(TRUE);
						PostClickRadioCCSwipe();
					}

					// (j.jones 2015-09-30 10:37) - PLID 67172 - if a refund, we don't have a token,
					// so disable the Refund To Original option
					if (IsRefund()) {						
						m_radioCCRefundToOriginalCard.EnableWindow(FALSE);
					}					
				}
				// (j.jones 2015-09-30 10:37) - PLID 67172 - default to refunding to the original payment
				// if we have a valid token for that payment
				else if (IsRefund() && bHasValidRefundToken) {
					m_radioCCSwipe.SetCheck(FALSE);
					m_radioCCCardNotPresent.SetCheck(FALSE);
					m_radioCCDoNotProcess.SetCheck(FALSE);
					m_radioCCRefundToOriginalCard.SetCheck(TRUE);
					m_radioCCRefundToOriginalCard.EnableWindow(TRUE);
					PostClickRadioCCRefundToOriginalCard();

					//default the merchant account to the account from the original payment
					NXDATALIST2Lib::IRowSettingsPtr pOldMerchantRow = m_CCMerchantAccountCombo->GetCurSel();
					if (m_PaymentToApplyTo.nICCPMerchantID == -1
						|| NULL == m_CCMerchantAccountCombo->SetSelByColumn(maccID, m_PaymentToApplyTo.nICCPMerchantID)) {

						//somehow the old merchant account isn't available - perhaps inactive
						//they won't be allowed to save this payment but will later be told why
						m_CCMerchantAccountCombo->PutCurSel(pOldMerchantRow);
					}
				}
			}
		}
		else {
			//not using ICCP
			m_nxstaticLabelCCTransactionType.ShowWindow(SW_HIDE);
			m_radioCCSwipe.ShowWindow(SW_HIDE);
			m_radioCCCardNotPresent.ShowWindow(SW_HIDE);
			m_radioCCDoNotProcess.ShowWindow(SW_HIDE);
			// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
			m_radioCCRefundToOriginalCard.ShowWindow(SW_HIDE);

			// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct. and zipcode
			GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_HIDE);
			m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_HIDE);
			m_nxstaticLabelCCZipcode.ShowWindow(SW_HIDE);
			m_nxeditCCZipcode.ShowWindow(SW_HIDE);

			// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
			GetDlgItem(IDC_CC_CARD_ON_FILE_COMBO)->ShowWindow(SW_HIDE);
			m_nxstaticLabelCCCardOnFile.ShowWindow(SW_HIDE);

			// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
			m_nxstaticLabelCCLast4.ShowWindow(SW_HIDE);
			m_nxeditCCLast4.ShowWindow(SW_HIDE);

			// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
			m_checkAddCCToProfile.ShowWindow(SW_HIDE);

			m_nxstaticLabelCheckNumOrCCExpDate.ShowWindow(SW_SHOW);
			m_nxstaticLabelBankNameOrCCCardNum.ShowWindow(SW_SHOW);
			m_nxstaticLabelAcctNumOrCCNameOnCard.ShowWindow(SW_SHOW);
			m_nxstaticLabelCCCardType.ShowWindow(SW_SHOW);
			m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_SHOW);
			m_nxeditCheckNumOrCCExpDate.ShowWindow(SW_SHOW);
			m_nxeditCheckNumOrCCExpDate.SetWindowText("");
			m_nxeditBankNameOrCCCardNum.ShowWindow(SW_SHOW);
			m_nxeditBankNameOrCCCardNum.SetWindowText("");
			m_nxeditAcctNumOrCCNameOnCard.ShowWindow(SW_SHOW);
			m_nxeditAcctNumOrCCNameOnCard.SetWindowText("");
			GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_SHOW);
			m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_SHOW);
			m_nxeditBankNumOrCCAuthNum.SetWindowText("");
			GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_SHOW);

			// (j.gruber 2007-07-03 15:01) - PLID 15416 - CC Processing
			if (m_bProcessCreditCards_NonICCP && ((!m_bHasAnyTransaction) && m_boIsNewPayment)) {

				// (d.thompson 2009-06-30) - PLID 34687 - Show the cc processing
				GetDlgItem(IDC_PROCESS_CC)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_PROCESS_CC)->EnableWindow(TRUE);

				GetDlgItem(IDC_BTN_PREVIEW)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_PRINT)->EnableWindow(FALSE);
			}

			// (b.spivey, September 29, 2011) - PLID 40567 - Show Card Swipe if in keyboard mode. 
			if (m_bKeyboardCardSwiper) {
				GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->EnableWindow(TRUE);
			}
		}

		//TES 3/19/2015 - PLID 65069 - Update whether "Gift Cert" shows for tips
		EnsureTipMethodCombo();
		//DRT 2/9/2005 - PLID 15566 - Call new Ensure function to make sure the received/change amount is setup.
		EnsureReceivedArea();

		GetDlgItem(IDC_REFUND_TO_EXISTING_GC_RADIO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REFUND_TO_NEW_GC_RADIO)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-23 13:13) - PLID 65281 - converted the gift certificate dropdown
		// into a searchable list
		GetDlgItem(IDC_SEARCH_GC_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GC_SEARCH_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GC_INCLUDE_OTHERS)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-26 08:56) - PLID 65283 - hide all GC controls
		GetDlgItem(IDC_LABEL_GIFT_CERT_NUMBER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_BALANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_PURCH_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_GC_EXP_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GIFT_CERT_NUMBER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_BALANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_PURCH_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_GC_EXP_DATE)->ShowWindow(SW_HIDE);
		// (j.jones 2015-03-26 09:37) - PLID 65285 - hide the GC X button
		GetDlgItem(IDC_BTN_CLEAR_GC)->ShowWindow(SW_HIDE);

		EnsureLabelArea();

		//TES 3/19/2015 - PLID 65069 - Update whether "Gift Cert" shows for tips
		EnsureTipMethodCombo();

		//for some reason the focus is set to the Cash Drawer dropdown, so focus needs to 
		//be set back to the radio button
		m_radioCharge.SetFocus();

	}NxCatchAll("Error in CPaymentDlg::PostClickRadioCharge()");
}

// (j.jones 2007-04-19 14:12) - PLID 25711 - added PostClick functions
// so the OnClick functions are only called when actually changed
void CPaymentDlg::PostClickRadioGC()
{
	try {
		GetDlgItem(IDC_QUICKBOOKS_BTN)->EnableWindow(FALSE);

		m_radioCash.SetCheck(0);
		m_radioCheck.SetCheck(0);
		m_radioCharge.SetCheck(0);
		m_radioGift.SetCheck(1);
		
		// (a.walling 2007-08-03 16:01) - PLID 26922 - update m_pCurPayMethod
		UpdateCurPayMethod();

		m_nxstaticLabelCheckNumOrCCExpDate.SetWindowText("");
		m_nxstaticLabelBankNameOrCCCardNum.SetWindowText("");
		m_nxstaticLabelAcctNumOrCCNameOnCard.SetWindowText("");
		m_nxstaticLabelCCCardType.SetWindowText("");
		m_nxstaticLabelBankNumOrCCAuthNum.SetWindowText("");
		m_nxstaticLabelCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
		m_nxstaticLabelBankNameOrCCCardNum.ShowWindow(SW_HIDE);
		m_nxstaticLabelAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);
		m_nxstaticLabelCCCardType.ShowWindow(SW_HIDE);
		m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_HIDE);
		m_nxeditCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
		m_nxeditBankNameOrCCCardNum.ShowWindow(SW_HIDE);
		m_nxeditAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_HIDE);
		m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
		m_nxstaticLabelCCTransactionType.ShowWindow(SW_HIDE);
		m_radioCCSwipe.ShowWindow(SW_HIDE);
		m_radioCCCardNotPresent.ShowWindow(SW_HIDE);
		m_radioCCDoNotProcess.ShowWindow(SW_HIDE);
		// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
		m_radioCCRefundToOriginalCard.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct. and zipcode
		GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_HIDE);
		m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_HIDE);
		m_nxstaticLabelCCZipcode.ShowWindow(SW_HIDE);
		m_nxeditCCZipcode.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
		GetDlgItem(IDC_CC_CARD_ON_FILE_COMBO)->ShowWindow(SW_HIDE);
		m_nxstaticLabelCCCardOnFile.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
		m_nxstaticLabelCCLast4.ShowWindow(SW_HIDE);
		m_nxeditCCLast4.ShowWindow(SW_HIDE);

		// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
		m_checkAddCCToProfile.ShowWindow(SW_HIDE);

		//DRT 2/9/2005 - PLID 15566 - Call new Ensure function to make sure the received/change amount is setup.
		EnsureReceivedArea();
		// (r.gonet 2015-04-20) - PLID 65326 - Hide the gift certificate mode radio buttons when in charges.
		if (m_radioRefund.GetCheck() == BST_CHECKED) {
			GetDlgItem(IDC_REFUND_TO_EXISTING_GC_RADIO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_REFUND_TO_NEW_GC_RADIO)->ShowWindow(SW_SHOW);
		}
		// (j.jones 2015-03-23 13:13) - PLID 65281 - converted the gift certificate dropdown
		// into a searchable list
		GetDlgItem(IDC_SEARCH_GC_LIST)->ShowWindow(SW_SHOW);
		// (j.jones 2015-05-11 15:13) - PLID 65281 - the GC Search label is for payments only, not refunds
		GetDlgItem(IDC_GC_SEARCH_LABEL)->ShowWindow(m_radioRefund.GetCheck() == BST_CHECKED ? SW_HIDE : SW_SHOW);
		GetDlgItem(IDC_GC_INCLUDE_OTHERS)->ShowWindow(SW_SHOW);
		// (j.jones 2015-03-26 08:56) - PLID 65283 - show all GC controls
		GetDlgItem(IDC_LABEL_GIFT_CERT_NUMBER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LABEL_GC_VALUE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LABEL_GC_BALANCE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LABEL_GC_PURCH_DATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LABEL_GC_EXP_DATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_GIFT_CERT_NUMBER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_GC_VALUE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_GC_BALANCE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_GC_PURCH_DATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_GC_EXP_DATE)->ShowWindow(SW_SHOW);
		// (j.jones 2015-03-26 09:37) - PLID 65285 - show the GC X button
		GetDlgItem(IDC_BTN_CLEAR_GC)->ShowWindow(SW_SHOW);
		// (r.gonet 2015-04-29 11:09) - PLID 65326 - Ensure the gift certificate controls enabled states are valid.
		EnsureGiftCertificateControls();

		EnsureLabelArea();

		// (j.gruber 2007-07-03 15:01) - PLID 15416 - CC Processing
		//change the buttons to not show Authorizing
		if (m_bProcessCreditCards_NonICCP) {

			// (d.thompson 2009-06-30) - PLID 34687 - hide cc processing
			GetDlgItem(IDC_PROCESS_CC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROCESS_CC)->EnableWindow(FALSE);

			GetDlgItem(IDC_BTN_PREVIEW)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_PRINT)->EnableWindow(TRUE);
		}

		// (b.spivey, September 29, 2011) - PLID 40567 - Hide Card Swipe
		GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->EnableWindow(FALSE); 

		//TES 3/19/2015 - PLID 65069 - Update whether "Gift Cert" shows for tips
		EnsureTipMethodCombo();

		//for some reason the focus is set to the Cash Drawer dropdown, so focus needs to 
		//be set back to the radio button
		//m_radioGift.SetFocus();
		// (j.jones 2015-05-07 09:03) - PLID 65281 - now we always set focus to the search box
		GetDlgItem(IDC_SEARCH_GC_LIST)->SetFocus();
		
	}NxCatchAll("Error in CPaymentDlg::PostClickRadioGC()");
}

void CPaymentDlg::OnTrySetSelFinishedComboCardName(long nRowEnum, long nFlags) 
{
	try {
		if (nFlags == dlTrySetSelFinishedFailure) {
			// This function can be called if after editing the card type list it could not reselect the previously selected card type
			// or if the card type could not be selected after swiping a credit card

			// (e.lally 2007-07-12) PLID 26590 - For existing payments, check if the credit card is inactive. 
				//If so, add it to the list and set the selection
			if(m_boIsNewPayment == FALSE && m_nCreditCardID != -1){
				_RecordsetPtr rs = CreateRecordset("SELECT ID, CardName "
					"FROM CreditCardNamesT "
					"WHERE Inactive = 1 AND ID = %li ", m_nCreditCardID);
				if(!rs->eof){
					IRowSettingsPtr pRow = m_CardNameCombo->GetRow(sriGetNewRow);
					long nCreditCardID = AdoFldLong(rs, "ID");
					pRow->PutValue(cccCardID, nCreditCardID);
					pRow->PutValue(cccCardName, _bstr_t(AdoFldString(rs, "CardName")));

					m_CardNameCombo->AddRow(pRow);
					m_CardNameCombo->SetSelByColumn(cccCardID, nCreditCardID);

				}
			}


			// We only want to handle the case were the card type could not be selected after swiping
			if (GetMainFrame()->DoesOPOSMSRDeviceExist() && m_strSwipedCardName != "") {
				CString strMsg;
				// (e.lally 2007-07-09) Fixed a typo in the message
				strMsg.Format("The credit card type \'%s\' could not be selected for the card just swiped because it does not\n"
							"exist in the card type list.  Please add this card type to the list.", m_strSwipedCardName);
				MessageBox(strMsg);
				// Set the card type to be nothing
				// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
				m_CardNameCombo->SetSelByColumn(cccCardName,_variant_t(""));
				// Set m_strSwipedCardName to blank because the card swipe has been handled
				m_strSwipedCardName = "";
			}
		}
	
	}NxCatchAll("Error in CPaymentDlg::OnTrySetSelFinishedComboCardName");
}


// (j.gruber 2007-08-02 16:59) - PLID 26633 - created a merchant copy receipt
// (d.thompson 2011-01-05) - PLID 42005 - Now takes an already-set location as a parameter, instead of calculating
//	and sending it back.
BOOL CPaymentDlg::PrintCreditReceipt(long nLocationID)
{
	// (d.thompson 2009-07-01) - PLID 34687 - Refunds don't require the patient to get a slip to sign, they just refund it
	//	then give them a receipt afterwards.
	if(m_radioRefund.GetCheck()) {
		return TRUE;
	}

	// (d.thompson 2009-04-10) - PLID 33933 - Debug only workaround because I'm tired of printing test receipts
#ifdef _DEBUG
	if(AfxMessageBox("DEBUG ONLY - Would you like to skip printing the first copy of the receipt?", MB_YESNO) == IDYES) {
		return TRUE;
	}
#endif

	BOOL bIsRefund = (m_radioRefund.GetCheck() == BST_CHECKED);

	CString strAmount;
	GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(strAmount);
	COleCurrency cyAmount = ParseCurrencyFromInterface(strAmount);

	CString strCardHolderName;
	m_nxeditAcctNumOrCCNameOnCard.GetWindowText(strCardHolderName);

	//to get here, they have to have a card number
	CString strCardName = VarString(m_CardNameCombo->GetValue(m_CardNameCombo->CurSel, cccCardName));

	//(e.lally 2007-10-30) PLID 27892 - The display is now always privatized, so we don't need to reformat it
	CString strCardNumber;
	m_nxeditBankNameOrCCCardNum.GetWindowText(strCardNumber);

	if (m_varPaymentID.vt != VT_I4) {
		return FALSE;
	}
	
	// (z.manning 2015-09-08 15:36) - PLID 67224 - Moved most of the receipt printing code to global financial utils.
	// Preserve the existing logic of only printing the merchant copy.
	return PrintCreditCardReceipt(this, TRUE, FALSE, VarLong(m_varPaymentID), nLocationID, bIsRefund, cyAmount, strCardHolderName
		, strCardName, strCardNumber, &m_bSwiped, NULL);
}

// (d.thompson 2009-06-22) - PLID 34687 - QBMS replaces IGS processing
BOOL CPaymentDlg::QBMS_CompleteCreditProcessing(long nLocationID, CPaymentSaveInfo *pInfo /*= NULL*/)
{
	try {
		//now we have to popup the proces credit card screen
		CString strCCNumber, strCCName, strCCExpDate;
		long nCardTypeID = 0, nType = 0, nProvID = -1;
		COleCurrency cyPayAmount(0, 0);

		//(e.lally 2007-10-30) PLID 27892 - The display is now always privatized, so we should pass in the full
		//card number and let the other dlg handle that.
		strCCNumber = m_strCreditCardNumber;
		m_nxeditAcctNumOrCCNameOnCard.GetWindowText(strCCName);
		m_nxeditCheckNumOrCCExpDate.GetWindowText(strCCExpDate);

		//this has already been checked previously, so we know it is valid
		nCardTypeID  = VarLong(m_CardNameCombo->GetValue(m_CardNameCombo->CurSel, cccCardID));

		if (m_radioPayment.GetCheck() ) {
			nType = 1;
		}
		else if (m_radioRefund.GetCheck()) {
			nType = 3;
		}
		else {
			// (d.thompson 2009-04-10) - PLID 33933 - Need to explicitly handle all cases.
			AfxThrowNxException("Invalid payment type for credit card processing.");
		}

		if(m_ProviderCombo->GetCurSel() != -1) {
			_variant_t var = m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), 0);
			if(var.vt != VT_NULL) {
				nProvID = VarLong(var);
			}
		}

		CString strAmount;
		GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(strAmount);
		cyPayAmount = ParseCurrencyFromInterface(strAmount);

		CString strApplyingToTransID;
		if(nType == 3) {
			//We need to provide the TransID for any payment we are attempting to refund.  If we can't acquire it, we can't
			//	allow the refund to process.
			bool bFound = false;
			// (j.jones 2010-09-22 11:27) - PLID 40631 - added support for Chase_CreditTransactionsT, QBMS takes priority
			// (j.jones 2015-09-30 10:37) - PLID 67172 - the transaction ID is already loaded in our struct
			if(m_PaymentToApplyTo.nPaymentID != -1 && m_PaymentToApplyTo.bHasPreICCPTransaction && !m_PaymentToApplyTo.strPreICCPTransactionID.IsEmpty()) {
				//we're applying to a pre-ICCP credit card payment that does have a transaction ID
				// (z.manning 2015-11-03 13:50) - PLID 67528 - Need to assign they payment transaction ID variable
				strApplyingToTransID = m_PaymentToApplyTo.strPreICCPTransactionID;
				bFound = true;
			}

			if(!bFound) {
				AfxMessageBox("No transaction ID could be found for the payment you are applying to.  You may not process a "
					"refund without specifying which transaction you are refunding.");
				return FALSE;
			}
		}

		// (j.gruber 2007-08-03 17:33) - PLID 26620 - process credit card dlg
		// (a.walling 2007-09-21 15:58) - PLID 27468 - Send the payment info pointer
		// (d.thompson 2009-04-10) - PLID 33957 - Added IGS version of processing.
		// (d.thompson 2009-06-22) - PLID 34687 - QBMS replaces IGS processing
		// (d.lange 2010-09-01 14:55) - PLID 40310 - no longer using CQBMS_ProcessCreditCardDlg, its now called CreditCardProcessingDlg
		CCreditCardProcessingDlg dlg(VarLong(m_varPaymentID), strCCNumber, strCCName, strCCExpDate,
							nCardTypeID, cyPayAmount, nType, m_PatientID, nProvID, 
							m_nDrawerID, nLocationID, m_bSwiped == FALSE ? false : true, m_strTrack2,
							NULL, pInfo, true, strApplyingToTransID);
		int nResult = dlg.DoModal();
		if (IDOK == nResult) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (d.thompson 2011-01-05) - PLID 42005 - Pulled this code out of PrintCreditReceiptHeader(), which was calculating it then sending
//	it back by reference through 3 other functions.
long CPaymentDlg::GetCurrentlySelectedLocationID()
{
	long nLocationID = -1;

	//figure out our location
	if(m_LocationCombo->GetCurSel()==-1) {
		if(CString((LPCTSTR)m_LocationCombo->ComboBoxText) == "") {
			nLocationID = GetCurrentLocationID();
		}
		else {
			//must be their current inactive location.
			if(m_varPaymentID.vt == VT_EMPTY) {
				if(m_varBillID.vt == VT_I4) {
					_RecordsetPtr rsLoc = CreateRecordset("SELECT LocationID FROM LineItemT WHERE ID IN (SELECT ID FROM ChargesT WHERE BillID = %li)", VarLong(m_varBillID));
					if(!rsLoc->eof) {
						nLocationID = AdoFldLong(rsLoc, "LocationID", GetCurrentLocationID());
					}
					else {
						nLocationID = GetCurrentLocationID();
					}
				}
				else {
					nLocationID = GetCurrentLocationID();
				}
			}
			else{
				_RecordsetPtr rsLoc = CreateRecordset("SELECT LocationID FROM LineItemT WHERE ID = %li", VarLong(m_varPaymentID));
				if(!rsLoc->eof) {
					nLocationID = AdoFldLong(rsLoc, "LocationID", GetCurrentLocationID());
				}
				else {
					nLocationID = GetCurrentLocationID();
				}
			}
		}
	}
	else {
		_variant_t var = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0);
		if(var.vt == VT_I4) {
			nLocationID = var.lVal;
		}
		else {
			nLocationID = GetCurrentLocationID();
		}
	}

	return nLocationID;
}

// (c.haag 2015-08-18) - PLID 67203 - Gets the currently selected merchant account ID, or -1 if none is selected
long CPaymentDlg::GetCurrentlySelectedMerchantAccountID()
{
	long nAccountID = -1;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_CCMerchantAccountCombo->GetCurSel();
	if (nullptr != pRow)
	{
		nAccountID = VarLong(pRow->Value[maccID], -1);
	}

	return nAccountID;
}


// (d.thompson 2009-06-22) - PLID 34687 - QBMS replaces IGS processing
// (d.thompson 2009-06-30) - PLID 34687 - This function no longer deletes the payment
//	on failure, it only returns FALSE indicating failure, and takes no action of its own.
BOOL CPaymentDlg::QBMS_ProcessTransaction(CPaymentSaveInfo* pInfo /*= NULL*/)
{
	// (j.gruber 2007-08-27 16:33) - PLID 15416 - added more handling if the receipt doesn't print
	try {
		//first, we have to print the receipt once with the new lines
		// (d.thompson 2011-01-05) - PLID 42005 - Get the current location here to start
		long nLocationID = GetCurrentlySelectedLocationID();
		if (PrintCreditReceipt(nLocationID)) {
			// (a.walling 2007-09-21 15:54) - PLID 27468 - Pass the payment info struct
			return QBMS_CompleteCreditProcessing(nLocationID, pInfo);		
		}
		else {
			//they either cancelled or something happened.  Check to see if they want to try printing again
			if (IDYES == MsgBox(MB_YESNO, "The receipt didn't print either due to error or cancellation.  This transaction cannot be processed without printing a receipt for the customer to sign.\r\n"
				"Would you like to try printing the receipt again?\r\n"
				"	- Click Yes to try printing the receipt again.\r\n"
				"	- Click No to cancel processing the transaction.") ){

				//they want to try again
				if (PrintCreditReceipt(nLocationID) ) {
					// (a.walling 2007-09-21 15:54) - PLID 27468 - Pass the payment info struct
					return QBMS_CompleteCreditProcessing(nLocationID, pInfo);
				}
				else {
					//they failed again, kill it
					MsgBox("The receipt has failed to print, the credit card transaction cannot continue.");
				}
			}
		}
	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (a.walling 2007-09-21 15:53) - PLID 27468 - Use the payment info struct
// (d.thompson 2009-04-10) - PLID 33933 - Copied and reworked for IGS processing
// (d.thompson 2009-06-22) - PLID 34687 - QBMS replaces IGS processing
//BOOL CPaymentDlg::IGS_ProcessTransaction(CPaymentSaveInfo* pInfo /*= NULL*/)
/*{
	// (j.gruber 2007-08-27 16:33) - PLID 15416 - added more handling if the receipt doesn't print
	try {
		//first, we have to print the receipt once with the new lines
		long nLocationID;
		if (PrintCreditReceipt(nLocationID)) {
			// (a.walling 2007-09-21 15:54) - PLID 27468 - Pass the payment info struct
			return IGS_CompleteCreditProcessing(nLocationID, pInfo);		
		}
		else {
			//they either cancelled or something happened.  Check to see if they want to try printing again
			if (IDYES == MsgBox(MB_YESNO, "The receipt didn't print either due to error or cancellation.  This transaction cannot be processed without printing a receipt for the customer to sign.\r\n"
				"Would you like to try printing the receipt again?\r\n"
				"	- Click Yes to try printing the receipt again.\r\n"
				"	- Click No to cancel processing the transaction.  This will DELETE this transaction.") ){

				//they want to try again
				if (PrintCreditReceipt(nLocationID) ) {
					// (a.walling 2007-09-21 15:54) - PLID 27468 - Pass the payment info struct
					return IGS_CompleteCreditProcessing(nLocationID, pInfo);
				}
				else {
					//they failed again, kill it
					MsgBox("The receipt has failed to print, this transaction will be deleted.");
					// (d.thompson 2009-04-16) - PLID 33933 - Do not force approved transactions to be deleted.  It shouldn't
					//	be approved at this point, but if it was, we can't delete it.
					DeletePayment(VarLong(m_varPaymentID));
					return FALSE;
				}
			}
			else {
				//we are deleting
				// (d.thompson 2009-04-16) - PLID 33933 - Do not force approved transactions to be deleted.  It shouldn't
				//	be approved at this point, but if it was, we can't delete it.
				DeletePayment(VarLong(m_varPaymentID));
				return FALSE;
			}
		}	
	} NxCatchAll("Error in CPaymentdlg::IGS_ProcessTransaction");

	//I don't think we'll ever get here, but just in case, we have to abort the payment
	// (d.thompson 2009-04-10) - PLID 33933 - Added try/catch here too
	try {
		MsgBox("There has been an error processing the payment.  This payment will be deleted.");
		// (d.thompson 2009-04-16) - PLID 33933 - Do not force approved transactions to be deleted.  It shouldn't
		//	be approved at this point, but if it was, we can't delete it.
		DeletePayment(VarLong(m_varPaymentID));
	} NxCatchAll("Error in IGS_ProcessTransaction : Cleanup.  Payment was not deleted, please do so manually.");
	return FALSE;
}*/


// (d.thompson 2009-04-10) - PLID 33933 - Old version for paymentech, never put live
//BOOL CPaymentDlg::PAYMENTECH_ProcessTransaction(CPaymentSaveInfo* pInfo /*= NULL*/)
/*
	// (j.gruber 2007-08-27 16:33) - PLID 15416 0 added more handling if the receipt doesn't print
	try {
		//first, we have to print the receipt once with the new lines
		long nLocationID;
		if (PrintCreditReceipt(nLocationID) ){

			// (a.walling 2007-09-21 15:54) - PLID 27468 - Pass the payment info struct
			return CompleteCreditProcessing(nLocationID, pInfo);		
		
		}
		else {

			//they either cancelled or something happened.  Check to see if they want to try printing again
			if (IDYES == MsgBox(MB_YESNO, "The receipt didn't print either due to error or cancellation.  This transaction cannot be processed without printing a receipt for the customer to sign.\n"
				"Would you like to try printing the receipt again?\n"
				"	- Click Yes to try printing the receipt again\n"
				"	- Click No to cancel processing the transaction.  This will DELETE this transaction") ){

				//they want to try again
				if (PrintCreditReceipt(nLocationID) ) {

					// (a.walling 2007-09-21 15:54) - PLID 27468 - Pass the payment info struct
					return CompleteCreditProcessing(nLocationID, pInfo);
				}
				else {
					//they failed again, kill it
					MsgBox("The receipt has failed to print, this transaction will be deleted.");

					DeletePayment(VarLong(m_varPaymentID), TRUE);
					
					return FALSE;
				}
			}
			else {

				//we are deleting
				DeletePayment(VarLong(m_varPaymentID), TRUE);

				return FALSE;
			}
		}	
	}NxCatchAll("Error in CPaymentdlg::ProcessTransaction");

	//I don't think we'll ever get here, but just in case, we have to abort the payment
	MsgBox("There has been an error processing the payment.  This payment will be deleted.");
	DeletePayment(VarLong(m_varPaymentID), TRUE);
	return FALSE;

	
}*/


BOOL CPaymentDlg::CheckCCFields() {

	CString strCCNumber, strExpDate, strCCName;
	long nCardType = -1;

	//(e.lally 2007-10-30) PLID 27892 - The display is now always privatized, but since this function
	//only checks for the empty state, it is ok to leave the check for the displayed value.
	m_nxeditBankNameOrCCCardNum.GetWindowText(strCCNumber);
	m_nxeditAcctNumOrCCNameOnCard.GetWindowText(strCCName);
	m_nxeditCheckNumOrCCExpDate.GetWindowText(strExpDate);

	strCCNumber.TrimLeft();
	strCCNumber.TrimRight();

	strExpDate.TrimLeft();
	strExpDate.TrimRight();

	strCCName.TrimRight();
	strCCName.TrimLeft();

	if (strCCName.IsEmpty() || strCCNumber.IsEmpty() || strExpDate.IsEmpty()) {
		return FALSE;
	}

	if (m_CardNameCombo->CurSel == -1) {
		return FALSE;
	}
	
	return TRUE;
}

LRESULT CPaymentDlg::OnPinPadTrackRead(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-08-03 16:53) - PLID 26922 - Added exception handling
	try {
		// (a.walling 2007-08-03 16:47) - PLID 26922 - Check for licensing and permission
		// (d.thompson 2010-09-02) - PLID 40371 - Any license type satisfies
		// (j.jones 2015-09-30 08:58) - PLID 67157 - if you have ICCP, you are allowed to make a credit
		// card payment, you just won't be able to process it
		if (g_pLicense && g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent) && !IsICCPEnabled()) {
			// they are licensed, check permission silently (will prompt for PW when saving)
			if (!CheckCurrentUserPermissions(bioCCProcess, sptWrite, FALSE, 0, TRUE, TRUE)) {
				// they don't have permission!
				MessageBox("You do not have permission to process credit cards. Please check with your office manager.");

				return 0;
			}
		}

		// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
		CString strTrackInfo;
		strTrackInfo = (LPCTSTR)_bstr_t((BSTR)lParam);

		CString strTrack1, strTrack2, strTrack3;

		if (PinPadUtils::GetTrack2InformationFromWholeSwipe(strTrackInfo, m_strTrack2, m_strTrack1, m_strTrack3)) { 

			CString strFirstName, strMiddleName, strLastName, strCardType, strCD, strSuffix, strTitle;
			COleDateTime dtExpire;

			//(e.lally 2007-10-30) PLID 27892 - Use our member variable for the card number to prevent any misuse of a local variable
			PinPadUtils::ParseTrackInformation(m_strTrack1, m_strTrack2, m_strTrack3, m_strCreditCardNumber, strFirstName,
				strMiddleName, strLastName, strSuffix, strTitle, strCardType, strCD, dtExpire);

			TRACE("Expiration Date: " + FormatDateTimeForInterface(dtExpire) + "\r\n");

			//now that we have the information, set up the interface
			if (!m_radioCharge.GetCheck())
				PostClickRadioCharge();

			// Fill the information from the credit card to the screen
			// Set the card type
			m_strSwipedCardName = strCardType;
			
			m_nCreditCardID = -1;
			long nResult = m_CardNameCombo->TrySetSelByColumn(cccCardName,_variant_t(m_strSwipedCardName));
			if (nResult == NXDATALISTLib::sriNoRow) {
				// The credit card type could not be selected
				CString strMsg;
				strMsg.Format("The credit card type \'%s\' could not be selected for the card just swiped because is does not\n"
							"exist in the card type list.  Please add this card type to the list.", strCardType);
				MessageBox(strMsg);
				// Set the card type to be nothing
				// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
				m_CardNameCombo->SetSelByColumn(cccCardName,_variant_t(""));
				// Set m_strSwipedCardName to blank because the card swipe has been handled
				m_strSwipedCardName = "";
			}
			else if (nResult == sriNoRowYet_WillFireEvent) {
				// OnTrySetSelFinishedComboCardName will handle it the selection and it will use m_strSwipedCardName to 
				// determine that it was called because a card was swiped and to get the name of the card
			}
			else {
				ASSERT(nResult >= 0);
				// Set m_strSwipedCardName to blank because the card swipe has been handled
				m_strSwipedCardName = "";
			}

			// Set the expiration date
			// Ensure we do not set it to an invalid date
			COleDateTime dtMin;
			dtMin.ParseDateTime("12/31/1899");
			if(dtExpire.m_status != COleDateTime::invalid && dtExpire >= dtMin) {
				m_nxeditCheckNumOrCCExpDate.SetWindowText(dtExpire.Format("%m/%y"));
			}

			// Set the credit card number
			//(e.lally 2007-10-30) PLID 27892 - Only display the last 4 digits and buffer with X's
			{
				CString strDisplayedCCNumber = MaskCCNumber(m_strCreditCardNumber);
				// (j.jones 2015-09-30 09:31) - PLID 67167 - ICCP uses a different control to show this
				if (!IsICCPEnabled()) {
					m_nxeditBankNameOrCCCardNum.SetWindowText(strDisplayedCCNumber);
				}
				else {
					m_nxeditCCLast4.SetWindowText(strDisplayedCCNumber.Right(4));
				}
			}

			// Set the name on the credit card
			// Format the name
			CString strName = "";
			if (strMiddleName == "" || strMiddleName == " ") {
				strName.Format("%s %s", strFirstName, strLastName);
			}
			else {
				strName.Format("%s %s %s", strFirstName, strMiddleName, strLastName);
			}

			// Add the Title if there is one
			if (strTitle != "") {
				CString strTemp;
				strTemp.Format("%s %s", strTitle, strName);
				strName = strTemp;
			}

			// Add the Suffix if there is one
			if (strSuffix != "") {
				CString strTemp;
				strTemp.Format("%s, %s", strName, strSuffix);
				strName = strTemp;
			}

			m_nxeditAcctNumOrCCNameOnCard.SetWindowText(FixCapitalization(strName));

			// (j.gruber 2007-07-03 16:01) - PLID 15416 - update our boolean that we have swiped the card
			m_bSwiped = TRUE;
			m_swtType = swtPinPad;
			
		}			

	} NxCatchAll("Error in CPaymentDlg::OnPinPadTrackRead");

	return 0;
}

LRESULT CPaymentDlg::OnPinPadInteracDebitDone(WPARAM wParam, LPARAM lParam) {

	return 0;
}

// (a.walling 2007-08-03 16:01) - PLID 26922 - update m_pCurPayMethod
void CPaymentDlg::UpdateCurPayMethod()
{
	try {
		if (m_radioCash.GetCheck())
			m_pCurPayMethod = &m_radioCash;
		else if (m_radioCheck.GetCheck())
			m_pCurPayMethod = &m_radioCheck;
		else if (m_radioCharge.GetCheck())
			m_pCurPayMethod = &m_radioCharge;
		else if (m_radioGift.GetCheck())
			m_pCurPayMethod = &m_radioGift;
		else
			m_pCurPayMethod = NULL;

	} NxCatchAllThrow("Error in UpdateCurPayMethod!");
	// throw that error to the caller!
}

// (a.walling 2007-09-21 17:45) - PLID 27468 - Open the cash drawer if needed
// (a.walling 2007-10-09 10:43) - PLID 9801 - Use the OR'ed EOpenDrawerReaons WORD to keep track of drawer info among 'save and add new' payments.
WORD CPaymentDlg::OpenCashDrawerIfNeeded(CPaymentSaveInfo* pInfo, BOOL bOpenDrawer)
{
	try {
		// EOpenDrawerReason
		WORD odr = m_odrPreviousReasons;
		BOOL bNeedToOpenDrawer = FALSE;

		if (pInfo) {
			CPaymentSaveInfo& Info = *pInfo;

			// (a.walling 2007-10-04 14:41) - PLID 26058 - Use GetProperty so we use workstation properties.
			// (a.walling 2007-08-07 12:03) - PLID 26068 - Open for checks only if they explicitly allow that (only admins can change the pref)
			BOOL bOpenDrawerForChecks = GetPropertyInt("POSCashDrawer_OpenDrawerForChecks", FALSE, 0, true);
			// (a.walling 2007-09-20 12:34) - PLID 9801 - Open for refunds only if explicitly allowed (this is an admin-only setting too)
			BOOL bOpenDrawerForRefunds = GetPropertyInt("POSCashDrawer_OpenDrawerForRefunds", FALSE, 0, true);
			// (a.walling 2007-09-21 09:31) - PLID 27468 - Open for tips and charges only if explicitly allowed
			BOOL bOpenDrawerForTips = GetPropertyInt("POSCashDrawer_OpenDrawerForTips", TRUE, 0, true);
			BOOL bOpenDrawerForCharges = GetPropertyInt("POSCashDrawer_OpenDrawerForCharges", FALSE, 0, true);

			// (a.walling 2007-09-20 12:13) - PLID 9801 - Only open for Refunds (if pref set) or Payments, not Adjustments
			if (	(IsPayment()	&& ((m_radioCheck.GetCheck() && bOpenDrawerForChecks) ||
										(m_radioGift.GetCheck() && bOpenDrawerForChecks) || // (a.walling 2007-09-20 12:14) - PLID 9801 - Treat gift certificates as checks
										(m_radioCharge.GetCheck() && bOpenDrawerForCharges) || // (a.walling 2007-09-21 15:21) - PLID 27468 - Allow open for charges too
										 m_radioCash.GetCheck()))
				||	(IsRefund()		&& bOpenDrawerForRefunds) ) // (a.walling 2007-09-20 12:16) - PLID 9801 - Open for refunds only if pref is set.
			{
				// only want to open if it is a cash (or maybe check/gift or charge) payment, or a refund if pref is set
				
				if (IsPayment() || (IsRefund() && bOpenDrawerForRefunds) ) {
					if (m_boIsNewPayment || m_bAmountChanged) {
						// only open if a new payment, or amount has been changed

						bNeedToOpenDrawer = TRUE;
						if (IsPayment()) {
							if (m_boIsNewPayment) {
								odr |= odrNewPayment;
							} else {
								odr |= odrPayment;
							}							
						} else if (IsRefund()) {
							if (m_boIsNewPayment) {
								odr |= odrNewRefund;
							} else {
								odr |= odrRefund;
							}
						}
					}
				}
			}
			
			// (a.walling 2007-09-20 16:37) - PLID 27468 - only open the drawer for tips if it is cash,
			// or a check or charge (and they have the pref enabled to open for those types of payments)
			if (	Info.GetCashTips() || 
					(Info.GetCheckTips() && bOpenDrawerForChecks) ||
					(Info.GetChargeTips() && bOpenDrawerForCharges) )
			{
				bNeedToOpenDrawer |= bOpenDrawerForTips;
				
				// even if they don't have the preference to open drawer for tips, we can still have a more accurate
				// auditing string just by adding this onto the "Payment" string that is currently being used.
				// (a.walling 2007-10-08 16:16) - PLID 9801 - This is all generated via the odr variable
				/*if (!strReason.IsEmpty()) {
					strReason += " and ";
				}
				strReason += "Tips";*/
				
				odr |= odrTips;
			}

			// (a.walling 2007-09-28 11:01) - PLID 27468 - Also open if any tips were deleted.
			if ( Info.GetDeletedExistingTips() ) {
				bNeedToOpenDrawer |= bOpenDrawerForTips;

				// (a.walling 2007-10-08 16:16) - PLID 9801 - This is all generated via the odr variable
				/*if (!strReason.IsEmpty()) {
					strReason += " and ";
				}
				strReason += "Deleted Tips";*/
				
				odr |= odrDeletedTips;
			}

			if (bNeedToOpenDrawer) {
				odr |= odrOpenDrawer;
			}
		}

		if (bOpenDrawer && ((odr & odrOpenDrawer) > 0)) {
			CString strReason = CPaymentSaveInfo::GetReasonFromWord(odr);

			// may be -1 for a new payment
			long nPaymentID = m_varPaymentID.vt == VT_I4 ? VarLong(m_varPaymentID, -1) : -1;
			
			// (a.walling 2007-10-08 15:44) - PLID 9801
			CString strMultiplePayments;
			BOOL bMultiplePayments = FALSE;

			if ((m_odrPreviousReasons & odrOpenDrawer) > 0) {
				nPaymentID = -1; // multiple payments
				bMultiplePayments = TRUE;
				strMultiplePayments = " multiple payments";
			} else {
				strMultiplePayments = " payment";
			}

			CString strDrawerName = "<No Drawer Selected>";

			if (m_pDrawerList != NULL && m_pDrawerList->GetCurSel() != sriNoRow) {
				IRowSettingsPtr pRow = m_pDrawerList->GetRow(m_pDrawerList->GetCurSel());
				if (pRow) {
					strDrawerName = "(" + VarString(pRow->GetValue(1), "<No Drawer Selected>") + ")";
				}
			}
			
			// (a.walling 2007-05-15 15:31) - PLID 9801 - Open up the cash drawer
			if (GetMainFrame()->GetCashDrawerDevice()) {
				CString strLastState = GetMainFrame()->GetCashDrawerDevice()->GetDrawerLastStateString();

				// (a.walling 2007-09-28 09:08) - PLID 26019 - Only audit if successfully opened
				long nResult = GetMainFrame()->GetCashDrawerDevice()->OpenDrawer();
				if (nResult == OPOS_SUCCESS) {
					AuditEvent(m_PatientID, m_strPatientName, BeginNewAuditEvent(), aeiPOSCashDrawerOpen, nPaymentID, strLastState, FormatString("Drawer Opened for%s: %s %s", strMultiplePayments, strReason, bMultiplePayments ? "" : strDrawerName), aepHigh, aetChanged);
				}
			}
		}

		return odr;
	} NxCatchAll("Error opening cash drawer");

	return m_odrPreviousReasons;
}

// (z.manning, 04/29/2008) - PLID 29836 - Moved the logic to update cash received here from OnKillFocusEditTotal
void CPaymentDlg::UpdateCashReceived(COleCurrency cyAmount)
{
	// (j.jones 2006-06-20 15:23) - PLID 21127 - we used to only auto-change the
	// cash received if the payment amount became greater than the case received,
	// but that frustrated clients when payments defaulted to a given amount
	// and the payment amount was for less money, so now we also change it anytime
	// the payment amount changes at all

	//if the previous amount is not the new amount, then update the cash received
	if(m_cyLastAmount != cyAmount) {
		SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(cyAmount,FALSE,TRUE));
	}
	else {
		CString strCashReceived;
		GetDlgItem(IDC_CASH_RECEIVED)->GetWindowText(strCashReceived);
		if(strCashReceived.GetLength() > 0) {
			COleCurrency cyCashReceived = ParseCurrencyFromInterface(strCashReceived);
			if(cyCashReceived.GetStatus() != COleCurrency::invalid) {
				if(cyCashReceived < cyAmount)
					SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(cyAmount,FALSE,TRUE));
			}
		}
		else {
			SetDlgItemText(IDC_CASH_RECEIVED,FormatCurrencyForInterface(cyAmount,FALSE,TRUE));
		}
	}
		
	OnKillfocusCashReceived();
}

// (j.gruber 2008-05-28 13:31) - PLID 27090 - added the on sel changing method
void CPaymentDlg::OnSelChangingComboLocation(long FAR* nNewSel) 
{
	try {	
		//don't need to worry about inactive because it won't trigger this since the selection is not changing (its staying -1), unless they really are changing it
		if (*nNewSel == sriNoRow) {
			*nNewSel = m_LocationCombo->GetCurSel();				
		}
	}NxCatchAll("Error in CPaymentDlg::OnSelChangingComboLocation");	
}

// (j.jones 2008-07-10 12:43) - PLID 28756 - this function will check
// which insurance company (if any) is selected, and override the 
// payment description and payment category 
void CPaymentDlg::TrySetDefaultInsuranceDescriptions()
{
	try {

		//return if refund is selected
		if(m_radioRefund.GetCheck()) {
			return;
		}

		//return if no insurance is selected
		if(m_InsuranceCombo->CurSel == -1) {
			return;
		}

		//return if patient is selected
		if(VarLong(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, rlcID), -1) == -1) {
			return;
		}

		CString strDefaultDesc = "";
		long nDefaultCategoryID = -1;

		if(m_radioPayment.GetCheck()) {

			strDefaultDesc = VarString(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, rlcDefaultPayDesc), "");
			nDefaultCategoryID = VarLong(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, rlcDefaultPayCategoryID), -1);
		}
		else if(m_radioAdjustment.GetCheck()) {
		
			strDefaultDesc = VarString(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, rlcDefaultAdjDesc), "");
			nDefaultCategoryID = VarLong(m_InsuranceCombo->GetValue(m_InsuranceCombo->CurSel, rlcDefaultAdjCategoryID), -1);		
		}
		else {
			return;
		}

		if(strDefaultDesc.IsEmpty()) {
			//if we aren't overriding the text, store the current text anyways so that
			//we re-set it after setting the category, because we don't want to
			//default the text to 
			m_nxeditEditDescription.GetWindowText(strDefaultDesc);
		}

		if(nDefaultCategoryID != -1) {
			OnSelChosenComboCategory(m_CategoryCombo->SetSelByColumn(0, nDefaultCategoryID));
			//normally when we manually select a category, it fills the description, but we do NOT do this on defaults,
			//and the surrounding code takes care of that
		}

		m_nxeditEditDescription.SetWindowText(strDefaultDesc);

	}NxCatchAll("Error in CPaymentDlg::TrySetDefaultInsuranceDescriptions");
}

// (c.haag 2010-09-20 15:34) - PLID 35723 - Called when switching payment methods to alert the
// user of potentially losing a credit card transaction. Returns FALSE if the user elected to cancel
// the switch.
BOOL CPaymentDlg::WarnOfExistingCCTransaction()
{
	// If we're changing the payment method to something besides charge, and the CC transaction was approved or is in
	// progress, warn the user and let them go back.
	// (j.jones 2015-09-30 10:37) - PLID 67172 - the payment to apply to is now a struct
	if (!m_bInitializing && m_pCurPayMethod == &m_radioCharge && m_PaymentToApplyTo.bHasAnyCCTransaction)
	{
		// If we get here, we are not initializing the dialog, the current payment method (by looking at the form
		// controls) is charge, and the payment we're applying ourselves to has a CC transaction. Warn the user of
		// the consequences of changing the payment method.
		CString strWarn;
		strWarn.Format("This [item] is associated with another item that has undergone credit card processing. Do you still wish "
			"to change the [item] method?");
		if(IsAdjustment()) {
			strWarn.Replace("[item]", "adjustment");
		}
		else if(IsRefund()) {
			strWarn.Replace("[item]", "refund");
		}
		else if(IsPrePayment()) {
			strWarn.Replace("[item]", "prepayment");
		}
		else {
			strWarn.Replace("[item]", "payment");
		}

		if (IDYES == AfxMessageBox(strWarn, MB_YESNO | MB_ICONQUESTION))
		{
			// Now try some reverse psychology. This will stop all those users who click yes to everything.
			strWarn.Format("Changing the method of [an] [item] associated with a credit card payment, "
				"combined with the patient later cancelling the transaction and being given cash back could result "
				"in the office losing money.\n\n"
				"Would you like to CANCEL your change to the [item] method?");
			if(IsAdjustment()) {
				strWarn.Replace("[an]", "an");
				strWarn.Replace("[item]", "adjustment");
			}
			else if(IsRefund()) {
				strWarn.Replace("[an]", "a");
				strWarn.Replace("[item]", "refund");
			}
			else if(IsPrePayment()) {
				strWarn.Replace("[an]", "a");
				strWarn.Replace("[item]", "prepayment");
			}
			else {
				strWarn.Replace("[an]", "a");
				strWarn.Replace("[item]", "payment");
			}

			if (IDNO == AfxMessageBox(strWarn	 ,MB_YESNO | MB_ICONWARNING))
			{
				return TRUE;
			}
		}
		// User changed their mind. Roll back the form change.
		m_radioCash.SetCheck(0);
		m_radioCheck.SetCheck(0);
		m_radioCharge.SetCheck(0);
		m_radioGift.SetCheck(0);		
		if (m_pCurPayMethod) {
			m_pCurPayMethod->SetCheck(1);
		}
		// Return FALSE meaning the payment method change was rolled back
		return FALSE;
	}
	else {
		// No need to do anything
		return TRUE;
	}
}

// (j.jones 2010-09-23 12:13) - PLID 27917 - supported trysetselfinished
void CPaymentDlg::OnTrySetSelFinishedAdjGroupcode(long nRowEnum, long nFlags)
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			// (j.jones 2010-09-23 11:54) - PLID 27917 - handle inactive codes
			if(m_nPendingGroupCodeID != -1) {
				_RecordsetPtr rsAdjCode = CreateParamRecordset("SELECT Description FROM AdjustmentCodesT WHERE ID = {INT}", m_nPendingGroupCodeID);
				if(!rsAdjCode->eof) {
					m_pGroupCodeList->PutComboBoxText(_bstr_t(AdoFldString(rsAdjCode, "Description", "")));
				}
				rsAdjCode->Close();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CPaymentDlg::OnTrySetSelFinishedAdjReason(long nRowEnum, long nFlags)
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			// (j.jones 2010-09-23 11:54) - PLID 27917 - handle inactive codes
			if(m_nPendingReasonCodeID != -1) {
				_RecordsetPtr rsAdjCode = CreateParamRecordset("SELECT Description FROM AdjustmentCodesT WHERE ID = {INT}", m_nPendingReasonCodeID);
				if(!rsAdjCode->eof) {
					m_pReasonList->PutComboBoxText(_bstr_t(AdoFldString(rsAdjCode, "Description", "")));
				}
				rsAdjCode->Close();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-10-11 10:34) - This function is called when we want to update m_nPayToApplyToID.
// (j.jones 2015-09-30 10:37) - PLID 67172 - renamed for clarity
// (j.jones 2015-09-30 10:34) - PLID 67171 - added bWarnAboutCreditCardMismatch,
// set it to true if you're making a refund for a payment and need to see if it's a refundable credit card
void CPaymentDlg::SetPaymentToApplyTo(long nPaymentIDToApplyTo, bool bWarnAboutCreditCardMismatch /*= true*/)
{
	// (j.jones 2015-09-30 10:37) - PLID 67172 - refactored this function to fill a PaymentToApplyTo struct,
	// which will cache credit card transaction information if the payment was processed as a CC
	
	//initialize as an empty item
	m_PaymentToApplyTo = PaymentToApplyTo();
	m_nPayToApplyToMethod = -1;
	// (r.gonet 2015-04-30) - PLID 65326 - Initialize to -1, which is a sentinel for nothing.
	m_nPayToApplyToGiftID = -1;

	if (nPaymentIDToApplyTo != -1)
	{
		// (j.jones 2015-09-30 10:12) - PLID 67168 - updated to include CardConnect
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT "
			"Convert(bit, CASE WHEN QBMS_CreditTransactionsT.ID IS NULL AND Chase_CreditTransactionsT.ID IS NULL AND CardConnect_CreditTransactionT.ID IS NULL THEN 0 ELSE 1 END) AS HasAnyCCTransaction, "
			"Convert(bit, CASE WHEN QBMS_CreditTransactionsT.ID IS NULL AND Chase_CreditTransactionsT.ID IS NULL THEN 0 ELSE 1 END) AS HasPreICCPTransaction, "
			"Convert(bit, CASE WHEN CardConnect_CreditTransactionT.ID IS NULL THEN 0 ELSE 1 END) AS HasICCPTransaction, "
			"CASE WHEN QBMS_CreditTransactionsT.ID Is Not Null THEN QBMS_CreditTransactionsT.TransID ELSE Chase_CreditTransactionsT.TxRefNum END AS PreICCPTransactionID, "
			"CardConnect_CreditTransactionT.AuthRetRef AS ICCPAuthRetRef, CardConnect_CreditTransactionT.AccountID AS ICCPMerchantID, "
			"PaymentsT.PayMethod, LineItemt.GiftID "
			"FROM PaymentsT "
			"LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN QBMS_CreditTransactionsT ON PaymentsT.ID = QBMS_CreditTransactionsT.ID "
			"LEFT JOIN Chase_CreditTransactionsT ON PaymentsT.ID = Chase_CreditTransactionsT.ID "
			"LEFT JOIN CardConnect_CreditTransactionT ON PaymentsT.ID = CardConnect_CreditTransactionT.ID "
			"WHERE PaymentsT.ID = {INT}", nPaymentIDToApplyTo);
		if (!prs->eof) {
			m_PaymentToApplyTo.nPaymentID = nPaymentIDToApplyTo;
			m_PaymentToApplyTo.bHasAnyCCTransaction = AdoFldBool(prs, "HasAnyCCTransaction", FALSE) ? true : false;
			m_PaymentToApplyTo.bHasPreICCPTransaction = AdoFldBool(prs, "HasPreICCPTransaction", FALSE) ? true : false;
			m_PaymentToApplyTo.bHasICCPTransaction = AdoFldBool(prs, "HasICCPTransaction", FALSE) ? true : false;
			m_PaymentToApplyTo.strPreICCPTransactionID = AdoFldString(prs, "PreICCPTransactionID", "");
			m_PaymentToApplyTo.strICCPAuthRetRef = AdoFldString(prs, "ICCPAuthRetRef", "");
			m_PaymentToApplyTo.nICCPMerchantID = AdoFldLong(prs, "ICCPMerchantID", -1);
			//TES 4/20/2015 - PLID 65655 - When applying to an existing payment, track its payment method
			m_nPayToApplyToMethod = AdoFldLong(prs, "PayMethod");
			// (r.gonet 2015-04-30) - PLID 65326 - Track the GiftID of the payment we are applying to.
			m_nPayToApplyToGiftID = AdoFldLong(prs, "GiftID", -1);

			// (j.jones 2015-09-30 10:34) - PLID 67171 - if bWarnAboutCreditCardMismatch is true,
			// this is going to be a refund to a payment, and the caller has not warned about CC
			// mismatches yet, and we need to
			if (bWarnAboutCreditCardMismatch) {
				// If ICCP is enabled, and the payment was not processed using ICCP, this will warn
				// the user that they cannot process the refund using the same card as the payment.

				//This just opens an IDOK messagebox, not yes / no,
				//so we do not need to check a return value.
				CheckWarnCreditCardPaymentRefundMismatch(nPaymentIDToApplyTo);
			}
		}
		prs->Close();
	}
}

// (b.spivey, October 04, 2011) - PLID 40567 - Get the card swipe as keyboard input, and then process it like a normal swipe. 
void CPaymentDlg::OnBnClickedKeyboardCardSwipe()
{
	// (b.spivey, November 18, 2011) - PLID 40567 - Added try/catch.
	try {
		CKeyboardCardSwipeDlg dlg(this);
		if(dlg.DoModal() == IDOK){
			OnMSRDataEvent((WPARAM)&dlg.mtiKeyboardSwipe, (LPARAM)&dlg.cciKeyboardSwipe); 
			
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-07-26 17:48) - PLID 26877 - added ability to filter adjustment reasons
void CPaymentDlg::OnBtnFilterReasonCodes()
{
	try {

		//when not filtering, this is the default where clause
		CString strDefaultWhereClause = "Type = 2 AND Inactive = 0";

		//see if there is currently a reason code
		long nCurReasonCodeID = -1;
		CString strComboBoxText = "";
		if(m_pReasonList->IsComboBoxTextInUse) {
			//keep the existing ID
			nCurReasonCodeID = m_nPendingReasonCodeID;
			strComboBoxText = (LPCTSTR)m_pReasonList->GetComboBoxText();
		}
		else {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReasonList->GetCurSel();
			if (pRow) {
				nCurReasonCodeID = VarLong(pRow->GetValue(rcccID), -1);
			}
		}

		BOOL bCurrentlyShowingAllRecords = (CString((LPCTSTR)m_pReasonList->WhereClause) == strDefaultWhereClause);

		if(nCurReasonCodeID != -1 && (bCurrentlyShowingAllRecords || m_pReasonList->IsComboBoxTextInUse)) {
			if(IDNO == DontShowMeAgain(this, "Filtering the reason code list will clear out your current selection.  Are you sure you wish to do this?", 
				"PaymentFilterReasonCodes", "Warning", FALSE, TRUE))
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

// (b.spivey, October 01, 2012) - PLID 50949 - Update the window caption based on what we're editing right now.
void CPaymentDlg::UpdateWindowText() 
{
	CString strPayType = "";

	if (IsPayment()) {
		if (IsPrePayment()) {
			strPayType = "Pre-Payment"; 
		}
		else {
			strPayType = "Payment";
		}
	}
	else if (IsAdjustment()) {
		strPayType = "Adjustment";
	}
	else if (IsRefund()) {
		strPayType = "Refund"; 
	}

	//show the patient name
	CString strTitle;
	strTitle.Format("%s%s for %s", (m_boIsNewPayment ? "New " : "Edit "), strPayType, m_strPatientName);
	SetWindowText(strTitle);
}

// (c.haag 2014-08-15) - PLID 63379 - Use our own DoModal that requires the caller to pass in their function name and line number
INT_PTR CPaymentDlg::DoModal(LPCTSTR szFuncName, int nLine)
{
	m_strCallingFuncName = szFuncName;
	m_nCallingLineNumber = nLine;
	return DoModal();
}

// (c.haag 2014-08-15) - PLID 63379 - Use our own DoModal that requires the caller to pass in their function name and line number
INT_PTR CPaymentDlg::DoModal()
{
	return __super::DoModal();
}

//TES 3/19/2015 - PLID 65069 - Update whether "Gift Cert" shows for tips
void CPaymentDlg::EnsureTipMethodCombo()
{
	//TES 4/16/2015 - PLID 65171 - If this is a refund, we need to fill the combo with the refund paymethod values
	CString strCash = "1";
	CString strCheck = "2";
	CString strCharge = "3";
	// (r.gonet 2015-04-30) - PLID 65326 - Added the gift paymethod
	CString strGiftCertificate = "4";
	if (m_radioRefund.GetCheck()) {
		strCash = "7";
		strCheck = "8";
		strCharge = "9";
		// (r.gonet 2015-04-30) - PLID 65326 - Added the gift refund paymethod
		strGiftCertificate = "10";
	}
	//TES 3/19/2015 - PLID 65069 - If this is a gift certificate payment, then it should definitely show Gift Cert
	// (r.gonet 2015-06-22) - PLID 65326 - Replaced hardcoded 4 with strGiftCertificate
	if (m_radioGift.GetCheck()) {
		m_pTipList->GetColumn(tlcMethod)->PutComboSource(_bstr_t("SELECT " + strCash + ", 'Cash' UNION SELECT " + strCheck + ", 'Check' UNION SELECT " + strCharge + ", 'Charge' UNION SELECT " + strGiftCertificate + ", 'Gift Cert'"));
	}
	else {
		//TES 3/19/2015 - PLID 65069 - This isn't a gift certificate payment, but if there are any gift certificate tips, we want to keep them, we will yell at the user
		// when they try to save if they haven't updated the tip methods yet
		bool bGiftTipFound = false;
		for (int i = 0; i < m_pTipList->GetRowCount() && !bGiftTipFound; i++) {
			EPayMethod ePayMethod = AsPayMethod(VarLong(m_pTipList->GetValue(i, tlcMethod), -1));
			// (r.gonet 2015-06-22) - PLID 65326 - Added GiftCertificateRefund
			if (ePayMethod == EPayMethod::GiftCertificatePayment || ePayMethod == EPayMethod::GiftCertificateRefund) {
				bGiftTipFound = true;
			}
		}
		if (bGiftTipFound) {
			// (r.gonet 2015-06-22) - PLID 65326 - Replaced hardcoded 4 with strGiftCertificate
			m_pTipList->GetColumn(tlcMethod)->PutComboSource(_bstr_t("SELECT " + strCash + ", 'Cash' UNION SELECT " + strCheck + ", 'Check' UNION SELECT " + strCharge + ", 'Charge' UNION SELECT " + strGiftCertificate + ", 'Gift Cert'"));
		}
		else {
			m_pTipList->GetColumn(tlcMethod)->PutComboSource(_bstr_t("SELECT " + strCash + ", 'Cash' UNION SELECT " + strCheck + ", 'Check' UNION SELECT " + strCharge + ", 'Charge'"));
		}
	}
}

//TES 3/25/2015 - PLID 65436 - Added, determines whether there's something (like, this being a voided payment) that means the whole dialog
// is not editable
BOOL CPaymentDlg::CanEdit()
{
	return m_bCanEdit;
}

//TES 3/25/2015 - PLID 65175 - Determines whether the current user should be able to edit the Tips section
BOOL CPaymentDlg::CanEditTips()
{
	//TES 3/25/2015 - PLID 65175 - If they can't edit anything, they can't edit tips
	if (!CanEdit()) {
		return FALSE;
	}
	//TES 3/25/2015 - PLID 65175 - If we're on a refund, they need the "Refund Tips" permission
	if (IsRefund() && !(GetCurrentUserPermissions(bioRefundTips) & sptWrite)) {
		return FALSE;
	}
	return TRUE;
}

// (j.jones 2015-03-23 16:21) - PLID 65281 - turned the gift certificate dropdown into a search
void CPaymentDlg::OnSelChosenSearchGcList(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			//do not clear out the loaded GC
			return;
		}

		//make sure they didn't select the "no results" row
		long nID = VarLong(pRow->GetValue(GiftCertificateSearchUtils::GCPaymentSearchColumns::gcpscID), GC_NO_RESULTS_ROW);
		if (nID == GC_NO_RESULTS_ROW) {
			//do not clear out the loaded GC
			return;
		}

		//assign this GC ID
		m_nSelectedGiftID = nID;

		// (j.jones 2015-03-25 17:05) - PLID 65283 - this function updates the fields on
		// the screen from the search result's row
		UpdateGiftCertificateControls_FromRow(pRow);

		// (j.jones 2015-03-23 16:22) - PLID 65281 - the remaining logic from the old DL1 dropdown
		// is now handled here
		NewGiftCertificateSelected(nID);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-26 11:08) - PLID 65281 - called after a GC is newly selected,
// either manually or by a barcode scan
void CPaymentDlg::NewGiftCertificateSelected(long nID)
{
	try {

		//the GC should have already been loaded onto the screen
		if (m_nSelectedGiftID != nID) {
			ASSERT(FALSE);
			ThrowNxException("NewGiftCertificateSelected called before the Gift Certificate was loaded!");
		}

		// (j.jones 2015-03-23 16:22) - PLID 65281 - all this logic was kept from the old DL1 dropdown

		if (m_bAmountChanged) {
			//if they've already entered an amount, don't do a thing, 
			//we do not want to overwrite their changes.
			return;
		}

		if (!GetDlgItem(IDC_EDIT_TOTAL)->IsWindowEnabled()) {
			//if it's disabled, we're in an editing state where the amt can't change, so don't change it.
			return;
		}

		// (r.gonet 2015-06-01 09:17) - PLID 65326 - Only initialize the total if this is a payment. For refunds, we should leave the total alone.
		if (!IsPayment()) {
			// (r.gonet 2015-06-01 09:17) - PLID 65326 - It is a refund. Leave the total to refund as the payment amount.
			return;
		}

		//when they choose something, look at the balance.  If it is > than the amount we're applying
		//	to, then leave the amt we're applying to.  If it is less than the amount we're applying to, 
		//	then put in the full balance (use up the card).  They can then save & add another.

		// (j.jones 2015-03-26 11:29) - PLID 65281 - the balance is now a field on the screen,
		// which should have already been loaded before this function was called
		CString strBalance = m_nxeditGCBalance.GetText();
		COleCurrency cyBalance = COleCurrency(0, 0);
		if (!cyBalance.ParseCurrency(strBalance)) {
			//this shouldn't be possible
			ASSERT(FALSE);
			cyBalance = COleCurrency(0, 0);
		}

		//m_cyFinalAmount is the amt we were passed in from the billing (if we came that way -- 0 otherwise)
		if (m_cyFinalAmount != COleCurrency(0, 0)) {
			if (cyBalance < m_cyFinalAmount) {
				//if the balance of our gift card is less than what is required, set our gift amount
				SetDlgItemText(IDC_EDIT_TOTAL, FormatCurrencyForInterface(cyBalance, FALSE));
			}
			else if (m_cyFinalAmount < cyBalance) {
				//the amount of payments needed are less than
				//the total balance of our gift card.  We'll use
				//the needed payment amount, and that will leave
				//a balance on our card.

				//Don't do anything to what is shown.
			}
			else {
				//The amounts are the same.  We can leave what's there, it doesn't matter
			}
		}
		else {
			//the final amount is 0, this means it wasn't set, we're probably on a new payment.
			//set the total field to our gift balance, they can modify if they only want to use 
			//part.
			SetDlgItemText(IDC_EDIT_TOTAL, FormatCurrencyForInterface(cyBalance, FALSE));
			m_bAmountChanged = FALSE;	//setting the text flags this, but we want them to be able to change it.
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-25 17:05) - PLID 65283 - updates the Gift Certificate fields
// on the dialog with the provided fields
void CPaymentDlg::UpdateGiftCertificateControls_FromFields(CString strGiftID, COleCurrency cyTotalValue, COleCurrency cyBalance, COleDateTime dtPurchasedDate, COleDateTime dtExpDate)
{
	try {

		m_nxeditGiftCNumber.SetWindowText(strGiftID);
		m_nxeditGCValue.SetWindowText(FormatCurrencyForInterface(cyTotalValue));
		m_nxeditGCBalance.SetWindowText(FormatCurrencyForInterface(cyBalance));
		m_nxeditGCPurchDate.SetWindowText(FormatDateTimeForInterface(dtPurchasedDate, NULL, dtoDate));
		//Exp. Date can be null
		if (dtExpDate.GetStatus() == COleCurrency::valid) {
			m_nxeditGCExpDate.SetWindowText(FormatDateTimeForInterface(dtExpDate, NULL, dtoDate));
		}
		else {
			m_nxeditGCExpDate.SetWindowText("");
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-25 17:05) - PLID 65283 - this function updates the fields on
// the screen from the search result's row
void CPaymentDlg::UpdateGiftCertificateControls_FromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		//this should not be called on a NULL row
		if (pRow == NULL) {
			ASSERT(FALSE);
			return;
		}

		//m_nSelectedGiftID should have been already assigned
		ASSERT(m_nSelectedGiftID == VarLong(pRow->GetValue(GiftCertificateSearchUtils::GCPaymentSearchColumns::gcpscID)));

		CString strGiftID = VarString(pRow->GetValue(GiftCertificateSearchUtils::GCPaymentSearchColumns::gcpscGiftID));
		COleCurrency cyTotalValue = VarCurrency(pRow->GetValue(GiftCertificateSearchUtils::GCPaymentSearchColumns::gcpscTotalValue));
		COleCurrency cyBalance = VarCurrency(pRow->GetValue(GiftCertificateSearchUtils::GCPaymentSearchColumns::gcpscBalance));
		COleDateTime dtPurchasedDate = VarDateTime(pRow->GetValue(GiftCertificateSearchUtils::GCPaymentSearchColumns::gcpscPurchasedDate));
		//Exp. Date can be null
		COleDateTime dtExpDate = VarDateTime(pRow->GetValue(GiftCertificateSearchUtils::GCPaymentSearchColumns::gcpscExpireDate), g_cdtNull);
		UpdateGiftCertificateControls_FromFields(strGiftID, cyTotalValue, cyBalance, dtPurchasedDate, dtExpDate);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-25 17:05) - PLID 65283 - this function updates the fields on
// the screen from a GiftCertificatesT.ID. Fails if the ID is not found.
void CPaymentDlg::UpdateGiftCertificateControls_FromID(long nID)
{
	try {

		//ID is required
		if (nID == -1) {
			ASSERT(FALSE);
			ClearGiftCertificate();
			ThrowNxException("UpdateGiftCertificateControls_FromID not given an ID!");
		}

		//pull this data from the API
		NexTech_Accessor::_GetGiftCertificatePaymentListIDLookupInputPtr pInput(__uuidof(NexTech_Accessor::GetGiftCertificatePaymentListIDLookupInput));
		pInput->SearchByGiftID = VARIANT_FALSE;
		pInput->ID = _bstr_t(AsString(nID));
		//since this is loading an existing GC, we will include expired/voided GCs
		pInput->IncludeExpiredAndVoidedGCs = VARIANT_TRUE;
		NexTech_Accessor::_FindGiftCertificatePaymentListResultPtr pResult =
			GetAPI()->GetGiftCertificateInfo_PaymentList(GetAPISubkey(), GetAPILoginToken(), pInput);

		if (pResult != NULL && pResult->Result != NULL) {
			m_nSelectedGiftID = (long)atoi(VarString(pResult->Result->ID));
			CString strGiftID = (LPCTSTR)pResult->Result->GiftID;
			COleCurrency cyTotalValue = AsCurrency(pResult->Result->TotalValue);
			COleCurrency cyBalance = AsCurrency(pResult->Result->Balance);
			COleDateTime dtPurchasedDate = pResult->Result->PurchaseDate;
			//Exp. Date can be null
			_variant_t varExpDate = pResult->Result->ExpDate->GetValueOrDefault(g_cvarNull);
			COleDateTime dtExpDate = VarDateTime(varExpDate, g_cdtNull);
			UpdateGiftCertificateControls_FromFields(strGiftID, cyTotalValue, cyBalance, dtPurchasedDate, dtExpDate);
		}
		else {
			//this should not be possible
			ASSERT(FALSE);
			ThrowNxException("UpdateGiftCertificateControls_FromID given an invalid GC ID %li!", nID);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-26 09:18) - PLID 65285 - added ability to clear the gift certificate
void CPaymentDlg::ClearGiftCertificate()
{
	try {

		m_nSelectedGiftID = -1;
		m_nxeditGiftCNumber.SetWindowText("");
		m_nxeditGCValue.SetWindowText("");
		m_nxeditGCBalance.SetWindowText("");
		m_nxeditGCPurchDate.SetWindowText("");
		m_nxeditGCExpDate.SetWindowText("");

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-26 09:34) - PLID 65285 - added ability to clear the current gift certificate
void CPaymentDlg::OnBtnClearGC()
{
	try {

		ClearGiftCertificate();

		//set focus in the GC search
		GetDlgItem(IDC_SEARCH_GC_LIST)->SetFocus();

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-04-20) - PLID 65326 - Handler for when the user clicks the refund to existing gift certificate radio button
void CPaymentDlg::OnBnClickedRefundToExistingGcRadio()
{
	try {
		ClearGiftCertificate();
		// (r.gonet 2015-04-29 11:09) - PLID 65326 - Enable the existing gift certificate fields.
		EnsureGiftCertificateControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-04-20) - PLID 65326 - Handler for when the user clicks the refund to new gift certificate radio button
void CPaymentDlg::OnBnClickedRefundToNewGcRadio()
{
	try {
		ClearGiftCertificate();
		// (r.gonet 2015-04-29 11:09) - PLID 65326 - Disable the existing gift certificate fields.
		EnsureGiftCertificateControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-04-29 11:09) - PLID 65326 - Ensures the gift certificate controls' enabled states are correct.
void CPaymentDlg::EnsureGiftCertificateControls()
{
	UpdateData(TRUE);
	BOOL bEnable = (IsPayment() || (IsRefund() && m_bRefundToExistingGC));
	GetDlgItem(IDC_SEARCH_GC_LIST)->EnableWindow(bEnable);
	// (j.jones 2015-03-26 09:37) - PLID 65285 - show the GC X button
	GetDlgItem(IDC_BTN_CLEAR_GC)->EnableWindow(bEnable);
	// (j.jones 2015-03-26 08:56) - PLID 65283 - show all GC controls
	GetDlgItem(IDC_LABEL_GIFT_CERT_NUMBER)->EnableWindow(bEnable);
	GetDlgItem(IDC_LABEL_GC_VALUE)->EnableWindow(bEnable);
	GetDlgItem(IDC_LABEL_GC_BALANCE)->EnableWindow(bEnable);
	GetDlgItem(IDC_LABEL_GC_PURCH_DATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_LABEL_GC_EXP_DATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_GIFT_CERT_NUMBER)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_GC_VALUE)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_GC_BALANCE)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_GC_PURCH_DATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_GC_EXP_DATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_GC_INCLUDE_OTHERS)->EnableWindow(bEnable);
	// (j.jones 2015-06-17 15:37) - PLID 65285 - enable/disable the GC X button
	GetDlgItem(IDC_BTN_CLEAR_GC)->EnableWindow(bEnable);
}

// (r.gonet 2015-04-20) - PLID 65327 - Returns the paymethod currently selected on the payment dialog.
// Returns EPayMethod::Invalid if nothing is checked.
EPayMethod CPaymentDlg::GetPayMethod()
{
	if (IsPayment()) {
		if (m_radioCash.GetCheck() == BST_CHECKED) {
			return EPayMethod::CashPayment;
		} else if (m_radioCheck.GetCheck() == BST_CHECKED) {
			return EPayMethod::CheckPayment;
		} else if (m_radioCharge.GetCheck() == BST_CHECKED) {
			return EPayMethod::ChargePayment;
		} else if (m_radioGift.GetCheck() == BST_CHECKED) {
			return EPayMethod::GiftCertificatePayment;
		}
	} else if (IsAdjustment()) {
		return EPayMethod::Adjustment;
	} else if (IsRefund()) {
		if (m_radioCash.GetCheck() == BST_CHECKED) {
			return EPayMethod::CashRefund;
		} else if (m_radioCheck.GetCheck() == BST_CHECKED) {
			return EPayMethod::CheckRefund;
		} else if (m_radioCharge.GetCheck() == BST_CHECKED) {
			return EPayMethod::ChargeRefund;
		} else if (m_radioGift.GetCheck() == BST_CHECKED) {
			return EPayMethod::GiftCertificateRefund;
		}
	}

	return EPayMethod::Invalid;
}

// (r.gonet 2015-04-20) - PLID 65327 - Returns the descriptive name of the pay method.
CString CPaymentDlg::GetPayMethodName(EPayMethod ePayMethod, CString strDefault, bool bIncludeLineItemTypeName/* = true*/, bool bCapitalized/* = false*/)
{
	CString strName;
	switch (ePayMethod) {
	case EPayMethod::CashPayment:
		strName = "cash" + CString(bIncludeLineItemTypeName ? " payment" : "");
		break;
	case EPayMethod::CheckPayment:
		strName = "check" + CString(bIncludeLineItemTypeName ? " payment" : "");
		break;
	case EPayMethod::ChargePayment:
		strName = "charge" + CString(bIncludeLineItemTypeName ? " payment" : "");
		break;
	case EPayMethod::GiftCertificatePayment:
		strName = "gift certificate" + CString(bIncludeLineItemTypeName ? " payment" : "");
		break;
	case EPayMethod::CashRefund:
		strName = "cash" + CString(bIncludeLineItemTypeName ? " refund" : "");
		break;
	case EPayMethod::CheckRefund:
		strName = "check" + CString(bIncludeLineItemTypeName ? " refund" : "");
		break;
	case EPayMethod::ChargeRefund:
		strName = "charge" + CString(bIncludeLineItemTypeName ? " refund" : "");
		break;
	case EPayMethod::GiftCertificateRefund:
		strName = "gift certificate" + CString(bIncludeLineItemTypeName ? " refund" : "");
		break;
	case EPayMethod::Adjustment:
	case EPayMethod::LegacyAdjustment:
		strName = "adjustment";
		break;
	case EPayMethod::Invalid:
		strName = "";
		break;
	default:
		ASSERT(FALSE);
		strName = "";
		break;
	}

	if (bCapitalized) {
		strName = FixCapitalization(strName);
	}

	if (strName == "") {
		strName = strDefault;
	}
	
	return strName;
}

// (j.jones 2015-09-30 09:18) - PLID 67163 - moves the Merchant Account combo to be next to
// the authorization # field, for ICCP only, for processed payments only
void CPaymentDlg::MoveICCPMerchantAccountCombo_Processed()
{
	if (m_bNeedToMoveICCPMerchantAccountCombo_Processed) {
		CRect rcMerchantAcctLabel, rcMerchantAcctCombo;
		CRect rcNameOnCardEdit, rcAuthNumLabel, rcAuthNumEdit;
		GetDlgItem(IDC_LABEL_CC_MERCHANT_ACCT)->GetWindowRect(rcMerchantAcctLabel);
		GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->GetWindowRect(rcMerchantAcctCombo);
		GetDlgItem(IDC_EDIT_ACCTNUM_OR_CCNAMEONCARD)->GetWindowRect(rcNameOnCardEdit);
		GetDlgItem(IDC_LABEL_BANKNUM_OR_CCAUTHNUM)->GetWindowRect(rcAuthNumLabel);
		GetDlgItem(IDC_EDIT_BANKNUM_OR_CCAUTHNUM)->GetWindowRect(rcAuthNumEdit);
		ScreenToClient(rcMerchantAcctLabel);
		ScreenToClient(rcMerchantAcctCombo);
		ScreenToClient(rcNameOnCardEdit);
		ScreenToClient(rcAuthNumLabel);
		ScreenToClient(rcAuthNumEdit);

		//move the merchant combo and label
		rcMerchantAcctLabel.SetRect(rcNameOnCardEdit.left, rcAuthNumLabel.top, rcNameOnCardEdit.right, rcAuthNumLabel.bottom);
		rcMerchantAcctCombo.SetRect(rcNameOnCardEdit.left, rcAuthNumEdit.top, rcNameOnCardEdit.right, rcAuthNumEdit.bottom);

		GetDlgItem(IDC_LABEL_CC_MERCHANT_ACCT)->MoveWindow(rcMerchantAcctLabel);
		GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->MoveWindow(rcMerchantAcctCombo);

		//lastly remove the : from Merchant Acct
		m_nxstaticLabelCCMerchantAcct.SetWindowText("Merchant Acct");

		m_bNeedToMoveICCPMerchantAccountCombo_Processed = false;
	}
}

// (j.jones 2015-09-30 09:39) - PLID 67166 - Loading a processed ICCP payment shows
// the same controls for all process types: swipe, 'card not present', or refund to original card
// This function will show/hide controls accordingly.
void CPaymentDlg::UpdateDisplayForProcessedICCPInfo()
{
	//this is an existing cardconnect transaction
	ASSERT(m_bHasICCPTransaction && VarLong(m_varPaymentID, -1) != -1);

	//show the merchant account - it should have already had a row selected
	GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_SHOW);
	m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_SHOW);

	//hide zip code
	m_nxstaticLabelCCZipcode.ShowWindow(SW_HIDE);
	m_nxeditCCZipcode.ShowWindow(SW_HIDE);

	//show Auth Num & Last 4
	m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_SHOW);
	m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_SHOW);
	m_nxstaticLabelCCLast4.ShowWindow(SW_SHOW);
	m_nxeditCCLast4.ShowWindow(SW_SHOW);

	//show the card type
	m_nxstaticLabelCCCardType.ShowWindow(SW_SHOW);
	GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_SHOW);

	//move the card type and merchant account combos
	MoveICCPCCCombo();
	MoveICCPMerchantAccountCombo_Processed();

	//hide the "add card to payment profile" checkbox
	m_checkAddCCToProfile.ShowWindow(SW_HIDE);
}

// (j.jones 2015-09-30 08:58) - PLID 67157 - added PostClick functions
// for CC processing types
void CPaymentDlg::PostClickRadioCCSwipe()
{
	try {

		//show all Swipe controls
		{
			// (j.jones 2015-09-30 09:18) - PLID 67163 - if this is an existing CardConnect transaction,
			// hide the zip code, show the auth # and Last 4 on the card
			if (m_bHasICCPTransaction && VarLong(m_varPaymentID,-1) != -1) {
				//existing CardConnect transaction
				UpdateDisplayForProcessedICCPInfo();
			}
			else {
				//new payment or non-CardConnect transaction

				// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct. and zipcode
				GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_SHOW);
				m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_SHOW);

				//if no merchant acct. is selected, select the first entry
				if (m_CCMerchantAccountCombo->GetCurSel() == NULL && m_CCMerchantAccountCombo->GetRowCount() > 0) {
					m_CCMerchantAccountCombo->PutCurSel(m_CCMerchantAccountCombo->GetFirstRow());
				}

				//show zip code
				m_nxstaticLabelCCZipcode.ShowWindow(SW_SHOW);
				m_nxeditCCZipcode.ShowWindow(SW_SHOW);

				//if the CC zip code is empty, fill from G1
				CString strCCZipCode;
				m_nxeditCCZipcode.GetWindowText(strCCZipCode);
				strCCZipCode.Trim();
				if (strCCZipCode.IsEmpty()) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT Zip FROM PersonT WHERE ID = {INT}", m_PatientID);
					if (!rs->eof) {
						m_nxeditCCZipcode.SetWindowText(AdoFldString(rs, "Zip", ""));
					}
					rs->Close();
				}

				//hide the auth num
				m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_HIDE);
				m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_HIDE);

				//hide the last 4
				m_nxstaticLabelCCLast4.ShowWindow(SW_HIDE);
				m_nxeditCCLast4.ShowWindow(SW_HIDE);

				//hide the card type
				m_nxstaticLabelCCCardType.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_HIDE);

				// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
				m_checkAddCCToProfile.ShowWindow(SW_SHOW);
			}
		}

		//hide all non-Swipe controls
		{
			// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
			GetDlgItem(IDC_CC_CARD_ON_FILE_COMBO)->ShowWindow(SW_HIDE);
			m_nxstaticLabelCCCardOnFile.ShowWindow(SW_HIDE);
			m_nxstaticLabelCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
			m_nxstaticLabelBankNameOrCCCardNum.ShowWindow(SW_HIDE);
			m_nxstaticLabelAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);					
			m_nxeditCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
			m_nxeditBankNameOrCCCardNum.ShowWindow(SW_HIDE);
			m_nxeditAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 08:58) - PLID 67157 - added PostClick functions
// for CC processing types
void CPaymentDlg::PostClickRadioCCCardNotPresent()
{
	try {

		//show all Card Not Present controls
		{
			// (j.jones 2015-09-30 09:39) - PLID 67166 - if this is an existing CardConnect transaction,
			// hide the zip code, show the auth # and Last 4 on the card
			if (m_bHasICCPTransaction && VarLong(m_varPaymentID, -1) != -1) {
				//existing CardConnect transaction
				UpdateDisplayForProcessedICCPInfo();
			}
			else {
				//new payment or non-CardConnect transaction

				// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct.
				GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_SHOW);
				m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_SHOW);

				//if no merchant acct. is selected, select the first entry
				if (m_CCMerchantAccountCombo->GetCurSel() == NULL && m_CCMerchantAccountCombo->GetRowCount() > 0) {
					m_CCMerchantAccountCombo->PutCurSel(m_CCMerchantAccountCombo->GetFirstRow());
				}

				// (j.jones 2015-09-30 09:15) - PLID 67158 - added zipcode
				m_nxstaticLabelCCZipcode.ShowWindow(SW_SHOW);
				m_nxeditCCZipcode.ShowWindow(SW_SHOW);

				//if the CC zip code is empty, fill from G1
				CString strCCZipCode;
				m_nxeditCCZipcode.GetWindowText(strCCZipCode);
				strCCZipCode.Trim();
				if (strCCZipCode.IsEmpty()) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT Zip FROM PersonT WHERE ID = {INT}", m_PatientID);
					if (!rs->eof) {
						m_nxeditCCZipcode.SetWindowText(AdoFldString(rs, "Zip", ""));
					}
					rs->Close();
				}

				// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
				GetDlgItem(IDC_CC_CARD_ON_FILE_COMBO)->ShowWindow(SW_SHOW);
				m_nxstaticLabelCCCardOnFile.ShowWindow(SW_SHOW);

				// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
				m_nxstaticLabelCCLast4.ShowWindow(SW_HIDE);
				m_nxeditCCLast4.ShowWindow(SW_HIDE);

				//hide the card type
				m_nxstaticLabelCCCardType.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_HIDE);

				//hide the auth num
				m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_HIDE);
				m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_HIDE);

				// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
				// (j.jones 2015-09-30 09:44) - PLID 67164 - only show the CC profile checkbox if <Manual Entry> is selected
				bool bManualEntry = false;
				{
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_CCCardOnFileCombo->GetCurSel();
					if (pRow != NULL && VarString(pRow->GetValue(ppcCardConnectProfileAccountID), "").IsEmpty()) {
						bManualEntry = true;
					}
				}
				m_checkAddCCToProfile.ShowWindow(bManualEntry ? SW_SHOW : SW_HIDE);

				//if manual entry is selected and there are cards on file,
				//change the combo box text blue to get their attention
				if (bManualEntry && m_CCCardOnFileCombo->GetRowCount() > 1) {
					m_CCCardOnFileCombo->PutComboForeColor(RGB(0, 0, 255));
				}
				else {
					m_CCCardOnFileCombo->PutComboForeColor(RGB(0, 0, 0));
				}
			}
		}

		//hide all non-Card Not Present controls
		{
			m_nxstaticLabelCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
			m_nxstaticLabelBankNameOrCCCardNum.ShowWindow(SW_HIDE);
			m_nxstaticLabelAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);
			m_nxeditCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
			m_nxeditBankNameOrCCCardNum.ShowWindow(SW_HIDE);
			m_nxeditAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);			
			
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 09:31) - PLID 67167 - moves the Card Type combo to be next to
// the Last 4 field, for ICCP only
void CPaymentDlg::MoveICCPCCCombo()
{
	if (m_bNeedToMoveICCPCCCombo) {
		CRect rcCardTypeLabel, rcCardTypeCombo, rcCardTypeButton;
		CRect rcNameOnCardEdit, rcLast4Label, rcLast4Edit;
		GetDlgItem(IDC_LABEL_CCCARDTYPE)->GetWindowRect(rcCardTypeLabel);
		GetDlgItem(IDC_COMBO_CARD_NAME)->GetWindowRect(rcCardTypeCombo);
		GetDlgItem(IDC_EDIT_CARD_NAME)->GetWindowRect(rcCardTypeButton);
		GetDlgItem(IDC_EDIT_ACCTNUM_OR_CCNAMEONCARD)->GetWindowRect(rcNameOnCardEdit);
		GetDlgItem(IDC_LABEL_CC_LAST4)->GetWindowRect(rcLast4Label);
		GetDlgItem(IDC_EDIT_CC_LAST4)->GetWindowRect(rcLast4Edit);
		ScreenToClient(rcCardTypeLabel);
		ScreenToClient(rcCardTypeCombo);
		ScreenToClient(rcCardTypeButton);
		ScreenToClient(rcNameOnCardEdit);
		ScreenToClient(rcLast4Label);
		ScreenToClient(rcLast4Edit);

		//move the card combo, label, and button
		rcCardTypeLabel.SetRect(rcNameOnCardEdit.left, rcLast4Label.top, rcCardTypeLabel.Width() + rcNameOnCardEdit.left, rcNameOnCardEdit.Height() + rcLast4Label.top);
		rcCardTypeCombo.SetRect(rcNameOnCardEdit.left, rcLast4Edit.top, rcCardTypeCombo.Width() + rcNameOnCardEdit.left - rcCardTypeButton.Width() - 3, rcCardTypeCombo.Height() + rcLast4Edit.top);
		rcCardTypeButton.SetRect(rcNameOnCardEdit.right - rcCardTypeButton.Width(), rcCardTypeCombo.top, rcNameOnCardEdit.right, rcCardTypeCombo.bottom);

		GetDlgItem(IDC_LABEL_CCCARDTYPE)->MoveWindow(rcCardTypeLabel);
		GetDlgItem(IDC_COMBO_CARD_NAME)->MoveWindow(rcCardTypeCombo);
		GetDlgItem(IDC_EDIT_CARD_NAME)->MoveWindow(rcCardTypeButton);

		m_bNeedToMoveICCPCCCombo = false;
	}
}

// (j.jones 2015-09-30 08:58) - PLID 67157 - added PostClick functions
// for CC processing types
void CPaymentDlg::PostClickRadioCCDoNotProcess()
{
	try {

		//show all Do Not Process controls
		{
			//the Card Type combo has to move, if we haven't already moved it
			MoveICCPCCCombo();

			//these are the conventional CC controls		
			m_nxstaticLabelAcctNumOrCCNameOnCard.ShowWindow(SW_SHOW);
			m_nxstaticLabelCCCardType.ShowWindow(SW_SHOW);
			m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_SHOW);
			m_nxeditAcctNumOrCCNameOnCard.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_SHOW);
			m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_SHOW);

			// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
			m_nxstaticLabelCCLast4.ShowWindow(SW_SHOW);
			m_nxeditCCLast4.ShowWindow(SW_SHOW);
		}

		//hide all non-Do Not Process controls
		{

			// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct. and zipcode
			GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_HIDE);
			m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_HIDE);
			m_nxstaticLabelCCZipcode.ShowWindow(SW_HIDE);
			m_nxeditCCZipcode.ShowWindow(SW_HIDE);

			// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
			GetDlgItem(IDC_CC_CARD_ON_FILE_COMBO)->ShowWindow(SW_HIDE);
			m_nxstaticLabelCCCardOnFile.ShowWindow(SW_HIDE);

			// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
			m_checkAddCCToProfile.ShowWindow(SW_HIDE);

			m_nxstaticLabelBankNameOrCCCardNum.ShowWindow(SW_HIDE);
			m_nxeditBankNameOrCCCardNum.ShowWindow(SW_HIDE);
			m_nxstaticLabelCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
			m_nxeditCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
void CPaymentDlg::PostClickRadioCCRefundToOriginalCard()
{
	try {

		//show all Refund To Original Card controls
		{
			// (j.jones 2015-09-30 09:39) - PLID 67166 - if this is an existing CardConnect transaction,
			// show the auth # and Last 4 on the card
			if (m_bHasICCPTransaction && VarLong(m_varPaymentID, -1) != -1) {
				//existing CardConnect transaction
				UpdateDisplayForProcessedICCPInfo();
			}
			else {
				//new refund or non-CardConnect transaction

				//only show the merchant account
				GetDlgItem(IDC_CC_MERCHANT_ACCT_COMBO)->ShowWindow(SW_SHOW);
				m_nxstaticLabelCCMerchantAcct.ShowWindow(SW_SHOW);

				//if no merchant acct. is selected, select the first entry
				if (m_CCMerchantAccountCombo->GetCurSel() == NULL && m_CCMerchantAccountCombo->GetRowCount() > 0) {
					m_CCMerchantAccountCombo->PutCurSel(m_CCMerchantAccountCombo->GetFirstRow());
				}

				// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
				m_nxstaticLabelCCLast4.ShowWindow(SW_HIDE);
				m_nxeditCCLast4.ShowWindow(SW_HIDE);

				//hide the card type
				m_nxstaticLabelCCCardType.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_COMBO_CARD_NAME)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EDIT_CARD_NAME)->ShowWindow(SW_HIDE);

				//hide the auth num
				m_nxstaticLabelBankNumOrCCAuthNum.ShowWindow(SW_HIDE);
				m_nxeditBankNumOrCCAuthNum.ShowWindow(SW_HIDE);

				//hide the zip code
				m_nxstaticLabelCCZipcode.ShowWindow(SW_HIDE);
				m_nxeditCCZipcode.ShowWindow(SW_HIDE);
			}
		}

		//hide all non-Refund To Original Card controls
		{			
			m_checkAddCCToProfile.ShowWindow(SW_HIDE);

			// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
			GetDlgItem(IDC_CC_CARD_ON_FILE_COMBO)->ShowWindow(SW_HIDE);
			m_nxstaticLabelCCCardOnFile.ShowWindow(SW_HIDE);

			m_nxstaticLabelCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
			m_nxstaticLabelBankNameOrCCCardNum.ShowWindow(SW_HIDE);
			m_nxstaticLabelAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);
			m_nxeditCheckNumOrCCExpDate.ShowWindow(SW_HIDE);
			m_nxeditBankNameOrCCCardNum.ShowWindow(SW_HIDE);
			m_nxeditAcctNumOrCCNameOnCard.ShowWindow(SW_HIDE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
void CPaymentDlg::OnRadioSwipeCard()
{
	try {

		PostClickRadioCCSwipe();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
void CPaymentDlg::OnRadioCardNotPresent()
{
	try {

		PostClickRadioCCCardNotPresent();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
void CPaymentDlg::OnRadioDoNotProcess()
{
	try {

		PostClickRadioCCDoNotProcess();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
void CPaymentDlg::OnRadioCcRefundToOriginalCard()
{
	try {

		PostClickRadioCCRefundToOriginalCard();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 09:40) - PLID 67165 - defines a thread to load CC payment profiles from the API
void LoadCCPaymentProfilesThread(HWND hwndNotify, const long nPatientPersonID, const bool bIncludeExpiredProfiles, const CString strDefaultProfileAccountID)
{
	try
	{
		NexTech_Accessor::_ICCPPaymentProfilesPtr pProfiles = GetAPI()->GetICCPPaymentProfiles(GetAPISubkey(), GetAPILoginToken()
			, _bstr_t(nPatientPersonID), bIncludeExpiredProfiles ? VARIANT_TRUE : VARIANT_FALSE);
		
		Nx::SafeArray<IUnknown*> *saProfiles = NULL;

		if (pProfiles != NULL && pProfiles->Profiles != NULL) {
			saProfiles = new Nx::SafeArray<IUnknown*>(pProfiles->Profiles);
		}

		// Post back to the main thread
		::PostMessage(hwndNotify, LoadCCPaymentProfilesThread_NotifyMessage, (WPARAM)saProfiles, (LPARAM)(LPCTSTR)strDefaultProfileAccountID);

	}NxCatchAllThread(__FUNCTION__);
}

// (j.jones 2015-09-30 09:40) - PLID 67165 - added ability to load payment profiles from the API, asynchronously
// if strDefaultProfileAccountID is filled, that account is selected, otherwise the default account is, or <manual entry>
void CPaymentDlg::BeginLoadCCPaymentProfilesThread(bool bIncludeExpiredProfiles, CString strDefaultProfileAccountID /*= ""*/)
{
	try {

		ASSERT(m_PatientID != -1);

		NxThread(boost::bind(&LoadCCPaymentProfilesThread, GetSafeHwnd(), m_PatientID, bIncludeExpiredProfiles, strDefaultProfileAccountID))->RunDetached();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-09-30 09:40) - PLID 67165 - added ability to load payment profiles from the API, asynchronously
LRESULT CPaymentDlg::OnCCPaymentProfilesThreadComplete(WPARAM wParam, LPARAM lParam)
{
	try {

		m_CCCardOnFileCombo->Clear();

		Nx::SafeArray<IUnknown*> *ptrsaProfiles = (Nx::SafeArray<IUnknown*>*)wParam;
		CString strDefaultProfileAccountID = (LPCTSTR)lParam;
		
		if (ptrsaProfiles != NULL) {
			Nx::SafeArray<IUnknown*> saProfiles(*ptrsaProfiles);
			for each (NexTech_Accessor::_ICCPPaymentProfilePtr pProfile in saProfiles)
			{
				NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_CCCardOnFileCombo->GetNewRow();
				pNewRow->PutValue(ppcCardConnectProfileAccountID, pProfile->ProfileAccountID);
				// (z.manning 2015-08-26 08:24) - PLID 67231 - Add the token
				pNewRow->PutValue(ppcCreditCardToken, pProfile->CreditCardToken);
				pNewRow->PutValue(ppcCardType, pProfile->AccountType);
				pNewRow->PutValue(ppcLastFour, pProfile->Last4Digits);
				pNewRow->PutValue(ppcDefault, pProfile->IsDefault ? g_cvarTrue : g_cvarFalse);

				if (pProfile->ExpirationDate->IsNull()) {
					pNewRow->PutValue(ppcExpDate, g_cvarNull);
					pNewRow->PutValue(ppcExpDateText, g_cvarNull);
				}
				else {
					COleDateTime dtExpiration = pProfile->ExpirationDate->GetValue();
					pNewRow->PutValue(ppcExpDate, COleVariant(dtExpiration));
					pNewRow->PutValue(ppcExpDateText, _bstr_t(ICCP::GetExpirationDateText(dtExpiration)));
				}

				//fill the combo text with Card Name - Last 4, MM/YY of the exp date, like "Visa - 1234, 06/18"
				CString strComboText, strLast4Exp;
				CString strCardType = VarString(pNewRow->GetValue(ppcCardType), "");
				{
					CString strLast4 = VarString(pNewRow->GetValue(ppcLastFour), "");
					CString strExpDate = VarString(pNewRow->GetValue(ppcExpDateText), "");
					strLast4Exp = strLast4;
					if (!strLast4.IsEmpty() && !strExpDate.IsEmpty()) {
						strLast4Exp += ", ";
					}
					strLast4Exp += strExpDate;
				}

				strComboText = strCardType;
				if (!strLast4Exp.IsEmpty()) {
					strComboText += " - ";
					strComboText += strLast4Exp;
				}
				pNewRow->PutValue(ppcComboText, _bstr_t(strComboText));

				//the API returns the results sorted by exp. date descending, but
				//with the default profile on top, so we need to add at the end,
				//not add sorted
				m_CCCardOnFileCombo->AddRowAtEnd(pNewRow, NULL);
			}

			//don't forget to delete our pointer
			delete ptrsaProfiles;
			ptrsaProfiles = NULL;
		}

		//now create the "manual entry" row, make it be the first row & the default
		{
			NXDATALIST2Lib::IRowSettingsPtr pManualEntryRow = NULL;

			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_CCCardOnFileCombo->GetNewRow();
			pNewRow->PutValue(ppcCardConnectProfileAccountID, g_cvarNull);
			pNewRow->PutValue(ppcCreditCardToken, g_cvarNull);
			pNewRow->PutValue(ppcCardType, _bstr_t(" < Manual Entry On Device >"));
			pNewRow->PutValue(ppcLastFour, g_cvarNull);
			pNewRow->PutValue(ppcDefault, g_cvarNull);
			pNewRow->PutValue(ppcExpDate, g_cvarNull);
			pNewRow->PutValue(ppcExpDateText, g_cvarNull);

			pNewRow->PutValue(ppcComboText, _bstr_t("< Manual Entry On Device >"));

			//if there are cards on file, change the text blue to get their attention
			if (m_CCCardOnFileCombo->GetRowCount() > 0) {
				pNewRow->PutForeColor(RGB(0, 0, 255));
			}
			
			pManualEntryRow = m_CCCardOnFileCombo->AddRowBefore(pNewRow, m_CCCardOnFileCombo->GetFirstRow());

			//always select the "< Manual Entry On Device >" row
			m_CCCardOnFileCombo->PutCurSel(pManualEntryRow);
			OnSelChosenCCCardOnFileCombo(pManualEntryRow);

			//if manual entry is selected and there are cards on file,
			//change the combo box text blue to get their attention
			if (m_CCCardOnFileCombo->GetRowCount() > 1) {
				m_CCCardOnFileCombo->PutComboForeColor(RGB(0, 0, 255));
			}
			else {
				m_CCCardOnFileCombo->PutComboForeColor(RGB(0, 0, 0));
			}
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2015-09-30 09:44) - PLID 67164 - only show the CC profile checkbox if <Manual Entry> is selected
void CPaymentDlg::OnSelChosenCCCardOnFileCombo(LPDISPATCH lpRow)
{
	try {

		//only show the CC profile checkbox if <Manual Entry> is selected, and
		//this is an ICCP credit card transaction using 'Card Not Present', and
		//this is a new payment (or somehow managed to not have a transaction)
		if (IsICCPEnabled() && m_radioCharge.GetCheck() && m_radioCCCardNotPresent.GetCheck()
			&& (!m_bHasICCPTransaction || m_varPaymentID.vt != VT_I4 || VarLong(m_varPaymentID, -1) == -1)) {

			//this is a new ICCP CNP credit card payment
			bool bManualEntry = false;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_CCCardOnFileCombo->GetCurSel();
			if (pRow != NULL && VarString(pRow->GetValue(ppcCardConnectProfileAccountID), "").IsEmpty()) {
				bManualEntry = true;
			}
			m_checkAddCCToProfile.ShowWindow(bManualEntry ? SW_SHOW : SW_HIDE);

			//if manual entry is selected and there are cards on file,
			//change the combo box text blue to get their attention
			if (bManualEntry && m_CCCardOnFileCombo->GetRowCount() > 1) {
				m_CCCardOnFileCombo->PutComboForeColor(RGB(0, 0, 255));
			}
			else {
				m_CCCardOnFileCombo->PutComboForeColor(RGB(0, 0, 0));
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-08-26 08:14) - PLID 67231
void CPaymentDlg::GetNonExpiredCreditCardTokens(OUT std::set<CString> &setTokens)
{
	setTokens.clear();

	NXDATALIST2Lib::IRowSettingsPtr pRow;
	for (pRow = m_CCCardOnFileCombo->GetFirstRow(); pRow != NULL; pRow = pRow = pRow->GetNextRow())
	{
		COleDateTime dtExpDate = VarDateTime(pRow->GetValue(ppcExpDate), g_cdtNull);
		if (dtExpDate.GetStatus() == COleDateTime::valid && AsDateNoTime(COleDateTime::GetCurrentTime()) <= AsDateNoTime(dtExpDate))
		{
			CString strToken = VarString(pRow->GetValue(ppcCreditCardToken), "");
			if (!strToken.IsEmpty()) {
				setTokens.insert(strToken);
			}
		}
	}
}

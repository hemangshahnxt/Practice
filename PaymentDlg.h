#pragma once
// PaymentDlg.h : header file
//
//Valid Windows-defined command IDs go up to 11 at the moment.  If I was a computer, I might decide to start my 
//IDs at 256, but I'm a human being, so I'm arbitrarily selecting 100

#include "NxAPI.h"
#include "GlobalFinancialUtils.h"


#define RETURN_PREVIEW	100
#define RETURN_PRINT	101

/////////////////////////////////////////////////////////////////////////////
// CPaymentDlg dialog

// (a.walling 2007-09-21 09:38) - PLID 24768 - See implementation beneath CPaymentDlg
struct CPaymentSaveInfo;

enum SwipeType;
enum EBatchPaymentPayType;

// (a.walling 2007-10-08 15:17) - PLID 9801 - Information passed in the NXM_NEW_PAYMENT message
enum EOpenDrawerReason {
	odrNone			= 0x0000,
	odrPayment		= 0x0002,
	odrRefund		= 0x0004,
	odrNewPayment	= 0x0008,
	odrNewRefund	= 0x0010,
	odrTips			= 0x0020,
	odrDeletedTips	= 0x0040,
	odrOpenDrawer	= 0x0080,
};

// (z.manning 2010-02-22 16:08) - PLID 37479
struct NewPaymentMessageInfo
{
	long nID1;
	long nID2;
	WORD nType;
	WORD odr;
};

// (j.jones 2011-11-10 13:30) - PLID 46348 - added structure for the bill
// to tell the co-payment how much to apply to each charge, or range of charges

class CoPayApplyInformation
{

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

public:
	COleCurrency cyCoPayAmount;
	long nPayGroupID;

	CArray<long, long> aryChargeIDs;

	CoPayApplyInformation() {
		cyCoPayAmount = COleCurrency(0,0);
		nPayGroupID = -1;
	}
};

class InsuranceCoPayApplyList
{
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

public:

	long nInsuredPartyID;
	CArray<CoPayApplyInformation*, CoPayApplyInformation*> aryCoPayApplyInformation;
	

	InsuranceCoPayApplyList() {
		nInsuredPartyID = -1;
	}

	~InsuranceCoPayApplyList() {
		for(int i=aryCoPayApplyInformation.GetSize()-1; i>=0; i--) {
			delete aryCoPayApplyInformation.GetAt(i);
		}
		aryCoPayApplyInformation.RemoveAll();
	}
};

// (r.gonet 2015-04-29 11:09) - PLID 65326 - Forward Declaration.
enum class EPayMethod;

// (j.jones 2015-09-30 10:37) - PLID 67172 - added a struct to track information about the payment
// that we are about to apply to, to know whether credit card processing can refund it
class PaymentToApplyTo {
public:
	long nPaymentID;				//the ID of the payment we are about to refund
	bool bHasAnyCCTransaction;		//does the payment have any credit card processing transaction?
	bool bHasPreICCPTransaction;	//is the credit card processing transaction pre-ICCP?
	bool bHasICCPTransaction;		//is the credit card processing transaction from ICCP?
	CString strPreICCPTransactionID;	//if not ICCP, this is either QBMS_CreditTransactionsT.TransID or Chase_CreditTransactionsT.TxRefNum
	CString strICCPAuthRetRef;		//if ICCP, this is the CardConnect_CreditTransactionT.AuthRetRef from the processed payment
	long nICCPMerchantID;			//if ICCP, this is the CardConnect_CreditTransactionT.AccountID from the processed payment

	PaymentToApplyTo() {
		nPaymentID = -1;
		bHasAnyCCTransaction = false;
		bHasPreICCPTransaction = false;
		bHasICCPTransaction = false;
		strPreICCPTransactionID = "";
		strICCPAuthRetRef = "";
		nICCPMerchantID = -1;
	}
};

class CPaymentDlg : public CNxDialog
{
private:
	// (c.haag 2010-10-07 13:49) - PLID 35723 - This is used in OnShowWindow so that any
	// called function can know the dialog is opening
	class CInitializingFlag
	{
	private:
		CPaymentDlg* m_pDlg;
	public:
		CInitializingFlag(CPaymentDlg* pDlg)
		{
			m_pDlg = pDlg;
			m_pDlg->m_bInitializing = TRUE;
		}
		~CInitializingFlag()
		{
			m_pDlg->m_bInitializing = FALSE;
		}
	};

// Construction
public:

	~CPaymentDlg();

	NXDATALISTLib::_DNxDataListPtr	m_ProviderCombo;
	NXDATALISTLib::_DNxDataListPtr	m_DescriptionCombo;
	NXDATALISTLib::_DNxDataListPtr	m_InsuranceCombo;
	NXDATALISTLib::_DNxDataListPtr	m_CategoryCombo;
	NXDATALISTLib::_DNxDataListPtr	m_LocationCombo;
	NXDATALISTLib::_DNxDataListPtr	m_pTipList;
	NXDATALISTLib::_DNxDataListPtr	m_pDrawerList;
	NXDATALISTLib::_DNxDataListPtr	m_CardNameCombo;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pGroupCodeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pReasonList;
	NXDATALIST2Lib::_DNxDataListPtr m_pReportList;

	// (j.jones 2015-09-30 09:15) - PLID 67158 - added CC merchant account
	NXDATALIST2Lib::_DNxDataListPtr m_CCMerchantAccountCombo;

	// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
	NXDATALIST2Lib::_DNxDataListPtr m_CCCardOnFileCombo;


	long m_PatientID;
	CString m_strPatientName;

	//default false, changes to true once they manually change the payment amount
	BOOL m_bAmountChanged;

	// TRUE if this payment does not already exist.
	BOOL m_boIsNewPayment;

	// Default to payment, adjustment, or refund? (For
	// new payments) //0 - Payment, 1 - Adjustment, 2 - Refund
	int m_iDefaultPaymentType;

	// (j.jones 2007-02-26 10:52) - PLID 24927 - used to disallow saving the payment as any other type
	// than what we created it as
	BOOL m_bForceDefaultPaymentType;
	//tells us if we're using m_bForceDefaultPaymentType for the purposes of Returned Products
	// (j.jones 2007-04-19 14:33) - PLID 25711 - converted m_bIsReturnedProductRecord into
	// m_bIsReturnedProductAdj and m_bIsReturnedProductRefund
	BOOL m_bIsReturnedProductAdj;
	BOOL m_bIsReturnedProductRefund;

	//True if we want a new payment to be a prepayment
	BOOL m_bIsPrePayment;

	// Default insurance company (For new payments)
	int m_iDefaultInsuranceCo;

	// BillID used to fill default description for new
	// payments
	COleVariant m_varBillID;

	long m_GuarantorID1,m_GuarantorID2;
	
	COleCurrency m_cyFinalAmount;

	// Maximum amount this payment or adjustment may be
	// for. This is disregarded if this is a refund.
	COleCurrency m_cyMaxAmount;

	COleVariant m_varChargeID;
	COleVariant m_varPaymentID;

	long m_DefLocationID;

	long m_QuoteID; //the ID of the linked quote, if there is one

	// (j.jones 2009-08-25 11:55) - PLID 31549 - send the ProcInfoID to the Payment, which removes
	// the PIC's responsibility to update ProcInfoPaymentsT
	long m_nProcInfoID;

	// True if you want to do an auto apply when the
	// payment is created
	BOOL m_ApplyOnOK;

	// True if you want to be prompted to shift remaining balance to patient, if not fully paid
	BOOL m_PromptToShift;

	// True if this is a co-payment and charge resps may need shifted
	BOOL m_bIsCoPay;

	// (b.eyers 2015-10-16) - PLID 67357 - moved
	long m_nBatchPaymentID;

	// (j.jones 2010-08-03 15:42) - PLID 39938 - we now take in the PayGroupID that this copay is for,
	// and the insured party ID we used to calculate the copay
	// (j.jones 2010-08-04 17:00) - PLID 38613 - we also take in what the bill's co-insurance was
	// (j.jones 2011-11-10 13:35) - PLID 46348 - overhauled these parameters so we can properly apply
	// one payment to multiple co-payment amounts per charge

	// (j.jones 2011-11-11 09:13) - PLID 46348 - in the (rare) case where this copay covers the needs of multiple
	// insured parties, this array holds objects with the InsuredPartyIDs that required a copay, and a breakdown
	// of how much of the copay should be applied to each charge
	// (this is a pointer to an array of pointers,handled by our caller, NOT deleted within this dialog)
	CArray<InsuranceCoPayApplyList*, InsuranceCoPayApplyList*> *m_parypInsuranceCoPayApplyList;

	// (j.jones 2011-11-09 16:17) - PLID 46348 - renamed to m_cyTotalCoInsuranceAmt to clearly reflect that this
	// represents the total co-insurance for the entire bill, it could be for multiple insurance resps.
	COleCurrency m_cyTotalCoInsuranceAmt;

	//the default amount for a copay
	COleCurrency m_cyCopayAmount;

	// True if user chooses to pull up previous credit card info.
	BOOL GetChargeInfo;

	//Used to remember the credit card ID while you edit the list.
	long m_nCreditCardID;

	//DRT 3/29/2004 - PLID 11655 - Need to remember the inactive prov id for tips
	long m_nTipInactiveProv;

	//DRT 3/30/2004 - Add the ID of the gift certificate this applies to.  This value is -1
	//	if no gift certificate is in use.  This is GiftCertificatesT.ID.
	long m_nGiftID;

	// (j.jones 2005-07-01 10:04) - force an applied adjustment to follow the rules of the maximum amount
	BOOL m_bAdjustmentFollowMaxAmount;

	CPaymentDlg(CWnd* pParent);   // standard constructor

	// (j.gruber 2007-07-03 12:22) - PLID 15416
	//This flag is set to true if the current user has a license for CC Processing and they have permission
	//	to write CC transactions.
	// (j.jones 2015-10-07 15:42) - PLID 67157 - this is only true for non-ICCP processing (Chase, Intuit)
	BOOL m_bProcessCreditCards_NonICCP;
	BOOL CheckCCFields();

	// (d.thompson 2009-06-22) - PLID 34687 - QBMS replaces IGS processing
	//BOOL IGS_CompleteCreditProcessing(long nLocationID, CPaymentSaveInfo* pInfo = NULL); // (a.walling 2007-09-21 15:55) - PLID 27468 - Added the payment info struct
	BOOL QBMS_CompleteCreditProcessing(long nLocationID, CPaymentSaveInfo *pInfo = NULL);
	BOOL QBMS_ProcessTransaction(CPaymentSaveInfo* pInfo = NULL);

	// (a.walling 2007-08-03 15:59) - PLID 26922 - Need to save the current pay method in case we cancel changing them
	NxButton* m_pCurPayMethod;
	void UpdateCurPayMethod(); // this updates the m_pCurPayMethod variable

	// (a.walling 2007-09-21 17:45) - PLID 27468 - Shared function for opening the cash drawer based on the payment info struct
	// (a.walling 2007-10-08 15:12) - PLID 9801 - Added option to not open cash drawer and just calculate if we should or not,
	// and return an ORed value of the reason we are opening it.
	WORD OpenCashDrawerIfNeeded(CPaymentSaveInfo* pInfo, BOOL bOpenDrawer);

	// (a.walling 2007-10-08 15:21) - PLID 9801 - Previous reasons for opening the drawer which were pended due to adding new
	//EOpenDrawerReason
	WORD m_odrPreviousReasons;

	// (d.thompson 2011-01-05) - PLID 42005 - Gathers the currently selected location ID from the interface.
	long GetCurrentlySelectedLocationID();

	// (c.haag 2015-08-18) - PLID 67203 - Gets the currently selected merchant account ID, or -1 if none is selected
	long GetCurrentlySelectedMerchantAccountID();

	// (b.spivey, March 06, 2013) - PLID 51186 - track string set by code. 
	CString m_strLastSetDescription; 

	// (r.gonet 2015-07-06 18:02) - PLID 65327 - If true, then the dialog has created a bill. If false, then the dialog has
	// not created any bill. Set by this class.
	bool m_bBillCreated = false;

private:
	// (j.gruber 2007-07-30 14:37) - PLID 26704 - variable for storing the payment this refund will be applied to
	// (c.haag 2010-10-11 10:34) - PLID 35723 - Privatized
	// (j.jones 2015-09-30 10:37) - PLID 67172 - converted into a struct so we know more information
	// about the payment we're going to apply to, mainly for credit card processing info.
	PaymentToApplyTo m_PaymentToApplyTo;

	// (z.manning 2015-08-26 08:14) - PLID 67231
	void GetNonExpiredCreditCardTokens(OUT std::set<CString> &setTokens);

	//TES 4/20/2015 - PLID 65655 - When applying to an existing payment, track its payment method
	long m_nPayToApplyToMethod;
	// (r.gonet 2015-04-30) - PLID 65326 - The LineItemT.GiftID value of the LineItem this is applying to.
	long m_nPayToApplyToGiftID = -1;
	// (r.gonet 2015-04-29 11:09) - PLID 65326 - Bound variable to the Refund to Existing Gift Certificate radio button.
	// Used when the checked state of the radio button may not be valid yet.
	BOOL m_bRefundToExistingGC = TRUE;
public:
	// (c.haag 2010-10-11 10:34) - PLID 35723 - This function is called when we want to update m_nPayToApplyToID.
	// (j.jones 2015-09-30 10:37) - PLID 67172 - renamed for clarity
	// (j.jones 2015-09-30 10:34) - PLID 67171 - added bWarnAboutCreditCardMismatch,
	// set it to true if you're making a refund for a payment and need to see if it's a refundable credit card
	void SetPaymentToApplyTo(long nPaymentIDToApplyTo, bool bWarnAboutCreditCardMismatch = true);

	// (j.gruber 2007-08-03 12:17) - PLID 26916 - variable for saying whether to show CC info,
	//FALSE if the payment being applied to this is not a charge
	BOOL m_bSetCreditCardInfo;

	//(e.lally 2007-10-30) PLID 27892 - Depreciated the m_strNumber for public functions
	void SetCreditCardNumber(CString strNewCardNumber);
	CString GetCreditCardNumber();

	// (j.gruber 2007-10-31 11:31) - PLID 15416 - make sure we don't allow swiping of processed payments
	BOOL m_bAllowSwipe;

	// (c.haag 2014-08-15) - PLID 63379 - Use our own DoModal that requires the caller to pass in their function name and line number
	INT_PTR DoModal(LPCTSTR szFuncName, int nLine);
private:
	// (c.haag 2014-08-15) - PLID 63379 - Make it so nobody outside this class can call DoModal() directly
	virtual INT_PTR DoModal();

	CString m_strCallingFuncName; // (c.haag 2014-08-15) - PLID 63379 - The calling function name
	int m_nCallingLineNumber; // (c.haag 2014-08-15) - PLID 63379 - The calling line number

public:
	// (c.haag 2014-08-15) - PLID 63379 - Everything below this line were public before my changes, so we need to keep it that way

// Dialog Data
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CPaymentDlg)
	enum { IDD = IDD_PAYMENT_DLG };
	NxButton	m_btnIncludeOtherGC;
	NxButton	m_btnTipsInDrawer;
	NxButton	m_btnPrePayment;
	CNxLabel m_nxlInfoLabel1;
	CNxLabel m_nxlInfoLabel2;
	CNxLabel m_nxlInfoLabel3;
	NxButton m_radioPayment;
	NxButton m_radioAdjustment;
	NxButton m_radioRefund;
	NxButton m_radioCash;
	NxButton m_radioCheck;
	NxButton m_radioCharge;
	NxButton m_radioGift;
	NxButton m_radioProcessCC;
	// (r.gonet 2015-04-20) - PLID 65326 - Radio buttons for setting if a refund goes to an existing gift certificate or a new one. Visible in Refunds->Gift Certificates
	NxButton m_radioRefundToExistingGiftCertificate;
	NxButton m_radioRefundToNewGiftCertificate;
	CString	m_strAmount;
	CString	m_strBankNumOrCCAuthNum;
	CString	m_strExpDate;
	CString	m_strNameOnCard;
	CDateTimePicker	m_date;
	CNxEdit	m_nxeditEditTotal;
	CNxEdit	m_nxeditEditDescription;
	CNxEdit	m_nxeditChangeGiven;
	CNxEdit	m_nxeditBankNumOrCCAuthNum;	
	CNxEdit	m_nxeditCashReceived;
	CNxStatic	m_nxsTotalTipAmt; // (a.walling 2010-01-12 15:13) - PLID 28961 - Now a static label
	CNxStatic	m_nxstaticReportLabel;
	CNxStatic	m_nxstaticLabel1;
	CNxStatic	m_nxstaticCurrencySymbol;
	CNxStatic	m_nxstaticLabel2;
	CNxStatic	m_nxstaticPayCatLabel;
	CNxStatic	m_nxstaticLabel3;
	CNxStatic	m_nxstaticLabel6;
	CNxStatic	m_nxstaticLocationLabel;
	CNxStatic	m_nxstaticLabel5;
	CNxStatic	m_nxstaticCashDrawerLabel;
	CNxStatic	m_nxstaticLabel11;
	CNxStatic	m_nxstaticAdjGroupcodeLabel;
	CNxStatic	m_nxstaticAdjReasonLabel;
	CNxStatic	m_nxstaticCashReceivedLabel;
	CNxStatic	m_nxstaticCurrencySymbolReceived;
	CNxStatic	m_nxstaticChangeGivenLabel;
	CNxStatic	m_nxstaticCurrencySymbolGiven;
	// (j.jones 2015-09-30 09:31) - PLID 67167 - renamed the generic fields to be more meaningful
	CNxStatic	m_nxstaticLabelCheckNumOrCCExpDate;	
	CNxStatic	m_nxstaticLabelBankNameOrCCCardNum;
	CNxStatic	m_nxstaticLabelAcctNumOrCCNameOnCard;
	CNxStatic	m_nxstaticLabelCCCardType;
	CNxStatic	m_nxstaticLabelBankNumOrCCAuthNum;	
	CNxEdit		m_nxeditCheckNumOrCCExpDate;
	CNxEdit		m_nxeditBankNameOrCCCardNum;
	CNxEdit		m_nxeditAcctNumOrCCNameOnCard;
	CNxStatic	m_nxstaticBottomRightNormal;
	CNxStatic	m_nxstaticBottomRightExtended;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnEditPayCat;
	CNxIconButton m_btnEditPayDesc;
	CNxIconButton m_btnEditCashDrawers;
	CNxIconButton m_btnEditCardName;
	CNxIconButton m_btnAddTip;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnSaveAndAddNew;
	CNxIconButton m_btnPrint;
	CNxIconButton m_btnPreview;
	CNxIconButton m_btnCalcPercent;
	CNxIconButton m_btnKeyboardCardSwipe; // (b.spivey, October 04, 2011) - PLID 40567 - card swipe dialog button. 
	// (j.jones 2012-07-26 17:48) - PLID 26877 - added ability to filter adjustment reasons
	CNxIconButton m_btnFilterReasonCodes;
	// (j.jones 2015-03-26 09:05) - PLID 65283 - added gift certificate fields
	CNxStatic	m_nxstaticGiftCNumber;
	CNxStatic	m_nxstaticGCValue;
	CNxStatic	m_nxstaticGCBalance;
	CNxStatic	m_nxstaticGCPurchDate;
	CNxStatic	m_nxstaticGCExpDate;
	CNxEdit		m_nxeditGiftCNumber;
	CNxEdit		m_nxeditGCValue;
	CNxEdit		m_nxeditGCBalance;
	CNxEdit		m_nxeditGCPurchDate;
	CNxEdit		m_nxeditGCExpDate;
	// (j.jones 2015-03-26 09:37) - PLID 65285 - added ability to clear the gift certificate
	CNxIconButton	m_btnClearGC;
	// (j.jones 2015-05-11 15:12) - PLID 65281 - added GC search label
	CNxStatic	m_nxstaticGCSearchLabel;
	// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
	CNxStatic	m_nxstaticLabelCCTransactionType;
	NxButton	m_radioCCSwipe;
	NxButton	m_radioCCCardNotPresent;
	NxButton	m_radioCCDoNotProcess;
	// (j.jones 2015-09-30 09:15) - PLID 67158 - added merchant acct. and zip code
	CNxStatic	m_nxstaticLabelCCMerchantAcct;
	CNxStatic	m_nxstaticLabelCCZipcode;
	CNxEdit		m_nxeditCCZipcode;
	// (j.jones 2015-09-30 09:19) - PLID 67161 - added CC profile checkbox
	NxButton	m_checkAddCCToProfile;
	// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
	NxButton	m_radioCCRefundToOriginalCard;	
	// (j.jones 2015-09-30 09:44) - PLID 67164 - added "card on file" combo
	CNxStatic	m_nxstaticLabelCCCardOnFile;
	// (j.jones 2015-09-30 09:31) - PLID 67167 - added dedicated CC Last 4 field
	CNxStatic	m_nxstaticLabelCCLast4;
	CNxEdit		m_nxeditCCLast4;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaymentDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnMSRDataEvent(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.jones 2015-03-23 13:13) - PLID 65281 - converted the gift certificate dropdown
	// into a searchable list
	NXDATALIST2Lib::_DNxDataListPtr m_GiftSearch;
	long m_nSelectedGiftID;
	// (r.gonet 2015-04-29 11:09) - PLID 65326 - The ID of the gift certificate refunded to originally. -1 on new refunds.
	// If a payment, then the original GiftID of the gift certificate being used as the payment source.
	long m_nOriginalGiftID = -1;
	EPayMethod m_eOriginalPayMethod;
	// (r.gonet 2015-05-05 14:37) - PLID 65657 - If this is a refund, and if this refund is refunding a payment paid by a gift certificate,
	// then this is that gift certificate's ID.
	long m_nRefundedFromGiftID = -1;

	// (j.jones 2015-03-26 11:08) - PLID 65281 - called after a GC is newly selected,
	// either manually or by a barcode scan
	void NewGiftCertificateSelected(long nID);

	// (j.jones 2015-03-25 17:05) - PLID 65283 - updates the Gift Certificate fields
	// on the dialog with the provided fields
	void UpdateGiftCertificateControls_FromFields(CString strGiftID, COleCurrency cyTotalValue, COleCurrency cyBalance, COleDateTime dtPurchasedDate, COleDateTime dtExpDate);	
	// (j.jones 2015-03-25 17:05) - PLID 65283 - this function updates the fields on
	// the screen from the search result's row
	void UpdateGiftCertificateControls_FromRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (j.jones 2015-03-25 17:05) - PLID 65283 - this function updates the fields on
	// the screen from a GiftCertificatesT.ID. Fails if the ID is not found.
	void UpdateGiftCertificateControls_FromID(long nID);

	void EnsureLabelArea();
	void EnsureReceivedArea();

	//TES 3/19/2015 - PLID 65069 - Update whether "Gift Cert" shows for tips
	void EnsureTipMethodCombo();

	// (a.walling 2006-11-16 09:20) - PLID 23550 - Interface helper functions for group and reason code controls
	void ShowGroupCodesAndReasons(OPTIONAL IN bool bShow = true); // show or hide the controls
	// (j.jones 2010-09-23 11:39) - PLID 27917 - obsolete function, they are no longer hardcoded
	//void InitGroupCodesAndReasons(); // populate the controls with values, since they are hardcoded

	COleCurrency m_cyLastAmount;

	CString m_strSwipedCardName;
	CString m_strCreditCardNumber;

	// (c.haag 2010-10-07 13:49) - PLID 35723 - Called during or after the state of a radio button has changed 
	void OnRadioCashUpdated();
	void OnRadioCheckUpdated();
	void OnRadioChargeUpdated();
	void OnRadioGiftCertUpdated();

	// (b.spivey, October 01, 2012) - PLID 50949 - Update window text based on type of 'payment'
	void UpdateWindowText();


	// (j.gruber 2008-05-28 13:31) - PLID 27090 - added the on sel changing method
	// Generated message map functions
	//{{AFX_MSG(CPaymentDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnChangeRadioPayment();
	afx_msg void OnClickRadioCash();
	afx_msg void OnClickRadioCharge();
	afx_msg void OnClickRadioCheck();
	afx_msg void OnClickRadioPayment();
	afx_msg void OnClickRadioAdjustment();
	afx_msg void OnClickRadioRefund();
	afx_msg void OnUpdateEditCheckNumOrCCExpDate();
	afx_msg void OnClickBtnPrint();
	afx_msg void OnClickBtnPreview();
	afx_msg void OnDestroy();
	afx_msg void OnEditPayDesc();
	afx_msg void OnEditPayCat();
	afx_msg void OnEditCardName();
	afx_msg void OnRequeryFinishedComboInsurance(short nFlags);
	afx_msg void OnProcessCC();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKillfocusEditCheckNumOrCCExpDate();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnCalcPercent();
	afx_msg void OnExportToQuickBooks();
	afx_msg void OnChangeEditTotal();
	afx_msg void OnTrySetSelFinishedComboLocation(long nRowEnum, long nFlags);
	afx_msg void OnSetfocusEditAcctNumOrCCNameOnCard();
	afx_msg void OnSelChosenComboCardName(long nRow);
	afx_msg void OnSelChosenComboDescription(long nRow);
	afx_msg void OnSelChosenComboLocation(long nRow);
	afx_msg void OnSelChosenComboCategory(long nRow);
	afx_msg void OnKillfocusCashReceived();
	afx_msg void OnKillfocusEditTotal();
	afx_msg void OnSelChosenComboInsurance(long nRow);
	afx_msg void OnAddTipBtn();
	afx_msg void OnShowTipsBtn();
	afx_msg void OnRButtonDownTipList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedTipList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedTipList(short nFlags);
	afx_msg void OnEditingFinishingTipList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnBtnSaveAndAddNew();
	afx_msg void OnClickRadioGiftCert();
	afx_msg void OnGcIncludeOthers();
	afx_msg void OnEditCashDrawers();
	afx_msg void OnTipsInDrawer();
	afx_msg void OnCheckPrepayment();
	afx_msg void OnSelChangingGroupCode(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingReason(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTrySetSelFinishedComboCardName(long nRowEnum, long nFlags);
	afx_msg LRESULT OnPinPadTrackRead(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPinPadInteracDebitDone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKillfocusEditBankNameOrCCCardNum();
	afx_msg void OnSelChangingComboLocation(long FAR* nNewSel);
	// (j.jones 2010-09-23 12:13) - PLID 27917 - supported trysetselfinished
	void OnTrySetSelFinishedAdjGroupcode(long nRowEnum, long nFlags);
	void OnTrySetSelFinishedAdjReason(long nRowEnum, long nFlags);
	// (b.spivey, September 29, 2011) - PLID 40567 - Support a button for keyboard swipe.
	afx_msg void OnBnClickedKeyboardCardSwipe(); 
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// (a.walling 2007-09-20 15:52) - PLID 27468
	BOOL SaveChanges(IN OUT CPaymentSaveInfo* pSaveInfo = NULL);
	BOOL IsPrePayment();
	BOOL IsRefund();
	BOOL IsAdjustment();
	BOOL IsPayment();
	void DisablePaymentTypes();
	void EnablePaymentTypes();
	void PrintReceipt(bool bPreview);
	void GetNonNegativeAmountExtent(int &nStart, int &nFinish);
	//(e.lally 2007-07-17) PLID  - Changed this function to use the ID, in case of duplicates
	void SetCreditCardInfoForType(const IN long nCrediCardID);
	void OnDeleteTip();
	void UpdateTotalTipAmt();

	// (z.manning 2016-02-15 11:13) - PLID 68258
	BOOL ShouldPromptForICCPAuthorization();
	BOOL ICCPAuthorize();

	// (z.manning 2016-02-15 11:32) - PLID 68258
	LineItem::CCProcessType GetCCProcessType();
	LineItem::CCProcessType GetCCProcessType(OUT CString &strCCProcessTypeNewAuditValue);
	
	// (j.jones 2008-07-10 12:43) - PLID 28756 - this function will check
	// which insurance company (if any) is selected, and override the 
	// payment description and payment category 
	void TrySetDefaultInsuranceDescriptions();

	BOOL m_bSaved;
	CBrush m_bg;
	bool m_bTipExtended;
	bool m_bHasNexSpa;
	bool m_bViewTips;
	CDWordArray m_aryDeleted;

	BOOL m_bLastCheckInfoLoaded;
	CString m_strLastBankNo;
	CString m_strLastCheckAcctNo;
	CString m_strLastRoutingNo;

	BOOL m_bLastCreditCardInfoLoaded;
	long m_nLastCCTypeID;

	// (z.manning 2016-02-15 14:58) - PLID 68258 - We changed ICCP payments to save before we process them.
	// This member can be used to determine if an ICCP payment has been saved but not yet processed.
	BOOL m_bNewICCPPaymentSavedButNotYetProcessed;

	// (j.jones 2007-03-27 17:17) - PLID 24080 - store batch payment information
	//long m_nBatchPaymentID; // (b.eyers 2015-10-16) - PLID 67357 - moved
	CString m_strBatchPaymentInfo;
	void OnBatchPaymentInfo();
	CRect m_rcInfoLabel1;
	// (j.jones 2014-07-02 09:18) - PLID 62562 - track the batch payment type
	EBatchPaymentPayType m_eBatchPayType;

	// (j.jones 2007-04-19 14:12) - PLID 25711 - added PostClick functions
	// so the OnClick functions are only called when actually changed
	void PostClickRadioPayment();
	void PostClickRadioAdjustment();
	void PostClickRadioRefund();
	void PostClickRadioCash();
	void PostClickRadioCheck();
	void PostClickRadioCharge();	
	void PostClickRadioGC();

	// (j.jones 2015-09-30 08:58) - PLID 67157 - added PostClick functions
	// for CC processing types
	void PostClickRadioCCSwipe();
	void PostClickRadioCCCardNotPresent();
	void PostClickRadioCCDoNotProcess();
	// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
	void PostClickRadioCCRefundToOriginalCard();

	// (j.jones 2015-09-30 09:40) - PLID 67165 - added ability to load payment profiles from the API, asynchronously
	// if strDefaultProfileAccountID is filled, that account is selected, otherwise the default account is, or <manual entry>
	void BeginLoadCCPaymentProfilesThread(bool bIncludeExpiredProfiles, CString strDefaultProfileAccountID = "");
	LRESULT OnCCPaymentProfilesThreadComplete(WPARAM wParam, LPARAM lParam);

	// (j.jones 2015-09-30 09:31) - PLID 67167 - flag to determine if we have moved
	// the CC card combo for ICCP
	bool m_bNeedToMoveICCPCCCombo;
	// (j.jones 2015-09-30 09:31) - PLID 67167 - moves the Card Type combo to be next to
	// the Last 4 field, for ICCP only
	void MoveICCPCCCombo();

	// (j.jones 2015-09-30 09:18) - PLID 67163 - flag to determine if we have moved the
	// Merchant Account combo for ICCP, when editing processed payments
	bool m_bNeedToMoveICCPMerchantAccountCombo_Processed;
	// (j.jones 2015-09-30 09:18) - PLID 67163 - moves the Merchant Account combo to be next to
	// the authorization # field, for ICCP only, for processed payments only
	void MoveICCPMerchantAccountCombo_Processed();

	// (j.jones 2015-09-30 09:39) - PLID 67166 - Loading a processed ICCP payment shows
	// the same controls for all process types: swipe, 'card not present', or refund to original card
	// This function will show/hide controls accordingly.
	void UpdateDisplayForProcessedICCPInfo();

	// (j.gruber 2007-07-11 14:51) - PLID 15416 - CC Processing
	//BOOL IGS_ProcessTransaction(CPaymentSaveInfo* pInfo = NULL); // (a.walling 2007-09-21 15:53) - PLID 27468 - Use the payment info struct
	// (d.thompson 2011-01-05) - PLID 42005 - nLocationID should now be filled before sending, will no longer return it.
	BOOL PrintCreditReceipt(long nLocationID);
	BOOL m_bSwiped;
	SwipeType m_swtType;
	CString m_strTrack1;
	CString m_strTrack2;
	CString m_strTrack3;
	long m_nDrawerID;
	// (j.jones 2015-09-30 10:12) - PLID 67168 - split HasTransaction into HasAnyCCTransaction, HasPreICCPTransaction, HasICCPTransaction
	bool m_bHasAnyTransaction;
	bool m_bHasPreICCPTransaction;
	bool m_bHasICCPTransaction;
	// (j.gruber 2007-07-17 15:42) - PLID 26710 - Pin pad
	BOOL m_bPinPadSwipeable;
	// (c.haag 2010-10-07 13:49) - PLID 35723 - This will be TRUE when the window is preparing itself
	// to show.
	BOOL m_bInitializing;

	//TES 3/25/2015 - PLID 65436 - We now store whether editing is disabled (for example, when it's a voided payment) as a member variable
	BOOL m_bCanEdit;
	//TES 3/25/2015 - PLID 65436 - Added, returns whether there's something (like, this being a voided payment) that means the whole dialog
	// is not editable
	BOOL CanEdit();
	//TES 3/25/2015 - PLID 65175 - Determines whether the current user should be able to edit the Tips section
	BOOL CanEditTips();

	void UpdateCashReceived(COleCurrency cyAmount); // (z.manning, 04/29/2008) - PLID 29836
	// (c.haag 2010-09-20 15:34) - PLID 35723 - Called when switching payment methods to alert the
	// user of potentially losing a credit card transaction. Returns FALSE if the user elected to cancel
	// the switch.
	BOOL WarnOfExistingCCTransaction();

	// (j.jones 2010-09-23 12:18) - PLID 27917 - used for inactive group & reason codes
	long m_nPendingGroupCodeID;
	long m_nPendingReasonCodeID;

	// (b.spivey, October 03, 2011) - PLID 40567 - variable for keyboard mode check
	// (b.spivey, November 18, 2011) - PLID 40567 - Corrected bool type. 
	bool m_bKeyboardCardSwiper; 

	// (j.jones 2012-07-26 17:48) - PLID 26877 - added ability to filter adjustment reasons
	afx_msg void OnBtnFilterReasonCodes();

	// (b.spivey, September 25, 2014) - PLID 63422 - Members to cut down on querying
	bool m_bIsLinkedLockboxPayment = false; 
	COleCurrency m_cyOriginalAmount = COleCurrency(0, 0); 

	// (j.jones 2015-03-23 16:21) - PLID 65281 - turned the gift certificate dropdown into a search
	void OnSelChosenSearchGcList(LPDISPATCH lpRow);

	// (d.lange 2016-05-12 16:02) - NX-100597 - Sets the tip datalist where clause
	void UpdateTipListWhereClause(long nPaymentID);

	// (j.jones 2015-09-30 08:58) - PLID 67157 - added CC processing radio buttons
	afx_msg void OnRadioSwipeCard();
	afx_msg void OnRadioCardNotPresent();
	afx_msg void OnRadioDoNotProcess();
	// (j.jones 2015-09-30 10:05) - PLID 67169 - added 'refund to original CC' radio
	afx_msg void OnRadioCcRefundToOriginalCard();
	// (j.jones 2015-09-30 09:44) - PLID 67164 - only show the CC profile checkbox if <Manual Entry> is selected
	void OnSelChosenCCCardOnFileCombo(LPDISPATCH lpRow);
	// (j.jones 2015-03-26 09:18) - PLID 65285 - added ability to clear the gift certificate
	void ClearGiftCertificate();
	// (j.jones 2015-03-26 09:34) - PLID 65285 - added ability to clear the current gift certificate
	afx_msg void OnBtnClearGC();
	// (r.gonet 2015-04-20) - PLID 65326 - Handler for when the user clicks the Refund To Existing Gift Certificate radio button.
	afx_msg void OnBnClickedRefundToExistingGcRadio();
	// (r.gonet 2015-04-20) - PLID 65326 - Handler for when the user clicks the Refund to New Gift Certificate radio button.
	afx_msg void OnBnClickedRefundToNewGcRadio();
	// (r.gonet 2015-04-29 11:09) - PLID 65326 - Ensures the enabled states of the gift certificate controls.
	void EnsureGiftCertificateControls();
	// (r.gonet 2015-04-20) - PLID 65327 - Returns the paymethod currently selected on the payment dialog.
	// Returns EPayMethod::Invalid if nothing is checked.
	EPayMethod GetPayMethod();
	// (r.gonet 2015-04-20) - PLID 65327 - Returns the descriptive name of the pay method.
	CString GetPayMethodName(EPayMethod ePayMethod, CString strDefault, bool bIncludeLineItemTypeName = true, bool bCapitalized = false);
};


// (a.walling 2007-09-21 09:36) - PLID 27468 - Structure to keep track of the various tips and their method of payment that have been saved
//TES 3/19/2015 - PLID 65069 - Tips can now be Gift Certificates
//TES 4/16/2015 - PLID 65171 - Tips can also now have the refund types of paymethods (7,8,9)
#define TIP_PAY_METHODS 10
// datalist values for tip pay method
#define TIP_PAY_CASH 1
#define TIP_PAY_CHECK 2
#define TIP_PAY_CHARGE 3
#define TIP_PAY_GIFT 4
#define TIP_PAY_INVALID_METHOD1 5
#define TIP_PAY_INVALID_METHOD2 6
#define TIP_PAY_CASH_REFUND	7
#define TIP_PAY_CHECK_REFUND 8
#define TIP_PAY_CHARGE_REFUND 9
#define TIP_PAY_GIFT_REFUND 10

// (a.walling 2007-09-21 16:25) - PLID 27468 - Added CArrays and an assignment operator
struct CPaymentSaveInfo
{
	CPaymentSaveInfo() {
		nTotalTips = 0;
		arNewTips.SetSize(TIP_PAY_METHODS);
		arChangedTips.SetSize(TIP_PAY_METHODS);
		
		for (int i = 0; i < TIP_PAY_METHODS; i++) {
			arNewTips[i] = 0;
			arChangedTips[i] = 0;
		}
	};

	CPaymentSaveInfo& operator=(const CPaymentSaveInfo &psi) {
		nTotalTips = psi.nTotalTips;

		arNewTips.Copy(psi.arNewTips);
		arChangedTips.Copy(psi.arChangedTips);
		arModifiedTipIDs.Copy(psi.arModifiedTipIDs);
		arExistingTipIDs.Copy(psi.arExistingTipIDs);
		// (a.walling 2007-09-28 10:55) - PLID 27468
		arDeletedTipIDs.Copy(psi.arDeletedTipIDs);

		return *this;
	};

	// (a.walling 2007-10-08 16:22) - PLID 9801 - Take a word of EOpenDrawerReason flags and return a string describing it for auditing
	static CString GetReasonFromWord(WORD odr) {
		CStringArray strReasons;

		if (odr & odrNewPayment) {
			strReasons.Add("New Payment");
		}
		if (odr & odrNewRefund) {
			strReasons.Add("New Refund");
		}
		if (odr & odrPayment) {
			strReasons.Add("Existing Payment");
		}
		if (odr & odrRefund) {
			strReasons.Add("Existing Refund");
		}
		if (odr & odrTips) {
			strReasons.Add("Tips");
		}
		if (odr & odrDeletedTips) {
			strReasons.Add("Deleted Tips");
		}

		CString str;
		for (int i = 0; i < strReasons.GetSize(); i++) {
			if (i > 0) {
				if (i == strReasons.GetSize() - 1) {
					str += " and ";
				} else {
					str += ", ";
				}
			}
			str += strReasons.GetAt(i);
		}

		return str;
	};

	void AddExistingTipID(long nID) {
		ASSERT(nID != -1);

		for (int i = 0; i < arExistingTipIDs.GetSize(); i++) {
			if (arExistingTipIDs[i] == nID) {
				return;
			}
		}

		arExistingTipIDs.Add(nID);
	}

	BOOL IsIDInExistingTipIDs(long nID) {
		if (nID == -1) return FALSE;

		for (int i = 0; i < arExistingTipIDs.GetSize(); i++) {
			if (arExistingTipIDs[i] == nID) {
				return TRUE;
			}
		}

		return FALSE;
	}

	void AddModifiedTipID(long nID) {
		ASSERT(nID != -1);

		for (int i = 0; i < arModifiedTipIDs.GetSize(); i++) {
			if (arModifiedTipIDs[i] == nID) {
				return;
			}
		}

		arModifiedTipIDs.Add(nID);
	}

	BOOL IsIDInModifiedTipIDs(long nID) {
		if (nID == -1) return FALSE;

		for (int i = 0; i < arModifiedTipIDs.GetSize(); i++) {
			if (arModifiedTipIDs[i] == nID) {
				return TRUE;
			}
		}

		return FALSE;
	}

	long nTotalTips;
	
	CArray<long, long> arNewTips;
	CArray<long, long> arChangedTips;

	CArray<long, long> arExistingTipIDs;
	CArray<long, long> arModifiedTipIDs;
	// (a.walling 2007-09-28 10:55) - PLID 27468 - Deleted tip info
	CArray<long, long> arDeletedTipIDs;

	// (a.walling 2007-09-28 10:56) - PLID 27468 - Deleted tip functions
	long GetDeletedExistingTips() {
		long n = 0;
		for (int i = 0; i < arDeletedTipIDs.GetSize(); i++) {
			if (arDeletedTipIDs[i] != -1) {
				n++;
			}
		}

		return n;
	}

	// All these functions only return the count of new or modified tips
	//TES 4/16/2015 - PLID 65171 - Include the refund paymethods
	long GetCashTips() {
		return arNewTips[TIP_PAY_CASH-1] + arChangedTips[TIP_PAY_CASH-1] + arNewTips[TIP_PAY_CASH_REFUND-1] + arChangedTips[TIP_PAY_CASH_REFUND-1];
	};

	long GetCheckTips() {
		return arNewTips[TIP_PAY_CHECK - 1] + arChangedTips[TIP_PAY_CHECK - 1] + arNewTips[TIP_PAY_CHECK_REFUND - 1] + arChangedTips[TIP_PAY_CHECK_REFUND-1];
	};

	long GetChargeTips() {
		return arNewTips[TIP_PAY_CHARGE-1] + arChangedTips[TIP_PAY_CHARGE-1] + arNewTips[TIP_PAY_CHARGE_REFUND-1] + arChangedTips[TIP_PAY_CHARGE_REFUND-1];
	};

	//TES 3/19/2015 - PLID 65069 - Added for consistency, not called anywhere
	long GetGiftTips() {
		return arNewTips[TIP_PAY_GIFT - 1] + arChangedTips[TIP_PAY_GIFT - 1] + arNewTips[TIP_PAY_GIFT_REFUND-1] + arChangedTips[TIP_PAY_GIFT_REFUND-1];
	}

	long GetChangedTips() {
		long n = 0;
		for (int i = 0; i < TIP_PAY_METHODS; i++) {
			n += arChangedTips[i];
		}
		return n;
	}

	long GetNewTips() {
		long n = 0;
		for (int i = 0; i < TIP_PAY_METHODS; i++) {
			n += arNewTips[i];
		}
		return n;
	}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

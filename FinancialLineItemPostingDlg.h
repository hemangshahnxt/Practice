#if !defined(AFX_FINANCIALLINEITEMPOSTINGDLG_H__BA761B69_3C3C_4B96_99DB_08B374587075__INCLUDED_)
#define AFX_FINANCIALLINEITEMPOSTINGDLG_H__BA761B69_3C3C_4B96_99DB_08B374587075__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FinancialLineItemPostingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFinancialLineItemPostingDlg dialog

#include "FinancialRc.h"

// (j.jones 2012-05-07 16:37) - PLID 37165 - this class is later defined in MultipleAdjustmentEntryDlg.h
class AdjustmentInfo;

// (j.jones 2012-05-08 08:56) - PLID 37165 - used to track adjustments per charge or revenue code
class ChargeAdjustmentInfo {

public:

	//these should be mutually exclusive, we wouldn't have both filled in
	long nChargeID;
	long nRevenueCodeID;
	CArray<AdjustmentInfo*, AdjustmentInfo*> aryAdjustmentInfo;

	ChargeAdjustmentInfo::ChargeAdjustmentInfo() {
		nChargeID = -1;
		nRevenueCodeID = -1;
	}
};

enum EBatchPaymentPayType;

class CFinancialLineItemPostingDlg : public CNxDialog
{
// Construction
public:

	// (z.manning 2008-06-19 11:00) - PLID 26544 - Replaced the #define column index variables
	// in the .cpp file with this enum.
	enum PostingListColumns
	{
		plcChargeID = 0,
		plcRevCodeID,
		plcServiceID,
		// (j.jones 2012-07-30 13:43) - PLID 47778 - Each charge will load the insured party's pay group copay $, copay %,
		// and/or coinsurance % (if any). This coinsurance is not the same field as the user-editable CoInsurance $ amount
		// that later gets saved to data.
		// (all three of these are null if a revenue code line)
		plcCoinsurancePercent,
		plcCopayMoney,
		plcCopayPercent,
		plcChargeDate,
		plcItemCode,
		plcBillNote,	// (j.jones 2010-06-11 11:55) - PLID 16704
		// (j.jones 2010-05-04 16:21) - PLID 25521 - renamed the existing pays columns to "Old" to differentiate
		// from the fact that we have editable columns for these values now
		plcOldExistingPatPays,
		plcOldExistingInsPays,
		// (j.jones 2012-07-30 16:41) - PLID 47778 - added a cache for "edited payment" and "edited adjustment",
		// so we know whether the user has manually edited the payment or adjustment on this datalist row, at
		// which point we will no longer attempt to auto-update those fields
		plcHasEditedPayment,
		plcHasEditedAdjustment,
		plcBeginDynamicCols,
	};

	CFinancialLineItemPostingDlg(CWnd* pParent);   // standard constructor
	~CFinancialLineItemPostingDlg();

	NXDATALISTLib::_DNxDataListPtr m_PostingList;

	NXDATALISTLib::_DNxDataListPtr m_ProviderCombo, m_LocationCombo;

	NXDATALISTLib::_DNxDataListPtr m_PayRespCombo, m_PayCatCombo, m_PayMethodCombo, m_PayCardTypeCombo;

	NXDATALISTLib::_DNxDataListPtr m_AdjCatCombo;

	NXDATALISTLib::_DNxDataListPtr m_BillInsuranceCombo, m_BillOtherInsuranceCombo;

	NXDATALISTLib::_DNxDataListPtr m_ShiftRespCombo;

	NXDATALISTLib::_DNxDataListPtr m_PayDescCombo, m_AdjDescCombo;

	// (a.walling 2006-11-16 10:05) - PLID 23552 - Group code and reason datalists
	NXDATALIST2Lib::_DNxDataListPtr m_pGroupCodeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pReasonList;

	// (j.jones 2014-06-30 14:17) - PLID 62642 - added chargebacks
	NXDATALIST2Lib::_DNxDataListPtr m_ChargebackCategoryCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_ChargebackDescriptionCombo;

	// (j.jones 2012-08-16 10:37) - PLID 52162 - Added ability to auto-post,
	// which should only be called when the dialog is invisible, not when it is modal.
	// If it cannot auto-post, the caller should display m_strAutoPostFailure and
	// then try to open again as a modal dialog.
	bool AutoPost();
	CString m_strAutoPostFailure;
	BOOL m_bIsAutoPosting;

	void SetDefaultData();

	void SetBatchPaymentData();
	
	// (j.jones 2012-08-16 10:13) - PLID 52162 - Changed the ID parameter so it always requires
	// the bill ID, and having a charge ID will filter on that charge only. Also moved nBatchPaymentID in here.	
	int DoModal(EBatchPaymentPayType ePayType, long nBillID, long nOnlyShowChargeID = -1, long nBatchPaymentID = -1, long nInsuredPartyID = -1, BOOL bUseRevCodes = FALSE, long nRevCodeID = -1);
	
	// (j.jones 2012-08-16 10:06) - PLID 52162 - added override for Create(), using our required parameters
	// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType
	// (j.jones 2015-10-20 08:48) - PLID 67377 - added default payment amount, used for auto-posting to
	// one charge only, when the default amount has already been calculated
	// (j.jones 2015-10-23 14:10) - PLID 67377 - added flag to indicate if this is a capitation payment
	virtual BOOL Create(UINT nIDTemplate, CWnd* pParentWnd, EBatchPaymentPayType ePayType, long nBillID,
		long nOnlyShowChargeID = -1, long nBatchPaymentID = -1, long nInsuredPartyID = -1, BOOL bUseRevCodes = FALSE, long nRevCodeID = -1,
		COleCurrency cyDefaultPaymentAmount = g_ccyInvalid, bool bIsCapitation = false);
	
	// (r.gonet 2016-01-25 09:16) - PLID 67942 - Sets a window to use as the "interface" window, which all message boxes displayed
	// by the line item posting dialog will use.
	void SetInterface(CWnd *pWnd);
	// (r.gonet 2016-01-25 09:16) - PLID 67942 - Gets the window to be used as the interface window, which all message boxes displayed
	// by the line item posting dialog will use.
	inline CWnd* GetInterface() const
	{
		return m_pInterface;
	}
	// (r.gonet 2016-01-25 10:37) - PLID 67942 - Wrapper around the MessageBox function which forces it to use the correct interface window.
	inline int MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption = NULL, UINT nType = MB_OK)
	{
		return GetInterface()->MessageBox(lpszText, lpszCaption, nType);
	}

	void LoadPostingList(); //faster than requerying with a big, nasty query for all the totals
	void AddPostingListChargeRow(long nChargeID);
	// (j.jones 2007-01-02 15:53) - PLID 24030 - added revenue code grouping
	// (j.jones 2012-07-31 10:28) - PLID 51863 - removed nBillID, it's always m_nBillID
	void AddPostingListRevCodeRow(long nRevenueCodeID);

	void SetCreditCardInfoForType(const IN long nCreditCardID);

	BOOL m_bGetChargeInfo; //stores the choice of wanting to re-load previous CC info

	bool PostPayments();
		
	// (j.jones 2014-07-01 11:44) - PLID 62553 - added an enumeration for whether this is a payment, adjustment,
	// chargeback payment, or chargeback adjustment
	// (j.jones 2015-10-26 11:11) - PLID 67451 - added version for capitation overpayment adjustment
	enum EPaymentType {
		ePayment = 0,
		eAdjustment,
		eChargebackPayment,
		eChargebackAdjustment,
		eCapitationOverpaymentAdjustment,
	};

	// (j.jones 2015-10-26 11:14) - PLID 67451 - added function to determine if one of the above enums
	// is an adjustment, to reduce repetition of enum comparisons
	bool IsPaymentTypeAdjustment(EPaymentType ePaymentType);

	// (j.jones 2012-05-09 09:50) - PLID 37165 - added GroupCodeID and ReasonCodeID, if not -1 they will save
	// on adjustments instead of the selected group/reason on the dialog
	long CreatePayment(COleCurrency cyPaymentAmt, EPaymentType ePaymentType, long nInsuredPartyID = -1, long nBatchPaymentID = -1, long nGroupCodeID = -1, long nReasonCodeID = -1);

	// (j.jones 2015-10-26 11:20) - PLID 67451 - added function to auto-create payment categories
	long AutoCreatePaymentCategory(CString strCategoryName);

	short ColumnToEdit(BOOL bStartOnLeft);

	// (j.jones 2007-05-21 10:01) - PLID 23751 - supported Line Item Posting auto-adjust options
	long m_nLineItemPostingAutoAdjust;
	// (j.jones 2012-08-14 09:54) - PLID 52076 - added sub-preference to only auto-adjust on primary
	long m_nLineItemPostingAutoAdjust_PrimaryOnly;
	// (j.jones 2013-07-03 16:13) - PLID 57226 - added preference for allowing adjustments on $0.00 payments
	long m_nLineItemPostingAutoAdjust_AllowOnZeroDollarPayments;

	// (j.jones 2012-07-30 10:58) - PLID 47778 - added ability to auto-calculate payment amounts
	long m_nLineItemPostingAutoPayment;
	// (j.jones 2012-08-28 17:10) - PLID 52335 - added the same ability for secondary payments
	long m_nLineItemPostingAutoPayment_Secondary;

	// (z.manning 2008-06-19 09:46) - PLID 26544 - Returns the default list of semi-colon delimited
	// column names for the dynamically ordered columns for the posting list.
	CString GetDefaultPostingDynamicColumnList();

	// (j.jones 2014-06-27 14:57) - PLID 62631 - exposed the PayType
	EBatchPaymentPayType GetPayType();

	// (a.walling 2008-04-02 09:45) - PLID 29497 - Use NxButtons for controls
	// (j.jones 2008-05-08 09:23) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CFinancialLineItemPostingDlg)
	enum { IDD = IDD_FINANCIAL_LINE_ITEM_POSTING_DLG };
	NxButton	m_btnGroupByRev;
	CNxIconButton	m_btnSwapInscos;
	CNxIconButton	m_btnPost;
	CNxIconButton	m_btnCancel;
	NxButton	m_checkShiftPaidAmounts;
	NxButton	m_checkAutoBatch;
	NxButton	m_checkAutoSwap;
	NxButton	m_radPostUnbatched;
	NxButton	m_radPostPaper;
	NxButton	m_radPostElectronic;
	CDateTimePicker	m_dtPayDate;
	CDateTimePicker	m_dtAdjDate;
	CNxEdit	m_nxeditPayTotal;
	CNxEdit	m_nxeditCashReceivedLip;
	CNxEdit	m_nxeditChangeGivenLip;
	// (j.jones 2015-09-30 11:04) - PLID 67175 - renamed to CCLast4
	CNxEdit	m_nxeditCCLast4;
	CNxEdit	m_nxeditCcExpDate;
	CNxEdit	m_nxeditCcNameOnCard;
	CNxEdit	m_nxeditEditAuthorization;
	CNxEdit	m_nxeditCheckNo;
	CNxEdit	m_nxeditAccountNumber;
	CNxEdit	m_nxeditBankName;
	CNxEdit	m_nxeditBankNumber;
	CNxEdit	m_nxeditPayDesc;
	CNxEdit	m_nxeditAdjTotal;
	CNxEdit	m_nxeditAdjDesc;
	CNxStatic	m_nxstaticPostingPatientNameLabel;
	CNxStatic	m_nxstaticPostingPatientIdLabel;
	CNxStatic	m_nxstaticPaymentLabel;
	CNxStatic	m_nxstaticPayAmtLabel;
	CNxStatic	m_nxstaticPayCurrencySymbol;
	CNxStatic	m_nxstaticPayDateLabel;
	CNxStatic	m_nxstaticPayRespLabel;
	CNxStatic	m_nxstaticPayCatLabel;
	CNxStatic	m_nxstaticPayMethodLabel;
	CNxStatic	m_nxstaticCashReceivedLipLabel;
	CNxStatic	m_nxstaticCurrencySymbolReceivedLip;
	CNxStatic	m_nxstaticChangeGivenLipLabel;
	CNxStatic	m_nxstaticCurrencySymbolGivenLip;
	CNxStatic	m_nxstaticCardTypeLabel;
	CNxStatic	m_nxstaticCcNumberLabel;
	CNxStatic	m_nxstaticCcExpDateLabel;
	CNxStatic	m_nxstaticCcNameOnCardLabel;
	CNxStatic	m_nxstaticAuthNumLabel;
	CNxStatic	m_nxstaticCheckNoLabel;
	CNxStatic	m_nxstaticAcctNumLabel;
	CNxStatic	m_nxstaticBankNameLabel;
	CNxStatic	m_nxstaticBankNumberLabel;
	CNxStatic	m_nxstaticPayDescLabel;
	CNxStatic	m_nxstaticAdjustmentLabel;
	CNxStatic	m_nxstaticAdjAmtLabel;
	CNxStatic	m_nxstaticAdjCurrencySymbol;
	CNxStatic	m_nxstaticAdjDateLabel;
	CNxStatic	m_nxstaticAdjCategoryLabel;
	CNxStatic	m_nxstaticAdjGroupcodeLabel;
	CNxStatic	m_nxstaticAdjReasonLabel;
	CNxStatic	m_nxstaticAdjDescriptionLabel;
	CNxStatic	m_nxstaticPaymentBalLabel;
	CNxStatic	m_nxstaticPaymentBalance;
	CNxStatic	m_nxstaticAdjustmentBalLabel;
	CNxStatic	m_nxstaticAdjustmentBalance;
	CNxStatic	m_nxstaticBillInscoLabel;
	CNxStatic	m_nxstaticBillOtherInscoLabel;
	CNxStatic	m_nxstaticCurrentBatchLabel;
	CNxStatic	m_nxstaticShiftBalancesLabel;
	CNxIconButton	m_btnConfigureColumns;
	CNxStatic	m_nxstaticTotalLabel0;
	CNxStatic	m_nxstaticTotalLabel1;
	CNxStatic	m_nxstaticTotalLabel2;
	CNxStatic	m_nxstaticTotalLabel3;
	CNxStatic	m_nxstaticTotalLabel4;
	CNxStatic	m_nxstaticTotalLabel5;
	CNxStatic	m_nxstaticTotalLabel6;
	CNxStatic	m_nxstaticTotalLabel7;
	CNxStatic	m_nxstaticTotalLabel8;
	// (j.jones 2010-05-04 16:09) - PLID 25521 - need a 9th total now
	CNxStatic	m_nxstaticTotalLabel9;
	// (b.spivey, January 04, 2012) - PLID 47121 - Need two more totals. 
	CNxStatic	m_nxstaticTotalLabel10;
	CNxStatic	m_nxstaticTotalLabel11;
	// (j.jones 2013-08-27 12:23) - PLID 57398 - added another column for copay
	CNxStatic	m_nxstaticTotalLabel12;

	CNxStatic	m_nxstaticTotal0;
	CNxStatic	m_nxstaticTotal1;
	CNxStatic	m_nxstaticTotal2;
	CNxStatic	m_nxstaticTotal3;
	CNxStatic	m_nxstaticTotal4;
	CNxStatic	m_nxstaticTotal5;
	CNxStatic	m_nxstaticTotal6;
	CNxStatic	m_nxstaticTotal7;
	CNxStatic	m_nxstaticTotal8;
	// (j.jones 2010-05-04 16:09) - PLID 25521 - need a 9th total now
	CNxStatic	m_nxstaticTotal9;
	// (b.spivey, January 04, 2012) - PLID 47121 - Need two more totals.
	CNxStatic	m_nxstaticTotal10;
	CNxStatic	m_nxstaticTotal11;
	// (j.jones 2013-08-27 12:23) - PLID 57398 - added another column for copay
	CNxStatic	m_nxstaticTotal12;

	// (j.jones 2010-05-17 16:51) - PLID 16503 - added ability to apply $0.00 payments
	NxButton	m_checkApplyZeroDollarPays;
	// (j.jones 2010-06-02 11:55) - PLID 37200 - added labels for unapplied amounts
	CNxStatic	m_nxstaticPatAmtToUnapplyLabel;
	CNxStatic	m_nxstaticPatAmtToUnapply;
	CNxStatic	m_nxstaticInsAmtToUnapplyLabel;
	CNxStatic	m_nxstaticInsAmtToUnapply;
	// (j.jones 2010-06-11 11:36) - PLID 16704 - added button for billing notes
	CNxIconButton	m_btnBillNotes;
	// (j.jones 2012-07-27 10:22) - PLID 26877 - added ability to filter reason codes
	CNxIconButton m_btnFilterReasonCodes;
	// (j.jones 2012-08-21 09:07) - PLID 52029 - added estimate payments button
	CNxIconButton m_btnEstPayments;
	// (j.jones 2014-06-30 14:17) - PLID 62642 - added chargebacks
	CNxStatic	m_nxstaticChargebackCategoryLabel;
	CNxStatic	m_nxstaticChargebackDescriptionLabel;
	CNxIconButton m_btnEditChargebackDescriptions;
	CNxEdit		m_nxeditChargebackDescription;
	// (j.jones 2014-07-01 09:04) - PLID 62551 - added chargeback total
	CNxStatic	m_nxstaticChargebackTotalLabel;
	CNxStatic	m_nxstaticChargebackTotal;
	// (j.jones 2014-07-01 09:22) - PLID 62552 - added an auto-adjust checkbox
	NxButton	m_checkAutoAdjustBalances;
	// (j.jones 2014-07-29 09:14) - PLID 63076 - added total payments labels, for vision only
	CNxStatic	m_nxstaticTotalPaymentsLabel;
	CNxStatic	m_nxstaticTotalPayments;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFinancialLineItemPostingDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	void CalculateTotals();

	long m_nBillID;		//will store the current bill ID
	long m_nChargeID;	//if we are posting to only one charge, then this is its ID
	long m_nPatientID;	//obviously, the patient this is for
	long m_nInsuredPartyID;	//the insured party currently being edited
	// (j.jones 2012-08-08 11:17) - PLID 47778 - m_bIsPrimaryIns is true if the current insured party
	// is the patient's primary insurance, either for medical or for vision
	BOOL m_bIsPrimaryIns;

	// (j.jones 2007-01-02 15:58) - PLID 24030 - added revenue code grouping
	long m_nRevCodeID;		//if we are posting to only one rev code, then this is its ID
	BOOL m_bUseRevCodes;	//lets the caller tell us whether or not to filter by rev code
	
	//this could be created from a batch payment
	long m_nBatchPaymentID;	//the batch payment that is at the root of this posting

	// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType
	EBatchPaymentPayType m_ePayType;

	// (j.jones 2015-10-20 08:53) - PLID 67377 - added default payment amount, only used
	// when posting to one charge
	COleCurrency m_cyDefaultPaymentAmount;

	// (j.jones 2015-10-23 14:10) - PLID 67377 - added flag to indicate if this is a capitation payment
	bool m_bIsCapitation;

	// (j.jones 2009-03-11 09:02) - PLID 32864 - track the bill's original insured party IDs and names, for auditing
	long m_nBillOldInsuredPartyID;
	long m_nBillOldOtherInsuredPartyID;
	CString m_strOldInsuranceCoName;
	CString m_strOldOtherInsuranceCoName;

	// (z.manning 2008-06-19 10:19) - PLID 26544 - Index variables for the dynamic columns
	short m_nChargesColIndex;
	short m_nPatRespColIndex;
	short m_nInsRespColIndex;
	// (j.jones 2010-05-04 16:14) - PLID 25521 - renamed to only be new payments
	short m_nNewPaysColIndex;
	short m_nAllowableColIndex;
	short m_nAdjustmentColIndex;
	short m_nPatBalanceColIndex;
	short m_nInsBalanceColIndex;
	// (j.jones 2010-05-04 16:09) - PLID 25521 - existing pat/ins pays are now visible
	short m_nExistingPatPaysColIndex;
	short m_nExistingInsPaysColIndex;
	// (b.spivey, January 03, 2012) - PLID 47121 - Deductible and Coinsurance
	// (j.jones 2012-07-30 14:26) - PLID 47778 - renamed to be a little clearer now that we also have a Coinsurance % column
	short m_nDeductibleAmtColIndex; 
	short m_nCoinsuranceAmtColIndex;
	// (b.spivey, January 16, 2012) - PLID 47121 - Removed m_nChargeCoinsHasRowColIndex variable.
	// (j.jones 2013-08-27 11:55) - PLID 57398 - added copay
	short m_nCopayAmtColIndex;

	//***When adding new columns, don't forget to increase DYNAMIC_COLUMN_COUNT in the .cpp.***//

	// (j.jones 2010-06-11 11:56) - PLID 16704 - added note icons for the datalist
	HICON m_hNotes;

	//Sums the adjustments column, puts that total in the edit box in the adjustment section.
	void AutoFillAdjustmentTotal();

	// (j.jones 2012-08-17 09:00) - PLID 47778 - added payment calculations
	//Sums the payments column, puts that total in the edit box in the payments section.
	void AutoFillPaymentTotal();

	// (a.walling 2006-11-15 09:42) - PLID 23552 - Initialize the group and reason code datalists with hardcoded values
	// (j.jones 2010-09-23 15:07) - PLID 40653 - obsolete
	//void InitGroupCodesAndReasons();

	// (a.walling 2006-11-15 09:42) - PLID 23552 - Enable/Disable the group and reason code controls
	void EnableGroupCodesAndReasons(OPTIONAL IN bool bShow /* = true */);

	CBrush m_greenbrush, m_bluebrush, m_graybrush;

	// (j.jones 2007-02-01 17:30) - PLID 24541 - for tracking focus changes
	BOOL m_bIsEditingCharges;

	// (r.gonet 2016-01-25 10:10) - PLID 67942 - Window to use as the interface for showing message boxes.
	CWnd *m_pInterface = nullptr;

	// (j.jones 2007-08-29 09:27) - PLID 27176 - added one function to calculate adjustments
	// (j.jones 2013-07-03 16:51) - PLID 57226 - changed the payment parameter to be a variant,
	// because payments can sometimes be null, and are treated differently than $0.00
	// (j.jones 2014-07-01 09:36) - PLID 62552 - renamed to reflect this is the medical adjustment logic	
	COleCurrency CalculateAdjustment_Medical(COleCurrency cyInsResp, COleCurrency cyPatResp, _variant_t varInsPays, COleCurrency cyAllowable);

	// (j.jones 2014-07-01 09:36) - PLID 62552 - added a vision-specific version of CalculateAdjustment
	COleCurrency CalculateAdjustment_Vision(COleCurrency cyInsResp, COleCurrency cyPatResp, _variant_t varInsPays);

	// (j.jones 2015-10-23 14:10) - PLID 67377 - added a capitation-specific version of CalculateAdjustment
	// which simply adjusts off the balance of the insurance resp., ignoring the allowable
	COleCurrency CalculateAdjustment_Capitation(COleCurrency cyInsResp, COleCurrency cyInsPays);

	// (j.jones 2012-07-30 10:58) - PLID 47778 - added ability to auto-calculate payment amounts
	// (j.jones 2012-08-21 09:26) - PLID 52029 - added bSilent, if FALSE then we can warn when we can't calculate
	// (j.jones 2012-08-28 17:20) - PLID 52335 - now requires the insurance resp. balance
	// (j.jones 2013-08-27 11:48) - PLID 57398 - Added copay manual amount, this is the amount they would have manually typed in.
	// Also renamed the other two manually typed in columns for clarity.
	COleCurrency CalculatePayment(BOOL bSilent, CString strItemCode, COleCurrency cyChargeTotal, COleCurrency cyInsBalance,
								  COleCurrency cyAllowable, _variant_t varCoInsurancePercent,
								  _variant_t varCopayMoney, _variant_t varCopayPercent,
								  _variant_t varDeductibleManualAmount, _variant_t varCoinsuranceManualAmount,
								  _variant_t varCopayManualAmount);

	// (j.jones 2012-07-31 09:26) - PLID 51863 - added ability to auto-calculate payment amounts by revenue code
	// (j.jones 2012-08-21 09:26) - PLID 52029 - added bSilent, if FALSE then we can warn when we can't calculate
	// (j.jones 2012-08-28 17:20) - PLID 52335 - now requires the insurance resp. balance
	// (j.jones 2013-08-27 11:48) - PLID 57398 - Added copay manual amount, this is the amount they would have manually typed in.
	// Also renamed the other two manually typed in columns for clarity.
	COleCurrency CalculatePaymentForRevenueCode(BOOL bSilent, CString strItemCode, long nRevCodeID, COleCurrency cyTotalCharges, COleCurrency cyTotalInsBalance,
		COleCurrency cyTotalAllowable, _variant_t varDeductibleManualAmount, _variant_t varCoinsuranceManualAmount, _variant_t varCopayManualAmount);

	// (z.manning 2008-06-19 09:52) - PLID 26544 - Removes and then reloads all dynamic columns
	// based on the global preference.
	void ReloadDynamicColumns();

	// (z.manning 2008-06-19 10:52) - PLID 26544 - Set the column index variable based on the given column name.
	void AssignColumnIndex(const CString strColName, const short nColIndex);

	// (j.jones 2010-05-05 09:34) - PLID 25521 - added ability to validate the loaded dynamic columns
	BOOL IsColumnListValid(CString strColumnList);

	// (z.manning 2008-06-19 11:51) - PLID 26544 - Sets the total field for the given index to the given value
	void SetTotal(const short nColIndex, const COleCurrency cyTotal);

	// (j.jones 2008-07-11 14:47) - PLID 28756 - this function will check
	// which insurance company is selected (if any), and override the 
	// payment description and payment category, as well as adjustments
	void TrySetDefaultInsuranceDescriptions();

	// (j.jones 2008-07-11 15:33) - PLID 28756 - moved most of the OnSelChosenPayInsRespCombo logic into this function
	void PostSelChosenPayInsRespCombo();

	// (j.jones 2010-06-02 13:47) - PLID 37200 - track the total to unapply
	COleCurrency m_cyUnappliedPatAmt;
	COleCurrency m_cyUnappliedInsAmt;

	// (j.jones 2012-05-07 16:31) - PLID 37165 - used to track adjustments per charge or revenue code
	CArray<ChargeAdjustmentInfo*, ChargeAdjustmentInfo*> m_aryChargeAdjustmentInfo;

	// (j.jones 2012-05-08 09:39) - PLID 37165 - Finds a ChargeAdjustmentInfo entry in m_aryChargeAdjustmentInfo
	// to match a charge ID or revenue code ID. Returns NULL if not found.
	ChargeAdjustmentInfo* FindChargeAdjustmentInfo(long nChargeID, long nRevenueCodeID);

	// (j.jones 2012-05-08 09:43) - PLID 37165 - opens the MultipleAdjustmentEntryDlg with a given
	// charge's information & adjustments
	void OpenMultipleAdjustmentEntryDlg(long nRow);

	// (j.jones 2012-05-08 10:26) - PLID 37165 - updates adjustment amounts in tracked pointers
	void UpdateAdjustmentPointers(COleCurrency cyNewAdjustmentTotal, ChargeAdjustmentInfo* pChargeAdjInfo);

	// (j.jones 2012-05-08 16:35) - PLID 37165 - used to clear our memory objects for adjustments
	void ClearAdjustmentPointers();

	// (j.jones 2012-08-14 14:39) - PLID 50285 - added ability to create a billing note with the user-entered
	// deductible and/or coinsurance values
	// (j.jones 2013-08-26 16:45) - PLID 57398 - also applies to copays now
	void CreateBillingNoteForDeductibleCoinsCopay(long nChargeID, _variant_t varDeductible, _variant_t varCoinsurance, _variant_t varCopay);

	// (j.jones 2014-06-27 11:28) - PLID 62548 - For Vision payments, this hides many controls and columns.
	// For Medical payments, will only hide vision-specific controls.
	void UpdateVisibleControls();

	// (j.jones 2014-07-01 11:41) - PLID 62553 - supported chargebacks
	void CreateChargeback(long nChargeID, COleCurrency cyChargebackAmt);

	// Generated message map functions
	//{{AFX_MSG(CFinancialLineItemPostingDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedPostingInsuranceCoList(short nFlags);
	afx_msg void OnRequeryFinishedPostingOtherInsuranceCoList(short nFlags);
	afx_msg void OnBtnSwapInscos();
	afx_msg void OnSelChosenPayTypeCombo(long nRow);
	afx_msg void OnSelChosenComboCardName(long nRow);
	afx_msg void OnEditCardName();
	afx_msg void OnEditingFinishingLineItemPostingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedLineItemPostingList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedPayInsRespCombo(short nFlags);
	afx_msg void OnSelChosenPayInsRespCombo(long nRow);
	afx_msg void OnSelChosenPostingInsuranceCoList(long nRow);
	afx_msg void OnSelChosenPostingOtherInsuranceCoList(long nRow);
	afx_msg void OnKillfocusCashReceived();
	afx_msg void OnKillfocusPayTotal();
	afx_msg void OnKillfocusAdjTotal();
	afx_msg void OnSelChosenPayDescriptionCombo(long nRow);
	afx_msg void OnSelChosenAdjDescriptionCombo(long nRow);
	afx_msg void OnEditPayDesc();
	afx_msg void OnEditAdjDesc();
	afx_msg void OnSelChangingReason(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingGroupCode(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnCheckGroupChargesByRevenueCode();
	afx_msg void OnFocusGainedLineItemPostingList();
	afx_msg void OnEditingStartingLineItemPostingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnDatetimechangePayDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnConfigureColumns();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	// (j.jones 2010-06-11 11:36) - PLID 16704 - added button for billing notes
	afx_msg void OnBtnBillNotes();
	// (j.jones 2010-06-11 11:36) - PLID 16704 - added ability to click on charge notes
	afx_msg void OnLeftClickLineItemPostingList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2012-05-07 09:13) - PLID 37165 - added ability to add multiple adjustments per charge
	void OnRButtonDownLineItemPostingList(long nRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2012-07-27 10:19) - PLID 26877 - added ability to filter reason codes
	afx_msg void OnBtnFilterReasonCodes();
	// (j.jones 2012-08-21 09:07) - PLID 52029 - added estimate payments button
	afx_msg void OnBtnEstPayments();
	// (j.jones 2014-06-30 14:58) - PLID 62642 - added chargebacks
	afx_msg void OnBtnEditCbDesc();
	afx_msg void OnSelChosenCbDescriptionCombo(LPDISPATCH lpRow);
	// (j.jones 2014-07-01 09:31) - PLID 62552 - this is for vision payments only
	afx_msg void OnCheckAutoAdjustBalances();
	// (r.gonet 2016-01-28 21:04) - PLID 67942 - Handle the event where the window is about to be created.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINANCIALLINEITEMPOSTINGDLG_H__BA761B69_3C3C_4B96_99DB_08B374587075__INCLUDED_)

#if !defined(AFX_EOBDLG_H__0B5007D3_CE13_4E84_B678_F8F2917E4851__INCLUDED_)
#define AFX_EOBDLG_H__0B5007D3_CE13_4E84_B678_F8F2917E4851__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EOBDlg.h : header file
//

#include "ANSI835Parser.h"
#include "OHIPERemitParser.h"
#include "FinancialRc.h"
#include "AlbertaAssessmentParser.h"

enum ERespStatus {
	ePassed = 0,
	eFailed,
	eSkipped,
};

// (j.jones 2011-04-04 16:29) - PLID 42571 - added a struct for
// returning InsuredPartyID and RespTypeT.CategoryPlacement
struct EInsuredPartyInfo {
	long nInsuredPartyID;
	long nCategoryPlacement;
};

/////////////////////////////////////////////////////////////////////////////
// CEOBDlg dialog

class CEOBDlg : public CNxDialog
{
// Construction
public:
	CEOBDlg(CWnd* pParent = NULL);   // standard constructor
	~CEOBDlg();

	// (j.jones 2008-12-19 08:46) - PLID 32519 - added ability to auto-open a file
	CString m_strAutoOpenFilePath;

	// (j.jones 2008-06-18 11:09) - PLID 21921 - added more NxStatics
	// (j.jones 2008-11-24 09:23) - PLID 32075 - added m_btnConfigAdjCodesToSkip
// Dialog Data
	//{{AFX_DATA(CEOBDlg)
	enum { IDD = IDD_EOB_DLG };
	// (r.gonet 2016-04-18) - NX-100162 - Renamed the button because the dialog it launches is not
	// just for ignored adjustment codes anymore.
	CNxIconButton	m_btnAdjustmentCodeSettings;
	CNxIconButton	m_btnImportOHIP;
	NxButton	m_btnAutoShift;
	// (j.jones 2013-03-18 14:22) - PLID 55574 - renamed this checkbox
	NxButton	m_checkOnlyShiftPostedCharges;
	CNxIconButton	m_btnEditAdjCat;
	CNxIconButton	m_btnEditPayCat;
	CNxIconButton	m_btnEditAdjDesc;
	CNxIconButton	m_btnEditPayDesc;
	CNxIconButton	m_btnPrint;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CProgressCtrl	m_progress;
	CNxIconButton	m_btnImport835;
	CNxEdit			m_progressStatus;
	CNxEdit	m_nxeditPayDesc;
	CNxEdit	m_nxeditAdjDesc;
	CNxStatic	m_nxstaticInscoNameLabel;
	CNxStatic	m_nxstaticProviderLabel;
	CNxStatic	m_nxstaticProviderNameLabel;
	CNxStatic	m_nxstaticPaymentLabel;
	CNxStatic	m_nxstaticTotalPaymentsLabel;
	CNxStatic	m_nxstaticPayDescLabel;
	CNxStatic	m_nxstaticPayCatLabel;
	CNxStatic	m_nxstaticAdjDescriptionLabel;
	CNxStatic	m_nxstaticAdjCategoryLabel;
	CNxStatic	m_nxstaticShiftBalancesLabel;
	CNxStatic	m_nxstaticInsCoLabel;
	CNxStatic	m_nxstaticInsCoDropdownLabel;
	CNxStatic	m_nxstaticProviderDropdownLabel;
	CNxStatic	m_nxstaticLocationDropdownLabel;
	CNxIconButton	m_btnAutoSkipDuplicates;	// (j.jones 2009-06-09 12:09) - PLID 33863
	// (j.jones 2009-07-01 16:27) - PLID 34778 - added m_nxstaticEstimatedOverpaymentLabel
	// (j.jones 2012-10-05 10:38) - PLID 52929 - renamed to reflect that this now handles underpayments too
	CNxStatic	m_nxstaticEstimatedOverpaymentUnderpaymentLabel;
	// (j.jones 2010-02-08 15:33) - PLID 37174 - added ability to configure import filtering
	CNxIconButton	m_btnConfigImportFilters;
	// (j.jones 2010-04-09 12:00) - PLID 31309 - added date controls
	CDateTimePicker	m_dtPayDate;
	CDateTimePicker	m_dtAdjDate;
	NxButton	m_checkEnablePayDate;
	NxButton	m_checkEnableAdjDate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEOBDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

private:
	
	// (j.jones 2012-10-12 10:26) - PLID 53149 - This dialog is now modeless.
	// It should still work if called modally, but it's not desireable, so this
	// line will cause a compilation failure if someone tries to open it modally.
	// Just remove this line if we really do want a modal version again.
	using CNxDialog::DoModal;

// Implementation
protected:
	// (s.tullis 2016-04-15 16:00) - NX-100211
	NXDATALIST2Lib::_DNxDataListPtr m_EOBList;

	NXDATALISTLib::_DNxDataListPtr m_InsuranceCombo, m_ProviderCombo, m_LocationCombo;

	NXDATALISTLib::_DNxDataListPtr m_ShiftRespCombo;

	NXDATALIST2Lib::_DNxDataListPtr m_PayDescCombo, m_AdjDescCombo, m_PayCatCombo, m_AdjCatCombo;

	CANSI835Parser m_EOBParser;

	// (j.jones 2008-06-16 17:21) - PLID 30413 - supported OHIP E-Remittance
	COHIPERemitParser m_OHIPParser;
	
	//TES 7/30/2014 - PLID 62580 - Changed m_bIsOhip to m_ERemitType
	enum ERemitType {
		ertAnsi,
		ertOhip,
		ertAlberta,
	};
	ERemitType m_ERemitType;

	//TES 7/30/2014 - PLID 62580 - Added support for Alberta Assessment files
	CAlbertaAssessmentParser m_AlbertaParser;

	//TES 8/4/2014 - PLID 62580 - Added support for Alberta Assessment files
	void ImportAlbertaFile();

	// (j.jones 2011-06-16 08:40) - PLID 39747 - tracks what our insurance combo filter should be,
	// if it is not DEFAULT_INSURANCE_FILTER, then we show the options to show all / show less
	CString m_strCurInsuranceComboFilter;

	// (j.jones 2011-01-04 14:40) - PLID 16423 - added billing notes
	HICON m_hNotes;
	
	// (j.jones 2008-06-18 13:53) - PLID 21921 - renamed the old ProcessEOB function into TryProcessEOB,
	// and split the ProcessEOB() functionality into its own sub-function
	BOOL TryProcessEOB();
	// (j.jones 2009-06-23 17:43) - PLID 32184 - changed the return value to be void
	// (j.jones 2009-07-07 15:32) - PLID 34805 - renamed the cyTotalPaymentAmt variable to cyInitialBatchPayAmt
	// for clarity, since the batch payment that is created may end up with a different amount
	// (j.jones 2009-09-25 11:03) - PLID 34453 - added parameters for the EOB pointers
	// (j.jones 2010-04-09 12:13) - PLID 31309 - added dtPayDate and dtAdjDate
	// (b.spivey, October 9th, 2014) PLID 62701 - Pass in the storage location. 
	void ProcessEOB(CString strFileName, const long nEOBID, const long nIndex, long nInsuranceCompanyID, CString strInsCoName,
		COleCurrency cyInitialBatchPayAmt, CString strPayDescription, long nPaymentCategoryID,
		long nLocationID, long nProviderID, long nAdjustmentCategoryID, CString strAdjDescription,
		CString strCheckNumber, CString strPayBankRoutingNo, CString strPayBankName, CString strPayAccount,
		COleDateTime dtPayDate, COleDateTime dtAdjDate,
		EOBInfo *pEOBInfo, OHIPEOBInfo *pOHIPEOBInfo, CString strParsedFile = "");

	// (j.jones 2012-04-24 09:56) - PLID 35306 - Called by ProcessEOB, this will iterate through the list
	// and post the EOB. It's a separate function so that it can make one pass to post reversals, and
	// a second pass to post normal payments.
	void PostEOBLineItems(const BOOL bOnlyPostReversedClaims, const long nEOBID, const long nIndex,
		const long nBatchPaymentID, IN OUT COleCurrency &cyBatchPayBalance, IN OUT COleCurrency &cyActualBatchPaymentAmount,
		const BOOL bAllowZeroDollarAdjustments, const COleDateTime dtAdjDate, const long nAdjustmentCategoryID, const CString strAdjDescription);

	void Print(long EOBID);
	
	//thee functions are used to Process the EOB
	// (j.jones 2009-03-05 11:32) - PLID 33235 - supported adding Billing Notes
	// (j.armen 2012-02-20 09:20) - PLID 34344 - these parameraters do not change inside the function, so declare them as const
	// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
	// (d.singleton 2014-10-15 14:35) - PLID 62698 - added claimnumber
	// (j.jones 2016-04-19 17:33) - NX-100246 - we now return the new payment ID
	// we now specify a separate amount to apply, to cover the case where the total payment we make is not fully applied,
	// such as when negative adjustments are present
	long ApplyPayment(const long &nBatchPaymentID, const long &nPatientID, const long &nPatientUserDefinedID,
		const long &nChargeID, const long &nInsuredPartyID,
		const COleCurrency &cyTotalPaymentAmt, const COleCurrency &cyPaymentAmtToApply,
		const CString &strBillingNote, const CString &strClaimNumber);

	// (j.jones 2008-02-20 11:30) - PLID 29007 - added nAdjustmentCategoryID and strGlobalAdjustmentDescription
	// (j.jones 2009-06-09 14:35) - PLID 34539 - required the BatchPaymentID
	// (j.jones 2010-04-09 12:10) - PLID 31309 - added dtAdjDate
	// (j.armen 2012-02-20 09:21) - PLID 34344 - these parameraters do not change inside the function, so declare them as const
	// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
	// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
	// (d.singleton 2014-10-15 14:35) - PLID 62698 - added claimnumber
	// (j.jones 2016-04-19 17:33) - NX-100246 - added nNewPaymentID, the ID from ApplyPayment, which negative adjustments will apply to
	void ApplyAdjustment(const long& nBatchPaymentID, const long& nPatientID,const long&  nPatientUserDefinedID,  const long& nChargeID, 
		const long& nInsuredPartyID, const COleCurrency& cyAdjustmentAmt, const COleDateTime& dtAdjDate, 
		const CString& strGroupCode, const CString& strReasonCode, const CString& strIndivAdjustmentDescription, 
		const long& nAdjustmentCategoryID, const CString& strGlobalAdjustmentDescription, const CString& strClaimNumber,
		const long& nNewPaymentID);

	// (j.jones 2008-11-17 12:07) - PLID 32007 - added bCalcOnly and cyInsAmtToShift, so we can call this function only to find out
	// how much will be shifted, or we can call it to actually perform the shifting
	void TryShiftRespToPatient(long nPatientID, long nChargeID, long nInsuredPartyID, COleCurrency cyPatResp, BOOL bCalcOnly, COleCurrency &cyInsAmtToShift);

	// (j.jones 2011-11-08 15:18) - PLID 46240 - removed cyPatResp as a parameter, it was unused
	// (j.jones 2012-08-23 12:49) - PLID 42438 - added nBillID
	// (j.jones 2013-03-18 13:51) - PLID 55574 - Removed cyPaymentAmount, for one it was inaccurately named
	// because it included adjustments, but it also wasn't needed. Added bChargeWasPosted, which is true if
	// we did post a payment > $0.00, or an adjustment > $0.00, or if the reported patient resp ('other' resp) is > $0.00.
	void TryShiftBalances(long nPatientID, long nBillID, long nChargeID, long nInsuredPartyID, BOOL bChargeWasPosted);

	// (j.dinatale 2013-03-25 17:39) - PLID 55825
	// (j.jones 2016-04-19 16:09) - NX-100161 - obsolete now
	//BOOL CheckForInvalidReversals();

	BOOL CheckEnsureResponsibilities();

	// (j.jones 2013-07-09 15:56) - PLID 55573 - Renamed to reflect that this will now will also verify
	// balances are what we expected them to be.
	void CheckShiftAndVerifyBalances();

	// (j.jones 2012-08-23 12:44) - PLID 42438 - if any claims now have a $0.00 balance,
	// this function will unbatch them if they are still batched (and not warn about it)
	void CheckUnbatchClaims();

	// (j.dinatale 2012-11-06 10:54) - PLID 50792 - need to unbatch MA18 claims
	// (j.jones 2013-07-18 11:26) - PLID 57616 - renamed to be generic
	void UnbatchCrossoverClaims();

	// (j.jones 2008-03-26 10:11) - PLID 29407 - cyAdjustmentAmount may actually be changed now within this function
	// (j.jones 2008-11-17 12:24) - PLID 32007 - added cyPatientResp
	// (j.jones 2009-04-07 15:04) - PLID 33862 - removed bDuplicate
	// (j.jones 2010-02-08 11:53) - PLID 37182 - added bIsPrimaryIns
	// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed cyPatientResp to cyOtherResp, because even though the
	// EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
	// (j.jones 2012-04-24 15:51) - PLID 35306 - Added reversed amount, so we know how much is currently applied to the
	// given insurance resp. that we're about to void. This is a positive value, not negative.
	// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
	// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
	// (j.jones 2016-04-19 14:33) - NX-100161 - renamed the adjustment amount to cyPositiveAdjustmentAmount
	// to clarify that we are only looking at the total of positive adjustments we are trying to apply to the charge
	ERespStatus EnsureValidResponsibilities(long nPatientID,long nPatientUserDefinedID ,long nChargeID, long nInsuredPartyID, BOOL bIsPrimaryIns,
		COleCurrency cyPaymentAmount, COleCurrency &cyPositiveAdjustmentAmount, const COleCurrency cyOtherResp, COleCurrency cyReversedAmount, CString strPatInsCoName);

	// (j.jones 2012-04-24 15:51) - PLID 35306 - calculate how much is currently applied
	// to the given insurance resp. that we're about to void
	//TES 9/18/2014 - PLID 62536 - Added pAlbertaCharge
	COleCurrency CalcAmountToBeReversed(long nChargeID, long nInsuredPartyID, const EOBLineItemInfo *pCharge, AlbertaAssessments::ChargeInfoPtr pAlbertaCharge);
	
	// (j.jones 2008-03-26 10:19) - PLID 29407 - given an adjustment, payment, and a balance, find what the new balance would be
	// after the payment is applied, and if the adjustment is greater than that value, reduce the adjustment amount
	// (j.jones 2011-03-07 16:09) - PLID 41877 - Added cyPatientRespNeeded, representing the amount the EOB says should
	// be a balance of the charge (for any resp, really), and we reduced the adjustment to account for it.
	// (j.jones 2011-11-02 08:54) - PLID 46232 - We now have a minimum adjustment amount, which is the most we can reduce the adjustment to.
	// I also removed cyAvailableBalance, because this shouldn't be called until after we've forced the ins. balance to be whatever we needed.
	// (j.jones 2011-11-08 14:34) - PLID 46240 - I renamed cyPatientRespNeeded to cyOtherRespNeeded, because even though the
	// EOB says patient resp, we need to remember this really means "resp. that isn't this insurance company"
	// (j.jones 2012-06-27 13:33) - PLID 51236 - added bWeAreShiftingPatientResp, so the reduction logic knows how to calculate the required "other resp" needed
	// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
	void ReduceAdjustmentValue(IN OUT COleCurrency &cyCurrentAdjustmentAmount, COleCurrency cyMinimumAdjustmentAmount,
								COleCurrency cyPaymentAmount, long nInsuredPartyID,
								long nPatientID,long nPatientUserDefinedID,  long nChargeID, const COleCurrency cyOtherRespNeeded, const BOOL bWeAreShiftingPatientResp);

	// (j.jones 2016-04-20 17:00) - NX-100251 - our unapply warnings are NxTaskDialogs that will return one
	// of the following options
	enum UnapplyWarningResult
	{
		eSkip = 1000,		//the user decided to skip posting this charge
		eProcess = 1001,	//the user agreed to process this charge & whatever changes we just warned about		
		eCancel = 1003,		//the user wants to abort processing this EOB
	};

	// (j.jones 2008-03-26 10:11) - PLID 29407 - all of these functions now take in the bReduceAdjustments parameter
	// (j.jones 2011-11-02 09:01) - PLID 46232 - renamed bReduceAdjustments to bCanReduceAdjustments, so we know which boolean it represents from EnsureValidResponsibilities
	// (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
	// (b.eyers 2016-04-15 12:11) - NX-100187 - added strPatInsCoName
	// (j.jones 2016-04-20 17:00) - NX-100251 - these now create their own messagebox and returns an enum for what the user chose to do
	UnapplyWarningResult GenerateUnapplyWarning_PossibleDuplicateWithNotEnoughResp(long nChargeID, long nPatientID,long nPatientUserDefinedID, BOOL bInsurance, BOOL bPatient, COleCurrency cyPaymentAmount, COleCurrency cyAdjustmentAmount, COleCurrency cyInsuranceTotalToUnapply, COleCurrency cyPatientTotalToUnapply, CString &strWarningToLog, BOOL bCanReduceAdjustments, CString strPatInsCoName);
	UnapplyWarningResult GenerateUnapplyWarning_NotEnoughResp(long nChargeID, long nPatientID,long nPatientUserDefinedID, BOOL bInsurance, BOOL bPatient, COleCurrency cyPaymentAmount, COleCurrency cyAdjustmentAmount, COleCurrency cyInsuranceTotalToUnapply, COleCurrency cyPatientTotalToUnapply, CString &strWarningToLog, BOOL bCanReduceAdjustments, CString strPatInsCoName);
	UnapplyWarningResult GenerateOverpaymentWarning(long nChargeID, long nPatientID,long nPatientUserDefinedID, COleCurrency cyInsBalance, COleCurrency cyPaymentAmount, COleCurrency cyAdjustmentAmount, CString &strWarningToLog, BOOL bCanReduceAdjustments /*, BOOL bDuplicate*/, CString strPatInsCoName);
	
	// (j.jones 2011-03-16 16:30) - PLID 42866 - Insurance ID is now split between original and corrected, in the rare cases that we receive a corrected ID
	// (j.jones 2011-04-04 16:30) - PLID 42571 - changed the return value to be a struct	
	// (j.armen 2012-02-20 09:22) - PLID 34344 - these parameraters do not change, so declare them as const
	// (j.jones 2012-05-01 15:02) - PLID 47477 - supported the ProcessedAs flag
	// (j.jones 2013-07-10 11:20) - PLID 57263 - replaced the ProcessedAs variable with the claim pointer
	// (j.jones 2016-04-13 16:20) - NX-100184 - added EOB pointer, removed redundant parameters that already existed in pClaim and pEOB,
	EInsuredPartyInfo CalcIndivPatientInsuredPartyID(const EOBInfo* pEOB, const EOBClaimInfo *pClaim, const long& nChargeID);

	// (j.jones 2016-04-13 16:20) - NX-100184 - made a new version of this function which contains all the old
	// policy matching logic with optional usage of the ProcessedAs flag
	EInsuredPartyInfo CalcIndivPatientInsuredPartyID_ByPolicyInfo(const EOBInfo* pEOB, const EOBClaimInfo *pClaim, const long& nChargeID, bool bUseProcessedAsFlag);

	// (j.jones 2011-04-04 16:30) - PLID 42571 - changed the return value to be a struct
	// (j.jones 2016-04-14 13:30) - NX-100184 - added an option to force checking balance only, and ignore
	// the highest resp, will return arbitrary first result if no resp has a balance
	EInsuredPartyInfo CalcInsuredPartyIDByResp(ADODB::_RecordsetPtr rs, long nChargeID, long nPatientID, bool bBalanceCheckOnly = false);

	// (j.jones 2012-05-01 15:36) - PLID 47477 - added function to filter by just the Processed As flag and HCFA group ID
	EInsuredPartyInfo CalcInsuredPartyIDByProcessedAs(long nChargeID, long nPatientID, long nProcessedAs, long nHCFAGroupID);

	// (j.jones 2012-04-20 16:40) - PLID 49846 - CalcReversedPaymentAndAdjustmentIDs will fill the
	// nReversedPaymentID and nReversedAdjustmentID for reversed line items, if any.
	void CalcReversedPaymentAndAdjustmentIDs(IN OUT EOBClaimInfo *pClaim, IN OUT EOBLineItemInfo *pCharge);

	//TES 8/4/2014 - PLID 62580 - Added support for Alberta Assessment files
	void CalcReversedPaymentIDs(AlbertaAssessments::ChargeInfoPtr pCharge);

	void CalcWarnNoInsuranceCompanyChosen();

	void EnableButtons(BOOL bEnabled);
	BOOL m_bIsLoading;

	CFile m_fileLogWarnings;
	BOOL m_bWarningsCreated;
	CString m_strWarningOutputFile;
	void AddWarningToLog(CString strWarning, CString strAction);
	// (j.jones 2011-03-21 13:29) - PLID 42917 - added EOBID, can be -1
	// if the log is opened prior to anything actually being posted
	BOOL OpenWarningLog(long nEOBID = -1);

	// (j.jones 2011-03-21 13:43) - PLID 42917 - this function backs up the warning
	// log to the server's NexTech\EOBWarningLogs path, and also ensures that files
	// > 30 days old are deleted
	void CloseWarningLogAndCopyToServer();

	CBrush m_greenbrush, m_bluebrush;

	// (j.jones 2011-02-08 15:06) - PLID 42392 - this is used to ensure that the preference
	// for a takeback adjustment category is linked to a valid category
	// (j.jones 2012-02-13 10:54) - PLID 48084 - this is now obsolete, we no longer make
	// adjustments instead of payments
	//long m_nAdjOverageTakebackCategoryID;

	// (j.jones 2008-07-11 16:35) - PLID 28756 - this function will check
	// which insurance company is selected, and override the 
	// payment description and payment category 
	void TrySetDefaultInsuranceDescriptions();

	// (j.jones 2009-03-05 10:06) - PLID 33076 - Added IncreaseBatchPaymentBalance,
	// which should only be used in OHIP postings, and is called when OHIP pays more
	// on the EOB than the check was for, in which case we increment the batch payment.
	void IncreaseBatchPaymentBalance(long nBatchPaymentID, COleCurrency cyAmtToIncrease);

	// (j.jones 2009-07-01 16:55) - PLID 34778 - UpdateOverpaymentLabel will check and see whether
	// the EOB, when fully posted, will potentially create a batch payment that is more
	// than the original EOB check amount was for. If so, the label will be updated.
	// (j.jones 2012-10-05 10:38) - PLID 52929 - renamed to reflect that this now handles underpayments too
	void UpdateOverpaymentUnderpaymentLabel();

	// (j.jones 2010-03-15 10:44) - PLID 32184 - added a local PeekAndPump function that
	// can optionally disable PeekAndPump usage for the posting process
	BOOL m_bEnablePeekAndPump;
	void PeekAndPump_EOBDlg();

	// (j.jones 2011-01-06 16:57) - PLID 41785 - added ability to create a billing note from patient
	// responsibility reasons & a patient responsibility amount
	void CreateBillingNoteFromPatientRespReasons(long nPatientID, long nChargeID, CString strInsuranceCoName,
											EOBClaimInfo *pClaim, EOBLineItemInfo *pCharge, COleCurrency cyPatResp);

	// (j.jones 2011-01-07 14:25) - PLID 41980 - added ability to create a billing note with detailed
	// patient resp. reasons and allowable information
	void CreateBillingNoteWithDetailedInfo(long nPatientID, long nChargeID, CString strInsuranceCoName,
											EOBClaimInfo *pClaim, EOBLineItemInfo *pCharge);

	//TES 9/18/2014 - PLID 62581 - Creates a note on the charge explaining why Alberta reduced a payment
	//TES 10/8/2014 - PLID 62581 - This creates a bill note now, so I removed ChargeID and added some information about the charge
	void CreateBillingNoteFromPartialPayment(long nPatientID, long nBillID, AlbertaAssessments::ChargeInfoPtr pCharge, const CString &strServiceCode, COleDateTime dtChargeDate, COleCurrency cyChargeAmount);
	//TES 9/18/2014 - PLID 62582 - Creates a note on the charge explaining why Alberta refused a claimm
	//TES 10/8/2014 - PLID 62582 - This creates a bill note now, so I removed ChargeID and added some information about the charge
	void CreateBillingNoteFromRefusal(long nPatientID, long nBillID, AlbertaAssessments::ChargeInfoPtr pCharge, const CString &strServiceCode, COleDateTime dtChargeDate, COleCurrency cyChargeAmount);
	//TES 9/26/2014 - PLID 62536 - Create a note when reversing a payment on a charge
	//TES 10/8/2014 - PLID 62536 - This creates a bill note now, so I removed ChargeID and added some information about the charge
	void CreateBillingNoteFromReversalReason(long nPatientID, long nBillID, AlbertaAssessments::ChargeInfoPtr pCharge, const CString &strServiceCode, COleDateTime dtChargeDate, COleCurrency cyChargeAmount);
	//TES 10/2/2014 - PLID 63821 - Sets the Bill Status Note to indicate that the bill is held for review, including any explanation codes
	void SetBillStatusNoteFromHoldReason(long nPatientID, long nBillID, AlbertaAssessments::ChargeInfoPtr pCharge);

	// (j.jones 2012-08-14 15:21) - PLID 50285 - moved CreateBillingNote to globalfinancialutils

	// (j.jones 2011-12-19 15:52) - PLID 43925 - added ability to save deductible and coinsurance information per charge
	void TrySaveCoinsurance(long nPatientID, CString strPatientName, long nChargeID, long nInsuredPartyID, EOBLineItemInfo *pCharge);

	// (j.jones 2011-03-16 10:21) - PLID 21559 - detects if any of our payments were under the allowed amount,
	// and offers to run the report to show them
	void CheckForPaymentsUnderAllowedAmount(long nEOBID);
	// (s.tullis 2016-04-15 16:00) - NX-100211 - changed return to row ptr
	// (j.jones 2011-06-02 09:48) - PLID 43931 - added FindChargeInList, which searches by ChargeID
	// or by charge pointer, and returns the row index, -1 if not found
	//TES 9/15/2014 - PLID 62777 - Added support for Alberta
	NXDATALIST2Lib::IRowSettingsPtr FindChargeInList(long nChargeID, EOBLineItemInfo *pCharge, OHIPEOBLineItemInfo *pOHIPCharge, AlbertaAssessments::ChargeInfoPtr pAlbertaCharge);
	// (s.tullis 2016-04-15 16:00) - NX-100211- changed return to row ptr
	// (j.jones 2012-04-18 16:44) - PLID 35306 - Added FindReversedSiblingRow, which searches by ChargeID/charge pointer,
	// and returns the row for the reversed charge.
	NXDATALIST2Lib::IRowSettingsPtr FindReversedSiblingRow(NXDATALIST2Lib::IRowSettingsPtr pSiblingRow, long nSiblingChargeID, const EOBClaimInfo *pSiblingClaim, const EOBLineItemInfo *pSiblingCharge);
	// (s.tullis 2016-04-15 16:00) - NX-100211 - change param to row ptr
	// (j.jones 2012-04-18 16:44) - PLID 35306 - Added ToggleSiblingRowSkipped, which will set the reversed sibling
	// of our given row ID to be skipped, per our passed-in parameter. Can also optionally color the row red.
	void ToggleSiblingRowSkipped(NXDATALIST2Lib::IRowSettingsPtr pSiblingRow, BOOL bSkipped, BOOL bColorRowRed = FALSE);

	// (j.dinatale 2013-01-10 08:46) - PLID 54544 - need a special function to determine if a charge is a reversal
	BOOL IsReversal(const EOBLineItemInfo *pCharge, const EOBClaimInfo *pClaim);

	// (j.jones 2012-04-19 15:38) - PLID 35306 - Returns TRUE if the claim is a reversal,
	// the and the payment/adjustment is negative and we could not match a payment/adjustment to void.
	BOOL IsUnpostableReversalCharge(const EOBLineItemInfo *pCharge, const EOBClaimInfo *pClaim);
	// (s.tullis 2016-04-15 16:00) - NX-100211- changed param to row ptr
	// (j.jones 2012-04-19 17:57) - PLID 35306 - Finds the given reversed RowID's sibling,
	// and returns the value of IsUnpostableReversalCharge for that sibling.
	BOOL HasUnpostableReversalSibling(NXDATALIST2Lib::IRowSettingsPtr pSiblingRow, long nSiblingChargeID, const EOBClaimInfo *pSiblingClaim, const EOBLineItemInfo *pSiblingCharge);

	// (j.jones 2012-04-24 08:47) - PLID 49846 - searches all currently loaded EOBs to see if a reversed payment ID already exists
	//TES 8/4/2014 - PLID 62580 - Added support for Alberta Assessment files
	BOOL IsReversedPaymentInUse(long nReversedPaymentID, bool bCheckAlberta);

	// (j.jones 2012-04-24 08:47) - PLID 49846 - searches all currently loaded EOBs to see if a reversed adjustment ID already exists
	BOOL IsReversedAdjustmentInUse(long nReversedAdjustmentID);

	// (j.jones 2012-05-02 10:09) - PLID 50138 - returns true if m_InsuranceCombo has no rows, or it has one
	// row and that row is one of our built-in "show all" / "show filtered" rows
	BOOL IsInsuranceComboEmpty();

	// (j.jones 2012-10-04 15:28) - PLID 52929 - added cache for the OHIPIgnoreMissingPatients preference
	BOOL m_bOHIPIgnoreMissingPatients;

	// (j.dinatale 2012-11-06 12:04) - PLID 50792 - Auto Unbatch MA18 claims
	// (j.dinatale 2012-12-19 11:22) - PLID 54256 - renamed to reflect N89
	BOOL m_bUnbatchMA18orN89;

	// (b.spivey, July 02, 2013) - PLID 56825 - create a bill note and apply it to a charge... maybe. 
	CString CreateBillingNoteFromAdjustmentReasons(long nPatientID, long nChargeID, EOBLineItemInfo *pCharge, bool bCreateOnCharge = false);

	NXDATALIST2Lib::IFormatSettingsPtr  GetInsuredPartyColumnCombo(long nPatientID);

	// (j.jones 2008-02-20 10:18) - PLID 29007 - added functions for manipulating the pay./adj. descriptions/categories
	// (j.jones 2008-06-16 17:24) - PLID 21921 - added OHIP support
	// (j.jones 2008-07-11 16:35) - PLID 28756 - added OnSelChosenEobInsuranceCombo
	// (j.jones 2008-11-24 09:36) - PLID 32075 - added OnBtnConfigEremitAdjCodesToSkip
	// (j.jones 2008-12-19 08:59) - PLID 32519 - added OnTimer
	// Generated message map functions
	//{{AFX_MSG(CEOBDlg)
	virtual BOOL OnInitDialog();	
	afx_msg void OnBtnImport835File();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPrintEob();
	afx_msg void OnEditPayDesc();
	afx_msg void OnEditAdjDesc();
	afx_msg void OnSelChosenPayDescriptionCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenAdjDescriptionCombo(LPDISPATCH lpRow);
	afx_msg void OnEditPayCat();
	afx_msg void OnEditAdjCat();
	afx_msg void OnBtnImportOhipFile();
	afx_msg void OnSelChosenEobInsuranceCombo(long nRow);
	// (r.gonet 2016-04-18) - NX-100162 - Renamed the function because this dialog is not
	// just for ignored adjustment codes anymore.
	afx_msg void OnBtnAdjustmentCodeSettings();
	afx_msg void OnTimer(UINT nIDEvent);
	// (r.farnworth 2014-01-28 15:53) - PLID 52596 - Revert the adjustment amount
	// (j.jones 2016-04-19 13:14) - NX-100161 - obsolete
	//afx_msg void OnRevertAdjustment();
	// (j.jones 2009-06-09 12:08) - PLID 33863 - this function will simply mark all "duplicate" charges as skipped
	afx_msg void OnBtnAutoSkipDuplicates();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
	// (j.jones 2010-02-08 15:33) - PLID 37174 - added ability to configure import filtering
	afx_msg void OnBtnConfigureEremitImportFiltering();
	// (j.jones 2010-04-09 13:03) - PLID 31309 - added date controls
	afx_msg void OnCheckEnablePayDate();
	afx_msg void OnCheckEnableAdjDate();
	// (j.jones 2011-06-15 17:50) - PLID 39747 - added OnRequeryFinished to add a show more/less row
	void OnRequeryFinishedEobInsuranceCombo(short nFlags);
	// (j.jones 2012-10-12 10:23) - PLID 53149 - added OnDestroy for cleanup
	afx_msg void OnDestroy();

	// (b.spivey, October 30, 2012) - PLID 49943 - Need to update bills as we process.
	void UpdateBillRefNumber(long nBillID, EOBClaimInfo* pClaim);
	// (d.singleton 2014-10-13 13:34) - PLID 62698 - New preference that will add the e-remit "claim number" to the description you see for the line items in the billing tab.
	void UpdatePaymentRefNumber(long nBillID, EOBClaimInfo* pClaim);
	// (b.spivey, November 07, 2012) - PLID 49943 - variable to cache in the dialog so this stays consistent. 
	long m_nEOBUpdateBillOriginaRefNo;
	// (s.tullis 2016-04-15 16:00) - NX-100211
	void EditingFinishingEobDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishedEobDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingStartingEobDetailList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);

	void RButtonDownEobDetailList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickEobDetailList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	CString GetCellInsuredPartyFormatSQL(long nPatientID);
	// (s.tullis 2016-04-20 9:48) - NX-100186 
	void EnsureWarnCorrectInsuredParties();
	// (s.tullis 2016-04-15 16:00) - NX-100299
	void TryUpdateInsuredParties(std::vector<long> &aryChargeIDs, const long &nInsuranceCoID);
	// (j.jones 2016-04-25 12:29) - NX-100299- added a query to use for payer ID matching,
	// returns all valid insured parties for a given charge & selected insurance company
	ADODB::_RecordsetPtr GetPayerIDMatchingRecordset(std::vector<long> &aryChargeIDs, const long &nInsuranceCoID);

	bool EnsureNoInvalidInsuredParties();

	// (j.jones 2016-04-26 14:08) - NX-100327 - added function to get the total adjustments on a charge,
	// which may skip secondary adjustments
	void GetTotalChargeAdjustments(NXDATALIST2Lib::IRowSettingsPtr pCurRow,
		OUT COleCurrency &cyTotalPositiveAdjustments,
		OUT COleCurrency &cyTotalNegativeAdjustments,
		OUT COleCurrency &cyTotalAdjustments);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EOBDLG_H__0B5007D3_CE13_4E84_B678_F8F2917E4851__INCLUDED_)

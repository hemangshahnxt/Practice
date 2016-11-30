#ifndef Practice_Global_Financial_Utilities_h
#define Practice_Global_Financial_Utilities_h

#pragma once



#include "SharedInsuranceUtils.h" //(e.lally 2010-02-18) PLID 37438

/*JMJ 2/13/2004 - Here are the SQL calculations for a Bill:

//For a Bill:

SELECT Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE ((100-Convert(float,[PercentOff]))/100) END)-(CASE WHEN [Discount] Is Null THEN 0 ELSE [Discount] END))) + 
(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE ((100-Convert(float,[PercentOff]))/100) END)-(CASE WHEN [Discount] Is Null THEN 0 ELSE [Discount] END))*(TaxRate-1)) + 
(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE ((100-Convert(float,[PercentOff]))/100) END)-(CASE WHEN [Discount] Is Null THEN 0 ELSE [Discount] END))*(TaxRate2-1)) 
)),2)) 
FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 
AND BillsT.ID = @BillID 


//For a Charge:

SELECT Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE ((100-Convert(float,[PercentOff]))/100) END)-(CASE WHEN [Discount] Is Null THEN 0 ELSE [Discount] END))) + 
(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE ((100-Convert(float,[PercentOff]))/100) END)-(CASE WHEN [Discount] Is Null THEN 0 ELSE [Discount] END))*(TaxRate-1)) + 
(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE ((100-Convert(float,[PercentOff]))/100) END)-(CASE WHEN [Discount] Is Null THEN 0 ELSE [Discount] END))*(TaxRate2-1)) 
)),2) 
FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
WHERE LineItemT.Deleted = 0 
AND ChargesT.ID = @ChargeID 

JMJ - in both cases, the calculation is as such:
Get the Amount * Quantity * Multipliers * PercentOff - Discount,
And then add the tax1 on that total, and then tax 2,
which means you'll do the base calculation three times

dbo.GetBillTotal() and dbo.GetChargeTotal() get the calculations using ChargeResps
so these queries are now used in very few places, mainly where we want to get the
pre-tax total or individual tax amounts. Still, we need to remember to follow this basic concept.
*/

// (j.jones 2010-04-07 17:58) - PLID 15224 - this enum is stored in ChargesT.IsEmergency,
// therefore you cannot change these values
enum ChargeIsEmergencyType {
	cietUseDefault = -1,
	cietBlank = 0,
	cietNo = 1,
	cietYes = 2,
};

// (j.jones 2010-06-10 09:53) - PLID 38507 - this enum is stored in BillsT.HCFABox13Over,
// do not change these, these are stored in data
enum HCFABox13Over {
	hb13_UseDefault = -1,
	hb13_No = 0,
	hb13_Yes = 1,
};

// (j.jones 2010-10-08 17:29) - PLID 40878 - enum for ANSI Version type,
// stored in data in EbillingFormatsT.ANSIVersion (therefore, you can't change these)
enum ANSIVersion {

	av4010 = 0,
	av5010 = 1,
};

// (j.jones 2010-12-27 16:12) - PLID 41852 - enum for checking the ability to override a hard close
// (j.jones 2011-01-19 12:06) - PLID 42149 - I moved the ECanOverrideHardClose enum to the header file
enum ECanOverrideHardClose {

	cohcNo = 0,
	cohcYes,
	cohcWithPass,
};

// (j.jones 2011-01-21 09:58) - PLID 42156 - added an enum for the billing dialog for whether the user can edit
// every field, some fields, or no fields
enum BillingAccessType {
	batFullAccess = 0,			//if they can edit every field (or might require a password for that field)
	batPartialAccess,			//if they can edit fields that aren't protected by a hard close
	batNoAccess,				//they can't edit any field
};

// (j.jones 2012-01-03 15:08) - PLID 47300 - added NOC enums, for CPTCodeT.NOCType
enum NOCTypes {
	noctDefault = -1,
	noctNo = 0,
	noctYes = 1,
};

// (j.jones 2014-04-23 13:53) - PLID 61840 - Added ANSI Hide 2310A settings,
// only used in the HCFA setup. These are stored in data and cannot be renumbered.
enum ANSI_Hide2310AOptions {
	hide2310A_Never = 0,
	hide2310A_When2420E = 1,
	hide2310A_When2420F = 2,
	hide2310A_When2420EorF = 3,
};

// (j.jones 2014-06-26 17:23) - PLID 62546 - added PayType enum for BatchPaymentsT.PayType,
// cannot be changed as this is stored in data
enum EBatchPaymentPayType {
	eNoPayment = 0,			//only used for adjustments and refunds, not real payments
	eMedicalPayment = 1,	//a normal batch payment
	eVisionPayment = 2,		//a vision batch payment
};

// (j.dinatale 2011-08-26 09:15) - PLID 44810 - finally added this because.... well it would make sense to have it, now wouldn't it?
namespace LineItem
{
	enum Type {
		// (r.gonet 2015-04-20) - PLID 65326 - Added Invalid, which is a sentinel value, not actually saved to the database.
		Invalid = -1,
		Payment = 1,
		Adjustment = 2,
		Refund = 3,
		Charge = 10,
		QuoteCharge = 11,
	};

	// (j.jones 2015-09-30 08:58) - PLID 67157 - added enum for PaymentsT.CCProcessType,
	// this is saved to data, the numbers cannot be changed
	enum CCProcessType {
		None = 0,				//in data, this is NULL, only used for non-CC payments/refunds/adjustments
		Swipe = 1,				//Swipe/Dip a card
		CardNotPresent = 2,		//Card Not Present (card on file)
		DoNotProcess = 3,		//Other (Do Not Process)
		RefundToOriginal = 4,	//Refund to original card (for refunds only)
	};
} //namespace LineItem

// (r.wilson 2012-07-23) PLID 50488 - Enum of Categories for Pay Groups
namespace PayGroupCategory
{
	enum Category
	{
		NoCategory = -1,
		OfficeVisit = 1,
		SurgicalCode = 2,
		DiagnosticCode = 3,
	};
}

// (j.dinatale 2012-11-06 15:36) - PLID 50792
namespace BatchChange{
	enum Status{
		None = 0,
		Batched = 1,
		Unbatched = 2,
	};
};

// (r.gonet 06/30/2014) - PLID 62533 - Added enumeration for Bill Status types. BillStatusT.Type
// Aside from EBillStatusType::None, this is saved to data and cannot be changed.
enum class EBillStatusType {
	None = -1,
	OnHold = 1,
};

// (r.gonet 2015-04-16) - PLID 65326 - Added an enum for the magic numbers in PaymentsT.PayMethod. Still ugly but at least its easier to understand.
// WARNING: This is saved to data. If you insert a new method in the middle of the enum list, it will screw things up for PaymentsT.PayMethod 
// unless you create a mod to fix the data.
// Note: If you add to this enumeration, you also need to add the new enum value to IsValidPayMethod and GetLineItemTypeFromPayMethod
enum class EPayMethod {
	Invalid = -1,
	Adjustment = 0,
	CashPayment = 1,
	CheckPayment = 2,
	ChargePayment = 3,
	GiftCertificatePayment = 4,
	// (r.gonet 2015-04-16) - PLID 65326 - j.jones says 5 never existed. I can find no record of it existing for at least the history of source control.
	LegacyAdjustment = 6, // (r.gonet 2015-04-16) - This is messed up. Right before we save the PayMethod, we change 6 into 0. I can't tell you how old this is because it is before source control.
	CashRefund = 7,
	CheckRefund = 8,
	ChargeRefund = 9,
	GiftCertificateRefund = 10
};
// (r.gonet 2015-04-20) - PLID 65326 - Returns true if nPayMethod is a valid pay method value. false otherwise.
bool IsValidPayMethod(long nPayMethod);
// (r.gonet 2015-04-20) - PLID 65326 - Returns the nPayMethod casted as an EPayMethod or returns a default if the nPayMethod is an invalid pay method.
// Note that this changes the legacy adjustments into adjustments behind the scenes.
EPayMethod AsPayMethod(long nPayMethod, EPayMethod eDefaultValue = EPayMethod::Invalid);
// (r.gonet 2015-04-20) - PLID 65326 - Gets whether a paymethod is a Payment, an Adjustment, or a Refund. Returns LineItem::Type::Invalid if the paymethod is not not handled.
LineItem::Type GetLineItemTypeFromPayMethod(EPayMethod e);


// Globally used financial functions
void RoundCurrency(COleCurrency& cy);

// (j.jones 2010-05-17 17:02) - PLID 16503 - added bAlwaysAllowZeroDollarApply
// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables,
// for when the apply is part of an E-Remittance posting
// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
// (r.gonet 2015-02-11 13:14) - PLID 64852 - Changed the default argument of bWarnLocationPatientResp, which was added in the lockbox project from 11500, to TRUE. 
//  This matches the comment in the function definition and corrects a bug where applying payments to bills with differing locations wasn't respecting the 
//  "When applying a payment to a charge of another location" preference.
BOOL ApplyPayToBill(long PayID, long PatientID, COleCurrency PayAmount, CString ClickedType, long BillID, long nInsuredPartyID = -1, 
					BOOL PointsToPayments = FALSE, BOOL bShiftToPatient = FALSE, BOOL bAdjustBalance = FALSE, BOOL bPromptToShift = TRUE, BOOL bWarnLocation = TRUE,
					BOOL bAlwaysAllowZeroDollarApply = FALSE, BOOL bDisableAllowableCheck = FALSE, BOOL bIsERemitPosting = FALSE, BOOL bWarnLocationPatientResp = TRUE);
//Depreciated function
// (j.jones 2009-10-06 12:03) - PLID 27713 - removed the old, unused function
//void ApplyInsurancePayToBill(long PayID, long PatientID, COleCurrency PayAmount, CString ClickedType, long BillID, long nInsuredPartyID, BOOL bShiftToPatient = FALSE, BOOL bAdjustBalance = FALSE, BOOL bPromptToShift = TRUE, BOOL bWarnLocation = TRUE);
// (j.jones 2010-05-17 17:02) - PLID 16503 - added bAlwaysAllowZeroDollarApply
// (j.jones 2010-06-02 17:44) - PLID 37200 - renamed to not be insurance-specific, and added two insured party IDs
// (j.jones 2011-03-23 10:49) - PLID 42936 - added bDisableAllowableCheck
void ApplyPayToBillWithRevenueCode(long PayID, long PatientID, COleCurrency PayAmount, long BillID, long RevenueCodeID, long nInsuredPartyIDToPost, long nInsuredPartyIDForRevCode, BOOL bShiftToPatient = FALSE, BOOL bAdjustBalance = FALSE, BOOL bPromptToShift = TRUE, BOOL bWarnLocation = TRUE,
											BOOL bAlwaysAllowZeroDollarApply = FALSE, BOOL bDisableAllowableCheck = TRUE);

// (j.jones 2006-12-29 11:28) - PLID 23160 - added support for revenue codes
// (j.jones 2013-07-22 11:43) - PLID 57653 - added the payment ID we applied
void PostInsuranceApply(long nPaymentID, long nInsuredPartyID, long nPatientID,
						long nRecordID, CString strClickedType, long nRevenueCodeID = -1);

// (j.jones 2011-03-16 09:23) - PLID 21559 - added override to not warn for allowables,
// for when the apply is part of an E-Remittance posting
// (j.jones 2011-03-21 17:40) - PLID 24273 - added flag to indicate that this was caused by an EOB
void AutoApplyPayToBill(long PayID, long PatientID, CString ClickedType, long BillID, BOOL bShiftToPatient = FALSE, BOOL bAdjustBalance = FALSE,
						BOOL bPromptToShift = TRUE, BOOL bWarnLocation = TRUE, BOOL bDisableAllowableCheck = FALSE, BOOL bIsERemitPosting = FALSE);
void AutoApplyPayToPay(long SrcPayID, long PatientID, CString ClickedType, long DestPayID);

// (j.jones 2006-12-29 09:39) - PLID 23160 - altered shifting functions to support revenue code
// (j.jones 2013-08-21 08:44) - PLID 58194 - added a required audit string so auditing the shift more clearly states what process shifted and why
void ShiftInsBalance(int ID, long PatientID, int SrcInsPartyID, int DstInsPartyID, CString strLineType, CString strAuditFromProcess, bool bShowDlg = true, long nRevenueCode = -1);

// (j.jones 2007-08-03 09:58) - PLID 25844 - Added flag to follow the "prompt for CoPay" setting on each CPT code
// (j.jones 2010-08-03 15:16) - PLID 39938 - replaced the "prompt for copay" flag with a PayGroupID
// (j.jones 2011-03-21 17:46) - PLID 24273 - added flag to indicate that this was caused by an EOB
// (j.armen 2013-06-29 10:14) - PLID 57384 - Idenitate ChargeRespDetailT - Changed some of the params to long
// (j.jones 2013-08-21 08:44) - PLID 58194 - added a required audit string so auditing the shift more clearly states what process shifted and why
void ShiftInsuranceResponsibility(int ID, long PatientID, long nSrcInsPartyID, long nDstInsPartyID, const CString& strLineType, COleCurrency cyAmtToShift,
								  CString strAuditFromProcess, COleDateTime dtDateOfShift,
								  long nRevenueCode = -1, long nOnlyShiftPayGroupID = -1, BOOL bIsERemitPosting = FALSE);

// (j.jones 2013-08-20 12:13) - PLID 39987 - Added EnsureInsuranceResponsibility, which shifts as needed
// from the source resp. to make the dest. resp. equal to the desired responsibility.
// It may only shift a portion of the money, or perhaps none at all if the resp. is <= what we are wanting.
//
// If the dest. resp is greater than our desired amount, we do nothing unless bForceExactResp is true.
// If bForceExactResp is enabled, we would shift any overage from the dest. resp. back to the source resp.
// to ensure the dest. resp. is == to our desired amount.
//
// Returns true if the responsibility could be successfully acquired (or if nothing changed because it was already available),
// returns false if we could not acquire the desired responsibility, likely due to existing applies that prevented shifting.

// (j.jones 2013-08-21 08:44) - PLID 58194 - added a required audit string so auditing the shift more clearly states what process shifted and why
bool EnsureInsuranceResponsibility(int ID, long PatientID, long nSrcInsPartyID, long nDstInsPartyID, const CString& strLineType, COleCurrency cyDesiredRespAmount,
								   CString strAuditFromProcess, COleDateTime dtDateOfShift,
								   bool bForceExactResp = false, long nRevenueCode = -1,
								   long nOnlyShiftPayGroupID = -1, BOOL bIsERemitPosting = FALSE);

BOOL IncreaseInsBalance(long ChargeID, long PatientID, long InsID, COleCurrency cyAmountToIncrease);
// (j.jones 2008-04-29 09:45) - PLID 29744 - renamed Responsibility parameter to nRespTypeID
COleCurrency AdjustBalance(long nID, long nBillID, long nPatientID, int iLineItemType, long nRespTypeID, long nInsuredPartyID);
void SwapInsuranceCompanies(long nBillID, long nSrcInsPartyID, long nDstInsPartyID, BOOL bShowWarnings = TRUE);

// (j.jones 2012-08-23 12:42) - PLID 42438 - added silent option
BOOL CheckUnbatchClaim(long nBillID, BOOL bSilent = FALSE);

BOOL GetBillTotals(int iBillID, long PatientID, COleCurrency *cyCharges, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency* cyRefunds, COleCurrency *cyInsResp);
// (j.jones 2011-03-08 10:48) - PLID 41877 - renamed the pay/adj/ref parameters to reflect what they really mean
BOOL GetChargeTotals(int iChargeID, long PatientID, COleCurrency *cyCharges, COleCurrency *cyPatientPayments, COleCurrency *cyPatientAdjustments, COleCurrency *cyPatientRefunds, COleCurrency *cyInsResp);
// (j.jones 2009-10-06 17:30) - PLID 36425 - this function is not currently used
//BOOL GetChargeRespTotals(int iChargeID, long PatientID, COleCurrency *cyCharges, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency *cyRefunds, COleCurrency *cyInsResp);

// (j.jones 2013-03-25 17:45) - PLID 55686 - added function to get the insured party ID with the highest balance
// on this bill, will return -1 if it is the patient, also will return -1 if the highest balance is zero
long GetBillInsuredPartyIDWithHighestBalance(long nBillID);
// (j.jones 2013-03-25 17:45) - PLID 55686 - added function to get the insured party ID with the highest balance
// on this charge, will return -1 if it is the patient, also will return -1 if the highest balance is zero
long GetChargeInsuredPartyIDWithHighestBalance(long nChargeID);

// (b.cardillo 2011-06-17 16:30) - PLID 39176 - We now offer a more efficient version of this function for callers who already know the insured party id
// (j.jones 2011-07-22 12:07) - PLID 42231 - removed the old GetBillInsuranceTotals, in favor of GetBillInsuranceTotals_InsParty, which I then renamed to the old name
BOOL GetBillInsuranceTotals(int iBillID, long PatientID, int nInsPartyID, COleCurrency *cyTotalResp, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency* cyRefunds);
BOOL GetChargeInsuranceTotals(int iChargeID, long PatientID, int nInsPartyID, COleCurrency *cyTotalResp, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency *cyRefunds);
// (j.jones 2009-10-06 17:30) - PLID 36426 - this function is not currently used
//BOOL GetChargeRespInsuranceTotals(int iChargeID, long PatientID, int iInsType, COleCurrency *cyTotalResp, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency *cyRefunds);
BOOL GetInactiveInsTotals(int InsuredPartyID, int BillID, int ChargeID, long PatientID, COleCurrency &cyResp, COleCurrency &cyApplies);

// (j.jones 2006-12-28 14:00) - PLID 23160 - added Revenue Code totals functions
BOOL GetRevenueCodeInsuranceTotals(long nRevenueCodeID, long nBillID, long nPatientID, long nInsuredPartyID, COleCurrency *cyTotalResp, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency* cyRefunds);
BOOL GetRevenueCodeInactiveInsTotals(int InsuredPartyID, long nBillID, long nRevenueCodeID, long PatientID, COleCurrency &cyResp, COleCurrency &cyApplies);
BOOL GetRevenueCodeTotals(long nRevenueCodeID, long nBillID, long PatientID, COleCurrency *cyCharges, COleCurrency *cyPayments, COleCurrency *cyAdjustments, COleCurrency *cyRefunds, COleCurrency *cyInsResp);

// (j.jones 2011-11-16 13:44) - PLID 46348 - added GetBillPatientBalance
COleCurrency GetBillPatientBalance(long nBillID, long nPatientID);

COleCurrency GetChargePatientResp(long nChargeID);
COleCurrency GetChargePatientBalance(long nChargeID, long nPatientID);
COleCurrency GetChargeInsResp(long nChargeID, long nInsuredPartyID);
COleCurrency GetChargeInsBalance(long nChargeID, long nPatientID, long nInsuredPartyID);
// (j.jones 2009-10-06 08:59) - PLID 21304 - added GetChargeTotalBalance
COleCurrency GetChargeTotalBalance(long nChargeID);

// (j.jones 2011-03-08 10:56) - PLID 41877 - added GetChargePatientRespAndInsBalance,
// which should really only be used in E-Remittance, it returns the total patient responsibility
// plus the total insurance balance for all insurance responsibilities
// (j.jones 2011-11-08 16:41) - PLID 46240 - this has been renamed, its purpose is now
// to find the charge's patient resp., balance for the given insured party, and responsibility
// for all other insured parties
COleCurrency GetChargeInsBalanceAndOtherResps(long nChargeID, long nInsuredPartyID);

// (j.jones 2011-11-08 16:41) - PLID 46240 - this finds the charge's patient resp. and responsibility
// for all other insured parties, completely ignoring the given insured party
COleCurrency GetChargePatientAndOtherRespTotals(long nChargeID, long nInsuredPartyID);

BOOL GetPatientTotal(long PatientID, COleCurrency *cyTotal);
BOOL GetPatientInsuranceTotal(long PatientID, COleCurrency *cyTotal);

BOOL GetPayAdjRefTotals(int iID, long PatientID, COleCurrency &cyInitialAmount, COleCurrency &cyOutgoingApplies, COleCurrency &cyIncomingApplies);

//(e.lally 2007-04-04) PLID 25487 - Added an optional BOOL flag for deleting charges.
BOOL DeleteBill(int iBillID, BOOL bDeleteCharges = TRUE); // Returns TRUE upon success
BOOL DeleteCharge(long iChargeID, long iBillID, BOOL boUpdateLineIDs, BOOL boDeleteBill = TRUE);
// (j.gruber 2007-07-31 16:26) - PLID 15416 - need to allow deleting a payment that is being voided
// (d.thompson 2009-04-16) - PLID 34004 - renamed BOOL parameter to clarify it is Paymentech only
// (z.manning 2016-02-15 15:08) - PLID 68258 - Changed return type to boolean
BOOL DeletePayment(int iPaymentID, BOOL bPAYMENTECHForceCCDelete  = FALSE);
// (j.jones 2011-03-21 17:10) - PLID 24273 - added a bool that tells auditing that an EOB posting caused the unapply
BOOL DeleteApply(long nApplyID, BOOL bWarn = TRUE, BOOL bUnappliedDuringEOBPosting = FALSE);
BOOL DeleteGiftCertificate(long nID, BOOL bIgnoreCharges = FALSE);

// (j.jones 2008-05-27 16:17) - PLID 27982 - this function should be called before
// deleting a bill or charge, and will warn if the bill or charge has serialized items,
// returns true/false if the user wishes to continue
// (j.jones 2013-07-23 10:01) - PLID 46493 - added flag for voiding, only needed to be used
// if we're only voiding a bill or charge, it is not needed if we're voiding & correcting
bool CheckWarnDeletingChargedProductItems(bool bIsBill, bool bIsVoiding, long nID);

// (j.jones 2008-06-09 16:32) - PLID 28392 - This function should be called before
// deleting a bill or charge, and will warn if the bill or charge has products that
// have been returned. Returns true/false if the user wishes to continue.
// If bIsEditingCharge is true, it means they are trying to edit a quantity or serialized item,
// and not deleting, so the warnings need tweaked accordingly
// (j.jones 2013-07-23 10:01) - PLID 46493 - added flag for voiding, only needed to be used
// if we're only voiding a bill or charge, it is not needed if we're voiding & correcting
bool CheckWarnAlteringReturnedProducts(bool bIsBill, bool bIsVoiding, long nID, bool bIsEditingCharge = false);

COleCurrency UnapplyFromBill(long nBillID, long nInsuredPartyID, BOOL bInsuranceApplies, COleCurrency cySumOfDesiredUnapplies);

// (j.jones 2010-06-02 16:40) - PLID 37200 - used in UnapplyFromCharge to tell the caller
// how much was unapplied from each PaymentID
struct UnappliedAmount {

	long nPaymentID;
	COleCurrency cyAmtUnapplied;
};

// (j.jones 2010-06-02 16:20) - PLID 37200 - added optional array of affected PaymentIDs
// (j.jones 2011-03-21 17:10) - PLID 24273 - added a bool that tells auditing that an EOB posting caused the unapply
COleCurrency UnapplyFromCharge(long nChargeID, long nInsuredPartyID, BOOL bInsuranceApplies, COleCurrency cyAmountToUnapply, CArray<UnappliedAmount, UnappliedAmount> *paryUnappliedPaymentIDs = NULL, BOOL bUnappliedDuringEOBPosting = FALSE);

// (j.jones 2007-01-04 09:35) - PLID 24030 - added revenue code support
// (j.jones 2010-06-02 16:20) - PLID 37200 - added optional array of affected PaymentIDs
COleCurrency UnapplyFromRevenueCode(long nBillID, long nRevenueCodeID, long nInsuredPartyID, BOOL bInsuranceApplies, COleCurrency cySumOfDesiredUnapplies, CArray<UnappliedAmount, UnappliedAmount> *paryUnappliedPaymentIDs = NULL);

// (j.jones 2014-01-14 15:51) - PLID 58726 - Given a correction ID for a payment,
// this function will copy all the applies from the original payment to the new payment,
// effectively reapplying the new payment identically to the old payment.
// If nSkipChargeID is filled, applies to that charge will not be copied.
// (j.jones 2015-02-19 15:14) - PLID 64938 - this is obsolete, corrections now automatically do this
//void CopyOriginalPaymentApplies(long nLineItemCorrectionID, bool bIsERemitPosting, long nSkipChargeID = -1);

// (j.jones 2008-06-17 09:03) - PLID 30410 - removed HCFAT from the program
//void ChargeToHCFA (long iBillID, long charge, bool Add);// (j.jones 2015-02-19 16:43) - PLID 

// (j.jones 2008-02-11 16:58) - PLID 28847 - added bSkipRespCheck to save a recordset if
// we already validated that the bill is ok prior to calling BatchBill
// (j.dinatale 2012-11-06 14:21) - PLID 50792 - return if BatchBill was successful or not
BatchChange::Status BatchBill(int iBillID, int iBatch, BOOL bSkipRespCheck = FALSE);
int FindHCFABatch(int iBillID);
int FindDefaultHCFABatch(long InsuredPartyID);
int FindDefaultUB92Batch(long InsuredPartyID);

int GetFormTypeFromBillID(long iBillID);
// (s.tullis 2016-02-24 16:48) - PLID 68319
int GetFormTypeFromLocationInsuranceSetup(long nInsuredpartyID, long nLocationID);
void UpdateBillClaimForm(long nBillID);

long GetInsuranceCoID(long InsuredPartyID);
int GetInsuranceCoTaxType(long InsuredPartyID);

long GetBillLocation(long BillID);
// (j.jones 2009-10-06 17:30) - PLID 36425 - this function is not currently used
//long GetBillPOS(long BillID);

// (c.haag 2009-03-10 12:33) - PLID 33431 - Use CString, not LPCTSTR.
BOOL CanEdit (const CString& str, long ID);

void CopyQuote(long QuoteID);

void DateToFieldString(CString& str);

/// <summary>
/// Calculates the total balance of available prepayments linked to a given quote.
/// Enabling bIncludePrepaysWithNoQuotes will also include prepayments not linked to any quote.
/// </summary>
COleCurrency CalculatePrePayments(long nPatientID, long nQuoteID, bool bIncludePrepaysWithNoQuotes);

/// <summary>
/// Calculates the total balance of available prepayments on the patient's account.
/// The optional filter can be used as a where clause on LineItemT or PaymentsT.
/// </summary>
COleCurrency CalculatePrePayments(long nPatientID, CSqlFragment sqlFilter);

COleCurrency CalculateRemainingBatchPaymentBalance(long BatchPayID);

BOOL AllowPaymentApply(long nPaymentID, long nDestID, CString strClickedType);

BOOL InvokeFinancialApplyDlg(int iDestID, int iSourceID, long PatientID, CString strClickedType);

BOOL IsZeroDollarPayment(long nPaymentID);

// 1 = Primary, 2 = Secondary, 3 = Tertiary
int GetInsuranceTypeFromID(int InsuredPartyID);
int GetInsuranceIDFromType(int PatientID, int iType);

// (j.jones 2009-10-23 10:22) - PLID 18558 - this requires the Place Of Service ID as well now
void WarnAllowedAmount(long ServiceID, long InsCoID, long ProviderID, long LocationID, long nPlaceOfServiceID, long nInsuredPartyID, long nChargeID, COleCurrency cyApplyAmt);

COleCurrency CalculateTax(COleCurrency cyPreTax, double dblTax, BOOL bReturnTotalVal = FALSE);
COleCurrency CalculateAmtQuantity(COleCurrency cyAmount, double dblQuantity);

// (j.jones 2010-08-04 12:29) - PLID 38613 - added a generic function to calculate percentages of currencies
// (j.jones 2012-07-30 14:38) - PLID 47778 - moved from billing to global financial utils
COleCurrency CalculatePercentOfAmount(COleCurrency cyAmount, long nPercent);

void WriteHCFAToHistoricTable(int iBillID, int iPatientID);
void CloseInvisibleNxControls();

CString GetNameFromRespTypeID(long nTypeID);
CString GetNameFromRespPartyID(long nInsPartyID);

// (j.jones 2009-10-06 12:22) - PLID 36426 - changed strRespID to nRespID
// (j.jones 2010-05-17 17:02) - PLID 16503 - added bAllowZeroDollarApply
void ApplyToDetails(long nChargeID, long nRespID, COleCurrency cyAmtToApply, long nApplyID, long nPayID, BOOL bAllowZeroDollarApply);

// (a.walling 2008-07-07 18:02) - PLID 29900 - Added patientID
BOOL CheckWarnGlobalPeriod(long nPatientID, COleDateTime dtToCheck = COleDateTime::GetCurrentTime(), BOOL bUseIgnoreDate = FALSE, COleDateTime dtToIgnore = COleDateTime::GetCurrentTime());

// (j.jones 2011-04-05 13:38) - PLID 42372 - added a struct to return more data in UseCLIANumber
struct ECLIANumber {

	BOOL bUseCLIANumber;
	CString strCLIANumber;
	BOOL bUseCLIAInHCFABox23;
	BOOL bUseCLIAInHCFABox32;
	BOOL bCLIAUseBillProvInHCFABox17;
};

// (j.jones 2009-02-06 16:25) - PLID 32951 - now this function requires the bill location
// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID, though
// in most cases they will be the same person
// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
ECLIANumber UseCLIANumber(long nBillID, long nInsuredPartyID, long nBillingProviderID, long nRenderingProviderID, long nLocationID);

CString GetDefaultBillingColumnWidths();
// (j.jones 2009-12-23 09:19) - PLID 32587 - added bShowInitialValue
CString GetDefaultQuoteColumnWidths(BOOL bIsMultiUsePackage, BOOL bShowInitialValue);

// (j.jones 2010-01-18 13:56) - PLID 36913 - added bForceTimePrompt, which will prompt for times
// even if we already have them, incase we provided defaults that we wish the user to confirm
BOOL CheckAnesthesia(long nServiceID, BOOL &bAnesthesia, COleCurrency &cyAnesthUnitCost, double &dblAnesthUnits, long &nAnesthMinutes, CString &strStartTime, CString &strEndTime, long nPlaceOfServiceID, BOOL bRoundUp = TRUE, BOOL bForceTimePrompt = FALSE);
void CalcAnesthesia(long nServiceID, COleCurrency &cyAnesthUnitCost, double &dblAnesthUnits, long nTotalAnesthMinutes, long nPlaceOfServiceID, BOOL bRoundUp = TRUE);

// (j.jones 2010-01-18 13:56) - PLID 36913 - added bForceTimePrompt, which will prompt for times
// even if we already have them, incase we provided defaults that we wish the user to confirm
BOOL CheckFacilityFee(long nServiceID, BOOL &bFacilityFee, COleCurrency &cyFacilityUnitCost, long &nFacilityMinutes, CString &strStartTime, CString &strEndTime, long nPlaceOfServiceID, BOOL bForceTimePrompt = FALSE);
void CalcFacilityFee(long nServiceID, COleCurrency &cyFacilityUnitCost, long nTotalFacilityMinutes, long nPlaceOfServiceID);

// (j.jones 2010-11-22 16:18) - PLID 39602 - added support for billing Assisting Codes, which work similarly to anesthesia & facility codes
// (j.jones 2011-10-31 16:43) - PLID 41558 - added minutes & start time as parameters
BOOL CheckAssistingCode(IN long nServiceID, OUT BOOL &bAssistingCode, OUT COleCurrency &cyAssistingCodeUnitCost, long &nMinutes, CString &strStartTime, CString &strEndTime);
void CalcAssistingCode(IN long nServiceID, IN long nTotalAssistingMinutes, IN long nBaseUnits, OUT COleCurrency &cyAssistingCodeUnitCost);

BOOL CheckProcedureDiscount(long nServiceID, long nPatientID, long &nPercentOff, COleDateTime dtBillDate, long nBillID);

void CalculateFinanceCharges();
// (j.jones 2013-08-01 14:19) - PLID 53317 - added ability to undo finance charges
void UndoFinanceCharges();

void AutoUpdateBatchPaymentDepositDates(long nBatchPaymentID);

COleCurrency CalculateTotalPackageValueWithTax(long nQuoteID);
// (j.jones 2007-04-23 13:03) - PLID 25735 - the Original package value
// may not equal the "Total" package value, as it is based off of the original "current amount"
// (j.gruber 2007-08-15 11:46) - PLID 25851 - changed function to allow just getting tax1 or tax2
COleCurrency CalculateOriginalPackageValueWithTax(long nQuoteID, long nTaxRateToCalculate = -1);

/// <summary>
/// Calculates the remaining balance of a package with and without tax,
/// taking into account any prepayments linked to the package, or payments
/// applied to billed uses of the package.
/// 
/// varOriginalCurrentAmount is PackagesT.OriginalCurrentAmount and should be filled
/// if the caller has access to it. Should only be null if it is not known.
/// </summary>
void CalculateRemainingPackageBalance(long nQuoteID, long nPatientID, const _variant_t varOriginalCurrentAmount, COleCurrency &cyRemBalanceNoTax, COleCurrency &cyRemBalanceWithTax);

/// <summary>
/// Calculates the remaining balance of a package only without tax,
/// taking into account any prepayments linked to the package, or payments
/// applied to billed uses of the package.
/// 
/// varOriginalCurrentAmount is PackagesT.OriginalCurrentAmount and should be filled
/// if the caller has access to it. Should only be null if it is not known.
/// </summary>
void CalculateRemainingPackageBalance(long nQuoteID, long nPatientID, const _variant_t varOriginalCurrentAmount, COleCurrency &cyRemBalanceNoTax);

/// <summary>
/// Calculates the remaining balance of a package, taking into account any prepayments
/// linked to the package, or payments applied to billed uses of the package.
/// Will not calculate the 'With Tax' variable if bCalculateValueWithTax is set to false.
/// 
/// varOriginalCurrentAmount is PackagesT.OriginalCurrentAmount and should be filled
/// if the caller has access to it. Should only be null if it is not known.
/// </summary>
void CalculateRemainingPackageBalance(long nQuoteID, long nPatientID, const _variant_t varOriginalCurrentAmount, COleCurrency &cyRemBalanceNoTax, bool bCalculateValueWithTax, COleCurrency &cyRemBalanceWithTax);

COleCurrency CalculateRemainingPackageValueWithTax(long nQuoteID);

// (j.jones 2007-03-26 14:45) - PLID 25287 - calculate any "overage" that may be required
// for a multi-use package to balance out with the package total
COleCurrency CalculateMultiUsePackageOverageAmount_ForTotal(long nQuoteID);
COleCurrency CalculateMultiUsePackageOverageAmount_ForBalance(long nQuoteID);
// (j.jones 2007-03-26 14:45) - PLID 25287 - calculate the target ServiceID that an overage
// would be applied to
// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
long CalculateMultiUsePackageOverageChargeID(long nQuoteID);

CString GetAnesthStartTimeFromBill(long nBillID);
CString GetAnesthEndTimeFromBill(long nBillID);
long GetAnesthMinutesFromBill(long nBillID);
CString CalculateAnesthesiaNoteForHCFA(long nBillID);
// (j.jones 2011-07-06 13:59) - PLID 44327 - added CalculateAnesthesiaNoteForANSI
CString CalculateAnesthesiaNoteForANSI(long nBillID);

// (a.walling 2006-11-15 09:42) - PLID 23550 - Generic datalist column enum for reason and group code lists
// (j.jones 2010-09-23 12:41) - PLID 40653 - obsolete now
/*
enum EGenericCodeDescriptionColumns {
	egcCode = 0,
	egcDescription
};
*/

//(e.lally 2007-007-09) PLID 26590 - We need to define an enum for the type of "credit card", used by each record in
	//CreditCardNamesT. This will be used for the credit card processing as defined by Paymentech
	//Since these values will be stored in data, be sure to only add to the bottom of the list.
// (d.thompson 2009-04-09) - PLID 33935 - Renamed to PAYMENTECH_
enum PAYMENTECH_ProcessingCardTypes{
	pctCredit = 1, //Credit cards
	pctUSDebit, //U.S. debit cards
	pctInterac, //Canadian debit cards
	pctAmex, //American Express cards
	pctVisa, //Visa Cards
};

// (j.gruber 2007-07-11 14:53) - PLID 15416 - CC Processing
enum SwipeType {
	swtMSRDevice,
	swtPinPad
};

//(e.lally 2007-10-30) PLID 27892 - Global function to take a credit card number and return the
// XXXXXXXXXXXX#### format of the number for privacy (pound signs being last 4 digits).
//(e.lally 2007-12-11) PLID 28325 - Renamed function from "Privatize" to "Mask" 
CString MaskCCNumber(IN const CString& strFullCCNumber);

// (j.jones 2008-02-11 15:26) - PLID 28847 - added function to check whether we can create a claim on a bill
// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created, and takes in
// bSilent to disable that warning
BOOL CanCreateInsuranceClaim(long nBillID, BOOL bSilent);
// (j.jones 2008-02-12 09:04) - PLID 28848 - added function to ensure that patient-only charges are not batched
void EnsurePatientChargesUnbatched(long nBillID);
// (j.jones 2008-02-12 15:37) - PLID 28848 - added function to ensure that patient-only charges are batched when
// shifted to insurance
void EnsurePatientChargesBatchedUponShift(long nChargeID);

// (j.jones 2008-06-24 09:20) - PLID 30455 - this function returns a query that is used
// both in the bill's appointment dropdown, and also used to validate that a given appointment
// will exist in that dropdown
CString GetBillAppointmentQuery();

// (j.jones 2008-09-09 09:08) - PLID 18695 - this function serves as a global lookup
// for the proper ANSI code for a given InsuranceTypeCode
// (j.jones 2010-10-15 14:29) - PLID 40953 - split into 4010 and 5010 usage
CString GetANSI4010_SBR09CodeFromInsuranceType(InsuranceTypeCode eCode);
CString GetANSI5010_SBR09CodeFromInsuranceType(InsuranceTypeCode eCode);
// (j.jones 2008-09-09 10:20) - PLID 18695 - this function serves as a global lookup
// for the proper NSF code for a given InsuranceTypeCode
CString GetNSFCodeFromInsuranceType(InsuranceTypeCode eCode);
// (r.gonet 12/03/2012) - PLID 53798 - Moved GetNameFromInsuranceType to NxPracticeSharedLib/SharedInsuranceUtils.h
// (j.jones 2009-01-16 12:24) - PLID 32762 - added GetANSI_2320SBR05_CodeFromInsuranceType
// because it has a different code list than the SBR09 codes
// (j.jones 2010-10-15 15:12) - PLID 40953 - renamed to reflect that this is 4010-only
CString GetANSI4010_2320SBR05_CodeFromInsuranceType(InsuranceTypeCode eCode);

// (c.haag 2009-03-10 09:11) - PLID 32433 - This function returns TRUE if a user is allowed to
// edit a bill, charge, payment, adjustment, refund or batch payment given that item's service
// date, the practice's preferences, and user permissions.
// (d.thompson 2009-09-02) - PLID 34694 - Added suppression of messages
// (j.jones 2011-01-21 09:26) - PLID 42156 - renamed the message suppression to bSilent, because it needs to also suppress password prompts,
// and also added a string for the billing dlg Edit button to use
// (r.gonet 07/09/2014) - PLID 62571 - Added an option to ignore financial closes if they are in effect.
BOOL CanChangeHistoricFinancial(const CString& strType, long ID, EBuiltInObjectIDs bio, ESecurityPermissionType perm, BOOL bSilent = FALSE, CString *pstrBillEditWarning = NULL, BOOL bIgnoreFinancialClose = FALSE);

// (c.haag 2009-03-10 09:11) - PLID 32433 - This function returns TRUE if a user is allowed to
// change a financial date to a certain day. This function applies to:
//
//	- Bill, Quote dates
//	- Charges, Quote Charge dates
//	- Payment, Adjustment, Refund dates
//	- Batch Payment dates
//
// Parameters:
//		strType: The type of item being tested in readable text ("Bill", "Payment"...)
//		dtServiceDate: The date of the item being tested (usually the service date, but it can be a bill date
//			or a quote date)
//		bWarnByFormField: TRUE if the user is in a modal form clicking on "Save", or doing something closely equivalent.
//			FALSE if they're trying to just delete a bill from a list of bills. This influences the fail message appearance.
//		permtype: The desired type of access. Standard permissions should have already been checked by now; this is only used
//			to see if we can override the preference.
//
// (d.thompson 2009-09-02) - PLID 34694 - Added suppression of messages
// (j.jones 2011-01-21 09:26) - PLID 42156 - renamed the message suppression to bSilent, because it needs to also suppress password prompts,
// and also added a string for the billing dlg Edit button to use
BOOL CanChangeHistoricFinancial_ByServiceDate(const CString& strType, COleDateTime dtServiceDate, BOOL bWarnByFormField,
											  ESecurityPermissionType permtype = sptWrite, BOOL bSilent = FALSE, CString *pstrBillEditWarning = NULL);

// (j.jones 2010-12-28 10:34) - PLID 41852 - used to enforce the ability to edit an input date against a hard close
BOOL CanChangeHistoricFinancial_ByInputDate(const CString& strType, COleDateTime dtInputDate, ESecurityPermissionType permtype = sptWrite);

//used as a return value for CanApplyLineItem and CanUnapplyLineItem
enum ECanApplyLineItemResult {
	ecalirCannotApply = 0,	//This line item cannot be applied or unapplied.
	ecalirCanApply,			//This line item can be applied or unapplied.
	ecalirCanApply_IDHasChanged,	//The original line item was closed, the user chose to correct,
										//and the passed-in ID changed. That new line item can be applied or unapplied.
};

// (j.jones 2011-08-24 17:18) - PLID 45176 - checks if the user has permission to apply, whether the source item is closed,
// and whether they can apply closed line items
// (j.jones 2013-07-01 09:42) - PLID 55517 - Now can potentially auto-correct the source item, if closed.
// If so, nSourcePayID will be changed.
ECanApplyLineItemResult CanApplyLineItem(IN OUT long &nSourcePayID, BOOL bSilent = FALSE);
//same concept as CanApplyLineItem, checks their permissions to unapply a line item (closed or unclosed),
//and offers to auto-correct closed line items.
ECanApplyLineItemResult CanUnapplyLineItem(IN OUT long &nApplyID, BOOL bSilent = FALSE);

// (j.jones 2013-07-01 13:31) - PLID 55517 - Made voiding & correcting a pay/adj/ref be a modular function
// that can optionally suppress the pre-correction warnings.
// If successful, return true. If bNeedCorrectedPaymentID is true, nPaymentID will change to reflect the new corrected ID.
// If it fails, return false. The payment ID will be unchanged.
bool VoidAndCorrectPayAdjRef(long &nPaymentID, CString strLineType, bool bNeedCorrectedPaymentID = false, bool bSilentWarnings = false);

// (j.jones 2009-09-16 12:44) - PLID 26481 - moved from CEligibilityRequestDlg so we could build
// the same content in multiple locations
// (j.jones 2013-03-28 15:25) - PLID 52182 - this is now in data, and thus obsolete
/*
void BuildEligibilityBenefitCategoryCombo(NXDATALIST2Lib::_DNxDataListPtr &pCategoryCombo,
										  int nCodeColumn, int nCategoryColumn);
void AddToEligibilityBenefitCategoryCombo(CString strCode, CString strCategory,
										  NXDATALIST2Lib::_DNxDataListPtr &pCategoryCombo,
										  int nCodeColumn, int nCategoryColumn);
*/

// (j.jones 2009-12-28 09:45) - PLID 32150 - placed queries for deleting service codes into
// one function such that the individual deletion in CCPTCodes::OnDeleteCpt() and mass deletion in
// CMainFrame::OnDeleteUnusedServiceCodes() would use the same delete code
void DeleteServiceCodes(CString &strSqlBatch, CString strServiceIDInClause);

// (j.jones 2012-03-27 09:41) - PLID 45752 - added ability to mass delete diagnosis codes,
// putting the delete code in one function so it can be reused when deleting one, or many
// (j.armen 2012-04-03 15:52) - PLID 48299 - Parameratized, Uses a CSqlFragment IN Clause
void DeleteDiagnosisCodes(CParamSqlBatch &sqlBatch, CSqlFragment &sqlDiagIDInClause);

// (j.jones 2012-03-27 09:41) - PLID 47448 - added ability to mass delete CPT modifiers,
// putting the delete code in one function so it can be reused when deleting one, or many
void DeleteCPTModifiers(CString &strSqlBatch, CString strModifierInClause);

// (j.jones 2010-04-14 08:45) - PLID 38194 - EBilling_Calculate2010_REF will calculate the ID and qualifier to send
// in either 2010AA or 2010AB for ANSI HCFA and ANSI UB claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
// 2010AA and 2010AB output the same REF data, but they have different Adv. Ebilling Setup overrides.
void EBilling_Calculate2010_REF(BOOL bIs2010AB, CString &strQualifier, CString &strID, CString &strLoadedFrom, BOOL bIsUBClaim,
								 long nProviderID, long nInsuranceCoID, long nLocationID, long nInsuredPartyID,
								 long nHCFASetupID, CString strDefaultBox33GRP,
								 long nUBSetupID, CString strUB04Box76Qual, long nBox82Num, long nBox82Setup);

// (j.jones 2010-04-14 10:04) - PLID 38194 - Ebilling_Calculate2310B_REF will calculate the ID and qualifier to send
// in 2310B for ANSI HCFA claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
void EBilling_Calculate2310B_REF(CString &strQualifier, CString &strID, CString &strLoadedFrom,
								 long nProviderID, long nHCFASetupID,
								 long nInsuranceCoID, long nInsuredPartyID, long nLocationID);

// (j.jones 2010-04-14 10:04) - PLID 38194 - Ebilling_Calculate2310E_REF will calculate the ID and qualifier to send
// in 2310E for ANSI HCFA claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
void EBilling_Calculate2310E_REF(CString &strQualifier, CString &strID, CString &strLoadedFrom,
								 long nSupervisingProviderID, long nHCFASetupID,
								 long nInsuranceCoID, long nInsuredPartyID, long nLocationID);

// (j.jones 2010-04-14 10:04) - PLID 38194 - Ebilling_Calculate2420A_REF will calculate the ID and qualifier to send
// in 2420A for ANSI HCFA claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
void EBilling_Calculate2420A_REF(CString &strQualifier, CString &strID, CString &strLoadedFrom,
								 long nProviderID, long nHCFASetupID,
								 long nInsuranceCoID, long nInsuredPartyID, long nLocationID);

// (j.jones 2010-04-14 10:04) - PLID 38194 - Ebilling_Calculate2310B_REF will calculate the ID and qualifier to send
// in 2310A for ANSI UB claims, in REF01/REF02. This does not cover the 
// "Additional REF Segment" setup from the Adv. Ebilling Setup.
void EBilling_Calculate2310A_REF(CString &strQualifier, CString &strID, CString &strLoadedFrom,
								 long nProviderID, long nUBSetupID, CString strUB04Box76Qual, long nBox82Num, long nBox82Setup,
								 long nInsuranceCoID, long nInsuredPartyID, long nLocationID);

// (j.jones 2010-04-14 09:19) - PLID 38194 - moved Box33PinANSI to GlobalFinancialUtils	
void EBilling_Box33PinANSI(CString &strIdentifier, CString &strID, long nProviderID, long nHCFASetupID,
						   long nInsuranceCoID, long nInsuredPartyID, long nLocationID);

// (j.jones 2010-04-14 09:19) - PLID 38194 - moved Box8283NumANSI to GlobalFinancialUtils	
void EBilling_Box8283NumANSI(CString &strIdentifier, CString &strID, long nProviderID, long nIndex,
							 long nInsuranceCoID, long nUBGroupID, BOOL bIsRefPhy);

// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils
void EBilling_Box17aANSI(long Box17a, CString Box17aQual, CString &strIdentifier, CString &strID, long RefPhyID);

// (j.jones 2010-07-23 14:09) - PLID 34105 - this function returns true/false if at least one
// HCFA group exists that can send claims with the "Assignments Of Benefits" not filled
// (j.jones 2010-07-27 09:19) - PLID 39854 - moved to CBlankAssignmentBenefitsDlg
//BOOL CanAssignmentOfBenefitsBeBlank();

// (j.jones 2010-07-23 14:45) - PLID 39783 - moved Accept Assignment calculations into these functions
BOOL GetAcceptAssignment_ByInsCo(long nInsuranceCoID, long nProviderID);
BOOL GetAcceptAssignment_ByInsuredParty(long nInsuredPartyID, long nProviderID);

// (j.jones 2010-09-24 13:32) - PLID 34518 - Given an insured party ID,
// open a payment for that insured party, default it to be a copay, and pull in the
// $ amount from the CoPay pay group. $0.00 otherwise.
void PromptForCopay(long nInsuredPartyID);

// displays all eligibility requests/responses for a patient, using the most-primary
// insured party on their account
void ShowAllEligibilityRequestsForInsuredParty_ByPatient(CWnd *pParentWnd, long nPatientID);
// displays all eligibility requests/responses for a patient, using the most-primary
// insured party on their account or the appointment's insured party
void ShowAllEligibilityRequestsForInsuredParty_ByPatientOrAppt(CWnd *pParentWnd, long nPatientID, long nAppointmentID);
//displays all eligibility requests/responses for a given insured party, which may generate a new request
//if our current list is outdated
void ShowAllEligibilityRequestsForInsuredParty(CWnd *pParentWnd, long nInsuredPartyID);
//master function that the other two call, may generate a new request
//if our current list is outdated
void ShowAllEligibilityRequestsForInsuredParty(CWnd *pParentWnd, long nInsuredPartyID, long nAppointmentID);

//For a given insured party, and optionally appointment, will check to see if an appropriate
//eligibility response exists within the past 3 days. If not, a new one will be created.
void AutoUpdateOutdatedEligibilityRequests(CWnd *pParentWnd, long nInsuredPartyID);
void AutoUpdateOutdatedEligibilityRequests(CWnd *pParentWnd, long nInsuredPartyID, long nAppointmentID);

//Auto-generates a new eligibility request for an insured party,
//if an appointment is provided it will be used for provider/location defaults.
//Returns false if a request was not created, or did not successfully send in real-time.
bool GenerateNewEligibilityRequestForInsuredParty(CWnd *pParentWnd, long nInsuredPartyID);
bool GenerateNewEligibilityRequestForInsuredParty(CWnd *pParentWnd, long nInsuredPartyID, long nAppointmentID);

// (j.jones 2010-10-19 09:55) - PLID 40931 - returns TRUE if at least one ANSI setup is set to use 5010
BOOL Is5010Enabled();

// (j.jones 2011-03-21 13:49) - PLID 42917 - these return the server's path to where
// we store EOBWarning logs and converted EOBs from E-Remittance.
const CString &GetNxEOBWarningLogsPath();
const CString &GetNxConvertedEOBsPath();

// (b.spivey, October 9th, 2014) PLID 62701 - return the path to the EOB storage location, 
const CString &GetNxEOBStoragePath();
//return the path of the file we moved to said storage. 
CString CopyParsedEOBToServerStorage(CString strInputFile);

namespace Nx
{
	// (a.walling 2014-03-19 10:05) - PLID 61346 - Generic structure to hold diagnosis codes
	struct DiagCode
	{
		long id;
		CString number;
		CString description;

		explicit DiagCode(long id = 0, CString number = CString(), CString description = CString())
			: id(id)
			, number(number)
			, description(description)
		{}
	};
}

// (j.jones 2011-04-20 10:45) - PLID 41490 - This function supports the ability for claims to ignore
//diagnosis codes not linked to charges. Taking in a Bill ID and an array of Charge IDs that will be
//sent on the claim, this function will fill aryNewDiagnosisCodesToExport with an ordered list of
//new diagnosis code IDs,aryNewDiagnosisDescriptionsToExport with their descriptions,
//and a list of diagnosis indexes that need to be skipped.
//Returns FALSE if the preference for this is off, or if nothing needs to be skipped.
//Returns TRUE only if something was skipped.
// (j.jones 2013-08-09 12:47) - PLID 57955 - Renamed to reflect that this is now called
// at all times, it might skip diagnosis codes based on the SkipUnlinkedDiagsOnClaims preference,
// but it will fill our arrays even if none are skipped.
// bUseICD10 to true to use ICD10, false to use ICD-9.
// (a.walling 2014-03-19 10:05) - PLID 61346 - Deprecated now by GetUniqueDiagnosisCodesForHCFA for forms and GetBillDiagCodes otherwise to get a vector of data
// The entire confusing 'skipped diag indexes' concept is going away
//BOOL GetDiagnosisCodesForHCFA(long nBillID, IN CArray<long, long> &aryChargeIDsToExport,
//							  OUT CStringArray &aryDiagnosisCodesToExport,
//							  OUT CStringArray &aryDiagnosisDescriptionsToExport,
//							  OUT CArray<long, long> &arySkippedDiagIndexes,
//							  bool bUseICD10);


// (j.jones 2011-04-20 10:45) - PLID 41490 - gets an array of all diagnosis codes on a bill, and their descriptions
// (j.jones 2011-04-20 10:45) - PLID 41490 - This function supports the ability for claims to ignore
//diagnosis codes not linked to charges. 
// (a.walling 2014-02-28 17:46) - PLID 61128 - works with BillDiagCodeT, has options for including ICD9 or ICD10
// row returns a recordset of the relevant information
// (a.walling 2014-03-05 16:05) - PLID 61202 - GetDiagnosisCodesForHCFA - Support ICD10 option, handle ChargeWhichCodesT
// (a.walling 2014-03-19 10:05) - PLID 61346 - The entire confusing 'skipped diag indexes' concept is going away. Everything can be handled with the vector of DiagCode.
std::vector<Nx::DiagCode> GetBillDiagCodes(
	long nBillID
	, bool bUseICD10, bool bSkipUnlinkedDiagsOnClaims
	, IN OPTIONAL const CArray<long, long>* paryChargeIDsToExport
);

// (a.walling 2014-03-19 10:05) - PLID 61346 - utility function to generate <BillDiags><Diag><ID>##</ID><OrderIndex>#</OrderIndex></Diag>...</BillDiags> xml 
// where OrderIndex is the order within the collection passed in
CString MakeBillDiagsXml(const std::vector<Nx::DiagCode>& diags);

// (a.walling 2014-03-17 14:01) - PLID 61401 - returns coalesced unique DiagCodes for the bill according to the ICD9/ICD10 state and passed in charge, and 'onlylinkedcharges' preference, and handles whichcodes.
void GetUniqueDiagnosisCodesForHCFA(long nBillID, IN CArray<long, long> &aryChargeIDsToExport,
							  OUT CStringArray &aryDiagnosisCodesToExport,
							  OUT CStringArray &aryDiagnosisDescriptionsToExport,
							  OUT CString& strBillDiagsXml,
							  bool bUseICD10);

// (j.jones 2011-04-20 11:28) - PLID 43330 - added a function to calculate
// how to load the diagnosis codes & descriptions on the claim
// (j.jones 2013-08-09 11:53) - PLID 57955 - Reworked this function, adding support for 12 diag codes (still only 4 description fields).
// This function simply takes in all the diag codes that should be on the array, and returns appropriate _Q()-escaped SQL-ready
// strings for diag fields 1 - 12 and diag desc. fields 1 - 4.
// Example: "'123.4' AS DiagCode, '432.1' AS DiagCode2," etc.
void CalculateHCFADiagCodeFields(IN CStringArray &aryNewDiagnosisCodesToExport,
								 IN CStringArray &aryNewDiagnosisCodeDescriptionsToExport,
								 OUT CString &strTop12DiagCodeFields, OUT CString &strTop4DiagCodeDescFields);

// (j.jones 2013-08-09 12:40) - PLID 57955 - Made a modular function to replace many code repeats of it.
// Takes in a bill ID and calculates all diagnosis codes that should show on that bill, returning appropriate
// _Q()-escaped SQL-ready strings for diag fields 1 - 12 and diag desc. fields 1 - 4.
// Example: "'123.4' AS DiagCode, '432.1' AS DiagCode2," etc.
// (a.walling 2014-03-06 08:47) - PLID 61216 - insured party ID for determining ICD10 state
void GenerateHCFADiagCodeFieldsForBill(bool bIsNewHCFA, long nBillID, long nInsuredPartyID, CSqlFragment sqlChargeFilter,
									   OUT CString &strTop12DiagCodeFields, OUT CString &strTop4DiagCodeDescFields,
									   OUT CString &strWhichCodes, OUT CString &strICDIndicator);

// (j.jones 2011-04-26 16:39) - PLID 42705 - added an enum for identifying
// how ChargeAllowablesT was filled, stored in ChargeAllowablesT.EntryMethod,
// only used for debugging purposes later
// ***stored in data, these cannot be changed***
enum ChargeAllowableEntryMethod {
	caemERemittance = 0,		//created by an E-Remit posting
	caemLineItemPosting = 1,	//created through Line Item Posting
	caemManualPayment = 2,		//created through a manual payment creation
};

// (j.jones 2011-04-26 17:20) - PLID 42705 - added function that safely changes an allowable on a charge,
// with proper auditing if necessary
void SaveChargeAllowable(long nPatientID, CString strPatientName, long nChargeID, long nInsuredPartyID,
						 COleCurrency cyAllowable, ChargeAllowableEntryMethod caemEntryMethod);

// (j.jones 2011-12-19 15:44) - PLID 43925 - added an enum for identifying
// how ChargeCoinsuranceT was filled, stored in ChargeCoinsuranceT.EntryMethod,
// only used for debugging purposes later
// ***stored in data, these cannot be changed***
enum ChargeCoinsuranceEntryMethod {
	ccemERemittance = 0,		//created by an E-Remit posting
	ccemLineItemPosting = 1,	//created through Line Item Posting
	ccemManualPayment = 2,		//created through a manual payment creation
};

// (j.jones 2011-12-19 15:42) - PLID 43925 - Added ability to save deductible and coinsurance info. to a charge.
// If we only have one of the two, the other currency should be $0.00.
// (j.jones 2012-08-14 14:27) - PLID 50285 - Now returns TRUE if it created a new record, FALSE if it updated an existing
// record. This won't often be referenced, but it's needed for calling code to tell if this is the first time we've saved
// this data for a charge & insured party.
// (j.jones 2013-08-27 10:58) - PLID 57398 - added copay
BOOL SaveChargeCoinsurance(long nPatientID, CString strPatientName, long nChargeID, long nInsuredPartyID,
						 COleCurrency cyDeductible, COleCurrency cyCoinsurance, COleCurrency cyCopay,
						 ChargeCoinsuranceEntryMethod ccemEntryMethod);

// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta
// (j.jones 2011-09-15 15:20) - PLID 44905 - Added IsLineItemCorrectionsEnabled_Beta,
// which will be removed when this feature is out of beta. Its return value determines
// if line item correction functionality is enabled.
//BOOL IsLineItemCorrectionsEnabled_Beta();

// (j.jones 2011-07-22 09:24) - PLID 44662 - made this be a global function
long CalculateAlbertaClaimNumberCheckDigit(CString strSequenceNumber);

// (j.jones 2011-09-13 15:35) - PLID 44887 - IsOriginalOrVoidLineItem
// returns TRUE if the line item is an original or corrected line item,
// or a balancing adjustment. These line items are read-only.
BOOL IsOriginalOrVoidLineItem(long nLineItemID);

// (j.jones 2011-09-13 15:35) - PLID 44887 - If the source line item for a given
// apply is part of a correction (ie. it is an original or void item, or a balancing
// adjustment), you cannot unapply it from where it is currently applied.
BOOL IsOriginalOrVoidApply(long nApplyID);

// (j.jones 2011-09-13 15:35) - PLID 44887 - IsVoidedBill
// returns TRUE if the bill is an original/voided bill.
// There is only one bill for this, not two. This bill is read-only.
BOOL IsVoidedBill(long nBillID);

// (j.jones 2011-09-13 15:35) - PLID 44887 - DoesBillHaveOriginalOrVoidCharges
// returns TRUE if the bill has charges in it that are Original or Void,
// which are read-only.
BOOL DoesBillHaveOriginalOrVoidCharges(long nBillID);

// (j.jones 2011-09-13 15:35) - PLID 44887 - DoesBillHaveUncorrectedCharges
// returns TRUE if the bill has charges in it that are NOT Original or Void,
// which are not read-only, which means the bill can be re-corrected.
BOOL DoesBillHaveUncorrectedCharges(long nBillID);

// (j.jones 2011-09-13 15:35) - PLID 44887 - Added enum for line item corrections
// types, this is not stored in data, just used by GetLineItemCorrectionStatus
// to identify a line item as original, void, corrected, or "normal" which means
// the line item is not a part of any corrections process.
enum LineItemCorrectionStatus
{
	licsNormal = 0,		//this line item is not involved in any correction process
	licsOriginal,		//this line item has been corrected, and is read-only
	licsVoid,			//this voided an Original line item OR cancelled out an original/void charge, and is read-only
	licsCorrected,		//this is a new line item that replaces the Original, and is editable
						//(if a Corrected line item is later corrected itself, it would return licsOriginal)
};

// (j.jones 2011-09-13 15:35) - PLID 44887 - GetLineItemCorrectionStatus
// returns a LineItemCorrectionsStatus enum for whether the line item
// has not been involved in a correction, or is an Original, Void, or
// Corrected line item. Original and Void line items are read-only.
// If a Corrected line item is later corrected itself, it would return
// licsOriginal, not licsCorrected.
LineItemCorrectionStatus GetLineItemCorrectionStatus(long nLineItemID);

// (j.jones 2011-09-13 15:35) - PLID 44887 - DoesBillHaveOriginalOrVoidApplies
// returns TRUE if the given bill has any apply which cannot be deleted,
// and therefore the bill cannot be deleted.
BOOL DoesBillHaveOriginalOrVoidApplies(long nBillID);

// (j.jones 2011-09-13 15:35) - PLID 44887 - DoesLineItemHaveOriginalOrVoidApplies
// returns TRUE if the given charge/payment has any apply which cannot be deleted,
// and therefore the charge cannot be deleted.
BOOL DoesLineItemHaveOriginalOrVoidApplies(long nLineItemID);

// (j.jones 2011-11-01 14:31) - PLID 45462 - IsLineItemAppliedToNewCorrection
// returns TRUE if the pay/adj/ref in question is applied to a line item that
// was later corrected, therefore it needs to be undone first
BOOL IsPaymentAppliedToNewCorrection(long nPaymentID, CString strOriginalCorrectionsTable, long nOriginalCorrectionID, OUT CString &strAppliedToType);

// (j.jones 2011-09-21 11:46) - PLID 45462 - checks permissions and closed status, 
// and warns if the item is closed
BOOL CanUndoCorrection(CString strType, COleDateTime dtCorrectionInputDate);

// (j.jones 2015-06-16 10:42) - PLID 50008 - Given the information on a corrected payment/adjustment/refund,
// this checks to see if the payment is applied in such a way that undoing the apply would cause a negative balance.
// Returns false if the user agreed to cancel the undo process.
// Returns true if the undo can continue.
// The array tracks any applies that need to be deleted when the correction undo process finishes.
bool UndoCorrection_CheckInvalidApplies(const CString strLineItemTypeName, 
	const long nCorrectionOriginalID, const long nCorrectionVoidingID, const long nCorrectionNewID,
	bool bVerboseWarnings, std::vector<long> &aryAppliesToUndo);

// (j.jones 2012-04-24 11:50) - PLID 49847 - Given a line item ID, see if it is corrected,
// and if so, return the corrected line item ID. If it's not corrected, return the original ID,
// implying it is still the "newest" version to use. If it has been voided, but not corrected,
// return -1.
// This uses a recursive ad-hoc function, so if you correct the correction, it finds the newest available version.
long FindNewestCorrectedLineItemID(long nOriginalLineItemID);

// (j.jones 2015-02-19 15:49) - PLID 64938 - used in FindNewestCorrectedApplyID
struct ApplyIDInfo {
	long nID;
	long nSourceID;
	long nDestID;

	ApplyIDInfo() {
		nID = -1;
		nSourceID = -1;
		nDestID = -1;
	}
};

// (j.jones 2015-02-19 15:26) - PLID 64938 - Given an AppliesT ID, see if it is corrected,
// and if so, return the corrected AppliesT ID. If it's not corrected, return the original ID,
// implying it is still the "newest" version to use.

// If it was voided, but not corrected, this returns -1 for the apply ID.
// It will also return -1 if the source & destination were corrected but an apply of the same amount no longer exists.
// This uses a recursive ad-hoc function, so if you correct the correction, it finds the newest available version.
ApplyIDInfo FindNewestCorrectedApply(long nOriginalApplyID);

// (j.jones 2012-07-26 09:57) - PLID 50489 - Simplifies code that compares four charge modifiers
// to one modifier. If the provided value is in use in any of the 4 provided modifier fields, return TRUE.
// Return FALSE if none of the provided fields contains the desired modifier.
BOOL ContainsModifier(CString strModifierToSearch, CString strChargeModifier1, CString strChargeModifier2, CString strChargeModifier3, CString strChargeModifier4);

// (j.jones 2012-08-08 11:22) - PLID 47778 - Returns TRUE not just if the insured party is the patient's first insurance,
// instead it returns TRUE if the insured party if its RespTypeT.Priority = 1, RespTypeT.CategoryPlacement = 1 or if the
// insured party has the "Send As Primary" option checked off.
BOOL IsInsuredPartySentAsPrimary(long nInsuredPartyID);

// (j.jones 2011-01-06 16:57) - PLID 41785 - added generic function to create a billing note
// (j.jones 2012-08-14 15:21) - PLID 50285 - moved from EOBDlg to globalfinancialutils
// (j.dinatale 2012-12-19 11:26) - PLID 54256 - function needs to be able to add notes to bills
void CreateBillingNote(long nPatientID, long nLineItemID, CString strNote, long nCategoryID, BOOL bShowOnStatement, BOOL bIsBillNote = FALSE);

//TES 10/2/2014 - PLID 63821 - Sets the given text as the "Status Note" on the given bill, with auditing
void SetBillStatusNote(long nBillID, long nPatientID, CString strNote);

// (j.dinatale 2012-12-28 11:26) - PLID 54365 - need to be able to get the provider's submitter prefix
// (j.dinatale 2012-12-31 13:04) - PLID 54382 - moved from ebilling.cpp
CString GetAlbertaProviderSubmitterPrefix(long nProviderID);

// (j.jones 2013-01-23 09:03) - PLID 54734 - Added a function to insert one bill into claim history.
// If bTrackLastSendDate is enabled, we will update HCFATrackT.LastSendDate.
// If bBatchedChargesOnly is enabled, only batched charges will add to claim history details, otherwise all charges are added.
void AddToClaimHistory(long nBillID, long nInsuredPartyID, ClaimSendType::ESendType eClaimSendType, CString strClearingHouseName = "",
					   BOOL bTrackLastSendDate = TRUE, BOOL bBatchedChargesOnly = TRUE);

// (j.jones 2013-01-25 12:16) - PLID 54853 - Called in ANSI E-Remittance to check whether
// a given code qualifier is one that we could have legitimately submitted. Also called in
// ANSI E-Billing exports to enforce that we would never send a qualifier not covered by this function.
BOOL IsValidServiceCodeQualifier(CString strQualifier);

// (j.jones 2013-07-18 11:23) - PLID 57616 - This function is meant to be called AFTER a claim is paid,
// and potentially already shifted & batched to secondary. It will check to see if the paying company is a primary
// company that is configured to NOT batch to secondary. It uses the bill's current 'main' insured party ID
// is the secondary insurer.
// If configured to not batch to this payer, it will unbatch the claim, add a billing note, and update claim history.
// nPaidInsuredPartyID is who paid, who was the 'primary' insurance on the claim, who crossed over to a secondary company.
BOOL CheckUnbatchCrossoverClaim(long nPatientID, long nBillID, long nPaidInsuredPartyID, COleDateTime dtPaymentDate,
								bool bBatchedChargesOnlyInClaimHistory, AuditEventItems aeiAuditItem,
								CString strAuditOldValue, CString strAuditNewValue);

// (j.jones 2013-07-18 11:22) - PLID 57616 - Unified function to handle when primary crossed over to secondary.
// This adds a note to History and Billing Notes, unbatches the claim, uniquely audits, and updates claim history.
// nCrossedOverInsuredPartyID is the insured party that was secondary, that was the insurance that the primary crossed over to,
// who we are unbatching on behalf of.
// From EOBs with MA18, it is who was the bill's OthrInsuredPartyID before posting.
// For manual postings, or EOBs without MA18, it's whoever is the bill's current InsuredPartyID.
void UnbatchCrossoverClaim(long nPatientID, long nBillID, long nCrossedOverInsuredPartyID, COleDateTime dtPaymentDate,
						   bool bBatchedChargesOnlyInClaimHistory, AuditEventItems aeiAuditItem,
						   CString strAuditOldValue, CString strAuditNewValue);

// (d.thompson 2009-08-18) - PLID 16758 - Moved the inactive quote forecolor here so it can be used elsewhere
#define INACTIVE_QUOTE_FORECOLOR	RGB(112,112,112)
// (d.thompson 2009-08-18) - PLID 16758 - Added a color for expired quotes.  Right now I'm just using the same color, but
//	we may decide to change that.
#define EXPIRED_QUOTE_FORECOLOR		INACTIVE_QUOTE_FORECOLOR

// (j.jones 2013-08-05 11:53) - PLID 57805 - defines the default HCFA upgrade date
// (j.jones 2014-03-28 11:53) - PLID 61589 - this is now 4/1/2014
const COleDateTime g_cdtDefaultHCFAUpgrade(2014, 4, 1, 0, 0, 0);

// (j.jones 2013-08-06 15:37) - PLID 57299 - Calculates the HCFA form to use for
// an insured party. Optionally takes in a bill ID, -1 if we don't have one.
bool UseNewHCFAForm(long nInsuredPartyID, OPTIONAL long nBillID = -1);

// (b.spivey March 5th, 2014) - PLID 61182 - Determines if the bill should be using ICD10 or not. 
bool ShouldUseICD10(long nInsuredPartyID, long nBillID);
// (b.spivey March 5th, 2014) - PLID 61247 
bool ShouldUseICD10(long nInsuredPartyID, COleDateTime dtProcessDate); 

// (j.jones 2014-05-02 15:24) - PLID 61838 - enum for ChargeLevelProviderConfigT.ReferringProviderOption, OrderingProviderOption, SupervisingProviderOption
enum ChargeLevelProviderConfigOption
{
	NoSelection = 0,		//this provider is not needed
	Required = 1,			//this provider is required, no special rule for the default, a specific provider ID may be provided
	ChargeProvider = 2,		//this provider is required, and defaults to the charge provider
	ClaimProvider = 3,		//this provider is required, and defaults to the charge's claim provider
};

// (j.jones 2014-04-30 13:19) - PLID 61838 - a struct for the return value of CalculateChargeClaimProviders
class ChargeClaimProviderSettings {
public:

	ChargeLevelProviderConfigOption eReferringProviderOption;
	long nReferringProviderID;

	ChargeLevelProviderConfigOption eOrderingProviderOption;
	long nOrderingProviderID;

	ChargeLevelProviderConfigOption eSupervisingProviderOption;
	long nSupervisingProviderID;

	ChargeClaimProviderSettings() {
		//initialize the settings
		eReferringProviderOption = ChargeLevelProviderConfigOption::NoSelection;
		nReferringProviderID = -1;

		eOrderingProviderOption = ChargeLevelProviderConfigOption::NoSelection;
		nOrderingProviderID = -1;

		eSupervisingProviderOption = ChargeLevelProviderConfigOption::NoSelection;
		nSupervisingProviderID = -1;
	}
};

// (j.jones 2014-04-30 13:19) - PLID 61838 - Calculate which specialty charge providers are required for a combination
// of service code, charge provider, bill location, and insured party.
// If they have no setup information, successive calls won't touch the database unless bForceReload is set to true.
ChargeClaimProviderSettings CalculateChargeClaimProviders(const long nServiceID, const long nProviderID, const long nLocationID, const long nInsuredPartyID, const bool bForceReload);

// (j.jones 2014-06-24 11:47) - PLID 60349 - Calculates the next insured party ID or resp type ID for
// a patient of the same insurance type (Medical, Vision, etc.).
// So if the current insured party ID is primary medical, the next insured party is Secondary Medical
// if one exists, or -1 if none exists.
long GetNextInsuredPartyIDByPriority(long nPatientID, long nCurInsuredPartyID);
long GetNextRespTypeByPriority(long nPatientID, long nRespTypeID);
void GetNextInsuredPartyByPriority(long nPatientID, long nRespTypeID, OUT long &nNextInsuredPartyID, OUT long &nNextRespTypeID);

// (j.jones 2014-07-08 09:07) - PLID 62570 - added the ability to split charges to a new bill
// returns the new bill ID, -1 if one was not made
long SplitChargesIntoNewBill(long nCurBillID, CArray<long, long> &aryChargeIDsToMove);
// (r.gonet 07/09/2014) - PLID 62571 - Changes a bill's on hold status.
bool SetBillOnHold(long nBillID, BOOL bOnHold, BOOL bSilent = FALSE);

//TES 9/24/2014 - PLID 62782 - Attempts to update the given bill to the given status.
//TES 9/30/2014 - PLID 62782 - This now sets the bill to the given status, unless it's On Hold, all other statuses will be overwritten by this function.
enum AlbertaBillingStatus;
bool TrySetAlbertaStatus(long nBillID, AlbertaBillingStatus eNewStatus);


// (s.tullis 2014-10-01 10:07) - PLID 62780 - Overload for TrySetAlbertaStatus for more than one bill
bool TrySetAlbertaStatus(MFCArray<long> arrBillID, AlbertaBillingStatus eNewStatus);

// (r.gonet 07/11/2014) - PLID 62556 - Simple class to store chargeback related line item IDs.
class Chargeback
{
public:
	long nID;
	long nChargeID;
	long nPaymentID;
	long nAdjustmentID;

	Chargeback()
		: nChargeID(-1), nPaymentID(-1), nAdjustmentID(-1)
	{
	}

	Chargeback(long nChargeID, long nPaymentID, long nAdjustmentID)
		: nChargeID(nChargeID), nPaymentID(nPaymentID), nAdjustmentID(nAdjustmentID)
	{
	}

	Chargeback(long nID, long nChargeID, long nPaymentID, long nAdjustmentID)
		: nID(nID), nChargeID(nChargeID), nPaymentID(nPaymentID), nAdjustmentID(nAdjustmentID)
	{
	}
};
typedef shared_ptr<Chargeback> ChargebackPtr;

// (r.gonet 07/11/2014) - PLID 62556 - Deletes a chargeback that is applied to a certain charge. Returns the amount to the batch payment.
// - chargeback - Reference to the chargeback to delete
// - bSilent - If true, then Practice will not display any warnings to the user. If false, Practice will warn the user about certain things.
// Returns true if the deletion was successful and false otherwise.
bool DeleteChargeback(Chargeback &chargeback, bool bSilent = false);

// (r.gonet 07/25/2014) - PLID 62556 - Returns the remaining amount on a batch payment
// - nBatchPaymentID: ID of the batch payment for which to get the remaining balance.
COleCurrency GetRemainingBatchPaymentAmount(long nBatchPaymentID);

//TES 7/25/2014 - PLID 63048 - Used to identify charges that cannot be deleted, due to being associated with Chargebacks
bool DoesChargeHaveChargeback(CSqlFragment sqlChargesTFilter);

//TES 7/25/2014 - PLID 63049 - Used to identify payments that cannot be deleted, due to being associated with Chargebacks
bool DoesPayHaveChargeback(CSqlFragment sqlPaymentsTFilter);

// (j.jones 2015-02-04 13:26) - PLID 64800 - repairs missing ChargeRespDetailT records
void EnsureChargeRespDetailRecord(long nChargeRespID);

// (j.jones 2015-03-20 14:12) - PLID 65402 - silently returns true
// if the LineItemT record is closed
bool IsLineItemClosed(long nLineItemID);

// (j.jones 2015-03-20 14:12) - PLID 65402 - silently returns true
// if the AppliesT record is closed
bool IsApplyClosed(long nApplyID);

// (j.jones 2015-03-20 15:33) - PLID 65402 - For when the user is about to create a refund
// for an applied payment, check the preference to automatically void & correct the payment
// and if it's enabled, do so.
// This will also check Unapply permissions to determine if the apply can proceed.
// If a Void & Correct happens, nPaymentID and nApplyID will be updated with the newest IDs.
// Returns true if the refund can continue being applied, false to abort the refund.
// nSourceRefundID isn't actually used for anything but warnings.
bool RefundingAppliedPayment_CheckVoidAndCorrect(IN OUT long &nPaymentID, IN OUT long &nApplyID, OPTIONAL long nRefundID = -1);

// (j.jones 2015-03-02 14:30) - PLID 64963 - standardized saving service categories via the API
// aryServiceIDs cannot be empty, aryCategoryIDs *can* be empty
// nDefaultCategoryID is -1 if there is no default, it cannot otherwise be an ID that is not in aryCategoryIDs
// bIsNewItem is true if you just created this item, false if it is an existing item.
// If bIsNewItem is false, the API will audit the category changes and send tablecheckers.
void UpdateServiceCategories(IN std::vector<long> &aryServiceIDs, IN std::vector<long> &aryCategoryIDs, IN long nDefaultCategoryID, bool bIsNewItem);

// (j.jones 2015-03-02 14:15) - PLID 64963 - standardized loading categories and descriptions
// for services/products that have multiple categories
void LoadServiceCategories(IN long nServiceID, OUT std::vector<long> &aryCategoryIDs, OUT CString &strCategoryNames, OUT long &nDefaultCategoryID);
// (j.jones 2015-03-03 13:43) - PLID 64965 - this version queries only based on the category IDs
void LoadServiceCategories(IN std::vector<long> &aryCategoryIDs, IN long nDefaultCategoryID, OUT CString &strCategoryNames);

// (j.jones 2015-04-23 13:59) - PLID 65711 - main function to transfer gift certificate balances
void TransferGiftCertificateAmount(long nSourceGiftID, long nDestGiftID, COleCurrency cyAmtToTransfer);

// (b.eyers 2015-10-06) - PLID 42101 - audits when the batch changes for an insurance claim
void AuditInsuranceBatch(std::vector<long> &aryBillIDs, long nNewBatch, long nOldBatch);

// (r.gonet 2015-04-24) - PLID 65327 - Opens the new Gift Certificate and presets the fields if the optional fields are filled in.
// (j.jones 2015-05-11 16:57) - PLID 65714 - added GCCreationStyle
enum class GCCreationStyle;
bool CreateNewGiftCertificate(CWnd *pParentWnd, GCCreationStyle eCreationStyle, OUT long &nNewGiftID, OUT CString &strNewCertNumber,
	OPTIONAL CString strCertNumber = "", OPTIONAL long nTypeID = -1,
	OPTIONAL COleCurrency cyValue = g_ccyNull, OPTIONAL COleCurrency cyDisplayedValue = g_ccyNull, OPTIONAL COleCurrency cyPrice = g_ccyNull,
	OPTIONAL long nProviderID = -1, OPTIONAL long nLocationID = -1,
	OPTIONAL COleDateTime dtPurchaseDate = g_cdtNull, OPTIONAL bool bPresetExpirationDate = false, OPTIONAL COleDateTime dtExpirationDate = g_cdtNull,
	OPTIONAL long nPurchasedByPatientID = -1, OPTIONAL bool bPresetReceivedByPatientID = false, OPTIONAL long nReceivedByPatientID = -1);

// (z.manning 2015-09-11 09:05) - PLID 67224
COleCurrency DoubleToOleCurrency(double dblAmount);
// (z.manning 2015-09-14 14:31) - PLID 67221
double OleCurrencyToDouble(const COleCurrency &cy);

// (z.manning 2015-07-23 09:35) - PLID 67241 - Checks the license and config property to see if ICCP is enabled
BOOL IsICCPEnabled();
// (z.manning 2015-09-04 09:09) - PLID 67236 - Added overload
BOOL IsICCPEnabled(BOOL bIgnorePreference);

//(j.camacho 2016-02-01) plid 68010
BOOL IsIntellechartToBeBilledEnabled();

// (z.manning 2015-07-22 10:30) - PLID 67241
void OpenPaymentProfileDlg(const long nPatientPersonID, CWnd *pwndParent);

// (j.jones 2015-09-30 10:34) - PLID 67171 - added a global function to see if a payment
// was processed under non-ICCP processing (Chase or Intuit)
bool IsCCPaymentProcessedPreICCP(long nPaymentID);

// (z.manning 2015-08-18 14:50) - PLID 67248
// (c.haag 2015-09-22) - PLID 67202 - Allow for custom prompts
BOOL PromptForCreditCardExpirationDate(CWnd *pwndParent, OUT COleDateTime &dtExpiration, const CString& strPromptOverride = "");

// (z.manning 2015-09-08 12:08) - PLID 67224 - Moved here from CPaymentDlg
CString CenterLine(CString strText, long nLineWidth);
CString LeftRightJustify(CString strTextLeft, CString strTextRight, long nLineWidth);
CString LeftJustify(CString strText, long nLineWidth);
//TES 12/6/2007 - PLID 28192 - Added the POS Printer as a parameter to these functions, so it can just be claimed
// once, and then passed into these functions for printing.
// (d.thompson 2011-01-05) - PLID 42005 - nLocationID should now be filled before sending, will no longer return it.
BOOL PrintCreditReceiptHeader(COPOSPrinterDevice *pPOSPrinter, long nLocationID);
BOOL PrintCreditReceiptMiddle(COPOSPrinterDevice *pPOSPrinter, BOOL bIsRefund, const CString &strCardName
	, const CString &strCardNumberToDisplay, BOOL *pbSwipe);
// (c.haag 2015-09-15) - PLID 67195 - We now take in a signature
BOOL PrintCreditReceiptFooter(COPOSPrinterDevice *pPOSPrinter, BOOL bMerchantCopy, BOOL bIsRefund
	, const COleCurrency &cyAmount, const CString &strCardHolderName, const CString& strSignatureBMPFileName);
// (j.jones 2015-09-30 10:56) - PLID 67180 - added signature bitmap stream, optional
BOOL PrintCreditCardReceipt(CWnd* pwndParent, BOOL bPrintMerchantCopy, BOOL bPrintCustomerCopy, long nPaymentID
	, long nLocationID, BOOL bIsRefund, const COleCurrency &cyAmount, const CString &strCardHolderName
	, const CString &strCardName, const CString &strCardNumberToDisplay, BOOL *pbSwipe, const IStreamPtr &pSignatureImageStream);
// (z.manning 2015-09-08 15:49) - PLID 67224 - ICCP receipt printing
void PrintICCPReceipts(CWnd *pwndParent, long nPaymentID, BOOL bMerchantCopy, BOOL bCustomerCopy);
// (z.manning 2015-09-09 09:52) - PLID 67226
void PromptToPrintICCPReceipts(CWnd *pwndParent, long nPaymentID);

// (j.jones 2015-09-30 10:56) - PLID 67180 - given a PaymentID and its ICCP signature safearray (a gzip file),
// returns an IStreamPtr of the signature image
IStreamPtr GetICCPSignatureBitmapStream(long nPaymentID, Nx::SafeArray<BYTE> &saGZippedSignature);

// (c.haag 2015-08-18) - PLID 67191 - Do a test swipe with a credit card. The card will not be charged.
// Returns TRUE on success, or FALSE on failure.
//		pwndParent - The parent window
//		nAccountID - The account ID of the merchant to test with
// Returns TRUE on success
BOOL DoCreditCardTestSwipe(CWnd* pwndParent, long nAccountID);

// (c.haag 2015-08-18) - PLID 67203 - Handles swiping and authorizing credit card payments
// (c.haag 2015-08-18) - PLID 67202 - Handles authorizing "Card Not Present" credit card payments
// (c.haag 2015-08-25) - PLID 67196 - Added nPaymentIDToRefund
// (z.manning 2015-08-31 09:39) - PLID 67230 - Added get siganture flag
// (z.manning 2015-09-01 11:35) - PLID 67229 - Added bWasSignedElectronically
//
//		pwndParent - The parent window
//		nLineItemID - The LineItemT.ID of the payment being authorized (which must exist before we can do an authorization)
//		nPaymentIDToRefund - If this is a refund transaction, then this is the ID of the payment to refund. Otherwise -1
//		bCardPresent - True if the card is present to swipe; false if it is not present
//		bGetSignature - True if the device should capture the signature, false if not
//		bTestAuthorization - True if this is only a test authorization and we are not charging the card
//		pmapNonExpiredCreditCardTokens - A set of existing tokens to check for duplicates if creating a profile(can be null)
//
//		nPatientPersonID - The person ID of the patient
//		nAccountID - The internal ID of the merchant account in data
//		cyAmount - The amount to charge. This should be a negative value if a refund is being performed.
//		dtCardExpiration - The expriation date of the card being used if already known
//		bCreatePaymentProfile - True to create a payment profile after authorization
//
//		strCreditCardToken - The credit card token
//		lpdispAuthorizationResult - The authorization result
//		bWasSignedElectronically - True if an electronic signature was captured during the transaction
//
// Returns TRUE on success
BOOL AuthorizeCreditCardTransaction(CWnd *pwndParent, long nLineItemID, long nPaymentIDToRefund, BOOL bCardPresent, BOOL bGetSignature, std::set<CString> *psetNonExpiredCreditCardTokens,
	long nPatientPersonID, long nAccountID, COleCurrency& cyAmount, COleDateTime &dtCardExpiration, BOOL bCreatePaymentProfile,
	IN OUT CString &strCreditCardToken, OUT LPDISPATCH &lpdispAuthorizationResult, OUT BOOL &bWasSignedElectronically);

// (c.haag 2015-08-18) - PLID 67203 - Records an authorized credit card transaction to data
// (c.haag 2015-08-25) - PLID 67196 - Added nPaymentIDToRefund
// (z.manning 2015-09-01 11:35) - PLID 67229 - Added bWasSignedElectronically
//
//		lpdispAuthorizationResult - The result of a prior call to ReadAndAuthorizePaymentSwipe
//
//		nAccountID - The internal ID of the merchant account in data
//		cyAmount - The amount the customer attempted to charge
//		bstrToken - The credit card token returned from a prior call to ReadAndAuthorizePaymentSwipe
//		nAuthorizedLineItemID - The ID of the authorized line item
//		nPaymentIDToRefund - If this is a refund transaction, then this is the ID of the payment to refund. Otherwise -1
//		bWasSignedElectronically - True if an electronic signature was captured during the transaction
//
void RecordAuthorizedTransaction(LPDISPATCH lpdispAuthorizationResult,
	long nAccountID, const COleCurrency& cyAmount, _bstr_t bstrToken, long nAuthorizedLineItemID, long nPaymentIDToRefund, BOOL bWasSignedElectronically);

// (c.haag 2015-08-25) - PLID 67197 - Refunds an authorized credit card payment
//
//		nPaymentID - The ID of the payment to refund
//		cyAmount - The amount to refund
//		lpdispAuthorizationResult - The authorization result
//
// Returns TRUE on success
BOOL RefundAuthorizedPayment(long nPaymentID, const COleCurrency& cyAmount, OUT LPDISPATCH& lpdispResult);

// (c.haag 2015-08-25) - PLID 67197 - Records an authorized refund
//
//		lpdispAuthorizationResult - The result of a prior call to ReadAndAuthorizePaymentSwipe
//
//		nAccountID - The internal ID of the merchant account in data
//		cyAmount - The amount the customer attempted to charge
//		bstrToken - The credit card token returned from a prior call to ReadAndAuthorizePaymentSwipe
//		nAuthorizedLineItemID - The ID of the authorized line item
//		nPaymentIDToRefund - If this is a refund transaction, then this is the ID of the payment to refund. Otherwise -1
//		bWasSignedElectronically - True if an electronic signature was captured during the transaction
//
void RecordAuthorizedPaymentRefund(LPDISPATCH lpdispAuthorizationResult,
	long nAccountID, const COleCurrency& cyAmount, _bstr_t bstrToken, long nAuthorizedLineItemID, long nPaymentIDToRefund, BOOL bWasSignedElectronically);

// (j.jones 2015-09-30 10:34) - PLID 67171 - This confusingly named function
// is meant to be called when about to make a refund for a credit card payment.
// If ICCP is enabled, and the payment was not processed using ICCP, this will warn
// the user that they cannot process the refund using the same card as the payment.
void CheckWarnCreditCardPaymentRefundMismatch(long nCreditCardPaymentID);

// (c.haag 2015-08-24) - PLID 67198 - Returns true if a payment was processed under ANY credit card processing
bool IsCCPaymentProcessed(long nPaymentID);

// (c.haag 2015-08-24) - PLID 67198 - Returns true if the patient has at least one payment associated with any kind of credit card transaction; approved or otherwise
bool DoesPatientHaveCreditCardTransactions(long nUserDefinedID);

// (c.haag 2015-08-24) - PLID 67198 - Returns true if the user is associated with any kind of credit card transaction; approved or otherwise
bool DoesUserHaveCreditCardTransactions(long nUserID);

// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format,
// returns EbillingFormatsT.ID
long GetDefaultEbillingANSIFormatID();

// (j.jones 2009-09-14 17:35) - PLID 35284 - added enum for ANSI Claim Type Code
// do not change these, these are stored in data
// (j.jones 2016-05-24 9:55) - NX-100707 - moved from ebilling to global financial utils
enum ANSI_ClaimTypeCode {

	ctcOriginal = 0,
	ctcCorrected = 1,
	ctcReplacement = 2,
	ctcVoid = 3,
};

// (j.jones 2016-05-24 14:21) - NX-100704 - added lookup for claim type codes & descriptions per export style
CString GetClaimTypeCode_HCFA(ANSI_ClaimTypeCode ctcType);
CString GetClaimTypeDescription_HCFA(ANSI_ClaimTypeCode ctcType);
CString GetClaimTypeCode_UB(ANSI_ClaimTypeCode ctcType);
CString GetClaimTypeDescription_UB(ANSI_ClaimTypeCode ctcType);
CString GetClaimTypeCode_Alberta(ANSI_ClaimTypeCode ctcType);
CString GetClaimTypeDescription_Alberta(ANSI_ClaimTypeCode ctcType);

// (j.jones 2009-12-17 14:54) - PLID 36641 - added enum for
// whether the response was confirmed/denied/invalid
//***DO NOT CHANGE THESE, THEY ARE STORED IN DATA***//
enum EligibilityResponseConfirmationStatusEnum {

	ercsConfirmed = 1,
	ercsDenied = 2,
	ercsInvalid = 3,
};

//The following three functions returns a SQL fragment that acts as an INNER JOIN
//on EligibilityResponsesT, can be used in any query that includes EligibilityResponsesT.

//This will filter only on responses that indicate they have active coverage.
CSqlFragment GetEligibilityActiveCoverageInnerJoin();
//This will filter only on responses that indicate they have inactive coverage.
CSqlFragment GetEligibilityInactiveCoverageInnerJoin();
//This will filter only on responses that have a failure reported.
CSqlFragment GetEligibilityFailedResponseInnerJoin();

//gets the highest priority insured party (e.g. RespType of 1) for this patient,
//returns a SQL query to return that TOP 1 InsuredPartyID
CSqlFragment GetPatientPrimaryInsuredPartyIDSql(long nPatientID);

#endif
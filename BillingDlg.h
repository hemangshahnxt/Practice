#pragma once

#include "InvUtils.h"
#include "NotesDlg.h"
#include "DiagCodeInfoFwd.h"

// BillingDlg.h : header file
//

enum ChargeLevelProviderConfigOption;
// (r.gonet 07/02/2014) - PLID 62567 - Forward declaration.
enum class EBillStatusType;

// (j.jones 2011-10-25 09:04) - PLID 46088 - changed into an enum, finally,
// that is properly named (these columns are the same for bills and for quotes)
enum SharedColumns {
	COLUMN_LINE_ID = 0,
	COLUMN_CHARGE_ID,
};

// (j.jones 2011-10-25 09:04) - PLID 46088 - changed into an enum, finally
// (r.gonet 2015-03-27 10:18) - PLID 65462 - WHEN YOU ADD A NEW COLUMN TO THIS ENUM, 
// YOU MUST ADD THE COLUMN'S WIDTH TO THE TWO CSV STRINGS IN GlobalFinancialUtils's GetDefaultBillingColumnWidths() AND TO DEFAULT_BILL_COLUMN_WIDTHS IN Preferences's ButtonCallbacks.h
enum BillColumns {
	// (d.singleton 2012-03-07 17:48) - PLID 49100 new column
	BILL_VALIDATION_STATUS = 2,
	BILL_COLUMN_DATE,
	COLUMN_SERVICE_DATE_TO,
	COLUMN_INPUT_DATE,
	BILL_COLUMN_PROVIDER,
	BILL_COLUMN_CLAIM_PROVIDER,			// (j.jones 2010-11-09 09:40) - PLID 31392 - added claim provider
	BILL_COLUMN_REFERRING_PROVIDER,		// (j.jones 2014-04-22 16:41) - PLID 61836
	BILL_COLUMN_ORDERING_PROVIDER,		// (j.jones 2014-04-22 16:41) - PLID 61836
	BILL_COLUMN_SUPERVISING_PROVIDER,	// (j.jones 2014-04-22 16:41) - PLID 61836
	COLUMN_PATCOORD,
	COLUMN_SERVICE_ID,
	BILL_COLUMN_CPT_CODE,
	BILL_COLUMN_CPT_SUB_CODE,
	BILL_COLUMN_CPT_CATEGORY, // (s.tullis 2015-03-24 09:28) - PLID 64973 -  CPT Category combo
	BILL_COLUMN_CPT_TYPE,
	BILL_COLUMN_CPT_TYPEOFSERVICE,
	COLUMN_MODIFIER1,
	COLUMN_MODIFIER2,
	COLUMN_MODIFIER3,
	COLUMN_MODIFIER4,
	// (j.jones 2011-10-25 09:04) - PLID 46088 - added Calls column, Alberta only
	COLUMN_CALLS,
	// (d.singleton 2012-05-08 14:45) - PLID 48152 added skills column, alberta only
	COLUMN_SKILL,
	// (d.singleton 2012-03-22 15:43) - PLID 49136 add notes column
	BILL_COLUMN_NOTES,
	BILL_COLUMN_WHICH_CODES,
	BILL_COLUMN_WHICH_CODES_EXT,
	BILL_COLUMN_DESCRIPTION,
	BILL_COLUMN_QUANTITY,
	BILL_COLUMN_UNIT_COST,
	// (r.gonet 2015-03-25 17:18) - PLID 65277 - Added Value to bills, which is to display GiftCertificatesT.Value.
	BILL_COLUMN_VALUE,
// (j.jones 2010-09-01 10:40) - PLID 40330 - added allowable to bills
	BILL_COLUMN_ALLOWABLE,
// (j.gruber 2009-03-05 17:14) - PLID 33351 - take out discount fields, add totaldiscount
	COLUMN_TOTAL_DISCOUNT,
	BILL_COLUMN_LINE_TOTAL,
	COLUMN_INS_RESP,
	COLUMN_INS_PARTY_ID,
	COLUMN_TAX_RATE_1,
	COLUMN_TAX_RATE_2,
//DRT 4/7/2006 - PLID 11734 - Changed from ProcCode to ItemType.  Remains column 27
	COLUMN_ITEM_TYPE,
	COLUMN_PRODUCT_ITEM_ID,
	COLUMN_ALLOCATION_DETAIL_LIST_ID,	// (j.jones 2007-12-14 10:38) - PLID 27988
	COLUMN_BATCHED,
	COLUMN_PACKAGE_CHARGE_REF_ID,
	BILL_COLUMN_ON_HOLD,				// (r.gonet 07/07/2014) - PLID 62569 - VT_BOOL column. True if the charge is on hold. False if it is not on hold. Not nullable.
	// (r.gonet 2015-03-27 10:18) - PLID 65462 - WHEN YOU ADD A NEW COLUMN TO THIS ENUM, 
	// YOU MUST ADD THE COLUMN'S WIDTH TO THE TWO CSV STRINGS IN GlobalFinancialUtils's GetDefaultBillingColumnWidths() AND TO DEFAULT_BILL_COLUMN_WIDTHS IN Preferences's ButtonCallbacks.h
};

//quote datalist columns
// (j.jones 2011-10-25 09:04) - PLID 46088 - changed into an enum, finally
// (r.gonet 2015-03-27 10:18) - PLID 65462 - WHEN YOU ADD A NEW COLUMN TO THIS ENUM, 
// YOU MUST ADD THE COLUMN'S WIDTH TO THE TWO CSV STRINGS IN GlobalFinancialUtils's GetDefaultQuoteColumnWidths() AND TO DEFAULT_QUOTE_COLUMN_WIDTHS IN Preferences's ButtonCallbacks.h
enum QuoteColumns {
	QUOTE_COLUMN_PROVIDER = 2,
	QUOTE_COLUMN_SERVICE_ID,
	QUOTE_COLUMN_CPT_CODE,
	QUOTE_COLUMN_CPT_SUB_CODE,
	QUOTE_COLUMN_CPT_CATEGORY, // (s.tullis 2015-03-24 09:28) - PLID 64973 -  CPT Category combo
	QUOTE_COLUMN_CPT_TYPE,
	QUOTE_COLUMN_MODIFIER1,
	QUOTE_COLUMN_MODIFIER2,
	QUOTE_COLUMN_MODIFIER3,
	QUOTE_COLUMN_MODIFIER4,
// (j.jones 2011-10-25 09:04) - PLID 46088 - added Calls column, Alberta only
	QUOTE_COLUMN_CALLS,
	// (d.singleton 2012-05-21 10:46) - PLID 48152 added skill column , alberta only
	QUOTE_COLUMN_SKILL,
	QUOTE_COLUMN_DESCRIPTION,
	QUOTE_COLUMN_QUANTITY,
	QUOTE_COLUMN_PACKAGE_QTY_REM,
	QUOTE_COLUMN_PACKAGE_ORIGINAL_QTY_REM,
	QUOTE_COLUMN_UNIT_COST,
	QUOTE_COLUMN_UNIT_COST_OUTSIDE_PRACTICE,
// (j.gruber 2009-10-15 16:28) - PLID 35947 - Added allowable column
// (j.jones 2010-09-01 10:51) - PLID 40330 - moved to be after the unit cost
	QUOTE_COLUMN_ALLOWABLE,
// (j.gruber 2009-03-05 17:20) - PLID 33351 - take out discount fields, add total discount link
	QUOTE_COLUMN_TOTAL_DISCOUNT,
	QUOTE_COLUMN_LINE_TOTAL,
	QUOTE_COLUMN_TAX_RATE_1,
	QUOTE_COLUMN_TAX_RATE_2,
// (j.jones 2007-07-06 13:44) - PLID 26098 - added item type column
	QUOTE_COLUMN_ITEM_TYPE,
	// (r.gonet 2015-03-27 10:18) - PLID 65462 - WHEN YOU ADD A NEW COLUMN TO THIS ENUM, 
	// YOU MUST ADD THE COLUMN'S WIDTH TO THE TWO CSV STRINGS IN GlobalFinancialUtils's GetDefaultQuoteColumnWidths() AND TO DEFAULT_QUOTE_COLUMN_WIDTHS IN Preferences's ButtonCallbacks.h
};

// (j.jones 2009-08-12 15:31) - PLID 35179 - track case history information
struct CaseHistoryInfo {

	long nCaseHistoryID;
	CString strCaseHistoryName;
	COleDateTime dtSurgeryDate;
};

//struct of a charge ID, it's resp type, and that resp's amount
struct RespPerCharge {	
	_variant_t InsuredPartyID;
	_variant_t InsAmount;
	// (j.jones 2007-02-27 17:49) - PLID 24844 - used for auditing
	_variant_t InsuranceCoName;
	_variant_t RespTypeName;
};

struct RPCList {	//contains an id and a list of RespPerCharge structures
	CArray<RespPerCharge, RespPerCharge> aryRPC;
};

// (j.jones 2013-01-15 11:21) - PLID 54187 - moved this define from BillExtraDiagCodesDlg
// This defines the max index that is allowed to be linked to a charge.
#define MAX_DIAG_Cs_INDEX 12

// (j.gruber 2009-03-23 17:03) - PLID 33355 - added new menu for preference
// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_PRACTICEFEES 41350
#define ID_BOTHPRACTICEANDOTHERFEES 41351

// (j.jones 2011-08-24 08:41) - PLID 44868 - defined the foreground gray
// color for original & void charges
#define CORRECTED_CHARGE_FOREGROUND_COLOR	RGB(128,128,128)

enum ApplyToAllPref {
	atapNotApplicable = 0,
	atapPracticeFeesOnly,
	atapAllLines,
};

// (z.manning 2011-03-28 17:37) - PLID 41423 - Renamed the enum
enum KeepQuoteDiscountPref {
	dpKeepDiscountSeparate = 0,
	dpIncludeDiscountInTotal,
	dpIgnorePreference
};

// (j.gruber 2009-03-05 12:25) - PLID 33351 - structure and array for discounts
// (z.manning 2011-03-28 16:49) - PLID 41423 - Changed the type of DiscountPreference to KeepQuoteDiscountPref (was a variant)
struct stDiscount {
	_variant_t ID;
	_variant_t PercentOff;
	_variant_t Discount;
	_variant_t CustomDiscountDescription;
	_variant_t DiscountCategoryID;
	_variant_t CouponID;
	KeepQuoteDiscountPref DiscountPreference;
};
#define NEW_DISCOUNT_ID -100

struct DiscountList {
	CArray<stDiscount, stDiscount> aryDiscounts;
};
//TES 7/1/2008 - PLID 26143 - This struct is used by SaveChanges() when applying existing payments.
struct PaymentToApply {
	long nID;
	long nInsuredPartyID;	
	COleCurrency cyRespAmount;
	// (j.jones 2009-10-27 17:32) - PLID 35934 - track the linked quote ID, original payment amount, and prepay status
	long nQuoteID;
	COleCurrency cyPayAmount;
	BOOL bIsPrePayment;
};

// (j.jones 2012-01-19 12:24) - PLID 47653 - this struct tracks the EMNID, the insured party ID
// that the EMN was *originally* added to the bill under, and the resp. name
struct BilledEMNInfo {

	long nEMNID;
	long nInsuredPartyID;
	CString strRespTypeName;
};

// (r.gonet 2016-04-07) - NX-100072 - Predeclare the enum.
enum ConditionDateType;

// (r.gonet 2016-04-07) - NX-100072 - Holds the additional claim dates.
struct ClaimDates {
	COleDateTime dtFirstVisitOrConsultationDate = g_cdtNull;
	COleDateTime dtInitialTreatmentDate = g_cdtNull;
	COleDateTime dtLastSeenDate = g_cdtNull;
	COleDateTime dtAcuteManifestationDate = g_cdtNull;
	COleDateTime dtLastXRayDate = g_cdtNull;
	COleDateTime dtHearingAndPrescriptionDate = g_cdtNull;
	COleDateTime dtAssumedCareDate = g_cdtNull;
	COleDateTime dtRelinquishedCareDate = g_cdtNull;
	COleDateTime dtAccidentDate = g_cdtNull;

	// (r.gonet 2016-04-07) - NX-100072 - Which date the second claim date, called the First Condition Date, on the Insurance tab refers to.
	ConditionDateType eConditionDateType;

	ClaimDates();

	// (r.gonet 2016-04-07) - NX-100072 - Gets the second claim date, called the First Condition Date, on the Insurance tab.
	COleDateTime GetFirstConditionDate() const;

	// (r.gonet 2016-04-07) - NX-100072 - Sets the second claim date, called the First Condition Date, on the Insurance tab.
	void SetFirstConditionDate(COleDateTime dtFirstConditionDate);

	void Clear();
	bool AnyFilledAndValid() const;
	CString GetValidationWarning() const;

private:
	void GetValidationWarning(CString& strWarningInvalid, long& nWarningInvalid, CString& strWarningLater, long& nWarningLater,
		COleDateTime dt, ConditionDateType eConditionDateType) const;
};

// (r.gonet 2016-04-07) - NX-100072 - Gets a description of a ConditionDateType enum value.
CString GetConditionDateTypeDescription(ConditionDateType eConditionDateType);

// (j.gruber 2009-10-16 12:09) - PLID 35947 - defines for easier readability
#define PRI_INSCO_NOT_SET  -1
#define PRI_INSCO_NOT_EXIST -2

// (a.walling 2014-02-24 11:25) - PLID 61003 - use smart pointers for BillingItem -- shared_ptr, since we don't have VS2010 or later yet to use unique_ptr.

struct BillingItem;
typedef shared_ptr<BillingItem> BillingItemPtr;

struct BillingItem 
	: private boost::noncopyable // RPCList and other things are not safely copyable due to ambiguous ownership
	, public boost::enable_shared_from_this<BillingItem>
{

	_variant_t LineID;
	_variant_t ChargeID;
	// (d.singleton 2012-03-07 17:49) - PLID 49100 new column
	_variant_t ValidationStatus;
	_variant_t Date;
	_variant_t ServiceDateTo;
	_variant_t Provider;
	// (j.jones 2010-11-09 10:03) - PLID 31392 - added claim provider
	_variant_t ClaimProvider;
	// (j.jones 2014-04-22 16:41) - PLID 61836 - added referring, ordering, supervising providers
	_variant_t ReferringProviderID;
	_variant_t ReferringProviderIDRequired;
	_variant_t OrderingProviderID;
	_variant_t OrderingProviderIDRequired;
	_variant_t SupervisingProviderID;
	_variant_t SupervisingProviderIDRequired;
	_variant_t PatCoordinator;
	_variant_t ServiceID;
	_variant_t CPTCode;
	_variant_t CPTSubCode;
	_variant_t CPTType;
	_variant_t TypeOfService;
	_variant_t Modifier1;
	_variant_t Modifier2;
	_variant_t Modifier3;
	_variant_t Modifier4;
	_variant_t Multiplier1;
	_variant_t Multiplier2;
	_variant_t Multiplier3;
	_variant_t Multiplier4;
	// (j.jones 2011-10-25 09:31) - PLID 46088 - added Calls column, Alberta only
	_variant_t Calls;
	// (d.singleton 2012-05-08 14:46) - PLID 48152 added skill column , alberta only
	_variant_t Skill;
	// (d.singleton 2012-03-22 15:43) - PLID 49136 add notes column
	_variant_t Notes;
	_variant_t Description;
	_variant_t Quantity;
	_variant_t PackageQtyRemaining;
	_variant_t OriginalPackageQtyRemaining;	// (j.jones 2009-12-22 17:04) - PLID 32587 - added a variable for original quantity
	// (j.gruber 2014-02-19 09:31) - PLID 60878 - take old whichCodes structure out	
	_variant_t UnitCost;
	_variant_t RVUValue;
	_variant_t RVUMultiplier;
	_variant_t InputDate;
	_variant_t LoginName;
	_variant_t LineTotal;
	_variant_t TaxRate1;
	_variant_t TaxRate2;
	//_variant_t PercentOff; // (j.gruber 2009-03-05 11:00) - PLID 33351 - removed to separate dialog
	//_variant_t Discount; // (j.gruber 2009-03-05 11:00) - PLID 33351 - removed to separate dialog
	_variant_t CPTCategoryID;
	_variant_t CPTCategoryCount; // (s.tullis 2015-04-07 16:54) - PLID 64975 - Added Category Count for Showing/Hiding/ Disabling category column later
	_variant_t CPTSubCategoryID;
	//_variant_t ProcCode;		//DRT 4/7/2006 - PLID 11734 - Removed this field, use ItemType instead.
	_variant_t OthrUnitCost;
	_variant_t ChargedProductItemListID;
	_variant_t ChargedAllocationDetailListID;	// (j.jones 2007-12-10 14:23) - PLID 27988
	_variant_t ItemType;		//DRT 4/1/2004 - Replace ProcCode (see ITEM_TYPE_... defines)
	_variant_t GiftID;			//DRT 4/1/2004 - Gift certificate ID.  NULL if not a gift card, -1 means no GC selected, + value is an ID in GiftCertificatesT
	_variant_t Batched;			//JMJ 4/4/2005 - boolean that determines if the charge should be included in the batch
	_variant_t PackageChargeRefID;	//JMJ 7/17/2005 - for a billed package, tracks the charge ID in the original package (for multi-use packages)
	//_variant_t HasDiscountCategory; // (j.gruber 2009-03-05 11:00) - PLID 33351 - removed to separate dialog // (j.gruber 2007-03-22 11:51) - PLID 24870 - adding discount categories
	//_variant_t DiscountCategoryID;  // (j.gruber 2009-03-05 11:00) - PLID 33351 - removed to separate dialog // (j.gruber 2007-03-22 11:51) - PLID 24870 - adding discount categories
	//_variant_t CustomDiscountDescription; // (j.gruber 2009-03-05 11:00) - PLID 33351 - removed to separate dialog // (j.gruber 2007-03-22 11:51) - PLID 24870 - adding discount categories
	//_variant_t CouponID; // (j.gruber 2009-03-05 11:00) - PLID 33351 - removed to separate dialog // (j.gruber 2007-04-03 17:27) - PLID 9796 - adding support for coupons
	_variant_t PointsUsed; // (a.walling 2007-05-24 09:30) - PLID 26114 - Points used for this charge
	// (r.gonet 2015-03-25 17:22) - PLID 65277 - Add the Value column to bills, for Gift Certificates only. (New column, show/hide)
	_variant_t Value;
	_variant_t HasAllowable; // (j.gruber 2007-08-21 10:33) - PLID 25846 - check allowable
	// (j.jones 2010-09-02 09:38) - PLID 40330 - changed Allowable to UnitAllowable, which is the Allowable for the charge
	// and what is used on the quote, TotalAllowable is multiplied by quantity & modifiers, and is used on the bill
	_variant_t UnitAllowable;
	_variant_t TotalAllowable;
	_variant_t NDCCode;	// (j.jones 2008-05-28 11:26) - PLID 28782 - track the NDC code per charge
	_variant_t EMRChargeID;		// (j.jones 2008-06-04 15:00) - PLID 30256 - tracks EMRChargesT.ID
	_variant_t AppointmentID;	// (j.jones 2008-06-24 10:36) - PLID 30457 - tracks AppointmentsT.ID
	_variant_t TotalDiscount;  // (j.gruber 2009-03-05 11:00) - PLID 33351 - added field
	// (j.jones 2009-08-12 18:15) - PLID 35206 - added more drug fields
	_variant_t DrugUnitPrice;
	_variant_t DrugUnitType;
	_variant_t DrugUnitQuantity;
	_variant_t PrescriptionNumber;
	// (j.jones 2010-04-08 11:51) - PLID 15224 - added IsEmergency
	_variant_t IsEmergency;
	//TES 6/29/2011 - PLID 44192 - Added GlassesOrderServiceID
	_variant_t GlassesOrderServiceID;
	// (r.gonet 07/08/2014) - PLID 62569 - The ID of the line item that voided the original charge
	_variant_t VoidingLineItemID;
	// (r.gonet 07/08/2014) - PLID 62569 - The ID of the line item that was the original charge
	_variant_t OriginalLineItemID;
	// (j.jones 2011-08-24 09:51) - PLID 44873 - added IsVoidingCharge, a boolean
	// that is true only if the ChargeID is in LineItemCorrectionsT.VoidingLineItemID
	_variant_t IsVoidingCharge;
	// (j.jones 2011-08-24 08:41) - PLID 44868 - added IsOriginalCharge, a boolean
	// that is true only if the ChargeID is in LineItemCorrectionsT.OriginalLineItemID
	_variant_t IsOriginalCharge;
	// (r.gonet 07/08/2014) - PLID 62569 - Whether the charge is a new charge from a correction
	_variant_t IsNewChargeFromCorrection;
	// (j.jones 2013-08-20 10:57) - PLID 57959 - added boolean to track whether
	// the service code changed on this charge, it only persists until it is saved
	_variant_t HasServiceCodeChanged;
	// (r.gonet 07/07/2014) - PLID 62569 - Added a boolean On Hold column to track whether the charge is on hold or not. Not persisted after saving.
	_variant_t OnHold;
	// (d.lange 2015-11-18 10:44) - PLID 67128 - The Insurance Co ID used to calculate the allowable
	_variant_t AllowableInsuranceCoID;

	//new - removed Ins1Resp, Ins2Resp, Ins3Resp
	RPCList* RPCList;
	// (r.gonet 08/05/2014) - PLID 63098 - Tracks the BillLabTestCodesT.ID values linked with this charge through ChargeLabTestCodesT
	std::vector<long> TestCodeList;

	// (j.gruber 2014-02-19 09:31) - PLID 60878 - store a map for whichcodes
	CChargeWhichCodesMapPtr whichCodes;

	// (j.gruber 2009-03-05 12:29) - PLID 33351 new Discount Structure
	DiscountList * DiscountList;

	// (d.singleton 2012-03-28 14:38) - PLID 49257
	CArray<UnsavedChargeNote, UnsavedChargeNote> m_arUnsavedChargeNotes;
	
private:
	BillingItem() {

		// (j.jones 2015-03-18 15:01) - PLID 64974 - ensured the category values default to null
		CPTCategoryID = g_cvarNull;
		CPTCategoryCount = (long)0; // (s.tullis 2015-04-07 16:54) - PLID 64975 - Added Category Count for Showing/Hiding/ Disabling category column later
		CPTSubCategoryID = g_cvarNull;
		UnitCost.vt = VT_NULL;
		OthrUnitCost.vt = VT_NULL;
		// (r.gonet 2015-03-25 17:22) - PLID 65277 - Add the Value column to bills, for Gift Certificates only. (New column, show/hide)
		Value.vt = VT_NULL;
		RPCList = NULL;
		DiscountList = NULL;
		PointsUsed = COleCurrency(0, 0);
		// (j.jones 2009-06-04 10:47) - PLID 34435 - default to NULL
		NDCCode = g_cvarNull;
		// (j.jones 2009-08-12 18:15) - PLID 35206 - added more drug fields
		DrugUnitPrice = COleCurrency(0,0);
		DrugUnitType = (LPCTSTR)"";
		DrugUnitQuantity = (double)0.0;
		PrescriptionNumber = (LPCTSTR)"";
		// (r.gonet 07/08/2014) - PLID 62569 - By default, charges will not be corrected. So set this to null.
		VoidingLineItemID = g_cvarNull;
		// (r.gonet 07/08/2014) - PLID 62569 - By default, charges will not be corrected. So set this to null.
		OriginalLineItemID = g_cvarNull;
		// (j.jones 2011-08-24 09:51) - PLID 44873 - added IsVoidingCharge, a boolean
		// that is true only if the ChargeID is in LineItemCorrectionsT.VoidingLineItemID
		IsVoidingCharge = g_cvarFalse;
		// (j.jones 2011-08-24 08:41) - PLID 44868 - added IsOriginalCharge, a boolean
		// that is true only if the ChargeID is in LineItemCorrectionsT.OriginalLineItemID
		IsOriginalCharge = g_cvarFalse;
		// (r.gonet 07/08/2014) - PLID 62569 - Charges will be original to begin with.
		IsNewChargeFromCorrection = g_cvarFalse;

		// (j.dinatale 2012-06-15 18:53) - PLID 51023 - initialize this to g_cvarNull because it is possible for places not to initialize it.
		GlassesOrderServiceID = g_cvarNull;

		// (j.jones 2013-08-20 10:57) - PLID 57959 - added boolean to track whether
		// the service code changed on this charge, it only persists until it is saved
		HasServiceCodeChanged = g_cvarFalse;

		// (j.jones 2014-04-22 16:41) - PLID 61836 - added referring, ordering, supervising providers
		ReferringProviderID = g_cvarNull;
		ReferringProviderIDRequired = g_cvarFalse;
		OrderingProviderID = g_cvarNull;
		OrderingProviderIDRequired = g_cvarFalse;
		SupervisingProviderID = g_cvarNull;
		SupervisingProviderIDRequired = g_cvarFalse;
		// (r.gonet 07/07/2014) - PLID 62569 - Initialize the charge to be not on-hold
		OnHold = g_cvarFalse;
	}

	~BillingItem() {

		// (j.jones 2009-06-04 10:46) - PLID 34480 - clean up memory
		if(DiscountList) {
			delete DiscountList;
		}

		if(RPCList) {
			delete RPCList;
		}
	}

protected:
	static void ProtectedDeleter(BillingItem* pItem)
	{ delete pItem; }

	// (a.walling 2014-02-24 11:25) - PLID 61003 - Private delete and construct ensures everything uses the smart pointer as the only interface to object construction and destruction
public:
	static BillingItemPtr Create()
	{ return BillingItemPtr(new BillingItem, &BillingItem::ProtectedDeleter); }
};



struct ProductItems {
	long ProductItemID;
	int SaveStatus; //uses CPI_ info
};

struct ChargedProductItemList {
	long ID;
	long ChargeID;
	CArray<ProductItems*,ProductItems*> ProductItemAry;
};

// (j.jones 2007-12-10 15:57) - PLID 27988 - used for caching allocation detail information
struct AllocationItems {
	long nAllocationDetailID;
	long nSaveStatus; //uses CPI_ defines
};

struct ChargedAllocationDetailList {
	
	long nID;
	long nChargeID;
	CArray<AllocationItems*,AllocationItems*> paryAllocationItems;
};

struct BilledQuote {
	long nQuoteID;
	BOOL bDeleteQuote;
};

// (j.jones 2007-07-03 16:57) - PLID 26098 - used for service / modifier linking
struct ServiceModifierLink {

	long nServiceID;
	CString strModifier;
	double dblMultiplier;
};

// (j.jones 2007-11-15 17:28) - PLID 27988 - used for caching allocation information
struct ProductAllocationInfoCache {
	BOOL bCompleted;			//determines whether we need to save the completion of this allocation
	long nProductID;			//a product ID that's in an allocation for this patient
	long nAllocationID;			//the allocation ID
	long nAllocationLocationID; //the allocation's location ID
	CString strLocationName;	//the location's name
	BOOL bExistsInMultiple;		//TRUE if the product exists in multiple allocations for the same patient / location
};

// (j.jones 2007-12-13 14:30) - PLID 27988 - used to determine whether we are billing an allocation
enum ProductAllocationReturnValue {
	
	arvAddNormally = 0,		//will be returned if a product should be billed normally
							//(either doesn't exist in an allocation, or the user declined to use it)
	arvUsingAllocation,		//will be returned if we are adding the allocation product
							//(it exists in an allocation and the user agreed to use it)
	arvAbortAdding,			//will be returned if we should cancel adding the allocation product
							//(likely if it exists, they didn't use it, and wished to cancel adding)
};

#define CPI_SAVENEW	1
#define	CPI_DELETE	2
#define	CPI_NONE	3

#define SCR_ABORT_SAVE	-1
#define SCR_NOT_SAVED	 0
#define SCR_SAVED		 1
#include "client.h"

/////////////////////////////////////////////////////////////////////////////
// CBillingDlg dialog

// (a.walling 2014-02-25 13:34) - PLID 61024 - Removed ancient Access error 3048 handling and nonexistent datalist Exception event

class CBillingDlg : public CNxDialog
{
//Enumerations
public:
	// (r.gonet 07/01/2014) - PLID 62531 - Enumeration for the m_pBillStatusCombo datalist columns.
	enum class EBillStatusComboColumns {
		ID = 0,
		Name,
		Type,
		Inactive,
		Custom,
	};

	// (r.gonet 07/01/2014) - PLID 62531 - Enumeration for the m_pBillStatusNoteCombo datalist columns.
	enum class EBillStatusNoteComboColumns {
		ID = 0,
		Text,
	};

// Construction
public:
	CBillingDlg(CWnd* pParent, UINT nIDTemplate);   // standard constructor
	~CBillingDlg();

private:
	scoped_ptr<class CBillingDiagSearchDlg> m_pBillingDiagSearch;	// (j.armen 2014-08-06 10:06) - PLID 63161

public:
	void SetStartingFocus();

private:
	void EndOfTabSequence();	// (j.armen 2014-08-12 09:39) - PLID 63334
	void BeginningOfTabSequence();	// (j.armen 2014-08-14 14:20) - PLID 63334
	bool VerifyChargeCodeOrder(std::pair<long, long> pair, long nOrderOffset); // (j.armen 2014-08-13 11:14) - PLID 63352

public:
	// (r.gonet 06/30/2014) - PLID 62531 - Bill status controls to assign a status to a bill.
	CNxStatic m_nxstaticBillStatus;
	NXDATALIST2Lib::_DNxDataListPtr m_pBillStatusCombo;
	// (r.gonet 06/30/2014) - PLID 62533 - ... button opens a dialog to edit the available bill statuses.
	CNxIconButton m_btnBillStatusConfig;

	// (r.gonet 06/30/2014) - PLID 62523 - Bill status note combo to choose a pre-made status note for the bill.
	CNxStatic m_nxstaticBillStatusNote;
	NXDATALIST2Lib::_DNxDataListPtr m_pBillStatusNoteCombo;
	// (r.gonet 06/30/2014) - PLID 62520 - ... button opens a dialog to edit the available bill status notes.
	CNxIconButton m_btnBillStatusNoteConfig;
	// (r.gonet 07/01/2014) - PLID 62524 - Bill status note edit box to let the user type status notes into and receive the
	// selection of the bill status note combo.
	CNxEdit m_nxeditBillStatusNote;

	// (d.singleton 2014-02-27 17:20) - PLID 61072 - clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
	long m_nBillToInsPartyID = -1;
	
	// (d.singleton 2012-03-07 17:49) - PLID 49100 added icons for the new validation column
	// (d.singleton 2012-03-22 15:32) - PLID 49136 added icon for the notes column
	HANDLE m_hIconCheck, m_hIconRedX, m_hIconWarning, m_hIconNotes, m_hIconHasNotes;

	// (a.walling 2014-02-24 11:25) - PLID 61003 - CPtrArray g_aryBillingTabInfoT in CBillingDlg et al should instead be a typed collection: vector<BillingItemPtr> m_billingItems.
	std::vector<BillingItemPtr> m_billingItems;
	std::vector<BillingItemPtr>& GetBillingItems()
	{ return m_billingItems; }

	//recursive functions to determine which column to edit next when you tab through the charge list
	int GetNextBillColumn(int nCol, int CurrentColumn, BOOL bIsShiftDown);
	int GetNextQuoteColumn(int nCol, int CurrentColumn, BOOL bIsShiftDown);

	long m_nPatientID;
	// (a.walling 2008-05-05 13:15) - PLID 29897 - Patient name
	CString GetBillPatientName();

	// (z.manning 2008-09-30 17:28) - PLID 31126 - Now returns a billing item pointer
	//TES 10/21/2008 - PLID 21432 - Added a nDefaultProviderID parameter, will override any other settings.
	// (j.gruber 2014-02-19 13:07) - PLID 60878 - changed the whichCodes Parameter
	// (r.farnworth 2014-12-11 08:58) - PLID 64163 - Add cyAmount parameter
	// (b.eyers 2015-06-23) - PLID 66208 - set ordering provider from hl7  
	BillingItemPtr AddNewChargeToBill(long nServiceID, COleCurrency cyAmount = g_ccyInvalid, double dblQuantity = -1.0, CChargeWhichCodesMapPtr pWhichCodes = CChargeWhichCodesMapPtr(),
		const CString &strMod1 = "", const CString &strMod2 = "", const CString &strMod3 = "", const CString &strMod4 = "",
		COleDateTime * pdtService = NULL, long nDefaultProviderID = -1, BOOL bHL7Bill = FALSE, long nOrdProvider = -1); // (v.maida 2014-12-26 09:24) - PLID 64470 - Add bHL7Bill parameter

	void SetAllChargesToBeUnsaved();

	CDWordArray m_aBillColumnStyles, m_aQuoteColumnStyles;

	CTableChecker m_DiagCodeChecker, m_CPTCodeChecker, 
		m_POSChecker, m_POSDesigChecker, m_ProviderChecker,
		m_ModiferChecker, m_ProductChecker, m_SurgeryChecker, m_PatCoordChecker;
	// (j.jones 2014-05-13 09:08) - PLID 61837 - added tablechecker for the charge claim provider setup
	CTableChecker m_ClaimProviderChecker;
	// (r.gonet 06/30/2014) - PLID 62531 - Added tablechecker for the Bill Status
	CTableChecker m_BillStatusChecker;
	// (r.gonet 06/30/2014) - PLID 62523 - Added tablechecker for the Bill Status Note
	CTableChecker m_BillStatusNoteChecker;

	//stores the ID and deletion status of the currently billed quote(s)
	CPtrArray m_arypBilledQuotes;

	// (j.jones 2009-08-06 10:26) - PLID 35120 - stores the IDs of the currently billed case histories
	// (j.jones 2009-08-12 15:32) - PLID 35179 - converted to an array of objects, for auditing
	CArray<CaseHistoryInfo*, CaseHistoryInfo*> m_arypBilledCaseHistories;

	// (j.jones 2009-08-11 16:32) - PLID 35142 - stores the IDs of the linked case histories when we opened the bill
	// (j.jones 2009-08-12 15:32) - PLID 35179 - converted to an array of objects, for auditing
	CArray<CaseHistoryInfo*, CaseHistoryInfo*> m_arypOldBilledCaseHistories;

	// (j.jones 2009-08-12 15:32) - PLID 35179 - added functions to clear our case history arrays
	void ClearCaseHistoryArray();
	void ClearOldCaseHistoryArray();

	//stores the ID of the currently billed EMR
	// (j.jones 2012-01-19 12:24) - PLID 47653 - this now tracks the EMNID, the insured party ID
	// that the EMN was *originally* added to the bill under, and the resp. name
	CArray<BilledEMNInfo,BilledEMNInfo &> m_aryBilledEMNInfo;

	// (j.dinatale 2012-03-20 10:33) - PLID 48893 - we need to keep track of selections made from our optical orders
	typedef std::set<long> OpticalOrderSet;
	OpticalOrderSet m_setBilledOpticalOrderIDs;

	//used if billing a package
	long m_RepeatPackageUses = 0;
	BOOL m_bIsABilledPackage = FALSE;
	BOOL m_bPaymentPlan = FALSE;
	long m_nPackageType = -1;
	long m_nPackageID = -1;

	// (j.jones 2015-02-24 08:57) - PLID 57494 - changed these from 'changed'
	// booleans to the actual count, so we only check legitimate changes
	long m_nLastPackageTotalCount = 0;
	long m_nLastPackageCount = 0;

	// (j.jones 2009-12-23 11:33) - PLID 32587 - made the original amount fields editable
	BOOL m_bPackageOriginalAmountChanged = FALSE;
	BOOL m_bPackageOriginalCountChanged = FALSE;

	//if the order of charges has been manually changed
	BOOL m_bOrderChanged;

	// Recordset that usually points to a list of
	// charges
	ADODB::_RecordsetPtr m_rsBill;

	// Recordset that contains information on doctors
	// and providers.
	ADODB::_RecordsetPtr m_rsDoctors;

	// Quote notes edit box
	CNxEdit* m_peditQuoteNotes;

	// Bill description edit box
	CNxEdit* m_peditBillDescription;

	// Bill date edit box
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	CDateTimePicker* m_peditBillDate;

	// Parent window
	// (r.gonet 07/02/2014) - PLID 62567 - Initialize to NULL
	CBillingModuleDlg* m_pBillingModuleWnd = NULL;

	// Points to the active listbox (Bill or quote listbox)
	// (j.jones 2011-10-04 08:48) - PLID 45799 - converted these three lists to a DL2
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	NXDATALIST2Lib::_DNxDataListPtr m_List;
	NXDATALIST2Lib::_DNxDataListPtr	m_QuoteList;

	NXDATALISTLib::_DNxDataListPtr	m_CPTCombo;
	NXDATALISTLib::_DNxDataListPtr	m_QuotesCombo;
	NXDATALISTLib::_DNxDataListPtr	m_CaseHistoryCombo;
	NXDATALISTLib::_DNxDataListPtr	m_SrgyCombo;
	NXDATALISTLib::_DNxDataListPtr	m_ProductsCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_EMRCombo;
	NXDATALISTLib::_DNxDataListPtr	m_PlaceOfServiceCombo;
	NXDATALISTLib::_DNxDataListPtr	m_DesignationCombo;
	NXDATALISTLib::_DNxDataListPtr m_LocationCombo;
	NXDATALISTLib::_DNxDataListPtr m_listBillTo;
	NXDATALISTLib::_DNxDataListPtr m_WhatToAddCombo;
	NXDATALISTLib::_DNxDataListPtr m_GiftCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_AppointmentCombo;	// (j.jones 2008-06-20 10:31) - PLID 26153
	NXDATALIST2Lib::_DNxDataListPtr m_GlassesOrderCombo; //TES 4/13/2011 - PLID 43249

	// (j.jones 2009-03-20 09:29) - PLID 9729 - added array of additional DiagCode IDs
	// (b.spivey March 10th, 2014) - PLID 60980 - renamed to help with code readability and track down areas to refactor. 
	// (j.armen 2014-08-07 11:03) - PLID 63227 - Use MFCArray	
	MFCArray<DiagCodeInfoPtr> m_arypDiagCodes;
	void ClearDiagCodesList();

	// (j.jones 2009-03-25 10:02) - PLID 33653 - supported auditing for extra diag codes
	// (b.spivey, February 26, 2014) - PLID 60975 -  renamed to help with code readability and track down areas to refactor. 
	// (j.armen 2014-08-07 11:03) - PLID 63227 - Use MFCArray
	MFCArray<DiagCodeInfoPtr> m_arypOldDiagCodes;
	void ClearOldDiagCodesList();

	// (j.jones 2009-03-24 10:11) - PLID 9729 - adds one of the 4 diag codes to a given list,
	// returns TRUE if the order index matches our code index, false if a mismatch
	// (j.gruber 2014-03-21 12:59) - PLID 61494 - no longer used - removed
	//BOOL AddDiagCodeToTrackedList(long nDiagCodeIndex, long nDiagCodeID, NXDATALISTLib::_DNxDataListPtr &pDiagCodeCombo, CArray<DiagCodeInfoPtr, DiagCodeInfoPtr> &aryDiagCodes);
	// (j.jones 2009-03-24 13:48) - PLID 9729 - this function will generate a comma-delimited string
	// of the diag codes in m_arypMoreDiagCodes, in order, and display them on the screen
	void UpdateDiagCodeList();

	// (j.gruber 2014-03-24 14:03) - PLID 61529
	void OutputInsuranceDiagCodeWarning(long nInsCoCode);
	BOOL OutputInsuranceDiagCodeWarning(long nInsCoID, const std::vector<long> &aryDiagIDs, BOOL bWantResponse);

	// (j.jones 2009-03-24 17:37) - PLID 9729 - this function will load extra diag codes and display them on the screen
	void LoadDiagnosisCodes();
	//(b.spivey - February 26, 2014) PLID 60994 - functions to load the default diag codes into our array, and to update G2 settings based on a preference.
	void LoadDefaultDiagnosisCodes();
	//(b.spivey - March 17th, 2014) PLID 61396 - Function to update G2 default diag codes.  
	void UpdateG2DefaultDiagnosisCodes(bool bPromptToSave);

	// Array that contains list of edited charges
	// by their Line ID's
	CDWordArray m_adwEditedCharges;

	// Array of ChargedProductItemLists
	CArray<ChargedProductItemList*,ChargedProductItemList*> m_aryChargedProductItems;

	void AddToChargedProductItemsArray(long ChargeID, long ChargedProductItemListID, CDWordArray &adwProductItemIDs);
	long LoadIntoChargedProductItemsArray(long ChargeID);
	void SaveChargedProductItemsArray(long ChargeID, long ChargedProductItemListID);
	void DeleteOneFromChargedProductItemsArray(long ChargedProductItemListID);
	void DeleteAllFromChargedProductItemsArray();
	CString GetProductItemWhereClause();
	BOOL ChangeProductItems(double &dblQuantity);

	long NewChargedProductItemListID();

	// (j.jones 2007-12-10 14:57) - PLID 27988 - supported ChargedAllocationDetailListID
	CArray<ChargedAllocationDetailList*, ChargedAllocationDetailList*> m_aryChargedAllocationDetails;
	void AddToChargedAllocationDetailsArray(long nChargeID, long nChargedAllocationDetailListID, CDWordArray &adwAllocationDetailIDs);
	long LoadIntoChargedAllocationDetailsArray(long nChargeID);
	void SaveChargedAllocationDetailsArray(long nChargeID, long nChargedAllocationDetailListID);
	void DeleteOneFromChargedAllocationDetailsArray(long nChargedAllocationDetailListID);
	void DeleteAllFromChargedAllocationDetailsArray();
	long NewChargedAllocationDetailListID();

	void SetChargeComboFocus();

	// Flag that determines if user has read-only
	// access
	// (j.jones 2011-01-21 09:58) - PLID 42156 - changed to an enum
	enum BillingAccessType m_eHasAccess;

	//Flags that determine that the combos have been queried
	inline const bool ShouldShowDiagExt() const { return !!GetRemotePropertyInt("IsShowDiagnosisCodeInDiagCsColumn", 0, 0, "<None>", true); };
	BOOL m_bCPTCombo;
	BOOL m_bSurgeryCombo;
	BOOL m_bProductsCombo;
	BOOL m_bQuotesCombo;
	BOOL m_bCaseHistoryCombo;
	BOOL m_bEMRCombo;
	BOOL m_bGiftCombo;
	BOOL m_bRefCombo;  //on the insurance tab
	BOOL m_bAppointmentCombo;	// (j.jones 2008-06-20 10:31) - PLID 26153
	BOOL m_bGlassesOrderCombo = FALSE;	//TES 4/13/2011 - PLID 43249

	inline const OLE_COLOR GetNxColor() const { return (m_EntryType == 1) ? 0xFFC0C0 : RGB(255, 179, 128); }

	enum BillEntryType m_EntryType;	// (j.armen 2014-08-06 10:06) - PLID 63161 - Use an enum

	// (j.jones 2007-11-15 15:05) - PLID 27988 - reworked this field,
	// which indicates that barcoding should be temporarily disabled,
	// so that it has reference counting
	long m_nDisableBarcodeMutex = 0;
	void DisableBarcoding();
	void EnableBarcoding();
	BOOL IsBarcodingDisabled();

	void CalcExpireDate();

	//adds a charge of a given amount to the bill
	void AddPaymentPlanCharge(COleCurrency cyAmount);

	// Adds a charge to the modified list of charges.
	// The modified list of charges in an array that
	// stores the ChargeID's of charges that were modified,
	// -not- created.
	// (j.jones 2011-08-24 08:41) - PLID 44868 - Added boolean for
	// bAddEvenIfCorrected, which should only be TRUE if the charge
	// we added to the modified list is permitted to save changes
	// even if it is "Original" or "Void", which is typically read-only.
	void AddToModifiedList(long ChargeID, BOOL bAddEvenIfCorrected = FALSE);

	void DeleteFromModifiedList(long ChargeID);
	
	void DeleteChargeFromList(long nLineID);

	// Sets the current bill ID. This should only be
	// called when making a new bill.
	void SetBillID(long BillID);

	// Gets the bill ID from the parent
	long GetBillID();

	// (r.gonet 07/09/2014) - PLID 62834 - Reloads the current bill
	void ReloadCurrentBill();
	// Called on OnShowWindow() to fill the list
	// of charges
	// (r.gonet 08/01/2014) - PLID 62834 - Added bReloadingCurrentBill, which should be
	// true if, and only if, the current, open bill is being reloaded while the billing dialog is open.
	void FillTabInfo(bool bReloadingCurrentBill = false);

	// Called to disable/enable windows depending on
	// the state of m_HasAccess.
	void ChangeAccess();

	// Saves all changes made to the charges of a bill.
	// If the bill is non-existed (The Bill ID is -2),
	// then a new bill is created. Returns SCR_ABORT_SAVE, SCR_NOT_SAVED, or SCR_SAVED
	int SaveChanges();

	// (a.walling 2014-02-25 17:59) - PLID 61031 - Refactor BillingDlg::SaveChanges - Break up into major component functions
	struct SaveInfo;

	int SaveChanges_Validate(SaveInfo& info);
	int SaveChanges_ValidateCPTICD9Link(SaveInfo& info);
	int SaveChanges_ValidateConditionDates(SaveInfo& info);
	int SaveChanges_G2DiagCodes(SaveInfo& info);
	int SaveChanges_TrackedAllocations(SaveInfo& info);
	void SaveChanges_BilledQuotes(SaveInfo& info);
	void SaveChanges_CaseHistories(SaveInfo& info);
	void SaveChanges_BilledEMNs(SaveInfo& info);
	void SaveChanges_DecrementPackageCount(SaveInfo& info);
	void SaveChanges_DeleteCharges(SaveInfo& info);
	void SaveChanges_Charges(SaveInfo& info);
	void SaveChanges_InventoryTodoAlarms(SaveInfo& info);
	void SaveChanges_Bill(SaveInfo& info);
	void SaveChanges_RewardPoints(SaveInfo& info);
	void SaveChanges_Tracking(SaveInfo& info);
	void SaveChanges_SendPatBalTableChecker(SaveInfo& info);
	void SaveChanges_Payments(SaveInfo& info);
	void SaveChanges_DeleteBilledQuotes(SaveInfo& info);


	// If the HCFA button was pressed, go through each new charge and
	// see if the unit cost is inconsistent with that defined by the
	// insurance company. If not, warn the user.
//	void WarnDifferentMultiFees();

	// (r.gonet 07/08/2014) - PLID 62569 - Puts a charge on hold and cascades this to the related line item corrections
	// - nLineID: Line the charge is on
	// - bOnHold: True to put the charge and its correction group on hold. False to remove a hold from the charge and its correction group.
	void CascadeOnHold(long nLineID, BOOL bOnHold);

	// builds the source string for the DiagCodes dropdown
	void BuildWhichCodesCombo(BOOL AutoBuild = FALSE);
	// (j.jones 2009-03-25 11:33) - PLID 9729 - added a recursive function to build this list
	void BuildDiagCsContent(CArray<long, long> &aryDiagIndexes, CString &strComboBoxText, CStringArray &aryList, int iLastIndex, CString strLastText);
	
	// (j.jones 2011-10-04 09:20) - PLID 45799 - changed charge lists to a DL2
	void OnLeftClickList(LPDISPATCH lpRow, long iColumn);
	void OnLeftClickQuoteList(LPDISPATCH lpRow, long iColumn);

	// (j.gruber 2007-03-27 11:53) - PLID 24870 - popup the category list
	// (a.wetta 2007-05-23 09:18) - PLID 26104 - Add the option to not show coupons
	// (j.gruber 2009-03-06 12:59) - PLID 33351 - take these functions out
	//long ShowDiscountCategoryList(long nRow, BOOL bIsBill, BOOL bShowCoupons = TRUE);
	//void ShowDiscountCategoryList(BillingItemPtr pItem, BOOL bIsBill, BOOL bShowCoupons = TRUE);
	// (j.gruber 2007-03-29 12:40) - PLID 25351 - added function
	//void CheckApplyDiscountThroughout(BillingItemPtr pOrigItem, BOOL bIsBill);	

	BOOL CheckForBlockedICD9Codes();
	// (j.jones 2013-04-10 11:39) - PLID 56179 - renamed to say services, since it can
	// be CPT codes or products now
	BOOL CheckForBlockedServices();
	BOOL CheckForLinkedServices();

	// (j.gruber 2014-02-28 11:40) - PLID 61078
	CString GetItemDescriptionForWarning(BillingItemPtr pItem, BOOL bAllowLinking);
	CString FormatDiagString(std::vector<std::pair<long, CString>> aryDiags);


	// Variables set in the initialization used in some places
	// throughout the module
	long m_Main_Physician = -1;
	long m_GuarantorID1 = -1;
	long m_GuarantorID2 = -1;

	// List of charges to remove from this bill when SaveChanges()
	// is called. The list is made up of Charge ID's
	CDWordArray m_adwDeletedCharges;

	// Total of the bill
	COleCurrency m_cyBillTotal;

	// Temporary bound item
	COleVariant m_varBoundItem;

	// Default date for charges
	COleDateTime m_cyDefaultChargeDate;

	// Default provider for charges
	long m_DefaultProvider;

	// (j.jones 2010-11-09 10:42) - PLID 31392 - added claim provider
	long m_DefaultClaimProvider;

	long GetDefaultBillProviderID();

	// Temporary column that was clicked on
	int m_iActiveColumn;

	// True if any charges were added or modified
	BOOL m_boChangesMade = FALSE;

	// True if OnShowWindow was called at least once
	BOOL m_boInitialized = FALSE;

	// True if we want to give the visible list focus after
	// we pressed tab
	BOOL m_boSetListFocus = FALSE;

	// (j.jones 2011-07-08 13:56) - PLID 26785 - added m_bSavedNewPackage
	BOOL m_bSavedNewPackage = FALSE;

	// Tax rate as determined by the practice table
	double m_fltPracticeTax1;
	double m_fltPracticeTax2;

	//when the default tax rates have changed, offer to update any taxed charges with the new rate
	void PromptUpdateTaxRates(BOOL bPOSChanged);

	//store the currently selected location and POS
	long m_nCurLocationID = -1;
	long m_nCurPlaceOfServiceID = -1;

	//TES 11/2/2006 - PLID 23314 - In OnShowWindow(), if this is a new bill, this bill's location and POS will be set to
	// this location, overriding any preferences.
	//TES 12/4/2005 - PLID 23732 - Split this into two separate variables, the location and POS can now be overriden separately.
	long m_nOverrideLocationID = -1;
	long m_nOverridePOSID = -1;


	//when the location is changed, prompt the user if any products are out of stock for that location
	void PromptInventoryLocationChanged(long nLocationID);

	// Number of insured charges (disabled for now)
	int m_nInsuredCharges;

	// (j.jones 2008-06-24 14:30) - PLID 30458 - added nAppointmentID parameter
	// (j.jones 2012-01-17 16:18) - PLID 47537 - Added EMNID as a parameter, and also
	// renamed from OnBillPackage as it was poorly named.
	// This function actually handles billing any quote or any package.
	void OnBillQuoteOrPackage(long QuoteID, long nAppointmentID = -1, long nEMNID = -1);

	// (j.jones 2008-06-24 14:31) - PLID 30458 - PostSelChosenComboQuote()
	// is used to bill a quote, but separate from the OnSelChosen function
	// (j.jones 2012-01-17 16:18) - PLID 47537 - added EMNID as a parameter
	void PostSelChosenComboQuote(long nQuoteID, long nAppointmentID = -1, long nEMNID = -1);

	// (j.jones 2007-08-10 10:44) - PLID 23769 - when billing a package or quote, tell the
	// OnSelChosenComboQuote function that it came from the message instead of the dropdown
	BOOL m_bQuoteFromPopupWindow = FALSE;

	// (j.dinatale 2012-01-13 17:41) - PLID 47514 - needed a way to determine if we are billing the entire EMR
	void OnBillEMR(long EMRID, long nInsuredPartyID, bool bBillEntireEMR = true);

	// (j.dinatale 2012-01-13 17:36) - PLID 47514 - need to break out the logic for billing an EMR
	void BillEMR(long EMRID, bool bBillEntireEMR);

	// (j.jones 2008-06-24 08:40) - PLID 30455 - added ability to bill from the schedule
	void OnBillAppointment(long nApptID);

	// (j.dinatale 2012-03-15 16:34) - PLID 47413 - added ability to bill from glasses order tab
	void OnBillGlassesOrder(long nGlassesOrderID, long nInsuredPartyID);

	//called from either the Case History or the EMR
	// (j.dinatale 2012-06-05 11:11) - PLID 49713 - add an external InsuredPartyID for optical orders
	BOOL AddNewExternalCpt(long ExternalID, long ExternalType, long nExtInsuredPartyID = -1, CString strInsCoName = "", CString strRespTypeName = "");
	// (j.jones 2007-12-14 13:41) - PLID 27988 - AddNewExternalProduct can also be called from
	// the end of an allocation - it should take in parameters and not be dependent on m_rsBill	
	// (j.dinatale 2012-06-05 11:11) - PLID 49713 - add an external InsuredPartyID for optical orders
	// (j.dinatale 2013-02-20 10:03) - PLID 54698 - need to determine if we should skip the "Do I Have Enough" prompting
	// (j.jones 2013-04-12 13:30) - PLID 56250 - renamed cyUnitCost to cyExternalPrice cost to clarify that it's the cost passed in by the called
	BOOL AddNewExternalProduct(long ExternalID, long ExternalType, long nProductID, CString strProductName, double dblQuantity, COleCurrency cyExternalPrice, long nDiagCount = -1, long nChargedProductItemListID = -1, long nChargedAllocationDetailListID = -1, long nExtInsuredPartyID = -1, CString strInsCoName = "", CString strRespTypeName = "", bool bSkipInvCountCheck = false);

	//sets focus to the m_listBillTo combo
	void SetFocusToBillToCombo();

	// Saves the datalist column widths if the user prefers it.
	void TrySaveListColumnWidths();
	// (d.thompson 2012-09-24) - PLID 52822
	long GetColumnCount(const CString& strColWidths);
	// (d.thompson 2012-09-25) - PLID 52822
	long GetSaveableColumnCount();
	long GetColumnWidth(const CString& strColWidths, short nColumn);

	// (c.haag 2004-01-28 09:24) - Maps that contain columns that
	// should be hidden based on user preference. A value of TRUE
	// means the column should be visible.
	CMap<long, long, BOOL, BOOL> m_mapSysHiddenBillCols;
	CMap<long, long, BOOL, BOOL> m_mapSysHiddenQuoteCols;

	COleDateTime m_dtOldDate;
	BOOL m_bUpdateDate;

	// (c.haag 2008-06-09 16:49) - PLID 27216 - We need these member variables to monitor
	// whether we are in the middle of a left click handler in case we get another one during
	// a discount pop-up.
	BOOL m_bHandlingLeftClickBillList = FALSE;
	BOOL m_bHandlingLeftClickQuoteList = FALSE;

	//(e.lally 2010-10-22) PLID 30253
	BOOL m_bHasOutsideDiscount = FALSE;

	void ToggleChargeBatchColumn(BOOL bShow);

	CString GetAnesthStartTime();
	CString GetAnesthEndTime();
	CString GetFacilityStartTime();
	CString GetFacilityEndTime();
	long GetAnesthMinutes();
	long GetFacilityMinutes();

	void SetAnesthStartTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetAnesthEndTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetFacilityStartTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetFacilityEndTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetAnesthMinutes(long nMinutes, BOOL bUpdateCharges = FALSE);
	void SetFacilityMinutes(long nMinutes, BOOL bUpdateCharges = FALSE);

	void OnAnesthesiaTimeChanged(long nNewMinutes, BOOL bPlaceOfServiceChanged = FALSE);
	void OnFacilityTimeChanged(long nNewMinutes, BOOL bPlaceOfServiceChanged = FALSE);

	// (j.jones 2011-11-01 08:55) - PLID 41558 - added assisting times
	CString GetAssistingStartTime();
	CString GetAssistingEndTime();
	long GetAssistingMinutes();
	void SetAssistingStartTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetAssistingEndTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetAssistingMinutes(long nMinutes, BOOL bUpdateCharges = FALSE);
	void OnAssistingTimeChanged(long nNewMinutes);

	BOOL CheckAllowAddAnesthesiaFacilityCharge(long nServiceID);

	void PostPlaceOfServiceChanged();

	void UpdatePalm();

	void PostChargeAdded();

	// (s.tullis 2014-08-06 12:24) - PLID 62779 -
	void SetAlbertaBillStatus();


	// (s.tullis 2014-08-06 12:24) - PLID 62779 -
	void RefreshBillStatusList();

	// (j.gruber 2014-02-19 10:35) - PLID 60878 - new whichCodes Structure
	void LoadDefaultWhichCodes(CChargeWhichCodesMapPtr &whichCodes);
	void ClearWhichCodesList();
	
	// (j.gruber 2014-02-20 11:02) - PLID 60942 - which codes loading
	void LoadBillWhichCodes(long nBillID);
	void LoadExistingWhichCodes(CChargeWhichCodesMapPtr &whichCodes, long nChargeID);
	void LoadEMRWhichCodes(CChargeWhichCodesMapPtr &whichCodes, long nDiagCount, long nExternalID);
	std::map<long, CChargeWhichCodesMapPtr> m_mapBillWhichCodes;
	boost::shared_ptr<DiagCodeInfo> GetDiagCodeInfoByIndex(long nIndex);
	void SaveToWhichCodesMap(CChargeWhichCodesMapPtr &whichCodes, _variant_t varNewValue);
	 
	// (j.gruber 2014-03-12 09:30) - PLID 60895
	void DisplayWhichCodes(NXDATALIST2Lib::IRowSettingsPtr &pRow, CChargeWhichCodesMapPtr whichCodes);
	CString GetIndexStringFromWhichCodeMap(CChargeWhichCodesMapPtr pWhichCodes);
	
	// (j.gruber 2014-02-26 15:17) - PLID 60898
	CString GetDiagDescription(DiagCodeInfoPtr pDiag);

	// (j.gruber 2014-02-25 13:09) - PLID 60901
	void ShowWhichCodesSelectionDialog(NXDATALIST2Lib::IRowSettingsPtr pRow);
	BillingItemPtr GetBillingItemByLineID(long nLineID);
	BOOL CanChargeBeEdited(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (j.gruber 2014-02-26 11:19) - PLID 61033
	void ReconcileWhichCodes();

	// (j.gruber 2014-02-24 14:55) - PLID 61008 - handler for removing diagcodes
	void RemoveDiagCodeFromWhichCodes(long nDiagCode9ID, long nDiagCode10ID);
	void RemoveDiagCode(long nOrderIndex, long nDiag9CodeID, long nDiag10CodeID);
	DiagCodeInfoPtr GetDiagCodeByPair(CChargeWhichCodePair pair, BOOL bExpectsExistance = TRUE);

	// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
	void ReplaceDiagCode(DiagCodeInfoPtr oldDiagCode, DiagCodeInfoPtr newDiagCode);
	void ReplaceDiagCodeInWhichCodes(DiagCodeInfoPtr oldDiagCode, DiagCodeInfoPtr newDiagCode);

	// (j.gruber 2014-02-24 10:36) - PLID 60893 - save new whichCodes Structure
	void AuditAndGenerateSaveStringForWhichCodes(bool bIsNewCharge, long nChargeID, long nBillID, CChargeWhichCodesMapPtr pWhichCodes, CSqlFragment &sqlWhichCodeSave);
	void GenerateWhichCodesAudit(long nChargeID, CChargeWhichCodesMapPtr pOriginalWhichCodes, CChargeWhichCodesMapPtr pCurrentWhichCodes);
	BOOL HaveWhichCodesChanged(CChargeWhichCodesMapPtr pOriginalWhichCodes, CChargeWhichCodesMapPtr pCurrentWhichCodes);

	// (j.gruber 2014-02-24 15:39) - PLID 61011
	void ReflectDiagCodeArrayToInterface();

	// (j.dinatale 2012-06-05 16:25) - PLID 49713 - clear out the insurance tab
	void ClearInsuranceTabInsPlans();

	// (j.jones 2007-07-03 16:31) - PLID 26098 - added ability to link modifiers to charges
	// when other charges are added
	void CheckApplyServiceModifierLinks();

	// (j.jones 2007-07-03 16:49) - PLID 26098 - used to cache Service Modifier links
	CPtrArray m_pServiceModifierLinks;
	void PopulateServiceModifierLinkArray();

	//(e.lally 2006-08-09) - We need a way to share the column number definitions
		//for places like billing_module_dlg.cpp that also use them.
	int GetServiceIDColumn();

	// (a.walling 2007-05-07 16:48) - PLID 14717 - Add a product to the bill by service ID
	// (j.jones 2008-06-11 12:51) - PLID 28379 - now these return a BillingItem pointer
	//TES 7/1/2008 - PLID 26143 - These functions now take a default quantity.
	//TES 7/16/2008 - PLID 27983 - This now also takes bAddedFromAllocation, which if true, means that the product being added was
	// on an allocation, and the user has already been prompted about that allocation.
	//TES 7/17/2008 - PLID 27983 - Added bMassAdding, set this to true to tell the code to not force all allocations to be completed,
	// it is then the caller's reponsibility to do that once it's finished adding charges.
	// (j.jones 2010-11-23 16:13) - PLID 41549 - added nPackageChargeRefID, optional cyUnitPrice override, and discount info
	// (b.eyers 2015-06-17) - PLID 66206 - added optional parameters: whichcodes, mods, service date, provider
	// (b.eyers 2015-06-23) - PLID 66208 - set ordering provider from hl7  
	BillingItemPtr AddNewProductToBillByServiceID(long nServiceID, double dblQtyDefault = 1.0, BOOL bAutoAddingSerialNumber = FALSE,
		BOOL bAddedFromAllocation = FALSE, bool bMassAdding = false,
		IN COleCurrency *cyUnitPrice = NULL, long nPackageChargeRefID = -1,
		long nLoadDiscountFromChargeID = -1, BOOL bLoadDiscountFromSurgery = FALSE,
		CChargeWhichCodesMapPtr pWhichCodes = CChargeWhichCodesMapPtr(), const CString &strMod1 = "",
		const CString &strMod2 = "", const CString &strMod3 = "", const CString &strMod4 = "",
		COleDateTime * pdtService = NULL, long nDefaultProviderID = -1, BOOL bHL7Bill = FALSE, long nOrdProvider = -1);

	// (j.jones 2008-06-11 16:24) - PLID 28379 - this can take in optional allocation information
	//TES 7/16/2008 - PLID 27983 - You can now pass in -2 for nFromAllocationID, which will tell this function to treat the product
	// as coming from an allocation, but not prompt the user about that allocation (that's the caller's responsibility).
	//TES 7/17/2008 - PLID 27983 - Added bMassAdding, set this to true to tell the code to not force all allocations to be completed,
	// it is then the caller's reponsibility to do that once it's finished adding charges.
	// (j.jones 2010-11-23 16:13) - PLID 41549 - added nPackageChargeRefID, optional cyUnitPrice override, and discount info
	// (b.eyers 2015-06-17) - PLID 66206 - added optional parameters: whichcodes, mods, service date, provider
	// (b.eyers 2015-06-23) - PLID 66208 - set ordering provider from hl7  
	BillingItemPtr AddNewProductToBill(long iNewRow, BOOL bAutoAddingSerialNum = FALSE,
		long nFromAllocationID = -1, long nFromAllocationDetailID = -1,
		double dblQtyDefault = 1.0, bool bMassAdding = false,
		IN COleCurrency *cyUnitPrice = NULL, long nPackageChargeRefID = -1,
		long nLoadDiscountFromChargeID = -1, BOOL bLoadDiscountFromSurgery = FALSE, 
		CChargeWhichCodesMapPtr pWhichCodes = CChargeWhichCodesMapPtr(), const CString &strMod1 = "",
		const CString &strMod2 = "", const CString &strMod3 = "", const CString &strMod4 = "",
		COleDateTime * pdtService = NULL, long nDefaultProviderID = -1, BOOL bHL7Bill = FALSE, long nOrdProvider = -1);

	// (j.gruber 2007-09-07 10:54) - PLID 25191 - internal handling of onSelChosen
	// (j.politis 2015-07-06 11:36) - PLID 65949 - Getting error when try to open edit a bill has  active location but not managed !
	// if you edit any bill for that loaction ( Dr.John Vine )  will give you the error .
	void ChangeLocationComboSelection(long nLocationID, BOOL bIsCalledInternally);

	//TES 7/1/2008 - PLID 26143 - Call this to tell the dialog to automatically apply these payments to the bill when saving.
	void ApplyPaymentIDs(const CDWordArray &dwaPaymentIDs);
	//TES 7/3/2008 - PLID 26143 - Set what the "Bill A" dropdown should be, the next time a bill is opened.
	inline void SetDefaultBillA(int nBillA) {m_nDefaultBillAOverride = nBillA;}

	// (j.jones 2009-03-19 17:19) - PLID 9729 - added m_btnMoreDiagCodes and m_editMoreDiagCodes
// Dialog Data
	//{{AFX_DATA(CBillingDlg)
	enum { IDD = IDD_BILLING_DLG };
	CNxIconButton	m_nxibEditPOSCodes;
	CNxIconButton	m_nxibEditCodes;
	CNxIconButton	m_nxibApplyDiscountToAll;
	CNxIconButton	m_nxibToggleIndivBatch;
	CNxIconButton	m_nxibShowSuggestedSales;
	// (j.gruber 2009-03-31 17:38) - PLID 33356 - added button
	CNxIconButton	m_nxibAdvancedDiscounting;
	NxButton	m_quoteExpCheck;
	NxButton	m_packageCheck;
	NxButton	m_radioRepeatPackage;
	NxButton	m_radioMultiUsePackage;	
	CNxIconButton	m_filterCodesButton;
	CNxEdit	m_nxeditQuoteExpDays;
	CNxEdit	m_nxeditQuoteExpireDate;
	CNxEdit	m_nxeditPackageTotalCount;
	CNxEdit	m_nxeditPackageCount;
	CNxEdit	m_nxeditPackageTotalCost;
	CNxEdit	m_nxeditPackageCurrentBalance;
	CNxStatic	m_nxstaticLabelResp;
	CNxStatic	m_nxstaticQuoteExpDayLabel;
	CNxStatic	m_nxstaticQuoteExpireDateLabel;
	CNxStatic	m_nxstaticLabelBilla;
	CNxStatic	m_nxstaticLabelDiag1;
	CNxStatic	m_nxstaticLabelDiag2;
	CNxStatic	m_nxstaticLabelDiag3;
	CNxStatic	m_nxstaticLabelDiag4;
	CNxStatic	m_nxstaticPackageTotalCountLabel;
	CNxStatic	m_nxstaticPackageCountLabel;
	CNxStatic	m_nxstaticPackageTotalCostLabel;
	CNxStatic	m_nxstaticPackageCurrentBalanceLabel;
	CNxStatic	m_nxstaticLabelPos;
	CNxStatic	m_nxstaticLabelDesig;
	CNxStatic	m_nxstaticPaidOutsideLabel;
	CNxStatic	m_nxstaticLabelTotalOutside;
	CNxStatic	m_nxstaticPatientTotalLabel;
	CNxStatic	m_nxstaticLabelTotal;
	CNxStatic	m_nxstaticDiscountTotalLabel;
	CNxStatic	m_nxstaticLabelDiscounts;
	// (j.jones 2009-12-22 14:36) - PLID 32587 - made the original amount fields editable
	CNxStatic	m_nxstaticPackageOriginalCurrentAmountLabel;
	CNxEdit		m_editPackageOriginalCurrentAmount;
	CNxStatic	m_nxstaticPackageOriginalCurrentCountLabel;
	CNxEdit		m_editPackageOriginalCurrentCount;
	NxButton	m_checkPackageShowInitialValues;
	//(e.lally 2010-10-22) PLID 30253
	CNxStatic	m_nxstaticOutsideDiscountTotalLabel;
	CNxStatic	m_nxstaticOutsideLabelDiscounts;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBillingDlg)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);	
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);

	// (j.dinatale 2012-06-25 09:28) - PLID 51138 - show additional charge info
	virtual LRESULT OnShowEditAdditionalChargeInfo(WPARAM wParam, LPARAM lParam);

	// (s.dhole 2011-05-25 16:38) - PLID  
	void ChangeServiceCode( BillingItemPtr pItem,long nServiceCodeID);

	// (d.singleton 2011-12-21 13:50) - PLID 40036 - CList to hold all the service id's with std fee of 0
	CList<long, long> m_lstZeroAmountCodes;
	//}}AFX_VIRTUAL
	
	
// Implementation
public:

	// (j.jones 2011-08-24 08:41) - PLID 44868 - Added boolean for
	// bSaveEvenIfCorrected, which should only be TRUE if the charge
	// we added to the modified list is permitted to save changes
	// even if it is "Original" or "Void", which is typically read-only.
	void SaveAllCharges(BOOL bSaveEvenIfCorrected);

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	void UpdatePatCoordIDs(_variant_t varCoordID);

	BOOL CheckHasInsuranceCharges();

	// (j.jones 2011-08-24 08:41) - PLID 44868 - returns true if any charge
	// is an "original" or "void" charge, and therefore read only
	BOOL HasOriginalOrVoidCharge();

	// (j.jones 2011-08-24 08:41) - PLID 44868 - Returns true if any charge
	// is NOT an "original" or "void" charge, and therefore writeable.
	// nLineIDToSkip is if you only want to check other lines, and skip one you
	// may already be editing
	BOOL HasUncorrectedCharge(long nLineIDToSkip = -1);

	// (j.jones 2011-08-24 08:41) - PLID 44868 - returns true if the given charge
	// is an "original" or "void" charge, and therefore read only
	// (r.gonet 2016-02-06 16:46) - PLID 68193 - Made constant.
	BOOL IsOriginalOrVoidCharge(long nChargeID) const;

	// (r.goldschmidt 2016-03-08 09:34) - PLID 68541 - UB04 Enhancements - get the min and max dates out list of billing items
	void GetMinAndMaxChargeDates(COleDateTime& dtMinDate, COleDateTime& dtMaxDate, bool excludeUnbatched = true, bool excludeOriginalOrVoidCharge = true);

	// (j.jones 2008-05-14 16:51) - PLID 30044 - takes in an array of CStrings for codes,
	// and returns TRUE if any charge on the bill (remember, could be a product) has a
	// matching ItemCode as any item in the given array
	BOOL DoesBillHaveMatchingServiceCode(CStringArray &aryCodes);

	// (a.walling 2007-05-04 11:29) - PLID 14717 - The suggested sales dialog has been closed
	void SetSuggestedSalesHidden();
	void SetSuggestionsExist(BOOL bExists); // colorize the button if suggestions exist

	// (j.jones 2014-05-29 10:57) - PLID 61837 - allows the insurance tab to refresh
	// the charge provider columns
	void TryShowAllChargeProviderColumns();

	// (j.gruber 2007-08-15 09:54) - PLID 27068 - I had to move this so I could use it from the billingmoduledlg
	BOOL GetIsAnyPackage();

	// (j.gruber 2007-08-21 13:03) - PLID 25846 - calculate percent copay based on allowable
	// (j.jones 2010-08-04 11:14) - PLID 38613 - broke this function down into multiple parts,
	// and took in an optional charge ID, if left as -1 we calculate for the whole bill	
	COleCurrency CalculatePercentOfBillTotalWithAllowables(long nPercent, long nFilterByChargeID = -1);
	COleCurrency CalculateBillTotalWithAllowables(long nFilterByChargeID = -1);
	// (j.jones 2012-07-30 14:38) - PLID 47778 - moved to global financial utils
	//COleCurrency CalculatePercentOfAmount(COleCurrency cyAmount, long nPercent);

	// (j.gruber 2007-09-05 16:05) - PLID 27254
	BOOL m_bQuoteHasBeenBilled = FALSE;	

	// (j.jones 2007-11-14 12:47) - PLID 27988 - FreeTrackedAllocations clears the m_paryAllocationInfo array safely,
	// and the m_aryProductAllocationInfo array
	void FreeTrackedAllocations();

	// (z.manning 2008-09-30 17:39) - PLID 31126 - Made this function public so we could call it
	// when creating HL7 bills
	// Calculates the total for the bound line item
	// (j.gruber 2009-03-05 11:13) - PLID 33351 - take out PercentOff and Discount and instead use TotalDiscount
	// (j.jones 2009-12-22 17:04) - PLID 32587 - added original quantity
	void CalculateLineTotal(COleCurrency cyUnitCost, COleCurrency cyOthrCost, double dblQuantity, double dblPackageQtyRemaining, double dblOriginalPackageQtyRemaining, double dblMultiplier1, double dblMultiplier2, double dblMultiplier3, double dblMultiplier4, double dblTax1, double dblTax2, COleCurrency cyTotalDiscount);

	// (d.thompson 2009-08-26) - PLID 33953
	void UpdateAllChargesWithNewServiceDate(_variant_t varNewDate);

	// (z.manning 2010-08-16 10:28) - PLID 40120 - Made this public
	//gets the InsuredPartyID by looking at the BillTo combo
	long GetCurrentBillToInsuredPartyID();

	// (j.jones 2013-07-11 08:53) - PLID 57148 - Retrieves the first charge provider sorted by LineID.
	// If Charge 1 has no provider, and Charge 2 has a provider, we'll return the provider ID on Charge 2.
	long GetFirstChargeProviderID();

	// (j.jones 2008-06-20 11:29) - PLID 26153 - added OnSelChosenComboAppointments
	// (j.jones 2009-03-19 17:57) - PLID 9729 - added OnBtnMoreDiagCodes
	// (j.gruber 2009-03-23 17:02) - PLID 33355 - added menu items for new preference
	// Generated message map functions
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBtnEditCodes();
	afx_msg void OnEditCPT();
	afx_msg void OnEditInventory();

	// (j.jones 2011-10-04 09:20) - PLID 45799 - changed charge lists to a DL2
	afx_msg void OnLButtonDownList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingStartingList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnLButtonDownQuoteList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownQuoteList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedQuoteList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingQuoteList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnDragBeginList(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	afx_msg void OnDragOverCellList(BOOL FAR* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnDragEndList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnDragBeginQuoteList(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	afx_msg void OnDragOverCellQuoteList(BOOL FAR* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnDragEndQuoteList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnLeftClickBillList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLeftClickListQuote(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingStartingQuoteList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);

	afx_msg void OnSelChangedComboDesignation(long nNewSel);
	afx_msg void OnSelChosenComboPlaceofservice(long nNewSel);
	afx_msg void OnSelChosenComboProducts(long nNewSel);
	afx_msg void OnSelChosenComboQuote(long nNewSel);
	afx_msg void OnSelChosenComboSrgy(long nNewSel);
	afx_msg void OnSelChosenComboCpt(long nRow);
	afx_msg void OnSelChosenComboEMR(LPDISPATCH lpRow);
	afx_msg void OnColumnClickingComboCpt(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnColumnClickingComboProducts(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnColumnClickingComboQuote(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnColumnClickingComboSrgy(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnPackageCheck();
	afx_msg void OnPackageTypeChanged();
	afx_msg void OnKillfocusPackageCount();
	afx_msg void OnSelChosenComboLocation(long nRow);
	afx_msg void OnKillfocusPackageCurrentBalance();
	afx_msg void OnKillfocusPackageTotalCost();
	afx_msg void OnKillfocusPackageTotalCount();
	afx_msg void OnChangePackageTotalCount();
	afx_msg void OnChangePackageCount();
	afx_msg void OnQuoteUseExpCheck();
	afx_msg void OnSelChosenComboCaseHistory(long nRow);
	afx_msg void OnRequeryFinishedComboSrgy(short nFlags);
	afx_msg void OnSelChosenComboBillTo(long nRow);
	afx_msg void OnTrySetSelFinishedComboPlaceofservice(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedComboLocation(long nRowEnum, long nFlags);
	afx_msg void OnKillfocusQuoteExpDays();
	afx_msg void OnEditCPTInsNotes();
	afx_msg void OnBtnEditPosCodes();
	afx_msg void OnSelChosenWhatToAddCombo(long nRow);
	afx_msg void OnSelChosenComboGift(long nRow);
	afx_msg void OnLaunchCodeLink();
	afx_msg void OnBtnToggleIndivBatch();	
	afx_msg void OnFilterCodes();
	afx_msg void OnBtnShowSuggestedSales();
	afx_msg void OnBtnApplyDiscountToAll();
	afx_msg void OnSelChangedComboLocation(long nNewSel);
	afx_msg void OnSelChosenComboAppointments(LPDISPATCH lpRow);
	afx_msg void OnBnClickedBtnAdvancedDiscounting();
	afx_msg void OnApplyDiscountToPracticeFeeLinesOnly();
	afx_msg void OnApplyDiscountToAllLines();
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
private:

	//m.hancock PLID 16457 - status of a filter for CPT codes and product lists
	bool m_bCptCodesFilter;
	bool m_bProductsFilter;

	//m.hancock PLID 16457 - member variables to store the default where clause for the CPT codes and products data lists
	//TES 7/16/2008 - PLID 27983 - Quotes and Bills now have slightly different lists of CPT Codes
	CString m_strQuoteCptCodesWhere;
	CString m_strBillCptCodesWhere;
	CString m_strProductsWhere;
	CString m_strDiagCodeWhere;
	
	//m.hancock PLID 16457 - Remove an existing filter on a data list
	void RemoveCodeFilter(NXDATALISTLib::_DNxDataListPtr &pDataListCombo, CNxIconButton *pFilterIcon, CString strDefaultWhereClause);
	
	//m.hancock PLID 16457 - Apply a filter to a data list
	bool ApplyCodeFilter(NXDATALISTLib::_DNxDataListPtr &pDataListCombo, CNxIconButton *pFilterIcon, CString strDefaultWhereClause, int nFieldToFilter, int nKeyField);

	// Sorts the list by the sum of the insurance line
	// total and regular line total
	void SortList();

	//boolean that lets us know if we have prompted them already to change the dates
	bool m_bPromptedChargeDate;
	bool m_bPromptedChargeServiceDateTo;
	bool m_bPromptedChargeInputDate;

	//Checks to see if all the items currently in the list are in the
	//default sort order
	bool IsListInDefaultSortedOrder();

	bool m_bIsInDefaultSortedOrder;

	//inserts a new charge in sorted order
	// (j.jones 2011-10-04 11:40) - PLID 45799 - now takes in a DL2 row
	void InsertRowSorted(NXDATALIST2Lib::IRowSettingsPtr pRowIndex, COleCurrency cyTotal);

	//renumbers the lineIDs of the list
	void ReNumberIDs();

	// Gets the fee for a charge for a particular ins.co. or doctor. If the related
	// CPT code does not exist in the CPT Codes table, the return value
	// is $0.00. If there is no multi-fee for this type of line item,
	// it returns the default unit cost. Otherwise, returns the fee.
	// (j.gruber 2007-08-21 10:24) - PLID 25846  - included returning allowable also
	// (j.jones 2008-11-14 09:08) - PLID 32037 - added optional passed-in value to describe the charge
	// (j.jones 2009-10-23 11:05) - PLID 18558 - this requires PlaceOfServiceID now
	// (b.cardillo 2015-11-03 20:47) - PLID 67121 - Take charge date into account
	COleCurrency GetMultiFee(long ServiceID, long DoctorID, long InsCoID, long LocationID, long PlaceOfServiceID, 
		const COleDateTime &dtServiceDate, BOOL &boMultiFeeExists, BOOL &boWarn,
		CString &strPrompt, BOOL &bHasAllowable, COleCurrency &cyAllowable, OPTIONAL IN CString strChargeInfo = "");

	// Creates a new bill. Only called on SaveChanges() after the user
	// defined the bill contents.
	long CreateNewBill();

	// This function adds or modifies an existing charge
	// This is only called on SaveChanges()
	// (j.gruber 2014-02-26 09:07) - PLID 60893 - add sql for which codes saving
	BOOL ApplyChargeToBill(long LineID, long ChargeID, BOOL bIsNewBill, CSqlFragment &sqFinalWhichCodes);

	// Called when a pop-up menu item in the list was selected
	void OnPopupSelection(long iItemID);

	// Called bu OnRightClickList or OnRightClickQuoteList
	// (j.jones 2011-10-04 09:20) - PLID 45799 - changed charge lists to a DL2
	void OnRightClick(LPDISPATCH lpRow, long iColumn);

	// Sorts the visible listbox, requeries it, and re-calculates
	// the bill total
	// (j.jones 2016-02-26 15:09) - PLID 68339 - added a flag to optionally force a package recalculation
	void Requery(bool bRecalculatePackageTotal = false);

	// (b.spivey, February 27, 2014) - PLID 61081 - Update the diagnosis code array to reflect values in the db. 
	void UpdateDiagnosisCodesReflectDatabase(); 

	// (b.spivey, March 7, 2014) - PLID 61245 - Don't let them add a duplicate combination. 
	long DetectDuplicateDiagnosisCode(DiagCodeInfoPtr pNewDiag); 
	// (j.gruber 2014-03-31 11:03) - PLID 61602 - override for just IDs
	long DetectDuplicateDiagnosisCode(long nDiagCode9ID, long nDiagCode10ID);

	// (j.gruber 2014-03-31 11:03) - PLID 61602 - add ICD-9 to existing ICD-10
	void SetICD9ForDiagCodeInfo(CChargeWhichCodePair pair);
	void UpdateWhichCodes(CChargeWhichCodePair oldPair, DiagCodeInfoPtr pNewInfoPtr);
	// (j.gruber 2014-03-31 12:44) - PLID 61605 - add ICD-10 to existing ICD-9
	void SetICD10ForDiagCodeInfo(CChargeWhichCodePair pair);

	// Only called on loading a bill and applying a previous quote. If
	// IsNew is true, then it MUST be a quote!!! Called to add a charge
	// to the visible listbox.
	// The second two parameters are only used when adding a quote that prompts for productitems.
	// (j.jones 2007-03-26 14:52) - PLID 25287 - supported cyOffsetAmount for the purposes of altering package values
	// (j.jones 2007-12-11 12:01) - PLID 27988 - added nChargedAllocationDetailListID,
	// also added a number of fields that the allocation can change, so we aren't dependent on m_rsBill for them
	// (j.jones 2008-06-24 15:02) - PLID 30458 - added nAppointmentID
	// (j.gruber 2009-03-17 09:39) - PLID 33351 - add a bool if its an error or not
	// (s.dhole 2011-06-13 11:26) - PLID 33666 This function will now return string msg of  Diagcode => pointer changes
	// (r.gonet 08/01/2014) - PLID 62834 - Added varOnHoldOverride, which can be either VT_BOOL or VT_NULL.
	// (s.tullis 2015-04-02 16:20) - PLID 64975 -Added Category Count for hiding/ showing category column
	CString AddChargeToList(BOOL IsNew, long nServiceID, double dblQuantity, CString strDescription, CString strItemCode, long nCategoryID, long nSubCategoryID, BOOL bIsPackage, long nCategoryCount  , long ChargedProductItemListID = -1, long nChargedAllocationDetailListID = -1, long nPackageChargeRefID = -1, COleCurrency cyOffsetAmount = COleCurrency(0, 0), long nAppointmentID = -1, _variant_t varOnHoldOverride = g_cvarNull);

	// Calculates the bill total
	// (j.jones 2015-02-24 08:57) - PLID 57494 - bRecalculatePackageTotal is now a parameter for this function
	void CalculateTotal(bool bRecalculatePackageTotal = false);

	// Sets default values for a newly created list item
	BOOL SetNewRecordDefaults(BillingItemPtr pNew, int& iLineID);

	// Adds a -2 (new and unsaved) charge to the visible list
	// (z.manning 2008-09-30 17:24) - PLID 31126 - Now return a BillingItem pointer
	// (b.eyers 2015-06-23) - PLID 66208 - set ordering provider from hl7  
	BillingItemPtr AppendChargeToList(long nProviderID = -1, BOOL bHL7Bill = FALSE, long nOrdProvider = -1); // (v.maida 2014-12-26 09:24) - PLID 64470 - Add bHL7Bill parameter

	// If the total cost has changed, and the insurance amount equaled
	// the old total cost, this will keep them equivalent after the change.
	BOOL MatchCostToResp(BillingItemPtr pBillingItem, const COleCurrency &cyOldTotal, const COleCurrency &cyNewTotal, const COleCurrency &cyNewTotalNoTax, const _variant_t &varInsResp, long nInsuredPartyID);
	void MatchCostAndInsurance(int record, COleCurrency cyOldTotal, COleCurrency cyNewTotal, COleCurrency cyNewTotalNoTax);

	// Returns the number of inventory items of a type nServiceID that
	// are charged to the bill, but have not been saved to the data.
	double GetUnsavedQuantityCount(long nServiceID);
	// (j.jones 2007-12-18 13:39) - PLID 28037 - searches tracked allocations
	// for quantities marked used, such that we can predict what the new
	// in stock amount will be
	double GetUnsavedAllocationQuantityCount(long nServiceID);

	void FillBillList();
	void FillQuoteList();

	//used to determine if the datalist is drawing
	BOOL m_bIsScreenEnabled;

	//reference count for the functions below
	long m_DisableRefCount;

	// Dialog template
	UINT m_nIDTemplate;

	//will disable/enable drawing of the datalist
	void DisableBillingScreen();
	void EnableBillingScreen();

	//gets the RespTypeID out of the BillTo combo
	long GetCurrentBillToRespID();
	//gets the InsuranceCoID by looking at the BillTo combo
	long GetCurrentBillToInsuranceCoID();
	// (j.jones 2007-02-28 08:46) - PLID 24844 - added to simplify auditing
	CString GetCurrentBillToRespTypeName();
	CString GetCurrentBillToInsuranceCoName();
	// (j.jones 2010-08-17 10:47) - PLID 40135 - added to get the current resp type category
	enum RespCategoryType GetCurrentBillToRespTypeCategoryID();
	// (d.lange 2015-11-19 12:49) - PLID 67127 - Returns the insurance company ID for the most-primary insurance
	// based on the selected insurance category
	long CBillingDlg::GetCurrentBillToMostPriInsCoIDForCategory();
	// (d.lange 2015-12-01 12:28) - PLID 67127 - Returns the _RecordsetPtr for determining the most-primary insurance
	// based on the given insured party
	ADODB::_RecordsetPtr CBillingDlg::GetMostPrimaryInsCoIDForInsuredPartyRecordset(long nInsuredPartyID);

	//looks up the RespPerCharge in the given list with the given ID
	bool FindRPCInList(RPCList *list, long nInsuredPartyID, RespPerCharge &rpc);
	//sets the amount in the RPCList for the given resp type
	bool SetListInsAmount(long nIndex, long nInsuredPartyID, COleVariant varAmount);
	void UpdateListInsAmount(RPCList *list, long nAryIndex, COleVariant varAmount);

	void OnOK();
	void OnCancel();

	// (j.jones 2010-02-05 17:28) - PLID 37251 - added bBillDateChanged
	BOOL ValidateChanges(BOOL bBillDateChanged);

	// (j.jones 2011-04-28 10:03) - PLID 43405 - there is a batch save
	// for both creating and updating charges, both batches need the same
	// declared variables, so this function prevents duplicate code
	void DeclareVariablesForChargeSave(IN OUT CString &strSqlBatch);

	// (r.gonet 08/05/2014) - PLID 63098 - Appends to a save batch SQL statements to save lab test codes linked to a charge to the database.
	// - strSqlBatch - The save batch to append to. If nUpdateForChargeID is -1, the batch must include a @ChargeID variable.
	// - aryParams - Save batch parameter array.
	// - nUpdateForChargeID - ID of the charge to link the test codes to. -1 if the charge is new. Non-negative if it is existing.
	// - pBillingItem - BillingItem associated with the charge. Should not be NULL.
	// - atAuditTrans - Audit transaction to audit to.
	void UpdateChargeLabTestCodes(IN OUT CString &strSqlBatch, IN OUT CNxParamSqlArray &aryParams,
		IN long nUpdateForChargeID, IN BillingItemPtr pBillingItem, IN OUT struct CAuditTransaction &atAuditTrans);

	// (j.jones 2011-04-27 17:38) - PLID 43405 - moved the ChargeRespT logic to its own function
	void UpdateChargeRespT(IN OUT CString &strSqlBatch, IN OUT CNxParamSqlArray &aryParams,
							IN long nUpdateForChargeID, IN BillingItemPtr pBillingItem, IN OUT struct CAuditTransaction &atAuditTrans,
							IN COleDateTime dtNewInputDate);

	// (j.jones 2011-04-28 09:29) - PLID 43405 - this now makes its changes in a batch
	void UpdateChargeRespDetails(IN OUT CString &strSqlBatch, IN OUT CNxParamSqlArray &aryParams,
								long nChargeRespID, BOOL bUpdateDate, COleDateTime dtOldDate, COleDateTime dtNewDate, COleCurrency cyAmount);
	
	/// <summary>
	/// Given a ChargeRespT.ID, a date, and the new ChargeRespT total,
	/// replace all ChargeRespDetailT entries with just one entry for
	/// the new date, with the desired total.
	/// Any existing ApplyDetailsT records will point to this new 
	/// ChargeRespDetailT record.
	/// </summary>
	/// <param name="strSqlBatch">The SQL batch that will execute this.</param>
	/// <param name="aryParams">Parameters for this SQL batch.</param>
	/// <param name="nChargeRespID">The ChargeRespT.ID to clear/rebuild the ChargeRespDetailT content for.</param>	
	/// <param name="cyTotalChargeRespAmount">The total ChargeRespT amount.</param>
	/// <param name="dtNewDate">The new date for the ChargeRespDetailT content.</param>
	void UpdateAllChargeRespDetailsToNewDate(IN OUT CString &strSqlBatch, IN OUT CNxParamSqlArray &aryParams,
		long nChargeRespID, COleCurrency cyTotalChargeRespAmount, COleDateTime dtNewDate);

	/// <summary>
	/// Given a ChargeRespT.ID, a date, a total amount that needs moved to begin aging on that date,
	/// replace all ChargeRespDetailT entries in the provided filter with just one entry for the new date,
	/// with the desired total.
	/// Any existing ApplyDetailsT records will point to this new ChargeRespDetailT record.
	/// This is used when only updating details in a specific date range.
	/// </summary>
	/// <param name="strSqlBatch">The SQL batch that will execute this.</param>
	/// <param name="aryParams">Parameters for this SQL batch.</param>
	/// <param name="nChargeRespID">The ChargeRespT.ID to clear/rebuild the ChargeRespDetailT content for.</param>
	/// <param name="sqlChargeRespDetailDateFilter">A filter on ChargeRespDetailT, such as when filtering on a date range. Must include a ChargeRespID filter.</param>
	/// <param name="cyTotalAmountToMove">The total amount that needs moved to the new date.</param>
	/// <param name="dtNewDate">The new date for the ChargeRespDetailT content.</param>
	void UpdateFilteredChargeRespDetailsToNewDate(IN OUT CString &strSqlBatch, IN OUT CNxParamSqlArray &aryParams,
								long nChargeRespID, CSqlFragment sqlChargeRespDetailFilter,
								COleCurrency cyTotalAmountToMove, COleDateTime dtNewDate);

	// (j.jones 2008-06-11 12:19) - PLID 28379 - now returns a BillingItem pointer for the row we modified
	BillingItemPtr CBillingDlg::CheckIncreaseDuplicateChargeQuantity(long ServiceID, BOOL bProduct, double dblAmountToIncrease = 1.0, BOOL bAutoAddingSerialNum = FALSE);
// (s.dhole 2011-05-17 10:40) - PLID 33666
	// (j.gruber 2014-03-21 13:00) - PLID 61494 - this function is no longer used
	//CString GetDiagCodesExtStr(CString strDiagcodes);
	// (s.dhole 2011-06-10 14:19) - PLID 33666 Thisb function will return diag code
	CString  GetDiagCodes(NXDATALISTLib::_DNxDataListPtr pDiagCombo , long nDiagID);

	void ResizeColumns();
	//cached column values
	BOOL m_bBillPatCoord = FALSE;
	BOOL m_bBillTax1 = TRUE;
	BOOL m_bBillTax2 = TRUE;
	// (j.jones 2010-09-01 10:38) - PLID 40330 - added allowable to bills
	BOOL m_bBillAllowable = FALSE;
	// (j.jones 2010-11-09 10:45) - PLID 31392 - added claim provider
	BOOL m_bBillClaimProvider = FALSE;
	// (j.jones 2014-04-23 10:17) - PLID 61836 - added referring, ordering, supervising providers
	bool m_bBillReferringProvider = false;
	bool m_bBillOrderingProvider = false;
	bool m_bBillSupervisingProvider = false;

	BOOL m_bShowChargeBatchColumn = FALSE;	

	void WarnTaxedPackage();

	COleCurrency GetPreTaxLineTotal(BillingItemPtr pBillingItem);


	BOOL GetIsRepeatPackage();
	BOOL GetIsMultiUsePackage();

	BOOL GetIsPackageQuantityChangeAllowed(double dblQtyChange, long nServiceID, long nPackageChargeRefID);

	// (a.walling 2007-04-26 09:57) - PLID 14717 - Suggested Sales dialog
	boost::scoped_ptr<class CBillingSuggestedSalesDlg> m_pSuggestedSalesDlg;

	// (r.gonet 2016-02-06 16:46) - PLID 68193 - Gets the count of non-voided
	// gift certificate charges on the bill.
	long GetGiftCertificateCount() const;
	// (r.gonet 2016-02-06 16:46) - PLID 68193 - Gets the count of non-voided
	// discountable charges on the bill.
	long GetDiscountableChargeCount() const;

	// (a.walling 2007-05-10 15:21) - PLID 25171 - Apply discount to all (created so barcodes can manually call this)
	// (j.gruber 2009-03-23 17:22) - PLID 33355 - added preference answer
	void ApplyDiscountToAll(ApplyToAllPref atapPref, long nCouponID = -1);

	// (j.gruber 2009-03-23 17:31) - PLID 33474 - moved to its own function
	BOOL ApplyDiscountFromBarCode(LPARAM lParam);

	// pointers to discount dialogs
	boost::scoped_ptr<class CDiscountBillDlg> m_pDiscountBillDlg;
	boost::scoped_ptr<class CDiscountCategorySelectDlg> m_pDiscountCategorySelectDlg;

	// (a.wetta 2007-05-08 10:27) - PLID 25959 - Gets the current discount amount of a service item
	BOOL GetServiceItemSaleDiscount(long nServiceID, double &dblPercentDiscount, COleCurrency &cyMoneyDiscount, CString &strSaleName, long &nDiscountCategoryID);

	// (j.jones 2007-11-13 17:27) - PLID 27988 - the following struct, arrays, and utility functions
	// are all used in conjunction with allocation usage
	
	//store basic allocation info in an array that tracks all products that are on all
	//allocations for the current patient, and those allocation locations
	CArray<ProductAllocationInfoCache*,ProductAllocationInfoCache*> m_aryProductAllocationInfoCache;
	
	//FindTrackedAllocationWithAvailableProduct checks our m_paryAllocationInfo array to
	//see if we have an allocation in memory that has the given product available to be billed,
	//and if so, returns it (otherwise returns NULL)
	InvUtils::AllocationMasterInfo* FindTrackedAllocationWithAvailableProduct(long nProductID);

	//returns a pointer to a ProductAllocationInfoCache object if a product matches for the current location,
	//returns NULL if no match, also populates strOtherLocation if at least one other location has an allocation
	//for the same product and patient
	ProductAllocationInfoCache* FindProductAllocationInfoCache(long nProductID, long nLocationID, CString &strOtherLocation);

	//PopulateProductToAllocationArray will repopulate m_aryProductAllocationInfo,
	//each time a bill is opened, even if the patient didn't change
	void PopulateProductToAllocationArray();

	//track an array of AllocationMasterInfo structs that
	// we have accessed in the current bill, though hopefully it will only have one entry in most cases
	CArray<InvUtils::AllocationMasterInfo*, InvUtils::AllocationMasterInfo*> m_paryAllocationInfo;	

	//FindTrackedAllocationInfoByID checks our m_paryAllocationInfo array to
	//see if we have an allocation in memory that matches the passed in ID,
	//and if so, returns it (otherwise returns NULL)
	InvUtils::AllocationMasterInfo* FindTrackedAllocationInfoByID(long nAllocationID);
	
	// (j.jones 2007-11-14 14:09) - PLID 27988 - CheckProductAgainstAllocations is the master function
	// that will take a product ID and name, find any active allocations for that patient that use that
	// product, make the user select the product from the allocation, and return a new quantity and/or
	// ChargedProductItemListID & ChargedAllocationDetailListID, if they are updated by the allocation.
	// Returns a ProductAllocationReturnValue enum to tell the caller how to proceed, either
	// add the allocated product, add the product normally, or cancel adding the product.
	//TES 7/16/2008 - PLID 27983 - Added an optional list of ProductItemIDs to pre-fill as "used" on the allocation.
	ProductAllocationReturnValue CheckProductAgainstAllocations(long &nProductID, CString &strProductName, double &dblQuantity, long &nChargedProductItemListID, long &nChargedAllocationDetailListID, OPTIONAL IN CArray<long,long> *parInitialProductItemIDs = NULL);

	// (j.jones 2007-11-15 13:58) - PLID 27988 - run through all allocations we have opened,
	// if any still have active products then we need to mark them as used or released
	// (returns FALSE if any allocations were left unresolved)
	BOOL ForceResolveAllAllocations();

	// (j.jones 2007-12-12 08:50) - PLID 27988 - commit any allocations we changed to data
	BOOL SaveTrackedAllocations();

	// (j.jones 2007-12-13 15:05) - PLID 27988 - if the given detail ID is in any allocation
	// currently tracked in memory, mark that detail as unbilled - do not change any other status
	void TryUnBillTrackedAllocationDetailID(long nAllocationDetailID);

	// (j.jones 2007-12-14 15:42) - PLID 27988 - checks and sees if we have charges linked
	// to allocations, returns TRUE if so
	BOOL HasChargesLinkedToAllocations();

	// (j.jones 2008-06-11 15:09) - PLID 28379 - this functionality is shared by
	// CheckProductAgainstAllocations and TryBillProductBySerialNumber
	//TES 7/16/2008 - PLID 27983 - Added an optional list of ProductItemIDs to pre-fill as "used" on the allocation.
	ProductAllocationReturnValue LaunchAllocationScreen(long &nProductID, CString &strProductName, double &dblQuantity, long &nChargedProductItemListID, long &nChargedAllocationDetailListID, InvUtils::AllocationMasterInfo *pInfo = NULL, long nUntrackedAllocationID = -1, long nAutoUseDetailID = -1, OPTIONAL IN CArray<long,long> *parInitialProductItemIDs = NULL);

	// (j.jones 2008-06-11 15:01) - PLID 28379 - TryBillProductBySerialNumber is called when barcoding,
	// it will try to find a product by a serial number, warn if it exists but cannot be used, add to
	// the bill if it exists and can be used, or returns silently if not found.
	// Returns TRUE if added, FALSE if not added.
	BOOL TryBillProductBySerialNumber(CString strSerialNumber);

	// (j.jones 2007-11-27 12:32) - PLID 28198 - cache the package information
	// (not all of these are used yet)
	COleCurrency m_cyOldPackageTotalAmount = COleCurrency(0, 0);
	long m_nOldPackageTotalCount = 0;
	COleCurrency m_cyOldPackageCurrentAmount = COleCurrency(0, 0);
	long m_nOldPackageCurrentCount = 0;
	COleCurrency m_cyOldPackageOriginalCurrentAmount = COleCurrency(0, 0);
	long m_nOldPackageOriginalCurrentCount = 0;
	// (j.jones 2009-12-23 16:53) - PLID 36699 - added old package type, to finish up this caching
	long m_nOldPackageType = -1;

	//TES 4/9/2008 - PLID 29585 - As far as the user is concerned, the Location is a property of the bill, but we
	// store it in the charge.  This variable is used so that when the location is changed, it only gets audited once,
	// even though it's being changed once per charge.
	bool m_bLocationAudited = false;

	//TES 7/1/2008 - PLID 26143 - An array of payments that our caller wants us to automatically apply when saving the bill.
	CDWordArray m_dwaPaymentIDsToApply;

	//TES 7/3/2008 - PLID 26143 - Our caller can forcibly tell us what "Bill A" option to default to.
	int m_nDefaultBillAOverride = -1;

	//TES 7/15/2008 - PLID 27983 - Checks whether the given ServiceID is a CPT Code that is linked to Products.  If so,
	// it prompts the user for which products to add instead of this code, and adds them.  It is the caller's responsibility
	// not to call this function in situations (i.e., on a quote), where it is OK to have placeholder CPT Codes.
	//If the function returns TRUE, that means it prompted the user, and added any products they selected, therefore the
	// caller should take no further action for this service ID.
	//If the function returns FALSE, that means no action was taken, and the caller should continue processing this service
	// ID like a normal action.
	//TES 7/17/2008 - PLID 27983 - Added bMassAdding, set this to true to tell the code to not force all allocations to be completed,
	// it is then the caller's reponsibility to do that once it's finished adding charges.
	// (j.jones 2010-11-23 16:13) - PLID 41549 - added nPackageChargeRefID, cyUnitPrice override, and discount info,
	// none of which is optional, so callers are forced to denote whether we have this information
	BOOL CheckLinkProducts(long nServiceID, double dDefaultQty, bool bMassAdding, IN COleCurrency *cyUnitPrice, long nPackageChargeRefID,
		long nLoadDiscountFromChargeID, BOOL bLoadDiscountFromSurgery);

	// (j.jones 2008-09-12 13:04) - PLID 4423 - added PostSelChosenComboBillTo,
	// to differentiate between a manual selection and a code-based selection
	void PostSelChosenComboBillTo(long nRow);

	// (j.jones 2008-11-14 08:41) - PLID 21149 - added UpdateAllChargesWithNewProvider, which will update all charges
	// but the original to have the original charge's provider, if they don't already, and check multifees
	void UpdateAllChargesWithNewProvider(_variant_t varProvider);

	// (j.jones 2008-11-14 10:12) - PLID 21149 - HasChargesWithDifferentProvider will take in a provider variant,
	// and return TRUE if any charge exists with a different provider
	BOOL HasChargesWithDifferentProvider(_variant_t varProvider);

	// (j.jones 2010-11-09 11:07) - PLID 31392 - added HasChargesWithDifferentClaimProvider, takes in a provider variant,
	// returns TRUE if any charge exists with a different claim provider (remember it can be -1)
	BOOL HasChargesWithDifferentClaimProvider(_variant_t varClaimProvider);

	// (j.jones 2010-11-09 11:15) - PLID 31392 - added UpdateAllChargesWithNewClaimProvider, which will update all charges
	// but the original to have the original charge's claim provider, if they don't already
	void UpdateAllChargesWithNewClaimProvider(_variant_t varClaimProvider);

	// (j.jones 2008-12-15 14:54) - PLID 32431 - added FindCPTRowInComboByServiceID and FindProductRowInComboByServiceID,
	// which is a safe way to call FindByColumn on each respective combo, that checks for a row of -2,
	// and if -2 is found, wait for the combo to requery and try again. Returns the Row ID,
	// and always returns -1 if no row is found.
	long FindCPTRowInComboByServiceID(long nServiceID, BOOL bAutoSelectRow);
	long FindProductRowInComboByServiceID(long nServiceID, BOOL bAutoSelectRow);

	// (j.gruber 2009-03-05 15:47) - PLID 33351 - setup the discount list
	// (j.dinatale 2012-04-10 16:34) - PLID 49519 - need to know if we are loading discounts from a glasses/contacts order
	void LoadDiscountList(BOOL bIsNew, long nChargeID, long nServiceID, DiscountList *pDiscountList, 
								   COleCurrency cyPointCost, COleCurrency cyRewardDiscountDollars, long nRewardDiscountPercent,
								   BOOL &bUsingPoints, COleCurrency &cyPointsUsed, BOOL bMarkExistingDiscounts = FALSE, BOOL bLoadFromSurgeries = FALSE, BOOL bLoadFromGlassesOrder = FALSE);
	void CalculateTotalDiscount(DiscountList *pDiscountList, COleCurrency cyCurrentPracticeTotal, COleCurrency cyCurrentOutsideTotal, long &nPercentOff, COleCurrency &cyTotalDollarDiscount, COleCurrency &cyTotalLineDiscount, KeepQuoteDiscountPref discountPreference);

	// (z.manning 2011-03-28 16:53) - PLID 41423 - Changed the type of eExistingDiscount to be an enum rather than
	// a variant to prevent different types of variants from being used for it.
	void AddToDiscountList(DiscountList *pDiscountList, _variant_t varDiscountID, _variant_t varPercentOff, _variant_t varDiscountAmt, _variant_t varCustomDiscountDesc, _variant_t varDiscountCategoryID, _variant_t varCouponID, KeepQuoteDiscountPref eExistingDiscount);
	void ShowDiscountDetailDialog(BillingItemPtr pItem, BOOL bIsBill);
	// (j.jones 2011-10-04 11:40) - PLID 45799 - now takes in a DL2 row
	void ShowDiscountDetailDialog(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bIsBill);
	void AuditAndGenerateSaveStringForDiscounts(BOOL bNewCharge, BillingItemPtr pItem, CString &strSaveString);
	void AuditAndGenerateSaveStringForNewDiscounts(BOOL bNewCharge, long ChargeID, DiscountList *pDiscountList, CString &strSaveString);
	

	// (j.gruber 2009-03-24 10:49) - PLID 33355 - added to calculate the line total;
	COleCurrency GetLineTotal(_variant_t varUnitCost, _variant_t varOtherFee, _variant_t varQuantity, 
		_variant_t varMultiplier1, _variant_t varMultiplier2, _variant_t varMultiplier3, _variant_t varMultiplier4,
		_variant_t varTax1, _variant_t varTax2, DiscountList *pDiscountList,
		BOOL bIncludeTax, BOOL bOnlyPracticeFee= FALSE, BOOL bOnlyOtherFee=FALSE);

	COleCurrency GetLineTotal(BillingItemPtr pItem, BOOL bIncludeTax, ApplyToAllPref atapPref);

	// (j.gruber 2009-04-03 17:23) - PLID 33246 - check the line total
	BOOL CanAddDiscount(BillingItemPtr pItem, BOOL bIncludeTax, ApplyToAllPref atapPref, long nPercentOff, COleCurrency cyDiscount);

	// (j.jones 2009-03-31 10:43) - PLID 33747 - added support for OHIP Premium Codes
	//returns TRUE if we processed something and the calling function SHOULD NOT add the code
	//(the user may have cancelled adding the premium code altogether)
	BOOL ProcessOHIPPremiumCode(long nServiceID);

	void RequeryFinishedComboQuote(short nFlags);

	// (j.jones 2009-09-14 15:37) - PLID 35382 - this function looks at all the new charges on the bill,
	// and tries to link case histories to the bill based on the procedures those charges are linked to
	void TryLinkCaseHistoriesByProcedure();

	// (j.jones 2009-09-14 15:48) - PLID 35382 - this function takes in a quote ID,
	// and tries to link case histories to the bill based on whether the case history
	// is on the same PIC as the quote
	void TryLinkCaseHistoriesByQuote(long nQuoteID);
	
	// (j.gruber 2009-10-16 11:26) - PLID 35947 - add allowable column
	// (j.jones 2010-09-01 11:19) - PLID 40330 - renamed to reflect that this is now used on bills
	// (b.cardillo 2015-11-24 11:02) - PLID 67121 - Changed second parameter name and made the meaning more 
	// firm so we could use it for updating New Fee as well.
	// Calculates the allowable for pItem and sets pItem to reflect that allowable. If bReflectOnExistingItem 
	// is TRUE, it is assumed the item already exists and therefore the Allowable should be updated on 
	// screen AND the item's New Fee should be calculated as well and if it has changed, the user should be 
	// prompted to change the charge's unit price both in memory and on screen.
	void LoadAllowableColumn(BillingItemPtr pItem, BOOL bReflectOnExistingItem);
	long m_nPrimaryInsCoID = PRI_INSCO_NOT_SET;
	
	// (j.gruber 2009-10-19 10:01) - PLID 35995 - update one charge at a time
	void UpdateOneChargeWithNewProvider(BillingItemPtr pItem, NXDATALIST2Lib::IRowSettingsPtr pRow, _variant_t varProvider);

	// (j.jones 2009-11-02 15:23) - PLID 44941 - given a string of quote IDs and an array
	// of applied payments see if we just finished billing a package and applied linked
	// prepayments to it, and adjust the balance as necessary
	void TryAdjustPackagePaymentDifference(CString strQuoteIDs, CArray<PaymentToApply, PaymentToApply&> &aryAppliedPayments);

	// (j.jones 2011-08-24 08:41) - PLID 44868 - TryChangeBillLocation will
	// attempt to switch to the new location ID, returns FALSE if the change
	// could not be made
	BOOL TryChangeBillLocation(long nNewLocationID);

	// (j.jones 2011-08-24 08:41) - PLID 44868 - TryChangePlaceOfService will
	// attempt to switch to the new POS ID, returns FALSE if the change
	// could not be made
	BOOL TryChangePlaceOfService(long nNewPlaceOfServiceID);
	
	// (j.jones 2009-12-22 14:36) - PLID 32587 - made the original amount fields editable
	void OnPackageShowInitialValues();
	afx_msg void OnKillfocusPackageOriginalCurrentAmount();
	afx_msg void OnKillfocusPackageOriginalCurrentCount();
	afx_msg void OnChangePackageOriginalCurrentAmount();
	afx_msg void OnChangePackageOriginalCurrentCount();

	// (j.jones 2010-08-30 13:46) - PLID 40293 - added a function to calculate if there are any
	// diagnosis codes on the bill (with an index < 9) that aren't linked to any charges
	BOOL CheckWarnUnlinkedDiagnosisCodes();

	//(c.copits 2010-10-01) PLID 40317 - Allow duplicate UPC codes for FramesData certification.
	long GetBestUPCProductInventory(_variant_t var);

	//(c.copits 2010-10-26) PLID 38598 - Warranty tracking system
	// This function will update warranty expiration dates
	void UpdateInvWarrantyExpDates(long ChargedProductItemListID);

	void OnSelChosenComboGlassesOrders(LPDISPATCH lpRow);
	// (j.jones 2011-08-24 08:41) - PLID 44868 - added OnSelChangedComboPlaceofservice
	void OnSelChangedComboPlaceofservice(long nNewSel);

	// (j.jones 2011-10-06 10:02) - PLID 44941 - store the combo source for the normal CPTModifierT source, used if not Alberta
	NXDATALIST2Lib::IFormatSettingsPtr m_pfsCPTModifierComboSource = NULL;

	

	// (j.jones 2011-10-06 10:23) - PLID 44941 - this function creates/regenerates m_pCPTModifierComboSource, not used with Alberta
	void RefreshCPTModifierComboSource();

	// (j.jones 2011-10-06 11:19) - PLID 44941 - fills the four modifier columns for a row with the appropriate
	// modifier combo source, whether US or Alberta, quote or a bill
	void SetModifierComboSource(NXDATALIST2Lib::IRowSettingsPtr pRow, BillingItemPtr pBillItem);

	// (j.jones 2011-10-06 10:02) - PLID 44941 - Alberta service codes have a unique set of modifiers per code.
	// When we load them into an IFormatSettingsPtr, we will map them for re-use in this Practice session.
	CMap<long, long, NXDATALIST2Lib::IFormatSettingsPtr, NXDATALIST2Lib::IFormatSettingsPtr> m_mapAlbertaServiceIDToModifierComboSource;
	// (s.tullis 2015-03-27 14:48) - PLID 64976- Fuction to get multi category warning string
	CString GetMultCategoryCPTWarning(std::vector<CString> arrNoCPTCatSelCharges);

	// (j.jones 2011-10-06 11:24) - PLID 44941 - Alberta service codes have a unique set of modifiers per code.
	// This will return the IFormatSettingsPtr for the given code. This function will pull from the map, or
	// generate new and add to the map if needed.
	NXDATALIST2Lib::IFormatSettingsPtr GetAlbertaModifierFormatSettingsForServiceID(long nServiceID);

	// (j.jones 2011-10-11 10:30) - PLID 44941 - called after a modifier is changed on a charge,
	// for Alberta only, so we can change the unit price of the charge
	// (j.jones 2012-01-23 09:09) - PLID 47695 - added bForceCalc parameter to force calculating
	// even if there are no modifiers, if FALSE, this will not change the cost unless there are
	// modifiers on the charge
	void UpdateChargePriceWithAlbertaModifiers(BillingItemPtr pBillItem, BOOL bForceCalc);

	// (j.jones 2011-10-11 12:42) - PLID 44941 - given an Alberta service code and modifier,
	// we calculate the new unit cost for the charge
	COleCurrency CalculateChargePriceFromAlbertaModifier(CString strModifier, long nServiceID, const COleCurrency cyBaseUnitCost,
		IN OUT double &dblCalls);

	// (j.gruber 2012-01-04 11:57) - PLID 46291 - sets the bill description based on the currently selected resp 
	void SetBillDescriptionBasedOnResp();

	// (d.singleton 2014-03-21 09:06) - PLID 61428 - remove old medassist code.
	// (d.singleton 2012-03-06 10:23) - PLID 25098 member array to hold the validation results
	//CArray<MedAssist::ParsedResults, MedAssist::ParsedResults> m_aryValidatedParsedResults;

	// (d.singleton 2012-03-30 10:42) - PLID 49257 function to store unsaved notes
	void AddUnsavedNotes(BOOL bIsUnsavedCharge, _variant_t varChargeID, _variant_t varLineID);



	afx_msg void OnBnClickedVerifyBill();

	// (j.jones 2014-04-23 10:28) - PLID 61836 - added a function to get the provider combo content
	void GetProviderComboSql(OUT CString &strProvCombo, OUT CString &strClaimProvCombo, OUT CString &strReferringProvCombo, OUT CString &strOrderingProvCombo, OUT CString &strSupervisingProvCombo);
	// (s.tullis 2015-03-24 09:28) - PLID 64973 - Gets the CPT category format pointer
	NXDATALIST2Lib::IFormatSettingsPtr GetCPTMultiCategoryCombo(IN BillingItemPtr pBillItem);
// (s.tullis 2015-03-24 09:28) - PLID 64973 -  Sets the CPT Category Format Pointer
	void SetCPTCategoryCombo(NXDATALIST2Lib::IRowSettingsPtr pRow, BillingItemPtr pBillItem, BillEntryType type );
	// (s.tullis 2015-04-14 14:13) - PLID 65537 - Fuction that Colors columns red if they need selected
	void UpDateBillQuoteListCellColors(NXDATALIST2Lib::IRowSettingsPtr pRow, BillingItemPtr pBillItem, BillEntryType type);

	// (c.haag 2014-03-04) - PLID 60928 - This is the bottleneck for adding new diagnosis codes to this dialog.
	// (j.gruber 2014-03-24 14:56) - PLID 61529 - add bSilent in case we don't want the warning
	// (j.armen 2014-03-25 14:21) - PLID 61517 - returns the order for this diag code
	long AddDiagCode(DiagCodeInfoPtr pNewDiag, BOOL bSilent = FALSE);

	void DiagOrderModified();	// (j.armen 2014-08-07 15:12) - PLID 63231
	void RemoveDiagCode();	// (j.armen 2014-08-07 17:12) - PLID 63236

public:
	// (j.gruber 2014-03-21 13:27) - PLID 61493 - created for
	// (j.gruber 2014-03-24 14:58) - PLID 61529 - add bSilent in case we don't want the warning
	// (j.armen 2014-03-25 14:21) - PLID 61517 - returns the order for this diag code
	long AddDiagCode(long nDiag9ID, long nDiag10ID, CString strDiag9Code, CString strDiag10Code, CString strDiag9Desc, CString strDiag10Desc, BOOL bSilent = FALSE);
	// (j.jones 2012-04-12 09:27) - PLID 49609 - Adds a new unsaved note to a charge.
	// This assumes the calling code will later update the datalist.
	void AddNewUnsavedChargeNote(BillingItemPtr pBillItem, CString strNoteToAdd, BOOL bIsClaimNote);

private:
	// (j.jones 2014-04-23 10:11) - PLID 61836 - generic function to force showing
	// a column, in the event it is currently at 0 widthm_bGlassesOrderCombo
	void ForceShowColumn(short iColumnIndex, long nPercentWidth, long nPixelWidth);

	// (j.jones 2014-04-23 10:11) - PLID 61836 - these functions will forcibly show the
	// Referring, Ordering, or Supervising provider columns in the charge list, if
	// they happen to be hidden	
	void ShowReferringProviderColumn();
	void ShowOrderingProviderColumn();
	void ShowSupervisingProviderColumn();
	// (s.tullis 2015-04-07 16:54) - PLID 64975  -force Show charge category column
	void ShowCPTCategoryColumn(BillEntryType type);

	// (r.gonet 2015-03-27 19:07) - PLID 65277 - Shows or hides the Value column. It should be shown only
	// when there is a gift certificate on the bill (or if the user resizes the columns).
	void ShowValueColumn(bool bShow);
	
	// (j.jones 2014-04-30 15:25) - PLID 61837 - Called when a charge is added,
	// or changed in such a way that may cause the Referring, Ordering, or
	// Supervising provider columns to show or hide.
	// Set bIsAdding if this is called while adding a new charge.
	// Set bUpdateProviders to false if you only want to show the columns,
	// but not try to load data.
	// Set bEditedClaimProvider to true if the claim provider just changed,
	// which causes thes calculations to behave somewhat differently.
	void TryShowChargeProviderColumns(BillingItemPtr pItem, bool bIsAdding, bool bUpdateProviders = true, bool bEditedClaimProvider = false);

	// (j.jones 2014-05-05 11:14) - PLID 61837 - helper function to TryShowChargeProviderColumns
	void CalculateChargeProviderColumn(const ChargeLevelProviderConfigOption eConfigOption, const long nConfigProviderID,
		const bool bUpdateProviders, const bool bEditedClaimProvider,
		const long nCurChargeProviderID, const long nCurClaimProviderID, const long nInsuredPartyID, const long nCurSpecialProviderID,
		OUT bool &bShowColumn, OUT bool &bNeedChangeProvider, OUT long &nNewSpecialProviderID);

	// (j.jones 2014-05-05 09:12) - PLID 61837 - calculates what the claim provider
	// would be on a charge if "use default claim provider" is selected
	long CalculateDefaultClaimProviderID(const long nCurChargeProviderID, const long nCurClaimProviderID, const long nInsuredPartyID);
	// (r.gonet 07/02/2014) - PLID 62567 - Sets the bill description edit box value.
	// The intention is to hide the prepending of the status prefix. The opposite of this function
	// is GetBillDescription()
	void SetBillDescription(CString strBillDescription);
	// (r.gonet 07/02/2014) - PLID 62567 - Gets the bill description edit box value, with the status prefix.
	CString GetBillDescriptionWithPrefix();
	// (r.gonet 07/02/2014) - PLID 62567 - Gets the bill description edit box value, without the status prefix.
	// The intention is to hide the prepending of the status prefix. The opposite of this function
	// is SetBillDescription(str)
	CString GetBillDescription();
	// (r.gonet 07/01/2014) - PLID 62525 - In the NoteDataT table, the username is appended to the end of the note,
	// but we don't want the username actually appearing in the bill editor dialog, so this function removes it.
	CString RemoveUsernameFromStatusNote(CString strStatusNote, CString strUsername);
	// (r.gonet 07/01/2014) - PLID 62525 - Appends the current user's username to a bill status note.
	CString AddCurrentUsernameToStatusNote(CString strStatusNote);
	//(s.dhole 3/24/2015 12:41 PM ) - PLID 61135
	BOOL IsCodeSelectedBasedOnWarning(const long nServiceID);
public:
	// (r.gonet 07/01/2014) - PLID 62567 - When the user selects a status, do stuff to the billing dialog if it
	// the selected status is a special type.
	void SelChosenBillStatusCombo(LPDISPATCH lpRow);
	// (r.gonet 07/01/2014) - PLID 62533 - In the bill, add an ellipsis button next to the status drop down to
	// open the bill status config dialog.
	afx_msg void OnBnClickedBillStatusConfigBtn();
	// (r.gonet 07/01/2014) - PLID 62523 - When the user selects a custom status note, then update the status note edit box.
	void SelChosenBillStatusNoteCombo(LPDISPATCH lpRow);
	// (r.gonet 07/01/2014) - PLID 62520 - In the bill, next to the status drop down, add an ellipsis button to
	afx_msg void OnBnClickedBillStatusNoteConfigBtn();

	// (r.gonet 07/02/2014) - PLID 62567 - Returns the currently selected bill status's type or None if no status is selected.
	EBillStatusType GetBillStatusType();
	// (r.gonet 07/02/2014) - PLID 62567 - Updates the tracked bill status type in the billing module dialog.
	void CurSelWasSetBillStatusCombo();
	// (r.gonet 07/09/2014) - PLID 62834 - Gets the number of charges on the bill that are not on hold.
	long GetNotOnHoldChargeCount();
	// (r.gonet 07/09/2014) - PLID 62834 - Gets the number of charges on the bill that are on hold.
	long GetOnHoldChargeCount();
	// (r.gonet 07/09/2014) - PLID 62834 - Returns the billing items that are on hold.
	void GetOnHoldCharges(std::vector<BillingItemPtr> &vecOnHoldCharges);
	// (r.gonet 07/09/2014) - PLID 62834 - Sets all charges in the bill that are on hold to not on hold.
	void ClearChargeHolds();

	// (j.jones 2014-07-28 09:31) - PLID 56662 - Added modular function to requery the product combo.
	// Will filter on the bill's location unless a specific location ID is provided.
	void RequeryProductCombo(long nLocationID = -1);

	// (j.jones 2016-04-07 15:43) - NX-100077 - used when billing a product that requires the provider
	// to default to the last provider who sold the product to this patient, will update the pointer accordingly
	void UpdateChargeToMostRecentProviderSold(IN OUT BillingItemPtr pBillingItem);
};

// (s.dhole 2012-04-05 15:34) - PLID 43785 Moved thes function from class
	long GetPercentOffSaveValue(_variant_t varPercentOff);
	CString GetDiscountSaveValue(_variant_t varDiscount);
	void GetDiscountCategorySaveValues(_variant_t varDiscountCategoryID, _variant_t varCouponID, _variant_t varCustomDiscountDesc, CString &strDiscountCategoryID, CString &strCouponID, CString &strCustomDiscountDesc);
	BOOL FindDiscountInArray(DiscountList *pDiscountList, long nDiscountID, stDiscount &Disc);
	CString GetDiscountCategoryName(long nDiscountCategoryID);
	CString GetCouponName(long nCouponID);
	CString GetAuditDiscountDescription(stDiscount Disc);
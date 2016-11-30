// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

// (j.jones 2007-11-13 17:14) - PLID 27988 - added when too many .h files in billingdlg.h referenced this header
#pragma once

#include "InvNew.h"

namespace InvUtils
{
	// (j.jones 2007-11-08 13:38) - PLID 27987 - added enums for Inv. Allocation statii

	//these IDs are saved in data and cannot be changed!
	enum InventoryAllocationStatus {
		iasDeleted = -1,
		iasActive = 1,
		iasCompleted = 2,
	};

	//these IDs are saved in data and cannot be changed!
	enum InventoryAllocationDetailStatus {
		iadsDeleted = -1,
		iadsActive = 1,
		iadsUsed = 2,
		iadsReleased = 3,
		iadsOrder = 4,	//TES 7/18/2008 - PLID 29478 - New status, for details where the products need to be ordered
	};

	// (c.haag 2007-12-03 10:26) - PLID 28204 - These ID's are saved in data and cannot be changed
	enum ProductItemStatus {
		pisPurchased = 0,
		pisConsignment = 1,
		pisWarranty = 2,
	};

	// (c.haag 2007-12-03 10:27) - PLID 28204 - These ID's are saved in data and cannot be changed
	enum OrderDetailsStatus {
		odsPurchased = 0,
		odsConsignment = 1,
		odsWarranty = 2,
	};

	//DRT 11/16/2007 - PLID 27990 - I translated these enums into the report, so please don't change them.
	//DRT 11/28/2007 - PLID 28215 - Moved to InvUtils.  These aren't saved in data, but they are used in reports
	//	and are now a more global concept.
	// (j.jones 2007-11-30 12:37) - PLID 27989 - added the On Hand filter
	enum OverviewStatusType {
		ostAll = -1,
		ostAvailable = 0,		
		ostAllocated = 1,
		ostUsed = 2,
		ostAdjusted = 3,
		ostOnHand = 4,
	};

	// (c.haag 2007-12-10 11:12) - PLID 28237 - Enum for the Barcode_CheckExistenceOfSerialNumber function
	// (c.haag 2008-06-18 11:05) - PLID 28339 - Rearranged so that results are presented in a specific order
	enum Barcode_CheckExistenceOfSerialNumberStatus {
		// Always returned
		stAvailable = 0,
		// Sometimes returned (depending on flags passed into Barcode_CheckExistenceOfSerialNumber)
		stCompletedAllocation,
		stAllocated,
		// Never returned
		stReturned,
		stInactive,
		stAdjusted,
		stBilled,
		stCaseHistory,
	};

	//TES 2/19/2008 - PLID 28954 - These are saved to data.  Do not change!!
	enum ItemReturnedFor {
		irfCredit = 0,
		irfExchange = 1,
	};

	// (c.haag 2008-02-29 12:10) - PLID 29115 - Enumerations for adding items
	// to inventory todo alarm transactions
	enum EInvTodoAlarmTransItemType { 
		eInvTrans_ProductID = 0,		// ProductT.ID (include all inventory locations)
		eInvTrans_ProductItemID,		// ProductItemT.ID (points to product and location id's)
		eInvTrans_AllocationID,			// PatientInvAllocationsT.ID (points to location id and details with product id's)
		eInvTrans_PatientID,			// PatientsT.PersonID (points to allocations)
		eInvTrans_ProductIDLocationID,	// The record points to two ID's; a product ID and location ID
	};

	// (b.eyers 2016-03-14) - PLID 
	//this is also in MarkupFormulaEditDlg.h
	enum RoundUpRules {
		NoRoundUpRule = 0,
		RoundUpNearestDollar = 1,
		RoundUpToNine = 2,
	};

	//DRT 11/28/2007 - PLID 28215 - Added constant text descriptions of the above status types.  Use these for consistency
	// (j.jones 2007-11-30 12:37) - PLID 27989 - added the On Hand filter
	// (a.walling 2013-02-26 11:07) - PLID 55333 - Do not define and implement CStrings in headers
	extern const char ostAll_Text[]; // = " <All Product Statuses>";
	extern const char ostAvailable_Text[]; // = "Available";
	extern const char ostAllocated_Text[]; // = "Allocated";
	extern const char ostUsed_Text[]; // = "Used";
	extern const char ostAdjusted_Text[]; // = "Adjusted / Returned";
	extern const char ostOnHand_Text[]; // = "On Hand";

	//TES 6/26/2008 - PLID 30522 - These are the hard-coded IDs (stored in DATA!) for the system categories for auto-generated
	// inventory adjustments
	const long g_nInitialAdjustmentID = -25;
	const long g_nReturnedByPatientID = -26;
	const long g_nReturnedToSupplierID = -27;
	// (j.jones 2009-01-16 11:15) - PLID 32684 - added hard-coded ID for inv. reconciliation adjustments
	const long g_nAdjByInvReconciliation = -28;

	// (c.haag 2007-12-10 11:05) - PLID 28237 - Added structure for optional extended output
	// (j.jones 2008-06-11 12:07) - PLID 28379 - added allocation info, if it is on an active allocation
	struct Barcode_CheckExistenceOfSerialNumberResultEx {
		_variant_t varProductLastCost;
		BOOL bProductHasSerialNum;
		BOOL bProductHasExpDate;
		long nProductLocationID;
		ProductItemStatus pisStatus;
		Barcode_CheckExistenceOfSerialNumberStatus stStatus;
		long nAllocationID;
		long nAllocationDetailID;
	};

	// (j.jones 2007-11-13 10:14) - PLID 27988 - defined the structs for Allocation memory object
	// (j.jones 2007-12-07 11:29) - PLID 28196 - split quantity into "current" and "original" fields

	struct AllocationDetailInfo {	//a memory-based instance of PatientInvAllocationDetailsT

		long nDetailID;				//PatientInvAllocationDetailsT.ID
		long nProductID;			//PatientInvAllocationDetailsT.ProductID
		CString strProductName;		//the product name
		long nProductItemID;		//PatientInvAllocationDetailsT.ProductItemID (may be -1)
		_variant_t varSerialNum;	//the ProductItemsT serial number (may be NULL)
		_variant_t varExpDate;		//the ProductItemsT expiration date (may be NULL)
		double dblCurQuantity;		//PatientInvAllocationDetailsT.Quantity (the quantity we've changed to, and needs saved)
		double dblOriginalQty;		//PatientInvAllocationDetailsT.Quantity (the quantity currently in data)
		InvUtils::InventoryAllocationDetailStatus iadsOriginalStatus;	//PatientInvAllocationDetailsT.Status
		InvUtils::InventoryAllocationDetailStatus iadsCurrentStatus;	//what we changed PatientInvAllocationDetailsT.Status to
		CString strNotes;			//PatientInvAllocationDetailsT.Notes // (j.jones 2007-12-06 10:57) - PLID 28196
		CString strOriginalNotes;	//PatientInvAllocationDetailsT.Notes
		BOOL bBilled;				//used to determine if we already billed, or are billing, this detail
		// (j.jones 2008-01-07 16:33) - PLID 28479 - track whether this item is billable at all, which will negate bBilled
		BOOL bIsProductBillable;	//determines whether the product is billable for the allocation's location
		// (c.haag 2008-03-11 14:04) - PLID 29255 - We now track the product item status.
		_variant_t varProductItemStatus;
		//TES 6/20/2008 - PLID 26152 - ProductItemsT.ToBeReturned - only meaningful if the allocationdetail is released, and is Purchased Inventory
		BOOL bToBeReturned;
		//TES 6/20/2008 - PLID 26152 - For auditing
		BOOL bOldToBeReturned;
		//TES 7/18/2008 - PLID 29478 - Useful in a few places when dealing with the "To Be Ordered" status.
		BOOL bIsSerialized;
	};

	struct AllocationMasterInfo {	//a memory-based instance of PatientInvAllocationsT

		long nAllocationID;			//PatientInvAllocationsT.ID
		long nPatientID;			//PatientInvAllocationsT.PatientID
		CString strPatientName;		//the patient's name
		long nLocationID;			//PatientInvAllocationsT.LocationID
		CString strLocationName;	//the location name
		long nAppointmentID;		//PatientInvAllocationsT.AppointmentID (may be -1)
		CString strAppointmentDesc;	//the appointment description
		long nCaseHistoryID;		//CaseHistoryAllocationLinkT.CaseHistoryID (may be -1) // (j.jones 2008-03-12 12:00) - PLID 29102
		CString strNotes;			//PatientInvAllocationsT.Notes
		InvUtils::InventoryAllocationStatus iasStatus;	//PatientInvAllocationsT.Status
		COleDateTime dtInputDate;	//PatientInvAllocationsT.InputDate

		CArray<AllocationDetailInfo*,AllocationDetailInfo*> paryAllocationDetailInfo; //the AllocationDetailInfo objects for this allocation

		BOOL bHadResolutionAlready;	//only used in billing, so we know the user was once asked to bill the products
	};

	// (j.jones 2008-02-28 10:16) - PLID 28080 - at long last, item_sql is now a proper function
	// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
	CSqlFragment GetInventoryItemSql(long nProductID, long nLocationID);

	// (j.jones 2008-09-15 13:13) - PLID 31376 - supported GetInventoryItemSql to be run
	// for multiple products at a time
	// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
	CSqlFragment GetInventoryItemSql(CArray<long, long> &aryServiceIDs, long nLocationID);

	// (j.jones 2009-01-07 16:24) - PLID 32648 - supported GetInventoryItemSql to be run
	// for multiple products, by taking in an IN clause instead of an array of IDs
	// the in clause should be a query that would work when called like:
	// "ProductT.ID IN (strInClause) "
	// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
	CSqlFragment GetInventoryItemSql(CSqlFragment sqlInClause, long nLocationID);

	// (j.jones 2007-11-14 13:14) - PLID 27988 - FreeAllocationMasterInfoObject can be
	// used by any class that tracks an AllocationMasterInfo pointer such that any
	// class can delete it using the same function
	void FreeAllocationMasterInfoObject(AllocationMasterInfo *pAllocationMasterInfo);

	CString Descendants(long id);
	void Descendants(CArray<long,long> &array);
	long GetDefaultCategory();
	long GetDefaultItem();
	void SetDefault (long item, long category);
	void SaveDefaultParameters();

	// (c.haag 2008-02-07 13:17) - PLID 28853 - Renamed from ChargeInventoryQuantity to EnsureInventoryTodoAlarms
	// because that's a closer description to what it actually does. Also removed unused quantity parameter.
	// (j.jones 2008-09-15 12:55) - PLID 31376 - EnsureInventoryTodoAlarms now supports being called
	// with an array of service IDs, to check on multiple products more efficiently
	//TES 11/14/2011 - PLID 44716 - This function needs to know whether it's being called from within a transaction
	void EnsureInventoryTodoAlarms(CArray<long, long> &aryServiceIDs, long nLocationID, bool bInTransaction);

	// (j.jones 2007-12-18 11:14) - PLID 28037 - converted to handle allocated counts
	void DoIHaveEnough(long nServiceID, long nLocationID , double dblInStockAdjustment = 0.0, double dblAllocatedAdjustment = 0.0, BOOL bQuote = FALSE);

	// (j.jones 2007-12-18 10:47) - PLID 28037 - converted to also return allocated counts
	BOOL CalcAmtOnHand(long nServiceID, long nLocationID, double &dblOnHand, double &dblAllocated, double dblInStockAdjustment = 0.0, double dblAllocatedAdjustment = 0.0);

	// (c.haag 2008-02-27 13:44) - PLID 29115 - This function will begin a transaction that, when
	// committed, will update inventory todo alarms based on inventory products and product items
	// that have changed
	int BeginInventoryTodoAlarmsTransaction();

	// (c.haag 2008-02-27 13:52) - PLID 29115 - This function adds an element to an inventory
	// todo alarm transaction
	void AddToInventoryTodoAlarmsTransaction(int nTransID, EInvTodoAlarmTransItemType idtype, long nID);
	void AddProductLocToInventoryTodoAlarmsTransaction(int nTransID, long nProductID, long nLocationID);

	// (c.haag 2008-02-27 15:47) - PLID 29115 - This function commits an inventory todo ensurance
	// transaction. The goal is to call EnsureInventoryTodoAlarms as few times as possible
	//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
	void CommitInventoryTodoAlarmsTransaction(int nTransID, bool bInSqlTransaction);

	// (c.haag 2008-02-27 16:10) - PLID 29115 - This function cancels an inventory todo ensurance
	// transaction.
	void RollbackInventoryTodoAlarmsTransaction(int nTransID);

	void TransferItems(long nProductID, double dblQuantity, long nSourceLocationID, long nDestLocationID, CString strNotes);

	//long GetAmountScheduled(long service, long location);

	void TryUpdateLastCostByProduct(long ProductID, COleCurrency cyCost, COleDateTime dtReceivedDate);

	void TryUpdateLastCostByOrder(long OrderID, COleDateTime dtReceivedDate);

	long GetAvailableProductItemCount(long nProductID, long nLocationID);

	// (a.walling 2007-06-06 13:24) - PLID 26238 - Safe Table Checker class to ensure cleanup occurs on destruction
	// (a.walling 2010-03-09 14:18) - PLID 37640 - Moved to Client.cpp / .h

	// (c.haag 2007-11-07 09:37) - PLID 27994 - This function returns the generic "from"
	// clause of an inventory item dropdown used in InvEditOrderDlg.cpp and InvEditReturnDlg.cpp.
	// This code was completely ripped from InvEditOrderDlg.cpp
	CString GetItemDropdownSqlFromClause(long nLocationID, long nSupplierID);

	// (c.haag 2007-11-09 16:23) - PLID 27994 - This utility function deletes a return group
	BOOL DeleteReturnGroup(long nID, BOOL bShowWarnings, CWnd* pWndWarningParent);

	// (j.jones 2007-11-30 15:10) - PLID 28251 - For purposes of barcoding, take in a serial number and LocationID,
	// see if it exists in the system, and if it is available to use. If exists but not available, explain why not,
	// unless bSilent is TRUE. The other information, such as ProductItemID and ProductID, are filled even if the product
	// is not available to use.

	// (c.haag 2007-12-04 17:41) - PLID 28237 - Added optional output so the caller doesn't have to fetch additional data
	// (c.haag 2007-12-05 12:37) - PLID 28237 - Added optional flag to allow product items that are allocated (but the allocation is incomplete)
	// (j.jones 2008-06-11 16:54) - PLID 28739 - added another flag to allow completed allocations
	// (c.haag 2008-06-18 11:54) - PLID 30427 - Added flags to allow filtering on product item status
	//TES 7/7/2008 - PLID 24726 - Added an optional array of ProductItemsT IDs that you specifically do not want returned by
// this function (you may have "used" them in memory, but not in the database)
	BOOL Barcode_CheckExistenceOfSerialNumber(const CString& strSerialNum, long nLocationID, BOOL bSilent, long &nProductItemID, long &nProductID, CString &strProductName, _variant_t &varExpDate,
		OPTIONAL OUT Barcode_CheckExistenceOfSerialNumberResultEx* pResult = NULL, OPTIONAL BOOL bAllowIncompleteAllocations = FALSE, OPTIONAL BOOL bAllowCompleteAllocations = FALSE,
		OPTIONAL BOOL bAllowPurchasedInv = TRUE, OPTIONAL BOOL bAllowConsignment = TRUE, OPTIONAL CArray<long,long> *parProductItemIDsToExclude = NULL);

	// (j.jones 2007-12-06 16:40) - PLID 28196 - PopulateAllocationInfo will populate a given
	// pAllocationMasterInfo based on nAllocationID
	// (j.jones 2008-02-20 09:23) - PLID 28948 - added a flag for whether to include released items
	void PopulateAllocationInfo(long nAllocationID, InvUtils::AllocationMasterInfo *&pAllocationMasterInfo, BOOL bIncludeReleased);

	// (j.jones 2007-12-07 15:41) - PLID 28196 - given an AllocationMasterInfo object that is to be completed,
	// add save statements and audits to the passed in strSqlBatch and nAuditTransactionID
	// (j.jones 2008-02-19 17:54) - PLID 28948 - renamed this function and gave it a parameter to determine if we
	// are completing the allocation, or just saving changes to an existing one
	BOOL GenerateAllocationSaveSql(BOOL bComplete, InvUtils::AllocationMasterInfo *pAllocationMasterInfo, CString &strSqlBatch, long &nAuditTransactionID);

	// (c.haag 2008-03-18 17:20) - PLID 29306 - This function will display a warning to the user, and return
	// TRUE if a product with the given barcode exists. If there is no such product, or an error occurs, this
	// function returns FALSE. If the code was scanned in, bWasScanning should be TRUE. If keyed in, FALSE.
	// bWasScanning affects the output of the message box.
	// (z.manning 2008-07-07 11:52) - PLID 30602 - Added an optional parameter to not display any message boxes.
	BOOL Barcode_CheckExistenceOfProductCode(const CString& strBarcode, BOOL bWasScanning, BOOL bSilent = FALSE);
	// (z.manning 2008-07-07 11:43) - PLID 30602 - Added an overload that also returns the name
	// of the product if the barcode exists.
	BOOL Barcode_CheckExistenceOfProductCode(const CString& strBarcode, BOOL bWasScanning, OUT CString &strProductName, BOOL bSilent = FALSE);

	// (j.jones 2008-03-19 15:05) - PLID 29311 - Given an order ID, see if an appt. is linked to it,
	// and if so, try to create an allocation for that patient, with the contents of the order.
	// won't warn if the order is not received or has no linked appt., and return silently.
	void TryCreateAllocationFromOrder(long nOrderID);

	// (c.haag 2008-03-20 13:05) - PLID 29335 - Warns the user, and returns TRUE if 
	// the serial number is in use for a given product. Returns FALSE if the serial is
	// not in use for the given product.
	BOOL IsProductItemSerialNumberInUse(long nProductID, const CString& strSerialNum);

	// (c.haag 2008-06-11 15:15) - PLID 28474 - This function returns the NoteCatsF entry where the
	// description is 'Inv Order'. This is used in several places.
	//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
	long GetInvOrderTodoCategory(bool bInTransaction);

	// (c.haag 2008-06-11 14:43) - PLID 28474 - Update order todo alarms
	void UpdateOrderTodoAlarms(long nOrderID);

	//TES 6/16/2008 - PLID 30394 - Are there any appointments that meet our criteria for needing an allocation, but don't
	// have one?
	BOOL DoAppointmentsWithoutAllocationsExist();

	// (c.haag 2008-06-25 11:05) - PLID 28438 - Returns TRUE if an order has at least one order
	// detail with a received date
	BOOL IsOrderPartiallyOrFullyReceived(long nOrderID);

	//TES 7/23/2008 - PLID 30802 - All the information that UpdateLinkedAllocationDetails needs.
	struct ReceivedOrderDetailInfo {
		long nOrderDetailID;
		long nQuantity;
		long nSourceOrderDetailID;
		bool bReceived;
	};

	//TES 7/23/2008 - PLID 30802 - Goes through all the order details, and updates any allocations linked to those details
	// to reflect the fact that they are now received.  Will output a list of allocation IDs that got updated, in case you
	// want to inform the user.
	void UpdateLinkedAllocationDetails(const CArray<ReceivedOrderDetailInfo,ReceivedOrderDetailInfo&> &arReceivedDetails, OUT CArray<long,long> &arUpdatedAllocationIDs);

	// (j.jones 2008-06-02 15:46) - PLID 28076 - AdjustProductItems will now fill an array with IDs
	// of products that need adjusted, and actually adjust them off later
	//(e.lally 2008-07-01) PLID 24534 - Added support for entering a serial/exp date per Unit of Order on positive product adjustments
	// (j.jones 2009-01-15 15:40) - PLID 32684 - moved from InvAdj to InvUtils
	// (j.jones 2009-03-09 12:21) - PLID 33096 - added strCreatingProductAdjustmentID
	// (j.jones 2009-07-09 17:09) - PLID 32684 - bDeclareNewProductItemID will control whether
	// the CProductItemsDlg::Save() function adds a declaration for @nNewProductItemID when generating sql batches
	BOOL AdjustProductItems(long nProductID, long nLocationID, double dblQuantity, CArray<long, long> &aryProductItemIDsToRemove,
						bool bSaveDataEntryQuery, CString &strSqlBatch, CString strCreatingProductAdjustmentID,
						BOOL bDeclareNewProductItemID = TRUE);

	// (z.manning 2010-06-23 12:10) - PLID 39311 - Moving some of the common code to create products in the db here from
	// InvNew dialog.
	// (j.jones 2015-03-03 14:40) - PLID 65111 - categories are no longer part of this function
	void AddCreateProductSqlToBatch(IN OUT CString &strSqlBatch, const CString strProductID, CString strName, BOOL bBillable, BOOL bTaxable1, BOOL bTaxable2, TrackStatus eTrackStatus, long nUserID);

	// (z.manning 2010-09-22 16:22) - PLID 40619
	//r.wilson 3/9/2012 PLID 46664 - Updated to take in whether or not to round up 
	// (b.eyers 2016-03-14) - PLID 68590 - RoundUp was changed from a bit to a long, at the addition of the item there are three 'rules' for rounding up
	// (j.jones 2016-05-17 11:13) - PLID-68615 - changed the price to a currency
	BOOL CalculateMarkupPrice(IN COleCurrency cyCost, IN CString strFormula, OUT COleCurrency &cyNewPrice, long nRoundUpRule);

	// (z.manning 2010-10-27 14:42) - PLID 40619 - Moved code here out of InvEditDlg
	BOOL UpdateSurgeriesForPriceChange(CWnd *pwndParent, IN OUT CParamSqlBatch &sqlBatch, const long nProductID, const COleCurrency cyNewPrice, BOOL bSilent = FALSE);
	BOOL UpdatePrefCardsForPriceChange(CWnd *pwndParent, IN OUT CParamSqlBatch &sqlBatch, const long nProductID, const COleCurrency cyNewPrice, BOOL bSilent = FALSE);

	// (j.gruber 2012-10-29 16:02) - PLID 53416 - created for
	// (j.gruber 2012-10-29 15:11) - PLID 53241 - moved here
	CString GetServiceLocationShopFeeAuditString(long nServiceID, CArray<long, long> *paryLocations);

	/// <summary>
	/// Should be called when receiving an order with serialized items to make sure
	/// another user hasn't already received them first.
	/// Given an order detail ID, this will return true if items have already been received.
	/// If so, the process of receiving should be cancelled.
	/// 
	/// This function does not need to know if we are only doing a partial receipt,
	/// because partial receipts split the order detail into a new one. If any product item
	/// has been received for our order detail, we cannot receive again.
	/// </summary>
	/// <param name="nOrderDetailID">The OrderDetailst.ID of the ordered product being received.</param>
	/// <returns>True if order detail has already been received, meaning the receiving process has to be aborted.
	/// False if the products can be safely marked received.</returns>
	bool HasOrderDetailReceivedProductItems(long nOrderDetailID);
	bool HasOrderDetailReceivedProductItems(std::vector<long> aryOrderDetailIDs);
}

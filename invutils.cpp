#include "stdafx.h"
#include "InvUtils.h"

#include "practice.h"
#include "mainfrm.h"
#include "GlobalUtils.h"

#include "Client.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"
#include "AuditTrail.h"
#include "InvTransferDlg.h"
#include "InternationalUtils.h"
#include "InvPatientAllocationDlg.h"
#include "MsgBox.h"
#include "TodoUtils.h"
#include "InvMultiSerialPickerDlg.h"
#include "NxExpression.h"
#include "MarkupFormulaEditDlg.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



namespace InvUtils
{
	// (a.walling 2013-02-26 11:07) - PLID 55333 - Do not define and implement CStrings in headers
	const char ostAll_Text[] = " <All Product Statuses>";
	const char ostAvailable_Text[] = "Available";
	const char ostAllocated_Text[] = "Allocated";
	const char ostUsed_Text[] = "Used";
	const char ostAdjusted_Text[] = "Adjusted / Returned";
	const char ostOnHand_Text[] = "On Hand";

//DRT 11/8/2007 - PLID 28042 - Added DefaultConsignment
// (j.jones 2007-11-28 08:43) - PLID 28196 - completed Allocations now decrement from inventory
// (j.jones 2007-12-17 10:59) - PLID 27988 - billed allocations do not decrement from inventory
// (j.jones 2007-12-18 11:10) - PLID 28037 - returned the count of uncompleted allocated items
// (c.haag 2008-02-07 12:35) - PLID 28853 - Now calculates consignment values, and returns the
// consignment reorder point and min. on hand
//DRT 2/7/2008 - PLID 28854 - Avail & AvailUO now reduce by the allocated products.
// (c.haag 2008-02-08 12:06) - PLID 28852 - Added Available fields for Consignment
// (c.haag 2008-02-08 15:44) - PLID 28852 - Implemented separate calculations for Consignment
// considering the HasSerialNum and HasExpDate flags, and cleaned up UO fields for ease of
// maintenance. *** IMPORTANT NOTE ABOUT PRODUCTITEMST FOR PRODUCTS THAT HAVE NEITHER FLAG SET:
// If a product has at least one serialize items when the user turns off serial tracking, those
// "dangling" items will NOT be used in any calculations.
// (c.haag 2008-02-21 09:29) - PLID 28852 - We are no longer using ConsignmentReOrderQuantity; but if
// we ever need to bring it back in the future, we can uncomment the ConsignmentReOrderQuantity line.
// (j.jones 2008-02-28 10:16) - PLID 28080 - actual and avail calculations now take case histories and
// allocations into account differently based on the license, the status of which must now be passed into
// this query. ***Since I added yet another parameter, I finally converted this into a function
// (j.jones 2008-03-07 14:55) - PLID 29204 - changed the 'avail' calculations to not be case or allocation-only
// based on the license, instead it calculates with the same logic as the 'actual', except that case history totals
// that aren't accounted for by an allocation will always decrement from Purchased stock, never Consignment
// This also means we no longer care what the license is.
// (c.haag 2008-04-24 10:00) - PLID 29770 - Added Active field
// (j.jones 2008-06-10 11:29) - PLID 27665 - added FacilityFee
//TES 6/16/2008 - PLID 27973 - Added Actual_StandardCalc, which returns the amount On Hand using the standard calculation,
// regardless of whether the product is serialized.  Also made it possible to pass in -1 to get the amounts for all locations.
//TES 7/3/2008 - PLID 24726 - Added SerialNumIsLotNum, a flag indicating whether the Serial Number should be treated as a 
// Lot Number (basically meaning that it doesn't have to be unique).
// (z.manning 2010-06-18 16:25) - PLID 39257 - Added FramesDataID
//(c.copits 2010-11-02) PLID 38598 - Warranty tracking system
//TES 5/24/2011 - PLID 43828 - Added IsContactLens

//TES 6/16/2008 - PLID 27973 - GetInventoryItemSql() uses this function to handle the case where they want all locations.
// (j.armen 2012-01-04 10:33) - PLID 29253 - Return a CSqlFragment for Parameratized GetInventoryItemSql()
CSqlFragment LocationFilter(const CString &strLocationField, long nLocationID)
{
	if(nLocationID == -1) {
		return CSqlFragment("1 = 1");
	}
	else {
		return CSqlFragment("{CONST_STRING} = {INT}", strLocationField, nLocationID);
	}
}

// (j.jones 2008-09-15 13:14) - PLID 31376 - convert this single product call into the multiple-product usage
// (j.jones 2009-01-07 16:26) - PLID 32648 - now this function calls the version of GetInventoryItemSql that takes in an IN clause
// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql(), passes along the array of Service ID's to be used in an IN clause
CSqlFragment GetInventoryItemSql(long nProductID, long nLocationID)
{
	CArray<long, long> aryServiceIDs;
	aryServiceIDs.Add(nProductID);

	return GetInventoryItemSql(aryServiceIDs, nLocationID);
}

// (j.jones 2008-09-15 13:13) - PLID 31376 - supported GetInventoryItemSql to be run for multiple products at a time
// (j.jones 2009-01-07 16:26) - PLID 32648 - now this function calls the version of GetInventoryItemSql that takes in an IN clause
// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql(), passes along a SqlFragment of Service ID's to be used in an IN clause
CSqlFragment GetInventoryItemSql(CArray<long, long> &aryServiceIDs, long nLocationID)
{
	return GetInventoryItemSql(CSqlFragment("{INTARRAY}", aryServiceIDs), nLocationID);
}

// (j.jones 2009-01-07 16:24) - PLID 32648 - supported GetInventoryItemSql to be run
// for multiple products, by taking in an IN clause instead of an array of IDs
// the in clause should be a query that would work when called like:
// "ProductT.ID IN (strInClause) "
// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql(), now takes in a SqlFragment IN Clause
// The query is broken up into multiple fragments based on subqueries
CSqlFragment GetInventoryItemSql(CSqlFragment sqlInClause, long nLocationID)
{
	// (j.armen 2011-12-20 15:43) - PLID 29253 - List of items to be selected from ProductMasterQ	
	// (j.gruber 2012-10-26 11:29) - PLID 53239 - multiLocationshopFees
	// (j.jones 2013-07-16 11:08) - PLID 57566 - added the NOCType
	// (a.wilson 2014-5-5) PLID 61831 - remove the old ServiceT.SendOrderingPhy field code.
	// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
	CSqlFragment sqlSelectProductMasterQ( 
		"ProductID, "
		"Actual, (CASE WHEN UseUU = 1 THEN Round(((Actual) / Convert(float,Conversion)),2) ELSE NULL END) As ActualUO, "
		"Avail, (CASE WHEN UseUU = 1 THEN Round(((Avail) / Convert(float,Conversion)),2) ELSE NULL END) As AvailUO, "
		"Ordered, (CASE WHEN UseUU = 1 THEN Round(((Ordered) / Convert(float,Conversion)),2) ELSE NULL END) As OrderedUO, "
		"ReOrderPoint, (CASE WHEN UseUU = 1 THEN Round(((ReOrderPoint) / Convert(float,Conversion)),2) ELSE NULL END) As ReOrderPointUO, "
		"ReOrderQuantity, (CASE WHEN UseUU = 1 THEN Round(((ReOrderQuantity) / Convert(float,Conversion)),2) ELSE NULL END) As ReOrderQuantityUO, "
		"ActualConsignment, (CASE WHEN UseUU = 1 THEN Round(((ActualConsignment) / Convert(float,Conversion)),2) ELSE NULL END) As ActualConsignmentUO, " 
		"AvailConsignment, (CASE WHEN UseUU = 1 THEN Round(((AvailConsignment) / Convert(float,Conversion)),2) ELSE NULL END) As AvailConsignmentUO, "
		"OrderedConsignment, (CASE WHEN UseUU = 1 THEN Round(((OrderedConsignment) / Convert(float,Conversion)),2) ELSE NULL END) As OrderedConsignmentUO, "
		"ConsignmentReOrderPoint, (CASE WHEN UseUU = 1 THEN Round(((ConsignmentReOrderPoint) / Convert(float,Conversion)),2) ELSE NULL END) As ConsignmentReOrderPointUO, "
		//"ConsignmentReOrderQuantity, (CASE WHEN UseUU = 1 THEN Round(((ConsignmentReOrderQuantity) / Convert(float,Conversion)),2) ELSE NULL END) As ConsignmentReOrderQuantityUO, "
		"(Actual - ActualConsignment) AS ActualPurchasedInv, (CASE WHEN UseUU = 1 THEN Round(((Actual - ActualConsignment) / Convert(float,Conversion)),2) ELSE NULL END) As ActualPurchasedInvUO, "
		"(Avail - AvailConsignment) AS AvailPurchasedInv, (CASE WHEN UseUU = 1 THEN Round(((Avail - AvailConsignment) / Convert(float,Conversion)),2) ELSE NULL END) As AvailPurchasedInvUO, "
		"(Ordered - OrderedConsignment) AS OrderedPurchasedInv, (CASE WHEN UseUU = 1 THEN Round(((Ordered - OrderedConsignment) / Convert(float,Conversion)),2) ELSE NULL END) As OrderedPurchasedInvUO, "
		"Name, Category, UnitDesc, Billable, Price, Taxable1, Taxable2, RevCodeUse, UB92Category, TrackableStatus, "
		"SupplierID, LastCost, Barcode, InsCode, Notes, Catalog, UseUU, UnitOfOrder, UnitOfUsage, Conversion, HasSerialNum, HasExpDate, "
		"SerialPerUO, IsEquipment, ProviderID, DefaultConsignment, OnHoldAllocationQty, CategoryID, Active, FacilityFee, "
		"Actual_StandardCalc, SerialNumIsLotNum, FramesDataID, WarrantyActive, WarrantyDays, IsContactLens, "
		" AdjustedShopFee, NOCType, RememberChargeProvider "
		);

	// (j.armen 2011-12-20 15:44) - PLID 29253 - List of items to be selected from ProductQ
	// (j.gruber 2012-10-26 12:55) - PLID 53239 - took out shopfee
	// (j.jones 2013-07-16 11:08) - PLID 57566 - added the NOCType
	// (j.jones 2016-01-19 08:46) - PLID 67680 - ensured the total inventory amount was rounded
	CSqlFragment sqlSelectProductQ(
		"ProductQ.ID AS ProductID, "
		/* Use the standard calculation only if no serial options are enabled,
		otherwise look explicitly at product items - return the grand total
		of product items so the dialog can simply subtract consignment values */
		"Round("
		"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
		"THEN Coalesce(AllProductItemsActualQ.Actual, 0) "
		"ELSE "
		"(CASE WHEN Sold Is Null THEN 0 ELSE 0 - Sold END) + "
		"(CASE WHEN Received Is Null THEN 0 ELSE Received END) + "
		"(CASE WHEN Adjusted Is Null THEN 0 ELSE Adjusted END) + "
		"(CASE WHEN TransferredTo Is Null THEN 0 ELSE TransferredTo END) + "
		"(CASE WHEN TransferredFrom Is Null THEN 0 ELSE TransferredFrom END) + "
		"(CASE WHEN CaseHistoryQty IS NULL THEN 0 ELSE 0 - CaseHistoryQty END) + "
		"(CASE WHEN UsedAllocationQty Is Null THEN 0 ELSE 0 - UsedAllocationQty END) "
		"END, 2) AS Actual, "

		//TES 6/16/2008 - PLID 27973 - In order to detect out-of-whack data, give the option to return the amount on hand
		// based on the standard calculation, even for serialized items.
		"Round("
		"(CASE WHEN Sold Is Null THEN 0 ELSE 0 - Sold END) + "
		"(CASE WHEN Received Is Null THEN 0 ELSE Received END) + "
		"(CASE WHEN Adjusted Is Null THEN 0 ELSE Adjusted END) + "
		"(CASE WHEN TransferredTo Is Null THEN 0 ELSE TransferredTo END) + "
		"(CASE WHEN TransferredFrom Is Null THEN 0 ELSE TransferredFrom END) + "
		"(CASE WHEN CaseHistoryQty IS NULL THEN 0 ELSE 0 - CaseHistoryQty END) + "
		"(CASE WHEN UsedAllocationQty Is Null THEN 0 ELSE 0 - UsedAllocationQty END) "
		", 2) AS Actual_StandardCalc, "

		/* Use the standard calculation only if no serial options are enabled,
		otherwise look explicitly at product items - return the grand total
		of product items so the dialog can simply subtract consignment values */
		/* the UnRelievedCaseHistoryQty needs subtracted from AllProductItemsAvailQ.Avail,
		if that field is used, otherwise the regular calculations such as
		UnRelievedCaseHistoryQty and OnHoldAllocationQty send back appropriate values*/
		"Round("
		"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
		"THEN Coalesce(AllProductItemsAvailQ.Avail, 0) "
		"	- Coalesce(UnRelievedCaseHistoryQty,0) "
		"ELSE "
		"(CASE WHEN Sold Is Null THEN 0 ELSE 0 - Sold END) + "
		"(CASE WHEN Received Is Null THEN 0 ELSE Received END) + "
		"(CASE WHEN Adjusted Is Null THEN 0 ELSE Adjusted END) + "
		"(CASE WHEN TransferredTo Is Null THEN 0 ELSE TransferredTo END) + "
		"(CASE WHEN TransferredFrom Is Null THEN 0 ELSE TransferredFrom END) + "
		"(CASE WHEN CaseHistoryQty IS NULL THEN 0 ELSE 0 - CaseHistoryQty END) + "
		"(CASE WHEN UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE 0 - UnRelievedCaseHistoryQty END) + "
		"(CASE WHEN UsedAllocationQty Is Null THEN 0 ELSE 0 - UsedAllocationQty END) + "
		"(CASE WHEN OnHoldAllocationsQ.OnHoldAllocationQty IS NULL THEN 0 ELSE 0 - OnHoldAllocationsQ.OnHoldAllocationQty END) "
		"END, 2) AS Avail, "

		"(CASE WHEN Ordered Is Null THEN 0 ELSE Ordered END) AS Ordered, "
		"(CASE WHEN ReOrderPoint Is Null THEN 0 ELSE ReOrderPoint END) AS ReOrderPoint, "
		"(CASE WHEN ReOrderQuantity Is Null THEN 0 ELSE ReOrderQuantity END) AS ReOrderQuantity, "

		"ProductQ.Name AS Name, CategoriesT.Name AS Category, UnitDesc, Billable, Price, Taxable1, Taxable2, RevCodeUse, "
		"UB92Category, TrackableStatus, SupplierID, LastCost, "
		"Barcode, InsCode, Notes, Catalog, CategoriesT.ID AS CategoryID, Active, FacilityFee, UseUU, UnitOfOrder, UnitOfUsage, Conversion, HasSerialNum, SerialNumIsLotNum, HasExpDate, SerialPerUO, "
		"IsEquipment, ProviderID, DefaultConsignment, Coalesce(OnHoldAllocationQty, Convert(float,0.0)) AS OnHoldAllocationQty, FramesDataID, WarrantyActive, WarrantyDays, "
		"IsContactLens, "

		/* Use the product item count only if serial options are enabled, otherwise always use 0 */
		"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
		"THEN Coalesce(ConsignmentProductsActualQ.ConsignmentActual, Convert(float,0)) "
		"ELSE Convert(float,0) END AS ActualConsignment, "

		/* Use the product item count only if serial options are enabled, otherwise always use 0 */
		"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
		"THEN COALESCE(Convert(float,(Coalesce(ConsignmentProductsAvailQ.ConsignmentAvail, 0))), Convert(float,0)) "
		"ELSE Convert(float,0) END AS AvailConsignment, "

		/* Use the product item count only if serial options are enabled, otherwise always use 0 */
		"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
		"THEN COALESCE(ConsignmentOrdered, Convert(int,0)) "
		"ELSE Convert(int,0) END AS OrderedConsignment, "

		/* Use the product item count only if serial options are enabled, otherwise always use 0 */
		"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
		"THEN COALESCE(ConsignmentReOrderPoint, Convert(int,0)) "
		"ELSE Convert(int,0) END AS ConsignmentReOrderPoint, "

		// (j.gruber 2012-10-26 15:30) - PLID 53239
		"CASE WHEN ManagedLocCount = CountShopFee THEN ShopFee ELSE CONVERT(money, -1) END as AdjustedShopFee, "

		// (j.jones 2013-07-16 11:08) - PLID 57566 - added the NOCType
		"NOCType, "

		// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
		"RememberChargeProvider "

		/* Use the product item count only if serial options are enabled, otherwise always use 0 */
		// (c.haag 2008-02-21 09:37) - PLID 28852 - We are no longer using ConsignmentReorderQuantity
		/*"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 " */
		/*"THEN COALESCE(ConsignmentReorderQuantity, Convert(int,0)) " */
		/*"ELSE Convert(int,0) END AS ConsignmentReorderQuantity " */);

	// (j.armen 2011-12-20 15:46) - PLID 29253 - List of Products
	// (j.gruber 2012-10-26 12:55) - PLID 53239 - took out shopfee
	// (j.jones 2013-07-16 11:08) - PLID 57566 - added the NOCType
	// (a.wilson 2014-5-5) PLID 61831 - remove the old ServiceT.SendOrderingPhy field code.
	// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
	CSqlFragment sqlProductQ(
		"SELECT "
		"	ProductT.ID, Name, UnitDesc, Billable, Price, Taxable1, Taxable2, RevCodeUse, "
		"	UB92Category, TrackableStatus, ReOrderPoint, ReorderQuantity, MultiSupplierT.SupplierID, Category, LastCost, "
		"	Barcode, InsCode, Notes, MultiSupplierT.Catalog, UseUU, UnitOfOrder, UnitOfUsage, Conversion, HasSerialNum, SerialNumIsLotNum, HasExpDate, SerialPerUO, IsEquipment, ProviderID, ProductT.DefaultConsignment, "
		"	ConsignmentReOrderPoint, Active, FacilityFee, "//, ConsignmentReorderQuantity " // (c.haag 2008-02-21 09:38) - We are no longer using ConsignmentReorderQuantity
		"	FramesDataID, WarrantyActive, WarrantyDays, IsContactLens, ServiceT.NOCType, ServiceT.RememberChargeProvider "
		"FROM ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
		"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
		"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
		"WHERE ProductT.ID IN ({SQL}) AND {SQL}",
		sqlInClause, LocationFilter("ProductLocationInfoT.LocationID", nLocationID));

	// (j.armen 2011-12-20 15:50) - PLID 29253 - Joined to BillsT in ChargeQ
	CSqlFragment sqlAllocationInfoQ(
		"SELECT "
		"	ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity "
		"FROM ChargedAllocationDetailsT "
		"INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
		"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"WHERE PatientInvAllocationDetailsT.Status = {CONST} AND PatientInvAllocationsT.Status = {CONST} "
		"GROUP BY ChargeID"
		, InvUtils::iadsUsed, InvUtils::iasCompleted);

	// (j.dinatale 2011-11-04 13:03) - PLID 46227 - need to exclude voiding and original line items so that way they do not impact our inventory counts
	// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
	// (j.armen 2011-12-20 15:51) - PLID 29253 - Joined to ProductQ'
	// (s.dhole 2012-03-26 12:05) - PLID 48597 Exclude Glasses order item qnt if mark as Off the shelf
	// (j.jones 2014-11-10 10:38) - PLID 63501 - reworked the IN clauses for GlassesOrderServiceT and BilledCaseHistoriesT such that they are now left joining
	// (r.goldschmidt 2015-06-08 15:56) - PLID 66234 - need to add some parentheses to correct a where clause
	/// <summary>
	/// Subquery that calculates and returns how many units of a given item at a given location have been Sold
	/// </summary>
	CSqlFragment sqlChargeQ(
		"SELECT "
		"	SUM (CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS Sold, ServiceID "
		"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
		"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"LEFT JOIN ({SQL}) AS AllocationInfoQ ON ChargesT.ID = AllocationInfoQ.ChargeID "
		"LEFT JOIN (SELECT BillID FROM BilledCaseHistoriesT GROUP BY BillID) AS BilledCaseHistoriesQ ON BillsT.ID = BilledCaseHistoriesQ.BillID "
		"LEFT JOIN CaseHistoryDetailsT ON CaseHistoryDetailsT.ItemID = ChargesT.ServiceID AND CaseHistoryDetailsT.ItemType = -1 "
		"LEFT JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID AND BilledCaseHistoriesT.BillID = BillsT.ID "
		"LEFT JOIN ("
		"	SELECT GlassesOrderServiceT.ID "
		"	FROM GlassesOrderServiceT "
		"	INNER JOIN ProductT ON GlassesOrderServiceT.ServiceID = ProductT.ID "
		"	WHERE GlassesOrderServiceT.IsDefaultProduct = 1 AND GlassesOrderServiceT.IsOffTheShelf = 0  "
		"	AND (ProductT.GlassesContactLensDataID IS NOT NULL OR ProductT.FramesDataID IS NOT NULL) "
		") AS GlassesOrderServiceQ ON ChargesT.GlassesOrderServiceID = GlassesOrderServiceQ.ID "
		"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Type = 10 AND ServiceID IN ({SQL}) AND {SQL} " 
		"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
		"AND BilledCaseHistoriesQ.BillID Is Null "
		"AND GlassesOrderServiceQ.ID Is Null "
		"GROUP BY ServiceID ",
		sqlAllocationInfoQ, sqlInClause, LocationFilter("LineItemT.LocationID", nLocationID));

	// (j.armen 2011-12-20 15:52) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlOrderQ(
		"SELECT "
		"	ProductID, SUM (QuantityOrdered) AS Received "
		"FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
		"WHERE OrderDetailsT.ProductID IN ({SQL}) AND {SQL} AND DateReceived IS NOT NULL "
			"AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0"
		"GROUP BY ProductID ",
		sqlInClause, LocationFilter("LocationID", nLocationID));
		
	// (j.armen 2011-12-20 15:52) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlOnOrderQ(
		"SELECT "
		"	ProductID, SUM (QuantityOrdered) AS Ordered "
		"FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
		"WHERE OrderDetailsT.ProductID IN ({SQL}) AND {SQL} AND DateReceived IS NULL "
		"AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0"
		"GROUP BY ProductID ",
		sqlInClause, LocationFilter("LocationID", nLocationID));

	// (j.armen 2011-12-20 15:53) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlAdjQ(
		"SELECT "
		"	SUM(Quantity) AS Adjusted, ProductID "
		"FROM ProductAdjustmentsT "
		"WHERE ProductID IN ({SQL}) AND {SQL} "
		"GROUP BY ProductID ",
		sqlInClause, LocationFilter("LocationID", nLocationID));

	// (j.armen 2011-12-20 15:53) - PLID 29253 - Joined to CaseHistoryDetailsT in UnRelievedCaseHistoryIndivSubQ
	CSqlFragment sqlUnRelievedLinkedAllocationQ(
		"SELECT "
		"	Sum(Quantity) AS ActiveAllocationQty, ProductID, CaseHistoryID "
		"FROM PatientInvAllocationsT "
		"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
		"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
		"WHERE PatientInvAllocationsT.Status = {CONST} "
		"	AND PatientInvAllocationDetailsT.Status = {CONST} "
		"	AND ProductID IN ({SQL}) AND {SQL} "
		"GROUP BY ProductID, CaseHistoryID"
		,InvUtils::iasActive, InvUtils::iadsActive, sqlInClause, LocationFilter("LocationID", nLocationID));

	// (j.armen 2011-12-20 15:55) - PLID 29253 - Subquery used by UnRelievedCaseHistorySubQ
	CSqlFragment sqlUnRelievedCaseHistoryIndivSubQ(
		"SELECT "
		"	(CASE WHEN SUM(Quantity) > SUM(Coalesce(ActiveAllocationQty,0)) THEN SUM(Quantity) - SUM(Coalesce(ActiveAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID "
		"FROM CaseHistoryDetailsT "
		"INNER JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
		"LEFT JOIN ({SQL}) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
		"WHERE ItemType = -1 AND CompletedDate Is Null AND ItemID IN ({SQL}) AND {SQL} "
		"GROUP BY ItemID, CaseHistoryT.ID ",
		sqlUnRelievedLinkedAllocationQ, sqlInClause, LocationFilter("LocationID", nLocationID));

	/* if there is a linked allocation with the same product, do not use the case history
	amount unless it is greater, in which case we use the overage*/
	// (j.armen 2011-12-20 15:56) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlUnRelievedCaseHistSubQ(
		"SELECT "
		"	Sum(CaseHistoryQty) AS UnRelievedCaseHistoryQty, ItemID "
		"FROM ({SQL}) AS CaseHistoryIndivSubQ "
		"GROUP BY ItemID ", 
		sqlUnRelievedCaseHistoryIndivSubQ);

	// (j.armen 2011-12-20 15:59) - PLID 29253 - Joined to CaseHistoryDetailsT, used in sqlCaseHistoryIndivSubQ
	CSqlFragment sqlLinkedAllocationQ(
		"SELECT "
		"	Sum(Quantity) AS UsedAllocationQty, ProductID, CaseHistoryID "
		"FROM PatientInvAllocationsT "
		"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
		"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
		"WHERE PatientInvAllocationsT.Status = {CONST} "
		"	AND PatientInvAllocationDetailsT.Status = {CONST} "
		"	AND ProductID IN ({SQL}) AND {SQL} "
		"GROUP BY ProductID, CaseHistoryID",
		InvUtils::iasCompleted, InvUtils::iadsUsed, sqlInClause, LocationFilter("LocationID", nLocationID));

	// (j.armen 2011-12-20 16:00) - PLID 29253 - Subquery used by CaseHistSubQ
	CSqlFragment sqlCaseHistoryIndivSubQ(
		"SELECT "
		"	(CASE WHEN SUM(Quantity) > SUM(Coalesce(UsedAllocationQty,0)) THEN SUM(Quantity) - SUM(Coalesce(UsedAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID "
		"FROM CaseHistoryDetailsT "
		"INNER JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
		"LEFT JOIN ({SQL}) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
		"WHERE ItemType = -1 AND CompletedDate Is Not Null AND ItemID IN ({SQL}) AND {SQL} "
		"GROUP BY ItemID, CaseHistoryT.ID ", 
		sqlLinkedAllocationQ, sqlInClause, LocationFilter("LocationID", nLocationID));

	/* if there is a linked allocation with the same product, do not use the case history
	amount unless it is greater, in which case we use the overage*/
	// (j.armen 2011-12-20 16:00) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlCaseHistSubQ(
		"SELECT "
		"	Sum(CaseHistoryQty) AS CaseHistoryQty, ItemID "
		"FROM ({SQL}) AS CaseHistoryIndivSubQ "
		"GROUP BY ItemID ",
		sqlCaseHistoryIndivSubQ);

	// (j.armen 2011-12-20 16:00) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlTransferredToSubQ(
		"SELECT "
		"	SUM(Amount) AS TransferredTo, ProductID "
		"FROM ProductLocationTransfersT "
		"WHERE ProductID IN ({SQL}) AND {SQL} "
		"GROUP BY ProductID ",
		sqlInClause, LocationFilter("DestLocationID", nLocationID));


	// (j.armen 2011-12-20 16:00) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlTransferredFromSubQ(
		"SELECT "
		"	-(SUM(Amount)) AS TransferredFrom, ProductID "
		"FROM ProductLocationTransfersT "
		"WHERE ProductID IN ({SQL}) AND {SQL} "
		"GROUP BY ProductID ",
		sqlInClause, LocationFilter("SourceLocationID", nLocationID));

	// (j.armen 2011-12-20 16:00) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlCompletedAllocationsQ(
		"SELECT "
		"	Sum(Quantity) AS UsedAllocationQty, ProductID "
		"FROM PatientInvAllocationsT "
		"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
		"WHERE PatientInvAllocationsT.Status = {CONST} "
		"	AND PatientInvAllocationDetailsT.Status = {CONST} "
		"	AND ProductID IN ({SQL}) AND {SQL} "
		"GROUP BY ProductID ",
		InvUtils::iasCompleted, InvUtils::iadsUsed, sqlInClause, LocationFilter("LocationID", nLocationID));

	// (j.armen 2011-12-20 16:01) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlOnHoldAllocationsQ(
		"SELECT "
		"	Sum(Quantity) AS OnHoldAllocationQty, ProductID "
		"FROM PatientInvAllocationsT "
		"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
		"WHERE PatientInvAllocationsT.Status = {CONST} "
		"	AND PatientInvAllocationDetailsT.Status = {CONST} "
		"	AND ProductID IN ({SQL}) AND {SQL} "
		"GROUP BY ProductID ", 
		InvUtils::iasActive, InvUtils::iadsActive, sqlInClause, LocationFilter("LocationID", nLocationID));

	// (j.armen 2011-12-20 16:01) - PLID 29253 - Joined to ProductQ
	// (j.jones 2014-11-10 14:18) - PLID 63501 - converted to use joins instead of a NOT IN clause
	CSqlFragment sqlAllProductItemsActualQ(
		"SELECT Count(ProductItemsT.ID) AS Actual, ProductItemsT.ProductID "
		"FROM ProductItemsT "
		"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"LEFT JOIN ("
		"	SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"		AND PatientInvAllocationsT.Status <> {CONST} "
		"		AND PatientInvAllocationDetailsT.Status = {CONST} "
		") AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"WHERE ChargedProductItemsT.ProductItemID Is Null "
		"AND AllocationsQ.ProductItemID Is Null "
		"AND ProductItemsT.Deleted = 0 "
		"AND {SQL} "
		"GROUP BY ProductItemsT.ProductID ", 
		InvUtils::iasDeleted, InvUtils::iadsUsed, LocationFilter("ProductItemsT.LocationID", nLocationID));
		
		// (j.armen 2011-12-20 16:01) - PLID 29253 - Joined to ProductQ
	// (j.jones 2014-11-10 14:18) - PLID 63501 - converted to use joins instead of a NOT IN clause
	CSqlFragment sqlAllProductItemsAvailQ(
		"SELECT Count(ProductItemsT.ID) AS Avail, ProductItemsT.ProductID "
		"FROM ProductItemsT "
		"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"LEFT JOIN ("
		"	SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"		AND PatientInvAllocationsT.Status <> {CONST} "
		"		AND PatientInvAllocationDetailsT.Status IN ({CONST},{CONST}) "
		") AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"WHERE ChargedProductItemsT.ProductItemID Is Null "
		"AND AllocationsQ.ProductItemID Is Null "
		"AND ProductItemsT.Deleted = 0 "
		"AND {SQL} "
		"GROUP BY ProductItemsT.ProductID ",
		InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed, LocationFilter("ProductItemsT.LocationID", nLocationID));

	// (j.armen 2011-12-20 16:01) - PLID 29253 - Joined to ProductQ
	// (j.jones 2014-11-10 14:18) - PLID 63501 - converted to use joins instead of a NOT IN clause
	CSqlFragment sqlConsignmentProductsActualQ(
		"SELECT Count(ID) AS ConsignmentActual, ProductItemsT.ProductID "
		"FROM ProductItemsT "
		"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"LEFT JOIN ("
		"	SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"		AND PatientInvAllocationsT.Status <> {CONST} "
		"		AND PatientInvAllocationDetailsT.Status = {CONST} "
		") AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"WHERE ChargedProductItemsT.ProductItemID Is Null "
		"AND AllocationsQ.ProductItemID Is Null "
		"AND ProductItemsT.Status = {CONST} "
		"AND ProductItemsT.Deleted = 0 "
		"AND {SQL} "
		"GROUP BY ProductItemsT.ProductID ", 
		InvUtils::iasDeleted, InvUtils::iadsUsed, InvUtils::pisConsignment,	LocationFilter("ProductItemsT.LocationID", nLocationID));

	// (j.armen 2011-12-20 16:01) - PLID 29253 - Joined to ProductQ
	// (j.jones 2014-11-10 14:18) - PLID 63501 - converted to use joins instead of a NOT IN clause
	CSqlFragment sqlConsignmentProductsAvailQ(
		"SELECT Count(ProductItemsT.ID) AS ConsignmentAvail, ProductItemsT.ProductID "
		"FROM ProductItemsT "
		"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"LEFT JOIN ("
		"	SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"		AND PatientInvAllocationsT.Status <> {CONST} "
		"		AND PatientInvAllocationDetailsT.Status IN ({CONST},{CONST}) "
		") AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"WHERE ChargedProductItemsT.ProductItemID Is Null "
		"AND AllocationsQ.ProductItemID Is Null "
		"AND ProductItemsT.Status = {CONST} "
		"AND ProductItemsT.Deleted = 0 "
		"AND {SQL} "
		"GROUP BY ProductItemsT.ProductID ", 
		InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed, InvUtils::pisConsignment, LocationFilter("ProductItemsT.LocationID", nLocationID));

		// (j.armen 2011-12-20 16:01) - PLID 29253 - Joined to ProductQ
	CSqlFragment sqlConsignmentOnOrderQ(
		"SELECT "
		"	ProductID, SUM (QuantityOrdered) AS ConsignmentOrdered "
		"FROM OrderDetailsT "
		"INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
		"WHERE {SQL} AND DateReceived IS NULL "
		"	AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 AND ForStatus = {CONST} "
		"GROUP BY ProductID"
		,LocationFilter("LocationID", nLocationID), InvUtils::odsConsignment);

	// (j.gruber 2012-10-26 11:32) - PLID 53239 - ShopFeeQ joined to productQ
	CSqlFragment sqlShopFeeQ(" SELECT ServiceID, ManagedLocCount, Min(ShopFee) as ShopFee, Min(CountshopFees) as CountShopFee FROM "
		"	(SELECT (SELECT Count(*) FROM LocationsT WHERE Managed = 1 AND Active = 1) as ManagedLocCount,  "
		"  	ServiceID, ShopFee, Count(*) as CountShopFees  "
		" 	FROM ServiceLocationInfoT  INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID "
		"   WHERE ServiceLocationInfoT.ServiceID IN ({SQL}) AND LocationsT.Managed = 1 AND LocationsT.Active = 1 "
		" 	GROUP BY ServiceID, ShopFee) InnerShopFeeQ "
		" 	GROUP BY ServiceID, ManagedLocCount ", sqlInClause);

	// (j.armen 2011-12-20 16:01) - PLID 29253 - Assemble all of the fragments we generated above and return the resulting fragment
	// (j.gruber 2012-10-26 11:35) - PLID 53239 - added shop fee
	return CSqlFragment(
		"SELECT {SQL} "
		"FROM ("
		"	SELECT {SQL} FROM ({SQL}) AS ProductQ "
		"	LEFT JOIN ({SQL}) AS ChargeQ ON ProductQ.ID = ChargeQ.ServiceID "
		"	LEFT JOIN ({SQL}) AS OrderQ ON ProductQ.ID = OrderQ.ProductID "
		"	LEFT JOIN ({SQL}) AS OnOrderQ ON ProductQ.ID = OnOrderQ.ProductID "
		"	LEFT JOIN ({SQL}) AS AdjQ ON ProductQ.ID = AdjQ.ProductID "
		"	LEFT JOIN ({SQL}) AS UnRelievedCaseHistSubQ ON ProductQ.ID = UnRelievedCaseHistSubQ.ItemID "
		"	LEFT JOIN ({SQL}) AS CaseHistSubQ ON ProductQ.ID = CaseHistSubQ.ItemID "
		"	LEFT JOIN ({SQL}) AS TransferredToSubQ ON ProductQ.ID = TransferredToSubQ.ProductID "
		"	LEFT JOIN ({SQL}) AS TransferredFromSubQ ON ProductQ.ID = TransferredFromSubQ.ProductID "
		"	LEFT JOIN ({SQL}) AS CompletedAllocationsQ ON ProductQ.ID = CompletedAllocationsQ.ProductID "
		"	LEFT JOIN ({SQL}) AS OnHoldAllocationsQ ON ProductQ.ID = OnHoldAllocationsQ.ProductID "
		"	LEFT JOIN ({SQL}) AS AllProductItemsActualQ ON ProductQ.ID = AllProductItemsActualQ.ProductID "
		"	LEFT JOIN ({SQL}) AS AllProductItemsAvailQ ON ProductQ.ID = AllProductItemsAvailQ.ProductID "
		"	LEFT JOIN ({SQL}) AS ConsignmentProductsActualQ ON ProductQ.ID = ConsignmentProductsActualQ.ProductID "
		"	LEFT JOIN ({SQL}) AS ConsignmentProductsAvailQ ON ProductQ.ID = ConsignmentProductsAvailQ.ProductID "
		"	LEFT JOIN ({SQL}) AS ConsignmentOnOrderQ ON ProductQ.ID = ConsignmentOnOrderQ.ProductID "
		"   LEFT JOIN ({SQL}) AS ShopFeeInfoQ ON ProductQ.ID = ShopFeeInfoQ.ServiceID "
		"	LEFT JOIN CategoriesT ON ProductQ.Category = CategoriesT.ID"
		") ProductMasterQ",
		sqlSelectProductMasterQ, sqlSelectProductQ,	sqlProductQ, sqlChargeQ,
		sqlOrderQ, sqlOnOrderQ, sqlAdjQ, sqlUnRelievedCaseHistSubQ, sqlCaseHistSubQ, 
		sqlTransferredToSubQ, sqlTransferredFromSubQ, sqlCompletedAllocationsQ, 
		sqlOnHoldAllocationsQ, sqlAllProductItemsActualQ, sqlAllProductItemsAvailQ,
		sqlConsignmentProductsActualQ, sqlConsignmentProductsAvailQ, sqlConsignmentOnOrderQ, sqlShopFeeQ);
}

_RecordsetPtr m_rsCategories = NULL;
long m_def_category = 0;
long m_def_item = 0;
bool m_initialized = false;

// (a.walling 2007-06-06 13:25) - PLID 26238 - Safe table checker (cleans up data when going out of scope)
CSafeTableChecker m_sCategoryChecker;

void EnsureCategoryData()
{
	// (a.walling 2007-06-06 13:16) - PLID 26238 - Recreate the data if we have a category checker object and it has changed
	// This used to never be updated until Practice has restarted.
	if (m_rsCategories == NULL || m_sCategoryChecker.Changed() )
	{
		if (m_rsCategories != NULL) {
			m_rsCategories->Close();
			m_rsCategories.Release();
		}

		m_rsCategories = CreateRecordset(adOpenStatic, 
										adLockReadOnly, 
										"SELECT ID, Parent FROM CategoriesT");
		m_rsCategories->PutRefActiveConnection(NULL);//disconnect

		// (a.walling 2007-06-06 13:15) - PLID 26238 - Create a table checker instance
		if (!m_sCategoryChecker.Attached()) 
			m_sCategoryChecker.Attach(NetUtils::CPTCategories);
	}
}

CString Descendants(long id)
{
	//returns a string for a WHERE clause using all descendants of the source id
	CString sql, strNumber;
	CArray<long,long> array;
	array.Add(id);
	Descendants(array);
	
	sql = " IN (";
	for (long i = 0; i < array.GetSize(); i++)
	{	strNumber.Format ("%i,", array[i]);
		sql += strNumber;
	}
	sql.SetAt(sql.GetLength() - 1, ')');//replace the last , with a )
	return sql;
}

void Descendants(CArray<long,long> &array)
{
	EnsureCategoryData();

	for (long i = 0; i < array.GetSize(); i++)
	{	m_rsCategories->MoveFirst();
		while (!m_rsCategories->eof)
		{	if (m_rsCategories->Fields->GetItem("Parent")->Value.lVal == array.GetAt(i))
				array.Add(m_rsCategories->Fields->GetItem("ID")->Value.lVal);
			m_rsCategories->MoveNext();
		}
	}
}

void SaveDefaultParameters()
{
	SetRemotePropertyInt("LastItemCategory", m_def_category,	-1, GetCurrentUserName());
	SetRemotePropertyInt("LastItem", m_def_item,				-1, GetCurrentUserName());
}


void LoadDefaultParameters()
{
	m_initialized = true;
	m_def_category = GetRemotePropertyInt("LastItemCategory",	0L, -1, GetCurrentUserName());
	m_def_item = GetRemotePropertyInt("LastItem",				0L, -1, GetCurrentUserName());
}

long GetDefaultCategory()
{
	if (!m_initialized)
		LoadDefaultParameters();
	return m_def_item;
}

long GetDefaultItem()
{	
	if (!m_initialized)
		LoadDefaultParameters();
	return m_def_item;
}

void SetDefault (long item, long category)
{
	m_initialized = true;
	m_def_item = item;
	m_def_category = category;
}

// (j.jones 2007-12-18 11:14) - PLID 28037 - converted to handle allocated counts
void DoIHaveEnough(long nServiceID, long nLocationID , double dblInStockAdjustment /*= 0.0*/, double dblAllocatedAdjustment /*= 0.0*/, BOOL bQuote /*= FALSE*/)
//called to warn the user we don't have enough of an item
{
	try {

		CString id, loc;
		double dblOnHand = 0.0;
		double dblAllocated = 0.0;
	
		if(!CalcAmtOnHand(nServiceID, nLocationID, dblOnHand, dblAllocated, dblInStockAdjustment, dblAllocatedAdjustment)) {
			return;
		}

		if (dblOnHand - dblAllocated <= 0) {
			long nAvailLoc = -1;
			//see if they have more than one managed location, and if another location has items in stock
			_RecordsetPtr rsLoc = CreateRecordset("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1 AND ID <> %li", nLocationID);
			while(!rsLoc->eof && nAvailLoc == -1) {
				long nLocID = AdoFldLong(rsLoc, "ID");				
				// (j.jones 2008-02-28 10:40) - PLID 28080 - converted item_sql into GetInventoryItemSql()
				// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
				_RecordsetPtr rsItems = CreateParamRecordset(GetInventoryItemSql(nServiceID, nLocID));
				if(!rsItems->eof) {
					if(AdoFldDouble(rsItems, "Actual",0.0) > 0) {
						nAvailLoc = nLocID;
					}
				}
				rsItems->Close();

				rsLoc->MoveNext();				
			}
			rsLoc->Close();

			CString strName = "";

			_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li", nServiceID);
			if(!rs->eof) {
				strName = AdoFldString(rs, "Name","");
			}
			rs->Close();

			CString strAllocated = "";
			if(dblAllocated > 0.0) {
				strAllocated.Format(", and %g allocated to patients", dblAllocated);
			}

			// (j.jones 2010-05-07 15:26) - PLID 37695 - check the permission, we will warn if they are not allowed
			// to bill out of stock products, but quoting is fine
			BOOL bCanBillOutOfStock = (GetCurrentUserPermissions(bioBill) & sptDynamic1) > 0 ? TRUE : FALSE;
			BOOL bCanBillOutOfStockWithPass = (GetCurrentUserPermissions(bioBill) & sptDynamic1WithPass) > 0 ? TRUE : FALSE;

			CString strPermissionWarning = "";
			// (j.jones 2010-08-13 17:03) - PLID 37695 - only warn if they are billing less than zero, not actual zero
			if(!bCanBillOutOfStock && dblOnHand - dblAllocated < 0) {
				if(bCanBillOutOfStockWithPass) {
					strPermissionWarning = "\n\nYour permissions require a password to be entered when you save a bill with out of stock products.";
				}
				else {
					strPermissionWarning = "\n\nYou do not have permission to save a bill with out of stock products.";
				}
			}
			
			// (z.manning, 08/03/2007) - PLID 26140 - There's now a preference for whether or not we should
			// ever prompt users to transfer items from another location.
			if(nAvailLoc != -1 && dblOnHand - dblAllocated < 0 && GetRemotePropertyInt("BillPromptTransferInventoryLocation", 1, 0, "<None>", true) == 1) {
				//another location has items in stock, so let's prompt them to transfer,
				//but only do so if we are going to have less than zero on hand
				CString str;
				if(!bQuote) {
					//for a bill
					str.Format("Warning: Practice shows you are now out of stock of %s\n"
						"(after saving the bill, there will be %g on hand%s).%s\n\n"
						"Would you like to transfer items from another location?", strName, dblOnHand, strAllocated, strPermissionWarning);
				}
				else {
					//for a quote
					str.Format("Warning: Practice shows that once this quote is billed,\n"
						"you will be out of stock of %s (potentially %g on hand%s).\n\n"
						"Would you like to transfer items from another location?", strName, dblOnHand, strAllocated);
				}
				if(IDYES == MessageBox(GetActiveWindow(),str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
					CInvTransferDlg dlg(NULL);
					dlg.DoModal(nServiceID, nAvailLoc, nLocationID);
				}
			}
			else {
				//else warn as usual
				if(!bQuote) {
					//for a bill
					MsgBox("Warning: Practice shows you are now out of stock of %s\n"
						"(after saving the bill, there will be %g on hand%s).%s", strName, dblOnHand, strAllocated, strPermissionWarning);
				}
				else {
					//for a quote
					MsgBox("Warning: Practice shows that once this quote is billed,\n"
						"you will be out of stock of %s (potentially %g on hand%s).", strName, dblOnHand, strAllocated);
				}
			}
		}
	}
	NxCatchAll("Could not determine inventory quantity.");
}

// (j.jones 2007-12-18 10:47) - PLID 28037 - converted to also return allocated counts
BOOL CalcAmtOnHand(long nServiceID, long nLocationID, double &dblOnHand, double &dblAllocated, double dblInStockAdjustment /*= 0.0*/, double dblAllocatedAdjustment /*= 0.0*/)
{
	_RecordsetPtr rs;
	FieldsPtr fields;
	
	//taken from CInvEditDlg::Refresh()

	try
	{
		EnsureRemoteData();

		// (j.jones 2008-02-28 10:40) - PLID 28080 - converted item_sql into GetInventoryItemSql()
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		rs = CreateParamRecordset(GetInventoryItemSql(nServiceID, nLocationID));

		if (rs->eof) {

			if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE Managed = 1 AND ID = %li AND TypeID = 1", nLocationID)) {
				//the location is no longer managed, so we are no longer tracking inventory for it,
				//and in turn we do not need to warn
				dblOnHand = 0.0;
				dblAllocated = 0.0;
				return FALSE;
			}
			else {
				//if the recordset is empty and it's not because of a location being unmanaged, then
				//it is most likely not an inventory item, which is an error state
				HandleException(NULL, "DoIHaveEnough called for non inventory item");
				dblOnHand = 0.0;
				dblAllocated = 0.0;
				return FALSE;
			}
		}

		fields = rs->Fields;

		if (AdoFldLong(fields, "TrackableStatus") != 2) {
			return FALSE;
		}

		dblOnHand = AdoFldDouble(fields, "Actual") + dblInStockAdjustment;
		dblAllocated = AdoFldDouble(fields, "OnHoldAllocationQty") + dblAllocatedAdjustment;

		return TRUE;

	}
	NxCatchAll("Could not calculate inventory quantity.");

	return FALSE;
}

void CreateInventoryTodoAlarm(long service, long location, TodoType tt,
							  FieldsPtr& fProduct, long nCatID)
{
	// (c.haag 2008-02-07 13:25) - PLID 28853 - This function creates an inventory-related
	// todo alarm. This code was moved from EnsureInventoryTodoAlarms.
	variant_t var;

	var = fProduct->Item["SupplierID"]->Value;

	//(e.lally 2007-05-04) PLID 25253 - Allow Todo to be created when no supplier exists
	// (c.haag 2008-06-09 11:48) - PLID 30321 - Now covered by TodoCreate. Passing in a person
	// ID of -1 is the same as writing a NULL to the new record.
	//CString strSupplierID;
	//if 	(var.vt != VT_I4) //we don't have a supplier 
	//	strSupplierID = "NULL";
	//else
	//	strSupplierID.Format("%li", VarLong(var));

	//open the list of users responsible for this item
	_RecordsetPtr rsResp = CreateParamRecordset("SELECT UserID FROM ProductResponsibilityT "
		"WHERE LocationID = {INT} AND ProductID = {INT}", location, service);
	if (!rsResp->eof)
	{
		FieldsPtr fResp = rsResp->Fields;
		while (!rsResp->eof)
		{	
			CString strNotes = AdoFldString(fProduct, "Name");
			// (c.haag 2008-02-08 17:07) - PLID 28853 - Make sure the user is aware that this
			// is for consignment
			if (ttConsignmentInvItem == tt) {
				strNotes += " (For Consignment)";
			}

			// (c.haag 2008-06-09 11:46) - PLID 30321 - Use the new global utility function
			long nTaskID = TodoCreate(COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), AdoFldLong(fResp, "UserID"),
				strNotes, "", service, tt, VarLong(var, -1), location, ttpLow, nCatID);
			// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
			CClient::RefreshTodoTable(nTaskID,-1 ,AdoFldLong(fResp, "UserID",-1),TableCheckerDetailIndex::tddisAdded);
		
			rsResp->MoveNext();
		}			
	}
}

// (c.haag 2008-02-07 13:15) - PLID 28853 - Renamed from ChangeInventoryQuantity to something more accurate.
// Also removed the quantity parameter.
// (j.jones 2008-09-15 12:55) - PLID 31376 - EnsureInventoryTodoAlarms now supports being called
// with an array of service IDs, to check on multiple products more efficiently
//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
void EnsureInventoryTodoAlarms(CArray<long, long> &aryServiceIDs, long nLocationID, bool bInTransaction)
//called whenever inventory count in stock changes
//Creates and deletes todo items for users
//does not actually change the inventory amount in stock - calling function should do that
{	

	_RecordsetPtr rsProduct;
	FieldsPtr fProduct;
	
	try {

		if(aryServiceIDs.GetSize() == 0 || nLocationID == -1) {
			//do nothing if we have invalid data,
			//but assert if we hit this case so we can find out why
			ASSERT(FALSE);
			return;
		}

		EnsureRemoteData();

		// (j.jones 2008-09-15 13:06) - PLID 31376 - get a comma-delimited list of IDs
		CString strServiceIDs;
		for(int i=0; i<aryServiceIDs.GetSize(); i++) {
			if(!strServiceIDs.IsEmpty()) {
				strServiceIDs += ",";
			}
			strServiceIDs += AsString(aryServiceIDs.GetAt(i));
		}

		// (c.haag 2008-06-12 10:13) - PLID 28474 - Use this handy function to get the category ID
		//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
		long nCatID = GetInvOrderTodoCategory(bInTransaction);

		//Clear previous todo items for these inventory items
		// (c.haag 2008-02-07 12:34) - PLID 28853 - Include both Purchased and Consignment alarms
		// (c.haag 2008-06-11 11:09) - PLID 30328 - Also delete from TodoAssignToT
		// (j.jones 2008-09-15 13:06) - PLID 31376 - supported multiple products
		long nRecordsAffected = 0;		
		ExecuteSql("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM TodoList "
			"WHERE LocationID = %li AND RegardingID IN (%s) AND RegardingType IN (%li, %li))",
			nLocationID, strServiceIDs, ttPurchasedInvItem, ttConsignmentInvItem);

		//needs to be separate from the prior execute so we can track nRecordsAffected
		ExecuteSql(&nRecordsAffected, adCmdText, 
			"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
			"DELETE FROM TodoList "
			"WHERE LocationID = %li AND RegardingID IN (%s) AND RegardingType IN (%li, %li)",
			nLocationID, strServiceIDs, ttPurchasedInvItem, ttConsignmentInvItem);

		if(nRecordsAffected != 0) {
			//we deleted todo alarms so we have to update all of them
			CClient::RefreshTable(NetUtils::TodoList, -1);
		}

		//Get Item Properties

		// (j.jones 2008-02-28 10:40) - PLID 28080 - converted item_sql into GetInventoryItemSql()
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		rsProduct = CreateParamRecordset(GetInventoryItemSql(aryServiceIDs, nLocationID));
		
		if(rsProduct->eof) {

			// (c.haag 2008-03-12 16:31) - PLID 29115 - Because it no longer matters whether the location is managed or not,
			// don't bother querying for it.
			//if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE Managed = 1 AND TypeID = 1 AND ID = %li", location)) {
				//the location is no longer managed, so we are no longer tracking inventory for it,
				//and in turn we do not need to create a todo alarm				
			//	return;
			//}
			//else {
				//if the recordset is empty and it's not because of a location being unmanaged, then
				//it is most likely not an inventory item, which is an error state
				// (c.haag 2008-03-12 16:25) - PLID 29115 - This is no longer true, necessarily. If we delete
				// an inventory item, EnsureInventoryTodoAlarms will get called for the purpose of clearing out
				// todo alarms for the product at the given location. If we get here through any other circumstance,
				// it still means the product is unavailable, and so we can't create todo alarms for it anyway.
				//ASSERT(FALSE);
				//HandleException(NULL, "EnsureInventoryTodoAlarms called for non inventory item");
				return;
			//}
		}

		// (j.jones 2008-09-15 13:19) - PLID 31376 - this can now return multiple records
		while(!rsProduct->eof) {

			fProduct = rsProduct->Fields;

			if (AdoFldLong(fProduct, "TrackableStatus") != 2)
				return;//item not trackable, so we don't care how many we have

			// (c.haag 2008-04-24 10:00) - PLID 29770 - If the product is inactive, do not create any alarms
			if (!AdoFldBool(fProduct, "Active")) {
				return;
			}

			long nProductID = VarLong(fProduct->Item["ProductID"]->Value);

			// (c.haag 2008-02-07 12:46) - PLID 28853 - The historic code is now contained completely within
			// its own conditional. Also changing from considering ALL kinds of inventory items to just Purchased
			// inventory items. Purchased inventory items are those that are not Consignment items. To do a proper 
			// comparison, we must do an epic math maneuver which includes consignment fields.
			
			const double dActualPurchased = AdoFldDouble(fProduct, "ActualPurchasedInv");
			const double dAvailPurchased = AdoFldDouble(fProduct, "AvailPurchasedInv");
			const double dOrderedPurchased = (double)AdoFldLong(fProduct, "OrderedPurchasedInv");
			const double dReorderPointPurchased = (double)AdoFldLong(fProduct, "ReOrderPoint");

			// (j.jones 2008-02-14 14:25) - PLID 28864 - now we have a preference to compare actual or available
			// to the reorder point, so we need to account for it here
			long nOrderByOnHandAmount = GetRemotePropertyInt("InvItem_OrderByOnHandAmount", 0, 0, "<None>", true);
			BOOL bOrderByActual = (nOrderByOnHandAmount == 0);

			double dblValueToCheck = 0.0;
			if(bOrderByActual) {
				dblValueToCheck = dActualPurchased;
			}
			else {
				dblValueToCheck = dAvailPurchased;
			}

			// (c.haag 2008-04-24 10:02) - PLID 29770 - Don't create purchased inventory todo alarms if
			// the general reorder point is non-positive
			if (dReorderPointPurchased > 0 && (dblValueToCheck + dOrderedPurchased <= dReorderPointPurchased))
			{
				// If we get here, the sum of the actual and ordered are less than or equal to the reorder point
				CreateInventoryTodoAlarm(nProductID, nLocationID, ttPurchasedInvItem, fProduct, nCatID);
			}

			// (c.haag 2008-02-07 13:23) - PLID 28853 - Also create a separate alarm for Consignment-only
			// items, if necessary.
			if (AdoFldBool(fProduct, "HasSerialNum", FALSE) || AdoFldBool(fProduct, "HasExpDate", FALSE)) {
				const double dActualConsignment = AdoFldDouble(fProduct, "ActualConsignment");
				const double dAvailConsignment = AdoFldDouble(fProduct, "AvailConsignment");
				const double dOrderedConsignment = AdoFldLong(fProduct, "OrderedConsignment");
				const double dConsignmentLevel = AdoFldLong(fProduct, "ConsignmentReOrderPoint");

				// (j.jones 2008-02-14 14:25) - PLID 28864 - now we have a preference to compare actual or available
				// to the reorder point, so we need to account for it here
				if(bOrderByActual) {
					dblValueToCheck = dActualConsignment;
				}
				else {
					dblValueToCheck = dAvailConsignment;
				}

				// (c.haag 2008-04-24 10:02) - PLID 29770 - Don't create consignment inventory todo alarms if
				// the consignment level is non-positive
				if (dConsignmentLevel > 0 && (dblValueToCheck + dOrderedConsignment < dConsignmentLevel))
				{
					// If we get here, the sum of the actual and ordered are less than or equal to the reorder point
					CreateInventoryTodoAlarm(nProductID, nLocationID, ttConsignmentInvItem, fProduct, nCatID);
				}
			}

			rsProduct->MoveNext();
		}
		rsProduct->Close();

	}NxCatchAll("Error in EnsureInventoryTodoAlarms");
}

// (c.haag 2008-02-27 13:45) - PLID 29115 - Inventory todo alarm transaction map structure

// This structure represents a single element (inventory product or serialized item)
struct ITTI // Stands for Inventory Todo Transaction Item
{
	EInvTodoAlarmTransItemType IDType;
	long nID;
	// Special case where the ID type actually indicates two ID's (only true if
	// IDType equals eInvTrans_ProductIDLocationID)
	long nProductID, nLocationID;

	BOOL operator ==(ITTI &cmp) {
		if (IDType != cmp.IDType) { return FALSE; }
		if (nID != cmp.nID) { return FALSE; }
		if (nProductID != cmp.nProductID) { return FALSE; }
		if (nLocationID != cmp.nLocationID) { return FALSE; }
		return TRUE;
	}

};

// This is an array of elements
typedef CArray<ITTI,ITTI&> CITTIArray;

// This maps transaction ID's to arrays of elements
static CMap<int, int, CITTIArray*, CITTIArray*> m_mapInvTodoTrans;

// This is used in generating transaction ID's
static long g_nNewInvTodoTransactionID = 1;

int BeginInventoryTodoAlarmsTransaction()
{
	// (c.haag 2008-02-27 13:44) - PLID 29115 - This function will begin a transaction that, when
	// committed, will update inventory todo alarms based on inventory products and product items
	// that have changed
	int nTransID = g_nNewInvTodoTransactionID++;
	m_mapInvTodoTrans.SetAt(nTransID, new CITTIArray);
	return nTransID;
}

void AddToInventoryTodoAlarmsTransaction(int nTransID, EInvTodoAlarmTransItemType idtype, long nID)
{
	// (c.haag 2008-02-27 13:52) - PLID 29115 - This function adds an element to an inventory
	// todo alarm transaction
	ITTI e;
	e.IDType = idtype;
	e.nID = nID;
	e.nProductID = -1; // Unused here
	e.nLocationID = -1; // Unused here
	CITTIArray* pary = NULL;
	if (!m_mapInvTodoTrans.Lookup(nTransID, pary) || NULL == pary) {
		ASSERT(FALSE);
		ThrowNxException("AddToInventoryTodoAlarmsTransaction tried to populate for invalid transaction ID %d!", nTransID);
	}

	// Validation
	if (-1 == nID) {
		ASSERT(FALSE);
		ThrowNxException("AddToInventoryTodoAlarmsTransaction called without a valid ID!");
	}

	// Check for duplicates
	const int nSize = pary->GetSize();
	for (int i=0; i < nSize; i++) {
		if (pary->GetAt(i) == e) {
			return; // This element already exists. Don't need to do anything
		}
	}

	// Add the element to the array
	pary->Add(e);
}

void AddProductLocToInventoryTodoAlarmsTransaction(int nTransID, long nProductID, long nLocationID)
{
	// (c.haag 2008-02-29 16:38) - PLID 29115 - This function adds an element to an inventory
	// todo alarm transaction
	ITTI e;
	e.IDType = eInvTrans_ProductIDLocationID;
	e.nID = -1; // Unused here
	e.nProductID = nProductID;
	e.nLocationID = nLocationID;
	CITTIArray* pary = NULL;
	if (!m_mapInvTodoTrans.Lookup(nTransID, pary) || NULL == pary) {
		ASSERT(FALSE);
		ThrowNxException("AddProductLocToInventoryTodoAlarmsTransaction tried to populate for invalid transaction ID %d!", nTransID);
	}

	// Validation
	if (-1 == nProductID || -1 == nLocationID) {
		ASSERT(FALSE);
		ThrowNxException("AddProductLocToInventoryTodoAlarmsTransaction called without a valid ID!");
	}

	// Check for duplicates
	const int nSize = pary->GetSize();
	for (int i=0; i < nSize; i++) {
		if (pary->GetAt(i) == e) {
			return; // This element already exists. Don't need to do anything
		}
	}

	// Add the element to the array
	pary->Add(e);
}

//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
void CommitInventoryTodoAlarmsTransaction(int nTransID, bool bInSqlTransaction)
{
	// (c.haag 2008-02-29 12:11) - PLID 29115 - This function commits an inventory todo ensurance
	// transaction. The goal is to call EnsureInventoryTodoAlarms as few times as possible given
	// the contents of m_mapInvTodoTrans
	CITTIArray* pary = NULL;
	if (!m_mapInvTodoTrans.Lookup(nTransID, pary) || NULL == pary) {
		ASSERT(FALSE);
		ThrowNxException("Called CommitInventoryTodoAlarmsTransaction for invalid transaction ID %d!", nTransID);
	}
	const int nSize = pary->GetSize();
	if (nSize > 0) {
		CArray<long,long> anProductIDs;
		CArray<long,long> anProductItemIDs;
		CArray<long,long> anAllocationIDs;
		CArray<long,long> anPatientIDs;
		CArray<long,long> anDualID_ProductIDs;
		CArray<long,long> anDualID_LocationIDs;
		int i;

		for (i=0; i < nSize; i++) {
			const ITTI e = pary->GetAt(i);

			// Append values to arrays of product ID's and product item ID's. We will use these arrays
			// to generate values that we need to call EnsureInventoryTodoAlarms.
			switch (e.IDType) {
			case eInvTrans_ProductID:
				anProductIDs.Add(e.nID);
				break;
			case eInvTrans_ProductItemID:
				anProductItemIDs.Add(e.nID);
				break;
			case eInvTrans_AllocationID:
				anAllocationIDs.Add(e.nID);
				break;
			case eInvTrans_PatientID:
				anPatientIDs.Add(e.nID);
				break;
			case eInvTrans_ProductIDLocationID:
				anDualID_ProductIDs.Add(e.nProductID);
				anDualID_LocationIDs.Add(e.nLocationID);
				break;
			default:
				ASSERT(FALSE); // This should never happen
				ThrowNxException("CommitInventoryTodoAlarmsTransaction was called for an invalid transaction!");
			}
		}

		// EnsureInventoryTodoAlarms requires a product id and a location id. Use the data in the arrays
		// to calculate these values.
		//
		// For ProductItemID's: We can look into ProductItemT to get the product ID and the location ID.
		//
		// For ProductID's: We are stuck with updating todo's for all feasible locations for the given
		// product ID's. To compound the issue, it's possible for product ID's to not point to any actual
		// records because they were deleted. So, we have to create a temporary table to fit them all in.
		//
		// Because I'm using UNION and not UNION ALL, any duplicate product-location combinations will be
		// grouped together.
		//
		CString strTmpTable = "SET NOCOUNT ON \r\nDECLARE @tmpProductT TABLE (ProductID INT NOT NULL) \r\n";
		if (anProductIDs.GetSize() > 0) {
			const int nCount = anProductIDs.GetSize();
			for (i=0; i < nCount; i++) {
				strTmpTable += FormatString("INSERT INTO @tmpProductT (ProductID) VALUES (%d)\r\n", anProductIDs[i]);
			}

			// (c.haag 2008-03-04 16:26) - Because ProductItemsT can have null locations, we need to
			// include ProductID's from those records, too
			if (anProductItemIDs.GetSize() > 0) {
				strTmpTable += FormatString("INSERT INTO @tmpProductT (ProductID) "
					"SELECT ProductID FROM ProductItemsT WHERE LocationID IS NULL AND ID IN (%s)\r\n",
					ArrayAsString(anProductItemIDs, false));
			}

		} // if (anProductIDs.GetSize() > 0) {
		strTmpTable += "SET NOCOUNT OFF \r\n";

		CString strExactIDs;
		const int nExactIDs = anDualID_ProductIDs.GetSize();
		for (i=0; i < nExactIDs; i++) {
			strExactIDs += FormatString("UNION SELECT %d AS ProductID, %d AS LocationID ",
				anDualID_ProductIDs[i], anDualID_LocationIDs[i]);
		}

		_RecordsetPtr prs = CreateRecordset(
			"%s"
			/* Product items */
			// (c.haag 2008-03-04 16:29) - Only include product items with non-null locations
			"SELECT ProductID, LocationID FROM ProductItemsT WHERE LocationID IS NOT NULL AND ID IN (%s) "
			"UNION "
			/* Products */
			"SELECT ProductID, LocationsT.ID AS LocationID FROM @tmpProductT, LocationsT "
				"WHERE LocationsT.Managed = 1 AND LocationsT.Active = 1 AND LocationsT.TypeID = 1 "
			/* Allocations */
			"UNION "
			"SELECT PatientInvAllocationDetailsT.ProductID, PatientInvAllocationsT.LocationID "
				"FROM PatientInvAllocationsT LEFT JOIN PatientInvAllocationDetailsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
				"WHERE ("
					"PatientInvAllocationsT.ID IN (%s) "
			/* Patients */
					"OR PatientInvAllocationsT.PatientID IN (%s) "
				") "
			"%s"
			,strTmpTable
			,ArrayAsString(anProductItemIDs, false),ArrayAsString(anAllocationIDs, false)
			,ArrayAsString(anPatientIDs, false)
			,strExactIDs);
		FieldsPtr f = prs->Fields;

		// (j.jones 2008-09-16 09:35) - PLID 31380 - EnsureInventoryTodoAlarms now supports being called
		// with multiple products at once, but with one location, so we need to track arrays of product IDs
		// per location ID, and call this function only once per needed location
		CArray<long, long> aryLocationIDs;
		CMap<long, long, CArray<long, long>*, CArray<long, long>*> mapProductArrayToLocationIDs;
		CArray<CArray<long, long>*, CArray<long, long>*> aryProductArrays;

		while (!prs->eof) {
			//when looping, just track in our array structure

			const long nProductID = AdoFldLong(f, "ProductID");
			const long nLocationID = AdoFldLong(f, "LocationID");
			
			BOOL bFoundLocID = FALSE;
			int i=0;
			for(i=0; i<aryLocationIDs.GetSize() && !bFoundLocID; i++) {
				if((long)(aryLocationIDs.GetAt(i)) == nLocationID) {
					bFoundLocID = TRUE;

					//track in the product array
					CArray<long, long> *pAry = NULL;
					mapProductArrayToLocationIDs.Lookup(nLocationID, pAry);
					if(pAry) {
						pAry->Add((long)nProductID);
					}
					else {
						//the array should exist
						ASSERT(FALSE);
					}
				}
			}
			if(!bFoundLocID) {
				//track this LocationID
				aryLocationIDs.Add(nLocationID);

				//add a new array
				CArray<long, long> *pAry = new CArray<long, long>;
				pAry->Add(nProductID);
				aryProductArrays.Add(pAry);

				mapProductArrayToLocationIDs.SetAt(nLocationID, pAry);
			}

			prs->MoveNext();
		}
		prs->Close();

		// (j.jones 2008-09-16 09:46) - PLID 31380 - now call EnsureInventoryTodoAlarms
		// for each location, for all products needed for that location
		for(i=0; i<aryLocationIDs.GetSize(); i++) {
			long nLocationID = (long)(aryLocationIDs.GetAt(i));

			CArray<long, long> *pAry = NULL;
			mapProductArrayToLocationIDs.Lookup(nLocationID, pAry);
			if(pAry) {
				//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
				InvUtils::EnsureInventoryTodoAlarms(*pAry, nLocationID, bInSqlTransaction);
			}
			else {
				//the array should exist
				ASSERT(FALSE);
			}
		}

		//now clear our arrays
		mapProductArrayToLocationIDs.RemoveAll();
		for(i=aryProductArrays.GetSize() - 1; i>=0; i--) {			
			CArray<long, long> *pAry = (CArray<long, long>*)aryProductArrays.GetAt(i);
			pAry->RemoveAll();
			delete pAry;
		}
		aryProductArrays.RemoveAll();
		aryLocationIDs.RemoveAll();

	} else {
		// No elements in the array; that means nothing changed, so don't change the todo alarms
	}

	// Now perform cleanup
	m_mapInvTodoTrans.RemoveKey(nTransID);
	delete pary;
}

void RollbackInventoryTodoAlarmsTransaction(int nTransID)
{
	// (c.haag 2008-02-27 16:10) - PLID 29115 - This function cancels an inventory todo ensurance
	// transaction.
	CITTIArray* pary = NULL;
	if (!m_mapInvTodoTrans.Lookup(nTransID, pary) || NULL == pary) {
		ASSERT(FALSE);
		ThrowNxException("Called RollbackInventoryTodoAlarmsTransaction for invalid transaction ID %d!", nTransID);
	}
	// Now perform cleanup
	m_mapInvTodoTrans.RemoveKey(nTransID);
	delete pary;
}


//TES 6/19/03: This function is never called, I don't really understand what it was supposed to do, and
//it references obsolete fields.
/*long GetAmountScheduled(long service, long location)
{
	//The following query will calculate the amount of a product needed in the future.
	//The query will search all appointments in the future that:
	//	- are surgeries, or minor/other procedures
	//	- have a CPT code associated with that procedure
	//	- that CPT code belongs to a surgery
	//	- the product in question belongs in that surgery
	//The sum is the end result of how many products are needed for upcoming procedures.

	try {

		_RecordsetPtr rs = CreateRecordset("SELECT Sum(SurgeryDetailsT2.Quantity) AS SumOfItems FROM AppointmentsT "
				"INNER JOIN ProcedureT ON AppointmentsT.AptPurposeID = ProcedureT.ID "
				"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"INNER JOIN ServiceT ON ProcedureT.ID = ServiceT.ProcedureID "
				"INNER JOIN SurgeryDetailsT ON ServiceT.ID = SurgeryDetailsT.ServiceID "
				"INNER JOIN SurgeryDetailsT AS SurgeryDetailsT2 ON SurgeryDetailsT.SurgeryID = SurgeryDetailsT2.SurgeryID "
				"WHERE AppointmentsT.Date > GetDate() AND AptTypeT.Category = 3 OR AptTypeT.Category = 4 OR AptTypeT.Category = 6 "
				"AND SurgeryDetailsT2.ServiceID = %li",service);

		if(rs->eof)
			return 0;

		long AmtNeeded = AdoFldLong(rs, "SumOfItems",0);

		rs->Close();

		return AmtNeeded;

	}NxCatchAll("Could not calculate amount scheduled.");

	return 0;
}*/

void TryUpdateLastCostByProduct(long ProductID, COleCurrency cyCost, COleDateTime dtReceivedDate) {

	try {

		//only update last cost if the date received is the most recent date

		if(!IsRecordsetEmpty("SELECT OrderT.ID FROM OrderT "
			"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.ID "
			"WHERE OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 AND DateReceived > GetDate() AND OrderDetailsT.ProductID = %li",ProductID))
			return;

		//DRT 6/14/2007 - PLID 26147 - Follow the preference -- either they always change it (0), never change it (1), or we prompt the user (2).
		//	Previously we always changed EXCEPT in the case of the new cost being $0.00.  We'll work that code in by only offering it in the 
		//	"always" case.
		CString strItemName;
		COleCurrency cyOldCost = COleCurrency(0, 0);
		long nChangeOrderCost = GetRemotePropertyInt("ChangeLastCostFromOrder", 0, 0, "<None>", true);
		if(nChangeOrderCost == 1) {
			//Never change the cost.  Just quit.
			return;
		}
		else {
			//For the old code and our new prompt, we may need this info.
			_RecordsetPtr rs = CreateRecordset("SELECT ServiceT.Name, LastCost FROM ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID WHERE ProductT.ID = %li", ProductID);
			if(!rs->eof) {
				cyOldCost = AdoFldCurrency(rs, "LastCost");
				strItemName = AdoFldString(rs, "Name");
			}
			else {
				//Old behavior -- quit if the item was not found
				return;
			}
			rs->Close();

			//Related fix -- Previously if you marked an order received which had no change in cost, it jumped down to the ASC
			//	check (wasting a bunch of time on irrelevant recordsets), and then audited that the price change, making old
			//	and new the same thing!  We don't want to do that.
			if(cyOldCost == cyCost) {
				return;
			}

			//This message is the same for the prompt option as the always use-but $0.00 option
			CString strMsg;
			strMsg.Format("The last cost for '%s' is %s and the new cost on this order is %s.\n"
				"Do you want to update the last cost to be %s?", strItemName, FormatCurrencyForInterface(cyOldCost), 
				FormatCurrencyForInterface(cyCost), FormatCurrencyForInterface(cyCost));
			if(nChangeOrderCost == 2) {
				//The user wishes to be prompted in all cases
				if(MessageBox(GetActiveWindow(), strMsg, "Practice", MB_ICONQUESTION|MB_YESNO) != IDYES) {
					return;
				}
			}
			else if(nChangeOrderCost == 0) {
				//The user wishes to always update the cost.  That being said, our old behavior prompted if the new cost was $0.00, so
				//	will continue to do that.
				if(cyOldCost != COleCurrency(0,0) && cyCost == COleCurrency(0,0)) {
					//the new cost is $0.00 and the old cost is not, perhaps a complimentary order? (A client does this, and asked for this code.)
					if(IDNO == MessageBox(GetActiveWindow(), strMsg, "Practice", MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
			}
			else {
				//Should not be possible, our property is not a valid value.  Do not update cost.
				ASSERT(FALSE);
				return;
			}
		}

		//we are changing the cost, so if ASC, check against existing preference cards
		// (j.jones 2009-08-26 08:53) - PLID 35124 - this is now for PreferenceCardsT only
		if(IsSurgeryCenter(false) && !IsRecordsetEmpty("SELECT ID FROM ProductT WHERE ID = %li AND LastCost <> Convert(money,'%s')",ProductID,_Q(FormatCurrencyForSql(cyCost)))) { 

			//still need to get the surgery settings, for defaults
			long nUpdateSurgeryPrices = GetRemotePropertyInt("UpdateSurgeryPrices",1,0,"<None>",TRUE);
			long nUpdateSurgeryPriceWhen = GetRemotePropertyInt("UpdateSurgeryPriceWhen",1,0,"<None>",TRUE);
			long nUpdatePreferenceCardPrices = GetRemotePropertyInt("UpdatePreferenceCardPrices",nUpdateSurgeryPrices,0,"<None>",TRUE);
			long nUpdatePreferenceCardPriceWhen = GetRemotePropertyInt("UpdatePreferenceCardPriceWhen",nUpdateSurgeryPriceWhen,0,"<None>",TRUE);

			COleCurrency cyCostToCheck = cyCost;
			//the product may be using UU/UO, meaning the cyCost passed in would be reduced by the conversion rate (for surgeries)
			_RecordsetPtr rs2 = CreateParamRecordset("SELECT Conversion FROM ProductT WHERE UseUU = 1 AND ID = {INT}",ProductID);
			if(!rs2->eof) {
				long nConversion = AdoFldLong(rs2, "Conversion",1);
				if(nConversion == 0)
					cyCostToCheck = COleCurrency(0,0);
				else {
					//reduce by the conversion rate
					cyCostToCheck = cyCost / nConversion;
					RoundCurrency(cyCostToCheck);
				}
			}
			rs2->Close();

			//3 means do nothing, so skip the check
			if(nUpdatePreferenceCardPrices != 3 &&
				!IsRecordsetEmpty("SELECT ID FROM PreferenceCardDetailsT WHERE ServiceID = %li AND Cost <> Convert(money,'%s')",ProductID,_Q(FormatCurrencyForSql(cyCostToCheck)))) {

				//2 means to always update, so don't prompt. 1 means to prompt.
				if(nUpdatePreferenceCardPrices == 2 || (nUpdatePreferenceCardPrices == 1 && IDYES==MessageBox(GetActiveWindow(),"There are preference cards that use this product but list a different last cost for it.\n"
					"Do you wish to update these preference cards to match this new cost?","Practice",MB_ICONQUESTION|MB_YESNO))) {					

					if(nUpdatePreferenceCardPriceWhen == 1)
						ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Cost = Convert(money, {STRING}) WHERE ServiceID = {INT}", FormatCurrencyForSql(cyCostToCheck), ProductID);
					else {
						ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Cost = Convert(money, {STRING}) WHERE ServiceID = {INT} AND ServiceID IN (SELECT ID FROM ProductT WHERE LastCostPerUU = Cost)", FormatCurrencyForSql(cyCostToCheck), ProductID);
					}
				}
			}
		}

		ExecuteParamSql("UPDATE ProductT SET LastCost = Convert(money,{STRING}) WHERE ID = {INT}", FormatCurrencyForSql(cyCost), ProductID);

		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, strItemName, nAuditID, aeiProductLastCost, ProductID, FormatCurrencyForInterface(cyOldCost), FormatCurrencyForInterface(cyCost), aepHigh, aetChanged);
		//end auditing


	}NxCatchAll("Could not update the last cost.");
}

void TryUpdateLastCostByOrder(long OrderID, COleDateTime dtReceivedDate) {

	try {

		_RecordsetPtr rs = CreateRecordset("SELECT ProductID, Amount FROM OrderT "
			"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
			"WHERE OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 AND OrderDetailsT.OrderID = %li",OrderID);

		while(!rs->eof) {

			long ProductID = AdoFldLong(rs, "ProductID",-1);
			COleCurrency cyAmount = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));

			TryUpdateLastCostByProduct(ProductID,cyAmount,dtReceivedDate);

			rs->MoveNext();
		}
		rs->Close();

	}NxCatchAll("Could not update the last cost of items in the order.");
}

void TransferItems(long nProductID, double dblQuantity, long nSourceLocationID, long nDestLocationID, CString strNotes) {

	try {
		ExecuteSql("INSERT INTO ProductLocationTransfersT "
			"(ID, ProductID, SourceLocationID, DestLocationID, Amount, Date, UserID, Notes) "
			"VALUES (%li, %li, %li, %li, %g, GetDate(), %li, '%s')",NewNumber("ProductLocationTransfersT", "ID"),
			nProductID, nSourceLocationID, nDestLocationID, dblQuantity, GetCurrentUserID(),_Q(strNotes));

		// (c.haag 2008-02-07 13:17) - PLID 28853 - Renamed from ChargeInventoryQuantity to EnsureInventoryTodoAlarms
		// because that's a closer description to what it actually does. Also removed unused quantity parameter.
		// (j.jones 2008-09-16 09:27) - PLID 31380 - EnsureInventoryTodoAlarms now supports multiple products,
		// but in this case it's just one product, and two locations, so it still needs called twice
		CArray<long, long> aryProductIDs;
		aryProductIDs.Add(nProductID);
		//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
		InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, nSourceLocationID, false);
		InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, nDestLocationID, false);

		//auditing
		CString strProduct, strSourceLocName, strDestLocName;
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li",nProductID);
		if(!rs->eof) {
			strProduct = AdoFldString(rs, "Name","");
		}
		rs->Close();
		rs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li",nSourceLocationID);
		if(!rs->eof) {
			strSourceLocName = AdoFldString(rs, "Name","");
		}
		rs->Close();
		rs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li",nDestLocationID);
		if(!rs->eof) {
			strDestLocName = AdoFldString(rs, "Name","");
		}
		rs->Close();
		
		CString strNew;
		strNew.Format("%g to %s", dblQuantity, strDestLocName);
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, strProduct, nAuditID, aeiProductLocationTransfer, nProductID, strSourceLocName, strNew, aepMedium, aetChanged);
	}NxCatchAll("Could not transfer inventory items.");
}

long GetAvailableProductItemCount(long nProductID, long nLocationID)
{
	// (j.jones 2007-11-26 10:12) - PLID 28037 - ensured we hide allocated items
	// (j.jones 2014-11-10 14:18) - PLID 63501 - converted to use joins instead of a NOT IN clause
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT COUNT(*) AS ProductItemCount "
		"FROM ProductItemsT "
		"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
		"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
		"LEFT JOIN ("
		"	SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"		AND PatientInvAllocationsT.Status <> {CONST} "
		"		AND PatientInvAllocationDetailsT.Status IN ({CONST}, {CONST}) "
		") AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"WHERE ProductItemsT.ProductID = {INT} AND ProductItemsT.LocationID = {INT} AND ChargesT.ID IS NULL "
		"AND CaseHistoryDetailsT.ID IS NULL AND ProductItemsT.Deleted = 0 "
		"AND AllocationsQ.ProductItemID Is Null",
		InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed, nProductID, nLocationID);

	return AdoFldLong(prs, "ProductItemCount", -1);
}

CString GetItemDropdownSqlFromClause(long nLocationID, long nSupplierID)
{
	// (c.haag 2007-11-07 09:37) - PLID 27994 - This function returns the generic "from"
	// clause of an inventory item dropdown used in InvEditOrderDlg.cpp and InvEditReturnDlg.cpp.
	// This code was completely ripped from InvEditOrderDlg.cpp
	//DRT 3/8/2007 - PLID 24823 - Added the barcode as a field in the item datalist combo, must be referenced
	//	here in the FROM clause.
	// (c.haag 2007-11-07 10:41) - PLID 27994 - Added HasSerialNum
	//DRT 11/8/2007 - PLID 27984 - Added ProductT.DefaultConsignment.  If the item is not serialized/expirable, always 0.
	//DRT 11/8/2007 - PLID 27984 - Also added ProductT.HasExpDate.
	// (j.jones 2007-11-8 11:15) - PLID 28196 - completed Allocations now decrement from Inventory
	// (c.haag 2007-12-07 17:09) - PLID 28286 - Added SerialPerUO
	// (j.jones 2007-12-17 11:08) - PLID 27988 - billed allocations do not decrement from inventory,
	// neither do billed case histories, which this query seemingly never supported to begin with
	// (j.jones 2008-02-07 11:05) - PLID 28851 - now this function also calculates consignment values,
	// and returns the consignment reorder point and min. on hand
	//DRT 2/7/2008 - PLID 28854 - Added Avail and AvailConsign calculations.  This is on hand minus allocated.
	// (j.jones 2008-02-28 12:12) - PLID 28080 - actual and avail calculations now take case histories and
	// allocations into account differently based on the license, the status of which must now be passed into
	// this query.
	// (j.jones 2008-03-07 14:55) - PLID 29204 - changed the 'avail' calculations to not be case or allocation-only
	// based on the license, instead it calculates with the same logic as the 'actual', except that case history totals
	// that aren't accounted for by an allocation will always decrement from Purchased stock, never Consignment.
	// This also means we no longer care what the license is.
	//TES 7/3/2008 - PLID 24726 - Added SerialNumIsLotNum, a flag indicating whether the Serial Number should be treated as a 
	// Lot Number (basically meaning that it doesn't have to be unique).
	// (j.jones 2009-01-05 16:36) - PLID 32621 - fixed the Actual/Avail calculations to take the UU/UO conversion rate into
	// account at all times, even when the product is serialized, because it didn't do so before
	// (j.jones 2016-01-19 08:46) - PLID 67680 - ensured the total inventory amount was rounded

	CString strFrom;
	strFrom.Format(
		"(SELECT "

		"Round(Convert(float,"

		//use the standard calculation only if no serial options are enabled,
		//otherwise look explicitly at product items - return the grand total
		//of product items so the dialog can simply subtract consignment values
		"(CASE WHEN TrackableStatus = 1 THEN Null ELSE "
			"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"THEN Coalesce(AllProductItemsActualQ.Actual, 0) "
			"ELSE "
				"((CASE WHEN Sold Is Null THEN 0 ELSE 0 - Sold END) + "
				"(CASE WHEN Received Is Null THEN 0 ELSE Received END) + "
				"(CASE WHEN TransferredTo Is Null THEN 0 ELSE TransferredTo END) + "
				"(CASE WHEN TransferredFrom Is Null THEN 0 ELSE TransferredFrom END) + "
				"(CASE WHEN CaseHistoryQty IS NULL THEN 0 ELSE 0 - CaseHistoryQty END) + "
				"(CASE WHEN Adjusted Is Null THEN 0 ELSE Adjusted END) + "
				"(CASE WHEN UsedAllocationQty Is Null THEN 0 ELSE 0 - UsedAllocationQty END)) "
			"END "
			"/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "			
		" END) "
		"), 2) AS Actual, "

		"Round(Convert(float, "

		//use the standard calculation only if no serial options are enabled,
		//otherwise look explicitly at product items - return the grand total
		//of product items so the dialog can simply subtract consignment values

		/* the UnRelievedCaseHistoryQty needs subtracted from AllProductItemsAvailQ.Avail,
		if that field is used, otherwise the regular calculations such as
		UnRelievedCaseHistoryQty and OnHoldAllocationQty send back appropriate values*/
		"(CASE WHEN TrackableStatus = 1 THEN Null ELSE "
			"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"	THEN Coalesce(AllProductItemsAvailQ.Avail, 0) "
			"		- Coalesce(UnRelievedCaseHistoryQty,0) "
			"ELSE "
				"((CASE WHEN Sold Is Null THEN 0 ELSE 0 - Sold END) + "
				"(CASE WHEN Received Is Null THEN 0 ELSE Received END) + "
				"(CASE WHEN TransferredTo Is Null THEN 0 ELSE TransferredTo END) + "
				"(CASE WHEN TransferredFrom Is Null THEN 0 ELSE TransferredFrom END) + "
				"(CASE WHEN CaseHistoryQty IS NULL THEN 0 ELSE 0 - CaseHistoryQty END) + "
				"(CASE WHEN Adjusted Is Null THEN 0 ELSE Adjusted END) + "
				"(CASE WHEN UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE 0 - UnRelievedCaseHistoryQty END) + "
				"(CASE WHEN UsedAllocationQty Is Null THEN 0 ELSE 0 - UsedAllocationQty END) + "
				"(CASE WHEN OnHoldAllocationsQ.OnHoldAllocationQty IS NULL THEN 0 ELSE 0 - OnHoldAllocationsQ.OnHoldAllocationQty END)) "				
			"END "
			"/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		" END) "
		"), 2) AS Avail, "

		"Convert(int,"
		"(CASE WHEN TrackableStatus = 1 THEN Null ELSE "
		"	CASE WHEN Ordered Is Null THEN 0 ELSE Ordered END "
		"	/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		"END) "		
		") AS Ordered, "

		//use the product item count only if serial options are enabled,
		//otherwise always use 0
		"Convert(float,"

		"(CASE WHEN TrackableStatus = 1 THEN Null ELSE "	
			"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"THEN Coalesce(ConsignmentProductsActualQ.ConsignmentActual, 0) "
			"ELSE 0 END "
			"/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		"END) "
		") AS ActualConsignment, "

		//use the product item count only if serial options are enabled,
		//otherwise always use 0
		"Convert(float, "
		
		"(CASE WHEN TrackableStatus = 1 THEN Null ELSE "
			"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"THEN Coalesce(ConsignmentProductsAvailQ.ConsignmentAvail, 0) "
			"ELSE 0 END "
			"/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		"END) "
		") AS AvailConsignment, "

		//use the ordered count only if serial options are enabled,
		//otherwise use NULL
		"Convert(int,"
		"(CASE WHEN TrackableStatus = 1 OR (HasSerialNum = 0 AND HasExpDate = 0) THEN Null ELSE "
		"	Coalesce(ConsignmentOnOrderQ.ConsignmentOrdered, 0) "
		"	/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		"END) "
		") AS OrderedConsignment, "

		"ProductQ.Name AS Name, CategoriesT.Name AS Category, UnitDesc, Billable, Price, Taxable1, Taxable2, Active, TrackableStatus, ProductQ.ID, "
		
		"Convert(float,"
		"(CASE WHEN TrackableStatus = 1 THEN NULL ELSE "
		"	ReOrderPoint "
		"	/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		"END) "
		") AS ReOrderPoint, "

		"Convert(int,"
		"(CASE WHEN TrackableStatus = 1 THEN NULL ELSE "
		"	ReorderQuantity "
		"	/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		"END) "
		") AS ReorderQuantity, "

		"Convert(float,"
		"(CASE WHEN TrackableStatus = 1 THEN NULL ELSE "
		"	ConsignmentReOrderPoint "
		"	/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		"END) "
		") AS ConsignmentReOrderPoint, "

		// (c.haag 2008-02-21 09:38) - PLID 28852 - We are no longer using ConsignmentReorderQuantity
		//"Convert(int,"
		//"(CASE WHEN TrackableStatus = 1 THEN NULL ELSE "
		//"	ConsignmentReorderQuantity "
		//"	/ CASE WHEN UseUU = 1 THEN Conversion ELSE 1.0 END "
		//"END) "
		//") AS ConsignmentReorderQuantity, "
				
		"SupplierID, LastCost, Catalog, Convert(int,UseUU) AS UseUU, UnitOfUsage, UnitOfOrder, Conversion, Barcode, HasSerialNum, SerialNumIsLotNum, DefaultConsignment, HasExpDate, SerialPerUO "

		"FROM ( "
		"SELECT ProductT.ID, Name, UnitDesc, Billable, Price, Taxable1, Taxable2, Active, TrackableStatus, ServiceT.Barcode, ProductT.HasSerialNum, ProductT.SerialNumIsLotNum, "
		"ReOrderPoint, ReorderQuantity, ConsignmentReOrderPoint, "/*ConsignmentReorderQuantity,*/" MultiSupplierT.SupplierID, Category, LastCost, MultiSupplierT.Catalog, UseUU, UnitOfUsage, UnitOfOrder, Conversion, "
		"convert(bit, CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 THEN ProductT.DefaultConsignment ELSE 0 END) AS DefaultConsignment, ProductT.HasExpDate, SerialPerUO "
		"FROM ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
		"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
		"INNER JOIN MultiSupplierT ON ProductT.ID = MultiSupplierT.ProductID "
		"WHERE TrackableStatus > 0 AND MultiSupplierT.SupplierID = [sup] AND ProductLocationInfoT.LocationID = [loc] "
		") AS ProductQ "

		// (j.dinatale 2011-11-04 13:24) - PLID 46227 - need to take into account voiding and original lineitems for financial corrections
		// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
		// (a.walling 2014-01-14 11:30) - PLID 60330 - The item dropdown when adding a new return for an inventory item does not adjust the 'actual' value for charges used on glasses/contacts orders that are not 'off the shelf' as defined by PLID 48597
		// (j.jones 2014-11-10 10:38) - PLID 63501 - reworked the IN clauses for GlassesOrderServiceT and BilledCaseHistoriesT such that they are now left joining
		// (r.goldschmidt 2015-06-08 15:56) - PLID 66234 - need to add some parentheses to correct a where clause
		"LEFT JOIN ( "
			"SELECT SUM (CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS Sold, ServiceID "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
			"	INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
			"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"	WHERE PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " AND PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " "
			"	GROUP BY ChargeID) AS AllocationInfoQ ON ChargesT.ID = AllocationInfoQ.ChargeID "
			"LEFT JOIN ("
			"	SELECT GlassesOrderServiceT.ID "
			"	FROM GlassesOrderServiceT "
			"	INNER JOIN ProductT ON GlassesOrderServiceT.ServiceID = ProductT.ID "
			"	WHERE GlassesOrderServiceT.IsDefaultProduct = 1 AND GlassesOrderServiceT.IsOffTheShelf = 0  "
			"	AND (ProductT.GlassesContactLensDataID IS NOT NULL OR ProductT.FramesDataID IS NOT NULL) "
			") AS GlassesOrderServiceQ ON ChargesT.GlassesOrderServiceID = GlassesOrderServiceQ.ID "
			"LEFT JOIN (SELECT BillID FROM BilledCaseHistoriesT GROUP BY BillID) AS BilledCaseHistoriesQ ON BillsT.ID = BilledCaseHistoriesQ.BillID "
			"LEFT JOIN CaseHistoryDetailsT ON CaseHistoryDetailsT.ItemID = ChargesT.ServiceID AND CaseHistoryDetailsT.ItemType = -1 "
			"LEFT JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID AND BilledCaseHistoriesT.BillID = BillsT.ID "
			"WHERE LineItemT.Deleted = 0 AND Type = 10 AND LineItemT.LocationID = [loc] "
			"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
			"AND BilledCaseHistoriesQ.BillID Is Null "
			"AND GlassesOrderServiceQ.ID Is Null "
			"GROUP BY ServiceID "
		") AS ChargeQ ON ProductQ.ID = ChargeQ.ServiceID "

		"LEFT JOIN ( "
			"SELECT ProductID, SUM (QuantityOrdered) AS Received "
			"FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
			"WHERE LocationID = [loc] AND DateReceived IS NOT NULL "
			"AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
			"GROUP BY ProductID "
		") AS OrderQ ON ProductQ.ID = OrderQ.ProductID "
		"LEFT JOIN ( "
			"SELECT ProductID, SUM (QuantityOrdered) AS Ordered "
			"FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
			"WHERE LocationID = [loc] AND DateReceived IS NULL "
			"AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
			"GROUP BY ProductID "
		") AS OnOrderQ ON ProductQ.ID = OnOrderQ.ProductID "
		"LEFT JOIN ( "
			"SELECT SUM(Quantity) AS Adjusted, ProductID "
			"FROM ProductAdjustmentsT "
			"WHERE LocationID = [loc] "
			"GROUP BY ProductID "
		") AS AdjQ ON ProductQ.ID = AdjQ.ProductID "
		"LEFT JOIN ( "
			"SELECT SUM(Amount) AS TransferredTo, ProductID "
			"FROM ProductLocationTransfersT "
			"WHERE DestLocationID = [loc] "
			"GROUP BY ProductID "
		") AS TransferredToSubQ ON ProductQ.ID = TransferredToSubQ.ProductID "
		"LEFT JOIN ( "
			"SELECT -(SUM(Amount)) AS TransferredFrom, ProductID "
			"FROM ProductLocationTransfersT "
			"WHERE SourceLocationID = [loc] "
			"GROUP BY ProductID "
		") AS TransferredFromSubQ ON ProductQ.ID = TransferredFromSubQ.ProductID "
		
		/* if there is a linked allocation with the same product, do not use the case history
		amount unless it is greater, in which case we use the overage*/
		"LEFT JOIN ( "
			"SELECT Sum(CaseHistoryQty) AS UnRelievedCaseHistoryQty, ItemID FROM ("
				"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(ActiveAllocationQty,0)) THEN SUM(Quantity) - SUM(Coalesce(ActiveAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID "
				"FROM CaseHistoryDetailsT INNER JOIN "
				"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
				"LEFT JOIN (SELECT Sum(Quantity) AS ActiveAllocationQty, ProductID, CaseHistoryID "
					"FROM PatientInvAllocationsT "
					"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
					"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
					"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " "
					"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " "
					"AND LocationID = [loc] "
					"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
				"WHERE ItemType = -1 AND CompletedDate Is Null AND LocationID = [loc] "
				"GROUP BY ItemID, CaseHistoryT.ID, ActiveAllocationQty "
			") AS CaseHistoryIndivSubQ GROUP BY ItemID "
		") AS UnRelievedCaseHistSubQ ON ProductQ.ID = UnRelievedCaseHistSubQ.ItemID "

		/* if there is a linked allocation with the same product, do not use the case history
		amount unless it is greater, in which case we use the overage*/
		"LEFT JOIN ( "
			"SELECT Sum(CaseHistoryQty) AS CaseHistoryQty, ItemID FROM ("
				"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(UsedAllocationQty,0)) THEN SUM(Quantity) - SUM(Coalesce(UsedAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID "
				"FROM CaseHistoryDetailsT INNER JOIN "
				"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
				"LEFT JOIN (SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, CaseHistoryID "
					"FROM PatientInvAllocationsT "
					"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
					"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
					"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " "
					"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " "
					"AND LocationID = [loc] "
					"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
				"WHERE ItemType = -1 AND CompletedDate Is Not Null AND LocationID = [loc] "
				"GROUP BY ItemID, CaseHistoryT.ID, UsedAllocationQty "
			") AS CaseHistoryIndivSubQ GROUP BY ItemID "
		") AS CaseHistSubQ ON ProductQ.ID = CaseHistSubQ.ItemID "

		"LEFT JOIN ( "
			"SELECT Sum(Quantity) AS UsedAllocationQty, ProductID "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " "
			"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " "
			"AND LocationID = [loc] "
			"GROUP BY ProductID "
		") AS CompletedAllocationsQ ON ProductQ.ID = CompletedAllocationsQ.ProductID "
		"LEFT JOIN (  "
		"	SELECT Sum(Quantity) AS OnHoldAllocationQty, ProductID  "
		"	FROM PatientInvAllocationsT  "
		"	INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID  "
		"	WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " "
		"	AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " "
		"	AND LocationID = [loc] "
		"	GROUP BY ProductID  "
		") AS OnHoldAllocationsQ ON ProductQ.ID = OnHoldAllocationsQ.ProductID "

		// (j.jones 2014-11-10 14:18) - PLID 63501  - converted to use joins instead of a NOT IN clause
		"LEFT JOIN ( "
		"	SELECT Count(ProductItemsT.ID) AS Actual, ProductItemsT.ProductID "
		"	FROM ProductItemsT "
		"	LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"	LEFT JOIN ("
		"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"			AND PatientInvAllocationsT.Status <> %li "
		"			AND PatientInvAllocationDetailsT.Status = %li "
		"	) AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"	WHERE ChargedProductItemsT.ProductItemID Is Null "
		"	AND AllocationsQ.ProductItemID Is Null "
		"	AND ProductItemsT.Deleted = 0 "
		"	AND ProductItemsT.LocationID = [loc] "
		"	GROUP BY ProductItemsT.ProductID "
		") AS AllProductItemsActualQ ON ProductQ.ID = AllProductItemsActualQ.ProductID "
		" "
		"LEFT JOIN ( "
		"	SELECT Count(ProductItemsT.ID) AS Avail, ProductItemsT.ProductID "
		"	FROM ProductItemsT "
		"	LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"	LEFT JOIN ("
		"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"			AND PatientInvAllocationsT.Status <> %li "
		"			AND PatientInvAllocationDetailsT.Status IN (%li, %li) "
		"	) AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"	WHERE ChargedProductItemsT.ProductItemID Is Null "
		"	AND AllocationsQ.ProductItemID Is Null "
		"	AND ProductItemsT.Deleted = 0 "
		"	AND ProductItemsT.LocationID = [loc] "
		"	GROUP BY ProductItemsT.ProductID "
		") AS AllProductItemsAvailQ ON ProductQ.ID = AllProductItemsAvailQ.ProductID "
		" "
		"LEFT JOIN ( "
		"	SELECT Count(ID) AS ConsignmentActual, ProductItemsT.ProductID "
		"	FROM ProductItemsT "
		"	LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"	LEFT JOIN ("
		"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"			AND PatientInvAllocationsT.Status <> %li "
		"			AND PatientInvAllocationDetailsT.Status = %li "
		"	) AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"	WHERE ChargedProductItemsT.ProductItemID Is Null "
		"	AND AllocationsQ.ProductItemID Is Null "
		"	AND ProductItemsT.Status = %li AND ProductItemsT.Deleted = 0 "
		"	AND ProductItemsT.LocationID = [loc] "
		"	GROUP BY ProductItemsT.ProductID "
		") AS ConsignmentProductsActualQ ON ProductQ.ID = ConsignmentProductsActualQ.ProductID "
		" "
		"LEFT JOIN ( "
		"	SELECT Count(ProductItemsT.ID) AS ConsignmentAvail, ProductItemsT.ProductID "
		"	FROM ProductItemsT "
		"	LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"	LEFT JOIN ("
		"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"			AND PatientInvAllocationsT.Status <> %li "
		"			AND PatientInvAllocationDetailsT.Status IN (%li, %li) "
		"	) AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
		"	WHERE ChargedProductItemsT.ProductItemID Is Null "
		"	AND AllocationsQ.ProductItemID Is Null "
		"	AND ProductItemsT.Status = %li "
		"	AND ProductItemsT.Deleted = 0 "
		"	AND ProductItemsT.LocationID = [loc] "
		"	GROUP BY ProductItemsT.ProductID "
		") AS ConsignmentProductsAvailQ ON ProductQ.ID = ConsignmentProductsAvailQ.ProductID "
		" "
		"LEFT JOIN ( "
			"SELECT ProductID, SUM (QuantityOrdered) AS ConsignmentOrdered "
			"FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
			"WHERE LocationID = [loc] AND DateReceived IS NULL "
			"AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 AND ForStatus = %li "
			"GROUP BY ProductID "
		") AS ConsignmentOnOrderQ ON ProductQ.ID = ConsignmentOnOrderQ.ProductID "
		"LEFT JOIN CategoriesT ON ProductQ.Category = CategoriesT.ID) AS ItemQ",
		InvUtils::iasDeleted, InvUtils::iadsUsed,
		InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed,
		InvUtils::iasDeleted, InvUtils::iadsUsed, InvUtils::pisConsignment,
		InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed, InvUtils::pisConsignment,
		InvUtils::odsConsignment);

	strFrom.Replace ("[loc]", AsString(nLocationID));
	strFrom.Replace ("[sup]", AsString(nSupplierID));

	return strFrom;
}

BOOL DeleteReturnGroup(long nID, BOOL bShowWarnings, CWnd* pWndWarningParent)
{
	// (c.haag 2007-11-09 16:23) - PLID 27994 - This utility function deletes a return group
	long nAuditTransactionID = -1;
	try {
		if (bShowWarnings) {
			if (NULL == pWndWarningParent) {
				// If we were not given a parent window for warnings, use the application window				
				pWndWarningParent = AfxGetMainWnd();
			} else {
				// If we were given a parent window for the warning message boxes, use it
			}

			if (IDNO == pWndWarningParent->MessageBox("Are you absolutely sure you wish to delete this supplier return, "
				"and all information related to it?", "Inventory Returns", MB_ICONWARNING | MB_YESNO))
			{			
				return FALSE; // The user changed their mind
			}
			else if (ReturnsRecords(FormatString("SELECT ID FROM ProductAdjustmentsT WHERE ID IN (SELECT ProductAdjustmentID FROM SupplierReturnItemsT WHERE ReturnGroupID = %d)", nID))) {
				if (IDNO == pWndWarningParent->MessageBox("By deleting this return, all changes to on-hand quantities made when the return was created will be undone. Are you sure you wish to continue?\n\n"
					"Click Yes if you would like to delete this return, and undo all on-hand quantity changes related to this return.\n\n"
					"Click No if you do not wish to delete this return.",
					"Inventory Returns", MB_ICONWARNING | MB_YESNO))
				{			
					return FALSE; // The user changed their mind
				}

			}
		}

		// The user didn't change their mind. Lets do it.

		// (c.haag 2007-11-12 17:04) - PLID 28028 - Generate auditing data by querying data
		nAuditTransactionID = BeginAuditTransaction();
		CString strGroupDesc;
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT Description FROM SupplierReturnGroupsT WHERE ID = {INT};\r\n"
			"SELECT SupplierReturnItemsT.ID, SupplierReturnItemsT.ProductItemID, "
			"	CASE WHEN NonSerialProductT.Name IS NULL THEN SerialProductT.Name ELSE NonSerialProductT.Name END AS Product, "
			"	ProductItemsT.SerialNum, SupplierReturnItemsT.Quantity "
			"FROM SupplierReturnItemsT "
			"LEFT JOIN ServiceT NonSerialProductT ON NonSerialProductT.ID = SupplierReturnItemsT.ProductID "
			"LEFT JOIN ProductItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
			"LEFT JOIN ServiceT SerialProductT ON SerialProductT.ID = ProductItemsT.ProductID "
			"WHERE SupplierReturnItemsT.ReturnGroupID = {INT};\r\n",
			nID, nID);

		if (prs->eof) {
			// The group no longer exists in data. Someone else may have deleted it or the ID is invalid (such as -1)
			ThrowNxException("Attempted to delete a supplier return group that does not exist (ID = %d)!", nID);
		} else {
			strGroupDesc = AdoFldString(prs, "Description");
		}
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupDeleted, nID, strGroupDesc, "", aepHigh, aetDeleted);


		// Now move on to the individual return line items
		prs = prs->NextRecordset(NULL);
		while (!prs->eof) {
			CString strItemAuditName;
			if (-1 == AdoFldLong(prs, "ProductItemID", -1)) {
				// Audit non-serialized item information
				strItemAuditName.Format("Group: '%s' Item: '%s' Qty: %g",
					strGroupDesc, AdoFldString(prs, "Product"), AdoFldDouble(prs, "Quantity"));
			} else {
				// Audit serialized item information
				strItemAuditName.Format("Group: '%s' Item: '%s' Serial: '%s' Qty: %g",
					strGroupDesc, AdoFldString(prs, "Product"), AdoFldString(prs, "SerialNum", ""), AdoFldDouble(prs, "Quantity"));
			}
			AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemDeleted, AdoFldLong(prs, "ID"), strItemAuditName, "", aepHigh, aetDeleted);
			prs->MoveNext();
		}


		// (c.haag 2007-11-12 17:04) - Now do the changes we mean to
		CString strSaveString = BeginSqlBatch();
		AddStatementToSqlBatch(strSaveString, "DECLARE @tProductAdjustmentIDs TABLE (ID INT NOT NULL)");
		AddStatementToSqlBatch(strSaveString, "INSERT INTO @tProductAdjustmentIDs (ID) SELECT ProductAdjustmentID FROM SupplierReturnItemsT WHERE ReturnGroupID = %d", nID);
		// (j.jones 2008-06-02 15:57) - PLID 28076 - clear the adjustment ID from these product items
		AddStatementToSqlBatch(strSaveString, "UPDATE ProductItemsT SET Deleted = 0, AdjustmentID = NULL WHERE ID IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ReturnGroupID = %d)", nID);
		AddStatementToSqlBatch(strSaveString, "DELETE FROM SupplierReturnItemsT WHERE ReturnGroupID = %d", nID);
		AddStatementToSqlBatch(strSaveString, "DELETE FROM ProductAdjustmentsT WHERE ID IN (SELECT ID FROM @tProductAdjustmentIDs)");
		AddStatementToSqlBatch(strSaveString, "DELETE FROM SupplierReturnGroupsT WHERE ID = %d", nID);
#ifdef _DEBUG
		//(e.lally 2008-04-10)- Switched to our CMsgBox dialog
		CMsgBox dlg(NULL);
		dlg.msg = strSaveString;
		dlg.DoModal();
#endif
		ExecuteSqlBatch(strSaveString);
		CommitAuditTransaction(nAuditTransactionID);
	} NxCatchAllSilentCallThrow(
		if (-1 != nAuditTransactionID) { RollbackAuditTransaction(nAuditTransactionID); }
		);

	// (c.haag 2007-11-14 09:56) - PLID 27992 - Now throw a table checker
	CClient::RefreshTable(NetUtils::SupplierReturnGroupsT, nID);

	return TRUE; // Success
}

// (j.jones 2007-11-14 13:14) - PLID 27988 - FreeAllocationMasterInfoObject can be
// used by any class that tracks an AllocationMasterInfo pointer such that any
// class can delete it using the same function
void FreeAllocationMasterInfoObject(AllocationMasterInfo *pAllocationMasterInfo)
{
	if(pAllocationMasterInfo) {

		for(int i=pAllocationMasterInfo->paryAllocationDetailInfo.GetSize()-1;i>=0;i--) {
			AllocationDetailInfo* pInfo = (AllocationDetailInfo*)(pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i));
			delete pInfo;
		}
		pAllocationMasterInfo->paryAllocationDetailInfo.RemoveAll();

		delete pAllocationMasterInfo;
		pAllocationMasterInfo = NULL;
	}
}

// (j.jones 2007-11-30 15:10) - PLID 28251 - For purposes of barcoding, take in a serial number and LocationID,
// see if it exists in the system, and if it is available to use. If exists but not available, explain why not,
// unless bSilent is TRUE. The other information, such as ProductItemID and ProductID, are filled even if the product
// is not available to use.
// (j.jones 2008-06-11 16:54) - PLID 28739 - added another flag to allow completed allocations
// (c.haag 2008-06-18 11:54) - PLID 30427 - Added flags to allow filtering on product item status
//TES 7/7/2008 - PLID 24726 - Added an optional array of ProductItemsT IDs that you specifically do not want returned by
// this function (you may have "used" them in memory, but not in the database)
BOOL Barcode_CheckExistenceOfSerialNumber(const CString& strSerialNum, long nLocationID, BOOL bSilent, long &nProductItemID, long &nProductID, CString &strProductName, _variant_t &varExpDate,
										  OPTIONAL OUT Barcode_CheckExistenceOfSerialNumberResultEx* pResult /* = NULL */,
										  OPTIONAL BOOL bAllowActiveAllocations /* = FALSE */,
										  OPTIONAL BOOL bAllowCompleteAllocations /*= FALSE*/,
										  OPTIONAL BOOL bAllowPurchasedInv /* = TRUE*/,
										  OPTIONAL BOOL bAllowConsignment /* = TRUE*/,
										  OPTIONAL CArray<long,long> *parProductItemIDsToExclude /*= NULL*/)
{
	try {
		//mostly lifted from the Overview Tab's query

		// (c.haag 2007-12-04 17:42) - PLID 28237 - Added other fields
		//TES 5/27/2008 - PLID 29459 - Updated to use the linked appointment date (if any) as the "DateUsed"
		// (j.jones 2008-06-02 16:16) - PLID 28076 - adjusted products now use the adjustment date,
		// but it can be null if it is old data prior to when we started tracking adjustment IDs
		// (j.jones 2008-06-11 12:11) - PLID 28379 - added AllocationID and AllocationDetailID
		//TES 6/18/2008 - PLID 29578 - Updated (and simplified) the query now that ProductItemsT has an OrderDetailID instead
		// of an OrderID
		//TES 7/3/2008 - PLID 24726 - Added SerialNumberIsLotNum 
		_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM "
			"(SELECT ProductItemsT.ID, ProductItemsT.ProductID, ServiceT.Name AS ProductName, "
			"ProductItemsT.ExpDate, LocationsT.ID AS LocationID, LocationsT.Name AS LocName, ProductItemsT.Status AS ProductItemStatus, "
			"ProductT.LastCost, ProductT.HasSerialNum, ProductT.SerialNumIsLotNum, ProductT.HasExpDate, AllocatedItemsQ.AllocationID, AllocatedItemsQ.AllocationDetailID, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN {INT} "
			"WHEN ServiceT.Active = 0 THEN {INT} "
			"WHEN ProductItemsT.Deleted = 1 THEN {INT} "
			"WHEN LineItemT.ID Is Not Null THEN {INT} "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN {INT} "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = {INT} THEN {INT} ELSE {INT} END) "
			"ELSE {INT} END AS StatusType, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN '' "
			"WHEN ServiceT.Active = 0 THEN '' "
			"WHEN ProductItemsT.Deleted = 1 THEN '' "
			"WHEN LineItemT.ID Is Not Null THEN ChargedPersonT.Last + ', ' + ChargedPersonT.First + ' ' + ChargedPersonT.Middle "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CasePersonT.Last + ', ' + CasePersonT.First + ' ' + CasePersonT.Middle "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedPersonT.Last + ', ' + AllocatedPersonT.First + ' ' + AllocatedPersonT.Middle "
			"ELSE '' END AS PatientName, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN SupplierReturnGroupsT.ReturnDate "
			"WHEN ServiceT.Active = 0 THEN NULL "
			"WHEN ProductItemsT.Deleted = 1 THEN ProductAdjustmentsT.Date "
			"WHEN LineItemT.ID Is Not Null THEN LineItemT.Date "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CaseHistoryT.SurgeryDate "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = {INT} THEN AllocatedItemsQ.CompletionDate ELSE AllocatedItemsQ.InputDate END) "
			"ELSE NULL END AS DateUsed "
			"FROM ProductItemsT "
			"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
			"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
			"LEFT JOIN (SELECT PatientInvAllocationsT.PatientID, ProductItemID, InputDate, COALESCE(AppointmentsT.StartTime, PatientInvAllocationsT.CompletionDate) AS CompletionDate, "
			"			PatientInvAllocationDetailsT.Status, PatientInvAllocationDetailsT.AllocationID, PatientInvAllocationDetailsT.ID AS AllocationDetailID "
			"			FROM PatientInvAllocationDetailsT "
			"		   INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		   LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"		   WHERE ((PatientInvAllocationDetailsT.Status = {INT} AND PatientInvAllocationsT.Status = {INT}) OR (PatientInvAllocationDetailsT.Status = {INT} AND PatientInvAllocationsT.Status = {INT})) "
			"		   ) AS AllocatedItemsQ ON ProductItemsT.ID = AllocatedItemsQ.ProductItemID "
			"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "
			"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"LEFT JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
			"LEFT JOIN PersonT AS AllocatedPersonT ON AllocatedItemsQ.PatientID = AllocatedPersonT.ID "
			"LEFT JOIN PersonT AS ChargedPersonT ON LineItemT.PatientID = ChargedPersonT.ID "
			"LEFT JOIN PersonT AS CasePersonT ON CaseHistoryT.PersonID = CasePersonT.ID "
			"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
			"LEFT JOIN OrderDetailsT ON ProductItemsT.OrderDetailID = OrderDetailsT.ID "
			"WHERE ProductItemsT.SerialNum = {STRING}) "
			"AS SerialCheckerQ ORDER BY StatusType",
			stReturned, stInactive, stAdjusted, stBilled, stCaseHistory, InvUtils::iadsUsed, stCompletedAllocation, stAllocated, stAvailable,
			InvUtils::iadsUsed, InvUtils::iadsActive, InvUtils::iasActive, InvUtils::iadsUsed, InvUtils::iasCompleted,
			strSerialNum);

		//the recordset is ordered such that in the case of multiple products with the same serial number,
		//any available product is the first record returned

		if(rs->eof) {
			//not found, return accordingly

			nProductItemID = -1;
			nProductID = -1;
			strProductName = "";
			return FALSE;
		}
		else {
			//TES 7/7/2008 - PLID 24726 - We'll need to know whether we're treating the serial number as a lot number.
			BOOL bSerialNumIsLotNum = AdoFldBool(rs, "SerialNumIsLotNum", VARIANT_FALSE);
			 
			//TES 7/16/2008 - PLID 27983 - This block of code used to only execute if there were multiple records, but it
			// applies just the same if there's only one.

			// First, try to eliminate results by traversing through the recordset to find 
			// only records that qualify given our optional flags.
			bool bContinue = true;
			CArray<long,long> anIDs;
			while (!rs->eof && bContinue) {

				Barcode_CheckExistenceOfSerialNumberStatus stStatus = (Barcode_CheckExistenceOfSerialNumberStatus)AdoFldLong(rs, "StatusType", stAvailable);
				ProductItemStatus pis = (ProductItemStatus)AdoFldLong(rs, "ProductItemStatus");
				long nID = AdoFldLong(rs, "ID");

				// See if the item even has a chance of being acceptable.
				//TES 7/7/2008 - PLID 24726 - If it's in our list to exclude, then we definitely can't use it.
				bool bIsExcluded = false;
				if(parProductItemIDsToExclude) {
					for(int i = 0; i < parProductItemIDsToExclude->GetSize() && !bIsExcluded; i++) {
						if(parProductItemIDsToExclude->GetAt(i) == nID) {
							bIsExcluded = true;
						}
					}
				}
				if(bIsExcluded) 
				{
					//TES 7/7/2008 - PLID 24726 - Do nothing.  This item is unacceptable, but future items might be
					// acceptable.

				} else if(stStatus == stAvailable || stStatus == stCompletedAllocation || stStatus == stAllocated)
				{
					// If we get here, the item *might* be acceptable. Find out for sure.
					// (c.haag 2008-06-18 11:58) - PLID 30427 - Factor in product item status
					if ((bAllowPurchasedInv && pisPurchased == pis) || (bAllowConsignment && pisConsignment == pis))
					{
						if(stStatus == stAvailable ||
							(bAllowActiveAllocations && stStatus == stAllocated)
							|| (bAllowCompleteAllocations && stStatus == stCompletedAllocation))
						{
							// Yes, the item is acceptable
							anIDs.Add(nID);
						}
					}
				}
				else {
					// If we get here, the item is unacceptable, and the rest of the items will also be unacceptable
					// because the sort order in the recordset tells us that this and all the rest of the records are
					// in use. Stop the iterating.
					bContinue = false;
				}
				rs->MoveNext();

			} // while (!rs->eof && bContinue) {

			// Now use the content of anIDs to proceed
			if (0 == anIDs.GetSize()) {
				// There are no acceptable records. Just go back to the first one, and the
				// code that follows will return failure.
				rs->MoveFirst();

				//TES 7/7/2008 - PLID 24726 - There may be some in the list that were acceptable but excluded, so loop
				// through until we find an unacceptable status.
				Barcode_CheckExistenceOfSerialNumberStatus stStatus = (Barcode_CheckExistenceOfSerialNumberStatus)AdoFldLong(rs, "StatusType", stAvailable);
				while((stStatus == stAvailable || 
					(bAllowActiveAllocations && stStatus == stAllocated)
					|| (bAllowCompleteAllocations && stStatus == stCompletedAllocation)) && !rs->eof ) {
					rs->MoveNext();
					if(!rs->eof) {
						stStatus = (Barcode_CheckExistenceOfSerialNumberStatus)AdoFldLong(rs, "StatusType", stAvailable);
					}
				}
				if(stStatus == stAvailable || 
					(bAllowActiveAllocations && stStatus == stAllocated)
					|| (bAllowCompleteAllocations && stStatus == stCompletedAllocation)) {
					//TES 7/7/2008 - PLID 24726 - If we get here, that means every item was acceptable.  Since none of them
					// got added to our array, that means they must all be excluded.  Therefore, we couldn't find any 
					// acceptable items, so just return FALSE.
					nProductItemID = -1;
					nProductID = -1;
					strProductName = "";
					return FALSE;
				}
			}
			else if (1 == anIDs.GetSize()) {
				// Only one record passed the test. Move to it, and let the code that
				// follows return success.
				rs->MoveFirst();
				while (!rs->eof) {
					if (AdoFldLong(rs, "ID") == anIDs[0]) {
						break;
					}
					rs->MoveNext();
				}
			}
			else {
				// If we get here, there are several genuinely qualifying records.

				// First, see if we're in silent mode. If so, just go to the first qualifying record.
				//TES 7/7/2008 - PLID 24726 - We also want the first qualifying record if we're treating serial numbers
				// as lot numbers, because in that case, any record is as good as any other.
				if (bSilent || bSerialNumIsLotNum) {

					rs->MoveFirst();
					while (!rs->eof) {
						if (AdoFldLong(rs, "ID") == anIDs[0]) {
							break;
						}
						rs->MoveNext();
					}

				} else {

					// Figures, we'll have to do this the hard way. Invoke the item chooser.
					CInvMultiSerialPickerDlg imsp(NULL);
					imsp.m_prs = rs;
					imsp.m_anIDs.Copy(anIDs);
					imsp.m_strSerialNum = strSerialNum;
					if (IDOK == imsp.DoModal()) {

						// If we get here, the user chose a specific item. Seek to the record
						// corresponding to the item, and let the code that follows do its normal
						// work.
						rs->MoveFirst();
						while (!rs->eof) {
							if (AdoFldLong(rs, "ID") == imsp.m_nResultID) {
								break;
							}
							rs->MoveNext();
						}						

					} else {
						// User changed their mind
						nProductItemID = -1;
						nProductID = -1;
						strProductName = "";
						return FALSE;
					}
				}
			}

			Barcode_CheckExistenceOfSerialNumberStatus stStatus = (Barcode_CheckExistenceOfSerialNumberStatus)AdoFldLong(rs, "StatusType", stAvailable);

			nProductItemID = AdoFldLong(rs, "ID", -1);
			nProductID = AdoFldLong(rs, "ProductID", -1);
			strProductName = AdoFldString(rs, "ProductName", "");
			varExpDate = rs->Fields->Item["ExpDate"]->Value;

			// (c.haag 2007-12-04 17:41) - PLID 28237 - Fill in data that the caller can use
			if (NULL != pResult) {
				pResult->varProductLastCost = rs->Fields->Item["LastCost"]->Value;
				pResult->bProductHasSerialNum = AdoFldBool(rs, "HasSerialNum", VARIANT_FALSE);
				pResult->bProductHasExpDate = AdoFldBool(rs, "HasExpDate", VARIANT_FALSE); 
				pResult->nProductLocationID = AdoFldLong(rs, "LocationID", -1);
				pResult->pisStatus = (ProductItemStatus)AdoFldLong(rs, "ProductItemStatus");
				pResult->stStatus = stStatus;
				// (j.jones 2008-06-11 12:08) - PLID 28379 - added nAllocationID and nAllocationDetailID
				if((bAllowActiveAllocations && stStatus == stAllocated)
					|| (bAllowCompleteAllocations && stStatus == stCompletedAllocation)) {
					pResult->nAllocationID = AdoFldLong(rs, "AllocationID", -1);
					pResult->nAllocationDetailID = AdoFldLong(rs, "AllocationDetailID", -1);
				}
				else {
					pResult->nAllocationID = -1;
					pResult->nAllocationDetailID = -1;
				}
			}

			// (c.haag 2008-06-18 11:58) - PLID 30427 - Factor in product item status
			ProductItemStatus pis = (ProductItemStatus)AdoFldLong(rs, "ProductItemStatus");
			if (!((bAllowPurchasedInv && pisPurchased == pis) || (bAllowConsignment && pisConsignment == pis))) {
				// Quietly fail if the record we require doesn't match the requested status
				return FALSE;
			}

			if(stStatus == stAvailable ||
				// (c.haag 2007-12-05 12:33) - PLID 28237 - In cases like transferring product items in and
				// out of consignment, we do allow for allocated product items to pass our filters so long as
				// they are not completed allocations.
				(bAllowActiveAllocations && stStatus == stAllocated)
				|| (bAllowCompleteAllocations && stStatus == stCompletedAllocation)) {

				//only one catch - is it available for our location?
				long nProductLocationID = AdoFldLong(rs, "LocationID", -1);
				// (c.haag 2007-12-05 09:32) - PLID 28237 - If the input location ID is -1, that means
				// any location is acceptable
				if(nProductLocationID != -1 && nLocationID != -1 && nProductLocationID != nLocationID) {
					//if there is an assigned location ID, and it's not our location, we
					//can't use this product item
					
					CString strLocationName = AdoFldString(rs, "LocName", "");

					if(!bSilent) {
						CString str;
						str.Format("The barcode you scanned matches up with a serial number for the product '%s', "
							"but that serial numbered product is associated with the '%s' location, "
							"and cannot be used at your current location.", strProductName, strLocationName);
						AfxMessageBox(str);
					}
					return FALSE;
				}

				//location checks out, so we can use it, so let's get out of here
				return TRUE;
			}

			//otherwise, if a warning is requested, give one
			if(!bSilent) {

				CString strPatientName = AdoFldString(rs, "PatientName", "");
				_variant_t varStatusDate = rs->Fields->Item["DateUsed"]->Value;

				CString strWarn;

				CString strDate = "";
				if(varStatusDate.vt == VT_DATE) {
					strDate = FormatDateTimeForInterface(VarDateTime(varStatusDate), NULL, dtoDate);
				}
				
				switch(stStatus) {

					case stReturned: {

						CString strDateString = "";
						if(!strDate.IsEmpty()) {
							strDateString.Format(" on %s", strDate);
						}						

						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s', "
							"but that serial numbered product was returned to its vendor%s.", strProductName, strDateString);
						}
						break;

					case stInactive: {
						
						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s', "
							"but that product has been marked inactive.", strProductName);
						}
						break;

					case stAdjusted: {

						// (j.jones 2008-06-02 16:23) - PLID 28076 - these won't have dates on old
						// data, as we are only just now tracking adjustment dates on serialized items
						CString strDateString = "";
						if(!strDate.IsEmpty()) {
							strDateString.Format(" on %s", strDate);
						}

						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s', "
							"but that serial numbered product was adjusted out of the system%s.", strProductName, strDateString);
						}
						break;

					case stBilled: {

						CString strDateString = "";
						if(!strDate.IsEmpty()) {
							strDateString.Format(" on %s", strDate);
						}
						
						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s', "
							"but that serial numbered product was billed to patient '%s'%s.", strProductName, strPatientName, strDateString);
						}
						break;

					case stCaseHistory: {

						CString strDateString = "";
						if(!strDate.IsEmpty()) {
							strDateString.Format(" with a surgery date of %s", strDate);
						}

						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s', "
							"but that serial numbered product exists on a case history for patient '%s'%s.", strProductName, strPatientName, strDateString);
						}
						break;

					case stCompletedAllocation: {

						CString strDateString = "";
						if(!strDate.IsEmpty()) {
							strDateString.Format(" on %s", strDate);
						}

						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s', "
							"but that serial numbered product was used on a completed allocation for patient '%s'%s.", strProductName, strPatientName, strDateString);
						}
						break;

					case stAllocated: {

						CString strDateString = "";
						if(!strDate.IsEmpty()) {
							strDateString.Format(", created on %s", strDate);
						}

						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s', "
							"but that serial numbered product is currently allocated to patient '%s'%s.", strProductName, strPatientName, strDateString);
						}
						break;
				}

				AfxMessageBox(strWarn);
			}

			return FALSE;
		}
		rs->Close();

	}NxCatchAll("Error in Barcode_CheckExistenceOfSerialNumber");
	
	nProductItemID = -1;
	nProductID = -1;
	strProductName = "";
	return FALSE;
}

// (j.jones 2007-12-06 16:40) - PLID 28196 - PopulateAllocationInfo will populate a given
// pAllocationMasterInfo based on nAllocationID
// (j.jones 2008-02-20 09:23) - PLID 28948 - added a flag for whether to include released items
void PopulateAllocationInfo(long nAllocationID, InvUtils::AllocationMasterInfo *&pAllocationMasterInfo, BOOL bIncludeReleased)
{
	try {
		
		//if already filled, then we don't need to do anything here
		if(pAllocationMasterInfo) {
			return;
		}

		//nAllocationID should never be -1!
		if(nAllocationID == -1) {
			ASSERT(FALSE);
			pAllocationMasterInfo = NULL;
			return;
		}

		//load the allocation information from data
		// (j.jones 2008-01-07 16:41) - PLID 28479 - added IsProductBillable
		// (c.haag 2008-03-11 14:07) - PLID 29255 - Added ProductItemStatus
		// (j.jones 2008-03-12 11:58) - PLID 29102 - added CaseHistoryID
		//TES 6/20/2008 - PLID 26152 - Added ProductItemsT.ToBeReturned
		//TES 7/18/2008 - PLID 29478 - Added IsSerialized
		// (j.jones 2009-02-10 12:01) - PLID 32779 - ensured the appt. subquery filtered
		// on the allocation's appt. ID, because somehow in SQL 2005 it was very slow otherwise
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientInvAllocationsT.PatientID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"PatientInvAllocationsT.LocationID, LocationsT.Name AS LocationName, "
			"PatientInvAllocationsT.AppointmentID, AppointmentsQ.DateTime, "
			"AppointmentsQ.AptTypePurpose, AppointmentsQ.AptResource, "
			"PatientInvAllocationsT.Notes, PatientInvAllocationsT.Status, "
			"PatientInvAllocationsT.InputDate, CaseHistoryAllocationLinkT.CaseHistoryID "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PersonT ON PatientInvAllocationsT.PatientID = PersonT.ID "
			"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
			"LEFT JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
			"LEFT JOIN (SELECT ID, Date + StartTime AS DateTime, Purpose AS AptTypePurpose, Resource AS AptResource "
			"			FROM (SELECT ResBasicQ.ID, ResBasicQ.Date, ResBasicQ.StartTime, dbo.GetResourceString(ResBasicQ.ID) AS Resource, "
			"				CASE WHEN ResBasicQ.AptPurposeName <> '' THEN AptTypeT.Name + ' - ' + ResBasicQ.AptPurposeName ELSE AptTypeT.Name END AS Purpose FROM AptTypeT "
			"				RIGHT JOIN (SELECT AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, dbo.GetPurposeString(AppointmentsT.ID) AS AptPurposeName, "
			"					CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) AS Date, convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS StartTime "
			"					FROM AppointmentsT WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ID IN (SELECT AppointmentID FROM PatientInvAllocationsT WHERE ID = {INT})) "
			"					AS ResBasicQ ON AptTypeT.ID = ResBasicQ.AptTypeID) "
			"				AS ResComplexQ) "
			"			AS AppointmentsQ ON PatientInvAllocationsT.AppointmentID = AppointmentsQ.ID "
			"WHERE PatientInvAllocationsT.ID = {INT}\r\n"
			""
			"SELECT PatientInvAllocationDetailsT.ID, PatientInvAllocationDetailsT.ProductID, "
			"ServiceT.Name AS ProductName, PatientInvAllocationDetailsT.ProductItemID, "
			"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
			"PatientInvAllocationDetailsT.Quantity, PatientInvAllocationDetailsT.Status, "
			"PatientInvAllocationDetailsT.Notes, ProductItemsT.Status AS ProductItemStatus, "
			"Convert(bit, CASE WHEN PatientInvAllocationDetailsT.ID IN "
			"		(SELECT AllocationDetailID FROM ChargedAllocationDetailsT "
			"		WHERE ChargeID IN (SELECT ID FROM LineItemT WHERE Deleted = 0)) "
			"		THEN 1 ELSE 0 END) AS Billed, "
			"ProductLocationInfoT.Billable AS IsProductBillable, "
			"ProductItemsT.ToBeReturned, "
			"Convert(bit, CASE WHEN ProductT.HasSerialNum = 1 OR ProductT.HasExpDate = 1 THEN 1 ELSE 0 END) AS IsSerialized "
			"FROM PatientInvAllocationDetailsT "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"INNER JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
			"LEFT JOIN ProductLocationInfoT ON ServiceT.ID = ProductLocationInfoT.ProductID AND ProductLocationInfoT.LocationID = PatientInvAllocationsT.LocationID "
			"WHERE PatientInvAllocationDetailsT.Status <> {INT} "
			"AND PatientInvAllocationDetailsT.AllocationID = {INT} ",
			nAllocationID,
			nAllocationID,
			InvUtils::iadsDeleted, nAllocationID);

		//load the allocation info
		if(!rs->eof) {

			//create the object
			pAllocationMasterInfo = new InvUtils::AllocationMasterInfo;

			//now populate it
			pAllocationMasterInfo->nAllocationID = nAllocationID;
			pAllocationMasterInfo->nPatientID = AdoFldLong(rs, "PatientID");
			pAllocationMasterInfo->strPatientName = AdoFldString(rs, "PatientName", "");
			pAllocationMasterInfo->nLocationID = AdoFldLong(rs, "LocationID");
			pAllocationMasterInfo->strLocationName = AdoFldString(rs, "LocationName", "");
			pAllocationMasterInfo->nAppointmentID = AdoFldLong(rs, "AppointmentID", -1);
			// (j.jones 2008-03-12 11:58) - PLID 29102 - added CaseHistoryID
			pAllocationMasterInfo->nCaseHistoryID = AdoFldLong(rs, "CaseHistoryID", -1);

			_variant_t varDateTime = rs->Fields->Item["DateTime"]->Value;
			if(pAllocationMasterInfo->nAppointmentID == -1 || varDateTime.vt == VT_NULL) {
				pAllocationMasterInfo->strAppointmentDesc = "<No Appointment Specified>";
			}
			else {
				//format a nice appt. description
				CString strFormat;
				strFormat.Format("%s - %s (%s)", FormatDateTimeForInterface(VarDateTime(varDateTime), NULL, dtoDateTime),
					AdoFldString(rs, "AptTypePurpose", ""), AdoFldString(rs, "AptResource", ""));
				pAllocationMasterInfo->strAppointmentDesc = strFormat;
			}
			pAllocationMasterInfo->strNotes = AdoFldString(rs, "Notes", "");
			pAllocationMasterInfo->iasStatus = (InvUtils::InventoryAllocationStatus)AdoFldLong(rs, "Status");
			pAllocationMasterInfo->dtInputDate = AdoFldDateTime(rs, "InputDate");

			// (j.jones 2007-12-14 16:41) - PLID 27988 - used only in billing
			pAllocationMasterInfo->bHadResolutionAlready = FALSE;
		}
		else {			
			//No data! Do nothing here, because when the caller sees that
			//pAllocationMasterInfo is still NULL, it will cleanly handle
			//the situation.
			return;
		}

		//load the allocation detail info
		rs = rs->NextRecordset(NULL);

		if(rs->eof) {
			//Should only be eof if all the details on the allocation are deleted,
			//in which case why were we even allowed to open this allocation?
			//Don't close out, just assert this oddity.
			ASSERT(FALSE);
		}

		while(!rs->eof) {

			//load allocation details

			// (j.jones 2008-02-20 09:23) - PLID 28948 - if bIncludeReleased is true,
			// skip released items - this function should not be called on an
			// allocation with all released items, but if that were to happen, we want
			// to return an empty allocation
			InvUtils::InventoryAllocationDetailStatus iadsStatus = (InvUtils::InventoryAllocationDetailStatus)AdoFldLong(rs, "Status");
			if(!bIncludeReleased && iadsStatus == InvUtils::iadsReleased) {
				rs->MoveNext();
				continue;
			}

			//create a new detail
			InvUtils::AllocationDetailInfo *pInfo = new InvUtils::AllocationDetailInfo;

			//now populate it
			pInfo->nDetailID = AdoFldLong(rs, "ID");
			pInfo->nProductID = AdoFldLong(rs, "ProductID");
			pInfo->strProductName = AdoFldString(rs, "ProductName", "");
			pInfo->nProductItemID = AdoFldLong(rs, "ProductItemID", -1);
			pInfo->varSerialNum = rs->Fields->Item["SerialNum"]->Value;
			pInfo->varExpDate = rs->Fields->Item["ExpDate"]->Value;
			pInfo->varProductItemStatus = rs->Fields->Item["ProductItemStatus"]->Value; // (c.haag 2008-03-11 14:06) - PLID 29255 - Product item status
			pInfo->dblOriginalQty = AdoFldDouble(rs, "Quantity", 1.0);
			pInfo->dblCurQuantity = pInfo->dblOriginalQty;
			pInfo->iadsOriginalStatus = iadsStatus;
			pInfo->iadsCurrentStatus = pInfo->iadsOriginalStatus; //initialize to match "original" status in data
			pInfo->strOriginalNotes = AdoFldString(rs, "Notes", "");
			pInfo->strNotes = pInfo->strOriginalNotes;
			pInfo->bBilled = AdoFldBool(rs, "Billed", FALSE);	// (j.jones 2007-12-12 10:54) - PLID 27988 - added bBilled
			// (j.jones 2008-01-07 16:39) - PLID 28479 - added bIsProductBillable
			pInfo->bIsProductBillable = AdoFldBool(rs, "IsProductBillable", FALSE);
			//TES 6/20/2008 - PLID 26152 - Added bToBeReturned and bOldToBeReturned
			pInfo->bOldToBeReturned = AdoFldBool(rs, "ToBeReturned", FALSE);
			pInfo->bToBeReturned = pInfo->bOldToBeReturned;

			//TES 7/18/2008 - PLID 29478 - Added bIsSerialized
			pInfo->bIsSerialized = AdoFldBool(rs, "IsSerialized", FALSE);

			//and add it to our list
			pAllocationMasterInfo->paryAllocationDetailInfo.Add(pInfo);

			rs->MoveNext();
		}

		rs->Close();

	}NxCatchAll("Error in PopulateAllocationInfo");
}

// (j.jones 2007-12-07 15:41) - PLID 28196 - given an AllocationMasterInfo object that is to be completed,
// add save statements and audits to the passed in strSqlBatch and nAuditTransactionID
// (j.jones 2008-02-19 17:54) - PLID 28948 - renamed this function and gave it a parameter to determine if we
// are completing the allocation, or just saving changes to an existing one
BOOL GenerateAllocationSaveSql(BOOL bComplete, InvUtils::AllocationMasterInfo *pAllocationMasterInfo, CString &strSqlBatch, long &nAuditTransactionID)
{
	try {

		if(pAllocationMasterInfo == NULL || pAllocationMasterInfo->nAllocationID == -1) {
			//this was called improperly
			ASSERT(FALSE);
			return FALSE;
		}

		// (j.jones 2008-02-19 17:55) - PLID 28948 - check the bComplete parameter to see if we should save this as completed
		if(bComplete) {
			//update the master record
			AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationsT "
				"SET Status = %li, CompletionDate = GetDate(), CompletedBy = %li "
				"WHERE ID = %li", InvUtils::iasCompleted, GetCurrentUserID(), pAllocationMasterInfo->nAllocationID);

			// (j.jones 2007-11-29 11:54) - PLID 28043 - audit the status change -	
			// should have been impossible to re-complete a completed allocation, so
			// just audit that it was changed from active to completed
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(pAllocationMasterInfo->nPatientID, pAllocationMasterInfo->strPatientName, nAuditTransactionID, aeiInvAllocationCompleted, pAllocationMasterInfo->nAllocationID, "Active", "Completed", aepMedium, aetChanged);
		}

		//now loop through all our details
		for(int i=0;i<pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)(pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i));
			if(pInfo) {
			
				long nDetailID = pInfo->nDetailID;

				long nServiceID = pInfo->nProductID;

				CString strProductName = pInfo->strProductName;
				
				long nProductItemID = pInfo->nProductItemID;
				CString strProductItemID = "NULL";
				if(nProductItemID != -1) {
					strProductItemID.Format("%li", nProductItemID);
				}

				//determine the status
				InvUtils::InventoryAllocationDetailStatus iadsOldStatus = pInfo->iadsOriginalStatus;
				InvUtils::InventoryAllocationDetailStatus iadsStatus = pInfo->iadsCurrentStatus;
				BOOL bOldToBeReturned = pInfo->bOldToBeReturned;

				CString strNotes = pInfo->strNotes;
				CString strOldNotes = pInfo->strOriginalNotes;
				strNotes.TrimLeft();
				strNotes.TrimRight();
				strOldNotes.TrimLeft();
				strOldNotes.TrimRight();

				//used for auditing
				CString strDesc = strProductName;
				_variant_t varSerialNumber = pInfo->varSerialNum;
				_variant_t varExpDate = pInfo->varExpDate;

				if(varSerialNumber.vt == VT_BSTR) {
					CString str;
					str.Format(", Serial Num: %s", VarString(varSerialNumber));
					strDesc += str;
				}
				if(varExpDate.vt == VT_DATE) {
					CString str;
					str.Format(", Exp. Date: %s", FormatDateTimeForInterface(VarDateTime(varExpDate), NULL, dtoDate));
					strDesc += str;
				}
				if(pInfo->dblOriginalQty != 1.0) {
					//only show the quantity if not 1.0
					CString str;
					str.Format(", Quantity: %g", pInfo->dblOriginalQty);
					strDesc += str;
				}

				//it is possible that we need to create new details, if we split the quantity
				//of a non-serialized item during the completion process
				if(nDetailID == -1) {
					//create a new detail

					CString strProductItemID = "NULL";
					if(pInfo->nProductItemID != -1) {
						//it should be logically impossible for us to be
						//creating a new serialized detail here
						ASSERT(FALSE);
						strProductItemID.Format("%li", pInfo->nProductItemID);
					}

					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientInvAllocationDetailsT "
						"(AllocationID, ProductID, ProductItemID, Quantity, Status, Notes) "
						"VALUES (%li, %li, %s, %g, %li, '%s')", pAllocationMasterInfo->nAllocationID, pInfo->nProductID,
						strProductItemID, pInfo->dblCurQuantity, pInfo->iadsCurrentStatus, _Q(pInfo->strNotes));

					//TES 6/20/2008 - PLID 26152 - Only check the ToBeReturned flag if it's a released, serialized, Purchased
					// Inventory item.
					BOOL bUpdatedToBeReturned = FALSE;
					if(pInfo->iadsCurrentStatus == InvUtils::iadsReleased &&
						(InvUtils::ProductItemStatus)VarLong(pInfo->varProductItemStatus, -1) == InvUtils::pisPurchased &&
						pInfo->nProductItemID != -1) {
						bUpdatedToBeReturned = TRUE;
						AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET ToBeReturned = %i WHERE ID = %li",
							pInfo->bToBeReturned ? 1 : 0, pInfo->nProductItemID);
					}
					// (j.jones 2007-12-07 15:37) - PLID 28043 - audit the creation
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(pAllocationMasterInfo->nPatientID, pAllocationMasterInfo->strPatientName, nAuditTransactionID, aeiInvAllocationDetailCreated, pAllocationMasterInfo->nAllocationID, "", strDesc, aepMedium, aetCreated);

					if(iadsStatus != InvUtils::iadsActive) {
						//if we just created a new detail, it should not be active
						//create a separate audit record to show the status change

						CString strOldValue, strNewValue;
						AuditEventItems aeiItem = aeiInvAllocationActive;
						
						strOldValue = strDesc;
						strOldValue += ", Status: <New>";

						if(iadsStatus == InvUtils::iadsUsed) {
							strNewValue = "Status: Used";
							aeiItem = aeiInvAllocationDetailUsed;
						}
						else if(iadsStatus == InvUtils::iadsReleased) {
							if(bUpdatedToBeReturned && pInfo->bToBeReturned) {
								strNewValue = "Status: Released - To Be Returned";
							}
							else {
								strNewValue = "Status: Released";
							}
							aeiItem = aeiInvAllocationDetailReleased;
						}
						else if(iadsStatus == InvUtils::iadsOrder) {
							//TES 7/18/2008 - PLID 29478 - New status
							strNewValue = "Status: To Be Ordered";
							aeiItem = aeiInvAllocationDetailToBeOrdered;
						}
						else {
							//should be impossible
							ASSERT(FALSE);
						}
						AuditEvent(pAllocationMasterInfo->nPatientID, pAllocationMasterInfo->strPatientName, nAuditTransactionID, aeiItem, pAllocationMasterInfo->nAllocationID, strOldValue, strNewValue, aepMedium, aetChanged);
					}
				}
				else {
					//update the detail in data
					//TES 7/21/2008 - PLID 29478 - The ProductItemID may have changed.
					long nProductItemID = pInfo->nProductItemID;
					CString strProductItemID = "NULL";
					if(nProductItemID != -1) {
						strProductItemID.Format("%li", nProductItemID);
					}
					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT SET Status = %li, Quantity = %g, Notes = '%s', ProductItemID = %s WHERE ID = %li", iadsStatus, pInfo->dblCurQuantity, _Q(strNotes), strProductItemID, nDetailID);

					BOOL bUpdatedToBeReturned = FALSE;
					if(pInfo->iadsCurrentStatus == InvUtils::iadsReleased &&
						(InvUtils::ProductItemStatus)VarLong(pInfo->varProductItemStatus, -1) != InvUtils::pisConsignment &&
						pInfo->nProductItemID != -1) {
						bUpdatedToBeReturned = TRUE;
						AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET ToBeReturned = %i WHERE ID = %li",
							pInfo->bToBeReturned ? 1 : 0, pInfo->nProductItemID);
					}

					// (j.jones 2007-11-29 12:04) - PLID 28043 - audit accordingly

					//audit the status change
					//TES 6/20/2008 - PLID 26152 - The ToBeReturned flag also counts as a status change.
					if(iadsStatus != iadsOldStatus ||
						(bUpdatedToBeReturned && bOldToBeReturned != pInfo->bToBeReturned) ){

						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}

						CString strOldValue, strNewValue;
						AuditEventItems aeiItem = aeiInvAllocationDetailUsed;
						
						strOldValue = strDesc;

						if(iadsOldStatus == InvUtils::iadsActive) {
							strOldValue += ", Status: Active";
						}
						else if(iadsOldStatus == InvUtils::iadsUsed) {
							//should be impossible
							ASSERT(FALSE);
							strOldValue += ", Status: Used";
						}
						else if(iadsOldStatus == InvUtils::iadsReleased) {
							//TES 6/20/2008 - PLID 26152 - If this was flagged as ToBeReturned, include that in our auditing.
							if(bOldToBeReturned) {
								strOldValue += ", Status: Released - To Be Returned";
							}
							else {
								strOldValue += ", Status: Released";
							}
						}
						else if(iadsOldStatus == InvUtils::iadsOrder) {
							//TES 7/18/2008 - PLID 29478 - New status
							strOldValue += ", Status: To Be Ordered";
						}
						else {
							//should be impossible
							ASSERT(FALSE);
						}

						if(iadsStatus == InvUtils::iadsActive) {
							//should be impossible
							//TES 8/21/2008 - PLID 29478 - This is possible now, if it was previously on order.
							//ASSERT(FALSE);
							strNewValue = "Status: Active";
							aeiItem = aeiInvAllocationDetailActive;
						}
						else if(iadsStatus == InvUtils::iadsUsed) {
							strNewValue = "Status: Used";
							aeiItem = aeiInvAllocationDetailUsed;
						}
						else if(iadsStatus == InvUtils::iadsReleased) {
							//TES 6/20/2008 - PLID 26152 - If we updated the ToBeReturned flag, include that in our auditing.
							if(bUpdatedToBeReturned && pInfo->bToBeReturned) {
								strNewValue = "Status: Released - To Be Returned";
							}
							else {
								strNewValue = "Status: Released";
							}
							aeiItem = aeiInvAllocationDetailReleased;
						}
						else if(iadsStatus == InvUtils::iadsOrder) {
							//TES 7/18/2008 - PLID 29478 - New status
							strNewValue = "Status: To Be Ordered";
							aeiItem = aeiInvAllocationDetailToBeOrdered;
						}
						else {
							//should be impossible
							ASSERT(FALSE);
						}
						AuditEvent(pAllocationMasterInfo->nPatientID, pAllocationMasterInfo->strPatientName, nAuditTransactionID, aeiItem, pAllocationMasterInfo->nAllocationID, strOldValue, strNewValue, aepMedium, aetChanged);
					}

					//audit the notes change
					if(strNotes != strOldNotes) {

						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}

						CString strOldValue;
						strOldValue = strDesc + ", Notes: " + strOldNotes;
						AuditEvent(pAllocationMasterInfo->nPatientID, pAllocationMasterInfo->strPatientName, nAuditTransactionID, aeiInvAllocationDetailNotes, pAllocationMasterInfo->nAllocationID, strOldValue, strNotes, aepMedium, aetChanged);
					}

					//audit the quantity change
					if(pInfo->dblCurQuantity != pInfo->dblOriginalQty) {
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(pAllocationMasterInfo->nPatientID, pAllocationMasterInfo->strPatientName, nAuditTransactionID, aeiInvAllocationDetailQuantity, pAllocationMasterInfo->nAllocationID, strDesc, AsString(pInfo->dblCurQuantity), aepMedium, aetChanged);
					}
				}
			}
		}

		return TRUE;

	}NxCatchAll("Error in GenerateAllocationSaveSql");

	return FALSE;
}

BOOL Barcode_CheckExistenceOfProductCode(const CString& strBarcode, BOOL bWasScanning, BOOL bSilent /* = FALSE */)
{
	CString strProductName;
	return Barcode_CheckExistenceOfProductCode(strBarcode, bWasScanning, strProductName, bSilent);
}

// (z.manning 2008-07-07 11:43) - PLID 30602 - Added an overload that also returns the name
// of the product if the barcode exists.
BOOL Barcode_CheckExistenceOfProductCode(const CString& strBarcode, BOOL bWasScanning, OUT CString &strProductName, BOOL bSilent /* = FALSE */)
{
	// (c.haag 2008-03-18 17:20) - PLID 29306 - This function will display a warning to the user, and return
	// TRUE if a product with the given barcode exists. If there is no such product, or an error occurs, this
	// function returns FALSE. If the code was scanned in, bWasScanning should be TRUE. If keyed in, FALSE.
	// bWasScanning affects the output of the message box.
	try {
		// This query is consistent with other functions that search for product barcodes, and the
		// calling functions that I've observed only use the first record. Our UI forces the barcode
		// of each product to be unique, anyway.
		_RecordsetPtr rs = CreateParamRecordset("SELECT ServiceT.Name FROM ProductT "
			"INNER JOIN ServiceT ON ServiceT.ID = ProductT.ID "
			"WHERE ServiceT.Barcode = {STRING} AND ServiceT.Active = 1 ",
			strBarcode);

		if (!rs->eof) {
			// We found a record. We must warn the user of it
			strProductName = AdoFldString(rs, "Name");
			// (z.manning 2008-07-07 11:53) - PLID 30602 - There's now an optional parameter to not
			// show the message box.
			if(!bSilent) {
				if (bWasScanning) {
					AfxMessageBox(FormatString("You have scanned the serial number '%s'; but the serial number also "
						"matches the barcode for the product '%s'.\n\n"
						"If the serial number you scanned is correct, please disregard this message.",
						strBarcode, strProductName), MB_ICONWARNING);
				} else {
					AfxMessageBox(FormatString("You have entered the serial number '%s'; but the serial number also "
						"matches the barcode for the product '%s'.\n\n"
						"If the serial number you entered is correct, please disregard this message.",
						strBarcode, strProductName), MB_ICONWARNING);
				}
			}
			
			return TRUE;
		} else {
			// No records found
			return FALSE;
		}
	}
	NxCatchAll("Error in Barcode_CheckExistenceOfProductCode");
	return FALSE;
}

// (j.jones 2008-03-19 15:05) - PLID 29311 - Given an order ID, see if an appt. is linked to it,
// and if so, try to create an allocation for that patient, with the contents of the order.
void TryCreateAllocationFromOrder(long nOrderID)
{
	try {

		if(nOrderID == -1) {
			return;
		}
		
		long nAppointmentID = -1;
		long nPatientID = -1;
		long nLocationID = -1;
		long nProviderID = -1;

		//confirm that the order has been at least partially received, and that the order has a linked appointment

		//this query will return at least one record if the order has a received product,
		//and will return appointment information if there is a linked appointment
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 "
			"AppointmentsT.ID AS ApptID, AppointmentsT.PatientID, OrderT.LocationID, ResourceProviderLinkT.ProviderID "
			"FROM OrderT "
			"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
			"LEFT JOIN OrderAppointmentsT ON OrderT.ID = OrderAppointmentsT.OrderID "
			"LEFT JOIN AppointmentsT ON OrderAppointmentsT.AppointmentID = AppointmentsT.ID "
			"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
			"LEFT JOIN ResourceProviderLinkT ON ResourceT.ID = ResourceProviderLinkT.ResourceID "
			"WHERE OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
			"AND AppointmentsT.PatientID <> -25 "
			"AND OrderDetailsT.DateReceived Is Not Null "
			"AND OrderT.ID = {INT}", nOrderID);
		if(rs->eof) {
			//would be eof only if there are no received products
			AfxMessageBox("An allocation cannot be created from this order because there are no products on this order that have been received yet.");
			return;
		}
		else {

			//if the order has a linked appointment, we will create an allocation
			//with that patient, appointment, order location, and potentially a provider
			nAppointmentID = AdoFldLong(rs, "ApptID",-1);

			if(nAppointmentID == -1) {
				//there isn't an appointment
				AfxMessageBox("An allocation cannot be created from this order because there is no appointment linked to this order.");
				return;
			}

			nPatientID = AdoFldLong(rs, "PatientID",-1);
			nLocationID = AdoFldLong(rs, "LocationID",-1);

			//we try to pull the provider from the appt. resource if possible
			nProviderID = AdoFldLong(rs, "ProviderID",-1);
		}
		rs->Close();

		if(nAppointmentID != -1) {

			//create an allocation with the information we have just loaded,
			//pass in the order ID so it can load from that order
			//add every received item from the order to the allocation,
			//including all serial numbers, provided the serial numbers
			//have not been used already

			CWaitCursor pWait;

			CInvPatientAllocationDlg dlg(NULL);
			dlg.m_nDefaultPatientID = nPatientID;
			dlg.m_nDefaultAppointmentID = nAppointmentID;
			dlg.m_nDefaultLocationID = nLocationID;
			dlg.m_nDefaultProviderID = nProviderID;
			dlg.m_nCreateFromOrderID = nOrderID;
			dlg.DoModal();
		}

	}NxCatchAll("Error in TryCreateAllocationFromOrder");
}

BOOL IsProductItemSerialNumberInUse(long nProductID, const CString& strSerialNum)
{
	// (c.haag 2008-03-20 13:05) - PLID 29335 - Warns the user, and returns TRUE if 
	// the serial number is in use for a given product. Returns FALSE if the serial is
	// not in use for the given product.
	_RecordsetPtr prs = CreateParamRecordset("SELECT ServiceT.Name FROM ServiceT "
		"INNER JOIN ProductItemsT ON ProductItemsT.ProductID = ServiceT.ID "
		"WHERE ServiceT.ID = {INT} AND ProductItemsT.SerialNum = {STRING} "
		"AND ProductItemsT.Deleted = 0 ",
		nProductID, strSerialNum);
	if (!prs->eof) {
		const CString strProductName = AdoFldString(prs, "Name");
		AfxMessageBox(FormatString("The serial number '%s' is already in use for the product '%s'",
			strSerialNum, strProductName), MB_ICONERROR | MB_OK);
		return TRUE;
	} else {
		return FALSE;
	}
}

// (c.haag 2008-06-11 15:15) - PLID 28474 - This function returns the NoteCatsF entry where the
// description is 'Inv Order'. This is used in several places.
//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
long InvUtils::GetInvOrderTodoCategory(bool bInTransaction)
{
	long nCatID = -1;
	_RecordsetPtr rs = CreateRecordset("SELECT ID FROM NoteCatsF WHERE Description = 'Inv Order'");
	if(!rs->eof) {
		nCatID = rs->Fields->Item["ID"]->Value.lVal;
	}
	else {
		//TES 8/1/2011 - PLID 44716 - Moved to GlobalUtils function
		//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
		nCatID = CreateNoteCategory("Inv Order", bInTransaction);
	}
	rs->Close();
	return nCatID;
}

// (c.haag 2008-06-11 14:43) - PLID 28474 - Update order todo alarms
void InvUtils::UpdateOrderTodoAlarms(long nOrderID)
{
	// First, delete any existing todo alarms for this order
	TodoDelete(FormatString("RegardingType = %d AND RegardingID = %d", ttInvOrder, nOrderID));

	// Now open the order
	_RecordsetPtr prs = CreateParamRecordset(
		// Order
		"SELECT DateDue, LocationID, Description FROM OrderT WHERE ID = {INT} AND Deleted = 0 AND DateDue IS NOT NULL\r\n"
		// Order Details
		"SELECT ProductID, CASE WHEN DateReceived IS NULL THEN 0 ELSE 1 END AS WasReceived FROM OrderDetailsT "
		"WHERE Deleted = 0 AND OrderID IN (SELECT ID FROM OrderT WHERE ID = {INT} AND Deleted = 0 AND DateDue IS NOT NULL)\r\n"
		, nOrderID, nOrderID);
	FieldsPtr fOrder = prs->Fields;

	//- Create to-do when creating order, if "date due" is set.
	// Read the order info
	if (prs->eof) {
		// If we get here, the order is either deleted, non-existent, or has no due date. So, don't
		// try to create any todo alarms
		return;
	}
	const COleDateTime dtDateDue = AdoFldDateTime(fOrder, "DateDue");
	const long nLocationID = AdoFldLong(fOrder, "LocationID");
	const CString strDescription = AdoFldString(fOrder, "Description");

	// Read the order details
	prs = prs->NextRecordset(NULL);
	FieldsPtr fDetails = prs->Fields;
	if (prs->eof) {
		// If we get here, there are no order details. So, don't try to create any todo alarms
		return;
	}
	BOOL bOrderFullyReceived = TRUE;
	CArray<long,long> anProductIDs;
	while (!prs->eof) {
		anProductIDs.Add( AdoFldLong(fDetails, "ProductID") );
		if (!AdoFldLong(fDetails, "WasReceived")) {
			bOrderFullyReceived = FALSE;
		}
		prs->MoveNext();
	}

	//- Set the 'start remind' to date due + 1.  This way they don't see it.
	const COleDateTime dtRemind = dtDateDue + COleDateTimeSpan(1,0,0,0);

	// Calculate the users responsible for this todo. These are the users responsible for the individual
	// products contained within the order. If there are none, we will use the current user ID instead.
	CArray<long,long> anUsers;
	_RecordsetPtr prsUsers = CreateRecordset("SELECT UserID FROM ProductResponsibilityT "
		"WHERE ProductID IN (%s) AND LocationID = %d", ArrayAsString(anProductIDs), nLocationID);
	while (!prsUsers->eof) {
		anUsers.Add(AdoFldLong(prsUsers, "UserID"));
		prsUsers->MoveNext();
	}
	if (0 == anUsers.GetSize()) {
		anUsers.Add(GetCurrentUserID());
	}

	// Create the todo
	//TES 11/15/2011 - PLID 44716 - GetInvOrderTodoCategory() needs to know if we're in a transaction
	long nTodoID = TodoCreate(dtRemind, dtRemind, anUsers, strDescription, "", nOrderID, ttInvOrder, -1, nLocationID,
		ttpLow, GetInvOrderTodoCategory(false));

	//- When order is marked complete, mark to-do complete (not vice versa)
	//- If partial inv is received, item remains in to do list until full order received.
	if (nTodoID > 0 && bOrderFullyReceived) {
		ExecuteParamSql("UPDATE TodoList SET Done = GetDate() WHERE TaskID = {INT}", nTodoID);
	}
}

//TES 6/16/2008 - PLID 30394 - Are there any appointments that meet our criteria for needing an allocation, but don't
// have one?
BOOL DoAppointmentsWithoutAllocationsExist()
{
	//TES 6/16/2008 - PLID 30394 - Keep in sync with ApptsWithoutAllocationsDlg.
	// (j.jones 2009-12-21 17:02) - PLID 35169 - we also want to show appointments that have allocations 
	// with any products marked "To Be Ordered", and those with orders that have any products still
	// unreceived
	return ReturnsRecords("SELECT AppointmentsT.ID "
		"FROM AppointmentsT INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID INNER JOIN AptTypeT ON "
		"AppointmentsT.AptTypeID = AptTypeT.ID INNER JOIN AppointmentPurposeT ON AppointmentsT.ID = "
		"AppointmentPurposeT.AppointmentID INNER JOIN ApptsRequiringAllocationsDetailT ON AptTypeT.ID = "
		"ApptsRequiringAllocationsDetailT.AptTypeID AND AppointmentPurposeT.PurposeID = "
		"ApptsRequiringAllocationsDetailT.AptPurposeID "
		"WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND "
		"AppointmentsT.PatientID > 0 AND "
		// (a.walling 2013-02-08 13:04) - PLID 55084 - Avoid large appointment index scans in the scheduler
		// dbo.AsDateNoTime(AppointmentsT.StartTime) ends up requring a full scan of the index
		"AppointmentsT.StartTime >= dbo.AsDateNoTime(getdate()) AND "
		"AppointmentsT.StartTime <= dbo.AsDateNoTime(DATEADD(day,%li,getdate())) "
		"AND "
		"("
			//show appointments that have neither an allocation or an order
			"("				
				"AppointmentsT.ID NOT IN (SELECT AppointmentID FROM PatientInvAllocationsT "
					"WHERE AppointmentID Is Not Null AND PatientInvAllocationsT.Status <> %li) "
				"AND AppointmentsT.ID NOT IN (SELECT AppointmentID FROM OrderAppointmentsT "
					"INNER JOIN OrderT ON OrderAppointmentsT.OrderID = OrderT.ID "
					"WHERE OrderT.Deleted = 0) "
			") "
			"OR "
				//show appointments that have an allocation that has any "to be ordered" product
				"AppointmentsT.ID IN (SELECT AppointmentID FROM PatientInvAllocationsT "
					"WHERE AppointmentID Is Not Null AND PatientInvAllocationsT.Status <> %li "
					"AND ID IN (SELECT AllocationID FROM PatientInvAllocationDetailsT WHERE Status = %li)) "
			"OR "
				//show appointments that have an order that has any unreceived product
				"AppointmentsT.ID IN (SELECT AppointmentID FROM OrderAppointmentsT "
					"INNER JOIN OrderT ON OrderAppointmentsT.OrderID = OrderT.ID "
					"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
					"WHERE OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
					"AND OrderDetailsT.DateReceived Is Null) "
		")",
		GetRemotePropertyInt("ApptsRequiringAllocationsDays", 14, 0, "<None>", true),
		InvUtils::iasDeleted, InvUtils::iasDeleted, InvUtils::iadsOrder);
}

// (c.haag 2008-06-25 11:05) - PLID 28438 - Returns TRUE if an order has at least one order
// detail with a received date
BOOL IsOrderPartiallyOrFullyReceived(long nOrderID)
{
	_RecordsetPtr prs = CreateParamRecordset("SELECT COUNT(*) AS OrderDetailCount FROM OrderDetailsT "
		"WHERE DateReceived IS NOT NULL AND Deleted = 0 AND OrderID = {INT}", nOrderID);
	long nCount = AdoFldLong(prs, "OrderDetailCount");
	return (nCount == 0) ? FALSE : TRUE;
}

//TES 7/23/2008 - PLID 30802 - Goes through all the order details, and updates any allocations linked to those details
// to reflect the fact that they are now received.  Will output a list of allocation IDs that got updated, in case you
	// want to inform the user.
void UpdateLinkedAllocationDetails(const CArray<ReceivedOrderDetailInfo,ReceivedOrderDetailInfo&> &arReceivedDetails, OUT CArray<long,long> &arUpdatedAllocationIDs)
{
	//TES 7/23/2008 - PLID 30802 - Resolve all the allocations tied to these order details, by splitting up 
	// allocation details as necessary, and updating their status. 

	// (j.jones 2010-01-04 14:16) - PLID 36080 - parameterized this function, and used sql batch executes per detail
	
	for(int i = 0; i < arReceivedDetails.GetSize(); i++) {
		ReceivedOrderDetailInfo rodi = arReceivedDetails[i];
		long nQuantityRemaining = rodi.nQuantity;
		//TES 7/23/2008 - PLID 30802 - Pull all the allocation details that have this order detail, ordered by appointment date then input date.
		_RecordsetPtr rsAllocDetails = CreateParamRecordset("SELECT PatientInvAllocationDetailsT.ID, "
			"PatientInvAllocationDetailsT.Quantity, PatientInvAllocationDetailsT.AllocationID "
			"FROM PatientInvAllocationDetailsT INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID "
			"= PatientInvAllocationsT.ID LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"WHERE PatientInvAllocationDetailsT.OrderDetailID = {INT} AND PatientInvAllocationDetailsT.Status = {INT} "
			"AND PatientInvAllocationsT.Status <> {INT} "
			"ORDER BY COALESCE(AppointmentsT.StartTime, '5000-12-31'), PatientInvAllocationsT.InputDate",
			rodi.nOrderDetailID, InvUtils::iadsOrder, InvUtils::iasDeleted);

		{
			while(!rsAllocDetails->eof && nQuantityRemaining > 0) {

				CString strSqlBatch;
				CNxParamSqlArray aryParams;

				long nDetailID = AdoFldLong(rsAllocDetails, "ID");
				long nQuantity = (long)AdoFldDouble(rsAllocDetails, "Quantity");

				//TES 7/24/2008 - PLID 30802 - Only do all this if this detail was actually received.
				if(rodi.bReceived) {
					//TES 7/23/2008 - PLID 30802 - Add to our list of updated allocations.
					long nAllocationID = AdoFldLong(rsAllocDetails, "AllocationID");
					bool bMatched = false;
					for(int nAllocation = 0; nAllocation < arUpdatedAllocationIDs.GetSize() && !bMatched; nAllocation++) {
						if(arUpdatedAllocationIDs[nAllocation] == nAllocationID) bMatched = true;
					}
					if(!bMatched) arUpdatedAllocationIDs.Add(nAllocationID);

					if(nQuantity > nQuantityRemaining) {
						//TES 7/23/2008 - PLID 30802 - We need to split up the detail.
						long nNewDetailQuantity = nQuantity - nQuantityRemaining;
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO PatientInvAllocationDetailsT (AllocationID, ProductID, Quantity, Status, Notes, "
							"OrderDetailID) SELECT AllocationID, ProductID, {INT}, {INT}, SUBSTRING(Notes + ' <split due to partially received order>', 1, 255), "
							"OrderDetailID FROM PatientInvAllocationDetailsT WHERE ID = {INT}", nNewDetailQuantity, InvUtils::iadsOrder,
							nDetailID);
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientInvAllocationDetailsT SET Quantity = {INT} WHERE ID = {INT}", 
							nQuantityRemaining, nDetailID);
						nQuantity = nQuantityRemaining;
					}

					//TES 7/23/2008 - PLID 30802 - Now, update the detail to be "Allocated"
					
					//TES 7/23/2008 - PLID 30802 - Check for associated ProductItemIDs.
					long nProductItemID = -1;
					// (j.jones 2010-01-04 14:40) - PLID 36080 - this can now return multiple results
					_RecordsetPtr rsProductItemID = CreateParamRecordset("SELECT ID FROM ProductItemsT WHERE OrderDetailID = {INT} "
						"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null)",
						rodi.nOrderDetailID);

					if(!rsProductItemID->eof) {

						long nRemainingAllocatedQuantity = nQuantity;

						//this should not be possible unless we received more quantity then we entered product items
						if(rsProductItemID->GetRecordCount() < nRemainingAllocatedQuantity) {
							ThrowNxException("InvUtils::UpdateLinkedAllocationDetails (1) failed to update allocation because less serialized products were entered than were received!");
						}

						while(!rsProductItemID->eof && nRemainingAllocatedQuantity > 0) {
							nProductItemID = (AdoFldLong(rsProductItemID, "ID"));

							//TES 7/23/2008 - PLID 30802 - Serialized allocation details can only have a quantity of 1.
							// (j.jones 2010-01-04 14:08) - PLID 36080 - ones marked 'to be ordered' can have a quantity
							// greater than 1, so we'll split them out if we find them

							if(nRemainingAllocatedQuantity == nQuantity) {				
								//we're on the first result, so just update the record, forcing the quantity to 1
								AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientInvAllocationDetailsT SET Quantity = 1.0, "
									"Status = {INT}, ProductItemID = {INT} WHERE ID = {INT}", InvUtils::iadsActive, nProductItemID, nDetailID);
							}
							else {
								//split the detail into a new one
								AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO PatientInvAllocationDetailsT "
									"(AllocationID, ProductID, ProductItemID, Quantity, Status, Notes, OrderDetailID) "
									"SELECT AllocationID, ProductID, {INT}, 1.0, {INT}, Notes, OrderDetailID "
									"FROM PatientInvAllocationDetailsT WHERE ID = {INT}",
									nProductItemID, InvUtils::iadsActive, nDetailID);
							}

							//TES 7/23/2008 - PLID 30802 - Decrease the quantity remaining on this order detail.
							nQuantityRemaining--;

							// (j.jones 2010-01-04 14:58) - PLID 36080 - also decrease the amount remaining on this allocation detail
							nRemainingAllocatedQuantity--;

							rsProductItemID->MoveNext();
						}
					}
					else {
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientInvAllocationDetailsT SET Status = {INT} WHERE ID = {INT}", 
							InvUtils::iadsActive, nDetailID);
						//TES 7/23/2008 - PLID 30802 - Decrease the quantity remaining on this order detail.
						nQuantityRemaining -= nQuantity;
					}
					rsProductItemID->Close();					
				}

				if(!strSqlBatch.IsEmpty()) {
					ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
				}

				rsAllocDetails->MoveNext();
			}
		}

		//TES 7/23/2008 - PLID 30802 - Now, if there's more quantity to resolve, pull those allocation details tied to this order detail's source detail.
		if(rodi.nSourceOrderDetailID != -1) {
			rsAllocDetails = CreateParamRecordset("SELECT PatientInvAllocationDetailsT.ID, "
				"PatientInvAllocationDetailsT.Quantity, PatientInvAllocationDetailsT.AllocationID "
				"FROM PatientInvAllocationDetailsT INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID "
				"= PatientInvAllocationsT.ID LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
				"WHERE PatientInvAllocationDetailsT.OrderDetailID = {INT} AND PatientInvAllocationDetailsT.Status = {INT} "
				"AND PatientInvAllocationsT.Status <> {INT} "
				"ORDER BY COALESCE(AppointmentsT.StartTime, '5000-12-31'), PatientInvAllocationsT.InputDate",
				rodi.nSourceOrderDetailID, InvUtils::iadsOrder, InvUtils::iasDeleted);

			{
				while(!rsAllocDetails->eof && nQuantityRemaining > 0) {

					CString strSqlBatch;
					CNxParamSqlArray aryParams;

					//TES 7/23/2008 - PLID 30802 - Basically the same as before, except we also update to point to the new detail.
					long nDetailID = AdoFldLong(rsAllocDetails, "ID");
					long nQuantity = (long)AdoFldDouble(rsAllocDetails, "Quantity");

					if(nQuantity > nQuantityRemaining) {
						//TES 7/23/2008 - PLID 30802 - We need to split up the detail.
						long nNewDetailQuantity = nQuantity - nQuantityRemaining;
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO PatientInvAllocationDetailsT (AllocationID, ProductID, Quantity, Status, Notes, "
							"OrderDetailID) SELECT AllocationID, ProductID, {INT}, {INT}, SUBSTRING(Notes + ' <split due to partially received order>', 1, 255), "
							"OrderDetailID FROM PatientInvAllocationDetailsT WHERE ID = {INT}", nNewDetailQuantity, InvUtils::iadsOrder,
							nDetailID);
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientInvAllocationDetailsT SET Quantity = {INT} WHERE ID = {INT}", 
							nQuantityRemaining, nDetailID);
						nQuantity = nQuantityRemaining;
					}

					if(!rodi.bReceived) {
						//TES 7/23/2008 - PLID 30802 - This order detail wasn't actually received, so all we actually need to do
						// is update this allocation to point to the new order detail ID.
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientInvAllocationDetailsT SET OrderDetailID = {INT} WHERE ID = {INT}",
							rodi.nOrderDetailID, nDetailID);
					}
					else {
						//TES 7/23/2008 - PLID 30802 - Add to our list of updated allocations.
						long nAllocationID = AdoFldLong(rsAllocDetails, "AllocationID");
						bool bMatched = false;
						for(int nAllocation = 0; nAllocation < arUpdatedAllocationIDs.GetSize() && !bMatched; nAllocation++) {
							if(arUpdatedAllocationIDs[nAllocation] == nAllocationID) bMatched = true;
						}
						if(!bMatched) arUpdatedAllocationIDs.Add(nAllocationID);
						
						//TES 7/23/2008 - PLID 30802 - Check for associated ProductItemIDs.
						long nProductItemID = -1;
						_RecordsetPtr rsProductItemID = CreateParamRecordset("SELECT ID FROM ProductItemsT WHERE OrderDetailID = {INT} "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null)",
							rodi.nOrderDetailID);
						if(!rsProductItemID->eof) {

							long nRemainingAllocatedQuantity = nQuantity;

							//this should not be possible unless we received more quantity then we entered product items
							if(rsProductItemID->GetRecordCount() < nRemainingAllocatedQuantity) {
								ThrowNxException("InvUtils::UpdateLinkedAllocationDetails (2) failed to update allocation because less serialized products were entered than were received!");
							}

							while(!rsProductItemID->eof && nRemainingAllocatedQuantity > 0) {
								nProductItemID = (AdoFldLong(rsProductItemID, "ID"));

								//TES 7/23/2008 - PLID 30802 - Serialized allocation details can only have a quantity of 1.
								// (j.jones 2010-01-04 14:08) - PLID 36080 - ones marked 'to be ordered' can have a quantity
								// greater than 1, so we'll split them out if we find them

								if(nRemainingAllocatedQuantity == nQuantity) {				
									//we're on the first result, so just update the record, forcing the quantity to 1
									AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientInvAllocationDetailsT SET Quantity = 1.0, "
										"Status = {INT}, ProductItemID = {INT}, OrderDetailID = {INT} "
										"WHERE ID = {INT}", InvUtils::iadsActive, nProductItemID, rodi.nOrderDetailID, nDetailID);
								}
								else {
									//split the detail into a new one
									AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO PatientInvAllocationDetailsT "
										"(AllocationID, ProductID, ProductItemID, Quantity, Status, Notes, OrderDetailID) "
										"SELECT AllocationID, ProductID, {INT}, 1.0, {INT}, Notes, {INT} "
										"FROM PatientInvAllocationDetailsT WHERE ID = {INT}",
										nProductItemID, InvUtils::iadsActive, rodi.nOrderDetailID, nDetailID);
								}

								//TES 7/23/2008 - PLID 30802 - Decrease the quantity remaining on this order detail.
								nQuantityRemaining--;

								// (j.jones 2010-01-04 14:58) - PLID 36080 - also decrease the amount remaining on this allocation detail
								nRemainingAllocatedQuantity--;

								rsProductItemID->MoveNext();
							}
						}
						else {
							AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientInvAllocationDetailsT SET Status = {INT}, "
								"OrderDetailID = {INT} WHERE ID = {INT}", 
								InvUtils::iadsActive, rodi.nOrderDetailID, nDetailID);
							//TES 7/23/2008 - PLID 30802 - Decrease the quantity remaining on this order detail.
							nQuantityRemaining -= nQuantity;
						}
						rsProductItemID->Close();
					}

					if(!strSqlBatch.IsEmpty()) {
						ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
					}

					rsAllocDetails->MoveNext();
				}
			}
		}
	}

	CString strSqlBatch;
	CNxParamSqlArray aryParams;

	for(i = 0; i < arReceivedDetails.GetSize(); i++) {
		ReceivedOrderDetailInfo rodi = arReceivedDetails[i];
		//TES 8/19/2008 - PLID 30802 - Now that we're all done, we need to ensure that there are no remaining "To Be Ordered" 
		// allocation details that point to any received order details, since any allocation details that could be marked Active,
		// have been marked Active.
		if(rodi.bReceived) {
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientInvAllocationDetailsT SET OrderDetailID = NULL "
				"WHERE OrderDetailID = {INT} AND Status = {INT}",
				rodi.nOrderDetailID, InvUtils::iadsOrder);
		}
	}

	if(!strSqlBatch.IsEmpty()) {
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
	}
}

// (j.jones 2008-06-02 15:46) - PLID 28076 - AdjustProductItems will now fill an array with IDs
// of products that need adjusted, and actually adjust them off later
// (j.jones 2009-01-15 15:40) - PLID 32749 - moved from InvAdj to InvUtils
// (j.jones 2009-03-09 12:21) - PLID 33096 - added strCreatingProductAdjustmentID
// (j.jones 2009-07-09 17:09) - PLID 32684 - bDeclareNewProductItemID will control whether
// the CProductItemsDlg::Save() function adds a declaration for @nNewProductItemID when generating sql batches
BOOL AdjustProductItems(long nProductID, long nLocationID, double dblQuantity, CArray<long, long> &aryProductItemIDsToRemove,
						bool bSaveDataEntryQuery, CString &strSqlBatch, CString strCreatingProductAdjustmentID,
						BOOL bDeclareNewProductItemID /*= TRUE*/)
{
	try {

		// (j.jones 2006-05-11 10:25) - this only adjusts by Unit Of Usage,
		// only Orders can enter information by UO
		//(e.lally 2008-07-01) PLID 24534 - Add support for adjusting by Unit of Order.

		// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items	
		// (j.jones 2009-01-15 15:48) - PLID 32749 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM ProductItemsT WHERE SerialNum Is Not Null "
			"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"			    WHERE (Status = {INT} OR Status = {INT}) "
			"				AND ProductItemID Is Not Null) "
			"AND Deleted = 0 AND ProductID = {INT} "
			"AND (ProductItemsT.LocationID = {INT} OR ProductItemsT.LocationID Is Null)",
			InvUtils::iadsActive, InvUtils::iadsUsed, nProductID, nLocationID);

		BOOL bHasItemsWSerial = !rs->eof;
		rs->Close();

		// (j.jones 2009-01-15 15:48) - PLID 32749 - parameterized
		rs = CreateParamRecordset("SELECT ID FROM ProductItemsT WHERE ExpDate Is Not Null "
			"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"			    WHERE (Status = {INT} OR Status = {INT}) "
			"				AND ProductItemID Is Not Null) "
			"AND Deleted = 0 AND ProductID = {INT} AND (ProductItemsT.LocationID = {INT} OR ProductItemsT.LocationID Is Null)",
			InvUtils::iadsActive, InvUtils::iadsUsed, nProductID, nLocationID);

		BOOL bHasItemsWExpDate = !rs->eof;
		rs->Close();
		
		BOOL bHasSerial = FALSE, bHasExpDate = FALSE;
		BOOL bUseUU = FALSE, bEnterSingleSerialPerUO = FALSE;
		long nUUConversion = 1;

		// (j.jones 2009-01-15 15:48) - PLID 32749 - parameterized and loaded additional product information
		rs = CreateParamRecordset("SELECT Name, UseUU, Conversion, SerialPerUO, "
			"HasSerialNum, HasExpDate "
			"FROM ServiceT "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"WHERE ServiceT.ID = {INT}", nProductID);
		if(!rs->eof) {

			bHasSerial = AdoFldBool(rs, "HasSerialNum",FALSE);
			bHasExpDate = AdoFldBool(rs, "HasExpDate",FALSE);
			bUseUU = AdoFldBool(rs, "UseUU",FALSE);

			//(e.lally 2008-07-01) PLID 24534 - Even though data is saved for these options, only set them if the product uses UU.
			if(bUseUU != FALSE){
				nUUConversion = AdoFldLong(rs, "Conversion",1);
				bEnterSingleSerialPerUO = AdoFldBool(rs, "SerialPerUO",FALSE);
			}
			else{
				nUUConversion = 1;
				bEnterSingleSerialPerUO = FALSE;
			}
		}
		rs->Close();

		if(!bHasSerial && !bHasExpDate && !bHasItemsWSerial && !bHasItemsWExpDate) {
			//if no applicable products, return true and move on
			return TRUE;
		}

		if((long)dblQuantity != dblQuantity) {
			AfxMessageBox("This product is being tracked by either a serial number or expiration date,\n"
						"and requires that it is adjusted in increments of 1.\n"
						"Please enter a whole number for the quantity to adjust.");
			return FALSE;
		}

		//(e.lally 2008-07-01) PLID 24534 - Check if the change in quantities is a whole number of UO (if using UU-UO and the single serial option is on)
		BOOL bCanUseUOAdjustment = FALSE;
		if(bEnterSingleSerialPerUO != FALSE && bUseUU != FALSE){
			//Make sure our difference in products and conversion rates are whole numbers
			if((long)dblQuantity == dblQuantity && (long)nUUConversion == nUUConversion){
				//if the remainder of ProdItemsQty / Conversion rate is zero, we can use an UO adjustment
				if((long)dblQuantity % (long)nUUConversion == 0){
					bCanUseUOAdjustment = TRUE;
				}
			}
		}

		// (d.thompson 2009-10-21) - PLID 36015 - Create your own dialog, don't
		//	use the shared one anymore.
		//CProductItemsDlg& dlg = GetMainFrame()->GetProductItemsDlg();
		CProductItemsDlg dlg(NULL);

		// (j.jones 2009-01-15 15:53) - PLID 32749 - supported sending the data entry query back to the parent
		dlg.m_bSaveDataEntryQuery = bSaveDataEntryQuery;
		dlg.m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!

		// (j.jones 2009-07-09 17:09) - PLID 32684 - m_bDeclareNewProductItemID will control whether
		// the CProductItemsDlg::Save() function adds a declaration for @nNewProductItemID when generating sql batches
		dlg.m_bDeclareNewProductItemID = bDeclareNewProductItemID;

		if(dblQuantity > 0.0) {
			//if the product does not currently require this info, return - we don't need to add
			if(!bHasSerial && !bHasExpDate)
				return TRUE;
			dlg.m_EntryType = PI_ENTER_DATA;
			//DRT 11/15/2007 - PLID 28008 - All ENTER_DATA types must not allow change of qty
			dlg.m_bDisallowQtyChange = TRUE;
			dlg.m_NewItemCount = (long)dblQuantity;
		}
		else if(dblQuantity < 0.0) {
			dlg.m_EntryType = PI_SELECT_DATA;
			dlg.m_CountOfItemsNeeded = -(long)dblQuantity;

			// (j.jones 2009-01-15 16:30) - PLID 32685 - if aryProductItemIDsToRemove has data in it, preload that data
			for(int i=0;i<aryProductItemIDsToRemove.GetSize();i++) {
				long nProductItemID = aryProductItemIDsToRemove.GetAt(i);
				dlg.m_adwProductItemIDs.Add(nProductItemID);
			}

			bCanUseUOAdjustment = FALSE;
		}
		else{
			return TRUE; //if they are making a adjustment of 0, get out!
		}

		//(e.lally 2008-07-01) PLID 24534 - check if we can use UO as an option, then prompt the user to see if they want to use that option
		if(dblQuantity > 0.0){
			if(bCanUseUOAdjustment != FALSE){
				//we are able to adjust based on Unit of Order, ask the user.
				UINT response = AfxMessageBox("Do you want to enter items based on Unit of Order (UO)?\n"
					"If 'No', the items in the list will be by Unit of Usage (UU).", MB_YESNO);
				if(response == IDNO){
					bCanUseUOAdjustment = FALSE;
				}
			}
			//Check if we're using UU and UO, the SerialPerUO option is on, but our add adjusted amount is not a whole UO
			else if(bUseUU != FALSE && bEnterSingleSerialPerUO != FALSE && (long)dblQuantity % (long)nUUConversion != 0){
				//Prompt to user to see if they want to cancel to update the value to be a full UO.
				UINT response = AfxMessageBox("The adjusted difference in Unit of Usage (UU) does not have a whole number for Unit of Order (UO). If you continue, "
					"the items in the list will be by Unit of Usage.", MB_OKCANCEL);
				if(response == IDCANCEL){
					return FALSE;
				}
			}
		}

		dlg.m_bUseSerial = bHasSerial;
		dlg.m_bUseExpDate = bHasExpDate;
		dlg.m_ProductID = nProductID;
		dlg.m_nLocationID = nLocationID;
		//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
		dlg.m_nOrderDetailID = -1;
		// (j.jones 2009-03-09 12:20) - PLID 33096 - pass in strCreatingProductAdjustmentID
		dlg.m_strCreatingAdjustmentID = strCreatingProductAdjustmentID;
		dlg.m_bDisallowQtyChange = TRUE;
		dlg.m_bAllowQtyGrow = FALSE;
		dlg.m_bIsAdjustment = TRUE;		
		dlg.m_bDisallowLocationChange = TRUE; // (c.haag 2008-06-25 12:12) - PLID 28438 - We only allow adjusting at LocationID
		dlg.m_bUseUU = bUseUU;
		if(bEnterSingleSerialPerUO != FALSE && bCanUseUOAdjustment !=FALSE){
			dlg.m_bSerialPerUO = TRUE;
		}
		else{
			dlg.m_bSerialPerUO = FALSE;
		}
		dlg.m_nConversion = nUUConversion;

		if(IDCANCEL == dlg.DoModal()) {
			//JMJ - 6/24/2003 - Even if a product does not require this information, WE require them to use these items up first.
			//if they have a problem with that, they will have to adjust off those items and re-add them.
			//That sounds cocky, but Meikin and I discussed that this is the best way. The bill follows the same logic.
			AfxMessageBox("The quantity of this product cannot be changed without appropriately modifying this required information.\n"
				"You will not be permitted to save this adjustment without updating this information.");
			return FALSE;
		}
		else {
			//we only need to do this if it is PI_SELECT_DATA
			if(dlg.m_EntryType == PI_SELECT_DATA) {

				// (j.jones 2009-01-15 16:34) - PLID 32685 - now clear our aryProductItemIDsToRemove list and rebuild it
				// with the selections from the dialog
				aryProductItemIDsToRemove.RemoveAll();

				for(int i=0;i<dlg.m_adwProductItemIDs.GetSize();i++) {								
					long nProductItemID = (long)dlg.m_adwProductItemIDs.GetAt(i);
					//DRT 10/2/03 - PLID 9467 - Don't remove the row, just mark it deleted now.
					// (j.jones 2008-06-02 15:47) - PLID 28076 - we no longer delete in this function,
					// instead fill our array with the IDs that need removed
					aryProductItemIDsToRemove.Add(nProductItemID);
				}
			}
		}

		// (j.jones 2009-01-15 15:53) - PLID 32749 - supported sending the data entry query back to the parent
		if(bSaveDataEntryQuery) {
			strSqlBatch += "\r\n";
			strSqlBatch += dlg.m_strSavedDataEntryQuery;
		}

		return TRUE;

	}NxCatchAll("Error in AdjustProductItems");

	AfxMessageBox("The quantity of this product cannot be changed without appropriately modifying this required information.\n"
				"You will not be permitted to save this adjustment without updating this information.");
	return FALSE;
}

// (z.manning 2010-06-23 12:10) - PLID 39311 - Moving some of the common code to create products in the db here from
// InvNew dialog.
// (j.jones 2015-03-03 14:40) - PLID 65111 - categories are no longer part of this function
void AddCreateProductSqlToBatch(IN OUT CString &strSqlBatch, const CString strProductID, CString strName, BOOL bBillable, BOOL bTaxable1, BOOL bTaxable2, TrackStatus eTrackStatus, long nUserID)
{
	AddStatementToSqlBatch(strSqlBatch, 
		"INSERT INTO ServiceT (ID, Name, Taxable1, Taxable2) SELECT %s, \'%s\', %i, %i"
		, strProductID, _Q(strName), bTaxable1 ? 1 : 0, bTaxable2 ? 1 : 0);
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ProductT (ID) SELECT %s", strProductID);

	AddStatementToSqlBatch(strSqlBatch, 
		"INSERT INTO ProductResponsibilityT (ProductID, UserID, LocationID) \r\n"
		"SELECT %s, %i, ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1"
		, strProductID, nUserID, GetCurrentLocation());

	//We decided to make the Billable and Trackable statuses apply to all (managed) locations in the new item dlg
	//and users can change it after the save.
	AddStatementToSqlBatch(strSqlBatch, 
		"INSERT INTO ProductLocationInfoT (ProductID, LocationID, TrackableStatus, Billable) \r\n"
		"SELECT %s, ID, %li, %li FROM LocationsT WHERE Managed = 1 AND TypeID = 1"
		, strProductID, eTrackStatus, bBillable ? 1 : 0);

	// (j.gruber 2012-12-04 11:44) - PLID 48655
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT %s, ID FROM LocationsT WHERE Managed = 1 "
						, strProductID);	
}

// (z.manning 2010-09-22 16:22) - PLID 40619
// (b.eyers 2016-03-14) - PLID 68590 - changed roundup to rounduprule which is now an long since there are more than two options for rounding now
// (j.jones 2016-05-17 11:13) - PLID-68615 - changed the price to a currency
BOOL CalculateMarkupPrice(IN COleCurrency cyCost, IN CString strFormula, OUT COleCurrency &cyNewPrice, long nRoundUpRule)
{
	strFormula.Replace(MARKUP_COST_OPERAND, FormatCurrencyForSql(cyCost));
	CNxExpression expressionMarkup(strFormula);
	double dblPrice = expressionMarkup.Evaluate();
	//r.wilson 3/9/2012 PLID  46664 -> If round up bit it true then round up to next dollar
	// (b.eyers 2016-03-14) - PLID 68590 - RoundUp is no longer a bool, there are more than two options for it now
	if (nRoundUpRule == RoundUpNearestDollar) {
		dblPrice = ceil(dblPrice);
	}
	else if (nRoundUpRule == RoundUpToNine) {
		//need to round up the single dollar to 9, round up first though so 149.30->150->159
		CString strPrice;
		strPrice.Format("%f", ceil(dblPrice));
		long nDecimal = strPrice.Find('.', 0);
		strPrice.Format("%s9", strPrice.Left(nDecimal - 1));
		dblPrice = atof(strPrice);
	}
	//and if it is NoRoundUpRule then do nothing

	// (j.jones 2016-05-17 11:13) - PLID-68615 - we now return a rounded currency
	long nPrice = (long)dblPrice;
	cyNewPrice = COleCurrency(nPrice, (long)((dblPrice - (double)nPrice) * 10000));
	RoundCurrency(cyNewPrice);

	if(expressionMarkup.GetError().IsEmpty()) {
		return TRUE;
	}
	else {
		cyNewPrice = COleCurrency(0, 0);
		return FALSE;
	}
}

// (z.manning 2010-10-27 14:42) - PLID 40619 - Moved code here out of InvEditDlg
BOOL UpdateSurgeriesForPriceChange(CWnd *pwndParent, IN OUT CParamSqlBatch &sqlBatch, const long nProductID, const COleCurrency cyNewPrice, BOOL bSilent /* = FALSE */)
{
	long nUpdateSurgeryPrices = GetRemotePropertyInt("UpdateSurgeryPrices",1,0,"<None>",TRUE);
	long nUpdateSurgeryPriceWhen = GetRemotePropertyInt("UpdateSurgeryPriceWhen",1,0,"<None>",TRUE);

	//3 means do nothing, so skip the check
	if(nUpdateSurgeryPrices != 3)
	{
		//2 means to always update, so don't prompt. 1 means to prompt.
		if(nUpdateSurgeryPrices == 2 || (nUpdateSurgeryPrices == 1 && (bSilent || IDYES==pwndParent->MessageBox("There are surgeries that use this product but list a different price for it.\n"
			"Do you wish to update these surgeries to match this new price?","Practice",MB_ICONQUESTION|MB_YESNO))))
		{
			if(nUpdateSurgeryPriceWhen == 1) {
				sqlBatch.Add("UPDATE SurgeryDetailsT SET Amount = {VT_CY} WHERE ServiceID = {INT}"
					, _variant_t(cyNewPrice), nProductID);
			}
			else {
				sqlBatch.Add("UPDATE SurgeryDetailsT SET Amount = {VT_CY} WHERE ServiceID = {INT} AND ServiceID IN (SELECT ID FROM ServiceT WHERE Price = Amount)"
					, _variant_t(cyNewPrice), nProductID);
			}
			return TRUE;
		}
	}
	return FALSE;
}

// (z.manning 2010-10-27 14:42) - PLID 40619 - Moved code here out of InvEditDlg
BOOL UpdatePrefCardsForPriceChange(CWnd *pwndParent, IN OUT CParamSqlBatch &sqlBatch, const long nProductID, const COleCurrency cyNewPrice, BOOL bSilent /* = FALSE */)
{
	// (j.jones 2009-08-26 08:40) - PLID 35124 - do the same for preference cards
	if(IsSurgeryCenter(false))
	{
		long nUpdatePreferenceCardPrices = GetRemotePropertyInt("UpdatePreferenceCardPrices",GetRemotePropertyInt("UpdateSurgeryPrices",1,0,"<None>",TRUE),0,"<None>",TRUE);
		long nUpdatePreferenceCardPriceWhen = GetRemotePropertyInt("UpdatePreferenceCardPriceWhen",GetRemotePropertyInt("UpdateSurgeryPriceWhen",1,0,"<None>",TRUE),0,"<None>",TRUE);

		//3 means do nothing, so skip the check
		if(nUpdatePreferenceCardPrices != 3)
		{
			//2 means to always update, so don't prompt. 1 means to prompt.
			if(nUpdatePreferenceCardPrices == 2 || (nUpdatePreferenceCardPrices == 1 && (bSilent || IDYES==pwndParent->MessageBox("There are preference cards that use this product but list a different price for it.\n"
				"Do you wish to update these preference cards to match this new price?","Practice",MB_ICONQUESTION|MB_YESNO))))
			{
				if(nUpdatePreferenceCardPriceWhen == 1) {
					sqlBatch.Add("UPDATE PreferenceCardDetailsT SET Amount = {VT_CY} WHERE ServiceID = {INT}"
						, _variant_t(cyNewPrice), nProductID);
				}
				else {
					sqlBatch.Add("UPDATE PreferenceCardDetailsT SET Amount = {VT_CY} WHERE ServiceID = {INT} AND ServiceID IN (SELECT ID FROM ServiceT WHERE Price = Amount)"
						, _variant_t(cyNewPrice), nProductID);
				}

				return TRUE;
			}
		}
	}
	return FALSE;
}

	// (j.gruber 2012-10-29 16:02) - PLID 53416 - created for
	// (j.gruber 2012-10-29 15:11) - PLID 53241 - moved here
CString GetServiceLocationShopFeeAuditString(long nServiceID, CArray<long, long> *paryLocations)
{
	CString strAudit = "";

	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT LocationsT.Name, ServiceLocationInfoT.ShopFee "
		" FROM ServiceLocationInfoT INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID "
		" WHERE ServiceLocationInfoT.ServiceID = {INT} AND ServiceLocationInfoT.LocationID IN ({INTARRAY})"
		, nServiceID, *paryLocations);
	while (!rs->eof) {
		CString strLocationName = AdoFldString(rs->Fields, "Name");
		COleCurrency cyShopFee = AdoFldCurrency(rs->Fields, "ShopFee");
	
		strAudit += strLocationName + ": " + FormatCurrencyForInterface(cyShopFee) + ", ";

		rs->MoveNext();
	}

	//take off the last comma
	strAudit = strAudit.Left(strAudit.GetLength() - 2);

	return strAudit;
		

}

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
bool HasOrderDetailReceivedProductItems(long nOrderDetailID)
{
	if (nOrderDetailID == -1)
	{
		//order hasn't been saved yet, so we know it could not possibly have been received
		return false;
	}

	std::vector<long> aryOrderDetailIDs;
	aryOrderDetailIDs.push_back(nOrderDetailID);
	return HasOrderDetailReceivedProductItems(aryOrderDetailIDs);
}

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
/// <param name="aryOrderDetailIDs">Vector of OrderDetailsT.IDs of the ordered products being received.</param>
/// <returns>True if order detail has already been received, meaning the receiving process has to be aborted.
/// False if the products can be safely marked received.</returns>
bool HasOrderDetailReceivedProductItems(std::vector<long> aryOrderDetailIDs)
{
	if (aryOrderDetailIDs.size() == 0)
	{
		//maybe the order hasn't been saved yet, either way nothing to check
		return false;
	}

	long nCount = 0;
	_RecordsetPtr rs = CreateParamRecordset("SELECT Count(*) AS CountReceived FROM ProductItemsT WHERE Deleted = 0 AND OrderDetailID IN ({INTVECTOR})", aryOrderDetailIDs);
	if (!rs->eof) {
		nCount = VarLong(rs->Fields->Item["CountReceived"]->Value, 0);
	}

	if (nCount > 0) {
		//We can't receive this order detail because items have already been received.
		//It does not matter if only some were received, if so a new order detail would
		//have been made. You cannot receive an order detail twice.
		return true;
	}
	else {
		//we can safely receive these items
		return false;
	}
}

}//end namespace


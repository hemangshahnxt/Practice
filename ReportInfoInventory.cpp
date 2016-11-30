////////////////
// DRT 8/6/03 - GetSqlInventory() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "InvUtils.h"

// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
// (j.dinatale 2011-11-07 09:52) - PLID 46226 - need to exclude voiding and original line items of financial corrections
// (s.dhole 2012-04-12 17:35) - PLID 49672 Exclude Off the shelf from Inventory report  
// (j.jones 2014-11-10 13:33) - PLID 64105 - reworked the IN clauses for GlassesOrderServiceT and BilledCaseHistoriesT such that they are now left joining
// (j.jones 2014-11-10 13:40) - PLID 64105 - moved the "Items Charged" subquery into one spot
// (r.goldschmidt 2015-06-08 15:56) - PLID 66234 - need to add some parentheses to correct a where clause
static CString strItemsChargedSubquery = FormatString("SELECT ServiceID, "
	"SUM(CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity, 0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity, 0) END) AS ChargeQuantity, LineItemT.LocationID  "
	"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
	"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
	"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
	"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
	"LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
	"	INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
	"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
	"	WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li "
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

	"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Type = 10 "
	"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
	"AND BilledCaseHistoriesQ.BillID Is Null "
	"AND GlassesOrderServiceQ.ID Is Null "
	"GROUP BY ServiceID, LineItemT.LocationID", InvUtils::iadsUsed, InvUtils::iasCompleted);

CString CReportInfo::GetSqlInventory(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;
	COleDateTime dtNext;
	COleDateTimeSpan  OneDay(1,0,0,0);
	switch (nID) {
	

case 2:
		//Product Price List
		/*	Version History	
			DRT 8/6/03 - Added CategoryID field for extended filter.
			// (z.manning, 06/05/2007) - PLID 26145 - We now hide inactive products.
			// (j.jones 2011-06-24 15:54) - PLID 15545 - added LastCost
		*/
		return _T("SELECT CategoriesT.Name AS CategoryName, "
		"ServiceT.Name AS ProductName, "
		"ProductT.UnitDesc, "
		"ServiceT.Price AS SalesPrice, MultiSupplierT.SupplierID AS SupplierID, PersonT.Company, "
		"ServiceT.Taxable1, ServiceT.Taxable2, CategoriesT.ID AS CategoryID, "
		"MultiSupplierT.Catalog, ProductT.LastCostPerUU AS LastCost "
		"FROM CategoriesT RIGHT JOIN ServiceT ON CategoriesT.ID = ServiceT.Category "
		"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
		"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
		"WHERE ServiceT.Active = 1 "
		);
		break;
	

case 3: 
		//Physical Inventory Tally Sheet
		/*	Version History
			DRT 3/7/03 - Added a filter that no longer shows Inactive Inventory Items in the tally sheet
			DRT 11/8/2004 - PLID 14645 - Fixed a bug that was forcing inventory items to have categories
				to show up in this report.
			TES 1/26/2005 - PLID 15363 - Added Barcode.
			// (j.jones 2007-11-28 11:25) - PLID 28196 - completed Allocations now decrement from Inventory
			// (j.jones 2007-12-18 09:03) - PLID 27988 - billed allocations do not decrement from inventory
			// (j.jones 2008-02-29 12:57) - PLID 29132 - case histories now don't decrement from Inventory if
			// they have a linked allocation, unless they have more of the product than the allocation
			// (this also means this report has to use the product items count if a product is serialized)
			//(e.lally 2008-10-01) PLID 31539 - Added support to handle filtering on multiple locations.
				//Just like a previous item, I realized that the ELSE query already does all the
				//joins and grouping on location that this would have required.
		*/
		{
			long nLocationID = this->nLocation;
			CString strLocationFilter;
			if(nLocationID > 0){
				strLocationFilter.Format("ProductLocationInfoT.LocationID = %li AND ", nLocationID);
			}
			else if(nLocationID == -2){
				//This will always be false
				strLocationFilter = "ProductLocationInfoT.LocationID IS NULL AND ";
			}
			else if(nLocationID == -3){
				strLocationFilter = "ProductLocationInfoT.LocationID IN(";
				CString strPart;
				for(int i=0; i < this->m_dwLocations.GetSize(); i++) {
					strPart.Format("%li, ", (long)this->m_dwLocations.GetAt(i));
					strLocationFilter += strPart;
				}
				strLocationFilter = strLocationFilter.Left(strLocationFilter.GetLength()-2) + ") AND ";
			}

			CString strSQL;
			strSQL.Format("SELECT ServiceT.ID, LocationsT.Name AS LocationName, CategoriesT.Name AS CategoryName,   "
				"ServiceT.Name AS ProductName, ProductT.UnitDesc,   "
				"PersonT.Company AS SupplierName,   "
				"ProductLocationInfoT.ReorderPoint,   "
				"ProductLocationInfoT.ReorderQuantity,   "

				//use the standard calculation only if no serial options are enabled,
				//otherwise look explicitly at product items

				"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
				"THEN Coalesce(ProductItemsActualQ.Actual, 0) "
				"ELSE "
				"(CASE WHEN (ReceivedSubQ.QuantityOrdered) IS NULL   "
				"THEN 0 ELSE ReceivedSubQ.QuantityOrdered END)   "
				"- (CASE WHEN (ChargeSubQ.ChargeQuantity) IS NULL   "
				"THEN 0 ELSE ChargeSubQ.ChargeQuantity END)   "
				"- (CASE WHEN (CaseHistSubQ.CaseHistQuantity) IS NULL   "
				"THEN 0 ELSE CaseHistSubQ.CaseHistQuantity END)   "
				"+ (CASE WHEN (AdjSubQ.AdjQuantity) IS NULL   "
				"THEN 0 ELSE AdjSubQ.AdjQuantity END) "
				"- (CASE WHEN (CompletedAllocationsQ.UsedAllocationQty) IS NULL   "
				"THEN 0 ELSE CompletedAllocationsQ.UsedAllocationQty END) "
				"+ (CASE WHEN (TransferToSubQ.TransferToQty) IS NULL   "
				"THEN 0 ELSE TransferToSubQ.TransferToQty END) "
				"- (CASE WHEN (TransferFromSubQ.TransferFromQty) IS NULL   "
				"THEN 0 ELSE TransferFromSubQ.TransferFromQty END) "
				"END AS NumInStock, "

				"CategoriesT.ID AS CategoryID, PersonT.ID AS SupplierID, "
				"ServiceT.Barcode "
				"FROM  "
				"ServiceT  "
				"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
				"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
				"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
				"LEFT OUTER JOIN "

				"/*Items Charged */ "
				// (j.jones 2014-11-10 13:40) - PLID 64105 - moved the "Items Charged" subquery into one spot				
    			"(" + strItemsChargedSubquery + ") ChargeSubQ ON "
				"ProductT.ID = ChargeSubQ.ServiceID AND ProductLocationInfoT.LocationID = ChargeSubQ.LocationID LEFT OUTER JOIN  "
				"/* Items received */ "
 				"(SELECT ProductID, SUM(QuantityOrdered)   "
       				"AS QuantityOrdered, OrderT.LocationID "
  				"FROM OrderDetailsT "
				"INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
  				"WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived IS NOT NULL) "
  				"GROUP BY ProductID, OrderT.LocationID "
				") ReceivedSubQ ON   "
				"ProductT.ID = ReceivedSubQ.ProductID AND ProductLocationInfoT.LocationID = ReceivedSubQ.LocationID LEFT OUTER JOIN  "
				"/* Items Adjusted */  "
 				"(SELECT ProductID, SUM(Quantity) AS AdjQuantity, LocationID "
  				"FROM ProductAdjustmentsT  "
  				"GROUP BY ProductID, LocationID) AdjSubQ ON   "
				"ProductT.ID = AdjSubQ.ProductID AND ProductLocationInfoT.LocationID = AdjSubQ.LocationID "

				/* if there is a linked allocation with the same product, do not use the case history
				amount unless it is greater, in which case we use the overage*/
				"/* Items On Completed Case Histories */  "
				"LEFT OUTER JOIN "
				"(SELECT Sum(CaseHistoryQty) AS CaseHistQuantity, ItemID, LocationID FROM ("
						"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(UsedAllocationQty,0))THEN SUM(Quantity) - SUM(Coalesce(UsedAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, CaseHistoryT.LocationID "
						"FROM CaseHistoryDetailsT INNER JOIN "
						"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
						"LEFT JOIN (SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, CaseHistoryID "
							"FROM PatientInvAllocationsT "
							"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
							"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
							"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " "
							"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " "
							"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
						"WHERE ItemType = -1 AND CompletedDate Is Not Null "
						"GROUP BY ItemID, CaseHistoryT.ID, CaseHistoryT.LocationID "
					") AS CaseHistoryIndivSubQ GROUP BY ItemID, LocationID "
				") AS CaseHistSubQ ON ProductT.ID = CaseHistSubQ.ItemID AND ProductLocationInfoT.LocationID = CaseHistSubQ.LocationID "

				"/*Items Transferred To This Location*/ "
				"LEFT OUTER JOIN "
				"(SELECT SUM(Amount) AS TransferToQty, ProductID, DestLocationID "
					"FROM ProductLocationTransfersT "
					"GROUP BY ProductID, DestLocationID "
				") AS TransferToSubQ ON ProductT.ID = TransferToSubQ.ProductID AND ProductLocationInfoT.LocationID = TransferToSubQ.DestLocationID "
				"/*Items Transferred From This Location*/ "
				"LEFT OUTER JOIN "
				"(SELECT SUM(Amount) AS TransferFromQty, ProductID, SourceLocationID "
					"FROM ProductLocationTransfersT "
					"GROUP BY ProductID, SourceLocationID "
				") AS TransferFromSubQ ON ProductT.ID = TransferFromSubQ.ProductID AND ProductLocationInfoT.LocationID = TransferFromSubQ.SourceLocationID "
				"LEFT OUTER JOIN ( "
					"SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, LocationID "
					"FROM PatientInvAllocationsT "
					"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
					"WHERE PatientInvAllocationsT.Status = %li "
					"AND PatientInvAllocationDetailsT.Status = %li "
					"GROUP BY ProductID, LocationID "
				") AS CompletedAllocationsQ ON ProductT.ID = CompletedAllocationsQ.ProductID AND CompletedAllocationsQ.LocationID = ProductLocationInfoT.LocationID "

				// (j.jones 2014-11-10 14:31) - PLID 64105 - converted to use joins instead of a NOT IN clause
				// (r.goldschmidt 2015-06-08 16:47) - PLID 66234 - fixing adjusted where clause conditional for allocation status
				"/*Product Items that are not used */"
				"LEFT JOIN ( "
				"	SELECT Count(ProductItemsT.ID) AS Actual, ProductItemsT.ProductID, ProductItemsT.LocationID "
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
				"	GROUP BY ProductItemsT.ProductID, ProductItemsT.LocationID "
				") AS ProductItemsActualQ ON ProductT.ID = ProductItemsActualQ.ProductID AND ProductItemsActualQ.LocationID = ProductLocationInfoT.LocationID "

				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"LEFT JOIN PersonT ON PersonT.ID = MultiSupplierT.SupplierID "
				"INNER JOIN LocationsT ON ProductLocationInfoT.LocationID = LocationsT.ID "
				"WHERE %s ServiceT.Active = 1 AND ProductLocationInfoT.TrackableStatus = 2 ",
				InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iasDeleted, InvUtils::iadsUsed,
				strLocationFilter);

				return _T(strSQL);
		}
		break;

	case 18:
		//Inventory On Order
		/*	Version History
			DRT 7/23/03 - Added an OrderDetailCost column to the report.
			// (j.jones 2008-06-19 16:32) - PLID 30446 - added PercentOff and Discount
			// (d.thompson 2009-03-02 13:51) - PLID 32344 - Added PurchaseOrderNum, 3 confirmation fields, and
				the order method.
		*/
		return _T("SELECT "
			"OrderT.ID, PersonT.Company AS SupplierName, OrderT.TrackingID, OrderDetailsT.QuantityOrdered,  "
			"OrderT.Notes, ServiceT.Name AS ProductName,  OrderT.LocationID as LocID, OrderT.Date AS DateOrdered, "
			"OrderDetailsT.DateReceived AS DateReceived, PersonT.ID AS SupplierID, LocationsT.Name, "			
			"Round(Convert(money,(Round(Convert(money,((((OrderDetailsT.ExtraCost + ((CASE WHEN OrderDetailsT.UseUU = 1 THEN Convert(int,OrderDetailsT.QuantityOrdered / Convert(float,OrderDetailsT.Conversion)) ELSE OrderDetailsT.QuantityOrdered END) * OrderDetailsT.Amount)) * ((100-Convert(float,PercentOff))/100)) - Discount))),2) * OrderT.Tax)),2) AS OrderDetailCost, "
			"OrderDetailsT.PercentOff, OrderDetailsT.Discount, OrderT.PurchaseOrderNum, OrderT.VendorConfirmed, OrderT.ConfirmationDate, "
			"OrderT.ConfirmationNumber, OrderMethodsT.Method AS OrderMethod "
			"FROM OrderT LEFT JOIN "
			"OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID LEFT JOIN LocationsT ON OrderT.LocationID = LocationsT.ID INNER JOIN "
			"ServiceT ON OrderDetailsT.ProductID = ServiceT.ID INNER JOIN "
			"ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN "
			"PersonT ON OrderT.Supplier = PersonT.ID "
			"LEFT JOIN OrderMethodsT ON OrderT.OrderMethodID = OrderMethodsT.ID "
			"WHERE (OrderT.Deleted = 0) AND (OrderDetailsT.Deleted = 0)");
		break;

case 180:
		//Inventory Items to be Ordered
		/*	Revision History
			DRT 4/23/03 - Made a pretty big change to the query that is not location-filtered.  Here was the problem:
					It was making a bunch of subqueries for the various items, then joining them all.  So, for example, if
					you had 3 rows of charged sums (3 locs for 1 item), and then 1 row of adjusted sums (1 location for 1 item), 
					it would join those together, but only on ProductID, meaning you could join location 3 (from chargesubq) with
					location 1 (from adjsubq) 3 times!  This obviously is quite bad.  I took the code from InvValues query and 
					finagled it a bit, it uses a much more sensible union query to calculate the number in stock for each location.
					I kept the exposed fields exactly the same, so no change to the .ttx file is needed.
					TODO:  I did not change the location-filtered query.  It works fine, because it doesn't fall into the same
					trap of joining on locations (since it's filtered on 1 location).  So eventually, maybe, 
					we should fix that query to use the union approach as well and just do a (much easier) filter on the LocationID.
					At least so noone accidentally copies it and removes the filter to something else.
			DRT 3/7/03 - Changed the filter from OnHand < ReorderQty to OnHand <= ReorderQty (from PLID #4820)
			// (j.jones 2007-11-28 11:52) - PLID 28196 - completed Allocations now decrement from Inventory
			// (j.jones 2007-12-18 09:03) - PLID 27988 - billed allocations do not decrement from inventory
			DRT 2/7/2008 - PLID 28854 - Added an 'available' field.  Note that this is just total minus "allocated", it does not factor
				case histories at this point in time, we are still deciding how to handle that.
			// (j.jones 2008-02-08 09:39) - PLID 28865 - supported ConsignmentReorderQuantity and ConsignmentReorderPoint,
			// which means we needed to split the results into what needs ordered for Purchased Inventory versus what needs
			// ordered for Consignment inventory, which sadly means a union query. (ConsignmentReorderQuantity was later removed)
			// I also removed the location filter query, because the "no location filter" query has long since been written
			// correctly such that it can use our standard reports filtering
			// (j.jones 2008-02-14 13:38) - PLID 28864 - we now have a preference to order items when the "actual" amount on hand
			// hits the reorder point, or when the "available" amount hits that point, and it is now supported in this report
			// (j.jones 2008-02-19 12:17) - PLID 28981 - consignment items now reorder when < the consignment reorder point,
			// whereas purchased inventory items continue to reorder when <= the reorder point
			// (j.jones 2008-02-21 09:43) - PLID 28852 - ConsignmentReorderQuantity was removed
			// (j.jones 2008-02-29 15:24) - PLID 29132 - case histories now don't decrement from Inventory if
			// they have a linked allocation, unless they have more of the product than the allocation
			// (j.jones 2008-03-07 14:55) - PLID 29235 - changed the 'avail' calculations to not be case or allocation-only
			// based on the license, instead it calculates with the same logic as the 'actual', except that case history totals
			// that aren't accounted for by an allocation will always decrement from General stock, never Consignment.
			// This also means we no longer care what the license is.
		*/

		//****Be sure to make any changes to both halves of this query, as it is a UNION now

		 {

			 // (j.jones 2008-02-14 13:39) - PLID 28864 - grab the ordering preference
			 // (j.jones 2008-02-19 12:18) - PLID 28981 - now we have separate logic
			 // for Consignment and Purchased Inventory
			long nOrderByOnHandAmount = GetRemotePropertyInt("InvItem_OrderByOnHandAmount", 0, 0, "<None>", true);
			CString strOrderWhere;
			if(nOrderByOnHandAmount == 0) {
				//the preference says to compare the "Actual" amount to the reorder point
				//needs to compare <= the ReorderPoint when Purchased Inventory
				//needs to compare < the ReorderPoint when Consignment
				strOrderWhere = "((ProductTypeID = 0 AND NumInStock <= ReOrderPoint) OR (ProductTypeID = 1 AND NumInStock < ReOrderPoint))";
			}
			else {
				//the preference says to compare the "Available" amount to the reorder point
				//needs to compare <= the ReorderPoint when Purchased Inventory
				//needs to compare < the ReorderPoint when Consignment
				strOrderWhere = "((ProductTypeID = 0 AND AvailableStock <= ReOrderPoint) OR (ProductTypeID = 1 AND AvailableStock < ReOrderPoint))";
			}

			CString strSql;
			strSql.Format("SELECT ProductTypeID, ProductTypeName, CategoryName, ProductName, UnitDesc, CASE WHEN Company Is Null THEN '' ELSE Company END AS SupplierName, ReorderPoint, ReorderQuantity, NumInStock,   "
			"InvListSubQ.LocID AS LocID, InvListSubQ.Location, InvListSubQ.SupID AS SupplierID, InvListSubQ.QuantityOnOrder, AvailableStock "
			" FROM ("
			//Purchased Inventory
			"SELECT 0 AS ProductTypeID, 'Purchased Inventory' AS ProductTypeName, PersonT.Company, PersonT.ID AS SupID, CategoriesT.Name AS CategoryName,     "
			"ServiceT.ID AS ProductID, ServiceT.Name AS ProductName, ProductT.UnitDesc,     "
			"ProductLocationInfoT.ReorderPoint,     "
			"ProductLocationInfoT.ReorderQuantity,     "

			//use the standard calculation only if no serial options are enabled,
			//otherwise look explicitly at product items

			/* the UnRelievedCaseHistoryQty needs subtracted from AllProductItemsAvailQ.Avail,
			if that field is used, otherwise the regular calculations such as
			UnRelievedCaseHistoryQty and OnHoldAllocationQty send back appropriate values*/

			"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"THEN Coalesce(GeneralProductsActualQ.GeneralActual, 0) "
			"ELSE "
			"(CASE WHEN (ReceivedSubQ.QuantityOrdered) IS NULL   "
			"THEN 0 ELSE ReceivedSubQ.QuantityOrdered END)   "
			"- (CASE WHEN (ChargeSubQ.ChargeQuantity) IS NULL   "
			"THEN 0 ELSE ChargeSubQ.ChargeQuantity END)   "
			"- (CASE WHEN (CaseHistSubQ.CaseHistQuantity) IS NULL   "
			"THEN 0 ELSE CaseHistSubQ.CaseHistQuantity END)   "
			"+ (CASE WHEN (AdjSubQ.AdjQuantity) IS NULL   "
			"THEN 0 ELSE AdjSubQ.AdjQuantity END) "
			"- (CASE WHEN (CompletedAllocationsQ.UsedAllocationQty) IS NULL   "
			"THEN 0 ELSE CompletedAllocationsQ.UsedAllocationQty END) "
			"+ (CASE WHEN (TransferToSubQ.TransferToQty) IS NULL   "
			"THEN 0 ELSE TransferToSubQ.TransferToQty END) "
			"- (CASE WHEN (TransferFromSubQ.TransferFromQty) IS NULL   "
			"THEN 0 ELSE TransferFromSubQ.TransferFromQty END) "
			"END "
			"AS NumInStock,  "
						
			"CASE WHEN GenOnOrderSubQ.QuantityOrdered Is Null THEN 0 ELSE GenOnOrderSubQ.QuantityOrdered END AS QuantityOnOrder, "

			"LocationsT.ID AS LocID, LocationsT.Name AS Location, "

			//use the standard calculation only if no serial options are enabled,
			//otherwise look explicitly at product items

			/* the UnRelievedCaseHistoryQty needs subtracted from AllProductItemsAvailQ.Avail,
			if that field is used, otherwise the regular calculations such as
			UnRelievedCaseHistoryQty and OnHoldAllocationQty send back appropriate values*/
			"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"THEN Coalesce(GeneralProductsAvailQ.GeneralAvail, 0) "
			"	- Coalesce(UnRelievedCaseHistSubQ.UnRelievedCaseHistQuantity,0) "
			"ELSE "
			"(CASE WHEN (ReceivedSubQ.QuantityOrdered) IS NULL   "
			"THEN 0 ELSE ReceivedSubQ.QuantityOrdered END)   "
			"- (CASE WHEN (ChargeSubQ.ChargeQuantity) IS NULL   "
			"THEN 0 ELSE ChargeSubQ.ChargeQuantity END)   "
			"- (CASE WHEN (CaseHistSubQ.CaseHistQuantity) IS NULL   "
			"THEN 0 ELSE CaseHistSubQ.CaseHistQuantity END)   "
			"+ (CASE WHEN (AdjSubQ.AdjQuantity) IS NULL   "
			"THEN 0 ELSE AdjSubQ.AdjQuantity END) "
			"- (CASE WHEN (CompletedAllocationsQ.UsedAllocationQty) IS NULL   "
			"THEN 0 ELSE CompletedAllocationsQ.UsedAllocationQty END) "
			"+ (CASE WHEN (TransferToSubQ.TransferToQty) IS NULL   "
			"THEN 0 ELSE TransferToSubQ.TransferToQty END) "
			"- (CASE WHEN (TransferFromSubQ.TransferFromQty) IS NULL   "
			"THEN 0 ELSE TransferFromSubQ.TransferFromQty END) "
			"- (CASE WHEN UnRelievedCaseHistSubQ.UnRelievedCaseHistQuantity IS NULL "
			"THEN 0 ELSE UnRelievedCaseHistSubQ.UnRelievedCaseHistQuantity END) "
			"- (CASE WHEN OnHoldAllocationsQ.OnHoldAllocationQty IS NULL "
			"THEN 0 ELSE OnHoldAllocationsQ.OnHoldAllocationQty END) "
			"END "
			"AS AvailableStock "

			"FROM ServiceT LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"INNER JOIN ProductLocationInfoT ON ServiceT.ID = ProductLocationInfoT.ProductID "
			"INNER JOIN LocationsT ON ProductLocationInfoT.LocationID = LocationsT.ID "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT OUTER JOIN    "
			"   "
			// (j.jones 2014-11-10 13:40) - PLID 64105 - moved the "Items Charged" subquery into one spot
			"/*Items Charged */   "
			" (" + strItemsChargedSubquery + ") ChargeSubQ ON     "
			"ProductT.ID = ChargeSubQ.ServiceID AND ProductLocationInfoT.LocationID = ChargeSubQ.LocationID LEFT OUTER JOIN    "
			"   "
			"/* Items received */   "
			" (SELECT OrderDetailsT.ProductID, SUM(QuantityOrdered) AS QuantityOrdered, OrderT.LocationID, LocationsT.Name AS Location "
			"  FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID LEFT JOIN LocationsT ON OrderT.LocationID = LocationsT.ID  "
			"  WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived Is Not Null) "
			"  GROUP BY OrderDetailsT.ProductID, LocationID, LocationsT.Name "
			"  ) ReceivedSubQ ON     "
			"ProductT.ID = ReceivedSubQ.ProductID AND ProductLocationInfoT.LocationID = ReceivedSubQ.LocationID LEFT OUTER JOIN    "
			"   "
			"/* Items Adjusted */    "
			" (SELECT ProductAdjustmentsT.ProductID, SUM(Quantity) AS AdjQuantity, LocationID, LocationsT.Name AS Location "
			"  FROM ProductAdjustmentsT LEFT JOIN LocationsT ON ProductAdjustmentsT.LocationID = LocationsT.ID "
			"  GROUP BY ProductAdjustmentsT.ProductID, LocationID, LocationsT.Name) AdjSubQ ON     "
			"   "
			"ProductT.ID = AdjSubQ.ProductID AND ProductLocationInfoT.LocationID = AdjSubQ.LocationID "
			"	"
			/* if there is a linked allocation with the same product, do not use the case history
			amount unless it is greater, in which case we use the overage*/
			"LEFT OUTER JOIN "
			"(SELECT ItemID, Sum(CaseHistoryQty) AS UnRelievedCaseHistQuantity, LocationID, LocationName AS Location FROM ("
					"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(ActiveAllocationQty,0)) THEN SUM(Quantity) - SUM(Coalesce(ActiveAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, CaseHistoryT.LocationID, LocationsT.Name AS LocationName "
					"FROM CaseHistoryDetailsT INNER JOIN "
					"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
					"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
					"LEFT JOIN (SELECT Sum(Quantity) AS ActiveAllocationQty, ProductID, CaseHistoryID "
						"FROM PatientInvAllocationsT "
						"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
						"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
						"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " "
						"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " "
						"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
					"WHERE ItemType = -1 AND CompletedDate Is Null "
					"GROUP BY ItemID, CaseHistoryT.ID, LocationID, LocationsT.Name "
				") AS CaseHistoryIndivSubQ "
				"GROUP BY ItemID, LocationID, LocationName "
			") AS UnRelievedCaseHistSubQ ON "
			"ProductT.ID = UnRelievedCaseHistSubQ.ItemID AND ProductLocationInfoT.LocationID = UnRelievedCaseHistSubQ.LocationID "
			"   "
			/* if there is a linked allocation with the same product, do not use the case history
			amount unless it is greater, in which case we use the overage*/
			"/* Items On Completed Case Histories */  "
			"LEFT OUTER JOIN "
			"(SELECT ItemID, Sum(CaseHistoryQty) AS CaseHistQuantity, LocationID, LocationName AS Location FROM ("
					"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(UsedAllocationQty,0))THEN SUM(Quantity) - SUM(Coalesce(UsedAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, CaseHistoryT.LocationID, LocationsT.Name AS LocationName "
					"FROM CaseHistoryDetailsT INNER JOIN "
					"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
					"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
					"LEFT JOIN (SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, CaseHistoryID "
						"FROM PatientInvAllocationsT "
						"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
						"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
						"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " "
						"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " "
						"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
					"WHERE ItemType = -1 AND CompletedDate Is Not Null "
					"GROUP BY ItemID, CaseHistoryT.ID, LocationID, LocationsT.Name "
				") AS CaseHistoryIndivSubQ "
				"GROUP BY ItemID, LocationID, LocationName "
			") AS CaseHistSubQ ON "
			"ProductT.ID = CaseHistSubQ.ItemID AND ProductLocationInfoT.LocationID = CaseHistSubQ.LocationID LEFT OUTER JOIN "

			"/* Items on order */   "
			" (SELECT OrderDetailsT.ProductID, SUM(QuantityOrdered) AS QuantityOrdered, OrderT.LocationID, LocationsT.Name AS Location "
			"  FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID LEFT JOIN LocationsT ON OrderT.LocationID = LocationsT.ID  "
			"  WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived Is Null) AND OrderDetailsT.ForStatus <> %li "
			"  GROUP BY OrderDetailsT.ProductID, LocationID, LocationsT.Name "
			"  ) GenOnOrderSubQ ON "
			"ProductT.ID = GenOnOrderSubQ.ProductID AND ProductLocationInfoT.LocationID = GenOnOrderSubQ.LocationID "

			"/*Items Transferred To This Location*/ "
			"LEFT OUTER JOIN "
			"(SELECT ProductLocationTransfersT.ProductID, SUM(Amount) AS TransferToQty, ProductLocationTransfersT.DestLocationID AS LocationID, LocationsT.Name AS Location "
				"FROM ProductLocationTransfersT LEFT JOIN LocationsT ON ProductLocationTransfersT.DestLocationID = LocationsT.ID  "
				"GROUP BY ProductLocationTransfersT.ProductID, DestLocationID, LocationsT.Name "
			") AS TransferToSubQ ON ProductT.ID = TransferToSubQ.ProductID AND ProductLocationInfoT.LocationID = TransferToSubQ.LocationID "

			"/*Items Transferred From This Location*/ "
			"LEFT OUTER JOIN "
			"(SELECT ProductLocationTransfersT.ProductID, SUM(Amount) AS TransferFromQty, ProductLocationTransfersT.SourceLocationID AS LocationID, LocationsT.Name AS Location "
				"FROM ProductLocationTransfersT LEFT JOIN LocationsT ON ProductLocationTransfersT.SourceLocationID = LocationsT.ID "
				"GROUP BY ProductLocationTransfersT.ProductID, SourceLocationID, LocationsT.Name "
			") AS TransferFromSubQ ON ProductT.ID = TransferFromSubQ.ProductID AND ProductLocationInfoT.LocationID = TransferFromSubQ.LocationID "
			" "
			"/*Items Used From Allocations*/ "
			"LEFT OUTER JOIN ( "
				"SELECT ProductID, Sum(Quantity) AS UsedAllocationQty, LocationID, LocationsT.Name AS Location "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"LEFT JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
				"WHERE PatientInvAllocationsT.Status = %li "
				"AND PatientInvAllocationDetailsT.Status = %li "
				"GROUP BY ProductID, LocationID, LocationsT.Name "
			") AS CompletedAllocationsQ ON ProductT.ID = CompletedAllocationsQ.ProductID AND ProductLocationInfoT.LocationID = CompletedAllocationsQ.LocationID "
			" "
			"/*Items allocated, not used or released yet */ "
			"LEFT JOIN ( "
				"SELECT ProductID, Sum(Quantity) AS OnHoldAllocationQty, LocationID, LocationsT.Name AS Location "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"LEFT JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
				"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " "
				"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " "
				"GROUP BY ProductID, LocationID, LocationsT.Name "
			") AS OnHoldAllocationsQ ON ProductT.ID = OnHoldAllocationsQ.ProductID AND ProductLocationInfoT.LocationID = OnHoldAllocationsQ.LocationID "
			" "
			// (j.jones 2014-11-10 14:31) - PLID 64105 - converted to use joins instead of a NOT IN clause
			"/*Items that are not consignment and not used */"
			"LEFT JOIN ( "
			"	SELECT Count(ProductItemsT.ID) AS GeneralActual, ProductItemsT.ProductID, ProductItemsT.LocationID, LocationsT.Name AS Location "
			"	FROM ProductItemsT "
			"	LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"	LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"	LEFT JOIN ("
			"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"		AND PatientInvAllocationsT.Status <> %li "
			"		AND PatientInvAllocationDetailsT.Status = %li "
			"	) AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
			"	WHERE ChargedProductItemsT.ProductItemID Is Null "
			"	AND AllocationsQ.ProductItemID Is Null "
			"	AND ProductItemsT.Status <> %li "
			"	AND ProductItemsT.Deleted = 0 "
			"	GROUP BY ProductItemsT.ProductID, ProductItemsT.LocationID, LocationsT.Name "
			") AS GeneralProductsActualQ ON ProductT.ID = GeneralProductsActualQ.ProductID AND ProductLocationInfoT.LocationID = GeneralProductsActualQ.LocationID "
			" "
			// (j.jones 2014-11-10 14:31) - PLID 64105 - converted to use joins instead of a NOT IN clause
			"/*Items that are not consignment and unavailable */"
			"LEFT JOIN ( "
			"	SELECT Count(ProductItemsT.ID) AS GeneralAvail, ProductItemsT.ProductID, ProductItemsT.LocationID, LocationsT.Name AS Location "
			"	FROM ProductItemsT "
			"	LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"	LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"	LEFT JOIN ("
			"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"		AND PatientInvAllocationsT.Status <> %li "
			"		AND PatientInvAllocationDetailsT.Status IN (%li,%li) "
			"	) AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
			"	WHERE ChargedProductItemsT.ProductItemID Is Null "
			"	AND AllocationsQ.ProductItemID Is Null "
			"	AND ProductItemsT.Status <> %li "
			"	AND ProductItemsT.Deleted = 0 "
			"	GROUP BY ProductItemsT.ProductID, ProductItemsT.LocationID, LocationsT.Name "
			") AS GeneralProductsAvailQ ON ProductT.ID = GeneralProductsAvailQ.ProductID AND ProductLocationInfoT.LocationID = GeneralProductsAvailQ.LocationID "
			" "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
			"WHERE ProductLocationInfoT.TrackableStatus = 2 AND ServiceT.Active = 1 "
			
			//Consignment inventory
			"UNION "
			"SELECT 1 AS ProductTypeID, 'Consignment' AS ProductTypeName, PersonT.Company, PersonT.ID AS SupID, CategoriesT.Name AS CategoryName,     "
			"ServiceT.ID AS ProductID, ServiceT.Name AS ProductName, ProductT.UnitDesc,     "
			"ProductLocationInfoT.ConsignmentReorderPoint AS ReorderPoint, "
			"1 AS ReorderQuantity, "

			//use the product item count only if serial options are enabled,
			//otherwise always use 0
			"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"THEN Coalesce(ConsignmentProductsActualQ.ConsignmentActual, 0) "
			"ELSE 0 END AS NumInStock, "
			
			"CASE WHEN ConsignOnOrderSubQ.QuantityOrdered Is Null THEN 0 ELSE ConsignOnOrderSubQ.QuantityOrdered END AS QuantityOnOrder, "
			
			"LocationsT.ID AS LocID, LocationsT.Name AS Location, "

			//use the product item count only if serial options are enabled,
			//otherwise always use 0
			"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"THEN Coalesce(ConsignmentProductsAvailQ.ConsignmentAvail, 0) "
			"ELSE 0 END AS AvailableStock "

			"FROM ServiceT LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"INNER JOIN ProductLocationInfoT ON ServiceT.ID = ProductLocationInfoT.ProductID "
			"INNER JOIN LocationsT ON ProductLocationInfoT.LocationID = LocationsT.ID "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT OUTER JOIN    "
			"   "
			// (j.jones 2014-11-10 13:40) - PLID 64105 - moved the "Items Charged" subquery into one spot				
			"/*Items Charged */   "
			" (" + strItemsChargedSubquery + ") ChargeSubQ ON     "
			"ProductT.ID = ChargeSubQ.ServiceID AND ProductLocationInfoT.LocationID = ChargeSubQ.LocationID LEFT OUTER JOIN    "
			"   "
			"/* Items received */   "
			" (SELECT OrderDetailsT.ProductID, SUM(QuantityOrdered) AS QuantityOrdered, OrderT.LocationID, LocationsT.Name AS Location "
			"  FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID LEFT JOIN LocationsT ON OrderT.LocationID = LocationsT.ID  "
			"  WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived Is Not Null) "
			"  GROUP BY OrderDetailsT.ProductID, LocationID, LocationsT.Name "
			"  ) ReceivedSubQ ON     "
			"ProductT.ID = ReceivedSubQ.ProductID AND ProductLocationInfoT.LocationID = ReceivedSubQ.LocationID LEFT OUTER JOIN    "
			"   "
			"/* Items Adjusted */    "
			" (SELECT ProductAdjustmentsT.ProductID, SUM(Quantity) AS AdjQuantity, LocationID, LocationsT.Name AS Location "
			"  FROM ProductAdjustmentsT LEFT JOIN LocationsT ON ProductAdjustmentsT.LocationID = LocationsT.ID "
			"  GROUP BY ProductAdjustmentsT.ProductID, LocationID, LocationsT.Name) AdjSubQ ON     "
			"   "
			"ProductT.ID = AdjSubQ.ProductID AND ProductLocationInfoT.LocationID = AdjSubQ.LocationID "
			"   "
			/* if there is a linked allocation with the same product, do not use the case history
			amount unless it is greater, in which case we use the overage*/
			"LEFT OUTER JOIN "
			"(SELECT ItemID, Sum(CaseHistoryQty) AS UnRelievedCaseHistQuantity, LocationID, LocationName AS Location FROM ("
					"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(ActiveAllocationQty,0)) THEN SUM(Quantity) - SUM(Coalesce(ActiveAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, CaseHistoryT.LocationID, LocationsT.Name AS LocationName "
					"FROM CaseHistoryDetailsT INNER JOIN "
					"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
					"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
					"LEFT JOIN (SELECT Sum(Quantity) AS ActiveAllocationQty, ProductID, CaseHistoryID "
						"FROM PatientInvAllocationsT "
						"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
						"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
						"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " "
						"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " "
						"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
					"WHERE ItemType = -1 AND CompletedDate Is Null "
					"GROUP BY ItemID, CaseHistoryT.ID, LocationID, LocationsT.Name "
				") AS CaseHistoryIndivSubQ "
				"GROUP BY ItemID, LocationID, LocationName "
			") AS UnRelievedCaseHistSubQ ON "
			"ProductT.ID = UnRelievedCaseHistSubQ.ItemID AND ProductLocationInfoT.LocationID = UnRelievedCaseHistSubQ.LocationID "
			"   "
			/* if there is a linked allocation with the same product, do not use the case history
			amount unless it is greater, in which case we use the overage*/
			"/* Items On Completed Case Histories */  "
			"LEFT OUTER JOIN "
			"(SELECT ItemID, Sum(CaseHistoryQty) AS CaseHistQuantity, LocationID, LocationName AS Location FROM ("
					"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(UsedAllocationQty,0))THEN SUM(Quantity) - SUM(Coalesce(UsedAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, CaseHistoryT.LocationID, LocationsT.Name AS LocationName "
					"FROM CaseHistoryDetailsT INNER JOIN "
					"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
					"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
					"LEFT JOIN (SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, CaseHistoryID "
						"FROM PatientInvAllocationsT "
						"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
						"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
						"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " "
						"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " "
						"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
					"WHERE ItemType = -1 AND CompletedDate Is Not Null "
					"GROUP BY ItemID, CaseHistoryT.ID, LocationID, LocationsT.Name "
				") AS CaseHistoryIndivSubQ "
				"GROUP BY ItemID, LocationID, LocationName "
			") AS CaseHistSubQ ON "
			"ProductT.ID = CaseHistSubQ.ItemID AND ProductLocationInfoT.LocationID = CaseHistSubQ.LocationID LEFT OUTER JOIN "

			"/* Items on order */   "
			" (SELECT OrderDetailsT.ProductID, SUM(QuantityOrdered) AS QuantityOrdered, OrderT.LocationID, LocationsT.Name AS Location "
			"  FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID LEFT JOIN LocationsT ON OrderT.LocationID = LocationsT.ID  "
			"  WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived Is Null) AND OrderDetailsT.ForStatus = %li "
			"  GROUP BY OrderDetailsT.ProductID, LocationID, LocationsT.Name "
			"  ) ConsignOnOrderSubQ ON "
			"ProductT.ID = ConsignOnOrderSubQ.ProductID AND ProductLocationInfoT.LocationID = ConsignOnOrderSubQ.LocationID "

			"/*Items Transferred To This Location*/ "
			"LEFT OUTER JOIN "
			"(SELECT ProductLocationTransfersT.ProductID, SUM(Amount) AS TransferToQty, ProductLocationTransfersT.DestLocationID AS LocationID, LocationsT.Name AS Location "
				"FROM ProductLocationTransfersT LEFT JOIN LocationsT ON ProductLocationTransfersT.DestLocationID = LocationsT.ID  "
				"GROUP BY ProductLocationTransfersT.ProductID, DestLocationID, LocationsT.Name "
			") AS TransferToSubQ ON ProductT.ID = TransferToSubQ.ProductID AND ProductLocationInfoT.LocationID = TransferToSubQ.LocationID "

			"/*Items Transferred From This Location*/ "
			"LEFT OUTER JOIN "
			"(SELECT ProductLocationTransfersT.ProductID, SUM(Amount) AS TransferFromQty, ProductLocationTransfersT.SourceLocationID AS LocationID, LocationsT.Name AS Location "
				"FROM ProductLocationTransfersT LEFT JOIN LocationsT ON ProductLocationTransfersT.SourceLocationID = LocationsT.ID "
				"GROUP BY ProductLocationTransfersT.ProductID, SourceLocationID, LocationsT.Name "
			") AS TransferFromSubQ ON ProductT.ID = TransferFromSubQ.ProductID AND ProductLocationInfoT.LocationID = TransferFromSubQ.LocationID "
			" "
			"/*Items Used From Allocations*/ "
			"LEFT OUTER JOIN ( "
				"SELECT ProductID, Sum(Quantity) AS UsedAllocationQty, LocationID, LocationsT.Name AS Location "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"LEFT JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
				"WHERE PatientInvAllocationsT.Status = %li "
				"AND PatientInvAllocationDetailsT.Status = %li "
				"GROUP BY ProductID, LocationID, LocationsT.Name "
			") AS CompletedAllocationsQ ON ProductT.ID = CompletedAllocationsQ.ProductID AND ProductLocationInfoT.LocationID = CompletedAllocationsQ.LocationID "
			" "
			"/*Items allocated, not used or released yet */ "
			"LEFT JOIN ( "
				"SELECT ProductID, Sum(Quantity) AS OnHoldAllocationQty, LocationID, LocationsT.Name AS Location "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"LEFT JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
				"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " "
				"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " "
				"GROUP BY ProductID, LocationID, LocationsT.Name "
			") AS OnHoldAllocationsQ ON ProductT.ID = OnHoldAllocationsQ.ProductID AND ProductLocationInfoT.LocationID = OnHoldAllocationsQ.LocationID "
			" "
			// (j.jones 2014-11-10 14:31) - PLID 64105 - converted to use joins instead of a NOT IN clause
			"/*Items that are consignment and not used */"
			"LEFT JOIN ( "
			"	SELECT Count(ProductItemsT.ID) AS ConsignmentActual, ProductItemsT.ProductID, ProductItemsT.LocationID, LocationsT.Name AS Location "
			"	FROM ProductItemsT "
			"	LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"	LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"	LEFT JOIN ("
			"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"		AND PatientInvAllocationsT.Status <> %li "
			"		AND PatientInvAllocationDetailsT.Status = %li "
			"	) AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
			"	WHERE ChargedProductItemsT.ProductItemID Is Null "
			"	AND AllocationsQ.ProductItemID Is Null "
			"	AND ProductItemsT.Status = %li "
			"	AND ProductItemsT.Deleted = 0 "
			"	GROUP BY ProductItemsT.ProductID, ProductItemsT.LocationID, LocationsT.Name "
			") AS ConsignmentProductsActualQ ON ProductT.ID = ConsignmentProductsActualQ.ProductID AND ProductLocationInfoT.LocationID = ConsignmentProductsActualQ.LocationID "
			" "
			// (j.jones 2014-11-10 14:31) - PLID 64105 - converted to use joins instead of a NOT IN clause
			"/*Items that are consignment and unavailable */"
			"LEFT JOIN ( "
			"	SELECT Count(ProductItemsT.ID) AS ConsignmentAvail, ProductItemsT.ProductID, ProductItemsT.LocationID, LocationsT.Name AS Location "
			"	FROM ProductItemsT "
			"	LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"	LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"	LEFT JOIN ("
			"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"		AND PatientInvAllocationsT.Status <> %li "
			"		AND PatientInvAllocationDetailsT.Status IN (%li, %li) "
			"	) AS AllocationsQ ON ProductItemsT.ID = AllocationsQ.ProductItemID "
			"	WHERE ChargedProductItemsT.ProductItemID Is Null "
			"	AND AllocationsQ.ProductItemID Is Null "
			"	AND ProductItemsT.Status = %li "
			"	AND ProductItemsT.Deleted = 0 "
			"	GROUP BY ProductItemsT.ProductID, ProductItemsT.LocationID, LocationsT.Name "
			") AS ConsignmentProductsAvailQ ON ProductT.ID = ConsignmentProductsAvailQ.ProductID AND ProductLocationInfoT.LocationID = ConsignmentProductsAvailQ.LocationID "
			" "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
			"WHERE ProductLocationInfoT.TrackableStatus = 2 AND ServiceT.Active = 1 "

			//end UNION
			") AS InvListSubQ    "
			"WHERE (%s) ",
			InvUtils::odsConsignment, InvUtils::iasCompleted, InvUtils::iadsUsed,
			InvUtils::iasDeleted, InvUtils::iadsUsed, InvUtils::pisConsignment,
			InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed, InvUtils::pisConsignment,
			InvUtils::odsConsignment, InvUtils::iasCompleted, InvUtils::iadsUsed,
			InvUtils::iasDeleted, InvUtils::iadsUsed, InvUtils::pisConsignment,
			InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed, InvUtils::pisConsignment,
			// (j.jones 2008-02-14 13:52) - PLID 28864 - check the preference to see when we should reorder
			strOrderWhere);

			return _T(strSql);
		}
		break;
	
	
		case 236:
			//Inventory Tracking
			/*	Version History
				DRT 4/10/2006 - PLID 11734 - Removed "ProcCode = 0" filter.
				// (j.jones 2007-11-28 11:52) - while making changes for PLID 28196, for allocations, I saw that this report is
				// not only out of date, but commented out of the system. It MUST be updated before ever reusing, as this query
				// isn't accounting for several features that affect inventory totals.
			*/
			{
			CString part1, part2, part3, part4;
			part1 =  _T("SELECT InvUnionQ.ItemType, InvUnionQ.Name AS ItemName, InvUnionQ.Quantity, InvUnionQ.Date AS Date,   "
			"ReceivedSubQ.QuantityOrdered, ChargeSubQ.ChargeQuantity, AdjSubQ.AdjQuantity, InvUnionQ.Notes,  "
			"(CASE WHEN (ReceivedSubQ.QuantityOrdered) IS NULL    "
			"THEN 0 ELSE ReceivedSubQ.QuantityOrdered END)    "
			"- (CASE WHEN (ChargeSubQ.ChargeQuantity) IS NULL    "
			"THEN 0 ELSE ChargeSubQ.ChargeQuantity END)    "
			"+ (CASE WHEN (AdjSubQ.AdjQuantity) IS NULL    "
			"THEN 0 ELSE AdjSubQ.AdjQuantity END) AS NumInStock,   "
			"LocationsT.Name AS LocName  "
			"FROM  "
			"(  "
			"/* All Received Items */  "
			"SELECT 'Received' AS ItemType, ProductT.ID, ServiceT.Name, QuantityOrdered AS Quantity, OrderDetailsT.DateReceived AS Date, OrderDetailsT.ID AS LineID, OrderT.LocationID, OrderT.Description AS Notes  "
			"FROM  "
			"OrderT LEFT OUTER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID   "
			"INNER JOIN ProductT ON OrderDetailsT.ProductID = ProductT.ID   "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
			"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
			"WHERE (OrderDetailsT.DateReceived Is Not Null) AND (OrderDetailsT.Deleted = 0) AND ProductID IN (SELECT ProductID FROM ProductLocationInfoT WHERE TrackableStatus = 2)  "
			"UNION   "
			"/* All Item Adjustments */  ");
			
			part2 = _T(" SELECT 'Adjustment' AS ItemType, ProductT.ID, ServiceT.Name, ProductAdjustmentsT.Quantity, ProductAdjustmentsT.Date AS Date, ProductAdjustmentsT.ID AS LineID, ProductAdjustmentsT.LocationID, ProductAdjustmentsT.Notes   "
			"FROM  "
			"ProductAdjustmentsT INNER JOIN ProductT ON ProductAdjustmentsT.ProductID = ProductT.ID   "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
			"WHERE ProductID IN (SELECT ProductID FROM ProductLocationInfoT WHERE TrackableStatus = 2) "
			"UNION  "
			"/* All Item Charges */  "
			"SELECT 'ChargeID ' AS ItemType, ProductT.ID, ServiceT.Name, ChargesT.Quantity * -1, LineItemT.Date AS Date, LineItemT.ID AS LineID, LineItemT.LocationID, LineItemT.Description AS Notes "
			"FROM  "
			"ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
			"LEFT OUTER JOIN ChargesT ON ServiceT.ID = ChargesT.ServiceID ");

			part3 = _T(	"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID   "
			"WHERE (LineItemT.Deleted = 0) AND (ProductID IN (SELECT ProductID FROM ProductLocationInfoT WHERE TrackableStatus = 2)) AND (LineItemT.Type = 10) "
			") AS InvUnionQ   "
			"LEFT OUTER JOIN   "
			"/*Items Charged */  "
			"    (SELECT ChargesT.ServiceID, SUM(ChargesT.Quantity) AS ChargeQuantity   "
			"  FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
			"  WHERE (LineItemT.Deleted = 0) AND LineItemT.Type = 10 "
			"  GROUP BY ChargesT.ServiceID) ChargeSubQ ON "
			"InvUnionQ.ID = ChargeSubQ.ServiceID LEFT OUTER JOIN "
			"/* Items received */ "
			" (SELECT ProductID, SUM(QuantityOrdered) "
			"       AS QuantityOrdered "
			"  FROM OrderDetailsT "
			"  WHERE (OrderDetailsT.Deleted = 0)  "
			"  GROUP BY ProductID, DateReceived   "
			"  HAVING (DateReceived IS NOT NULL)) ReceivedSubQ ON ");			
			part4 = _T(	"InvUnionQ.ID = ReceivedSubQ.ProductID LEFT OUTER JOIN "
			"/* Items Adjusted */   "
			" (SELECT ProductID, SUM(Quantity) AS AdjQuantity   "
			"  FROM ProductAdjustmentsT   "
			"  GROUP BY ProductID) AdjSubQ ON    "
			"InvUnionQ.ID = AdjSubQ.ProductID "
			"INNER JOIN  "
			"LocationsT ON InvUnionQ.LocationID = LocationsT.ID  ");

			return part1 + part2 + part3 + part4;
			break;
			}

		case 267: {
			//Inventory Values
			/*Revision History:
				DRT 1/6/03 - Reworked part of the query.  It was joining category -> product -> subQ.  This had the
						effect of putting items in the report that had no activity whatsoever, so they would show up with no information (and under no 
						location!).  I reworked how the joins go together so it correctly does subQ -> product -> category, which works out much nicer
						for the report display.
				JMJ 3/13/02 - Enabled only the ASC query. Fixed the cost calculations to accomodate UU/UO values.
				JMJ 7/29/03 - Ensured that a tracked product would show up always, even with no activity
				DRT 2/13/2004 - PLID 10566 - Made the report not show inactive inventory items.
				// (j.jones 2007-11-29 08:35) - PLID 28196 - completed Allocations now decrement from Inventory
				// (j.jones 2007-12-18 09:20) - PLID 27988 - billed allocations do not decrement from inventory
				// (j.jones 2008-02-29 14:51) - PLID 29132 - case histories now don't decrement from Inventory if
				// they have a linked allocation, unless they have more of the product than the allocation
				// (this also means this report has to use the product items count if a product is serialized)
				// (j.jones 2008-04-07 12:29) - PLID 29574 - We had changed this report to calculate serialized quantities
				// based off of ProductItemsT counts, but that cannot be done reliably with a date filter with our current
				// structure. Since this report does not split by purchased inventory and consignment, and it would mean bad
				// data if the calculated total didn't match the product item total, I am reverting this to be the calculated
				// quantities at all times.
				TES 5/27/2008 - PLID 29459 - Updated to use the linked appointment date (if any) as the "Status Date"
					for Allocations, instead of the allocation's CompletedDate (which is still used if there is no linked
					appointment).
			*/

			CString strSql;
			strSql.Format("SELECT ServiceT.ID, CategoriesT.Name AS CategoryName, "
			"ServiceT.Name AS ProductName, ProductT.UnitDesc, "
			"PersonT.Company AS SupplierName, "
			"ProductT.UseUU, "
			"ProductT.Conversion, "

			//use the standard calculation only if no serial options are enabled,
			//otherwise look explicitly at product items
			// (j.jones 2008-04-07 12:30) - PLID 29574 - changed to always be calculated
			/*"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
			"THEN Coalesce(ProductItemsActualQ.Actual, 0) "
			"ELSE SUM(OnHandSubQ.Quantity) END AS NumInStock, "*/
			"SUM(OnHandSubQ.Quantity) AS NumInStock, "
			
			"CategoriesT.ID AS CategoryID, "
			"ServiceT.Price AS Cost, ProductT.LastCost, PersonT.ID AS SupplierID, "
			"OnHandSubQ.LocationID AS LocID, LocationsT.Name AS Location "
			"FROM ( "

			// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
			// (j.dinatale 2011-11-07 09:52) - PLID 46226 - need to exclude voiding and original line items of financial corrections
			// (s.dhole 2012-04-09 17:40) - PLID 49537 Exclude Off the shelf from Inventory report 
			// (j.jones 2014-11-10 13:35) - PLID 64105 - reworked the IN clauses for GlassesOrderServiceT and BilledCaseHistoriesT such that they are now left joining
			// (r.goldschmidt 2015-06-08 15:56) - PLID 66234 - need to add some parentheses to correct a where clause

			"/*Items Charged */ "
			"	SELECT ServiceID AS ProductID, -1*SUM(CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS Quantity, LineItemT.LocationID "
			"	FROM ChargesT "
			"	LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
			"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
			"	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li "
			"		GROUP BY ChargeID) AS AllocationInfoQ ON ChargesT.ID = AllocationInfoQ.ChargeID "
			"	LEFT JOIN ("
			"		SELECT GlassesOrderServiceT.ID "
			"		FROM GlassesOrderServiceT "
			"		INNER JOIN ProductT ON GlassesOrderServiceT.ServiceID = ProductT.ID "
			"		WHERE GlassesOrderServiceT.IsDefaultProduct = 1 AND GlassesOrderServiceT.IsOffTheShelf = 0  "
			"		AND (ProductT.GlassesContactLensDataID IS NOT NULL OR ProductT.FramesDataID IS NOT NULL) "
			"	) AS GlassesOrderServiceQ ON ChargesT.GlassesOrderServiceID = GlassesOrderServiceQ.ID "
			"	LEFT JOIN (SELECT BillID FROM BilledCaseHistoriesT GROUP BY BillID) AS BilledCaseHistoriesQ ON BillsT.ID = BilledCaseHistoriesQ.BillID "
			"LEFT JOIN CaseHistoryDetailsT ON CaseHistoryDetailsT.ItemID = ChargesT.ServiceID AND CaseHistoryDetailsT.ItemType = -1 "
			"LEFT JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID AND BilledCaseHistoriesT.BillID = BillsT.ID "
			"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND LineItemT.Date < @ReportDateTo AND BillsT.EntryType = 1 "
			"	AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
			"	AND BilledCaseHistoriesQ.BillID Is Null "
			"	AND GlassesOrderServiceQ.ID Is Null "
			"	GROUP BY ServiceID, LocationID "

			"/* Items received */ "
			"	UNION ALL SELECT ProductID, SUM(QuantityOrdered) AS Quantity, OrderT.LocationID "
			"	   AS QuantityOrdered "
			"	FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
			"	WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived IS NOT NULL) AND DateReceived < @ReportDateTo "
			"	GROUP BY ProductID, LocationID "
			"/* Items adjusted */ "
			"	UNION ALL SELECT ProductID, SUM(Quantity) AS Quantity, ProductAdjustmentsT.LocationID "
			"	FROM ProductAdjustmentsT WHERE ProductAdjustmentsT.Date < @ReportDateTo "
			"	GROUP BY ProductID, LocationID "
			"/* Items Transferred To */ "
			"	UNION ALL SELECT ProductID, SUM(Amount) AS Quantity, ProductLocationTransfersT.DestLocationID AS LocationID "
			"	FROM ProductLocationTransfersT WHERE ProductLocationTransfersT.Date < @ReportDateTo "
			"	GROUP BY ProductID, DestLocationID "
			"/* Items Transferred From */ "
			"	UNION ALL SELECT ProductID, -1*(SUM(Amount)) AS Quantity, ProductLocationTransfersT.SourceLocationID AS LocationID "
			"	FROM ProductLocationTransfersT WHERE ProductLocationTransfersT.Date < @ReportDateTo "
			"	GROUP BY ProductID, SourceLocationID "

			/* if there is a linked allocation with the same product, do not use the case history
			amount unless it is greater, in which case we use the overage*/
			/* but if a serialized product, we have to use the case history amt. if
			the linked product items are not in the allocation*/
			"/* Items On Completed Case Histories */ "
			"	UNION ALL SELECT ItemID AS ProductID, -1*Sum(CaseHistoryQty) AS Quantity, LocationID FROM ("
					"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(UsedAllocationQty,0))THEN SUM(Quantity) - SUM(Coalesce(UsedAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, CaseHistoryT.LocationID "
					"FROM CaseHistoryDetailsT INNER JOIN "
					"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
					"LEFT JOIN (SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, CaseHistoryID "
						"FROM PatientInvAllocationsT "
						"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
						"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
						"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " "
						"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " "
						"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
					"WHERE ItemType = -1 AND CompletedDate Is Not Null AND CompletedDate < @ReportDateTo "
					"GROUP BY ItemID, CaseHistoryT.ID, CaseHistoryT.LocationID "
				") AS CaseHistoryIndivSubQ GROUP BY ItemID, LocationID "

			"/*Items Used From Allocations*/ "
			"	UNION ALL SELECT ProductID, -1*(SUM(Quantity)) AS Quantity, PatientInvAllocationsT.LocationID "
			"	FROM PatientInvAllocationsT "
			"	INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"	LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"	WHERE PatientInvAllocationsT.Status = %li "
			"	AND PatientInvAllocationDetailsT.Status = %li "
			"	AND COALESCE(AppointmentsT.StartTime,PatientInvAllocationsT.CompletionDate) < @ReportDateTo "
			"	GROUP BY ProductID, PatientInvAllocationsT.LocationID "
			"/* Include a 0 for each item to ensure it shows up in the list */"
			"	UNION ALL SELECT ProductT.ID AS ProductID, 0 AS Quantity, LocationsT.ID AS LocationID "
			"	FROM ProductT CROSS JOIN LocationsT WHERE LocationsT.Managed = 1 "
			"	) OnHandSubQ "
			" "

			"/*Product Items that are not used */"
			// (j.jones 2008-04-07 12:30) - PLID 29574 - changed the report to always be calculated,
			// we can comment this in again once we can make it reliably filter on date			
			// (r.goldschmidt 2015-06-08 16:47) - PLID 66234 - change from  = completed to <> deleted for filter on PatientInvAllocationsT.Status
			/*
			//this is NOT unioned, but instead it is a separate value
			"LEFT JOIN ( "
			"	SELECT Count(ProductItemsT.ID) AS Actual, ProductItemsT.ProductID, ProductItemsT.LocationID "
			"	FROM ProductItemsT "
			"	WHERE ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"	AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"		AND PatientInvAllocationsT.Status <> " + AsString((long)InvUtils::iasDeleted) + " "
			"		AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + ") "
			"	AND ProductItemsT.Deleted = 0 "
			"	GROUP BY ProductItemsT.ProductID, ProductItemsT.LocationID "
			") AS ProductItemsActualQ ON OnHandSubQ.ProductID = ProductItemsActualQ.ProductID AND ProductItemsActualQ.LocationID = OnHandSubQ.LocationID "
			*/

			"LEFT JOIN ProductT ON OnHandSubQ.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "			
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"LEFT JOIN LocationsT ON OnHandSubQ.LocationID = LocationsT.ID "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"LEFT JOIN PersonT ON PersonT.ID = MultiSupplierT.SupplierID "
			"WHERE ServiceT.Active = 1 AND ServiceT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE TrackableStatus = 2 AND LocationID = OnHandSubQ.LocationID) "
			"GROUP BY ServiceT.ID, CategoriesT.Name, ServiceT.Name, ProductT.UnitDesc, PersonT.Company, ProductT.UseUU, "
			"ProductT.Conversion, CategoriesT.ID, ServiceT.Price, ProductT.LastCost, PersonT.ID, OnHandSubQ.LocationID, LocationsT.Name, "
			"HasSerialNum, HasExpDate "
			/*", ProductItemsActualQ.Actual "*/,
			InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iasCompleted, InvUtils::iadsUsed);
			dtNext = DateTo + OneDay;
			strSql.Replace("@ReportDateTo", dtNext.Format("'%m/%d/%Y'"));
			return _T(strSql);
			}
			break;
	
		case 274: {
			//Product History
			/*	Version History
				DRT 7/28/03 - Added an extended filter for 'Ordered' (1), 'Charged'(2), 'Adjusted'(3), 'Case History'(4)
						Note:  The Case History only shows up if you have an ASC license.  See CReports::LoadExtendedFilter for more info.
				JMJ 7/30/03 - Added Items Transferred To (5) and Items Transferred From (6)
				DRT 8/4/03 - Added SupplierName and CategoryName.  Made report editable.
				TES 9/5/03 - Added Description
				TES 4/15/04 - Had it not filter out adjustments with 0 quantity if they have a cost.
				// (j.jones 2007-11-29 08:35) - PLID 28196 - completed Allocations now decrement from Inventory
				// (j.jones 2007-12-18 09:25) - PLID 27988 - billed allocations do not decrement from inventory
				// (j.jones 2008-02-29 14:56) - PLID 29132 - case histories now don't decrement from Inventory if
				// they have a linked allocation, unless they have more of the product than the allocation
				TES 5/27/2008 - PLID 29459 - Updated to use the linked appointment date (if any) as the "Status Date"
					for Allocations, instead of the allocation's CompletedDate (which is still used if there is no linked
					appointment).
				// (j.jones 2008-06-19 16:32) - PLID 30446 - supported PercentOff and Discount in the CostToPractice calculation
				// (c.haag 2008-06-24 15:49) - PLID 28356 - Added Items transferred to consignment
				// (c.haag 2008-06-24 15:49) - PLID 28356 - Added Items purchased from consignment
				//TES 6/25/2008 - PLID 26142 - Added the adjustment category to the "Adjustment" status.
				//DRT 2/23/2009 - PLID 32986 - Modified a .rpt formula to rename the "Returned" type (no query change)
				// (j.gruber 2009-03-25 15:10) - PLID 33696 - updated charge discount structure
				// (j.jones 2011-07-18 10:33) - PLID 44600 - corrected the case history quantity calculation to always
				// use the case history quantity, not the product items quantity
		*/

			CString strSql;
			strSql.Format("SELECT ProductHistorySubQ.ProductID AS ProdID,  "
			"ServiceT.Name,  "
			"ProductHistorySubQ.Location AS LocID, "
			"ProductHistorySubQ.Date AS Date,  "
			"ProductHistorySubQ.CostToPractice AS Cost,  "
			"ProductHistorySubQ.TaxAmount,  "
			"ProductHistorySubQ.EventType, "
			"ProductHistorySubQ.Qty, "
			"ProductHistorySubQ.UserName, "
			"LocationsT.Name AS Location,  "
			"ProductHistorySubQ.Type AS ItemType, PersonT.Company AS SupplierName, CategoriesT.Name AS CategoryName, "
			"ProductHistorySubQ.Description "
			"FROM( "
			" "
			" /* Orders Received */ "
			"SELECT  "
			"OrderDetailsT.ID, OrderDetailsT.ProductID,  "			
			"-1*(Sum(Round(Convert(money,(Round(Convert(money,((((OrderDetailsT.ExtraCost + ((CASE WHEN OrderDetailsT.UseUU = 1 THEN Convert(int,OrderDetailsT.QuantityOrdered / Convert(float,OrderDetailsT.Conversion)) ELSE OrderDetailsT.QuantityOrdered END) * OrderDetailsT.Amount)) * ((100-Convert(float,PercentOff))/100)) - Discount))),2) * OrderT.Tax)),2))) AS CostToPractice, "
			"0 AS TaxAmount,  "
			"OrderT.LocationID AS Location, "
			"OrderDetailsT.DateReceived AS Date,  "
			"'Order' AS EventType, OrderDetailsT.QuantityOrdered AS Qty, UsersT.UserName, 1 AS Type, "
			"OrderT.Description "
			"FROM OrderDetailsT LEFT JOIN (OrderT LEFT JOIN UsersT ON OrderT.CreatedBy = UsersT.PersonID) ON OrderDetailsT.OrderID = OrderT.ID "
			"WHERE OrderDetailsT.Deleted = 0 AND OrderT.Deleted = 0 AND OrderDetailsT.DateReceived Is Not Null "
			"GROUP BY OrderDetailsT.ID, OrderDetailsT.ProductID, OrderT.LocationID, OrderDetailsT.DateReceived, OrderDetailsT.QuantityOrdered, UsersT.UserName, OrderT.Description "
			" "

			// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
			// (j.dinatale 2011-11-07 09:52) - PLID 46226 - need to exclude voiding and original line items of financial corrections
			// (s.dhole 2012-04-09 18:15) - PLID 49540 Exclude Off the shelf from Inventory report 
			// (j.jones 2014-11-10 13:37) - PLID 64105 - reworked the IN clauses for GlassesOrderServiceT and BilledCaseHistoriesT such that they are now left joining
			// (r.goldschmidt 2015-06-08 15:56) - PLID 66234 - need to add some parentheses to correct a where clause
			" /* Charged Items */ "
			"UNION SELECT "
			"LineItemT.ID, ChargesT.ServiceID,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS TotalCharge, "
			"Round(convert(money, Convert(money,((LineItemT.[Amount]*ChargesT.Quantity*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate-1)))),2) + "
			"Round(convert(money, Convert(money,((LineItemT.[Amount]*ChargesT.Quantity*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate2-1)))),2) AS TotalTax, "
			"LineItemT.LocationID,  "
			"LineItemT.Date, "
			"'Charge' AS EventType, -1*(CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS Qty, LineItemT.InputName, 2 AS Type, "
			"LineItemT.Description "
			"FROM ChargesT "
			"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
			"LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
			"	INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
			"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"	WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li "
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
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.EntryType = 1 "
			"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
			"AND ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) > 0 "
			"AND BilledCaseHistoriesQ.BillID Is Null "
			"AND GlassesOrderServiceQ.ID Is Null "
			" "
			" /* Adjusted Items */ "
			"UNION SELECT ProductAdjustmentsT.ID, ProductAdjustmentsT.ProductID,  "
			"ProductAdjustmentsT.Amount,  "
			"0 AS TaxAmount,  "
			"ProductAdjustmentsT.LocationID, "
			"ProductAdjustmentsT.Date, "
			"CASE WHEN ProductAdjustmentCategoriesT.Name Is Null THEN 'Adjustment' ELSE 'Adjustment - ' + "
			"ProductAdjustmentCategoriesT.Name END AS EventType, ProductAdjustmentsT.Quantity AS Qty, UsersT.UserName, "
			"3 AS Type, ProductAdjustmentsT.Notes AS Description "
			"FROM ProductAdjustmentsT LEFT JOIN UsersT ON ProductAdjustmentsT.Login = UsersT.PersonID  "
			"LEFT JOIN ProductAdjustmentCategoriesT ON ProductAdjustmentsT.ProductAdjustmentCategoryID = ProductAdjustmentCategoriesT.ID "
			"WHERE ProductAdjustmentsT.Quantity <> 0 OR ProductAdjustmentsT.Amount <> 0 "
			" "
			" /* Items Transferred To */ "
			"UNION SELECT ProductLocationTransfersT.ID, ProductLocationTransfersT.ProductID, "
			"0 AS Amount, "
			"0 AS TaxAmount, "
			"ProductLocationTransfersT.DestLocationID AS LocationID, "
			"ProductLocationTransfersT.Date, "
			"'Transfer' AS EventType, ProductLocationTransfersT.Amount AS Qty, UsersT.UserName, 5 AS Type, "
			"ProductLocationTransfersT.Notes AS Description "
			"FROM ProductLocationTransfersT LEFT JOIN UsersT ON ProductLocationTransfersT.UserID = UsersT.PersonID  "
			"WHERE ProductLocationTransfersT.Amount <> 0 "
			" "
			" /* Items Transferred From */ "
			"UNION SELECT ProductLocationTransfersT.ID, ProductLocationTransfersT.ProductID, "
			"0 AS Amount, "
			"0 AS TaxAmount, "
			"ProductLocationTransfersT.SourceLocationID AS LocationID, "
			"ProductLocationTransfersT.Date, "
			"'Transfer' AS EventType, -1*ProductLocationTransfersT.Amount AS Qty, UsersT.UserName, 6 AS Type, "
			"ProductLocationTransfersT.Notes AS Description "
			"FROM ProductLocationTransfersT LEFT JOIN UsersT ON ProductLocationTransfersT.UserID = UsersT.PersonID  "
			"WHERE ProductLocationTransfersT.Amount <> 0 "
			" "

			/* if there is a linked allocation with the same product, do not use the case history
			amount unless it is greater, in which case we use the overage*/
			/* but if a serialized product, we have to use the case history amt. if
			the linked product items are not in the allocation*/
			" /* Case History Items */ "
			"UNION SELECT ID, ItemID, Round(Convert(money,Amount*Quantity),2) AS TotalAmount, "
			"0 AS TaxAmount, LocationID, CompletedDate AS Date, 'Case History' AS EventType, "
			"-1*Quantity AS Qty, MailSentQ.Sender AS UserName, 4 AS Type, Name AS Description "
			"FROM (SELECT CaseHistoryT.ID, CaseHistoryDetailsT.ItemID, CaseHistoryT.LocationID, "
			"	CaseHistoryT.CompletedDate, Min(Amount) AS Amount, "
			"	(CASE WHEN SUM(Quantity) > SUM(COALESCE(UsedAllocationQty,0)) "
			"		THEN SUM(Quantity) - SUM(COALESCE(UsedAllocationQty,0)) "
			"		ELSE 0 END) AS Quantity, "
			"	CaseHistoryT.Name "
			"	FROM CaseHistoryDetailsT "
			"	INNER JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
			"	INNER JOIN ProductT ON CaseHistoryDetailsT.ItemID = ProductT.ID "			
			"	LEFT JOIN (SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, CaseHistoryID "
			"		FROM PatientInvAllocationsT "
			"		INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"		INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
			"		WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " "
			"		AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " "
			"		GROUP BY ProductID, CaseHistoryID "
			"	) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
			"	WHERE ItemType = -1 AND CompletedDate Is Not Null "
			"	GROUP BY ItemID, CaseHistoryT.ID, CaseHistoryDetailsT.ID, ProductT.HasSerialNum, ProductT.HasExpDate, "
			"	CaseHistoryT.LocationID, CaseHistoryT.Name, CaseHistoryT.CompletedDate "
			"	HAVING ("
            "		(CASE WHEN SUM(Quantity) > SUM(COALESCE(UsedAllocationQty,0)) "
			"			THEN SUM(Quantity) - SUM(COALESCE(UsedAllocationQty,0)) "
			"			ELSE 0 END) <> 0) "
			") AS CaseHistorySubQ "
			" "
			"LEFT JOIN (SELECT Sender, InternalRefID FROM MailSent WHERE InternalTblName = 'CaseHistoryT') MailSentQ ON CaseHistorySubQ.ID = MailSentQ.InternalRefID "
			" "			
			" /* Allocated Items */ "
			"UNION SELECT PatientInvAllocationDetailsT.ID, PatientInvAllocationDetailsT.ProductID, "
			"0 AS Amount, 0 AS TaxAmount, "
			"PatientInvAllocationsT.LocationID, COALESCE(AppointmentsT.StartTime,PatientInvAllocationsT.CompletionDate) AS Date, "
			"'Allocation' AS EventType, -1*PatientInvAllocationDetailsT.Quantity AS Qty, UsersT.UserName, 7 AS Type, "
			"ServiceT.Name AS Description "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"INNER JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
			"LEFT JOIN UsersT ON PatientInvAllocationsT.CompletedBy = UsersT.PersonID "
			"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"WHERE PatientInvAllocationsT.Status = %li "
			"AND PatientInvAllocationDetailsT.Status = %li "
			" "
			" /* Items transferred to consignment */ "
			"UNION SELECT ProductItemsStatusHistoryT.ID, ProductItemsT.ProductID AS ProdID, "
			"0 AS Amount, 0 AS TaxAmount, "
			"ProductItemsT.LocationID, ProductItemsStatusHistoryT.Date, "
			"'Transfer' AS EventType, 0 AS Qty, UsersT.Username, 8 AS Type,  "
			"'Item ' + LTRIM(RTRIM(CASE WHEN SerialNum IS NULL THEN '' ELSE SerialNum END + '   ' +  "
			"   CASE WHEN ExpDate IS NULL THEN '' ELSE Convert(nvarchar, ExpDate, 1) END)) + ' assigned to consignment' AS Description "
			"FROM ProductItemsStatusHistoryT "
			"INNER JOIN ProductItemsT ON ProductItemsT.ID = ProductItemsStatusHistoryT.ProductItemID "
			"LEFT JOIN UsersT ON UsersT.PersonID = ProductItemsStatusHistoryT.UserID "
			"WHERE ProductItemsT.Deleted = 0 AND (ProductItemsStatusHistoryT.OldStatus = 0 AND ProductItemsStatusHistoryT.NewStatus = 1) "
			" "
			" /* Items purchased from consignment */ "
			"UNION SELECT ProductItemsStatusHistoryT.ID, ProductItemsT.ProductID AS ProdID, "
			"-1*ProductItemsStatusHistoryT.Amount AS Amount, 0 AS TaxAmount, "
			"ProductItemsT.LocationID, ProductItemsStatusHistoryT.Date, "
			"'Purchase' AS EventType, 0 AS Qty, UsersT.Username, 9 AS Type,  "
			"'Item ' + LTRIM(RTRIM(CASE WHEN SerialNum IS NULL THEN '' ELSE SerialNum END + '   ' +  "
			"   CASE WHEN ExpDate IS NULL THEN '' ELSE Convert(nvarchar, ExpDate, 1) END)) + ' purchased from consignment' AS Description "
			"FROM ProductItemsStatusHistoryT "
			"INNER JOIN ProductItemsT ON ProductItemsT.ID = ProductItemsStatusHistoryT.ProductItemID "
			"LEFT JOIN UsersT ON UsersT.PersonID = ProductItemsStatusHistoryT.UserID "
			"WHERE ProductItemsT.Deleted = 0 AND (ProductItemsStatusHistoryT.OldStatus = 1 AND ProductItemsStatusHistoryT.NewStatus = 0) "
			" "
			")AS ProductHistorySubQ INNER JOIN (ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID) ON ProductHistorySubQ.ProductID = ServiceT.ID "
			" LEFT JOIN LocationsT ON ProductHistorySubQ.Location = LocationsT.ID "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"WHERE ProductHistorySubQ.ProductID IN (SELECT ProductID FROM ProductLocationInfoT WHERE TrackableStatus = 2) ",
			InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iasCompleted, InvUtils::iadsUsed);

			return _T(strSql);
			}
			break;

		case 323: {
			//Serial Numbered / Expirable Products By Patient
			/*	Version History
				DRT 7/12/2004 - PLID 13417 - Added cost
				JMJ 1/25/2005 - PLID 18994 - Added UserDefinedID
				// (j.jones 2007-11-29 08:35) - PLID 28196 - completed Allocations now decrement from Inventory
				// (j.jones 2007-12-18 09:38) - PLID 27988 - billed allocations do not decrement from inventory
				DRT 1/7/2008 - PLID 28476 - Added the 'Purchased Inv' vs 'Consignment' status
				// (j.jones 2008-02-22 10:22) - PLID 28951 - Exposed ProductItemsT.Status as a StatusID field for the report to read and use in formulas				
				TES 5/27/2008 - PLID 29459 - Updated to use the linked appointment date (if any) as the Service Date
					for Allocations, instead of the allocation's CompletedDate (which is still used if there is no linked
					appointment).
				// (j.jones 2008-06-06 10:07) - PLID 27110 - If a product is returned from a bill, we use a dummy product
				// as a placeholder on that bill, which references the original product with ProductItemsT.ReturnedFrom.
				// This report needs to ignore all ProductItems with a ReturnedFrom value.
				// (j.jones 2014-11-10 14:31) - PLID 64105 - converted allocations to use joins instead of a NOT IN clause
		*/
			//****Any changes made here should also probably be made in CGeneral2Dlg::SetSerializedProductListFromClause,
			//so be sure to double check that query to ensure it is accurate*****//

			CString strSql;
			strSql.Format("SELECT "
					"ProductID AS ProductID, Name, SerialNum, ExpDate, ServiceDate AS TDate, InputDate AS IDate, PatName, UserDefinedID, PatID AS PatID, SupplierID AS SupplierID, "
					"Cost, Coalesce(LocName,'') AS LocName, LocationID AS LocID, ItemStatus, Status AS StatusID FROM "
					"(SELECT "
					"ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, LineItemT.Date AS ServiceDate, LineItemT.InputDate AS InputDate, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, PersonT.ID AS PatID, MultiSupplierT.SupplierID AS SupplierID, ProductT.LastCost AS Cost, "
					"LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
					"CASE WHEN ProductItemsT.Status = 0 THEN 'Purchased Inv' WHEN ProductItemsT.Status = 1 THEN 'Consignment' ELSE '<Other>' END AS ItemStatus, ProductItemsT.Status "
					"FROM "
					"ProductT "
					"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
					"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
					"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
					"INNER JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
					"INNER JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
					"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
					"WHERE /*ProductT.HasSerialNum = 1 AND*/ ReturnedFrom Is Null AND (SerialNum Is Not Null OR ExpDate Is Not Null) AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND ProductItemsT.Deleted = 0 "
					"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
					"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
					"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
					"	AND PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status = %li) "
					"UNION SELECT "
					"ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, CaseHistoryT.SurgeryDate AS ServiceDate, CaseHistoryT.CompletedDate AS InputDate, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, PersonT.ID AS PatID, MultiSupplierT.SupplierID AS SupplierID, ProductT.LastCost AS Cost, "
					"LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
					"CASE WHEN ProductItemsT.Status = 0 THEN 'Purchased Inv' WHEN ProductItemsT.Status = 1 THEN 'Consignment' ELSE '<Other>' END AS ItemStatus, ProductItemsT.Status "
					"FROM "
					"ProductT "
					"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
					"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
					"INNER JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
					"INNER JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
					"INNER JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
					"INNER JOIN PersonT ON CaseHistoryT.PersonID = PersonT.ID "
					"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
					"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
					"WHERE ReturnedFrom Is Null AND (SerialNum Is Not Null OR ExpDate Is Not Null) AND CompletedDate Is Not Null AND ProductItemsT.Deleted = 0 "
					"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
					"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
					"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
					"	AND PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status = %li) "
					"UNION SELECT "
					"ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, COALESCE(AppointmentsT.StartTime,PatientInvAllocationsT.CompletionDate) AS ServiceDate, PatientInvAllocationsT.InputDate, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, PersonT.ID AS PatID, MultiSupplierT.SupplierID AS SupplierID, ProductT.LastCost AS Cost, "
					"LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
					"CASE WHEN ProductItemsT.Status = 0 THEN 'Purchased Inv' WHEN ProductItemsT.Status = 1 THEN 'Consignment' ELSE '<Other>' END AS ItemStatus, ProductItemsT.Status "
					"FROM "
					"ProductT "
					"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
					"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
					"INNER JOIN PatientInvAllocationDetailsT ON ProductItemsT.ID = PatientInvAllocationDetailsT.ProductItemID "
					"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
					"INNER JOIN PersonT ON PatientInvAllocationsT.PatientID = PersonT.ID "
					"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
					"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
					"LEFT JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
					"WHERE ReturnedFrom Is Null AND (SerialNum Is Not Null OR ExpDate Is Not Null) AND PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status = %li AND ProductItemsT.Deleted = 0) AS SerializedProductsQ",
					InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iadsUsed);

			return _T(strSql);
			}
			break;

		case 324:
			//Serial Numbered / Expirable Products In Stock
			// (j.jones 2007-11-26 10:56) - PLID 28037 - now hides allocated items
			// (j.jones 2014-11-10 14:31) - PLID 64105 - converted allocations to use joins instead of a NOT IN clause
			return _T("SELECT "
					"ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, MultiSupplierT.SupplierID AS SupplierID, "
					"Coalesce(LocationsT.Name,'') AS LocName, LocationsT.ID AS LocID "
					"FROM "
					"ProductT "
					"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
					"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
					"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
					"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
					"LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
					"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
					"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
					"WHERE (SerialNum Is Not Null OR ExpDate Is Not Null) AND ChargesT.ID Is Null AND CaseHistoryDetailsT.ID Is Null AND ProductItemsT.Deleted = 0 "
					"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
					"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
					"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
					"	AND PatientInvAllocationsT.Status <> -1 "
					"	AND (PatientInvAllocationDetailsT.Status = 1 OR PatientInvAllocationDetailsT.Status = 2)) ");
			break;

		case 326:
			//Inventory Items Given
			/*	Version History
				DRT 1/30/03 - Added the quantity, and changed the report to show the last cost + price fields as  Qty * <field>
				DRT 2/2/04 - PLID 7539 - Added the ability to create merge group from this report.
				// (z.manning, 09/11/2006) - PLID 22323 - We now correctly use the LastCost per unit of usage.
			*/
			return _T("SELECT "
				"ServiceT.Price, ServiceT.Name AS ProductName, ServiceT.ID AS ItemID, ProductT.LastCostPerUU AS LastCost, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName,  "
				"LineItemT.Date AS Date, LineItemT.LocationID AS LocID, PersonT.ID AS PatID, ChargesT.Quantity, MultiSupplierT.SupplierID AS SupplierID "
				"FROM "
				"ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
				"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"WHERE "
				"LineItemT.Amount = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type = 10");
			break;

		case 331:
		case 368:
		case 371:
			//Inventory Barcodes
			/*	Version History
				DRT 3/11/03 - Added a "by category" version of this, but it uses the same query and the same report file, just provides extra filters
							I also added the supplier id as "SupplierID" for filtering purposes
			*/
			//(c.copits 2010-08-31) PLID 40316 - Alert user of new prices for frames in inventory
			//(c.copits 2010-09-16) PLID 40316 - Alert user of new prices for frames in inventory
			//(c.copits 2010-09-23) PLID 40316 - Use PreviousPrice instead of PreviousCost; pull price
			//	from ServiceT instead of ProductT
			//TES 5/16/2011 - PLID 43712 - Added LastReceivedDate
			// (b.spivey, October 07, 2011) - PLID 45873 - Added several framesdata fields. 
			if (strListBoxFormat == "FramesPriceChanges") {
				return _T("SELECT ServiceT.ID AS ProductID, ServiceT.Name, ProductT.UnitDesc, ServiceT.Barcode, "
				"ServiceT.Price AS Price, ServiceT.Category AS CatID, "
				"MultiSupplierT.SupplierID AS SupplierID, MultiSupplierT.SupplierID AS SupplierID, "
				"LastReceivedQ.LastReceivedDate AS LastReceivedDate, ProductT.LastCost, FramesDataT.ManufacturerName AS Vendor, "
				"FramesDataT.StyleName, FramesDataT.ColorDescription, FramesDataT.ColorCode, FramesDataT.Eye, "
				"FramesDataT.Bridge, FramesDataT.Temple, FramesDataT.DBL, FramesDataT.A, FramesDataT.B, FramesDataT.ED, "
				"FramesDataT.EDAngle, FramesDataT.Circumference, FramesDataT.YearIntroduced " 
				"FROM "
				"ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"INNER JOIN FramesDataT ON ProductT.FramesDataID = FramesDataT.ID "
				"LEFT JOIN (SELECT ProductID, Max(LastReceivedDate) AS LastReceivedDate FROM "
				"	(SELECT ProductID, Max(Date) AS LastReceivedDate "
				"		FROM ProductAdjustmentsT WHERE Quantity > 0 GROUP BY ProductID "
				"		UNION SELECT ProductID, Max(DateReceived) AS LastReceivedDate FROM OrderDetailsT "
				"		WHERE DateReceived Is Not Null AND Deleted = 0 GROUP BY ProductID) "
				"	AS ReceivedQ GROUP BY ProductID "
				") AS LastReceivedQ ON ProductT.ID = LastReceivedQ.ProductID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"WHERE COALESCE(ProductT.PreviousPrice, 0) <> ServiceT.Price ORDER BY ServiceT.Name"); 
			}
			else{
				return _T("SELECT ServiceT.ID AS ProductID, ServiceT.Name, ProductT.UnitDesc, ServiceT.Barcode, "
				"ServiceT.Price AS Price, ServiceT.Category AS CatID, "
				"MultiSupplierT.SupplierID AS SupplierID, MultiSupplierT.SupplierID AS SupplierID, "
				"LastReceivedQ.LastReceivedDate AS LastReceivedDate, ProductT.LastCost, FramesDataT.ManufacturerName AS Vendor, "
				"FramesDataT.StyleName, FramesDataT.ColorDescription, FramesDataT.ColorCode, FramesDataT.Eye, "
				"FramesDataT.Bridge, FramesDataT.Temple, FramesDataT.DBL, FramesDataT.A, FramesDataT.B, FramesDataT.ED, "
				"FramesDataT.EDAngle, FramesDataT.Circumference, FramesDataT.YearIntroduced " 
				"FROM "
				"ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"LEFT JOIN FramesDataT ON ProductT.FramesDataID = FramesDataT.ID "
				"LEFT JOIN (SELECT ProductID, Max(LastReceivedDate) AS LastReceivedDate FROM "
				"	(SELECT ProductID, Max(Date) AS LastReceivedDate "
				"		FROM ProductAdjustmentsT WHERE Quantity > 0 GROUP BY ProductID "
				"		UNION SELECT ProductID, Max(DateReceived) AS LastReceivedDate FROM OrderDetailsT "
				"		WHERE DateReceived Is Not Null AND Deleted = 0 GROUP BY ProductID) "
				"	AS ReceivedQ GROUP BY ProductID "
				") AS LastReceivedQ ON ProductT.ID = LastReceivedQ.ProductID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"WHERE Barcode IS NOT NULL AND Rtrim(Barcode) <> '' ORDER BY ServiceT.Name"); 
			}
			break;

case 379: {
		//Products with Reserved Quantities
		/*	Version History
			Created (ASC-only)
			DRT 4/22/03 - Fixed a calculation bug - we need to SUBTRACT sold + case history quantites, not add them.  Also fixed a problem
					with the Location filter.
			// (j.jones 2007-11-28 11:15) - PLID 28196 - completed Allocations now decrement from Inventory
			// (j.jones 2007-12-18 10:00) - PLID 27988 - billed allocations do not decrement from inventory
			// (j.jones 2008-02-29 15:24) - PLID 29132 - reworked the calculations in this report to account for
			// the new ways we calculate the available amount concerning case histories and allocations
			// (this also means this report has to use the product items count if a product is serialized)
			// (j.jones 2008-03-07 14:55) - PLID 29235 - changed the 'avail' calculations to not be case or allocation-only
			// based on the license, instead it calculates with the same logic as the 'actual', except that case history totals
			// that aren't accounted for by an allocation will always decrement from General stock, never Consignment.
			// This also means we no longer care what the license is.		
			// (e.lally 2008-09-30) PLID 31540 - Added support to handle filtering on multiple locations.
				//I went to add support for each of the subqueries, only to realize that the ELSE query already does all the
				//joins and grouping on location that this would have required. I did a QA comparison of the sub-query filtering
				//versus filtering at the end after all the joins and found that we are only taking an 8% performance hit by
				//giving us only one query to manage.		
			// (j.jones 2016-01-19 08:46) - PLID 67680 - ensured the total inventory amount was rounded
		*/

		long nLocationID = this->nLocation;
		CString strLocationFilter;
		if(nLocationID > 0){
			strLocationFilter.Format("ProductQ.LocationID = %li AND ", nLocationID);
		}
		else if(nLocationID == -2){
			//This will always be false
			strLocationFilter = "ProductQ.LocationID IS NULL AND ";
		}
		else if(nLocationID == -3){
			strLocationFilter = "ProductQ.LocationID IN(";
			CString strPart;
			for(int i=0; i < this->m_dwLocations.GetSize(); i++) {
				strPart.Format("%li, ", (long)this->m_dwLocations.GetAt(i));
				strLocationFilter += strPart;
			}
			strLocationFilter = strLocationFilter.Left(strLocationFilter.GetLength()-2) + ") AND ";
		}

		//No Location filters applied
		CString strSQL;
		strSQL.Format("SELECT ProductQ.ID, ProductQ.Name, ProductQ.SupplierID AS SupplierID, ProductQ.Category AS CategoryID, ProductQ.SupplierName, "

		//use the standard calculation only if no serial options are enabled,
		//otherwise look explicitly at product items				
		"/* Total On Hand (physical) */ "
		"Round("
		"CASE WHEN HasSerialNum = 1 OR HasExpDate = 1 "
		"THEN Coalesce(ProductItemsActualQ.Actual, 0) "
		"ELSE "
		"CASE WHEN ChargeQ.Sold IS NULL THEN 0 ELSE (-1*ChargeQ.Sold) END + CASE WHEN OrderQ.Received IS NULL THEN 0 ELSE OrderQ.Received END "
		"+ CASE WHEN AdjQ.Adjusted IS NULL THEN 0 ELSE AdjQ.Adjusted END "
		"+ CASE WHEN TransferredToQ.TransferredTo IS NULL THEN 0 ELSE TransferredToQ.TransferredTo END "
		"+ CASE WHEN TransferredFromQ.TransferredFrom IS NULL THEN 0 ELSE TransferredFromQ.TransferredFrom END "
		"+ CASE WHEN CompletedAllocationsQ.UsedAllocationQty IS NULL THEN 0 ELSE (-1*CompletedAllocationsQ.UsedAllocationQty) END "
		"+ CASE WHEN CaseHistSubQ.CaseHistoryQty IS NULL THEN 0 ELSE (-1*CaseHistSubQ.CaseHistoryQty) END "
		"END, 2) AS TotalOnHand, "

		"/* Items On Hold */ "
		"Round("
		"Coalesce(UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty, 0) "
		"+ Coalesce(OnHoldAllocationsQ.OnHoldAllocationQty, 0) "
		", 2) AS QuantityOnHold, "

		"/* Amount Available */ "
		"Round("
		"CASE WHEN ChargeQ.Sold IS NULL THEN 0 ELSE (-1*ChargeQ.Sold) END + CASE WHEN OrderQ.Received IS NULL THEN 0 ELSE OrderQ.Received END "
		"+ CASE WHEN AdjQ.Adjusted IS NULL THEN 0 ELSE AdjQ.Adjusted END "
		"+ CASE WHEN TransferredToQ.TransferredTo IS NULL THEN 0 ELSE TransferredToQ.TransferredTo END "
		"+ CASE WHEN TransferredFromQ.TransferredFrom IS NULL THEN 0 ELSE TransferredFromQ.TransferredFrom END "
		"+ CASE WHEN CompletedAllocationsQ.UsedAllocationQty IS NULL THEN 0 ELSE (-1*CompletedAllocationsQ.UsedAllocationQty) END "
		"+ CASE WHEN CaseHistSubQ.CaseHistoryQty IS NULL THEN 0 ELSE (-1*CaseHistSubQ.CaseHistoryQty) END "
		"- CASE WHEN UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty END "
		"- CASE WHEN OnHoldAllocationsQ.OnHoldAllocationQty IS NULL THEN 0 ELSE OnHoldAllocationsQ.OnHoldAllocationQty END "
		", 2) AS AmountAvail, "

		"ProductQ.LocName "
		"FROM (  "
		"SELECT ProductT.ID, ProductLocationInfoT.LocationID, LocationsT.Name AS LocName, ServiceT.Name, UnitDesc, Price, Taxable1, Taxable2, RevCodeUse,  "
		"UB92Category, ReOrderPoint, ReorderQuantity, MultiSupplierT.SupplierID, Category, LastCost,  "
		"Barcode, ProductT.Notes, MultiSupplierT.Catalog, UseUU, UnitOfOrder, UnitOfUsage, Conversion, HasSerialNum, HasExpDate,  "
		"CASE WHEN PersonT.ID IS NULL THEN 'No Supplier' ELSE PersonT.Company END AS SupplierName "
		"FROM ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
		"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
		"INNER JOIN LocationsT ON ProductLocationInfoT.LocationID = LocationsT.ID "
		"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
		"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID  /*WHERE ProductT.ID = [id]*/  "
		") AS ProductQ  "

		// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
		// (j.dinatale 2011-11-07 09:52) - PLID 46226 - need to exclude voiding and original line items of financial corrections
		"LEFT JOIN (  "
		"/* "
		"	ChargeQ totals all charged products, leaving out all items which are to be included in CaseHistSubQ (below) "
		"*/ "
		"	SELECT SUM (CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS Sold, ServiceID, LineItemT.LocationID "
		"	FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
		"	LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
		"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
		"	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID  "
		"	LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
		"		INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
		"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"		WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li "
		"		GROUP BY ChargeID) AS AllocationInfoQ ON ChargesT.ID = AllocationInfoQ.ChargeID "
		"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Type = 10 "
		"	AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
		"	AND (BillsT.ID NOT IN (SELECT BillID FROM BilledCaseHistoriesT) OR ChargesT.ServiceID NOT IN  "
		"	(SELECT ItemID FROM CaseHistoryDetailsT "
		"	INNER JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID "
		"	WHERE BilledCaseHistoriesT.BillID = BillsT.ID "
		"	AND ItemType = -1))  "
		"	GROUP BY ServiceID, LineItemT.LocationID "
		") AS ChargeQ ON ProductQ.ID = ChargeQ.ServiceID AND ProductQ.LocationID = ChargeQ.LocationID "
		"LEFT JOIN (  "
		"	SELECT ProductID, SUM (QuantityOrdered) AS Received, OrderT.LocationID "
		"	FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID  "
		"	WHERE DateReceived IS NOT NULL  "
		"	AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
		"	GROUP BY ProductID, OrderT.LocationID "
		") AS OrderQ ON ProductQ.ID = OrderQ.ProductID AND ProductQ.LocationID = OrderQ.LocationID "
		"LEFT JOIN (  "
		"	SELECT ProductID, SUM (QuantityOrdered) AS Ordered, OrderT.LocationID "
		"	FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID  "
		"	WHERE DateReceived IS NULL  "
		"	AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
		"	GROUP BY ProductID, OrderT.LocationID "
		") AS OnOrderQ ON ProductQ.ID = OnOrderQ.ProductID AND ProductQ.LocationID = OnOrderQ.LocationID "
		"LEFT JOIN (  "
		"	SELECT SUM(Quantity) AS Adjusted, ProductID, LocationID "
		"	FROM ProductAdjustmentsT  "
		"	GROUP BY ProductID, LocationID "
		") AS AdjQ ON ProductQ.ID = AdjQ.ProductID AND ProductQ.LocationID = AdjQ.LocationID "
		"LEFT JOIN (  "
		"	SELECT SUM(Amount) AS TransferredTo, ProductID, DestLocationID AS LocationID "
		"	FROM ProductLocationTransfersT  "
		"	GROUP BY ProductID, DestLocationID "
		") AS TransferredToQ ON ProductQ.ID = TransferredToQ.ProductID AND ProductQ.LocationID = TransferredToQ.LocationID "
		"LEFT JOIN (  "
		"	SELECT -1*SUM(Amount) AS TransferredFrom, ProductID, SourceLocationID AS LocationID "
		"	FROM ProductLocationTransfersT  "
		"	GROUP BY ProductID, SourceLocationID "
		") AS TransferredFromQ ON ProductQ.ID = TransferredFromQ.ProductID AND ProductQ.LocationID = TransferredFromQ.LocationID "

		/* if there is a linked allocation with the same product, do not use the case history
		amount unless it is greater, in which case we use the overage*/
		"/* "
		"	UnRelievedCaseHistSubQ totals the items in incompleted Case Histories. (on reserve number) "
		"*/ "
		"LEFT JOIN "
		"(SELECT Sum(CaseHistoryQty) AS UnRelievedCaseHistoryQty, ItemID, LocationID FROM ("
				"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(ActiveAllocationQty,0)) THEN SUM(Quantity) - SUM(Coalesce(ActiveAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, CaseHistoryT.LocationID "
				"FROM CaseHistoryDetailsT INNER JOIN "
				"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
				"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
				"LEFT JOIN (SELECT Sum(Quantity) AS ActiveAllocationQty, ProductID, CaseHistoryID "
					"FROM PatientInvAllocationsT "
					"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
					"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
					"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " "
					"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " "
					"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
				"WHERE ItemType = -1 AND CompletedDate Is Null "
				"GROUP BY ItemID, CaseHistoryT.ID, LocationID "
			") AS CaseHistoryIndivSubQ "
			"GROUP BY ItemID, LocationID "
		") AS UnRelievedCaseHistSubQ ON ProductQ.ID = UnRelievedCaseHistSubQ.ItemID AND ProductQ.LocationID = UnRelievedCaseHistSubQ.LocationID "

		/* if there is a linked allocation with the same product, do not use the case history
		amount unless it is greater, in which case we use the overage*/
		"/* "
		"	CaseHistSubQ totals the items that are included in completed Case Histories. "
		"*/ "
		"LEFT JOIN ("
		"SELECT Sum(CaseHistoryQty) AS CaseHistoryQty, ItemID, LocationID FROM ("
				"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(UsedAllocationQty,0))THEN SUM(Quantity) - SUM(Coalesce(UsedAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, LocationID "
				"FROM CaseHistoryDetailsT INNER JOIN "
				"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
				"LEFT JOIN (SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, CaseHistoryID "
					"FROM PatientInvAllocationsT "
					"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
					"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
					"WHERE PatientInvAllocationsT.Status = %li "
					"AND PatientInvAllocationDetailsT.Status = %li "
					"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
				"WHERE ItemType = -1 AND CompletedDate Is Not Null "
				"GROUP BY ItemID, CaseHistoryT.ID, CaseHistoryT.LocationID "
			") AS CaseHistoryIndivSubQ GROUP BY ItemID, LocationID "
		") AS CaseHistSubQ ON ProductQ.ID = CaseHistSubQ.ItemID AND ProductQ.LocationID = CaseHistSubQ.LocationID "

		"LEFT JOIN ( "
			"SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, LocationID "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"WHERE PatientInvAllocationsT.Status = %li "
			"AND PatientInvAllocationDetailsT.Status = %li "
			"GROUP BY ProductID, LocationID "
		") AS CompletedAllocationsQ ON ProductQ.ID = CompletedAllocationsQ.ProductID AND CompletedAllocationsQ.LocationID = ProductQ.LocationID "

		"LEFT JOIN ( "
			"SELECT Sum(Quantity) AS OnHoldAllocationQty, ProductID, LocationID "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"WHERE PatientInvAllocationsT.Status = %li "
			"AND PatientInvAllocationDetailsT.Status = %li "
			"GROUP BY ProductID, LocationID "
		") AS OnHoldAllocationsQ ON ProductQ.ID = OnHoldAllocationsQ.ProductID AND OnHoldAllocationsQ.LocationID = ProductQ.LocationID "

		// (j.jones 2014-11-10 14:31) - PLID 64105 - converted allocations to use joins instead of a NOT IN clause
		// (r.goldschmidt 2015-06-08 16:47) - PLID 66234 - change from  = completed to <> deleted for filter on PatientInvAllocationsT.Status
		"/*Product Items that are not used */"
		"LEFT JOIN ( "
		"	SELECT Count(ProductItemsT.ID) AS Actual, ProductItemsT.ProductID, ProductItemsT.LocationID "
		"	FROM ProductItemsT "
		"	WHERE ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
		"	AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"		AND PatientInvAllocationsT.Status <> %li "
		"		AND PatientInvAllocationDetailsT.Status = %li) "
		"	AND ProductItemsT.Deleted = 0 "
		"	GROUP BY ProductItemsT.ProductID, ProductItemsT.LocationID "
		") AS ProductItemsActualQ ON ProductQ.ID = ProductItemsActualQ.ProductID AND ProductItemsActualQ.LocationID = ProductQ.LocationID "

		"LEFT JOIN CategoriesT ON ProductQ.Category = CategoriesT.ID "
		" "
		"WHERE %s "
		"Coalesce(UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty, 0) "
		"+ Coalesce(OnHoldAllocationsQ.OnHoldAllocationQty, 0) "
		"> 0",
		InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iasActive, InvUtils::iadsActive, InvUtils::iasDeleted, InvUtils::iadsUsed,
		strLocationFilter);

		return _T(strSQL);
		}
		break;

		case 400:
			//Inactive Inventory Items
			/*	Version History
				DRT 7/16/03 - Created.
			*/
			return _T("SELECT ServiceT.ID AS ServiceID, ProductT.UnitDesc, ServiceT.Name, CategoriesT.Name AS CatName, ServiceT.Price, CategoriesT.ID AS CatID, MultiSupplierT.SupplierID AS SupplierID "
				"FROM "
				"ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
				"WHERE ServiceT.Active = 0");
			break;

		case 461:
			//Inventory Adjustments
			/*	Version History
				//TES 6/25/2008 - PLID 26142 - Added ProductAdjustmentCategoriesT fields.
			*/
			return _T("SELECT ProductID, Date AS Date, UsersT.UserName, Quantity, Amount, LocationsT.ID AS LocID, LocationsT.Name AS LocName,  "
			"ProductAdjustmentsT.Notes AS AdjustmentNotes, ServiceT.Name AS ProductName,  "
			"/*Custom Fields*/ "
			"ServiceT.Category AS CategoryID, CategoriesT.Name AS CatName, ServiceT.Price AS StandardPrice, ProductT.Notes AS ProductNotes, "
			"ProductT.UnitDesc, ProductAdjustmentCategoriesT.Name AS AdjustmentCategory, "
			"ProductAdjustmentCategoryID AS AdjCategoryID "
			"FROM "
			"ProductAdjustmentsT "
			"INNER JOIN ServiceT ON ProductAdjustmentsT.ProductID = ServiceT.ID "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN ProductAdjustmentCategoriesT ON ProductAdjustmentsT.ProductAdjustmentCategoryID = ProductAdjustmentCategoriesT.ID "
			"LEFT JOIN LocationsT ON ProductAdjustmentsT.LocationID = LocationsT.ID "
			"LEFT JOIN UsersT ON ProductAdjustmentsT.Login = UsersT.PersonID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID");
			break;

		case 602:
			// Product Transfers
			/*	Version History
				// (d.moore 2007-07-23) - PLID 26149 - Created.
			*/
			return _T(
				"SELECT ProductT.ID AS ProductID, "
					"ServiceT.Name AS Name, "
					"ProductT.UnitDesc AS UnitDesc, "
					"CategoriesT.ID AS CategoryID, "
					"CategoriesT.Name AS Category, "
					"SourceLocationT.ID AS LocID, "
					"SourceLocationT.ID AS SourceLocationID, "
					"SourceLocationT.Name AS SourceLocation, "
					"DestLocationT.ID AS DestLocationID, "
					"DestLocationT.Name AS DestLocation, "
					"ProductLocationTransfersT.Amount AS Amount, "
					"ProductLocationTransfersT.Date AS Date, "
					"UsersT.PersonID AS UserID, "
					"UsersT.UserName AS UserName "
				"FROM ProductLocationTransfersT "
					"INNER JOIN ProductT ON ProductT.ID = ProductLocationTransfersT.ProductID "
					"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
					"INNER JOIN LocationsT SourceLocationT ON ProductLocationTransfersT.SourceLocationID = SourceLocationT.ID "
					"INNER JOIN LocationsT DestLocationT ON ProductLocationTransfersT.DestLocationID = DestLocationT.ID "
					"LEFT JOIN UsersT ON ProductLocationTransfersT.UserID = UsersT.PersonID "
					"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID");
			break;

		case 616:
			/*	Version History
				// (c.haag 2007-11-15 13:01) - PLID 28103 - Supplier returns
				// (c.haag 2008-01-08 09:36) - PLID 28103 - Added Completed flag
				// (c.haag 2008-03-07 12:47) - PLID 29170 - Added usage of nExtraID as the return group ID
				// (c.haag 2008-03-07 14:45) - PLID 29170 - Added ReturnedFor
				// (j.jones 2008-05-27 10:02) - PLID 30167 - added Supplier fields, including AccountName
				//TES 1/27/2009 - PLID 32824 - Added VendorConfirmed, ConfirmationDate, ConfirmationNumber
			*/
			{
				CString strPreviewWhere;
				if(nExtraID != -1) {
					strPreviewWhere += FormatString("WHERE SupplierReturnGroupsT.ID = %d", nExtraID);
				}
				return _T(
					CString("SELECT "
						"/* Group fields */ "
						"SupplierReturnGroupsT.ID AS ReturnGroupID, SupplierReturnGroupsT.Description, SupplierReturnGroupsT.Notes AS GroupNote, "
						"LocationsT.Name AS LocationName, SupplierReturnGroupsT.LocationID AS LocID, SupplierReturnGroupsT.ReturnDate AS ReturnDate, "
						"SupplierReturnMethodT.Method, SupplierReturnGroupsT.TrackingNumber, "
						"/*Supplier Information*/ "
						"PersonT.Company, PersonT.Address1 AS SupAddr1, PersonT.Address2 AS SupAddr2, PersonT.City AS SupCity, PersonT.State AS SupState, PersonT.Zip AS SupZip, SupplierT.CCNumber, PersonT.CompanyID, PersonT.Fax AS SupFax,  "
						"PersonT.WorkPhone, SupplierT.AccountName, "
						"/* Item fields */ "
						"SupplierReturnItemsT.ID AS ReturnItemID, "
						"CASE WHEN NonSerialProductT.Name IS NULL THEN SerialProductT.Name ELSE NonSerialProductT.Name END AS ProductName, "
						"ProductItemsT.SerialNum, SupplierReturnItemsT.Quantity, SupplierReturnReasonT.Reason, SupplierReturnItemsT.CreditAmount, "
						"SupplierReturnItemsT.Completed AS Completed, SupplierReturnItemsT.CompletedDate AS DateCompleted, SupplierReturnItemsT.AmountReceived, SupplierReturnItemsT.Notes AS ItemNote, "
						"CASE WHEN SupplierReturnItemsT.ProductID IS NULL THEN ProductItemsT.ProductID ELSE SupplierReturnItemsT.ProductID END AS ProductID, SupplierReturnItemsT.ReturnedFor, "
						"SupplierReturnGroupsT.VendorConfirmed, SupplierReturnGroupsT.ConfirmationDate, SupplierReturnGroupsT.ConfirmationNumber "
						"FROM SupplierReturnItemsT "
						"/* Group joins */ "
						"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnGroupsT.ID = SupplierReturnItemsT.ReturnGroupID "
						"LEFT JOIN PersonT ON PersonT.ID = SupplierReturnGroupsT.SupplierID "
						"LEFT JOIN SupplierT ON PersonT.ID = SupplierT.PersonID "
						"LEFT JOIN LocationsT ON LocationsT.ID = SupplierReturnGroupsT.LocationID "
						"LEFT JOIN SupplierReturnMethodT ON SupplierReturnMethodT.ID = SupplierReturnGroupsT.ReturnMethodID "
						"/* Item joins */ "
						"LEFT JOIN ServiceT NonSerialProductT ON NonSerialProductT.ID = SupplierReturnItemsT.ProductID "
						"LEFT JOIN ProductItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
						"LEFT JOIN ServiceT SerialProductT ON SerialProductT.ID = ProductItemsT.ProductID "
						"LEFT JOIN SupplierReturnReasonT ON SupplierReturnReasonT.ID = SupplierReturnItemsT.ReturnReasonID ")
						+ strPreviewWhere;
					);
			}
			break;

		// (c.haag 2008-03-07 12:31) - PLID 29170 - Deprecated
		/*case 617:
			{*/
				/*	Version History
					// (c.haag 2007-11-16 09:09) - PLID 28120 - Allergan Product Transfer Summary Sheet (but not actually changed for it)
					TES 2/18/2008 - PLID 28954 - Added SupplierT.AccountName and SupplierReturnItemsT.ReturnedFor
					TES 2/18/2008 - PLID 28953 - Added PersonT.CompanyID, and LocationsT.City and .State.
				*/
				/*CString strPreviewWhere = "WHERE SupplierReturnItemsT.Completed = 0";
				if(nExtraID != -1) {
					strPreviewWhere += FormatString(" AND SupplierReturnGroupsT.ID = %d", nExtraID);
				}
				return _T("SELECT "*/
					//"/* Group fields */ "
					/*"SupplierReturnGroupsT.ID AS ReturnGroupID, SupplierReturnGroupsT.Description, SupplierReturnGroupsT.Notes AS GroupNote, "
					"LocationsT.Name AS LocationName, SupplierReturnGroupsT.LocationID AS LocID, PersonT.Company, SupplierReturnGroupsT.ReturnDate AS ReturnDate, "
					"SupplierReturnMethodT.Method, SupplierReturnGroupsT.TrackingNumber, "
					"SupplierT.AccountName, "
					"PersonT.CompanyID AS AccountNumber, LocationsT.City, LocationsT.State, "*/
					//"/* Item fields */ "
					/*"SupplierReturnItemsT.ID AS ReturnItemID, "
					"CASE WHEN NonSerialProductT.Name IS NULL THEN SerialProductT.Name ELSE NonSerialProductT.Name END AS ProductName, "
					"ProductItemsT.SerialNum, SupplierReturnItemsT.Quantity, SupplierReturnReasonT.Reason, SupplierReturnItemsT.CreditAmount, "
					"SupplierReturnItemsT.CompletedDate AS CompletedDate, SupplierReturnItemsT.AmountReceived, SupplierReturnItemsT.Notes AS ItemNote, "
					"SupplierReturnItemsT.ProductID, SupplierReturnItemsT.ReturnedFor "
				"FROM SupplierReturnItemsT "*/
					//"/* Group joins */ "
					/*"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnGroupsT.ID = SupplierReturnItemsT.ReturnGroupID "
					"LEFT JOIN PersonT ON PersonT.ID = SupplierReturnGroupsT.SupplierID "
					"LEFT JOIN SupplierT ON PersonT.ID = SupplierT.PersonID "
					"LEFT JOIN LocationsT ON LocationsT.ID = SupplierReturnGroupsT.LocationID "
					"LEFT JOIN SupplierReturnMethodT ON SupplierReturnMethodT.ID = SupplierReturnGroupsT.ReturnMethodID "*/
					//"/* Item joins */ "
					/*"LEFT JOIN ServiceT NonSerialProductT ON NonSerialProductT.ID = SupplierReturnItemsT.ProductID "
					"LEFT JOIN ProductItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
					"LEFT JOIN ServiceT SerialProductT ON SerialProductT.ID = ProductItemsT.ProductID "
					"LEFT JOIN SupplierReturnReasonT ON SupplierReturnReasonT.ID = SupplierReturnItemsT.ReturnReasonID "
					) + strPreviewWhere;
			}
		break;*/

		case 618:	//Inventory Overview
		case 620:	//Consignment List
			{
				/*	Version History
					DRT 11/17/2007 - PLID 27990 - Created.  This query to start is basically the same query as that in the consignment dialog.
					DRT 11/26/2007 - PLID 27990 - The report was renamed since the tab was renamed, and added several new fields and filters.
					// (j.jones 2007-11-29 10:43) - PLID 28196 - accounted for completed allocations as "used",
					// though the status is superceded if also billed or on a case history					
					(c.haag 2007-12-03 11:55) - PLID 28204 - Version 2 - Changed Consignment bit to integer Status enumeration
					(c.haag 2007-12-12 16:42) - PLID 28357 - Created Consignment list report to use this excellent query
					(c.haag 2008-02-06 16:42) - PLID 28642 - Version 3 - Added allocation detail notes
					// (j.jones 2008-03-05 17:05) - PLID 29202 - added provider information and filter
					TES 5/27/2008 - PLID 29459 - Updated to use the linked appointment date (if any) as the "Date Used"
						for Allocations, instead of the allocation's CompletedDate (which is still used if there is no linked
						appointment).
					// (j.jones 2008-06-02 16:16) - PLID 28076 - adjusted products now use the adjustment date,
					// but it can be null if it is old data prior to when we started tracking adjustment IDs
					// (j.jones 2008-06-06 10:07) - PLID 27110 - If a product is returned from a bill, we use a dummy product
					// as a placeholder on that bill, which references the original product with ProductItemsT.ReturnedFrom.
					// This report needs to ignore all ProductItems with a ReturnedFrom value.
					//TES 6/18/2008 - PLID 29578 - Updated (and simplified) the query now that ProductItemsT has an OrderDetailID instead
					// of an OrderID
					//TES 6/25/2008 - PLID 26142 - Added the Adjustment Category to the 'Adjusted' Status
					//TES 9/3/2008 - PLID 29779 - Added the DateOrdered field.
					//TES 9/3/2008 - PLID 31237 - Aliased DateReceived AS DateReceived so that the filtering would work.
					// (j.jones 2009-02-09 12:56) - PLID 32775 - renamed 'Charged' to 'Billed'
					// (j.jones 2009-03-09 12:54) - PLID 33096 - product items created from adjustments can now use the adjustment date
					// as the date received, but order date will remain null
					// (j.jones 2009-03-18 15:56) - PLID 33580 - added Consignment Paid fields
				*/

				//Extra stuff.  When previewing this from the Inventory module, we need a bunch of filters.  So the view generates an extended
				//	WHERE clause and sends it over here.  This will be blank if we're just previewing from the reports module
				CString strPreviewWhere = strReportName;

				return _T(FormatString("SELECT ProductItemsT.ID, ProductItemsT.ProductID AS ProductID, ServiceT.Name AS ProductName, "
				"LocationsT.ID AS LocID, Coalesce(LocationsT.Name, '<No Location>') AS LocName, "
				"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"ProductItemsT.Status AS ProductItemStatus, ServiceT.Category AS Category, "
				"CASE WHEN ProductItemsT.Status = %d THEN 'Consignment' ELSE CASE WHEN ProductItemsT.Status = %d THEN 'Warranty' ELSE 'Purchased Inv.' END END AS ProductType, "
				"CASE WHEN CreatingProductAdjustmentsT.Date Is Not Null THEN CreatingProductAdjustmentsT.Date ELSE OrderDetailsT.DateReceived END AS DateReceived, "
				"AllocationDetailNotes, "
				" "
				"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN 3 "
				"WHEN ProductItemsT.Deleted = 1 THEN 3 "
				"WHEN LineItemT.ID Is Not Null THEN 2 "
				"WHEN CaseHistoryDetailsT.ID Is Not Null THEN 2 "
				"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
				"	(CASE WHEN AllocatedItemsQ.Status = 2 THEN 2 ELSE 1 END) "
				"ELSE 0 END AS StatusType, "
				" "
				"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN 'Returned' "
				"WHEN ProductItemsT.Deleted = 1 THEN CASE WHEN ProductAdjustmentCategoriesT.Name Is Null THEN 'Adjusted' "
				"	ELSE 'Adjusted - ' + ProductAdjustmentCategoriesT.Name END "
				"WHEN LineItemT.ID Is Not Null THEN 'Billed' "
				"WHEN CaseHistoryDetailsT.ID Is Not Null THEN 'Case History' "
				"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
				"	(CASE WHEN AllocatedItemsQ.Status = 2 THEN 'Used & Not Billed' ELSE 'Allocated' END) "
				"ELSE 'Available' END AS Status, "
				" "
				"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN NULL "
				"WHEN ProductItemsT.Deleted = 1 THEN NULL "
				"WHEN LineItemT.ID Is Not Null THEN ChargedPersonT.ID "
				"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CasePersonT.ID "
				"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedPersonT.ID "
				"ELSE NULL END AS PatID, "
				" "
				"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN '' "
				"WHEN ProductItemsT.Deleted = 1 THEN '' "
				"WHEN LineItemT.ID Is Not Null THEN ChargedPersonT.Last + ', ' + ChargedPersonT.First + ' ' + ChargedPersonT.Middle "
				"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CasePersonT.Last + ', ' + CasePersonT.First + ' ' + CasePersonT.Middle "
				"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedPersonT.Last + ', ' + AllocatedPersonT.First + ' ' + AllocatedPersonT.Middle "
				"ELSE '' END AS PatientName, "
				" "
				"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN SupplierReturnGroupsT.ReturnDate "
				"WHEN ProductItemsT.Deleted = 1 THEN ProductAdjustmentsT.Date "
				"WHEN LineItemT.ID Is Not Null THEN LineItemT.Date "
				"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CaseHistoryT.SurgeryDate "
				"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
				"	(CASE WHEN AllocatedItemsQ.Status = 2 THEN AllocatedItemsQ.CompletionDate ELSE AllocatedItemsQ.InputDate END) "
				"ELSE NULL END AS DateUsed, "

				"CASE WHEN ChargesT.ID Is Not Null THEN ChargesT.DoctorsProviders "
				"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CaseHistoryT.ProviderID "
				"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedItemsQ.ProviderID "
				"ELSE NULL END AS ProvID, "
				"CASE WHEN ChargesT.ID Is Not Null THEN ChargeProviderPersonT.Last + ', ' + ChargeProviderPersonT.First + ' ' + ChargeProviderPersonT.Middle "
				"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CaseProviderPersonT.Last + ', ' + CaseProviderPersonT.First + ' ' + CaseProviderPersonT.Middle "
				"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedItemsQ.ProviderName "
				"ELSE NULL END AS ProvName, "
				"OrderT.Date AS DateOrdered, "
				"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.Paid ELSE NULL END AS ConsignmentPaid, "
				"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.PaidDate ELSE NULL END AS ConsignmentPaidDate, "
				"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.PaidAmount ELSE NULL END AS ConsignmentPaidAmount, "
				"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.InvoiceNum ELSE '' END AS ConsignmentInvoiceNum "

				"FROM ProductItemsT "
				"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
				"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
				"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
				"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
				"LEFT JOIN (SELECT PatientInvAllocationsT.PatientID, ProductItemID, PatientInvAllocationsT.InputDate, COALESCE(AppointmentsT.StartTime,CompletionDate) AS CompletionDate, PatientInvAllocationDetailsT.Status, "
				"		   PatientInvAllocationsT.ProviderID, ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProviderName, "
				"		   PatientInvAllocationDetailsT.Notes AS AllocationDetailNotes FROM PatientInvAllocationDetailsT "
				"		   INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
				"		   LEFT JOIN PersonT ProviderPersonT ON PatientInvAllocationsT.ProviderID = ProviderPersonT.ID "
				"		   LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
				"		   WHERE ((PatientInvAllocationDetailsT.Status = 1 AND PatientInvAllocationsT.Status = 1) OR (PatientInvAllocationDetailsT.Status = 2 AND PatientInvAllocationsT.Status = 2)) "
				"		   ) AS AllocatedItemsQ ON ProductItemsT.ID = AllocatedItemsQ.ProductItemID "
				"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
				"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "
				"LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
				"LEFT JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
				"LEFT JOIN PersonT AS AllocatedPersonT ON AllocatedItemsQ.PatientID = AllocatedPersonT.ID "
				"LEFT JOIN PersonT AS ChargedPersonT ON LineItemT.PatientID = ChargedPersonT.ID "
				"LEFT JOIN PersonT AS CasePersonT ON CaseHistoryT.PersonID = CasePersonT.ID "
				"LEFT JOIN PersonT AS ChargeProviderPersonT ON ChargesT.DoctorsProviders = ChargeProviderPersonT.ID "
				"LEFT JOIN PersonT AS CaseProviderPersonT ON CaseHistoryT.ProviderID = CaseProviderPersonT.ID "
				"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
				"LEFT JOIN ProductAdjustmentCategoriesT ON ProductAdjustmentsT.ProductAdjustmentCategoryID = ProductAdjustmentCategoriesT.ID "
				"LEFT JOIN OrderDetailsT ON ProductItemsT.OrderDetailID = OrderDetailsT.ID "
				"LEFT JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
				"LEFT JOIN ProductAdjustmentsT CreatingProductAdjustmentsT ON ProductItemsT.CreatingAdjustmentID = CreatingProductAdjustmentsT.ID "
				"WHERE ProductItemsT.ReturnedFrom Is Null AND ServiceT.Active = 1 %s ",
				InvUtils::pisConsignment, InvUtils::pisWarranty,
				InvUtils::pisConsignment, InvUtils::pisConsignment, InvUtils::pisConsignment, InvUtils::pisConsignment,
				strPreviewWhere));
			}
			break;

		case 619:	// Allocation List
			{
				/*	Version History
					(c.haag 2007-11-20 17:48) - PLID 28084 - Created
					// (j.jones 2008-03-05 14:35) - PLID 29201 - changed the provider filter to be the
					// allocation provider, not the G1 provider, also added the provider name
					// (j.jones 2008-05-19 11:17) - PLID 29492 - the "completed & billed" status
					// now takes into account whether the allocation is linked to a billed case history
					TES 5/27/2008 - PLID 29459 - Updated to use the linked appointment date (if any) as the "Status Date"
						for Allocations, instead of the allocation's CompletedDate (which is still used if there is no linked
						appointment)
					// (j.jones 2008-06-02 16:16) - PLID 28076 - adjusted products now use the adjustment date,
					// but it can be null if it is old data prior to when we started tracking adjustment IDs
					//TES 7/18/2008 - PLID 29478 - Changed to reflect the new "To Be Ordered" status.
					// (j.jones 2009-08-06 09:50) - PLID 35120 - added BilledCaseHistoriesT
				*/
				return _T("SELECT AllocationID, PatientID AS PatID, LocationID AS LocID, "
				"AppointmentID AS ApptID, AllocationNotes, AllocationStatus, InputDate AS Date, "
				"AllocationDetailID, ProductID, ProductItemID, Quantity, AllocationDetailStatus, "
				"AllocationDetailNotes, ProductName, SerialNum, ExpDate, UserDefinedID, FullName, "
				"LocationName AS PersonLocation, AllocationListSubQ.ProviderID AS ProvID, AllocationListSubQ.StartTime AS StartTime, "
				"EndTime, ApptType, ApptPurpose, AllocationListSubQ.StatusType AS StatusType, StatusText, "
				"AllocationListSubQ.AllocationStatusDate AS AllocationStatusDate, "
				"AllocationListSubQ.DetailStatusType AS DetailStatusType, "
				"AllocationListSubQ.DetailStatusText AS DetailStatusText, "
				"Coalesce(AllocationListSubQ.ProviderName, '') AS ProvName "

				"FROM (SELECT "
				"PatientInvAllocationsT.ID AS AllocationID, "
				"PatientInvAllocationsT.PatientID , "
				"PatientInvAllocationsT.LocationID, "
				"PatientInvAllocationsT.AppointmentID, "
				"PatientInvAllocationsT.Notes AS AllocationNotes, "
				"PatientInvAllocationsT.Status AS AllocationStatus, "
				"PatientInvAllocationsT.InputDate, "
				"PatientInvAllocationsT.ProviderID, "

				"PatientInvAllocationDetailsT.ID AS AllocationDetailID, "
				"PatientInvAllocationDetailsT.ProductID, "
				"PatientInvAllocationDetailsT.ProductItemID, "
				"PatientInvAllocationDetailsT.Quantity, "
				"PatientInvAllocationDetailsT.Status AS AllocationDetailStatus, "
				"PatientInvAllocationDetailsT.Notes AS AllocationDetailNotes, "

				"ServiceT.Name AS ProductName, "
				"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "

				"PatientsT.UserDefinedID, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
				"LocationsT.Name AS LocationName, "

				"ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProviderName, "

				"AppointmentsT.StartTime, AppointmentsT.EndTime, "
				"AptTypeT.Name AS ApptType, "
				"dbo.GetPurposeString(AppointmentsT.ID) AS ApptPurpose, "

				/* (c.haag 2008-01-08 16:15) - This is the granular version */
				"CASE WHEN (BilledCaseHistoryQ.ID Is Not Null AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + ") OR "
				"PatientInvAllocationDetailsT.ID IN (SELECT AllocationDetailID FROM ChargedAllocationDetailsT) THEN 1 "
				"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " THEN 2 "
				"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsReleased) + " THEN 3 "
				"ELSE 0 END AS DetailStatusType, "

				"CASE WHEN (BilledCaseHistoryQ.ID Is Not Null AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + ") OR "
				"PatientInvAllocationDetailsT.ID IN (SELECT AllocationDetailID FROM ChargedAllocationDetailsT) THEN 'Completed & Billed' "
				"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " THEN 'Completed & Not Billed' "
				"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsReleased) + " THEN 'Released' "
				"ELSE 'Active' END AS DetailStatusText, "
				
				/* (c.haag 2008-01-08 16:15) - This is the "consistent with the Allocations tab" version */
				"CASE WHEN PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " AND (PatientInvAllocationDetailsT.Status IS NULL OR PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " OR PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsOrder) + ") THEN 0 "
				"WHEN "
				//is linked to a billed case history
				"BilledCaseHistoryQ.ID Is Not Null OR "
				//has no unbilled products that are billable
				"(PatientInvAllocationsT.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.Status = " + AsString((long)InvUtils::iadsUsed) + " "
				"			AND InvAllocDetailQ.ID NOT IN (SELECT ChargeQ.AllocationDetailID FROM ChargedAllocationDetailsT ChargeQ "
				"			WHERE ChargeQ.ChargeID IN (SELECT LineItemQ.ID FROM LineItemT LineItemQ WHERE LineItemQ.Deleted = 0)) "
				"			AND InvAllocDetailQ.ProductID IN (SELECT ProductLocQ.ProductID FROM ProductLocationInfoT ProductLocQ WHERE ProductLocQ.Billable = 1 AND ProductLocQ.LocationID = PatientInvAllocationsT.LocationID)) "
				//does not have only released products
				"AND PatientInvAllocationsT.ID NOT IN (SELECT InvAllocQ.ID FROM PatientInvAllocationsT InvAllocQ WHERE InvAllocQ.Status = " + AsString((long)InvUtils::iasCompleted) + " "
				"			AND InvAllocQ.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.Status = " + AsString((long)InvUtils::iadsUsed) + "))"
				//does not have only non-billable products
				"AND PatientInvAllocationsT.ID NOT IN (SELECT InvAllocQ.ID FROM PatientInvAllocationsT InvAllocQ WHERE InvAllocQ.Status = " + AsString((long)InvUtils::iasCompleted) + " "
				"			AND InvAllocQ.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.ProductID IN "
				"				(SELECT ProductLocQ.ProductID FROM ProductLocationInfoT ProductLocQ WHERE ProductLocQ.Billable = 1 AND ProductLocQ.LocationID = PatientInvAllocationsT.LocationID))) "
				") "
				"THEN 1 "
				"ELSE 2 END AS StatusType, "

				"CASE WHEN PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " AND (PatientInvAllocationDetailsT.Status IS NULL OR PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + ") THEN 'Active' "
				"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsOrder) + " THEN 'To Be Ordered' "
				"WHEN "
				//is linked to a billed case history
				"BilledCaseHistoryQ.ID Is Not Null OR "
				//has no unbilled products that are billable
				"(PatientInvAllocationsT.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.Status = " + AsString((long)InvUtils::iadsUsed) + " "
				"			AND InvAllocDetailQ.ID NOT IN (SELECT ChargeQ.AllocationDetailID FROM ChargedAllocationDetailsT ChargeQ "
				"			WHERE ChargeQ.ChargeID IN (SELECT LineItemQ.ID FROM LineItemT LineItemQ WHERE LineItemQ.Deleted = 0)) "
				"			AND InvAllocDetailQ.ProductID IN (SELECT ProductLocQ.ProductID FROM ProductLocationInfoT ProductLocQ WHERE ProductLocQ.Billable = 1 AND ProductLocQ.LocationID = PatientInvAllocationsT.LocationID)) "
				//does not have only released products
				"AND PatientInvAllocationsT.ID NOT IN (SELECT InvAllocQ.ID FROM PatientInvAllocationsT InvAllocQ WHERE InvAllocQ.Status = " + AsString((long)InvUtils::iasCompleted) + " "
				"			AND InvAllocQ.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.Status = " + AsString((long)InvUtils::iadsUsed) + ")) "
				//does not have only non-billable products
				"AND PatientInvAllocationsT.ID NOT IN (SELECT InvAllocQ.ID FROM PatientInvAllocationsT InvAllocQ WHERE InvAllocQ.Status = " + AsString((long)InvUtils::iasCompleted) + " "
				"			AND InvAllocQ.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.ProductID IN "
				"				(SELECT ProductLocQ.ProductID FROM ProductLocationInfoT ProductLocQ WHERE ProductLocQ.Billable = 1 AND ProductLocQ.LocationID = PatientInvAllocationsT.LocationID))) "
				") "
				"THEN 'Completed & Billed' "
				"ELSE 'Completed & Not Billed' END AS StatusText, "

				"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN SupplierReturnGroupsT.ReturnDate "
				"WHEN ProductItemsT.Deleted = 1 THEN ProductAdjustmentsT.Date "
				"WHEN LineItemT.ID Is Not Null THEN LineItemT.Date "
				"WHEN BilledCaseHistoryQ.ID Is Not Null THEN BilledCaseHistoryQ.Date "
				"WHEN PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " THEN COALESCE(AppointmentsT.StartTime,PatientInvAllocationsT.CompletionDate) "
				"ELSE PatientInvAllocationsT.InputDate END AS AllocationStatusDate "

				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "				
				"LEFT JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
				"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
				"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
				"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
				"LEFT JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
				"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "				
				"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "

				"LEFT JOIN (SELECT CaseHistoryT.ID, BillsT.Date, AllocationID "
				"	FROM CaseHistoryAllocationLinkT "
				"	INNER JOIN CaseHistoryT ON CaseHistoryAllocationLinkT.CaseHistoryID = CaseHistoryT.ID "
				"	INNER JOIN BilledCaseHistoriesT ON CaseHistoryT.ID = BilledCaseHistoriesT.CaseHistoryID "
				"	INNER JOIN BillsT ON BilledCaseHistoriesT.BillID = BillsT.ID "
				"	WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND CaseHistoryT.CompletedDate Is Not Null "
				") AS BilledCaseHistoryQ ON PatientInvAllocationsT.ID = BilledCaseHistoryQ.AllocationID "

				"LEFT JOIN PersonT ProviderPersonT ON PatientInvAllocationsT.ProviderID = ProviderPersonT.ID "

				"WHERE PatientInvAllocationsT.Status <> " + AsString((long)InvUtils::iasDeleted) + " AND PatientInvAllocationDetailsT.Status <> " + AsString((long)InvUtils::iadsDeleted) + " "
				"AND (((PatientInvAllocationDetailsT.Status IN (" + AsString((long)InvUtils::iadsActive) + "," + AsString((long)InvUtils::iadsOrder) + ") AND PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + ") "
				"OR (PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + ")))) AllocationListSubQ " + strReportName;
				);
			}
			break;

		case 621:	// Consignment History by Date
			{
			/*	Version History
				(c.haag 2007-12-14 15:34) - PLID 28357 - Created.
				(c.haag 2007-12-26 12:56) - PLID 28357 - Improved support for ProductName
				TES 6/18/2008 - PLID 29578 - Updated the query now that ProductItemsT has an OrderDetailID instead
					of an OrderID.
				// (j.jones 2009-03-18 15:56) - PLID 33580 - added Consignment Paid fields to every group,
				//and added a new "paid history" grouping
			*/
			return _T(
				"SELECT StatusDate AS Date, TransferAmount, Username, StatusText, Notes, ProductName, SerialNum, ExpDate, Category, LocationName, "
				"ProductItemID, HistoryQ.ProductID AS ProductID, CategoryID, LocationID AS LocID, "
				"Paid, PaidDate, PaidAmount, PaidInvoiceNum "
				"FROM ("
				"/* Consignment <=> Non-Consignment transfer history */ "
				"SELECT ProductItemsStatusHistoryT.Date AS StatusDate, "
				"ProductItemsStatusHistoryT.Amount AS TransferAmount, "
				"UsersT.Username, "
				"CASE WHEN NewStatus = 1 THEN 'Consignment' ELSE CASE WHEN NewStatus = 2 THEN 'Warranty' ELSE 'Purchased Inv.' END END AS StatusText, "
				"ProductItemsStatusHistoryT.Notes, "
				"ServiceT.Name AS ProductName, ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"CategoriesT.Name AS Category, LocationsT.Name AS LocationName,  "
				"ProductItemsStatusHistoryT.ProductItemID AS ProductItemID, "
				"ProductItemsT.ProductID, ServiceT.Category AS CategoryID,  "
				"ProductItemsT.LocationID, "
				"ProductItemsT.Paid, ProductItemsT.PaidDate, ProductItemsT.PaidAmount, ProductItemsT.InvoiceNum AS PaidInvoiceNum "
				"FROM ProductItemsStatusHistoryT "
				"LEFT JOIN ProductItemsT ON ProductItemsT.ID = ProductItemsStatusHistoryT.ProductItemID "
				"LEFT JOIN ServiceT ON ServiceT.ID = ProductItemsT.ProductID "
				"LEFT JOIN CategoriesT ON CategoriesT.ID = ServiceT.Category "
				"LEFT JOIN LocationsT ON LocationsT.ID = ProductItemsT.LocationID "
				"LEFT JOIN UsersT ON UsersT.PersonID = ProductItemsStatusHistoryT.UserID "
				"WHERE ProductItemsT.Status = 1 AND ProductItemsT.Deleted = 0 "
				"UNION "
				"/* Order arrival history */ "
				"SELECT OrderDetailsT.DateReceived AS StatusDate, OrderDetailsT.Amount, NULL AS Username, "
				"'Order Arrived' AS StatusText, "
				"OrderT.Notes AS Notes, "
				"ServiceT.Name AS ProductName, ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"CategoriesT.Name AS Category, LocationsT.Name AS LocationName,  "
				"ProductItemsT.ID AS ProductItemID, "
				"ProductItemsT.ProductID, ServiceT.Category AS CategoryID,  "
				"ProductItemsT.LocationID, "
				"ProductItemsT.Paid, ProductItemsT.PaidDate, ProductItemsT.PaidAmount, ProductItemsT.InvoiceNum AS PaidInvoiceNum "
				"FROM ProductItemsT "
				"INNER JOIN OrderDetailsT ON ProductItemsT.OrderDetailID = OrderDetailsT.ID "
				"INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
				"LEFT JOIN ServiceT ON ServiceT.ID = ProductItemsT.ProductID "
				"LEFT JOIN CategoriesT ON CategoriesT.ID = ServiceT.Category "
				"LEFT JOIN LocationsT ON LocationsT.ID = ProductItemsT.LocationID "
				"WHERE ProductItemsT.Status = 1 AND ProductItemsT.Deleted = 0 AND OrderDetailsT.Deleted = 0 AND OrderT.Deleted = 0 "
				"UNION "
				"/* Allocation history */ "
				"SELECT PatientInvAllocationsT.InputDate AS StatusDate, NULL AS Amount, NULL AS Username, "
				"CASE WHEN PatientInvAllocationDetailsT.Status = 1 AND PatientInvAllocationsT.Status = 1 THEN 'Allocated' ELSE 'Used By Patient' END AS StatusText, "
				"convert(nvarchar,PatientInvAllocationsT.Notes) AS Notes, "
				"ServiceT.Name AS ProductName, ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"CategoriesT.Name AS Category, LocationsT.Name AS LocationName,  "
				"ProductItemsT.ID AS ProductItemID, "
				"ProductItemsT.ProductID, ServiceT.Category AS CategoryID,  "
				"ProductItemsT.LocationID, "
				"ProductItemsT.Paid, ProductItemsT.PaidDate, ProductItemsT.PaidAmount, ProductItemsT.InvoiceNum AS PaidInvoiceNum "
				"FROM ProductItemsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
				"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID  "
				"LEFT JOIN ServiceT ON ServiceT.ID = ProductItemsT.ProductID "
				"LEFT JOIN CategoriesT ON CategoriesT.ID = ServiceT.Category "
				"LEFT JOIN LocationsT ON LocationsT.ID = ProductItemsT.LocationID "
				"WHERE ((PatientInvAllocationDetailsT.Status = 1 AND PatientInvAllocationsT.Status = 1) OR (PatientInvAllocationDetailsT.Status = 2 AND PatientInvAllocationsT.Status = 2))  "
				"AND ProductItemsT.Status = 1 AND ProductItemsT.Deleted = 0 "
				"/* Return history */ "
				"UNION "
				"SELECT ProductAdjustmentsT.Date AS StatusDate, ProductAdjustmentsT.Amount AS Amount, "
				"UsersT.Username, CASE WHEN SupplierReturnItemsT.Completed = 0 THEN 'Pending Return' ELSE 'Returned' END AS StatusText, "
				"SupplierReturnItemsT.Notes AS Notes, "
				"ServiceT.Name AS ProductName, ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"CategoriesT.Name AS Category, LocationsT.Name AS LocationName,  "
				"ProductItemsT.ID AS ProductItemID, "
				"ProductItemsT.ProductID, ServiceT.Category AS CategoryID,  "
				"ProductItemsT.LocationID, "
				"ProductItemsT.Paid, ProductItemsT.PaidDate, ProductItemsT.PaidAmount, ProductItemsT.InvoiceNum AS PaidInvoiceNum "
				"FROM ProductItemsT "
				"INNER JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
				"INNER JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
				"INNER JOIN ProductAdjustmentsT ON SupplierReturnItemsT.ProductAdjustmentID = ProductAdjustmentsT.ID "
				"LEFT JOIN ServiceT ON ServiceT.ID = ProductItemsT.ProductID "
				"LEFT JOIN CategoriesT ON CategoriesT.ID = ServiceT.Category "
				"LEFT JOIN LocationsT ON LocationsT.ID = ProductItemsT.LocationID "
				"LEFT JOIN UsersT ON ProductAdjustmentsT.Login = UsersT.PersonID "
				"WHERE ProductItemsT.Status = 1 "
				"UNION "
				"/* Consignment Paid history */ "
				"SELECT ProductItemsT.PaidDate AS StatusDate, ProductItemsT.PaidAmount AS Amount, NULL AS Username, "
				"'Paid Consignment' AS StatusText, "
				"'' AS Notes, "
				"ServiceT.Name AS ProductName, ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"CategoriesT.Name AS Category, LocationsT.Name AS LocationName,  "
				"ProductItemsT.ID AS ProductItemID, "
				"ProductItemsT.ProductID, ServiceT.Category AS CategoryID,  "
				"ProductItemsT.LocationID, "
				"ProductItemsT.Paid, ProductItemsT.PaidDate, ProductItemsT.PaidAmount, ProductItemsT.InvoiceNum AS PaidInvoiceNum "
				"FROM ProductItemsT "
				"LEFT JOIN ServiceT ON ServiceT.ID = ProductItemsT.ProductID "
				"LEFT JOIN CategoriesT ON CategoriesT.ID = ServiceT.Category "
				"LEFT JOIN LocationsT ON LocationsT.ID = ProductItemsT.LocationID "
				"WHERE ProductItemsT.Status = 1 AND ProductItemsT.Deleted = 0 AND ProductItemsT.Paid = 1 "
				") HistoryQ ");
			}
			break;

		case 623:	//Physical Inventory - Serialized - Tally Sheet
			{
			/*	Version History
				DRT 1/8/2008 - PLID 28477 - Created.  Note that this *mostly* matches the "Show Serialized Items" button in Inv, 
					but this list includes Allocated (not yet used) items.
				DRT 2/7/2008 - PLID 28854 - Included 'is allocated' flag so the report can show a count of "available" in addition
					to the full "On hand" value.  Note that at the present time, interaction b/t Allocation and Case Histories is
					undefined, so the "is allocated" field ignores case histories altogether.
				// (j.jones 2008-02-08 09:05) - PLID 28865 - supported ConsignmentReorderQuantity and ConsignmentReorderPoint
				// (j.jones 2008-02-21 09:43) - PLID 28852 - ConsignmentReorderQuantity was removed
				// (j.jones 2008-02-22 09:12) - PLID 28951 - This query needed to return totals, such that the report
				// can properly generate totals per product and also per location. The report could not previously do this
				// on its own, because it cannot total existing totals, when those totals are calculations as is used for
				// the "Needed" column. Thus we need to give the first level of totals, per product, in the returned query.
				// (j.jones 2008-03-10 08:58) - PLID 29235 - changed the 'avail' calculation to account for case histories,
				// such that case history totals that aren't accounted for by an allocation will always decrement from
				// General stock, never from Consignment
			*/

			CString strBaseFromClause;
			//this base from clause is used to find all product items and related data
			strBaseFromClause.Format("ProductItemsT "
				"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
				"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
				"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
				"LEFT JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID AND ProductItemsT.LocationID = ProductLocationInfoT.LocationID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
				"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
				"LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
				"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
				"LEFT JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
				"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID ");

			CString strBaseWhereClause;
			//this base where clause is used to exclude product items that have been returned, adjusted,
			//used on a case history or bill, or used on a completed allocation
			// (j.jones 2014-11-10 14:31) - PLID 64105 - converted allocations to use joins instead of a NOT IN clause
			strBaseWhereClause.Format(""
				"ChargesT.ID IS NULL 			/*Only show those not charged*/ "
				"AND CaseHistoryDetailsT.ID IS NULL 		/*Completed case histories*/ "
				"AND SupplierReturnitemsT.ID IS NULL		/*Not returned to supplier*/ "
				"AND ProductItemsT.Deleted = 0			/*Remove deleted*/ "
				" "
				"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
				"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
				"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
				"	AND PatientInvAllocationsT.Status <> %li "
				"		AND (PatientInvAllocationDetailsT.Status = %li)) /* Only show those not in completed allocations */ "
				" "
				, InvUtils::iasDeleted, InvUtils::iadsUsed);

			CString strFinalQuery;
			//this final query will pull the individual product item results from the base from + base where clauses,
			//but also total that information and return the totals in the same query, per product item
			strFinalQuery.Format("SELECT ProductItemsT.ID, ProductT.ID AS ProductID, ServiceT.Name AS ProductName, "
				"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"ProductItemsT.Status, ProductItemsT.LocationID AS LocID, Coalesce(LocationsT.Name, '') AS LocName, "
				"CategoriesT.Name AS CategoryName, CategoriesT.ID AS CategoryID, PersonT.ID AS SupplierID, ServiceT.BarCode, "
				"ProductT.UnitDesc, PersonT.Company AS SupplierName, ProductLocationInfoT.ReorderPoint, ProductLocationInfoT.ReorderQuantity, "
				"ProductLocationInfoT.ConsignmentReorderPoint, "
				"CASE WHEN ProductItemsT.ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE Status = 1) THEN 1 ELSE 0 END AS IsAllocated, "
				"TotalCountGeneralByProduct, TotalCountConsignmentByProduct, CountAllocatedGeneralByProduct, CountAllocatedConsignmentByProduct, "
				"Convert(int,UnRelievedCaseHistQuantity) AS CaseHistoryOnHold, Coalesce(CountAllocatedGeneralByProduct,0) + Convert(int,Coalesce(UnRelievedCaseHistQuantity,0)) AS CountAvailableGeneralByProduct "
				""
				"FROM %s "

				//now re-join the same clause again, but totalled
				"LEFT JOIN (SELECT "
				"Sum(CASE WHEN Status = 1 THEN 0 ELSE 1 END) AS TotalCountGeneralByProduct, "
				"Sum(CASE WHEN Status = 1 THEN 1 ELSE 0 END) AS TotalCountConsignmentByProduct, "
				"Sum(CASE WHEN Status = 1 THEN 0 ELSE IsAllocated END) AS CountAllocatedGeneralByProduct, "
				"Sum(CASE WHEN Status = 1 THEN IsAllocated ELSE 0 END) AS CountAllocatedConsignmentByProduct, "
				"ProductID, LocationID FROM ("
				"	SELECT ProductItemsT.Status, ProductItemsT.ProductID, ProductItemsT.LocationID, "
				"	CASE WHEN ProductItemsT.ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE Status = 1) THEN 1 ELSE 0 END AS IsAllocated "
				"	FROM %s "
				"	WHERE %s "
				"	) AS ProductTotalsSubQ "
				"	GROUP BY ProductID, LocationID) AS ProductTotalsQ "
				"ON ProductLocationInfoT.ProductID = ProductTotalsQ.ProductID "
				"AND ProductLocationInfoT.LocationID = ProductTotalsQ.LocationID "

				/* calculate products on hold by uncompleted case histories */
				/* if there is a linked allocation with the same product, do not use the case history
				amount unless it is greater, in which case we use the overage*/
				"LEFT JOIN "
				"(SELECT ItemID, Sum(CaseHistoryQty) AS UnRelievedCaseHistQuantity, LocationID FROM ("
						"SELECT (CASE WHEN SUM(Quantity) > SUM(Coalesce(ActiveAllocationQty,0)) THEN SUM(Quantity) - SUM(Coalesce(ActiveAllocationQty,0)) ELSE 0 END) AS CaseHistoryQty, ItemID, CaseHistoryT.LocationID "
						"FROM CaseHistoryDetailsT INNER JOIN "
						"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
						"LEFT JOIN (SELECT Sum(Quantity) AS ActiveAllocationQty, ProductID, CaseHistoryID "
							"FROM PatientInvAllocationsT "
							"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
							"INNER JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
							"WHERE PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " "
							"AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " "
							"GROUP BY ProductID, CaseHistoryID) AS LinkedAllocationQ ON CaseHistoryDetailsT.CaseHistoryID = LinkedAllocationQ.CaseHistoryID AND CaseHistoryDetailsT.ItemID = LinkedAllocationQ.ProductID "
						"WHERE ItemType = -1 AND CompletedDate Is Null "
						"GROUP BY ItemID, CaseHistoryT.ID, LocationID "
					") AS CaseHistoryIndivSubQ "
					"GROUP BY ItemID, LocationID "
				") AS UnRelievedCaseHistSubQ "
				"ON ProductLocationInfoT.ProductID = UnRelievedCaseHistSubQ.ItemID "
				"AND ProductLocationInfoT.LocationID = UnRelievedCaseHistSubQ.LocationID "

				"WHERE %s ", strBaseFromClause, strBaseFromClause, strBaseWhereClause, strBaseWhereClause);

			return _T(strFinalQuery);
			}
			break;

		case 655:	//Consignment Turn Rate by Month
			{
			/*	Version History
				(d.thompson 2008-12-15) - PLID 32426 - Created.  Based off the Consignment History by Date report.  However we
					only need look at the "used" items.				
			*/
			return _T(
				"SELECT PatientInvAllocationsT.InputDate AS StatusDate, "
				"CASE WHEN PatientInvAllocationDetailsT.Status = 1 AND PatientInvAllocationsT.Status = 1 THEN 'Allocated' ELSE 'Used By Patient' END AS StatusText, "
				"convert(nvarchar,PatientInvAllocationsT.Notes) AS Notes, ProductItemsT.ProductID AS ProductID, ServiceT.Name AS ProductName, ProductItemsT.SerialNum,  "
				"ProductItemsT.ExpDate, ServiceT.Category AS CategoryID, CategoriesT.Name AS Category, ProductItemsT.LocationID AS LocID, LocationsT.Name AS LocationName,  "
				"ProductItemsT.ID AS ProductItemID, ProductLocationInfoT.ConsignmentReorderPoint "
				" "
				"FROM ProductItemsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
				"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID  "
				"LEFT JOIN ServiceT ON ServiceT.ID = ProductItemsT.ProductID "
				"LEFT JOIN CategoriesT ON CategoriesT.ID = ServiceT.Category "
				"LEFT JOIN LocationsT ON LocationsT.ID = ProductItemsT.LocationID "
				"LEFT JOIN ProductLocationInfoT ON ServiceT.ID = ProductLocationInfoT.ProductID AND ProductLocationInfoT.LocationID = ProductItemsT.LocationID "
				" "
				"WHERE /*Detials and Alloc of 1 means an allocation, otherwise it's 'Used'*/ (PatientInvAllocationDetailsT.Status <> 1 OR PatientInvAllocationsT.Status <> 1) "
				"AND ProductItemsT.Deleted = 0 "
				"AND ProductLocationInfoT.ConsignmentReorderPoint <> 0");
			}
			break;
		case 727:
			/* Version History
			   j.fouts 2012-05-16 - PLID 50211 - Created.
			*/
			return _T("SELECT CASE WHEN PersonT.ID IS NULL THEN 'Available' ELSE PersonT.First + ' ' + PersonT.Last END AS Name, "
						"ServiceT.Name AS InventoryItem, ProductRequestsT.ActualStart AS OutSince, "
						"(CASE WHEN ProductRequestsT.Indefinite = 1 THEN NULL ELSE ProductRequestsT.RequestTo END) AS DueBackBy, "
						"ProductT.Notes AS Notes, ProductT.UnitDesc AS UnitDesc "
						"FROM ProductT "
						"LEFT JOIN (ProductRequestsT "
						"INNER JOIN PersonT ON ProductRequestsT.Recipient = PersonT.ID) ON ProductT.ID = ProductRequestsT.ProductID "
						"AND ProductRequestsT.ActualEnd IS NULL "
						"AND ProductRequestsT.ActualStart IS NOT NULL "
						"INNER JOIN ServiceT ON ServiceT.ID = ProductT.ID "
						"INNER JOIN ProductRequestableT "
						"ON ProductRequestableT.ProductID = ProductT.ID	 "
						"WHERE ProductRequestableT.IsRequestable = 1");
			break;

		case 728:
			/* Version History
			   j.fouts 2012-05-16 - PLID 50211 - Created.
			*/
			return _T("SELECT CategoriesT.Name AS Category, "
						"CASE WHEN PersonT.ID IS NULL THEN 'Available' ELSE PersonT.First + ' ' + PersonT.Last END AS Satus, "
						"ServiceT.Name AS Name, ProductRequestsT.ActualStart AS OutSince, "
						"(CASE WHEN ProductRequestsT.Indefinite = 1 THEN NULL ELSE ProductRequestsT.RequestTo END) AS DueBackBy, "
						"ProductT.Notes AS Notes, ProductT.UnitDesc AS UnitDesc "
						"FROM ProductT "
						"INNER JOIN ServiceT ON ServiceT.ID = ProductT.ID "
						"INNER JOIN ProductRequestableT ON ProductRequestableT.ProductID = ProductT.ID "
						"LEFT JOIN (ProductRequestsT "
						"INNER JOIN PersonT ON ProductRequestsT.Recipient = PersonT.ID) ON ProductT.ID = ProductRequestsT.ProductID "
						"AND ProductRequestsT.ActualEnd IS NULL "
						"AND ProductRequestsT.ActualStart IS NOT NULL "
						"LEFT JOIN CategoriesT ON CategoriesT.ID = ServiceT.Category "
						"WHERE ProductRequestableT.IsRequestable = 1");
			break;
		default:
		return _T("");
		break;
	}
}
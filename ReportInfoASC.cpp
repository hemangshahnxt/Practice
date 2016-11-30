////////////////
// JMJ 3/30/2004 - GetSqlASC() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "InvUtils.h"

CString CReportInfo::GetSqlASC(long nSubLevel, long nSubRepNum) const
{
	switch (nID) {

	case 341: {
		//Case History
		/*	Version History
			DRT 3/17/03 - Created from the individual report.  The report file is still called CaseHistorySingle, but it's used
					in both places.
					NOTE:  There is a special case handled in ExternalForm.cpp that handles the filtering.  It's quite shady, 
					but I can't really find a better way to do it without rewriting the entire way the ExternalForm works.
			
			TES 7/17/03 - Added filter on completed vs. incompleted.

			JMJ 8/3/2006 - PLID 21561 - added location fields

			TES 5/9/2007 - PLID 29494 - Changed the alias for this report from CaseHistoryQ to AliasQ, having it as CaseHistoryQ
					was leading to an ASSERTion when filtering because CaseHistoryQ appears in the query.

			// (j.jones 2008-07-01 11:44) - PLID 30581 - made the Cost field be NULL for personnel if they do not have
			// read permission for the contact cost

			// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
			// (j.jones 2011-03-28 16:38) - PLID 42575 - ensured we referenced CaseHistoryDetailsT.Billable
		*/

		BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

		CString strSql;
		strSql.Format("SELECT "
			"CaseHistoryQ.ID AS CaseID, "
			"CaseHistoryQ.Name AS SurgName, "
			"CaseHistoryQ.PersonID AS PatID, "
			"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = CaseHistoryQ.PersonID) AS PatName, "
			"CaseHistoryQ.ProviderID AS ProvID, "
			"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = CaseHistoryQ.ProviderID) AS ProvName, "
			"dbo.GetCaseHistoryProcedures(CaseHistoryQ.ID) AS ProcedureName, "
			"CaseHistoryQ.SurgeryDate AS SurgDate, "
			"CaseHistoryQ.Notes, "
			"(CASE WHEN ItemType = -1 THEN (SELECT Name FROM ServiceT WHERE ServiceT.ID = ItemID) ELSE CASE WHEN ItemType = -2 THEN "
			"(SELECT Code + ' - ' + Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = ItemID) ELSE "
			"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = ItemID) END END) AS ItemName, "
			"(CASE WHEN ItemType = -1 THEN 'Inventory' ELSE CASE WHEN ItemType = -2 THEN 'Service' ELSE 'Personnel' END END) AS ItemTypeName, "
			"(CASE WHEN ((ItemType = -1 OR ItemType = -2) AND CaseHistoryDetailsT.Billable = 1) THEN CaseHistoryDetailsT.Amount ELSE CONVERT(MONEY, 0.00) END) AS Amount, "
			"CASE WHEN ItemType = -3 THEN %s ELSE CaseHistoryDetailsT.Cost END AS Cost, "
			"CaseHistoryDetailsT.Quantity, "
			"CaseHistoryQ.Completed AS CompletedVal, "
			"LocationsT.ID AS LocID, LocationsT.Name AS LocationName, "
			"Convert(int, %s) AS HideTotalCost "
			"FROM (SELECT CaseHistoryT.*, CASE WHEN CaseHistoryT.CompletedDate Is Null THEN 0 ELSE 1 END AS Completed FROM CaseHistoryT) AS CaseHistoryQ "
			"INNER JOIN CaseHistoryDetailsT ON CaseHistoryQ.ID = CaseHistoryDetailsT.CaseHistoryID "
			"LEFT JOIN LocationsT ON CaseHistoryQ.LocationID = LocationsT.ID ",
			bCanViewPersonCosts ? "CaseHistoryDetailsT.Cost" : "NULL",
			bCanViewPersonCosts ? "0" : "CASE WHEN ItemType = -3 THEN 1 ELSE 0 END");
		
		return strSql;
		break;
	}

	case 387: {
		//Individual Case Histories
		/*  Version History
			DRT 4/9/03 - Created, basically copied from the Case History report.  This is now what is printed
			from the Patients module.  This uses the same special handling as the Case History report.
			TES 7/17/03 - Added filter on completed vs. incompleted.
			JMM - Added durations in durations of items
			JMJ 8/3/2006 - PLID 21562 - added location fields
			// (j.jones 2008-07-01 14:19) - PLID 30581 - made the Cost field be NULL for personnel if they do not have
			// read permission for the contact cost
			// (j.gruber 2008-07-08 16:26) - PLID 30645 - added preop, post op diag codes, anesthesia type, title, and an
			// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
			// (j.jones 2011-03-28 16:38) - PLID 42575 - ensured we referenced CaseHistoryDetailsT.Billable
			// (d.thompson 2014-03-27) - PLID 61270 - Added fields for ICD-10
		*/

		BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

		CString strSql;
		strSql.Format("SELECT "
			"CaseHistoryQ.ID AS CaseID, "
			"CaseHistoryQ.Name AS SurgName, "
			"CaseHistoryQ.PersonID AS PatID, "
			"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = CaseHistoryQ.PersonID) AS PatName, "
			"CaseHistoryQ.ProviderID AS ProvID, "
			"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = CaseHistoryQ.ProviderID) AS ProvName, "
			"dbo.GetCaseHistoryProcedures(CaseHistoryQ.ID) AS ProcedureName, "
			"CaseHistoryQ.SurgeryDate AS SurgDate, "
			"CaseHistoryQ.Notes, "
			"(CASE WHEN ItemType = -1 THEN (SELECT Name FROM ServiceT WHERE ServiceT.ID = ItemID) ELSE CASE WHEN ItemType = -2 THEN "
			"(SELECT Code + ' - ' + Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = ItemID) ELSE "
			"(SELECT First + ' ' + Last + ', ' + Title FROM PersonT WHERE ID = ItemID) END END) AS ItemName, "
			"(CASE WHEN ItemType = -1 THEN 'Inventory' ELSE CASE WHEN ItemType = -2 THEN 'Service' ELSE 'Personnel' END END) AS ItemTypeName, "
			"(CASE WHEN ((ItemType = -1 OR ItemType = -2) AND CaseHistoryDetailsT.Billable = 1) THEN CaseHistoryDetailsT.Amount ELSE CONVERT(MONEY, 0.00) END) AS Amount, "
			"CASE WHEN ItemType = -3 THEN %s ELSE CaseHistoryDetailsT.Cost END AS Cost, "
			"CaseHistoryDetailsT.Quantity, "
			"CaseHistoryQ.LogPreopIn, CaseHistoryQ.LogPreopOut, "
			"CaseHistoryQ.LogOpRoomIn, CaseHistoryQ.LogOpRoomOut, "
			"CaseHistoryQ.LogRecoveryIn, CaseHistoryQ.LogRecoveryOut, "
			"CaseHistoryQ.LogAnesthesiaIn, CaseHistoryQ.LogAnesthesiaOut, "
			"CaseHistoryQ.LogFacilityIn, CaseHistoryQ.LogFacilityOut, "
			"CaseHistoryQ.Completed AS CompletedVal, "
			"DateDiff(mi, LogPreopIn, LogPreopOut) As PreOpDuration, "
			"DateDiff(mi, LogOpRoomIn, LogOpRoomOut) As OpRoomDuration, "
			"DateDiff(mi, LogAnesthesiaIn, LogAnesthesiaOut) As AnesthesiaDuration, "
			"DateDiff(mi, LogRecoveryIn, LogRecoveryOut) As RecoveryDuration, "
			"CaseHistoryQ.LogSurgeonIn, CaseHistoryQ.LogSurgeonOut, "
			"CaseHistoryQ.Log23HourRoomIn, CaseHistoryQ.Log23HourRoomOut, "
			"DateDiff(mi, LogSurgeonIn, LogSurgeonOut) As SurgeonDuration, "
			"DateDiff(mi, Log23HourRoomIn, Log23HourRoomOut) As Duration23HourRoom, "
			"LocationsT.ID AS LocID, LocationsT.Name AS LocationName, "
			"Convert(int, %s) AS HideTotalCost, "
			" AnesType, "
			" DiagPreDx1.CodeNumber as ICD9PreDx1Code, DiagPreDx1.CodeDesc as ICD9PreDx1Desc, "
			" DiagPreDx2.CodeNumber as ICD9PreDx2Code, DiagPreDx2.CodeDesc as ICD9PreDx2Desc, "
			" DiagPreDx3.CodeNumber as ICD9PreDx3Code, DiagPreDx3.CodeDesc as ICD9PreDx3Desc, "
			" DiagPreDx4.CodeNumber as ICD9PreDx4Code, DiagPreDx4.CodeDesc as ICD9PreDx4Desc, "
			" DiagPostDx1.CodeNumber as ICD9PostDx1Code, DiagPostDx1.CodeDesc as ICD9PostDx1Desc, "
			" DiagPostDx2.CodeNumber as ICD9PostDx2Code, DiagPostDx2.CodeDesc as ICD9PostDx2Desc, "
			" DiagPostDx3.CodeNumber as ICD9PostDx3Code, DiagPostDx3.CodeDesc as ICD9PostDx3Desc, "
			" DiagPostDx4.CodeNumber as ICD9PostDx4Code, DiagPostDx4.CodeDesc as ICD9PostDx4Desc, "
			" ICD10PreDx1.CodeNumber AS ICD10PreDx1Code, ICD10PreDx1.CodeDesc AS ICD10PreDx1Desc, "
			" ICD10PreDx2.CodeNumber AS ICD10PreDx2Code, ICD10PreDx2.CodeDesc AS ICD10PreDx2Desc, "
			" ICD10PreDx3.CodeNumber AS ICD10PreDx3Code, ICD10PreDx3.CodeDesc AS ICD10PreDx3Desc, "
			" ICD10PreDx4.CodeNumber AS ICD10PreDx4Code, ICD10PreDx4.CodeDesc AS ICD10PreDx4Desc, "
			" ICD10PostDx1.CodeNumber AS ICD10PostDx1Code, ICD10PostDx1.CodeDesc AS ICD10PostDx1Desc, "
			" ICD10PostDx2.CodeNumber AS ICD10PostDx2Code, ICD10PostDx2.CodeDesc AS ICD10PostDx2Desc, "
			" ICD10PostDx3.CodeNumber AS ICD10PostDx3Code, ICD10PostDx3.CodeDesc AS ICD10PostDx3Desc, "
			" ICD10PostDx4.CodeNumber AS ICD10PostDx4Code, ICD10PostDx4.CodeDesc AS ICD10PostDx4Desc, "
			" (SELECT CASE WHEN Nurse <> 0 then 1 ELSE CASE WHEN Anesthesiologist <> 0 then 2 ELSE 0 END END FROM ContactsT WHERE CaseHistoryDetailsT.ItemID = ContactsT.PersonID AND CaseHistoryDetailsT.ItemType = -3) AS PersonType, "
			" AppointmentsT.StartTime as ApptDateTime "
			"FROM (SELECT CaseHistoryT.*, CASE WHEN CaseHistoryT.CompletedDate Is Null THEN 0 ELSE 1 END AS Completed FROM CaseHistoryT) AS CaseHistoryQ "
			"INNER JOIN CaseHistoryDetailsT ON CaseHistoryQ.ID = CaseHistoryDetailsT.CaseHistoryID "
			"LEFT JOIN LocationsT ON CaseHistoryQ.LocationID = LocationsT.ID "
			"LEFT JOIN DiagCodes DiagPreDx1 ON CaseHistoryQ.PreOpDx1 = DiagPreDx1.ID "
			"LEFT JOIN DiagCodes DiagPreDx2 ON CaseHistoryQ.PreOpDx2 = DiagPreDx2.ID "
			"LEFT JOIN DiagCodes DiagPreDx3 ON CaseHistoryQ.PreOpDx3 = DiagPreDx3.ID "
			"LEFT JOIN DiagCodes DiagPreDx4 ON CaseHistoryQ.PreOpDx4 = DiagPreDx4.ID "
			"LEFT JOIN DiagCodes DiagPostDx1 ON CaseHistoryQ.PostOpDx1 = DiagPostDx1.ID "
			"LEFT JOIN DiagCodes DiagPostDx2 ON CaseHistoryQ.PostOpDx2 = DiagPostDx2.ID "
			"LEFT JOIN DiagCodes DiagPostDx3 ON CaseHistoryQ.PostOpDx3 = DiagPostDx3.ID "
			"LEFT JOIN DiagCodes DiagPostDx4 ON CaseHistoryQ.PostOpDx4 = DiagPostDx4.ID "
			"LEFT JOIN DiagCodes ICD10PreDx1 ON CaseHistoryQ.PreOpDx1ICD10 = ICD10PreDx1.ID "
			"LEFT JOIN DiagCodes ICD10PreDx2 ON CaseHistoryQ.PreOpDx2ICD10 = ICD10PreDx2.ID "
			"LEFT JOIN DiagCodes ICD10PreDx3 ON CaseHistoryQ.PreOpDx3ICD10 = ICD10PreDx3.ID "
			"LEFT JOIN DiagCodes ICD10PreDx4 ON CaseHistoryQ.PreOpDx4ICD10 = ICD10PreDx4.ID "
			"LEFT JOIN DiagCodes ICD10PostDx1 ON CaseHistoryQ.PostOpDx1ICD10 = ICD10PostDx1.ID "
			"LEFT JOIN DiagCodes ICD10PostDx2 ON CaseHistoryQ.PostOpDx2ICD10 = ICD10PostDx2.ID "
			"LEFT JOIN DiagCodes ICD10PostDx3 ON CaseHistoryQ.PostOpDx3ICD10 = ICD10PostDx3.ID "
			"LEFT JOIN DiagCodes ICD10PostDx4 ON CaseHistoryQ.PostOpDx4ICD10 = ICD10PostDx4.ID "
			"LEFT JOIN AppointmentsT ON CaseHistoryQ.AppointmentID = AppointmentsT.ID "			
			,
			bCanViewPersonCosts ? "CaseHistoryDetailsT.Cost" : "NULL",
			bCanViewPersonCosts ? "0" : "CASE WHEN ItemType = -3 THEN 1 ELSE 0 END");

		return strSql;
		break;
	}

	case 492:
		//Patient Flow
		/*	Version History
			JMJ - 3/30/2004 - Created
		*/
		return _T("SELECT CaseHistoryT.Name AS CaseDescription, CaseHistoryT.SurgeryDate AS TDate, "
			"AppointmentsT.Date AS ApptDate, AppointmentsT.StartTime AS StartTime, "
			"dbo.GetCaseHistoryProcedures(CaseHistoryT.ID) AS ProcedureName, "
			"CaseHistoryT.PersonID AS PatID, CaseHistoryT.ProviderID AS ProvID, "
			"CaseHistoryT.LocationID AS LocID, CaseHistoryT.CompletedDate, "
			"(CASE WHEN CaseHistoryT.CompletedDate Is Null THEN 0 ELSE 1 END) AS Completed, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"PersonProviderT.Last + ', ' + PersonProviderT.First + ' ' + PersonProviderT.Middle AS ProvName, "
			"LogPreOpIn, LogPreOpOut, LogAnesthesiaIn, LogAnesthesiaOut, LogSurgeonIn, LogSurgeonOut, "
			"LogOpRoomIn, LogOpRoomOut, LogRecoveryIn, LogRecoveryOut, Log23HourRoomIn, Log23HourRoomOut "
			"FROM CaseHistoryT "
			"LEFT JOIN AppointmentsT ON CaseHistoryT.AppointmentID = AppointmentsT.ID "
			"LEFT JOIN PersonT ON CaseHistoryT.PersonID = PersonT.ID "
			"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT PersonProviderT ON CaseHistoryT.ProviderID = PersonProviderT.ID");
		break;

	case 493:
		//Surgeries Scheduled Without A Case History
		/*	Version History
			JMJ - 3/30/2004 - Created
		*/
		return _T("SELECT AppointmentsT.Date AS TDate, AppointmentsT.StartTime, "
			"dbo.GetResourceString(AppointmentsT.ID) AS Resource, AptTypeT.Name AS Type, "
			"Coalesce(dbo.GetPurposeString(AppointmentsT.ID),'<No Purpose Selected>') AS Purpose, "
			"(CASE WHEN AppointmentsT.PatientID > 0 THEN Last + ', ' + First + ' ' + Middle ELSE '' END) AS PatName, "
			"AppointmentsT.LocationID AS LocID, LocationsT.Name AS LocName, AppointmentsT.PatientID AS PatID "
			"FROM AppointmentsT "
			"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
			"WHERE AppointmentsT.Status <> 4 " //not cancelled
			"AND AptTypeT.Category = 4 " //surgery appts. only
			"AND AppointmentsT.PatientID > 0 "
			"AND AppointmentsT.ID NOT IN (SELECT AppointmentID FROM CaseHistoryT WHERE AppointmentID Is Not Null)");
		break;

	case 494: {
		//Service Codes Credentialed By Provider
		/*	Version History
			JMJ - 3/30/2004 - Created
		*/
		CString strSql ="SELECT CPTCodeQ.CPTCodeID AS CPTID, CPTDescription, Code, SubCode, TypeOfService, StdFee, DoctorsQ.DoctorID AS ProvID, ProvName, (CASE WHEN TotalBilled IS NULL THEN 0 ELSE TotalBilled END) AS TotalBilled, "
			"(CASE WHEN CredCPTCodesQ.CPTCodeID IS Null THEN 0 ELSE 1 END) AS Credentialed "
			"FROM "
			"(SELECT ServiceT.ID AS CPTCodeID, Name AS CPTDescription, Code, SubCode, TypeOfService, Price AS StdFee FROM CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID) AS CPTCodeQ "
			"CROSS JOIN "
			"(SELECT ID AS DoctorID, Last + ', ' + First + ' ' + Middle + ' ' + Title AS ProvName FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) AS DoctorsQ "
			"LEFT JOIN (SELECT CPTCodeID, ProviderID AS DoctorID FROM CredentialedCPTCodesT) AS CredCPTCodesQ ON CPTCodeQ.CPTCodeID = CredCPTCodesQ.CPTCodeID AND DoctorsQ.DoctorID = CredCPTCodesQ.DoctorID "
			"LEFT JOIN (SELECT ServiceID, Sum(Quantity) AS TotalBilled, DoctorsProviders FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE Deleted = 0 @DateFilter "
			"GROUP BY ServiceID, DoctorsProviders) AS BilledTotalsQ ON CPTCodeQ.CPTCodeID = BilledTotalsQ.ServiceID AND DoctorsQ.DoctorID = BilledTotalsQ.DoctorsProviders ";

		if(nDateRange == 1) {
			if(nDateFilter == 1) {
				CString strDateFilter;
				COleDateTimeSpan dtSpan;
				COleDateTime dt;
				dtSpan.SetDateTimeSpan(1,0,0,0);
				dt = DateTo;
				dt += dtSpan;
				strDateFilter.Format("AND LineItemT.Date >= '%s' AND LineItemT.Date < '%s' ", DateFrom.Format("%Y-%m-%d"), dt.Format("%Y-%m-%d"));
				strSql.Replace("@DateFilter", strDateFilter);
			}
		}
		else {
			//There's no date filter
			strSql.Replace("@DateFilter", "");
		}

		return _T(strSql);
		}
		break;

	case 495:	//Supplies Below Minimum By Case History
	case 496:	//Supplies Below Minimum By Product		
		
		/*	Version History
			JMJ - 3/31/2004 - Created (mostly copied from the Products Reserved inventory report)
			// (j.jones 2007-11-28 11:25) - PLID 28196 - completed Allocations now decrement from Inventory
			// (j.jones 2007-12-17 11:11) - PLID 27988 - billed allocations do not decrement from inventory
			// (j.jones 2016-01-19 08:46) - PLID 67680 - ensured the total inventory amount was rounded
		*/
		if(this->nLocation > 0){
			//Location filters applied
			CString strSQL;
			strSQL.Format("SELECT SuppliesBelowMinQ.ID AS ProductID, SuppliesBelowMinQ.Name, SuppliesBelowMinQ.SupplierID, SuppliesBelowMinQ.CategoryID, SuppliesBelowMinQ.SupplierName, "
			"SuppliesBelowMinQ.TotalOnHand, SuppliesBelowMinQ.UnRelievedCaseHistoryQty, SuppliesBelowMinQ.AmountAvail, "
			"SuppliesBelowMinQ.ReorderPoint, SuppliesBelowMinQ.ReorderQuantity, SuppliesBelowMinQ.LocName, SuppliesBelowMinQ.LocationID AS LocID, "
			"CaseHistoryQ.Name AS CaseHistoryDescription, CaseHistoryQ.PersonID AS PatID, CaseHistoryQ.PatName, CaseHistoryQ.LocationID, CaseHistoryQ.ProviderID AS ProvID, "
			"CaseHistoryQ.ProvName, CaseHistoryQ.SurgeryDate AS TDate, CaseHistoryQ.StartTime, CaseHistoryQ.Quantity AS AmountNeeded "
			"FROM "
			"(SELECT ProductQ.ID, ProductQ.Name, ProductQ.SupplierID AS SupplierID, ProductQ.Category AS CategoryID, ProductQ.SupplierName, "

			"/* Total On Hand (physical) */ "
			"Round("
			"CASE WHEN ChargeQ.Sold IS NULL THEN 0 ELSE (-1*ChargeQ.Sold) END + CASE WHEN OrderQ.Received IS NULL THEN 0 ELSE OrderQ.Received END "
			"+ CASE WHEN AdjQ.Adjusted IS NULL THEN 0 ELSE AdjQ.Adjusted END "
			"+ CASE WHEN TransferredToQ.TransferredTo IS NULL THEN 0 ELSE TransferredToQ.TransferredTo END "
			"+ CASE WHEN TransferredFromQ.TransferredFrom IS NULL THEN 0 ELSE TransferredFromQ.TransferredFrom END "
			"+ CASE WHEN CompletedAllocationsQ.UsedAllocationQty IS NULL THEN 0 ELSE (-1*CompletedAllocationsQ.UsedAllocationQty) END "
			"+ CASE WHEN CaseHistSubQ.CaseHistoryQty IS NULL THEN 0 ELSE (-1*CaseHistSubQ.CaseHistoryQty) END "
			", 2) AS TotalOnHand, "

			"/* Incomplete Case Histories */ "
			"Round(CASE WHEN UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty END, 2) AS UnRelievedCaseHistoryQty,  "

			"/* Amount Available */ "
			"Round("
			"CASE WHEN ChargeQ.Sold IS NULL THEN 0 ELSE (-1*ChargeQ.Sold) END + CASE WHEN OrderQ.Received IS NULL THEN 0 ELSE OrderQ.Received END "
			"+ CASE WHEN AdjQ.Adjusted IS NULL THEN 0 ELSE AdjQ.Adjusted END "
			"+ CASE WHEN TransferredToQ.TransferredTo IS NULL THEN 0 ELSE TransferredToQ.TransferredTo END "
			"+ CASE WHEN TransferredFromQ.TransferredFrom IS NULL THEN 0 ELSE TransferredFromQ.TransferredFrom END "
			"+ CASE WHEN CompletedAllocationsQ.UsedAllocationQty IS NULL THEN 0 ELSE (-1*CompletedAllocationsQ.UsedAllocationQty) END "
			"+ CASE WHEN CaseHistSubQ.CaseHistoryQty IS NULL THEN 0 ELSE (-1*CaseHistSubQ.CaseHistoryQty) END -  "
			"CASE WHEN UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty END "
			", 2) AS AmountAvail, "

			"ReOrderPoint, ReOrderQuantity, ProductQ.LocName, ProductQ.LocationID "
			"FROM (  "
			"SELECT ProductT.ID, LocationsT.Name AS LocName, ProductLocationInfoT.LocationID, ServiceT.Name, UnitDesc, Billable, Price, Taxable1, Taxable2, RevCodeUse,  "
			"UB92Category, TrackableStatus, ReOrderPoint, ReorderQuantity, MultiSupplierT.SupplierID, Category, LastCost,  "
			"Barcode, ProductT.Notes, MultiSupplierT.Catalog, UseUU, UnitOfOrder, UnitOfUsage, Conversion, HasSerialNum, HasExpDate,  "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Supplier' ELSE PersonT.Company END AS SupplierName "
			"FROM ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
			"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
			"INNER JOIN LocationsT ON ProductLocationInfoT.LocationID = LocationsT.ID "
			"WHERE ProductLocationInfoT.LocationID = %li "
			") AS ProductQ "

			// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT\
			// (j.dinatale 2011-11-07 09:52) - PLID 46226 - need to exclude voiding and original line items of financial corrections
			"LEFT JOIN ( "
			"/* "
			"	ChargeQ totals all charged products, leaving out all items which are to be included in CaseHistSubQ (below) "
			"	also leaves out items from completed allocations "
			"*/ "
			"	SELECT SUM (CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS Sold, ServiceID  "
			"	FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
			"	LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
			"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
			"	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID  "
			"	LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li "
			"		GROUP BY ChargeID) AS AllocationInfoQ ON ChargesT.ID = AllocationInfoQ.ChargeID "
			"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Type = 10 AND LineItemT.LocationID = %li "
			"	AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
			"	AND (BillsT.ID NOT IN (SELECT BillID FROM BilledCaseHistoriesT) OR ChargesT.ServiceID NOT IN  "
			"	(SELECT ItemID FROM CaseHistoryDetailsT "
			"	INNER JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID "
			"	WHERE BilledCaseHistoriesT.BillID = BillsT.ID "
			"	AND ItemType = -1))  "
			"	GROUP BY ServiceID  "
			") AS ChargeQ ON ProductQ.ID = ChargeQ.ServiceID  "

			"LEFT JOIN (  "
			"	SELECT ProductID, SUM (QuantityOrdered) AS Received  "
			"	FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID  "
			"	WHERE LocationID = %li AND DateReceived IS NOT NULL  "
			"	AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
			"	GROUP BY ProductID  "
			") AS OrderQ ON ProductQ.ID = OrderQ.ProductID  "
			"LEFT JOIN (  "
			"	SELECT ProductID, SUM (QuantityOrdered) AS Ordered  "
			"	FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID  "
			"	WHERE LocationID = %li AND DateReceived IS NULL  "
			"	AND OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
			"	GROUP BY ProductID  "
			") AS OnOrderQ ON ProductQ.ID = OnOrderQ.ProductID  "
			"LEFT JOIN (  "
			"	SELECT SUM(Quantity) AS Adjusted, ProductID  "
			"	FROM ProductAdjustmentsT  "
			"	WHERE LocationID = %li "
			"	GROUP BY ProductID  "
			") AS AdjQ ON ProductQ.ID = AdjQ.ProductID  "
			"LEFT JOIN (  "
			"	SELECT SUM(Amount) AS TransferredTo, ProductID  "
			"	FROM ProductLocationTransfersT  "
			"	WHERE DestLocationID = %li "
			"	GROUP BY ProductID  "
			") AS TransferredToQ ON ProductQ.ID = TransferredToQ.ProductID  "
			"LEFT JOIN (  "
			"	SELECT -1*SUM(Amount) AS TransferredFrom, ProductID  "
			"	FROM ProductLocationTransfersT  "
			"	WHERE SourceLocationID = %li "
			"	GROUP BY ProductID  "
			") AS TransferredFromQ ON ProductQ.ID = TransferredFromQ.ProductID  "
			"/* "
			"	UnRelievedCaseHistSubQ totals the items in incompleted Case Histories. (on reserve number) "
			"*/ "
			"LEFT JOIN (  "
			"	SELECT SUM(Quantity) AS UnRelievedCaseHistoryQty, ItemID  "
			"	FROM CaseHistoryDetailsT INNER JOIN  "
			"	CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID  "
			"	WHERE ItemType = -1 AND CompletedDate Is Null AND LocationID = %li "
			"	GROUP BY ItemID  "
			") AS UnRelievedCaseHistSubQ ON ProductQ.ID = UnRelievedCaseHistSubQ.ItemID  "
			"/* "
			"	CaseHistSubQ totals the items that are included in completed Case Histories. "
			"*/ "
			"LEFT JOIN (  "
			"	SELECT SUM(Quantity) AS CaseHistoryQty, ItemID  "
			"	FROM CaseHistoryDetailsT INNER JOIN  "
			"	CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID  "
			"	WHERE ItemType = -1 AND CompletedDate Is Not Null AND LocationID = %li "
			"	GROUP BY ItemID  "
			") AS CaseHistSubQ ON ProductQ.ID = CaseHistSubQ.ItemID  "
			"LEFT JOIN ( "
				"SELECT Sum(Quantity) AS UsedAllocationQty, ProductID "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"WHERE PatientInvAllocationsT.Status = %li "
				"AND PatientInvAllocationDetailsT.Status = %li "
				"AND LocationID = %li "
				"GROUP BY ProductID "
			") AS CompletedAllocationsQ ON ProductQ.ID = CompletedAllocationsQ.ProductID "
			"LEFT JOIN CategoriesT ON ProductQ.Category = CategoriesT.ID "
			" "
			"WHERE (CASE WHEN UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty END) > 0) "
			"AS SuppliesBelowMinQ "
			"INNER JOIN "
			"	(SELECT CaseHistoryT.ID, CaseHistoryT.Name, CaseHistoryT.PersonID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS PatName, "
			"	CaseHistoryT.LocationID, CaseHistoryT.ProviderID, PersonT2.Last + ', ' + PersonT2.First + ' ' + PersonT2.Middle AS ProvName, "
			"	CaseHistoryDetailsT.ItemID, CaseHistoryT.SurgeryDate, AppointmentsT.StartTime, Sum(CaseHistoryDetailsT.Quantity) AS Quantity "
			"	FROM CaseHistoryT INNER JOIN CaseHistoryDetailsT ON CaseHistoryT.ID = CaseHistoryDetailsT.CaseHistoryID "
			"	LEFT JOIN PersonT PersonT1 ON CaseHistoryT.PersonID = PersonT1.ID "
			"	LEFT JOIN PersonT PersonT2 ON CaseHistoryT.ProviderID = PersonT2.ID "
			"	LEFT JOIN AppointmentsT ON CaseHistoryT.AppointmentID = AppointmentsT.ID "
			"	WHERE CaseHistoryT.CompletedDate Is Null AND CaseHistoryDetailsT.ItemType = -1 AND CaseHistoryT.LocationID = %li "
			"	GROUP BY CaseHistoryT.ID, CaseHistoryT.Name, CaseHistoryT.PersonID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, "
			"	CaseHistoryT.LocationID, CaseHistoryT.ProviderID, PersonT2.Last + ', ' + PersonT2.First + ' ' + PersonT2.Middle, CaseHistoryDetailsT.ItemID, CaseHistoryT.SurgeryDate, AppointmentsT.StartTime "
			"	) AS CaseHistoryQ ON SuppliesBelowMinQ.ID = CaseHistoryQ.ItemID "
			"WHERE SuppliesBelowMinQ.AmountAvail < ReorderPoint ",
			nLocation, InvUtils::iadsUsed, InvUtils::iasCompleted, nLocation, nLocation, nLocation, nLocation, nLocation, nLocation, nLocation, nLocation, nLocation, InvUtils::iasCompleted, InvUtils::iadsUsed, nLocation);

			return _T(strSQL);
		}
		else {
			//No Location filters applied
			CString strSQL;
			strSQL.Format("SELECT SuppliesBelowMinQ.ID AS ProductID, SuppliesBelowMinQ.Name, SuppliesBelowMinQ.SupplierID, SuppliesBelowMinQ.CategoryID, SuppliesBelowMinQ.SupplierName, "
			"SuppliesBelowMinQ.TotalOnHand, SuppliesBelowMinQ.UnRelievedCaseHistoryQty, SuppliesBelowMinQ.AmountAvail, "
			"SuppliesBelowMinQ.ReorderPoint, SuppliesBelowMinQ.ReorderQuantity, SuppliesBelowMinQ.LocName, SuppliesBelowMinQ.LocationID AS LocID, "
			"CaseHistoryQ.Name AS CaseHistoryDescription, CaseHistoryQ.PersonID AS PatID, CaseHistoryQ.PatName, CaseHistoryQ.LocationID, CaseHistoryQ.ProviderID AS ProvID, "
			"CaseHistoryQ.ProvName, CaseHistoryQ.SurgeryDate AS TDate, CaseHistoryQ.StartTime, CaseHistoryQ.Quantity AS AmountNeeded "
			"FROM "
			"(SELECT ProductQ.ID, ProductQ.Name, ProductQ.SupplierID AS SupplierID, ProductQ.Category AS CategoryID, ProductQ.SupplierName, "
			
			"/* Total On Hand (physical) */ "
			"Round("
			"CASE WHEN ChargeQ.Sold IS NULL THEN 0 ELSE (-1*ChargeQ.Sold) END + CASE WHEN OrderQ.Received IS NULL THEN 0 ELSE OrderQ.Received END "
			"+ CASE WHEN AdjQ.Adjusted IS NULL THEN 0 ELSE AdjQ.Adjusted END "
			"+ CASE WHEN TransferredToQ.TransferredTo IS NULL THEN 0 ELSE TransferredToQ.TransferredTo END "
			"+ CASE WHEN TransferredFromQ.TransferredFrom IS NULL THEN 0 ELSE TransferredFromQ.TransferredFrom END "
			"+ CASE WHEN CompletedAllocationsQ.UsedAllocationQty IS NULL THEN 0 ELSE (-1*CompletedAllocationsQ.UsedAllocationQty) END "
			"+ CASE WHEN CaseHistSubQ.CaseHistoryQty IS NULL THEN 0 ELSE (-1*CaseHistSubQ.CaseHistoryQty) END "
			", 2) AS TotalOnHand, "

			"/* Incomplete Case Histories */ "
			"Round(CASE WHEN UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty END, 2) AS UnRelievedCaseHistoryQty, "

			"/* Amount Available */ "
			"Round("
			"CASE WHEN ChargeQ.Sold IS NULL THEN 0 ELSE (-1*ChargeQ.Sold) END + CASE WHEN OrderQ.Received IS NULL THEN 0 ELSE OrderQ.Received END "
			"+ CASE WHEN AdjQ.Adjusted IS NULL THEN 0 ELSE AdjQ.Adjusted END "
			"+ CASE WHEN TransferredToQ.TransferredTo IS NULL THEN 0 ELSE TransferredToQ.TransferredTo END "
			"+ CASE WHEN TransferredFromQ.TransferredFrom IS NULL THEN 0 ELSE TransferredFromQ.TransferredFrom END "
			"+ CASE WHEN CompletedAllocationsQ.UsedAllocationQty IS NULL THEN 0 ELSE (-1*CompletedAllocationsQ.UsedAllocationQty) END "
			"+ CASE WHEN CaseHistSubQ.CaseHistoryQty IS NULL THEN 0 ELSE (-1*CaseHistSubQ.CaseHistoryQty) END -  "
			"CASE WHEN UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty END "
			", 2) AS AmountAvail, "
			
			"ReOrderPoint, ReOrderQuantity, ProductQ.LocName, ProductQ.LocationID "
			"FROM ( "
			"SELECT ProductT.ID, ProductLocationInfoT.LocationID, LocationsT.Name AS LocName, ServiceT.Name, UnitDesc, Price, Taxable1, Taxable2, RevCodeUse,  "
			"UB92Category, ReOrderPoint, ReorderQuantity, MultiSupplierT.SupplierID, Category, LastCost, "
			"Barcode, ProductT.Notes, MultiSupplierT.Catalog, UseUU, UnitOfOrder, UnitOfUsage, Conversion, HasSerialNum, HasExpDate, "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Supplier' ELSE PersonT.Company END AS SupplierName "
			"FROM ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
			"INNER JOIN LocationsT ON ProductLocationInfoT.LocationID = LocationsT.ID "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
			/*"WHERE ProductT.ID = [id] "*/
			") AS ProductQ "

			// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
			// (j.dinatale 2011-11-07 09:52) - PLID 46226 - need to exclude voiding and original line items of financial corrections
			"LEFT JOIN ( "
			"/* "
			"	ChargeQ totals all charged products, leaving out all items which are to be included in CaseHistSubQ (below) "
			"	also leaves out items from completed allocations "
			"*/ "
			"	SELECT SUM (CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS Sold, ServiceID, LineItemT.LocationID "
			"	FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
			"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
			"	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"	LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li "
			"		GROUP BY ChargeID) AS AllocationInfoQ ON ChargesT.ID = AllocationInfoQ.ChargeID "
			"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Type = 10 "
			"	AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
			"	AND (BillsT.ID NOT IN (SELECT BillID FROM BilledCaseHistoriesT) OR ChargesT.ServiceID NOT IN "
			"	(SELECT ItemID FROM CaseHistoryDetailsT "
			"	INNER JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID "
			"	WHERE BilledCaseHistoriesT.BillID = BillsT.ID "
			"	AND ItemType = -1)) "
			"	GROUP BY ServiceID, LineItemT.LocationID "
			") AS ChargeQ ON ProductQ.ID = ChargeQ.ServiceID AND ProductQ.LocationID = ChargeQ.LocationID "

			"LEFT JOIN ( "
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
			"/* "
			"	UnRelievedCaseHistSubQ totals the items in incompleted Case Histories. (on reserve number) "
			"*/ "
			"LEFT JOIN (  "
			"	SELECT SUM(Quantity) AS UnRelievedCaseHistoryQty, ItemID, LocationID "
			"	FROM CaseHistoryDetailsT INNER JOIN  "
			"	CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID  "
			"	WHERE ItemType = -1 AND CompletedDate Is Null "
			"	GROUP BY ItemID, LocationID "
			") AS UnRelievedCaseHistSubQ ON ProductQ.ID = UnRelievedCaseHistSubQ.ItemID AND ProductQ.LocationID = UnRelievedCaseHistSubQ.LocationID "
			"/* "
			"	CaseHistSubQ totals the items that are included in completed Case Histories. "
			"*/ "
			"LEFT JOIN (  "
			"	SELECT SUM(Quantity) AS CaseHistoryQty, ItemID, LocationID "
			"	FROM CaseHistoryDetailsT INNER JOIN  "
			"	CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID  "
			"	WHERE ItemType = -1 AND CompletedDate Is Not Null "
			"	GROUP BY ItemID, LocationID "
			") AS CaseHistSubQ ON ProductQ.ID = CaseHistSubQ.ItemID AND ProductQ.LocationID = CaseHistSubQ.LocationID "
			"LEFT JOIN ( "
				"SELECT Sum(Quantity) AS UsedAllocationQty, ProductID, LocationID "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"WHERE PatientInvAllocationsT.Status = %li "
				"AND PatientInvAllocationDetailsT.Status = %li "
				"GROUP BY ProductID, LocationID "
			") AS CompletedAllocationsQ ON ProductQ.ID = CompletedAllocationsQ.ProductID "
			"LEFT JOIN CategoriesT ON ProductQ.Category = CategoriesT.ID "
			" "
			"WHERE (CASE WHEN UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty IS NULL THEN 0 ELSE UnRelievedCaseHistSubQ.UnRelievedCaseHistoryQty END) > 0) "
			"AS SuppliesBelowMinQ "
			"INNER JOIN "
			"	(SELECT CaseHistoryT.ID, CaseHistoryT.Name, CaseHistoryT.PersonID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS PatName, "
			"	CaseHistoryT.LocationID, CaseHistoryT.ProviderID, PersonT2.Last + ', ' + PersonT2.First + ' ' + PersonT2.Middle AS ProvName, "
			"	CaseHistoryDetailsT.ItemID, CaseHistoryT.SurgeryDate, AppointmentsT.StartTime, Sum(CaseHistoryDetailsT.Quantity) AS Quantity "
			"	FROM CaseHistoryT INNER JOIN CaseHistoryDetailsT ON CaseHistoryT.ID = CaseHistoryDetailsT.CaseHistoryID "
			"	LEFT JOIN PersonT PersonT1 ON CaseHistoryT.PersonID = PersonT1.ID "
			"	LEFT JOIN PersonT PersonT2 ON CaseHistoryT.ProviderID = PersonT2.ID "
			"	LEFT JOIN AppointmentsT ON CaseHistoryT.AppointmentID = AppointmentsT.ID "
			"	WHERE CaseHistoryT.CompletedDate Is Null AND CaseHistoryDetailsT.ItemType = -1 "
			"	GROUP BY CaseHistoryT.ID, CaseHistoryT.Name, CaseHistoryT.PersonID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, "
			"	CaseHistoryT.LocationID, CaseHistoryT.ProviderID, PersonT2.Last + ', ' + PersonT2.First + ' ' + PersonT2.Middle, CaseHistoryDetailsT.ItemID, CaseHistoryT.SurgeryDate, AppointmentsT.StartTime "
			"	) AS CaseHistoryQ ON SuppliesBelowMinQ.ID = CaseHistoryQ.ItemID AND SuppliesBelowMinQ.LocationID = CaseHistoryQ.LocationID "
			"WHERE SuppliesBelowMinQ.AmountAvail < ReorderPoint ",
			InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iasCompleted, InvUtils::iadsUsed);

			return _T(strSQL);
		}
		break;

	// (j.jones 2009-09-17 09:23) - PLID 16703 - split this report into two report files using the same query
	case 539:		//Case History Cost / Profit Analysis By Provider
	case 678: {		//Case History Cost / Profit Analysis By Procedure

		// (j.jones 2008-07-01 16:02) - PLID 30581 - this report is not available unless you have the
		// permission to read the contact costs

		// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
		// (j.jones 2009-09-01 11:04) - PLID 35383 - overhauled so we can optionally link with bills
		// (j.jones 2009-09-17 09:29) - PLID 16703 - Converted details to a subreport, and added ProcedureIDs,
		// as a 255 character string to be used  to group by procedure combinations (as you cannot group by
		// the memo value of procedure names), and also added a 255-character ProcedureNameAbridged field for
		// use in sorting. Added HoursScheduled as a float value of how many hours the surgery appointment lasted.
		// Added total values to the main report query.

		long nCaseHistoryCostProfitAnalysis_CalcType = GetRemotePropertyInt("CaseHistoryCostProfitAnalysis_CalcType", 1, 0, "<None>", true);

		// (j.jones 2009-09-01 11:04) - PLID 35383 - the report query will always show the case history information,
		// and cost-having items on the case history (everything but service codes), and we will have different union
		// clauses of profitable "amounts" (billed, or billable) based on how we're calculating profit

		CString strUnionAmounts;

		if(nCaseHistoryCostProfitAnalysis_CalcType == 3) {
			//billable items on the case history
			//the pretax and total amount fields return the same value, since tax does not exist on the case history
			strUnionAmounts = "SELECT 2 AS LineType, CaseHistoryT.ID AS CaseHistoryID, "
				"NULL AS BillID, NULL AS BillDate, "
				"NULL AS BillInputDate, '<No Linked Bill>' AS BillDescription, "
				"(CASE WHEN ItemType = -1 THEN 'Inventory' ELSE 'Service' END) AS ItemTypeName, "
				"(CASE WHEN ItemType = -1 THEN (SELECT Name FROM ServiceT WHERE ServiceT.ID = ItemID) ELSE CASE WHEN ItemType = -2 THEN "
				"(SELECT Code + ' - ' + Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = ItemID) ELSE "
				"'' END END) AS ItemDesc, "
				"CaseHistoryDetailsT.Quantity, CaseHistoryDetailsT.Amount, NULL AS Cost, "
				"Round(Convert(money, CaseHistoryDetailsT.Amount * CaseHistoryDetailsT.Quantity),2) AS TotalAmountPreTax, "
				"Round(Convert(money, CaseHistoryDetailsT.Amount * CaseHistoryDetailsT.Quantity),2) AS TotalAmount, "
				"NULL AS TotalCost "
				"FROM CaseHistoryT "
				"INNER JOIN CaseHistoryDetailsT ON CaseHistoryT.ID = CaseHistoryDetailsT.CaseHistoryID "
				"INNER JOIN PersonT ON CaseHistoryT.PersonID = PersonT.ID "
				"INNER JOIN PersonT PersonProviderT ON CaseHistoryT.ProviderID = PersonProviderT.ID "
				"INNER JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
				"WHERE (CaseHistoryDetailsT.ItemType = -1 OR CaseHistoryDetailsT.ItemType = -2) "
				"AND CaseHistoryDetailsT.Billable = 1";
		}
		else {
			//linked bills
			strUnionAmounts = "SELECT 2 AS LineType, BilledCaseHistoriesT.CaseHistoryID, "
				"BillsT.ID AS BillID, BillsT.Date AS BillDate, "
				"BillsT.InputDate AS BillInputDate, BillsT.Description AS BillDescription, "				
				"(CASE WHEN ProductT.ID Is Not Null THEN 'Inventory' ELSE 'Service' END) AS ItemTypeName, "
				"ChargesT.ItemCode + ' ' + LineItemT.Description AS ItemDesc, "
				"ChargesT.Quantity, LineItemT.Amount, NULL AS Cost, "
				"Round(Convert(money, ((( "
				"LineItemT.Amount * ChargesT.Quantity * "
				" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * "
				" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
				" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END) * "
				" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)) * "				
				" CASE WHEN [TotalPercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[TotalPercentOff])/100.0)) END "
				" ) - "
				" CASE WHEN LineItemT.Amount > 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
				" ) "
				" ), 2) AS TotalAmountPreTax, "
				"dbo.GetChargeTotal(ChargesT.ID) AS TotalAmount, "
				"NULL AS TotalCost "
				"FROM BillsT "
				"INNER JOIN BilledCaseHistoriesT ON BillsT.ID = BilledCaseHistoriesT.BillID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"LEFT JOIN (SELECT ChargeID, Sum(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE Deleted = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
				"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0";
		
			if(nCaseHistoryCostProfitAnalysis_CalcType == 2) {
				//facility & anesthesia charges only - not just ones using anesthesia billing
				//or facility billing, just the regular anesthesia and facility fee flags
				strUnionAmounts += " AND (ServiceT.Anesthesia = 1 OR ServiceT.FacilityFee = 1)";
			}
		}
		
		switch (nSubLevel) {
			case 1:	{	//subreport of details

				//case history detail query - loads all the main details on the case history, then 
				//loads all the objects on the case history that have a cost (not service codes) 
				//with a LineType of 1, and uses the above queries to calculate where the billable/billed
				//amounts come from, with a LineType of 2
				CString strSql;
				strSql.Format("SELECT CaseHistoryT.ID, CaseHistoryT.PersonID AS PatID, CaseHistoryT.ProviderID AS ProvID, "					
					"CaseHistoryT.LocationID AS LocID, CaseHistoryT.CompletedDate, "
					"CaseHistoryT.SurgeryDate AS TDate, "
					"CASE WHEN CaseHistoryT.CompletedDate Is Null THEN 0 ELSE 1 END AS CompletedVal, "
					"LineItemQ.BillID, LineItemQ.BillDate, "
					"LineItemQ.BillInputDate, LineItemQ.BillDescription, "
					"LineItemQ.LineType, LineItemQ.ItemTypeName, LineItemQ.ItemDesc, LineItemQ.Quantity, LineItemQ.Amount, LineItemQ.Cost, "
					"LineItemQ.TotalAmountPreTax, LineItemQ.TotalAmount, LineItemQ.TotalCost "
					"FROM CaseHistoryT "	
					"INNER JOIN ("
					"SELECT 1 AS LineType, CaseHistoryDetailsT.CaseHistoryID, "
					"NULL AS BillID, NULL AS BillDate, "
					"NULL AS BillInputDate, '' AS BillDescription, "			
					"(CASE WHEN ItemType = -1 THEN 'Inventory' ELSE CASE WHEN ItemType = -2 THEN 'Service' ELSE 'Personnel' END END) AS ItemTypeName, "
					"(CASE WHEN ItemType = -1 THEN (SELECT Name FROM ServiceT WHERE ServiceT.ID = ItemID) ELSE CASE WHEN ItemType = -2 THEN "
					"(SELECT Code + ' - ' + Name FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = ItemID) ELSE "
					"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = ItemID) END END) AS ItemDesc, "
					"CaseHistoryDetailsT.Quantity, "
					"NULL AS Amount, CaseHistoryDetailsT.Cost, "
					"NULL AS TotalAmountPreTax, NULL AS TotalAmount, "
					"Round(Convert(money, CaseHistoryDetailsT.Cost * CaseHistoryDetailsT.Quantity),2) AS TotalCost "
					"FROM CaseHistoryDetailsT "
					"WHERE ItemType <> -2 "
					"UNION ALL %s "
					") AS LineItemQ ON CaseHistoryT.ID = LineItemQ.CaseHistoryID", strUnionAmounts);
				
				return _T(strSql);

				break;
			}
			case 0: {	//main report
				//main case history query - loads all the main details on the case history, then 
				//loads all the objects on the case history that have a cost (not service codes) 
				//with a LineType of 1, and uses the above queries to calculate where the billable/billed
				//amounts come from, with a LineType of 2
				CString strSql;
				strSql.Format("SELECT CaseHistoryT.ID, CaseHistoryT.Name, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
					"CaseHistoryT.PersonID AS PatID, CaseHistoryT.ProviderID AS ProvID, "
					"PersonProviderT.Last + ', ' + PersonProviderT.First + ' ' + PersonProviderT.Middle AS ProvName, "
					"CaseHistoryT.LocationID AS LocID, LocationsT.Name AS LocName, CaseHistoryT.CompletedDate, "
					"Coalesce(dbo.GetCaseHistoryProcedures(CaseHistoryT.ID), '<No Procedures>') AS ProcedureName, "
					"Left(Coalesce(dbo.GetCaseHistoryProcedures(CaseHistoryT.ID), '<No Procedures>'), 255) AS ProcedureNameAbridged, "
					"Left(dbo.GetCaseHistoryProcedureIDs(CaseHistoryT.ID), 255) AS ProcedureIDs, "
					"CaseHistoryT.SurgeryDate AS TDate, "
					"CASE WHEN CaseHistoryT.CompletedDate Is Null THEN 0 ELSE 1 END AS CompletedVal, "
					"Coalesce(CaseHistoryCostTotalsQ.TotalCaseCost, Convert(money,0)) AS TotalCaseCost, "
					"Coalesce(CaseHistoryAmountTotalsQ.TotalCaseAmountPreTax, Convert(money,0)) AS TotalCaseAmountPreTax, "
					"Coalesce(CaseHistoryAmountTotalsQ.TotalCaseAmount, Convert(money,0)) AS TotalCaseAmount, "
					"AppointmentsQ.HoursScheduled, "
					"Round(Convert(money, CASE WHEN AppointmentsQ.HoursScheduled Is Null THEN 0 ELSE CaseHistoryCostTotalsQ.TotalCaseCost / AppointmentsQ.HoursScheduled END),2) AS CaseCostPerHour "
					"FROM CaseHistoryT "			
					"INNER JOIN PersonT ON CaseHistoryT.PersonID = PersonT.ID "
					"INNER JOIN PersonT PersonProviderT ON CaseHistoryT.ProviderID = PersonProviderT.ID "
					"INNER JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
					"LEFT JOIN (SELECT ID, "
					"	Convert(float, DateDiff(minute, dbo.AsDateNoTime(Date) + dbo.AsTimeNoDate(StartTime), dbo.AsDateNoTime(Date) + dbo.AsTimeNoDate(EndTime))) / 60.0 AS HoursScheduled "
					"	FROM AppointmentsT WHERE Status <> 4) AS AppointmentsQ ON CaseHistoryT.AppointmentID = AppointmentsQ.ID "
					"LEFT JOIN (SELECT CaseHistoryID, Sum(Round(Convert(money, Cost * Quantity),2)) AS TotalCaseCost "
					"	FROM CaseHistoryDetailsT "
					"	GROUP BY CaseHistoryID) AS CaseHistoryCostTotalsQ ON CaseHistoryT.ID = CaseHistoryCostTotalsQ.CaseHistoryID "
					"LEFT JOIN (SELECT CaseHistoryID, Sum(TotalAmountPreTax) AS TotalCaseAmountPreTax, Sum(TotalAmount) AS TotalCaseAmount "
					"	FROM (%s) AS LineItemQ "
					"	GROUP BY CaseHistoryID) AS CaseHistoryAmountTotalsQ ON CaseHistoryT.ID = CaseHistoryAmountTotalsQ.CaseHistoryID", strUnionAmounts);
				
				return _T(strSql);
			}
		}

		return _T("");
	}
	break;

	case 556: {	//Preference Cards

		// (j.jones 2008-07-01 11:44) - PLID 30581 - made the Cost field be NULL for personnel if they do not have
		// read permission for the contact cost
		// (j.gruber 2009-03-25 14:24) - PLID 33691 - changed surgery structure
		// (j.jones 2009-08-19 15:30) - PLID 35124 - converted to the new preference cards structure

		BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);
		
		CString strSql;
		strSql.Format("SELECT PreferenceCardsT.ID AS PreferenceCardID, PreferenceCardsT.Name AS PreferenceCardName, "
			"PreferenceCardsT.Description AS PreferenceCardDescription, "
			"PreferenceCardDetailsT.Amount, PreferenceCardDetailsT.Quantity, "
			"Round(Convert(money, (Amount * Quantity)),2) AS LineTotal, "
			"PreferenceCardDetailsT.Billable, "
			"CASE WHEN PreferenceCardDetailsT.PersonID Is Not Null THEN %s ELSE PreferenceCardDetailsT.Cost END AS Cost, "
			"CASE WHEN PreferenceCardDetailsT.PersonID IS NOT NULL THEN "
			"	PersonnelT.Last + ', ' + PersonnelT.First + ' ' + PersonnelT.Middle "
			"	ELSE ServiceT.Name END AS DetailDescription, "
			"DoctorsT.ID AS ProvID, "
			"Coalesce(DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle + ' ' + DoctorsT.Title, '') AS ProvName, "
			"CASE WHEN ServiceID Is Null THEN 'Personnel' WHEN CPTCodeT.ID Is Null THEN 'Inventory' ELSE 'Service Code' END AS ItemType, "
			"Convert(int, %s) AS HideTotalCost "
			"FROM PreferenceCardsT "
			"LEFT JOIN PreferenceCardDetailsT ON PreferenceCardsT.ID = PreferenceCardDetailsT.PreferenceCardID "
			"LEFT JOIN PreferenceCardProvidersT ON PreferenceCardsT.ID = PreferenceCardProvidersT.PreferenceCardID "
			"LEFT JOIN ServiceT ON PreferenceCardDetailsT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN PersonT PersonnelT ON PreferenceCardDetailsT.PersonID = PersonnelT.ID "
			"LEFT JOIN PersonT DoctorsT ON PreferenceCardProvidersT.ProviderID = DoctorsT.ID ",
			bCanViewPersonCosts ? "PreferenceCardDetailsT.Cost" : "NULL",
			bCanViewPersonCosts ? "0" : "CASE WHEN PreferenceCardDetailsT.PersonID IS NOT NULL THEN 1 ELSE 0 END");

		return strSql;
		break;
	}

	//no report was found with that id
	default:
		return _T("");
		break;
	}


}
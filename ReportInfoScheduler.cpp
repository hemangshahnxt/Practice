////////////////
// DRT 8/6/03 - GetSqlScheduler() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "InvUtils.h"
#include "TemplateHitSet.h"
#include "TemplateItemEntryGraphicalDlg.h"
#include "DateTimeUtils.h"
#include "GlobalStringUtils.h"

using namespace ADODB;

// TODO: Use the actual SQL limits here
const COleDateTime dtMin(1800, 1, 1, 0, 0, 0);
const COleDateTime dtMax(2030, 12, 31, 0, 0, 0);
const COleDateTime dtToday = COleDateTime::GetCurrentTime();

CString CReportInfo::GetSqlScheduler(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;
	switch (nID) {
	

	case 17:	//Daily Schedule - special case report
		// (j.gruber 2008-09-17 13:03) - PLID 30284 - added sublevel and subreportnum
		return GetSqlDailySchedule(false, nSubLevel, nSubRepNum);
		break;

	case 16://Weekly Schedule
	case 393://Weekly Schedule (PP)
		{
		/*	Version History
			DRT 7/30/03 - Added PrivFax field
			JMM 2/3/04	- Added custom Text Fields
			TES 2/17/04 - Enabled Appointment subfilters.
			e.lally 8/4/05 - Added AuthNum and the More Info fields: CellPhone, OtherPhone, Email, Pager, 
				PreferredContact, Duration, CreatedLogin, ModifiedDate, ModifiedLogin, LastLM, PrimaryReferralSource
			JMM 9/28/05 - Changed the AuthNum field to use the sql function
			(j.anspach 05-30-2006 15:15 PLID 19090) - Adding AppointmentsT.ArrivalTime
			// (j.gruber 2008-09-11 12:26) - PLID 30284 - added allocations
			//(e.lally 2008-11-18) PLID 32070 - Fixed exception when filtering on appointment type
			// (j.jones 2011-04-05 09:57) - PLID 39448 - added up to 6 insco names
			// (j.armen 2011-07-06 11:10) - PLID 44205 - added confirmed by
			// (j.gruber 2012-09-11 10:04) - PLID 52033 - added appt ins fields
		*/
		//These both use the same .rpt file.  Also, the PP version uses ProvID for the Resource filter.
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500).
		CString strStatus;
		if(nID==393 && GetRemotePropertyInt("ClassicReptShowCancelledAppts",0,0,GetCurrentUserName())) strStatus = "1=1";
		else strStatus = "AppointmentsT.Status <> 4";

		
		switch (nSubLevel) {

			

			case 0: 

				return _T("SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.First,  "
				"    PersonT.Middle, PersonT.Last, ResourceT.Item,  "
				"    AppointmentsT.Date AS Date,  "
				"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name AS Type,  "
				"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, PersonT.HomePhone, PersonT.Fax, "
				"    PersonT.WorkPhone, PersonT.CellPhone, PersonT.OtherPhone, PersonT.Email, PersonT.Pager, "
				"    AppointmentsT.Confirmed,  AppointmentsT.ConfirmedBy, "
				"    CASE WHEN PatientsT.PreferredContact = 1 THEN 'Home' "
				"         WHEN PatientsT.PreferredContact = 2 THEN 'Work' "
				"         WHEN PatientsT.PreferredContact = 3 THEN 'Mobile' "
				"         WHEN PatientsT.PreferredContact = 4 THEN 'Pager' "
				"         WHEN PatientsT.PreferredContact = 5 THEN 'Other' "
				"         WHEN PatientsT.PreferredContact = 6 THEN 'Email' "
				"         ELSE '' END AS PreferredContact, "
				"    convert(datetime, convert(varchar, AppointmentsT.StartTime, 14)) AS StartTime, "
				"    convert(datetime, convert(varchar, AppointmentsT.EndTime, 14)) AS EndTime, "
				"    DATEDIFF(minute, AppointmentsT.StartTime, AppointmentsT.EndTime) AS Duration, "
				"    AppointmentsT.CreatedLogin, AppointmentsT.ModifiedDate, AppointmentsT.ModifiedLogin, AppointmentsT.LastLM, "
				"    PersonT.PrivHome, PersonT.PrivWork, "
				"    ResourceT.ID AS ID, ResourceT.ID AS ProvID, AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, AptShowStateT.ID AS ShowState, PersonT.BirthDate, "
				"	 AppointmentsT.AptTypeID AS SetID, PrivFax, CASE WHEN AppointmentsT.Status = 4 THEN 'Cancelled' ELSE AptShowStateT.Name END AS StateText, "
				"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 1) AS CustomText1, "
				"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 2) AS CustomText2, "
				"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 3) AS CustomText3, "
				"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 4) AS CustomText4,  "
				"    AppointmentsT.ID AS ApptID, "
				"    (SELECT Name FROM ReferralSourceT WHERE ReferralSourceT.PersonID = PatientsT.ReferralID) AS PrimReferralSource, "
				"    dbo.GetActiveAuthNumsByApptID(AppointmentsT.ID) AS AuthNum, "
				"    (SELECT TOP 1 ID FROM PatientInvAllocationsT WHERE AppointmentID = AppointmentsT.ID AND Status != " + AsString((long)InvUtils::iasDeleted) + " ) AS AllocationID, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 1)) AS PriInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 2)) AS SecInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 3)) AS ThirdInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 4)) AS FourthInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 5)) AS FifthInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 6)) AS SixthInsCoName, "
				"    apptPriInsQ.InsCoName as ApptPriIns, \r\n"
				"    apptSecInsQ.InsCoName as ApptSecIns, \r\n"
				"    CASE WHEN ApptPriInsQ.InsCoID IS NULL THEN '' ELSE dbo.GetApptInsuranceList(AppointmentsT.ID) END as ApptInsList \r\n" 
				"FROM (AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID) "
				"    LEFT OUTER JOIN AptTypeT ON  "
				"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN "
				"    (AppointmentResourceT INNER JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID) ON  "
				"    AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT OUTER JOIN "
				"    PersonT INNER JOIN "
				"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
				"    AppointmentsT.PatientID = PatientsT.PersonID LEFT JOIN "
				"	 AptShowStateT ON AppointmentsT.ShowState = AptShowstateT.ID "
				"			 LEFT JOIN (SELECT AptInsQ.AppointmentID, InsuranceCoT.Name as InsCoName, InsuranceCoT.PersonID as InsCoID FROM \r\n"
				"			(SELECT * FROM AppointmentInsuredPartyT WHERE Placement = 1) AptInsQ \r\n"
				"			LEFT JOIN InsuredPartyT ON AptInsQ.InsuredPartyID = InsuredPartyT.PersonID \r\n"
				"			LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ) ApptPriInsQ \r\n"
				"			ON AppointmentsT.ID = ApptPriInsQ.AppointmentID \r\n"
				"			 LEFT JOIN (SELECT AptInsQ.AppointmentID, InsuranceCoT.Name as InsCoName, InsuranceCoT.PersonID as InsCoID FROM \r\n"
				"			(SELECT * FROM AppointmentInsuredPartyT WHERE Placement = 2) AptInsQ \r\n"
				"			LEFT JOIN InsuredPartyT ON AptInsQ.InsuredPartyID = InsuredPartyT.PersonID \r\n"
				"			LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ) ApptSecInsQ \r\n"
				"			ON AppointmentsT.ID = ApptSecInsQ.AppointmentID \r\n"
				"WHERE (" + strStatus + ")");
			
			break;
			

			case 1:
	
				switch (nSubRepNum) {

				
					case 0: 

						//Inventory Allocations
						// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
						if (! g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
								//(e.lally 2008-11-18) PLID 32070 - Added additional joins so we can use the same subfilters without exceptions
									//The where clause of 1=0 as before should prevent a performance hit
								return _T("SELECT -1 as AllocationID, -1 AS PatID, -1 AS LocID, "
								" -1 AS ApptID, '' AS AllocationNotes, -1 as AllocationStatus, GetDate() AS Date, "
								" -1 AS AllocationDetailID, -1 AS ProductID, -1 AS ProductItemID, -1 AS Quantity, -1 AS AllocationDetailStatus, "
								" '' AS AllocationDetailNotes, '' AS ProductName, '' AS SerialNum, getDate() as ExpDate, -1 as UserDefinedID, '' as FullName, "
								" '' AS PersonLocation, -1 AS ProvID, getDate() as StartTime, "
								" GetDAte() AS EndTime, '' AS ApptType, '' AS ApptPurpose, -1 AS StatusType, '' AS StatusText, "
								" getDate() AS AllocationStatusDate, "
								" -1  AS DetailStatusType, "
								" '' DetailStatusText, "
								" '' AS ProvName, "
								"  -1 as AllocLocID, -1 as ID, -1 as ShowState "
								"FROM "
								"AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
								"LEFT JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID "
								"LEFT JOIN AppointmentResourceT ON  AppointmentsT.ID = AppointmentResourceT.AppointmentID "
								"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
								"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
								"LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID "
								"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
								"WHERE 1 = 0 ");
							}
							else {
								//(e.lally 2008-11-18) PLID 32070 - Added join to AptTypeT for when the user filters on Appointment type

								return  _T("SELECT AllocationID, ApptPatID AS PatID, ApptLocID AS LocID, "
									"AppointmentID AS ApptID, AllocationNotes, AllocationStatus, ApptStartTime AS Date, "
									"AllocationDetailID, ProductID, ProductItemID, Quantity, AllocationDetailStatus, "
									"AllocationDetailNotes, ProductName, SerialNum, ExpDate, UserDefinedID, FullName, "
									"LocationName AS PersonLocation, ResourceID AS ProvID, AllocationListSubQ.StartTime AS StartTime, "
									"EndTime, ApptType, ApptPurpose, AllocationListSubQ.StatusType AS StatusType, StatusText, "
									"AllocationListSubQ.AllocationStatusDate AS AllocationStatusDate, "
									"AllocationListSubQ.DetailStatusType AS DetailStatusType, "
									"AllocationListSubQ.DetailStatusText AS DetailStatusText, "
									"Coalesce(AllocationListSubQ.ProviderName, '') AS ProvName, "
									"LocationID as AllocLocID, "
									"ResourceID AS ID, "
									" ShowState AS ShowState "
									
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
									"ELSE PatientInvAllocationsT.InputDate END AS AllocationStatusDate, "
									"AppointmentsT.PatientID as ApptPatID, AppointmentsT.LocationID as ApptLocID, "
									"AppointmentsT.StartTime as ApptStartTime, "
									"AppointmentResourceT.ResourceID as ResourceID, "
									"AppointmentsT.ShowState AS ShowState, "
									"AppointmentsT.AptTypeID AS AptTypeID "

									"FROM AppointmentsT INNER JOIN PatientInvAllocationsT ON AppointmentsT.ID = PatientInvAllocationsT.AppointmentID "
									"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
									"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
									"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "				
									"LEFT JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
									"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
									"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
									"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
									"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
									"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
									"LEFT JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
									"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "				
									"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "

									// (j.jones 2009-08-06 09:50) - PLID 35120 - added BilledCaseHistoriesT
									"LEFT JOIN (SELECT CaseHistoryT.ID, BillsT.Date, AllocationID "
									"	FROM CaseHistoryAllocationLinkT "
									"	INNER JOIN CaseHistoryT ON CaseHistoryAllocationLinkT.CaseHistoryID = CaseHistoryT.ID "
									"	INNER JOIN BilledCaseHistoriesT ON CaseHistoryT.ID = BilledCaseHistoriesT.CaseHistoryID "
									"	INNER JOIN BillsT ON BilledCaseHistoriesT.BillID = BillsT.ID "
									"	WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND CaseHistoryT.CompletedDate Is Not Null "
									") AS BilledCaseHistoryQ ON PatientInvAllocationsT.ID = BilledCaseHistoryQ.AllocationID "

									"LEFT JOIN PersonT ProviderPersonT ON PatientInvAllocationsT.ProviderID = ProviderPersonT.ID "
									"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "

									"WHERE " + strStatus + " AND PatientInvAllocationsT.Status <> " + AsString((long)InvUtils::iasDeleted) + " AND PatientInvAllocationDetailsT.Status <> " + AsString((long)InvUtils::iadsDeleted) + " "
									"AND (((PatientInvAllocationDetailsT.Status IN (" + AsString((long)InvUtils::iadsActive) + "," + AsString((long)InvUtils::iadsOrder) + ") AND PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + ") "
									"OR (PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + ")))) AllocationListSubQ "
									"LEFT JOIN AptTypeT ON AllocationListSubQ.AptTypeID = AptTypeT.ID ";
									);

							}
					break;

					default:
						return "";
					break;
				}
			break;

			default:
				return "";
			break;
		}
	}
	break;
	

	case 19: //Monthly Schedule
	case 394://Monthly Schedule (PP)		
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500).
		/*	Version History
			DRT 6/19/03 - Removed a link to AptPurposeT from AptPurposeID in AppointmentsT, it's obsolete, and the table
					wasn't even used.
			DRT 7/30/03 - Added PrivFax field
			TES 2/17/04 - Enabled Appointment subfilters.
			e.lally 8/4/05 - Added AuthNum and the More Info fields: CellPhone, OtherPhone, Email, Pager, 
				PreferredContact, Duration, CreatedLogin, ModifiedDate, ModifiedLogin, LastLM, PrimaryReferralSource
			JMM 9/28/05 - Changed the AuthNum field to use the sql function
			(j.anspach 05-30-2006 15:47 PLID 19090) - Adding AppointmentsT.ArrivalTime
			// (j.gruber 2008-09-17 12:59) - PLID 30284 - added allocations
			//(e.lally 2008-11-18) PLID 32070 - Fixed exception when filtering on appointment type
			// (j.jones 2011-04-05 09:57) - PLID 39448 - added up to 6 insco names
			// (j.armen 2011-07-06 11:10) - PLID 44205 - added confirmed by
			// (j.gruber 2012-09-11 10:05) - PLID 52033 - added appt ins fields
		*/
		{
		CString strStatus;
		if(nID==394 && GetRemotePropertyInt("ClassicReptShowCancelledAppts",0,0,GetCurrentUserName())) strStatus = "1=1";
		else strStatus = "AppointmentsT.Status <> 4";

		switch (nSubLevel) {
			case 0: 
				return _T("SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.First,  "
				"    PersonT.Middle, PersonT.Last, ResourceT.Item,  "
				"    AppointmentsT.Date AS Date, "
				"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name AS Type,  "
				"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, PersonT.HomePhone,  "
				"    PersonT.WorkPhone, PersonT.CellPhone, PersonT.OtherPhone, PersonT.Email, PersonT.Pager, "
				"    AppointmentsT.Confirmed,  AppointmentsT.ConfirmedBy, PersonT.Fax, "
				"    CASE WHEN PatientsT.PreferredContact = 1 THEN 'Home' "
				"         WHEN PatientsT.PreferredContact = 2 THEN 'Work' "
				"         WHEN PatientsT.PreferredContact = 3 THEN 'Mobile' "
				"         WHEN PatientsT.PreferredContact = 4 THEN 'Pager' "
				"         WHEN PatientsT.PreferredContact = 5 THEN 'Other' "
				"         WHEN PatientsT.PreferredContact = 6 THEN 'Email' "
				"         ELSE '' END AS PreferredContact, "
				"    convert(datetime, convert(varchar, AppointmentsT.StartTime, 14)) AS StartTime, "
				"    convert(datetime, convert(varchar, AppointmentsT.EndTime, 14)) AS EndTime, "
				"    DATEDIFF(minute, AppointmentsT.StartTime, AppointmentsT.EndTime) AS Duration, "
				"    AppointmentsT.CreatedLogin, AppointmentsT.ModifiedDate, AppointmentsT.ModifiedLogin, AppointmentsT.LastLM, "
				"    PersonT.PrivHome, PersonT.PrivWork, "
				"    ResourceT.ID AS ID, AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, AptShowStateT.ID AS ShowState, PersonT.BirthDate, "
				"    ResourceT.ID AS ProvID, AppointmentsT.AptTypeID AS SetID, PrivFax, CASE WHEN AppointmentsT.Status = 4 THEN 'Cancelled' ELSE AptShowStateT.Name END AS StateText, "
				"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 1) AS CustomText1, "
				"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 2) AS CustomText2, "
				"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 3) AS CustomText3, "
				"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 4) AS CustomText4,  "
				"    AppointmentsT.ID AS ApptID, "
				"    (SELECT Name FROM ReferralSourceT WHERE ReferralSourceT.PersonID = PatientsT.ReferralID) AS PrimReferralSource, "
				"    dbo.GetActiveAuthNumsByApptID(AppointmentsT.ID) AS AuthNum, "
				"    (SELECT top 1 ID FROM PatientInvAllocationsT WHERE AppointmentID = AppointmentsT.ID AND Status != " + AsString((long)InvUtils::iasDeleted) + " ) AS AllocationID, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 1)) AS PriInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 2)) AS SecInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 3)) AS ThirdInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 4)) AS FourthInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 5)) AS FifthInsCoName, "
				"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 6)) AS SixthInsCoName, "
				"    apptPriInsQ.InsCoName as ApptPriIns, \r\n"
				"    apptSecInsQ.InsCoName as ApptSecIns, \r\n"
				"    CASE WHEN ApptPriInsQ.InsCoID IS NULL THEN '' ELSE dbo.GetApptInsuranceList(AppointmentsT.ID) END as ApptInsList \r\n" 

				"FROM (AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID) LEFT OUTER JOIN "
				"    AptTypeT ON  "
				"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN "
				"    AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN "
				"	 ResourceT ON  "
				"    AppointmentResourceT.ResourceID = ResourceT.ID LEFT OUTER JOIN "
				"    PersonT INNER JOIN "
				"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
				"    AppointmentsT.PatientID = PatientsT.PersonID LEFT JOIN "
				"	 AptShowStateT ON AppointmentsT.ShowState = AptShowstateT.ID "
				"			 LEFT JOIN (SELECT AptInsQ.AppointmentID, InsuranceCoT.Name as InsCoName, InsuranceCoT.PersonID as InsCoID FROM \r\n"
				"			(SELECT * FROM AppointmentInsuredPartyT WHERE Placement = 1) AptInsQ \r\n"
				"			LEFT JOIN InsuredPartyT ON AptInsQ.InsuredPartyID = InsuredPartyT.PersonID \r\n"
				"			LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ) ApptPriInsQ \r\n"
				"			ON AppointmentsT.ID = ApptPriInsQ.AppointmentID \r\n"
				"			 LEFT JOIN (SELECT AptInsQ.AppointmentID, InsuranceCoT.Name as InsCoName, InsuranceCoT.PersonID as InsCoID FROM \r\n"
				"			(SELECT * FROM AppointmentInsuredPartyT WHERE Placement = 2) AptInsQ \r\n"
				"			LEFT JOIN InsuredPartyT ON AptInsQ.InsuredPartyID = InsuredPartyT.PersonID \r\n"
				"			LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ) ApptSecInsQ \r\n"
				"			ON AppointmentsT.ID = ApptSecInsQ.AppointmentID \r\n"
				"WHERE (" + strStatus + ")");

				case 1:
	
				switch (nSubRepNum) {

				
					case 0: 

						//Inventory Allocations
						// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
							if (! g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
								//(e.lally 2008-11-18) PLID 32070 - Added additional joins so we can use the same subfilters without exceptions
									//The where clause of 1=0 as before should prevent a performance hit
								return _T("SELECT -1 as AllocationID, -1 AS PatID, -1 AS LocID, "
								" -1 AS ApptID, '' AS AllocationNotes, -1 as AllocationStatus, GetDate() AS Date, "
								" -1 AS AllocationDetailID, -1 AS ProductID, -1 AS ProductItemID, -1 AS Quantity, -1 AS AllocationDetailStatus, "
								" '' AS AllocationDetailNotes, '' AS ProductName, '' AS SerialNum, getDate() as ExpDate, -1 as UserDefinedID, '' as FullName, "
								" '' AS PersonLocation, -1 AS ProvID, getDate() as StartTime, "
								" GetDAte() AS EndTime, '' AS ApptType, '' AS ApptPurpose, -1 AS StatusType, '' AS StatusText, "
								" getDate() AS AllocationStatusDate, "
								" -1  AS DetailStatusType, "
								" '' DetailStatusText, "
								" '' AS ProvName, "
								"  -1 as AllocLocID, -1 as ID, -1 as ShowState "
								"FROM "
								"AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
								"LEFT JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID "
								"LEFT JOIN AppointmentResourceT ON  AppointmentsT.ID = AppointmentResourceT.AppointmentID "
								"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
								"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
								"LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID "
								"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
								"WHERE 1 = 0 ");
							}
							else {
								//(e.lally 2008-11-18) PLID 32070 - Added join to AptTypeT for when the user filters on Appointment type

								return  _T("SELECT AllocationID, ApptPatID AS PatID, ApptLocID AS LocID, "
									"AppointmentID AS ApptID, AllocationNotes, AllocationStatus, ApptStartTime AS Date, "
									"AllocationDetailID, ProductID, ProductItemID, Quantity, AllocationDetailStatus, "
									"AllocationDetailNotes, ProductName, SerialNum, ExpDate, UserDefinedID, FullName, "
									"LocationName AS PersonLocation, ResourceID AS ProvID, AllocationListSubQ.StartTime AS StartTime, "
									"EndTime, ApptType, ApptPurpose, AllocationListSubQ.StatusType AS StatusType, StatusText, "
									"AllocationListSubQ.AllocationStatusDate AS AllocationStatusDate, "
									"AllocationListSubQ.DetailStatusType AS DetailStatusType, "
									"AllocationListSubQ.DetailStatusText AS DetailStatusText, "
									"Coalesce(AllocationListSubQ.ProviderName, '') AS ProvName, "
									"LocationID as AllocLocID, "
									"ResourceID AS ID, "
									" ShowState AS ShowState "
									
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
									"ELSE PatientInvAllocationsT.InputDate END AS AllocationStatusDate, "
									"AppointmentsT.PatientID as ApptPatID, AppointmentsT.LocationID as ApptLocID, "
									"AppointmentsT.StartTime as ApptStartTime, "
									"AppointmentResourceT.ResourceID as ResourceID, "
									"AppointmentsT.ShowState AS ShowState, "
									"AppointmentsT.AptTypeID AS AptTypeID "

									"FROM AppointmentsT INNER JOIN PatientInvAllocationsT ON AppointmentsT.ID = PatientInvAllocationsT.AppointmentID "
									"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
									"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
									"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "				
									"LEFT JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
									"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
									"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
									"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
									"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
									"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
									"LEFT JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
									"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "				
									"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "

									// (j.jones 2009-08-06 09:50) - PLID 35120 - added BilledCaseHistoriesT
									"LEFT JOIN (SELECT CaseHistoryT.ID, BillsT.Date, AllocationID "
									"	FROM CaseHistoryAllocationLinkT "
									"	INNER JOIN CaseHistoryT ON CaseHistoryAllocationLinkT.CaseHistoryID = CaseHistoryT.ID "
									"	INNER JOIN BilledCaseHistoriesT ON CaseHistoryT.ID = BilledCaseHistoriesT.CaseHistoryID "
									"	INNER JOIN BillsT ON BilledCaseHistoriesT.BillID = BillsT.ID "
									"	WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND CaseHistoryT.CompletedDate Is Not Null "
									") AS BilledCaseHistoryQ ON PatientInvAllocationsT.ID = BilledCaseHistoryQ.AllocationID "

									"LEFT JOIN PersonT ProviderPersonT ON PatientInvAllocationsT.ProviderID = ProviderPersonT.ID "
									"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "

									"WHERE " + strStatus + " AND PatientInvAllocationsT.Status <> " + AsString((long)InvUtils::iasDeleted) + " AND PatientInvAllocationDetailsT.Status <> " + AsString((long)InvUtils::iadsDeleted) + " "
									"AND (((PatientInvAllocationDetailsT.Status IN (" + AsString((long)InvUtils::iadsActive) + "," + AsString((long)InvUtils::iadsOrder) + ") AND PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + ") "
									"OR (PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + ")))) AllocationListSubQ "
									"LEFT JOIN AptTypeT ON AllocationListSubQ.AptTypeID = AptTypeT.ID ";
									);

							}
					break;

					default:
						return "";
					break;
				}
			break;

			default:
				return "";
			break;
		}
	}


	case 418:  {//Appointments By Place Of Service

		/*Version History
			JMM - 07/30/2003 Created
			TES 8/4/03 - Included EndTime
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			TES 7/21/05 - Added UserDefinedID
			TES 8/23/05 - Changed resource filtering.
			(j.anspach 05-30-2006 16:08 PLID 19090) - Adding AppointmentsT.ArrivalTime
		*/

		CString sql = "SELECT AppointmentsT.StartTime "
		"    AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    AppointmentsT.Date AS Date, AppointmentsT.LocationID as LocID, "
		"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item,  "
		"    AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone,  "
		"    CASE WHEN dbo.GetPurposeString(AppointmentsT.ID) IS NULL THEN 'No Purpose' ELSE dbo.GetPurposeString(AppointmentsT.ID) END AS Purpose,  "
		"    AptTypeT.ID AS ID, "
		"-1 AS ResourceID, "
		"    CASE WHEN AppointmentsT.PatientID = -25 THEN 'No Patient Selected' ELSE Last + ', ' + First + ' ' + Middle END AS PatName, AppointmentsT.ID AS ApptID, "
		"	 AppointmentsT.PatientID, PersonT.ID AS PatID, PersonT.PrivHome, PersonT.PrivWork, PersonT.Email, LocationsT.Name AS LocName, PatientsT.UserDefinedID "
		"FROM AptTypeT RIGHT OUTER JOIN "
		"    AppointmentsT ON  "
		"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER "
		"     JOIN "
		"    PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    AppointmentsT.PatientID = PatientsT.PersonID "
		"    LEFT JOIN LocationsT ON AppointmentsT.LocationId = LocationsT.ID "
		"WHERE (AppointmentsT.Status <> 4)";
		return _T(sql);
		break;
		}

	case 23:
		{
		//Appointments (By Type)
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500).
		/*	Version History
			DRT 4/3/03 - Removed the filter for PersonID > 0.  There are many people who want to see all appointments
					here, not much reason to filter it out.  I conversed with Meikin about it, we decided on a few
					that should filter, and a few that shouldn't.
			DRT 6/19/03 - Removed the PurposeID field, it was pulling from an obsolete field (AptPurposeID).  Also removed
					the join to AptPurposeT, which was joining on the same field, but unused.
			JMM - 7/30/03 made the report show no show appointments, per PLID 8581
			TES - 8/4/03 - Included the end time.
			TES 2/17/04 - Enabled Appointment subfilters.
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			CAH 4/29/04 - Added UserDefinedID
			DRT 6/29/2004 - PLID 13252 - Added counts on the report file per group.  Changed into smry/dtld version.  Removed
				the idiotic naming of "SurgeriesR".  This report will now be known as "ApptsByType"
			JMM 5/11/2005 - PLID  16498 - Added General 1 custom fields
			TES 8/23/2005 - Changed resource filtering.
			JMM 9/28/05 - Added Auth Nums
			(j.anspach 05-31-2006 11:28 PLID 19090) - Adding AppointmentsT.ArrivalTime
			// (j.jones 2009-09-28 12:40) - PLID 35194 - added patient gender and birthdate, insurance company names,
			//and appt. length in minutes
			// (j.jones 2009-11-09 16:05) - PLID 36242 - added patient address fields, and insurance IDs
			// (r.goldschmidt 2014-02-03 15:03) - PLID 42818 - added appointment status field
		*/
		CString sql = "SELECT AppointmentsT.StartTime AS StartTime, AppointmentsT.EndTime, "
			"PersonT.First, PersonT.Middle, PersonT.Last, "
			"AppointmentsT.Date AS Date, AppointmentsT.LocationID as LocID, "
			"AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item, "
			"AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone, "
			"CASE WHEN dbo.GetPurposeString(AppointmentsT.ID) IS NULL THEN 'No Purpose' ELSE dbo.GetPurposeString(AppointmentsT.ID) END AS Purpose, "
			"AptTypeT.ID AS ID, -1 AS ResourceID, "
			"CASE WHEN AppointmentsT.PatientID = -25 THEN '<No Patient Selected>' ELSE Last + ', ' + First + ' ' + Middle END AS PatName, AppointmentsT.ID AS ApptID, "
			"AppointmentsT.PatientID, PersonT.ID AS PatID, CASE WHEN AppointmentsT.PatientID = -25 THEN NULL ELSE PatientsT.UserDefinedID END AS UserDefinedID, PersonT.PrivHome, PersonT.PrivWork, PersonT.Email, "
			"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
			"AptShowStateT.Name AS ShowState, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 1) AS CustomText1, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 2) AS CustomText2, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 3) AS CustomText3, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 4) AS CustomText4,  "
			"PersonT.WorkPhone, PersonT.CellPhone, dbo.GetActiveAuthNumsByApptID(AppointmentsT.ID) AS AuthNums, "
			"PersonT.BirthDate, CASE WHEN PersonT.Gender = 2 THEN 'Female' WHEN PersonT.Gender = 1 THEN 'Male' ELSE '' END AS Gender, "
			"PrimaryInsuranceQ.Name AS PrimaryInsuranceCo, SecondaryInsuranceQ.Name AS SecondaryInsuranceCo, "
			"PrimaryInsuranceQ.IDForInsurance AS PrimaryIDForInsurance, PrimaryInsuranceQ.PolicyGroupNum AS PrimaryPolicyGroupNum, "
			"SecondaryInsuranceQ.IDForInsurance AS SecondaryIDForInsurance, SecondaryInsuranceQ.PolicyGroupNum AS SecondaryPolicyGroupNum, "
			"DATEDIFF(minute, AppointmentsT.StartTime, AppointmentsT.EndTime) AS DurationMinutes "
			"FROM AppointmentsT "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowstateT.ID "
			"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN (SELECT PatientID, Name, IDForInsurance, PolicyGroupNum FROM InsuredPartyT "
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"	WHERE RespTypeID = 1) AS PrimaryInsuranceQ ON PatientsT.PersonID = PrimaryInsuranceQ.PatientID "
			"LEFT JOIN (SELECT PatientID, Name, IDForInsurance, PolicyGroupNum FROM InsuredPartyT "
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"	WHERE RespTypeID = 2) AS SecondaryInsuranceQ ON PatientsT.PersonID = SecondaryInsuranceQ.PatientID "
			"WHERE (AppointmentsT.Status <> 4)";
		return _T(sql);
		break;
		}

	case 278:
		{

		//manual ttx file make notes memo field length 4000, ResourceName memo, length 500
		//Appointments By Purpose
		/* Version History
			TES 8/4/03 - Added EndTime
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			TES 8/23/05 - Changed resource filtering.
			(j.anspach 05-30-2006 16:08 PLID 19090) - Adding AppointmentsT.ArrivalTime
			(a.wilson 2012-3-20) PLID 49058 - adding the fields birthdate and userdefinedid to the select statement.
		*/
		CString sql = "SELECT AppointmentsT.StartTime "
		"    AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    AppointmentsT.Date AS Date, AppointmentsT.LocationID as LocID, "
		"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS ResourceName,  "
		"    AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone,  "
		"    AppointmentPurposeT.PurposeID AS PurposeID, CASE WHEN AptPurposeT.Name IS NULL THEN 'No Purpose' ELSE AptPurposeT.Name END AS Purpose,  "
		"    AptTypeT.ID AS ID, -1 AS ResourceID, "
		"    Last + ', ' + First + ' ' + Middle AS PatName, AppointmentsT.ID AS ApptID, AppointmentsT.PatientID, "
		"	 PersonT.ID AS PatID, PersonT.PrivHome, PersonT.PrivWork, PersonT.Email, PatientsT.UserDefinedID, PersonT.Birthdate "
		"FROM AptTypeT RIGHT OUTER JOIN "
		"    AppointmentsT ON  "
		"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN "
		"	 AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID LEFT JOIN "
		"    AptPurposeT  ON  AppointmentPurposeT.PurposeID = AptPurposeT.ID LEFT JOIN "
		"    PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    AppointmentsT.PatientID = PatientsT.PersonID "
		"WHERE (AppointmentsT.Status <> 4) AND (PersonT.ID > 0) AND (AppointmentsT.ShowState <> 3)";
		return _T(sql);
		break;
		}
	

	case 288:
		{
		//Appointments By Patient Coordinator
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500), and the Item field needs to be set to a memo field (length 500)
		/*	Version History
			DRT 6/19/03 - Removed the join to AptPurposeT, which was joining on the AptPurposeID field, which is obsolete.
			TES 8/4/03 - Included EndTime
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			(j.anspach 05-30-2006 16:09 PLID 19090) - Adding AppointmentsT.ArrivalTime
			// (j.gruber 2006-11-14 09:20) - PLID 23538 - added userdefinedID to the report
		*/
		CString sql = "SELECT AppointmentsT.StartTime "
		"    AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    AppointmentsT.Date AS Date, AppointmentsT.LocationID as LocID, "
		"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item,  "
		"    AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone,  "
		"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, "
		"    AptTypeT.ID AS ID, "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, AppointmentsT.ID AS ApptID, AppointmentsT.PatientID, "
		"	 PersonT.ID AS PatID, PersonT.PrivHome, PersonT.PrivWork, PatientsT.EmployeeID AS PatCoordID,  "
		"	 CASE WHEN PatientsT.EmployeeID IS NOT NULL THEN PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle ELSE 'No Patient Coordinator' END AS PatCoord, "
		"    PatientsT.UserDefinedID AS UserDefinedID "
 		"FROM AptTypeT RIGHT OUTER JOIN "
		"    AppointmentsT ON  "
		"    AppointmentsT.AptTypeID = AptTypeT.ID "
		"    LEFT JOIN PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    AppointmentsT.PatientID = PatientsT.PersonID "
		"	 LEFT JOIN PersonT PersonT1 ON PatientsT.EmployeeID = PersonT1.ID "
		"WHERE (AppointmentsT.Status <> 4) AND (PersonT.ID > 0) AND (AppointmentsT.ShowState <> 3)";

		return _T(sql);
		break;
		}

	case 50:
		{
		//Appointments
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500), and the Item field needs to be set to a memo field (length 500)
		/*	Version History
			DRT 4/3/03 - Removed the filter for PersonID > 0.  There are many people who want to see all appointments
					here, not much reason to filter it out.  I conversed with Meikin about it, we decided on a few
					that should filter, and a few that shouldn't.
			DRT 6/19/03 - Removed the join to AptPurposeT, which was joining on the AptPurposeID field, which is obsolete.
			TES 8/4/03 - Included EndTime
			DRT 2/9/2004 - Added InputDate.  Also made report editable.
			TES 2/17/04 - Enabled Appointment subfilters.
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			JMM 5/11/2005 - PLID 16497 - added General 1 custom text fields
			TES 8/23/05 - Changed resource filtering.
			// (j.gruber 2008-10-23 08:39) - PLID 31797 - added referral source and short purpose
			// (f.gelderloos 2013-07-24 11:45) - PLID 33920 Add UserDefinedID to report data
		*/
		CString sql = "SELECT AppointmentsT.StartTime "
				 "AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last, "
				 "AppointmentsT.Date AS Date, "
				 "AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item, "
				 "AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone, "
				 "dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, "
				 "AptTypeT.ID AS ID, "
			"-1 AS ResourceID, "
				 "CASE WHEN AppointmentsT.PatientID = -25 THEN 'No Patient Selected' ELSE Last + ', ' + First + ' ' + Middle END AS PatName, "
				  "AppointmentsT.PatientID, "
			"AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, PersonT.ID AS PatID, PersonT.PrivHome, PersonT.PrivWork, PersonT.Email, "
				  "AppointmentsT.CreatedDate AS InputDate, AppointmentsT.ID AS ApptID, "
				 "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 1) AS CustomText1, "
				 "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 2) AS CustomText2, "
				 "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 3) AS CustomText3, "
				 "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 4) AS CustomText4, "
				 "ReferralSourceT.Name as PrimaryRefSource, dbo.GetPurposeString(AppointmentsT.ID) AS PurposeShort, PatientsT.UserDefinedID AS UserDefinedID "
			"FROM (AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID) LEFT OUTER JOIN "
				 "AptTypeT ON "
				 "AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN "
				 "PersonT INNER JOIN "
				 "PatientsT ON PersonT.ID = PatientsT.PersonID ON "
				 "AppointmentsT.PatientID = PatientsT.PersonID "
				  "LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"WHERE (AppointmentsT.Status <> 4) AND (AppointmentsT.ShowState <> 3)";

		return _T(sql);
		break;
		}


	case 63:
		{//Scheduled and Not Billed (Bill and Appt Date)
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500)
		/*	Version History
			DRT 6/19/03 - Removed the join to AptPurposeT, which was joining on the AptPurposeID field, which is obsolete.
			TES 2/17/04 - Enabled Appointment subfilters.
		*/
		CString sql = "SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"AppointmentsT.Date AS Date,  "
		"AppointmentsT.StartTime,  "
		"AppointmentsT.EndTime,  "
		"AptTypeT.Name,  "
		"dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, "
		"AppointmentsT.Notes, ResourceT.Item,  "
		"ResourceT.ID AS ResourceID,  "
		"AppointmentsT.AptTypeID AS SetID, AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, "
		"AppointmentsT.ID AS ApptID "
		"FROM PersonT INNER JOIN "
		"    PatientsT ON  "
		"    PersonT.ID = PatientsT.PersonID RIGHT OUTER JOIN "
		"    AppointmentsT ON PatientsT.PersonID = AppointmentsT.PatientID "
		"	 LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID LEFT OUTER JOIN "
		"    AptTypeT ON "
		"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN "
		"AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
		"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
		"LEFT OUTER JOIN "
		"    (SELECT LineItemT.PatientID, LineItemT.Date AS TDate FROM LineItemT WHERE LineItemT.Type=10 AND LineItemT.Deleted = 0 "
		"    ) AS SchedAndNotBilledFilterQ ON AppointmentsT.Date = SchedAndNotBilledFilterQ.TDate AND  "
		"    AppointmentsT.PatientID = SchedAndNotBilledFilterQ.PatientID "
		"WHERE PatientsT.UserDefinedID>0 AND AppointmentsT.Status<>4 AND SchedAndNotBilledFilterQ.PatientID Is Null AND SchedAndNotBilledFilterQ.TDate Is Null AND AppointmentsT.ShowState <> 3 "; // 3 = no show
		return _T(sql);
		break;
		}

	case 699:
		{//Scheduled and Not Billed By Billed Appointments		
		/*	Version History
			// (j.gruber 2010-07-21 11:47) - PLID 37826 - created from original scheduled and not billed
		*/
		CString sql = "SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"AppointmentsT.Date AS Date,  "
		"AppointmentsT.StartTime,  "
		"AppointmentsT.EndTime,  "
		"AptTypeT.Name,  "
		"dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, "
		"AppointmentsT.Notes, ResourceT.Item,  "
		"ResourceT.ID AS ResourceID,  "
		"AppointmentsT.AptTypeID AS SetID, AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, "
		"AppointmentsT.ID AS ApptID "
		"FROM PersonT INNER JOIN "
		"    PatientsT ON  "
		"    PersonT.ID = PatientsT.PersonID RIGHT OUTER JOIN "
		"    AppointmentsT ON PatientsT.PersonID = AppointmentsT.PatientID "
		"	 LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID LEFT OUTER JOIN "
		"    AptTypeT ON "
		"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN "
		"AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
		"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
		"WHERE AppointmentsT.ID NOT IN (SELECT AppointmentID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 AND AppointmentID IS NOT NULL) AND PatientsT.UserDefinedID>0 AND AppointmentsT.Status<>4 AND AppointmentsT.ShowState <> 3 "; // 3 = no show
		return _T(sql);
		break;
		}

	case 90:
		//Appointments Without Prepayments
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500)
		/*	Version History
			DRT 4/3/03 - Reworked the WHERE clause, it was somewhat shady (I think it was showing no show appts if the prepayment was null, but 
					not if it was 0).  Also added a filter for PersonID > 0 - any -25 patients are obviously going to have no prepayments, it
					doesn't make sense to list them.
			TES 5/5/03 - GROUPed BY everything.
			DRT 6/19/03 - Removed the join to AptPurposeT, which was joining on the AptPurposeID field, which is obsolete.
			TES 2/17/04 - Enabled Appointment subfilters.
			DRT 9/28/2005 - PLID 17707 - AptTypeT.ID is now aliased as AptTypeID, not ApptID.
		*/
		return _T("SELECT AppointmentsT.PatientID AS PatID, PatientsT.UserDefinedID AS PatientID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
		"AptTypeT.Name AS Type,  "
		"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName,  "
		"AppointmentsT.Date AS Date,  "
		"AppointmentsT.Notes,  "
		"AppointmentsT.StartTime,  "
		"AppointmentsT.EndTime,  "
		"ResourceT.Item AS Resource,  "
		"ResourceT.ID AS ResourceID,  "
		"ResourceT.ID AS ID, AptTypeT.ID AS AptTypeID, AppointmentsT.LocationID AS LocID, LocationsT.Name, "
		"AppointmentsT.ID AS ApptID "
		"FROM AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID LEFT JOIN "
		"	 AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN "
		"	 ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID LEFT OUTER JOIN "
		"    AptTypeT ON "
		"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN "
		"    PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    AppointmentsT.PatientID = PatientsT.PersonID LEFT OUTER JOIN "
		"    PaymentsT INNER JOIN "
		"    LineItemT ON PaymentsT.ID = LineItemT.ID ON  "
		"    AppointmentsT.PatientID = LineItemT.PatientID AND  "
		"    AppointmentsT.Date = LineItemT.Date  "
		"WHERE (PersonT.ID > 0) AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND ((PaymentsT.PrePayment Is Null) OR (PaymentsT.PrePayment=0)) "
		"GROUP BY AppointmentsT.PatientID, PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, AptTypeT.Name, "
		"AppointmentsT.ID, AppointmentsT.Date, AppointmentsT.Notes, AppointmentsT.StartTime, AppointmentsT.EndTime, ResourceT.Item, "
		"ResourceT.ID, AptTypeT.ID, AppointmentsT.LocationID, LocationsT.Name");
		break;

	case 93:
		{
		//No Show Patients by Referral
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500), and the Item field needs to be set to a memo field (length 500)
		/*	Version History
			DRT 6/19/03 - Removed the join to AptPurposeT, which was joining on the AptPurposeID field, which is obsolete.
			DRT 2/10/2004 - PLID 10646 - Made report editable, added code to GetTtxContents() function to handle special field lengths
			TES 2/17/04 - Enabled Appointment subfilters.
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			TES 8/23/05 - Changed resource filtering
			(a.wilson 2012-3-20) PLID 49058 - adding the userdefinedid and birthdate.
		*/
		CString sql = "SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
		"ReferralSourceT.Name AS RefName,  "
		"AppointmentsT.Notes,  "
		"AptTypeT.Name AS Type,  "
		"dbo.GetPurposeString(AppointmentsT.ID) AS Purpose,  "
		"dbo.GetResourceString(AppointmentsT.ID) AS Item,  "
		"AppointmentsT.StartTime,  "
		"AppointmentsT.EndTime,  "
		"AppointmentsT.PatientID,  "
		"AppointmentsT.Date AS Date,  "
		"PersonT.HomePhone,  "
		"PersonT.WorkPhone,  "
		"AppointmentsT.LocationID AS LocID, "
		"LocationsT.Name AS Location, PersonT.ID AS PatID, "
		"-1 AS ResourceID, "
		"PersonT.PrivHome, PersonT.PrivWork, AppointmentsT.AptTypeID AS TypeID, "
		"AppointmentsT.ID AS ApptID, PatientsT.UserDefinedID, PersonT.Birthdate "
		"FROM PersonT INNER JOIN "
		"    (PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) ON  "
		"    PersonT.ID = PatientsT.PersonID RIGHT OUTER JOIN "
		"    (AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID) LEFT OUTER JOIN "
		"    AptTypeT ON "
		"    AppointmentsT.AptTypeID = AptTypeT.ID ON  "
		"    PatientsT.PersonID = AppointmentsT.PatientID "
		"WHERE AppointmentsT.PatientID>0 AND AppointmentsT.Status<>4 AND AppointmentsT.ShowState=3 "; // 3 = no show
		return _T(sql);
		break;
		}


	case 108:
		//Scheduled with Pending Balances
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500)
		/*	Version History
			DRT 3/14/03 - Rewrote the balance part of the query, now it selects 3 fields, pat resp, pri resp, sec resp, instead
					of 1 total balance.  The report was updated to include this, as well as remove some extraneous info (date 
					showing twice, end time taken out for space considerations)
			DRT 4/9/03 - Fixed a problem with the last change that was not calculating the insurance balances right when no payments
					for that insured party existed.
			DRT 6/19/03 - Added OtherResp field, so that the balance on the report can total correctly
			DRT 7/28/03 - Made the report editable.
			TES 2/17/04 - Enabled Appointment subfilters.
			JMJ 2/3/06 - Fixed problem where if there were no charges and only pay/adj/ref, patients didn't show up in the list
		*/
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
			"SplitBalanceQ.PatResp, "
			"SplitBalanceQ.PriResp, "
			"SplitBalanceQ.SecResp, "
			"SplitBalanceQ.OtherResp, "
			"AppointmentsT.Date AS Date,  "
			"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName,  "
			"AppointmentsT.StartTime,  "
			"AppointmentsT.EndTime,  "
			"AppointmentsT.Notes,  "
			"AppointmentsT.Status,  "
			"ResourceT.Item,  "
			"AppointmentsT.LocationID AS LocID,  "
			"LocationsT.Name AS Location, ResourceT.ID AS ResourceID,  "
			"AppointmentsT.ID AS ApptID "
			""
			"FROM  "
			"    AppointmentsT LEFT OUTER JOIN  "
			"	 AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN  "
			"	 ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID LEFT JOIN  "
			"    LocationsT ON   "
			"    AppointmentsT.LocationID = LocationsT.ID RIGHT OUTER  "
			"     JOIN  "
			"    PersonT INNER JOIN  "
			"    PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
			"    AppointmentsT.PatientID = PatientsT.PersonID LEFT OUTER JOIN  "
			""
			"/* Start SplitBalanceQ */ "
			""
			"	(SELECT	PersonT.ID AS PatID, "
			"		Sum(CASE WHEN RespTypeID = -2 THEN Total ELSE 0 END) AS PatResp,  "
			"		Sum(CASE WHEN RespTypeID = 1 THEN Total ELSE 0 END) AS PriResp,  "
			"		Sum(CASE WHEN RespTypeID = 2 THEN Total ELSE 0 END) AS SecResp, "
			"		Sum(CASE WHEN RespTypeID <> 1 AND RespTypeID <> 2 AND RespTypeID <> -2 THEN Total ELSE 0 END) AS OtherResp "
			"	FROM  "
			"	PersonT LEFT JOIN  "
			"		(SELECT PersonT.ID AS PatientID, Sum(Coalesce(ChargeSubQ.TotalAmt,0)) AS Total, "
			"		CASE WHEN InsuredPartyT.RespTypeID IS NULL THEN -2 ELSE InsuredPartyT.RespTypeID END AS RespTypeID "
			""
			"		FROM PersonT LEFT JOIN "
			"			(SELECT PersonT.ID AS PatientID, Sum(ChargeRespT.Amount) AS TotalAmt, CASE WHEN ChargeRespT.InsuredPartyID IS NULL THEN -1 ELSE ChargeRespT.InsuredPartyID END AS InsuredPartyID "
			"			FROM PersonT LEFT JOIN BillsT ON PersonT.ID = BillsT.PatientID "
			"			LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"			LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"			LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"			WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
			"			GROUP BY PersonT.ID, ChargeRespT.InsuredPartyID "
			""
			"			UNION "
			""
			"			SELECT LineItemT.PatientID AS PatientID, -Sum(LineItemT.Amount) AS PayAmt, PaymentsT.InsuredPartyID "
			"			FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"			WHERE LineItemT.Deleted = 0 AND LineItemT.Type <= 3 "
			"			GROUP BY LineItemT.PatientID, PaymentsT.InsuredPartyID "
			"			) ChargeSubQ "
			"		ON PersonT.ID = ChargeSubQ.PatientID "
			"		LEFT JOIN InsuredPartyT ON ChargeSubQ.InsuredPartyID = InsuredPartyT.PersonID "
			""
			"		GROUP BY PersonT.ID, CASE WHEN InsuredPartyT.RespTypeID IS NULL THEN -2 ELSE InsuredPartyT.RespTypeID END) BalanceSubQ "
			"	ON PersonT.ID = BalanceSubQ.PatientID "
			"	GROUP BY PersonT.ID) SplitBalanceQ "
			" "
			"/* End SplitBalanceQ */ "
			" "
			"ON PatientsT.PersonID = SplitBalanceQ.PatID  "
			"WHERE (AppointmentsT.Status != 4) AND (AppointmentsT.ShowState <> 3) AND (SplitBalanceQ.PatResp + SplitBalanceQ.PriResp + SplitBalanceQ.SecResp <> 0)"); // 3 = no show
		break;

	case 193:
		{
		//Appointments Cancelled
		//Note: When generating .ttx file, the Purpose field needs to be manually set to a memo field (length 500), 
		//		also the notes field needs to be manually set to a memo field (length 4000)
		/*	Version History
			DRT 4/3/03 - Removed a filter for PersonID > 0.  Often times we want to see all cancelled appointments, but can't find 
					ones for no patient.  Also a cancelled meeting is something they probably want to see as well.
			e.lally 01/02/2008 - PLID 26613 - Added input username, input user's name, and input date
		*/
		CString sql = "SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     AppointmentsT.Date AS Date,  "
		"    AppointmentsT.StartTime, "
		"    AppointmentsT.EndTime, "
		"	 AppointmentsT.CancelledBy, AppointmentsT.CancelledDate AS CancelledDate, "
		"    AppointmentsT.Notes, ResourceT.ID AS ResourceID,  "
		"    ResourceT.Item, dbo.GetPurposeString(AppointmentsT.ID) AS Purpose,  "
		"    AptTypeT.Name, AptTypeT.ID AS ApptTypeID, "
		"AppointmentsT.LocationID AS LocID, "
		"LocationsT.Name AS Location, AppointmentsT.ID AS ApptID, "
		"CASE WHEN AptCancelReasonT.Description IS NOT NULL THEN AptCancelReasonT.Description ELSE CASE WHEN Len(CancelledReason) = 0 THEN '<None Specified>' ELSE CancelledReason END END AS CancelReason, "
		//use for suppressing ASC-related notes
		"1 AS Suppress, AppointmentsT.CreatedLogin AS InputUser, PersonT_Input.First AS InputUserFirst, "
		"PersonT_Input.Middle AS InputUserMiddle, PersonT_Input.Last AS InputUserLast, AppointmentsT.CreatedDate "
		
		"FROM PersonT INNER JOIN "
		"    PatientsT ON  "
		"    PersonT.ID = PatientsT.PersonID LEFT OUTER JOIN "
		"	 AppointmentsT ON PatientsT.PersonID = AppointmentsT.PatientID "
		"	 LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
		"	 LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
		"	 LEFT JOIN AptCancelReasonT ON AppointmentsT.CancelReasonID = AptCancelReasonT.ID "
		"    LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
		"	 LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
		"	 LEFT JOIN UsersT ON AppointmentsT.CreatedLogin = UsersT.Username "
		"	 LEFT JOIN PersonT PersonT_Input ON UsersT.PersonID = PersonT_Input.ID "
		"WHERE (AppointmentsT.Status = 4)";
		return _T(sql);
		break;
		}

	case 199:
		//New Patients By Appointment Type
		//Note: When generating .ttx file, the Purpose field needs to be manually set to a memo field (length 500)
		/*	Version History
			DRT 8/6/03 - Added a subreport that shows all new patients that did not schedule at all.
			TES 2/17/04 - Enabled Appointment subfilters.
		*/
		switch(nSubLevel) {
		case 1:	//subreport
			return _T("SELECT PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,   "
				"PersonT.FirstContactDate AS Date,  "
				"ReferralSourceT.PersonID AS RefID, ReferralSourceT.Name AS RefName,   "
				"PatientsT.UserDefinedID, PersonT.Location AS LocID,  "
				"LocationsT.Name AS Location "
				"FROM PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID INNER JOIN  "
				"PatientsT ON PersonT.ID = PatientsT.PersonID  "
				"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
				"WHERE (PatientsT.PersonID > 0) AND PersonT.ID NOT IN (SELECT PatientID FROM AppointmentsT)");
			break;
		default:
			return _T("SELECT PatientsT.PersonID AS PatID,  "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
				"PersonT.FirstContactDate AS Date,  "
				"AptTypeT.ID AS ApptTypeID,  "
				"AptTypeT.Name AS ApptName,  "
				"AppointmentsT.Date AS ApptDate,  "
				"ReferralSourceT.PersonID AS RefID,  "
				"ReferralSourceT.Name AS RefName,  "
				"ResourceT.ID AS ID,  "
				"ResourceT.Item,  "
				"dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, "
				"AppointmentsT.CreatedDate, "
				"PatientsT.UserDefinedID, "
				"PersonT.Location AS LocID, "
				"LocationsT.Name AS Location, "
				"AppointmentsT.ID AS ApptID "
				"FROM (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) INNER JOIN "
				"    (PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) ON  "
				"    PersonT.ID = PatientsT.PersonID RIGHT OUTER JOIN "
				"    AppointmentsT LEFT OUTER JOIN "
				"    AptTypeT ON "
				"    AppointmentsT.AptTypeID = AptTypeT.ID ON  "
				"    PatientsT.PersonID = AppointmentsT.PatientID LEFT OUTER JOIN "
				"	 AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN "
				"    ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
				"WHERE (PatientsT.PersonID > 0) AND (AppointmentsT.ShowState <> 3) AND (AppointmentsT.Status <> 4) "); // 3 = no show
			break;
		}
		break;

/*	case 200:
		//Consult to Appointment
		return _T("SELECT ConsultToApptSubQ.PatID AS PatID, ConsultToApptSubQ.UserDefinedID, ConsultToApptSubQ.PatName,   "
		"    ConsultToApptSubQ.Name AS ConsultName,   "
		"    ConsultToApptSubQ.Date AS Date, AptTypeT.Name AS ApptName,   "
		"    AppointmentsT.Date AS ApptDate,   "
		"    AptPurposeT.Name AS ApptPurpose,   "
		"    AppointmentsT.ID AS ApptID, ResourceT.ID AS ID, ResourceT.Item,   "
		"    AppointmentsT.Status,  "
		"AppointmentsT.LocationID AS LocID,  "
		"LocationsT.Name AS Location, AptTypeT.ID AS SetID, ReferralSourceT.Name AS RefName  "
		"FROM AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID LEFT OUTER JOIN  "
		"        (SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
		"           PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle  "
		"            AS PatName, AptPurposeT.Name AS Purpose,   "
		"           AptTypeT.Name, MAX(  "
		"           AppointmentsT.Date) AS Date,   "
		"           AppointmentsT.Status  "
		"      FROM PatientsT INNER JOIN  "
		"           PersonT ON   "
		"           PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN  "
		"           AppointmentsT LEFT OUTER JOIN  "
		"           AptPurposeT ON   "
		"           AppointmentsT.AptPurposeID = AptPurposeT.ID LEFT  "
		"            OUTER JOIN  "
		"           AptTypeT ON   "
		"           AppointmentsT.AptTypeID = AptTypeT.ID ON   "
		"           PersonT.ID = AppointmentsT.PatientID  "
		"      GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID,   "
 		"          PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
		"            AptPurposeT.Name, AptTypeT.Name,   "
		"           AppointmentsT.Status  "
		"      HAVING (AptTypeT.Name LIKE '%Consult%') AND   "
		"           (AppointmentsT.Status <> 4))   "
		"    ConsultToApptSubQ ON AppointmentsT.PatientID = ConsultToApptSubQ.PatID LEFT JOIN "
 		"   ResourceT ON AppointmentsT.ResourceID = ResourceT.ID LEFT JOIN "
		"    AptPurposeT ON AppointmentsT.AptPurposeID = AptPurposeT.ID LEFT JOIN "
 		"   AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID LEFT JOIN "
		"	PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID LEFT JOIN "
		"	ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
		"WHERE ((AppointmentsT.Status <> 4) AND   "
 		"   (ConsultToApptSubQ.PatID > 0) AND (CONVERT(varchar, AppointmentsT.Date, 1)  "
 		"   <> ConsultToApptSubQ.Date) AND (ConsultToApptSubQ.Name <> AptTypeT.Name) AND (AppointmentsT.ShowState <> 3))"); // 3 = no show
		break;*/
	

	case 249:
		{
			//Scheduling Activity
			//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
			//and the Purpose field needs to be manually set to a memo field (length 500), and the Item field needs to be set to a memo field (length 500)
			/*	Version History
				DRT 7/21/03 - Added InputDate field, also put it into the report
				JMM 1/30/03 - Added Modified Date field as filter
				TES 2/17/04 - Enabled Appointment subfilters.
				TES 3/4/04 - Removed references to AppointmentsT.ResourceID
				TES 8/23/05 - Changed resource filtering.
				// (j.gruber 2008-12-03 15:45) - PLID 32314 - added next appt date and next surg date
				TES 8/5/2009 - PLID 35047 - Don't count "Other Procedural" appointments as surgeries
				// (j.armen 2011-07-06 11:10) - PLID 44205 - added confirmed by
			*/
			CString sql = 
				"SELECT PersonT.ID AS PatID, PatientsT.UserDefinedID, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
					"Confirmed, AppointmentsT.ConfirmedBy, CONVERT(bit, CASE WHEN WaitingListT.ID > 0 THEN 1 ELSE 0 END) AS MoveUp, "
					"AptShowStateT.Name AS ShowState, AppointmentsT.Date AS Date, StartTime, EndTime, "
					"dbo.GetResourceString(AppointmentsT.ID) AS Item, AptTypeT.Name, AptTypeT.ID AS AptTypeID, "
					"dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, AppointmentsT.LocationID AS LocID, "
					"AppointmentsT.Notes, AppointmentsT.Status, -1 AS ItemID, "
					"AppointmentsT.CreatedDate AS IDate, AppointmentsT.ModifiedDate AS MDate, "
					"AppointmentsT.ID AS ApptID, "
					" (SELECT Min(Date) FROM AppointmentsT InnerApptsT WHERE InnerApptsT.PatientID = AppointmentsT.PatientID AND InnerApptsT.StartTime > AppointmentsT.StartTime AND InnerApptsT.Status <> 4 AND InnerApptsT.ShowState <> 3) AS NextAppt, "
					" (SELECT Min(Date) FROM AppointmentsT InnerApptsT WHERE InnerApptsT.PatientID = AppointmentsT.PatientID AND InnerApptsT.StartTime > AppointmentsT.StartTime AND InnerApptsT.Status <> 4 AND InnerApptsT.ShowState <> 3 "
					" AND InnerApptsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4))) As NextSurgDate "
				"FROM "
				"AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN WaitingListT ON AppointmentsT.ID = WaitingListT.AppointmentID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID LEFT JOIN "
				"AptShowStateT ON AppointmentsT.ShowState = AptShowstateT.ID ";
			return _T(sql);
			break;
		}

		/*case 250:
			//Screening to Appointment
			return _T("SELECT ScreeningToApptSubQ.PatID AS PatID, ScreeningToApptSubQ.UserDefinedID, ScreeningToApptSubQ.PatName,   "
			"    ScreeningToApptSubQ.Name AS ConsultName,   "
			"    ScreeningToApptSubQ.Date AS Date, AptTypeT.Name AS ApptName,   "
			"    AppointmentsT.Date AS ApptDate,   "
			"    AptPurposeT.Name AS ApptPurpose,   "
			"    AppointmentsT.ID AS ApptID, ResourceT.ID AS ID, ResourceT.Item,   "
			"    AppointmentsT.Status,  "
			"AppointmentsT.LocationID AS LocID,  "
			"LocationsT.Name AS Location, AptTypeT.ID AS SetID, ReferralSourceT.Name AS RefName  "
			"FROM AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID LEFT OUTER JOIN  "
			"        (SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
			"           PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle  "
			"            AS PatName, AptPurposeT.Name AS Purpose,   "
			"           AptTypeT.Name, MAX(  "
			"           AppointmentsT.Date) AS Date,   "
			"           AppointmentsT.Status  "
			"      FROM PatientsT INNER JOIN  "
			"           PersonT ON   "
			"           PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN  "
			"           AppointmentsT LEFT OUTER JOIN  "
			"           AptPurposeT ON   "
			"           AppointmentsT.AptPurposeID = AptPurposeT.ID LEFT  "
			"            OUTER JOIN  "
			"           AptTypeT ON   "
			"           AppointmentsT.AptTypeID = AptTypeT.ID ON   "
			"           PersonT.ID = AppointmentsT.PatientID  "
			"      GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID,   "
 			"          PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"            AptPurposeT.Name, AptTypeT.Name,   "
			"           AppointmentsT.Status  "
			"      HAVING (AptTypeT.Name LIKE '%Screening%') AND   "
			"           (AppointmentsT.Status <> 4))   "
			"   ScreeningToApptSubQ ON AppointmentsT.PatientID = ScreeningToApptSubQ.PatID LEFT JOIN "
 			"   ResourceT ON AppointmentsT.ResourceID = ResourceT.ID LEFT JOIN "
			"   AptPurposeT ON AppointmentsT.AptPurposeID = AptPurposeT.ID LEFT JOIN "
 			"   AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID LEFT JOIN "
			"	PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID LEFT JOIN "
			"	ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"WHERE ((AppointmentsT.Status <> 4) AND   "
 			"   (ScreeningToApptSubQ.PatID > 0) AND (CONVERT(varchar, AppointmentsT.Date, 1)  "
 			"   <> ScreeningToApptSubQ.Date) AND (ScreeningToApptSubQ.Name <> AptTypeT.Name) AND (AppointmentsT.ShowState <> 3))"); // 3 = no show
			break;*/

		case 253:
			//Procedures by Date
			/* Version History
				TES 2/17/04 - Enabled Appointment subfilters.
			*/
			return _T("SELECT PersonT.Last + ', ' + First + ' ' + Middle AS Name, PersonT.ID AS PatID,  "
			"AppointmentsT.Date AS Date, ResourceT.ID AS ResourceID, ResourceT.Item AS Resource,  "
			"AppointmentPurposeT.PurposeID AS ProcedureID, ProcedureT.Name AS Surgery,  "
			"AppointmentsT.ID AS ApptID "
			"FROM AppointmentsT   "
			"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
			"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
			"INNER JOIN (AppointmentPurposeT INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
			"INNER JOIN (AppointmentResourceT INNER JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID) ON AppointmentsT.ID = AppointmentResourceT.AppointmentID  "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE AptTypeT.Category = 4 AND AppointmentsT.Status <> 4 ");
			break;

		case 263:
			{
			//Procedures Billed by Date
			/* Version History
				TES 2/17/04 - Enabled Appointment subfilters.
				TES 3/4/04 - Removed references to AppointmentsT.ResourceID
				TES 8/23/05 - Changed resource filtering.
			*/
			//Note:  Make sure to manually fix the ttx file to make the Resource field a memo (length 500)
			CString sql = "SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, PersonT.ID AS PatID, "
				"PatientsT.UserDefinedID, AppointmentsT.Date AS Date, "
				"-1 AS ResourceID, "
				"dbo.GetResourceString(AppointmentsT.ID) AS Resource, AppointmentPurposeT.PurposeID AS ProcedureID, "
				"ProcedureT.Name AS Surgery, "
				"(SELECT Sum(dbo.GetChargeTotal(ChargesT.ID)) FROM LineItemT INNER JOIN ChargesT LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT AS CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number ON LineItemT.ID = ChargesT.ID WHERE ChargesT.BillID = BillsT.ID) AS Amount, BillsT.ID AS BillID, "
				"AppointmentsT.ID AS ApptID "
				"FROM ((AppointmentsT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON AppointmentsT.PatientID = PersonT.ID) INNER JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID) INNER JOIN (ServiceT INNER JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID) ON AppointmentPurposeT.PurposeID = ServiceT.ProcedureID INNER JOIN ChargesT ON ServiceT.ID = ChargesT.ServiceID INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"WHERE BillID = ( "
				"SELECT Top 1  "
				"BillsT.ID "
				"FROM (ChargesT INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID) LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"WHERE ServiceT.ProcedureID IS Not Null AND ServiceT.ProcedureID = AppointmentPurposeT.PurposeID AND BillsT.PatientID = AppointmentsT.PatientID AND BillsT.Deleted = 0 AND EXISTS (SELECT LineItemT.ID FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID WHERE Date = AppointmentsT.Date AND BillID = BillsT.ID)"
				"GROUP BY BillsT.ID, ServiceT.ProcedureID, BillsT.PatientID, BillsT.Date "
				") "
				"AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
				"GROUP BY PersonT.Last, PersonT.First, PersonT.Middle, PersonT.ID, PatientsT.UserDefinedID, AppointmentsT.Date, "
				"AppointmentPurposeT.PurposeID, ProcedureT.Name, BillsT.ID, AppointmentsT.ID";
			return _T(sql);
			break;
			}

		case 264:
			//Scheduling Totals
			/* Version History
				TES 2/17/04 - Enabled Appointment subfilters.
			*/
			return _T("SELECT AppointmentsT.ID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "
			"	 CASE WHEN PersonT.ID < 0 THEN 'No Patient' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS PatName, "
			"    ResourceT.Item, ResourceT.ID AS ResourceID,  "
			"    AppointmentsT.Date AS Date,   "
			"    AptTypeT.Name AS SetName,   "
			"    AptTypeT.ID AS SetID, "
			"    AptPurposeT.Name AS Purpose,  "
			"    AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, "
			"    AppointmentsT.ID AS ApptID "
			"FROM (AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID) LEFT OUTER JOIN  "
			"    (AppointmentPurposeT INNER JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID) ON   "
			"    AppointmentsT.ID = AppointmentPurposeT.AppointmentID LEFT OUTER JOIN "
			"    AptTypeT ON   "
			"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN  "
			"    (AppointmentResourceT INNER JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID) ON   "
			"    AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT OUTER JOIN  "
			"    PersonT INNER JOIN  "
			"    PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
			"    AppointmentsT.PatientID = PatientsT.PersonID  "
			"WHERE (AppointmentsT.Status <> 4) AND (AppointmentsT.ShowState <> 3) ");
			break;

		case 281:
			{
			//Consults to Procedures
			//Note:  Make sure to manually fix the notes field to be a memo of length 4000,
			/* Version History
				TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			*/
			switch(nSubLevel) {
			case 1:
				switch(nSubRepNum) {
				case 0:
					{
					//consult sublevel
						return _T("SELECT "
							"ConsultsSubQ.ID AS ConsultID, "
							"ProceduresSubQ.ID, ProceduresSubQ.PatientID AS PatID, AptPurposeT.Name AS Purpose, "
							"ProceduresSubQ.StartTime AS Date, ProceduresSubQ.EndTime, ProceduresSubQ.Notes, ProceduresSubQ.LocationID AS LocID, "
							"ProceduresSubQ.PurposeID AS PurposeID "
							"FROM "
							"  AptPurposeT INNER JOIN "
							"   (SELECT "
							"      AppointmentsT.ID AS ID, AppointmentsT.PatientID, "
							"      AppointmentPurposeT.PurposeID, "
							"      AppointmentsT.StartTime AS StartTime, "
							"      AppointmentsT.EndTime AS EndTime, "
							"      AppointmentsT.Notes AS Notes, AppointmentsT.LocationID "
							"   FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
							"   WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4) AND AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 "
							"   ) AS ProceduresSubQ ON AptPurposeT.ID = ProceduresSubQ.PurposeID "
							"INNER JOIN "
							"   (SELECT AppointmentsT.ID, "
							"      AppointmentsT.PatientID, "
							"      AppointmentPurposeT.PurposeID, "
							"      AppointmentsT.AptTypeID, "
							"      AppointmentsT.StartTime, "
							"      AppointmentsT.EndTime, "
							"      AppointmentsT.Notes, "
							"      AppointmentsT.LocationID "
							"   FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
							"   WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 AND AppointmentsT.StartTime < GetDate() "
							"   ) AS ConsultsSubQ ON ProceduresSubQ.PatientID = ConsultsSubQ.PatientID AND AptPurposeT.ID = ConsultsSubQ.PurposeID AND ConsultsSubQ.StartTime < ProceduresSubQ.StartTime");
						break;
					}//end case 0
				default:
					return _T("");
					break;
				}//end switch - nSubRepNum
			default:
				//Manually switch ConsPurposes and ConsDesc to be memo fields.
					{
						CString sql = "SELECT ConsultsSubQ.ID, ConsultsSubQ.PatientID AS PatID, "
							"   PatientsT.UserDefinedID, "
							"   PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
							"   ConsultsSubQ.StartTime AS Date, "
							"   ConsultsSubQ.EndTime, "
							"   ConsultsSubQ.PurposeID AS PurposeID, "
							"   AptPurposeT.Name AS ConsPurposes, "
							"   ConsultsSubQ.Notes AS ConsDesc, "
							"   AptPurposeT.Name AS Purpose, "
							"   ConsultsSubQ.LocationID AS LocID "
							"FROM "
							"/* This selects all consults which have a surgery at a date later than the consult date.  It also gives you the ability to look at the procedure "
							"		information, but that is read from a stored subreport */ "
							"  AptPurposeT INNER JOIN "
							"   (SELECT AppointmentsT.ID, "
							"      AppointmentsT.PatientID, "
							"      AppointmentPurposeT.PurposeID, "
							"      AppointmentsT.AptTypeID, "
							"      AppointmentsT.StartTime, "
							"      AppointmentsT.EndTime, "
							"      AppointmentsT.Notes, "
							"      AppointmentsT.LocationID "
							"   FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
							"   WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 AND AppointmentsT.StartTime < GetDate() "
							"	@ReportDateFrom @ReportDateTo "
							"   ) AS ConsultsSubQ ON AptPurposeT.ID = ConsultsSubQ.PurposeID "
							"INNER JOIN "
							"   (SELECT AppointmentsT.ID, AppointmentsT.PatientID, "
							"      AppointmentPurposeT.PurposeID, "
							"      AppointmentsT.StartTime AS StartTime, "
							"      AppointmentsT.EndTime AS EndTime, "
							"      AppointmentsT.Notes AS Notes "
							"   FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
							"   WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4) AND AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 "
							"   ) AS ProceduresSubQ ON ConsultsSubQ.PatientID = ProceduresSubQ.PatientID AND AptPurposeT.ID = ProceduresSubQ.PurposeID AND ProceduresSubQ.StartTime > ConsultsSubQ.StartTime "
							"   LEFT JOIN (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) ON ConsultsSubQ.PatientID = PersonT.ID"
							"	GROUP BY ConsultsSubQ.ID, ConsultsSubQ.PatientID , "
							"   PatientsT.UserDefinedID, "
							"   PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
							"   ConsultsSubQ.StartTime, "
							"   ConsultsSubQ.EndTime, "
							"   ConsultsSubQ.PurposeID, "
							"   AptPurposeT.Name, "
							"   ConsultsSubQ.Notes, "
							"   AptPurposeT.Name, "
							"   ConsultsSubQ.LocationID ";

						if(nDateRange > 0){	//date range chosen
							CString str;
							str.Format("AND AppointmentsT.Date >= %s", DateFrom.Format("'%m/%d/%Y'"));
							sql.Replace("@ReportDateFrom", str);
							COleDateTimeSpan dtSpan;
							COleDateTime dt;
							dtSpan.SetDateTimeSpan(1,0,0,0);
							dt = DateTo;
							dt += dtSpan;
							str.Format("AND AppointmentsT.Date < %s", dt.Format("'%m/%d/%Y'"));
							sql.Replace("@ReportDateTo", str);
						}
						else{	//no date range chosen, so no filter
							sql.Replace("@ReportDateFrom", "");
							sql.Replace("@ReportDateTo", "");
						}

						return _T(sql);						
						break;
					}//end default
				}//end switch
			}//end report

	case 287:
		{
			//Appointments by Input Date
			/*	Version History
				DRT 4/10/03 - The label was saying "Appt Date", but it was actually filtering on CreatedDate.  Changed
						the label, but putting this note here in case someone says "hey that used to filter on appt date".
						Well no it didn't, it just pretended to.
				TES 3/4/04 - Implemented multi-resource support!
				TES 7/13/04 - Don't forget to manually set Notes field to be a memo in the .ttx
				TES 8/23/05 - Changed resource filtering.
				DRT 4/7/2006 - PLID 14563 - Added primary referral
				(j.anspach 05-30-2006 16:09 PLID 19090) - Adding AppointmentsT.ArrivalTime
				// (j.gruber 2007-10-31 16:08) - PLID 27939 - fixed the joins
			*/
			CString sql = "SELECT AppointmentsT.StartTime "
			"    AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last,  "
			"    AppointmentsT.Date AS ApptDate, "
			"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item,  "
			"    AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone,  "
			"    AptTypeT.ID AS AptTypeID, "
			"-1 AS ResourceID, "
			"    CASE WHEN PersonT.ID = -25 THEN 'No Patient' ELSE Last + ', ' + First + ' ' + Middle END AS PatName, AppointmentsT.ID AS ApptID, AppointmentsT.PatientID, "
			"AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, PersonT.ID AS PatID, PersonT.PrivHome, "
			"PersonT.PrivWork, AppointmentsT.CreatedDate AS Date, AppointmentsT.CreatedLogin, "
			" dbo.GetPurposeString(AppointmentsT.ID) AS PurposeString, ReferralSourceT.Name AS PrimaryReferralSource "
			" FROM AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
			" LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
			" LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID  "
			" LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID "
			" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID  "

			"WHERE (AppointmentsT.Status <> 4) AND "
			"    (AppointmentsT.ShowState <> 3)";
			return _T(sql);
		}
		break;

	case 298:
		{
		//Scheduling Count
		/*	Version History
			DRT 5/23/03 - Added dtld/smry option (dtld shows purposes).  No ttx change, but I changed the PurposeName
					to show as <No Purpose> if it was NULL.
			DRT 6/19/03 - Removed references to AptPurposeID, it is an obsolete field.
			TES 2/16/04 - Changed ShowState to be the ID, which some formulas in the report expect.  It seems obvious
					that nobody has run this report since June 19, which is disappointing.
			TES 2/17/04 - Enabled Appointment subfilters.
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
		*/
		CString sql = "SELECT AppointmentsT.ID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "
			"PersonT.First + ', ' + PersonT.Last + ' ' + PersonT.Middle AS PatName, "
			"ResourceT.Item, ResourceT.ID AS ResourceID,  "
			"AppointmentsT.Date AS Date,   "
			"CASE WHEN AptTypeT.ID IS NULL THEN 'No Type' ELSE AptTypeT.Name END AS TypeName, "
			"AptTypeT.ID AS SetID, "
			"CASE WHEN AptPurposeT.Name IS NULL THEN '<No Purpose>' ELSE AptPurposeT.Name END AS PurposeName, "
			"AppointmentsT.LocationID AS LocID, AptShowStateT.ID AS ShowState, "
			"AppointmentsT.ID AS ApptID "
			"FROM AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
			"LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			"LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID LEFT JOIN "
			"AptShowStateT ON AppointmentsT.ShowState = AptShowstateT.ID "
			"LEFT JOIN "
			"AptTypeT ON   "
			"AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
			"LEFT JOIN PersonT INNER JOIN  "
			"PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
			"AppointmentsT.PatientID = PatientsT.PersonID  "
			"WHERE (AppointmentsT.Status <> 4) ";
			return _T(sql);
		break;
		}

	case 303: //No Show...
	case 304: //No Show...
	case 305: //No Show...
	case 306: //No Show...
	case 381: //No Show...
		/*	Version History
			DRT 2/10/2004 - PLID 10646 - Made all these reports editable.  Code is in GetTtxContents() function to handle the special lengths
				of scheduler fields.
			TES 2/17/04 - Enabled Appointment subfilters.
			(a.wilson 2012-3-20) PLID 49065 - adding the birthdate field for no show reports.
		*/
	

		if (saExtraValues.GetSize()) {
			
			//they are using the extended filter, so we have to format the SQL statement accordingly
			CString strSQL;

			CString strResourceIDs;
			for(int i = 0; i < saExtraValues.GetSize(); i++) {
				strResourceIDs += "'" + _Q(saExtraValues[i]) + "',";
			}
			strResourceIDs.TrimRight(",");
			
			strSQL.Format("SELECT PatientsT.UserDefinedID, CASE WHEN PersonT.ID IS NULL THEN 'No Patient' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS PatName, AppointmentsT.Date AS Date, AptTypeT.ID AS TypeID, AptTypeT.Name AS TypeName, "
			"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, LocationsT.ID AS LocID, LocationsT.Name AS CreatedLoc, ReferralSourceT.PersonID AS ReferralID, "
			"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END AS ReferralName, PersonT1.ID AS CoordID, CASE WHEN PersonT1.ID IS NULL THEN 'No Coordinator' ELSE PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle END AS CoordName, AppointmentsT.StartTime, AppointmentsT.EndTime, "
			"AppointmentsT.Notes, PersonT.WorkPhone, PersonT.HomePhone, PersonT.CellPhone, PersonT.Email, PersonT.ID AS PatID, CASE WHEN LocationsT2.ID IS NULL THEN 'No Location' ELSE LocationsT2.Name END AS LocName, AppointmentsT.CreatedLogin, "
			"AppointmentsT.ID AS ApptID, PersonT.Birthdate "
			""
			"FROM "
			"AppointmentsT INNER JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
			"LEFT JOIN LocationsT LocationsT2 ON PersonT.Location = LocationsT2.ID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN UsersT ON PatientsT.EmployeeID = UsersT.PersonID LEFT JOIN PersonT AS PersonT1 ON UsersT.PersonID = PersonT1.ID "
			"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID  "
			"WHERE (AppointmentsT.ShowState = 3) AND (AppointmentsT.Status <> 4) AND AppointmentResourceT.ResourceID IN (%s) "
			"GROUP BY  PatientsT.UserDefinedID, CASE WHEN PersonT.ID IS NULL THEN 'No Patient' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END, AppointmentsT.Date, AptTypeT.ID, AptTypeT.Name, "
			"dbo.GetPurposeString(AppointmentsT.ID), LocationsT.ID, LocationsT.Name, ReferralSourceT.PersonID, "
			"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END, PersonT1.ID, CASE WHEN PersonT1.ID IS NULL THEN 'No Coordinator' ELSE PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle END, AppointmentsT.StartTime, AppointmentsT.EndTime, "
			"AppointmentsT.Notes, PersonT.WorkPhone, PersonT.HomePhone, PersonT.CellPhone, PersonT.Email, PersonT.ID, CASE WHEN LocationsT2.ID IS NULL THEN 'No Location' ELSE LocationsT2.Name END, AppointmentsT.CreatedLogin, AppointmentsT.ID, PersonT.Birthdate ",
			strResourceIDs);

			return strSQL;
						
		}
		else {

			
			return _T("SELECT PatientsT.UserDefinedID, CASE WHEN PersonT.ID IS NULL THEN 'No Patient' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS PatName, AppointmentsT.Date AS Date, AptTypeT.ID AS TypeID, AptTypeT.Name AS TypeName, "
				"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, LocationsT.ID AS LocID, LocationsT.Name AS CreatedLoc, ReferralSourceT.PersonID AS ReferralID, "
				"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END AS ReferralName, PersonT1.ID AS CoordID, CASE WHEN PersonT1.ID IS NULL THEN 'No Coordinator' ELSE PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle END AS CoordName, AppointmentsT.StartTime, AppointmentsT.EndTime, "
				"AppointmentsT.Notes, PersonT.WorkPhone, PersonT.HomePhone, PersonT.CellPhone, PersonT.Email, PersonT.ID AS PatID, CASE WHEN LocationsT2.ID IS NULL THEN 'No Location' ELSE LocationsT2.Name END AS LocName, AppointmentsT.CreatedLogin, "
				"AppointmentsT.ID AS ApptID, PersonT.Birthdate "
				""
				"FROM "
				"AppointmentsT INNER JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
				"LEFT JOIN LocationsT LocationsT2 ON PersonT.Location = LocationsT2.ID "
				"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
				"LEFT JOIN UsersT ON PatientsT.EmployeeID = UsersT.PersonID LEFT JOIN PersonT AS PersonT1 ON UsersT.PersonID = PersonT1.ID "
				""
				"WHERE AppointmentsT.ShowState = 3 AND AppointmentsT.Status <> 4");
		}
		break;

	case 307:
		//Consults To Surgeries (by Patient)
	case 308:
		//Consults To Surgeries (by Location)
	case 309:
		//Consults To Surgeries (by Referral Source)
	case 310:
		//Consults To Surgeries (by Patient Coordinator)
	case 382:
		//Consults To Surgeries (by Type)
		/*	Version History
			DRT 4/3/03 - Added (by Type) version
			DRT 6/17/03 - Added a few fields, made it editable.
			DRT 6/19/03 - Removed references to AptPurposeID, it's obsolete
			TES 8/4/03 - Included EndTime
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
		*/
		return _T("SELECT ConsultsSubQ.PatientID AS PatID, "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"ConsultsSubQ.StartTime AS Date, "
			"ConsultsSubQ.EndTime AS ConsEndTime, "
			"ConsultsSubQ.Notes AS ConsDesc, "
			"dbo.GetPurposeString(ConsultsSubQ.ID) AS Purpose, "
			"ProceduresSubQ.Notes AS SurgDesc, "
			"ProceduresSubQ.StartTime AS SurgTime, "
			"ProceduresSubQ.EndTime AS SurgEndTime, "
			"PersonT.Location AS LocID, "
			"CASE WHEN LocationsT2.Name IS NULL THEN 'No Location' ELSE LocationsT2.Name END AS LocName, "
			"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Source' ELSE ReferralSourceT.Name END AS RefName, "
			"CASE WHEN PersonT1.ID IS NULL THEN 'No Coordinator' ELSE PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle END AS CoordName, "
			"CASE WHEN LocationsT.ID IS NULL THEN 'No Location' ELSE LocationsT.Name END AS CreatedLoc, "
			"dbo.GetResourceString(ConsultsSubQ.ID) AS ResourceName, PersonT.HomePhone, PersonT.WorkPhone, PersonT.Email, PersonT.CellPhone, AptTypeT.Name AS ConsType, AptTypeT.ID AS TypeID, "
			"ConsultsSubQ.CreatedLogin, AptTypeT2.Name AS SurgType, CASE WHEN PatientsT.CurrentStatus = 1 THEN 'Patient' WHEN PatientsT.CurrentStatus = 2 THEN 'Prospect' WHEN PatientsT.CurrentStatus = 3 THEN 'Patient/Prospect' END AS PatientStatus, "
			"dbo.GetPurposeString(ProceduresSubQ.ApptID) AS SurgeryPurpose,  dbo.GetPurposeString(ProceduresSubQ.ApptID) AS SurgeryPurposeShort, (SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 1 AND PersonID = PersonT.ID) AS Custom1, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 2 AND PersonID = PersonT.ID) AS Custom2, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 3 AND PersonID = PersonT.ID) AS Custom3, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 4 AND PersonID = PersonT.ID) AS Custom4 "
			"FROM "
			"/* This is intended to select the first consult against the first surgery ... we're not comparing the purposes */"
			"(SELECT Min(AppointmentsT.ID) AS ID, AppointmentsT.PatientID, "
			"  Min(AppointmentsT.AptTypeID) AS AptTypeID, "
			"  Min(AppointmentsT.StartTime) AS StartTime, "
			"  Min(AppointmentsT.EndTime) AS EndTime, "
			"  Min(AppointmentsT.CreatedLogin) AS CreatedLogin, "
			"  Min(AppointmentsT.Notes) AS Notes, "
			"  Min(AppointmentsT.LocationID) AS LocationID "
			"FROM AppointmentsT "
			"WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 AND AppointmentsT.StartTime < GetDate() "
			"GROUP BY PatientID "
			")AS ConsultsSubQ INNER JOIN "
			"(SELECT AppointmentsT.PatientID, "
			"  Min(AppointmentsT.StartTime) AS StartTime, "
			"  Min(AppointmentsT.EndTime) AS EndTime, "
			"  Min(AppointmentsT.Notes) AS Notes, "
			"  Min(AppointmentsT.AptTypeID) AS AptTypeID, "
			"  Min(AppointmentsT.ID) AS ApptID "
			"FROM AppointmentsT "
			"WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4) AND AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 "
			"GROUP BY AppointmentsT.PatientID "
			") AS ProceduresSubQ ON ConsultsSubQ.PatientID = ProceduresSubQ.PatientID AND ProceduresSubQ.StartTime > ConsultsSubQ.StartTime "
			"LEFT JOIN (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) ON ConsultsSubQ.PatientID = PersonT.ID "
			"LEFT JOIN AptTypeT ON ConsultsSubQ.AptTypeID = AptTypeT.ID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN PersonT PersonT1 ON PatientsT.EmployeeID = PersonT1.ID "
			"LEFT JOIN LocationsT ON ConsultsSubQ.LocationID = LocationsT.ID "
			"LEFT JOIN LocationsT LocationsT2 ON PersonT.Location = LocationsT2.ID "
			"LEFT JOIN AptTypeT AptTypeT2 ON ProceduresSubQ.AptTypeID = AptTypeT2.ID");
		break;
	case 311:
		//Consults To Surgeries (by Resource)
		/*	Version History
			DRT 6/19/03 - Removed references to AptPurposeID, it's obsolete
			TES 8/4/03 - Included EndTime
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
		*/
		return _T("SELECT ConsultsSubQ.PatientID AS PatID, "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"ConsultsSubQ.StartTime AS Date, "
			"ConsultsSubQ.EndTime AS ConsEndTime, "
			"ConsultsSubQ.Notes AS ConsDesc, "
			"dbo.GetPurposeString(ConsultsSubQ.ID) AS Purpose, "
			"ProceduresSubQ.Notes AS SurgDesc, "
			"ProceduresSubQ.StartTime AS SurgTime, "
			"ProceduresSubQ.EndTime AS SurgEndTime, "
			"PersonT.Location AS LocID, "
			"CASE WHEN LocationsT2.Name IS NULL THEN 'No Location' ELSE LocationsT2.Name END AS LocName, "
			"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Source' ELSE ReferralSourceT.Name END AS RefName, "
			"CASE WHEN PersonT1.ID IS NULL THEN 'No Coordinator' ELSE PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle END AS CoordName, "
			"CASE WHEN LocationsT.ID IS NULL THEN 'No Location' ELSE LocationsT.Name END AS CreatedLoc, "
			"ResourceT.Item AS ResourceName, PersonT.HomePhone, PersonT.WorkPhone, PersonT.Email, PersonT.CellPhone, AptTypeT.Name AS ConsType, AptTypeT.ID AS TypeID, "
			"ConsultsSubQ.CreatedLogin, AptTypeT2.Name AS SurgType, CASE WHEN PatientsT.CurrentStatus = 1 THEN 'Patient' WHEN PatientsT.CurrentStatus = 2 THEN 'Prospect' WHEN PatientsT.CurrentStatus = 3 THEN 'Patient/Prospect' END AS PatientStatus, "
			"dbo.GetPurposeString(ProceduresSubQ.ApptID) AS SurgeryPurpose, dbo.GetPurposeString(ProceduresSubQ.ApptID) AS SurgeryPurposeShort,  (SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 1 AND PersonID = PersonT.ID) AS Custom1, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 2 AND PersonID = PersonT.ID) AS Custom2, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 3 AND PersonID = PersonT.ID) AS Custom3, "
			"(SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 4 AND PersonID = PersonT.ID) AS Custom4 "
			"FROM "
			"/* This is intended to select the first consult against the first surgery ... we're not comparing the purposes */"
			"(SELECT Min(AppointmentsT.ID) AS ID, AppointmentsT.PatientID, "
			"  Min(AppointmentsT.AptTypeID) AS AptTypeID, "
			"  Min(AppointmentsT.StartTime) AS StartTime, "
			"  Min(AppointmentsT.EndTime) AS EndTime, "
			"  Min(AppointmentsT.CreatedLogin) AS CreatedLogin, "
			"  Min(AppointmentsT.Notes) AS Notes, "
			"  Min(AppointmentsT.LocationID) AS LocationID "
			"FROM AppointmentsT "
			"WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 AND AppointmentsT.StartTime < GetDate() "
			"GROUP BY PatientID "
			")AS ConsultsSubQ INNER JOIN "
			"(SELECT AppointmentsT.PatientID, "
			"  Min(AppointmentsT.StartTime) AS StartTime, "
			"  Min(AppointmentsT.EndTime) AS EndTime, "
			"  Min(AppointmentsT.Notes) AS Notes, "
			"  Min(AppointmentsT.AptTypeID) AS AptTypeID, "
			"  Min(AppointmentsT.ID) AS ApptID "
			"FROM AppointmentsT "
			"WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4) AND AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 "
			"GROUP BY AppointmentsT.PatientID "
			") AS ProceduresSubQ ON ConsultsSubQ.PatientID = ProceduresSubQ.PatientID AND ProceduresSubQ.StartTime > ConsultsSubQ.StartTime "
			"LEFT JOIN (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) ON ConsultsSubQ.PatientID = PersonT.ID "
			"LEFT JOIN AptTypeT ON ConsultsSubQ.AptTypeID = AptTypeT.ID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN PersonT PersonT1 ON PatientsT.EmployeeID = PersonT1.ID "
			"LEFT JOIN LocationsT ON ConsultsSubQ.LocationID = LocationsT.ID "
			"LEFT JOIN (AppointmentResourceT INNER JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID) ON ConsultsSubQ.ID = AppointmentResourceT.AppointmentID "
			"LEFT JOIN LocationsT LocationsT2 ON PersonT.Location = LocationsT2.ID "
			"LEFT JOIN AptTypeT AptTypeT2 ON ProceduresSubQ.AptTypeID = AptTypeT2.ID");
		break;

	case 312:
		//Consults without Surgeries (by Patient)
	case 313:
		//Consults without Surgeries (by Location)
	case 314:
		//Consults without Surgeries (by Referral Source)
	case 315:
		//Consults without Surgeries (by Patient Coordinator)
	case 383:
		//Consults without Surgeries (by Type)
		/*	Version History
			1/14/03 - DRT - Removed no show surgeries from affecting the results.  If you have a consult, but no show a surgery, you will be listed as 
					"consulted without surgery" since you didn't show up (strangely the "by resource" version already had this functionality)
			DRT 4/3/03 - Added (by Type) version
			TES 2/17/04 - Enabled Appointment subfilters.
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
		*/
		{
		CString strSQL = _T("SELECT "
			"PatientPurposesConsultNoSurgeryQ.PatientID AS PatID, "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"dbo.GetPurposeString(PatientPurposesConsultNoSurgeryQ.ID) AS PurposeName, AptTypeT.Name AS TypeName, AptTypeT.ID AS TypeID, "
			"PersonT.Location AS LocID, CASE WHEN LocationsT.ID IS NULL THEN 'No Location' ELSE LocationsT.Name END AS CreateLocName, "
			"dbo.GetResourceString(PatientPurposesConsultNoSurgeryQ.ID) AS ResourceName, "
			"CASE WHEN PersonT2.ID IS NULL THEN 'No Coordinator' ELSE PersonT2.Last + ', ' + PersonT2.First + ' ' + PersonT2.Middle END AS CoordName, "
			"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Source' ELSE ReferralSourceT.Name END AS ReferralName, "
			"PatientPurposesConsultNoSurgeryQ.Date AS Date, PatientPurposesConsultNoSurgeryQ.StartTime, PatientPurposesConsultNoSurgeryQ.EndTime, "
			"CASE WHEN LocationsT2.ID IS NULL THEN 'No Location' ELSE LocationsT2.Name END AS LocName, PatientPurposesConsultNoSurgeryQ.CreatedLogin, PersonT.HomePhone, PersonT.WorkPhone, "
			"PersonT.Email, PersonT.CellPhone, PatientPurposesConsultNoSurgeryQ.Notes, "
			"PatientPurposesConsultNoSurgeryQ.ID AS ApptID "
			"FROM PersonT "
			"INNER JOIN "
			"(/* PatientPurposesConsultNoSurgeryQ: Subquery to generate the list a patients who have a consult appointment not followed by a surgery */ "
			"  /* For each consult */ "
			"  SELECT AppointmentsT.ID, AppointmentsT.PatientID, Appointmentst.AptTypeID, AppointmentsT.LocationID, AppointmentsT.Date, AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.CreatedLogin, AppointmentsT.Notes "
			"  FROM AppointmentsT "
			"  WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND AppointmentsT.StartTime >= @ReportDateFrom AND AppointmentsT.StartTime < @ReportDateTo AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND "
			"  /* Ignore it if there is any other appointment whose patient is the same, but whose type is a surgery, and date is later */ "
			"  NOT EXISTS (SELECT * FROM AppointmentsT AptsSurgeryQ WHERE AptsSurgeryQ.Status <> 4 AND AptsSurgeryQ.ShowState <> 3 AND AptsSurgeryQ.PatientID = AppointmentsT.PatientID AND AptsSurgeryQ.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4) AND AptsSurgeryQ.StartTime >= AppointmentsT.StartTime) "
			"  GROUP BY AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, AppointmentsT.LocationID, AppointmentsT.Date, AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.CreatedLogin, AppointmentsT.Notes "
			") PatientPurposesConsultNoSurgeryQ ON PersonT.ID = PatientPurposesConsultNoSurgeryQ.PatientID "
			"LEFT JOIN AptTypeT ON PatientPurposesConsultNoSurgeryQ.AptTypeID = AptTypeT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN PersonT PersonT2 ON PatientsT.EmployeeID = PersonT2.ID "
			"LEFT JOIN LocationsT ON PatientPurposesConsultNoSurgeryQ.LocationID = LocationsT.ID "
			"LEFT JOIN LocationsT LocationsT2 ON PersonT.Location = LocationsT2.ID "
			"WHERE PersonT.ID > 0");
			if(nDateRange > 0){	//date range chosen
				strSQL.Replace("@ReportDateFrom", DateFrom.Format("'%m/%d/%Y'"));
				COleDateTimeSpan dtSpan;
				COleDateTime dt;
				dtSpan.SetDateTimeSpan(1,0,0,0);
				dt = DateTo;
				dt += dtSpan;
				strSQL.Replace("@ReportDateTo", dt.Format("'%m/%d/%Y'"));
			}
			else{	//no date range chosen, so select from whatever the low date is, up until today ... there isn't any reason to show consults w/o surgeries when the consult hasn't even happened yet
				COleDateTime dtToday;
				dtToday = COleDateTime::GetCurrentTime();
				strSQL.Replace("@ReportDateFrom", dtMin.Format("'%m/%d/%Y'"));
				strSQL.Replace("@ReportDateTo", dtToday.Format("'%m/%d/%Y'"));
			}
			return strSQL;
		break;
		}
	case 316:
		//Consults without Surgeries (by Resource)
		/* Version History
			TES 2/17/04 - Enabled Appointment subfilters.
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
		*/
		{
		CString strSQL = _T("SELECT "
			"PatientPurposesConsultNoSurgeryQ.PatientID AS PatID, "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"dbo.GetPurposeString(PatientPurposesConsultNoSurgeryQ.ID) AS PurposeName, AptTypeT.Name AS TypeName, AptTypeT.ID AS TypeID, "
			"PersonT.Location AS LocID, CASE WHEN LocationsT.ID IS NULL THEN 'No Location' ELSE LocationsT.Name END AS CreateLocName, "
			"ResourceT.Item AS ResourceName, "
			"CASE WHEN PersonT2.ID IS NULL THEN 'No Coordinator' ELSE PersonT2.Last + ', ' + PersonT2.First + ' ' + PersonT2.Middle END AS CoordName, "
			"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Source' ELSE ReferralSourceT.Name END AS ReferralName, "
			"PatientPurposesConsultNoSurgeryQ.Date AS Date, PatientPurposesConsultNoSurgeryQ.StartTime, PatientPurposesConsultNoSurgeryQ.EndTime, "
			"CASE WHEN LocationsT2.ID IS NULL THEN 'No Location' ELSE LocationsT2.Name END AS LocName, PatientPurposesConsultNoSurgeryQ.CreatedLogin, PersonT.HomePhone, PersonT.WorkPhone, "
			"PersonT.Email, PersonT.CellPhone, PatientPurposesConsultNoSurgeryQ.Notes, "
			"PatientPurposesConsultNoSurgeryQ.ID AS ApptID "
			"FROM PersonT "
			"INNER JOIN "
			"(/* PatientPurposesConsultNoSurgeryQ: Subquery to generate the list a patients who have a consult appointment not followed by a surgery */ "
			"  /* For each consult */ "
			"  SELECT AppointmentsT.ID, AppointmentsT.PatientID, Appointmentst.AptTypeID, AppointmentsT.LocationID, AppointmentsT.Date, AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.CreatedLogin, AppointmentsT.Notes "
			"  FROM AppointmentsT "
			"  WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND AppointmentsT.StartTime >= @ReportDateFrom AND AppointmentsT.StartTime < @ReportDateTo AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND "
			"  /* Ignore it if there is any other appointment whose patient is the same, but whose type is a surgery, and date is later */ "
			"  NOT EXISTS (SELECT * FROM AppointmentsT AptsSurgeryQ WHERE AptsSurgeryQ.Status <> 4 AND AptsSurgeryQ.PatientID = AppointmentsT.PatientID AND AptsSurgeryQ.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4) AND AptsSurgeryQ.StartTime >= AppointmentsT.StartTime) "
			"  GROUP BY AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, AppointmentsT.LocationID, AppointmentsT.Date, AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.CreatedLogin, AppointmentsT.Notes "
			") PatientPurposesConsultNoSurgeryQ ON PersonT.ID = PatientPurposesConsultNoSurgeryQ.PatientID "
			"LEFT JOIN AptTypeT ON PatientPurposesConsultNoSurgeryQ.AptTypeID = AptTypeT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN PersonT PersonT2 ON PatientsT.EmployeeID = PersonT2.ID "
			"LEFT JOIN LocationsT ON PatientPurposesConsultNoSurgeryQ.LocationID = LocationsT.ID "
			"LEFT JOIN LocationsT LocationsT2 ON PersonT.Location = LocationsT2.ID "
			"LEFT JOIN (AppointmentResourceT INNER JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID) ON PatientPurposesConsultNoSurgeryQ.ID = AppointmentResourceT.AppointmentID "
			"WHERE PersonT.ID > 0");
			if(nDateRange > 0){	//date range chosen
				strSQL.Replace("@ReportDateFrom", DateFrom.Format("'%m/%d/%Y'"));
				COleDateTimeSpan dtSpan;
				COleDateTime dt;
				dtSpan.SetDateTimeSpan(1,0,0,0);
				dt = DateTo;
				dt += dtSpan;
				strSQL.Replace("@ReportDateTo", dt.Format("'%m/%d/%Y'"));
			}
			else{	//no date range chosen, so select from whatever the low date is, up until today ... there isn't any reason to show consults w/o surgeries when the consult hasn't even happened yet
				COleDateTime dtToday;
				dtToday = COleDateTime::GetCurrentTime();
				strSQL.Replace("@ReportDateFrom", dtMin.Format("'%m/%d/%Y'"));
				strSQL.Replace("@ReportDateTo", dtToday.Format("'%m/%d/%Y'"));
			}
			return strSQL;
		break;
		}

	case 317:
		//Consults Cancelled and Not Rescheduled (by Patient)
	case 318:
		//Consults Cancelled and Not Rescheduled (by Location)
	case 319:
		//Consults Cancelled and Not Rescheduled (by Referral Source)
	case 320:
		//Consults Cancelled and Not Rescheduled (by Patient Coordinator)
	case 384:
		//Consults Cancelled and Not Rescheduled (by Type)
		/* Version History 
			TES 2/17/04 - Enabled Appointment subfilters.
		*/
		return _T("SELECT "
			"AppointmentsT.ID AS ApptID, Date AS Date, StartTime, EndTime, AppointmentsT.Notes, AptTypeT.Name AS TypeName, dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END AS RefName, "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Patient' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS PatName, CASE WHEN PersonT1.ID IS NULL THEN 'No Coordinator' ELSE PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle END AS CoordName, "
			"CASE WHEN LocationsT.ID IS NULL THEN 'No Location' ELSE LocationsT.Name END AS CreatedLoc, LocationsT.ID AS LocID, PersonT.ID AS PatID, ReferralSourceT.PersonID AS ReferralID, "
			"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email, AppointmentsT.CreatedLogin, CASE WHEN LocationsT2.ID IS NULL THEN 'No Location' ELSE LocationsT2.Name END AS LocName, "
			"CASE WHEN AptCancelReasonT.Description IS NOT NULL THEN AptCancelReasonT.Description ELSE CASE WHEN Len(CancelledReason) = 0 THEN '<None Specified>' ELSE CancelledReason END END AS CancelReason "
			"FROM "
			"AppointmentsT INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN PersonT PersonT1 ON PatientsT.EmployeeID = PersonT1.ID "
			"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
			"LEFT JOIN LocationsT LocationsT2 ON PersonT.Location = LocationsT2.ID "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN AptCancelReasonT ON AppointmentsT.CancelReasonID = AptCancelReasonT.ID "
			"INNER JOIN "
			"(SELECT AppointmentsT.ID "
			"FROM AppointmentsT "
			"WHERE AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status = 4 AND AppointmentsT.PatientID > 0 "
			") AS ConsultsSubQ ON AppointmentsT.ID = ConsultsSubQ.ID "
			"WHERE AppointmentsT.PatientID NOT IN "
			"/*Patients that have a consult past the date above, but not canceled*/ "
			"(SELECT ApptsQ.PatientID "
			"FROM AppointmentsT ApptsQ "
			"WHERE (ApptsQ.Date >= AppointmentsT.Date) AND (ApptsQ.Status <> 4) AND ApptsQ.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) "
			")");
		break;

	case 370: //Scheduling Productivity by User
		/*	Version History
			DRT 4/23/03 - Removed "use group" filter.  It didn't work, and I'm not sure how much it applies.  Put in a suggestion to re-add it
					later if we feel it's important.
			DRT 8/6/03 - Added a group/filter option for the report.  However, the way this query is setup, filters cannot be used without
					triggering a bug in sql server.  See the comments in the bUseFilter section for more info.
					Also fixed a bug with the date filters not filtering on the first day of your date range.
			JMM 5/11/04 - Added Surgeries Scheduled which links the surgeries to the consults by patientID, not procedure
			TES 8/5/2009 - PLID 35047 - Don't count "Other Procedural" appointments as surgeries
			AS UserName
			// (j.jones 2010-02-04 15:43) - PLID 36500 - changed UserName to be exposed AS UserName
		*/
		{
			CString strSql = "SELECT CreatedLogin AS UserName, Sum(ConsultsScheduled) as ConsultsScheduled, Sum(ConsultsSeen) AS ConsultsSeen, Sum(ConsultsBilled) AS ConsultsBilled,  "
				" Sum(TotalBilled) AS TotalBilled, Sum(totalReceived) As TotalReceived, Sum(SurgeriesScheduled) AS SurgeriesScheduled FROM  ( "
				
				" SELECT ConsultsScheduledQ.CreatedLogin, ConsultsScheduled, ConsultsSeen, ConsultsBilled, TotalBilled, TotalReceived, 0 AS SurgeriesScheduled FROM  "
				" "
				"(SELECT CreatedLogin, Count(ID) AS ConsultsScheduled FROM AppointmentsT WHERE AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 @DateFilter @LocationFilter @UseGroupFilter "
				"GROUP BY CreatedLogin) ConsultsScheduledQ LEFT JOIN  "
				" "
				"(SELECT CreatedLogin, Count(ID) AS ConsultsSeen FROM AppointmentsT WHERE AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 AND (AppointmentsT.ShowState = 1 OR AppointmentsT.ShowState = 2) @DateFilter @LocationFilter @UseGroupFilter "
				"GROUP BY CreatedLogin) ConsultsSeenQ ON ConsultsScheduledQ.CreatedLogin = ConsultsSeenQ.CreatedLogin LEFT JOIN "
				" "
				"(SELECT CreatedLogin, Count(ID) AS ConsultsBilled FROM AppointmentsT WHERE AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 AND (AppointmentsT.ShowState = 1 OR AppointmentsT.ShowState = 2) AND PatientID IN (SELECT PatientID FROM LineItemT WHERE Type = 10 AND Deleted = 0 AND LineItemT.Date >= AppointmentsT.Date) @DateFilter @LocationFilter @UseGroupFilter "
				"GROUP BY CreatedLogin) ConsultsBilledQ ON ConsultsScheduledQ.CreatedLogin = ConsultsBilledQ.CreatedLogin LEFT JOIN  "
				" "
				" "
				"(SELECT CreatedLogin, Sum(LineQ.Amount) AS TotalBilled FROM  "
				"(BillsT LEFT JOIN UsersT ON BillsT.PatCoord = UsersT.PersonID) INNER JOIN (SELECT ChargesT.BillID, LineItemT.PatientID, LineItemT.Date, dbo.GetChargeTotal(ChargesT.ID) AS Amount FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0) AS LineQ ON BillsT.ID = LineQ.BillID "
				"INNER JOIN  (SELECT PatientID, Date, CreatedLogin FROM AppointmentsT WHERE AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 AND (AppointmentsT.ShowState = 1 OR AppointmentsT.ShowState = 2) @DateFilter @LocationFilter @UseGroupFilter) ConsultsQ ON LineQ.PatientID = ConsultsQ.PatientID "
				"WHERE ConsultsQ.Date <= LineQ.Date " 
				"GROUP BY CreatedLogin) TotalBilledQ ON ConsultsScheduledQ.CreatedLogin = TotalBilledQ.CreatedLogin LEFT JOIN  "
				" "
				"(SELECT CreatedLogin, Sum(appliesT.Amount) AS TotalReceived FROM  "
				"BillsT INNER JOIN (SELECT LineItemT.ID, ChargesT.BillID, LineItemT.PatientID, LineItemT.Date, dbo.GetChargeTotal(ChargesT.ID) AS Amount FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0) AS LineQ ON BillsT.ID = LineQ.BillID INNER JOIN AppliesT ON LineQ.ID = AppliesT.DestID INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID  "
				"INNER JOIN  (SELECT PatientID, AppointmentsT.Date, AppointmentsT.CreatedLogin FROM AppointmentsT WHERE AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND AppointmentsT.Status <> 4 AND (AppointmentsT.ShowState = 1 OR AppointmentsT.ShowState = 2) @DateFilter @LocationFilter @UseGroupFilter) ConsultsQ ON LineQ.PatientID = ConsultsQ.PatientID  "
				"WHERE ConsultsQ.Date <= LineQ.Date AND LineItemT.Deleted = 0 "
				"GROUP BY CreatedLogin) TotalReceivedQ ON ConsultsScheduledQ.CreatedLogin = TotalReceivedQ.CreatedLogin "
				" UNION "
				" SELECT CreatedLogin, 0,0,0,0,0, Count(ID) AS SurgeriesScheduled FROM AppointmentsT WHERE AptTypeID IN  "
				" (SELECT ID FROM AptTypeT WHERE  "
				" AptTypeT.Category = 4 OR AptTypeT.Category = 3) AND AppointmentsT.Status <> 4 AND (AppointmentsT.ShowState = 1 OR AppointmentsT.ShowState = 2) "
				" @DateFilter @LocationFilter @UseGroupFilter "
				" Group By CreatedLogin) SchedProdQ  Group By CreatedLogin " ; 

			//Fix the date filtering.
			//DRT 8/6/03 -  This was filtering > and <, so if you tried filtering on 1 single day, it could never have 
			//		worked, and would have always excluded the first day in your range!
			if(nDateRange == 1) {
				if(nDateFilter == 1) {
					CString strDateFilter;
					COleDateTimeSpan dtSpan;
					COleDateTime dt;
					dtSpan.SetDateTimeSpan(1,0,0,0);
					dt = DateTo;
					dt += dtSpan;
					strDateFilter.Format(" AND Date >= '%s' AND Date < '%s' ", DateFrom.Format("%Y-%m-%d"), dt.Format("%Y-%m-%d"));
					strSql.Replace("@DateFilter", strDateFilter);
				}
				else if(nDateFilter == 2) {
					CString strDateFilter;
					COleDateTimeSpan dtSpan;
					COleDateTime dt;
					dtSpan.SetDateTimeSpan(1,0,0,0);
					dt = DateTo;
					dt += dtSpan;
					strDateFilter.Format(" AND CreatedDate >= '%s' AND CreatedDate < '%s' ", DateFrom.Format("%Y-%m-%d"), dt.Format("%Y-%m-%d"));
					strSql.Replace("@DateFilter", strDateFilter);
				}
			}
			else {
				//There's no date filter
				strSql.Replace("@DateFilter", "");
			}

			//Now the location filtering.
			//(e.lally 2008-10-01) PLID 31543 - Added support for filtering on multiple locations.
			if(nLocation > 0) {
				CString strLocationFilter;
				strLocationFilter.Format(" AND AppointmentsT.LocationID = %li", nLocation);
				strSql.Replace("@LocationFilter", strLocationFilter);
			}
			else if(nLocation == -1){
				strSql.Replace("@LocationFilter", "");
			}
			else if(nLocation == -3){
				CString strLocationFilter = " AND AppointmentsT.LocationID IN(";
				CString strPart;
				for(int i=0; i < m_dwLocations.GetSize(); i++) {
					strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
					strLocationFilter += strPart;
				}
				strLocationFilter = strLocationFilter.Left(strLocationFilter.GetLength()-2) + ")";
				strSql.Replace("@LocationFilter", strLocationFilter);
			}
			else if(nLocation == -2){
				//This will always be false
				CString strLocationFilter = " AND AppointmentsT.LocationID IS NULL ";
				strSql.Replace("@LocationFilter", strLocationFilter);

			}
			else {
				ASSERT(FALSE);
				strSql.Replace("@LocationFilter", "");
			}

			//DRT 8/6/03 - Allow them to do a 'use group' or 'use filter' filter as well
			if(bUseGroup) {
				CString str, strFilter = DefGetGroupFilter(nSubLevel, nSubRepNum);

				//we want to strip off the "(<something> IN " part of it
				long nIn = strFilter.Find("IN");

				strFilter = strFilter.Right(strFilter.GetLength() - nIn - 3);

				str.Format(" AND (PatientID IN %s", strFilter);
				strSql.Replace("@UseGroupFilter", str);
			}
			else if(bUseFilter) {
				//There is a problem with running filters here, the Billing total queries just crap out with an 
				//'Internal Server Error'.  Apparently it should be fixed in service pack 4 for SQL 2000.  The bug
				//has to do with using an aggregate function in a subquery.  I'm not entirely sure why it works w/ groups and
				//not filters.
				//http://support.microsoft.com/default.aspx?scid=http://support.microsoft.com:80/support/kb/articles/Q290/8/17.ASP&NoWebContent=1

				//Until that point, this code is unusable.  They shouldn't be able to get in here but just in case....
				MsgBox("Filters are not enabled in this report.  Please remove them from your filter criteria.");
				strSql.Replace("@UseGroupFilter", "");	//no filter
				/*
				CString str, strFilter = DefGetFilter(nSubLevel, nSubRepNum);

				//we want to strip off the "(<something> IN " part of it
				long nIn = strFilter.Find("IN");

				strFilter = strFilter.Right(strFilter.GetLength() - nIn - 3);

				str.Format(" AND (PatientID IN %s", strFilter);
				strSql.Replace("@UseGroupFilter", str);
				*/
			}
			else {
				strSql.Replace("@UseGroupFilter", "");	//no filter
			}
			//OK, we're finally done.
			return _T(strSql);
		}

	case 377:
		{
		//Appointments by Status
		/*	Version History
			DRT 3/12/03 - Show all appointments grouped by ShowState.  Due to some poor naming conventions in the scheduler itself, 
					the "ShowState" field in the table is shown as "Status" in the ResEntry.  So this report is named ".. by Status"
					to be less confusing for users
			DRT 6/19/03 - Removed references to AptPurposeID, it's obsolete
			TES 8/4/03 - Included EndTime
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			TES 8/23/05 - Changed resource filtering.
			(j.anspach 05-30-2006 16:09 PLID 19090) - Adding AppointmentsT.ArrivalTime
		*/
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500), and the Item field needs to be set to a memo field (length 500)
		CString sql = "SELECT AppointmentsT.StartTime "
		"    AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    AppointmentsT.Date AS Date, "
		"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item,  "
		"    AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone,  "
		"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose,  "
		"    AptTypeT.ID AS ID, -1 AS ResourceID, "
		"    Last + ', ' + First + ' ' + Middle AS PatName, AppointmentsT.ID AS ApptID, AppointmentsT.PatientID, "
		"    AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, PersonT.ID AS PatID, PersonT.PrivHome, PersonT.PrivWork, PersonT.Email, "
		"    AptShowStateT.Name AS ShowState, "
		"    AppointmentsT.ShowState AS StateID "
		"FROM (AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID) LEFT OUTER JOIN "
		"    AptTypeT ON "
		"    AppointmentsT.AptTypeID = AptTypeT.ID LEFT JOIN "
		"	 AptShowStateT ON AppointmentsT.ShowState = AptShowstateT.ID "
		"    LEFT JOIN PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    AppointmentsT.PatientID = PatientsT.PersonID "
		"WHERE (AppointmentsT.Status <> 4) AND (PersonT.ID > 0)";

		return _T(sql);
		break;
		}


	case 395:
		//Unapplied Superbill IDs
		/*	Version History
			DRT 5/29/03 - Added the report.  Shows all superbill IDs which are not applied to a charge.  Also gives you the option
					to filter out the ones that are already marked as Void.
			DRT 1/28/2004 - PLID 10094 - Added an external filter for resource.  Made a special case filtering thing due to the way we join appointment resources.  See
					external form for this report ID for details.
			TES 2/17/04 - Enabled Appointment subfilters.
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID 
			JMJ 7/1/05 - Fixed bug that caused superbills linked to deleted charges not to show up
		*/
		return _T("SELECT SavedID, PrintedOn AS PrintDate, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"AppointmentsT.Date AS ApptDate, AppointmentsT.StartTime, EndTime, AptShowStateT.Name AS ShowState, AppointmentsT.Status, Void AS VoidType, VoidDate, VoidUser, "
			"AppointmentsT.LocationID AS LocID, PersonT.ID AS PatID, "
			"AppointmentsT.ID AS ApptID "
			"FROM "
			"PrintedSuperbillsT INNER JOIN PersonT ON PatientID = PersonT.ID INNER JOIN AppointmentsT ON ReservationID = AppointmentsT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID LEFT JOIN "
			"AptShowStateT ON AppointmentsT.ShowState = AptShowstateT.ID "
			" "
			"WHERE SavedID NOT IN (SELECT SuperbillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE SuperbillID IS NOT NULL AND Deleted = 0) ");
		break;

	case 427:
		{
		//Appointments by Template
		/* Version History
			TES 2/17/04 - Enabled Appointment subfilters.
			(j.anspach 05-30-2006 16:10 PLID 19090) - Adding AppointmentsT.ArrivalTime
			(z.manning, 10/26/2006) - PLID 23856 - Updated query now that StartDate, EndDate,
				and resources are properties of line items. Also removed any references to PivotDate
				and made sure the query factors in template and template item exceptions.
			(z.manning, 03/08/2007) - PLID 23245 - This query's from clause is a mess, so I added DISTINCT
				to the select so we don't pull the exact same appt more than once.
		*/
		CString sql = 
		"SELECT DISTINCT AppointmentsT.ArrivalTime, AppointmentsT.StartTime,  \r\n"
		"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  \r\n"
		"	AppointmentsT.Date AS Date, AppointmentsT.LocationID AS LocID, AppointmentsT.Notes,  \r\n"
		"	CASE WHEN AptTypeT.Name Is Null THEN '<No Type>' ELSE AptTypeT.Name END AS Type,  \r\n"
		"	dbo.GetResourceString(AppointmentsT.ID) AS Item,  \r\n"
		"	CASE WHEN dbo.GetPurposeString(AppointmentsT.ID) Is Null THEN '<No Purpose>' ELSE dbo.GetPurposeString(AppointmentsT.ID) END AS Purpose,  \r\n"
		"	ResourceT.ID AS ResourceID, PersonT.ID AS PatID, PatientsT.UserDefinedID,  \r\n"
		"	LocationsT.Name AS Location, TemplateT.Name, TemplateT.Color, TemplateT.Priority,  \r\n"
		"	AppointmentsT.ID AS ApptID, TemplateT.ID AS TemplateID  \r\n"
		"FROM AppointmentsT  \r\n"
		"   LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID  \r\n"
		"   LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID  \r\n"
		"	LEFT JOIN (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID)  \r\n"
		"		ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID  \r\n"
		"		LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  \r\n"
		",TemplateT  \r\n"
		"	INNER JOIN TemplateItemT ON TemplateT.ID = TemplateItemT.TemplateID  \r\n"
		"	LEFT JOIN TemplateItemResourceT ON TemplateItemT.ID = TemplateItemResourceT.TemplateItemID  \r\n"
		"	LEFT JOIN TemplateDetailsT ON TemplateItemT.ID = TemplateDetailsT.TemplateItemID  \r\n"
		"	LEFT JOIN TemplateExceptionT ON TemplateT.ID = TemplateExceptionT.TemplateID  \r\n"
		"WHERE  \r\n"
		"	AppointmentsT.Status <> 4 AND  \r\n"
		"	(TemplateExceptionT.Flags IS NULL OR (TemplateExceptionT.Flags & 2) = 0) AND  \r\n"
		// (c.haag 2014-11-06) - PLID 63589 - We need to exclude appointments whose days rest on any exception day for the template line item
		"	AppointmentsT.Date NOT IN (SELECT TemplateItemExceptionT.Date FROM TemplateItemExceptionT WHERE TemplateItemExceptionT.TemplateItemID = TemplateItemT.ID) AND \r\n"
		"	(TemplateItemT.StartDate IS NULL OR TemplateItemT.StartDate <= AppointmentsT.Date) AND  \r\n"
		"	(TemplateItemT.EndDate IS NULL OR TemplateItemT.EndDate >= AppointmentsT.Date) AND  \r\n"
//An appt overlaps a template if the appointment starts before the template end time, and ends after the template start time (see how that works?).
		"	(CONVERT(datetime,CONVERT(nvarchar, TemplateItemT.EndTime, 108)) > convert(datetime,(convert(nvarchar, AppointmentsT.StartTime,8)))) AND  \r\n"
		"	(CONVERT(datetime,CONVERT(nvarchar, TemplateItemT.StartTime, 108)) < convert(datetime,(convert(nvarchar, AppointmentsT.EndTime,8)))) AND  \r\n"
		"	((TemplateItemT.AllResources = 1) OR (TemplateItemResourceT.ResourceID = AppointmentResourceT.ResourceID)) AND ((  \r\n"
		"		(TemplateItemT.Scale = 1) \r\n"
		"	) OR (  \r\n"
		"		(TemplateItemT.Scale = 2) AND  \r\n"
		"		((DateDiff(dd, TemplateItemT.StartDate, AppointmentsT.Date) % [Period]) = 0) AND  \r\n"
		"		((DatePart(dw, AppointmentsT.Date) - 1) = TemplateDetailsT.DayOfWeek)  \r\n"
		"	) OR (  \r\n"
		"		(TemplateItemT.Scale = 3) AND  \r\n"
		"		((DateDiff(wk, TemplateItemT.StartDate, AppointmentsT.Date) % Period) = 0) AND  \r\n"
		"		((DatePart(dw, AppointmentsT.Date) - 1) = TemplateDetailsT.DayOfWeek)  \r\n"
		"	) OR (  \r\n"
		"		(TemplateItemT.Scale = 4) AND  \r\n"
		"		(TemplateItemT.MonthBy = 2) AND  \r\n"
		"		((DateDiff(mm, TemplateItemT.StartDate, AppointmentsT.Date) % Period) = 0) AND  \r\n"
		"		(DatePart(dd, AppointmentsT.Date) = TemplateItemT.DayNumber)  \r\n"
		"	) OR (  \r\n"
		"		(TemplateItemT.Scale = 4) AND   \r\n"
		"		(TemplateItemT.MonthBy = 1) AND  \r\n"
		"		( ( ((DatePart(dd, AppointmentsT.Date) - 1) / 7 + 1) = PatternOrdinal) OR (PatternOrdinal = -1 AND DATEPART(mm, DATEADD(ww, 1, AppointmentsT.Date)) != DATEPART(mm, AppointmentsT.Date)) ) AND    \r\n"
		"		((DateDiff(mm, TemplateItemT.StartDate, AppointmentsT.Date) % Period) = 0) AND  \r\n"
		"		((DatePart(dw, AppointmentsT.Date) - 1) = TemplateDetailsT.DayOfWeek)  \r\n"
		"	) OR (  \r\n"
		"		(TemplateItemT.Scale = 5) AND  \r\n"
		"		(DatePart(mm, TemplateItemT.StartDate) = DatePart(mm, AppointmentsT.Date)) AND  \r\n"
		"		(DatePart(dd, TemplateItemT.StartDate) = DatePart(dd, AppointmentsT.Date))  \r\n"
		"	))";
		return _T(sql);
		}
		break;

	case 432: //Appts cancelled, not rescheduled.
		/* Version History
			TES 8/6/03 - Created, copied from Cnslt canclled, not rescheduled.
			// (s.dhole 2012-02-16 15:56) - PLID 44444 Added AppointmentsT.AptTypeID AS AptTypeID 
		*/
		return _T("SELECT "
			"AppointmentsT.ID AS ApptID, Date AS Date, StartTime, EndTime, AppointmentsT.Notes, AptTypeT.Name AS TypeName, dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END AS RefName, "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Patient' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS PatName, CASE WHEN PersonT1.ID IS NULL THEN 'No Coordinator' ELSE PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle END AS CoordName, "
			"CASE WHEN LocationsT.ID IS NULL THEN 'No Location' ELSE LocationsT.Name END AS CreatedLoc, LocationsT.ID AS LocID, PersonT.ID AS PatID, ReferralSourceT.PersonID AS ReferralID, "
			"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email, AppointmentsT.CreatedLogin, CASE WHEN LocationsT2.ID IS NULL THEN 'No Location' ELSE LocationsT2.Name END AS LocName, "
			"CASE WHEN AptCancelReasonT.Description IS NOT NULL THEN AptCancelReasonT.Description ELSE CASE WHEN Len(CancelledReason) = 0 THEN '<None Specified>' ELSE CancelledReason END END AS CancelReason, "
			" AppointmentsT.AptTypeID AS AptTypeID "
			"FROM "
			"AppointmentsT INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN PersonT PersonT1 ON PatientsT.EmployeeID = PersonT1.ID "
			"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
			"LEFT JOIN LocationsT LocationsT2 ON PersonT.Location = LocationsT2.ID "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN AptCancelReasonT ON AppointmentsT.CancelReasonID = AptCancelReasonT.ID "
			"INNER JOIN "
			"(SELECT AppointmentsT.ID "
			"FROM AppointmentsT "
			"WHERE AppointmentsT.Status = 4 AND AppointmentsT.PatientID > 0 "
			") AS ConsultsSubQ ON AppointmentsT.ID = ConsultsSubQ.ID "
			"WHERE AppointmentsT.PatientID NOT IN "
			"/*Patients that have a consult past the date above, but not canceled*/ "
			"(SELECT ApptsQ.PatientID "
			"FROM AppointmentsT ApptsQ "
			"WHERE (ApptsQ.Date >= AppointmentsT.Date) AND (ApptsQ.Status <> 4)  "
			")");
		break;

	case 440: //Appointments by Patient
		/*Version History
			TES 8/8/03 - Created, plagiarized from the Appointments tab PP.
			TES 2/17/04 - Enabled Appointment subfilters.
			TES 3/4/04 - Enabled mult-resource support (!)
			JMM 10/26/05 - Changed the second status to status 2
			(j.anspach 05-30-2006 16:10 PLID 19090) - Adding AppointmentsT.ArrivalTime
			// (j.jones 2010-02-04 15:43) - PLID 36500 - ensured the Date field was exposed "AS Date"
		*/
		return _T("SELECT AppointmentsT.StartTime, AppointmentsT.EndTime, "
			"    PersonT.First, PersonT.Middle, PersonT.Last,  "
			"    PersonT.Last + ', ' + PersonT.Middle + ' ' + PersonT.First AS PatName, "
			"    AppointmentsT.Date AS Date, "
			"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item,  "
			"    AppointmentsT.AptTypeID, AppointmentsT.Status, PersonT.HomePhone,  "
			"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose,  "
			"    AptTypeT.ID, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.PrivHome, "
			"	 PersonT.PrivWork, AppointmentsT.Status AS Status2, AppointmentsT.ShowState, "
			"    AppointmentsT.ID AS ApptID "
			"FROM PersonT INNER JOIN "
			"    PatientsT ON  "
			"    PersonT.ID = PatientsT.PersonID RIGHT OUTER JOIN "
			"    AppointmentsT ON  "
			"    PatientsT.PersonID = AppointmentsT.PatientID LEFT OUTER JOIN "
			"    AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE (PatientsT.PersonID > 0)");
		break;

	case 442:
		{
		//Appointments By Referring Physician
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500), and the Item field needs to be set to a memo field (length 500)
		/*	Version History
			DRT 9/2/03 - Created.  Based off Appts by Pat Coord
			TES 3/4/04 - Removed references to AppointmentsT.ResourceID
			TES 8/23/05 - Changed resource filtering.
			(j.anspach 05-30-2006 16:08 PLID 19090) - Adding AppointmentsT.ArrivalTime
			// (j.gruber 2012-12-10 10:10) - PLID 54122 - added purpose short and appt location name
		*/
		CString sql = "SELECT AppointmentsT.StartTime "
		"    AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    AppointmentsT.Date AS Date, AppointmentsT.LocationID as LocID, "
		"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item,  "
		"    AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone,  "
		"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, "
		"    AptTypeT.ID AS ID, "
		"-1 AS ResourceID, "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, AppointmentsT.ID AS ApptID, AppointmentsT.PatientID, "
		"	 PersonT.ID AS PatID, PersonT.PrivHome, PersonT.PrivWork, PatientsT.DefaultReferringPhyID AS RefPhysID,  "
		"	 CASE WHEN PersonT1.ID IS NOT NULL THEN PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle ELSE 'No Referring Physician' END AS RefPhysName, "
		"    dbo.GetPurposeString(AppointmentsT.ID) AS PurposeShort, LocationsT.Name as ApptLocName "
 		"FROM AptTypeT RIGHT OUTER JOIN "
		"    AppointmentsT ON  "
		"    AppointmentsT.AptTypeID = AptTypeT.ID "
		"    LEFT JOIN PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    AppointmentsT.PatientID = PatientsT.PersonID "
		"	 LEFT JOIN PersonT PersonT1 ON PatientsT.DefaultReferringPhyID = PersonT1.ID "
		"    LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
		"WHERE (AppointmentsT.Status <> 4) AND (PersonT.ID > 0) AND (AppointmentsT.ShowState <> 3)";

		return _T(sql);
		break;
		}

	case 500: //Scheduling Productivity by Resource
		/*	Version History
			TES 4/19/04 - Created, copied off of Scheduling Productivity by User
			(a.walling 2006-10-19 11:10) - PLID 23157 - Fixed duplication of charges which led to wildly inaccurate sums.
		*/
		{
			CString strSql = "SELECT AppointmentsBookedQ.Item AS UserName, CASE WHEN AppointmentsRequested Is Null THEN 0 ELSE AppointmentsRequested END AS AppointmentsRequested, AppointmentsBooked, AppointmentsBilled, TotalBilled, TotalReceived FROM  "
				" "
				"(SELECT ResourceT.Item, Count(AppointmentsT.ID) AS AppointmentsBooked FROM AppointmentsT LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID WHERE AppointmentsT.Status <> 4 @DateFilter @LocationFilter @UseGroupFilter "
				"GROUP BY ResourceT.Item) AppointmentsBookedQ LEFT JOIN "
				" "
				"(SELECT ResourceT.Item, Count(AppointmentsT.ID) AS AppointmentsRequested FROM AppointmentsT INNER JOIN ResourceT ON AppointmentsT.RequestedResourceID = ResourceT.ID WHERE AppointmentsT.Status <> 4 @DateFilter @LocationFilter @UseGroupFilter "
				"GROUP BY ResourceT.Item) AppointmentsRequestedQ ON AppointmentsRequestedQ.Item = AppointmentsBookedQ.Item LEFT JOIN  "
				" "
				"(SELECT ResourceT.Item, Count(AppointmentsT.ID) AS AppointmentsBilled FROM AppointmentsT LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID WHERE AppointmentsT.Status <> 4 AND PatientID IN (SELECT PatientID FROM LineItemT WHERE Type = 10 AND Deleted = 0 AND LineItemT.Date >= AppointmentsT.Date) @DateFilter @LocationFilter @UseGroupFilter "
				"GROUP BY ResourceT.Item) AppointmentsBilledQ ON AppointmentsBookedQ.Item = AppointmentsBilledQ.Item LEFT JOIN  "
				" "
				"(SELECT Item, Sum(Amount) AS TotalBilled FROM  "
				"(SELECT LineQ.PatientID, ChargeID, BillID, Amount, Item FROM"
				"(BillsT LEFT JOIN UsersT ON BillsT.PatCoord = UsersT.PersonID) INNER JOIN (SELECT ChargesT.BillID, LineItemT.PatientID, LineItemT.Date, dbo.GetChargeTotal(ChargesT.ID) AS Amount, ChargesT.ID AS ChargeID FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0) AS LineQ ON BillsT.ID = LineQ.BillID "
				"INNER JOIN  (SELECT PatientID, Date, ResourceT.Item FROM AppointmentsT LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID WHERE AppointmentsT.Status <> 4 @DateFilter @LocationFilter @UseGroupFilter) ConsultsQ ON LineQ.PatientID = ConsultsQ.PatientID "
				"WHERE ConsultsQ.Date <= LineQ.Date "
				"GROUP BY ChargeID, Amount, LineQ.PatientID, Item, BillID) InnerQ "
				"GROUP BY Item) TotalBilledQ ON AppointmentsBilledQ.Item = TotalBilledQ.Item LEFT JOIN  "
				" "
				"(SELECT Item, Sum(Amount) AS TotalReceived FROM  "
				"(SELECT LineQ.PatientID, ChargeID, BillID, AppliesT.Amount, Item FROM "
				"BillsT INNER JOIN (SELECT LineItemT.ID, ChargesT.BillID, LineItemT.PatientID, LineItemT.Date, dbo.GetChargeTotal(ChargesT.ID) AS Amount, ChargesT.ID AS ChargeID FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0) AS LineQ ON BillsT.ID = LineQ.BillID INNER JOIN AppliesT ON LineQ.ID = AppliesT.DestID INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID  "
				"INNER JOIN  (SELECT PatientID, AppointmentsT.Date, ResourceT.Item FROM AppointmentsT LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID WHERE AppointmentsT.Status <> 4 @DateFilter @LocationFilter @UseGroupFilter) ConsultsQ ON LineQ.PatientID = ConsultsQ.PatientID  "
				"WHERE ConsultsQ.Date <= LineQ.Date AND LineItemT.Deleted = 0 "
				"GROUP BY ChargeID, AppliesT.Amount, LineQ.PatientID, Item, BillID) InnerQ "
				"GROUP BY Item) TotalReceivedQ ON AppointmentsBookedQ.Item = TotalReceivedQ.Item ";

			//Fix the date filtering.
			//DRT 8/6/03 -  This was filtering > and <, so if you tried filtering on 1 single day, it could never have 
			//		worked, and would have always excluded the first day in your range!
			if(nDateRange == 1) {
				if(nDateFilter == 1) {
					CString strDateFilter;
					COleDateTimeSpan dtSpan;
					COleDateTime dt;
					dtSpan.SetDateTimeSpan(1,0,0,0);
					dt = DateTo;
					dt += dtSpan;
					strDateFilter.Format(" AND Date >= '%s' AND Date < '%s' ", DateFrom.Format("%Y-%m-%d"), dt.Format("%Y-%m-%d"));
					strSql.Replace("@DateFilter", strDateFilter);
				}
				else if(nDateFilter == 2) {
					CString strDateFilter;
					COleDateTimeSpan dtSpan;
					COleDateTime dt;
					dtSpan.SetDateTimeSpan(1,0,0,0);
					dt = DateTo;
					dt += dtSpan;
					strDateFilter.Format(" AND CreatedDate >= '%s' AND CreatedDate < '%s' ", DateFrom.Format("%Y-%m-%d"), dt.Format("%Y-%m-%d"));
					strSql.Replace("@DateFilter", strDateFilter);
				}
			}
			else {
				//There's no date filter
				strSql.Replace("@DateFilter", "");
			}

			//Now the location filtering.
			//(e.lally 2008-10-01) PLID 31543 - Added support for filtering on multiple locations.
			if(nLocation > 0) {
				CString strLocationFilter;
				strLocationFilter.Format(" AND AppointmentsT.LocationID = %li", nLocation);
				strSql.Replace("@LocationFilter", strLocationFilter);
			}
			else if(nLocation == -1){
				strSql.Replace("@LocationFilter", "");
			}
			else if(nLocation == -3){
				CString strLocationFilter = " AND AppointmentsT.LocationID IN(";
				CString strPart;
				for(int i=0; i < m_dwLocations.GetSize(); i++) {
					strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
					strLocationFilter += strPart;
				}
				strLocationFilter = strLocationFilter.Left(strLocationFilter.GetLength()-2) + ")";
				strSql.Replace("@LocationFilter", strLocationFilter);
			}
			else if(nLocation == -2){
				//This will always be false
				CString strLocationFilter = " AND AppointmentsT.LocationID IS NULL ";
				strSql.Replace("@LocationFilter", strLocationFilter);

			}
			else {
				ASSERT(FALSE);
				strSql.Replace("@LocationFilter", "");
			}

			//DRT 8/6/03 - Allow them to do a 'use group' or 'use filter' filter as well
			if(bUseGroup) {
				CString str, strFilter = DefGetGroupFilter(nSubLevel, nSubRepNum);

				//we want to strip off the "(<something> IN " part of it
				long nIn = strFilter.Find("IN");

				strFilter = strFilter.Right(strFilter.GetLength() - nIn - 3);

				str.Format(" AND (PatientID IN %s", strFilter);
				strSql.Replace("@UseGroupFilter", str);
			}
			else if(bUseFilter) {
				//There is a problem with running filters here, the Billing total queries just crap out with an 
				//'Internal Server Error'.  Apparently it should be fixed in service pack 4 for SQL 2000.  The bug
				//has to do with using an aggregate function in a subquery.  I'm not entirely sure why it works w/ groups and
				//not filters.
				//http://support.microsoft.com/default.aspx?scid=http://support.microsoft.com:80/support/kb/articles/Q290/8/17.ASP&NoWebContent=1

				//Until that point, this code is unusable.  They shouldn't be able to get in here but just in case....
				MsgBox("Filters are not enabled in this report.  Please remove them from your filter criteria.");
				strSql.Replace("@UseGroupFilter", "");	//no filter
				
				/*CString str, strFilter = DefGetFilter(nSubLevel, nSubRepNum);

				//we want to strip off the "(<something> IN " part of it
				long nIn = strFilter.Find("IN");

				strFilter = strFilter.Right(strFilter.GetLength() - nIn - 3);

				str.Format(" AND (PatientID IN %s", strFilter);
				strSql.Replace("@UseGroupFilter", str);*/
				
			}
			else {
				strSql.Replace("@UseGroupFilter", "");	//no filter
			}
			//OK, we're finally done.
			return _T(strSql);
		}



	/* Version History
	   10-26-04 JMM - Created 
	   // (m.hancock 2006-08-15 14:42) - PLID 19534 - Added InTime and OutTime to query results
	   // (c.haag 2009-03-17 10:28) - PLID 29371 - Changed Date and ApptDate from [Date] to [StartTime] because we want the times, too
	   // (j.gruber 2011-06-17 14:37) - PLID 34018 - took out cancelled and no showed appts
	*/
	case 549: //Appt Duration By Type
	
		return _T("SELECT AppointmentsT.PatientID, AppointmentsT.StartTime AS [Date], DurationT.Duration, "
			" PersonT.First, PersonT.Middle, PersonT.Last, AptTypeT.Name, AppointmentsT.AptTypeID AS TypeID, PersonT.ID AS PatID, "
			" AppointmentsT.StartTime AS ApptDate, AppointmentsT.LocationID AS LocID, DurationT.StartTime AS InTime, DurationT.EndTime AS OutTime "
			" FROM AppointmentsT  "
			" INNER JOIN  "
			" (SELECT InAppts.AppointmentID, DATEDIFF(n, StartTime, EndTime) AS Duration, StartTime, EndTime FROM  "
			" (SELECT AppointmentID, TimeStamp as StartTime FROM AptShowStateHistoryT WHERE ShowStateID = 1) InAppts "
			" INNER JOIN  "
			" (SELECT AppointmentID, TimeStamp as EndTime FROM AptShowStateHistoryT WHERE ShowStateID = 2) OutAppts " 
			" ON InAppts.AppointmentID = OutAppts.AppointmentID "
			" GROuP BY InAppts.AppointmentID, startTime, endtime) DurationT "
			" ON AppointmentsT.ID = DurationT.AppointmentID "
			" LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			" LEFT JOIN PatientsT ON PersonT.Id = PatientsT.PersonID " 
			" LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.Id"
			" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 ");
		break;

	case 551: //Consult Resource Productivity
	{
		//TES 8/23/2005 - Took out the resource filtering stuff; it's unnecessary because the report groups on resource.
		CString strSQL;

		strSQL = "SELECT ConsultAptT.ID AS ConsultAptID, SxAptT.ID AS SxAptID, ConsultAptT.PatientID AS PatID, "
		"AppointmentResourceT.ResourceID AS ResourceID, ResourceT.Item AS ConsultAptResource,"
		"PatientsT.MainPhysician AS ProvID, "
				   "(SELECT TOP 1 BillsT.Location AS Location  "
				   "FROM BillsT   "
				   "LEFT JOIN ChargesT ON ChargesT.BillID = BillsT.ID  "
				   "LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
				   "LEFT JOIN LineItemT ON LineItemT.ID = ChargesT.ID "
				   "WHERE BillsT.PatientID = PersonT.ID AND LineItemT.Date = SxAptT.Date AND  "
					   "ServiceT.ProcedureID IN (SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = SxAptT.ID) "
					   "AND BillsT.Deleted = 0) AS LocID, "
				   "dbo.GetResourceString(ConsultAptT.ID) AS ConsultResourceList, "
				   "PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
				   "(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle FROM PersonT WHERE ID = PatientsT.MainPhysician) AS Doctor,  "
				   "ConsultAptT.Date AS ConsultAptDate, SxAptT.Date AS SxAptDate, "
				   "(SELECT Name FROM AptTypeT WHERE ID = ConsultAptT.AptTypeID) AS ConsultAptType,"
				   "(SELECT Name FROM AptTypeT WHERE ID = SxAptT.AptTypeID) AS SxAptType, dbo.GetPurposeString(SxAptT.ID) AS SxAptPurpose,"
				   "(SELECT TOP 1 dbo.GetBillTotal(BillsT.ID) AS Location  "
				   "FROM BillsT   "
				   "LEFT JOIN ChargesT ON ChargesT.BillID = BillsT.ID  "
				   "LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
				   "LEFT JOIN LineItemT ON LineItemT.ID = ChargesT.ID "
				   "WHERE BillsT.PatientID = PersonT.ID AND LineItemT.Date = SxAptT.Date AND  "
					   "ServiceT.ProcedureID IN (SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = SxAptT.ID) "
					   "AND BillsT.Deleted = 0) AS BillTotal,"
				   "(SELECT TOP 1 (SELECT Name FROM LocationsT WHERE ID = BillsT.Location) AS Location  "
				   "FROM BillsT   "
				   "LEFT JOIN ChargesT ON ChargesT.BillID = BillsT.ID  "
				   "LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
				   "LEFT JOIN LineItemT ON LineItemT.ID = ChargesT.ID "
				   "WHERE BillsT.PatientID = PersonT.ID AND LineItemT.Date = SxAptT.Date AND  "
					   "ServiceT.ProcedureID IN (SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = SxAptT.ID) "
					   "AND BillsT.Deleted = 0) AS BillLocation, "
					   "(SELECT Name FROM ReferralSourceT WHERE PatientsT.ReferralID = ReferralSourceT.PersonID) AS Referral "
			"FROM PersonT "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN AppointmentsT AS ConsultAptT ON ConsultAptT.PatientID = PatientsT.PersonID AND "
				  "ConsultAptT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1)   "
				  "AND ConsultAptT.Status <> 4 "
				  "LEFT JOIN AppointmentResourceT ON ConsultAptT.ID = AppointmentResourceT.AppointmentID  "
					"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID  "
					"LEFT JOIN AppointmentsT AS SxAptT ON SxAptT.PatientID = ConsultAptT.PatientID AND "
				  "SxAptT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4)   "
				  "AND SxAptT.Status <> 4 "
			"WHERE PersonT.ID > 0 AND SxAptT.Date IS NOT NULL AND SxAptT.Date >= ConsultAptT.Date "
					  "/*This is to make sure that the first consult and the first surgery are linked together*/ "
				  "AND (SELECT TOP 1 ID FROM AppointmentsT AS ConsultCheckT WHERE PersonT.ID = ConsultCheckT.PatientID AND "
					  "ConsultCheckT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND ConsultCheckT.Status <> 4 ORDER BY Date, ID) = ConsultAptT.ID "
					  "AND (SELECT TOP 1 ID FROM AppointmentsT AS SxCheckT WHERE PersonT.ID = SxCheckT.PatientID AND "
					  "SxCheckT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4) AND SxCheckT.Status <> 4 "
					  "AND SxCheckT.Date > ConsultAptT.Date ORDER BY Date, ID) = SxAptT.ID";

		return _T(strSQL);
	break;		
	}

	case 583:	// Time scheduled productivity by logged time
		/*Version History
		// (j.gruber 2007-05-04 11:08) - PLID 25872 - changed the extended filter to explude non patient appts and made resource the external filter
		// (d.moore 2007-07-25) - PLID 17519 - Set up to use Appointment Type as the external filter instead of Resource.
		//  I also altered the query to ignore inactive resources.
		// (j.gruber 2009-07-09 15:39) - PLID 20398 - made it count the number of appts
		*/
	{
		CString strSql;
		strSql.Format("SELECT Date AS Date, AppointmentResourceT.ResourceID AS ResourceID, ResourceUserLinkT.UserID, UsersT.UserName, LocationsT.ID AS LocID, ResourceT.Item AS Resource, SUM(DateDiff(mi, StartTime, EndTime)) AS MinutesScheduled, "
			"AptTypeT.ID AS AppointmentTypeID, AptTypeT.NAME AS AppointmentTypeName, "
			"(SELECT SUM(DateDiff(mi, UT2.CheckIn, UT2.CheckOut)) AS Diff "
			"FROM UserTimesT UT2 "
			"INNER JOIN UsersT ON UT2.UserID = UsersT.PersonID AND UsersT.PersonID > 0 "
			"WHERE (DateDiff(dd, UT2.CheckIn, date) = 0 AND UT2.CheckOut IS NOT NULL AND UT2.UserID = ResourceUserLinkT.UserID)) AS MinutesLogged, "
			" Count(AppointmentsT.ID) as CountOfAppts "
			"FROM AppointmentsT  "
			"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID  "
			"LEFT JOIN ResourceT ON ResourceT.ID = AppointmentResourceT.ResourceID  "
			"LEFT JOIN ResourceLocationConnectT ON ResourceLocationConnectT.ResourceID = AppointmentResourceT.ResourceID "
			"LEFT JOIN LocationsT ON LocationsT.ID = ResourceLocationConnectT.LocationID "
			"LEFT JOIN ResourceUserLinkT ON ResourceUserLinkT.ResourceID = ResourceT.ID "
			"LEFT JOIN UsersT ON ResourceUserLinkT.UserID = UsersT.PersonID "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE (ResourceUserLinkT.UserID IS NOT NULL AND AppointmentsT.Status <> 4 AND ResourceT.Inactive = 0) "
			"GROUP BY Date, AppointmentResourceT.ResourceID, ResourceUserLinkT.UserID, ResourceT.Item, LocationsT.ID, UsersT.UserName, AptTypeT.ID, AptTypeT.NAME "
			"ORDER BY Resource, Date");
		
		return  _T(strSql);
		break;
	}	

	case 630:	// Time scheduled productivity by logged time by Location
		/*Version History
		// (j.gruber 2008-06-26 12:26) - PLID 26137 - created from the Time scheduled productivity by logged time  report
		// (j.gruber 2009-07-09 15:41) - PLID 20398 - added # of appts
		*/
	{
		CString strSql;
		strSql.Format("SELECT AppointmentsT.ID, Date AS Date, AppointmentResourceT.ResourceID AS ResourceID, ResourceUserLinkT.UserID, UsersT.UserName, LocationsT.ID AS LocID, ResourceT.Item AS Resource, DateDiff(mi, StartTime, EndTime) AS MinutesScheduled, "
			"AptTypeT.ID AS AppointmentTypeID, AptTypeT.NAME AS AppointmentTypeName, "
			"(SELECT SUM(DateDiff(mi, UT2.CheckIn, UT2.CheckOut)) AS Diff "
			"FROM UserTimesT UT2 "
			"INNER JOIN UsersT ON UT2.UserID = UsersT.PersonID AND UsersT.PersonID > 0 "
			"WHERE (DateDiff(dd, UT2.CheckIn, date) = 0 AND UT2.CheckOut IS NOT NULL AND UT2.UserID = ResourceUserLinkT.UserID AND UT2.LocationID = LocationsT.ID)) AS MinutesLogged, "
			" LocationsT.Name as LocName, StartTime, EndTime, "
			" Count(AppointmentsT.ID) as CountOfAppts "
			"FROM UserTimesT "
			"LEFT JOIN LocationsT ON UserTimesT.LocationID = LocationsT.ID "
			" LEFT JOIN UsersT ON UserTimesT.UserID = UsersT.PersonID "
			" LEFT JOIN ResourceUserLinkT ON UsersT.PersonID = ResourceUserLinkT.UserID "
			" LEFT JOIN ResourceT ON ResourceUserLinkT.ResourceID = ResourceT.ID "
			" LEFT JOIN AppointmentResourceT ON ResourceT.ID = AppointmentResourceT.ResourceID "
			" LEFT JOIN AppointmentsT ON AppointmentResourceT.AppointmentID = AppointmentsT.ID "
			" LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE (ResourceUserLinkT.UserID IS NOT NULL AND AppointmentsT.Status <> 4 AND ResourceT.Inactive = 0) AND AppointmentsT.LocationID = UserTimesT.LocationID "
			" AND DateDiff(dd, UserTimesT.CheckIn, date) = 0 "
			"GROUP BY Date, AppointmentResourceT.ResourceID, ResourceUserLinkT.UserID, ResourceT.Item, LocationsT.ID, UsersT.UserName, AptTypeT.ID, AptTypeT.NAME, LocationsT.Name, AppointmentsT.ID, StartTime, EndTime "
			"ORDER BY Resource, Date");
		
		return  _T(strSql);
		break;
	}	

	case 601: // Waiting List
		/*Version History
			// (d.moore 2007-07-17 11:52) - PLID 26537 - Created a new report for tracking entries
			//  in the waiting list.
			// (j.gruber 2008-07-10 10:18) - PLID 30669 - take out cancelled appts
			//DRT 8/29/2008 - PLID 29240 - Remove 'In' and 'Out' appointments
		*/
	{
		CString strSql;
		strSql.Format("SELECT PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"PersonT.HomePhone, PersonT.WorkPhone,  AptTypeT.NAME AS ApptType, ResourceT.Item AS Resource, "
			"dbo.GetWaitListPurposeString(WaitingListT.ID) AS PurposeList, WaitingListT.CreatedDate AS CreatedDate, "
			"WaitingListItemT.StartDate, WaitingListItemT.EndDate,  WaitingListItemT.StartTime, "
			"WaitingListItemT.EndTime, "
			"CASE WHEN (WaitingListT.AppointmentID > 0) THEN 'Yes' ELSE 'No' END AS HasAppointment, "
			"PersonT.Location AS LocID, LocationsT.Name,  Persont.ID AS PatID,  WaitingListT.TypeID AS ApptTypeID, "
			"WaitingListItemResourceT.ResourceID AS ResourceID "
			"FROM PersonT "
				"INNER JOIN WaitingListT  ON PersonT.ID = WaitingListT.PatientID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN AptTypeT ON WaitingListT.TypeID = AptTypeT.ID "
				"LEFT JOIN WaitingListItemT ON WaitingListT.ID = WaitingListItemT.WaitingListID "
				"LEFT JOIN WaitingListItemResourceT ON WaitingListItemT.ID = WaitingListItemResourceT.ItemID "
				"LEFT JOIN ResourceT ON WaitingListItemResourceT.ResourceID = ResourceT.ID "
				"LEFT JOIN LocationsT ON Persont.Location = LocationsT.ID "
				"LEFT JOIN AppointmentsT ON WaitingListT.AppointmentID = AppointmentsT.ID "
				"WHERE (AppointmentsT.ID IS NULL OR (AppointmentsT.Status <> 4 AND AppointmentsT.ShowState NOT IN (1, 2)))");
		
		return  _T(strSql);
		break;
	}
	
	case 628:
		{
		//Appointments Without Allocations
		//Note: When generating .ttx file, the Notes field needs to be manually set to a memo field (length 4000), 
		//and the Purpose field needs to be manually set to a memo field (length 500), and the Resource field needs to be set to a memo field (length 500)
		/*	Version History
			TES 6/18/2008 - PLID 30395 - Created.  Based off Appointments by Referring Physician, and 
				CApptsWithoutAllocationsDlg
			(j.jones 2009-12-21 17:02) - PLID 35169 - we also want to show appointments that have allocations 
			with any products marked "To Be Ordered", and those with orders that have any products still
			unreceived
		*/
		CString sql;
		sql.Format("SELECT AppointmentsT.StartTime "
			"    AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last,  "
			"    AppointmentsT.Date AS Date, AppointmentsT.LocationID as LocID, "
			"    AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name AS Type, dbo.GetResourceString(AppointmentsT.ID) AS Resource,  "
			"    AppointmentsT.AptTypeID AS SetID, PersonT.HomePhone,  "
			"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, "
			"    AptTypeT.ID AS ID, "
			"-1 AS ResourceID, "
			"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, AppointmentsT.ID AS ApptID, AppointmentsT.PatientID, "
			"	 PersonT.ID AS PatID, PersonT.PrivHome, PersonT.PrivWork "
 			"FROM AptTypeT RIGHT OUTER JOIN "
			"    AppointmentsT ON  "
			"    AppointmentsT.AptTypeID = AptTypeT.ID "
			"	INNER JOIN AppointmentPurposeT ON AppointmentsT.ID = "
			"	AppointmentPurposeT.AppointmentID INNER JOIN ApptsRequiringAllocationsDetailT ON AptTypeT.ID = "
			"	ApptsRequiringAllocationsDetailT.AptTypeID AND AppointmentPurposeT.PurposeID = "
			"	ApptsRequiringAllocationsDetailT.AptPurposeID "
			"    LEFT JOIN PersonT INNER JOIN "
			"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
			"    AppointmentsT.PatientID = PatientsT.PersonID "
			"WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND "
			"AppointmentsT.PatientID > 0 AND "
			// (a.walling 2013-02-08 13:04) - PLID 55084 - Avoid large appointment index scans in the scheduler
			// dbo.AsDateNoTime(AppointmentsT.StartTime) ends up requring a full scan of the index			
			"AppointmentsT.StartTime >= dbo.AsDateNoTime(getdate()) AND "
			"AppointmentsT.StartTime < dbo.AsDateNoTime(DATEADD(day,%li,getdate())) "
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
			")"
			"GROUP BY AppointmentsT.StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last, "
			"AppointmentsT.Date, AppointmentsT.LocationID, AppointmentsT.ArrivalTime, AppointmentsT.Notes, AptTypeT.Name, "
			"AppointmentsT.ID, AppointmentsT.AptTypeID, PersonT.HomePhone, AptTypeT.ID, AppointmentsT.PatientID, "
			"PersonT.ID, PersonT.PrivHome, PersonT.PrivWork", 
			GetRemotePropertyInt("ApptsRequiringAllocationsDays", 14, 0, "<None>", true),
			InvUtils::iasDeleted, InvUtils::iasDeleted, InvUtils::iadsOrder);

		return _T(sql);
		break;
		}

	case 670:
		// Time Scheduled Productivity By Scheduler Templates
		//
		// Note: Because of the way I use a dialog to get the data to run this report, if you ever
		// need to call CreateAllTtxFiles again for this report, you're probably better off doing
		// it somewhere after you've logged into Practice.
		//
		// Version History
		// (z.manning 2009-07-10 09:21) - PLID 22054 - Created
		// (j.jones 2010-12-09 11:57) - PLID 35058 - changed to not display resources that had no appointments scheduled
		// (j.jones 2010-12-09 12:26) - PLID 41782 - changed to not display appointments that do not fall on one of our templates
		// (f.gelderloos 2013-07-11 10:14) - PLID 57429 - re-wrote query
		{
			COleDateTime dtStart, dtEnd;
			if(nDateRange == -1) {
				// (z.manning 2009-07-15 19:10) - PLID 22054 - All Dates-- Let's go ahead and manually pull the
				// min and max appt dates as we must have a date range to cycle through. It's possible they may
				// have templates before and/or after these dates, but I see no reason to go outside the range
				// of appt dates.
				_RecordsetPtr prs = CreateRecordset(
					"SELECT Min(Date) AS MinDate, Max(Date) AS MaxDate FROM AppointmentsT WHERE Status <> 4");
				if(prs->eof) {
					// No appts
					dtStart = dtEnd = COleDateTime::GetCurrentTime();
				}
				else {
					dtStart = AdoFldDateTime(prs->GetFields(), "MinDate");
					dtEnd = AdoFldDateTime(prs->GetFields(), "MaxDate");
				}
			}
			else {
				dtStart = DateFrom;
				dtEnd = DateTo;
			}

			// (z.manning 2009-07-20 08:38) - PLID 22054 - Let's go ahead and handle the external template
			// ID filter manually. First, we need to load the IDs.
			CString strExtFilterPlaceholder = FormatString("{%s.%s} IN ", strRecordSource, strFilterField);
			CString strLocalExternalFilter = strExternalFilter;
			CArray<long,long> arynTemplateIDFilter;
			if(strLocalExternalFilter.Find(strExtFilterPlaceholder) != -1) {
				strLocalExternalFilter.Replace(strExtFilterPlaceholder, "");
				strLocalExternalFilter.TrimLeft("( ");
				strLocalExternalFilter.TrimRight(") ");
				// (z.manning 2010-04-08 11:30) - PLID 38112 - This loads from the temp table so we must use the snapshot connection
				_RecordsetPtr prsTemplateIDs = CreateRecordsetStd(GetRemoteDataSnapshot(), strLocalExternalFilter);
				for(; !prsTemplateIDs->eof; prsTemplateIDs->MoveNext()) {
					const long nTemplateID = AdoFldLong(prsTemplateIDs->GetFields(), "ID");
					arynTemplateIDFilter.Add(nTemplateID);
				}
			}
			else {
				// (z.manning 2009-07-18 12:43) - We should have found that unless we have no external filter
				ASSERT(strExternalFilter.IsEmpty());
			}

			// (z.manning 2009-07-16 07:13) - PLID 22054 - Why are we creating a dialog here? Well, the
			// (extremely complex) logic to load all of the top priority templates (which is exactly what
			// we need to get the total time available) is already done in this dialog. Let's just create
			// and invisible version of the template editor and call the functions we need to get that data.
			// (z.manning 2014-12-01 16:33) - PLID 64205 - Pass in the dialog type
			CTemplateItemEntryGraphicalDlg *pdlgTemplate = new CTemplateItemEntryGraphicalDlg(stetNormal, NULL);
			pdlgTemplate->m_bVisible = FALSE;
			pdlgTemplate->m_bLoadAllResources = TRUE;
			if(arynTemplateIDFilter.GetSize() > 0) {
				// (z.manning 2009-07-20 08:39) - PLID 22054 - If we have an external to filter to apply
				// then do so.
				// (s.tullis 2015-08-24 14:37) - PLID 66411 - Renamed
				pdlgTemplate->SetTemplateIDFilter(arynTemplateIDFilter);
			}
			pdlgTemplate->Create(IDD_TEMPLATE_ITEM_ENTRY_GRAPHICAL);

			const CString strTempTable = FormatString("#MinutesT_%u", GetTickCount());
			try {
				// (z.manning 2009-07-16 07:15) - PLID 22054 - Failsafe to ensure the temp table does not already exist.
				// (a.walling 2009-09-08 13:55) - PLID 35178 - Use the snapshot connection
				ExecuteSql(GetRemoteDataSnapshot(),"DROP TABLE %s", strTempTable);
			}NxCatchAllIgnore();
			// (z.manning 2009-07-16 07:16) - PLID 22054 - I originally tried this using a table variable, however,
			// when running this report over a huge date range it was erroring out due to memory issues. So let's
			// just use a temp table so we can commit our potentially massive amount of inserts every so often.
			// (a.walling 2009-09-08 13:55) - PLID 35178 - Use the snapshot connection
			// (j.jones 2010-12-09 12:26) - PLID 41782 - broke this table down into an entry per each block of time
			// templated, so the table may have multiple entries per resource and date
			ExecuteSql(GetRemoteDataSnapshot(),
				"CREATE TABLE %s \r\n"
				"( \r\n"
				"	ResourceID int NOT NULL, \r\n"
				"	Date datetime NOT NULL, \r\n"
				"	StartTime datetime NOT NULL, \r\n"
				"	EndTime datetime NOT NULL \r\n"
				") \r\n"
				, strTempTable);

			CArray<long,long> arynResourceIDs;
			pdlgTemplate->GetAllLoadedResourceIDs(arynResourceIDs);

			// (z.manning 2009-07-20 08:41) - PLID 22054 - We need to loop through every day in our possible
			// date range in order to get template info since TemplateHitAllP must be called for a specific date.
			CString strInsert;
			COleDateTimeSpan dtsOneDay = COleDateTimeSpan(1, 0, 0, 0);
			long nInsertStatementCount = 1;
			for(COleDateTime dtDate = dtStart; dtDate <= dtEnd; dtDate += dtsOneDay)
			{
				// (z.manning 2009-07-20 09:00) - PLID 22054 - Load all the template info for the current date
				// on the template editor dialog.
				pdlgTemplate->ClearTotalMinuteMap();
				pdlgTemplate->ClearStartEndTimeArray();
				pdlgTemplate->SetActiveDate(dtDate);
				pdlgTemplate->UpdateTemplateReservations();
				
				// (z.manning 2009-07-20 09:01) - PLID 22054 - Loop through each resource and get the total
				// number of minutes available for scheduling on that date and store the results in a temp table.
				for(int nResourceIndex = 0; nResourceIndex < arynResourceIDs.GetSize(); nResourceIndex++) {
					const long nResourceID = arynResourceIDs.GetAt(nResourceIndex);
					// (j.jones 2010-12-09 12:50) - PLID 41782 - we no longer need the total minutes, instead we need the start and end times
					// for all resources templated on this day
					//long nMinutes = pdlgTemplate->GetTotalMinutesByResourceID(nResourceID);
					CArray<ResourceStartEndTime, ResourceStartEndTime> aryStartEndTimes;
					pdlgTemplate->GetStartEndTimeArrayForResourceID(nResourceID, aryStartEndTimes);

					for(int nTimeIndex = 0; nTimeIndex < aryStartEndTimes.GetSize(); nTimeIndex++) {

						ResourceStartEndTime rsetInfo = (ResourceStartEndTime)aryStartEndTimes.GetAt(nTimeIndex);
					
						CString strTemp = FormatString(
							"INSERT INTO %s (ResourceID, Date, StartTime, EndTime) VALUES (%li, '%s', '%s', '%s')\r\n"
							, strTempTable, nResourceID, FormatDateTimeForSql(dtDate, dtoDate), FormatDateTimeForSql(rsetInfo.dtStartTime, dtoTime), FormatDateTimeForSql(rsetInfo.dtEndTime, dtoTime));
						GrowFastConcat(strInsert, strTemp.GetLength(), strTemp);
						
						//track how many insert statements we've created
						nInsertStatementCount++;

						// (z.manning 2009-07-20 09:01) - PLID 22054 - We need to commit our SQL every so
						// oftent to avoid running out of memory due to a single query with a massive amount
						// of insert statements.
						if(nInsertStatementCount % 300 == 0) {
							// (a.walling 2009-09-08 13:55) - PLID 35178 - Use the snapshot connection
							// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
							NxAdo::PushMaxRecordsWarningLimit pmr(300);
							ExecuteSqlStd(GetRemoteDataSnapshot(), strInsert);
							strInsert.Empty();
						}
					}
				}
			}

			if(!strInsert.IsEmpty()) {
				// (a.walling 2009-09-08 13:55) - PLID 35178 - Use the snapshot connection
				// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
				NxAdo::PushMaxRecordsWarningLimit pmr(300);
				ExecuteSqlStd(GetRemoteDataSnapshot(), strInsert);
			}

			// (z.manning 2009-07-16 07:34) - PLID 22054 - Destory our invisible dialog as it's no longer needed.
			pdlgTemplate->DestroyWindow();
			delete pdlgTemplate;
			pdlgTemplate = NULL;

			// (z.manning 2009-07-20 09:02) - PLID 22054 - Finally, let's load the report query based on
			// the temp table we just populated.
			// (j.jones 2010-12-09 12:00) - PLID 35058 - INNER joined the ResourceT and AppointmentsT tables
			// so this report never shows resources that weren't scheduled
			// (j.jones 2010-12-09 12:58) - PLID 41782 - our temp table is now broken down by each template block
			// per resource on the given date, our times are now defined by start and end times
			// (j.kuziel 2013-11-07) - PLID 59161 - Broken up TotalMinutes into
			// days instead of the entire date range which was wrong.
			// (j.kuziel 2013-11-05) - PLID 59382 - Fixed appointments not completely within a template block to be counted.
			// (s.tullis 2015-08-24 14:37) - PLID 66411 - Fixed issue with appointment counts
			CString strSql = FormatString(
				"SELECT Appt.Date AS Date, ApptResLink.ResourceID AS ResourceID, ResLoc.LocationID AS LocID, Res.Item AS Resource, "
				"SUM(DATEDIFF(MINUTE, "
				"	CASE WHEN dbo.AsTimeNoDate(Appt.StartTime) < dbo.AsTimeNoDate(M2.StartTime) THEN dbo.AsTimeNoDate(M2.StartTime) ELSE dbo.AsTimeNoDate(Appt.StartTime) END, "
				"	CASE WHEN dbo.AsTimeNoDate(Appt.EndTime) > dbo.AsTimeNoDate(M2.EndTime) THEN dbo.AsTimeNoDate(M2.EndTime) ELSE dbo.AsTimeNoDate(Appt.EndTime) END "
				")) AS MinutesScheduled, "
				"Appt.AptTypeID AS AptTypeID, AptTypeT.Name AS AptTypeName, "
				"TotalMinutes, COUNT(DISTINCT Appt.ID) AS CountOfAppts, Loc.Name AS Location "
				"FROM AppointmentsT Appt "
				"LEFT JOIN AptTypeT ON Appt.AptTypeID = AptTypeT.ID "
				"INNER JOIN AppointmentResourceT ApptResLink ON Appt.ID = ApptResLink.AppointmentID "
				"INNER JOIN ResourceT Res ON ApptResLink.ResourceID = Res.ID "
				"INNER JOIN ( "
					"SELECT "
					"ResourceID, Date, SUM(DateDiff(MINUTE, StartTime, EndTime)) AS TotalMinutes "
					"FROM %s M "
					"GROUP BY ResourceID, Date) MinutesT ON Res.ID = MinutesT.ResourceID "
					"AND Appt.Date = MinutesT.Date "
				"INNER JOIN %s M2 ON Res.ID = M2.ResourceID "
					"AND dbo.AsDateNoTime(Appt.Date) = dbo.AsDateNoTime(M2.Date) "
					"AND dbo.AsTimeNoDate(Appt.StartTime) < dbo.AsTimeNoDate(M2.EndTime) "
					"AND dbo.AsTimeNoDate(Appt.EndTime) > dbo.AsTimeNoDate(M2.StartTime) "
				"LEFT JOIN ResourceLocationConnectT ResLoc ON Res.ID = ResLoc.ResourceID "
				"LEFT JOIN LocationsT Loc ON ResLoc.LocationID = Loc.ID "
				"WHERE Appt.Status <> 4 AND Res.Inactive = 0 "
				"GROUP BY Appt.Date, ApptResLink.ResourceID, ResLoc.LocationID, Res.Item, Appt.AptTypeID, AptTypeT.Name, Loc.Name, MinutesT.TotalMinutes ",
				strTempTable, strTempTable);

			// (z.manning 2009-07-20 09:03) - PLID 22054 - We can't drop the temp table here because we 
			// haven't actually run the query yet. But that's fine since it's just a temp table anyway.

			return strSql;
		}
		break;

	case 672:
		// Room History
		//
		// Version History
		// (a.walling 2009-08-04 09:05) - PLID 23318 - Created
		// (j.jones 2009-09-29 10:58) - PLID 35691 - added SecondsAtThisStatus, SecondsInRoomInstance,
		// and TimesCheckedIntoRoom, for use in report calculations, and removed the breakdown by resource &
		// purpose because it was unnecessary and gave confusing results, altering the purpose of this report
		// Also ensured this report only displays room appointments that have left a room (either Ready To
		// Check Out, or Checked Out)
		// (j.jones 2010-12-03 09:30) - PLID 41626 - ensured this report ignores waiting rooms

		return _T("SELECT RoomsT.ID AS RoomID, RoomsT.Name AS RoomName, PersonT.ID AS PatID, PatientsT.UserDefinedID, "
			"PrefixT.Prefix, PersonT.First, PersonT.Middle, PersonT.Last, PatientsT.NickName, PersonT.Title, "
			"AppointmentsT.ID AS AppointmentID, AppointmentsT.Date AS Date, "
			"AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.ArrivalTime, "
			"AptTypeT.Name AS TypeName, AppointmentsT.CreatedDate, AppointmentsT.CreatedLogin, "
			"AppointmentsT.NoShowDate, AppointmentsT.CancelledDate, "
			"dbo.GetPurposeString(AppointmentsT.ID) AS Purposes, "
			"dbo.GetResourceString(AppointmentsT.ID) AS Resources, "
			"LocationsT.ID AS LocID, LocationsT.Name AS Location, "
			"RoomAppointmentsT.ID AS RoomAppointmentID, RoomAppointmentsT.CheckInTime, "
			"RoomAppointmentHistoryT.UpdateTime, UsersT.UserName AS UpdatedBy, RoomStatusT.Name AS UpdateStatus, "
			"CASE WHEN RoomAppointmentHistoryT.StatusID IN (-1,0) THEN NULL "
				"ELSE (SELECT DateDiff(second, RoomAppointmentHistoryT.UpdateTime, Coalesce(Min(RoomAppointmentHistoryT_Next.UpdateTime), GetDate())) "
				"FROM RoomAppointmentHistoryT RoomAppointmentHistoryT_Next "
				"WHERE RoomAppointmentHistoryT_Next.UpdateTime > RoomAppointmentHistoryT.UpdateTime "
				"AND RoomAppointmentHistoryT_Next.RoomAppointmentID = RoomAppointmentHistoryT.RoomAppointmentID) END AS SecondsAtThisStatus, "
			"TotalTimePerRoomApptQ.SecondsInRoomAppt AS SecondsInRoomInstance, "
			"(SELECT Count(*) FROM RoomAppointmentsT WHERE AppointmentID = AppointmentsT.ID AND RoomID = RoomsT.ID) AS TimesCheckedIntoRoom "
			"FROM RoomsT "
			"INNER JOIN RoomAppointmentsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"INNER JOIN RoomAppointmentHistoryT ON RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
			"INNER JOIN RoomStatusT ON RoomStatusT.ID = RoomAppointmentHistoryT.StatusID "
			"INNER JOIN UsersT ON RoomAppointmentHistoryT.UpdateUserID = UsersT.PersonID "
			"INNER JOIN AppointmentsT ON AppointmentsT.ID = RoomAppointmentsT.AppointmentID "
			"INNER JOIN LocationsT ON LocationsT.ID = AppointmentsT.LocationID "
			"INNER JOIN (SELECT RoomAppointmentID, DateDiff(second, Min(UpdateTime), Coalesce(Max(UpdateTime), GetDate())) AS SecondsInRoomAppt "
				"FROM RoomAppointmentHistoryT "
				"WHERE (RoomAppointmentHistoryT.StatusID <> -1 "
				"OR RoomAppointmentID NOT IN (SELECT RoomAppointmentID FROM RoomAppointmentHistoryT WHERE StatusID = 0)) "
				"GROUP BY RoomAppointmentID) AS TotalTimePerRoomApptQ ON RoomAppointmentsT.ID = TotalTimePerRoomApptQ.RoomAppointmentID "
			"LEFT JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID "
			"LEFT JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID "
			"LEFT JOIN PatientsT ON PatientsT.PersonID = AppointmentsT.PatientID "
			"LEFT JOIN PrefixT ON PrefixT.ID = PersonT.PrefixID "
			"WHERE RoomsT.WaitingRoom = 0 "
			"AND (RoomAppointmentHistoryT.StatusID <> -1 "
			"OR RoomAppointmentsT.ID NOT IN (SELECT RoomAppointmentID FROM RoomAppointmentHistoryT WHERE StatusID = 0)) "
			"AND AppointmentsT.Status <> 4 "
			"AND RoomAppointmentsT.ID IN (SELECT RoomAppointmentID FROM RoomAppointmentHistoryT WHERE StatusID IN (0, -1)) ");
		break;

	case 683:
		// Room Manager History By Type/Purpose
		//
		// Version History
		// (j.jones 2009-09-30 09:00) - PLID 30180 - created
		// This report only displays room appointments that have been fully checked out.
		// (j.jones 2010-12-03 09:30) - PLID 41626 - ensured that waiting rooms are not shown, 
		// but are included in the time calculated as "waiting", prior to being checked in a real room
		// (j.jones 2011-07-29 13:09) - PLID 44481 - fixed waiting room calculations to be a left join,
		// for when the patient was never put into a waiting room
		//(s.dhole 3/19/2015 5:24 PM ) - PLID PLID 60229  rename fild name  AppointmentID to ApptID

		return _T("SELECT RoomsT.ID AS RoomID, RoomsT.Name AS RoomName, PersonT.ID AS PatID, PatientsT.UserDefinedID, "
			"CASE WHEN AppointmentsT.PatientID = -25 THEN 'No Patient Selected' ELSE Last + ', ' + First + ' ' + Middle END AS PatName, "
			"AppointmentsT.ID AS ApptID, AppointmentsT.Date AS Date, "
			"AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.ArrivalTime, "
			"AptTypeT.ID AS AptTypeID, Coalesce(AptTypeT.Name, '<No Type>') AS TypeName, "
			"AppointmentsT.CreatedDate, AppointmentsT.CreatedLogin, "
			"AptPurposeT.ID AS PurposeID, Coalesce(AptPurposeT.Name, '<No Purpose>') AS PurposeName, "
			"Coalesce(dbo.GetPurposeString(AppointmentsT.ID), '<No Purpose>') AS Purposes, "
			"dbo.GetResourceString(AppointmentsT.ID) AS Resources, "
			"LocationsT.ID AS LocID, LocationsT.Name AS Location, "
			"RoomAppointmentsT.ID AS RoomAppointmentID, RoomAppointmentsT.CheckInTime, "
			"RoomAppointmentHistoryT.UpdateTime, UsersT.UserName AS UpdatedBy, RoomStatusT.Name AS UpdateStatus, "
			"Convert(bit, CASE WHEN RoomAppointmentHistoryT.StatusID = 0 THEN 1 ELSE 0 END) AS IsInCheckoutList, "
			"CASE WHEN RoomAppointmentHistoryT.StatusID IN (-1,0) THEN NULL "
				"ELSE (SELECT DateDiff(second, RoomAppointmentHistoryT.UpdateTime, Coalesce(Min(RoomAppointmentHistoryT_Next.UpdateTime), GetDate())) "
				"FROM RoomAppointmentHistoryT RoomAppointmentHistoryT_Next "
				"WHERE RoomAppointmentHistoryT_Next.UpdateTime > RoomAppointmentHistoryT.UpdateTime "
				"AND RoomAppointmentHistoryT_Next.RoomAppointmentID = RoomAppointmentHistoryT.RoomAppointmentID) END AS SecondsAtThisStatus, "
			"CASE WHEN RoomAppointmentHistoryT.StatusID <> 0 THEN NULL "
				"ELSE (SELECT DateDiff(second, RoomAppointmentHistoryT.UpdateTime, Coalesce(Min(RoomAppointmentHistoryT_Next.UpdateTime), GetDate())) "
				"FROM RoomAppointmentHistoryT RoomAppointmentHistoryT_Next "
				"WHERE RoomAppointmentHistoryT_Next.UpdateTime > RoomAppointmentHistoryT.UpdateTime "
				"AND RoomAppointmentHistoryT_Next.RoomAppointmentID = RoomAppointmentHistoryT.RoomAppointmentID) END AS SecondsInCheckoutListAtThisStatus, "
			"Coalesce(TotalTimeInCheckoutListQ.TotalSecondsInCheckoutList, 0) AS TotalSecondsInCheckoutList, "
			"TotalTimePerRoomApptQ.SecondsInRoomAppt AS SecondsInRoomInstance, "
			"TotalRoomTimePerApptQ.SecondsInAllRooms, "
			"FirstTimeInAnyRoomQ.FirstTimeInARoom, TimeMarkedInQ.TimeMarkedIn, "
			// (j.jones 2010-12-03 09:44) - PLID 41626 - wait time is calculated as checkin time until moved into *any*
			// room, including waiting rooms, plus the total time spent in waiting rooms for this appt.
			"CASE WHEN TimeMarkedInQ.TimeMarkedIn Is Null THEN 0 ELSE "
				"DateDiff(second, TimeMarkedInQ.TimeMarkedIn, FirstTimeInAnyRoomQ.FirstTimeInARoom) END "
				"+ Coalesce(TotalWaitingRoomTimePerApptQ.SecondsInAllWaitingRooms, 0) "
			"	AS SecondsInWaitingArea "
			"FROM RoomsT "
			"INNER JOIN RoomAppointmentsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"INNER JOIN RoomAppointmentHistoryT ON RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
			"INNER JOIN RoomStatusT ON RoomStatusT.ID = RoomAppointmentHistoryT.StatusID "
			"INNER JOIN UsersT ON RoomAppointmentHistoryT.UpdateUserID = UsersT.PersonID "
			"INNER JOIN AppointmentsT ON AppointmentsT.ID = RoomAppointmentsT.AppointmentID "
			"LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			"LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
			"INNER JOIN LocationsT ON LocationsT.ID = AppointmentsT.LocationID "
			"INNER JOIN (SELECT RoomAppointmentID, DateDiff(second, Min(UpdateTime), Coalesce(Max(UpdateTime), GetDate())) AS SecondsInRoomAppt "
				"FROM RoomAppointmentHistoryT "
				"INNER JOIN RoomAppointmentsT ON RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
				"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"WHERE RoomsT.WaitingRoom = 0 AND (RoomAppointmentHistoryT.StatusID <> -1 "
				"OR RoomAppointmentID NOT IN (SELECT RoomAppointmentID FROM RoomAppointmentHistoryT WHERE StatusID = 0)) "
				"GROUP BY RoomAppointmentID) AS TotalTimePerRoomApptQ ON RoomAppointmentsT.ID = TotalTimePerRoomApptQ.RoomAppointmentID "
			"INNER JOIN (SELECT AppointmentID, Sum(SecondsInRoomAppt) AS SecondsInAllRooms "
				"FROM RoomAppointmentsT "
				"INNER JOIN (SELECT RoomAppointmentID, DateDiff(second, Min(UpdateTime), Coalesce(Max(UpdateTime), GetDate())) AS SecondsInRoomAppt "
					"FROM RoomAppointmentHistoryT "
					"INNER JOIN RoomAppointmentsT ON RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
					"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
					"WHERE RoomsT.WaitingRoom = 0 AND (RoomAppointmentHistoryT.StatusID <> -1 "
					"OR RoomAppointmentID NOT IN (SELECT RoomAppointmentID FROM RoomAppointmentHistoryT WHERE StatusID = 0)) "
					"GROUP BY RoomAppointmentID) AS TotalTimePerRoomApptQ ON RoomAppointmentsT.ID = TotalTimePerRoomApptQ.RoomAppointmentID "
				"GROUP BY AppointmentID) AS TotalRoomTimePerApptQ ON AppointmentsT.ID = TotalRoomTimePerApptQ.AppointmentID "
			"LEFT JOIN (SELECT AppointmentID, Sum(SecondsInWaitingRoomAppt) AS SecondsInAllWaitingRooms "
				"FROM RoomAppointmentsT "
				"INNER JOIN (SELECT RoomAppointmentID, DateDiff(second, Min(UpdateTime), Coalesce(Max(UpdateTime), GetDate())) AS SecondsInWaitingRoomAppt "
					"FROM RoomAppointmentHistoryT "
					"INNER JOIN RoomAppointmentsT ON RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
					"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
					"WHERE RoomsT.WaitingRoom = 1 AND (RoomAppointmentHistoryT.StatusID <> -1 "
					"OR RoomAppointmentID NOT IN (SELECT RoomAppointmentID FROM RoomAppointmentHistoryT WHERE StatusID = 0)) "
					"GROUP BY RoomAppointmentID) AS TotalTimePerWaitingRoomApptQ ON RoomAppointmentsT.ID = TotalTimePerWaitingRoomApptQ.RoomAppointmentID "
				"GROUP BY AppointmentID) AS TotalWaitingRoomTimePerApptQ ON AppointmentsT.ID = TotalWaitingRoomTimePerApptQ.AppointmentID "
			"LEFT JOIN (SELECT AppointmentID, "
				"Sum(DateDiff(second, ReadyToCheckoutHistoryQ.UpdateTime, Coalesce(CheckoutHistoryQ.UpdateTime, GetDate()))) AS TotalSecondsInCheckoutList "
				"FROM RoomAppointmentsT "
				"INNER JOIN (SELECT RoomAppointmentID, UpdateTime "
					"FROM RoomAppointmentHistoryT "
					"WHERE StatusID = 0) AS ReadyToCheckoutHistoryQ ON RoomAppointmentsT.ID = ReadyToCheckoutHistoryQ.RoomAppointmentID "
				"LEFT JOIN (SELECT RoomAppointmentID, UpdateTime "
					"FROM RoomAppointmentHistoryT "
					"WHERE StatusID = -1) AS CheckoutHistoryQ ON RoomAppointmentsT.ID = CheckoutHistoryQ.RoomAppointmentID "
				"GROUP BY AppointmentID) AS TotalTimeInCheckoutListQ ON AppointmentsT.ID = TotalTimeInCheckoutListQ.AppointmentID "
			"LEFT JOIN (SELECT AppointmentID, Min(CheckInTime) AS FirstTimeInARoom "
				"FROM RoomAppointmentHistoryT "
				"INNER JOIN RoomAppointmentsT ON RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
				"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				// (j.jones 2010-12-03 09:43) - PLID 41626 - include waiting rooms for this calculation
				//"WHERE RoomsT.WaitingRoom = 0 "
				"GROUP BY AppointmentID) AS FirstTimeInAnyRoomQ ON AppointmentsT.ID = FirstTimeInAnyRoomQ.AppointmentID "
			"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn "
				"FROM AptShowStateHistoryT "
				"WHERE ShowStateID = 1 "
				"AND AppointmentID NOT IN (SELECT AppointmentID FROM RoomAppointmentsT "
					"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
					"WHERE RoomsT.WaitingRoom = 0 AND CheckInTime < TimeStamp) "
				"GROUP BY AppointmentID) AS TimeMarkedInQ ON AppointmentsT.ID = TimeMarkedInQ.AppointmentID "
			"LEFT JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID "
			"INNER JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID "
			"INNER JOIN PatientsT ON PatientsT.PersonID = AppointmentsT.PatientID "
			"WHERE AppointmentsT.Status <> 4 "
			"AND RoomsT.WaitingRoom = 0 "
			"AND RoomAppointmentsT.ID IN (SELECT RoomAppointmentID FROM RoomAppointmentHistoryT WHERE StatusID = -1)");
		break;

	case 694: /* Appointment Reminders (CellTrust) 

			Version History
			(c.haag 2010-03-31 15:51) - PLID 37998 - Initial implementation
			// (z.manning 2010-11-30 17:03) - PLID 41660 - Updated with new data structure

		*/
		return _T("SELECT AppointmentRemindersT.ID, ReminderSent AS Date, ServerID, StatusText, SentTo, AppointmentRemindersT.Status, "
			"AppointmentsT.StartTime AS StartTime, AppointmentsT.EndTime, PersonT.First, PersonT.Middle, PersonT.Last, "
			"    AppointmentsT.Date AS ApptDate,  "
			"	 AppointmentsT.Notes, AptTypeT.Name AS Type, dbo.GetResourceString(AppointmentsT.ID) AS Resources,   "
			"	 AppointmentsT.Status AS ApptStatus, AppointmentsT.ShowState,  "
			"	 PersonT.HomePhone,   "
			"	 dbo.GetPurposeString(AppointmentsT.ID) AS Purposes,   "
			"	 CASE WHEN AppointmentsT.PatientID = -25 THEN 'No Patient Selected' ELSE Last + ', ' + First + ' ' + Middle END AS PatName,  "
			"	 PatientsT.UserDefinedID AS ChartNumber, "
			"	 LocationsT.Name AS Location, PersonT.PrivHome, PersonT.PrivWork, PersonT.Email, PersonT.ID AS PatID "
			"FROM AppointmentRemindersT "
			"LEFT JOIN AppointmentsT ON AppointmentsT.ID = AppointmentRemindersT.AppointmentID "
			"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
			"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  ");

	case 758: /*Scheduling Mix Rules By Resource

			  Version History
			  TES 11/21/2014 - PLID 64114 - Initial implementation
			  */
		return R"(
			SELECT ScheduleMixRulesT.ID AS RuleID, ScheduleMixRulesT.Name AS RuleName, RuleMaxQ.MaxAppts, 
			ResourceT.ID AS ResourceID, ResourceT.Item AS ResourceName, 
			PersonT.Last AS PatientLast, PersonT.First AS PatientFirst, PersonT.Middle AS PatientMiddle, PatientsT.UserDefinedID, 
			AppointmentsT.Date AS Date, AppointmentsT.StartTime, AptTypeT.Name AS AptTypeName, dbo.GetPurposeString(AppointmentsT.ID) AS AptPurposeName, 
			LocationsT.ID AS LocID, LocationsT.Name AS Location, InsuranceCoT.Name AS Insurance

			FROM ScheduleMixRulesT INNER JOIN(SELECT RuleID, ResourceID, Sum(MaxAppts) AS MaxAppts FROM ScheduleMixRuleDetailsT GROUP BY RuleID, ResourceID) RuleMaxQ ON ScheduleMixRulesT.ID = RuleMaxQ.RuleID

			INNER JOIN ScheduleMixRuleDetailsT ON ScheduleMixRulesT.ID = ScheduleMixRuleDetailsT.RuleID AND RuleMaxQ.ResourceID = ScheduleMixRuleDetailsT.ResourceID
			LEFT JOIN ScheduleMixRuleLocationsT ON ScheduleMixRulesT.ID = ScheduleMixRuleLocationsT.RuleID
			LEFT JOIN ScheduleMixRuleInsuranceCosT ON ScheduleMixRulesT.ID = ScheduleMixRuleInsuranceCosT.RuleID
			INNER JOIN
			(
			AppointmentsT LEFT JOIN(PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID INNER JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID
			LEFT JOIN
			(
			AppointmentPurposeT
			INNER JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID
			) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID
			INNER JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID
			INNER JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID
			LEFT JOIN
			(
			AppointmentInsuredPartyT
			INNER JOIN InsuredPartyT ON AppointmentInsuredPartyT.InsuredPartyID = InsuredPartyT.PersonID
			INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID
			) ON AppointmentInsuredPartyT.AppointmentID = AppointmentsT.ID AND AppointmentInsuredPartyT.Placement = 1
			) ON ScheduleMixRulesT.StartDate <= AppointmentsT.Date
			AND(ScheduleMixRulesT.EndDate IS NULL OR AppointmentsT.Date <= ScheduleMixRulesT.EndDate)
			AND(ScheduleMixRuleInsuranceCosT.InsuranceCoID IS NULL OR ScheduleMixRuleInsuranceCosT.InsuranceCoID = InsuranceCoT.PersonID)
			AND(ScheduleMixRuleLocationsT.LocationID IS NULL OR ScheduleMixRuleLocationsT.LocationID = AppointmentsT.LocationID)
			AND ScheduleMixRuleDetailsT.ResourceID = ResourceT.ID
			AND(ScheduleMixRuleDetailsT.AptTypeID IS NULL OR ScheduleMixRuleDetailsT.AptTypeID = AppointmentsT.AptTypeID)
			AND(ScheduleMixRuleDetailsT.AptPurposeID IS NULL OR ScheduleMixRuleDetailsT.AptPurposeID = AptPurposeT.ID)
			WHERE AppointmentsT.Status <> 4
			GROUP BY ScheduleMixRulesT.ID, ScheduleMixRulesT.Name, RuleMaxQ.MaxAppts, 
			ResourceT.ID, ResourceT.Item, 
			PersonT.Last, PersonT.First, PersonT.Middle, PatientsT.UserDefinedID, 
			AppointmentsT.Date, AppointmentsT.StartTime, AptTypeT.Name, AppointmentsT.ID, 
			LocationsT.ID, LocationsT.Name, InsuranceCoT.Name
			)";

	default:
		return _T("");
		break;
	}
}
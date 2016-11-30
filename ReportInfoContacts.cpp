////////////////
// DRT 8/6/03 - GetSqlContacts() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"

CString CReportInfo::GetSqlContacts(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;
	switch (nID) {
	

	case 10:
		//Contact List
		/* Version History:
			JMM 10/27/05 - added email address and cell phone number"
			// (m.hancock 2006-10-24 11:13) - PLID 22216 - Added PersonT.Archived, PersonT.ID AS PatID to query
		*/
		return _T("SELECT PersonT.ID, PersonT.First, PersonT.Middle, PersonT.Last,   "
			"PersonT.City, PersonT.State, PersonT.Zip,   "
			"    PersonT.HomePhone, PersonT.WorkPhone, PersonT.Address1,   "
			"    PersonT.Address2, PersonT.Company, PersonT.CellPhone, PersonT.Email, PersonT.Archived AS Archived, PersonT.ID AS PatID  "
			"FROM PersonT LEFT JOIN  "
			"    ContactsT ON PersonT.ID = ContactsT.PersonID "
			"    LEFT JOIN SupplierT ON PersonT.ID = SupplierT.PersonID "
			"    LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"    LEFT JOIN ReferringPhyST ON PersonT.ID = ReferringPhySt.PersonID "
			"    LEFT JOIN UsersT ON PersonT.ID = UsersT.PersonID AND UsersT.PersonID > 0 "
			"WHERE (ContactsT.PersonID IS NOT NULL) OR (SupplierT.PersonID IS NOT NULL) OR "
			"(ProvidersT.PersonID IS NOT NULL) OR (ReferringPhyST.PersonID IS NOT NULL) OR "
			"(UsersT.PersonID IS NOT NULL)");
		break;
	
	case 15:
		//Contact To Do List
		/*	Version History
			TES 7/23/03: Added filter for Start Reminding Date
			DRT 7/13/2004 - PLID 13439 - Added to ttx file function because the Note field is > 255 chars.
			e.lally 2007-05-04 - PLID 25253 - Account for null person IDs.
			(c.haag 2008-06-30 12:26) - PLID 30565 - Updated for new multi-assignee todo structure
			// (j.jones 2010-02-04 14:59) - PLID 36500 - reworked outer query to avoid filtering issues
		*/
		return _T("SELECT TaskType, Task, Priority, Contact, Notes, SubQ.Date AS Date, SubQ.Done AS Done, SubQ.TaskID AS TaskID, AssignID, "
			"	AssignName, SubQ.StateID AS StateID, SubQ.RemindDate AS RemindDate, SubQ.PatID AS PatID, AssignFullName, AssignIDs "
			"FROM (SELECT NoteCatsF.Description AS TaskType, ToDoList.Task, ToDoList.Priority, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Contact, "
			"    ToDoList.Notes, ToDoList.Deadline AS Date, ToDoList.Done AS Done,    "
			"    ToDoList.TaskID AS TaskID, NULL AS AssignID, dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignName, "
			"   CASE WHEN ToDoList.Done Is Null THEN 1 ELSE 2 END AS StateID,  "
			"	PersonT.HomePhone, PersonT.Email, ToDoList.Remind AS RemindDate, "
			"	ToDoList.PersonID AS PatID, dbo.GetTodoAssignToFullNamesString(ToDoList.TaskID) AS AssignFullName, "
			"	dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignIDs "
			"FROM ToDoList LEFT JOIN PersonT ON ToDoList.PersonID = PersonT.ID  "
			"LEFT JOIN PersonT PersonT2 ON ToDoList.EnteredBy = PersonT2.ID "
			"LEFT JOIN NoteCatsF ON ToDoList.CategoryID = NoteCatsF.ID "
			"WHERE PersonT.ID NOT IN (SELECT PersonID FROM PatientsT) OR PersonT.ID IS NULL "
			") AS SubQ");
		break;
	

	case 76:
		//Referring Physicians List
		/*	Version History
			DRT 7/23/03 - Added RefPhysID field to the query and added an external filter to let
				you choose referring physicians, in case you wanted to print a list of some of them
				and take it somewhere (PLID 7870)
			DRT 7/13/2004 - PLID 13439 - Reverified for editing.
			JMM 5/12/2005 - PLID 16206 - Added the all the other IDs to the query
			// (m.hancock 2006-10-23 16:57) - PLID 22216 - Added PersonT.Archived, PersonT.ID AS PatID to query
			// (j.gruber 2006-11-08 11:16) - PLID 23353 - added NPI to the report
		*/
		return _T("SELECT PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    PersonT.Address1, PersonT.Address2, PersonT.City,  "
		"    PersonT.State, PersonT.Zip, PersonT.HomePhone,  "
		"    PersonT.WorkPhone, ReferringPhyST.ReferringPhyID,  "
		"    ReferringPhyST.UPIN, ReferringPhyST.PersonID, PersonT.Fax, PersonT.Company, "
		"	 PersonT.ID AS RefPhysID, "
		"    BlueshieldID, FedEmployerID, DEANumber, MedicareNumber, MedicaidNumber, "
		"    WorkersCompNumber, OtherIDNumber, License, TaxonomyCode, PersonT.Archived AS Archived, PersonT.ID AS PatID, NPI  "
		"FROM ReferringPhyST INNER JOIN "
		"    PersonT ON ReferringPhyST.PersonID = PersonT.ID");
		break;
	
	case 81:
		//Doctor/Provider List
		/* Version History
			// (m.hancock 2006-10-24 10:08) - PLID 22216 - Added PersonT.Archived, PersonT.ID AS PatID to query
		*/
		return _T("SELECT PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    PersonT.Address1, PersonT.Address2, PersonT.City,  "
		"    PersonT.Zip, PersonT.HomePhone, ProvidersT.UPIN, PersonT.Location as LocID, "
		"    ProvidersT.EIN, ProvidersT.[Fed Employer ID],  "
		"    PersonT.SocialSecurity, ProvidersT.[DEA Number],  "
		"    ProvidersT.[BCBS Number], ProvidersT.[Medicare Number],  "
		"    ProvidersT.[Workers Comp Number],  "
		"    ProvidersT.[Other ID Number],  "
		"    ProvidersT.[Other ID Description], PersonT.State,  "
		"    ProvidersT.[Medicaid Number], PersonT.Archived AS Archived, PersonT.ID AS PatID   "
		"FROM ProvidersT INNER JOIN "
		"    PersonT ON ProvidersT.PersonID = PersonT.ID");
		break;
	case 82: 
		//Employee List
		/* Version History
			// (m.hancock 2006-10-24 10:59) - PLID 22216 - Added PersonT.Archived, PersonT.ID AS PatID to query
		*/
		return _T("SELECT PersonT.Address1, PersonT.Address2, PersonT.City,  "
		"    PersonT.State, PersonT.Zip, PersonT.HomePhone,  "
		"    PersonT.WorkPhone, PersonT.BirthDate,  "
		"    PersonT.SocialSecurity, PersonT.First, PersonT.Middle,  "
		"    PersonT.Last, PersonT.Location as LocID, UsersT.UserName, PersonT.Archived AS Archived, PersonT.ID AS PatID "
		"FROM UsersT INNER JOIN "
		"    PersonT ON UsersT.PersonID = PersonT.ID AND UsersT.PersonID > 0");
		break;
	


	case 99: 
		//Supplier List
		/* Version History
			// (m.hancock 2006-10-24 11:25) - PLID 22216 - Added PersonT.Archived, PersonT.ID AS PatID to query
		*/
		return _T("SELECT PersonT.Company, PersonT.CompanyID, PersonT.Address1, PersonT.Address2, PersonT.City, "
			"PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.Fax, PersonT.Archived AS Archived, PersonT.ID AS PatID  "
			"FROM SupplierT INNER JOIN  "
			"PersonT ON SupplierT.PersonID = PersonT.ID");
		break;

	case 290: 
		//Other Contact List
		/* Version History
			// (m.hancock 2006-10-24 11:21) - PLID 22216 - Added PersonT.Archived, PersonT.ID AS PatID to query
		*/
		return _T("SELECT PersonT.Company, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, "
			"PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.Fax, PersonT.OtherPhone, PersonT.Email, "
			"PersonT.Archived AS Archived, PersonT.ID AS PatID  "
			"FROM ContactsT INNER JOIN  "
			"PersonT ON ContactsT.PersonID = PersonT.ID");
		break;


		default:
		return _T("");
		break;

	case 742:
   //Paid Time Off By Department
   /*Version History
		(j.deskurakis 2012-09-28) - PLID 29105 - Created
   */

	   return _T("select Employee as Employee, year as year, PatID as PatID, isNULL(DepID, -1) as DepID, replace(isNULL(Department, 'No Department'), '<Managers>', 'Managers') as Department, VacaAll, "
				"isNULL(t2a.vacaused, 0) as vacaused, isNULL((VacaAll - t2a.vacaused), 0) as VacaRemain, SickAll, isNULL(t2a.sickused, 0) as sickused, "
				"isNULL((SickAll - t2a.sickused), 0) as SickRemain, isNULL(t2a.otherused, 0) as otherused, HireDate, (VacaAll+SickAll) as totALL, "
				"isNULL((vacaused + sickused + otherused), 0) as totUsed, isNULL(((VacaAll - t2a.vacaused) + (SickAll - t2a.sickused)), 0) "
				"as totRemain from (SELECT UserName AS Employee, AttendanceAllowanceHistoryT.Year as year, PersonID as PatID, DepartmentsT.ID AS DepID, "
				"DepartmentsT.Name AS Department, AttendanceAllowanceHistoryT.VacationAllowance AS VacaAll, AttendanceAllowanceHistoryT.SickAllowance AS SickAll, "
				"UsersT.DateOfHire As HireDate FROM UsersT LEFT JOIN UserDepartmentLinkT ON UsersT.PersonID = UserDepartmentLinkT.UserID LEFT JOIN DepartmentsT "
				"ON UserDepartmentLinkT.DepartmentID = DepartmentsT.ID LEFT JOIN AttendanceAllowanceHistoryT ON UsersT.PersonID = AttendanceAllowanceHistoryT.UserID "
				"WHERE (DepartmentsT.ID IS NULL OR DepartmentsT.ID = (SELECT Top 1 DepartmentID FROM UserDepartmentLinkT WHERE UserID = "
				"PersonID)) AND AttendanceAllowanceHistoryT.Year like '20%%' and UserName like '%.%%' GROUP BY AttendanceAllowanceHistoryT.Year, UserName, PersonID, "
				"DepartmentsT.Name, DepartmentsT.ID, DateOfHire, AttendanceAllowanceHistoryT.VacationAllowance, "
				"AttendanceAllowanceHistoryT.SickAllowance) as T1a LEFT JOIN (select year as yr, userid, sum(vacation) as vacaused, sum(sick) "
				"as sickused, sum(other) as otherused from (select userid, Year(date) year, Vacation, Sick, Other from AttendanceAppointmentsT) "
				"as t1 group by userid, year) as t2a on t1a.PatID = t2a.userid and t1a.year = t2a.yr ");
		break;

	}
}

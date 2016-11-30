////////////////
// DRT 8/6/03 - GetSqlCharges() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "SharedInsuranceUtils.h"

CString CReportInfo::GetSqlCharges(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;
	switch (nID) {
	


	case 121:
		//Bills By Doctor / Provider
		/*	Version History
			DRT 6/18/03 - Changed Ins3Resp field to be the total of all non-pri/sec/pat resp, so the
					report adds up correctly.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
		*/
		return _T("SELECT BillsT.ID, PatientsT.UserDefinedID,  "
		"PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     BillsT.Date AS TDate, ChargesT.DoctorsProviders AS ProvID,  "
		"    SUM(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) AS PatTotal,  "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID = 1 THEN ChargeRespT.Amount ELSE 0 End) AS Ins1Total,  "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID = 2 THEN ChargeRespT.Amount ELSE 0 End) AS Ins2Total,  "
		"    SUM(CASE WHEN ChargeRespT.Amount Is Null THEN 0 ELSE ChargeRespT.Amount End) AS BillTotal, BillsT.Description,  "
		"    BillsT.EntryType, BillsT.InputDate AS IDate,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN ChargeRespT.Amount ELSE 0 End) AS Ins3Total, "
		"    LocationsT.ID AS LocID, LocationsT.Name AS Location "
		"FROM (ChargesT LEFT JOIN (ChargeRespT LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) LEFT OUTER JOIN "
		"    PersonT PersonT1 ON  "
		"    ChargesT.DoctorsProviders = PersonT1.ID LEFT OUTER JOIN "
		"    CPTModifierT ON  "
		"    ChargesT.CPTModifier = CPTModifierT.Number LEFT OUTER JOIN "
		"    LineItemT ON  "
		"    ChargesT.ID = LineItemT.ID RIGHT OUTER JOIN "
		"    BillsT ON ChargesT.BillID = BillsT.ID LEFT OUTER JOIN "
		"    PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    BillsT.PatientID = PatientsT.PersonID "
		"	 LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
		"	 LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"WHERE (LineItemT.Deleted = 0) AND (BillsT.Deleted = 0) AND  "
		"    (PatientsT.PersonID > 0) AND GCTypesT.ServiceID IS NULL "
		"GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
		"    BillsT.Date, ChargesT.DoctorsProviders, BillsT.Description,  "
		"    BillsT.EntryType, BillsT.InputDate,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, BillsT.ID, LocationsT.ID, LocationsT.Name "
		"HAVING (BillsT.EntryType = 1) "
		"");
		break;
	case 122:
		//Bills by Patient
		/*	Version History
			DRT 6/18/03 - Changed Ins3Resp field to be the total of all non-pri/sec/pat resp, so the
				report adds up correctly.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
		*/
		return _T("SELECT ChargesT.BillID, PatientsT.PersonID AS PatID,  "
		"    BillsT.Date AS TDate, ChargesT.DoctorsProviders AS ProvID,  "
		"    CASE WHEN SUM(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) Is Null THEN 0 ELSE SUM(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) End AS PatTotal,  "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID = 1 THEN ChargeRespT.Amount ELSE 0 End) AS Ins1Total,  "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID = 2 THEN ChargeRespT.Amount ELSE 0 End) AS Ins2Total,  "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN ChargeRespT.Amount ELSE 0 End) AS Ins3Total, "
		"    SUM(CASE WHEN ChargeRespT.Amount Is Null THEN 0 ELSE ChargeRespT.Amount End) AS BillTotal, BillsT.Description,  "
		"    BillsT.EntryType, BillsT.InputDate AS IDate,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS "
		"    ProvName, LocationsT.ID AS LocID, LocationsT.Name AS Location "
		"FROM (ChargesT LEFT JOIN (ChargeRespT LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) LEFT OUTER JOIN "
		"    PersonT PersonT1 ON  "
		"    ChargesT.DoctorsProviders = PersonT1.ID LEFT OUTER JOIN "
		"    CPTModifierT ON  "
		"    ChargesT.CPTModifier = CPTModifierT.Number LEFT OUTER JOIN "
		"    LineItemT ON  "
		"    ChargesT.ID = LineItemT.ID RIGHT OUTER JOIN "
		"    BillsT ON ChargesT.BillID = BillsT.ID LEFT OUTER JOIN "
		"    PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    BillsT.PatientID = PatientsT.PersonID "
		"	 LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
		"	 LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"WHERE (PatientsT.PersonID > 0) AND (BillsT.Deleted = 0) AND  "
		"    (LineItemT.Deleted = 0) AND GCTypesT.ServiceID IS NULL "
		"GROUP BY ChargesT.BillID, PatientsT.PersonID, BillsT.Date,  "
		"    ChargesT.DoctorsProviders, BillsT.Description, BillsT.EntryType,  "
		"    BillsT.InputDate,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, LocationsT.ID, LocationsT.Name "
		"HAVING (BillsT.EntryType = 1)");
		break;
	case 123:
		//Bills By Patient Coordinator
		/*	Version History
			DRT 6/18/03 - Changed Ins3Resp field to be the total of all non-pri/sec/pat resp, so the
				report adds up correctly.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
		*/
		return _T("SELECT BillsT.ID AS BillID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     BillsT.Date AS TDate, ProvidersT.PersonID AS ProvID,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
		"    SUM(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) AS PatTotal,  "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID = 1 THEN ChargeRespT.Amount ELSE 0 End) AS Ins1Total,  "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID = 2 THEN ChargeRespT.Amount ELSE 0 End) AS Ins2Total,  "
		"    SUM(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN ChargeRespT.Amount ELSE 0 End) AS Ins3Total, "
		"    SUM(CASE WHEN ChargeRespT.Amount Is Null THEN 0 ELSE ChargeRespT.Amount End) AS BillTotal, BillsT.Description,  "
		"    BillsT.EntryType, PatientsT.ReferralID, "
		"    BillsT.InputDate AS IDate, BillsT.Date AS BDate,  "
		"    PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    PersonT1.First AS Expr1, PersonT1.Middle AS Expr2,  "
		"    PersonT1.Last AS Expr3, "
		"    PersonT2.First AS Expr4,  "
		"    PersonT2.Middle AS Expr5, PersonT2.Last AS Expr6,  "
		"    PersonT2.Last + ', ' + PersonT2.Middle + ' ' + PersonT2.First AS PtCoordName, "
		"     UsersT.PersonID AS EmpID, LocationsT.ID AS LocID, LocationsT.Name AS Location, "
		"	  BillsT.PatCoord AS PatCoord "
		"FROM PersonT RIGHT OUTER JOIN "
		"    BillsT LEFT OUTER JOIN "
		"    (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) RIGHT OUTER JOIN "
		"    PersonT PersonT1 RIGHT OUTER JOIN "
		"    ProvidersT RIGHT OUTER JOIN "
		"    (ChargesT LEFT JOIN (ChargeRespT LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID)ON  "
		"    ProvidersT.PersonID = ChargesT.DoctorsProviders ON  "
		"    PersonT1.ID = ProvidersT.PersonID ON  "
		"    LineItemT.ID = ChargesT.ID LEFT OUTER JOIN "
		"    CPTModifierT ON  "
		"    ChargesT.CPTModifier = CPTModifierT.Number ON  "
		"    BillsT.ID = ChargesT.BillID LEFT OUTER JOIN "
		"    UsersT LEFT OUTER JOIN PersonT PersonT2 ON "
		"	 UsersT.PersonID = PersonT2.ID ON "
		"	 UsersT.PersonID = BillsT.PatCoord "
		"	 RIGHT OUTER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID ON "
		"	 PersonT.ID = PatientsT.PersonID "
		"	 LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"WHERE (LineItemT.Deleted = 0) AND (BillsT.Deleted = 0) AND  "
		"    (PatientsT.PersonID > 0) AND GCTypesT.ServiceID IS NULL "
		"GROUP BY BillsT.ID, PatientsT.PersonID, BillsT.Date,  "
		"    ProvidersT.PersonID, BillsT.Description, BillsT.EntryType,  "
		"    PatientsT.ReferralID, BillsT.InputDate,  "
		"    PersonT.First, PersonT.Middle, PersonT.Last, PersonT1.First,  "
		"    PersonT1.Middle, PersonT1.Last, "
		"    PersonT2.First, PersonT2.Middle,  "
		"    PersonT2.Last, UsersT.PersonID, LocationsT.ID, LocationsT.Name, BillsT.PatCoord "
		"HAVING (BillsT.EntryType = 1)");
		break;
	case 125:
		//Charges By Category
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			(e.lally 2007-06-18) PLID 26347 - Formatted query to be more readable.
			   - Added pre-tax total (after discounts), tax1 total, tax2 total fields.
			// (j.gruber 2009-03-25 10:43) - PLID 33691 - change discount structure
			// (d.thompson 2012-06-19) - PLID 50832 - This now follows the preference to automatically include 
				subcategories when the parent category is chosen.  I also removed the "special handling" for category
				filtering that previously existed.
			// (d.thompson 2014-03-11) - PLID 61271 - ICD-10 update
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
		"PersonT.Zip,  "
		"LineItemT.Date AS TDate,  "
		"BillsT.Date AS BDate,  "
		"ChargesT.ID,  "
		"ChargesT.ItemCode,  "
		"LineItemT.Description,  "
		"ICD9T1.CodeNumber as ICD9Code1, "
		"ICD10T1.CodeNumber as ICD10Code1, "
		"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
		"ChargesT.Quantity,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
		"ChargesT.BillID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"ProvidersT.PersonID AS ProvID,  "
		"LineItemT.Amount AS PracBillFee,  "
		"CPTModifierT.Number AS Modifier,  "
		"CPTModifierT2.Number AS Modifier2,  "
		"CategoriesQ.CategoryID AS CategoryID,  "
		"CategoriesQ.Category,  "
		"PersonT.City,  "
		"PersonT.State,  "
		"BillsT.EntryType,  "
		"LineItemT.InputDate AS IDate,  "
		"CategoriesQ.SubCategory,  "
		"CategoriesQ.ParentID AS ParentID,  "
		"ServiceT.Name, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, ServiceT.Category AS CatFilterID, "
		"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2)) AS PreTaxTotal, "
		"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2)) as Tax1Total, "
		"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2)) as Tax2Total "
		
		"FROM ChargesT "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID "		
		"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
		"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number " 
		"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
		"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
		"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID "
		"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID "
		"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID "
		"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID "
		"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"			FROM CategoriesT "
		"			WHERE CategoriesT.Parent=0 "
		"			UNION "
		"			SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"			FROM CategoriesT "
		"			RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"			WHERE SubCategoriesT.Parent<>0) AS CategoriesQ "
		"ON ServiceT.Category = CategoriesQ.CategoryID "
		"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
		"WHERE BillsT.EntryType=1 AND BillsT.Deleted=0 AND LineItemT.Deleted=0 AND GCTypesT.ServiceID IS NULL "
		"");
		break;

	case 421:
		//Charges By Category by Patient Coordinator
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			(d.thompson 2014-03-18) - PLID 61272 - Added ICD-10 fields
		*/
		return _T("SELECT PatientsT.PersonID AS PatID, PersonT.Zip, LineItemT.Date AS TDate, BillsT.Date AS BDate, ChargesT.ID, ChargesT.ItemCode, LineItemT.Description,  "
			"ICD9T1.CodeNumber as ICD9Code1, ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
			"ChargesT.Quantity, dbo.GetChargeTotal(ChargesT.ID) AS Amount, ChargesT.BillID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
			"ProvidersT.PersonID AS ProvID, LineItemT.Amount AS PracBillFee, CPTModifierT.Number AS Modifier, CPTModifierT2.Number AS Modifier2, CategoriesQ.CategoryID AS CategoryID, "
			"CategoriesQ.Category, PersonT.City, PersonT.State, BillsT.EntryType, LineItemT.InputDate AS IDate, CategoriesQ.SubCategory, CategoriesQ.ParentID AS ParentID, "
			"ServiceT.Name, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, ServiceT.Category AS CatFilterID,  "
			"PCPersonT.Last + ', ' + PCPersonT.First + ' ' + PCPersonT.Middle AS PCName  "
			" "
			"FROM  "
			"	( "
			"		( "
			"			( "
			"				( "
			"					( "
			"						( "
			"							( "
			"								(ChargesT LEFT JOIN  "
			"									(LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID)  "
			"								ON ChargesT.ID = LineItemT.ID "
			"								)  "
			"							LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number  "
			"							LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"							)  "
			"						LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			"						)  "
			"					LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
			"					)  "
			"				INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID  "
			"				LEFT JOIN PersonT PCPersonT ON ChargesT.PatCoordID = PCPersonT.ID  "
			"				LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			"				LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"				LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"				)  "
			"			LEFT JOIN  "
			"				(PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)  "
			"			ON BillsT.PatientID = PatientsT.PersonID "
			"			)  "
			"		LEFT JOIN ServiceT ON (ChargesT.ServiceID = ServiceT.ID) "
			"		)  "
			"	LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID  "
			"	LEFT JOIN  "
			"		(SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID  "
			"		FROM CategoriesT  "
			"		WHERE CategoriesT.Parent=0 "
			" "
			"		UNION  "
			" "
			"		SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID  "
			"		FROM CategoriesT  "
			"		RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent  "
			"		WHERE (((SubCategoriesT.Parent)<>0)) "
			"		) AS CategoriesQ  "
			"	ON ServiceT.Category = CategoriesQ.CategoryID "
			"	)  "
			"LEFT JOIN CPTCodeT ON (ChargesT.ServiceID = CPTCodeT.ID)  "
			"WHERE BillsT.EntryType=1 AND BillsT.Deleted=0 AND LineItemT.Deleted=0 AND GCTypesT.ServiceID IS NULL");
	break;

	case 124:
		//Bills By Referral Source
		/*	Version History
			DRT 6/18/03 - Removed a lot of *really* stupid stuff.  Pretty much this report, which claims to be a report of bills, 
					was grouping in such a way that it really was just a report of charges.  Except it showed all the bill information
					on the report (except the amt, which was for the charge).  It also listed "service date" and "bill date" on the report,
					but they were actually the exact same field!
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
		*/
		return _T("SELECT BillsT.ID AS BillID,   "
			"BillsT.PatientID AS PatID,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,   "
			"BillsT.Date AS TDate,   "
			"ProvidersT.PersonID AS ProvID,   "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,   "
			"CASE WHEN Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) Is Null THEN 0 ELSE Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) End AS PatTotal,   "
			"Sum(CASE WHEN InsuredPartyT.RespTypeID = 1 THEN ChargeRespT.Amount ELSE 0 End) AS Ins1Total,   "
			"Sum(CASE WHEN InsuredPartyT.RespTypeID = 2 THEN ChargeRespT.Amount ELSE 0 End) AS Ins2Total,   "
			"Sum(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN ChargeRespT.Amount ELSE 0 End) AS Ins3Total,   "
			"Sum(ChargeRespT.Amount) AS BillTotal,   "
			"BillsT.Description,   "
			"BillsT.EntryType,   "
			"PatientsT.ReferralID AS ReferralID,   "
			"ReferralSourceT.Name,   "
			"BillsT.InputDate AS IDate,   "
			"BillsT.Date AS BDate,  "
			"LocationsT.ID AS LocID,  "
			"LocationsT.Name AS Location  "
			"FROM (((((BillsT LEFT JOIN (ChargesT LEFT JOIN (ChargeRespT LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) ON BillsT.ID = ChargesT.BillID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number) LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID) LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID) LEFT JOIN ((PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"WHERE BillsT.Deleted=0 AND LineItemT.Deleted=0 AND GCTypesT.ServiceID IS NULL "
			"GROUP BY BillsT.ID, BillsT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, BillsT.Date, ProvidersT.PersonID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
			"BillsT.Description, BillsT.EntryType, PatientsT.ReferralID, ReferralSourceT.Name, BillsT.InputDate, BillsT.Date, LocationsT.ID, LocationsT.Name  "
			"HAVING BillsT.EntryType=1");
		break;
	case 126:
		//Charges By Category (Cross Tab)
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			(v.maida - 2014-03-18 16:47) - PLID 61333 - Modify the Charges by Category (Cross-tab) report for ICD-10.
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
		"PersonT.Zip,  "
		"LineItemT.Date AS TDate,  "
		"BillsT.Date AS BDate,  "
		"ChargesT.ID,  "
		"ChargesT.ItemCode,  "
		"LineItemT.Description,  "
		"ICD9T1.CodeNumber as ICD9Code1,  "
		"ICD10T1.CodeNumber as ICD10Code1,  "
		"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
		"ChargesT.Quantity,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
		"ChargesT.BillID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"ProvidersT.PersonID AS ProvID,  "
		"LineItemT.Amount AS PracBillFee,  "
		"CPTModifierT.Number AS Modifier,  "
		"CategoriesQ.CategoryID AS CategoryID,  "
		"CategoriesQ.Category,  "
		"PersonT.City,  "
		"PersonT.State,  "
		"BillsT.EntryType,  "
		"LineItemT.InputDate AS IDate,  "
		"CategoriesQ.SubCategory,  "
		"CategoriesQ.ParentID AS ParentID,  "
		"ServiceT.Name, "
		"PatientsT.UserDefinedID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, ServiceT.Category AS CatFilterID "
		"FROM ((((((((ChargesT LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID) INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID) LEFT JOIN ServiceT ON (ChargesT.ServiceID = ServiceT.ID)) "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0))) AS CategoriesQ ON ServiceT.Category = CategoriesQ.CategoryID) LEFT JOIN CPTCodeT ON (ChargesT.ServiceID = CPTCodeT.ID) "
		"WHERE (((BillsT.EntryType)=1) AND ((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) AND GCTypesT.ServiceID IS NULL "
		"");
		break;
	case 130:
		//Charges by Doctor/Provider
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			(e.lally 2007-06-18) PLID 26347 - Formatted query to be more readable.
			    - Added pre-tax total (after discounts), tax1 total, tax2 total fields.
			// (j.gruber 2009-03-25 10:45) - PLID 33691 - change discount fields
			// (d.thompson 2014-03-18) - PLID 61337 - Added ICD-10 fields
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
		"PersonT.Zip,  "
		"LineItemT.Date AS TDate,  "
		"BillsT.Date AS BDate,  "
		"ChargesT.ID,  "
		"ChargesT.ItemCode,  "
		"LineItemT.Description,  "
		"ICD9T1.CodeNumber as ICD9Code1, \r\n "
		"ICD10T1.CodeNumber as ICD10Code1, \r\n "
		"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
		"ChargesT.Quantity,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
		"ChargesT.BillID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"ProvidersT.PersonID AS ProvID,  "
		"LineItemT.Amount AS PracBillFee,  "
		"CPTModifierT.Number AS Modifier,  "
		"CPTModifierT2.Number AS Modifier2, "
		"CategoriesT.ID AS CatID,  "
		"CategoriesT.Name AS CatName,  "
		"PersonT.City,  "
		"PersonT.State,  "
		"BillsT.EntryType,  "
		"LineItemT.InputDate AS IDate, "
		"PatientsT.UserDefinedID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, "
		"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2)) AS PreTaxTotal, "
		"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2)) as Tax1Total, "
		"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2)) as Tax2Total "

		"FROM ChargesT "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID "		
		"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
		"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
		"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
		"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
		"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
		"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
		"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
		"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
		"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "

		"WHERE BillsT.EntryType=1 AND BillsT.Deleted=0 AND LineItemT.Deleted=0 AND GCTypesT.ServiceID IS NULL ");
		break;
	case 131:
		//Charges by Doctor / Provider (Cross Tab)
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			// (f.gelderloos 2013-07-25 09:19) - PLID 56749 Only primary diagcode is displayed (from BillsT.Diag1ID),
				modifying to include Diag2,Diag3, and Diag4. This can expand to an 'infinite' number of Diag fields, however
				these are the most-used fields.
			(v.maida - 2014-03-19 10:00) - PLID 61338 - Modify Charges by Doctor / Provider (Cross Tab) report for ICD-10. 
		*/
		return _T("SELECT PatientsT.PersonID AS PatID "
					",PersonT.Zip "
					",LineItemT.DATE AS TDate "
					",BillsT.DATE AS BDate "
					",ChargesT.ID "
					",ChargesT.ItemCode "
					",LineItemT.Description "
					",ICD9T1.CodeNumber as ICD9Code1 "
					",ICD9T2.CodeNumber as ICD9Code2 "
					",ICD9T3.CodeNumber as ICD9Code3 "
					",ICD9T4.CodeNumber as ICD9Code4 "
					",ICD10T1.CodeNumber as ICD10Code1 "
					",ICD10T2.CodeNumber as ICD10Code2 "
					",ICD10T3.CodeNumber as ICD10Code3 "
					",ICD10T4.CodeNumber as ICD10Code4 "
					",PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName "
					",ChargesT.Quantity "
					",dbo.GetChargeTotal(ChargesT.ID) AS Amount "
					",ChargesT.BillID "
					",PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName "
					",ProvidersT.PersonID AS ProvID "
					",LineItemT.Amount AS PracBillFee "
					",CPTModifierT.Number AS Modifier "
					",CategoriesT.ID AS CatID "
					",CategoriesT.NAME AS CatName "
					",PersonT.City "
					",PersonT.STATE "
					",BillsT.EntryType "
					",LineItemT.InputDate AS IDate "
					",PatientsT.UserDefinedID "
					",LineItemT.LocationID AS LocID "
					",LocationsT.NAME AS Location "
				"FROM ( "
					"( "
						"( "
							"( "
								"( "
									"( "
										"ChargesT LEFT JOIN ( "
											"LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
											") ON ChargesT.ID = LineItemT.ID "
										") LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
									") LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
								"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
								") LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
							") LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
						") INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID "
					"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID "
					"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID "
					"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID "
					"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID "
					"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID "
					"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID "
					"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID "
					"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID "
					") "
				"LEFT JOIN ( "
					"PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					") ON BillsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
				"WHERE ( "
						"((BillsT.EntryType) = 1) "
						"AND ((BillsT.Deleted) = 0) "
						"AND ((LineItemT.Deleted) = 0) "
						") "
					"AND GCTypesT.ServiceID IS NULL");
		break;
	case 134:
		//Charges by Place Of Service Designation
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			// (z.manning 2008-06-24 14:22) - PLID 29323 - Added charge patient coordinator ID and name
			// (r.gonet 03/26/2012) - PLID 49041 - Added Referring Physician and Affiliate Physician fields
			// (d.thompson 2014-03-18) - PLID 61339 - Added ICD-10 fields
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
		"PersonT.Zip,  "
		"LineItemT.Date AS TDate,  "
		"BillsT.Date AS BDate,  "
		"ChargesT.ID,  "
		"ChargesT.ItemCode,  "
		"LineItemT.Description,  "
		"ICD9T1.CodeNumber as ICD9Code1, \r\n "
		"ICD10T1.CodeNumber as ICD10Code1, \r\n "
		"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
		"ChargesT.Quantity,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
		"ChargesT.BillID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"ProvidersT.PersonID AS ProvID,  "
		"LineItemT.Amount AS PracBillFee,  "
		"CPTModifierT.Number AS Modifier,  "
		"CPTModifierT2.Number AS Modifier2, "
		"CategoriesT.ID AS CatID,  "
		"CategoriesT.Name AS CatName,  "
		"CASE WHEN PlaceCodes Is Null THEN '0' ELSE PlaceCodes End AS PofServiceID,  "
		"PlaceOfServiceCodesT.PlaceName AS PlaceOfServiceName,  "
		"PersonT.City,  "
		"PersonT.State,  "
		"BillsT.EntryType,  "
		"BillsT.Deleted,  "
		"LineItemT.InputDate AS IDate, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, "
		"ChargesT.PatCoordID, "
			"PatCoord.Last + ', ' + PatCoord.First + ' ' + PatCoord.Middle AS PatCoordName, "
			"CASE WHEN BillsT.RefPhyID IS NOT NULL AND BillsT.RefPhyID = -1 THEN NULL ELSE BillsT.RefPhyID END AS RefPhyID, "
			"ReferringPhysPerson.Last + ', ' + ReferringPhysPerson.First + ' ' + ReferringPhysPerson.Middle AS ReferringPhysName, "
			"BillsT.AffiliatePhysID, "
			"AffiliatePhysPerson.Last + ', ' + AffiliatePhysPerson.First + ' ' + AffiliatePhysPerson.Middle AS AffiliatePhysName, "
			"BillAffiliateStatusT.Name AS AffiliateStatus, "
			"BillAffiliateStatusHistoryT.Date AS AffiliatePhysDate, "
			"BillsT.AffiliatePhysAmount, "
			"BillsT.AffiliateNote "
			"FROM "
			"( "
				"( "
					"( "
						"( "
							"( "
								"( "
									"( "
										"ChargesT "
										"LEFT JOIN "
										"( "
											"LineItemT "
											"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
										") ON ChargesT.ID = LineItemT.ID "
									") "
									"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
								") "
								"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
								"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
							") "
							"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
						") "
						"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
					") "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
					"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
					"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
					"LEFT JOIN BillAffiliateStatusT ON BillsT.AffiliateStatusID = BillAffiliateStatusT.ID "
					"LEFT JOIN BillAffiliateStatusHistoryT ON BillsT.ID = BillAffiliateStatusHistoryT.BillID "
						"AND BillsT.AffiliateStatusID = BillAffiliateStatusHistoryT.StatusID "
				") "
				"LEFT JOIN "
				"( "
					"PatientsT "
					"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				") ON BillsT.PatientID = PatientsT.PersonID "
			") "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"LEFT JOIN PersonT PatCoord ON ChargesT.PatCoordID = PatCoord.ID "
			"LEFT JOIN PersonT ReferringPhysPerson ON BillsT.RefPhyID = ReferringPhysPerson.ID "
			"LEFT JOIN PersonT AffiliatePhysPerson ON BillsT.AffiliatePhysID = AffiliatePhysPerson.ID "
			"WHERE "
			"( "
				"( "
					"( "
						"BillsT.EntryType "
					") = 1 "
				") "
				"AND "
				"( "
					"( "
						"BillsT.Deleted "
					") = 0 "
				") "
				"AND "
				"( "
					"( "
						"LineItemT.Deleted "
					") = 0 "
				") "
			") "
			"AND GCTypesT.ServiceID IS NULL");
		break;
	case 135:
		//Charges by Zip Code
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			(d.thompson 2014-03-18) - PLID 61340 - Added ICD-10 field
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
			"CASE WHEN Right(PersonT.Zip,1) = '-' THEN Left(PersonT.Zip, Len(PersonT.Zip)-1) ELSE PersonT.Zip END AS Zip,  "
			"LineItemT.Date AS TDate, BillsT.Date AS BDate, ChargesT.ID, ChargesT.ItemCode, LineItemT.Description,  "
			"ICD9T1.CodeNumber as ICD9Code1, ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
			"ChargesT.Quantity, dbo.GetChargeTotal(ChargesT.ID) AS Amount, ChargesT.BillID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, ProvidersT.PersonID AS ProvID,  "
			"LineItemT.Amount AS PracBillFee, CPTModifierT.Number AS Modifier, CPTModifierT2.Number AS Modifier2,  "
			"CategoriesT.ID AS CatID, CategoriesT.Name AS CatName, PersonT.City, PersonT.State, BillsT.EntryType,  "
			"LineItemT.InputDate AS IDate, LineItemT.LocationID AS LocID, LocationsT.Name AS Location  "
			"FROM  "
			"	( "
			"		( "
			"			( "
			"				( "
			"					( "
			"						(ChargesT LEFT JOIN  "
			"							(LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID)  "
			"						ON ChargesT.ID = LineItemT.ID "
			"						)  "
			"					LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
			"					LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"					)  "
			"				LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number  "
			"				LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"				)  "
			"			LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			"			)  "
			"		LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
			"		)  "
			"	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID  "
			"	LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			"	LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"	LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"	)  "
			"LEFT JOIN  "
			"	(PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)  "
			"ON BillsT.PatientID = PatientsT.PersonID  "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID  "
			"WHERE BillsT.EntryType=1 AND BillsT.Deleted=0 AND LineItemT.Deleted=0 AND GCTypesT.ServiceID IS NULL ");
		break;


	case 158:
		//Charges By Service Code By Patient
		/*	Version history
			DRT - 6/18/03 - Added Ins3Resp field, which is all non-pri/sec/pat insurance amounts.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			JMJ - 10/31/2005 - PLID 17899 - added support for TOS, RVU, and Global Period days
			(j.jones 2011-04-04 11:56) - PLID 39521 - added VisionPriResp and VisionSecResp,
			and renamed Ins3Resp to InsOtherResp, but did not change its calculation
		*/
		return _T("SELECT SubQ.UserDefinedID, SubQ.PatID AS PatID, "
		"SubQ.BirthDate, SubQ.PatName, SubQ.ProvID AS ProvID, SubQ.ProvName, SubQ.CPTCode, "
		"SubQ.ItemSubCode, SubQ.Description, SubQ.GlobalPeriod, SubQ.RVU, SubQ.TypeOfService, "
		"SubQ.Total, SubQ.Ins1Resp, SubQ.Ins2Resp, SubQ.InsOtherResp, SubQ.VisionPriResp, SubQ.VisionSecResp, "
		"SubQ.TDate AS TDate, SubQ.IDate AS IDate, SubQ.ID, SubQ.CPTID AS CPTID, SubQ.LocID AS LocID, "
		"SubQ.Location, SubQ.Quantity FROM "
		"(SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.BirthDate, "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     ChargesT.DoctorsProviders AS ProvID,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
		"     ChargesT.ItemCode AS CPTCode, ChargesT.ItemSubCode,  "
		"    LineItemT.Description, CPTCodeT.GlobalPeriod, CPTCodeT.RVU, CPTCodeT.TypeOfService, "
		"    Total = SUM(CASE WHEN ChargeRespT.Amount IS NULL then 0 else ChargeRespT.Amount END), "
		"    Ins1Resp = SUM(Case WHEN InsuredPartyT.RespTypeID = 1 then ChargeRespT.Amount Else 0 END), "
		"    Ins2Resp = SUM(CASE WHEN InsuredPartyT.RespTypeID = 2 then ChargeRespT.Amount Else 0 END), "
		"	 InsOtherResp = SUM(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN ChargeRespT.Amount ELSE 0 END), "
		"	 VisionPriResp = SUM(CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN ChargeRespT.Amount ELSE 0 END), "
		"	 VisionSecResp = SUM(CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN ChargeRespT.Amount ELSE 0 END), "
		"    LineItemT.Date AS TDate, LineItemT.InputDate AS IDate,  "
		"    ChargesT.ID, ServiceT.ID AS CPTID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, ChargesT.Quantity "
		"FROM PatientsT INNER JOIN "
		"    BillsT ON PatientsT.PersonID = BillsT.PatientID INNER JOIN "
		"    PersonT ON  "
		"    PatientsT.PersonID = PersonT.ID RIGHT OUTER JOIN "
		"    (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN "
		"    ChargesT ON LineItemT.ID = ChargesT.ID LEFT OUTER JOIN "
		"    InsuredPartyT RIGHT OUTER JOIN "
		"    ChargeRespT ON  "
		"    InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID ON  "
		"    ChargesT.ID = ChargeRespT.ChargeID ON  "
		"    BillsT.ID = ChargesT.BillID LEFT OUTER JOIN "
		"    PersonT PersonT1 ON  "
		"    ChargesT.DoctorsProviders = PersonT1.ID LEFT OUTER JOIN "
		"    CPTCodeT INNER JOIN "
		"    ServiceT ON CPTCodeT.ID = ServiceT.ID ON  "
		"    ChargesT.ServiceID = ServiceT.ID LEFT OUTER JOIN "
		"    CPTModifierT ON  "
		"    ChargesT.CPTModifier = CPTModifierT.Number "
		"	 LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"	 LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE (LineItemT.Deleted = 0) AND (BillsT.EntryType = 1) AND  "
		"    (BillsT.Deleted = 0) AND CPTCodeT.ID Is Not NULL AND GCTypesT.ServiceID IS NULL "
		"GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID,  "
		"    PersonT.BirthDate, "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
		"    ChargesT.DoctorsProviders,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
		"    ChargesT.ItemCode, ChargesT.ItemSubCode,  "
		"    LineItemT.Description, CPTCodeT.GlobalPeriod, CPTCodeT.RVU, CPTCodeT.TypeOfService, ServiceT.ID, "
		"    LineItemT.Date, LineItemT.InputDate, ChargesT.ID, LineItemT.LocationID, LocationsT.Name, ChargesT.Quantity) AS SubQ");
		break;
	case 160:
		//Charges By Service Date
	case 363:
		//Charges By Input Date
	case 431:
		//Charges By Bill Date
		/*	Version History
			DRT 8/5/03 - Added charges by bill date report.  The other 2 are too horribly mangled to even 
				figure out what they are doing, so my hopes and dreams of making this an easy editable fix
				are dashed.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			// (j.gruber 2007-03-19 10:25) - PLID 25198 - fix the diagnosis codes to show the charges diagnosis codes
			// (j.jones 2010-10-26 16:42) - PLID 41003 - added Charge Input Name as an available field
			// (j.gruber 2014-03-28 10:28) - PLID 61341 - updated for ICD-10
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
		"PersonT.Zip,  "
		"LineItemT.Date AS TDate,  "
		"BillsT.Date AS BDate,  "
		"ChargesT.ID,  "
		"ChargesT.ItemCode,  "
		"LineItemT.Description,  "

		"ICD9T1.CodeNumber as ICD9Code1, \r\n "
		"ICD9T2.CodeNumber as ICD9Code2, \r\n "
		"ICD9T3.CodeNumber as ICD9Code3, \r\n "
		"ICD9T4.CodeNumber as ICD9Code4, \r\n "

		"ICD10T1.CodeNumber as ICD10Code1, \r\n "
		"ICD10T2.CodeNumber as ICD10Code2, \r\n "
		"ICD10T3.CodeNumber as ICD10Code3, \r\n "
		"ICD10T4.CodeNumber as ICD10Code4, \r\n "

		"ChargeWhichCodesListV.WhichCodes9, \r\n"
		"ChargeWhichCodesListV.WhichCodes10, \r\n"
		"ChargeWhichCodesListV.WhichCodesBoth, \r\n"
		
		"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
		"ChargesT.Quantity,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
		"ChargesT.BillID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"ProvidersT.PersonID AS ProvID,  "
		"LineItemT.Amount AS PracBillFee,  "
		"CPTModifierT.Number AS Modifier,  "
		"CPTModifierT2.Number AS Modifier2, "
		"CategoriesT.ID AS CatID,  "
		"CategoriesT.Name AS CatName,  "
		"PersonT.City,  "
		"PersonT.State,  "
		"BillsT.EntryType,  "
		"LineItemT.InputDate AS IDate, "
		"PatientsT.UserDefinedID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, "
		"LineItemT.InputName AS InputName, "
		"InsCo.name AS InsuranceCompanyName "//j.camacho (7/3/2014) - PLID 62719
		"FROM ChargesT LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
		"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
		"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
		"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
		"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
		"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"LEFT JOIN InsuredPartyT InsParty ON InsParty.PersonID=BillsT.InsuredPartyID \r\n" //j.camacho (7/3/2014) - PLID 62719
		"LEFT JOIN InsuranceCoT InsCo ON InsCo.PersonID=InsParty.InsuranceCoID \r\n"//j.camacho (7/3/2014) - PLID 62719

		
		"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
		"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
		"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
		"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
		"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
		"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
		"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
		"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
		"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "

		"LEFT JOIN ChargeWhichCodesListV ON ChargesT.ID = ChargeWhichCodesListV.ChargeID \r\n "

		"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) "
		"ON BillsT.PatientID = PatientsT.PersonID "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"WHERE (((BillsT.EntryType)=1) AND ((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) AND GCTypesT.ServiceID IS NULL ");
		break;
	case 159:
		//Charges By Service Code By Provider
		/*	Version history
			DRT - 6/18/03 - Added Ins3Resp field, which is all non-pri/sec/pat insurance amounts.
			DRT 7/30/03 - Made report editable (PLID 4184)
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold. (inner join with cptcodet)
			JMJ - 10/31/2005 - PLID 17899 - added support for TOS, RVU, and Global Period days
			(j.jones 2011-04-04 11:56) - PLID 39521 - added VisionPriResp and VisionSecResp,
			and renamed Ins3Resp to InsOtherResp, but did not change its calculation
			// (a.wilson 2012-02-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
		*/
		return _T("SELECT SubQ.UserDefinedID, SubQ.PatID AS PatID, "
		"SubQ.PatName, SubQ.ProvID AS ProvID, SubQ.ProvName, SubQ.CPTCode, "
		"SubQ.ItemSubCode, SubQ.Description, SubQ.GlobalPeriod, SubQ.RVU, SubQ.TypeOfService, "
		"SubQ.Total, SubQ.Ins1Resp, SubQ.Ins2Resp, SubQ.InsOtherResp, SubQ.VisionPriResp, SubQ.VisionSecResp, "
		"SubQ.TDate AS TDate, SubQ.IDate AS IDate, SubQ.ID, SubQ.CPTID AS CPTID, SubQ.LocID AS LocID, "
		"SubQ.Location, SubQ.Quantity FROM "
		"(SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     ChargesT.DoctorsProviders AS ProvID,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
		"     ChargesT.ItemCode AS CPTCode, ChargesT.ItemSubCode,  "
		"    LineItemT.Description, CPTCodeT.GlobalPeriod, CPTCodeT.RVU, CPTCodeT.TypeOfService, "
		"    Total = SUM(CASE WHEN ChargeRespT.Amount IS NULL then 0 else ChargeRespT.Amount END), "
		"    Ins1Resp = SUM(CASE WHEN InsuredPartyT.RespTypeID = 1 then ChargeRespT.Amount Else 0 END), "
		"    Ins2Resp = SUM(CASE WHEN InsuredPartyT.RespTypeID = 2 then ChargeRespT.Amount Else 0 END), "
		"	 InsOtherResp = SUM(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN ChargeRespT.Amount ELSE 0 END), "
		"	 VisionPriResp = SUM(CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN ChargeRespT.Amount ELSE 0 END), "
		"	 VisionSecResp = SUM(CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN ChargeRespT.Amount ELSE 0 END), "
		"    LineItemT.Date AS TDate, LineItemT.InputDate AS IDate,  "
		"    ChargesT.ID, ServiceT.ID AS CPTID, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, ChargesT.Quantity "
		"FROM PatientsT INNER JOIN "
		"    BillsT ON PatientsT.PersonID = BillsT.PatientID INNER JOIN "
		"    PersonT ON  "
		"    PatientsT.PersonID = PersonT.ID RIGHT OUTER JOIN "
		"    (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN "
		"    ChargesT ON LineItemT.ID = ChargesT.ID LEFT OUTER JOIN "
		"    InsuredPartyT RIGHT OUTER JOIN "
		"    ChargeRespT ON  "
		"    InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID ON  "
		"    ChargesT.ID = ChargeRespT.ChargeID ON  "
		"    BillsT.ID = ChargesT.BillID LEFT OUTER JOIN "
		"    PersonT PersonT1 ON  "
		"    ChargesT.DoctorsProviders = PersonT1.ID LEFT OUTER JOIN "
		"    CPTCodeT INNER JOIN "
		"    ServiceT ON CPTCodeT.ID = ServiceT.ID ON  "
		"    ChargesT.ServiceID = ServiceT.ID LEFT OUTER JOIN "
		"    CPTModifierT ON  "
		"    ChargesT.CPTModifier = CPTModifierT.Number "
		"	 LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"	 LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE (LineItemT.Deleted = 0) AND (BillsT.EntryType = 1) AND  "
		"    (BillsT.Deleted = 0) AND CPTCodeT.ID Is Not NULL AND GCTypesT.ServiceID IS NULL "
		"GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
		"    ChargesT.DoctorsProviders,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
		"    ChargesT.ItemCode, ChargesT.ItemSubCode,  "
		"    LineItemT.Description, CPTCodeT.GlobalPeriod, CPTCodeT.RVU, CPTCodeT.TypeOfService, ServiceT.ID, "
		"    LineItemT.Date, LineItemT.InputDate, ChargesT.ID, LineItemT.LocationID, LocationsT.Name, ChargesT.Quantity) AS SubQ");
		break;
	case 161:
		//Charges By Insurance Company
		/*	Version History
			DRT 6/18/03 - Changed the "resptypeid" field (which is still poorly named) to use the RespTypeT table, instead of the usual 1 = pri, 2 = sec, etc.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			// (j.gruber 2009-11-09 09:41) - PLID 36203 add a bunch of fields to the charges by insurance report
			//(e.lally 2011-11-08) PLID 45541 - Added financial class
			// (j.jones 2013-05-08 11:49) - PLID 55066 - added Calls, Alberta Claim Number, and Referring Physician NPI
		*/
		return _T("SELECT ChargeID AS ChargeID, InputDate, PatID AS PatID, PatName as PatName, ChargeAmount AS ChargeAmount, "
			" InsResp As InsResp, Applies As Applies, AppliedPays as AppliedPays, AppliedAdj AS AppliedAdj, "
			" AppliedRef As AppliedRef, InsCoID As InsCoID, InsCoName AS InsCoName, TDate AS TDate, Description, ProvID AS ProvID, "
			" ProvName AS ProvName, ItemCode, ItemSubCode, RespTypeID AS RespTypeID, IDate AS IDate, UserDefinedID, LocID AS LocID, "
			" Location, Balance AS Balance, "
			" BillID, ProviderNPI, "
			" RefPhysFirst, RefPhysMiddle,  RefPhysLast, RefPhysTitle, RefPhysNPI, "
			" InsAdd1, InsAdd2, InsCoCity, InsCoState, InsCoZip, "
			" IDForInsurance, PosName, "
			" InsRefAuthNum, InsRefStartDate, InsRefEndDate, "
			" InsRefNumVisits, InsRefComments, Birthdate, "
			" Address1, Address2, City, State, Zip, PriInsIDForInsurance, "
			" FinancialClassID, FinancialClass, "
			" Calls, AlbertaClaimNumber, BillReferringPhysicianNPI "
			"\r\n"
			" FROM "
			" (SELECT ChargesT.ID AS ChargeID, "
			"LineItemT.InputDate, "
			"PatientsT.PersonID AS PatID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
			"/*Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) AS PatResp, */"
			"Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null THEN ChargeRespT.Amount ELSE 0 End) AS InsResp, "
			"Sum((CASE WHEN [AppliesQ].[PayAmount] Is Null THEN 0 ELSE AppliesQ.PayAmount End)+(CASE WHEN [AppliesQ].[AdjAmount] Is Null THEN 0 ELSE AppliesQ.AdjAmount End)) AS Applies, "
			"Sum((CASE WHEN [AppliesQ].[PayAmount] Is Null THEN 0 ELSE AppliesQ.PayAmount End)) AS AppliedPays, "
			"Sum((CASE WHEN [AppliesQ].[AdjAmount] Is Null THEN 0 ELSE AppliesQ.AdjAmount End)) AS AppliedAdj, "
			"0 AS AppliedRef, "
			"InsuranceCoT.PersonID AS InsCoID, "
			"InsuranceCoT.Name AS InsCoName, "
			"LineItemT.Date AS TDate, "
			"LineItemT.Description, "
			"ProvidersT.PersonID AS ProvID, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
			"ChargesT.ItemCode, "
			"ChargesT.ItemSubCode, "
			"RespTypeT.TypeName AS RespTypeID, "
			"LineItemT.InputDate AS IDate, "
			"PatientsT.UserDefinedID, "
			"LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  "
			"(Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null THEN ChargeRespT.Amount ELSE 0 End) - Sum((CASE WHEN [AppliesQ].[PayAmount] Is Null THEN 0 ELSE AppliesQ.PayAmount End)+(CASE WHEN [AppliesQ].[AdjAmount] Is Null THEN 0 ELSE AppliesQ.AdjAmount End))) AS Balance, " 
			" BillsT.ID as BillID, ProvidersT.NPI as ProviderNPI, "
			" RefPhysPersonT.First as RefPhysFirst, RefPhysPersonT.Middle as RefPhysMiddle,  RefPhysPersonT.Last as RefPhysLast, RefPhysPersonT.Title as RefPhysTitle, ReferringPhysT.NPI as RefPhysNPI, "
			" InsCoPersonT.Address1 as InsAdd1, InsCoPersonT.Address2 as InsAdd2, InsCoPersonT.City as InsCoCity, InsCoPersonT.State as InsCoState, InsCoPersonT.Zip as InsCoZip, "
			" InsuredPartyT.IDForInsurance, POST.Name as PosName, "
			" InsuranceReferralsT.AuthNum as InsRefAuthNum, InsuranceReferralsT.StartDate as InsRefStartDate, InsuranceReferralsT.EndDate as InsRefEndDate, "
			" InsuranceReferralsT.NumVisits as InsRefNumVisits, InsuranceReferralsT.Comments as InsRefComments, PersonT.Birthdate, "
			" PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
			" (SELECT IDForInsurance FROM InsuredPartyT WHERE RespTypeID = 1 AND PatientID = PatientsT.PersonID) as PriInsIDForInsurance, "
			" InsuranceCoT.FinancialClassID, FinancialClassT.Name AS FinancialClass, "
			" ChargesT.Calls, AlbertaTransNumQ.AlbertaTransNum AS AlbertaClaimNumber, BillReferringPhysT.NPI AS BillReferringPhysicianNPI "
			"\r\n"
			"FROM (((((((ChargesT LEFT JOIN ((ChargeRespT LEFT JOIN (SELECT AppliesT.RespID, Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END ) AS PayAmount, Sum(CASE WHEN LineItemT.Type = 2 THEN Appliest.Amount ELSE 0 END) AS AdjAmount FROM AppliesT LEFT JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID GROUP BY AppliesT.RespID) AS AppliesQ ON ChargeRespT.ID = AppliesQ.RespID) LEFT JOIN (InsuredPartyT INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID) ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID) "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number) "
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID) "
			"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) "
			"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID) "
			"LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID) "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"LEFT JOIN InsuranceReferralsT ON BillsT.InsuranceReferralID = InsuranceReferralsT.ID "
			"LEFT JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "
			"LEFT JOIN PersonT RefPhysPersonT ON ReferringPhysT.PersonID = RefPhysPersonT.ID "
			"LEFT JOIN LocationsT POST ON BillsT.Location = POST.ID "
			"LEFT JOIN PersonT InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID "
			"LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID "
			"LEFT JOIN ReferringPhysT BillReferringPhysT ON BillsT.RefPhyID = BillReferringPhysT.PersonID "
			"LEFT JOIN (SELECT Max(ID) AS NewestID, ChargeID FROM ClaimHistoryDetailsT "
			"	GROUP BY ChargeID) AS MostRecentClaimHistoryDetailQ ON ChargesT.ID = MostRecentClaimHistoryDetailQ.ChargeID "
			"LEFT JOIN (SELECT ID, AlbertaTransNum FROM ClaimHistoryDetailsT) AS AlbertaTransNumQ ON MostRecentClaimHistoryDetailQ.NewestID = AlbertaTransNumQ.ID "
			"WHERE ((BillsT.Deleted)=0) AND ((BillsT.EntryType)=1) AND ((LineItemT.Deleted)=0) AND GCTypesT.ServiceID IS NULL "
			"\r\n"
			"GROUP BY "
			"ChargesT.ID, LineItemT.InputDate, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
			"InsuranceCoT.PersonID, InsuranceCoT.Name, LineItemT.Date, LineItemT.Description, ProvidersT.PersonID, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, ChargesT.ItemCode, ChargesT.ItemSubCode, LineItemT.InputDate, PatientsT.UserDefinedID, LineItemT.LocationID, LocationsT.Name, RespTypeT.TypeName, "
			" BillsT.ID, ProvidersT.NPI, "
			" RefPhysPersonT.First, RefPhysPersonT.Middle,  RefPhysPersonT.Last, RefPhysPersonT.Title, ReferringPhysT.NPI, "
			" InsCoPersonT.Address1, InsCoPersonT.Address2, InsCoPersonT.City, InsCoPersonT.State, InsCoPersonT.Zip, "
			" InsuredPartyT.IDForInsurance, POST.Name, "
			" InsuranceReferralsT.AuthNum, InsuranceReferralsT.StartDate, InsuranceReferralsT.EndDate, "
			" InsuranceReferralsT.NumVisits, InsuranceReferralsT.Comments, PersonT.Birthdate,  "
			" PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
			" InsuranceCoT.FinancialClassID, FinancialClassT.Name, "
			" ChargesT.Calls, AlbertaTransNumQ.AlbertaTransNum, BillReferringPhysT.NPI "
			"\r\n"
			"HAVING ((Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null THEN ChargeRespT.Amount End)<>0))) SubQ");
		break;
	case 162:
		//Charges By Patient By Service Code
		/*	Version History
			DRT 6/18/03 - Rewrote the query, it was just wrong before.  It is supposed to show 1 record for every charge, and the amounts split
					for that.  It was showing 1 record for every responsibility, and it made the report a lot harder to read than it should
					have been.  Also added the Ins3Resp field for insurance that is non-pri/sec/pat.
			JMM 9/03/2003 - added primary referral source
			DRT 10/1/03 - Fixed a bug where it was displaying the provider name from G1 but filtering on the charge provider.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold. (inner join with cptcodet)
			(j.jones 2011-04-04 11:56) - PLID 39521 - added VisionPriResp and VisionSecResp,
			and renamed Ins3Resp to InsOtherResp, but did not change its calculation
			// (a.wilson 2012-02-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
		*/
		return _T("SELECT UserDefinedID, PatID AS PatID, PatName, ProvID AS ProvID,  "
			"ProvName, CPTCode, ItemSubCode, Description, Total, Ins1Resp, Ins2Resp, InsOtherResp, SubQ.VisionPriResp, SubQ.VisionSecResp, "
			"TDate AS TDate, IDate AS IDate, ServiceID AS ServiceID, LocID AS LocID, Location, Name AS ReferralSource, RefFirst, RefMiddle, RefLast  "
			"FROM	 "
			"	(SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"	ChargesT.DoctorsProviders AS ProvID,   "
			"	PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
			"	CPTCodeT.Code AS CPTCode, CPTCodeT.SubCode AS ItemSubCode,   "
			"	LineItemT.Description,   "
			"	Total = Sum(CASE WHEN (ChargeRespT.Amount) IS NULL THEN 0 ELSE (ChargeRespT.Amount) END),  "
			"	Ins1Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID = 1 then (ChargeRespT.Amount) Else 0 END),  "
			"	Ins2Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID = 2 then (ChargeRespT.Amount) Else 0 END),  "
			"	InsOtherResp = Sum(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN (ChargeRespT.Amount) Else 0 END),  "
			"	VisionPriResp = SUM(CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN ChargeRespT.Amount ELSE 0 END), "
			"	VisionSecResp = SUM(CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN ChargeRespT.Amount ELSE 0 END), "
			"	LineItemT.Date AS TDate, LineItemT.InputDate AS IDate,   "
			"	ServiceT.ID AS ServiceID,  "
			"	LineItemT.LocationID AS LocID,  "
			"	LocationsT.Name AS Location, ReferralSourceT.Name,PersonRefPhys.First as RefFirst, PersonRefPhys.Middle as RefMiddle, PersonRefPhys.Last AS RefLast  "
			"	FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"	INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID "
			"	LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"	LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID "
			"	LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"	INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"	LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredpartyID = InsuredPartyT.PersonID "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"   LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"   LEFT JOIN PersonT PersonRefPhys ON PatientsT.DefaultReferringPhyID = PersonRefPhys.ID "
			"	LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"	LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"	WHERE LineItemT.Deleted = 0 AND BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND GCTypesT.ServiceID IS NULL "
			"	GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
			"	ChargesT.DoctorsProviders, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, CPTCodeT.Code,  "
			"	CPTCodeT.SubCode, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, ChargesT.ID, ServiceT.ID,  "
			"	LineItemT.LocationID, LocationsT.Name, ReferralSourceT.Name, PersonRefPhys.First, PersonRefPhys.Middle, PersonRefPhys.Last "
			"	) SubQ");
		break;
	

case 194:
		//Deleted Bills
		// (j.gruber 2008-09-08 12:43) - PLID 27308 - added provider info and filter
		// (j.gruber 2009-03-25 10:47) - PLID 33691 - change discount structure - allow deleted discounts since this is the deleted bills report
		//this might not be completely accurate if they had some deleted discounts on the bill prior to the deletion of the bill, however it would work the 
		//same for charges also, so we decided to leave it be
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		// (r.goldschmidt 2016-02-08 13:01) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
		return _T("SELECT BillsT.ID AS BillID, BillsT.Description AS BillDesc,   "
			"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
			"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"     BillsT.DeleteDate AS TDate, BillsT.Date AS BillDate,   "
			"    Sum(ChargeRespT.Amount) AS Total, "
			"    UsersT.UserName AS DeletedBy,  "
			"BillsT.Location AS LocID, LocationsT.Name AS Location, ChargesT.DoctorsProviders as ProvID,  "
			"DocPersonT.First as DocFirst, DocPersonT.Last as DocLast, DocPersonT.Title as DocTitle " 
			"FROM (BillsT LEFT JOIN LocationsT ON BillsT.Location = LocationsT.ID) INNER JOIN  "
			"    (ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID) ON BillsT.ID = ChargesT.BillID INNER JOIN  "
			"    PatientsT ON   "
			"    BillsT.PatientID = PatientsT.PersonID INNER JOIN  "
			"    PersonT ON PatientsT.PersonID = PersonT.ID INNER JOIN  "
			"    UsersT ON BillsT.DeletedBy = UsersT.PersonID "
			"	 LEFT JOIN PersonT DocPersonT ON ChargesT.DoctorsProviders = DocPersonT.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"WHERE BillsT.Deleted = 1 AND BillsT.EntryType = 1 "
			"GROUP BY BillsT.ID, BillsT.Description, PatientsT.UserDefinedID, PatientsT.PersonID,   "
			"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,   "
			"    BillsT.DeleteDate, BillsT.Date, PersonT.Last,   "
			"    UsersT.UserName, BillsT.Location, LocationsT.Name, "
			"    ChargesT.DoctorsProviders, DocPersonT.First, DocPersonT.Last, DocPersonT.Title ");
		break;
	case 195:
		//Deleted Charges
		// (j.gruber 2008-09-08 12:43) - PLID 27308 - added provider info and filter
		// (j.gruber 2009-03-25 11:19) - PLID 33691 - changed discount structure - allow deleted discounts since this is the deleted bills report
		//this might not be completely accurate if they had some deleted discounts on the bill prior to the deletion of the bill
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		// (r.goldschmidt 2016-02-08 13:02) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
		return _T("SELECT ChargesT.ID AS ChargeID, ChargesT.BillID,  "
		"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     BillsT.Note AS BillDesc,  "
		"    LineItemT.Description AS ChargeDesc, SUM(ChargeRespT.Amount) AS Amount,  "
		"    LineItemT.DeleteDate AS TDate, LineItemT.DeletedBy,  "
		"    BillsT.Date AS BillDate, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, ChargesT.DoctorsProviders as ProvID, "
		"DocPersonT.First as DocFirst, DocPersonT.Last as DocLast, DocPersonT.Title as DocTitle " 
		"FROM BillsT INNER JOIN "
		"    ChargesT INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID ON  "
		"    BillsT.ID = ChargesT.BillID INNER JOIN "
		"    PersonT INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    LineItemT.PatientID = PatientsT.PersonID "
		"	 LEFT JOIN PersonT DocPersonT ON ChargesT.DoctorsProviders = DocPersonT.ID "
		"	 LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
		"WHERE (LineItemT.Deleted = 1) AND (LineItemT.Type = 10) "
		"GROUP BY ChargesT.ID, ChargesT.BillID, PatientsT.UserDefinedID, "
		"PatientsT.PersonID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"    BillsT.Note, LineItemT.Description, "
		"    LineItemT.DeleteDate, LineItemT.DeletedBy, BillsT.Date, LineItemT.LocationID, LocationsT.Name, "
		"	 ChargesT.DoctorsProviders, DocPersonT.First, DocPersonT.Last, DocPersonT.Title");
		break;
	

	case 218:
		//Charges By Inv Item By Patient
		/*	Version History
			DRT 6/19/03 - Borrowed query from Charges by Patient by Inv Item.  See comments there (220) for why.
			DRT 10/1/03 - Fixed a bug where it was displaying the provider name from G1 but filtering on the charge provider.
			MSC 5/13/04 - Joined ProductT so that we can filter on the suppier
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			// (a.wilson 2012-02-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
		*/
		return _T("SELECT UserDefinedID, PatID AS PatID, PatName, ProvID AS ProvID, "
			"ProvName, Description, Total, Ins1Resp, Ins2Resp, Ins3Resp, "
			"TDate AS TDate, IDate AS IDate, ServiceID AS ServiceID, LocID AS LocID, Location, Quantity, MultiSupplierT.SupplierID AS SupplierID "
			"FROM "
			"	(SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"	ChargesT.DoctorsProviders AS ProvID, "
			"	PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
			"	LineItemT.Description,  "
			"	Total = Sum(CASE WHEN (ChargeRespT.Amount) IS NULL THEN 0 ELSE (ChargeRespT.Amount) END), "
			"	Ins1Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID = 1 then (ChargeRespT.Amount) Else 0 END), "
			"	Ins2Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID = 2 then (ChargeRespT.Amount) Else 0 END), "
			"	Ins3Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN (ChargeRespT.Amount) Else 0 END), "
			"	LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
			"	ServiceT.ID AS ServiceID, "
			"	LineItemT.LocationID AS LocID, "
			"	LocationsT.Name AS Location, ChargesT.Quantity "
			"	FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			"	INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID  "
			"	LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID  "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			"	LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID  "
			"	LEFT JOIN ProductT ON ChargesT.ServiceID = ProductT.ID  "
			"	LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
			"	LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredpartyID = InsuredPartyT.PersonID  "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
			"	LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"	WHERE LineItemT.Deleted = 0 AND BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND GCTypesT.ServiceID IS NULL "
			"	GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"	ChargesT.DoctorsProviders, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
			"	LineItemT.Description, LineItemT.Date, LineItemT.InputDate, ChargesT.ID, ServiceT.ID,   "
			"	LineItemT.LocationID, LocationsT.Name, ChargesT.Quantity  "
			"	) SubQ "
			"LEFT JOIN ProductT on ProductT.ID = SubQ.ServiceID "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID ");
		break;

	case 219:
		//Charges By Inv Item By Provider
		/*	Version History
			DRT 6/19/03 - Borrowed query from Charges by Patient by Inv Item.  See comments there (220) for why.
			DRT 10/1/03 - Fixed a bug where it was displaying the provider name from G1 but filtering on the charge provider.
			MSC 5/13/04 - Joined ProductT so that we can filter on the suppier
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			JMM 8/3/2006 - PLID 21170 - Added BillID to the report
			// (a.wilson 2012-02-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
		*/
		return _T("SELECT UserDefinedID, PatID AS PatID, PatName, ProvID AS ProvID,   "
			"ProvName, Description, Total, Ins1Resp, Ins2Resp, Ins3Resp,  "
			"TDate AS TDate, IDate AS IDate, ServiceID AS ServiceID, LocID AS LocID, Location, Quantity, MultiSupplierT.SupplierID AS SupplierID, BillID "
			"FROM	  "
			"	(SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,    "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,   "
			"	ChargesT.DoctorsProviders AS ProvID,    "
			"	PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,   "
			"	LineItemT.Description,    "
			"	Total = Sum(CASE WHEN (ChargeRespT.Amount) IS NULL THEN 0 ELSE (ChargeRespT.Amount) END),   "
			"	Ins1Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID = 1 then (ChargeRespT.Amount) Else 0 END),   "
			"	Ins2Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID = 2 then (ChargeRespT.Amount) Else 0 END),   "
			"	Ins3Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN (ChargeRespT.Amount) Else 0 END),   "
			"	LineItemT.Date AS TDate, LineItemT.InputDate AS IDate,    "
			"	ServiceT.ID AS ServiceID,   "
			"	LineItemT.LocationID AS LocID,   "
			"	LocationsT.Name AS Location, ChargesT.Quantity, ChargesT.BillID  "
			"	FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			"	INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID  "
			"	LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID  "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			"	LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID  "
			"	LEFT JOIN ProductT ON ChargesT.ServiceID = ProductT.ID  "
			"	LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
			"	LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredpartyID = InsuredPartyT.PersonID  "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
			"	LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"	WHERE LineItemT.Deleted = 0 AND BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND GCTypesT.ServiceID IS NULL "
			"	GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"	ChargesT.DoctorsProviders, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
			"	LineItemT.Description, LineItemT.Date, LineItemT.InputDate, ChargesT.ID, ServiceT.ID,   "
			"	LineItemT.LocationID, LocationsT.Name, ChargesT.Quantity, ChargesT.BillID  "
			"	) SubQ "
			"	LEFT JOIN ProductT on ProductT.ID = SubQ.ServiceID "
			"	LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID ");
		break;

	case 220:
		//Charges By Patient By Inv Item
		/*	Version History
			DRT 6/19/03 - Rewrote query, the old one sucked.  See comments from yesterday for Charges by Patient by Service Code, 
					I did the same thing here (modified that query, actually)
			DRT 10/1/03 - Fixed a bug where it was displaying the provider name from G1 but filtering on the charge provider.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			// (a.wilson 2012-02-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
		*/
		return _T("SELECT UserDefinedID, PatID AS PatID, PatName, ProvID AS ProvID,   "
			"ProvName, Description, Total, Ins1Resp, Ins2Resp, Ins3Resp,  "
			"TDate AS TDate, IDate AS IDate, ServiceID AS ServiceID, LocID AS LocID, Location   "
			"FROM	  "
			"	(SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,    "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,   "
			"	ChargesT.DoctorsProviders AS ProvID,    "
			"	PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,   "
			"	LineItemT.Description,    "
			"	Total = Sum(CASE WHEN (ChargeRespT.Amount) IS NULL THEN 0 ELSE (ChargeRespT.Amount) END),   "
			"	Ins1Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID = 1 then (ChargeRespT.Amount) Else 0 END),   "
			"	Ins2Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID = 2 then (ChargeRespT.Amount) Else 0 END),   "
			"	Ins3Resp = Sum(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN (ChargeRespT.Amount) Else 0 END),   "
			"	LineItemT.Date AS TDate, LineItemT.InputDate AS IDate,    "
			"	ServiceT.ID AS ServiceID,   "
			"	LineItemT.LocationID AS LocID,   "
			"	LocationsT.Name AS Location   "
			"	FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			"	INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID  "
			"	LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID  "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			"	LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID  "
			"	LEFT JOIN ProductT ON ChargesT.ServiceID = ProductT.ID  "
			"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
			"	LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredpartyID = InsuredPartyT.PersonID  "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
			"	LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"	WHERE LineItemT.Deleted = 0 AND BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND GCTypesT.ServiceID IS NULL "
			"	GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
			"	ChargesT.DoctorsProviders, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,  "
			"	LineItemT.Description, LineItemT.Date, LineItemT.InputDate, ChargesT.ID, ServiceT.ID,   "
			"	LineItemT.LocationID, LocationsT.Name  "
			"	) SubQ ");
		break;	
	

	case 225:
		//Diagnosis Codes By Provider
		/*	Version History
		// (j.jones 2009-03-27 09:01) - PLID 33714 - supported infinite diagnosis codes on a bill
		// (j.jones 2011-12-05 09:21) - PLID 44878 - hid original/void line items
		// (d.thompson 2014-03-14) - PLID 61342 - Updated for ICD-10.  This used to be a giant UNION of code1 U code2 U ... U extra codes.  Now that 
		//	all codes are external, we dropped the unions and just pull from the codes.
		*/
		return _T("SELECT _DiagCodesByProvQ.PatID AS PatID, _DiagCodesByProvQ.UserDefinedID, _DiagCodesByProvQ.ICD9Code, "
		"_DiagCodesByProvQ.ICD10Code, _DiagCodesByProvQ.TDate AS TDate, "
		"_DiagCodesByProvQ.Description, _DiagCodesByProvQ.IDate AS IDate, _DiagCodesByProvQ.ProvID AS ProvID, _DiagCodesByProvQ.PatName, _DiagCodesByProvQ.DocName, "
		"_DiagCodesByProvQ.ItemCode, _DiagCodesByProvQ.ItemSubCode, _DiagCodesByProvQ.ICD9CodeDesc, _DiagCodesByProvQ.ICD10CodeDesc, _DiagCodesByProvQ.LocID AS LocID "
		"FROM ("
		"SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, ICD9T.CodeNumber as ICD9Code, ICD10T.CodeNumber as ICD10Code, "
		"LineItemT.Date AS TDate, "
		"LineItemT.Description, LineItemT.InputDate AS IDate, ChargesT.DoctorsProviders AS ProvID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"DoctorsT.Last + ', ' + Doctorst.First + ' ' + DoctorsT.Middle AS DocName, "
		"ChargesT.ItemCode, ChargesT.ItemSubCode, ICD9T.CodeDesc as ICD9CodeDesc, ICD10T.CodeDesc as ICD10CodeDesc, LineItemT.LocationID AS LocID "
		"FROM "
		"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID "
		"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
		"LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
		"LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n" 	
		"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
		"LEFT JOIN PersonT AS DoctorsT ON ChargesT.DoctorsProviders = DoctorsT.ID "
		"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID "
		"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
		"LEFT JOIN BillCorrectionsT OrigBillsT ON BillsT.ID = OrigBillsT.OriginalBillID "
		"WHERE "
		"(ICD9T.CodeNumber Is Not Null OR ICD10T.CodeNumber IS NOT NULL) AND "
		"(LineItemT.Deleted = 0) "
		"AND OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL "
		"AND OrigBillsT.OriginalBillID Is Null "
		") AS _DiagCodesByProvQ ");
		break;
	

case 237:
		//Quotes By Patient Coordinator
		// (j.gruber 2009-03-25 11:23) - PLID 33691 - change discount structure
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		// (r.goldschmidt 2016-02-08 13:05) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
		return _T("SELECT BillsT.ID BillID, ChargesT.ID ChargeID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
			"BillsT.Date AS TDate, BillsT.InputDate AS IDate, BillsT.Description,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS PatCoord, "
			"UsersT.PersonID AS PatCoordID, CASE WHEN (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) IS NULL THEN 0 ELSE (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END as PercentOff, LineItemT.Description AS ChargeDesc,  "
			"ChargesT.ItemCode, CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) IS NULL THEN Convert(money, 0) ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END as Discount, ChargesT.OthrBillFee, ChargesT.TaxRate, "
			"Sum(ChargeRespT.Amount) AS Total, LineItemT.LocationID AS LocID, "
			"ChargesT.DoctorsProviders AS ProvID "
			"FROM "
			"BillsT LEFT OUTER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID  "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT OUTER JOIN LocationsT ON BillsT.Location = LocationsT.ID "
			"LEFT OUTER JOIN UsersT ON BillsT.PatCoord = UsersT.PersonID "
			"LEFT OUTER JOIN PersonT PersonT1 ON UsersT.PersonID = PersonT1.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"GROUP BY BillsT.ID, ChargesT.ID, PatientsT.PersonID, PatientsT.UserDefinedID, BillsT.Date, BillsT.InputDate, BillsT.Description, PersonT.Location, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, UsersT.PersonID, LineItemT.Description,  "
			"Chargest.ItemCode, ChargesT.OthrBillFee, ChargesT.DoctorsProviders, Chargest.TaxRate, LineItemT.LocationID");
		break;
	case 241:
		//Charges by Place Of Service 
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			TES 5/28/2008 - PLID 28683 - Added CPTID, which is now the external filter (the POS filter got moved to the 
				extended filter).
			// (z.manning 2008-06-24 14:21) - PLID 29323 - Added charge patient coordinator ID and name
			// (r.gonet 03/26/2012) - PLID 49041 - Added Referring Physician and Affiliate Physician (and related fields)
			// (d.thompson 2014-03-18) - PLID 61343 - Added ICD-10 fields
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
		"PersonT.Zip,  "
		"LineItemT.Date AS TDate, "
		"BillsT.Date AS BDate,  "
		"ChargesT.ID,  "
		"ChargesT.ItemCode,  "
		"LineItemT.Description,  "
		"ICD9T1.CodeNumber as ICD9Code1, \r\n "
		"ICD10T1.CodeNumber as ICD10Code1, \r\n "
		"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
		"ChargesT.Quantity,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
		"ChargesT.BillID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"ProvidersT.PersonID AS ProvID,  "
		"LineItemT.Amount AS PracBillFee,  "
		"CPTModifierT.Number AS Modifier,  "
		"CPTModifierT2.Number AS Modifier2, "
		"CategoriesT.ID AS CatID,  "
		"CategoriesT.Name AS CatName,  "
		"BillsT.Location AS ServiceLocation, "
		"LocationsT1.Name AS POService, "
		"PersonT.City,  "
		"PersonT.State,  "
		"BillsT.EntryType, " 
		"BillsT.Deleted,  "
		"LineItemT.InputDate AS IDate, "
		"LineItemT.LocationID AS LocID, "
		"LocationsT.Name AS Location, "
		"ChargesT.ServiceID AS CPTID, "
		"ChargesT.PatCoordID, "
			"PatCoord.Last + ', ' + PatCoord.First + ' ' + PatCoord.Middle AS PatCoordName, "
			"CASE WHEN BillsT.RefPhyID IS NOT NULL AND BillsT.RefPhyID = -1 THEN NULL ELSE BillsT.RefPhyID END AS RefPhyID, "
			"ReferringPhysPerson.Last + ', ' + ReferringPhysPerson.First + ' ' + ReferringPhysPerson.Middle AS ReferringPhysName, "
			"BillsT.AffiliatePhysID, "
			"AffiliatePhysPerson.Last + ', ' + AffiliatePhysPerson.First + ' ' + AffiliatePhysPerson.Middle AS AffiliatePhysName, "
			"BillAffiliateStatusT.Name AS AffiliateStatus, "
			"BillAffiliateStatusHistoryT.Date AS AffiliatePhysDate, "
			"BillsT.AffiliatePhysAmount, "
			"BillsT.AffiliateNote "
			""
			"FROM "
			"( "
				"( "
					"( "
						"( "
							"( "
								"( "
									"( "
										"ChargesT "
										"LEFT JOIN "
										"( "
											"LineItemT "
											"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
										") ON ChargesT.ID = LineItemT.ID "
									") "
									"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
								") "
								"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
								"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
							") "
							"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
						") "
						"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
					") "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
					"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
					"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
					"LEFT JOIN BillAffiliateStatusT ON BillsT.AffiliateStatusID = BillAffiliateStatusT.ID "
					"LEFT JOIN BillAffiliateStatusHistoryT ON BillsT.ID = BillAffiliateStatusHistoryT.BillID "
						"AND BillsT.AffiliateStatusID = BillAffiliateStatusHistoryT.StatusID "
				") "
				"LEFT JOIN LocationsT AS LocationsT1 ON BillsT.Location = LocationsT1.ID "
				"LEFT JOIN "
				"( "
					"PatientsT "
					"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				") ON BillsT.PatientID = PatientsT.PersonID "
			") "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"LEFT JOIN PersonT PatCoord ON ChargesT.PatCoordID = PatCoord.ID "
			"LEFT JOIN PersonT ReferringPhysPerson ON BillsT.RefPhyID = ReferringPhysPerson.ID "
			"LEFT JOIN PersonT AffiliatePhysPerson ON BillsT.AffiliatePhysID = AffiliatePhysPerson.ID "
			"WHERE "
			"( "
				"( "
					"( "
						"BillsT.EntryType "
					") = 1 "
				") "
				"AND "
				"( "
					"( "
						"BillsT.Deleted "
					") = 0 "
				") "
				"AND "
				"( "
					"( "
						"LineItemT.Deleted "
					") = 0 "
				") "
			") "
			"AND GCTypesT.ServiceID IS NULL ");
		break;
	case 242:
		//Charges by Location
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
		*/
		return _T("SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, ChargesT.DoctorsProviders AS ProvID, LineItemT.Date AS TDate, LineItemT.InputDate AS IDate,  "
			"LineItemT.LocationID AS LocID, LocationsT.Name AS LocationName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, ChargesT.ItemCode, LineItemT.Amount AS UnitFee,  "
			"ChargesT.CPTModifier, LineItemT.Description, ChargesT.Quantity, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount  "
			"FROM "
			"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND GCTypesT.ServiceID IS NULL ");
		break;


		case 247:
			//Bills converted from Quotes
			/*	Version History
				DRT 8/27/2004 - PLID 14000 - This report *will* display any gift certificates.  This should really never happen.
				DRT 3/9/2005 - PLID 15891 - Added patient coordinator (from the bill) - Fields:  UserName, PersonID, Last, First, Middle
					Also added an extended filter on patient coordinator.  Note that this is the pt coordinator the Quote, because
					bills do not have pat coords.
				JMJ 4/21/2006 - PLID 20224 - removed BillsT.BilledQuote in lieu of BilledQuotesT
				(a.walling 2006-10-10 13:11) - PLID 20236 - Overhaul. Now the root of the report hierarchy are Bills rather than Quotes.
					since multiple quotes may be referenced on a bill. so show the bill, the charges, and the quotes used. What we had
					originally was more like "Quotes converted to Bills" rather than Bills converted from Quotes.
				// fixed the filtering on PatCoord. Had accidently changed it to filter on balances which did not work entirely.
				// also changed the Idate/Tdate discrepancy to always look at bills and added quote input date
				// (j.gruber 2009-03-25 11:25) - PLID 33691 - change discount structure
				// (j.jones 2010-02-04 15:55) - PLID 36500 - removed outer query to fix filtering assertions
			*/
			switch (nSubLevel) {
				case 0: // main report
					return _T("SELECT BilledQuotesT.BillID, "
							"	BillsT.PatientID AS PatID, "
							"	PatientsT.UserDefinedID, "
							"	BillsT.Description AS BillDescription, "
							"	BillsT.Date AS TDate, "
							"	BillsT.InputDate AS IDate, "
							"	ChargesT.ID AS ChargeID, "
							"	ChargesT.ItemCode, "
							"	LineItemT.Description, "
							"	dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
							"	LineItemT.LocationID AS LocID, "
							"	ChargesT.DoctorsProviders AS ProvID, "
							"	PersonT.Last AS PatientLast, "
							"	PersonT.First AS PatientFirst, "
							"	PersonT.Middle AS PatientMiddle, "
							"   BillsT.PatCoord AS PatCoordID "

							"FROM BilledQuotesT "
							"LEFT JOIN BillsT ON BilledQuotesT.BillID = BillsT.ID "
							"LEFT JOIN BillsT QuotesQ ON BilledQuotesT.QuoteID = BillsT.ID "
							"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
							"LEFT JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
							"LEFT JOIN PersonT ON BillsT.PatientID = PersonT.ID "
							"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
							"WHERE BillsT.Deleted = 0 "
							"	AND LineItemT.Deleted = 0 "
							"GROUP BY BilledQuotesT.BillID, "
							"		BillsT.PatientID, "
							"		PatientsT.UserDefinedID, "
							"		BillsT.Description, "
							"		BillsT.Date, "
							"		BillsT.InputDate, "
							"		ChargesT.ID, "
							"		ChargesT.ItemCode, "
							"		LineItemT.Description, "
							"		dbo.GetChargeTotal(ChargesT.ID), "
							"		LineItemT.LocationID, "
							"		ChargesT.DoctorsProviders,"
							"		PersonT.Last, "
							"		PersonT.First, "
							"		PersonT.Middle, "
							"		BillsT.PatCoord");
					break;
				case 1:	// sub report (quotes attached to the bill)
					// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
					// (r.goldschmidt 2016-02-08 13:06) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
					return _T("SELECT	QuotesQ.ID AS QuoteID, "
							"	BilledQuotesT.BillID AS BillID, "
							"	(SELECT SUM(ChargeRespT.Amount) AS Total "
							"        FROM     BillsT "
							"                 LEFT JOIN ChargesT "
							"                   ON BillsT.ID = ChargesT.BillID "
							"                 INNER JOIN LineItemT "
							"                   ON ChargesT.ID = LineItemT.ID "
							"				  LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
							"        WHERE    BillsT.EntryType = 2 "
							"		AND ChargesT.BillID = QuotesQ.ID "
							"        GROUP BY BillsT.ID,BillsT.Description,BillsT.DATE, BillsT.PatCoord) AS QuoteTotal, "

							"	QuotesQ.Description AS QuoteDesc, "
							"	QuotesQ.Date AS QuoteDate, "
							"	QuotesQ.InputDate AS QuoteInputDate, "
							"	QuotesQ.PatCoord AS PatCoordID, "
							"	PersonT.Last AS PatCoordLast, "
							"	PersonT.First AS PatCoordFirst, "
							"	PersonT.Middle AS PatCoordMiddle, "
							"	UsersT.UserName AS PatCoordUserName, "
							"	CASE WHEN PackagesT.Type = 1 THEN PackagesT.TotalCount ELSE NULL END AS PackageCount, "
							"	PackagesT.Type AS PackageType, "
							"	CASE 	WHEN PackagesT.Type = 1 THEN 'Repeatable' "
							"		WHEN PackagesT.Type = 2 THEN 'Multi-Use' "
							"		ELSE NULL END AS PackageTypeName "

							"FROM BilledQuotesT "

							"LEFT JOIN PackagesT ON PackagesT.QuoteID = BilledQuotesT.QuoteID "
							"LEFT JOIN BillsT QuotesQ ON QuotesQ.ID = BilledQuotesT.QuoteID "
							"LEFT JOIN UsersT ON QuotesQ.PatCoord = UsersT.PersonID "
							"LEFT JOIN PersonT ON UsersT.PersonID = PersonT.ID "
							"WHERE QuotesQ.Deleted = 0");
					break;
				default:
					ASSERT(FALSE);
					return _T("");
			}
			break;

	case 248:
			//Unconverted Quotes

			/*	Version History
				(a.walling 2006-06-16 09:42) - PLID 20146 - Added package info, so unbilled packages don't just show up as normal quotes
				JMJ 4/21/2006 - PLID 20224 - removed BillsT.BilledQuote in lieu of BilledQuotesT
				// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
				// (j.gruber 2009-03-25 11:27) - PLID 33691 - change discount structure
				// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
				// (r.goldschmidt 2016-02-08 13:07) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
			*/

			return _T("SELECT BillsT.ID BillID, ChargesT.ID ChargeID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "
			"  BillsT.Date AS TDate, BillsT.InputDate AS IDate, "
			"  BillsT.Description AS Description, "
			"  (CASE WHEN PackagesT.Type Is Null THEN 'Quotes' WHEN PackagesT.Type = 1 THEN 'Repeatable Packages' WHEN PackagesT.Type = 2 THEN 'Multi-Use Packages' END) AS PackageType,"
			"  PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"  UsersT.PersonID AS PatCoordID, CASE WHEN (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) IS NULL THEN 0 ELSE (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END as PercentOff, LineItemT.Description AS ChargeDesc, "
			"  ChargesT.ItemCode, CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) IS NULL THEN convert(money, 0) ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END as Discount, ChargesT.OthrBillFee, ChargesT.TaxRate, "
			"  Sum(ChargeRespT.Amount) AS Total, "
			"  LineItemT.LocationID AS LocID, BillsT.PatCoord AS CoordID, ChargesT.DoctorsProviders AS ProvID, "
			"  (CASE WHEN PackagesT.Type = 2 THEN ChargesT.Quantity ELSE Null END) AS PackageChargeQty, (CASE WHEN PackagesT.Type = 1 THEN PackagesT.TotalCount ELSE Null END) AS PackageQty "
			"FROM "
			"  BillsT LEFT OUTER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID  "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT OUTER JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
			"LEFT OUTER JOIN LocationsT ON BillsT.Location = LocationsT.ID "
			"LEFT OUTER JOIN UsersT ON BillsT.PatCoord = UsersT.PersonID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND BillsT.Active = 1 AND LineItemT.Deleted = 0 "
			"  AND BillsT.ID NOT IN (SELECT QuoteID FROM BilledQuotesT WHERE BillID IN (SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) "
			"  AND (PackagesT.QuoteID Is Null OR (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0))) "
			"GROUP BY BillsT.ID, ChargesT.ID, PatientsT.PersonID, PatientsT.UserDefinedID, BillsT.Date, BillsT.InputDate, BillsT.Description, PersonT.Location, "
			"  PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, UsersT.PersonID, LineItemT.Description, "
			"  Chargest.ItemCode, ChargesT.OthrBillFee, ChargesT.DoctorsProviders, Chargest.TaxRate, LineItemT.LocationID, BillsT.PatCoord, "
			"  PackagesT.TotalCount, PackagesT.Type, ChargesT.Quantity");
			/*
			return _T("SELECT BillsT.ID BillID, ChargesT.ID ChargeID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,   "
			"BillsT.Date AS TDate, BillsT.InputDate AS IDate, BillsT.Description,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"UsersT.PersonID AS PatCoordID, ChargesT.PercentOff, LineItemT.Description AS ChargeDesc,   "
			"ChargesT.ItemCode, ChargesT.Discount, ChargesT.OthrBillFee, ChargesT.TaxRate,  "
			"Sum(Round(Convert(money,(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([PercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[PercentOff]))/100.0) END)-(CASE WHEN([Discount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [Discount] END))*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1))),2)) AS Total, LineItemT.LocationID AS LocID, BillsT.PatCoord AS CoordID,  "
			"ChargesT.DoctorsProviders AS ProvID, (CASE WHEN PackagesT.Type Is NOT Null THEN ChargesT.Quantity ELSE Null END) AS PackageChargeQty, (CASE WHEN PackagesT.Type = 1 THEN PackagesT.TotalCount ELSE Null END) AS PackageQty"
			"FROM  "
			"BillsT LEFT OUTER JOIN ChargesT ON BillsT.ID = ChargesT.BillID  "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
			"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID   "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			"LEFT OUTER JOIN LocationsT ON BillsT.Location = LocationsT.ID  "
			"LEFT OUTER JOIN UsersT ON BillsT.PatCoord = UsersT.PersonID  "
			"LEFT OUTER JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
			"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND BillsT.Active = 1 AND LineItemT.Deleted = 0 "
			"AND BillsT.ID NOT IN (SELECT QuoteID FROM BilledQuotesT WHERE BillID IN (SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) "
			"GROUP BY BillsT.ID, ChargesT.ID, PatientsT.PersonID, PatientsT.UserDefinedID, BillsT.Date, BillsT.InputDate, BillsT.Description, PersonT.Location,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, UsersT.PersonID, Chargest.PercentOff, LineItemT.Description,   "
			"Chargest.ItemCode, ChargesT.Discount, ChargesT.OthrBillFee, ChargesT.DoctorsProviders, Chargest.TaxRate, LineItemT.LocationID, BillsT.PatCoord, PackagesT.TotalCount, PackagesT.Type, ChargesT.Quantity");
			*/
			break;

	case 246:
			//Printed HCFA's
		// (j.jones 2008-09-05 10:19) - PLID 30288 - supported MailSentNotesT
			return _T("SELECT MailSent.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"MailSentNotesT.Note, MailSent.Selection, MailSent.Sender, MailSent.Date AS TDate, MailSent.Location AS LocID, PatientsT.MainPhysician AS ProvID  "
			"FROM MailSent "
			"INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
			"LEFT JOIN PersonT ON MailSent.PersonID = PersonT.ID  "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE MailSentNotesT.Note Like '%HCFA Printed%'");
			break;

	case 275:
		//Charges By Category By Provider
		/*	Version History
			DRT 3/12/03 - Added another field for filtering categories.  The previous one never really worked, because it's filtering on ParentID - your item
					may have been a subcategory, in which case it would just do nothing.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			TES 4/21/2005 - Added UserDefinedID
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			(v.maida - 2014-03-18 16:01) - PLID 61344 - Modify Charges by Category by Provider for ICD-10
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
		"PatientsT.UserDefinedID, "
		"PersonT.Zip,  "
		"LineItemT.Date AS TDate,  "
		"BillsT.Date AS BDate,  "
		"ChargesT.ID,  "
		"ChargesT.ItemCode,  "
		"LineItemT.Description,  "
		"ICD9T1.CodeNumber as ICD9Code1, "
		"ICD10T1.CodeNumber as ICD10Code1, "
		"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
		"ChargesT.Quantity,  "
		"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
		"ChargesT.BillID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
		"ProvidersT.PersonID AS ProvID,  "
		"LineItemT.Amount AS PracBillFee,  "
		"CPTModifierT.Number AS Modifier,  "
		"CPTModifierT2.Number AS Modifier2,  "
		"CategoriesQ.CategoryID AS CategoryID,  "
		"CategoriesQ.Category,  "
		"PersonT.City,  "
		"PersonT.State,  "
		"BillsT.EntryType,  "
		"LineItemT.InputDate AS IDate,  "
		"CategoriesQ.SubCategory,  "
		"CategoriesQ.ParentID AS ParentID,  "
		"ServiceT.Name, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, ServiceT.Category AS CatFilterID "
		"FROM ((((((((ChargesT LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID) INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID) LEFT JOIN ServiceT ON (ChargesT.ServiceID = ServiceT.ID)) "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0))) AS CategoriesQ ON ServiceT.Category = CategoriesQ.CategoryID) LEFT JOIN CPTCodeT ON (ChargesT.ServiceID = CPTCodeT.ID) "
		"WHERE (((BillsT.EntryType)=1) AND ((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) AND GCTypesT.ServiceID IS NULL "
		"");
		break;

	case 276:
		//Charges by Diag Code by Service Code - query blatantly stolen and modified from diag codes by provider
	case 389:
		//Charge Totals by Diag Code
		/*	Version History
			DRT 4/23/03 - Removed the extended filter, there was nothing in the code to fill it in, but it was enabled.
			DRT 2/2/2004 - PLID 9813 - Made the diag code by service code report editable.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			// (j.jones 2009-03-27 09:34) - PLID 33714 - supported infinite diagnosis codes on a bill
			// (j.gruber 2014-03-25 13:50) - PLID 61345 - update for ICD-10
			// (j.gruber 2014-03-31 13:37) - PLID 61545 - added ChargeID so that we can fix the report totals
		*/
		return _T("SELECT _DiagCodesByProvQ.PatID AS PatID, _DiagCodesByProvQ.UserDefinedID, _DiagCodesByProvQ.ICD9Code AS ICD9Code, _DiagCodesByProvQ.ICD10Code AS ICD10Code, _DiagCodesByProvQ.TDate AS TDate,  "
			"_DiagCodesByProvQ.Description, _DiagCodesByProvQ.IDate AS IDate, _DiagCodesByProvQ.ProvID AS ProvID, _DiagCodesByProvQ.PatName, _DiagCodesByProvQ.DocName,  "
			"_DiagCodesByProvQ.ItemCode, _DiagCodesByProvQ.ItemSubCode, _DiagCodesByProvQ.ICD9CodeDesc, _DiagCodesByProvQ.ICD10CodeDesc, _DiagCodesByProvQ.LocID AS LocID, _DiagCodesByProvQ.Amount,  "
			"_DiagCodesByProvQ.CodeName, _DiagCodesByProvQ.ServiceID AS ServiceID, _DiagCodesByProvQ.ChargeID "
			"FROM ( "
			"SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, ICD9T.CodeNumber AS ICD9Code, ICD10T.CodeNumber AS ICD10Code, LineItemT.Date AS TDate,  "
			"LineItemT.Description, LineItemT.InputDate AS IDate, ChargesT.DoctorsProviders AS ProvID,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"DoctorsT.Last + ', ' + Doctorst.First + ' ' + DoctorsT.Middle AS DocName,  "
			"ServiceT.ID AS ServiceID, ServiceT.Name AS CodeName, ChargesT.ItemCode, ChargesT.ItemSubCode, ICD9T.CodeDesc as ICD9CodeDesc, ICD10T.CodeDesc as ICD10CodeDesc, LineItemT.LocationID AS LocID,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount, ChargesT.ID as ChargeID  "
			"FROM  "
			"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID   "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID "
			"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID  "
			"LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID  "
			"LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID= ICD10T.ID  "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			"LEFT JOIN PersonT AS DoctorsT ON ChargesT.DoctorsProviders = DoctorsT.ID  "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"WHERE  "
			"(BillDiagCodeT.ICD9DiagID IS Not Null OR BillDiagCodeT.ICD10DiagID IS NOT NULL) AND  "
			"(LineItemT.Deleted = 0) AND GCTypesT.ServiceID IS NULL "
			") AS _DiagCodesByProvQ ");
		break;

	case 286:
		//Charge Totals By Provider
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
		*/
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, "
			"BillsT.Date AS BillDate, LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
			"LineItemT.Description, ChargesT.ItemCode, "
			"CASE WHEN EXISTS (SELECT ID FROM AppliesT WHERE DestID = ChargesT.ID) THEN (SELECT Sum(Amount) FROM AppliesT WHERE DestID = ChargesT.ID) ELSE 0 END AS AppliedAmount, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
			"LineItemT.LocationID AS LocID, "
			"ChargesT.DoctorsProviders AS ProvID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName "
			"FROM LineItemT INNER JOIN (((ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID) LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) ON LineItemT.ID = ChargesT.ID INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GCTypesT.ServiceID IS NULL ");
		break;

	case 404:
		//Charges by Input Name
		/*	Version History
			TES 7/17/03 - Created, all charges grouped by the person who entered them.
			MSC 5/12/04 - Added Place Of Service to be an eligible field
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			// (f.dinatale 2010-09-30) - PLID 39741 - Added CPTCodeT.Code AS CPTCode.
			// (f.dinatale 2010-10-01) - PLID 39741 - Fixed the fields so that now they display ICD-9 rather than CPT codes.
			// (j.jones 2010-10-26 16:24) - PLID 41004 - added BillID and BillInputDate as an available field
			// (d.singleton 2014-03-28 14:56) - PLID 61347 - Modify Charges by Input Name report for ICD-10
		*/
		return _T("SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, ChargesT.DoctorsProviders AS ProvID, LineItemT.Date AS TDate, LineItemT.InputDate AS IDate,  "
			"LineItemT.LocationID AS LocID, LocationsT.Name AS LocationName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, ChargesT.ItemCode, LineItemT.Amount AS UnitFee,  "
			"ChargesT.CPTModifier, LineItemT.Description, ChargesT.Quantity, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
			"LineItemT.InputName AS IName, "
			"POS.Name AS PlaceOfService, "
			"ICD9T1.CodeNumber as ICD9Code1, \r\n "
			"ICD9T2.CodeNumber as ICD9Code2, \r\n "
			"ICD9T3.CodeNumber as ICD9Code3, \r\n "
			"ICD9T4.CodeNumber as ICD9Code4, \r\n "
			"ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"ICD10T2.CodeNumber as ICD10Code2, \r\n "
			"ICD10T3.CodeNumber as ICD10Code3, \r\n "
			"ICD10T4.CodeNumber as ICD10Code4, \r\n "
			"insCo.name AS InsuranceCompanyName, \r\n" //j.camacho (7/3/2014) - PLID 62719
			"BillsT.ID AS BillID, BillsT.InputDate AS BillInputDate "
			"FROM "
			"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN PersonT PersonT1 ON ChargesT.DoctorsProviders = PersonT1.ID "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
			"LEFT JOIN InsuredPartyT InsParty ON InsParty.PersonID=BillsT.InsuredPartyID \r\n" //j.camacho (7/3/2014) - PLID 62719
			"LEFT JOIN InsuranceCoT InsCo ON InsCo.PersonID=InsParty.InsuranceCoID \r\n" //j.camacho (7/3/2014) - PLID 62719
			"LEFT JOIN LocationsT POS ON BillsT.Location = POS.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND GCTypesT.ServiceID IS NULL");
		break;

	case 409:
		//Charges by Referring Physician
		/*	Version History
			DRT 7/23/03 - Created.  Mostly copied off of the Charges by Date report. (PLID 6424)
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
		*/
		return _T("SELECT PatientsT.PersonID AS PatID, PersonT.Zip, LineItemT.Date AS TDate, BillsT.Date AS BDate, ChargesT.ID, "
			"ChargesT.ItemCode, LineItemT.Description, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
			"ChargesT.Quantity, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
			"ChargesT.BillID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, ProvidersT.PersonID AS ProvID, "
			"LineItemT.Amount AS PracBillFee, CPTModifierT.Number AS Modifier, CPTModifierT2.Number AS Modifier2, CategoriesT.ID AS CatID, "
			"CategoriesT.Name AS CatName, PersonT.City, PersonT.State, BillsT.EntryType, LineItemT.InputDate AS IDate, "
			"PatientsT.UserDefinedID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
			"ReferringPhysT.PersonID AS RefPhysID, "
			"CASE WHEN PersonRef.ID IS NULL THEN '<No Referring Phys>' ELSE PersonRef.Last + ', ' + PersonRef.First + ' ' + PersonRef.Middle END AS RefPhysName "
			"FROM "
			"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
			"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			"LEFT JOIN PersonT PersonProv ON ProvidersT.PersonID = PersonProv.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "
			"LEFT JOIN PersonT PersonRef ON ReferringPhysT.PersonID = PersonRef.ID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"WHERE "
			"BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GCTypesT.ServiceID IS NULL ");
		break;

	case 410:
		//Charges by Referral Source
		/*	Version History
			DRT 7/23/03 - Created.  Mostly copied off of the Charges by Date report. (PLID 4285)
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
		*/
		return _T("SELECT PatientsT.PersonID AS PatID, PersonT.Zip, LineItemT.Date AS TDate, BillsT.Date AS BDate, ChargesT.ID, "
			"ChargesT.ItemCode, LineItemT.Description, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
			"ChargesT.Quantity, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
			"ChargesT.BillID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, ProvidersT.PersonID AS ProvID, "
			"LineItemT.Amount AS PracBillFee, CPTModifierT.Number AS Modifier, CPTModifierT2.Number AS Modifier2, CategoriesT.ID AS CatID, "
			"CategoriesT.Name AS CatName, PersonT.City, PersonT.State, BillsT.EntryType, LineItemT.InputDate AS IDate, "
			"PatientsT.UserDefinedID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
			"ReferralSourceT.PersonID AS RefSrcID, "
			"CASE WHEN ReferralSourceT.PersonID IS NULL THEN '<No Referral Source>' ELSE ReferralSourceT.Name END AS RefSrcName "
			"FROM "
			"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
			"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			"LEFT JOIN PersonT PersonProv ON ProvidersT.PersonID = PersonProv.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"WHERE "
			"BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GCTypesT.ServiceID IS NULL ");
		break;

	case 411:
		//Charges by Referring Patient
		/*	Version History
			DRT 7/23/03 - Created.  Mostly copied off of the Charges by Date report. (PLID 4284)
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
		*/
		return _T("SELECT PatientsT.PersonID AS PatID, PersonT.Zip, LineItemT.Date AS TDate, BillsT.Date AS BDate, ChargesT.ID, "
			"ChargesT.ItemCode, LineItemT.Description, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
			"ChargesT.Quantity, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
			"ChargesT.BillID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, ProvidersT.PersonID AS ProvID, "
			"LineItemT.Amount AS PracBillFee, CPTModifierT.Number AS Modifier, CPTModifierT2.Number AS Modifier2, CategoriesT.ID AS CatID, "
			"CategoriesT.Name AS CatName, PersonT.City, PersonT.State, BillsT.EntryType, LineItemT.InputDate AS IDate, "
			"PatientsT.UserDefinedID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
			"RefPersonT.ID AS RefPatID, "
			"CASE WHEN RefPersonT.ID IS NULL THEN '<No Referring Patient>' ELSE RefPersonT.Last + ', ' + RefPersonT.First + ' ' + RefPersonT.Middle END AS RefPatName "
			"FROM "
			"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
			"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			"LEFT JOIN PersonT PersonProv ON ProvidersT.PersonID = PersonProv.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT RefPersonT ON PatientsT.ReferringPatientID = RefPersonT.ID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"WHERE "
			"BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GCTypesT.ServiceID IS NULL ");
		break;

	case 438:
		//Outside Fees Quoted by Charge Code
		/* Version History
			TES 8/7/03 - Created. Based on the Quotes by Patient Coordinator
			JMJ 4/21/2006 - PLID 20224 - removed BillsT.BilledQuote in lieu of BilledQuotesT
			// (j.gruber 2009-03-25 11:31) - PLID 33691 - changed discount structure
			// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
			// (r.goldschmidt 2016-02-08 13:09) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
		*/
		return _T("SELECT BillsT.ID BillID, ChargesT.ID ChargeID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
			"BillsT.Date AS TDate, BillsT.InputDate AS IDate, BillsT.Description,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS PatCoord, "
			"UsersT.PersonID AS PatCoordID, CASE WHEN (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) IS NULL THEN 0 ELSE (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END as PercentOff, LineItemT.Description AS ChargeDesc,  "
			"CASE WHEN ChargesT.ItemCode = '' THEN ServiceT.Name ELSE ChargesT.ItemCode END AS ItemCode, CASE WHEN(SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) IS NULL THEN convert(money,0) ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END as Discount, "
			"Sum(Round(Convert(money,(((CASE WHEN OthrBillFee Is Null THEN 0 ELSE OthrBillFee END)*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100.0) END)-(CASE WHEN([TotalDiscount] Is Null OR (OthrBillFee = 0 OR LineItemT.Amount > 0)) THEN 0 ELSE [TotalDiscount] END))*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1))),2)) AS OthrBillFee, "
			"ChargesT.TaxRate, "
			"Sum(ChargeRespT.Amount) AS Total, "
			"LineItemT.LocationID AS LocID, ChargesT.ServiceID AS CPTID, CASE WHEN BillsT.ID IN (SELECT QuoteID FROM BilledQuotesT WHERE BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1)) THEN 1 ELSE 0 END AS Converted, "
			"ChargesT.DoctorsProviders AS ProvID "
			"FROM "
			"BillsT LEFT OUTER JOIN (ChargesT LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID) ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID  "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT OUTER JOIN LocationsT ON BillsT.Location = LocationsT.ID "
			"LEFT OUTER JOIN UsersT ON BillsT.PatCoord = UsersT.PersonID "
			"LEFT OUTER JOIN PersonT PersonT1 ON UsersT.PersonID = PersonT1.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
			"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"AND ChargesT.OthrBillFee <> 0 "
			"GROUP BY BillsT.ID, ChargesT.ID, PatientsT.PersonID, PatientsT.UserDefinedID, BillsT.Date, BillsT.InputDate, BillsT.Description, PersonT.Location, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, UsersT.PersonID, LineItemT.Description,  "
			"Chargest.ItemCode, ChargesT.OthrBillFee, ChargesT.DoctorsProviders, Chargest.TaxRate, LineItemT.LocationID, ChargesT.ServiceID, ServiceT.Name");

		case 470:
		//Charges By Service Modifier
		/*	Version history
			JMM - 02/06/04 - Created
			TES - 02/24/2004 - Modified to account for the fact that charges can have multiple modifiers.
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Replaced ChargesT.ProcCode
		*/
		return _T(" SELECT SubQ.UserDefinedID, SubQ.PatID AS PatID, SubQ.BirthDate, SubQ.PatName, SubQ.ProvID AS ProvID, SubQ.ProvName, SubQ.CPTCode,	"
			" SubQ.ItemSubCode, SubQ.Description, SubQ.Total, SubQ.Ins1Resp, SubQ.Ins2Resp, SubQ.Ins3Resp, SubQ.TDate AS TDate, SubQ.IDate AS IDate,"
			" SubQ.ID, SubQ.CPTID AS CPTID, SubQ.LocID AS LocID, SubQ.Location, SubQ.Quantity, "
			" SubQ.ModiD AS ModiD, SUbQ.ModName as ModName "
			" FROM (SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,      PersonT.BirthDate,   "
			" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,      ChargesT.DoctorsProviders AS ProvID,    "
			" PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,      ChargesT.ItemCode AS CPTCode, ChargesT.ItemSubCode,  "
			" LineItemT.Description,      Total = SUM(CASE WHEN ChargeRespT.Amount IS NULL then 0 else ChargeRespT.Amount END),  "
			" Ins1Resp = SUM(Case WHEN InsuredPartyT.RespTypeID = 1 then ChargeRespT.Amount Else 0 END),    "
			" Ins2Resp = SUM(CASE WHEN InsuredPartyT.RespTypeID = 2 then ChargeRespT.Amount Else 0 END), 	"
			" Ins3Resp = SUM(CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN ChargeRespT.Amount ELSE 0 END), "
			"  LineItemT.Date AS TDate, LineItemT.InputDate AS IDate,      ChargesT.ID, ServiceT.ID AS CPTID, LineItemT.LocationID AS LocID, "
			" LocationsT.Name AS Location, ChargesT.Quantity, CPTModifierT.Number AS ModiD, CPTmodifierT.Note AS ModName "
			" FROM PatientsT INNER JOIN     BillsT ON PatientsT.PersonID = BillsT.PatientID INNER JOIN     PersonT ON     "
			" PatientsT.PersonID = PersonT.ID RIGHT OUTER JOIN     (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN   "
			" ChargesT ON LineItemT.ID = ChargesT.ID LEFT OUTER JOIN     InsuredPartyT RIGHT OUTER JOIN     ChargeRespT ON     "
			" InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID ON      ChargesT.ID = ChargeRespT.ChargeID ON      BillsT.ID = ChargesT.BillID LEFT OUTER JOIN   "
			" PersonT PersonT1 ON      ChargesT.DoctorsProviders = PersonT1.ID LEFT OUTER JOIN     CPTCodeT INNER JOIN   "
			" ServiceT ON CPTCodeT.ID = ServiceT.ID ON      ChargesT.ServiceID = ServiceT.ID LEFT OUTER JOIN "
			"     (SELECT ChargesT.ID, ChargesT.CPTModifier FROM ChargesT WHERE ChargesT.CPTModifier Is Not Null "
			"      UNION SELECT ChargesT.ID, ChargesT.CPTModifier2 FROM ChargesT WHERE ChargesT.CPTModifier2 Is Not Null "
			"      UNION SELECT ChargesT.ID, ChargesT.CPTModifier3 FROM ChargesT WHERE ChargesT.CPTModifier3 Is Not Null "
			"      UNION SELECT ChargesT.ID, ChargesT.CPTModifier4 FROM ChargesT WHERE ChargesT.CPTModifier4 Is Not Null "
			"     ) ChargeModifiersQ ON ChargesT.ID = ChargeModifiersQ.ID LEFT JOIN CPTModifierT ON     "
			" ChargeModifiersQ.CPTModifier = CPTModifierT.Number "
			" LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			" WHERE (LineItemT.Deleted = 0) AND (BillsT.EntryType = 1) AND     "
			" (BillsT.Deleted = 0) AND CPTCodeT.ID IS NOT NULL AND GCTypesT.ServiceID IS NULL "
			" GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, "
			"    PersonT.BirthDate,     PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,      ChargesT.DoctorsProviders,    "
			" PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,      ChargesT.ItemCode, ChargesT.ItemSubCode,     "
			" LineItemT.Description, ServiceT.ID,     LineItemT.Date, LineItemT.InputDate, ChargesT.ID, LineItemT.LocationID, LocationsT.Name,  "
			" ChargesT.Quantity, CPTModifierT.Number, CPTModifierT.Note) AS SubQ");
		break;


		case 479:
		//Charges By Insurance Company By Service Code
		/*	Version History
			JMM 2-23-04 - Created by Charges by Insurance Company
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 8/18/2005 - PLID 17294 - Removed the second IDate field and reverified the ttx file.
			DRT 6/15/2007 - PLID 23634 - Added diagnosis code fields and Whichcodes descriptor
			//(e.lally 2011-11-08) PLID 45541 - Added financial class
			// (j.gruber 2012-01-05 10:31) - PLID 42415 - include inventory items too
			// (j.gruber 2014-03-24 11:14) - PLID 61348 - update for ICD-10
		*/
		return _T("SELECT ChargeID AS ChargeID, InputDate as IDate, PatID AS PatID, PatName as PatName, ChargeAmount AS ChargeAmount, "
			" InsResp As InsResp, Applies As Applies, AppliedPays as AppliedPays, AppliedAdj AS AppliedAdj, "
			" AppliedRef As AppliedRef, InsCoID As InsID, InsCoName AS InsCoName, TDate AS TDate, Description, ProvID AS ProvID, "
			" ProvName AS ProvName, ItemCode, ItemSubCode, RespTypeID AS RespTypeID, UserDefinedID, LocID AS LocID, "
			" Location, Balance AS Balance, ServiceID AS CPTID, \r\n" 
			" ICD9Code1, \r\n "
			" ICD9Code2, \r\n "
			" ICD9Code3, \r\n "
			" ICD9Code4, \r\n "
			" ICD10Code1, \r\n "
			" ICD10Code2, \r\n "
			" ICD10Code3, \r\n "
			" ICD10Code4, \r\n "
			" ICD9CodeDesc1, \r\n "
			" ICD9CodeDesc2, \r\n "
			" ICD9CodeDesc3, \r\n "
			" ICD9CodeDesc4, \r\n "
			" ICD10CodeDesc1, \r\n "
			" ICD10CodeDesc2, \r\n "
			" ICD10CodeDesc3, \r\n "
			" ICD10CodeDesc4, \r\n "
			" WhichCodes, \r\n"
			" FinancialClassID, FinancialClass "
			"\r\n"
			"FROM "
			" (SELECT ChargesT.ID AS ChargeID, "
			"LineItemT.InputDate, "
			"PatientsT.PersonID AS PatID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
			"/*Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) AS PatResp, */"
			"Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null THEN ChargeRespT.Amount ELSE 0 End) AS InsResp, "
			"Sum((CASE WHEN [AppliesQ].[PayAmount] Is Null THEN 0 ELSE AppliesQ.PayAmount End)+(CASE WHEN [AppliesQ].[AdjAmount] Is Null THEN 0 ELSE AppliesQ.AdjAmount End)) AS Applies, "
			"Sum((CASE WHEN [AppliesQ].[PayAmount] Is Null THEN 0 ELSE AppliesQ.PayAmount End)) AS AppliedPays, "
			"Sum((CASE WHEN [AppliesQ].[AdjAmount] Is Null THEN 0 ELSE AppliesQ.AdjAmount End)) AS AppliedAdj, "
			"0 AS AppliedRef, "
			"InsuranceCoT.PersonID AS InsCoID, "
			"InsuranceCoT.Name AS InsCoName, "
			"LineItemT.Date AS TDate, "
			"LineItemT.Description, "
			"ProvidersT.PersonID AS ProvID, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
			"ChargesT.ItemCode, "
			"ChargesT.ItemSubCode, "
			"RespTypeT.TypeName AS RespTypeID, "
			"LineItemT.InputDate AS IDate, "
			"PatientsT.UserDefinedID, "
			"LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  "
			"(Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null THEN ChargeRespT.Amount ELSE 0 End) - Sum((CASE WHEN [AppliesQ].[PayAmount] Is Null THEN 0 ELSE AppliesQ.PayAmount End)+(CASE WHEN [AppliesQ].[AdjAmount] Is Null THEN 0 ELSE AppliesQ.AdjAmount End))) AS Balance,  \r\n" 
			" ChargesT.ServiceID, \r\n"
			
			"ICD9T1.CodeNumber as ICD9Code1, \r\n "
			"ICD9T2.CodeNumber as ICD9Code2, \r\n "
			"ICD9T3.CodeNumber as ICD9Code3, \r\n "
			"ICD9T4.CodeNumber as ICD9Code4, \r\n "
			"ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"ICD10T2.CodeNumber as ICD10Code2, \r\n "
			"ICD10T3.CodeNumber as ICD10Code3, \r\n "
			"ICD10T4.CodeNumber as ICD10Code4, \r\n "
			"ICD9T1.CodeDesc as ICD9CodeDesc1, \r\n "
			"ICD9T2.CodeDesc as ICD9CodeDesc2, \r\n "
			"ICD9T3.CodeDesc as ICD9CodeDesc3, \r\n "
			"ICD9T4.CodeDesc as ICD9CodeDesc4, \r\n "
			"ICD10T1.CodeDesc as ICD10CodeDesc1, \r\n "
			"ICD10T2.CodeDesc as ICD10CodeDesc2, \r\n "
			"ICD10T3.CodeDesc as ICD10CodeDesc3, \r\n "
			"ICD10T4.CodeDesc as ICD10CodeDesc4,  \r\n "

			" ChargeWhichCodesFlatV.WhichCodes "
			", InsuranceCoT.FinancialClassID, FinancialClassT.Name AS FinancialClass "
			"\r\n"
			"FROM (((((((ChargesT LEFT JOIN ((ChargeRespT LEFT JOIN (SELECT AppliesT.RespID, Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END ) AS PayAmount, Sum(CASE WHEN LineItemT.Type = 2 THEN Appliest.Amount ELSE 0 END) AS AdjAmount FROM AppliesT LEFT JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID GROUP BY AppliesT.RespID) AS AppliesQ ON ChargeRespT.ID = AppliesQ.RespID) LEFT JOIN (InsuredPartyT INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID) ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID) "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number) "
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID) "
			"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) "
			"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID) "
			"LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID) "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"LEFT JOIN ChargeWhichCodesFlatV ON ChargesT.ID = ChargeWhichCodesFlatV.ChargeID \r\n "
			"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
			"LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID \r\n"
			"WHERE ((BillsT.Deleted)=0) AND ((BillsT.EntryType)=1) AND ((LineItemT.Deleted)=0) AND GCTypesT.ServiceID IS NULL "
			" AND (CPTCodeT.ID IS NOT NULL OR ProductT.ID IS NOT NULL) \r\n"
			"\r\n"
			"GROUP BY "
			"ChargesT.ID, LineItemT.InputDate, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
			"InsuranceCoT.PersonID, InsuranceCoT.Name, LineItemT.Date, LineItemT.Description, ProvidersT.PersonID, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, ChargesT.ItemCode, ChargesT.ItemSubCode, LineItemT.InputDate, PatientsT.UserDefinedID, "
			"LineItemT.LocationID, LocationsT.Name, RespTypeT.TypeName, ChargesT.ServiceID,  \r\n "
			"ICD9T1.CodeNumber, \r\n "
			"ICD9T2.CodeNumber, \r\n "
			"ICD9T3.CodeNumber, \r\n "
			"ICD9T4.CodeNumber, \r\n "
			"ICD10T1.CodeNumber, \r\n "
			"ICD10T2.CodeNumber, \r\n "
			"ICD10T3.CodeNumber, \r\n "
			"ICD10T4.CodeNumber, \r\n "
			"ICD9T1.CodeDesc, \r\n "
			"ICD9T2.CodeDesc, \r\n "
			"ICD9T3.CodeDesc, \r\n "
			"ICD9T4.CodeDesc, \r\n "
			"ICD10T1.CodeDesc, \r\n "
			"ICD10T2.CodeDesc, \r\n "
			"ICD10T3.CodeDesc, \r\n "
			"ICD10T4.CodeDesc,  \r\n "
			"ChargeWhichCodesFlatV.WhichCodes, \r\n "
			"InsuranceCoT.FinancialClassID, FinancialClassT.Name "
			"\r\n"
			"HAVING ((Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Not Null THEN ChargeRespT.Amount End)<>0))) SubQ");
		break;

	case 441:
		//Discounted Bills
		/*	Version History
			DRT 8/29/03 - Created.  This shows all bills which were created from a quote that was discounted (say that 5 times fast).
			TES 3/8/04 - Completely re-written, since you can now discount bills directly.
			(e.lally 2007-06-18) PLID 26347 - Added Pre-tax charge amount, pre-tax discount total, tax1 total, tax2 total fields
			(d.moore 2007-08-24) - PLID 25166 - Added fields for the monetary discount and the percentage off
			  as well as the description of the discount. I also modified the WHERE part of the query. It was
			  letting some non-discounted items through.	
			  // (j.gruber 2009-04-01 09:11) - PLID 33787 - added subreport for new discount structure, kept old fields also in case of edited reports
			// (j.politis 2015-08-18 15:21) - PLID 28369 - We need to fix how we round tax 1 and tax 2, added round function around (tax1 + tax2) when calculating TotalDiscount
		*/

		switch (nSubLevel) { //main report
			case 0:
				return _T("SELECT BillsT.ID AS BillID, TotalPercentOff, TotalDiscount as SumDiscounts, "
					"Sum(Round(Convert(money, "
					"((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 0 ELSE [TotalPercentOff] END)/100)+(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END))) "
					"+Round(((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 0 ELSE [TotalPercentOff] END)/100)+(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate-1)) "
					"+((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 0 ELSE [TotalPercentOff] END)/100)+(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate2-1)),2) "
					"),2)) AS TotalDiscount, "
					" Sum((Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2))) as AmtBeforeDiscount, "
					" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription, "
					"dbo.GetBillTotal(BillsT.ID) AS BillTotal, BillsT.Date AS BDate,  "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
					"BillsT.InputDate AS IDate, PersonCoord.ID AS CoordID,   "
					"PersonCoord.Last + ', ' + PersonCoord.First + ' ' + PersonCoord.Middle AS CoordName, LocationsT.ID AS LocID,  "
					"LocationsT.Name AS LocName, Convert(money,SUM(LineItemT.Amount * ChargesT.Quantity)) As PreTaxChargeTotal, "
					"Convert(money,SUM(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 0 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate-1)))),2))) as Tax1Total, "
					"Convert(money,SUM(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 0 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate2-1)))),2))) as Tax2Total, "
					"Convert(money,SUM(Round(Convert(money, ((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 0 ELSE [TotalPercentOff] END)/100)+(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END)))),2))) AS PreTaxDiscountTotal, "
					" ChargesT.ID as ChargeID, LineItemT.Description as ChargeDescription, dbo.GetChargeTotal(ChargesT.ID) as ChargeTotal "
					"FROM   "
					"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID  "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
					"LEFT JOIN   "
					"	(SELECT ChargeID, Sum(Amount) AS TotalAmt FROM ChargeRespT GROUP BY ChargeID) RespQ  "
					"ON ChargesT.ID = RespQ.ChargeID  "
					"LEFT JOIN PersonT ON BillsT.PatientID = PersonT.ID  "
					"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
					"LEFT JOIN PersonT PersonCoord ON BillsT.PatCoord = PersonCoord.ID  "
					"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
					"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
					"WHERE EntryType = 1 AND LineItemT.Type = 10 "
					"AND BillsT.ID IN (SELECT BillID FROM ChargesT WHERE ID IN (SELECT ChargeID FROM ChargeDiscountsT WHERE DELETED = 0)) "
					"AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
					"GROUP BY BillsT.ID, BillsT.Date, PersonT.Last, PersonT.First, PersonT.Middle, PatientsT.UserDefinedID, "
					"PatientsT.PersonID, BillsT.InputDate, PersonCoord.ID, PersonCoord.Last, PersonCoord.First, PersonCoord.Middle, "
					"LocationsT.ID, LocationsT.Name, TotalPercentOff, TotalDiscount, ChargesT.ID, LineItemT.Description");

			break;

			case 1:

				// (j.gruber 2009-04-01 09:16) - PLID 33787 - added subreport
				switch (nSubRepNum) {
					case 0: {
						CString str;
						str.Format("SELECT ID, ChargeID, BillID, PercentOff, Discount, DiscountCategoryDescription, DiscountTotal, ProvID as ProvID, LocID as LocID, PatientID as PatID, BDate as BDate, IDate as IDate FROM (  "
							" 		SELECT ChargeDiscountsT.ID, ChargeDiscountsT.ChargeID, ChargesT.BillID, PercentOff, Discount, CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE  "
							" 		 CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE  "
							" 		 (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription, "
							" 		 Round(Convert(money,  "
							" 			((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE [PercentOff] END)/100)))  "
							" 			),2) "
							" + " /*otherbillfee*/
							" 		 Round(Convert(money,  "
							" 			((([othrbillfee]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE [PercentOff] END)/100)))  "
							"+ (CASE WHEN([Discount] Is Null) THEN 0 ELSE [Discount] END) "
							" 			),2) "
							" AS DiscountTotal, ChargesT.DoctorsProviders as ProvID, LineItemT.LocationID as LocID, LineItemT.PatientID, BillsT.Date as BDate, BillsT.InputDate as IDate "
							" 		 FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
							"		LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
							" WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND ChargeDiscountsT.DELETED = 0 ) Q"); 
						
							return _T(str);
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
		
		break;

	case 487: //Discounted Quotes
		/* Version History
			TES 3/8/04 - Created.
			JMJ 4/21/2006 - PLID 20224 - removed BillsT.BilledQuote in lieu of BilledQuotesT
			(d.moore 2007-08-24) - PLID 25166 - Added fields for the monetary discount and the percentage off
			  as well as the description of the discount. I also modified the WHERE part of the query. It was
			  letting some non-discounted items through.
			  // (j.gruber 2009-04-01 16:55) - PLID 33787 - updated for discount structure
		*/

		switch (nSubLevel) { //main report
			case 0:
		
				return _T("SELECT BillsT.ID AS QuoteID, TotalDiscount as SumDiscount, TotalPercentOff, "
					" Sum((Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2)   "
					" +   "
					" Round(Convert(money,((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2))  "
					" -  "
					" Round(Convert(money, ((((CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
					" 					(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)* "
					" 					(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
					" 					(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)* "
					" 					(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
					" 					*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)) "
					" "					
					" +  "
					" 					((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*  "
					" 					(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*  "
					" 					(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*  "
					" 					(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*  "
					" 					(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)) "
					" 					*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1) "
					" 					)) "
					" 					*CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END  "
					" 					 - Coalesce([TotalDiscount], CONVERT(money, 0))), 2) )	 "
					" 					AS TotalDiscount, 					 "
					""
					"					 Sum((Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2)   "
					" 					 +   "
					"					 Round(Convert(money,((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2))) as AmtBeforeDiscount, "
					"  "
					" 					Sum((Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2)   "
					" 					 +   "
					" 					 Round(Convert(money,((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))),2))  "
					" 					 -  "
					" 					Round(Convert(money, ((((CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
					" 					(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)* "
					" 					(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
					" 					(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)* "
					" 					(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)) "
					" 					 "
					"					+ "
					" 					((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*  "
					" 					(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*  "
					" 					(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*  "

					" 					(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*  "
					" 					(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)) "
					" 					)) "
					" 					 "
					" 					*CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END  "
					" 					 - Coalesce([TotalDiscount], CONVERT(money, 0))), 2) )	 "
					" 					AS PreTaxTotalDiscount,  "
					" " 
					" 					dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription,  " 
					"dbo.GetQuoteTotal(BillsT.ID,1) AS QuoteTotal, BillsT.Date AS QuoteDate,  "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
					"BillsT.InputDate AS IDate, PersonCoord.ID AS CoordID,   "
					"PersonCoord.Last + ', ' + PersonCoord.First + ' ' + PersonCoord.Middle AS CoordName, LocationsT.ID AS LocID,  "
					"LocationsT.Name AS LocName, CASE WHEN BillsT.ID IN (SELECT QuoteID FROM BilledQuotesT WHERE BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1)) THEN 1 ELSE 0 END AS IsBilled, "
					" ChargesT.ID as ChargeID, LineItemT.Description as ChargeDescription, dbo.GetChargeTotal(ChargesT.ID) as ChargeTotal "				
					
					"FROM   "
					"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID  "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
					"LEFT JOIN   "
					"	(SELECT ChargeID, Sum(Amount) AS TotalAmt FROM ChargeRespT GROUP BY ChargeID) RespQ  "
					"ON ChargesT.ID = RespQ.ChargeID  "
					"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID  "
					"LEFT JOIN PersonT ON BillsT.PatientID = PersonT.ID  "
					"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
					"LEFT JOIN PersonT PersonCoord ON BillsT.PatCoord = PersonCoord.ID  "
					"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
					"WHERE EntryType = 2 AND LineItemT.Type = 11 "
					"AND BillsT.ID IN (SELECT BillID FROM ChargesT WHERE ID IN (SELECT ChargeID FROM ChargeDiscountsT WHERE DELETED = 0)) "
					"AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
					"GROUP BY BillsT.ID, BillsT.Date, PersonT.Last, PersonT.First, PersonT.Middle, PatientsT.UserDefinedID, "
					"PatientsT.PersonID, BillsT.InputDate, PersonCoord.ID, PersonCoord.Last, PersonCoord.First, PersonCoord.Middle, "
					"LocationsT.ID, LocationsT.Name, TotalPercentOff, TotalDiscount, ChargesT.ID, LineItemT.Description");
				break;

				case 1:

				// (j.gruber 2009-04-01 09:16) - PLID 33787 - added subreport
				switch (nSubRepNum) {
					case 0: {
						CString str;
						str.Format("SELECT ID, ChargeID, BillID, PercentOff, Discount, DiscountCategoryDescription, DiscountTotal, ProvID as ProvID, LocID as LocID, PatientID as PatID, BDate as QuoteDate, IDate as IDate FROM (  "
							" 		SELECT ChargeDiscountsT.ID, ChargeDiscountsT.ChargeID, ChargesT.BillID, PercentOff, Discount, CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE  "
							" 		 CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE  "
							" 		 (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription, "
							" 		 Round(Convert(money,  "
							" 			((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE [PercentOff] END)/100)))  "
							" 			),2) "
							" + " //otherbillfee
							" 		 Round(Convert(money,  "
							" 			((([othrbillfee]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE [PercentOff] END)/100)))  "
							"+ (CASE WHEN([Discount] Is Null) THEN 0 ELSE [Discount] END) "
							" 			),2) "
							" AS DiscountTotal, ChargesT.DoctorsProviders as ProvID, LineItemT.LocationID as LocID, LineItemT.PatientID, BillsT.Date as BDate, BillsT.InputDate as IDate "
							" 		 FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
							"		LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
							" WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND ChargeDiscountsT.DELETED = 0 ) Q"); 
						
							return _T(str);
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
	case 498:
		//Bill Totals
		/*	Version History
			DRT 4/7/2004 - This is a report for some sale Kevin is trying to make.  It needs tested, refined, etc.  I have
				the excel sheet it was designed from.  Some of the things in here need fixed - it's just grabbing the provider
				from the first charge, that needs to be cleaned up.  We might actually want to base this off of Case Histories, 
				it looks like it would make more sense (this client does have ASC).  Also the title sucks, but I'm not sure
				what to call it.
		*/
		return _T("/*  "
			"Needed fields:  "
			"  Bill Date  "
			"  CPT1, 2, 3, 4, 5, 6 - Comma delimited list of cpt codes?  "
			"  Billed Chgs  "
			"  Total Pays  "
			"  Total Adjustments  "
			"  Balance Due  "
			"  "
			"Unknown fields:  "
			"  Spec (specialty?  we don't track)  "
			"  Payor Code  "
			"  Billed Adj vs Contracted Writeoff (making total adjustments)  "
			"  Bad Debt  "
			"*/  "
			"SELECT   "
			"(SELECT CPTCodeT.Code FROM ChargesT INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID   "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = BillsT.ID AND ChargesT.LineID = 1) AS Chg1,   "
			"(SELECT CPTCodeT.Code FROM ChargesT INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID   "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = BillsT.ID AND ChargesT.LineID = 2) AS Chg2,   "
			"(SELECT CPTCodeT.Code FROM ChargesT INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID   "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = BillsT.ID AND ChargesT.LineID = 3) AS Chg3,   "
			"(SELECT CPTCodeT.Code FROM ChargesT INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID   "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = BillsT.ID AND ChargesT.LineID = 4) AS Chg4,   "
			"(SELECT CPTCodeT.Code FROM ChargesT INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID   "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = BillsT.ID AND ChargesT.LineID = 5) AS Chg5,   "
			"(SELECT CPTCodeT.Code FROM ChargesT INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID   "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = BillsT.ID AND ChargesT.LineID = 6) AS Chg6,   "
			"TotalChargesQ.TotalChgs, BillsT.Date AS BDate, BillsT.InputDate AS IDate, "
			"CASE WHEN AppliedPaysQ.TotalPays IS NULL THEN 0 ELSE AppliedPaysQ.TotalPays END AS TotalPays,  "
			"CASE WHEN AppliedAdjQ.TotalAdj IS NULL THEN 0 ELSE AppliedAdjQ.TotalAdj END AS TotalAdj,  "
			"TotalChargesQ.TotalChgs -   "
			"  CASE WHEN AppliedPaysQ.TotalPays IS NULL THEN 0 ELSE AppliedPaysQ.TotalPays END -   "
			"  CASE WHEN AppliedAdjQ.TotalAdj IS NULL THEN 0 ELSE AppliedAdjQ.TotalAdj END AS Balance,  "
			"PersonProv.ID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName,  "
			"PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName "
			"FROM  "
			"BillsT  "
			"LEFT JOIN "
			"	(SELECT ChargesT.BillID, ChargesT.DoctorsProviders AS ProvID "
			"	FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
			"	WHERE LineItemT.Deleted = 0 AND ChargesT.LineID = 1 "
			"	) ProvQ ON BillsT.ID = ProvQ.BillID "
			"LEFT JOIN   "
			"	(SELECT ChargesT.BillID, Sum(Amount) AS TotalChgs  "
			"	FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
			"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10  "
			"	GROUP BY ChargesT.BillID  "
			"	) TotalChargesQ ON BillsT.ID = TotalChargesQ.BillID  "
			"LEFT JOIN   "
			"	(SELECT ChargesT.BillID, SUM(AppliesT.Amount) AS TotalPays  "
			"	FROM LineItemT LinePays LEFT JOIN AppliesT ON LinePays.ID = AppliesT.SourceID  "
			"	LEFT JOIN LineItemT LineCharges ON AppliesT.DestID = LineCharges.ID  "
			"	LEFT JOIN ChargesT ON LineCharges.ID = ChargesT.ID  "
			"	WHERE LinePays.Type = 1 AND LinePays.Deleted = 0  "
			"	AND LineCharges.Type = 10 AND LineCharges.Deleted = 0   "
			"	AND LineCharges.ID IS NOT NULL  "
			"	GROUP BY ChargesT.BillID  "
			"	) AppliedPaysQ ON BillsT.ID = AppliedPaysQ.BillID  "
			"LEFT JOIN  "
			"	(SELECT ChargesT.BillID, SUM(AppliesT.Amount) AS TotalAdj  "
			"	FROM LineItemT LinePays LEFT JOIN AppliesT ON LinePays.ID = AppliesT.SourceID  "
			"	LEFT JOIN LineItemT LineCharges ON AppliesT.DestID = LineCharges.ID  "
			"	LEFT JOIN ChargesT ON LineCharges.ID = ChargesT.ID  "
			"	WHERE LinePays.Type = 2 AND LinePays.Deleted = 0  "
			"	AND LineCharges.Type = 10 AND LineCharges.Deleted = 0   "
			"	AND LineCharges.ID IS NOT NULL  "
			"	GROUP BY ChargesT.BillID  "
			"	) AppliedAdjQ ON BillsT.ID = AppliedAdjQ.BillID  "
			"LEFT JOIN PersonT PersonProv ON ProvID = PersonProv.ID "
			"LEFT JOIN PersonT ON BillsT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" "
			"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1");
		break;

	case 542:
		//Charges By Category Split by Service/Product
		/*	Version History
			DRT 7/1/2004 - PLID 13292 - Created from Charges by Category - Has an extra group on Service or Product or GC
			DRT 8/27/2004 - PLID 14000 - Report does not show gift certificates sold.
			DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode
			(v.maida - 2014-03-18 09:53) - PLID 61349 - Modify report for ICD-10. 
		*/
		return _T("SELECT PatientsT.PersonID AS PatID, PersonT.Zip, LineItemT.Date AS TDate, BillsT.Date AS BDate, "
		"ChargesT.ID, ChargesT.ItemCode, LineItemT.Description, ICD9T1.CodeNumber as ICD9Code1, ICD10T1.CodeNumber as ICD10Code1, "
		"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, ChargesT.Quantity, "
		"dbo.GetChargeTotal(ChargesT.ID) AS Amount, ChargesT.BillID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
		"ProvidersT.PersonID AS ProvID, LineItemT.Amount AS PracBillFee, CPTModifierT.Number AS Modifier, CPTModifierT2.Number AS Modifier2, "
		"CategoriesQ.CategoryID AS CategoryID, CategoriesQ.Category, PersonT.City, PersonT.State, BillsT.EntryType, LineItemT.InputDate AS IDate, "
		"CategoriesQ.SubCategory, CategoriesQ.ParentID AS ParentID, ServiceT.Name, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
		"ServiceT.Category AS CatFilterID, CASE WHEN ChargesT.ServiceID IN (SELECT ID FROM ProductT) THEN 'Product' "
		"	WHEN ChargesT.ServiceID IN (SELECT ServiceID FROM GCTypesT) THEN 'Gift Certificate' "
		"	WHEN ChargesT.ServiceID IN (SELECT ID FROM CPTCodeT)	THEN 'Service' ELSE '' END AS SplitType "
		"FROM ((((((((ChargesT LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID) INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID) LEFT JOIN ServiceT ON (ChargesT.ServiceID = ServiceT.ID)) "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		"LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
		"FROM CategoriesT "
		"WHERE (((CategoriesT.Parent)=0)) "
		"UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
		"FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
		"WHERE (((SubCategoriesT.Parent)<>0))) AS CategoriesQ ON ServiceT.Category = CategoriesQ.CategoryID) LEFT JOIN CPTCodeT ON (ChargesT.ServiceID = CPTCodeT.ID) "
		"WHERE (((BillsT.EntryType)=1) AND ((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) AND GCTypesT.ServiceID IS NULL "
		"");
		break;

	case 546:
		//Gift Certificates Sold
		/*	Version History
			DRT 8/27/2004 - PLID 14000 - Report to show all gift certificates sold.  Editable fields 
				available for all sorts of fun things.
			(a.walling 2007-03-22 15:48) - PLID 25113 - Turned the query into GCSoldQ for external and extended filters
			// (j.jones 2010-02-04 15:57) - PLID 36500 - removed outer query to fix filtering assertions
			// (j.dinatale 2011-09-30 14:53) - PLID 45773 - need to exclude voided and original line items from financial corrections
			// (j.jones 2015-04-27 09:52) - PLID 65295 - we now pull the value & balance from a shared function,
			// which added support for the new Value field, Refunds, and GC Transfers
			// (b.eyers 2016-03-11) - PLID 68231 - Value on the report is now the GCValue from lineitemt, fixed this too to allow grouping by recharged gcs
		*/
		return _T("SELECT ChargesT.ID, GiftCertificatesT.PurchaseDate AS PurchDate, LineItemT.InputDate AS InputDate,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS Price, "
			"GCBalanceQ.TotalValue, "
			"GCBalanceQ.AmtUsed, "
			"GCBalanceQ.Balance, "
			"LineItemT.GCValue AS ValueSold, "
			"GiftCertificatesT.ExpDate, GiftCertificatesT.GiftID, "
			"LocationsT.ID AS LocID, LocationsT.Name AS LocName, PatientsT.UserDefinedID,  "
			"PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"PersonProv.ID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName,  "
			"ServiceT.Name AS GCType, ReferralSourceT.PersonID AS ReferralID, ReferralSourceT.Name AS ReferralName,  "
			"PersonRefPat.ID AS RefPatID, PersonRefPat.Last + ', ' + PersonRefPat.First + ' ' + PersonRefPat.Middle AS RefPatName,  "
			"PersonRefPhys.ID AS RefPhysID, PersonRefPhys.Last + ', ' + PersonRefPhys.First + ' ' + PersonRefPhys.Middle AS RefPhysName, "
			"ServiceT.ID AS ServiceID "
			" "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN GiftCertificatesT ON LineItemT.GiftID = GiftCertificatesT.ID "
			"INNER JOIN (" + GetGiftCertificateValueQuery() + ") AS GCBalanceQ ON GiftCertificatesT.ID = GCBalanceQ.ID "
			"INNER JOIN ServiceT ON chargest.ServiceID = ServiceT.ID "
			"INNER JOIN GCTypesT ON servicet.id = GCTypesT.ServiceID  "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT ON GiftCertificatesT.PurchasedBy = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT PersonProv ON ChargesT.DoctorsProviders = PersonProv.ID "
			"LEFT JOIN PersonT PersonRefPat ON PatientsT.ReferringPatientID = PersonRefPat.ID "
			"LEFT JOIN PersonT PersonRefPhys ON PatientsT.DefaultReferringPhyID = PersonRefPhys.ID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 "
			"AND OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL ");
		break;

	case 559:
		//Finance Charges
		// (j.jones 2009-06-10 14:00) - PLID 34583 - added extra data to the report to allow
		// showing the created charge info. and the overdue charge info.
		return _T("SELECT FinanceChargesT.Amount AS FinChargeAmount, "
			"FinanceChargesT.Date AS TDate, FinanceChargesT.InputDate AS IDate, "
			"PersonT.ID AS PatID, DoctorsT.ID AS ProvID, "
			"CreatedLineItemT.LocationID AS LocID, "
			"OriginalLineItemT.Date AS OriginalChargeDate, "
			"CreatedLineItemT.Date AS CreatedChargeDate, "
			"OriginalLineItemT.InputDate AS OriginalChargeInputDate, "
			"CreatedLineItemT.InputDate AS CreatedChargeInputDate, "
			"dbo.GetChargeTotal(OriginalLineItemT.ID) AS OriginalChargeAmount, "
			"dbo.GetChargeTotal(CreatedLineItemT.ID) AS CreatedChargeAmount, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle + ' ' + DoctorsT.Title AS ProvName, "
			"LocationsT.Name AS LocName, "
			"PatientsT.UserDefinedID, "
			"OriginalLineItemT.Description AS OriginalDescription, "
			"CreatedLineItemT.Description AS CreatedDescription, "
			"FinanceChargesT.ID AS ID, OriginalChargeID, FinanceChargeID "
			""
			"FROM FinanceChargesT "
			"INNER JOIN ChargesT OriginalChargesT ON FinanceChargesT.OriginalChargeID = OriginalChargesT.ID "
			"INNER JOIN LineItemT OriginalLineItemT ON OriginalChargesT.ID = OriginalLineItemT.ID "
			"INNER JOIN ChargesT CreatedChargesT ON FinanceChargesT.FinanceChargeID = CreatedChargesT.ID "
			"INNER JOIN LineItemT CreatedLineItemT ON CreatedChargesT.ID = CreatedLineItemT.ID "
			"LEFT JOIN PersonT ON CreatedLineItemT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT DoctorsT ON CreatedChargesT.DoctorsProviders = DoctorsT.ID "
			"LEFT JOIN LocationsT ON CreatedLineItemT.LocationID = LocationsT.ID "
			"WHERE CreatedLineItemT.Deleted = 0 AND OriginalLineItemT.Deleted = 0");
		break;

	case 575:
		//Quoted Packages
		/*	Version History
			// (m.hancock 2006-08-10 17:03) - PLID 18624 - Created report
			// (j.gruber 2007-08-06 10:41) - PLID 26139 - added Pat coord, POs, location, and Prepayment info
			// (z.manning, 09/24/2007) - PLID 27490 - Removed ORDER BY clause so the Create Merge Field button works.
			// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
			// (j.gruber 2010-09-09 13:27) - PLID 36283 - added tax rates
			// (s.tullis 2014-08-19 13:14) - PLID 34769 - Add unit description field for inventory to the package quote report.
		*/
		return _T("SELECT "
			"/* Patient details */ "
			"BillsT.PatientID AS PatID, PatientsT.UserDefinedID AS UserDefinedID, "
			"PersonT.First AS FirstName, PersonT.Middle AS MiddleName, PersonT.Last AS LastName, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			""
			"/* Quote / Bill details */ "
			"PackagesT.QuoteID AS QuoteID, BillsT.Active, BillsT.Date AS QuoteDate, BillsT.PatCoord as PatCoordID, PatCoordT.First as PatCoordFirst, PatCoordT.Middle as PatCoordMiddle, PatCoordT.Last as PatCoordLast, "
			"BillsT.Location AS POSID, PosT.Name as POSName, "
			"LocationsT.ID AS BillLocationID, LocationsT.Name as BillLocName, LocationsT.ID AS LocID, "
			""
			"/* Package details */ "
			"BillsT.Date AS Date, BillsT.Description, PackagesT.Type, TotalAmount, UnbilledAmount, TotalCount, CurrentCount, "
			"(CASE WHEN TotalCount = CurrentCount THEN 1 WHEN CurrentCount = 0 THEN 2 ELSE 3 END) AS PackageStatus, ProductT.UnitDesc AS UnitDescription, "
			""
			" /*Prepayment Details */ "
			" LineItemT.ID AS PrePayID, LineItemT.Date as PrePayDate, LineItemT.Amount as PrePayAmount,  "

			" /* Tax Amounts */ "
			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [ChargeLineT].[Amount] Is Null THEN 0 ELSE [ChargeLineT].[Amount] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN Coalesce([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,Coalesce([TotalPercentOff],0))/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN ChargeLineT.Amount > 0 OR OthrBillFee = 0 THEN Coalesce([TotalDiscount], convert(money, 0)) ELSE 0 END "
			")* /* Tax */ "
			" (ChargesT.[TaxRate]-1)  "
			"),2) AS PracOnlyTax1Total,  "

			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [ChargeLineT].[Amount] Is Null THEN 0 ELSE [ChargeLineT].[Amount] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN [TotalPercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[TotalPercentOff])/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN ChargeLineT.Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount], Convert(money,0)) ELSE 0 END "
			")* /* Tax */ "
			" (ChargesT.[TaxRate2]-1)  "
			"),2) AS PracOnlyTax2Total,  "

			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [ChargesT].[OthrBillFee] Is Null THEN 0 ELSE [ChargesT].[OthrBillFee] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN Coalesce([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,Coalesce([TotalPercentOff], 0))/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN ChargeLineT.Amount = 0 AND OthrBillFee > 0 THEN Coalesce([TotalDiscount], convert(money,0)) ELSE 0 END "
			")* /* Tax */ "
			"(ChargesT.[TaxRate]-1)  "
			"),2) AS OtherOnlyTax1Total, "

			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [ChargesT].[OthrBillFee] Is Null THEN 0 ELSE [ChargesT].[OthrBillFee] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN Coalesce([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,Coalesce([TotalPercentOff], 0))/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN ChargeLineT.Amount = 0 AND OthrBillFee > 0 THEN Coalesce([TotalDiscount], convert(money,0)) ELSE 0 END "
			")* /* Tax */ "
			"(ChargesT.[TaxRate2]-1)  "
			"),2) AS OtherOnlyTax2Total "

			"FROM ( "
			"   SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount AS UnbilledAmount, PackagesT.Type, "
			"	(CASE WHEN PackagesT.Type = 1 THEN PackagesT.TotalCount WHEN PackagesT.Type = 2 THEN PackageChargesQ.MultiUseTotalCount ELSE 0 END) AS TotalCount, "
			"	(CASE WHEN PackagesT.Type = 1 THEN PackagesT.CurrentCount WHEN PackagesT.Type = 2 THEN PackageChargesQ.MultiUseCurrentCount ELSE 0 END) AS CurrentCount "
			"	FROM PackagesT LEFT JOIN ( "
			"		SELECT ChargesT.BillID, Sum(Quantity) AS MultiUseTotalCount, Sum(PackageQtyRemaining) AS MultiUseCurrentCount "
			"		FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 "
			"		AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
			"		GROUP BY BillID "
			"	) AS PackageChargesQ ON PackagesT.QuoteID = PackageChargesQ.BillID "
			") AS PackagesT "
			"INNER JOIN BillsT ON PackagesT.QuoteID = BillsT.ID "
			"LEFT JOIN PersonT ON PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT PatCoordT ON BillsT.PatCoord = PatCoordT.ID "
			"LEFT JOIN PaymentsT ON BillsT.ID = PaymentsT.QuoteID "
			"LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN LocationsT POST ON BillsT.Location = POST.ID "
			"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN ProductT ON ProductT.ID= ServiceT.ID "
			"LEFT JOIN LineItemT ChargeLineT ON ChargesT.ID = ChargeLineT.ID "			
			"LEFT JOIN LocationsT ON ChargeLineT.LocationID = LocationsT.ID "
			"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
			"WHERE ChargeLineT.Deleted = 0 AND BillsT.Deleted = 0 AND (LineItemT.Deleted IS NULL OR LineItemT.Deleted = 0) "
			"AND (ChargeLineT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
			);
		break;

		case 714:
			/* Version History 
				// (j.gruber 2011-10-10 14:34) - PLID 45361 - created for
			*/

			return _T(" SELECT PatientsT.PersonID as PatID, PatientsT.UserDefinedID as PatientID,ProvidersT.PersonID as ProvID, LocationsT.ID as LocID, \r\n "
					  " PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, \r\n "
					  " PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, \r\n "
					  " AffPersonT.First As AffiliateFirst, AffPersonT.Last As AffiliateLast, AffPersonT.Middle As AffiliateMiddle, AffPersonT.Title As AffiliateTitle,  \r\n "
					  " AffPersonT.Address1 As AffiliateAddress1, AffPersonT.Address2 As AffiliateAddress2,  AffPersonT.City As AffiliateCity, AffPersonT.State As AffiliateState,  \r\n "
					  " AffPersonT.Zip As AffiliateZip, AffPersonT.HomePhone As AffiliateHPhone, AffPersonT.WorkPhone As AffiliateWorkPhone, AffiliatesT.NPI as AffiliateNPI, \r\n "
					  " AffPersonT.Company as AffiliateCompany, AffPersonT.CompanyID As AffiliateAccountID,  \r\n "
					  " CustomContact1.Name as AffiliateCustom1FieldName, CustomContact1.TextParam as AffiliateCustom1Data, \r\n "
					  " CustomContact2.Name as AffiliateCustom2FieldName, CustomContact2.TextParam as AffiliateCustom2Data, \r\n "
					  " CustomContact3.Name as AffiliateCustom3FieldName, CustomContact3.TextParam as AffiliateCustom3Data, \r\n "
					  " CustomContact4.Name as AffiliateCustom4FieldName, CustomContact4.TextParam as AffiliateCustom4Data, \r\n "
					  " BillsT.ID as BillID, BillsT.Description As BillDescription, BillsT.Date as BillDate, BillsT.InputDate as BillIDate, \r\n "
					  " CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name as ServiceName, \r\n "
					  " BillsT.AffiliatePhysID as AffiliatePhysID, BillsT.AffiliatePhysAmount, BillsT.AffiliateNote,  \r\n "
					  " BillsT.AffiliateStatusID as StatusID, BillAffiliateStatusT.Name as AffiliateStatusName,   \r\n "
					  " BillAffiliateStatusHistoryT.Date as AffiliatePhysStatusDate, BillAffiliateStatusT.Type as AffiliateStatusType,  \r\n "
					  " ChargesT.ID as ChargeID, LineItemT.Date as ChargeDate, LineItemT.InputDate as ChargeIDate, LineItemT.Description as ChargeDescription, \r\n "
					  " PersonProvT.First as ProvFirst, PersonProvT.Middle as ProvMiddle, PersonProvT.Last as ProvLast, PersonProvT.Title as ProvTitle, ProvidersT.NPI as ProvNPI,  \r\n "
					  " LocationsT.Name as ChargeLocation, LocationsT.Address1 as ChargeLocAddress1, LocationsT.Address2 as ChargeLocAddress2, LocationsT.City as ChargeLocCity, LocationsT.State AS ChargeLocState, LocationsT.Zip as ChargeLocZip, LocationsT.Phone as ChargeLocPhone, \r\n "
					  " POST.Name as POSName, POST.Address1 as POSAddress1,POST.Address2 as POSAddress2, POST.City as POSCity, POST.State AS POSState, POST.Zip as POSZip, \r\n "
					  " Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN(TotalPercentOff Is Null) THEN 1 ELSE ((100-Convert(float,TotalPercentOff))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate-1)))),2) AS TotalTax1,   \r\n "
					  " Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN(TotalPercentOff Is Null) THEN 1 ELSE ((100-Convert(float,TotalPercentOff))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate2-1)))),2) AS TotalTax2, \r\n "
					  " Convert(money, Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN TotalPercentOff Is Null THEN 1 ELSE ((100-Convert(float,TotalPercentOff))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))),2)) AS PreTaxChargeAmt,     \r\n "
					  " (Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))), 2)-Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN TotalPercentOff Is Null THEN 1 ELSE ((100-Convert(float,TotalPercentOff))/100) END) -(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))), 2)) AS TotalDiscount, 	  \r\n "
					  " TotalPercentOff AS PercentDiscount, TotalDiscount AS DollarDiscount, coalesce(TotalDiscountsQ.DiscountCategory, '') as DiscountCategory, \r\n "
					  " ChargeTotalsT.Total,ChargeTotalsT.PatientResp, ChargeTotalsT.Ins1Resp, \r\n"
					  " ChargeTotalsT.Ins2Resp, ChargeTotalsT.InsOtherResp, ChargeTotalsT.VisionPriResp,  \r\n"
					  " ChargeTotalsT.VisionSecResp, ChargeTotalsT.VisionOtherResp, \r\n"
					  "  \r\n "
					  " TotalPays = SUM(CASE WHEN PayLineT.Type = 1 THEN AppliesT.Amount ELSE 0 END),  \r\n "
					  " PatientPays = SUM(CASE WHEN PayLineT.Type = 1 AND PaymentsT.InsuredPartyID = -1 THEN AppliesT.Amount ELSE 0 END),  \r\n "
					  " Ins1Pays = SUM(Case WHEN PayLineT.Type = 1 AND PayRespTypeT.CategoryType = 1 AND PayRespTypeT.Priority = 1 then AppliesT.Amount Else 0 END),  \r\n "
					  " Ins2Pays = SUM(CASE WHEN PayLineT.Type = 1 AND PayRespTypeT.CategoryType = 1 AND PayRespTypeT.Priority = 2 then AppliesT.Amount Else 0 END),  \r\n "
					  " InsOtherPays = SUM(CASE WHEN PayLineT.Type = 1 AND PayRespTypeT.CategoryType = 1 AND PayRespTypeT.Priority NOT IN (1,2)THEN AppliesT.Amount ELSE 0 END),  \r\n "
					  " VisionPriPays = SUM(CASE WHEN PayLineT.Type = 1 AND PayRespTypeT.CategoryType = 2 AND PayRespTypeT.CategoryPlacement = 1 THEN AppliesT.Amount ELSE 0 END),  \r\n "
					  " VisionSecPays = SUM(CASE WHEN PayLineT.Type = 1 AND PayRespTypeT.CategoryType = 2 AND PayRespTypeT.CategoryPlacement = 2 THEN AppliesT.Amount  ELSE 0 END),  \r\n "
					  " VisionOtherPays = SUM(CASE WHEN PayLineT.Type = 1 AND PayRespTypeT.CategoryType = 2 AND PayRespTypeT.CategoryPlacement IS NULL THEN AppliesT.Amount ELSE 0 END), \r\n "
					  " CategoriesT.ID as CategoryID, CategoriesT.Name as CategoryName  "
					  "  \r\n "
					  "  \r\n "
					  " FROM (SELECT InnerLineItemT.* FROM LineItemT InnerLineItemT \r\n "
							" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON InnerLineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID \r\n " 
							" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON InnerLineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID \r\n " 
							" WHERE InnerLineItemT.DELETED = 0 AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID IS NULL AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID IS NULL \r\n " 
					  " ) LineItemT \r\n "
					  " INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  \r\n "					  

					  " LEFT JOIN ( \r\n "
					  "   SELECT InnerChargesT.ID, \r\n "
					  "   Total = Sum(CASE WHEN InnerChargeRespT.Amount IS NULL then 0 else InnerChargeRespT.Amount END),   \r\n "
					  "	  PatientResp = Sum(CASE WHEN InnerChargeRespT.InsuredPartyID IS NULL OR InnerChargeRespT.InsuredPartyID = -1 THEN InnerChargeRespT.Amount ELSE 0 END),   \r\n "
					  "   Ins1Resp = Sum(Case WHEN InnerRespTypeT.CategoryType = 1 AND InnerRespTypeT.Priority = 1 then InnerChargeRespT.Amount Else 0 END),   \r\n "
					  "   Ins2Resp = Sum(CASE WHEN InnerRespTypeT.CategoryType = 1 AND InnerRespTypeT.Priority = 2 then InnerChargeRespT.Amount Else 0 END),  \r\n "
					  "   InsOtherResp = Sum(CASE WHEN InnerRespTypeT.CategoryType = 1 AND InnerRespTypeT.Priority <> 1 AND InnerRespTypeT.Priority <> 2 THEN InnerChargeRespT.Amount ELSE 0 END),  \r\n "
					  "   VisionPriResp = Sum(CASE WHEN InnerRespTypeT.CategoryType = 2 AND InnerRespTypeT.CategoryPlacement = 1 THEN InnerChargeRespT.Amount ELSE 0 END),  \r\n "
					  "   VisionSecResp = Sum(CASE WHEN InnerRespTypeT.CategoryType = 2 AND InnerRespTypeT.CategoryPlacement = 2 THEN InnerChargeRespT.Amount ELSE 0 END),   \r\n "
					  "   VisionOtherResp = Sum(CASE WHEN InnerRespTypeT.CategoryType = 2 AND InnerRespTypeT.CategoryPlacement IS NULL THEN InnerChargeRespT.Amount ELSE 0 END) \r\n "
					  "   FROM ChargesT InnerChargesT LEFT JOIN ChargeRespT InnerChargeRespT ON InnerChargesT.ID = InnerChargeRespT.ChargeID  \r\n "
					  "   LEFT JOIN InsuredPartyT InnerInsPartyT ON InnerChargeRespT.InsuredPartyID = InnerInsPartyT.PersonID  \r\n "
					  "   LEFT JOIN RespTypeT InnerRespTypeT ON InnerInsPartyT.RespTypeID = InnerRespTypeT.ID \r\n "
					  "   GROUP BY InnerChargesT.ID \r\n "
					  " ) ChargeTotalsT ON ChargesT.ID = ChargeTotalsT.ID   \r\n "
					  " LEFT JOIN (SELECT ChargeID, TotalPercentOff, TotalDiscount,   \r\n "
					  " 		 (CASE WHEN CountID = 1 THEN   \r\n "
					  " 		 	Coalesce(CASE WHEN TotalDiscountsSubQ.DiscountCategoryID IS NULL OR TotalDiscountsSubQ.DiscountCategoryID = -3 THEN 'No Category'          \r\n "
					  " 		 		  ELSE CASE WHEN TotalDiscountsSubQ.DiscountCategoryID = -1 THEN TotalDiscountsSubQ.CustomDiscountDesc   \r\n "
					  " 		 					ELSE CASE WHEN TotalDiscountsSubQ.DiscountCategoryID = -2 THEN COALESCE((SELECT Description FROM CouponsT WHERE ID = TotalDiscountsSubQ.CouponID), 'Coupon - Unspecified')   \r\n "
					  " 		 							  ELSE COALESCE((SELECT Description FROM DiscountCategoriesT WHERE ID = TotalDiscountsSubQ.DiscountCategoryID), 'No Category') END   \r\n "
					  " 		 					END   \r\n "
					  " 		 		  END, '')  \r\n "
					  " 		 	ELSE dbo.GetChargeDiscountList(ChargeID) END) as DiscountCategory   \r\n "
					  " 		    FROM   \r\n "
					  " 		 (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount, Count(ID) as CountID, Min(DiscountCategoryID) as DiscountCategoryID, Min(CouponID) as CouponID, Min(CustomDiscountDesc) as CustomDiscountDesc FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsSubQ) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID   \r\n "
					  " LEFT JOIN (SELECT InnerBillsT.* FROM BillsT InnerBillsT LEFT JOIN BillCorrectionsT ON InnerBillsT.ID = BillCorrectionsT.OriginalBillID WHERE InnerBillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL) BillsT ON ChargesT.BillID = BillsT.ID \r\n "					  
					  " LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID \r\n "
					  " LEFT JOIN AppliesT ON ChargeRespT.ID = AppliesT.RespID \r\n "
					  " LEFT JOIN ( SELECT InnerLineItemT.* FROM LineItemT InnerLineItemT \r\n "
							" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaysQ ON InnerLineItemT.ID = LineItemCorrections_OriginalPaysQ.OriginalLineItemID \r\n " 
							" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaysQ ON InnerLineItemT.ID = LineItemCorrections_VoidingPaysQ.VoidingLineItemID \r\n " 
							" WHERE InnerLineItemT.DELETED = 0 AND InnerLineItemT.Type = 1 AND LineItemCorrections_OriginalPaysQ.OriginalLineItemID IS NULL AND LineItemCorrections_VoidingPaysQ.VoidingLineItemID IS NULL \r\n " 
					  " )PayLineT ON AppliesT.SourceID = PayLineT.ID \r\n "
					  " LEFT JOIN PaymentsT ON PayLineT.ID = PaymentsT.ID \r\n "
					  " LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID 	 \r\n "					  
					  " LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
					  " LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID 	 \r\n "
					  " LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID 	 \r\n "
					  " LEFT JOIN ReferringPhysT AffiliatesT ON BillsT.AffiliatePhysID = AffiliatesT.PersonID \r\n "
					  " LEFT JOIN PersonT AffPersonT ON AffiliatesT.PersonID = AffPersonT.ID \r\n "
					  " LEFT JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID \r\n "
					  " LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID \r\n "
					  " LEFT JOIN BillAffiliateStatusT ON BillsT.AffiliateStatusID = BillAffiliateStatusT.ID  \r\n "
					  " LEFT JOIN BillAffiliateStatusHistoryT ON BillsT.ID = BillAffiliateStatusHistoryT.BillID  \r\n "
					  " AND BillsT.AffiliateStatusID = BillAffiliateStatusHistoryT.StatusID  \r\n "
					  " LEFT JOIN (SELECT PersonID, TextParam, Name FROM CustomFieldDataT INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID WHERE FieldID = 6) AS CustomContact1 ON AffPersonT.ID = CustomContact1.PersonID \r\n "
					  " LEFT JOIN (SELECT PersonID, TextParam, Name FROM CustomFieldDataT INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID WHERE FieldID = 7) AS CustomContact2 ON AffPersonT.ID = CustomContact2.PersonID \r\n "
					  " LEFT JOIN (SELECT PersonID, TextParam, Name FROM CustomFieldDataT INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID WHERE FieldID = 8) AS CustomContact3 ON AffPersonT.ID = CustomContact3.PersonID \r\n "
					  " LEFT JOIN (SELECT PersonID, TextParam, Name FROM CustomFieldDataT INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID WHERE FieldID = 9) AS CustomContact4 ON AffPersonT.ID = CustomContact4.PersonID \r\n "
					  " LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID \r\n "
					  " LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID		 \r\n "
					  " LEFT JOIN InsuredPartyT PayInsPartyT ON PaymentsT.InsuredPartyID = PayInsPartyT.PersonID \r\n "
					  " LEFT JOIN RespTypeT PayRespTypeT ON PayInsPartyT.RespTypeID = PayRespTypeT.ID		 \r\n "
					  " LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n "
					  " LEFT JOIN LocationsT POST ON BillsT.Location = POST.ID \r\n "
					  " LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID \r\n "
					  " LEFT JOIN PersonT PersonProvT on ProvidersT.PersonID = PersonProvT.ID \r\n "
					  "  \r\n "
					  " WHERE LineItemT.Deleted = 0 and BillsT.Deleted = 0 and LineItemT.Type = 10 \r\n "
					  " AND BillsT.EntryType = 1 AND \r\n "
					  " (BillsT.AffiliatePhysID IS NOT NULL OR BillsT.AffiliatePhysAmount IS NOT NULL) \r\n "
					  "  \r\n "
					  " GROUP BY PatientsT.PersonID, PatientsT.UserDefinedID, \r\n "
					  " PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, \r\n "
					  " PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, \r\n "
					  " AffPersonT.First, AffPersonT.Last, AffPersonT.Middle, AffPersonT.Title,  \r\n "
					  " AffPersonT.Address1, AffPersonT.Address2,  AffPersonT.City,AffPersonT.State,  \r\n "
					  " AffPersonT.Zip, AffPersonT.HomePhone, AffPersonT.WorkPhone,  \r\n "
					  " AffPersonT.Company, AffPersonT.CompanyID,  \r\n "
					  " CustomContact1.Name, CustomContact1.TextParam, \r\n "
					  " CustomContact2.Name, CustomContact2.TextParam, \r\n "
					  " CustomContact3.Name, CustomContact3.TextParam, \r\n "
					  " CustomContact4.Name, CustomContact4.TextParam, \r\n "
					  " BillsT.ID,BillsT.AffiliatePhysAmount, BillsT.AffiliateNote,  \r\n "
					  " BillsT.AffiliateStatusID, BillAffiliateStatusT.Name,   \r\n "
					  " BillAffiliateStatusHistoryT.Date, BillAffiliateStatusT.Type,  \r\n "
					  " ChargesT.ID, LineItemT.Amount, ChargesT.Quantity,  \r\n "
					  " ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, \r\n "
					  " TotalDiscountsQ.TotalPercentOff, TotalDiscountsQ.TotalDiscount, TotalDiscountsQ.DiscountCategory, \r\n "
					  " ChargesT.TaxRate, ChargesT.TaxRate2, \r\n "					  
					  " BillsT.Description, LineItemT.Description, \r\n "
					  " PersonProvT.First, PersonProvT.Middle, PersonProvT.Last, PersonProvT.Title, ProvidersT.NPI,  \r\n "
					  " LocationsT.Name, LocationsT.Address1, LocationsT.Address2, LocationsT.City, LocationsT.State, LocationsT.Zip, LocationsT.Phone, \r\n "
					  " POST.Name, POST.Address1,POST.Address2, POST.City, POST.State, POST.Zip, ProvidersT.PersonID, LocationsT.ID, \r\n "
					  " AffiliatesT.NPI, CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name, \r\n "
					  " BillsT.Date, BillsT.InputDate, LineItemT.Date, LineItemT.InputDate, BillsT.AffiliatePhysID, \r\n "
					  " CategoriesT.ID, CategoriesT.Name, \r\n "					  
					  " ChargeTotalsT.Total,ChargeTotalsT.PatientResp, ChargeTotalsT.Ins1Resp, ChargeTotalsT.Ins2Resp, ChargeTotalsT.InsOtherResp, ChargeTotalsT.VisionPriResp, \r\n"
					  " ChargeTotalsT.VisionSecResp, ChargeTotalsT.VisionOtherResp \r\n"
					  "");					  
		break;

		case 752:

			// (j.gruber 2013-09-10 09:46) - PLID 50529 - created Unbatch Claim Charges
			// (s.tullis 2014-03-25 09:14) - PLID 61351 - Modify the Unbatch Claim Charges report for ICD-10

			return _T(
			FormatString(
			" SELECT TotalsPerRespQ.ChargeID, ChargesT.BillID, LineItemT.PatientID as PatID, PatientsT.UserDefinedID as PatientID, 	  \r\n "
			" ChargeSumQ.ChargeSum, \r\n "
			" TotalsPerRespQ.RespAmount,  \r\n "
			" TotalsPerRespQ.TotalPays as TotalRespPay, \r\n "
			" RespAmount - TotalPays as RespBal, \r\n "
			" TotalsPerRespQ.InsuredPartyID, InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, \r\n "
			" RespTypeT.TypeName AS RespTypeName, InsuranceCoT.HCFASetupGroupID, HistoryQ.LastDate as LastSentDate,  \r\n "
			" PersonT.Last as PatLast, PersonT.First as PatFirst, PersonT.Middle as PatMiddle,  \r\n "
			" LineItemT.Date as TDate, LineItemT.InputDate as IDate, ClaimHistoryTracerQ.LastTracerDate,  \r\n "
			" LastPaymentQ.LastPaymentDate, LineItemT.Description as ChargeDescription, LineItemT.LocationID as LocID, LocationsT.Name as LocationName, \r\n "
			" POST.ID as POSTID, POST.Name as POSName,  \r\n "
			" ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, \r\n "
			" Convert ( nvarchar(255),ChargeWhichCodesFlatV.WhichCodes) as WhichCodes, \r\n "
			" Convert ( nvarchar(255),WhichCodesBoth)as WhichDiagCodes, \r\n "
			"ICD9T1.CodeNumber as ICD9Code1, \r\n "
			"ICD9T2.CodeNumber as ICD9Code2, \r\n "
			"ICD9T3.CodeNumber as ICD9Code3, \r\n "
			"ICD9T4.CodeNumber as ICD9Code4, \r\n "
			"ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"ICD10T2.CodeNumber as ICD10Code2, \r\n "
			"ICD10T3.CodeNumber as ICD10Code3, \r\n "
			"ICD10T4.CodeNumber as ICD10Code4, \r\n "
			"ICD9T1.CodeDesc as ICD9CodeDesc1, \r\n "
			"ICD9T2.CodeDesc as ICD9CodeDesc2, \r\n "
			"ICD9T3.CodeDesc as ICD9CodeDesc3, \r\n "
			"ICD9T4.CodeDesc as ICD9CodeDesc4, \r\n "
			"ICD10T1.CodeDesc as ICD10CodeDesc1, \r\n "
			"ICD10T2.CodeDesc as ICD10CodeDesc2, \r\n "
			"ICD10T3.CodeDesc as ICD10CodeDesc3, \r\n "
			"ICD10T4.CodeDesc as ICD10CodeDesc4,  \r\n "
			" ChargesT.DoctorsProviders as ProvID, ProvPersonT.First As ProvFirst, ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, ProvPersonT.Title as ProvTitle, \r\n "
			" ChargesT.ClaimProviderID as ClaimProvID, ClaimProvPersonT.First As ClaimProvFirst, ClaimProvPersonT.Middle as ClaimProvMiddle, ClaimProvPersonT.Last as ClaimProvLast, ClaimProvPersonT.Title as ClaimProvTitle, \r\n "
			" ChargesT.Quantity as ChargeQuantity,  \r\n "
			" ServiceT.Name as ServiceDescription, \r\n "
			" CPTCodeT.Code as ServiceCode, \r\n "	
			" CASE WHEN CPTCodeT.ID IS NULL THEN 0 ELSE 1 END AS IsCPT, \r\n "
			" CASE WHEN LastDate IS NULL THEN 1 ELSE 2 END AS HasBeenSent  \r\n "
			" FROM  \r\n "
			"	(SELECT ChargesT.ID AS ChargeID, ChargeRespT.Amount as RespAmount, ChargeRespT.InsuredPartyID,  \r\n "
			"	 Sum(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END) AS TotalPays \r\n "
			"	 FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  \r\n "
			"	 INNER JOIN \r\n "
			"	 ChargeRespT on ChargesT.ID = ChargeRespT.ChargeID 					  \r\n "
			"	 LEFT JOIN  \r\n "
			"		(SELECT AppliesT.RespID, Sum(AppliesT.Amount) AS Amount FROM PaymentsT  \r\n "
			"	  	 INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  \r\n "
			"		 WHERE LineItemT.Deleted = 0 AND PaymentsT.InsuredPartyID Is Not Null  \r\n "
			"		 GROUP BY AppliesT.RespID  \r\n "
			"		 ) PaysQ  \r\n "
			"	 ON ChargeRespT.ID = PaysQ.RespID 		          \r\n "
			"	 WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10  \r\n "
			"     AND ChargeRespT.InsuredPartyID Is Not Null  \r\n "
			"     GROUP BY ChargesT.ID, ChargeRespT.Amount, ChargeRespT.InsuredPartyID, ChargeRespT.ID \r\n "
			" ) TotalsPerRespQ  \r\n "
			" INNER JOIN ChargesT ON TotalsPerRespQ.ChargeID = ChargesT.ID  \r\n "
			" INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  \r\n "
			" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID \r\n "
			" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID \r\n "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID \r\n "
			" LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n "
			" LEFT JOIN LocationsT POST on BillsT.Location = POST.ID \r\n "
			" LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID \r\n "
			" LEFT JOIN PersonT ClaimProvPersonT ON ChargesT.ClaimProviderID = ClaimProvPersonT.ID \r\n "
			" LEFT JOIN ChargeWhichCodesListV ON ChargesT.ID = ChargeWhichCodesListV.ChargeID \r\n "
			" LEFT JOIN ChargeWhichCodesFlatV ON ChargesT.ID = ChargeWhichCodesFlatV.ChargeID \r\n "
			"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "

			" LEFT JOIN InsuredPartyT ON TotalsPerRespQ.InsuredPartyID = InsuredPartyT.PersonID  \r\n "
			" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID  \r\n "
			" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID  \r\n "
			" LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID  \r\n "
			" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n "
			" LEFT JOIN  \r\n "
			"	(SELECT Max(Date) AS LastDate, ClaimHistoryDetailsT.ChargeID, InsuredPartyID  \r\n "
			"	 FROM ClaimHistoryT LEFT JOIN ClaimHistoryDetailsT ON ClaimHistoryT.ID = ClaimHistoryDetailsT.ClaimHistoryID \r\n "
			"	 WHERE SendType >= %li \r\n "
			"	 GROUP BY ClaimHistoryDetailsT.ChargeID, InsuredPartyID  \r\n "
			"   ) HistoryQ  \r\n "
			" ON TotalsPerRespQ.ChargeID = HistoryQ.ChargeID AND TotalsPerRespQ.InsuredPartyID = HistoryQ.InsuredPartyID  \r\n "
			" LEFT JOIN (SELECT ChargeID, Sum(Amount) as ChargeSum FROM ChargeRespT GROUP BY ChargeID) ChargeSumQ \r\n "
			" ON TotalsPerRespQ.ChargeID = ChargeSumQ.ChargeID \r\n "
			" LEFT JOIN  \r\n "
			"	(SELECT ChargeID, Max(Date) AS LastTracerDate, InsuredPartyID FROM ClaimHistoryT  \r\n "
			"	 INNER JOIN ClaimHistoryDetailsT ON ClaimHistoryT.ID = ClaimHistoryDetailsT.ClaimHistoryID  \r\n "
			"	 WHERE SendType = %li GROUP BY ChargeID, InsuredPartyID  \r\n "
			"	 ) AS ClaimHistoryTracerQ  \r\n "
			" ON TotalsPerRespQ.ChargeID = ClaimHistoryTracerQ.ChargeID AND TotalsPerRespQ.InsuredPartyID = ClaimHistoryTracerQ.InsuredPartyID  \r\n "
			" LEFT JOIN  \r\n "
			"	(SELECT Max(LineItemT.Date) AS LastPaymentDate, PaymentsT.InsuredPartyID,  \r\n "
			"	 ChargesT.ID as ChargeID  \r\n "
			"	 FROM PaymentsT  \r\n "
			"	 INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  \r\n "
			"	 INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID  \r\n "
			"	 INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID  \r\n "
			"	 INNER JOIN LineItemT LineItemT2 ON ChargesT.ID = LineItemT2.ID  \r\n "
			"	 WHERE LineItemT.Deleted = 0 AND LineItemT2.Deleted = 0  \r\n "
			"	 GROUP BY PaymentsT.InsuredPartyID, ChargesT.ID  \r\n "
			"     ) LastPaymentQ  \r\n "
			" ON TotalsPerRespQ.ChargeID = LastPaymentQ.ChargeID AND TotalsPerRespQ.InsuredPartyID = LastPaymentQ.InsuredPartyID 	 \r\n "
			"  \r\n "
			" WHERE (RespAmount - TotalPays) <> 0 AND ((ChargesT.Batched = 0) OR (ChargesT.Batched = 1 AND ChargesT.BillID NOT IN (SELECT BillID FROM HCFATrackT)))  \r\n " ,  
			ClaimSendType::Electronic,ClaimSendType::TracerLetter));
		break;

		case 753:	//Charges By Test Code
			/*

			(a.wilson 2014-08-05 12:28) - PLID 63107 - Created.

			*/
			return R"(
SELECT BLTCT.ID AS LabTestCodeID, 
BLTCT.Code AS LabTestCode, 
BLTCT.Description AS LabTestCodeDescription, 
PatientsT.PersonID AS PatID, 
PatientsT.UserDefinedID AS PatientID, 
PersonT.FullName PatientName, 
PersonT.City AS PatientCity, 
PersonT.State AS PatientState, 
PersonT.Zip AS PatientZip, 
ProvidersT.PersonID AS ProvID, 
PersonT1.FullName AS ProvName, 
LineItemT.Date AS TDate, 
LineItemT.Description, 
LineItemT.Amount AS PracBillFee, 
LineItemT.InputDate AS IDate, 
LineItemT.LocationID AS LocID, 
LocationsT.Name AS Location, 
BillsT.EntryType, 
BillsT.Date AS BDate, 
ChargesT.ID, 
ChargesT.BillID, 
ChargesT.ItemCode, 
ChargesT.Quantity, 
dbo.GetChargeTotal(ChargesT.ID) AS Amount, 
ICD9T1.CodeNumber as ICD9Code1, 
ICD10T1.CodeNumber as ICD10Code1 
FROM ChargesT 
INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID 
LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID 		
LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID 
LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID 
LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID 
LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID 
LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID 
LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID 
LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID 
INNER JOIN ChargeLabTestCodesT CLTCT ON ChargesT.ID = CLTCT.ChargeID 
INNER JOIN BillLabTestCodesT BLTCT ON CLTCT.BillLabTestCodeID = BLTCT.ID 
LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID  
WHERE BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND GCTypesT.ServiceID IS NULL )";
			break;
			//Charges by Billed Category
			/*	Version History
			// (s.tullis 2015-06-17 16:22) - 66292 - Added 
			*/
			case 760:
			return _T("SELECT PatientsT.PersonID AS PatID,  "
				"PersonT.Zip,  "
				"LineItemT.Date AS TDate,  "
				"BillsT.Date AS BDate,  "
				"ChargesT.ID,  "
				"ChargesT.ItemCode,  "
				"LineItemT.Description,  "
				"ICD9T1.CodeNumber as ICD9Code1, "
				"ICD10T1.CodeNumber as ICD10Code1, "
				"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
				"ChargesT.Quantity,  "
				"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
				"ChargesT.BillID,  "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
				"ProvidersT.PersonID AS ProvID,  "
				"LineItemT.Amount AS PracBillFee,  "
				"CPTModifierT.Number AS Modifier,  "
				"CPTModifierT2.Number AS Modifier2,  "
				"CategoriesQ.CategoryID AS CategoryID,  "
				"CategoriesQ.Category,  "
				"PersonT.City,  "
				"PersonT.State,  "
				"BillsT.EntryType,  "
				"LineItemT.InputDate AS IDate,  "
				"CategoriesQ.SubCategory,  "
				"CategoriesQ.ParentID AS ParentID,  "
				"ServiceT.Name, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, ChargesT.Category AS CatFilterID, "
				"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2)) AS PreTaxTotal, "
				"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2)) as Tax1Total, "
				"(Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2)) as Tax2Total "

				"FROM ChargesT "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
				"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
				"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
				"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
				"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID "
				"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID "
				"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID "
				"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
				"LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID "
				"			FROM CategoriesT "
				"			WHERE CategoriesT.Parent=0 "
				"			UNION "
				"			SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID "
				"			FROM CategoriesT "
				"			RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent "
				"			WHERE SubCategoriesT.Parent<>0) AS CategoriesQ "
				"ON ChargesT.Category = CategoriesQ.CategoryID "
				"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"WHERE BillsT.EntryType=1 AND BillsT.Deleted=0 AND LineItemT.Deleted=0 AND GCTypesT.ServiceID IS NULL "
				"");
			break;
	default:
		return _T("");
		break;
	}
}

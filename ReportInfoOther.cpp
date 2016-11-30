////////////////
// DRT 8/6/03 - GetSqlOther() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "EmrUtils.h"
#include "GlobalLabUtils.h"
#include "WhereClause.h"
#include "GlobalDataUtils.h"

CString CReportInfo::GetSqlOther(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;

	// (f.dinatale 2010-10-15) - PLID 40876 - SSN Masking Permissions
	BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
	BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);

	switch (nID) {
	

case 118:
		//Data Issues
		return _T("SELECT LineItemT.Amount AS ChargeAmt,  "
		"    AppliesT.Amount AS ApplyAmt, LineItemT.PatientID,  "
		"    LineItemT.ID AS LineID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
		"    LineItemT.Description "
		"FROM PatientsT INNER JOIN "
		"    LineItemT ON  "
		"    PatientsT.PersonID = LineItemT.PatientID INNER JOIN "
		"    ChargesT ON LineItemT.ID = ChargesT.ID INNER JOIN "
		"    PersonT ON  "
		"    PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN "
		"    AppliesT ON LineItemT.ID = AppliesT.DestID "
		"WHERE (LineItemT.Type = 10) AND (AppliesT.Amount IS NOT NULL)  "
		"    AND (AppliesT.PointsToPayments = 0) AND  "
		"    (LineItemT.Amount > LineItemT.Amount)");
		break;
	

case 204:
		//Check Bill Date
		return _T("SELECT ID, PatientsT.UserDefinedID AS PatientID, Date AS TDate, InputDate AS IDate "
		"FROM BillsT INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
		"WHERE (PatientID > 0)");
		break;
	case 206:
		//Check Charges Date
		return _T("SELECT ChargesT.ID, ChargesT.BillID,  "
		"    ChargesT.ServiceDateFrom AS TDate,  "
		"    ChargesT.ServiceDateTo AS IDate "
		"FROM ChargesT");
		break;
	case 207:
		//Check Charges To/From Dates
		return _T("SELECT ChargesT.ID, ChargesT.BillID, ChargesT.ServiceDateFrom, ChargesT.ServiceDateTo "
		"FROM ChargesT "
		"WHERE (((ChargesT.ServiceDateFrom)>[ServiceDateTo])) OR (((ChargesT.ServiceDateTo)<[ServiceDateFrom])); "
		"");
		break;
	case 205:
		//Check Birthday Date
		return _T("SELECT PatientsT.UserDefinedID AS ID, PersonT.BirthDate AS TDate "
		"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
		"WHERE (((PatientsT.PersonID)>0))");
		break;
	case 208:
		//Check Line Item Date
		return _T("SELECT LineItemT.ID, PatientsT.UserDefinedID AS PatientID, LineItemT.InputDate AS IDate, LineItemT.Date AS TDate "
		"FROM LineItemT INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
		"WHERE (((LineItemT.PatientID)>0)); "
		"");
		break;
	case 209:
		//Check Medication Date
		return _T("SELECT PatientMedications.ID, PatientsT.UserDefinedID AS PatientID,  "
		"    PatientMedications.PrescriptionDate AS TDate "
		"FROM PatientMedications INNER JOIN PatientsT ON PatientMedications.PatientID = PatientsT.PersonID "
		" WHERE PatientMedications.Deleted = 0");
		break;
	case 210:
		//Check Notes Date
		return _T("SELECT ID, Date AS TDate "
		"FROM Notes");
		break;
	case 211:
		//Check Payments Date
		return _T("SELECT PaymentsT.ID, PaymentsT.DepositDate AS TDate "
		"FROM PaymentsT "
		"WHERE (((PaymentsT.DepositDate) IS NOT NULL))");
		break;
	case 212:
		//Check Reservations Date
		return _T("SELECT AppointmentsT.ID, PatientsT.UserDefinedID AS PatientID, AppointmentsT.Date as TDate "
		"FROM AppointmentsT INNER JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID");
		break;
	case 213:
		//Check To Do List Dates
		return _T("SELECT TaskID, Remind AS TDate, Deadline AS IDate "
		"FROM ToDoList");
		break;
	case 214:
		//Check Insured Parties Without Insurance
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     InsuredPartyT.PersonID, PersonT1.First, PersonT1.Middle,  "
		"    PersonT1.Last "
		"FROM PersonT PersonT1 INNER JOIN "
		"    InsuredPartyT ON  "
		"    PersonT1.ID = InsuredPartyT.PersonID LEFT OUTER JOIN "
		"    InsuranceCoT ON  "
		"    InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID RIGHT OUTER "
		"     JOIN "
		"    PatientsT INNER JOIN "
		"    PersonT ON PatientsT.PersonID = PersonT.ID ON  "
		"    InsuredPartyT.PatientID = PatientsT.PersonID "
		"WHERE (InsuredPartyT.PersonID IS NOT NULL) AND  "
		"    (InsuredPartyT.InsuranceCoID IS NULL)");
		break;
	
	case 254:
		//EMR Data By Provider
		/*	Version History
			(b.cardillo 2004-06-11 16:33) - PLID 12867 - Made it join on the EMRSelectT table in order to pull the correct list Data selection values.
			(b.cardillo 2004-06-15 08:56) - PLID 12867 - Made the DetailData include Text and '<Image>' in addition to the list Data.
			t.schneider 6/29/2004 - PLID 9722 - Implemented multi-procedure stuff.
			TES 9/21/2004 - PLID 14154 - Implemented EmrDetailElementsT table
			TES 8/8/2006 - PLID 21868 - De-implemented EmrDetailElementsT table
			// (j.gruber 2007-01-04 09:48) - PLID 24035 - changed to support multi providers in EMR
			// (j.gruber 2007-01-08 15:48) - PLID 24161 - add secondary provider names
			// (c.haag 2008-06-17 17:32) - PLID 30319 - Changed name calculation for details
			DRT 7/22/2008 - PLID 30807 - Added Slider types to the report.  Also added Narrative data, and a flag for IsRtf.  Also I re-verified
				the TTX, there were several issues with the old version.  I also re-formatted the report slightly so there's less wasted space.
				// (j.gruber 2008-07-24 12:46) - PLID 30829 - added templateID for filtering
			// (j.jones 2010-06-22 14:29) - PLID 37981 - supported generic tables, although technically this report does not properly support tables
			// (z.manning 2011-05-23 09:32) - PLID 33114 - Filter on EMR charting permissions
			TES 9/3/2014 - PLID 63513 - Fixed bug in the way narrative text was being converted to NVARCHAR
			// (d.lange 2016-05-05 11:57) - PLID 65500 - Filters out deleted EMR providers
		*/
		return _T(FormatString("SELECT EMRProvidersT.ProviderID AS ProvID, (DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle + ' ' + DoctorsT.Title) AS DoctorName, "
			"EMRProcedureT.ProcedureID AS ProcedureID, ProcedureT.Name AS ProcedureName, "
			"CASE WHEN EmrInfoT.ID = %li THEN EMRDetailsT.MacroName "
			"	WHEN EMRInfoT.DataSubType = %li THEN EMRDetailsT.MergeOverride "
			"	ELSE EmrInfoT.Name END AS DetailName, "
			"(CASE WHEN EMRInfoT.Datatype IN (2,3) THEN EMRDataT.Data WHEN EMRInfoT.Datatype = 1 THEN CONVERT(NVARCHAR(3000), EMRDetailsT.Text) "
			"WHEN EMRInfoT.Datatype = 4 THEN '<Image>' WHEN EMRInfoT.DataType = 5 THEN convert(nvarchar(255), EMRDetailsT.SliderValue) "
			"WHEN EMRInfoT.Datatype = 6 THEN case when charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) = 0 then '' else CONVERT(NVARCHAR(4000),right(EMRDetailsT.Text, len(EMRDetailsT.Text) - charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) -16)) end "
			"ELSE '<Unknown EMR Data Type>' END) AS DetailData, "
			"Count(CASE WHEN EMRInfoT.Datatype IN (2,3) THEN EMRDataT.Data WHEN EMRInfoT.Datatype = 1 THEN CONVERT(NVARCHAR(3000), EMRDetailsT.Text) "
			"WHEN EMRInfoT.Datatype = 4 THEN '<Image>' WHEN EMRInfoT.DataType = 5 THEN convert(nvarchar(255), EMRDetailsT.SliderValue) "
			"WHEN EMRInfoT.Datatype = 6 THEN case when charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) = 0 then '' else CONVERT(NVARCHAR(4000),right(EMRDetailsT.Text, len(EMRDetailsT.Text) - charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) -16)) end "
			"ELSE '<Unknown EMR Data Type>' END) AS CountOfData, "
			"EMRMasterT.PatientAge, "
			"EMRMasterT.PatientGender, "
			"EMRMasterT.Date AS TDate, "
			"EMRMasterT.InputDate AS IDate, "
			"EMRMasterT.LocationID AS LocID, "
			"EMRMasterT.ID AS EmrID, "
			"dbo.GetEMNSecondaryProviderList(EmrMasterT.ID) as SecondaryDocName, CASE WHEN EMRInfoT.DataType = 6 THEN 1 ELSE 0 END AS IsRtf, "
			"EMRMasterT.TemplateID as TemplateID "
			"FROM (SELECT * FROM EMRMasterT WHERE Deleted = 0) AS EMRMasterT "
			"LEFT JOIN (SELECT * FROM EMRDetailsT WHERE Deleted = 0) AS EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"LEFT JOIN EMRSelectT ON EmrDetailsT.ID = EmrSelectT.EmrDetailID "
			"LEFT JOIN EMRDataT ON EmrSelectT.EMRDataID = EMRDataT.ID "
			"LEFT JOIN (SELECT * FROM EMRProcedureT WHERE Deleted = 0) AS EMRProcedureT ON EMRMasterT.ID = EMRProcedureT.EMRID "
			"LEFT JOIN ProcedureT ON EMRProcedureT.ProcedureID = ProcedureT.ID "
			"LEFT JOIN (SELECT * FROM PersonT) AS PatientPersonT ON EMRMasterT.PatientID = PatientPersonT.ID "
			"LEFT JOIN (SELECT ProviderID, EMRID FROM EMRProvidersT WHERE Deleted = 0) AS EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID "
			"LEFT JOIN (SELECT * FROM PersonT) AS DoctorsT ON EMRProvidersT.ProviderID = DoctorsT.ID "
			"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
			"WHERE EmrMasterT.Deleted = 0 " + GetEmrChartPermissionFilter().Flatten() + " "
			"GROUP BY ProviderID, (DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle + ' ' + DoctorsT.Title), EmrProcedureT.ProcedureID, ProcedureT.Name, "
			"CASE WHEN EmrInfoT.ID = %li THEN EMRDetailsT.MacroName "
			"	WHEN EMRInfoT.DataSubType = %li THEN EMRDetailsT.MergeOverride "
			"	ELSE EmrInfoT.Name END, "
			"(CASE WHEN EMRInfoT.Datatype IN (2,3) THEN EMRDataT.Data WHEN EMRInfoT.Datatype = 1 THEN CONVERT(NVARCHAR(3000), EMRDetailsT.Text) "
			"WHEN EMRInfoT.Datatype = 4 THEN '<Image>' WHEN EMRInfoT.DataType = 5 THEN convert(nvarchar(255), EMRDetailsT.SliderValue) "
			"WHEN EMRInfoT.Datatype = 6 THEN case when charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) = 0 then '' else CONVERT(NVARCHAR(4000),right(EMRDetailsT.Text, len(EMRDetailsT.Text) - charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) -16)) end "
			"ELSE '<Unknown EMR Data Type>' END), "
			"EMRInfoT.DataType, EMRMasterT.PatientAge, EMRMasterT.PatientGender, EMRMasterT.Date, EMRMasterT.InputDate, EMRMasterT.LocationID, "
			"EMRMasterT.ID, CASE WHEN EMRInfoT.DataType = 6 THEN 1 ELSE 0 END, EMRMasterT.TemplateID ",
			EMR_BUILT_IN_INFO__TEXT_MACRO, eistGenericTable, EMR_BUILT_IN_INFO__TEXT_MACRO, eistGenericTable));
		break;

	case 255:
		//EMR Totals By Age/Gender
		/*	Version History
			TES 6/29/2004: Implemented multi-procedure support.
			// (j.gruber 2007-01-04 12:54) - PLID 24035 - made it support multi providers
			// (j.gruber 2007-01-08 15:50) - PLID 24161 - add secondary provider names
			// (j.gruber 2008-07-24 12:47) - PLID 30829 - added templateID for filtering
			// (j.jones 2010-12-08 09:34) - PLID 41728 - ensured that a patient age in "months" shows in the < 18 year bucket
		*/
		return _T("SELECT EMRProcedureT.ProcedureID, EMRMasterT.Date AS TDate, EMRMasterT.InputDate AS IDate, dbo.GetEMNProviderList(EMRMasterT.ID) AS DocName, ProcedureT.Name AS ProcedureName, "
			"(CASE WHEN PatientAge LIKE '%month%' OR PatientAge <= 18 THEN 'Total 18 years or younger' WHEN (PatientAge>18 AND PatientAge<=34) THEN 'Total 19 years to 34 years' "
			"WHEN (PatientAge>34 AND PatientAge<=50) THEN 'Total 35 years to 50 years' WHEN (PatientAge>50 AND PatientAge<=64) THEN 'Total 51 years to 64 years' "
			"WHEN (PatientAge>=65) THEN 'Total 65 years or over' ELSE 'No age recorded' END) AS AgeGroup, Count(CASE WHEN PatientAge <= 18 THEN 1 WHEN (PatientAge>18 AND PatientAge<=34) THEN 2 WHEN (PatientAge>34 AND PatientAge<=50) THEN 3 "
			"WHEN (PatientAge>50 AND PatientAge<=64) THEN 4 WHEN (PatientAge>=65) THEN 5 ELSE 0 END) AS TotalInAgeGroup, Sum(CASE WHEN PatientGender=1 THEN 1 ELSE 0 END) AS Male, Sum(CASE WHEN PatientGender=2 THEN 1 ELSE 0 END) AS Female, "
			"EMRMasterT.LocationID AS LocID, EMRProcedureT.ProcedureID AS ProcedureID, EMRMasterT.ID AS EmrID, EMRMasterT.LocationID AS LocID, "
			"dbo.GetEMNSecondaryProviderList(EMRMasterT.ID) AS SecondaryDocName, "
			"EMRMasterT.TemplateID as TemplateID "
			"FROM (SELECT * FROM EMRMasterT WHERE Deleted = 0) AS EMRMasterT LEFT JOIN (SELECT * FROM EMRProcedureT WHERE Deleted = 0) AS EmrProcedureT ON EMRMasterT.ID = EmrProcedureT.EMRID LEFT JOIN ProcedureT ON EMRProcedureT.ProcedureID = ProcedureT.ID "
			"GROUP BY EMRProcedureT.ProcedureID, EMRMasterT.Date, EMRMasterT.InputDate, ProcedureT.Name, "
			"(CASE WHEN PatientAge LIKE '%month%' OR PatientAge <= 18 THEN 'Total 18 years or younger' WHEN (PatientAge>18 AND PatientAge<=34) THEN 'Total 19 years to 34 years' "
			"WHEN (PatientAge>34 AND PatientAge<=50) THEN 'Total 35 years to 50 years' WHEN (PatientAge>50 AND PatientAge<=64) THEN 'Total 51 years to 64 years' "
			"WHEN (PatientAge>=65) THEN 'Total 65 years or over' ELSE 'No age recorded' END), EMRMasterT.LocationID, EMRMasterT.ID, EMRMasterT.TemplateID");
		break;

	case 256 :
		//EMR Data By Patient
		/*	Version History
			(b.cardillo 2004-06-11 16:33) - PLID 12867 - Made it join on the EMRSelectT table in order to pull the correct list Data selection values.
			(b.cardillo 2004-06-15 08:56) - PLID 12867 - Made the DetailData include Text and '<Image>' in addition to the list Data.
			TES 6/29/2004 - PLID 9722 - Implemented multi-procedure support.
			TES 9/21/2004 - PLID 14154 - Implemented EmrDetailElementsT
			DRT 9/27/2005 - PLID 17664 - There were 2 'EMRMasterT.LocationID AS LocID' in the SELECT... this killed the Create Merge Group.  I removed the latter.
			TES 8/8/2006 - PLID 21868 - De-implemented EmrDetailElementsT table
			// (j.gruber 2007-01-04 10:47) - PLID 24035 - made it support multi provider
			// (j.gruber 2007-01-08 15:51) - PLID 24161 - add secondary provider names
			// (c.haag 2008-06-17 17:32) - PLID 30319 - Changed name calculation for details
			DRT 7/22/2008 - PLID 30807 - Added Text and Slider types to the report.  Also added Narrative data, and a flag for IsRtf.  Also I re-verified
				the TTX, there were several issues with the old version.  I also re-formatted the report slightly so there's less wasted space.
				// (j.gruber 2008-07-24 12:47) - PLID 30829 - added templateID for filtering
			// (z.manning 2011-05-23 09:28) - PLID 33114 - Handle charting permissions
			TES 9/3/2014 - PLID 63513 - Fixed bug in the way narrative text was being converted to NVARCHAR
		*/
		return _T("SELECT dbo.GetEMNProviderList(EMRMasterT.ID) AS DocName, "
			"EMRMasterT.PatientID AS PatID, (PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle + ' ' + PatientPersonT.Title) AS PatientName, "
			"EMRProcedureT.ProcedureID AS ProcedureID, ProcedureT.Name AS ProcedureName, "
			"CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS DetailName, "
			"(CASE WHEN EMRInfoT.Datatype IN (2,3) THEN EMRDataT.Data WHEN EMRInfoT.Datatype = 1 THEN CONVERT(NVARCHAR(4000), EMRDetailsT.Text) "
			"WHEN EMRInfoT.Datatype = 4 THEN '<Image>' WHEN EMRInfoT.DataType = 5 THEN convert(nvarchar(255), EMRDetailsT.SliderValue) "
			"WHEN EMRInfoT.Datatype = 6 THEN case when charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) = 0 then '' else CONVERT(NVARCHAR(4000),right(EMRDetailsT.Text, len(EMRDetailsT.Text) - charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) -16)) end "
			"ELSE '<Unknown EMR Data Type>' END) AS DetailData, "
			"Count(CASE WHEN EMRInfoT.Datatype IN (2,3) THEN EMRDataT.Data WHEN EMRInfoT.Datatype = 1 THEN CONVERT(NVARCHAR(4000), EMRDetailsT.Text) "
			"WHEN EMRInfoT.Datatype = 4 THEN '<Image>' WHEN EMRInfoT.DataType = 5 THEN convert(nvarchar(255), EMRDetailsT.SliderValue) "
			"WHEN EMRInfoT.Datatype = 6 THEN case when charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) = 0 then '' else CONVERT(NVARCHAR(4000),right(EMRDetailsT.Text, len(EMRDetailsT.Text) - charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) -16)) end "
			"ELSE '<Unknown EMR Data Type>' END) AS CountOfData, "
			"EMRMasterT.PatientAge, "
			"EMRMasterT.PatientGender, "
			"EMRMasterT.Date AS TDate, "
			"EMRMasterT.InputDate AS IDate, "
			"EMRMasterT.LocationID AS LocID, "
			"EMRDetailsT.OrderID, "
			"EMRMasterT.ID AS EmrID, "
			"dbo.GetEMNSecondaryProviderList(EMRMasterT.ID) AS SecondaryDocName, "
			"CASE WHEN EMRInfoT.Datatype = 6 then 1 else 0 end as IsRtf, "
			"EMRMasterT.TemplateID as TemplateID "
			"FROM (SELECT * FROM EMRMasterT WHERE Deleted = 0) AS EMRMasterT "
			"LEFT JOIN (SELECT * FROM EMRDetailsT WHERE Deleted = 0) AS EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"LEFT JOIN EMRSelectT ON EmrDetailsT.ID = EmrSelectT.EmrDetailID "
			"LEFT JOIN EMRDataT ON EmrSelectT.EMRDataID = EMRDataT.ID "
			"LEFT JOIN (SELECT * FROM EMRProcedureT WHERE Deleted = 0) AS EMRProcedureT ON EMRMasterT.ID = EMRProcedureT.EMRID "
			"LEFT JOIN ProcedureT ON EMRProcedureT.ProcedureID = ProcedureT.ID "
			"LEFT JOIN (SELECT * FROM PersonT) AS PatientPersonT ON EMRMasterT.PatientID = PatientPersonT.ID "
			"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
			"WHERE EmrMasterT.Deleted = 0 " + GetEmrChartPermissionFilter().Flatten() + " "
			"GROUP BY EMRMasterT.PatientID, "
			"(PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle + ' ' + PatientPersonT.Title), EMRProcedureT.ProcedureID, "
			"ProcedureT.Name, "
			"CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END, "
			"(CASE WHEN EMRInfoT.Datatype IN (2,3) THEN EMRDataT.Data WHEN EMRInfoT.Datatype = 1 THEN CONVERT(NVARCHAR(4000), EMRDetailsT.Text) "
			"WHEN EMRInfoT.Datatype = 4 THEN '<Image>' WHEN EMRInfoT.DataType = 5 THEN convert(nvarchar(255), EMRDetailsT.SliderValue) "
			"WHEN EMRInfoT.Datatype = 6 THEN case when charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) = 0 then '' else CONVERT(NVARCHAR(4000),right(EMRDetailsT.Text, len(EMRDetailsT.Text) - charindex('ENDNXRTFHEADER##', EMRDetailsT.Text) -16)) end "
			"ELSE '<Unknown EMR Data Type>' END), "
			"EMRInfoT.DataType, EMRMasterT.PatientAge, EMRMasterT.PatientGender, EMRMasterT.Date, EMRMasterT.InputDate, EMRMasterT.LocationID, EMRDetailsT.OrderID, "
			"EMRMasterT.ID, CASE WHEN EMRInfoT.Datatype = 6 then 1 else 0 end, EMRMasterT.TemplateID "
			"HAVING EMRInfoT.DataType IN (1, 2, 3, 5, 6)");
		break;
		
	case 277:
		//Check Bad Appointments
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, 	 "
			" AppointmentsT.Date, AptPurposeT.Name AS Purpose, AptTypeT.Name AS Type  "
			" FROM (AppointmentsT INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID)  "
			" LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			" LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID  "
			" LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
			" WHERE AptPurposeT.ID Is Not Null AND AptTypeT.ID Is Not Null AND  "
			" /*No AptPurposeType entry matches this pair*/ "
			" (NOT EXISTS (SELECT * FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptPurposeID = AppointmentPurposeT.PurposeID AND AptPurposeTypeT.AptTypeID = AppointmentsT.AptTypeID)  "
			" /*The Purpose is procedural and the Type is not*/ "
			" OR	(AppointmentPurposeT.PurposeID IN (SELECT AptPurposeT.ID FROM AptPurposeT INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID)  "
			" AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 0))  "
			" /*The Type is procedural and the Purpose is not*/ "
			" OR (AppointmentPurposeT.PurposeID NOT IN (SELECT AptPurposeT.ID FROM AptPurposeT INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID)  "
			" AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category <> 0))) ");
		break;

	case 332:
		//EMR Data by Item
		/*	Version History
			1/28/03 - Date is aliased as TDate, the filter (in the header) was trying to use Date.
			(b.cardillo 2004-06-11 16:33) - PLID 12867 - Made it join on the EMRSelectT table in order to pull the correct list Data selection values.
			TES 9/21/2004 - PLID 14154 - Implemented EmrDetailElementsT
			TES 8/23/2005 - Changed procedure filtering.
			TES 8/8/2006 - PLID 21868 - De-implemented EmrDetailElementsT table
			// (c.haag 2008-06-17 17:37) - PLID 30319 - Changed name calculation for details
			// (z.manning 2011-05-23 09:23) - PLID 33114 - Filter on chart permissions
		*/
		{
			CString strSql = FormatString("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PersonT.ID AS PatID, "
				"PatientsT.UserDefinedID, PatientsT.MainPhysician AS ProviderID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
				"-1 AS ProcedureID, "
				"dbo.GetEmrString(EmrMasterT.ID) AS ProcName, EMRMasterT.LocationID AS LocID, EMRMasterT.Date AS TDate, "
				"EMRDataT.Data AS Data, EMRDataT.ID AS DataID, EMRDetailsT.EMRInfoID AS ItemID, "
				"CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS Item, "
				"EMRMasterT.PatientAge, "
				"EMRMasterT.PatientGender, "
				"EMRMasterT.ID AS EmrID "				
				"FROM (SELECT * FROM EMRMasterT WHERE Deleted = 0) AS EMRMasterT LEFT JOIN (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID "
				"ON PersonT.ID = EMRMasterT.PatientID "
				"LEFT JOIN (SELECT * FROM EMRDetailsT WHERE Deleted = 0) AS EMRDetailsT LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID ON EMRMasterT.ID = EMRDetailsT.EMRID "
				"LEFT JOIN EMRSelectT ON EmrDetailsT.ID = EmrSelectT.EmrDetailID "
				"LEFT JOIN EMRDataT ON EmrSelectT.EMRDataID = EMRDataT.ID "
				"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID \r\n"
				"WHERE EMRInfoT.DataType IN (2, 3) %s "
				, GetEmrChartPermissionFilter().Flatten());
				return strSql;
		}
		break;

	case 568:
		// Labs by Patient
		/* Version History
			7/17/2006 a.walling - Report created.
			JMM - PLID  21568 - took out providerID and changed filter to be extended filter
			JMM - PLID  21572 - Add a bunch of fields
			e.lally 2007-02-20 - PLID 24766 - Removed the order by clause because the create merge
				group feature throws an exception with it included. The report changes the sort anyways.
			e.lally 2007-02-21 - PLID 24836 - Removed the deleted field, added a filter for deleted labs,
				and updated the report file as needed.
			(j.jones 2007-07-19 16:55) - PLID 26751 - converted diagnosis and clinical diagnosis into text fields
			(e.lally 2007-08-08) PLID 26978 - Patient name uses the name stored on the lab now.
			(e.lally 2007-11-30) PLID 26698 - Added biopsy date as the default date filter option.
			// (j.gruber 2008-10-16 13:08) - PLID 31433 - add lab results
			// (z.manning 2008-10-30 12:16) - PLID 31864 - Added to be ordered and type
			//TES 12/3/2008 - PLID 32302 - Accounted for NULL AnatomyIDs.
			//TES 12/8/2008 - PLID 32359 - Added the Result Status.
			//TES 12/9/2008 - PLID 32380 - Renamed one of the two fields being aliased as ResultName to ResultFlagName
			// (the hard-coded .ttx was already calling it ResultFlagName, so the only thing this affects is creating
			// merge groups based off this report).  Same for ResultValue => ResultFlagValue
			TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
			(c.haag 2009-06-22 11:12) - PLID 34189 - Added ResultUnits
			TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
			TES 12/8/2009 - PLID 36512 - Restored AnatomySide
			// (z.manning 2010-04-30 15:43) - PLID 37553 - We now have a view to pull anatomic location
			// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
			// (d.lange 2011-01-14 16:09) - PLID 29065 - Added Biopsy Type
			// (c.haag 2011-01-17) - PLID 41806 - Replaced LabsQ completed date with GetLabCompletedDate
			// (z.manning 2011-06-17 15:13) - PLID 43966 - SocialSecurity was missing its alias
			//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
		*/
		{
			CString strSql;
			strSql.Format("SELECT"
				"	LabsQ.ID AS LabID,"
				"	LabsQ.PatientLast + ', ' + LabsQ.PatientFirst + ' ' + LabsQ.PatientMiddle AS Name,"
				"	LabsQ.PatientID AS PatID,"
				"	CASE WHEN LabsQ.Specimen IS NULL THEN"
				"		LabsQ.FormNumberTextID"
				"	ELSE"
				"		LabsQ.FormNumberTextID + ' - ' + LabsQ.Specimen"
				"	END AS Form,"
				"	LabResultsT.SlideTextID AS Slide,"
				"	LabsQ.ClinicalData AS ClinicalData,"
				"	LabsQ.InputDate AS IDate,"
				"	LabResultFlagsT.Name AS ResultFlagName,"
				"	LabResultFlagsT.Value AS ResultFlagValue,"
				"	LabAnatomicLocationQ.AnatomicLocation AS Anatomy,"
				"	LabBiopsyTypeT.Description AS BiopsyType,"
				"	LabResultsT.DiagnosisDesc,"
				"	LabResultsT.ClinicalDiagnosisDesc,"
				"	LocationsT.Name AS LabLocationName,"
				"	LabsQ.LocationID AS LocID,"
				"	LabsQ.LabLocationID AS LabLocID, "
				"   LabsQ.BiopsyDate AS BiopsyDate, "
				"   LabsQ.Specimen, "
				"   dbo.GetLabProviderString(LabsQ.ID) as Providers, "
				"   dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, "
				"   MedAsstPersonT.ID AS MedAsstID, "
				"   MedAsstPersonT.Last AS MedAsstLast, "
				"   MedAsstPersonT.First AS MedAsstFirst,  "
				"   MedAsstPersonT.Middle AS MedAsstMiddle,  "
				"   dbo.GetLabCompletedDate(LabsQ.ID) AS DateOfDischarge, "
				"   LabResultsT.Name as ResultName, "
				"   LabResultsT.DateReceived as ResultReceived, "
				"   LabResultsT.Value as ResultValue, "
				"	LabResultsT.Units AS ResultUnits, "
				"   LabResultsT.Reference as ResultReference, "
				"	LabsQ.ToBeOrdered, "
				"	LabsQ.Type, "
				"	LabResultStatusT.Description AS ResultStatus, "
				"	LabsQ.InitialDiagnosis "
				" FROM LabsT LabsQ"
				" LEFT JOIN (SELECT * FROM LabResultsT WHERE Deleted = 0) LabResultsT ON LabsQ.ID = LabResultsT.LabID "
				" LEFT JOIN LabResultFlagsT"
				"	ON LabResultsT.FlagID = LabResultFlagsT.ID"
				" LEFT JOIN LabAnatomyT"
				"	ON LabsQ.AnatomyID = LabAnatomyT.ID"
				" LEFT JOIN AnatomyQualifiersT "
				"	ON LabsQ.AnatomyQualifierID = AnatomyQualifiersT.ID "
				" LEFT JOIN PersonT"
				"	ON LabsQ.PatientID = PersonT.ID"
				" LEFT JOIN LocationsT"
				"	ON LabsQ.LabLocationID = LocationsT.ID"
				" LEFT JOIN PersonT MedAsstPersonT ON LabsQ.MedAssistant = MedAsstPersonT.ID "
				" LEFT JOIN LabResultStatusT "
				"	ON LabResultsT.StatusID = LabResultStatusT.ID "
				" LEFT JOIN LabAnatomicLocationQ ON LabsQ.ID = LabAnatomicLocationQ.LabID "
				" LEFT JOIN LabBiopsyTypeT ON LabsQ.BiopsyTypeID = LabBiopsyTypeT.ID "
				" WHERE LabsQ.Deleted = 0 AND %s", 
				((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
				GetAllowedLocationClause("LabsQ.LocationID"));
			return strSql;
		}
		break;

		

		case 577: //EMNs By Procedure
			/*Version History
				(a.walling 2006-09-26 12:27) - Created
				// (j.gruber 2007-01-04 11:31) - PLID 24035 - madeit support multiple providers
				// (j.gruber 2007-01-08 15:51) - PLID 24161 - add secondaryprovider names
				// (z.manning 2011-05-24 11:35) - PLID 33114 - Check EMR chart permissions
				// (j.jones 2011-07-05 17:49) - PLID 44432 - supported custom statuses
			*/
			return _T("SELECT DISTINCT"
				" EMRMasterT.ID AS EMNID,"
				" CASE WHEN EMRMasterT.Description = '' THEN '<No Description>' ELSE EMRMasterT.Description END AS EMNDescription, "
				" EMRStatusListT.Name AS Status,"
				" PatientsT.UserDefinedID AS UserDefinedID,"
				" EMRMasterT.PatientID AS PatID,"
				" COALESCE(PatientLast, PatientQ.Last) + ', ' + COALESCE(PatientFirst, PatientQ.First) + ' ' + COALESCE(PatientMiddle, PatientQ.Middle) AS PatientName,"
				"dbo.GetEMNProviderList(EMRMasterT.ID) AS DocName, "
				" EMRGroupsT.ID AS EMRID,"
				" EMRGroupsT.Description AS EMRDescription,"
				" EMRMasterT.InputDate AS EMNInputDate,"
				" EMRMasterT.Date AS EMNDate,"
				" EMRMasterT.ModifiedDate AS EMNLastModifiedDate,"
				" LocationsT.Name AS Location,"
				" LocationsT.ID AS LocID,"
				" EMRProcedureT.ProcedureID AS ProcedureID,"
				" ProcedureT.Name AS ProcedureName,"
				" ProcedureT.OfficialName AS ProcedureOfficialName,"
				" EMRMasterT.PatientAge AS PatientAge,"
				" CASE WHEN EMRMasterT.PatientGender = 2 THEN 'Female' WHEN EMRMasterT.PatientGender = 1 THEN 'Male' ELSE NULL END AS PatientGender, "
				" dbo.GetEMNSecondaryProviderList(EMRMasterT.ID) AS SecondaryDocName "
				" FROM EMRMasterT"
				" LEFT JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID"
				" LEFT JOIN PicT ON PicT.EMRGroupID = EMRGroupsT.ID"
				" LEFT JOIN EMRProcedureT ON EMRMasterT.ID = EMRProcedureT.EMRID"
				" LEFT JOIN ProcedureT ON EMRProcedureT.ProcedureID = ProcedureT.ID"
				" LEFT JOIN PersonT PatientQ ON EMRMasterT.PatientID = PatientQ.ID"
				" LEFT JOIN PatientsT ON EMRMasterT.PatientID = PatientsT.PersonID"
				" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID"
				" LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
				" LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
				" WHERE (EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND (PicT.IsCommitted = 1 OR PicT.IsCommitted IS NULL)) AND EMRProcedureT.ProcedureID IS NOT NULL "
				+ GetEmrChartPermissionFilter().Flatten());

			break;


		case 596:	//Sales Forecast / Pipeline (Internal Only)
			/*	Version History
				DRT 5/31/2007 - PLID 25892 - Created.
			*/
			return _T("SELECT OpportunitiesT.ID, OpportunitiesT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, OpportunitiesT.Name,  "
				"OpportunitiesT.TypeID, OpportunityTypesT.Name AS TypeName, OpportunitiesT.CategoryID, OpportunityCategoriesT.Name AS CategoryName,  "
				"OpportunitiesT.CurrentStageID, OpportunityStagesT.Name AS StageName, DateClosedQ.DateClosed,  "
				"CASE WHEN OpportunityProposalsT.ID IS NULL THEN OpportunitiesT.EstPrice ELSE OpportunityProposalsT.SavedTotal END AS Amount, OpportunitiesT.Notes,  "
				"PriCoordT.PersonID AS PriCoordID, PriCoordT.Username, OpportunityStagesT.DefaultPercent, OpportunitiesT.EstCloseDate AS EstCloseDate "
				" "
				"FROM OpportunitiesT "
				"LEFT JOIN (SELECT OpportunityID, MAX(ID) AS CurrentProposalID FROM OpportunityProposalsT GROUP BY OpportunityID) AS MaxProposalQ ON OpportunitiesT.ID = MaxProposalQ.OpportunityID "
				"LEFT JOIN OpportunityProposalsT ON MaxProposalQ.CurrentProposalID = OpportunityProposalsT.ID "
				"LEFT JOIN PersonT ON OpportunitiesT.PersonID = PersonT.ID "
				"LEFT JOIN OpportunityTypesT ON OpportunitiesT.TypeID = OpportunityTypesT.ID "
				"LEFT JOIN OpportunityCategoriesT ON OpportunitiesT.CategoryID = OpportunityCategoriesT.ID "
				"LEFT JOIN OpportunityStagesT ON OpportunitiesT.CurrentStageID = OpportunityStagesT.ID "
				"LEFT JOIN (SELECT OpportunityID, MIN(DateActive) AS DateClosed FROM OpportunityStageHistoryT WHERE StageID IN (7, 8, 9, 10) GROUP BY OpportunityID) AS DateClosedQ ON OpportunitiesT.ID = DateClosedQ.OpportunityID "
				"LEFT JOIN UsersT PriCoordT ON OpportunitiesT.PrimaryCoordID = PriCoordT.PersonID ");
			break;

		case 597:	//High Sales Opportunities (Internal Only)
			/*	Version History
				DRT 6/1/2007 - PLID 25892 - Created.
			*/
			return _T("SELECT OpportunitiesT.ID, OpportunitiesT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, OpportunitiesT.Name,  "
				"OpportunitiesT.TypeID, OpportunityTypesT.Name AS TypeName, OpportunitiesT.CategoryID AS CategoryID, OpportunityCategoriesT.Name AS CategoryName,  "
				"OpportunitiesT.CurrentStageID, OpportunityStagesT.Name AS StageName, DateClosedQ.DateClosed,  "
				"CASE WHEN OpportunityProposalsT.ID IS NULL THEN OpportunitiesT.EstPrice ELSE OpportunityProposalsT.SavedTotal END AS Amount, OpportunitiesT.Notes,  "
				"OpportunityStagesT.DefaultPercent, OpportunitiesT.EstCloseDate AS EstCloseDate, dbo.GetOpportunityAssociatesString(OpportunitiesT.ID) AS Associates,  "
				"dbo.GetOpportunityAllianceString(OpportunitiesT.ID) AS Alliance, dbo.GetOpportunityCompetitionString(OpportunitiesT.ID) AS Competition,  "
				"PriCoordT.PersonID AS PriCoordID, PriCoordT.Username AS PriCoordinator, SecCoordT.PersonID AS SecCoordID, SecCoordT.Username AS SecCoordinator "
				" "
				"FROM OpportunitiesT "
				"LEFT JOIN (SELECT OpportunityID, MAX(ID) AS CurrentProposalID FROM OpportunityProposalsT GROUP BY OpportunityID) AS MaxProposalQ ON OpportunitiesT.ID = MaxProposalQ.OpportunityID "
				"LEFT JOIN OpportunityProposalsT ON MaxProposalQ.CurrentProposalID = OpportunityProposalsT.ID "
				"LEFT JOIN PersonT ON OpportunitiesT.PersonID = PersonT.ID "
				"LEFT JOIN OpportunityTypesT ON OpportunitiesT.TypeID = OpportunityTypesT.ID "
				"LEFT JOIN OpportunityCategoriesT ON OpportunitiesT.CategoryID = OpportunityCategoriesT.ID "
				"LEFT JOIN OpportunityStagesT ON OpportunitiesT.CurrentStageID = OpportunityStagesT.ID "
				"LEFT JOIN (SELECT OpportunityID, MIN(DateActive) AS DateClosed FROM OpportunityStageHistoryT WHERE StageID IN (7, 8, 9, 10) GROUP BY OpportunityID) AS DateClosedQ ON OpportunitiesT.ID = DateClosedQ.OpportunityID "
				"LEFT JOIN UsersT PriCoordT ON OpportunitiesT.PrimaryCoordID = PriCoordT.PersonID "
				"LEFT JOIN UsersT SecCoordT ON OpportunitiesT.SecondaryCoordID = SecCoordT.PersonID");
			break;

		case 598:	//Sales Revenue Charts (Internal Only)
			/*	Version History
				DRT 6/1/2007 - PLID 25892 - Created.
			*/
			return _T("SELECT OpportunitiesT.ID, OpportunitiesT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, OpportunitiesT.Name,  "
				"OpportunitiesT.TypeID, OpportunityTypesT.Name AS TypeName, OpportunitiesT.CategoryID, OpportunityCategoriesT.Name AS CategoryName,  "
				"OpportunitiesT.CurrentStageID, OpportunityStagesT.Name AS StageName, DateClosedQ.DateClosed AS CloseDate,  "
				"CASE WHEN OpportunityProposalsT.ID IS NULL THEN OpportunitiesT.EstPrice ELSE OpportunityProposalsT.SavedTotal END AS Amount, OpportunitiesT.Notes,  "
				"OpportunityStagesT.DefaultPercent, OpportunitiesT.EstCloseDate, dbo.GetOpportunityAssociatesString(OpportunitiesT.ID) AS Associates,  "
				"dbo.GetOpportunityAllianceString(OpportunitiesT.ID) AS Alliance, dbo.GetOpportunityCompetitionString(OpportunitiesT.ID) AS Competition,  "
				"PriCoordT.PersonID AS PriCoordID, PriCoordT.Username AS PriCoordinator, SecCoordT.PersonID AS SecCoordID, SecCoordT.Username AS SecCoordinator "
				" "
				"FROM OpportunitiesT "
				"LEFT JOIN (SELECT OpportunityID, MAX(ID) AS CurrentProposalID FROM OpportunityProposalsT GROUP BY OpportunityID) AS MaxProposalQ ON OpportunitiesT.ID = MaxProposalQ.OpportunityID "
				"LEFT JOIN OpportunityProposalsT ON MaxProposalQ.CurrentProposalID = OpportunityProposalsT.ID "
				"LEFT JOIN PersonT ON OpportunitiesT.PersonID = PersonT.ID "
				"LEFT JOIN OpportunityTypesT ON OpportunitiesT.TypeID = OpportunityTypesT.ID "
				"LEFT JOIN OpportunityCategoriesT ON OpportunitiesT.CategoryID = OpportunityCategoriesT.ID "
				"LEFT JOIN OpportunityStagesT ON OpportunitiesT.CurrentStageID = OpportunityStagesT.ID "
				"LEFT JOIN (SELECT OpportunityID, MIN(DateActive) AS DateClosed FROM OpportunityStageHistoryT WHERE StageID IN (7, 8, 9, 10) GROUP BY OpportunityID) AS DateClosedQ ON OpportunitiesT.ID = DateClosedQ.OpportunityID "
				"LEFT JOIN UsersT PriCoordT ON OpportunitiesT.PrimaryCoordID = PriCoordT.PersonID "
				"LEFT JOIN UsersT SecCoordT ON OpportunitiesT.SecondaryCoordID = SecCoordT.PersonID "
				" "
				"/*TODO:  Are 7/8 considered 'won'?  they are not in the list view*/ "
				"WHERE OpportunitiesT.CurrentStageID IN (7, 8, 9, 10)");
			break;

		case 599:	//Sales Quarterly Targets (Internal Only)
			/*	Version History
				DRT 6/25/2007 - PLID 25892 - Created.
			*/
			switch(nSubLevel) {
			case 1:
				switch(nSubRepNum) {
				case 0:
					//Summary per week query
					return _T("SELECT StartOfWeek AS StartOfWeek, COUNT(*) AS TotalSales "
						"FROM  "
						"(SELECT OpportunityStageHistoryT.DateActive AS ClosedDate,  "
						"/* Get the date of the Sunday before our event */ "
						"convert(datetime, convert(nvarchar, dateadd(d, -1 * datepart(dw, OpportunityStageHistoryT.DateActive) + 1, OpportunityStageHistoryT.DateActive), 101)) AS StartOfWeek "
						" "
						"FROM OpportunitiesT "
						"INNER JOIN OpportunityProposalsT ON OpportunitiesT.ActiveProposalID = OpportunityProposalsT.ID "
						"/* To get the date it 'closed', we look for the first history entry which is of one of our 'closed' types */ "
						"LEFT JOIN (SELECT OpportunityID, MIN(ID) AS ID FROM OpportunityStageHistoryT WHERE StageID IN (7, 8, 9, 10) GROUP BY OpportunityID) MinHistoryQ ON OpportunitiesT.ID = MinHistoryQ.OpportunityID "
						"LEFT JOIN OpportunityStageHistoryT ON MinHistoryQ.ID = OpportunityStageHistoryT.ID "
						" "
						"WHERE OpportunitiesT.CurrentStageID IN (7, 8, 9, 10) "
						"/* Only current quarter */ "
						"AND Year(OpportunityStageHistoryT.DateActive) = Year(getdate()) AND DatePart(q, OpportunityStageHistoryT.DateActive) = DatePart(q, getdate()) "
						") SubQ "
						"GROUP BY StartOfWeek");
					break;
				default:
					//This is not legit
					return _T("");
					break;
				}
			default:
				//This is the main report query
				return _T(
					"SELECT PrimaryCoordID, RepAmount, Status, ClosedDate, Quota, Username, QtrID AS QtrID "
					"FROM "
					"(SELECT OpportunitiesT.PrimaryCoordID, OpportunityProposalsT.SavedTotal / (CASE WHEN OpportunitiesT.SecondaryCoordID IS NULL THEN 1 ELSE 2 END) AS RepAmount, "
					"CASE WHEN OpportunitiesT.CurrentStageID IN (7) THEN 1 ELSE 2 END AS Status, /* 1 = Log, 2 = Sale */ "
					"OpportunityStageHistoryT.DateActive AS ClosedDate, UserQuotasT.Quota, UsersT.Username, "
					"convert(int, convert(nvarchar, Year(OpportunityStageHistoryT.DateActive)) + convert(nvarchar, datepart(qq, OpportunityStageHistoryT.DateActive))) AS QtrID "
					" "
					"FROM OpportunitiesT "
					"INNER JOIN OpportunityProposalsT ON OpportunitiesT.ActiveProposalID = OpportunityProposalsT.ID "
					"INNER JOIN UsersT ON OpportunitiesT.PrimaryCoordID = UsersT.PersonID "
					"LEFT JOIN UserQuotasT ON UsersT.PersonID = UserQuotasT.UserID "
					"/* To get the date it 'closed', we look for the first history entry which is of one of our 'closed' types */ "
					"LEFT JOIN (SELECT OpportunityID, MIN(ID) AS ID FROM OpportunityStageHistoryT WHERE StageID IN (7, 8, 9, 10) GROUP BY OpportunityID) MinHistoryQ ON OpportunitiesT.ID = MinHistoryQ.OpportunityID "
					"LEFT JOIN OpportunityStageHistoryT ON MinHistoryQ.ID = OpportunityStageHistoryT.ID "
					" "
					"WHERE OpportunitiesT.CurrentStageID IN (7, 8, 9, 10) "
					/* Grab all the secondary coordinators for their half */
					"UNION SELECT OpportunitiesT.SecondaryCoordID, OpportunityProposalsT.SavedTotal / 2 AS RepAmount, "
					"CASE WHEN OpportunitiesT.CurrentStageID IN (7) THEN 1 ELSE 2 END AS Status, /* 1 = Log, 2 = Sale */ "
					"OpportunityStageHistoryT.DateActive AS ClosedDate, UserQuotasT.Quota, UsersT.Username, "
					"convert(int, convert(nvarchar, Year(OpportunityStageHistoryT.DateActive)) + convert(nvarchar, datepart(qq, OpportunityStageHistoryT.DateActive))) AS QtrID "
					""
					"FROM OpportunitiesT "
					"INNER JOIN OpportunityProposalsT ON OpportunitiesT.ActiveProposalID = OpportunityProposalsT.ID "
					"INNER JOIN UsersT ON OpportunitiesT.SecondaryCoordID = UsersT.PersonID "
					"LEFT JOIN UserQuotasT ON UsersT.PersonID = UserQuotasT.UserID "
					"/* To get the date it 'closed', we look for the first history entry which is of one of our 'closed' types */ "
					"LEFT JOIN (SELECT OpportunityID, MIN(ID) AS ID FROM OpportunityStageHistoryT WHERE StageID IN (7, 8, 9, 10) GROUP BY OpportunityID) MinHistoryQ ON OpportunitiesT.ID = MinHistoryQ.OpportunityID "
					"LEFT JOIN OpportunityStageHistoryT ON MinHistoryQ.ID = OpportunityStageHistoryT.ID "
					" "
					"WHERE OpportunitiesT.CurrentStageID IN (7, 8, 9, 10) AND OpportunitiesT.SecondaryCoordID IS NOT NULL "
					") MainSubQ ");
 					/* TODO:  ClosedDate.  This may need to be a field.  Is it closed when proposal signed?  Deposit?  full payment?  What if signed in q2, but no deposit until q3, or signed & deposit 
					  in q2 but no full pay until q3? */ 

					/* TODO:  when a deal is split, does the amount count half toward each target? */
				break;
			}

			return _T("");
			break;

		case 600:	//Sales YTD Summaries (Internal Only)
			/*	Version History
				DRT 6/26/2007 - PLID 25892 - Created.  This is the same main query as 599 except per year instead of per quarter
			*/
			return _T(
				"SELECT PrimaryCoordID, RepAmount, Status, ClosedDate, Quota, Username, YearID AS YearID "
				"FROM "
				"(SELECT OpportunitiesT.PrimaryCoordID, OpportunityProposalsT.SavedTotal / (CASE WHEN OpportunitiesT.SecondaryCoordID IS NULL THEN 1 ELSE 2 END) AS RepAmount, "
				"CASE WHEN OpportunitiesT.CurrentStageID IN (7) THEN 1 ELSE 2 END AS Status, /* 1 = Log, 2 = Sale */ "
				"OpportunityStageHistoryT.DateActive AS ClosedDate, UserQuotasT.Quota, UsersT.Username, "
				"Year(OpportunityStageHistoryT.DateActive) AS YearID "
				" "
				"FROM OpportunitiesT "
				"INNER JOIN OpportunityProposalsT ON OpportunitiesT.ActiveProposalID = OpportunityProposalsT.ID "
				"INNER JOIN UsersT ON OpportunitiesT.PrimaryCoordID = UsersT.PersonID "
				"LEFT JOIN (SELECT UserID, Year(BeginRange) AS Yr, SUM(Quota) AS Quota FROM UserQuotasT GROUP BY UserID, Year(BeginRange)) UserQuotasT  ON UsersT.PersonID = UserQuotasT.UserID "
				"/* To get the date it 'closed', we look for the first history entry which is of one of our 'closed' types */ "
				"LEFT JOIN (SELECT OpportunityID, MIN(ID) AS ID FROM OpportunityStageHistoryT WHERE StageID IN (7, 8, 9, 10) GROUP BY OpportunityID) MinHistoryQ ON OpportunitiesT.ID = MinHistoryQ.OpportunityID "
				"LEFT JOIN OpportunityStageHistoryT ON MinHistoryQ.ID = OpportunityStageHistoryT.ID "
				" "
				"WHERE OpportunitiesT.CurrentStageID IN (7, 8, 9, 10) "
				"/* Grab all the secondary coordinators for their half */ "
				"UNION SELECT OpportunitiesT.SecondaryCoordID, OpportunityProposalsT.SavedTotal / 2 AS RepAmount, "
				"CASE WHEN OpportunitiesT.CurrentStageID IN (7) THEN 1 ELSE 2 END AS Status, /* 1 = Log, 2 = Sale */ "
				"OpportunityStageHistoryT.DateActive AS ClosedDate, UserQuotasT.Quota, UsersT.Username, "
				"Year(OpportunityStageHistoryT.DateActive)YearID "
				" "
				"FROM OpportunitiesT "
				"INNER JOIN OpportunityProposalsT ON OpportunitiesT.ActiveProposalID = OpportunityProposalsT.ID "
				"INNER JOIN UsersT ON OpportunitiesT.SecondaryCoordID = UsersT.PersonID "
				"LEFT JOIN (SELECT UserID, Year(BeginRange) AS Yr, SUM(Quota) AS Quota FROM UserQuotasT GROUP BY UserID, Year(BeginRange)) UserQuotasT  ON UsersT.PersonID = UserQuotasT.UserID "
				"/* To get the date it 'closed', we look for the first history entry which is of one of our 'closed' types */ "
				"LEFT JOIN (SELECT OpportunityID, MIN(ID) AS ID FROM OpportunityStageHistoryT WHERE StageID IN (7, 8, 9, 10) GROUP BY OpportunityID) MinHistoryQ ON OpportunitiesT.ID = MinHistoryQ.OpportunityID "
				"LEFT JOIN OpportunityStageHistoryT ON MinHistoryQ.ID = OpportunityStageHistoryT.ID "
				" "
				"WHERE OpportunitiesT.CurrentStageID IN (7, 8, 9, 10) AND OpportunitiesT.SecondaryCoordID IS NOT NULL "
				") MainSubQ ");
			break;

		case 651:	//Problem List by Patient
		{
			/*	Version History
				(e.lally 2008-07-23) PLID 30732 - Created - This is the same as the Problem List (PP) (652) so we may want to combine them into
					//a global function unless we forsee having different field needs.
				(e.lally 2008-07-30) PLID 30732 - Added EMR ID, EMR Date, EMN ID, Topic ID fields 
				(e.lally 2008-08-06) PLID 30965 - Added diagnosis code description to the code number to reflect the changes to the dialog.
				// (j.jones 2009-05-26 08:49) - PLID 34151 - added problem diagnosis, chronicity, and supported multiple items per problem
				// (j.jones 2009-05-27 09:31) - PLID 34352 - supported lab problems
				// (j.jones 2009-06-02 09:47) - PLID 34441 - added ProblemID and RegardingTypeName
				// (j.jones 2014-03-26 14:04) - PLID 61560 - added ICD-10 support, which also removed and renamed the previous diagnosis fields
			*/
			CString strSql;
			strSql.Format(
				"SELECT "
				"PatientsT.PersonID AS PatID, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, \r\n"
				//determine what object was flagged as a problem and grab the corresponding joined field
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN EMRProblemGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRMasterGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicGroup.Description "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationGroup.Description "
				"ELSE '<None>' END AS EMRName, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN EMRProblemGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRMasterGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicGroup.ID "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationGroup.ID "
				"ELSE '' END AS EmrID, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN EMRProblemGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRMasterGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicGroup.InputDate "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationGroup.InputDate "
				"ELSE '' END AS EMRDate, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRProblemMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicMaster.Description "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationMaster.Description "
				"ELSE '<None>' END AS EMNName, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRProblemMaster.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicMaster.ID "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailMaster.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagMaster.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeMaster.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationMaster.ID "
				"ELSE '' END AS EmnID, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRProblemMaster.Date "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicMaster.Date "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailMaster.Date "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagMaster.Date "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeMaster.Date "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationMaster.Date "
				"ELSE NULL END AS EMNDate, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRProblemTopic.Name "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailTopic.Name "
					//Topics for Diag code, Service Code, and Medication are always More Info so we are ignoring them
				"ELSE '' END AS TopicName, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRProblemTopic.ID "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailTopic.ID "
					//Topics for Diag code, Service Code, and Medication are always More Info so we are ignoring them
				"ELSE '' END AS TopicID, \r\n"
				""
				//Get the name of whatever *it* is that was added as a problem.
				"CONVERT(NVARCHAR(2000), "
				//Labs use a special combination of data for their description
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*Lab*/ THEN COALESCE(LabsT.FormNumberTextID, '') + '-' + COALESCE(LabsT.Specimen, '') + ' - ' + CASE WHEN LabsT.Type = %li THEN COALESCE(LabAnatomyT.Description, '') ELSE LabsT.ToBeOrdered END "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Patient*/ THEN PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				//Since EMRs, EMNs, and Topics are just the name of those things themselves pick from one of those.
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN EMRProblemGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRProblemMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRProblemTopic.Name "
				""
				//Or get the diagnosis code, Charge description, or medication name.
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis*/ THEN "
				"	CASE WHEN EMRDiagCodes9.ID Is Not Null AND EMRDiagCodes10.ID Is Not Null THEN EMRDiagCodes10.CodeNumber + ' - ' + EMRDiagCodes10.CodeDesc + ' (' + EMRDiagCodes9.CodeNumber + ')' "
				"	ELSE Coalesce(EMRDiagCodes10.CodeNumber + ' - ' + EMRDiagCodes10.CodeDesc, EMRDiagCodes9.CodeNumber + ' - ' + EMRDiagCodes9.CodeDesc) END "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*service code*/ THEN EMRChargesT.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*medication*/ THEN DrugDataT.Data "
				//Details and Detail elements have that funky override field to worry about, otherwise grab their regular Name.
				"ELSE "
				"	CASE WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Item*/ OR EMRProblemLinkT.EMRRegardingType = %li /*data item*/) THEN "
				"		CASE WHEN EmrDetailsT.MergeOverride IS NULL THEN ("
				"			CASE WHEN EMRInfoT.ID = -27 THEN EmrDetailsT.MacroName "
				"			ELSE EMRInfoT.Name END) "
				"		ELSE EmrDetailsT.MergeOverride END "
				"	ELSE '' END "
				"END) AS ProblemItem, \r\n"

				"EmrProblemsT.Description AS ProblemDescription, "
				"EmrProblemsT.EnteredDate AS EnteredDate, "
				"EmrProblemsT.OnsetDate AS OnsetDate, "
				"EmrProblemStatusT.Name as Status, "
				"EmrProblemsT.ModifiedDate AS ModifiedDate, "
				"UsersT.Username AS UserModifiedBy, "
				"ICD9T.CodeNumber as ICD9Code, "				
				"ICD10T.CodeNumber as ICD10Code, "
				"ICD9T.CodeDesc as ICD9CodeDesc, "
				"ICD10T.CodeDesc as ICD10CodeDesc, "
				"EMRProblemChronicityT.Name AS Chronicity, "
				"EMRProblemsT.ID AS ProblemID, "

				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN 'EMR' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN 'EMN' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN 'Topic' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ THEN 'EMR Item' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/ THEN 'EMR List Item' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN 'Diagnosis Code' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN 'Charge' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN 'Medication' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Patient*/ THEN 'Patient' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Lab*/ THEN 'Lab' "
				"ELSE '' END AS RegardingTypeName \r\n"

				""
				"FROM EMRProblemsT  "
				//if multiple links exist, the problem will show on the report multiple times
				"INNER JOIN EMRProblemLinkT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID "
				"INNER JOIN PatientsT ON EMRProblemsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				//Find the most recent modified date for each problem ID
				"LEFT JOIN  (SELECT ProblemID, Max(Date) AS Date FROM EMRProblemHistoryT GROUP BY ProblemID) Max_EMRProblemHistoryT "
				"	ON EMRPRoblemsT.ID = Max_EMRProblemHistoryT.ProblemID "
				//rejoin the regular problem history table using the max modified date for each problem to get the rest of the fields.
				"LEFT JOIN EMRProblemHistoryT ON Max_EMRProblemHistoryT.Date = EMRProblemHistoryT.Date "
				"	AND Max_EMRProblemHistoryT.ProblemID = EMRProblemHistoryT.ProblemID "
				"LEFT JOIN EMRProblemStatusT ON EMRProblemHistoryT.StatusID = EMRProblemStatusT.ID "
				"LEFT JOIN EmrDetailsT ON EMRProblemLinkT.EMRRegardingID = EmrDetailsT.ID "
				"LEFT JOIN EMRInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID "
				"LEFT JOIN UsersT ON EMRProblemHistoryT.UserID = UsersT.PersonID "

				//If the Detail was flagged as a problem, we need to link the EMR/EMN on its ID
				"LEFT JOIN EMRTopicsT AS EMRDetailTopic ON EMRDetailsT.EMRTopicID = EMRDetailTopic.ID "
				"LEFT JOIN EMRMasterT AS EMRDetailMaster ON EMRDetailsT.EMRID = EMRDetailMaster.ID "
				"LEFT JOIN EMRGroupsT AS EMRDetailGroup ON EMRDetailMaster.EMRGroupID = EMRDetailGroup.ID "
				

				//If the Topic was flagged as a problem, we need to link the EMR/EMN on its ID
				"LEFT JOIN EMRTopicsT AS EMRProblemTopic ON EMRProblemLinkT.EMRRegardingID = EMRProblemTopic.ID "
				"LEFT JOIN EMRMasterT AS EMRTopicMaster ON EMRProblemTopic.EMRID = EMRTopicMaster.ID "
				"LEFT JOIN EMRGroupsT AS EMRTopicGroup ON EMRTopicMaster.EMRGroupID = EMRTopicGroup.ID "
				""
				//If the EMN was flagged as a problem, we need to link the EMR on its ID
				"LEFT JOIN EMRMasterT AS EMRProblemMaster ON EMRProblemLinkT.EMRRegardingID = EMRProblemMaster.ID "
				"LEFT JOIN EMRGroupsT AS EMRMasterGroup ON EMRProblemMaster.EMRGroupID = EMRMasterGroup.ID "
				
				//If the EMR was flagged as a problem, we need to link nothing (just itself)
				"LEFT JOIN EMRGroupsT AS EMRProblemGroup ON EMRProblemLinkT.EMRRegardingID = EMRProblemGroup.ID "
				
				//If the Diagnosis code was flagged as a problem, we need to link the EMR/EMN on its ID 
					//(we don't care about the more info topic)
				"LEFT JOIN EMRDiagCodesT ON EMRProblemLinkT.EMRRegardingID = EMRDiagCodesT.ID "
				"LEFT JOIN DiagCodes EMRDiagCodes9 ON EMRDiagCodes9.ID = EMRDiagCodesT.DiagCodeID "
				"LEFT JOIN DiagCodes EMRDiagCodes10 ON EMRDiagCodes10.ID = EMRDiagCodesT.DiagCodeID_ICD10 "
				"LEFT JOIN EMRMasterT AS EMRDiagMaster ON EMRDiagMaster.ID = EMRDiagCodesT.EMRID "
				"LEFT JOIN EMRGroupsT AS EMRDiagGroup ON EMRDiagGroup.ID = EMRDiagMaster.EMRGroupID "
				
				//If the service code was flagged as a problem, we need to link the EMR/EMN on its ID 
					//(we don't care about the more info topic)
				"LEFT JOIN EMRChargesT ON EMRProblemLinkT.EMRRegardingID = EMRChargesT.ID "
				"LEFT JOIN EMRMasterT AS EMRChargeMaster ON EMRChargeMaster.ID = EMRChargesT.EMRID "
				"LEFT JOIN EMRGroupsT AS EMRChargeGroup ON EMRChargeGroup.ID = EMRChargeMaster.EMRGroupID "
				
				//If the Medication was flagged as a problem, we need to link the EMR/EMN on its ID 
					//(we don't care about the more info topic)
				"LEFT JOIN EMRMedicationsT ON EMRProblemLinkT.EMRRegardingID = EMRMedicationsT.MedicationID "
				"LEFT JOIN PatientMedications ON EMRMedicationsT.MedicationID = PatientMedications.ID "
				"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				"LEFT JOIN EMRDataT AS DrugDataT ON DrugList.EMRDataID = DrugDataT.ID "
				"LEFT JOIN EMRMasterT AS EMRMedicationMaster ON EMRMedicationMaster.ID = EMRMedicationsT.EMRID "
				"LEFT JOIN EMRGroupsT AS EMRMedicationGroup ON EMRMedicationGroup.ID = EMRMedicationMaster.EMRGroupID "
				""
				//If a lab is flagged as a problem, link LabsT on its ID
				"LEFT JOIN LabsT ON EMRProblemLinkT.EMRRegardingID = LabsT.ID "
				"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID "
				""
				//a problem itself can have a diagnosis code
				"LEFT JOIN DiagCodes ICD9T ON EMRProblemsT.DiagCodeID = ICD9T.ID "
				"LEFT JOIN DiagCodes ICD10T ON EMRProblemsT.DiagCodeID_ICD10 = ICD10T.ID "
				""
				"LEFT JOIN EMRProblemChronicityT ON EMRProblemsT.ChronicityID = EMRProblemChronicityT.ID "
				""
				"WHERE EMRProblemsT.Deleted = 0 ", 
				eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrTopic, eprtEmrItem, eprtEmrDataItem,
				eprtEmrTopic, eprtEmrItem, eprtEmrDataItem,
				eprtLab, ltBiopsy, eprtUnassigned, eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtEmrItem, eprtEmrDataItem,
				eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtUnassigned, eprtLab);
			return _T(strSql);
			}
			break;


			case 689:	//EMN Charges and Diagnosis Codes By EMN
				/* Version History
				// (j.gruber 2010-01-18 12:01) - PLID 34166 - created
					(d.thompson 2010-02-18) - PLID 37237 - Added secondary EMR provider info, patient marital status
					// (j.gruber 2010-02-22 13:34) - PLID 37467 - added resource and payments total
					// (j.gruber 2010-03-09 14:37) - PLID 37690 - additional fields
					// (j.gruber 2010-04-19 10:45) - PLID 38089 - new sub report for payments
					// (j.gruber 2010-05-17 13:10) - PLID 38695 - add if patient name or address changed same day as EMN
					// (j.gruber 2010-05-17 15:07) - PLID 38696 - added payment plan information
					// (j.gruber 2010-08-04 11:15) - PLID 39963 - change copay structure
					// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
					// (j.gruber 2011-02-07 13:55) - PLID 41865 - Added EMN More Info Note and G1 Note
					// (j.jones 2011-04-11 11:48) - PLID 43224 - exposed CPTCodeT.Billable
					// (r.farnworth 2013-08-19 11:36) - PLID 58076 - Removed all subqueries from the select statement to reduce slowness in report
					// (r.farnworth 2014-01-23 15:41) - PLID 60449 - Remove the AddressChanged Field to reduce slowness
					// (r.gonet 03/31/2014) - PLID 61452 - Removed CodeNumber from SubReport 2. Added ICD9Code and ICD10Code to SubReport 2.
					//                                   - Removed CodeNumber and CodeDesc from SubReport 3. Added ICD9Code, ICD10Code, ICD9CodeDesc, and ICD10CodeDesc to SubReport 3.
				*/

				switch (nSubLevel) {

					case 0:  //main report

						strSQL.Format(
						"SELECT PatientsT.UserDefinedID, PersonT.ID as PatID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.ZIp, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s) as SocSecNum, PersonT.Gender, "
						" PersonT.EmergFirst, PersonT.EmergLast, PersonT.EmergHPhone, PersonT.EmergWPhone, PersonT.EmergRelation, "
						" PatientsT.Occupation, PatientsT.EmployerFirst, PatientsT.EmployerMiddle, PatientsT.EmployerLast, PatientsT.EmployerAddress1, "
						" PatientsT.EmployerAddress2, PatientsT.EmployerCity, PatientsT.EmployerState, PatientsT.EmployerZip,  "
						" RefPhysPersonT.First as RefPhysFirst, RefPhysPersonT.Middle as RefPhysMiddle, RefPhysPersonT.Last as RefPhysLast, RefPhysPersonT.Title as RefPhysTitle,  "
						" RefPhysPersonT.Address1 as RefPhysAddress1, RefPhysPersonT.Address2 as RefPhysAddress2, RefPhysPersonT.City as RefPhysCity, RefPhysPersonT.State as RefPhysState, RefPhysPersonT.Zip as RefPhysZip, RefPhysPersonT.WorkPhone as RefPhysWorkPhone, ReferringPhysT.NPI as RefPhysNPI,  "
						" PriInsCoT.Name as PriInsCoName,  "
						" PriInsCoPersonT.Address1 as PriInsCoAdd1, PriInsCoPersonT.Address2 as PriInsCoAdd2, PriInsCoPersonT.City as PriInsCoCity, PriInsCoPersonT.State as PriInsCoState, PriInsCoPersonT.Zip as PriInsCoZip,    "
						" PriInsPersonT.First as PriInsFirst, PriInsPersonT.Middle as PriInsMiddle, PriInsPersonT.Last as PriInsLast, PriInsPersonT.Address1 as PriInsAdd1, PriInsPersonT.Address2 as PriInsAdd2, PriInsPersonT.City as PriInsCity, PriInsPersonT.State as PriInsState, PriInsPersonT.Zip as PriInsZip, "
						" PriInsPartyT.Employer as PriInsEmployer, "
						" PriInsPersonT.BirthDate as PriInsBirthdate, dbo.MaskSSN(PriInsPersonT.SocialSecurity, %s) as PriInsSSN, PriInsPartyT.RelationtoPatient as PriInsRelation, "
						" PriInsPartyT.PersonID as PriInsPartyID, PriInsPartyT.IDForInsurance as PriIDForInsurance, PriInsPartyT.PolicyGroupNum as PriPolGroupNum, PriInsPartyT.Copay as PriCopay, PriInsPartyT.CopayPercent as PriCopayPercent,  "
						" PriInsPartyT.EffectiveDate as PriInsEffDate, "
						" ISNULL(PriCountT.PriCountRefs, 0) AS PriCountRefs, "
						" SecInsCoT.Name as SecInsCoName,  "
						" SecInsCoPersonT.Address1 as SecInsCoAdd1, SecInsCoPersonT.Address2 as SecInsCoAdd2, SecInsCoPersonT.City as SecInsCoCity, SecInsCoPersonT.State as SecInsCoState, SecInsCoPersonT.Zip as SecInsCoZip,    "
						" SecInsPersonT.First as SecInsFirst, SecInsPersonT.Middle as SecInsMiddle, SecInsPersonT.Last as SecInsLast, SecInsPersonT.Address1 as SecInsAdd1, SecInsPersonT.Address2 as SecInsAdd2, SecInsPersonT.City as SecInsCity, SecInsPersonT.State as SecInsState, SecInsPersonT.Zip as SecInsZip, "
						" SecInsPartyT.Employer as SecInsEmployer, "
						" SecInsPersonT.BirthDate as SecInsBirthdate, dbo.MaskSSN(SecInsPersonT.SocialSecurity, %s) as SecInsSSN, SecInsPartyT.RelationtoPatient as SecInsRelation, "
						" SecInsPartyT.PersonID as SecInsPartyID, SecInsPartyT.IDForInsurance as SecIDForInsurance, SecInsPartyT.PolicyGroupNum as SecPolGroupNum, SecInsPartyT.Copay as SecCopay, SecInsPartyT.CopayPercent as SecCoPayPercent, "
						" SecInsPartyT.EffectiveDate as SecInsEffDate, "
						" ISNULL(SecCountT.SecCountRefs, 0) AS SecCountRefs, "
						" RespPartyT.First as RespPartyFirst, RespPartyT.Middle as RespPartyMiddle, RespPartyT.Last as RespPartyLast, RespPartyT.Address1 as RespPartyAddress1, RespPartyT.Address2 as RespPartyAddress2, RespPartyT.City as RespPartyCity, RespPartyT.State as RespPartyState, RespPartyT.Zip as RespPartyZip, "
						" RespPartyT.HomePhone as RespPartyHomePhone, ResponsiblePartyT.Employer as RespPartyEmployer, "
						" EMRMasterT.ID as EMRID, EMRMasterT.Date as Date, EMRMasterT.Description as EMRDescription, EMRMasterT.InputDate as IDate, "						
						" ISNULL(EMRChargeCountT.ChargeCount, 0) AS ChargeCount, "
						" ISNULL(EMRDiagCountT.DiagCount, 0) AS DiagCount, "
						" EMRMasterT.LocationID as LocID, EMRProvidersT.ProviderID as ProvID, "
						" EmrProvPersonT.First as DocFirst, EmrProvPersonT.Middle as DocMiddle, EmrProvPersonT.Last as DocLast, EmrProvPersonT.Title as DocTitle, "
						" EMRSecondaryProvidersT.ProviderID as SecProvID, PatientsT.MaritalStatus, "
						" EmrSecondaryProvPersonT.First as SecDocFirst, EmrSecondaryProvPersonT.Middle as SecDocMiddle, EmrSecondaryProvPersonT.Last as SecDocLast, EmrSecondaryProvPersonT.Title as SecDocTitle, "
						" ResourceIDT.Resource, "
						" TotalPaysT.TotalPays, "
						" EMRMasterT.AdditionalNotes as EMNMoreInfoNote, PersonT.Note as PatientNote, "
						" PersonT.Email, PatientsT.DefaultInjuryDate, "
						"CASE WHEN PreferredContact = 0 THEN '<No Preference>' ELSE "
						" CASE WHEN PreferredContact = 1 THEN 'Home Phone' ELSE "
						" CASE WHEN PreferredContact = 2 THEN 'Work Phone' ELSE "
						" CASE WHEN PreferredContact = 3 THEN 'Mobile Phone' ELSE "
						" CASE WHEN PreferredContact = 4 THEN 'Pager' ELSE "
						" CASE WHEN PreferredContact = 5 THEN 'Other Phone' ELSE "
						" CASE WHEN PreferredContact = 6 THEN 'Email' ELSE "
						" CASE WHEN PreferredContact = 7 THEN 'Text Messaging' ELSE '' "
						" END END END END END END END END AS PreferredContact, " 
						" PersonT.PrivHome, PersonT.PrivWork, PersonT.PrivCell, PersonT.PrivEmail, "
						" LastSurgeryT.LastSurgeryDate, "
						" PCPPersonT.First as PCPFirst, PCPPersonT.Middle as PCPMiddle, PCPPersonT.Last as PCPLast, PCPPersonT.Title as PCPTitle,  "
						" TerInsCoT.Name as TerInsCoName,  "
						" TerInsCoPersonT.Address1 as TerInsCoAdd1, TerInsCoPersonT.Address2 as TerInsCoAdd2, TerInsCoPersonT.City as TerInsCoCity, TerInsCoPersonT.State as TerInsCoState, TerInsCoPersonT.Zip as TerInsCoZip,    "
						" TerInsPersonT.First as TerInsFirst, TerInsPersonT.Middle as TerInsMiddle, TerInsPersonT.Last as TerInsLast, TerInsPersonT.Address1 as TerInsAdd1, TerInsPersonT.Address2 as TerInsAdd2, TerInsPersonT.City as TerInsCity, TerInsPersonT.State as TerInsState, TerInsPersonT.Zip as TerInsZip, "
						" TerInsPartyT.Employer as TerInsEmployer, "
						" TerInsPersonT.BirthDate as TerInsBirthdate, dbo.MaskSSN(TerInsPersonT.SocialSecurity, %s) as TerInsSSN, TerInsPartyT.RelationtoPatient as TerInsRelation, "
						" TerInsPartyT.PersonID as TerInsPartyID, TerInsPartyT.IDForInsurance as TerIDForInsurance, TerInsPartyT.PolicyGroupNum as TerPolGroupNum, TerInsPartyT.Copay as TerCopay, TerInsPartyT.CopayPercent as TerCoPayPercent, "
						" TerInsPartyT.EffectiveDate as TerInsEffDate "						
						" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						" LEFT JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "
						" LEFT JOIN PersonT RefPhysPersonT ON ReferringPhysT.PersonID = RefPhysPersonT.ID "
						" LEFT JOIN (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay, PayGroupID FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage, ServicePayGroupsT.ID As PayGroupID FROM "
						"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
						" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
						" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID ) PriInsPartyT ON PatientsT.PersonID = PriInsPartyT.PatientID and PriInsPartyT.RespTypeID = 1 "
						" LEFT JOIN PersonT PriInsPersonT ON PriInsPartyT.PersonID = PriInsPersonT.ID "
						" LEFT JOIN (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay, PayGroupID FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage, ServicePayGroupsT.ID As PayGroupID FROM "
						"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
						" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
						" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID )SecInsPartyT ON PatientsT.PersonID = SecInsPartyT.PatientID and SecInsPartyT.RespTypeID = 2 "
						" LEFT JOIN PersonT SecInsPersonT ON SecInsPartyT.PersonID = SecInsPersonT.ID "
						" LEFT JOIN InsuranceCoT PriInsCoT ON PriInsPartyT.InsuranceCoID = PriInsCoT.PersonID "
						" LEFT JOIN PersonT PriInsCoPersonT ON PriInsCoT.PersonID = PriInsCoPersonT.ID "						
						" LEFT JOIN InsuranceCoT SecInsCoT ON SecInsPartyT.InsuranceCoID = SecInsCoT.PersonID "
						" LEFT JOIN PersonT SecInsCoPersonT ON SecInsCoT.PersonID = SecInsCoPersonT.ID "						
						" LEFT JOIN ResponsiblePartyT ON PatientsT.PrimaryRespPartyID = ResponsiblePartyT.PersonID "
						" LEFT JOIN PersonT RespPartyT ON ResponsiblePartyT.PersonID = RespPartyT.ID "
						" LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID "						
						" LEFT JOIN (SELECT * FROM EMRProvidersT WHERE Deleted = 0) EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID "
						" LEFT JOIN PersonT EmrProvPersonT ON EMRProvidersT.ProviderID = EMRProvPersonT.ID "						
						" LEFT JOIN (SELECT * FROM EMRSecondaryProvidersT WHERE Deleted = 0) EMRSecondaryProvidersT ON EMRMasterT.ID = EMRSecondaryProvidersT.EMRID "
						" LEFT JOIN PersonT EmrSecondaryProvPersonT ON EMRSecondaryProvidersT.ProviderID = EmrSecondaryProvPersonT.ID "						
						" LEFT JOIN ReferringPhysT PCPT ON PatientsT.PCP = PCPT.PersonID "
						" LEFT JOIN PersonT PCPPersonT ON PCPT.PersonID = PCPPersonT.ID "					
						" LEFT JOIN " 
						"(SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay, PayGroupID FROM InsuredPartyT LEFT JOIN "
								"(SELECT InsuredPartyID, CopayMoney, CopayPercentage, ServicePayGroupsT.ID As PayGroupID FROM "
									"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
								" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
							" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
							" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
							" WHERE RespTypeT.Priority = 3 )TerInsPartyT "
						" ON PatientsT.PersonID = TerInsPartyT.PatientID "
						" LEFT JOIN PersonT TerInsPersonT ON TerInsPartyT.PersonID = TerInsPersonT.ID "
						" LEFT JOIN InsuranceCoT TerInsCoT ON TerInsPartyT.InsuranceCoID = TerInsCoT.PersonID "
						" LEFT JOIN PersonT TerInsCoPersonT ON TerInsCoT.PersonID = TerInsCoPersonT.ID "
						" LEFT JOIN "
						" (SELECT InsuredPartyID, Count(ID) AS 'PriCountRefs' FROM InsuranceReferralsT GROUP BY InsuredPartyID) PriCountT ON PriCountT.InsuredPartyID = PriInsPartyT.PersonID "
						" LEFT JOIN "
						" (SELECT InsuredPartyID, Count(ID) AS 'SecCountRefs' FROM InsuranceReferralsT GROUP BY InsuredPartyID) SecCountT ON SecCountT.InsuredPartyID = SecInsPartyT.PersonID "
						" LEFT JOIN "
						" (SELECT EMRID, Count(ID) AS 'ChargeCount' FROM EMRChargesT WHERE Deleted = 0 GROUP BY EMRID) EMRChargeCountT ON EMRChargeCountT.EMRID = EmrMasterT.ID "
						" LEFT JOIN "
						" (SELECT EMRID, Count(ID) AS 'DiagCount' FROM EMRDiagCodesT WHERE  Deleted = 0 GROUP BY EMRID) EMRDiagCountT ON EMRDiagCountT.EMRID = EmrMasterT.ID "
						" LEFT JOIN "
						" (SELECT dbo.GetResourceString(FirstID) AS 'Resource', InnerID, Date FROM "
						" (SELECT MAX(ID) AS 'FirstID', InnerID, AppointmentsT.Date FROM AppointmentsT "
						" INNER JOIN (SELECT PatientID as 'InnerID', DATE,  MIN(StartTime) AS 'MinStartTime' FROM AppointmentsT "
						" WHERE STATUS <> 4 GROUP BY Date, PatientID ) Q "
						" ON Q.InnerID = AppointmentsT.PatientID AND Q.Date = AppointmentsT.Date AND Q.MinStartTime = AppointmentsT.StartTime GROUP BY InnerID, AppointmentsT.Date ) P "
						" )ResourceIDT on ResourceIDT.InnerID = EMRMasterT.PatientID "
						" AND CONVERT(DATETIME, CONVERT(NVARCHAR, ResourceIDT.Date, 23)) = CONVERT(DATETIME, CONVERT(NVARCHAR, EMRMasterT.DATE, 23)) "
						" LEFT JOIN ( "
						" SELECT SUM(LineItemT.amount) AS 'TotalPays', Date, PatientID FROM LineItemT WHERE Type = 1 AND Deleted = 0 GROUP BY PatientID, Date ) TotalPaysT "
						" ON TotalPaysT.PatientID = EMRMasterT.PatientID AND "
						" CONVERT(DATETIME, CONVERT(NVARCHAR, TotalPaysT.DATE, 23)) = CONVERT(DATETIME, CONVERT(NVARCHAR, EMRMasterT.DATE, 23)) "
						" LEFT JOIN ( "
						" SELECT  EMRMasterT.ID, Max(StartTime) AS 'LastSurgeryDate' FROM AppointmentsT "
						" LEFT JOIN EMRMasterT ON AppointmentsT.PatientID = EMRMasterT.PatientID "
						" WHERE AppointmentsT.Status <> 4 AND ShowState <> 3 "
						" AND AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category IN (3,4,6)) "
						" AND AppointmentsT.DATE <= EMRMasterT.DATE GROUP BY EMRMasterT.ID "
						" )LastSurgeryT ON LastSurgeryT.ID = EMRMasterT.ID "
						" WHERE  "
						" CurrentStatus <> 4 AND EMRMasterT.Deleted = 0 AND (EMRMasterT.ID IN (SELECT EMRID FROM EMRChargesT WHERE DELETED = 0) OR EMRMasterT.ID IN (SELECT EMRID FROM EMRDiagCodesT WHERE DELETED = 0)) ",
						((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
						((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
						((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
						((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1")); 

						return _T(strSQL);
				break;

				case 1: //EMN Charges

					switch (nSubRepNum) {
					case 0:  //primary insurance referrals
						return _T("SELECT PersonT.ID as PatID, "						
							" EMRMasterT.ID AS EMRID, EMRMasterT.Date as Date, EMRMasterT.InputDate as IDate, "
							" EMRProvidersT.ProviderID as ProvID, EMRMasterT.LocationID as LocID, "
							" PriInsPartyT.PersonID as PriInsPartyID, PriInsRefT.ID as PriInsRefID, PriInsRefT.AuthNum as PriInsRefAuthNu, PriInsRefT.StartDate as PriInsRefStartDate, PriInsRefT.EndDate as PriInsRefEndDate, PriInsRefT.NumVisits as PriInsRefNumVisits, PriInsRefLocationT.Name as PriInsRefLocation,  "
							" PriInsRefProvT.First as PriInsRefDocFirst, PriInsRefProvT.Middle as PriInsRefDocMiddle, PriInsRefProvT.Last as PriInsRefDocLast, PriInsRefProvT.Title as PriInsRefDocTitle, PriInsRefT.Comments as PriInsRefComments, "
							" PriInsRefDiagCodes.CodeNumber as PriInsRefDiagCode, PriInsRefCPTT.Code as PriInsRefServiceCode, "						
							" Coalesce(PriInsRefCPTT.Billable, 1) AS PriInsRefServiceCodeBillable "
							" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
							" LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID "
							" LEFT JOIN (SELECT * FROM EMRProvidersT WHERE Deleted = 0) EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID "
							" LEFT JOIN InsuredPartyT PriInsPartyT ON PatientsT.PersonID = PriInsPartyT.PatientID and PriInsPartyT.RespTypeID = 1 "
							" LEFT JOIN PersonT PriInsPersonT ON PriInsPartyT.PersonID = PriInsPersonT.ID "
							" LEFT JOIN InsuranceReferralsT PriInsRefT ON PriInsPartyT.PersonID = PriInsRefT.InsuredPartyID "
							" LEFT JOIN PersonT PriInsRefProvT ON PriInsRefT.ProviderID = PriInsRefProvT.ID "
							" LEFT JOIN LocationsT PriInsRefLocationT ON PriInsRefT.LocationID = PriInsRefLocationT.ID "
							" LEFT JOIN InsuranceReferralCPTCodesT PriInsRefCPTCodesT ON PriInsRefT.ID = PriInsRefCPTCodesT.ReferralID "
							" LEFT JOIN CPTCodeT PriInsRefCPTT ON PriInsRefCPTCodesT.ServiceID = PriInsRefCPTT.ID "
							" LEFT JOIN InsuranceReferralDiagsT PriInsRefDiagT ON PriInsRefT.ID = PriInsRefDiagT.ReferralID "
							" LEFT JOIN DiagCodes PriInsRefDiagCodes ON PriInsRefDiagT.DiagID = PriInsRefDiagCodes.ID "
							" WHERE  "
							" CurrentStatus <> 4 AND EMRMasterT.Deleted = 0 "); 
					break;
					case 1:  //secondary insurance referrals
						return _T("SELECT PersonT.ID as PatID, "						
							" EMRMasterT.ID AS EMRID, EMRMasterT.Date as Date, EMRMasterT.InputDate as IDate, "
							" EMRProvidersT.ProviderID as ProvID, EMRMasterT.LocationID as LocID, "
							" SecInsPartyT.PersonID as SecInsPartyID, SecInsRefT.ID as SecInsRefID, SecInsRefT.AuthNum as SecInsRefAuthNu, SecInsRefT.StartDate as SecInsRefStartDate, SecInsRefT.EndDate as SecInsRefEndDate, SecInsRefT.NumVisits as SecInsRefNumVisits, SecInsRefLocationT.Name as SecInsRefLocation,  "
							" SecInsRefProvT.First as SecInsRefDocFirst, SecInsRefProvT.Middle as SecInsRefDocMiddle, SecInsRefProvT.Last as SecInsRefDocLast, SecInsRefProvT.Title as SecInsRefDocTitle, SecInsRefT.Comments as SecInsRefComments, "
							" SecInsRefDiagCodes.CodeNumber as SecInsRefDiagCode, SecInsRefCPTT.Code as SecInsRefServiceCode, "
							" Coalesce(SecInsRefCPTT.Billable, 1) AS SecInsRefServiceCodeBillable "
							" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
							" LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID "
							" LEFT JOIN (SELECT * FROM EMRProvidersT WHERE Deleted = 0) EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID "
							" LEFT JOIN InsuredPartyT SecInsPartyT ON PatientsT.PersonID = SecInsPartyT.PatientID and SecInsPartyT.RespTypeID = 2 "
							" LEFT JOIN PersonT SecInsPersonT ON SecInsPartyT.PersonID = SecInsPersonT.ID "
							" LEFT JOIN InsuranceReferralsT SecInsRefT ON SecInsPartyT.PersonID = SecInsRefT.InsuredPartyID "
							" LEFT JOIN PersonT SecInsRefProvT ON SecInsRefT.ProviderID = SecInsRefProvT.ID "
							" LEFT JOIN LocationsT SecInsRefLocationT ON SecInsRefT.LocationID = SecInsRefLocationT.ID "
							" LEFT JOIN InsuranceReferralCPTCodesT SecInsRefCPTCodesT ON SecInsRefT.ID = SecInsRefCPTCodesT.ReferralID "
							" LEFT JOIN CPTCodeT SecInsRefCPTT ON SecInsRefCPTCodesT.ServiceID = SecInsRefCPTT.ID "
							" LEFT JOIN InsuranceReferralDiagsT SecInsRefDiagT ON SecInsRefT.ID = SecInsRefDiagT.ReferralID "
							" LEFT JOIN DiagCodes SecInsRefDiagCodes ON SecInsRefDiagT.DiagID = SecInsRefDiagCodes.ID "
							" WHERE  "
							" CurrentStatus <> 4 AND EMRMasterT.Deleted = 0 ");
					break;
					case 2:

						//emr charges
						return _T("SELECT PersonT.ID as PatID, "						
							" EMRMasterT.ID AS EMRID, EMRMasterT.Date as Date, EMRMasterT.Description as EMRDescription, EMRMasterT.InputDate as IDate, "
							" EMRChargesT.ID as EMRChargeID, EMRChargesT.Description, EMRChargesT.Quantity, EMRChargesT.UnitCost, EMRChargesT.CPTModifier1, EMRChargesT.CPTModifier2, EMRChargesT.CPTModifier3, EMRChargesT.CPTModifier4, "
							" CPTMod1T.Note as CPTMod1Note, CPTMod1T.Multiplier as CPTMod1Multipler, "
							" CPTMod2T.Note as CPTMod2Note, CPTMod2T.Multiplier as CPTMod2Multipler, "
							" CPTMod3T.Note as CPTMod3Note, CPTMod3T.Multiplier as CPTMod3Multipler, "
							" CPTMod4T.Note as CPTMod4Note, CPTMod4T.Multiplier as CPTMod4Multipler, "
							" ICD9T.CodeNumber AS ICD9Code, "
							" ICD10T.CodeNumber AS ICD10Code, "
							" EMRProvidersT.ProviderID as ProvID, EMRMasterT.LocationID as LocID, "
							" CPTCodeT.Code as CPTCode, CPTCodeT.SubCode, ServiceT.Name as ServiceName, ServiceT.Price as ServicePrice, "
							" CASE WHEN CPTCodeT.ID IS NULL THEN 0 ELSE 1 END as IsCPT, "
							" CASE WHEN ProductT.ID IS NULL THEN 0 ELSE 1 END as IsProduct, "
							" Coalesce(CPTCodeT.Billable, 1) AS ServiceBillable "
							" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
							" LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID "
							" LEFT JOIN EMRChargesT ON EMRMasterT.ID = EMRChargesT.EMRID "						
							" LEFT JOIN ServiceT ON EMRChargesT.ServiceID = ServiceT.ID "
							" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
							" LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
							" LEFT JOIN CPTModifierT CPTMod1T ON EMRChargesT.CPTModifier1 = CPTMod1T.Number "
							" LEFT JOIN CPTModifierT CPTMod2T ON EMRChargesT.CPTModifier2 = CPTMod2T.Number "
							" LEFT JOIN CPTModifierT CPTMod3T ON EMRChargesT.CPTModifier3 = CPTMod3T.Number "
							" LEFT JOIN CPTModifierT CPTMod4T ON EMRChargesT.CPTModifier4 = CPTMod4T.Number "							
							" LEFT JOIN EMRChargesToDiagCodesT ON EMRChargesT.ID = EMRChargesToDiagCodesT.ChargeID "
							" LEFT JOIN DiagCodes ICD9T ON EMRChargesToDiagCodesT.DiagCodeID = ICD9T.ID "
							" LEFT JOIN DiagCodes ICD10T ON EMRChargesToDiagCodesT.DiagCodeID_ICD10 = ICD10T.ID "
							" LEFT JOIN (SELECT * FROM EMRProvidersT WHERE Deleted = 0) EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID "
							" WHERE  "
							" CurrentStatus <> 4 AND EMRMasterT.Deleted = 0 AND EMRChargesT.DELETED = 0 "); 
					break;

					case 3:  //EMN DiagCodes

						return _T("SELECT PersonT.ID as PatID, "
							" EMRMasterT.ID AS EMRID, EMRMasterT.Date as Date, EMRMasterT.Description as EMRDescription, EMRMasterT.InputDate as IDate, "
							" EMRDiagCodesT.ID as EMRDiagCodeID, "
							" ICD9T.CodeNumber AS ICD9Code, "
							" ICD10T.CodeNumber AS ICD10Code, "
							" ICD9T.CodeDesc AS ICD9CodeDesc, "
							" ICD10T.CodeDesc AS ICD10CodeDesc, "
							" EMRProvidersT.ProviderID as ProvID, EMRMasterT.LocationID as LocID "
							" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
							" LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID "
							" LEFT JOIN EMRDiagCOdesT ON EMRMasterT.ID = EMRDiagCodesT.EMRID "
							" LEFT JOIN DiagCodes ICD9T ON EMRDiagCodesT.DiagCodeID = ICD9T.ID "
							" LEFT JOIN DiagCodes ICD10T ON EMRDiagCodesT.DiagCodeID_ICD10 = ICD10T.ID "
							" LEFT JOIN (SELECT * FROM EMRProvidersT WHERE Deleted = 0) EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID "
							" WHERE  "
							" CurrentStatus <> 4 AND EMRMasterT.Deleted = 0 AND EMRDiagcodesT.DELETED = 0"); 
					break;

					case 4:						

						//payments
						return _T("SELECT PersonT.ID as PatID, "						
							" EMRMasterT.ID AS EMRID, EMRMasterT.Date as Date, EMRMasterT.Description as EMRDescription, EMRMasterT.InputDate as IDate, "
							" EMRProvidersT.ProviderID as ProvID, EMRMasterT.LocationID as LocID, "
							" LineItemT.Date as PayDate, LineItemT.InputDate as PayIDate, LineItemT.Description as PayDesc, LineItemT.Amount as PayAmount, "
							" PaymentsT.PaymentGroupID as CatID, PaymentGroupsT.GroupName as PayCatName, "
							" PersonProvT.First as ProvFirst, PersonProvT.Last as ProvLast, PersonProvT.Middle as ProvMiddle, PersonProvT.Title as ProvTitle, "
							" PaymentsT.InsuredPartyID as PayInsPartyID, "
							" PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
							" PaymentsT.PayMethod, CreditCardNamesT.CardName AS CCType, "
							" CASE WHEN Len(PaymentPlansT.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentPlansT.CCNumber, 4) END  as CCNumber, "
							" PaymentPlansT.CCHoldersName "
							" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
							" LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID "
							" LEFT JOIN (SELECT * FROM EMRProvidersT WHERE Deleted = 0) EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID "
							" LEFT JOIN (SELECT * FROM LineItemT WHERE DELETED = 0 AND Type = 1) LineItemT ON "
							" EMRMasterT.PatientID = LineItemT.PatientId AND  "
							"  convert(datetime, convert(nvarchar, EMRMasterT.Date, 23)) = convert(datetime, convert(nvarchar, LineItemT.Date, 23)) "
							" LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
							" LEFT JOIN PersonT PersonProvT ON PaymentsT.ProviderID = PersonProvT.ID "
							" LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
							" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
							" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
							" WHERE  "
							" CurrentStatus <> 4 AND EMRMasterT.Deleted = 0 "); 
					break;


					case 5: //images	
						// (j.gruber 2010-03-09 14:25) - PLID 37674 - added images to report
						{
						//build the inner Sql for the mailsent pathnames
						CString strInnerSql;

						CString strIDs =  GetRemotePropertyMemo("EMNReportImageCategories", "", 0, "<None>");
						CString strImageInserts;

						// (j.gruber 2010-04-15 11:55) - PLID 38090 - if they want to supress images, then blank out the string
						if(bExtended && saExtraValues.GetSize()) {
							strIDs = "";
						}

						CString strTempTable;
						strTempTable.Format("#TempImages%lu", GetTickCount());

						if (!strIDs.IsEmpty()) {

							//see if we can find a comma
							long nResult = strIDs.Find(",");
							CString strTemp = strIDs;														

							long nCount = 1;

							while (nResult != -1) {
								CString strID = strTemp.Left(nResult);
								// (j.gruber 2010-03-19 13:23) - PLID 37624 - only find image files
								strInnerSql += FormatString("(SELECT * FROM (SELECT PersonT.ID as PatID,  "
									" (SELECT top 1 PathName FROM MailSent WHERE CategoryID = %s AND PersonID = PersonT.ID ORDER BY Date DESC) as PathName, "
									" %s as CatID, (SELECT Description FROM NoteCatsF WHERE ID = %s) as CatName, %li as CountID "
									" FROM PersonT) Q%s WHERE PathName IS NOT NULL AND (Right(PathName, 3) IN ('png', 'jpg', 'bmp', 'gif', 'tif', 'pcx')  "
									" OR Right(PathName, 4) IN ('jpeg', 'tiff', 'jfif')))  UNION ALL ", strID, strID, strID, nCount, strID);

								strTemp = strTemp.Right(strTemp.GetLength() - (nResult + 1));
								strTemp.TrimLeft();
								strTemp.TrimRight();
								nResult = strTemp.Find(",");
								nCount++;
							}

							//now do the last one
							strInnerSql += FormatString("(SELECT * FROM (SELECT PersonT.ID as PatID,  "
									" (SELECT top 1 PathName FROM MailSent WHERE CategoryID = %s AND PersonID = PersonT.ID ORDER BY Date DESC) as PathName, "
									" %s as CatID, (SELECT Description FROM NoteCatsF WHERE ID = %s) as CatName, %li as CountID "
									" FROM PersonT) Q%s WHERE PathName IS NOT NULL AND (Right(PathName, 3) IN ('png', 'jpg', 'bmp', 'gif', 'tif', 'pcx')  "
									" OR Right(PathName, 4) IN ('jpeg', 'tiff', 'jfif')))  ", strTemp, strTemp, strTemp, nCount, strTemp);
						

							//now combine it with the filtered results so that we aren't doing this for every patient/category combination
							CString strOuterSql;
							strOuterSql.Format(" SELECT PersonT.ID as PatID,  "
								" EMRMasterT.ID AS EMRID, EMRMasterT.Date as Date, EMRMasterT.Description as EMRDescription, EMRMasterT.InputDate as IDate,  "
								" EMRProvidersT.ProviderID as ProvID, EMRMasterT.LocationID as LocID, "
								" MailSent.pathname, MailSent.CatID, MailSent.CatName, MailSent.CountID  "
								" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
								" LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID 						 "
								" LEFT JOIN (SELECT * FROM EMRProvidersT WHERE Deleted = 0) EMRProvidersT ON EMRMasterT.ID = EMRProvidersT.EMRID 							 "
								" LEFT JOIN (%s) MailSent ON PersonT.ID = MailSent.PatID "
								" WHERE CurrentStatus <> 4 AND EMRMasterT.Deleted = 0  "
								" AND MailSent.PathName IS NOT NULL ", strInnerSql);

							//filter
							CString strFilter;
							// sending in 3 for the subreport so that it'll filter correctly
							//because we don't want it to filter below, but we want it to filter here
							AddPartToClause(strFilter, GetDateFilter(nSubLevel, 3));
							AddPartToClause(strFilter, GetLocationFilter(nSubLevel, 3));
							AddPartToClause(strFilter, GetPatientFilter(nSubLevel, 3));
							AddPartToClause(strFilter, GetProviderFilter(nSubLevel, 3));
							AddPartToClause(strFilter, GetExtraFilter(nSubLevel, 3));
							AddPartToClause(strFilter, GetGroupFilter(nSubLevel, 3));
							AddPartToClause(strFilter, GetFilterFilter(nSubLevel, 3));

							strFilter.TrimLeft();
							strFilter.TrimRight();

							if (!strFilter.IsEmpty()) {
								ConvertCrystalToSql(strOuterSql, strFilter);
								AddFilter(strOuterSql, strFilter, TRUE);
							}

							//now run the query to get our information
							ADODB::_RecordsetPtr rs = CreateRecordset(GetRemoteDataSnapshot(), "SELECT PatID, PathName, CatID, CatName FROM (%s) Q GROUP BY PatID, PathName, CatID, CatName, CountID ORDER BY CountID ASC ", strOuterSql);
							while (! rs->eof) {

								//first, get our variables we'll want to readd to the query
								long nPatID = AdoFldLong(rs, "PatID");
								CString strPathName = AdoFldString(rs, "PathName", "");
								CString strCatName = AdoFldString(rs, "CatName", "");
								long nCatID = AdoFldLong(rs, "CatID", -1);

								//now create the image blob
								if(!strPathName.IsEmpty())
								{	
									if (strPathName.FindOneOf("\\/") == -1) { // does not contain a path separator
										// so include our images path
										// (j.gruber 2010-04-22 16:07) - PLID 38262 - set the silent flag
										CString strBasePath = GetPatientDocumentPath(nPatID, TRUE);
										strPathName = strBasePath ^ strPathName;
									}
									
									HBITMAP hImage = NULL;
									if(NxGdi::LoadImageFile(strPathName, hImage))
									{
										// Now save the contents of the bitmap
										// to a stream to beging the process of getting a valid byte stream to store in
										// the database.
										// Note: Crystal does not seem to accept
										// jpeg's for this (or at least not our version of it). PNG files worked, but the quality
										// was noticably worse than BMP so since we're only storing these bitmaps very
										// temporily in a temp table I decided to just use BMP.
										IStreamPtr pImageStream;
										Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromHBITMAP(hImage, NULL);
										// (j.gruber 2010-03-19 13:22) - PLID 37674 - changed to be able to switch bitmap or png
										long nResult = -1;
										if (GetRemotePropertyInt("EMNReportImagesType", 0, 0, "<None>") == 0 ) {
											//format bitmap
											nResult = NxGdi::SaveToBMPStream(pBitmap, pImageStream);
										}
										else {
											//use png
											nResult = NxGdi::SaveToPNGStream(pBitmap, pImageStream);
										}
										
										if(pBitmap != NULL &&  nResult == Gdiplus::Ok)
										{
											// Seek back to the beginning of the bitmap stream.
											LARGE_INTEGER dlibMove;
											dlibMove.HighPart = dlibMove.LowPart = 0;
											dlibMove.QuadPart = 0;
											pImageStream->Seek(dlibMove, STREAM_SEEK_SET, NULL);
											// Get the size of the bitmap in the stream.
											STATSTG stats;
											pImageStream->Stat(&stats, STATFLAG_NONAME);
											// Everywhere that we'll be using
											// size only takes a max of 2^32 bytes, so if the image is > 4 GB then taking
											// just the low part here will fail, but I'm willing to take that chance.
											DWORD dwSize = stats.cbSize.LowPart;
											BYTE *pImageBytes = NULL;
											COleSafeArray sa;
											// Create a variant array of bytes
											// to store the bitmap's bytes.
											sa.CreateOneDim(VT_UI1, dwSize);
											sa.AccessData((LPVOID*)&pImageBytes);
											// Read the bitmap into our
											// byte array that's attached to our variant array.
											ULONG nBytesRead;
											pImageStream->Read(pImageBytes, dwSize, &nBytesRead);
											ASSERT(nBytesRead == dwSize);
											sa.UnaccessData();
											delete pBitmap;

											// Now generate the SQL
											// to insert these images into a table variable that is declared
											// as part of the query for this report.
											strImageInserts += FormatString(
												"INSERT INTO %s (PatID, CatID, CatName, CatImage) VALUES (%li, %li, '%s', %s) \r\n"
												, strTempTable, nPatID, nCatID, _Q(strCatName), CreateByteStringFromSafeArrayVariant(sa.Detach()));
										}

										if(hImage != NULL) {
											DeleteObject(hImage);
										}
									}
								}

								rs->MoveNext();
							}
						}
						

						//alrighty, now we just need to get everything together and run it
						CString strFinalSql;
						strFinalSql.Format(""
							" SET NOCOUNT ON \r\n"
							" CREATE TABLE %s (PatID int not null, CatID int not null, CatName nVarChar(50), CatImage image) \r\n "
							" %s \r\n "
							" SET NOCOUNT OFF \r\n "
							" SELECT PatID, CatImage, CatID, CatName FROM %s "						
							, strTempTable, strImageInserts,strTempTable);						

						return _T(strFinalSql);
					}
					break;				

					default:
						return _T("");
					break;
					}
				break;

				default:
					return _T("");
				break;
			}
			break;
			//(j.deskurakis 2013-01-24) - PLID 54973 - reports 696 and 697 both revised to match new tables and available data
			// (f.dinatale 2010-07-16) - PLID 39541 - Added reports for HL7 Integration Info.
			case 696:
			case 697:
				return _T("SELECT client as ClientName, ClientID AS PatID, StartDate AS StartDate, "
						"IntegrationBillToT.Description AS BillTo, IntegrationLabTypesT.Description AS IntegrationType, "
						"Notes AS Notes, ExpirationDate AS Expiration, Lab as Lab, AssignedTo as AssignedTo, currentStatus as currentStatus, "
						"PO as PO, Paid as Paid, GoLive as GoLive, Trained as Trained, IncidentID as IncidentID "
						"FROM LabsInternalV "
						"LEFT JOIN IntegrationBillToT ON LabsInternalV.IntegrationsBillTo = IntegrationBillToT.IntegrationBillToID "
						"LEFT JOIN IntegrationLabTypesT ON LabsInternalV.IntegrationLabType = IntegrationLabTypesT.LabTypeID ");
				break;


			case 702:
				/*Version History
					Data Conversions
					(j.armen 3-9-2011) - PLID 42146 - Added report for listing data conversions for sales.
			   */
				return _T(
					"SELECT PersonT.ID AS PatID, PersonT.First AS First, PersonT.Last AS Last, PersonT.Company AS Company, \r\n"
					// (m.thurston 9-6-2012) - PLID 52034 - FinishedDateOld needed at IssueT.FinishedDate
					"	IssueT.ID AS IssueID, IssueT.EnteredDate AS EnteredDate, IssueT.FinishedDateOld AS FinishedDate, IssueT.Description AS Description, \r\n"
					"	UsersT.PersonID AS UserID, UsersT.UserName AS UserName, \r\n"
					"	IssueCategoryT.ID AS IssueCategoryID, IssueCategoryT.Description AS SourceSystem \r\n"
					"FROM IssueT \r\n"
					"INNER JOIN UsersT ON IssueT.UserID = UsersT.PersonID \r\n"
					"INNER JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID \r\n"
					"INNER JOIN PersonT ON IssueT.ClientID = PersonT.ID \r\n"
					"WHERE ParentID = 29 \r\n");
				break;

	case 708: {
		/*	Version History
				Developer Escalation Requests
				(b.savon 2011-06-24) - PLID 44179 - Added report to review developer escalation requests
										by support user
		*/
		COleDateTimeSpan  OneDay(1,0,0,0);
		COleDateTime dtStart(2000, 1, 1, 0, 0, 0);
		COleDateTime dtDateTo;
		dtDateTo = DateTo+OneDay;
		CString strQuery;

		if(nDateRange == -1){
			strQuery =	"DECLARE @DateFrom DATETIME "
						"DECLARE @DateTo DATETIME "
						"SET @DateFrom = " + dtStart.Format("'%m/%d/%Y'") +
						" SET @DateTo = " + dtDateTo.Format("'%m/%d/%Y'");
		} else{
			strQuery =	"DECLARE @DateFrom DATETIME "
						"DECLARE @DateTo DATETIME "
						"SET @DateFrom = " + DateFrom.Format("'%m/%d/%Y'") +
						" SET @DateTo = " + dtDateTo.Format("'%m/%d/%Y'");
		}

		strQuery +=	" SELECT	DISTINCT SupportUserListT.[Support User Name],  "
							"CASE "
							"	WHEN([Total Requests] IS NULL) "
							"		THEN 0 "
							"	ELSE [Total Requests] " 
							"END AS [Total Requests], "
							"CASE "
							"	WHEN([Unrated Request] IS NULL) "
							"		THEN 0 "
							"	ELSE [Unrated Request] "
							"END AS [Unrated Request], "
							"CASE "
							"	WHEN([Required A Developer] IS NULL)  "
							"		THEN 0  "
							"	ELSE [Required A Developer] "
							"END AS [Required A Developer], "
							"CASE "
							"	WHEN([Advanced Technical Issue] IS NULL) "
							"		THEN 0 "
							"	ELSE [Advanced Technical Issue] "
							"END AS [Advanced Technical Issue], "
							"CASE "
							"	WHEN([Should Not Have Been Escalated] IS NULL) "
							"		THEN 0 "
							"	ELSE [Should Not Have Been Escalated]  "
							"END AS [Should Not Have Been Escalated], "
							"CASE  "
							"	WHEN([Dev Not Able To Solve Issue] IS NULL) "
							"		THEN 0 "
							"	ELSE [Dev Not Able To Solve Issue] "
							"END AS [Dev Not Able To Solve Issue] "
					"FROM	( "
								"SELECT	PersonID AS [Support User ID], UserName AS [Support User Name] "
								"FROM	UsersT "
								"		LEFT JOIN "
								"		PersonT WITH(NOLOCK) "
								"		ON UsersT.PersonID = PersonT.ID "
								"		LEFT JOIN "
								"		NxAssignedRolesT "
								"		ON NxAssignedRolesT.UserID = PersonT.ID "
								"		LEFT JOIN "
								"		EscalationRequestT "
								"		ON	PersonT.ID = EscalationRequestT.RequesterUserID "
								"WHERE	PersonID > 0 "
								"AND		PersonT.Archived = 0 "
								"AND		NxAssignedRolesT.RoleID <> 0 "
								") AS SupportUserListT "
								"LEFT JOIN "
								"( "	
								"	SELECT	COUNT(RequesterUserID) AS [Required A Developer], RequesterUserID AS [Support User ID] "
								"	FROM	EscalationRequestT "
								"	WHERE	ResolutionCodeID = 1 "		
								"		AND	RequestDate >= @DateFrom "
								"		AND RequestDate <= @DateTo "
								"GROUP BY RequesterUserID "
								") AS RequiredADevT "
								"ON SupportUserListT.[Support User ID] = RequiredADevT.[Support User ID] "
								"LEFT JOIN "
								"( "
								"	SELECT	COUNT(RequesterUserID) AS [Advanced Technical Issue], RequesterUserID AS [Support User ID] "
								"	FROM	EscalationRequestT "
								"	WHERE	ResolutionCodeID = 2 "
								"		AND	RequestDate >= @DateFrom "
								"		AND RequestDate <= @DateTo "
								"	GROUP BY RequesterUserID "
								") AS AdvancedTechIssueT "
								"ON	SupportUserListT.[Support User ID] = AdvancedTechIssueT.[Support User ID] "
								"LEFT JOIN "
								"( " 
								"	SELECT	COUNT(RequesterUserID) AS [Should Not Have Been Escalated], RequesterUserID AS [Support User ID] "
								"	FROM	EscalationRequestT "
								"	WHERE	ResolutionCodeID = 3 "
								"		AND	RequestDate >= @DateFrom "
								"		AND RequestDate <= @DateTo "
								"	GROUP BY RequesterUserID "
								") AS NoEscalationT "
								"ON	SupportUserListT.[Support User ID] = NoEscalationT.[Support User ID] "
								"LEFT JOIN "
								"( "
								"	SELECT	COUNT(RequesterUserID) AS [Dev Not Able To Solve Issue], RequesterUserID AS [Support User ID] "
								"	FROM	EscalationRequestT "
								"	WHERE	ResolutionCodeID = 4  "
								"		AND	RequestDate >= @DateFrom "
								"		AND RequestDate <= @DateTo "
								"	GROUP BY RequesterUserID "
								") AS DevNotAbleToSolveT "
								"ON	SupportUserListT.[Support User ID] = DevNotAbleToSolveT.[Support User ID] "
								"LEFT JOIN "
								"( "
								"	SELECT	COUNT(RequesterUserID) AS [Total Requests], RequesterUserID AS [Support User ID] "
								"	FROM	EscalationRequestT "
								"	WHERE	RequestDate >= @DateFrom "
								"		AND RequestDate <= @DateTo "
								"		AND ( ResolutionCodeID <> 5 OR ResolutionCodeID IS NULL) "
								"	GROUP BY RequesterUserID "
								") AS TotalT "
								"ON	SupportUserListT.[Support User ID] = TotalT.[Support User ID] "
								"LEFT JOIN " 
								"( "
								"	SELECT	COUNT(RequesterUserID) AS [Unrated Request], RequesterUserID AS [Support User ID] "
								"	FROM	EscalationRequestT "
								"	WHERE	RequestDate >= @DateFrom "
								"		AND RequestDate <= @DateTo "
								"		AND	ResolutionCodeID IS NULL "
								"	GROUP BY RequesterUserID "
								") AS DevNotRespondedT " 
								"ON	SupportUserListT.[Support User ID] = DevNotRespondedT.[Support User ID] "
						"WHERE	[Total Requests] > 0 "
						"ORDER BY [Total Requests] DESC ";
		return(strQuery);
		
		break;
	}
	case 712: {
		/*	Version History
				Developer Escalation Requests - Support Rating
				(b.savon 2011-07-19) - PLID 44523 - Added report to review developer escalation requests
										support ratings for each developer
		*/
		COleDateTimeSpan  OneDay(1,0,0,0);
		COleDateTime dtStart(2000, 1, 1, 0, 0, 0);
		COleDateTime dtDateTo;
		dtDateTo = DateTo+OneDay;
		CString strQuery;

		if(nDateRange == -1){
			strQuery =	"DECLARE @DateFrom DATETIME "
						"DECLARE @DateTo DATETIME "
						"SET @DateFrom = " + dtStart.Format("'%m/%d/%Y'") +
						" SET @DateTo = " + dtDateTo.Format("'%m/%d/%Y'");
		} else{
			strQuery =	"DECLARE @DateFrom DATETIME "
						"DECLARE @DateTo DATETIME "
						"SET @DateFrom = " + DateFrom.Format("'%m/%d/%Y'") +
						" SET @DateTo = " + dtDateTo.Format("'%m/%d/%Y'");
		}

		strQuery +=	" SELECT	DISTINCT DeveloperUserListT.[Developer User Name], "
					"		COALESCE([Total Requests], 0) AS [Total Requests], "
					"		COALESCE([Unrated Request], 0) AS [Unrated Request], "
					"		COALESCE([Very Helpful], 0) AS [Very Helpful], "
					"		COALESCE([Helpful], 0) AS [Helpful], "
					"		COALESCE([Somewhat Helpful], 0) AS [Somewhat Helpful], "
					"		COALESCE([Not Helpful], 0) AS [Not Helpful]	  "
					"FROM	( "
					"			SELECT	PersonID AS [Developer User ID], UserName AS [Developer User Name] "
					"			FROM	UsersT "
					"					LEFT JOIN "
					"					PersonT WITH(NOLOCK) "
					"					ON UsersT.PersonID = PersonT.ID "
					"					LEFT JOIN "
					"					NxAssignedRolesT "
					"					ON NxAssignedRolesT.UserID = PersonT.ID "
					"					LEFT JOIN "
					"					EscalationRequestT "
					"					ON	PersonT.ID = EscalationRequestT.AssignedUserID "
					"			WHERE	PersonID > 0 "
					"			AND		PersonT.Archived = 0 "
					"			AND		NxAssignedRolesT.RoleID = 0 "
					"			) AS DeveloperUserListT "
					"			LEFT JOIN "
					"			( 	"
					"				SELECT	COUNT(AssignedUserID) AS [Very Helpful], AssignedUserID AS [Developer User ID] "
					"				FROM	EscalationRequestT "
					"				WHERE	SupportRatingID = 1 "		
					"					AND	RequestDate >= @DateFrom "
					"					AND RequestDate <= @DateTo "
					"			GROUP BY AssignedUserID "
					"			) AS VeryHelpfulT "
					"			ON DeveloperUserListT.[Developer User ID] = VeryHelpfulT.[Developer User ID] "
					"			LEFT JOIN "
					"			( "
					"				SELECT	COUNT(AssignedUserID) AS [Helpful], AssignedUserID AS [Developer User ID] "
					"				FROM	EscalationRequestT "
					"				WHERE	SupportRatingID = 2 "
					"					AND	RequestDate >= @DateFrom "
					"					AND RequestDate <= @DateTo "
					"				GROUP BY AssignedUserID "
					"			) AS HelpfulT "
					"			ON	DeveloperUserListT.[Developer User ID] = HelpfulT.[Developer User ID] "
					"			LEFT JOIN "
					"			(  "
					"				SELECT	COUNT(AssignedUserID) AS [Somewhat Helpful], AssignedUserID AS [Developer User ID] "
					"				FROM	EscalationRequestT "
					"				WHERE	SupportRatingID = 3 "
					"					AND	RequestDate >= @DateFrom "
					"					AND RequestDate <= @DateTo "
					"				GROUP BY AssignedUserID "
					"			) AS SomewhatHelpfulT "
					"			ON	DeveloperUserListT.[Developer User ID] = SomewhatHelpfulT.[Developer User ID] "
					"			LEFT JOIN "
					"			( "
					"				SELECT	COUNT(AssignedUserID) AS [Not Helpful], AssignedUserID AS [Developer User ID] "
					"				FROM	EscalationRequestT "
					"				WHERE	SupportRatingID = 4  "
					"					AND	RequestDate >= @DateFrom "
					"					AND RequestDate <= @DateTo "
					"				GROUP BY AssignedUserID "
					"			) AS NotHelpfulT "
					"			ON	DeveloperUserListT.[Developer User ID] = NotHelpfulT.[Developer User ID] "
					"			LEFT JOIN "
					"			( "
					"				SELECT	COUNT(AssignedUserID) AS [Total Requests], AssignedUserID AS [Developer User ID] "
					"				FROM	EscalationRequestT "
					"				WHERE	RequestDate >= @DateFrom "
					"					AND RequestDate <= @DateTo "
					"					AND ( SupportRatingID <> 5 OR SupportRatingID IS NULL) "
					"				GROUP BY AssignedUserID "
					"			) AS TotalT "
					"			ON	DeveloperUserListT.[Developer User ID] = TotalT.[Developer User ID] "
					"			LEFT JOIN  "
					"			( "
					"				SELECT	COUNT(AssignedUserID) AS [Unrated Request], AssignedUserID AS [Developer User ID] "
					"				FROM	EscalationRequestT "
					"				WHERE	RequestDate >= @DateFrom "
					"					AND RequestDate <= @DateTo "
					"					AND	SupportRatingID IS NULL "
					"				GROUP BY AssignedUserID "
					"			) AS SupNotRespondedT  "
					"			ON	DeveloperUserListT.[Developer User ID] = SupNotRespondedT.[Developer User ID] "
					"WHERE	[Total Requests] > 0 " 
					"ORDER BY [Total Requests] DESC";
		return(strQuery);
		
		break;
	}

	// (j.luckoski 2012/03/28) - PLID 48809 - Added in report to show clients with training time left greater than 0
	case 724: {
		CString strQuery;
		strQuery = "SELECT PersonT.ID AS PatID, Last, First, dbo.AsDateNoTime((SELECT TOP 1 T2.Date FROM TrainingTimeAndAppointmentViewT T2 "
			 "WHERE T2.PurchaseID IS NOT NULL AND T2.PersonID = PersonT.ID "
			 "ORDER BY T2.Date Desc)) AS LastPurchaseDate, "
			 "CASE WHEN (SELECT ISNULL(SUM(T3.PurchasedHours),0) - ISNULL(SUM(T3.TrainingUsed),0) FROM TrainingTimeAndAppointmentViewT T3 "
			 "WHERE T3.PersonID = PersonT.ID GROUP BY T3.PersonID) > 0 THEN "
			 "(SELECT ISNULL(SUM(T3.PurchasedHours),0) - ISNULL(SUM(T3.TrainingUsed),0) FROM TrainingTimeAndAppointmentViewT T3  "
			 "WHERE T3.PersonID = PersonT.ID GROUP BY T3.PersonID) ELSE 0 "
			 "END AS RemainingHours FROM PersonT GROUP BY PersonT.ID, Last, First HAVING (CASE WHEN (SELECT ISNULL(SUM(T3.PurchasedHours),0) - ISNULL(SUM(T3.TrainingUsed),0) FROM TrainingTimeAndAppointmentViewT T3 "
			 "WHERE T3.PersonID = PersonT.ID GROUP BY T3.PersonID) > 0 THEN "
			 "(SELECT ISNULL(SUM(T3.PurchasedHours),0) - ISNULL(SUM(T3.TrainingUsed),0) FROM TrainingTimeAndAppointmentViewT T3  "
			 "WHERE T3.PersonID = PersonT.ID GROUP BY T3.PersonID) ELSE 0 END) > 0 ";
		return _T(strQuery);

		break;
	}

	default:
		return _T("");
		break;
	}
}

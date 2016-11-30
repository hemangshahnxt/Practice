
////////////////
// (j.gruber 2008-07-14 16:31) - PLID 28976 - Created GetSqlPracticeAnalysis() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "DateTimeUtils.h"


CString CReportInfo::GetSqlPracticeAnalysis(long nSubLevel, long nSubRepNum) const
{
	switch (nID) {
		
		case 633:  //number of charges by Prov Monthly
		case 634: //number of charges by Prov Quarterly
		case 635: //number of charges by Cat Seg Monthly
		case 636: //number of charges by Cat Seg Quarterly
		case 637: //Sum of charges by Prov Monthly
		case 638: //Sum of charges by Prov Quarterly
		case 639: //Sum of charges by Cat Seg
		case 640: //Sum of charges by Cat Seg Quarterly
		case 649:  //number of bills by master procedure Monthly
		case 650:  //number of bills by master procedure Quarterly

			/*Version History
				// (j.gruber 2008-07-21 16:58) - PLID 30797 - Created
				// (j.gruber 2014-03-27 12:57) - PLID 61466 - updated for ICD-10
			*/
		
			
		return _T("SELECT PatientsT.PersonID AS PatID,  "
		"PersonT.Zip,  "
		"LineItemT.Date AS TDate,  "
		"BillsT.Date AS BDate,  "
		"ChargesT.ID,  "
		"CPTCodeT.Code, "
		"CPTCodeT.SubCode, "
		"ServiceT.Name as ServiceName, "
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
		"SegCatsT.ID as SegCategoryID, SegCatsT.Name as SegCategoryName, "
		"SegProvsT.ID as SegProviderID, SegProvsT.Name as SegProviderName,  "
		"SegLocsT.ID as SegLocationID, SegLocsT.Name as SegLocationName,  "
		"Month(LineItemT.Date) * 100 + Day(LineItemT.Date) AS TMonthDay, "
		"Month(LineItemT.InputDate) * 100 + Day(LineItemT.InputDate) AS IMonthDay, "
		"ChargesT.ServiceID as ServiceID, "
		"CASE WHEN MastProcT.ID IS NULL THEN ProcedureT.ID ELSE MastProcT.ID END AS ProcID, "
		"CASE WHEN MastProcT.ID IS NULL THEN ProcedureT.Name ELSE MastProcT.Name END AS ProcName, "
		"Month(BillsT.Date) * 100 + Day(BillsT.Date) AS BMonthDay "
		"FROM ChargesT LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
		"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
		"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
		"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
		"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
		"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		
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

		"LEFT JOIN PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
		"ON BillsT.PatientID = PatientsT.PersonID "
		"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
		" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 1)) AS SegmentCategoriesT ON CategoriesT.ID = SegmentCategoriesT.ItemID "
		" LEFT JOIN ReportSegmentsT SegCatsT ON SegmentCategoriesT.SegmentID = SegCatsT.ID "
		" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 2)) AS SegmentProvidersT ON ProvidersT.PersonID = SegmentCategoriesT.ItemID "
		" LEFT JOIN ReportSegmentsT SegProvsT ON SegmentProvidersT.SegmentID = SegProvsT.ID "
		" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 3)) AS SegmentLocationsT ON LocationsT.ID = SegmentLocationsT.ItemID "
		" LEFT JOIN ReportSegmentsT SegLocsT ON SegmentLocationsT.SegmentID = SegLocsT.ID "
		" LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
		" LEFT JOIN ProcedureT MastProcT ON ProcedureT.MasterProcedureID = MastProcT.ID "
		" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"WHERE (((BillsT.EntryType)=1) AND ((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) AND GCTypesT.ServiceID IS NULL ");
		break;

		case 641: //Resp. Percentage by Prov Monthly
		case 642: //Resp. Percentage by Prov Quarterly
		case 643: //Resp. Percentage by Cat Seg Monthly
		case 644: //Resp. Percentage by Cat Seg Quarterly

			/*Version History
				// (j.gruber 2008-07-21 16:58) - PLID 30798 - Created
				// (a.walling 2014-04-09 09:30) - PLID 61695 - support new WhichCodes structure as nvarchar(50) for compatibility
			*/
		

			return _T("SELECT PatientsT.PersonID AS PatID,  "
				"LineItemT.Date AS TDate,  "
				"BillsT.Date AS BDate,  "
				"ChargesT.ID,  "
				"CPTCodeT.Code, "
				"CPTCodeT.SubCode, "
				"ServiceT.Name as ServiceName, "
				"CONVERT(NVARCHAR(50), ChargeWhichCodesFlatV.WhichCodes) AS WhichCodes, "
				"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,  "
				"Sum(ChargeRespT.Amount) as ChargeAmountTotal,  "
				"CASE WHEN ChargeRespT.InsuredPartyID IS NULL THEN -1 ELSE ChargeRespT.InsuredPartyID END AS InsPartyID, "
				"ChargesT.BillID,  "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
				"ProvidersT.PersonID AS ProvID,  "
				"CategoriesT.ID AS CatID,  "
				"CategoriesT.Name AS CatName,  "
				"BillsT.EntryType,  "
				"LineItemT.InputDate AS IDate, "
				"PatientsT.UserDefinedID, "
				"LineItemT.LocationID AS LocID, "
				"LocationsT.Name AS Location, "
				"SegCatsT.ID as SegCategoryID, SegCatsT.Name as SegCategoryName, "
				"SegProvsT.ID as SegProviderID, SegProvsT.Name as SegProviderName,  "
				"SegLocsT.ID as SegLocationID, SegLocsT.Name as SegLocationName,  "
				"Month(LineItemT.Date) * 100 + Day(LineItemT.Date) AS TMonthDay, "
				"Month(LineItemT.InputDate) * 100 + Day(LineItemT.InputDate) AS IMonthDay, "
				" ChargesT.ServiceID as ServiceID "
				"FROM ChargeRespT LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID "
				"LEFT JOIN ChargeWhichCodesFlatV ON ChargesT.ID = ChargeWhichCodesFlatV.ChargeID "
				"LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
				"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
				"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
				"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
				"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
				"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"LEFT JOIN PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"ON BillsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
				" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 1)) AS SegmentCategoriesT ON CategoriesT.ID = SegmentCategoriesT.ItemID "
				" LEFT JOIN ReportSegmentsT SegCatsT ON SegmentCategoriesT.SegmentID = SegCatsT.ID "
				" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 2)) AS SegmentProvidersT ON ProvidersT.PersonID = SegmentCategoriesT.ItemID "
				" LEFT JOIN ReportSegmentsT SegProvsT ON SegmentProvidersT.SegmentID = SegProvsT.ID "
				" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 3)) AS SegmentLocationsT ON LocationsT.ID = SegmentLocationsT.ItemID "
				" LEFT JOIN ReportSegmentsT SegLocsT ON SegmentLocationsT.SegmentID = SegLocsT.ID "
				"WHERE (((BillsT.EntryType)=1) AND ((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) AND GCTypesT.ServiceID IS NULL "
				"Group By PatientsT.PersonID,  "
				"LineItemT.Date,  "
				"BillsT.Date,  "
				"ChargesT.ID,  "
				"CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name,  "
				"ChargeWhichCodesFlatV.WhichCodes, "
				"PersonT1.Last, PersonT1.First,PersonT1.Middle,  "
				"ChargeRespT.InsuredPartyID,"
				"ChargesT.BillID,  "
				"PersonT.Last,PersonT.First,PersonT.Middle,  "
				"ProvidersT.PersonID,  "
				"CategoriesT.ID,  "
				"CategoriesT.Name,  "
				"BillsT.EntryType,  "
				"LineItemT.InputDate, "
				"PatientsT.UserDefinedID, "
				"LineItemT.LocationID, "
				"LocationsT.Name, "
				"SegCatsT.ID, SegCatsT.Name, "
				"SegProvsT.ID, SegProvsT.Name,"
				"SegLocsT.ID, SegLocsT.Name, ChargesT.ServiceID");
			break;

		case 645: //Top Service Codes billed Monthly by Count
		case 646: //Top Service Codes billed Quarterly by Count
		case 647: //Top Service Codes billed Monthly by Amount
		case 648: //Top Service Codes billed Quarterly by Amount

			/*Version History
				// (j.gruber 2008-07-21 16:58) - PLID 30800 - Created
				// (j.gruber 2014-03-27 11:46) - PLID 61476 - updated for ICD-10
			*/
			{

				CString strFilter;
				COleDateTime dtFrom, dtTo;
				if (nDateRange == -1) {
					dtFrom.SetDate(1800,01,01);
					dtTo.SetDate(5000,12,31);
				}
				else {
				
					dtFrom = DateFrom;
					dtTo = DateTo;
				}

				CString strDateTo, strDateFrom;
				if (bUseAllYears) {
					//service date
					if (nDateRange > 0) {
						ASSERT(!bOneDate);
						if (!bOneDate) {
							CString strAns;
							int nToVal = DateTo.GetMonth()*100 + DateTo.GetDay(); //(3/12 = 312)
							int nFromVal = DateFrom.GetMonth()*100 + DateFrom.GetDay();
							CString strField;
							if (nDateFilter == 1) {
								strField = " (Month(LineItemT.Date) * 100 + Day(LineItemT.Date)) ";
							}
							else {
								strField = " (Month(LineItemT.InputDate) * 100 + Day(LineItemT.InputDate)) ";
							}
					
							//must be >= in case we choose the same day for both!
							if(nToVal >= nFromVal){
								strAns.Format(" AND (%s >= %li AND %s <= %li)", strField, nFromVal, strField, nToVal);
							}
							else {
								strAns.Format(" AND ( (%s >= %li AND %s <= 1231) OR (%s <= %li AND %s >= 101}) )", strField, nFromVal, strField, strField, nToVal, strField);
							}
							strDateTo = strAns;
						}
					}
				}
				else {
					strDateTo.Format(" AND LineItemT.Date < DateAdd(day, 1, '%s')", FormatDateTimeForSql(dtTo, dtoDate));
					strDateFrom.Format(" AND LineItemT.Date >=  '%s'", FormatDateTimeForSql(dtFrom, dtoDate));
				}

				strFilter += strDateTo;
				strFilter += strDateFrom;

				//location
				//(e.lally 2008-09-30) PLID 31542 - We need to handle filtering on multiple locations
				CString strLoc="";
				if (nLocation > 0) {
					strLoc.Format(" AND LineItemT.LocationID = %li", nLocation);
				}
				else if(nLocation == -2) {
					strLoc.Format(" AND LineItemT.LocationID IS NULL");
				}
				else if (nLocation == -3) {
					strLoc = " AND LineItemT.LocationID IN(";
					CString strPart;
					for(int i=0; i < m_dwLocations.GetSize(); i++) {
						strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
						strLoc += strPart;
					}
					strLoc = strLoc.Left(strLoc.GetLength()-2) + ")";
				}
				strFilter += strLoc;

				//provider
				CString strProv;
				if (nProvider > 0) {
					strProv.Format(" AND ChargesT.DoctorsProviders = %li", nProvider);
					
				} else if(nProvider == -2) {
					strProv.Format(" AND ChargesT.DoctorsProviders IS NULL OR PaymentsT.ProviderID = -1");
				}
				else if(nProvider == -3) {
					CString strAns;
					CString strPart;
					for(int i=0; i < m_dwProviders.GetSize(); i++) {
						strPart.Format("%li, ", (long)m_dwProviders.GetAt(i));
						strAns += strPart;
					}
					strProv.Format(" AND ChargesT.DoctorsProviders IN (%s)", strAns.Left(strAns.GetLength()-2));
				}
				strFilter += strProv;

				//Category Segment
				CString strSegment;
				if (bExtended && saExtraValues.GetSize()) {
					CString strAns;
					CString strPart;
					for(int i=0; i < saExtraValues.GetSize(); i++) {
						strPart.Format("%s, ", saExtraValues[i]);
						strAns += strPart;
					}
					strSegment.Format(" AND SegCatsT.ID IN (%s)", strAns.Left(strAns.GetLength()-2));

					strFilter += strSegment;
				}

				CString strOrder;
				if (nID == 645 || nID == 646) {
					//by Count
					strOrder = "Count(LineItemT.ID) ";
				}
				else {
					//by amount
					strOrder = "Sum(dbo.GetChargeTotal(LineItemT.ID)) ";
				}

			CString strSql;
			strSql.Format("SELECT PatientsT.PersonID AS PatID,  "
			"PersonT.Zip,  "
			"LineItemT.Date AS TDate,  "
			"BillsT.Date AS BDate,  "
			"ChargesT.ID,  "
			"CPTCodeT.Code, "
			"CPTCodeT.SubCode, "
			"ServiceT.Name as ServiceName, "

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
			"SegCatsT.ID as SegCategoryID, SegCatsT.Name as SegCategoryName, "
			"SegProvsT.ID as SegProviderID, SegProvsT.Name as SegProviderName,  "
			"SegLocsT.ID as SegLocationID, SegLocsT.Name as SegLocationName,  "
			"Month(LineItemT.Date) * 100 + Day(LineItemT.Date) AS TMonthDay, "
			"Month(LineItemT.InputDate) * 100 + Day(LineItemT.InputDate) AS IMonthDay, "
			"ChargesT.ServiceID as ServiceID "
			"FROM ChargesT LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
			"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "			
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
			"LEFT JOIN PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"ON BillsT.PatientID = PatientsT.PersonID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 1)) AS SegmentCategoriesT ON CategoriesT.ID = SegmentCategoriesT.ItemID "
			" LEFT JOIN ReportSegmentsT SegCatsT ON SegmentCategoriesT.SegmentID = SegCatsT.ID "
			" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 2)) AS SegmentProvidersT ON ProvidersT.PersonID = SegmentCategoriesT.ItemID "
			" LEFT JOIN ReportSegmentsT SegProvsT ON SegmentProvidersT.SegmentID = SegProvsT.ID "
			" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 3)) AS SegmentLocationsT ON LocationsT.ID = SegmentLocationsT.ItemID "
			" LEFT JOIN ReportSegmentsT SegLocsT ON SegmentLocationsT.SegmentID = SegLocsT.ID "
			" LEFT JOIN CptCodeT ON ServiceT.ID = CPTCodeT.ID "
			"WHERE (((BillsT.EntryType)=1) AND ((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) AND GCTypesT.ServiceID IS NULL "
			" AND ChargesT.ServiceID IN (SELECT top 10 ServiceID "
				" FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				" LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
				" LEFT JOIN (SELECT * FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 1)) AS SegmentCategoriesT ON CategoriesT.ID = SegmentCategoriesT.ItemID "
				" LEFT JOIN ReportSegmentsT SegCatsT ON SegmentCategoriesT.SegmentID = SegCatsT.ID "				
				" WHERE (LineItemT.Deleted = 0) AND ServiceT.ID NOT IN (SELECT ServiceID FROM GCTypesT) AND ServiceT.ID NOT IN (SELECT ID FROM AdministrativeFeesT) %s GROUP BY ServiceID ORDER BY %s DESC)", strFilter, strOrder);
				return strSql;
			}
		break;


	//no report was found with that id
	default:
		return _T("");
		break;
	}


}

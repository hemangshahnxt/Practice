//statement Sql

#include "stdafx.h"
#include "NxStandard.h"
#include "GlobalReportUtils.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "Practice.h"
#include "MainFrm.h"
#include "NxReportJob.h"
#include "peplus.h"
#include "Reports.h"
#include "ChildFrm.h"
#include "ReportInfo.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "WhereClause.h"
#include "StatementSql.h"

CStatementSqlBuilder::CStatementSqlBuilder(ADODB::_ConnectionPtr pConnection, CReportInfo *pReport, long nSubLevel, long nSubRepNum)
	: CStatementSqlBuilder(pConnection, pReport, nSubLevel, nSubRepNum, FALSE)
{
}

CStatementSqlBuilder::CStatementSqlBuilder(ADODB::_ConnectionPtr pConnection, CReportInfo *pReport, long nSubLevel, long nSubRepNum, BOOL bIsEStatement)
{
	this->m_pConnection = pConnection;
	this->m_pReport = pReport;
	this->m_nSubLevel = nSubLevel;
	this->m_nSubRepNum = nSubRepNum;
	this->m_bIsEStatement = bIsEStatement;
}

//PLID 19451 - This function creates a temp table which is then used to filter the main statement by patient
CString CStatementSqlBuilder::CreateStatementFilterTable() {	
	try {

		//first get the variables that this query will use
		CString strHideUnappliedPrepayments = GetStatementUnAppliedPrePaymentsString();
		CString strInnerQuery;

		// (j.gruber 2007-03-12 16:05) - PLID 22121 - made the statement by location filter by locations also
		// (a.wilson 2012-2-24) PLID 48380 - Removed ':' from queries to fix compatibility change errors.
		//TES 7/17/2014 - PLID 62565 - Added code to hide chargebacks if requested.
		// NOTE: Throughout this file, I chose to concatenate that code right wherever it needed to be, rather than using the printf functionality.
		// The reason for this is that it was driving me nuts trying to match up existing %s's with their matching variable, so this will 
		// hopefully be a bit easier to decipher in future.
		// (c.haag 2016-02-11) - PLID 68236 - Split single Chargebacks joins into two joins to avoid producing super slow query plans

		switch (m_pReport->nExtendedStyle) {

			case 4: //everything except location reports

				strInnerQuery.Format(" SELECT PatID, PatTotResp, InsTotResp, ProvID, LocID FROM ("
				" SELECT PatID, PatTotResp, InsTotResp, PatientsT.MainPhysician AS ProvID, "
				" PersonT.Location AS LocID FROM (SELECT StatementPatBalanceSub.PatID,  "
				"   SUM(StatementPatBalanceSub.PatCharge - StatementPatBalanceSub.PatPay) "
				" AS PatTotResp,  "
				"   SUM(StatementPatBalanceSub.InsCharge - StatementPatBalanceSub.InsPays) "
				" AS InsTotResp "
				" FROM (SELECT StatementAllData.PatID,  "
				" PatCharge = CASE WHEN StatementAllData.Type "
				"  = 10 THEN (StatementAllData.Total - StatementAllData.Insurance) "
				"  ELSE 0 END,  "
				" PatPay = CASE WHEN StatementAllData.Type "
				"  = 1 OR "
				" StatementAllData.Type = 2 THEN CASE WHEN "
				"  StatementAllData.Insurance <> 0 THEN 0 ELSE "
				"  StatementAllData.Total END ELSE CASE WHEN "
				"  StatementAllData.Type = 3 THEN CASE WHEN "
				"  StatementAllDAta.Insurance <> 0 THEN 0 ELSE "
				"  StatementAllData.Total * - 1 END ELSE 0 END "
				"  END,  "
				" InsCharge = CASE WHEN StatementAllData.Type "
				"  = 10 THEN StatementAllData.Insurance ELSE "
				"  0 END,  "
				" InsPays = CASE WHEN StatementAllData.Type "
				"  = 1 OR "
				" StatementAllData.Type = 2 THEN StatementAllData.Insurance "
				"  ELSE CASE WHEN StatementAllData.Type "
				"  = 3 THEN StatementAllData.Insurance * - 1 "
				"  ELSE 0 END END "
				" FROM (SELECT StmtCharges.ID,  "
				"   StmtCharges.PatID,  "
				"   StmtCharges.Type,  "
				"   StmtCharges.Total,  "
				"   StmtCharges.Insurance, "
				"   1 as StatementType, -1 as GroupFixID "
				"  FROM (SELECT LineItemT.ID,  "
				"  LineItemT.PatientID AS PatID, "
				"   LineItemT.Type,  "
				"  Total = CASE WHEN SUM(ChargeRespT.Amount) "
				"   IS NULL  "
				"  THEN 0 ELSE SUM(ChargeRespT.Amount) "
				"   END,  "
				"   Insurance = SUM(CASE WHEN "
				"   ChargeRespT.InsuredPartyID "
				"   IS NOT NULL and ChargeRespT.InsuredPartyID <> -1 "
				"  THEN ChargeRespT.Amount "
				"   ELSE 0 END) "
				" FROM LineItemT LEFT OUTER JOIN "
				"  ChargeRespT on LineItemT.ID = ChargeRespT.ChargeID "
				" WHERE (LineItemT.Deleted = 0)  "
				"	  AND (LineItemT.Type = 10) "
				"  GROUP BY LineItemT.ID,  "
				"  LineItemT.PatientID,  "
				"  LineItemT.Type)  "
				"   AS StmtCharges "
				"  UNION "
				"  SELECT StmtPays.ID, StmtPays.PatID,  "
				"   StmtPays.Type,  "
				"   StmtPays.UnAppliedAmount,  "
				"   StmtPays.Insurance,   "
				"   2 as StatementType, -2 as GroupFixID "
				"  FROM (SELECT LineItemT.ID,  "
				"  Insurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) "
				"   > 0 THEN CASE WHEN [LineItemT].[Type] "
				"   = 3 THEN - 1 * MIN([LineItemT].[Amount]) "
				"   + SUM(CASE WHEN [AppliesT].[Amount] "
				"   IS NULL  "
				"  THEN 0 ELSE [AppliesT].[Amount] "
				"   END)  "
				"  ELSE MIN([LineItemT].[Amount]) "
				"   - SUM(CASE WHEN [AppliesT].[Amount] "
				"   IS NULL  "
				"  THEN 0 ELSE [AppliesT].[Amount] "
				"   END) END ELSE 0 END,  "
				"  LineItemT.PatientID AS PatID, "
				"   LineItemT.Type,  "
				"  UnAppliedAmount = CASE WHEN "
				"   [LineItemT].[Type] = 3 THEN "
				"   - 1 * MIN([LineItemT].[Amount]) "
				"   + SUM(CASE WHEN [AppliesT].[Amount] "
				"   IS NULL  "
				"  THEN 0 ELSE [AppliesT].[Amount] "
				"   END)  "
				"  ELSE MIN([LineItemT].[Amount]) "
				"   - SUM(CASE WHEN [AppliesT].[Amount] "
				"   IS NULL  "
				"  THEN 0 ELSE [AppliesT].[Amount] "
				"   END) END "
				"  FROM AppliesT RIGHT OUTER JOIN "
				"  PaymentsT ON  "
				"  AppliesT.SourceID = PaymentsT.ID "
				"   RIGHT OUTER JOIN "
				"  LineItemT ON  "
				"  PaymentsT.ID = LineItemT.ID "
				"  LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
				"  LEFT JOIN ChargeBacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
				"  WHERE (LineItemT.Type < 10)  "
				"  AND  "
				"  (LineItemT.Deleted = 0) "
				"  %s " 
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
				"  GROUP BY LineItemT.ID,  "
				"  LineItemT.PatientID,  "
				"  LineItemT.Type "
				" HAVING ((Min(LineItemT.Amount) = 0 AND Min(AppliesT.Amount) IS NULL) OR (CASE WHEN "
				" [LineItemT].[Type] = 3 THEN "
				"  - 1 * MIN([LineItemT].[Amount])  "
				"  + SUM(CASE WHEN [AppliesT].[Amount] IS "
				"  NULL  THEN 0 ELSE [AppliesT].[Amount] END)  "
				" ELSE MIN([LineItemT].[Amount])  "
				" - SUM(CASE WHEN [AppliesT].[Amount] IS "
				" NULL  THEN 0 ELSE [AppliesT].[Amount] END ) "
				" END <> 0)) )  "
				"   AS StmtPays "
				"  UNION "
				"  SELECT StmtApplies.ChargeID,  "
				"   StmtApplies.PatID,  "
				"   StmtApplies.Type,  "
				"   StmtApplies.ApplyAmount,  "
				"   StmtApplies.Insurance, "
				"   3 as StatementType, StmtApplies.PaymentID as GroupFixID " 
				"  FROM (SELECT * "
				"   FROM (SELECT AppliesT.DestID "
				"  AS ChargeID,  "
				"  AppliesT.ID as PaymentID, "
				"  LineItemT1.PatientID AS PatID,  "
				"  LineItemT1.Type,  "
				"  ApplyAmount = CASE  "
				"		WHEN LineItemT1.Type = 3 then -1 * AppliesT.Amount  "
				"		ELSE AppliesT.Amount "
				"		End, "
				"   Insurance = CASE "
				"   WHEN [PaymentsT].[InsuredPartyID] "
				"   > 0 THEN  "
				"		CASE WHEN LineItemT1.Type = 3 then -1 * [AppliesT].[Amount] "
				"		 ELSE AppliesT.Amount "
				"		 END "
				" ELSE 0 END "
				" FROM LineItemT LineItemT1 "
				" RIGHT OUTER JOIN "
				" PaymentsT ON  "
				" LineItemT1.ID = PaymentsT.ID "
				" RIGHT OUTER JOIN "
				" AppliesT ON  "
				" PaymentsT.ID = AppliesT.SourceID "
				" LEFT OUTER JOIN "
				" LineItemT ON  "
				" LineItemT.ID = AppliesT.DestID "
				" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
				" LEFT JOIN ChargeBacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
				" WHERE (LineItemT1.Deleted "
				" = 0) AND  "
				" (LineItemT.Deleted = "
				" 0) AND "
				" (AppliesT.PointsToPayments "
				" = 0) " 
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
				" )  "
				" AS StatementDataAppliesCharges "
				"  UNION "
				"  SELECT * "
				"  FROM (SELECT AppliesT.DestID "
				"  AS ChargeID,  "
				"  AppliesT.ID as PaymentID, "
				"  LineItemT1.PatientID AS "
				"  PatID,  "
				"  LineItemT1.Type, ApplyAmount = CASE "
				"  WHEN LineItemT1.Type = 3 then -1 * AppliesT.Amount  "
				"		ELSE AppliesT.Amount "
				"		END, "
				"	Insurance  =  "
				" CASE WHEN PaymentsT.InsuredPartyID "
				"   > 0 THEN  "
				"CASE WHen LineItemT1.Type = 3 then -1 * AppliesT.Amount "
				"						   ELSE AppliesT.Amount "
				"END "
				" ELSE 0 END "
				"   FROM LineItemT LineItemT1 "
				"  RIGHT OUTER JOIN "
				"  PaymentsT ON  "
				"  LineItemT1.ID = PaymentsT.ID "
				"  LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID "
				"  LEFT JOIN ChargeBacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID "
				"  RIGHT OUTER JOIN "
				"  LineItemT RIGHT OUTER "
				"  JOIN "
				" AppliesT ON  "
				" LineItemT.ID = AppliesT.DestID "
				" LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID "
				" LEFT JOIN ChargeBacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID "
				"  ON  "
				" PaymentsT.ID = AppliesT.SourceID "
				" WHERE (LineItemT1.Deleted "
				"  = 0) AND  "
				" (LineItemT1.Deleted "
				"  = 0) AND "
				" (AppliesT.PointsToPayments "
				"  = 1) " 
					+ GetStatementChargebackString("ChargebacksPayments1") + " " 
					+ GetStatementChargebackString("ChargebacksAdjustments1") + " " 
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
				" )  "
				"  AS StatementDataAppliesPays) "
				"   AS StmtApplies)  "
				" AS StatementAllData)  "
				"   AS StatementPatBalanceSub "
				" GROUP BY StatementPatBalanceSub.PatID)  "
				"  AS StatementPatBalance LEFT JOIN PatientsT ON StatementPatBalance.PatID = PatientsT.PersonID"
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID) StatmentBalanceQ ", strHideUnappliedPrepayments);
			break;


			case 6:  //location reports
				{
					CStringArray aryStr;
					m_pReport->GetExtraValues(aryStr);

					CString strLocation, strLocation2;
					if (aryStr.GetSize() > 0) {
						CString strTemp = aryStr.GetAt(0);
						long nResult = strTemp.Find(":");
						strTemp = strTemp.Left(nResult);
						if (strTemp == "-1")  {
							strLocation = "";
							strLocation2 = "";
						}
						else {
							strLocation.Format(" AND (LineItemT.LocationID = %li) ", atoi(strTemp));
							strLocation2.Format(" AND (LineItemT1.LocationID = %li) ", atoi(strTemp));
						}
					}
					else {
						strLocation = "";
						strLocation2= "";
					}		

					

				//TES 7/17/2014 - PLID 62564 - Added code to hide chargebacks when requested
				strInnerQuery.Format(" SELECT PatID, PatTotResp, InsTotResp, ProvID, LocID FROM ("
					" SELECT PatID, PatTotResp, InsTotResp, PatientsT.MainPhysician AS ProvID, "
					" PersonT.Location AS LocID FROM (SELECT StatementPatBalanceSub.PatID,  "
					"   SUM(StatementPatBalanceSub.PatCharge - StatementPatBalanceSub.PatPay) "
					" AS PatTotResp,  "
					"   SUM(StatementPatBalanceSub.InsCharge - StatementPatBalanceSub.InsPays) "
					" AS InsTotResp "
					" FROM (SELECT StatementAllData.PatID,  "
					" PatCharge = CASE WHEN StatementAllData.Type "
					"  = 10 THEN (StatementAllData.Total - StatementAllData.Insurance) "
					"  ELSE 0 END,  "
					" PatPay = CASE WHEN StatementAllData.Type "
					"  = 1 OR "
					" StatementAllData.Type = 2 THEN CASE WHEN "
					"  StatementAllData.Insurance <> 0 THEN 0 ELSE "
					"  StatementAllData.Total END ELSE CASE WHEN "
					"  StatementAllData.Type = 3 THEN CASE WHEN "
					"  StatementAllDAta.Insurance <> 0 THEN 0 ELSE "
					"  StatementAllData.Total * - 1 END ELSE 0 END "
					"  END,  "
					" InsCharge = CASE WHEN StatementAllData.Type "
					"  = 10 THEN StatementAllData.Insurance ELSE "
					"  0 END,  "
					" InsPays = CASE WHEN StatementAllData.Type "
					"  = 1 OR "
					" StatementAllData.Type = 2 THEN StatementAllData.Insurance "
					"  ELSE CASE WHEN StatementAllData.Type "
					"  = 3 THEN StatementAllData.Insurance * - 1 "
					"  ELSE 0 END END "
					" FROM (SELECT StmtCharges.ID,  "
					"   StmtCharges.PatID,  "
					"   StmtCharges.Type,  "
					"   StmtCharges.Total,  "
					"   StmtCharges.Insurance, "
					"   1 as StatementType, -1 as GroupFixID "
					"  FROM (SELECT LineItemT.ID,  "
					"  LineItemT.PatientID AS PatID, "
					"   LineItemT.Type,  "
					"  Total = CASE WHEN SUM(ChargeRespT.Amount) "
					"   IS NULL  "
					"  THEN 0 ELSE SUM(ChargeRespT.Amount) "
					"   END,  "
					"   Insurance = SUM(CASE WHEN "
					"   ChargeRespT.InsuredPartyID "
					"   IS NOT NULL and ChargeRespT.InsuredPartyID <> -1 "
					"  THEN ChargeRespT.Amount "
					"   ELSE 0 END) "
					" FROM LineItemT LEFT OUTER JOIN "
					"  ChargeRespT on LineItemT.ID = ChargeRespT.ChargeID "
					" WHERE (LineItemT.Deleted = 0)  "
					"	  AND (LineItemT.Type = 10) "
					"  %s "
					"  GROUP BY LineItemT.ID,  "
					"  LineItemT.PatientID,  "
					"  LineItemT.Type)  "
					"   AS StmtCharges "
					"  UNION "
					"  SELECT StmtPays.ID, StmtPays.PatID,  "
					"   StmtPays.Type,  "
					"   StmtPays.UnAppliedAmount,  "
					"   StmtPays.Insurance,   "
					"   2 as StatementType, -2 as GroupFixID "
					"  FROM (SELECT LineItemT.ID,  "
					"  Insurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) "
					"   > 0 THEN CASE WHEN [LineItemT].[Type] "
					"   = 3 THEN - 1 * MIN([LineItemT].[Amount]) "
					"   + SUM(CASE WHEN [AppliesT].[Amount] "
					"   IS NULL  "
					"  THEN 0 ELSE [AppliesT].[Amount] "
					"   END)  "
					"  ELSE MIN([LineItemT].[Amount]) "
					"   - SUM(CASE WHEN [AppliesT].[Amount] "
					"   IS NULL  "
					"  THEN 0 ELSE [AppliesT].[Amount] "
					"   END) END ELSE 0 END,  "
					"  LineItemT.PatientID AS PatID, "
					"   LineItemT.Type,  "
					"  UnAppliedAmount = CASE WHEN "
					"   [LineItemT].[Type] = 3 THEN "
					"   - 1 * MIN([LineItemT].[Amount]) "
					"   + SUM(CASE WHEN [AppliesT].[Amount] "
					"   IS NULL  "
					"  THEN 0 ELSE [AppliesT].[Amount] "
					"   END)  "
					"  ELSE MIN([LineItemT].[Amount]) "
					"   - SUM(CASE WHEN [AppliesT].[Amount] "
					"   IS NULL  "
					"  THEN 0 ELSE [AppliesT].[Amount] "
					"   END) END "
					"  FROM AppliesT RIGHT OUTER JOIN "
					"  PaymentsT ON  "
					"  AppliesT.SourceID = PaymentsT.ID "
					"   RIGHT OUTER JOIN "
					"  LineItemT ON  "
					"  PaymentsT.ID = LineItemT.ID "
					"  LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"  LEFT JOIN ChargeBacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"  WHERE (LineItemT.Type < 10)  "
					"  AND  "
					"  (LineItemT.Deleted = 0) %s "
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
					"   "
					"  %s "
					"  GROUP BY LineItemT.ID,  "
					"  LineItemT.PatientID,  "
					"  LineItemT.Type "
					" HAVING ((Min(LineItemT.Amount) = 0 AND Min(AppliesT.Amount) IS NULL) OR (CASE WHEN "
					" [LineItemT].[Type] = 3 THEN "
					"  - 1 * MIN([LineItemT].[Amount])  "
					"  + SUM(CASE WHEN [AppliesT].[Amount] IS "
					"  NULL  THEN 0 ELSE [AppliesT].[Amount] END)  "
					" ELSE MIN([LineItemT].[Amount])  "
					" - SUM(CASE WHEN [AppliesT].[Amount] IS "
					" NULL  THEN 0 ELSE [AppliesT].[Amount] END ) "
					" END <> 0)) )  "
					"   AS StmtPays "
					"  UNION "
					"  SELECT StmtApplies.ChargeID,  "
					"   StmtApplies.PatID,  "
					"   StmtApplies.Type,  "
					"   StmtApplies.ApplyAmount,  "
					"   StmtApplies.Insurance, "
					"   3 as StatementType, StmtApplies.PaymentID as GroupFixID " 
					"  FROM (SELECT * "
					"   FROM (SELECT AppliesT.DestID "
					"  AS ChargeID,  "
					"  AppliesT.ID as PaymentID, "
					"  LineItemT1.PatientID AS PatID,  "
					"  LineItemT1.Type,  "
					"  ApplyAmount = CASE  "
					"		WHEN LineItemT1.Type = 3 then -1 * AppliesT.Amount  "
					"		ELSE AppliesT.Amount "
					"		End, "
					"   Insurance = CASE "
					"   WHEN [PaymentsT].[InsuredPartyID] "
					"   > 0 THEN  "
					"		CASE WHEN LineItemT1.Type = 3 then -1 * [AppliesT].[Amount] "
					"		 ELSE AppliesT.Amount "
					"		 END "
					" ELSE 0 END "
					" FROM LineItemT LineItemT1 "
					" RIGHT OUTER JOIN "
					" PaymentsT ON  "
					" LineItemT1.ID = PaymentsT.ID "
					" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					" RIGHT OUTER JOIN "
					" AppliesT ON  "
					" PaymentsT.ID = AppliesT.SourceID "
					" LEFT OUTER JOIN "
					" LineItemT ON  "
					" LineItemT.ID = AppliesT.DestID "
					" WHERE (LineItemT1.Deleted "
					" = 0) AND  "
					" (LineItemT.Deleted = "
					" 0) AND "
					" (AppliesT.PointsToPayments "
					" = 0) %s " 
						+ GetStatementChargebackString("ChargebacksPayments") + " "
						+ GetStatementChargebackString("ChargebacksAdjustments") + " "
					")  "
					" AS StatementDataAppliesCharges "
					"  UNION "
					"  SELECT * "
					"  FROM (SELECT AppliesT.DestID "
					"  AS ChargeID,  "
					"  AppliesT.ID as PaymentID, "
					"  LineItemT1.PatientID AS "
					"  PatID,  "
					"  LineItemT1.Type, ApplyAmount = CASE "
					"  WHEN LineItemT1.Type = 3 then -1 * AppliesT.Amount  "
					"		ELSE AppliesT.Amount "
					"		END, "
					"	Insurance  =  "
					" CASE WHEN PaymentsT.InsuredPartyID "
					"   > 0 THEN  "
					"CASE WHen LineItemT1.Type = 3 then -1 * AppliesT.Amount "
					"						   ELSE AppliesT.Amount "
					"END "
					" ELSE 0 END "
					"   FROM LineItemT LineItemT1 "
					"  RIGHT OUTER JOIN "
					"  PaymentsT ON  "
					"  LineItemT1.ID = PaymentsT.ID "
					"  LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID "
					"  LEFT JOIN ChargebacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID "
					"  RIGHT OUTER JOIN "
					"  LineItemT RIGHT OUTER "
					"  JOIN "
					" AppliesT ON  "
					" LineItemT.ID = AppliesT.DestID "
					" LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID "
					" LEFT JOIN ChargebacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID "
					"  ON  "
					" PaymentsT.ID = AppliesT.SourceID "
					" WHERE (LineItemT1.Deleted "
					"  = 0) AND  "
					" (LineItemT1.Deleted "
					"  = 0) AND  "
					" (AppliesT.PointsToPayments "
					"  = 1) %s " 
						+ GetStatementChargebackString("ChargebacksPayments1") + " " 
						+ GetStatementChargebackString("ChargebacksAdjustments1") + " "
						+ GetStatementChargebackString("ChargebacksPayments") + " "
						+ GetStatementChargebackString("ChargebacksAdjustments") + " "
					" )  "
					"  AS StatementDataAppliesPays) "
					"   AS StmtApplies)  "
					" AS StatementAllData)  "
					"   AS StatementPatBalanceSub "
					" GROUP BY StatementPatBalanceSub.PatID)  "
					"  AS StatementPatBalance LEFT JOIN PatientsT ON StatementPatBalance.PatID = PatientsT.PersonID"
					" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID) StatmentBalanceQ ", strLocation, strLocation, strHideUnappliedPrepayments, strLocation, strLocation2);
				}
			break;

			
		}


		//now get our filters
		CString strFilter;
		switch (m_pReport->nID) {
			//by patient
			case 169:
			case 353:
			case 434:
			case 436:
			case 337 : //by location
			case 355:
			case 435: //by last send date
			case 437: //by last send date 7.0
			
				//filter on patient, provider, balance, group, and filter
				AddPartToClause(strFilter, m_pReport->GetPatientFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strFilter, m_pReport->GetProviderFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strFilter, m_pReport->GetExtraFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strFilter, m_pReport->GetGroupFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strFilter, m_pReport->GetFilterFilter(m_nSubLevel, m_nSubRepNum));
			break;

			case 483:
			case 485: 
				
				//filter on patient, balance, group, and filter
				AddPartToClause(strFilter, m_pReport->GetPatientFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strFilter, m_pReport->GetExtraFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strFilter, m_pReport->GetGroupFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strFilter, m_pReport->GetFilterFilter(m_nSubLevel, m_nSubRepNum));

			break;
		}
		
		strFilter.TrimLeft();
		strFilter.TrimRight();


		strFilter.Replace("PatientStatementsQ", "StatmentBalanceQ");
		strFilter.Replace("StatementSubQ", "StatmentBalanceQ");
		strFilter.Replace("PatStatementsByLocQ", "StatmentBalanceQ");
		strFilter.Replace("PatStatements70ByProvQ", "StatmentBalanceQ");
		strFilter.Replace("PatStatementsByProvQ", "StatmentBalanceQ");

		if (!strFilter.IsEmpty()) {
			m_pReport->ConvertCrystalToSql(strInnerQuery, strFilter);
			AddFilter(strInnerQuery, strFilter, TRUE);
		}

		//now that we have concatenated our filters to our inner query, we now need to create our temp table
		CString strTableName;
		strTableName.Format("#TempStatement%lu", GetTickCount());
		
		// (a.walling 2009-09-08 14:02) - PLID 35178 - Use the snapshot connection
		ExecuteSql(GetConnection(), "CREATE TABLE %s (ID INT NOT NULL PRIMARY KEY) ", strTableName);

#ifdef _DEBUG
		CString str;
		str.Format("INSERT INTO %s (ID) "
			" SELECT PatID FROM (%s) StmtTempQ ", strTableName, strInnerQuery);
		MsgBox(str);
#endif

		//now insert into the table
		// (a.walling 2009-09-08 14:02) - PLID 35178 - Use the snapshot connection
		ExecuteSql(GetConnection(), "INSERT INTO %s (ID) "
			" SELECT PatID FROM (%s) StmtTempQ ", strTableName, strInnerQuery);

		//we are all set to go now, so just pass our table name back to the calling function
		return strTableName;
	}NxCatchAll("Error generating Statement Inner Query");

	//if we got here, we failed
	return "";
					
}




// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, use a CComplexReportQuery object
CComplexReportQuery CStatementSqlBuilder::GetStatementSql()
{
	// (a.walling 2006-10-24 13:12) - PLID 16059 - Multiple Resp. Parties.
	//		Filter on PersonT.ID (show multiple) or Primary (No multiple, default)
	CString strRespFilter;
	long nMultiple = GetRemotePropertyInt("StatementsPrintMultipleParties", 0, 0, "<None>", true);
	if ( (nMultiple == 0) || (m_bIsEStatement) ) { // (a.walling 2007-01-15 09:29) - PLID 7069 - Don't use multiple parties when using e-statements
		strRespFilter = " WHERE ResponsiblePartyT.PersonID = PatientsT.PrimaryRespPartyID ";
	}
	else {
		strRespFilter = "";
	}

	// (j.gruber 2007-01-08 16:49) - PLID 17606 - add last payment date and amount to all statement queries
	// (j.gruber 2007-02-22 12:59) - PLID 23408 - added credit card info, check info, and patient coordinator to the queries
	// (j.gruber 2009-04-02 15:03) - PLID 33797 - changed discount fields to sums and added a optional subreport
	// (j.gruber 2009-11-05 17:24) - PLID 36217 - add provider NPI
	// (j.gruber 2009-12-24 12:31) - PLID 17122 - add modifier fields

	switch (m_nSubLevel) {
		case 0: 
		switch (m_pReport->nID) {

		case 169 :
		case 353:
		case 434:
		case 436:
			{

				long nTransFormat, nHeaderFormat;
				nTransFormat = GetRemotePropertyInt("SttmntTransProvFormat", 0, 0, "<None>");
				nHeaderFormat = GetRemotePropertyInt("SttmntHeaderProvFormat", 0, 0, "<None>");
				CString strTransProv = GetStatementProvString(nTransFormat);
				CString strHeaderProv = GetStatementProvString(nHeaderFormat);
				CString strHideUnAppliedPrepayments = GetStatementUnAppliedPrePaymentsString();
				CString strChargeDescription = GetStatementChargeDescription();

				//PLID 19451: create the temp table to filter with
				CString strInnerFilter = "", strInnerFilterApp = "";
				if (!m_pReport->GetFilter(m_nSubLevel, m_nSubRepNum).IsEmpty())  {
					CString strTmpTable = CreateStatementFilterTable();
					strInnerFilter.Format(" AND LineItemT.PatientID IN (SELECT ID FROM %s) ", strTmpTable);
					strInnerFilterApp.Format(" AND LineItemT1.PatientID IN (SELECT ID FROM %s) ", strTmpTable);

					//if we got here and have no tmp table, that means we got an error in the last function
					if (strTmpTable.IsEmpty()) {
						return CComplexReportQuery();
					}
				}
				else {
					//we have no filters, so running the extra part isn't going to help us, so just skip it
				}
				
				
				// (j.gruber 2007-05-01 17:09) - PLID 25745 - take out the credit card number
				// (j.gruber 2007-05-15 09:08) - PLID 25987 - take out credit card expiration dates
				// (j.gruber 2007-05-30 11:42) - PLID 25978 - add discount info and discount category to statements
				// (e.lally 2007-07-13) PLID 26670 - Updated all references to PaymentPlansT. CCType with link to CardName, aliased as CCType where applicable.
				// (j.jones 2008-09-05 10:19) - PLID 30288 - supported MailSentNotesT
				// (j.gruber 2009-11-25 12:12) - PLID 36430 - added cptcode
				// (j.gruber 2010-06-14 15:22) - PLID 36484 - added billing notes
				// (j.gruber 2011-07-01 16:53) - PLID 44831 - take out void and originals
				// (a.wilson 2012-2-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
				// (j.gruber 2014-03-03 14:27) - PLID 61144 - add new diag codes, chane whichcode structure
				//TES 7/17/2014 - PLID 62565 - Added code to hide chargebacks when requested
				// (j.jones 2015-03-09 09:48) - PLID 64937 - if the description begins with 'Corrected Charge',
				// 'Corrected Payment', etc., strip that off
				// (r.goldschmidt 2015-11-10 16:30) - PLID 65568 - Last payment should exclude voided payments
				// (c.haag 2016-02-11) - PLID 68236 - Split single Chargebacks joins into two joins to avoid producing super slow query plans
				// (c.haag 2016-02-11) - PLID 68251 - Replaced XML PATH evaluations with CTEs to improve performance. I also reformatted
				// the query to be easier to read.
				CString strCTE = R"(
;WITH ChargeDiagCodes_CTE(RowNumber, ChargeID, ICD9DiagCodeNumber, ICD10DiagCodeNumber)
AS
(
	SELECT 
		ROW_NUMBER() OVER (PARTITION BY ChargeWhichCodesT.ChargeID ORDER BY BillDiagCodeT.OrderIndex ASC) AS RowNumber,
		ChargeWhichCodesT.ChargeID,
		ICD9DiagCodesT.CodeNumber AS ICD9DiagCodeNumber,
		ICD10DiagCodesT.CodeNumber AS ICD10DiagCodeNumber
	FROM ChargeWhichCodesT
	INNER JOIN BillDiagCodeT ON BillDiagCodeT.ID = ChargeWhichCodesT.BillDiagCodeID
	LEFT JOIN DiagCodes ICD9DiagCodesT ON BillDiagCodeT.ICD9DiagID = ICD9DiagCodesT.ID
	LEFT JOIN DiagCodes ICD10DiagCodesT ON BillDiagCodeT.ICD10DiagID = ICD10DiagCodesT.ID
)
)";
				CString part1, part1b, part1c, part1d, part1e, part2, part3, part4, part5, part6, part7, part8, part8b, part9, part10, part11, part11b, part11c, part12, part10b;
				part1.Format(R"(
SELECT
  StatementSubQ.ID,
  StatementSubQ.PatientID,
  StatementSubQ.PatID AS PatID,
  StatementSubQ.Type,
  StatementSubQ.Total,
  StatementSubQ.Description,
  StatementSubQ.Date AS TDate,
  StatementSubQ.Insurance,
  PatPerson.Last,
  PatPerson.First,
  PatPerson.Middle,
  PatPerson.Address1,
  PatPerson.Address2,
  PatPerson.City,
  PatPerson.State,
  PatPerson.Zip,
  PatPerson.First + ' ' + PatPerson.Middle + ' ' + PatPerson.Last AS PatForward,
  PatPerson.Last + ', ' + PatPerson.First + ' ' + PatPerson.Middle AS PatComma,
  (%s WHERE PersonT.ID = ProvPerson.ID) AS DocName,
  ProvPerson.Address1 AS DocAddress1,
  ProvPerson.Address2 AS DocAddress2,
  ProvPerson.City AS DocCity,
  ProvPerson.State AS DocState,
  ProvPerson.Zip AS DocZip,
  [ProvidersT].[Fed Employer ID] AS ProvTaxID,
  LocationsT.Name PracName,
  LocationsT.Address1 PracAddress1,
  LocationsT.Address2 AS PracAddress2,
  LocationsT.City PracCity,
  LocationsT.State PracState,
  LocationsT.Zip AS PracZip,
  LocationsT.Phone AS PracPhone,
  LocationsT.Fax AS PracFax,
  StatementSubQ.ProvID AS ProvID2,
  StatementSubQ.BillId,
  StatementSubQ.BillDate AS BillDate,
  StatementSubQ.BillDescription,
  PatPerson.Birthdate,
  ICD9T1.CodeNumber AS ICD9Code1,
  ICD9T2.CodeNumber AS ICD9Code2,
  ICD9T3.CodeNumber AS ICD9Code3,
  ICD9T4.CodeNumber AS ICD9Code4,
  ICD10T1.CodeNumber AS ICD10Code1,
  ICD10T2.CodeNumber AS ICD10Code2,
  ICD10T3.CodeNumber AS ICD10Code3,
  ICD10T4.CodeNumber AS ICD10Code4,
  WhichCodesQ.WhichCodes9,
  WhichCodesQ.WhichCodes10,
  WhichCodesQ.WhichCodesBoth,
  PatPerson.Location,
  StatementSubQ.StatementType,
  StatementSubQ.GroupFixID,
  StatementSubQ.Appdate,
  StatementSubQ.StartTime,
  StatementSubQ.ARDate,
)", strHeaderProv);
				part1b = R"(  StatementSubQ.PatTotResp,
  StatementSubQ.InsTotResp, )";
				part1c = R"(  StatementSubQ.Age,
  StatementSubQ.TransProv,
  StatementSubQ.Prepayment,
  StatementSubQ.Quantity,
  StatementSubQ.Thirty,
  StatementSubQ.Sixty,
  StatementSubQ.Ninety,
  StatementSubQ.NinetyPlus,
  ProvidersT.EIN AS ProvEIN,
  ProvidersT.License AS ProvLicense,
  ProvidersT.UPIN AS ProvUPIN,
  ProvidersT.[DEA Number] AS ProvDEA,
  ProvidersT.[BCBS Number] AS ProvBCBS,
  ProvidersT.[Medicare Number] AS ProvMedicare,
  ProvidersT.[Medicaid Number] AS ProvMedicaid,
  ProvidersT.[Workers Comp Number] AS ProvWorkersComp,
  ProvidersT.[Other ID Number] AS ProvOtherID,
  ProvidersT.[Other ID Description] AS ProvOtherIDDesc,
  StatementEndQ.DocName AS DocName2,
  StatementEndQ.ProvID AS ProvID,
  StatementEndQ.SuppressStatement,
  StatementEndQ.PatID AS PatID2,
  StatementEndQ.StatementNote,
  StatementEndQ.PriInsCo,
  StatementEndQ.PriGuarForward,
  StatementEndQ.PriGuarComma,
  StatementEndQ.PriInsFirst,
  StatementEndQ.PriInsMiddle,
  StatementEndQ.PriInsLast,
  StatementEndQ.PersonID,
  StatementEndQ.SecInsCo,
  StatementEndQ.SecGuarForward,
  StatementEndQ.SecGuarComma,
  StatementEndQ.SecInsfirst,
  StatementEndQ.SecInsMiddle,
  StatementEndQ.SecInsLast,
  StatementEndQ.PersID,
  StatementSubQ.LocationFixID AS LocID,
  StatementEndQ.RespID,
  StatementEndQ.RespFirst,
  StatementEndQ.RespMiddle,
  StatementEndQ.RespLast,
  StatementEndQ.RespAdd1,
  StatementEndQ.RespAdd2,
  StatementEndQ.RespCity,
  StatementEndQ.RespState,
  StatementEndQ.RespZip,
  (SELECT
      MAX(Date)
    FROM
      MailSent
      INNER JOIN MailSentNotesT
      ON MailSent.MailID = MailSentNotesT.MailID
    WHERE
      (MailSentNotesT.Note LIKE '%%Patient Statement%%Printed%%' OR
      MailSentNotesT.Note LIKE '%%Patient Statement%%Run%%' OR
      MailSentNotesT.Note LIKE'%%E-Statement%%Exported%%' ) AND
      PersonID = StatementSubQ.PatID
  ) AS LastSentDate,
  PatPerson.HomePhone AS PatPhone,
  StatementSubQ.FullChargeNoTax,
  StatementSubQ.ChargeTax1,
  StatementSubQ.ChargeTax2,
  StatementSubQ.TaxRate1,
  StatementSubQ.TaxRate2,
  (SELECT
      COUNT(*)
    FROM
      ResponsiblePartyT
    WHERE
      PatientID = StatementSubQ.PatID
  ) AS RespPartyCount,
  (SELECT
      Top 1 Date
    FROM
      PaymentsT
      INNER JOIN LineItemT
      ON PaymentsT.ID = LineItemT.ID LEFT
      JOIN LineItemCorrectionsT OriginalLineItemsT
      ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID LEFT
      JOIN LineItemCorrectionsT VoidingLineItemsT
      ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
    WHERE
      LineItemT.DELETED = 0 AND
      PaymentsT.InsuredPartyID = -1 AND
      LineItemT.PatientID = StatementSubQ.PatID AND
      LineItemT.Type = 1 AND
      OriginalLineItemsT.OriginalLineItemID IS NULL AND
      VoidingLineItemsT.VoidingLineItemID IS NULL
    ORDER BY
      LineItemT.InputDate DESC
  ) AS LastPatientPaymentDate,
  (SELECT
      Top 1 Date
    FROM
      PaymentsT
      INNER JOIN LineItemT
      ON PaymentsT.ID = LineItemT.ID LEFT
      JOIN ChargebacksT
      ON PaymentsT.ID = ChargebacksT.PaymentID LEFT
      JOIN LineItemCorrectionsT OriginalLineItemsT
      ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID LEFT
      JOIN LineItemCorrectionsT VoidingLineItemsT
      ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
    WHERE
      LineItemT.DELETED = 0 AND
      PaymentsT.InsuredPartyID <> -1 AND
      LineItemT.PatientID = StatementSubQ.PatID AND
      LineItemT.Type = 1 AND
      ChargebacksT.ID IS NULL AND
      OriginalLineItemsT.OriginalLineItemID IS NULL AND
      VoidingLineItemsT.VoidingLineItemID IS NULL
    ORDER BY
      LineItemT.InputDate DESC
  ) AS LastInsurancePaymentDate,
  (SELECT
      Top 1 Amount
    FROM
      PaymentsT
      INNER JOIN LineItemT
      ON PaymentsT.ID = LineItemT.ID LEFT
      JOIN LineItemCorrectionsT OriginalLineItemsT
      ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID LEFT
      JOIN LineItemCorrectionsT VoidingLineItemsT
      ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
    WHERE
      LineItemT.DELETED = 0 AND
      PaymentsT.InsuredPartyID = -1 AND
      LineItemT.PatientID = StatementSubQ.PatID AND
      LineItemT.Type = 1 AND
      OriginalLineItemsT.OriginalLineItemID IS NULL AND
      VoidingLineItemsT.VoidingLineItemID IS NULL
    ORDER BY
      LineItemT.InputDate DESC
  ) AS LastPatientPaymentAmount,
  (SELECT
      Top 1 Amount
    FROM
      PaymentsT
      INNER JOIN LineItemT
      ON PaymentsT.ID = LineItemT.ID LEFT
      JOIN ChargebacksT
      ON PaymentsT.ID = ChargebacksT.PaymentID LEFT
      JOIN LineItemCorrectionsT OriginalLineItemsT
      ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID LEFT
      JOIN LineItemCorrectionsT VoidingLineItemsT
      ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
    WHERE
      LineItemT.DELETED = 0 AND
      PaymentsT.InsuredPartyID <> -1 AND
      LineItemT.PatientID = StatementSubQ.PatID AND
      LineItemT.Type = 1 AND
      ChargebacksT.ID IS NULL AND
      OriginalLineItemsT.OriginalLineItemID IS NULL AND
      VoidingLineItemsT.VoidingLineItemID IS NULL
    ORDER BY
      LineItemT.InputDate DESC
  ) AS LastInsurancePaymentAmount,
  StatementSubQ.CCType,
  CASE WHEN Len(StatementSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(StatementSubQ.CCNumber,
  4) END AS CCNumber,
  StatementSubQ.CheckNo,
  StatementSubQ.BankNo,
  StatementSubQ.CheckAcctNo,
  StatementSubQ.CCHoldersName,
  Convert(datetime, NULL) AS CCExpDate,
  StatementSubQ.CCAuthNo,
  StatementSubQ.BankRoutingNum,
  StatementEndQ.PatCoordFirst,
  StatementEndQ.PatCoordMiddle,
  StatementEndQ.PatCoordLast,
  StatementSubQ.PercentOff,
  StatementSubQ.DiscountAmt,
  StatementSubQ.DiscountCategoryDesc,
  StatementEndQ.ProviderNPI,
  StatementSubQ.CPTCode,
  StatementSubQ.CPTModifier1,
  StatementSubQ.CPTModifier2,
  StatementSubQ.CPTModifier3,
  StatementSubQ.CPTModifier4,
  StatementSubQ.BillStatementNote,
  StatementSubQ.LineItemStatementNote,
  CASE WHEN StatementSubQ.BillStatementNote = '' OR StatementSubQ.BillStatementNote IS NULL THEN 1 ELSE 0 END AS SuppressBillStatementNote,
  CASE WHEN StatementSubQ.LineItemStatementNote = '' OR StatementSubQ.LineItemStatementNote IS NULL THEN 1 ELSE 0 END AS SuppressLineItemStatementNote
FROM
  (SELECT
      StatementAllData.*,
      MIN(NextApp.AppDate)
      AS AppDate,
      MIN(NextApp.StartTime)
      AS StartTime,
      LineItemT.Date
      AS ARDate,
      CASE WHEN StatementAR.Thirty IS NULL THEN 0 ELSE StatementAR.Thirty END
      AS Thirty,
      CASE WHEN StatementAR.Sixty IS NULL THEN 0 ELSE StatementAR.Sixty END
      AS Sixty,
      CASE WHEN StatementAR.Ninety IS NULL THEN 0 ELSE StatementAR.Ninety END
      AS Ninety,
      CASE WHEN StatementAR.NinetyPlus IS NULL THEN 0 ELSE StatementAR.NinetyPlus END
      AS NinetyPlus,
)";

				part1d = R"(      StatementPatBalance.PatTotResp, StatementPatBalance.InsTotResp, )";
				part1e = R"(      Age = CASE WHEN StatementAllData.BirthDate IS NULL THEN - 1 ELSE DATEDIFF(YYYY,
      StatementAllData.Birthdate,
      GetDate()) - CASE WHEN MONTH(StatementAllData.Birthdate) > MONTH(GetDate()) OR
      (MONTH(StatementAllData.Birthdate) = MONTH(GetDate()) AND
      DAY(StatementAllData.Birthdate) > DAY(GetDate())) THEN 1 ELSE 0 END END
    FROM
      (SELECT
          StmtCharges.ID,
          StmtCharges.PatientID,
          StmtCharges.PatID,
          StmtCharges.Type,
          StmtCharges.Total,
          StmtCharges.Description,
          StmtCharges.Date,
          StmtCharges.Insurance,
          StmtCharges.ProvID,
          StmtCharges.BillID,
          StmtCharges.BillDate,
          StmtCharges.BillDescription,
          StmtCharges.BirthDate,
          1
          AS StatementType,
          -1
          AS GroupFixID,
          StmtCharges.LocationFixID,
          StmtCharges.TransProv,
          StmtCharges.PrePayment,
          StmtCharges.FullChargeNoTax,
          StmtCharges.ChargeTax1,
          StmtCharges.ChargeTax2,
          StmtCharges.TaxRate1,
          StmtCharges.TaxRate2,
          StmtCharges.Quantity,
'' AS CCType,
'' AS CCNumber,
'' AS CheckNo,
'' AS BankNo,
'' AS CheckAcctNo,
'' AS CCHoldersName,
          NULL
          AS CCExpDate,
'' AS CCAuthNo,
'' AS BankRoutingNum,
          StmtCharges.PercentOff,
          StmtCharges.DiscountAmt,
          StmtCharges.DiscountCategoryDesc,
          StmtCharges.CPTCode,
          StmtCharges.CPTModifier1,
          StmtCharges.CPTModifier2,
          StmtCharges.CPTModifier3,
          StmtCharges.CPTModifier4,
          StmtCharges.BillStatementNote,
          StmtCharges.LineItemStatementNote
)";
				part2.Format(R"(        FROM
          (SELECT
              LineItemT.ID,
              PatientsT.UserDefinedID
              AS PatientID,
              LineItemT.PatientID
              AS PatID,
              LineItemT.Type,
              Total = CASE WHEN SUM(ChargeRespT.Amount) IS NULL THEN 0 ELSE SUM(ChargeRespT.Amount) END,
              Description = %s,
              LineItemT.Date,
              Insurance = SUM(CASE WHEN ChargeRespT.InsuredPartyID IS NOT NULL AND ChargeRespT.InsuredPartyID <> -1 THEN ChargeRespT.Amount ELSE 0 END),
              ProvidersT.PersonID
              AS ProvID,
              BillsT.ID
              AS BillID,
              BillsT.Date
              AS BillDate,
              BillsT.Description
              AS BillDescription,
              PersonT.BirthDate,
              LineItemT.LocationID
              AS LocationFixID,
              Round(Convert(money,(((MIN(LineItemT.[Amount]) *Min([Quantity]) *(CASE WHEN(MIN(CPTMultiplier1) IS NULL) THEN 1 ELSE MIN(CPTMultiplier1) END) *(CASE WHEN MIN(CPTMultiplier2) IS NULL THEN 1 ELSE MIN(CPTMultiplier2) END) *(CASE WHEN MIN(CPTMultiplier3) IS NULL THEN 1 ELSE MIN(CPTMultiplier3) END) *(CASE WHEN MIN(CPTMultiplier4) IS NULL THEN 1 ELSE MIN(CPTMultiplier4) END) *(CASE WHEN(MIN([TotalPercentOff]) IS NULL) THEN 1 ELSE((100-Convert(float,
              MIN([TotalPercentOff]))) /100) END) -(CASE WHEN MIN([TotalDiscount]) IS NULL THEN 0 ELSE MIN([TotalDiscount]) END))))),
              2)
              AS FullChargeNoTax,
              Round(Convert(money,(((MIN(LineItemT.[Amount]) *Min([Quantity]) *(CASE WHEN(MIN(CPTMultiplier1) IS NULL) THEN 1 ELSE MIN(CPTMultiplier1) END) *(CASE WHEN MIN(CPTMultiplier2) IS NULL THEN 1 ELSE MIN(CPTMultiplier2) END) *(CASE WHEN MIN(CPTMultiplier3) IS NULL THEN 1 ELSE MIN(CPTMultiplier3) END) *(CASE WHEN MIN(CPTMultiplier4) IS NULL THEN 1 ELSE MIN(CPTMultiplier4) END) *(CASE WHEN(MIN([TotalPercentOff]) IS NULL) THEN 1 ELSE((100-Convert(float,
              MIN([TotalPercentOff]))) /100) END) -(CASE WHEN MIN([TotalDiscount]) IS NULL THEN 0 ELSE MIN([TotalDiscount]) END))))),
              2) * MIN((ChargesT.TaxRate) - 1)
              AS ChargeTax1,
              Round(Convert(money,(((MIN(LineItemT.[Amount]) *Min([Quantity]) *(CASE WHEN(MIN(CPTMultiplier1) IS NULL) THEN 1 ELSE MIN(CPTMultiplier1) END) *(CASE WHEN MIN(CPTMultiplier2) IS NULL THEN 1 ELSE MIN(CPTMultiplier2) END) *(CASE WHEN MIN(CPTMultiplier3) IS NULL THEN 1 ELSE MIN(CPTMultiplier3) END) *(CASE WHEN MIN(CPTMultiplier4) IS NULL THEN 1 ELSE MIN(CPTMultiplier4) END) *(CASE WHEN(MIN([TotalPercentOff]) IS NULL) THEN 1 ELSE((100-Convert(float,
              MIN([TotalPercentOff]))) /100) END) -(CASE WHEN MIN([TotalDiscount]) IS NULL THEN 0 ELSE MIN([TotalDiscount]) END))))),
              2) * MIN((ChargesT.TaxRate2) - 1)
              AS ChargeTax2,
              MIN(ChargesT.TaxRate)
              AS TaxRate1,
              MIN(ChargesT.TaxRate2)
              AS TaxRate2,
              (%s
                WHERE
                  PersonT.ID = ChargesT.DoctorsProviders
              )
              AS TransProv,
              0
              AS PrePayment,
              ChargesT.Quantity,
              COALESCE(TotalPercentOff,
              0)
              AS PercentOff,
              COALESCE(TotalDiscount,
              convert(money,
              0))
              AS DiscountAmt,
              dbo.GetChargeDiscountList(ChargesT.ID)
              AS DiscountCategoryDesc,
              CPTCodeT.Code
              AS CPTCode,
              ChargesT.CPTModifier
              AS CPTModifier1,
              ChargesT.CPTModifier2,
              ChargesT.CPTModifier3,
              ChargesT.CPTModifier4,
              (SELECT
                  Left(Note,
                  1000)
                FROM
                  Notes
                WHERE
                  ShowOnStatement = 1 AND
                  Notes.BillID = BillsT.ID
              )
              AS BillStatementNote,
              (SELECT
                  Left(Note,
                  1000)
                FROM
                  Notes
                WHERE
                  ShowOnStatement = 1 AND
                  Notes.LineItemID = ChargesT.ID
              )
              AS LineItemStatementNote
)"
, strChargeDescription, strTransProv);

				part3.Format(R"(            FROM
              LineItemT LEFT
              OUTER JOIN PatientsT LEFT
              OUTER JOIN ProvidersT
              ON PatientsT.MainPhysician = ProvidersT.PersonID LEFT
              OUTER JOIN PersonT PersonT1
              ON ProvidersT.PersonID = PersonT1.ID LEFT
              OUTER JOIN LocationsT RIGHT
              OUTER JOIN PersonT
              ON LocationsT.ID = PersonT.Location
              ON PatientsT.PersonID = PersonT.ID
              ON LineItemT.PatientID = PatientsT.PersonID LEFT
              OUTER JOIN BillsT RIGHT
              OUTER JOIN ServiceT LEFT
              OUTER JOIN CPTCodeT
              ON ServiceT.ID = CPTCodeT.ID RIGHT
              OUTER JOIN ChargesT
              ON ServiceT.ID = ChargesT.ServiceID
              ON BillsT.ID = ChargesT.BillID LEFT
              JOIN LineItemCorrectionsT LineItemCorrectionsT_NewCharge
              ON ChargesT.ID = LineItemCorrectionsT_NewCharge.NewLineItemID LEFT
              JOIN BillDiagCodeFlat4V
              ON BillsT.ID = BillDiagCodeFlat4V.BillID LEFT
              JOIN DiagCodes ICD9T1
              ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID LEFT
              JOIN DiagCodes ICD9T2
              ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID LEFT
              JOIN DiagCodes ICD9T3
              ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID LEFT
              JOIN DiagCodes ICD9T4
              ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID LEFT
              JOIN DiagCodes ICD10T1
              ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID LEFT
              JOIN DiagCodes ICD10T2
              ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID LEFT
              JOIN DiagCodes ICD10T3
              ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID LEFT
              JOIN DiagCodes ICD10T4
              ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID LEFT
              JOIN
              (SELECT
                  ChargeID,
                  SUM(Percentoff)
                  AS TotalPercentOff,
                  SUM(Discount)
                  AS TotalDiscount
                FROM
                  ChargeDiscountsT
                WHERE
                  DELETED = 0
                GROUP BY
                  ChargeID
              ) TotalDiscountsQ
              ON ChargesT.ID = TotalDiscountsQ.ChargeID LEFT
              OUTER JOIN ChargeRespT
              ON ChargesT.ID = ChargeRespT.ChargeID
              ON LineItemT.ID = ChargesT.ID LEFT
              JOIN BillCorrectionsT
              ON BillsT.ID = BillCorrectionsT.OriginalBillID LEFT
              JOIN LineItemCorrectionsT
              ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID LEFT
              JOIN LineItemCorrectionsT VoidT
              ON ChargesT.ID = VoidT.VoidingLineItemID
            WHERE
              (PatientsT.PersonID > 0) AND
              (LineItemT.Deleted = 0) AND
              (BillsT.Deleted = 0) AND
              BillCorrectionsT.ID IS NULL AND
              LineItemCorrectionsT.ID IS NULL AND
              VoidT.ID IS NULL AND
              (LineItemT.Type = 10) %s
            GROUP BY
              LineItemT.ID,
              PatientsT.UserDefinedID,
              LineItemT.PatientID,
              LineItemT.Type,
              CPTCodeT.Code,
              LineItemT.Description,
              LineItemCorrectionsT_NewCharge.NewLineItemID,
              LineItemT.Date,
              ProvidersT.PersonID,
              BillsT.ID,
              BillsT.Date,
              BillsT.Description,
              PersonT.BirthDate,
              LineItemT.LocationID,
              ChargesT.DoctorsProviders,
              ChargesT.Quantity,
              TotalPercentOff,
              TotalDiscount,
              ChargesT.ID,
              CPTCodeT.Code,
              LineItemCorrectionsT_NewCharge.NewLineItemID,
              ChargesT.CPTModifier,
              ChargesT.CPTModifier2,
              ChargesT.CPTModifier3,
              ChargesT.CPTModifier4
          )
          AS StmtCharges
)"
, strInnerFilter);

				part4 = R"(        UNION
        SELECT
          StmtPays.ID,
          StmtPays.PatientID,
          StmtPays.PatID,
          StmtPays.Type,
          StmtPays.UnAppliedAmount,
          StmtPays.Description,
          StmtPays.Date,
          StmtPays.Insurance,
          StmtPays.ProvID,
          StmtPays.BillID,
          StmtPays.BillDate,
          StmtPays.BillDescription,
          StmtPays.BirthDate,
          2
          AS StatementType,
          -2
          AS GroupFixID,
          StmtPays.LocationFixID,
          StmtPays.TransProv,
          StmtPays.PrePayment,
          0
          AS FullChargeNoTax,
          0
          AS ChargeTax1,
          0
          AS ChargeTax2,
          0
          AS TaxRate1,
          0
          AS TaxRate2,
          0
          AS Quantity,
          StmtPays.CCType,
          StmtPays.CCNumber,
          StmtPays.CheckNo,
          StmtPays.BankNo,
          StmtPays.CheckAcctNo,
          StmtPays.CCHoldersName,
          StmtPays.CCExpDate,
          StmtPays.CCAuthNo,
          StmtPays.BankRoutingNum,
          0
          AS PercentOff,
          Convert(money,
          0)
          AS DiscountAmt,
'' AS DiscountCategoryDesc,
'' AS CPTCode,
'' AS CPTModifier1,
'' AS CPTModifier2,
'' AS CPTModifier3,
'' AS CPTModifier4,
          StmtPays.BillStatementNote,
          StmtPays.LineItemStatementNote
        FROM
          (SELECT
              LineItemT.ID,
              Insurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) > 0 THEN CASE WHEN [LineItemT].[Type] = 3 THEN MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END,
)";

				part5.Format(R"(              PatientsT.UserDefinedID
              AS PatientID,
              PatientsT.PersonID
              AS PatID,
              ProvidersT.PersonID
              AS ProvID,
              LineItemT.Type,
              UnAppliedAmount = CASE WHEN [LineItemT].[Type] = 3 THEN MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END,(CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
              NULL AND
              LineItemT.Type = 1 AND
              Left(LineItemT.Description,
              Len('Corrected Payment' )) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description,
              Len(LineItemT.Description) -Len('Corrected Payment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
              NULL AND
              LineItemT.Type = 2 AND
              Left(LineItemT.Description,
              Len('Corrected Adjustment' )) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description,
              Len(LineItemT.Description) -Len('Corrected Adjustment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
              NULL AND
              LineItemT.Type = 3 AND
              Left(LineItemT.Description,
              Len('Corrected Refund' )) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description,
              Len(LineItemT.Description) -Len('Corrected Refund' ))) ELSE LineItemT.Description END)
              AS Description,
              LineItemT.Date,
              LineItemT.ID
              AS BillID,(CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
              NULL AND
              LineItemT.Type = 1 AND
              Left(LineItemT.Description,
              Len('Corrected Payment' )) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description,
              Len(LineItemT.Description) -Len('Corrected Payment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
              NULL AND
              LineItemT.Type = 2 AND
              Left(LineItemT.Description,
              Len('Corrected Adjustment' )) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description,
              Len(LineItemT.Description) -Len('Corrected Adjustment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
              NULL AND
              LineItemT.Type = 3 AND
              Left(LineItemT.Description,
              Len('Corrected Refund' )) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description,
              Len(LineItemT.Description) -Len('Corrected Refund' ))) ELSE LineItemT.Description END)
              AS BillDescription,
              LineItemT.Date
              AS BillDate,
              PersonT.BirthDate,
              LineItemT.LocationID
              AS LocationFixID,
              (%s
                WHERE
                  PersonT.ID = PaymentsT.ProviderID
              )
              AS TransProv,
              PaymentsT.PrePayment
              AS Prepayment,
              CreditCardNamesT.CardName
              AS CCType,
              PaymentPlansT.CCNumber,
              PaymentPlansT.CheckNo,
              PaymentPlansT.BankNo,
              PaymentPlansT.CheckAcctNo,
              PaymentPlansT.CCHoldersName,
              PaymentPlansT.CCExpDate,
              PaymentPlansT.CCAuthNo,
              PaymentPlansT.BankRoutingNum,
'' AS BillStatementNote,
              (SELECT
                  Left(Note,
                  1000)
                FROM
                  Notes
                WHERE
                  ShowOnStatement = 1 AND
                  Notes.LineItemID = PaymentsT.ID
              )
              AS LineItemStatementNote
            FROM
              LineItemT
              INNER JOIN PaymentsT
              ON LineItemT.ID = PaymentsT.ID LEFT
              JOIN PaymentPlansT
              ON PaymentsT.ID = PaymentPlansT.ID 
			  -- (c.haag 2016-02-09) - PLID 68236 - Join ChargebacksT twice; one per field instead of trying to join them once (causing spooling-related ineffiency)
			  LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID 
			  LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID 
			  LEFT JOIN CreditCardNamesT
              ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID LEFT
              JOIN AppliesT
              ON PaymentsT.ID = AppliesT.SourceID LEFT
              JOIN PatientsT
              ON LineItemT.PatientID = PatientsT.PersonID
              INNER JOIN PersonT
              ON PatientsT.PersonID = PersonT.ID LEFT
              JOIN LocationsT
              ON PersonT.Location = LocationsT.ID LEFT
              JOIN ProvidersT
              ON PatientsT.MainPhysician = ProvidersT.PersonID LEFT
              JOIN PersonT PersonT1
              ON ProvidersT.PersonID = PersonT1.ID LEFT
              JOIN LineItemCorrectionsT
              ON PaymentsT.ID = LineItemCorrectionsT.OriginalLineItemID LEFT
              JOIN LineItemCorrectionsT VoidT
              ON PaymentsT.ID = VoidT.VoidingLineItemID LEFT
              JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay
              ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID
            WHERE
              (LineItemT.Type < 10) AND
              (LineItemT.Deleted = 0) AND
              LineItemCorrectionsT.ID IS NULL AND
              VoidT.ID IS NULL AND
              (PatientsT.PersonID > 0) %s %s
)"
// -- (c.haag 2016-02-09) - PLID 68236 - Use the new chargeback tables
+ GetStatementChargebackString("ChargebacksPayments") + " " + GetStatementChargebackString("ChargebacksAdjustments")
, strTransProv, strHideUnAppliedPrepayments, strInnerFilter);

				part6 = R"(            GROUP BY
              LineItemT.ID,
              PatientsT.UserDefinedID,
              PatientsT.PersonID,
              ProvidersT.PersonID,
              LineItemT.Type,
              LineItemT.Description,
              LineItemT.Date,
              LineItemCorrectionsT_NewPay.NewLineItemID,
              PersonT.BirthDate,
              LineItemT.LocationID,
              PaymentsT.ProviderID,
              PaymentsT.PrePayment,
              CreditCardNamesT.CardName,
              PaymentPlansT.CCNumber,
              PaymentPlansT.CheckNo,
              PaymentPlansT.BankNo,
              PaymentPlansT.CheckAcctNo,
              PaymentPlansT.CCHoldersName,
              PaymentPlansT.CCExpDate,
              PaymentPlansT.CCAuthNo,
              PaymentPlansT.BankRoutingNum,
              PaymentsT.ID
            HAVING
              ((MIN(LineItemT.Amount) = 0 AND
              MIN(AppliesT.Amount) IS NULL) OR
              (CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END <> 0))
          )
          AS StmtPays
        UNION
)";

				part7 = R"(        SELECT
          StmtApplies.ChargeID,
          StmtApplies.PatientID,
          StmtApplies.PatID,
          StmtApplies.Type,
          StmtApplies.ApplyAmount,
          StmtApplies.Description,
          StmtApplies.Date,
          StmtApplies.Insurance,
          StmtApplies.ProvID,
          StmtApplies.BillID,
          StmtApplies.BillDate,
          StmtApplies.BillDescription,
          StmtApplies.BirthDate,
          3
          AS StatementType,
          StmtApplies.PaymentID
          AS GroupFixID,
          StmtApplies.LocationFixID,
          StmtApplies.TransProv,
          StmtApplies.PrePayment,
          0
          AS FullChargeNoTax,
          0
          AS ChargeTax1,
          0
          AS ChargeTax2,
          0
          AS TaxRate1,
          0
          AS TaxRate2,
          0
          AS Quantity,
          StmtApplies.CCType,
          StmtApplies.CCNumber,
          StmtApplies.CheckNo,
          StmtApplies.BankNo,
          StmtApplies.CheckAcctNo,
          StmtApplies.CCHoldersName,
          StmtApplies.CCExpDate,
          StmtApplies.CCAuthNo,
          StmtApplies.BankRoutingNum,
          0
          AS PercentOff,
          Convert(money,
          0)
          AS DiscountAmt,
'' AS DiscountCategoryDesc,
'' AS CPTCode,
'' AS CPTModifier1,
'' AS CPTModifier2,
'' AS CPTModifier3,
'' AS CPTModifier4,
          StmtApplies.BillStatementNote,
          StmtApplies.LineItemStatementNote
        FROM
          (SELECT
              *
            FROM
              (SELECT
                  AppliesT.DestID
                  AS ChargeID,
                  AppliesT.ID
                  AS PaymentID,
                  PatientsT.UserDefinedID
                  AS PatientID,
                  PatientsT.PersonID
                  AS PatID,
                  LineItemT1.Type,
                  AppliesT.Amount
                  AS ApplyAmount,
                  LineItemT1.Date,
                  LineItemT1.InputDate,(CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT1.Type = 1 AND
                  Left(LineItemT1.Description,
                  Len('Corrected Payment' )) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description,
                  Len(LineItemT1.Description) -Len('Corrected Payment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT1.Type = 2 AND
                  Left(LineItemT1.Description,
                  Len('Corrected Adjustment' )) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description,
                  Len(LineItemT1.Description) -Len('Corrected Adjustment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT1.Type = 3 AND
                  Left(LineItemT1.Description,
                  Len('Corrected Refund' )) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description,
                  Len(LineItemT1.Description) -Len('Corrected Refund' ))) ELSE LineItemT1.Description END)
                  AS Description,
                  Insurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN [AppliesT].[Amount] ELSE 0 END,
)";

				part8.Format(R"(                  
                  PatientsT.SuppressStatement,
                  ProvidersT.PersonID
                  AS ProvID,
                  BillsT.ID
                  AS BillID,
                  BillsT.Date
                  AS BillDate,
                  BillsT.Description
                  AS BillDescription,
                  PersonT.BirthDate,
                  LineItemT.LocationID
                  AS LocationFixID,
                  (%s
                    WHERE
                      PersonT.ID = ChargesT.DoctorsProviders
                  )
                  AS TransProv,
                  0
                  AS PrePayment,
                  CreditCardNamesT.CardName
                  AS CCType,
                  PaymentPlansT.CCNumber,
                  PaymentPlansT.CheckNo,
                  PaymentPlansT.BankNo,
                  PaymentPlansT.CheckAcctNo,
                  PaymentPlansT.CCHoldersName,
                  PaymentPlansT.CCExpDate,
                  PaymentPlansT.CCAuthNo,
                  PaymentPlansT.BankRoutingNum,
                  (SELECT
                      Left(Note,
                      1000)
                    FROM
                      Notes
                    WHERE
                      ShowOnStatement = 1 AND
                      Notes.BillID = BillsT.ID
                  )
                  AS BillStatementNote,
                  (SELECT
                      Left(Note,
                      1000)
                    FROM
                      Notes
                    WHERE
                      ShowOnStatement = 1 AND
                      Notes.LineItemID = PaymentsT.ID
                  )
                  AS LineItemStatementNote
                FROM
                  PersonT LEFT
                  OUTER JOIN LocationsT
                  ON PersonT.Location = LocationsT.ID RIGHT
                  OUTER JOIN LineItemT LineItemT1 RIGHT
                  OUTER JOIN PaymentsT
                  ON LineItemT1.ID = PaymentsT.ID 
				  LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID
				  LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID 
				  LEFT JOIN PaymentPlansT
                  ON PaymentsT.ID = PaymentPlansT.ID LEFT
                  JOIN CreditCardNamesT
                  ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID RIGHT
                  OUTER JOIN AppliesT
                  ON PaymentsT.ID = AppliesT.SourceID )"
					, strTransProv);

				part8b.Format(R"(                  LEFT OUTER JOIN LineItemT RIGHT
                  OUTER JOIN ChargesT LEFT
                  OUTER JOIN BillsT LEFT
                  OUTER JOIN PatientsT
                  ON BillsT.PatientID = PatientsT.PersonID
                  ON ChargesT.BillID = BillsT.ID
                  ON LineItemT.ID = ChargesT.ID
                  ON AppliesT.DestID = ChargesT.ID
                  ON PersonT.ID = PatientsT.PersonID LEFT
                  JOIN BillDiagCodeFlat4V
                  ON BillsT.ID = BillDiagCodeFlat4V.BillID LEFT
                  JOIN DiagCodes ICD9T1
                  ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID LEFT
                  JOIN DiagCodes ICD9T2
                  ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID LEFT
                  JOIN DiagCodes ICD9T3
                  ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID LEFT
                  JOIN DiagCodes ICD9T4
                  ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID LEFT
                  JOIN DiagCodes ICD10T1
                  ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID LEFT
                  JOIN DiagCodes ICD10T2
                  ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID LEFT
                  JOIN DiagCodes ICD10T3
                  ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID LEFT
                  JOIN DiagCodes ICD10T4
                  ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID LEFT
                  OUTER JOIN PersonT PersonT1 RIGHT
                  OUTER JOIN ProvidersT
                  ON PersonT1.ID = ProvidersT.PersonID
                  ON PatientsT.MainPhysician = ProvidersT.PersonID LEFT
                  JOIN BillCorrectionsT
                  ON BillsT.ID = BillCorrectionsT.OriginalBillID LEFT
                  JOIN LineItemCorrectionsT
                  ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID LEFT
                  JOIN LineItemCorrectionsT VoidT
                  ON ChargesT.ID = VoidT.VoidingLineItemID LEFT
                  JOIN LineItemCorrectionsT LineItemCorrections2T
                  ON PaymentsT.ID = LineItemCorrections2T.OriginalLineItemID LEFT
                  JOIN LineItemCorrectionsT Void2T
                  ON PaymentsT.ID = Void2T.VoidingLineItemID LEFT
                  JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay
                  ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID
                WHERE
                  (LineItemT1.Deleted = 0) AND
                  (LineItemT.Deleted = 0) AND
                  (BillsT.Deleted = 0) AND
                  (AppliesT.PointsToPayments = 0) AND
                  BillCorrectionsT.ID IS NULL AND
                  LineItemCorrectionsT.ID IS NULL AND
                  VoidT.ID IS NULL AND
                  LineItemCorrections2T.ID IS NULL AND
                  Void2T.ID IS NULL AND
                  (PatientsT.PersonID > 0) %s %s %s
              )
              AS StatementDataAppliesCharges
)"
, strInnerFilter
// (c.haag 2016-02-09) - PLID 68236 - Join on both chargeback tables
, GetStatementChargebackString("ChargebacksPayments")
, GetStatementChargebackString("ChargebacksAdjustments"));


				part9.Format(R"(            UNION
            SELECT
              *
            FROM
              (SELECT
                  AppliesT.DestID
                  AS ChargeID,
                  AppliesT.ID
                  AS PaymentID,
                  PatientsT.UserDefinedID
                  AS PatientID,
                  PatientsT.PersonID
                  AS PatID,
                  LineItemT1.Type,
                  AppliesT.Amount
                  AS ApplyAmount,
                  LineItemT1.Date,
                  LineItemT1.InputDate,(CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT1.Type = 1 AND
                  Left(LineItemT1.Description,
                  Len('Corrected Payment' )) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description,
                  Len(LineItemT1.Description) -Len('Corrected Payment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT1.Type = 2 AND
                  Left(LineItemT1.Description,
                  Len('Corrected Adjustment' )) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description,
                  Len(LineItemT1.Description) -Len('Corrected Adjustment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT1.Type = 3 AND
                  Left(LineItemT1.Description,
                  Len('Corrected Refund' )) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description,
                  Len(LineItemT1.Description) -Len('Corrected Refund' ))) ELSE LineItemT1.Description END)
                  AS Description,
                  CASE WHEN PaymentsT.InsuredPartyID > 0 THEN AppliesT.Amount ELSE 0 END
                  AS Insurance,
                  PatientsT.SuppressStatement,
                  ProvidersT.PersonID
                  AS ProvID,
                  LineItemT.ID
                  AS BillID,
                  LineItemT.Date
                  AS BillDate,(CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT.Type = 1 AND
                  Left(LineItemT.Description,
                  Len('Corrected Payment' )) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description,
                  Len(LineItemT.Description) -Len('Corrected Payment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT.Type = 2 AND
                  Left(LineItemT.Description,
                  Len('Corrected Adjustment' )) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description,
                  Len(LineItemT.Description) -Len('Corrected Adjustment' ))) WHEN LineItemCorrectionsT_NewPay.NewLineItemID IS NOT
                  NULL AND
                  LineItemT.Type = 3 AND
                  Left(LineItemT.Description,
                  Len('Corrected Refund' )) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description,
                  Len(LineItemT.Description) -Len('Corrected Refund' ))) ELSE LineItemT.Description END)
                  AS BillDescription,
                  PersonT.BirthDate,
                  LineItemT.LocationID
                  AS LocationFixID,
                  (%s
                    WHERE
                      PersonT.ID = PaymentsT.ProviderID
                  )
                  AS TransProv,
                  0
                  AS Prepayment,
                  CreditCardNamesT.CardName
                  AS CCType,
                  PaymentPlansT.CCNumber,
                  PaymentPlansT.CheckNo,
                  PaymentPlansT.BankNo,
                  PaymentPlansT.CheckAcctNo,
                  PaymentPlansT.CCHoldersName,
                  PaymentPlansT.CCExpDate,
                  PaymentPlansT.CCAuthNo,
                  PaymentPlansT.BankRoutingNum,
'' AS BillStatementNote,
                  (SELECT
                      Left(Note,
                      1000)
                    FROM
                      Notes
                    WHERE
                      ShowOnStatement = 1 AND
                      Notes.LineItemID = PaymentsT.ID
                  )
                  AS LineItemStatementNote
                FROM
                  PersonT PersonT1 RIGHT
                  OUTER JOIN ProvidersT
                  ON PersonT1.ID = ProvidersT.PersonID RIGHT
                  OUTER JOIN PersonT LEFT
                  OUTER JOIN LocationsT
                  ON PersonT.Location = LocationsT.ID RIGHT
                  OUTER JOIN LineItemT LineItemT1 LEFT
                  OUTER JOIN PatientsT
                  ON 
)"
, strTransProv);

				part10.Format(R"(				  LineItemT1.PatientID = PatientsT.PersonID
                  ON PersonT.ID = PatientsT.PersonID
                  ON ProvidersT.PersonID = PatientsT.MainPhysician RIGHT
                  OUTER JOIN PaymentsT
                  ON LineItemT1.ID = PaymentsT.ID LEFT
                  JOIN PaymentPlansT
                  ON PaymentsT.ID = PaymentPlansT.ID LEFT
                  JOIN CreditCardNamesT
                  ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID 
				  LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID
				  LEFT JOIN ChargebacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID 
                  RIGHT OUTER JOIN LineItemT RIGHT
                  OUTER JOIN AppliesT
                  ON LineItemT.ID = AppliesT.DestID
                  ON PaymentsT.ID = AppliesT.SourceID 
				  LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID
				  LEFT JOIN ChargebacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID 
				  LEFT JOIN LineItemCorrectionsT
                  ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID LEFT
                  JOIN LineItemCorrectionsT VoidT
                  ON LineItemT.ID = VoidT.VoidingLineItemID LEFT
                  JOIN LineItemCorrectionsT LineItemCorrections2T
                  ON LineItemT1.ID = LineItemCorrections2T.OriginalLineItemID LEFT
                  JOIN LineItemCorrectionsT Void2T
                  ON LineItemT1.ID = Void2T.VoidingLineItemID LEFT
                  JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay
                  ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID
                WHERE
                  (LineItemT1.Deleted = 0) AND
                  (LineItemT1.Deleted = 0) AND
                  (AppliesT.PointsToPayments = 1) AND
                  LineItemCorrectionsT.ID IS NULL AND
                  VoidT.ID IS NULL AND
                  LineItemCorrections2T.ID IS NULL AND
                  Void2T.ID IS NULL AND
                  (PatientsT.PersonID > 0) %s %s %s %s %s
              )
              AS StatementDataAppliesPays
          )
          AS StmtApplies
      )
      AS StatementAllData LEFT
      JOIN
      (SELECT
          AppointmentsT.Date
          AS AppDate,
          AppointmentsT.StartTime,
          AppointmentsT.PatientID
          AS PatID
        FROM
          AppointmentsT
        WHERE
          (((AppointmentsT.Date) > GetDate()) AND
          (AppointmentsT.PatientID > 0) AND
          ((AppointmentsT.Status) <> 4))
      )
      AS NextApp
      ON NextApp.PatID = StatementAllData.PatID LEFT
      JOIN LineItemT
      ON LineItemT.ID = StatementAllData.ID LEFT
      JOIN
      (SELECT
          CASE WHEN Thirty IS NULL THEN 0 ELSE Thirty END
          AS Thirty,
          CASE WHEN Sixty IS NULL THEN 0 ELSE SIXTY END
          AS Sixty,
          CASE WHEN Ninety IS NULL THEN 0 ELSE NINETY END
          AS Ninety,
          CASE WHEN NINETYPLUS IS NULL THEN 0 ELSE NinetyPlus END
          AS NinetyPlus,
          PatientID
        FROM
          (SELECT
              PatAR.PatientID,
              SUM(PatAR.Thirty)
              AS Thirty,
              SUM(PatAR.Sixty)
              AS Sixty,
              SUM(PatAR.Ninety)
              AS Ninety,
              SUM(PatAR.NinetyPlus)
              AS NinetyPlus
            FROM
              (SELECT
                  SUM((CASE WHEN Thirty.ChargeAmount IS NULL THEN 0 ELSE Thirty.ChargeAmount END) -(CASE WHEN Thirty.PPAmount IS NULL THEN 0 ELSE Thirty.PPAmount END))
                  AS Thirty,
                  0
                  AS Sixty,
                  0
                  AS Ninety,
                  0
                  AS NinetyPlus,
                  PatientID
                FROM
                  (
                  (SELECT
                      SUM(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END)
                      AS ChargeAmount,
                      0
                      AS PPAmount,
                      PatientID
                    FROM
                      (SELECT
                          SUM(ChargeRespDetailT.Amount)
                          AS ChargeAmt,
                          LineItemT.PatientID,
                          ChargeRespDetailT.ID
                          AS DetailID
                        FROM
                          ChargeRespDetailT LEFT
                          JOIN ChargeRespT
                          ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID LEFT
                          JOIN LineItemT
                          ON ChargeRespT.ChargeID = LineItemT.ID
                        WHERE
                          (LineItemT.Deleted = 0) AND
                          (LineItemT.Type = 10) AND
                          (ChargeRespDetailT.Date >= DATEADD(dd,
                          -30,
                          getDate())) AND
                          (ChargeRespT.InsuredPartyID IS NULL OR
                          ChargeRespT.InsuredPartyID = -1) %s
                        GROUP BY
                          LineItemT.PatientID,
                          ChargeRespDetailT.ID
                      )
                      AS Charges LEFT
                      JOIN
                      (SELECT
                          SUM(ApplyDetailsT.Amount)
                          AS PayAmt,
                          ApplyDetailsT.DetailID
                        FROM
                          ApplyDetailsT
                        GROUP BY
                          ApplyDetailsT.DetailID
                      )
                      AS Pays
                      ON Charges.DetailID = Pays.DetailID
                    GROUP BY
                      PatientID
                  )
)"
, strInnerFilterApp
// (c.haag 2016-02-09) - PLID 68236 - Use the new chargeback tables
, GetStatementChargebackString("ChargebacksPayments1"), GetStatementChargebackString("ChargebacksAdjustments1")
, GetStatementChargebackString("ChargebacksPayments"), GetStatementChargebackString("ChargebacksAdjustments")
, strInnerFilter
);

				part10 += FormatString(R"(                UNION
                  (SELECT
                      0
                      AS ChargeAmount,
                      SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END),
                      LineItemT.PatientID
                    FROM
                      LineItemT LEFT
                      JOIN
                      (SELECT
                          DestID,
                          SUM(Amount)
                          AS Amount
                        FROM
                          AppliesT LEFT
                          JOIN PaymentsT
                          ON AppliesT.SourceID = PaymentsT.ID
                        WHERE
                          (InsuredPartyID = -1)
                        GROUP BY
                          DestID
                      )
                      AS PPayQ
                      ON LineItemT.ID = PPayQ.DestID LEFT
                      JOIN
                      (SELECT
                          SourceID,
                          SUM(Amount)
                          AS Amount
                        FROM
                          AppliesT
                        GROUP BY
                          SourceID
                      )
                      AS ApplyQ
                      ON LineItemT.ID = ApplyQ.SourceID
                      INNER JOIN PaymentsT
                      ON LineItemT.ID = PaymentsT.ID
                    WHERE
                      (LineItemT.Deleted = 0) AND
                      (LineItemT.Type < 10) AND
                      (LineItemT.Date >= DATEADD(dd,
                      -30,
                      getDate())) AND
                      (PaymentsT.InsuredPartyID = -1) %s %s
                    GROUP BY
                      LineItemT.PatientID
                  ))
                  AS Thirty
                GROUP BY
                  Thirty.PatientID
                UNION
                SELECT
                  0
                  AS Thirty,
                  SUM((CASE WHEN Sixty.ChargeAmount IS NULL THEN 0 ELSE Sixty.ChargeAmount END) -(CASE WHEN Sixty.PPAmount IS NULL THEN 0 ELSE Sixty.PPAmount END))
                  AS Sixty,
                  0
                  AS Ninety,
                  0
                  AS NinetyPlus,
                  PatientID
                FROM
                  (
                  (SELECT
                      SUM(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END)
                      AS ChargeAmount,
                      0
                      AS PPAmount,
                      PatientID
                    FROM
                      (SELECT
                          SUM(ChargeRespDetailT.Amount)
                          AS ChargeAmt,
                          LineItemT.PatientID,
                          ChargeRespDetailT.ID
                          AS DetailID
                        FROM
                          ChargeRespDetailT LEFT
                          JOIN ChargeRespT
                          ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID LEFT
                          JOIN LineItemT
                          ON ChargeRespT.ChargeID = LineItemT.ID
                        WHERE
                          (LineItemT.Deleted = 0) AND
                          (LineItemT.Type = 10) AND
                          (ChargeRespDetailT.Date >= DATEADD(dd,
                          -60,
                          getDate())) AND
                          (ChargeRespDetailT.Date <= DATEADD(dd,
                          -30,
                          getDate())) AND
                          (ChargeRespT.InsuredPartyID IS NULL OR
                          ChargeRespT.InsuredPartyID = -1) %s
                        GROUP BY
                          LineItemT.PatientID,
                          ChargeRespDetailT.ID
                      )
                      AS Charges LEFT
                      JOIN
                      (SELECT
                          SUM(ApplyDetailsT.Amount)
                          AS PayAmt,
                          ApplyDetailsT.DetailID
                        FROM
                          ApplyDetailsT
                        GROUP BY
                          ApplyDetailsT.DetailID
                      )
                      AS Pays
                      ON Charges.DetailID = Pays.DetailID
                    GROUP BY
                      PatientID
                  )
                UNION
                  (SELECT
                      0
                      AS ChargeAmount,
                      SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END),
                      LineItemT.PatientID
                    FROM
                      LineItemT LEFT
                      JOIN
                      (SELECT
                          DestID,
                          SUM(Amount)
                          AS Amount
                        FROM
                          AppliesT LEFT
                          JOIN PaymentsT
                          ON AppliesT.SourceID = PaymentsT.ID
                        WHERE
                          (InsuredPartyID = -1)
                        GROUP BY
                          DestID
                      )
                      AS PPayQ
                      ON LineItemT.ID = PPayQ.DestID LEFT
                      JOIN
                      (SELECT
                          SourceID,
                          SUM(Amount)
                          AS Amount
                        FROM
                          AppliesT
                        GROUP BY
                          SourceID
                      )
                      AS ApplyQ
                      ON LineItemT.ID = ApplyQ.SourceID
                      INNER JOIN PaymentsT
                      ON LineItemT.ID = PaymentsT.ID
                    WHERE
                      (LineItemT.Deleted = 0) AND
                      (LineItemT.Type < 10) AND
                      (LineItemT.Date >= DATEADD(dd,
                      -60,
                      getDate())) AND
                      (LineItemT.Date <= DATEADD(dd,
                      -30,
                      getDate())) AND
                      (PaymentsT.InsuredPartyID = -1) %s %s
                    GROUP BY
                      LineItemT.PatientID
                  ))
                  AS Sixty
                GROUP BY
                  Sixty.PatientID
                UNION
                SELECT
                  0
                  AS Thirty,
                  0
                  AS Sixty,
                  SUM((CASE WHEN Ninety.ChargeAmount IS NULL THEN 0 ELSE Ninety.ChargeAmount END) -(CASE WHEN Ninety.PPAmount IS NULL THEN 0 ELSE Ninety.PPAmount END))
                  AS Ninety,
                  0
                  AS NinetyPlus,
                  PatientID
                FROM
                  (
                  (SELECT
                      SUM(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END)
                      AS ChargeAmount,
                      0
                      AS PPAmount,
                      PatientID
                    FROM
                      (SELECT
                          SUM(ChargeRespDetailT.Amount)
                          AS ChargeAmt,
                          LineItemT.PatientID,
                          ChargeRespDetailT.ID
                          AS DetailID
                        FROM
                          ChargeRespDetailT LEFT
                          JOIN ChargeRespT
                          ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID LEFT
                          JOIN LineItemT
                          ON ChargeRespT.ChargeID = LineItemT.ID
                        WHERE
                          (LineItemT.Deleted = 0) AND
                          (LineItemT.Type = 10) AND
                          (ChargeRespDetailT.Date <= DATEADD(dd,
                          -60,
                          getDate())) AND
                          (ChargeRespDetailT.Date >= DATEADD(dd,
                          -90,
                          getDate())) AND
                          (ChargeRespT.InsuredPartyID IS NULL OR
                          ChargeRespT.InsuredPartyID = -1) %s
                        GROUP BY
                          LineItemT.PatientID,
                          ChargeRespDetailT.ID
                      )
                      AS Charges LEFT
                      JOIN
                      (SELECT
                          SUM(ApplyDetailsT.Amount)
                          AS PayAmt,
                          ApplyDetailsT.DetailID
                        FROM
                          ApplyDetailsT
                        GROUP BY
                          ApplyDetailsT.DetailID
                      )
                      AS Pays
                      ON Charges.DetailID = Pays.DetailID
                    GROUP BY
                      PatientID
                  )
                UNION
                  (SELECT
                      0
                      AS ChargeAmount,
                      SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END),
                      LineItemT.PatientID
                    FROM
                      LineItemT LEFT
                      JOIN
                      (SELECT
                          DestID,
                          SUM(Amount)
                          AS Amount
                        FROM
                          AppliesT LEFT
                          JOIN PaymentsT
                          ON AppliesT.SourceID = PaymentsT.ID
                        WHERE
                          (InsuredPartyID = -1)
                        GROUP BY
                          DestID
                      )
                      AS PPayQ
                      ON LineItemT.ID = PPayQ.DestID LEFT
                      JOIN
                      (SELECT
                          SourceID,
                          SUM(Amount)
                          AS Amount
                        FROM
                          AppliesT
                        GROUP BY
                          SourceID
                      )
                      AS ApplyQ
                      ON LineItemT.ID = ApplyQ.SourceID
                      INNER JOIN PaymentsT
                      ON LineItemT.ID = PaymentsT.ID
                    WHERE
                      (LineItemT.Deleted = 0) AND
                      (LineItemT.Type < 10) AND
                      (LineItemT.Date <= DATEADD(dd,
                      -60,
                      getDate())) AND
                      (LineItemT.Date >= DATEADD(dd,
                      -90,
                      getDate())) AND
                      (PaymentsT.InsuredPartyID = -1) %s %s
                    GROUP BY
                      LineItemT.PatientID
                  ))
                  AS Ninety
                GROUP BY
                  Ninety.PatientID
                UNION
                SELECT
                  0
                  AS Thirty,
                  0
                  AS Sixty,
                  0
                  AS Ninety,
                  SUM((CASE WHEN NinetyPlus.ChargeAmount IS NULL THEN 0 ELSE NinetyPlus.ChargeAmount END) -(CASE WHEN NinetyPlus.PPAmount IS NULL THEN 0 ELSE NinetyPlus.PPAmount END))
                  AS NinetyPlus,
                  PatientID
                FROM
                  (
                  (SELECT
                      SUM(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END)
                      AS ChargeAmount,
                      0
                      AS PPAmount,
                      PatientID
                    FROM
                      (SELECT
                          SUM(ChargeRespDetailT.Amount)
                          AS ChargeAmt,
                          LineItemT.PatientID,
                          ChargeRespDetailT.ID
                          AS DetailID
                        FROM
                          ChargeRespDetailT LEFT
                          JOIN ChargeRespT
                          ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID LEFT
                          JOIN LineItemT
                          ON ChargeRespT.ChargeID = LineItemT.ID
                        WHERE
                          (LineItemT.Deleted = 0) AND
                          (LineItemT.Type = 10) AND
                          (ChargeRespDetailT.Date <= DATEADD(dd,
                          -90,
                          getDate())) AND
                          (ChargeRespT.InsuredPartyID IS NULL OR
                          ChargeRespT.InsuredPartyID = -1) %s
                        GROUP BY
                          LineItemT.PatientID,
                          ChargeRespDetailT.ID
                      )
                      AS Charges LEFT
                      JOIN
                      (SELECT
                          SUM(ApplyDetailsT.Amount)
                          AS PayAmt,
                          ApplyDetailsT.DetailID
                        FROM
                          ApplyDetailsT
                        GROUP BY
                          ApplyDetailsT.DetailID
                      )
                      AS Pays
                      ON Charges.DetailID = Pays.DetailID
                    GROUP BY
                      PatientID
                  )
                UNION
                  (SELECT
                      0
                      AS ChargeAmount,
                      SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END),
                      LineItemT.PatientID
                    FROM
                      LineItemT LEFT
                      JOIN
                      (SELECT
                          DestID,
                          SUM(Amount)
                          AS Amount
                        FROM
                          AppliesT LEFT
                          JOIN PaymentsT
                          ON AppliesT.SourceID = PaymentsT.ID
                        WHERE
                          (InsuredPartyID = -1)
                        GROUP BY
                          DestID
                      )
                      AS PPayQ
                      ON LineItemT.ID = PPayQ.DestID LEFT
                      JOIN
                      (SELECT
                          SourceID,
                          SUM(Amount)
                          AS Amount
                        FROM
                          AppliesT
                        GROUP BY
                          SourceID
                      )
                      AS ApplyQ
                      ON LineItemT.ID = ApplyQ.SourceID
                      INNER JOIN PaymentsT
                      ON LineItemT.ID = PaymentsT.ID
                    WHERE
                      (LineItemT.Deleted = 0) AND
                      (LineItemT.Type < 10) AND
                      (LineItemT.Date <= DATEADD(dd,
                      -90,
                      getDate())) AND
                      (PaymentsT.InsuredPartyID = -1) %s %s
                    GROUP BY
                      LineItemT.PatientID
                  ))
                  AS NinetyPlus
                GROUP BY
                  NinetyPlus.PatientID
              ) PatAR
            GROUP BY
              PatAR.PatientID
          )
          AS PatientAR
      )
      AS StatementAR
      ON StatementAllData.PatID = StatementAR.PatientID
)"
, strHideUnAppliedPrepayments, strInnerFilter
, strInnerFilter
, strHideUnAppliedPrepayments, strInnerFilter
, strInnerFilter
, strHideUnAppliedPrepayments, strInnerFilter
, strInnerFilter
, strHideUnAppliedPrepayments, strInnerFilter
);

				part10b.Format("   LEFT JOIN "
					"   (SELECT StatementPatBalanceSub.PatID,  "
					"   SUM(StatementPatBalanceSub.PatCharge - StatementPatBalanceSub.PatPay) "
					" AS PatTotResp,  "
					"   SUM(StatementPatBalanceSub.InsCharge - StatementPatBalanceSub.InsPays) "
					" AS InsTotResp "
					" FROM (SELECT StatementAllData.PatID,  "
					" PatCharge = CASE WHEN StatementAllData.Type "
					"  = 10 THEN (StatementAllData.Total - StatementAllData.Insurance) "
					"  ELSE 0 END,  "
					" PatPay = CASE WHEN StatementAllData.Type "
					"  = 1 OR "
					" StatementAllData.Type = 2 THEN CASE WHEN "
					"  StatementAllData.Insurance <> 0 THEN 0 ELSE "
					"  StatementAllData.Total END ELSE CASE WHEN "
					"  StatementAllData.Type = 3 THEN CASE WHEN "
					"  StatementAllDAta.Insurance <> 0 THEN 0 ELSE "
					"  StatementAllData.Total * - 1 END ELSE 0 END "
					"  END,  "
					" InsCharge = CASE WHEN StatementAllData.Type "
					"  = 10 THEN StatementAllData.Insurance ELSE "
					"  0 END,  "
					" InsPays = CASE WHEN StatementAllData.Type "
					"  = 1 OR "
					" StatementAllData.Type = 2 THEN StatementAllData.Insurance "
					"  ELSE CASE WHEN StatementAllData.Type "
					"  = 3 THEN StatementAllData.Insurance * - 1 "
					"  ELSE 0 END END "
					" FROM (SELECT StmtCharges.ID,  "
					"   StmtCharges.PatID,  "
					"   StmtCharges.Type,  "
					"   StmtCharges.Total,  "
					"   StmtCharges.Insurance, "
					"   1 as StatementType, -1 as GroupFixID "
					"  FROM (SELECT LineItemT.ID,  "
					"  LineItemT.PatientID AS PatID, "
					"   LineItemT.Type,  "
					"  Total = CASE WHEN SUM(ChargeRespT.Amount) "
					"   IS NULL  "
					"  THEN 0 ELSE SUM(ChargeRespT.Amount) "
					"   END,  "
					"   Insurance = SUM(CASE WHEN "
					"   ChargeRespT.InsuredPartyID "
					"   IS NOT NULL and ChargeRespT.InsuredPartyID <> -1 "
					"  THEN ChargeRespT.Amount "
					"   ELSE 0 END) "
					" FROM LineItemT LEFT OUTER JOIN "
					"  ChargeRespT on LineItemT.ID = ChargeRespT.ChargeID "
					" WHERE (LineItemT.Deleted = 0)  "
					"	  AND (LineItemT.Type = 10) "
					"  GROUP BY LineItemT.ID,  "
					"  LineItemT.PatientID,  "
					"  LineItemT.Type)  "
					"   AS StmtCharges "
					"  UNION "
					"  SELECT StmtPays.ID, StmtPays.PatID,  "
					"   StmtPays.Type,  "
					"   StmtPays.UnAppliedAmount,  "
					"   StmtPays.Insurance,   "
					"   2 as StatementType, -2 as GroupFixID "
					"  FROM (SELECT LineItemT.ID,  "
					"  Insurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) "
					"   > 0 THEN CASE WHEN [LineItemT].[Type] "
					"   = 3 THEN - 1 * MIN([LineItemT].[Amount]) "
					"   + SUM(CASE WHEN [AppliesT].[Amount] "
					"   IS NULL  "
					"  THEN 0 ELSE [AppliesT].[Amount] "
					"   END)  "
					"  ELSE MIN([LineItemT].[Amount]) "
					"   - SUM(CASE WHEN [AppliesT].[Amount] "
					"   IS NULL  "
					"  THEN 0 ELSE [AppliesT].[Amount] "
					"   END) END ELSE 0 END,  "
					"  LineItemT.PatientID AS PatID, "
					"   LineItemT.Type,  "
					"  UnAppliedAmount = CASE WHEN "
					"   [LineItemT].[Type] = 3 THEN "
					"   - 1 * MIN([LineItemT].[Amount]) "
					"   + SUM(CASE WHEN [AppliesT].[Amount] "
					"   IS NULL  "
					"  THEN 0 ELSE [AppliesT].[Amount] "
					"   END)  "
					"  ELSE MIN([LineItemT].[Amount]) "
					"   - SUM(CASE WHEN [AppliesT].[Amount] "
					"   IS NULL  "
					"  THEN 0 ELSE [AppliesT].[Amount] "
					"   END) END "
					"  FROM AppliesT RIGHT OUTER JOIN "
					"  PaymentsT ON  "
					"  AppliesT.SourceID = PaymentsT.ID "
					"  LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"  LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"   RIGHT OUTER JOIN "
					"  LineItemT ON  "
					"  PaymentsT.ID = LineItemT.ID "
					"  WHERE (LineItemT.Type < 10)  "
					"  AND  "
					"  (LineItemT.Deleted = 0) %s "
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
					"  GROUP BY LineItemT.ID,  "
					"  LineItemT.PatientID,  "
					"  LineItemT.Type "
					" HAVING ((Min(LineItemT.Amount) = 0 AND Min(AppliesT.Amount) IS NULL) OR (CASE WHEN "
					" [LineItemT].[Type] = 3 THEN "
					"  - 1 * MIN([LineItemT].[Amount])  "
					"  + SUM(CASE WHEN [AppliesT].[Amount] IS "
					"  NULL  THEN 0 ELSE [AppliesT].[Amount] END)  "
					" ELSE MIN([LineItemT].[Amount])  "
					" - SUM(CASE WHEN [AppliesT].[Amount] IS "
					" NULL  THEN 0 ELSE [AppliesT].[Amount] END ) "
					" END <> 0)) )  "
					"   AS StmtPays "
					"  UNION "
					"  SELECT StmtApplies.ChargeID,  "
					"   StmtApplies.PatID,  "
					"   StmtApplies.Type,  "
					"   StmtApplies.ApplyAmount,  "
					"   StmtApplies.Insurance, "
					"   3 as StatementType, StmtApplies.PaymentID as GroupFixID "
					"  FROM (SELECT * "
					"   FROM (SELECT AppliesT.DestID "
					"  AS ChargeID,  "
					"  AppliesT.ID as PaymentID, "
					"  LineItemT1.PatientID AS PatID,  "
					"  LineItemT1.Type,  "
					"  ApplyAmount = CASE  "
					"		WHEN LineItemT1.Type = 3 then -1 * AppliesT.Amount  "
					"		ELSE AppliesT.Amount "
					"		End, "
					"   Insurance = CASE "
					"   WHEN [PaymentsT].[InsuredPartyID] "
					"   > 0 THEN  "
					"		CASE WHEN LineItemT1.Type = 3 then -1 * [AppliesT].[Amount] "
					"		 ELSE AppliesT.Amount "
					"		 END "
					" ELSE 0 END "
					" FROM LineItemT LineItemT1 "
					" RIGHT OUTER JOIN "
					" PaymentsT ON  "
					" LineItemT1.ID = PaymentsT.ID "
					" RIGHT OUTER JOIN "
					" AppliesT ON  "
					" PaymentsT.ID = AppliesT.SourceID "
					" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					" LEFT OUTER JOIN "
					" LineItemT ON  "
					" LineItemT.ID = AppliesT.DestID "
					" WHERE (LineItemT1.Deleted "
					" = 0) AND  "
					" (LineItemT.Deleted = "
					" 0) AND "
					" (AppliesT.PointsToPayments "
					" = 0) "
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
					" )  "
					" AS StatementDataAppliesCharges "
					"  UNION "
					"  SELECT * "
					"  FROM (SELECT AppliesT.DestID "
					"  AS ChargeID,  "
					"  AppliesT.ID as PaymentID, "
					"  LineItemT1.PatientID AS "
					"  PatID,  "
					"  LineItemT1.Type, ApplyAmount = CASE "
					"  WHEN LineItemT1.Type = 3 then -1 * AppliesT.Amount  "
					"		ELSE AppliesT.Amount "
					"		END, "
					"	Insurance  =  "
					" CASE WHEN PaymentsT.InsuredPartyID "
					"   > 0 THEN  "
					"CASE WHen LineItemT1.Type = 3 then -1 * AppliesT.Amount "
					"						   ELSE AppliesT.Amount "
					"END "
					" ELSE 0 END "
					"   FROM LineItemT LineItemT1 "
					"  RIGHT OUTER JOIN "
					"  PaymentsT ON  "
					"  LineItemT1.ID = PaymentsT.ID "
					"  LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID "
					"  LEFT JOIN ChargebacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID "
					"  RIGHT OUTER JOIN "
					"  LineItemT RIGHT OUTER "
					"  JOIN "
					" AppliesT ON  "
					" LineItemT.ID = AppliesT.DestID "
					"  ON  "
					" PaymentsT.ID = AppliesT.SourceID "
					" LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID "
					" LEFT JOIN ChargebacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID "
					" WHERE (LineItemT1.Deleted "
					"  = 0) AND  "
					" (LineItemT1.Deleted "
					"  = 0) AND "
					" (AppliesT.PointsToPayments "
					"  = 1) "
					+ GetStatementChargebackString("ChargebacksPayments1") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments1") + " "
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
					" )  "
					"  AS StatementDataAppliesPays) "
					"   AS StmtApplies)  "
					" AS StatementAllData)  "
					"   AS StatementPatBalanceSub "
					" GROUP BY StatementPatBalanceSub.PatID)  "
					"  AS StatementPatBalance ON  "
					"  StatementAllData.PatID = StatementPatBalance.PatID ", strHideUnAppliedPrepayments);

				part11 = R"(    GROUP BY
      StatementAllData.ID,
      StatementAllData.PatientID,
      StatementAllData.PatID,
      StatementAllData.Type,
      StatementAllData.Total,
      StatementAllData.Description,
      StatementAllData.Date,
      StatementAllData.Insurance,
      StatementAllData.ProvID,
      StatementAllData.BillID,
      StatementAllData.BillDate,
      StatementAllData.BillDescription,
      StatementAllData.BirthDate,
      LineItemT.Date,
      StatementAR.Thirty,
      StatementAR.Sixty,
      StatementAR.Ninety,
      StatementAR.NinetyPlus,
)";


				part11b = _T(" StatementPatBalance.PatTotResp, StatementPatBalance.InsTotResp, ");

				part11c = R"(      StatementAllData.GroupFixID,
      StatementAllData.StatementType,
      StatementAllData.LocationFixID,
      StatementAllData.TransProv,
      StatementAllData.PrePayment,
      StatementAllData.FullChargeNoTax,
      StatementAllData.ChargeTax1,
      StatementAllData.ChargeTax2,
      StatementAllData.TaxRate1,
      StatementAllData.TaxRate2,
      StatementAllData.Quantity,
      StatementAllData.CCType,
      StatementAllData.CCNumber,
      StatementAllData.CheckNo,
      StatementAllData.BankNo,
      StatementAllData.CheckAcctNo,
      StatementAllData.CCHoldersName,
      StatementAllData.CCExpDate,
      StatementAllData.CCAuthNo,
      StatementAllData.BankRoutingNum,
      StatementAllData.PercentOff,
      StatementAllData.DiscountAmt,
      StatementAllData.DiscountCategoryDesc,
      StatementAllData.CPTCode,
      StatementAllData.CPTModifier1,
      StatementAllData.CPTModifier2,
      StatementAllData.CPTModifier3,
      StatementAllData.CPTModifier4,
      StatementAllData.BillStatementNote,
      StatementAllData.LineItemStatementNote
  )
  AS StatementSubQ 

										  LEFT JOIN PersonT PatPerson ON PatPerson.ID = StatementSubQ.PatID
  LEFT JOIN ProvidersT ON ProvidersT.PersonID = StatementSubQ.ProvID
  LEFT JOIN PersonT ProvPerson ON ProvPerson.ID = ProvidersT.PersonID
  LEFT JOIN LocationsT ON LocationsT.ID = PatPerson.Location
  LEFT JOIN ChargesT ON ChargesT.ID = StatementSubQ.ID
  LEFT JOIN BillDiagCodeFlat4V ON ChargesT.BillID = BillDiagCodeFlat4V.BillID 
  LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID 
  LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID 
  LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID 
  LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID 
  LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID
  LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID
  LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID 
  LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID
  LEFT JOIN
    (SELECT
        ChargesT.ID
        AS ChargeID,
                          STUFF(
	(CASE WHEN ChargeICDCodes1T.ICD9DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes1T.ICD9DiagCodeNumber ELSE '' END)
	+ (CASE WHEN ChargeICDCodes2T.ICD9DiagCodeNumber IS NOT NULL THEN ', ' + ChargeICDCodes2T.ICD9DiagCodeNumber ELSE '' END)
	+ (CASE WHEN ChargeICDCodes3T.ICD9DiagCodeNumber IS NOT NULL THEN ', ' + ChargeICDCodes3T.ICD9DiagCodeNumber ELSE '' END)
	+ (CASE WHEN ChargeICDCodes4T.ICD9DiagCodeNumber IS NOT NULL THEN ', ' + ChargeICDCodes4T.ICD9DiagCodeNumber ELSE '' END), 1, 2, '') AS WhichCodes9,

										                  STUFF(
	(CASE WHEN ChargeICDCodes1T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes1T.ICD10DiagCodeNumber ELSE '' END)
	+ (CASE WHEN ChargeICDCodes2T.ICD10DiagCodeNumber IS NOT NULL THEN ', ' + ChargeICDCodes2T.ICD10DiagCodeNumber ELSE '' END)
	+ (CASE WHEN ChargeICDCodes3T.ICD10DiagCodeNumber IS NOT NULL THEN ', ' + ChargeICDCodes3T.ICD10DiagCodeNumber ELSE '' END)
	+ (CASE WHEN ChargeICDCodes4T.ICD10DiagCodeNumber IS NOT NULL THEN ', ' + ChargeICDCodes4T.ICD10DiagCodeNumber ELSE '' END), 1, 2, '') AS WhichCodes10,

										                  STUFF(
	(CASE 
		WHEN ChargeICDCodes1T.ICD9DiagCodeNumber IS NOT NULL AND ChargeICDCodes1T.ICD10DiagCodeNumber IS NULL THEN  ', ' + ChargeICDCodes1T.ICD9DiagCodeNumber 
		WHEN ChargeICDCodes1T.ICD9DiagCodeNumber IS NULL AND ChargeICDCodes1T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes1T.ICD10DiagCodeNumber 
		WHEN ChargeICDCodes1T.ICD9DiagCodeNumber IS NOT NULL AND ChargeICDCodes1T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes1T.ICD10DiagCodeNumber + ' (' + ChargeICDCodes1T.ICD9DiagCodeNumber  + ')'
		ELSE ''
	 END)
+	(CASE 
		WHEN ChargeICDCodes2T.ICD9DiagCodeNumber IS NOT NULL AND ChargeICDCodes2T.ICD10DiagCodeNumber IS NULL THEN  ', ' + ChargeICDCodes2T.ICD9DiagCodeNumber 
		WHEN ChargeICDCodes2T.ICD9DiagCodeNumber IS NULL AND ChargeICDCodes2T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes2T.ICD10DiagCodeNumber 
		WHEN ChargeICDCodes2T.ICD9DiagCodeNumber IS NOT NULL AND ChargeICDCodes2T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes2T.ICD10DiagCodeNumber + ' (' + ChargeICDCodes2T.ICD9DiagCodeNumber  + ')'
		ELSE ''
	 END)	 
+	(CASE 
		WHEN ChargeICDCodes3T.ICD9DiagCodeNumber IS NOT NULL AND ChargeICDCodes3T.ICD10DiagCodeNumber IS NULL THEN  ', ' + ChargeICDCodes3T.ICD9DiagCodeNumber 
		WHEN ChargeICDCodes3T.ICD9DiagCodeNumber IS NULL AND ChargeICDCodes3T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes3T.ICD10DiagCodeNumber 
		WHEN ChargeICDCodes3T.ICD9DiagCodeNumber IS NOT NULL AND ChargeICDCodes3T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes3T.ICD10DiagCodeNumber + ' (' + ChargeICDCodes3T.ICD9DiagCodeNumber  + ')'
		ELSE ''
	 END)
+	(CASE 
		WHEN ChargeICDCodes4T.ICD9DiagCodeNumber IS NOT NULL AND ChargeICDCodes4T.ICD10DiagCodeNumber IS NULL THEN  ', ' + ChargeICDCodes4T.ICD9DiagCodeNumber 
		WHEN ChargeICDCodes4T.ICD9DiagCodeNumber IS NULL AND ChargeICDCodes4T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes4T.ICD10DiagCodeNumber 
		WHEN ChargeICDCodes4T.ICD9DiagCodeNumber IS NOT NULL AND ChargeICDCodes4T.ICD10DiagCodeNumber IS NOT NULL THEN  ', ' + ChargeICDCodes4T.ICD10DiagCodeNumber + ' (' + ChargeICDCodes4T.ICD9DiagCodeNumber  + ')'
		ELSE ''
	 END)
				, 1, 2,'' ) AS WhichCodesBoth
    FROM
        ChargesT
		LEFT JOIN ChargeDiagCodes_CTE ChargeICDCodes1T ON ChargesT.ID = ChargeICDCodes1T.ChargeID AND ChargeICDCodes1T.RowNumber = 1
  		LEFT JOIN ChargeDiagCodes_CTE ChargeICDCodes2T ON ChargesT.ID = ChargeICDCodes2T.ChargeID AND ChargeICDCodes2T.RowNumber = 2
	 	LEFT JOIN ChargeDiagCodes_CTE ChargeICDCodes3T ON ChargesT.ID = ChargeICDCodes3T.ChargeID AND ChargeICDCodes3T.RowNumber = 3
		LEFT JOIN ChargeDiagCodes_CTE ChargeICDCodes4T ON ChargesT.ID = ChargeICDCodes4T.ChargeID AND ChargeICDCodes4T.RowNumber = 4

										    ) WhichCodesQ 
	ON ChargesT.ID = WhichCodesQ.ChargeID

					  LEFT OUTER JOIN
  (SELECT
      *
    FROM
      (SELECT
          PersonT1.First + ' ' + PersonT1.Middle + ' ' + PersonT1.Last + ' ' + PersonT1.Title
)";

				part12 = R"(          AS DocName,
          PersonT1.ID
          AS ProvID,
          PatientsT.SuppressStatement,
          PatientsT.PersonID
          AS PatID,
          PatientsT.PrimaryRespPartyID
          AS PrimaryRespID,
          PatientsT.StatementNote,
          PatCoordT.First
          AS PatCoordFirst,
          PatCoordT.Middle
          AS PatCoordMiddle,
          PatCoordT.Last
          AS PatCoordLast,
          ProvidersT.NPI
          AS ProviderNPI
        FROM
          PatientsT LEFT
          OUTER JOIN PersonT PersonT1
          ON PatientsT.MainPhysician = PersonT1.ID LEFT
          JOIN ProvidersT
          ON PersonT1.ID = ProvidersT.PersonID LEFT
          JOIN PersonT PatCoordT
          ON PatientsT.EmployeeID = PatCoordT.ID LEFT
          OUTER JOIN PersonT
          ON PatientsT.PersonID = PersonT.ID
      )
      AS PatInfo LEFT
      OUTER JOIN
      (SELECT
          InsuranceCoT.Name
          AS PriInsCo,
          PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last
          AS PriGuarForward,
          PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle
          AS PriGuarComma,
          PersonT.First
          AS PriInsFirst,
          PersonT.Middle
          AS PriInsMiddle,
          PersonT.Last
          AS PriInsLast,
          PatientsT.PersonID
          AS PersonID
        FROM
          InsuranceCoT RIGHT
          OUTER JOIN PersonT RIGHT
          OUTER JOIN InsuredPartyT
          ON PersonT.ID = InsuredPartyT.PersonID
          ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID RIGHT
          OUTER JOIN PatientsT
          ON InsuredPartyT.PatientID = PatientsT.PersonID
        WHERE
          InsuredPartyT.RespTypeID = 1
      )
      AS PriInsInfo
      ON PatInfo.PatID = PriInsInfo.PersonID LEFT
      OUTER JOIN
      (SELECT
          InsuranceCoT.Name
          AS SecInsCo,
          PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last
          AS SecGuarForward,
          PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle
          AS SecGuarComma,
          PersonT.First
          AS SecInsFirst,
          PersonT.Middle
          AS SecInsMiddle,
          PersonT.Last
          AS SecInsLast,
          PatientsT.PersonID
          AS PersID
        FROM
          InsuranceCoT RIGHT
          OUTER JOIN PersonT RIGHT
          OUTER JOIN InsuredPartyT
          ON PersonT.ID = InsuredPartyT.PersonID
          ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID RIGHT
          OUTER JOIN PatientsT
          ON InsuredPartyT.PatientID = PatientsT.PersonID
        WHERE
          InsuredPartyT.RespTypeID = 2
      )
      AS SecInsInfo
      ON PatInfo.PatID = SecInsInfo.PersID LEFT
      OUTER JOIN
      (SELECT
          ResponsiblePartyT.PersonID
          AS RespID,
          ResponsiblePartyT.PatientID
          AS RespPatID,
          First
          AS RespFirst,
          Middle
          AS RespMiddle,
          Last
          AS RespLast,
          Address1
          AS RespAdd1,
          Address2
          AS RespAdd2,
          City
          AS RespCity,
          State
          AS RespState,
          Zip
          AS RespZip
        FROM
          PersonT
          INNER JOIN ResponsiblePartyT
          ON PersonT.ID = ResponsiblePartyT.PersonID LEFT
          JOIN PatientsT
          ON ResponsiblePartyT.PatientID = PatientsT.PersonID
        WHERE
          ResponsiblePartyT.PersonID = PatientsT.PrimaryRespPartyID
      )
      AS ResPartyT
      ON PatInfo.PatID = ResPartyT.RespPatID
  )
  AS StatementEndQ
  ON StatementSubQ.PatID = StatementEndQ.PatID
 WHERE (StatementEndQ.SuppressStatement = 0))";

				// (a.walling 2006-10-24 13:12) - PLID 16059 - Multiple Responsible parties
				part12.Replace("@RespFilter", strRespFilter);

				CString strSQL;

				if (m_bIsEStatement) {
					//add part1b, part1d, part10b if its an e-statement
					strSQL = _T(part1 + part1b + part1c + part1d + part1e + part2 + part3 + part4 + part5 + part6 + part7 + part8 + part8b + part9 + part10 + part10b + part11 + part11b + part11c + part12);
				}
				else {
					strSQL = _T(part1 + part1c + part1e + part2 + part3 + part4 + part5 + part6 + part7 + part8 + part8b + part9 + part10 + part11 + part11c + part12);
				}

				//now we have to add the other filters that we haven't already filtered on
				CString strMoreFilters;
				AddPartToClause(strMoreFilters, m_pReport->GetLocationFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strMoreFilters, m_pReport->GetDateFilter(m_nSubLevel, m_nSubRepNum));
				AddPartToClause(strMoreFilters, m_pReport->GetExternalFilter(m_nSubLevel, m_nSubRepNum));

				strMoreFilters.TrimLeft();
				strMoreFilters.TrimRight();

				if (!strMoreFilters.IsEmpty()) {
					m_pReport->ConvertCrystalToSql(strSQL, strMoreFilters);
					AddFilter(strSQL, strMoreFilters, TRUE);
				}
				return CComplexReportQuery(strCTE, strSQL);
				}
				break;


	//Statement By Location
			
		case 337 :
		case 355:
		case 435: //by last send date
		case 437: //by last send date 7.0
			return CComplexReportQuery(GetStatementByLocationSql(strRespFilter));

			
		break;

				case 483:
				case 485: {

				//Patient Statements by Provider
				long nTransFormat, nHeaderFormat;
				nTransFormat = GetRemotePropertyInt("SttmntTransProvFormat", 0, 0, "<None>");
				nHeaderFormat = GetRemotePropertyInt("SttmntHeaderProvFormat", 0, 0, "<None>");
				CString strTransProv = GetStatementProvString(nTransFormat);
				CString strHeaderProv = GetStatementProvString(nHeaderFormat);
				CString strHideUnAppliedPrepayments = GetStatementUnAppliedPrePaymentsString();
				CString strChargeDescription = GetStatementChargeDescription();


				//PLID 19451: create the temp table to filter with
				CString strInnerFilter = "", strInnerFilterApp = "";
				CString strTmpTable;
				if (!m_pReport->GetFilter(m_nSubLevel, m_nSubRepNum).IsEmpty())  {
					strTmpTable = CreateStatementFilterTable();
					strInnerFilter.Format(" AND LineItemT.PatientID IN (SELECT ID FROM %s) ", strTmpTable);
					strInnerFilterApp.Format(" AND LineItemT1.PatientID IN (SELECT ID FROM %s) ", strTmpTable);

					//if we got here and have no tmp table, that means we got an error in the last function
					if (strTmpTable.IsEmpty()) {
						return CComplexReportQuery();
					}
				}
				else {
					//we have no filters, so running the extra part isn't going to help us, so just skip it
				}

				// (j.gruber 2008-07-02 16:47) - PLID 29553 - added last payment info by provider
				CString strTempTableName;
				strTempTableName = GetLastProviderPaymentInformationSql(strTmpTable, m_pReport->nPatient);
				
				// (j.gruber 2007-05-01 17:12) - PLID 25745 - only show the last 4 digits of the ccnumber
				// (j.gruber 2007-05-15 09:08) - PLID 25987 - take out credit card expiration dates
				// (j.gruber 2007-05-30 12:31) - PLID 25978 - add discount fields and discount categories
				// (e.lally 2007-07-13) PLID 26670 - Updated all references to PaymentPlansT. CCType with link to CardName, aliased as CCType where applicable.
				// (j.gruber 2008-07-03 16:15) - PLID 29533 - made the last payment information show per provider taking into account applied information
				// (j.jones 2008-09-05 10:22) - PLID 30288 - supported MailSentNotesT
				// (j.gruber 2009-11-05 17:28) - PLID 36217 add Provider NPI to the statement
				// (j.gruber 2009-11-25 12:22) - PLID 36430 - added cpt code to the statement
				// (j.gruber 2009-12-24 12:45) - PLID 17122 - added cptmodifiers
				// (j.gruber 2010-02-19 09:46) - PLID 37447 - re-aliased the providerNPI to provNPI
				// (j.gruber 2010-06-14 15:42) - PLID 36484 - added billing notes
				// (j.gruber 2011-07-05 10:10) - PLID 44831 - took out voids and originals
				// (a.wilson 2012-2-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
				// (j.gruber 2014-03-04 12:55) - PLID 61156 - ICD10 update and whichCodes and Diag refactor
				//TES 7/17/2014 - PLID 62935 - Added code to hide chargebacks when requested
				// (j.jones 2015-03-09 09:48) - PLID 64937 - if the description begins with 'Corrected Charge',
				// 'Corrected Payment', etc., strip that off
				CString part1, part2, part3, part4, part5, part6, part7, part8, part8b, part9, part10, part11, part12,  part13, part14,part15, part16, part17;
				part1 = _T(" SELECT StatementSubQ.ID, StatementSubQ.PatientID, StatementSubQ.PatID as PatID, StatementSubQ.Type, StatementSubQ.Total, StatementSubQ.Description, StatementSubQ.Date as TDate,  "
				" StatementSubQ.Insurance, StatementSubQ.Last, StatementSubQ.First, StatementSubQ.Middle, StatementSubQ.Address1, StatementSubQ.Address2, "
				" StatementSubQ.City, StatementSubQ.State, StatementSubQ.Zip, StatementSubQ.PatForward, StatementSubQ.PatComma, StatementSubQ.DocName, StatementSubQ.DocAddress1, StatementSubQ.DocAddress2, StatementSubQ.DocCity, StatementSubQ.DocState, StatementSubQ.DocZip, StatementSubQ.ProvTaxID,  StatementSubQ.PracName, StatementSubQ.PracAddress1, "
				" StatementSubQ.PracAddress2, StatementSubQ.PracCity, StatementSubQ.PracState, StatementSubQ.PracZip, StatementSubQ.PracPhone, StatementSubQ.PracFax, StatementSubQ.ProvID as ProvID2, StatementSubQ.BillId, "
				" StatementSubQ.BillDate as BillDate, StatementSubQ.BillDescription, StatementSubQ.Birthdate, \r\n "
				" StatementSubQ.ICD9Code1, StatementSubQ.ICD9Code2, StatementSubQ.ICD9Code3, StatementSubQ.ICD9Code4, \r\n"
				" StatementSubQ.ICD10Code1, StatementSubQ.ICD10Code2, StatementSubQ.ICD10Code3, StatementSubQ.ICD10Code4, \r\n"
				" StatementSubQ.WhichCodes9, StatementSubQ.WhichCodes10, StatementSubQ.WhichCodesBoth, \r\n"
				" StatementSubQ.Location, StatementSubQ.StatementType, StatementSubQ.GroupFixID, StatementSubQ.Appdate, StatementSubQ.StartTime, StatementSubQ.ARDate, StatementSubQ.Age,  "
				" StatementSubQ.TransProv, StatementSubQ.Prepayment, StatementSubQ.Quantity, StatementSubQ.Thirty, StatementSubQ.Sixty, StatementSubQ.Ninety, StatementSubQ.NinetyPlus,  "
				" StatementSubQ.ProvEIN, StatementSubQ.ProvLicense, StatementSubQ.ProvUPIN, StatementSubQ.ProvDEA, StatementSubQ.ProvBCBS, StatementSubQ.ProvMedicare, StatementSubQ.ProvMedicaid, StatementSubQ.ProvWorkersComp, StatementSubQ.ProvOtherID, StatementSubQ.ProvOtherIDDesc, "
				" StatementEndQ.DocName as DocName2, StatementSubQ.TransProvID as ProvID, StatementEndQ.SuppressStatement, StatementEndQ.PatID as PatID2, StatementEndQ.StatementNote, StatementEndQ.PriInsCo, StatementEndQ.PriGuarForward, StatementEndQ.PriGuarComma, "
				" StatementEndQ.PriInsFirst, StatementEndQ.PriInsMiddle, StatementEndQ.PriInsLast, StatementEndQ.PersonID, StatementEndQ.SecInsCo, StatementEndQ.SecGuarForward, StatementEndQ.SecGuarComma, StatementEndQ.SecInsfirst, StatementEndQ.SecInsMiddle, "
				" StatementEndQ.SecInsLast, StatementEndQ.PersID, "
				" StatementSubQ.LocationFixID AS LocID,  "
				" StatementEndQ.RespID, StatementEndQ.RespFirst, StatementEndQ.RespMiddle, StatementEndQ.RespLast, StatementEndQ.RespAdd1, StatementEndQ.RespAdd2, StatementEndQ.RespCity, "
				" StatementEndQ.RespState, StatementEndQ.RespZip, "
				" (SELECT Max(Date) FROM MailSent INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
				"	WHERE (MailSentNotesT.Note Like '%%Patient Statement%%Printed%%' OR MailSentNotesT.Note Like '%%Patient Statement%%Run%%' OR MailSentNotesT.Note Like '%%E-Statement%%Exported%%') AND PersonID = StatementSubQ.PatID) AS LastSentDate, "
				" StatementSubQ.PatPhone, "
				" StatementSubQ.FullChargeNoTax, StatementSubQ.ChargeTax1, StatementSubQ.ChargeTax2, StatementSubQ.TaxRate1, StatementSubQ.TaxRate2, "
				" StatementSubQ.TransProvAdd1, StatementSubQ.TransProvAdd2, StatementSubQ.TransProvCity, StatementSubQ.TransProvState, StatementSubQ.TransProvZip, StatementSubQ.TransProvFirst, StatementSubQ.TransProvMiddle, StatementSubQ.TransProvLast, StatementSubQ.TransProvID, "
				" (SELECT COUNT(*) FROM ResponsiblePartyT WHERE PatientID = StatementSubQ.PatID) AS RespPartyCount, "
				" StatementLastPatPayInfoQ.LastPayDate as LastPatientPaymentDate, StatementLastInsPayInfoQ.LastPayDate as LastInsurancePaymentDate, "
				" StatementLastPatPayInfoQ.LastPayAmt as LastPatientPaymentAmount, StatementLastInsPayInfoQ.LastPayAmt as LastInsurancePaymentAmount, "
				" StatementSubQ.CCType, CASE WHEN Len(StatementSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(StatementSubQ.CCNumber, 4) END as CCNumber, StatementSubQ.CheckNo, StatementSubQ.BankNo, StatementSubQ.CheckAcctNo, "
				" StatementSubQ.CCHoldersName, Convert(DateTime, NULL) AS CCExpDate, StatementSubQ.CCAuthNo, StatementSubQ.BankRoutingNum, " 
				" StatementEndQ.PatCoordFirst, StatementEndQ.PatCoordMiddle, StatementEndQ.PatCoordLast, "
				" StatementSubQ.PercentOff, StatementSubQ.DiscountAmt, StatementSubQ.DiscountCategoryDesc, StatementSubQ.TransProviderNPI as ProvNPI, StatementSubQ.CPTCode, "
				" StatementSubQ.CPTModifier1, StatementSubQ.CPTModifier2, StatementSubQ.CPTModifier3, StatementSubQ.CPTModifier4, "
				" StatementSubQ.BillStatementNote, StatementSubQ.LineItemStatementNote, "
				" CASE WHEN StatementSubQ.BillStatementNote = '' OR StatementSubQ.BillStatementNote IS NULL THEN 1 ELSE 0 END as SuppressBillStatementNote, "
				" CASE WHEN StatementSubQ.LineItemStatementNote = '' OR StatementSubQ.LineItemStatementNote IS NULL  THEN 1 ELSE 0 END as SuppressLineItemStatementNote "
				"FROM (SELECT StatementAllData.*, MIN(NextApp.AppDate)  "
				"  AS AppDate, MIN(NextApp.StartTime) AS StartTime,  "
				"  LineItemT.Date AS ARDate,  "
				"  CASE WHEN StatementAR.Thirty IS NULL THEN 0 ELSE StatementAR.Thirty END AS Thirty, "
				"  CASE WHEN StatementAR.Sixty IS NULL THEN 0 ELSE StatementAR.Sixty END AS Sixty, "
				"  CASE WHEN StatementAR.Ninety IS NULL THEN 0 ELSE StatementAR.Ninety END AS Ninety, "
				"  CASE WHEN StatementAR.NinetyPlus IS NULL THEN 0 ELSE StatementAR.NinetyPlus END AS NinetyPlus, "
				"  Age = CASE WHEN StatementAllData.BirthDate IS NULL "
				"   THEN - 1 ELSE DATEDIFF(YYYY, StatementAllData.Birthdate, GetDate()) -  "
				"		CASE WHEN MONTH(StatementAllData.Birthdate) > MONTH(GetDate()) OR (MONTH(StatementAllData.Birthdate) = MONTH(GetDate()) AND DAY(StatementAllData.Birthdate) > DAY(GetDate())) "
				"			THEN 1 ELSE 0 END "
				"  END "
				"  FROM (SELECT StmtCharges.ID, StmtCharges.PatientID,  "
				"  StmtCharges.PatID, StmtCharges.Type,  "
				"  StmtCharges.Total, StmtCharges.Description,  "
				"  StmtCharges.Date, StmtCharges.Insurance,  "
				"  StmtCharges.Last, StmtCharges.First,  "
				"  StmtCharges.Middle, StmtCharges.Address1,  "
				"  StmtCharges.Address2, StmtCharges.City,  "
				"  StmtCharges.State, StmtCharges.Zip,  "
				"  StmtCharges.PatForward,  "
				"  StmtCharges.PatComma,  "
				"  StmtCharges.DocName,  "
				"  StmtCharges.DocAddress1, "
				"  StmtCharges.DocAddress2, "
				"  StmtCharges.DocCity, StmtCharges.DocState, StmtCharges.DocZip, "
				"  StmtCharges.PracName,  "
				"  StmtCharges.PracAddress1,  "
				"  StmtCharges.PracAddress2,  "
				"  StmtCharges.PracCity, StmtCharges.PracState,  "
				"  StmtCharges.PracZip, StmtCharges.PracPhone,  "
				"  StmtCharges.PracFax, StmtCharges.ProvID,  "
				"  StmtCharges.BillID, StmtCharges.BillDate,  "
				"  StmtCharges.BillDescription,  "
				"  StmtCharges.BirthDate, \r\n"
				"  StmtCharges.ICD9Code1,  \r\n"
				"  StmtCharges.ICD9Code2,  \r\n"
				"  StmtCharges.ICD9Code3,  \r\n"
				"  StmtCharges.ICD9Code4,  \r\n"
				"  StmtCharges.ICD10Code1,  \r\n"
				"  StmtCharges.ICD10Code2,  \r\n"
				"  StmtCharges.ICD10Code3,  \r\n"
				"  StmtCharges.ICD10Code4,  \r\n"
				"  StmtCharges.WhichCodes9,  \r\n"
				"  StmtCharges.WhichCodes10,  \r\n"
				"  StmtCharges.WhichCodesBoth,  \r\n"
				"  StmtCharges.Location, "
				"  1 as StatementType, -1 as GroupFixID, "
				"  StmtCharges.LocationFixID, "
				"  StmtCharges.ProvTaxID, StmtCharges.TransProv, StmtCharges.PrePayment,  "
				"  StmtCharges.ProvEIN, StmtCharges.ProvLicense, StmtCharges.ProvUPIN, StmtCharges.ProvDEA, StmtCharges.ProvBCBS, StmtCharges.ProvMedicare, StmtCharges.ProvMedicaid, StmtCharges.ProvWorkersComp, StmtCharges.ProvOtherID, StmtCharges.ProvOtherIDDesc, StmtCharges.HomePhone AS PatPhone, "
				"  StmtCharges.FullChargeNoTax, StmtCharges.ChargeTax1, StmtCharges.ChargeTax2, StmtCharges.TaxRate1, StmtCharges.TaxRate2, "
				"  StmtCharges.TransProvAdd1, StmtCharges.TransProvAdd2, StmtCharges.TransProvCity, StmtCharges.TransProvState, StmtCharges.TransProvZip, StmtCharges.TransProvFirst, StmtCharges.TransProvMiddle, StmtCharges.TransProvLast, StmtCharges.TransProvID, StmtCharges.Quantity, "
				" '' as CCType, '' AS CCNumber,'' AS CheckNo, '' AS BankNo,  '' AS CheckAcctNo, "
				" '' AS CCHoldersName, NULL AS CCExpDate, '' AS CCAuthNo, '' AS BankRoutingNum, "
				"  StmtCharges.PercentOff, StmtCharges.DiscountAmt, StmtCharges.DiscountCategoryDesc, StmtCharges.TransProviderNPI, StmtCharges.CPTCode, "
				"  StmtCharges.CPTModifier1, StmtCharges.CPTModifier2, StmtCharges.CPTModifier3, StmtCharges.CPTModifier4, "
				" StmtCharges.BillStatementNote, StmtCharges.LineItemStatementNote ");
				part2.Format("  FROM (SELECT LineItemT.ID,  "
				" PatientsT.UserDefinedID AS PatientID,  "
				" LineItemT.PatientID AS PatID,  "
				" LineItemT.Type,  "
				" Total = CASE WHEN SUM(ChargeRespT.Amount) "
				"  IS NULL  "
				" THEN 0 ELSE SUM(ChargeRespT.Amount) "
				"  END,  "
				" Description = %s,"
				" LineItemT.Date,  "
				" Insurance = SUM(CASE WHEN ChargeRespT.InsuredPartyID "
				"  IS NOT NULL AND ChargeRespT.InsuredPartyID <> -1  "
				" THEN ChargeRespT.Amount ELSE 0 END), "
				"  PersonT.Last, PersonT.Middle,  "
				" PersonT.First, PersonT.Address1,  "
				" PersonT.Address2, PersonT.City,  "
				" PersonT.State, PersonT.Zip,  "
				" PersonT.HomePhone, "
				" LocationsT.Name PracName,  "
				" LocationsT.Address1 PracAddress1,  "
				" LocationsT.Address2 AS PracAddress2,  "
				" LocationsT.City PracCity,  "
				" LocationsT.State PracState,  "
				" LocationsT.Zip AS PracZip,  "
				" LocationsT.Phone AS PracPhone,  "
				" LocationsT.Fax AS PracFax,  "
				" ProvidersT.PersonID AS ProvID,  "
				" (%s WHERE PersonT.ID = TransProvPersonT.ID) AS DocName,  "
				" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
				" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
				" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				"  AS PatComma,  "
				" PersonT.First + ' ' + PersonT.Middle + ' ' + "
				"  PersonT.Last AS PatForward,  "
				" BillsT.ID AS BillID,  "
				" BillsT.Date AS BillDate,  "
				" BillsT.Description AS BillDescription,  "
				" PersonT.BirthDate, \r\n"
				" ICD9T1.CodeNumber AS ICD9Code1,  \r\n"
				" ICD9T2.CodeNumber AS ICD9Code2,  \r\n"
				" ICD9T3.CodeNumber AS ICD9Code3,  \r\n"
				" ICD9T4.CodeNumber AS ICD9Code4,  \r\n"
				" ICD10T1.CodeNumber AS ICD10Code1,  \r\n"
				" ICD10T2.CodeNumber AS ICD10Code2,  \r\n"
				" ICD10T3.CodeNumber AS ICD10Code3,  \r\n"
				" ICD10T4.CodeNumber AS ICD10Code4,  \r\n"				
				" WhichCodesQ.WhichCodes9,  \r\n"
				" WhichCodesQ.WhichCodes10,  \r\n"
				" WhichCodesQ.WhichCodesBoth,  \r\n"
				" PersonT.Location, LineItemT.LocationID as LocationFixID, "
				" [ProvidersT].[Fed Employer ID] AS ProvTaxID,  "
				" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
				" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2)  AS FullChargeNoTax,   "
				" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate) - 1) AS ChargeTax1, "
				" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate2) - 1) AS ChargeTax2, "
				" Min(ChargesT.TaxRate) AS TaxRate1, Min(ChargesT.TaxRate2) AS TaxRate2, "
				" (%s WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 as PrePayment, 			 TransProvPersonT.First AS TransProvFirst, TransProvPersonT.Middle AS TransProvMiddle, TransProvPersonT.Last As TransProvLast, TransProvPersonT.Address1 As TransProvAdd1, TransProvPersonT.Address2 As TransProvAdd2, TransProvPersonT.City As TransProvCity, TransProvPersonT.State As TransProvState, TransProvPersonT.Zip AS TransProvZip, TransProvPersonT.ID AS TransProvID, ChargesT.Quantity, "
				" COALESCE(TotalPercentOff,0) as PercentOff, Coalesce(TotalDiscount, convert(money,0)) as DiscountAmt, "
				" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDesc, TransProvidersT.NPI as TransProviderNPI, CPTCodeT.Code as CPTCode, "
				" ChargesT.CPTModifier as CPTModifier1, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, "
				" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
				" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = ChargesT.ID) AS LineItemStatementNote "
				, strChargeDescription, strHeaderProv, strTransProv );
				part3.Format(" FROM LineItemT LEFT OUTER JOIN "
				" PatientsT LEFT OUTER JOIN "
				" ProvidersT ON  "
				" PatientsT.MainPhysician = ProvidersT.PersonID "
				"  LEFT OUTER JOIN "
				" PersonT PersonT1 ON  "
				" ProvidersT.PersonID = PersonT1.ID LEFT OUTER "
				"  JOIN "
				" LocationsT RIGHT OUTER JOIN "
				" PersonT ON  "
				" LocationsT.ID = PersonT.Location ON  "
				" PatientsT.PersonID = PersonT.ID ON  "
				" LineItemT.PatientID = PatientsT.PersonID LEFT "
				"  OUTER JOIN "
				" BillsT RIGHT OUTER JOIN "
				" ServiceT LEFT OUTER JOIN "
				" CPTCodeT ON  "
				" ServiceT.ID = CPTCodeT.ID RIGHT OUTER "
				"  JOIN "
				" ChargesT ON  "
				" ServiceT.ID = ChargesT.ServiceID ON  "
				" BillsT.ID = ChargesT.BillID \r\n "
				" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewCharge ON ChargesT.ID = LineItemCorrectionsT_NewCharge.NewLineItemID \r\n"
				
				" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
				" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
				" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
				" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
				" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
				" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
				" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
				" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
				" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
							
				" LEFT JOIN  \r\n"
				" (SELECT ChargesT.ID as ChargeID, "
				" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
				" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
				" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
				" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
				" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
				" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
				" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
				" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
				" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
				" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
				" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
				" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
				" STUFF((SELECT ', ' +  \r\n "
				" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
					 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
					 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
				" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
				" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
				" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
				" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
				" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
				" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
				" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
				" FROM ChargesT \r\n "
				" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "
				
				" LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID  "
				" LEFT OUTER JOIN "
				" ChargeRespT ON  "
				" ChargesT.ID = ChargeRespT.ChargeID ON "
				"  LineItemT.ID = ChargesT.ID "
				" LEFT JOIN PersonT TransProvPersonT ON ChargesT.DoctorsProviders = TransProvPersonT.ID "				
				" LEFT JOIN ProvidersT TransProvidersT ON TransProvPersonT.ID = TransProvidersT.PersonID "
				" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
				" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
				" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
				" WHERE (PatientsT.PersonID > 0) AND  "
				" (LineItemT.Deleted = 0) AND  "
				" (BillsT.Deleted = 0) AND  "
				" BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND "
				" (LineItemT.Type = 10) %s "
				" GROUP BY LineItemT.ID,  "
				" PatientsT.UserDefinedID,  "
				" LineItemT.PatientID, LineItemT.Type,  "
				" CPTCodeT.Code, LineItemT.Description, LineItemCorrectionsT_NewCharge.NewLineItemID,  "
				" LineItemT.Date, PersonT.Last,  "
				" PersonT.Middle, PersonT.First,  "
				" PersonT.Address1, PersonT.Address2,  "
				" PersonT.City, PersonT.State,  "
				" PersonT.Zip, PersonT.HomePhone, LocationsT.Name,  "
				" LocationsT.Address1,  "
				" LocationsT.Address2, LocationsT.City,  "
				" LocationsT.State, LocationsT.Zip,  "
				" LocationsT.Phone, LocationsT.Fax,  "
				" ProvidersT.PersonID, PersonT1.Last,  "
				" PersonT1.First, PersonT1.Middle, PersonT1.Title, "
				" BillsT.ID, BillsT.Date, BillsT.Description,  "
				" PersonT.BirthDate, \r\n "
				" ICD9T1.CodeNumber,  \r\n"
				" ICD9T2.CodeNumber,  \r\n"
				" ICD9T3.CodeNumber,  \r\n"
				" ICD9T4.CodeNumber,  \r\n"
				" ICD10T1.CodeNumber,  \r\n"
				" ICD10T2.CodeNumber,  \r\n"
				" ICD10T3.CodeNumber,  \r\n"
				" ICD10T4.CodeNumber,  \r\n"
				" WhichCodesQ.WhichCodes9,  \r\n"
				" WhichCodesQ.WhichCodes10,  \r\n"
				" WhichCodesQ.WhichCodesBoth,  \r\n"
				" PersonT.Location, LineItemT.LocationID, [ProvidersT].[Fed Employer ID], ChargesT.DoctorsProviders,  "
				" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
				" PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PersonT1.ID, TransProvPersonT.First, TransProvPersonT.Middle, TransProvPersonT.Last, TransProvPersonT.Address1, TransProvPersonT.Address2, TransProvPersonT.City, TransProvPersonT.State, TransProvPersonT.Zip, TransProvPersonT.ID, ChargesT.Quantity, "
				" TotalPercentOff, TotalDiscount, ChargesT.ID, TransProvidersT.NPI, CPTCodeT.Code, LineItemCorrectionsT_NewCharge.NewLineItemID, ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4) "
				"  AS StmtCharges ", strInnerFilter);
				part4 = _T("UNION "
				" SELECT StmtPays.ID, StmtPays.PatientID,  "
				"  StmtPays.PatID, StmtPays.Type,  "
				"  StmtPays.UnAppliedAmount,  "
				"  StmtPays.Description, StmtPays.Date,  "
				"  StmtPays.Insurance, StmtPays.Last,  "
				"  StmtPays.First, StmtPays.Middle,  "
				"  StmtPays.Address1, StmtPays.Address2,  "
				"  StmtPays.City, StmtPays.State, StmtPays.Zip,  "
				"  StmtPays.PatForward, StmtPays.PatComma,  "
				"  StmtPays.DocName, "
				"  StmtPays.DocAddress1, StmtPays.DocAddress2, StmtPays.DocCity, StmtPays.DocState, StmtPays.DocZip, "
				"  StmtPays.PracName,  "
				"  StmtPays.PracAddress1,  "
				"  StmtPays.PracAddress2, StmtPays.PracCity,  "
				"  StmtPays.PracState, StmtPays.PracZip,  "
				"  StmtPays.PracPhone, StmtPays.PracFax,  "
				"  StmtPays.ProvID, StmtPays.BillID,  "
				"  StmtPays.BillDate, StmtPays.BillDescription,  "
				"  StmtPays.BirthDate, \r\n"
				"  StmtPays.ICD9Code1,  \r\n"
				"  StmtPays.ICD9Code2,  \r\n"
				"  StmtPays.ICD9Code3,  \r\n"
				"  StmtPays.ICD9Code4,  \r\n"
				"  StmtPays.ICD10Code1,  \r\n"
				"  StmtPays.ICD10Code2,  \r\n"
				"  StmtPays.ICD10Code3,  \r\n"
				"  StmtPays.ICD10Code4,  \r\n"
				"  StmtPays.WhichCodes9,  \r\n"
				"  StmtPays.WhichCodes10,  \r\n"
				"  StmtPays.WhichCodesBoth,  \r\n"
				"  StmtPays.Location, "
				"  2 as StatementType, -2 as GroupFixID,  "
				"  StmtPays.LocationFixID, "
				"  StmtPays.ProvTaxID,  "
				"  StmtPays.TransProv, "
				"  StmtPays.PrePayment,  "
				"  StmtPays.ProvEIN, StmtPays.ProvLicense, StmtPays.ProvUPIN, StmtPays.ProvDEA, StmtPays.ProvBCBS, StmtPays.ProvMedicare, StmtPays.ProvMedicaid, StmtPays.ProvWorkersComp, StmtPays.ProvOtherID, StmtPays.ProvOtherIDDesc, StmtPays.HomePhone AS PatPhone, "
				"  0 AS FullChargeNoTax, 0 AS ChargeTax1, 0 AS ChargeTax2, 0 AS TaxRate1, 0 As TaxRate2, "
				"  StmtPays.TransProvAdd1, StmtPays.TransProvAdd2, StmtPays.TransProvCity, StmtPays.TransProvState, StmtPays.TransProvZip, StmtPays.TransProvFirst, StmtPays.TransProvMiddle, StmtPays.TransProvLast, StmtPays.TransProvID, 0 As Quantity, "
				" StmtPays.CCType, StmtPays.CCNumber, StmtPays.CheckNo, StmtPays.BankNo, StmtPays.CheckAcctNo, "
				" StmtPays.CCHoldersName, StmtPays.CCExpDate, StmtPays.CCAuthNo, StmtPays.BankRoutingNum, " 
				" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, StmtPays.TransProviderNPI, '' as CPTCode, "
				" '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, " 
				" StmtPays.BillStatementNote, "
				" StmtPays.LineItemStatementNote "
				" FROM (SELECT LineItemT.ID,  "
				" Insurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) "
				"  > 0 THEN CASE WHEN [LineItemT].[Type] "
				"  = 3 THEN MIN([LineItemT].[Amount])  "
				" - SUM(CASE WHEN [AppliesT].[Amount] IS "
				"  NULL  "
				" THEN 0 ELSE [AppliesT].[Amount] END)  "
				" ELSE MIN([LineItemT].[Amount])  "
				" - SUM(CASE WHEN [AppliesT].[Amount] IS "
				"  NULL  "
				" THEN 0 ELSE [AppliesT].[Amount] END)  "
				" END ELSE 0 END, PersonT.Last,  "
				" PersonT.First, PersonT.Middle,  "
				" PersonT.Address1, PersonT.Address2,  "
				" PersonT.City, PersonT.State,  "
				" PersonT.Zip, PersonT.HomePhone,  "
				" LocationsT.Address1 AS PracAddress1,  "
				" LocationsT.Address2 AS PracAddress2,  "
				" LocationsT.Name AS PracName,  "
				" LocationsT.City AS PracCity,  "
				" LocationsT.State AS PracState,  "
				" LocationsT.Zip AS PracZip,  "
				" LocationsT.Phone AS PracPhone,  "
				" LocationsT.Fax AS PracFax,  ");
				part5.Format(" PatientsT.UserDefinedID AS PatientID,  "
				" PatientsT.PersonID AS PatID,  "
				" ProvidersT.PersonID AS ProvID,  "
				" (%s WHERE PersonT.ID = TransProvPersonT.ID) AS DocName,  "
				" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
				" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
				" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				"  AS PatComma,  "
				" PersonT.First + ' ' + PersonT.Middle + ' ' + "
				"  PersonT.Last AS PatForward,  "
				" LineItemT.Type,  "
				" UnAppliedAmount = CASE WHEN [LineItemT].[Type] "
				"  = 3 THEN MIN([LineItemT].[Amount])  "
				" - SUM(CASE WHEN [AppliesT].[Amount] IS "
				"  NULL  "
				" THEN 0 ELSE [AppliesT].[Amount] END)  "
				" ELSE MIN([LineItemT].[Amount])  "
				" - SUM(CASE WHEN [AppliesT].[Amount] IS "
				"  NULL  "
				" THEN 0 ELSE [AppliesT].[Amount] END)  "
				" END, "
				" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
				"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
				"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
				"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
				"ELSE LineItemT.Description END) AS Description,  "
				" LineItemT.Date, LineItemT.ID AS BillID,  "
				" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
				"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
				"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
				"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
				"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
				" LineItemT.Date AS BillDate,  "
				" PersonT.BirthDate, \r\n"
				" '' AS ICD9Code1,  \r\n"
				" '' AS ICD9Code2, \r\n"
				" '' AS ICD9Code3,  \r\n"
				" '' AS ICD9Code4, \r\n"
				" '' AS ICD10Code1,  \r\n"
				" '' AS ICD10Code2, \r\n"
				" '' AS ICD10Code3,  \r\n"
				" '' AS ICD10Code4, \r\n"
				" '' AS WhichCodes9,  \r\n"
				" '' AS WhichCodes10,  \r\n"
				" '' AS WhichCodesBoth,  \r\n"
				" PersonT.Location, "
				" LineItemT.LocationID as LocationFixID,  "
				" [ProvidersT].[Fed Employer ID] AS ProvTaxID,  "
				" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
				" (%s WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, PaymentsT.PrePayment As Prepayment, "
				" TransProvPersonT.First AS TransProvFirst, TransProvPersonT.Middle AS TransProvMiddle, TransProvPersonT.Last As TransProvLast, TransProvPersonT.Address1 As TransProvAdd1, TransProvPersonT.Address2 As TransProvAdd2, TransProvPersonT.City As TransProvCity, TransProvPersonT.State As TransProvState, TransProvPersonT.Zip AS TransProvZip, TransProvPersonT.ID AS TransProvID, "
				" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
				" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, TransProvidersT.NPI as TransProviderNPI, " 
				" '' AS BillStatementNote, "
				" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
				" FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
				" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
				" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID  "
				" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
				" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID  "
				" LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
				" LEFT JOIN PersonT TransProvPersonT ON PaymentsT.ProviderID = TransProvPersonT.ID "
				" LEFT JOIN ProvidersT TransProvidersT ON TransProvPersonT.ID = TransProvidersT.PersonID "
				" LEFT JOIN LineItemCorrectionsT ON PaymentsT.ID = LineItemCorrectionsT.OriginalLineItemID "
				" LEFT JOIN LineItemCorrectionsT VoidT ON PaymentsT.ID = VoidT.VoidingLineItemID "
				" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
				" WHERE (LineItemT.Type < 10) AND  "
				" (LineItemT.Deleted = 0) AND  "
				"  LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND "
				" (PatientsT.PersonID > 0) %s %s " 
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
					, strHeaderProv, strTransProv, strHideUnAppliedPrepayments, strInnerFilter);
				part6 = _T(	" GROUP BY LineItemT.ID, PersonT.Last,  "
				" PersonT.First, PersonT.Middle,  "
				" PersonT.Address1, PersonT.Address2,  "
				" PersonT.City, PersonT.State,  "
				" PersonT.Zip, PersonT.HomePhone, LocationsT.Address1,  "
				" LocationsT.Address2, LocationsT.Name,  "
				" LocationsT.City, LocationsT.State,  "
				" LocationsT.Zip, LocationsT.Phone,  "
				" LocationsT.Fax,  "
				" PatientsT.UserDefinedID,  "
				" PatientsT.PersonID,  "
				" ProvidersT.PersonID, LineItemT.Type,  "
				" LineItemT.Description, LineItemT.Date,  "
				" LineItemCorrectionsT_NewPay.NewLineItemID, "
				" PersonT1.Last, PersonT1.Middle, PersonT1.Title,  "
				" PersonT1.First, PersonT.BirthDate,  "
				" PersonT.Location, "
				" LineItemT.LocationID , "
				" [ProvidersT].[Fed Employer ID],  "
				" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
				" PaymentsT.ProviderID, "
				" PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PaymentsT.PrePayment, PersonT1.ID, "
				" TransProvPersonT.First, TransProvPersonT.Middle, TransProvPersonT.Last, TransProvPersonT.Address1, TransProvPersonT.Address2, TransProvPersonT.City, TransProvPersonT.State, TransProvPersonT.Zip, TransProvPersonT.ID, "
				" CreditCardNamesT.CardName, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
				" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, TransProvidersT.NPI, PaymentsT.ID " 
				" HAVING ((Min(LineItemT.Amount) = 0 AND Min(AppliesT.Amount) IS NULL) OR (CASE WHEN "
				" [LineItemT].[Type] = 3 THEN "
				"  - 1 * MIN([LineItemT].[Amount])  "
				"  + SUM(CASE WHEN [AppliesT].[Amount] IS "
				"  NULL  THEN 0 ELSE [AppliesT].[Amount] END)  "
				" ELSE MIN([LineItemT].[Amount])  "
				" - SUM(CASE WHEN [AppliesT].[Amount] IS "
				" NULL  THEN 0 ELSE [AppliesT].[Amount] END ) "
				" END <> 0)) ) AS StmtPays "
				" UNION ");
				part7 =  _T("  SELECT StmtApplies.ChargeID,  "
				"  StmtApplies.PatientID, StmtApplies.PatID,  "
				"  StmtApplies.Type, StmtApplies.ApplyAmount,  "
				"  StmtApplies.Description, StmtApplies.Date,  "
				"  StmtApplies.Insurance, StmtApplies.Last,  "
				"  StmtApplies.First, StmtApplies.Middle,  "
				"  StmtApplies.Address1, StmtApplies.Address2,  "
				"  StmtApplies.City, StmtApplies.State,  "
				"  StmtApplies.Zip, StmtApplies.PatForward,  "
				"  StmtApplies.PatComma, StmtApplies.DocName,  "
				"  StmtApplies.DocAddress1, StmtApplies.DocAddress2, StmtApplies.DocCity, "
				"  StmtApplies.DocState, StmtApplies.DocZip, "
				"  StmtApplies.PracName,  "
				"  StmtApplies.PracAddress1,  "
				"  StmtApplies.PracAddress2,  "
				"  StmtApplies.PracCity, StmtApplies.PracState,  "
				"  StmtApplies.PracZip, StmtApplies.PracPhone,  "
				"  StmtApplies.PracFax, StmtApplies.ProvID,  "
				"  StmtApplies.BillID, StmtApplies.BillDate,  "
				"  StmtApplies.BillDescription,  "
				"  StmtApplies.BirthDate, \r\n"
				"  StmtApplies.ICD9Code1,  \r\n"
				"  StmtApplies.ICD9Code2,  \r\n"
				"  StmtApplies.ICD9Code3,  \r\n"
				"  StmtApplies.ICD9Code4,  \r\n"
				"  StmtApplies.ICD10Code1,  \r\n"
				"  StmtApplies.ICD10Code2,  \r\n"
				"  StmtApplies.ICD10Code3,  \r\n"
				"  StmtApplies.ICD10Code4,  \r\n"
				"  StmtApplies.WhichCodes9,  \r\n"
				"  StmtApplies.WhichCodes10,  \r\n"
				"  StmtApplies.WhichCodesBoth,  \r\n"
				"  StmtApplies.Location,  "
				"  3 as StatementType, StmtApplies.PaymentID as GroupFixID, "
				"  StmtApplies.LocationFixID, "
				"  StmtApplies.ProvTaxID, "
				"  StmtApplies.TransProv, "
				"  StmtApplies.PrePayment,  "
				"  StmtApplies.ProvEIN, StmtApplies.ProvLicense, StmtApplies.ProvUPIN, StmtApplies.ProvDEA, StmtApplies.ProvBCBS, StmtApplies.ProvMedicare, StmtApplies.ProvMedicaid, StmtApplies.ProvWorkersComp, StmtApplies.ProvOtherID, StmtApplies.ProvOtherIDDesc, StmtApplies.HomePhone AS PatPhone,  "
				"  0 AS FullChargeNoTax, 0 AS ChargeTax1, 0 AS ChargeTax2, 0 AS TaxRate1, 0 As TaxRate2, "
				"  StmtApplies.TransProvAdd1, StmtApplies.TransProvAdd2, StmtApplies.TransProvCity, StmtApplies.TransProvState, StmtApplies.TransProvZip, StmtApplies.TransProvFirst, StmtApplies.TransProvMiddle, StmtApplies.TransProvLast, StmtApplies.TransProvID, 0 As Quantity, "
				" StmtApplies.CCType, StmtApplies.CCNumber, StmtApplies.CheckNo, StmtApplies.BankNo, StmtApplies.CheckAcctNo, "
				" StmtApplies.CCHoldersName, StmtApplies.CCExpDate, StmtApplies.CCAuthNo, StmtApplies.BankRoutingNum, " 
				" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, StmtApplies.TransProviderNPI, '' as CPTCode, "
				"  '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, " 
				" StmtApplies.BillStatementNote, StmtApplies.LineItemStatementNote "
				" FROM (SELECT * "
				" FROM (SELECT AppliesT.DestID AS ChargeID,  "
				"   AppliesT.ID AS PaymentID, "
				"   PatientsT.UserDefinedID AS PatientID, "
				"   PatientsT.PersonID AS PatID,  "
				"   LineItemT1.Type,  "
				"   AppliesT.Amount AS ApplyAmount,  "
				"   LineItemT1.Date,  "
				"   LineItemT1.InputDate,  "
				" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
				"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
				"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
				"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
				" ELSE LineItemT1.Description END) AS Description,  "
				"   Insurance = CASE WHEN [PaymentsT].[InsuredPartyID] "
				"   > 0 THEN [AppliesT].[Amount] ELSE "
				"   0 END, PersonT.Last,  "
				"   PersonT.First, PersonT.Middle,  "
				"   PersonT.Address1,  "
				"   PersonT.Address2, PersonT.CIty,  "
				"   PersonT.State, PersonT.Zip, PersonT.HomePhone, "
				"   LocationsT.Name AS PracName,  "
				"   LocationsT.Address1 AS PracAddress1, "
				"   LocationsT.Address2 AS PracAddress2, ");
				part8.Format(" LocationsT.City AS PracCity,  "
				"   LocationsT.State AS PracState,  "
				"   LocationsT.Zip AS PracZip,  "
				"   LocationsT.Phone AS PracPhone,  "
				"   LocationsT.Fax AS PracFax,  "
				"   PatientsT.SuppressStatement,  "
				"   ProvidersT.PersonID AS ProvID,  "
				"   (%s WHERE PersonT.ID = TransProvPersonT.ID) AS DocName, "
				" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
				" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
				"   PersonT.First + ' ' + PersonT.Middle "
				"   + ' ' + PersonT.Last AS PatForward, "
				"   PersonT.Last + ', ' + PersonT.First "
				"   + ' ' + PersonT.Middle AS PatComma, "
				"   BillsT.ID AS BillID,  "
				"   BillsT.Date AS BillDate,  "
				"   BillsT.Description AS BillDescription, "
				"   PersonT.BirthDate,  \r\n "
				"   ICD9T1.CodeNumber AS ICD9Code1,  \r\n"
				"	ICD9T2.CodeNumber AS ICD9Code2,  \r\n"
				"	ICD9T3.CodeNumber AS ICD9Code3,  \r\n"				
				"	ICD9T4.CodeNumber AS ICD9Code4,  \r\n"
				"   ICD10T1.CodeNumber AS ICD10Code1,  \r\n"
				"	ICD10T2.CodeNumber AS ICD10Code2,  \r\n"
				"	ICD10T3.CodeNumber AS ICD10Code3,  \r\n"				
				"	ICD10T4.CodeNumber AS ICD10Code4,  \r\n"
				"   WhichCodesQ.WhichCodes9,  \r\n"
				"   WhichCodesQ.WhichCodes10,  \r\n"
				"   WhichCodesQ.WhichCodesBoth,  \r\n"
				"   PersonT.Location, "
				"   LineItemT.LocationID as LocationFixID,  "
				"   [ProvidersT].[Fed Employer ID] AS ProvTaxID,  "
				" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
				"   (%s WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 AS PrePayment, "
				" TransProvPersonT.First AS TransProvFirst, TransProvPersonT.Middle AS TransProvMiddle, TransProvPersonT.Last As TransProvLast, TransProvPersonT.Address1 As TransProvAdd1, TransProvPersonT.Address2 As TransProvAdd2, TransProvPersonT.City As TransProvCity, TransProvPersonT.State As TransProvState, TransProvPersonT.Zip AS TransProvZip, TransProvPersonT.ID AS TransProvID, "
				" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
				" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, TransProvidersT.NPI as TransProviderNPI, " 
				" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
				" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
				"  FROM PersonT LEFT OUTER JOIN " 
				"   LocationsT ON  "
				"   PersonT.Location = LocationsT.ID RIGHT "
				"   OUTER JOIN "
				"   LineItemT LineItemT1 RIGHT OUTER "
				"   JOIN "
				"   PaymentsT ON  "
				"   LineItemT1.ID = PaymentsT.ID "
				"   LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				"	LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
				"	LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
				"   RIGHT "
				"   OUTER JOIN "
				"   AppliesT ON  "
				"   PaymentsT.ID = AppliesT.SourceID ", strHeaderProv, strTransProv);
				part8b.Format("   LEFT OUTER JOIN "
				"   LineItemT RIGHT OUTER JOIN "
				"   ChargesT LEFT OUTER JOIN "
				"   BillsT LEFT OUTER JOIN "
				"   PatientsT ON  "
				"   BillsT.PatientID = PatientsT.PersonID "
				"   ON  "
				"   ChargesT.BillID = BillsT.ID ON  "
				"   LineItemT.ID = ChargesT.ID ON  "
				"   AppliesT.DestID = ChargesT.ID ON "
				"   PersonT.ID = PatientsT.PersonID "

				" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
				" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
				" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
				" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
				" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
				" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
				" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
				" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
				" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
							
				" LEFT JOIN  \r\n"
				" (SELECT ChargesT.ID as ChargeID, "
				" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
				" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
				" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
				" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
				" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
				" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
				" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
				" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
				" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
				" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
				" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
				" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
				" STUFF((SELECT ', ' +  \r\n "
				" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
					 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
					 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
				" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
				" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
				" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
				" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
				" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
				" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
				" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
				" FROM ChargesT \r\n "
				" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "
				
				"	LEFT OUTER JOIN "
				"   PersonT PersonT1 RIGHT OUTER JOIN "
				"   ProvidersT ON  "
				"   PersonT1.ID = ProvidersT.PersonID "
				"   ON  "
				"   PatientsT.MainPhysician = ProvidersT.PersonID "
				" LEFT JOIN PersonT TransProvPersonT ON ChargesT.DoctorsProviders = TransProvPersonT.ID "
				" LEFT JOIN ProvidersT TransProvidersT ON TransProvPersonT.ID = TransProvidersT.PersonID "
				" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
				" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
				" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
				" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON PaymentsT.ID = LineItemCorrections2T.OriginalLineItemID "
				" LEFT JOIN LineItemCorrectionsT Void2T ON PaymentsT.ID = Void2T.VoidingLineItemID "
				" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
				"  WHERE (LineItemT1.Deleted = 0) AND  "
				"   (LineItemT.Deleted = 0) AND  "
				"   (BillsT.Deleted = 0) AND  "
				"   (AppliesT.PointsToPayments = 0)  "
				"    AND BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL "
				"   AND (PatientsT.PersonID > 0) %s " 
					+ GetStatementChargebackString("ChargebacksPayments") + " "
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
				" )  "
				" AS StatementDataAppliesCharges ", strInnerFilter);
				part9.Format("UNION "
				" SELECT * "
				" FROM (SELECT AppliesT.DestID AS ChargeID,  "
				"   AppliesT.ID AS PaymentID, "
				"   PatientsT.UserDefinedID AS PatientID, "
				"   PatientsT.PersonID AS PatID,  "
				"   LineItemT1.Type,  "
				"   AppliesT.Amount AS ApplyAmount,  "
				"   LineItemT1.Date,  "
				"   LineItemT1.InputDate,  "
				" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
				"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
				"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
				"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
				" ELSE LineItemT1.Description END) AS Description,  "
				"   CASE WHEN PaymentsT.InsuredPartyID "
				"   > 0 THEN AppliesT.Amount ELSE 0 "
				"   END AS Insurance,  "
				"   PersonT.Last, PersonT.First,  "
				"   PersonT.Middle,  "
				"   PersonT.Address1,  "
				"   PersonT.Address2, PersonT.City,  "
				"   PersonT.State, PersonT.Zip, PersonT.HomePhone, "
				"   LocationsT.Name AS PracName,  "
				"   LocationsT.Address1 AS PracAddress1, "
				"   LocationsT.Address2 AS PracAddress2, "
				"   LocationsT.City AS PracCity,  "
				"   LocationsT.State AS PracState,  "
				"   LocationsT.Zip AS PracZip,  "
				"   LocationsT.Phone AS PracPhone,  "
				"   LocationsT.Fax AS PracFax,  "
				"   PatientsT.SuppressStatement,  "
				"   ProvidersT.PersonID AS ProvID,  "
				"   (%s WHERE PersonT.ID = TransProvPersonT.ID) AS DocName, "
				" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
				" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
				"   PersonT.First + ' ' + PersonT.Middle "
				"   + ' ' + PersonT.Last AS PatForward, "
				"   PersonT.Last + ', ' + PersonT.First "
				"   + ' ' + PersonT.Middle AS PatComma, "
				"   LineItemT.ID AS BillID,  "
				"   LineItemT.Date AS BillDate,  "
				" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
				"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
				"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
				"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
				"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
				"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
				"   PersonT.BirthDate,  \r\n"
				"   '' AS ICD9Code1,  \r\n"
				"   '' AS ICD9Code2,  \r\n"
				"   '' AS ICD9Code3,  \r\n"
				"   '' AS ICD9Code4,  \r\n"
				"   '' AS ICD10Code1,  \r\n"
				"   '' AS ICD10Code2,  \r\n"
				"   '' AS ICD10Code3,  \r\n"
				"   '' AS ICD10Code4,  \r\n"
				"   '' AS WhichCodes9,  \r\n"
				"   '' AS WhichCodes10,  \r\n"
				"   '' AS WhichCodesBoth,  \r\n"
				"   PersonT.Location, "
				"   LineItemT.LocationID as LocationFixID, "
				"   [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
				" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
				"   (%s WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, 0 As Prepayment,  "
				"  TransProvPersonT.First AS TransProvFirst, TransProvPersonT.Middle AS TransProvMiddle, TransProvPersonT.Last As TransProvLast, TransProvPersonT.Address1 As TransProvAdd1, TransProvPersonT.Address2 As TransProvAdd2, TransProvPersonT.City As TransProvCity, TransProvPersonT.State As TransProvState, TransProvPersonT.Zip AS TransProvZip, TransProvPersonT.ID AS TransProvID, "
				" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
				" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, TransProvidersT.NPI as TransProviderNPI, " 
				" '' AS BillStatementNote, "
				" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
				"  FROM PersonT PersonT1 RIGHT OUTER "
				"   JOIN "
				"   ProvidersT ON  "
				"   PersonT1.ID = ProvidersT.PersonID "
				"   RIGHT OUTER JOIN "
				"   PersonT LEFT OUTER JOIN "
				"   LocationsT ON  "
				"   PersonT.Location = LocationsT.ID RIGHT "
				"   OUTER JOIN "
				"   LineItemT LineItemT1 LEFT OUTER "
				"   JOIN "
				"   PatientsT ON  ", strHeaderProv, strTransProv);
				part10.Format("   LineItemT1.PatientID = PatientsT.PersonID "
				"   ON  "
				"   PersonT.ID = PatientsT.PersonID ON "
				"   ProvidersT.PersonID = PatientsT.MainPhysician "
				"   RIGHT OUTER JOIN "
				"   PaymentsT ON  "
				"   LineItemT1.ID = PaymentsT.ID RIGHT "
				"   OUTER JOIN "
				"   LineItemT RIGHT OUTER JOIN "
				"   AppliesT ON  "
				"   LineItemT.ID = AppliesT.DestID ON "
				"   PaymentsT.ID = AppliesT.SourceID "
				"	LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID "
				"	LEFT JOIN ChargebacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID "
				"	LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID "
				"	LEFT JOIN ChargebacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID "
				"   LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				" LEFT JOIN PersonT TransProvPersonT ON PaymentsT.ProviderID = TransProvPersonT.ID "
				" LEFT JOIN ProvidersT TransProvidersT ON TransProvPersonT.ID = TransProvidersT.PersonID "
				" LEFT JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
				" LEFT JOIN LineItemCorrectionsT VoidT ON LineItemT.ID = VoidT.VoidingLineItemID "
				" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON LineItemT1.ID = LineItemCorrections2T.OriginalLineItemID "
				" LEFT JOIN LineItemCorrectionsT Void2T ON LineItemT1.ID = Void2T.VoidingLineItemID "	
				" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
				"  WHERE (LineItemT1.Deleted = 0) AND  "
				"   (LineItemT1.Deleted = 0) AND  "
				"    LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL AND "
				"   (AppliesT.PointsToPayments = 1)  "
				"   AND (PatientsT.PersonID > 0) %s " 
					+ GetStatementChargebackString("ChargebacksPayments1") + " " 
					+ GetStatementChargebackString("ChargebacksAdjustments1") + " "
					+ GetStatementChargebackString("ChargebacksPayments") + " " 
					+ GetStatementChargebackString("ChargebacksAdjustments") + " "
				" )  "
				" AS StatementDataAppliesPays)  "
				"  AS StmtApplies) AS StatementAllData LEFT JOIN "
				"   (SELECT AppointmentsT.Date AS AppDate,  "
				"   AppointmentsT.StartTime,  "
				"   AppointmentsT.PatientID AS PatID "
				" FROM AppointmentsT "
				" WHERE (((AppointmentsT.Date) > GetDate()) AND  "
				"   (AppointmentsT.PatientID > 0) AND  "
				"   ((AppointmentsT.Status) <> 4))) AS NextApp ON  "
				"  NextApp.PatID = StatementAllData.PatID LEFT JOIN "
				"  LineItemT ON LineItemT.ID = StatementAllData.ID LEFT  "
				"  JOIN "
				"  ( SELECT CASE WHEN Thirty IS NULL THEN 0 ELSE Thirty END AS Thirty, CASE WHEN Sixty IS NULL THEN 0 ELSE SIXTY END AS Sixty, CASE WHEN Ninety IS NULL THEN 0 ELSE NINETY END AS Ninety,  "
				"  CASE WHEN NINETYPLUS  IS NULL THEN 0 ELSE NinetyPlus END AS NinetyPlus, PatientID, ProviderID  "
				"  FROM(  "
				"  SELECT PatAR.PatientID, PatAR.ProviderID,  Sum(PatAR.Thirty) AS Thirty, Sum(PatAR.Sixty) AS Sixty, Sum(PatAR.Ninety) AS Ninety, Sum(PatAR.NinetyPlus) AS NinetyPlus  "
				"  FROM  "
				"  (SELECT Sum((CASE WHEN Thirty.ChargeAmount IS NULL THEN 0 ELSE Thirty.ChargeAmount END) - (CASE WHEN Thirty.PPAmount IS NULL THEN 0 ELSE Thirty.PPAmount END)) AS Thirty, 0 AS Sixty, 0 As Ninety, 0 AS NinetyPlus, PatientID, ProviderID   "
				"  FROM   "
				"  ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, ProviderID FROM    "
				"  (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, ChargesT.DoctorsProviders as ProviderID FROM ChargeRespDetailT LEFT JOIN  "
				"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID   "
				"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
				"   LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd,-30, getDate()))  AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  %s "
				"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, ChargesT.DoctorsProviders) As Charges   "
				"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM   "
				"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID  "
				"   GROUP BY PatientID, ProviderID)  "
				"   UNION   "
				"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, PaymentsT.ProviderID   "
				"   FROM   "
				"   LineItemT LEFT JOIN    "
				"   (SELECT DestID, Sum(Amount) as Amount   "
				"   FROM AppliesT   "
				"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID    "
				"   WHERE (InsuredPartyID = -1)   "
				" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
				"   LEFT JOIN  "
				" 	(SELECT SourceID, Sum(Amount) AS Amount   "
				"   FROM AppliesT   "
				"  	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID   "
				"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
				"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd,-30, getDate())) AND (PaymentsT.InsuredPartyID = -1) %s %s "
				"   GROUP BY LineItemT.PatientID, PaymentsT.ProviderID)) AS Thirty    "
				"   GROUP BY Thirty.PatientID, Thirty.ProviderID   "
				"   UNION    "
				"   SELECT 0 AS Thirty, Sum((CASE WHEN Sixty.ChargeAmount IS NULL THEN 0 ELSE Sixty.ChargeAmount END) - (CASE WHEN Sixty.PPAmount IS NULL THEN 0 ELSE Sixty.PPAmount END)) AS Sixty, 0 AS Ninety, 0 AS NinetyPlus, PatientID, ProviderID    "
				"   FROM   "
				"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, ProviderID FROM    "
				"  	(SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, ChargesT.DoctorsProviders AS ProviderID FROM ChargeRespDetailT LEFT JOIN   "
				"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID   "
				" 	LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
				"	LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
				" 	WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date <= DATEADD(dd, -30, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  %s "
				" 	GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, ChargesT.DoctorsProviders) As Charges   "
				"   LEFT JOIN    "
				" 	(SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM   "
				" 	ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays   "
				"   ON Charges.DetailID = Pays.DetailID  "
				"   GROUP BY PatientID, ProviderID)  "
				"   UNION   "
				"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, PaymentsT.ProviderID  "
				"   FROM   "
				"   LineItemT LEFT JOIN   "
				"   (SELECT DestID, Sum(Amount) as Amount   "
				"   	FROM AppliesT    "
				" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID    "
				"  	WHERE (InsuredPartyID = -1)   "
				" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID   "
				"   LEFT JOIN  "
				" 	(SELECT SourceID, Sum(Amount) AS Amount   "
				"  	FROM AppliesT   "
				" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID   "
				"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
				"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd, -60, getDate())) AND (LineItemT.Date <= DATEADD(dd, -30, getDate())) AND (PaymentsT.InsuredPartyID = -1) %s %s "
				"   GROUP BY LineItemT.PatientID, PaymentsT.ProviderID)) AS Sixty   "
				"   GROUP BY Sixty.PatientID, Sixty.ProviderID  "
				"   UNION   "
				"   SELECT 0 AS Thirty, 0 AS Sixty, Sum((CASE WHEN Ninety.ChargeAmount IS NULL THEN 0 ELSE Ninety.ChargeAmount END) - (CASE WHEN Ninety.PPAmount IS NULL THEN 0 ELSE Ninety.PPAmount END)) AS Ninety, 0 AS NinetyPlus, PatientID, ProviderID   "
				"   FROM  "
				"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, ProviderID FROM   "
				"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, ChargesT.DoctorsProviders AS ProviderID FROM ChargeRespDetailT LEFT JOIN  "
				"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
				"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
				"   LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date >= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1) %s "
				"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, ChargesT.DoctorsProviders) As Charges  "
				"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM   "
				"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID  "
				"   GROUP BY PatientID, ProviderID)  "
				"   UNION   "
				"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, PaymentsT.ProviderID  "
				"   FROM   "
				"   LineItemT LEFT JOIN   "
				"  	(SELECT DestID, Sum(Amount) as Amount   "
				"  	FROM AppliesT   "
				"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
				"  	WHERE (InsuredPartyID = -1)  "
				"  	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
				"   LEFT JOIN  "
				"  	(SELECT SourceID, Sum(Amount) AS Amount  "
				" 	FROM AppliesT  "
				" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
				"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
				"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd, -60, getDate())) AND (LineItemT.Date >= DATEADD(dd, -90, getDate())) AND (PaymentsT.InsuredPartyID = -1) %s %s "
				"   GROUP BY LineItemT.PatientID, PaymentsT.ProviderID)) AS Ninety  "
				"   GROUP BY Ninety.PatientID, Ninety.ProviderID  "
				"    UNION    "
				"   SELECT 0 AS Thirty, 0 AS Sixty, 0 AS Ninety, Sum((CASE WHEN NinetyPlus.ChargeAmount IS NULL THEN 0 ELSE NinetyPlus.ChargeAmount END) - (CASE WHEN NinetyPlus.PPAmount IS NULL THEN 0 ELSE NinetyPlus.PPAmount END)) AS NinetyPlus, PatientID, ProviderID   "
				"   FROM   "
				"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, ProviderID FROM   "
				"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, ChargesT.DoctorsProviders As ProviderID FROM ChargeRespDetailT LEFT JOIN  "
				"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID   "
				"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID   "
				"   LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1) %s "
				"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, ChargesT.DoctorsProviders) As Charges   "
				"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
				"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID   "
				"   GROUP BY PatientID, ProviderID)   "
				"    UNION    "
				"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, PaymentsT.ProviderID "
				"   FROM   "
				"   LineItemT LEFT JOIN   "
				"  	(SELECT DestID, Sum(Amount) as Amount  "
				" 	FROM AppliesT   "
				" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
				"  	WHERE (InsuredPartyID = -1)   "
				" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID   "
				"    LEFT JOIN   "
				" 	(SELECT SourceID, Sum(Amount) AS Amount   "
				" 	FROM AppliesT   "
				" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
				"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
				"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd,-90, getDate())) AND (PaymentsT.InsuredPartyID = -1)  %s %s "
				"   GROUP BY LineItemT.PatientID, PaymentsT.ProviderID)) AS NinetyPlus  "
				"   GROUP BY NinetyPlus.PatientID, NinetyPlus.ProviderID) PatAR   "
				"   GROUP BY PatAR.PatientID, PatAR.ProviderID) AS PatientAR) AS StatementAR ON StatementAllData.PatID = StatementAR.PatientID "
				"   AND StatementAllData.TransProvID = StatementAR.ProviderID "
				"  GROUP BY StatementAllData.ID, StatementAllData.PatientID,  "
				"  StatementAllData.PatID, StatementAllData.Type,  ", strInnerFilterApp, strInnerFilter, strHideUnAppliedPrepayments, strInnerFilter, strInnerFilter, strHideUnAppliedPrepayments, strInnerFilter, strInnerFilter, strHideUnAppliedPrepayments, strInnerFilter, strInnerFilter, strHideUnAppliedPrepayments, strInnerFilter);
				part16 = _T("  StatementAllData.Total, StatementAllData.Description,  "
				"  StatementAllData.Date, StatementAllData.Insurance,  "
				"  StatementAllData.Last, StatementAllData.First,  "
				"  StatementAllData.Middle, StatementAllData.Address1,  "
				"  StatementAllData.Address2, StatementAllData.City,  "
				"  StatementAllData.State, StatementAllData.Zip,  "
				"  StatementAllData.PatForward,  "
				"  StatementAllData.PatComma,  "
				"  StatementAllData.DocName,  "
				"  StatementAllData.DocAddress1, "
				"  StatementAllData.DocAddress2, "
				"  StatementAllData.DocCity, "
				"  StatementAllData.DocState, "
				"  StatementAllData.DocZip, "
				"  StatementAllData.PracName,  "
				"  StatementAllData.PracAddress1,  "
				"  StatementAllData.PracAddress2,  "
				"  StatementAllData.PracCity, StatementAllData.PracState,  "
				"  StatementAllData.PracZip, StatementAllData.PracPhone,  "
				"  StatementAllData.PracFax, StatementAllData.ProvID,  "
				"  StatementAllData.BillID, StatementAllData.BillDate,  "
				"  StatementAllData.BillDescription,  "
				"  StatementAllData.BirthDate, LineItemT.Date,  \r\n"
				"  StatementAllData.ICD9Code1,  \r\n"
				"  StatementAllData.ICD9Code2,  \r\n"
				"  StatementAllData.ICD9Code3,  \r\n"
				"  StatementAllData.ICD9Code4,  \r\n"
				"  StatementAllData.ICD10Code1,  \r\n"
				"  StatementAllData.ICD10Code2,  \r\n"
				"  StatementAllData.ICD10Code3,  \r\n"
				"  StatementAllData.ICD10Code4,  \r\n"
				"  StatementAllData.WhichCodes9,  \r\n"
				"  StatementAllData.WhichCodes10,  \r\n"
				"  StatementAllData.WhichCodesBoth,  \r\n"
				"  StatementAllData.Location,  "
				"  StatementAR.Thirty, "
				"  StatementAR.Sixty, "
				"  StatementAR.Ninety, "
				"  StatementAR.NinetyPlus, "
				"  StatementAllData.GroupFixID, StatementAllData.StatementType, StatementAllData.LocationFixID, "
				"  StatementAllData.DocAddress1, StatementAllData.DocAddress2, StatementAllData.DocCity, StatementAllData.DocState, StatementAllData.DocZip, "
				"  StatementAllData.ProvTaxID, StatementAllData.TransProv, StatementAllData.PrePayment,  "
				"  StatementAllData.ProvEIN, StatementAllData.ProvLicense, StatementAllData.ProvUPIN, StatementAllData.ProvDEA, StatementAllData.ProvBCBS, StatementAllData.ProvMedicare, StatementAllData.ProvMedicaid, StatementAllData.ProvWorkersComp, StatementAllData.ProvOtherID, StatementAllData.ProvOtherIDDesc, StatementAllData.PatPhone, "
				"  StatementAllData.FullChargeNoTax, StatementAllData.ChargeTax1, StatementAllData.ChargeTax2, StatementAllData.TaxRate1, StatementAllData.TaxRate2,  StatementAllData.FullChargeNoTax, StatementAllData.ChargeTax1, StatementAllData.ChargeTax2, StatementAllData.TaxRate1, StatementAllData.TaxRate2, StatementAllData.TransProvAdd1, StatementAllData.TransProvAdd2, StatementAllData.TransProvCity, StatementAllData.TransProvState, StatementAllData.TransProvZip, StatementAllData.TransProvFirst, StatementAllData.TransProvMiddle, StatementAllData.TransProvLast, StatementAllData.TransProvID, StatementAllData.Quantity,  "
				"  StatementAllData.CCType, StatementAllData.CCNumber, StatementAllData.CheckNo, StatementAllData.BankNo, StatementAllData.CheckAcctNo, "
				" StatementAllData.CCHoldersName, StatementAllData.CCExpDate, StatementAllData.CCAuthNo, StatementAllData.BankRoutingNum, StatementAllData.PercentOff, StatementAllData.DiscountAmt, StatementAllData.DiscountCategoryDesc, StatementAllData.TransProviderNPI, StatementAllData.CPTCode, StatementAllData.CPTModifier1, StatementAllData.CPTModifier2, StatementAllData.CPTModifier3, StatementAllData.CPTModifier4, "
				" StatementAllData.BillStatementNote, StatementAllData.LineItemStatementNote )  "
				" AS StatementSubQ LEFT OUTER JOIN "
				" (SELECT * "
				"  FROM (SELECT PersonT1.First + ' ' + PersonT1.Middle + ' ' + PersonT1.Last + ' ' + PersonT1.Title ");
				part17.Format("  AS DocName, PersonT1.ID AS ProvID,  "
				"  PatientsT.SuppressStatement,  "
				"  PatientsT.PersonID AS PatID,  "
				"  PatientsT.PrimaryRespPartyID AS PrimaryRespID, "
				"  PatientsT.StatementNote, "
				"  PatCoordT.First as PatCoordFirst, PatCoordT.Middle as PatCoordMiddle, PatCoordT.Last as PatCoordLast "
				" FROM PatientsT LEFT OUTER JOIN "
				"  PersonT PersonT1 ON  "
				"  PatientsT.MainPhysician = PersonT1.ID LEFT OUTER "
				"   JOIN "
				"  PersonT ON PatientsT.PersonID = PersonT.ID "
				"  LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID )  "
				"   AS PatInfo LEFT OUTER JOIN "
				" (SELECT InsuranceCoT.Name AS PriInsCo,  "
				" PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last "
				" AS PriGuarForward,  "
				" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				" AS PriGuarComma,  "
				" PersonT.First AS PriInsFirst,  "
				" PersonT.Middle AS PriInsMiddle,  "
				" PersonT.Last AS PriInsLast,  "
				" PatientsT.PersonID AS PersonID "
				"  FROM InsuranceCoT RIGHT OUTER JOIN "
				" PersonT RIGHT OUTER JOIN "
				" InsuredPartyT ON  "
				" PersonT.ID = InsuredPartyT.PersonID ON  "
				" InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				" RIGHT OUTER JOIN "
				" PatientsT ON  "
				" InsuredPartyT.PatientID = PatientsT.PersonID "
				"  WHERE InsuredPartyT.RespTypeID = 1)  "
				"   AS PriInsInfo ON  "
				"   PatInfo.PatID = PriInsInfo.PersonID LEFT OUTER JOIN "
				" (SELECT InsuranceCoT.Name AS SecInsCo,  "
				" PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last "
				" AS SecGuarForward,  "
				" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				" AS SecGuarComma,  "
				" PersonT.First AS SecInsFirst,  "
				" PersonT.Middle AS SecInsMiddle,  "
				" PersonT.Last AS SecInsLast,  "
				" PatientsT.PersonID AS PersID "
				"  FROM InsuranceCoT RIGHT OUTER JOIN "
				" PersonT RIGHT OUTER JOIN "
				" InsuredPartyT ON  "
				" PersonT.ID = InsuredPartyT.PersonID ON  "
				" InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				" RIGHT OUTER JOIN "
				" PatientsT ON  "
				" InsuredPartyT.PatientID = PatientsT.PersonID "
				"  WHERE InsuredPartyT.RespTypeID = 2)  "
				"   AS SecInsInfo ON PatInfo.PatID = SecInsInfo.PersID "
				"  LEFT OUTER JOIN "
				" (SELECT ResponsiblePartyT.PersonID as RespID, ResponsiblePartyT.PatientID AS RespPatID, First as RespFirst, Middle as RespMiddle, Last as RespLast, Address1 as RespAdd1, Address2 as RespAdd2,  "
				" City as RespCity, State as RespState, Zip as RespZip "
				" FROM PersonT Inner Join ResponsiblePartyT ON PersonT.ID = ResponsiblePartyT.PersonID LEFT JOIN PatientsT ON ResponsiblePartyT.PatientID = PatientsT.PersonID @RespFilter) AS ResPartyT  "
				" ON PatInfo.PatID = ResPartyT.RespPatID)  "
				" AS StatementEndQ ON  "
				" StatementSubQ.PatID = StatementEndQ.PatID "
				" LEFT JOIN "
				"(SELECT * FROM [%s]) AS StatementLastPatPayInfoQ " 
				" ON CASE WHEN StatementSubQ.TransProvID IS NULL THEN -1 ELSE StatementSubQ.TransProvID END = "
				" CASE WHEN StatementLastPatPayInfoQ.ProvID IS NULL THEN -1 ELSE StatementLastPatPayInfoQ.ProvID END "
				" AND StatementSubQ.PatID = StatementLastPatPayInfoQ.PatID "
				" AND StatementLastPatPayInfoQ.IsInsurance = 0 "
				" LEFT JOIN "
				"(SELECT * FROM [%s]) AS StatementLastInsPayInfoQ " 
				" ON CASE WHEN StatementSubQ.TransProvID IS NULL THEN -1 ELSE StatementSubQ.TransProvID END = "
				" CASE WHEN StatementLastInsPayInfoQ.ProvID IS NULL THEN -1 ELSE StatementLastInsPayInfoQ.ProvID END "
				" AND StatementSubQ.PatID = StatementLastInsPayInfoQ.PatID "
				" AND StatementLastInsPayInfoQ.IsInsurance = 1 "
				"WHERE (StatementEndQ.SuppressStatement = 0)", strTempTableName, strTempTableName);

					// (a.walling 2006-10-24 13:12) - PLID 16059 - Multiple Responsible parties
					part17.Replace("@RespFilter", strRespFilter);
				
					CString strSQL = part1 + part2 + part3 + part4 + part5 + part6 + part7 + part8 + part8b + part9 + part10 + part11 + part12 + part13 + part14 + part15 + part16 + part17;

					//now we have to add the other filters that we haven't already filtered on
					CString strMoreFilters;
					AddPartToClause(strMoreFilters, m_pReport->GetLocationFilter(m_nSubLevel, m_nSubRepNum));
					AddPartToClause(strMoreFilters, m_pReport->GetProviderFilter(m_nSubLevel, m_nSubRepNum));
					AddPartToClause(strMoreFilters, m_pReport->GetDateFilter(m_nSubLevel, m_nSubRepNum));
					AddPartToClause(strMoreFilters, m_pReport->GetExternalFilter(m_nSubLevel, m_nSubRepNum));

					strMoreFilters.TrimLeft();
					strMoreFilters.TrimRight();
					
					if (!strMoreFilters.IsEmpty()) {
						m_pReport->ConvertCrystalToSql(strSQL, strMoreFilters);
						AddFilter(strSQL, strMoreFilters, TRUE);
					}		
					return CComplexReportQuery(strSQL);

				}		
				break;



				case 234: //Individual Patient Statement
				case 354:
				{



				CString strLoc;
				
				if (m_pReport->nLocation == -1) {
								strLoc = " ";
				}
				else {
					strLoc.Format("AND (LineItemT.LocationID  = %li)", m_pReport->nLocation);
				}


				CString strBillID, strNoUnAppPays;
				if (m_pReport->nExtraID == -1) {
					strBillID = " ";
					strNoUnAppPays = " ";
				}
				else {
					strBillID.Format(" AND (BillsT.ID = %li) ", m_pReport->nExtraID);
					strNoUnAppPays = " AND (1 = 0) ";
				}


				long nTransFormat, nHeaderFormat;
				nTransFormat = GetRemotePropertyInt("SttmntTransProvFormat", 0, 0, "<None>");
				nHeaderFormat = GetRemotePropertyInt("SttmntHeaderProvFormat", 0, 0, "<None>");
				CString strTransProv = GetStatementProvString(nTransFormat);
				CString strHeaderProv = GetStatementProvString(nHeaderFormat);
				CString strHideUnAppliedPrepayments = GetStatementUnAppliedPrePaymentsString();
				CString strChargeDescription = GetStatementChargeDescription();
							


				// (j.gruber 2007-05-01 17:13) - PLID 25745 - only show the last 4 digits of the ccnumber
				// (j.gruber 2007-05-15 09:08) - PLID 25987 - take out credit card expiration dates
				// (j.gruber 2007-05-29 17:21) - PLID 26096 - change the joins of the top part of the applies query
				// (j.gruber 2007-05-30 12:37) - PLID 25978 - added discount fields and discount categories
				// (j.gruber 2007-06-28 16:35) - PLID 26501 - parameterize the individiual statement
				// (j.gruber 2007-06-29 09:37) - PLID 26501 - parameterized the individual statement
				// (e.lally 2007-07-13) PLID 26670 - Updated all references to PaymentPlansT. CCType with link to CardName, aliased as CCType where applicable.
				// (j.gruber 2008-07-03 15:26) - PLID 27499 - changed the filter on payments applied to chargs and payments applied to payments query
				// (j.jones 2008-09-05 10:23) - PLID 30288 - supported MailSentNotesT
				// (j.gruber 2009-11-05 17:36) - PLID 36217 - add providerNPI
				// (j.gruber 2009-11-25 12:25) - PLID 36430 - added cptcode
				// (j.gruber 2009-12-24 12:48) - PLID 17122 - added cptmodifiers
				// (j.gruber 2010-02-19 09:46) - PLID 37447 - re-aliased the providerNPI to provNPI
				// (j.gruber 2010-06-14 15:46) - PLID 36484 - add billing notes
				// (j.gruber 2011-07-05 10:19) - PLID 44831 - take out voids and originals
				// (a.wilson 2012-2-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
				// (j.gruber 2014-03-04 14:11) - PLID 61167 - update for ICD10, and diag and whichcodes refactor
				//TES 7/17/2014 - PLID 62565 - Added code to hide chargebacks when requested
				// (j.jones 2015-03-09 09:48) - PLID 64937 - if the description begins with 'Corrected Charge',
				// 'Corrected Payment', etc., strip that off
				// (r.goldschmidt 2015-11-10 16:25) - PLID 65568 - Last payment should exclude voided payments
				CString part1, part2, part3, part4, part5, part6, part7, part8, part9, part10, part11;
						part1.Format(" SELECT StatementSubQ.ID, StatementSubQ.PatientID, StatementSubQ.PatID as PatID, StatementSubQ.Type, StatementSubQ.Total, StatementSubQ.Description, StatementSubQ.Date as TDate,  "
						" StatementSubQ.Insurance, StatementSubQ.Last, StatementSubQ.First, StatementSubQ.Middle, StatementSubQ.Address1, StatementSubQ.Address2, "
						" StatementSubQ.City, StatementSubQ.State, StatementSubQ.Zip, StatementSubQ.PatForward, StatementSubQ.PatComma, StatementSubQ.DocName, StatementSubQ.DocAddress1, StatementSubQ.DocAddress2, StatementSubQ.DocCity, StatementSubQ.DocState, StatementSubQ.DocZip, StatementSubQ.ProvTaxID, StatementSubQ.PracName, StatementSubQ.PracAddress1, "
						" StatementSubQ.PracAddress2, StatementSubQ.PracCity, StatementSubQ.PracState, StatementSubQ.PracZip, StatementSubQ.PracPhone, StatementSubQ.PracFax, StatementSubQ.ProvID as ProvID2, StatementSubQ.BillId, "
						" StatementSubQ.BillDate, StatementSubQ.BillDescription, StatementSubQ.Birthdate, \r\n"
						" StatementSubQ.ICD9Code1, StatementSubQ.ICD9Code2, StatementSubQ.ICD9Code3, StatementSubQ.ICD9Code4, \r\n"
						" StatementSubQ.ICD10Code1, StatementSubQ.ICD10Code2, StatementSubQ.ICD10Code3, StatementSubQ.ICD10Code4, \r\n"
						" StatementSubQ.WhichCodes9, StatementSubQ.WhichCodes10, StatementSubQ.WhichCodesBoth, "
						" StatementSubQ.Location, StatementSubQ.StatementType, StatementSubQ.GroupFixID, StatementSubQ.Appdate, StatementSubQ.StartTime, StatementSubQ.ARDate, StatementSubQ.Age,  "
						" StatementSubQ.TransProv, StatementSubQ.PrePayment, StatementSubQ.Quantity, StatementSubQ.Thirty, StatementSubQ.Sixty, StatementSubQ.Ninety, StatementSubQ.NinetyPlus, "
						" StatementSubQ.ProvEIN, StatementSubQ.ProvLicense, StatementSubQ.ProvUPIN, StatementSubQ.ProvDEA, StatementSubQ.ProvBCBS, StatementSubQ.ProvMedicare, StatementSubQ.ProvMedicaid, StatementSubQ.ProvWorkersComp, StatementSubQ.ProvOtherID, StatementSubQ.ProvOtherIDDesc, "
						" StatementEndQ.DocName as DocName2, StatementEndQ.ProvID as ProvID, StatementEndQ.SuppressStatement, StatementEndQ.PatID as PatID2, StatementEndQ.StatementNote, StatementEndQ.PriInsCo, StatementEndQ.PriGuarForward, StatementEndQ.PriGuarComma, "
						" StatementEndQ.PriInsFirst, StatementEndQ.PriInsMiddle, StatementEndQ.PriInsLast, StatementEndQ.PersonID, StatementEndQ.SecInsCo, StatementEndQ.SecGuarForward, StatementEndQ.SecGuarComma, StatementEndQ.SecInsfirst, StatementEndQ.SecInsMiddle, "
						" StatementEndQ.SecInsLast, StatementEndQ.PersID, "
						" StatementSubQ.LocationFixID AS LocID, StatementEndQ.RespID, StatementEndQ.RespFirst, StatementEndQ.RespMiddle, StatementEndQ.RespLast, StatementEndQ.RespAdd1, StatementEndQ.RespAdd2, StatementEndQ.RespCity, "
						" StatementEndQ.RespState, StatementEndQ.RespZip, "
						" (SELECT Max(Date) FROM MailSent INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
						"	WHERE (MailSentNotesT.Note Like '%%Patient Statement%%Printed%%' OR MailSentNotesT.Note Like '%%Patient Statement%%Run%%' OR MailSentNotesT.Note Like '%%E-Statement%%Exported%%') AND PersonID = StatementSubQ.PatID) AS LastSentDate, "
						" StatementSubQ.PatPhone,  "
						" StatementSubQ.FullChargeNoTax, StatementSubQ.ChargeTax1, StatementSubQ.ChargeTax2, StatementSubQ.TaxRate1, StatementSubQ.TaxRate2, "
						" (SELECT COUNT(*) FROM ResponsiblePartyT WHERE PatientID = StatementSubQ.PatID) AS RespPartyCount, "
						" (SELECT Top 1 Date FROM PaymentsT "
						"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
						"	LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
						"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
						"	WHERE LineItemT.DELETED = 0 AND PaymentsT.InsuredPartyID = -1 AND LineItemT.PatientID = StatementSubQ.PatID AND LineItemT.Type = 1 "
						"		AND OriginalLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL "
						"	ORDER BY LineItemT.InputDate DESC) AS LastPatientPaymentDate, "
						" (SELECT Top 1 Date FROM PaymentsT "
						"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
						"	LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
						"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
						"	LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
						"	LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
						"	WHERE LineItemT.DELETED = 0 AND PaymentsT.InsuredPartyID <> -1 AND LineItemT.PatientID = StatementSubQ.PatID AND LineItemT.Type = 1 "
						"		AND ChargebacksPayments.ID Is Null AND ChargebacksAdjustments.ID Is Null "
						"		AND OriginalLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL "
						"	ORDER BY LineItemT.InputDate DESC) AS LastInsurancePaymentDate, "
						" (SELECT Top 1 Amount FROM PaymentsT "
						"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
						"	LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
						"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
						"	WHERE LineItemT.DELETED = 0 AND PaymentsT.InsuredPartyID = -1 AND LineItemT.PatientID = StatementSubQ.PatID AND LineItemT.Type = 1 "
						"		AND OriginalLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL "
						"	ORDER BY LineItemT.InputDate DESC) AS LastPatientPaymentAmount, "
						" (SELECT Top 1 Amount FROM PaymentsT "
						"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
						"	LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
						"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
						"	LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
						"	LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
						"	WHERE LineItemT.DELETED = 0 AND PaymentsT.InsuredPartyID <> -1 AND LineItemT.PatientID = StatementSubQ.PatID AND LineItemT.Type = 1 "
						"		AND ChargebacksPayments.ID Is Null AND ChargebacksAdjustments.ID Is Null "
						"		AND OriginalLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL "
						"	ORDER BY LineItemT.InputDate DESC) AS LastInsurancePaymentAmount, "
						" StatementSubQ.CCType, CASE WHEN Len(StatementSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(StatementSubQ.CCNumber, 4) END as CCNumber, StatementSubQ.CheckNo, StatementSubQ.BankNo, StatementSubQ.CheckAcctNo, "
						" StatementSubQ.CCHoldersName, Convert(datetime, NULL) AS CCExpDate, StatementSubQ.CCAuthNo, StatementSubQ.BankRoutingNum, " 
						" StatementEndQ.PatCoordFirst, StatementEndQ.PatCoordMiddle, StatementEndQ.PatCoordLast, "
						" StatementSubQ.PercentOff, StatementSubQ.DiscountAmt, StatementSubQ.DiscountCategoryDesc, StatementEndQ.ProviderNPI as ProvNPI, StatementSubQ.CPTCode,  "
						" StatementSubQ.CPTModifier1, StatementSubQ.CPTModifier2, StatementSubQ.CPTModifier3, StatementSubQ.CPTModifier4, "
						" StatementSubQ.BillStatementNote, StatementSubQ.LineItemStatementNote, "
						" CASE WHEN StatementSubQ.BillStatementNote = '' OR StatementSubQ.BillStatementNote IS NULL THEN 1 ELSE 0 END as SuppressBillStatementNote, "
						" CASE WHEN StatementSubQ.LineItemStatementNote = '' OR StatementSubQ.LineItemStatementNote IS NULL  THEN 1 ELSE 0 END as SuppressLineItemStatementNote "

								"FROM "
								"/*Begin StatementSubQ*/ "
								"/*StatementSubQ brings it all together*/  "
								"(SELECT StatementAllData.*,  Min(NextApp.AppDate)as AppDate, Min(NextApp.StartTime) as StartTime, LineItemT.Date as ARDate, "
								" Age =   "
								"       CASE "
								"	  WHEN StatementAllData.BirthDate Is Null then "
								"		-1 "
								"	  ELSE  "
								"		DATEDIFF(YYYY, StatementAllData.Birthdate, GetDate()) - "
								"			CASE WHEN MONTH(StatementAllData.Birthdate) > MONTH(GetDate()) OR (MONTH(StatementAllData.Birthdate) = MONTH(GetDate()) AND DAY(StatementAllData.Birthdate) > DAY(GetDate())) "
								"				  THEN 1 ELSE 0 END "
								"       END, "
								"  CASE WHEN StatementAR.Thirty IS NULL THEN 0 ELSE StatementAR.Thirty END AS Thirty, "
								"  CASE WHEN StatementAR.Sixty IS NULL THEN 0 ELSE StatementAR.Sixty END AS Sixty, "
								"  CASE WHEN StatementAR.Ninety IS NULL THEN 0 ELSE StatementAR.Ninety END AS Ninety, "
								"  CASE WHEN StatementAR.NinetyPlus IS NULL THEN 0 ELSE StatementAR.NinetyPlus END AS NinetyPlus "
								"FROM "
								"/*Begin StatementAllData*/ "
								"/*StatementAllData is the big union query that unions all the separate queries together*/ "
								"/*the separate queries being Charges, Payments, Applied Charges and Payments*/ "
								"( "
								"/*Begin Union Charges*/ "
								"SELECT StmtCharges.ID, StmtCharges.PatientID, StmtCharges.PatID, StmtCharges.Type, StmtCharges.Total,  "
								"StmtCharges.Description, StmtCharges.Date, StmtCharges.Insurance, StmtCharges.Last, StmtCharges.First, StmtCharges.Middle, "
								"StmtCharges.Address1, StmtCharges.Address2, StmtCharges.City, StmtCharges.State, StmtCharges.Zip, StmtCharges. PatForward, "
								"StmtCharges.PatComma, StmtCharges.DocName, StmtCharges.DocAddress1, StmtCharges.DocAddress2, StmtCharges.DocCity, StmtCharges.DocState, StmtCharges.DocZip, StmtCharges.PracName, StmtCharges.PracAddress1, StmtCharges.PracAddress2, "
								"StmtCharges.PracCity, StmtCharges.PracState, StmtCharges.PracZip, StmtCharges.PracPhone, StmtCharges.PracFax, "
								"StmtCharges.ProvID, StmtCharges.BillID, StmtCharges.BillDate, StmtCharges.BillDescription, StmtCharges.BirthDate, \r\n "
								"StmtCharges.ICD9Code1, StmtCharges.ICD9Code2, StmtCharges.ICD9Code3, StmtCharges.ICD9Code4, \r\n "
								"StmtCharges.ICD10Code1, StmtCharges.ICD10Code2, StmtCharges.ICD10Code3, StmtCharges.ICD10Code4, \r\n "
								"StmtCharges.WhichCodes9, StmtCharges.WhichCodes10, StmtCharges.WhichCodesBoth, \r\n "
								"StmtCharges.Location, 1 as StatementType, -1 as GroupFixID, StmtCharges.LocationFixID, StmtCharges.ProvTaxID, StmtCharges.TransProv, StmtCharges.PrePayment,  "
								"StmtCharges.ProvEIN, StmtCharges.ProvLicense, StmtCharges.ProvUPIN, StmtCharges.ProvDEA, StmtCharges.ProvBCBS, StmtCharges.ProvMedicare, StmtCharges.ProvMedicaid, StmtCharges.ProvWorkersComp, StmtCharges.ProvOtherID, StmtCharges.ProvOtherIDDesc, StmtCharges.HomePhone AS PatPhone, "
								"StmtCharges.FullChargeNoTax, StmtCharges.ChargeTax1, StmtCharges.ChargeTax2, StmtCharges.TaxRate1, StmtCharges.TaxRate2, StmtCharges.Quantity, "
								" '' as CCType, '' AS CCNumber,'' AS CheckNo, '' AS BankNo,  '' AS CheckAcctNo, "
								" '' AS CCHoldersName, NULL AS CCExpDate, '' AS CCAuthNo, '' AS BankRoutingNum, " 
								"  StmtCharges.PercentOff, StmtCharges.DiscountAmt, StmtCharges.DiscountCategoryDesc, StmtCharges.CPTCode, "
								" StmtCharges.CPTModifier1, StmtCharges.CPTModifier2, StmtCharges.CPTModifier3, StmtCharges.CPTModifier4, "
								" StmtCharges.BillStatementNote, StmtCharges.LineItemStatementNote "
								"FROM ");
				   part2.Format("/*begin StmtCharges. StmtCharges gathers all the information you need for any charge*/ "
								"/*this query will give you the ChargeID, PatientID, UserDefinedID, Type, Generates a Total Charge,*/ "
								"/*date, insurance charge, first name, last name, middle name, address1, address2, city,*/ "
								"/*state, zip, phone, fax, provID, Doctor's name, billID, BillDate, BillDescription, Birthdate,*/ "
								"/*diagCodes, and whichCodes field*/  "
								"(SELECT  LineItemT.ID, PatientsT.UserDefinedID as PatientID, LineItemT.PatientID as PatID, LineItemT.Type, "
								"Total =  "
								"	CASE "
								"	   WHEN Sum(ChargeRespT.Amount) is Null then "
								"		0 "
								"	   ELSE  "
								"		Sum(ChargeRespT.Amount) "
								"	   END, Description = %s, "
								"LineItemT.Date,   "
								"Insurance =  "
								"	Sum(CASE  "
								"		WHEN ChargeRespT.InsuredPartyID is  not NULL AND ChargeRespT.InsuredPartyID <> -1  "
								"			then  ChargeRespT.Amount "
								"			else 0 "
								"	END), "
								"PersonT.Last, PersonT.Middle, PersonT.First, PersonT.Address1, PersonT.Address2, "
								"PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Name PracName, LocationsT.Address1 PracAddress1, LocationsT.Address2 as PracAddress2, "
								"LocationsT.City PracCity, LocationsT.State PracState, LocationsT.Zip as PracZip, LocationsT.Phone as PracPhone, "
								"LocationsT.Fax as PracFax,  "
								"ProvidersT.PersonID as ProvID, (%s WHERE PersonT.ID = PersonT1.ID) as DocName, "
								" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
								" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
								"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward, "
								"BillsT.ID as BillID, BillsT.Date as BillDate, BillsT.Description as BillDescription, PersonT.BirthDate, \r\n "
								" ICD9T1.CodeNumber AS ICD9Code1, \r\n "
								" ICD9T2.CodeNumber AS ICD9Code2, \r\n"
								" ICD9T3.CodeNumber AS ICD9Code3, \r\n "
								" ICD9T4.CodeNumber AS ICD9Code4, \r\n "
								" ICD10T1.CodeNumber AS ICD10Code1, \r\n "
								" ICD10T2.CodeNumber AS ICD10Code2, \r\n"
								" ICD10T3.CodeNumber AS ICD10Code3, \r\n "
								" ICD10T4.CodeNumber AS ICD10Code4, \r\n "
								" WhichCodesQ.WhichCodes9, \r\n"
								" WhichCodesQ.WhichCodes10, \r\n"
								" WhichCodesQ.WhichCodesBoth, \r\n"								
								" PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
								" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
								" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2)  AS FullChargeNoTax,   "
								" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate) - 1) AS ChargeTax1, "
								" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate2) - 1) AS ChargeTax2, "
								" Min(ChargesT.TaxRate) AS TaxRate1, Min(ChargesT.TaxRate2) AS TaxRate2, "
								" (%s WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 As Prepayment, ChargesT.Quantity,  "
								" TotalPercentOff as PercentOff, TotalDiscount as DiscountAmt, "
								" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDesc, CPTCodeT.Code as CPTCode, "
								" ChargesT.CPTModifier as CPTModifier1, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = ChargesT.ID) AS LineItemStatementNote "

								" FROM LineItemT LEFT OUTER JOIN "
								"    PatientsT LEFT OUTER JOIN "
								"    ProvidersT ON  ", strChargeDescription, strHeaderProv, strTransProv);
				   part3.Format("    PatientsT.MainPhysician = ProvidersT.PersonID LEFT OUTER JOIN "
								"    PersonT PersonT1 ON  "
								"    ProvidersT.PersonID = PersonT1.ID LEFT OUTER JOIN "
								"    LocationsT RIGHT OUTER JOIN "
								"    PersonT ON LocationsT.ID = PersonT.Location ON  "
								"    PatientsT.PersonID = PersonT.ID ON  "
								"    LineItemT.PatientID = PatientsT.PersonID LEFT OUTER JOIN "
								"    BillsT RIGHT OUTER JOIN "
								"    ServiceT LEFT OUTER JOIN "
								" CPTCodeT ON  "
								"    ServiceT.ID = CPTCodeT.ID RIGHT OUTER JOIN "
								"    ChargesT ON ServiceT.ID = ChargesT.ServiceID ON  "
								"    BillsT.ID = ChargesT.BillID LEFT OUTER JOIN "
								"    ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID ON  "
								"    LineItemT.ID = ChargesT.ID "
								"	LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewCharge ON ChargesT.ID = LineItemCorrectionsT_NewCharge.NewLineItemID \r\n"
								"	LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
							
								" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
								" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
											
								" LEFT JOIN  \r\n"
								" (SELECT ChargesT.ID as ChargeID, "
								" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
								" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
								" STUFF((SELECT ', ' +  \r\n "
								" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
									 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
									 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
								" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
								" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
								" FROM ChargesT \r\n "
								" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "
								
								" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
								" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
								"WHERE (PatientsT.PersonID = ? ) AND (%s >= ? ) AND (%s <=  ? ) AND (LineItemT.Deleted = 0) AND (BillsT.Deleted = 0) AND (LineItemT.Type = 10) AND "
								" BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL  "
								" %s %s "
								"GROUP BY LineItemT.ID, PatientsT.UserDefinedID, LineItemT.PatientID, LineItemT.Type, "
								"CPTCodeT.Code, LineItemT.Description, LineItemCorrectionsT_NewCharge.NewLineItemID, "
								"LineItemT.Date, "
								"PersonT.Last, PersonT.Middle, PersonT.First, PersonT.Address1, PersonT.Address2, "
								"PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Name, LocationsT.Address1, LocationsT.Address2, "
								"LocationsT.City, LocationsT.State, LocationsT.Zip, LocationsT.Phone, "
								"LocationsT.Fax, ProvidersT.PersonID, PersonT1.Last,PersonT1.First, PersonT1.Middle, PersonT1.Title, "
								"BillsT.ID, BillsT.Date, BillsT.Description, PersonT.BirthDate, \r\n"
								" ICD9T1.CodeNumber, \r\n "
								" ICD9T2.CodeNumber, \r\n "
								" ICD9T3.CodeNumber, \r\n "
								" ICD9T4.CodeNumber, \r\n "
								" ICD10T1.CodeNumber, \r\n "
								" ICD10T2.CodeNumber, \r\n "
								" ICD10T3.CodeNumber, \r\n "
								" ICD10T4.CodeNumber, \r\n "
								" WhichCodesQ.WhichCodes9, \r\n"
								" WhichCodesQ.WhichCodes10, \r\n"
								" WhichCodesQ.WhichCodesBoth, \r\n"
								" PersonT.Location, LineItemT.LocationID, [ProvidersT].[Fed Employer ID], ChargesT.DoctorsProviders, "
								" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
								"PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PersonT1.ID, ChargesT.Quantity, "
								" TotalPercentOff, TotalDiscount, ChargesT.ID, CPTCodeT.Code, LineItemCorrectionsT_NewCharge.NewLineItemID, ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4  "
								") as StmtCharges "
								"/*end StmtCharges*/ "
								"/*End Union Charges*/ "
								"UNION ", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT.Date", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT.Date", strLoc, strBillID);
				
				   part4.Format("/*Begin Union Payments*/ "
								"SELECT StmtPays.ID, StmtPays.PatientID, StmtPays.PatID, StmtPays.Type, StmtPays.UnAppliedAmount,  "
								"StmtPays.Description, StmtPays.Date, StmtPays.Insurance, StmtPays.Last, StmtPays.First, StmtPays.Middle, "
								"StmtPays.Address1, StmtPays.Address2, StmtPays.City, StmtPays.State, StmtPays.Zip, StmtPays.PatForward, "
								"StmtPays.PatComma, StmtPays.DocName, StmtPays.DocAddress1, StmtPays.DocAddress2, StmtPays.DocCity, StmtPays.DocState, StmtPays.DocZip, "
								"StmtPays.PracName, StmtPays.PracAddress1, StmtPays.PracAddress2, "
								"StmtPays.PracCity, StmtPays.PracState, StmtPays.PracZip, StmtPays.PracPhone, StmtPays.PracFax, "
								"StmtPays.ProvID, StmtPays.BillID, StmtPays.BillDate, StmtPays.BillDescription, StmtPays.BirthDate, \r\n"
								"StmtPays.ICD9Code1, StmtPays.ICD9Code2, StmtPays.ICD9Code3, StmtPays.ICD9Code4, \r\n"
								"StmtPays.ICD10Code1, StmtPays.ICD10Code2, StmtPays.ICD10Code3, StmtPays.ICD10Code4, \r\n"
								"StmtPays.WhichCodes9, StmtPays.WhichCodes10, StmtPays.WhichCodesBoth, \r\n "
								"StmtPays.Location, 2 as StatementType, -2 as GroupFixID, StmtPays.LocationFixID, StmtPays.ProvTaxID, StmtPays.TransProv, StmtPays.Prepayment, "
								"StmtPays.ProvEIN, StmtPays.ProvLicense, StmtPays.ProvUPIN, StmtPays.ProvDEA, StmtPays.ProvBCBS, StmtPays.ProvMedicare, StmtPays.ProvMedicaid, StmtPays.ProvWorkersComp, StmtPays.ProvOtherID, StmtPays.ProvOtherIDDesc, StmtPays.HomePhone AS PatPhone, "
								"  0 AS FullChargeNoTax, 0 AS ChargeTax1, 0 AS ChargeTax2, 0 AS TaxRate1, 0 As TaxRate2, 0 AS Quantity, "
								" StmtPays.CCType, StmtPays.CCNumber, StmtPays.CheckNo, StmtPays.BankNo, StmtPays.CheckAcctNo, "
								" StmtPays.CCHoldersName, StmtPays.CCExpDate, StmtPays.CCAuthNo, StmtPays.BankRoutingNum, " 
								" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, '' as CPTCode, '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, " 
								" StmtPays.BillStatementNote, "
								" StmtPays.LineItemStatementNote "
								"FROM ( "
								"/*StmtPays is the query that gets all the information you need for payments, this includes:*/ "
								"/*PaymentID, Insurance, Last Name, First Name, Middle Name, Address1, Address2,*/  "
								"SELECT LineItemT.ID, Insurance=   "
								"	CASE "
								"	   WHEN MIN([PaymentsT].[InsuredPartyID])>0 then "
								"		CASE "
								"		   WHEN [LineItemT].[Type]= 3 then "
								"			MIN([LineItemT].[Amount])-Sum( "
								"			CASE "
								"			   WHEN [AppliesT].[Amount] is NULL then 0 "
								"			   ELSE [AppliesT].[Amount] "
								"			END) "
								"		   ELSE Min([LineItemT].[Amount])-Sum( ");
				   part5.Format("			CASE  "
								"			    WHEN [AppliesT].[Amount] is NULL THEN 0 "
								"			    ELSE [AppliesT].[Amount] "
								"			END) "
								"		   END "
								"	   ELSE 0 "
								"	END, "
								"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, "
								"PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Address1 as PracAddress1, LocationsT.Address2 as PracAddress2, "
								"LocationsT.Name as PracName, LocationsT.City as PracCity, LocationsT.State as PracState, LocationsT.Zip as PracZip, "
								"LocationsT.Phone as PracPhone, LocationsT.Fax as PracFax, PatientsT.UserDefinedID as PatientID, PatientsT.PersonID as PatID, "
								"ProvidersT.PersonID as ProvID,  "
								" (%s WHERE PersonT.ID = PersonT1.ID) as DocName,  "
								" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
								" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
								"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, "
								"PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward, LineItemT.Type, "
								"UnAppliedAmount =  "
								"	CASE "
								"	   WHEN [LineItemT].[Type]=3 then "
								"		MIN([LineItemT].[Amount])-Sum( "
								"		CASE  "
								"		   WHEN [AppliesT].[Amount] is Null then 0 "
								"		   ELSE [AppliesT].[Amount] "
								"		END) "
								"	   ELSE Min([LineItemT].[Amount])-Sum( "
								"		CASE  "
								"		   WHEN [AppliesT].[Amount] is Null then 0 "
								"		   ELSE [AppliesT].[Amount] "
								"		END) "
								"	   END, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
								"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
								"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
								"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
								" ELSE LineItemT.Description END) AS Description,  "
								"LineItemT.Date, LineItemT.ID as BillID, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
								"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
								"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
								"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
								"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
								" LineItemT.Date as BillDate, PersonT.BirthDate, \r\n"
								" '' as ICD9Code1, '' as ICD9Code2, '' AS ICD9Code3, '' AS ICD9Code4, \r\n"
								" '' as ICD10Code1, '' as ICD10Code2, '' AS ICD10Code3, '' AS ICD10Code4, \r\n"
								" '' AS WhichCodes9, '' AS WhichCodes10, '' AS WhichCodesBoth,   \r\n"
								" PersonT.Location, LineItemT.LocationID AS LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
								" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
								"  (%s WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, PaymentsT.Prepayment As Prepayment, "
								" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
								" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
								" '' AS BillStatementNote, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
								, strHeaderProv, strTransProv);
				 part6.Format(  " FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
								" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
								" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
								" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
								" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
								" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID  "
								" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
								" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID  "
								" LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
								" LEFT JOIN LineItemCorrectionsT ON PaymentsT.ID = LineItemCorrectionsT.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT VoidT ON PaymentsT.ID = VoidT.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
								"WHERE (PatientsT.PersonID = ? ) AND (LineItemT.Date >= ? ) AND (LineItemT.Date <= ? ) AND (LineItemT.Type < 10) AND "
								"  LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND "
								" (LineItemT.Deleted = 0)  %s  %s  %s " 
									+ GetStatementChargebackString("ChargebacksPayments") + " " 
									+ GetStatementChargebackString("ChargebacksAdjustments") + " " 
								"GROUP BY LineItemT.ID,   "
								"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, "
								"PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Address1, LocationsT.Address2, "
								"LocationsT.Name, LocationsT.City, LocationsT.State, LocationsT.Zip, "
								"LocationsT.Phone, LocationsT.Fax, PatientsT.UserDefinedID, PatientsT.PersonID, "
								"ProvidersT.PersonID,  "
								"LineItemT.Type, LineItemT.Description, LineItemT.Date,  "
								"LineItemCorrectionsT_NewPay.NewLineItemID, "
								"PersonT1.Last, PersonT1.Middle, PersonT1.Title, PersonT1.First, PersonT.BirthDate, PersonT.Location, LineItemT.LocationID, [ProvidersT].[Fed Employer ID], PaymentsT.ProviderID, "
								"ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
								"PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PaymentsT.Prepayment, PersonT1.ID, "
								" CreditCardNamesT.CardName, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
								" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, PaymentsT.ID " 
								" HAVING ((Min(LineItemT.Amount) = 0 AND Min(AppliesT.Amount) IS NULL) OR (CASE WHEN "
								" [LineItemT].[Type] = 3 THEN "
								"  - 1 * MIN([LineItemT].[Amount])  "
								"  + SUM(CASE WHEN [AppliesT].[Amount] IS "
								"  NULL  THEN 0 ELSE [AppliesT].[Amount] END)  "
								" ELSE MIN([LineItemT].[Amount])  "
								" - SUM(CASE WHEN [AppliesT].[Amount] IS "
								" NULL  THEN 0 ELSE [AppliesT].[Amount] END ) "
								" END <> 0)) ) AS StmtPays "
								"UNION ", strLoc, strHideUnAppliedPrepayments, strNoUnAppPays);
				part7.Format(   "/* Begin StmtApplies, this is all the information about applied payments, it it a union query unioning payments applied to payments*/  "
								"/*and payments applied to charges*/ "
								"SELECT StmtApplies.ChargeID, StmtApplies.PatientID, StmtApplies.PatID, StmtApplies.Type, StmtApplies.ApplyAmount,  "
								"StmtApplies.Description, StmtApplies.Date, StmtApplies.Insurance, StmtApplies.Last, StmtApplies.First, StmtApplies.Middle, "
								"StmtApplies.Address1, StmtApplies.Address2, StmtApplies.City, StmtApplies.State, StmtApplies.Zip, StmtApplies.PatForward, "
								"StmtApplies.PatComma, StmtApplies.DocName, StmtApplies.DocAddress1, StmtApplies.DocAddress2, StmtApplies.DocCity, StmtApplies.DocState, StmtApplies.DocZip, "
								"StmtApplies.PracName, StmtApplies.PracAddress1, StmtApplies.PracAddress2, "
								"StmtApplies.PracCity, StmtApplies.PracState, StmtApplies.PracZip, StmtApplies.PracPhone, StmtApplies.PracFax, "
								"StmtApplies.ProvID, StmtApplies.BillID, StmtApplies.BillDate, StmtApplies.BillDescription, StmtApplies.BirthDate, \r\n"
								"StmtApplies.ICD9Code1, StmtApplies.ICD9Code2, StmtApplies.ICD9Code3, StmtApplies.ICD9Code4, \r\n"
								"StmtApplies.ICD10Code1, StmtApplies.ICD10Code2, StmtApplies.ICD10Code3, StmtApplies.ICD10Code4, \r\n"
								" StmtApplies.WhichCodes9, StmtApplies.WhichCodes10, StmtApplies.WhichCodesBoth, \r\n"
								" StmtApplies.Location, 3 as StatementType, StmtApplies.PaymentID as GroupFixID, StmtApplies.LocationFixID, StmtApplies.ProvTaxID, StmtApplies.TransProv, StmtApplies.PrePayment,  "
								"StmtApplies.ProvEIN, StmtApplies.ProvLicense, StmtApplies.ProvUPIN, StmtApplies.ProvDEA, StmtApplies.ProvBCBS, StmtApplies.ProvMedicare, StmtApplies.ProvMedicaid, StmtApplies.ProvWorkersComp, StmtApplies.ProvOtherID, StmtApplies.ProvOtherIDDesc, StmtApplies.HomePhone AS PatPhone, "
								"  0 AS FullChargeNoTax, 0 AS ChargeTax1, 0 AS ChargeTax2, 0 AS TaxRate1, 0 As TaxRate2, 0 As Quantity, "
								" StmtApplies.CCType, StmtApplies.CCNumber, StmtApplies.CheckNo, StmtApplies.BankNo, StmtApplies.CheckAcctNo, "
								" StmtApplies.CCHoldersName, StmtApplies.CCExpDate, StmtApplies.CCAuthNo, StmtApplies.BankRoutingNum, " 
								" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, '' as CPTCode, '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, "  
								" StmtApplies.BillStatementNote, StmtApplies.LineItemStatementNote "
								"FROM  "
								"/*Payments applied to charges*/ "
								"(SELECT * FROM (SELECT AppliesT.DestID as ChargeID, AppliesT.ID as PaymentID, PatientsT.UserDefinedID as PatientID, PatientsT.PersonID as PatID, LineItemT1.Type, "
								"AppliesT.Amount as ApplyAmount, LineItemT1.Date, LineItemT1.InputDate, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
								"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
								"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
								"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
								" ELSE LineItemT1.Description END) AS Description,  "
								"Insurance =  "
								"	CASE "
								"		WHEN [PaymentsT].[InsuredPartyID] > 0 then "
								"			[AppliesT].[Amount] "
								"		ELSE 0 "
								"	END, "
								"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.CIty, PersonT.State, PersonT.Zip, PersonT.HomePhone, "
								"LocationsT.Name as PracName, LocationsT.Address1 as PracAddress1, LocationsT.Address2 as PracAddress2, LocationsT.City as PracCity, "
								"LocationsT.State as PracState, LocationsT.Zip as PracZip, LocationsT.Phone as PracPhone, LocationsT.Fax as PracFax, "
								"PatientsT.SuppressStatement, ProvidersT.PersonID as ProvID, (%s WHERE PersonT.ID = PersonT1.ID) as DocName, "
								" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
								" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, ", strHeaderProv);
				 part8.Format(  "PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward,  "
								"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, "
								"BillsT.ID as BillID, BillsT.Date as BillDate, BillsT.Description as BillDescription, PersonT.BirthDate, \r\n "
								" ICD9T1.CodeNumber AS ICD9Code1, ICD9T2.CodeNumber AS ICD9Code2, ICD9T3.CodeNumber AS ICD9Code3, ICD9T4.CodeNumber AS ICD9Code4, \r\n"
								" ICD10T1.CodeNumber AS ICD10Code1, ICD10T2.CodeNumber AS ICD10Code2, ICD10T3.CodeNumber AS ICD10Code3, ICD10T4.CodeNumber AS ICD10Code4, \r\n"
								" WhichCodesQ.WhichCodes9, WhichCodesQ.WhichCodes10, WhichCodesQ.WhichCodesBoth, \r\n"
								" PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
								" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
								" (%s WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 As Prepayment,  "
								" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
								" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
								/*"FROM PersonT LEFT OUTER JOIN "
								"    LocationsT ON  "
								"    PersonT.Location = LocationsT.ID RIGHT OUTER JOIN "
								"    LineItemT LineItemT1 RIGHT OUTER JOIN "
								"    PaymentsT ON  "
								"    LineItemT1.ID = PaymentsT.ID "
								"    LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID " 
								"    RIGHT OUTER JOIN "
								"    AppliesT ON  "
								"    PaymentsT.ID = AppliesT.SourceID LEFT OUTER JOIN "
								"    LineItemT RIGHT OUTER JOIN "
								"    ChargesT LEFT OUTER JOIN "
								"    BillsT LEFT OUTER JOIN "
								"    PatientsT ON BillsT.PatientID = PatientsT.PersonID ON  "
								"    ChargesT.BillID = BillsT.ID ON LineItemT.ID = ChargesT.ID ON  "
								"    AppliesT.DestID = ChargesT.ID ON  "
								"    PersonT.ID = PatientsT.PersonID LEFT OUTER JOIN "
								"    PersonT PersonT1 RIGHT OUTER JOIN "
								"    ProvidersT ON PersonT1.ID = ProvidersT.PersonID ON  "
								"    PatientsT.MainPhysician = ProvidersT.PersonID "
								" LEFT JOIN DiagCodes DiagCodes1 ON BillsT.Diag1ID = DiagCodes1.ID LEFT JOIN DiagCodes DiagCodes2 ON BillsT.Diag2ID = DiagCodes2.ID LEFT JOIN DiagCodes DiagCodes3 ON BillsT.Diag3ID = DiagCodes3.ID LEFT JOIN DiagCodes DiagCodes4 ON BillsT.Diag4ID = DiagCodes4.ID "
								*/
								" FROM LineItemT LineItemT1 LEFT JOIN PaymentsT ON LineItemT1.ID = PaymentsT.ID "
								" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
								" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
								" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
								" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
								" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
								" LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
								" LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
								" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
								" LEFT JOIN PatientsT ON LineItemT1.PatientID = PatientsT.PersonID "
								" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
								" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
								" LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID "
								" LEFT JOIN PersonT PersonT1 on ProvidersT.PersonID = PersonT1.ID "

								" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
								" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
											
								" LEFT JOIN  \r\n"
								" (SELECT ChargesT.ID as ChargeID, "
								" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
								" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
								" STUFF((SELECT ', ' +  \r\n "
								" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
									 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
									 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
								" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
								" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
								" FROM ChargesT \r\n "
								" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "
								
								" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
								" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON PaymentsT.ID = LineItemCorrections2T.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT Void2T ON PaymentsT.ID = Void2T.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
								"WHERE (PatientsT.PersonID = ? ) AND (%s >= ? ) AND (%s <= ? ) AND (LineItemT1.Deleted = 0) AND (LineItemT.Deleted = 0) AND (BillsT.Deleted = 0) "
								" AND BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL "
								" AND (AppliesT.PointsToPayments = 0) %s %s " 
									+ GetStatementChargebackString("ChargebacksPayments") + " "
									+ GetStatementChargebackString("ChargebacksAdjustments") + " "
								") AS StatementDataAppliesCharges "
								"UNION "
								"/*Payments applied to payments*/ "
								"SELECT * FROM (SELECT AppliesT.DestID AS ChargeID,  AppliesT.ID as PaymentID, "
								"    PatientsT.UserDefinedID AS PatientID,  "
								"    PatientsT.PersonID AS PatID, LineItemT1.Type,  "
								"    AppliesT.Amount AS ApplyAmount, LineItemT1.Date,  "
								"    LineItemT1.InputDate, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
								"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
								"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
								"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
								" ELSE LineItemT1.Description END) AS Description,  "
								"    CASE WHEN (PaymentsT.InsuredPartyID) > 0 THEN AppliesT.Amount ELSE 0 END AS Insurance, PersonT.Last, PersonT.First,  "
								"    PersonT.Middle, PersonT.Address1, PersonT.Address2,  "
								"    PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, "
								"    LocationsT.Name AS PracName,  "
								"    LocationsT.Address1 AS PracAddress1,  ", strTransProv, m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT1.Date", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT1.Date", strLoc, strBillID);
				   part9.Format("    LocationsT.Address2 AS PracAddress2,  "
								"    LocationsT.City AS PracCity, LocationsT.State AS PracState,  "
								"    LocationsT.Zip AS PracZip, LocationsT.Phone AS PracPhone,  "
								"    LocationsT.Fax AS PracFax, PatientsT.SuppressStatement,  "
								"    ProvidersT.PersonID AS ProvID,  "
								"    (%s WHERE PersonT.ID = PersonT1.ID) AS DocName, "
								" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
								" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
								"     PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS PatForward, "
								"     PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatComma, "
								" LineItemT.ID as BillID, LineItemT.Date as BillDate, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
								"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
								"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
								"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
								"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
								" PersonT.BirthDate, \r\n"
								" '' as ICD9Code1, '' as ICD9Code2, '' as ICD9Code3, '' as ICD9Code4, \r\n"
								" '' as ICD10Code1, '' as ICD10Code2, '' as ICD10Code3, '' as ICD10Code4, \r\n"
								" '' as WhichCodes9, '' as WhichCodes10, '' as WhichCodesBoth,   \r\n"
								" PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
								" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
								" (%s WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, 0 As PrePayment,  "
								" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
								" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
								" '' AS BillStatementNote, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
								" FROM PersonT PersonT1 RIGHT OUTER JOIN "
								"    ProvidersT ON  "
								"    PersonT1.ID = ProvidersT.PersonID RIGHT OUTER JOIN "
								"    PersonT LEFT OUTER JOIN "
								"    LocationsT ON  "
								"    PersonT.Location = LocationsT.ID RIGHT OUTER JOIN "
								"    LineItemT LineItemT1 LEFT OUTER JOIN "
								"    PatientsT ON LineItemT1.PatientID = PatientsT.PersonID ON  "
								"    PersonT.ID = PatientsT.PersonID ON  "
								"    ProvidersT.PersonID = PatientsT.MainPhysician RIGHT OUTER JOIN "
								"    PaymentsT ON  "
								"    LineItemT1.ID = PaymentsT.ID "
								"   LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
								"   LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
								"	LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID "
								"	LEFT JOIN ChargebacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID "
								"   RIGHT OUTER JOIN "
								"    LineItemT RIGHT OUTER JOIN "
								"    AppliesT ON LineItemT.ID = AppliesT.DestID ON  "
								"    PaymentsT.ID = AppliesT.SourceID "
								"	LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID "
								"	LEFT JOIN ChargebacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID "
								" LEFT JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT VoidT ON LineItemT.ID = VoidT.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON LineItemT1.ID = LineItemCorrections2T.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT Void2T ON LineItemT1.ID = Void2T.VoidingLineItemID "		
								" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
								"WHERE (PatientsT.PersonID = ? ) AND (%s >= ? ) AND (%s <= ? ) "
								" AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL "
								" AND (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 1) %s %s "
								+ GetStatementChargebackString("ChargebacksPayments1") + " " 
								+ GetStatementChargebackString("ChargebacksAdjustments1") + " "
								+ GetStatementChargebackString("ChargebacksPayments") + " " 
								+ GetStatementChargebackString("ChargebacksAdjustments") + " "
								" ) AS StatementDataAppliesPays) AS StmtApplies) as StatementAllData "
								"/*this ends StmtApplies and StatementAllData*/ "
								"LEFT JOIN ", strHeaderProv, strTransProv, 
								m_pReport->nDateFilter == 2 ? "LineItemT.Date" : "LineItemT1.Date",
								m_pReport->nDateFilter == 2 ? "LineItemT.Date" : "LineItemT1.Date",
								strLoc, strNoUnAppPays);
				  part10.Format("/* this is a query that returns the time and date of the next appointment for the patient and the patientID so that it cam be linked to the main query*/ "
								"(SELECT AppointmentsT.Date AS AppDate, AppointmentsT.StartTime, AppointmentsT.PatientID as PatID "
								"FROM AppointmentsT  "
								"WHERE (AppointmentsT.Date > GetDate()) AND (AppointmentsT.PatientID = ? ) AND (AppointmentsT.Status <> 4)) AS NextApp ON NextApp.PatID = StatementAllData.PatID LEFT JOIN LineItemT on LineItemT.ID = StatementAllData.ID "
								"LEFT JOIN "
								"  (SELECT CASE WHEN Thirty IS NULL THEN 0 ELSE Thirty END AS Thirty, CASE WHEN Sixty IS NULL THEN 0 ELSE SIXTY END AS Sixty, CASE WHEN Ninety IS NULL THEN 0 ELSE NINETY END AS Ninety, "
								"  CASE WHEN NINETYPLUS  IS NULL THEN 0 ELSE NinetyPlus END AS NinetyPlus, PatientID "
								"  FROM( "
								"  SELECT PatAR.PatientID, Sum(PatAR.Thirty) AS Thirty, Sum(PatAR.Sixty) AS Sixty, Sum(PatAR.Ninety) AS Ninety, Sum(PatAR.NinetyPlus) AS NinetyPlus "
								"  FROM "
								"  (SELECT Sum((CASE WHEN Thirty.ChargeAmount IS NULL THEN 0 ELSE Thirty.ChargeAmount END) - (CASE WHEN Thirty.PPAmount IS NULL THEN 0 ELSE Thirty.PPAmount END)) AS Thirty, 0 AS Sixty, 0 As Ninety, 0 AS NinetyPlus, PatientID  "
								"  FROM  "
								"  ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID FROM   "
								"  (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID FROM ChargeRespDetailT LEFT JOIN "
								"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
								"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd,-30, getDate()))  AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (PatientID = ? ) "
								"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID) As Charges  "
								"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
								"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID "
								"   GROUP BY PatientID) "
								"   UNION  "
								"   (SELECT 0 AS ChargeAmount, SUM(CASE WHEN LineItemT.Type = 3 THEN (LineItemT.Amount) ELSE LineItemT.Amount END - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID  "
								"   FROM  "
								"   LineItemT LEFT JOIN   "
								"   (SELECT DestID, Sum(Amount) as Amount  "
								"   FROM AppliesT  "
								"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
								"   WHERE (InsuredPartyID = -1)  "
								" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID "
								"   LEFT JOIN "
								" 	(SELECT SourceID, Sum(Amount) AS Amount  "
								"   FROM AppliesT  "
								"  	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
								"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd,-30, getDate())) AND (PaymentsT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) %s "
								"   GROUP BY LineItemT.PatientID)) AS Thirty   "
								"   GROUP BY Thirty.PatientID  "
								"   UNION   "
								"   SELECT 0 AS Thirty, Sum((CASE WHEN Sixty.ChargeAmount IS NULL THEN 0 ELSE Sixty.ChargeAmount END) - (CASE WHEN Sixty.PPAmount IS NULL THEN 0 ELSE Sixty.PPAmount END)) AS Sixty, 0 AS Ninety, 0 AS NinetyPlus, PatientID   "
								"   FROM  "
								"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID FROM   "
								"  	(SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID FROM ChargeRespDetailT LEFT JOIN  "
								"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
								" 	LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
								" 	WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date <= DATEADD(dd, -30, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) "
								" 	GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID) As Charges  "
								"   LEFT JOIN   "
								" 	(SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
								" 	ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays  "
								"   ON Charges.DetailID = Pays.DetailID "
								"   GROUP BY PatientID) "
								"   UNION  "
								"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID "
								"   FROM  "
								"   LineItemT LEFT JOIN  "
								"   (SELECT DestID, Sum(Amount) as Amount  "
								"   	FROM AppliesT   "
								" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
								"  	WHERE (InsuredPartyID = -1)  "
								" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
								"   LEFT JOIN "
								" 	(SELECT SourceID, Sum(Amount) AS Amount  "
								"  	FROM AppliesT  "
								" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
								"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd, -60, getDate())) AND (LineItemT.Date <= DATEADD(dd, -30, getDate())) AND (PaymentsT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) %s "
								"   GROUP BY LineItemT.PatientID)) AS Sixty  "
								"   GROUP BY Sixty.PatientID "
								"   UNION  "
								"   SELECT 0 AS Thirty, 0 AS Sixty, Sum((CASE WHEN Ninety.ChargeAmount IS NULL THEN 0 ELSE Ninety.ChargeAmount END) - (CASE WHEN Ninety.PPAmount IS NULL THEN 0 ELSE Ninety.PPAmount END)) AS Ninety, 0 AS NinetyPlus, PatientID  "
								"   FROM "
								"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID FROM  "
								"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID FROM ChargeRespDetailT LEFT JOIN "
								"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID "
								"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date >= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) "
								"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID) As Charges "
								"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
								"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID "
								"   GROUP BY PatientID) "
								"   UNION  "
								"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID "
								"   FROM  "
								"   LineItemT LEFT JOIN  "
								"  	(SELECT DestID, Sum(Amount) as Amount  "
								"  	FROM AppliesT  "
								"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
								"  	WHERE (InsuredPartyID = -1) "
								"  	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID "
								"   LEFT JOIN "
								"  	(SELECT SourceID, Sum(Amount) AS Amount "
								" 	FROM AppliesT "
								" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID "
								"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd, -60, getDate())) AND (LineItemT.Date >= DATEADD(dd, -90, getDate())) AND (PaymentsT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? )  %s "
								"   GROUP BY LineItemT.PatientID)) AS Ninety "
								"   GROUP BY Ninety.PatientID "
								"    UNION   "
								"   SELECT 0 AS Thirty, 0 AS Sixty, 0 AS Ninety, Sum((CASE WHEN NinetyPlus.ChargeAmount IS NULL THEN 0 ELSE NinetyPlus.ChargeAmount END) - (CASE WHEN NinetyPlus.PPAmount IS NULL THEN 0 ELSE NinetyPlus.PPAmount END)) AS NinetyPlus, PatientID  "
								"   FROM  "
								"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID FROM  "
								"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID FROM ChargeRespDetailT LEFT JOIN "
								"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
								"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (LineItemT.PatientID =  ? ) "
								"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID) As Charges  "
								"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM "
								"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID  "
								"   GROUP BY PatientID)  "
								"    UNION   "
								"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID  "
								"   FROM  "
								"   LineItemT LEFT JOIN  "
								"  	(SELECT DestID, Sum(Amount) as Amount "
								" 	FROM AppliesT  "
								" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
								"  	WHERE (InsuredPartyID = -1)  "
								" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
								"    LEFT JOIN  "
								" 	(SELECT SourceID, Sum(Amount) AS Amount  "
								" 	FROM AppliesT  "
								" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID "
								"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd,-90, getDate())) AND (PaymentsT.InsuredPartyID = -1) AND (LineItemT.PatientID = ? ) %s "
								"   GROUP BY LineItemT.PatientID)) AS NinetyPlus "
								"   GROUP BY NinetyPlus.PatientID) PatAR  "
								"   GROUP BY PatAR.PatientID) AS PatientAR) AS StatementAR ON StatementAllData.PatID = StatementAR.PatientID "
								"Group By StatementAllData.ID, StatementAllData.PatientId, StatementAllData.PatID, StatementAllData.Type, StatementAllData.Total, "
								"StatementAllData.Date, StatementAllData.Description, StatementAllData.Insurance, StatementAllData.Last, "
								"StatementAllData.First, StatementAllData.Middle, StatementAllData.Address1, StatementAllData.Address2, StatementAllData.City, "
								"StatementAllData.State, StatementAllData.Zip, StatementAllData.PatForward, StatementAllData.PatComma, StatementAllData.DocName, "
								"StatementAllData.DocAddress1, StatementAllData.DocAddress2, StatementAllData.DocCity, StatementAllData.DocState, StatementAllData.DocZip, "
								"StatementAllData.PracName, StatementAllData.PracAddress1, StatementAllData.PracAddress2, StatementAllData.PracCity, StatementAllData.PracState, "
								"StatementAllData.PracZip, StatementAllData.PracPhone, StatementAllData.PracFax, StatementAllData.BillId, StatementAllData.BillDate, "
								"StatementAllData.BillDescription, StatementAllData.Birthdate, \r\n"
								"StatementAllData.ICD9Code1, StatementAllData.ICD9Code2, StatementAllData.ICD9Code3, StatementAllData.ICD9Code4, \r\n"
								"StatementAllData.ICD10Code1, StatementAllData.ICD10Code2, StatementAllData.ICD10Code3, StatementAllData.ICD10Code4, \r\n"
								"StatementAllData.WhichCodes9, StatementAllData.WhichCodes10, StatementAllData.WhichCodesBoth, \r\n"
								"StatementAllData.DocAddress1, StatementAllData.DocAddress2, StatementAllData.DocCity, StatementAllData.DocState, StatementAllData.DocZip, "
								"StatementAllData.ProvId, StatementAllData.Location, StatementAllData.Date, LineItemT.Date, StatementAllData.StatementType, StatementAllData.GroupFixID, StatementAllData.LocationFixID, StatementAllData.ProvTaxID, StatementAllData.TransProv, StatementAllData.PrePayment, "
								"StatementAllData.ProvEIN, StatementAllData.ProvLicense, StatementAllData.ProvUPIN, StatementAllData.ProvDEA, StatementAllData.ProvBCBS, StatementAllData.ProvMedicare, StatementAllData.ProvMedicaid, StatementAllData.ProvWorkersComp, StatementAllData.ProvOtherID, StatementAllData.ProvOtherIDDesc, StatementAR.Thirty, StatementAR.Sixty, StatementAR.Ninety, StatementAR.NinetyPlus, StatementAllData.PatPhone, "
								" StatementAllData.FullChargeNoTax, StatementAllData.ChargeTax1, StatementAllData.ChargeTax2, StatementAllData.TaxRate1, StatementAllData.TaxRate2, StatementAllData.Quantity, "
								"  StatementAllData.CCType, StatementAllData.CCNumber, StatementAllData.CheckNo, StatementAllData.BankNo, StatementAllData.CheckAcctNo, "
								" StatementAllData.CCHoldersName, StatementAllData.CCExpDate, StatementAllData.CCAuthNo, StatementAllData.BankRoutingNum, StatementAllData.PercentOff, StatementAllData.DiscountAmt, StatementAllData.DiscountCategoryDesc, StatementAllData.CPTCode, StatementAllData.CPTModifier1, StatementAllData.CPTModifier2, StatementAllData.CPTModifier3, StatementAllData.CPTModifier4, StatementAllData.BillStatementNote, StatementAllData.LineItemStatementNote  ) as StatementSubQ "
								"LEFT OUTER JOIN  "
								"/*this query returns all the patient information that the statement needs for a patient.  Eventually the patient demographics in the above queries*/ "
								"/*needs to be phased out and all of the information can come from this query*/ "
								"/*Begin StatementEndQ*/ "
								"(Select *  "
								"FROM "
								"/*this is a patient info query*/ "
								"(SELECT PersonT1.First + ' ' + PersonT1.Middle + ' ' + PersonT1.Last + ' ' + PersonT1.Title "
								"     AS DocName, PersonT1.ID as ProvID, PatientsT.SuppressStatement,  "
								"    PatientsT.PersonID AS PatID, PatientsT.PrimaryRespPartyID AS PrimaryRespID, PatientsT.StatementNote,  "
								"  PatCoordT.First as PatCoordFirst, PatCoordT.Middle as PatCoordMiddle, PatCoordT.Last as PatCoordLast, ProvidersT.NPI as ProviderNPI "
								"FROM PatientsT LEFT OUTER JOIN "
								"    PersonT PersonT1 ON  "
								"    PatientsT.MainPhysician = PersonT1.ID LEFT OUTER JOIN "
								"    PersonT ON PatientsT.PersonID = PersonT.ID "
								"    LEFT JOIN ProvidersT ON PersonT1.ID = ProvidersT.PersonID "
								"    LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
								"WHERE (PatientsT.PersonID = ? ))  "
								"AS PatInfo  "
								"/*end PatInfo*/ "
								"LEFT OUTER JOIN "
								"/*this query returns primary insurance information*/ "
								"(SELECT InsuranceCoT.Name as PriInsCo, "
								"	PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS "
								"     PriGuarForward,  "
								"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PriGuarComma, ", strHideUnAppliedPrepayments, strHideUnAppliedPrepayments, strHideUnAppliedPrepayments, strHideUnAppliedPrepayments);
				part11.Format(  "     PersonT.First AS PriInsFirst,  "
								"    PersonT.Middle AS PriInsMiddle,  "
								"    PersonT.Last AS PriInsLast, PatientsT.PersonID as PersonID "
								"FROM InsuranceCoT RIGHT OUTER JOIN "
								"    PersonT RIGHT OUTER JOIN "
								"    InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID ON  "
								"    InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID RIGHT "
								"     OUTER JOIN "
								"    PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
								"WHERE (PatientsT.PersonID = ? ) AND (InsuredPartyT.RespTypeID = 1)) AS  PriInsInfo  "
								"ON PatInfo.PatID = PriInsInfo.PersonID "
								"/*End PriInsInfo*/ "
								"LEFT Outer JOIN "
								"/*This query returns secondary insurance information*/ "
								"(SELECT InsuranceCoT.Name as SecInsCo, "
								"	PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS "
								"     SecGuarForward,  "
								"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS SecGuarComma, "
								"     PersonT.First AS SecInsFirst,  "
								"    PersonT.Middle AS SecInsMiddle,  "
								"    PersonT.Last AS SecInsLast, PatientsT.PersonID as PersID "
								"FROM InsuranceCoT RIGHT OUTER JOIN "
								"    PersonT RIGHT OUTER JOIN "
								"    InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID ON  "
								"    InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID RIGHT "
								"     OUTER JOIN "
								"    PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
								" WHERE (PatientsT.PersonID = ? ) AND (InsuredPartyT.RespTypeID = 2)) AS SecInsInfo "
								" /*end SecInsInfo*/ "
								" ON PatInfo.PatID = SecInsInfo.PersID  "
								" Left Outer Join "
								" (SELECT ResponsiblePartyT.PersonID as RespID, ResponsiblePartyT.PatientID AS RespPatID, First as RespFirst, Middle as RespMiddle, Last as RespLast, Address1 as RespAdd1, Address2 as RespAdd2,  "
								" City as RespCity, State as RespState, Zip as RespZip "
								" FROM PersonT Inner Join ResponsiblePartyT ON PersonT.ID = ResponsiblePartyT.PersonID LEFT JOIN PatientsT ON ResponsiblePartyT.PatientID = PatientsT.PersonID @RespFilter) AS ResPartyT  "
								" ON PatInfo.PatID = ResPartyT.RespPatID) AS StatementEndQ ON StatementSubQ.PatID = StatementEndQ.PatID "
								"/*end StatementEndQ*/");

						// (a.walling 2006-10-24 13:12) - PLID 16059 - Multiple Responsible parties
						part11.Replace("@RespFilter", strRespFilter);

						return CComplexReportQuery(part1 + part2 + part3 + part4 + part5 + part6 + part7 + part8 + part9 + part10 + part11);
						 }

					 case 338: //Individual Patient Statement By Location
					 case 356:
						 return CComplexReportQuery(GetStatementByLocationSql(strRespFilter));
					 break;

		

					 default:
						 return CComplexReportQuery(GetMoreStatementSql());
					 break;
				}
		  //end switch nSubLevel 0
		  break;

		  case 1: {

			  //we only have 1 subreport, so we won't switch again here
			  switch (m_pReport->nID) {

				  //batch reports
				  case 169:	
				  case 337:
				  case 353:
				  case 355:
				  case 434:
				  case 435:
				  case 436:
				  case 437:
				  case 483: 
				  case 485:
					  {

					CString strTmpTable;
					if (!m_pReport->GetFilter(m_nSubLevel, m_nSubRepNum).IsEmpty())  {
						strTmpTable = CreateStatementFilterTable();						

						//if we got here and have no tmp table, that means we got an error in the last function
						if (strTmpTable.IsEmpty()) {
							return CComplexReportQuery();
						}
					}
				  
			  
					CString str;
					str.Format("SELECT ID, ChargeID, BillID, PercentOff, Discount, PatientID, DiscountCategoryDescription FROM ( "
					"SELECT ID, ChargeID, (SELECT PatientID FROM LineItemT WHERE ID = ChargeID) as PatientID, (SELECT BillID FROM ChargesT WHERE ID = ChargeDiscountsT.ChargeID) AS BillID, PercentOff, Discount, CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE "
					" CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE "
					" (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription "
					" FROM ChargeDiscountsT WHERE DELETED = 0 ) Q "
					" WHERE PatientID IN (SELECT ID FROM %s)", strTmpTable);
							
					return CComplexReportQuery(str);
					  }
					break;


					//individuals
				  case 234:
				  case 338:
				  case 354:
				  case 356:
				  case 484:
				  case 486:
					  {

					  CString str;
					  str.Format("SELECT ID, ChargeID, BillID, PercentOff, Discount, PatientID, DiscountCategoryDescription FROM ( "
						  "SELECT ID, ChargeID, (SELECT PatientID FROM LineItemT WHERE ID = ChargeID) as PatientID, (SELECT BillID FROM ChargesT WHERE ID = ChargeDiscountsT.ChargeID) AS BillID, PercentOff, Discount, CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE "
							" CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE "
							" (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription "
							" FROM ChargeDiscountsT WHERE DELETED = 0 ) Q "
							" WHERE PatientID = ?");
					  return CComplexReportQuery(str);
					  }
					break;

				  default:
					  return CComplexReportQuery();
				  break;
				}
			}
		  break;

		  default:
			  return CComplexReportQuery();
		  break;
	  }
}



CString CStatementSqlBuilder::GetMoreStatementSql()
{
		// (a.walling 2006-10-24 13:12) - PLID 16059 - Multiple Resp. Parties.
		//		Filter on PersonT.ID (show multiple) or Primary (No multiple, default)
		CString strRespFilter;
		long nMultiple = GetRemotePropertyInt("StatementsPrintMultipleParties", 0, 0, "<None>", true);
		if ( (nMultiple == 0) || (m_bIsEStatement) ) { // (a.walling 2007-01-15 09:29) - PLID 7069 - Don't use multiple parties when using e-statements
			strRespFilter = " WHERE ResponsiblePartyT.PersonID = PatientsT.PrimaryRespPartyID ";
		}
		else {
			strRespFilter = "";
		}

		// (j.gruber 2007-01-08 16:49) - PLID 17606 - add last payment date and amount to all statement queries
		switch (m_pReport->nID) {

			case 484:
			case 486: {
				 CString strChargeProv, strPayProv;
				
				if (m_pReport->nLocation == -1) {
								strChargeProv = " ";
								strPayProv = " ";
				}
				else {
					strChargeProv.Format("AND (ChargesT.DoctorsProviders = %li)", m_pReport->nProvider);
					strPayProv.Format("AND (PaymentsT.ProviderID = %li)", m_pReport->nProvider);
				}

				CString strBillID, strNoUnAppPays;
				if (m_pReport->nExtraID == -1) {
					strBillID = " ";
					strNoUnAppPays = " ";
						 
				}
				else {
					strBillID.Format(" AND (BillsT.ID = %li) ", m_pReport->nExtraID);
					strNoUnAppPays = " AND (1=0) ";

				}

				// (j.gruber 2008-07-02 16:47) - PLID 29553 - added last payment info by provider
				CString strTempTableName;
				strTempTableName = GetLastProviderPaymentInformationSql("", m_pReport->nPatient);


				long nTransFormat, nHeaderFormat;
				nTransFormat = GetRemotePropertyInt("SttmntTransProvFormat", 0, 0, "<None>");
				nHeaderFormat = GetRemotePropertyInt("SttmntHeaderProvFormat", 0, 0, "<None>");
				CString strTransProv = GetStatementProvString(nTransFormat);
				CString strHeaderProv = GetStatementProvString(nHeaderFormat);
				CString strHideUnAppliedPrepayments = GetStatementUnAppliedPrePaymentsString();
				CString strChargeDescription = GetStatementChargeDescription();
				CString strLoc;
							


				// (j.gruber 2007-05-01 17:14) - PLID 25745 - only show the last 4 digits of the cc number
				// (j.gruber 2007-05-15 09:08) - PLID 25987 - take out credit card expiration dates
				// (j.gruber 2007-05-29 17:21) - PLID 26096 - change the joins of the top part of the applies query
				// (j.gruber 2007-06-29 09:37) - PLID 26501 - parameterized the individual statement
				// (e.lally 2007-07-13) PLID 26670 - Updated all references to PaymentPlansT. CCType with link to CardName, aliased as CCType where applicable.
				// (j.gruber 2008-07-03 16:15) - PLID 29533 - made the last payment information show per provider taking into account applied information
				// (j.gruber 2008-07-03 15:29) - PLID 27499 - corrected the date filter on the payments applied to charges and payment applied to payments queries
				// (j.jones 2008-09-05 10:23) - PLID 30288 - supported MailSentNotesT
				// (j.gruber 2009-11-05 17:38) - PLID 36217 added provider NPI
				// (j.gruber 2009-11-25 12:27) - PLID 36430 - add CPT Code
				// (j.gruber 2009-12-24 12:52) - PLID 17122 - added CPTModifiers
				// (j.gruber 2010-02-19 09:46) - PLID 37447 - re-aliased the providerNPI to provNPI
				// (j.gruber 2010-06-14 15:50) - PLID 36484 - added billing notes
				// (j.gruber 2011-07-05 10:39) - PLID 44831 - take out originals and voids
				// (a.wilson 2012-2-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
				// (j.gruber 2014-03-04 16:07) - PLID 61167 - updated for ICD10 and Diag and whichcodes refactor
				//TES 7/17/2014 - PLID 62935 - Added code to hide chargebacks when requested
				// (j.jones 2015-03-09 09:48) - PLID 64937 - if the description begins with 'Corrected Charge',
				// 'Corrected Payment', etc., strip that off
				// (c.haag 2016-02-11) - PLID 68236 - Split single Chargebacks joins into two joins to avoid producing super slow query plans
				CString part1, part2, part3, part4, part5, part6, part7, part8, part9, part10a, part10ab, part10b, part10bc, part10bcd, part10c, part11;
						part1.Format(" SELECT StatementSubQ.ID, StatementSubQ.PatientID, StatementSubQ.PatID as PatID, StatementSubQ.Type, StatementSubQ.Total, StatementSubQ.Description, StatementSubQ.Date as TDate,  "
						" StatementSubQ.Insurance, StatementSubQ.Last, StatementSubQ.First, StatementSubQ.Middle, StatementSubQ.Address1, StatementSubQ.Address2, "
						" StatementSubQ.City, StatementSubQ.State, StatementSubQ.Zip, StatementSubQ.PatForward, StatementSubQ.PatComma, StatementSubQ.DocName, StatementSubQ.DocAddress1, StatementSubQ.DocAddress2, StatementSubQ.DocCity, StatementSubQ.DocState, StatementSubQ.DocZip, StatementSubQ.ProvTaxID, StatementSubQ.PracName, StatementSubQ.PracAddress1, "
						" StatementSubQ.PracAddress2, StatementSubQ.PracCity, StatementSubQ.PracState, StatementSubQ.PracZip, StatementSubQ.PracPhone, StatementSubQ.PracFax, StatementSubQ.ProvID as ProvID2, StatementSubQ.BillId, "
						" StatementSubQ.BillDate, StatementSubQ.BillDescription, StatementSubQ.Birthdate, \r\n"
						" StatementSubQ.ICD9Code1, StatementSubQ.ICD9Code2, StatementSubQ.ICD9Code3, StatementSubQ.ICD9Code4, \r\n"
						" StatementSubQ.ICD10Code1, StatementSubQ.ICD10Code2, StatementSubQ.ICD10Code3, StatementSubQ.ICD10Code4, \r\n"
						" StatementSubQ.WhichCodes9, StatementSubQ.WhichCodes10, StatementSubQ.WhichCodesBoth, "
						" StatementSubQ.Location, StatementSubQ.StatementType, StatementSubQ.GroupFixID, StatementSubQ.Appdate, StatementSubQ.StartTime, StatementSubQ.ARDate, StatementSubQ.Age,  "
						" StatementSubQ.TransProv, StatementSubQ.PrePayment, StatementSubQ.Quantity, StatementSubQ.Thirty, StatementSubQ.Sixty, StatementSubQ.Ninety, StatementSubQ.NinetyPlus, "
						" StatementSubQ.ProvEIN, StatementSubQ.ProvLicense, StatementSubQ.ProvUPIN, StatementSubQ.ProvDEA, StatementSubQ.ProvBCBS, StatementSubQ.ProvMedicare, StatementSubQ.ProvMedicaid, StatementSubQ.ProvWorkersComp, StatementSubQ.ProvOtherID, StatementSubQ.ProvOtherIDDesc, "
						" StatementEndQ.DocName as DocName2, StatementSubQ.TransProvID as ProvID, StatementEndQ.SuppressStatement, StatementEndQ.PatID as PatID2, StatementEndQ.StatementNote, StatementEndQ.PriInsCo, StatementEndQ.PriGuarForward, StatementEndQ.PriGuarComma, "
						" StatementEndQ.PriInsFirst, StatementEndQ.PriInsMiddle, StatementEndQ.PriInsLast, StatementEndQ.PersonID, StatementEndQ.SecInsCo, StatementEndQ.SecGuarForward, StatementEndQ.SecGuarComma, StatementEndQ.SecInsfirst, StatementEndQ.SecInsMiddle, "
						" StatementEndQ.SecInsLast, StatementEndQ.PersID, "
						" StatementSubQ.LocationFixID AS LocID, StatementEndQ.RespID, StatementEndQ.RespFirst, StatementEndQ.RespMiddle, StatementEndQ.RespLast, StatementEndQ.RespAdd1, StatementEndQ.RespAdd2, StatementEndQ.RespCity, "
						" StatementEndQ.RespState, StatementEndQ.RespZip, "
						" (SELECT Max(Date) FROM MailSent INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
						"	WHERE (MailSentNotesT.Note Like '%%Patient Statement%%Printed%%' OR MailSentNotesT.Note Like '%%Patient Statement%%Run%%' OR MailSentNotesT.Note Like '%%E-Statement%%Exported%%') AND PersonID = StatementSubQ.PatID) AS LastSentDate, "
						" StatementSubQ.PatPhone,  "
						" StatementSubQ.FullChargeNoTax, StatementSubQ.ChargeTax1, StatementSubQ.ChargeTax2, StatementSubQ.TaxRate1, StatementSubQ.TaxRate2, "
						" StatementSubQ.TransProvAdd1, StatementSubQ.TransProvAdd2, StatementSubQ.TransProvCity, StatementSubQ.TransProvState, StatementSubQ.TransProvZip, StatementSubQ.TransProvFirst, StatementSubQ.TransProvMiddle, StatementSubQ.TransProvLast, StatementSubQ.TransProvID, "
						" (SELECT COUNT(*) FROM ResponsiblePartyT WHERE PatientID = StatementSubQ.PatID) AS RespPartyCount, "
						" StatementLastPatPayInfoQ.LastPayDate as LastPatientPaymentDate, StatementLastInsPayInfoQ.LastPayDate as LastInsurancePaymentDate, "
						" StatementLastPatPayInfoQ.LastPayAmt as LastPatientPaymentAmount, StatementLastInsPayInfoQ.LastPayAmt as LastInsurancePaymentAmount, "
						" StatementSubQ.CCType, CASE WHEN Len(StatementSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(StatementSubQ.CCNumber, 4) END as CCNumber, StatementSubQ.CheckNo, StatementSubQ.BankNo, StatementSubQ.CheckAcctNo, "
						" StatementSubQ.CCHoldersName, Convert(datetime, NULL)  AS CCExpDate, StatementSubQ.CCAuthNo, StatementSubQ.BankRoutingNum, " 
						" StatementEndQ.PatCoordFirst, StatementEndQ.PatCoordMiddle, StatementEndQ.PatCoordLast, "
						" StatementSubQ.PercentOff, StatementSubQ.DiscountAmt, StatementSubQ.DiscountCategoryDesc, StatementSubQ.TransProviderNPI as ProvNPI, StatementSubQ.CPTCode, "
						" StatementSubQ.CPTModifier1, StatementSubQ.CPTModifier2, StatementSubQ.CPTModifier3, StatementSubQ.CPTModifier4, "
						" StatementSubQ.BillStatementNote, StatementSubQ.LineItemStatementNote, "
						" CASE WHEN StatementSubQ.BillStatementNote = '' OR StatementSubQ.BillStatementNote IS NULL THEN 1 ELSE 0 END as SuppressBillStatementNote, "
						" CASE WHEN StatementSubQ.LineItemStatementNote = '' OR StatementSubQ.LineItemStatementNote IS NULL  THEN 1 ELSE 0 END as SuppressLineItemStatementNote "

								"FROM "
								"/*Begin StatementSubQ*/ "
								"/*StatementSubQ brings it all together*/  "
								"(SELECT StatementAllData.*,  Min(NextApp.AppDate)as AppDate, Min(NextApp.StartTime) as StartTime, LineItemT.Date as ARDate, "
								" Age =   "
								"       CASE "
								"	  WHEN StatementAllData.BirthDate Is Null then "
								"		-1 "
								"	  ELSE  "
								"		DATEDIFF(YYYY, StatementAllData.Birthdate, GetDate()) - "
								"			CASE WHEN MONTH(StatementAllData.Birthdate) > MONTH(GetDate()) OR (MONTH(StatementAllData.Birthdate) = MONTH(GetDate()) AND DAY(StatementAllData.Birthdate) > DAY(GetDate())) "
								"				THEN 1 ELSE 0 END "
								"       END, "
								"  CASE WHEN StatementAR.Thirty IS NULL THEN 0 ELSE StatementAR.Thirty END AS Thirty, "
								"  CASE WHEN StatementAR.Sixty IS NULL THEN 0 ELSE StatementAR.Sixty END AS Sixty, "
								"  CASE WHEN StatementAR.Ninety IS NULL THEN 0 ELSE StatementAR.Ninety END AS Ninety, "
								"  CASE WHEN StatementAR.NinetyPlus IS NULL THEN 0 ELSE StatementAR.NinetyPlus END AS NinetyPlus "
								"FROM "
								"/*Begin StatementAllData*/ "
								"/*StatementAllData is the big union query that unions all the separate queries together*/ "
								"/*the separate queries being Charges, Payments, Applied Charges and Payments*/ "
								"( "
								"/*Begin Union Charges*/ "
								"SELECT StmtCharges.ID, StmtCharges.PatientID, StmtCharges.PatID, StmtCharges.Type, StmtCharges.Total,  "
								"StmtCharges.Description, StmtCharges.Date, StmtCharges.Insurance, StmtCharges.Last, StmtCharges.First, StmtCharges.Middle, "
								"StmtCharges.Address1, StmtCharges.Address2, StmtCharges.City, StmtCharges.State, StmtCharges.Zip, StmtCharges. PatForward, "
								"StmtCharges.PatComma, StmtCharges.DocName, StmtCharges.DocAddress1, StmtCharges.DocAddress2, StmtCharges.DocCity, StmtCharges.DocState, StmtCharges.DocZip, StmtCharges.PracName, StmtCharges.PracAddress1, StmtCharges.PracAddress2, "
								"StmtCharges.PracCity, StmtCharges.PracState, StmtCharges.PracZip, StmtCharges.PracPhone, StmtCharges.PracFax, "
								"StmtCharges.ProvID, StmtCharges.BillID, StmtCharges.BillDate, StmtCharges.BillDescription, StmtCharges.BirthDate, \r\n"
								"StmtCharges.ICD9Code1, StmtCharges.ICD9Code2, StmtCharges.ICD9Code3, StmtCharges.ICD9Code4, \r\n"
								"StmtCharges.ICD10Code1, StmtCharges.ICD10Code2, StmtCharges.ICD10Code3, StmtCharges.ICD10Code4, \r\n"
								"StmtCharges.WhichCodes9, StmtCharges.WhichCodes10, StmtCharges.WhichCodesBoth, \r\n"
								"StmtCharges.Location, 1 as StatementType, -1 as GroupFixID, StmtCharges.LocationFixID, StmtCharges.ProvTaxID, StmtCharges.TransProv, StmtCharges.PrePayment,  "
								"StmtCharges.ProvEIN, StmtCharges.ProvLicense, StmtCharges.ProvUPIN, StmtCharges.ProvDEA, StmtCharges.ProvBCBS, StmtCharges.ProvMedicare, StmtCharges.ProvMedicaid, StmtCharges.ProvWorkersComp, StmtCharges.ProvOtherID, StmtCharges.ProvOtherIDDesc, StmtCharges.HomePhone AS PatPhone, "
								"StmtCharges.FullChargeNoTax, StmtCharges.ChargeTax1, StmtCharges.ChargeTax2, StmtCharges.TaxRate1, StmtCharges.TaxRate2, "
								"StmtCharges.TransProvAdd1, StmtCharges.TransProvAdd2, StmtCharges.TransProvCity, StmtCharges.TransProvState, StmtCharges.TransProvZip, StmtCharges.TransProvFirst, StmtCharges.TransProvMiddle, StmtCharges.TransProvLast, StmtCharges.TransProvID, StmtCharges.Quantity, "
								" '' as CCType, '' AS CCNumber,'' AS CheckNo, '' AS BankNo,  '' AS CheckAcctNo, "
								" '' AS CCHoldersName, NULL AS CCExpDate, '' AS CCAuthNo, '' AS BankRoutingNum, " 
								"  StmtCharges.PercentOff, StmtCharges.DiscountAmt, StmtCharges.DiscountCategoryDesc, StmtCharges.TransProviderNPI, StmtCharges.CPTCode, "
								" StmtCharges.CPTModifier1, StmtCharges.CPTModifier2, StmtCharges.CPTModifier3, StmtCharges.CPTModifier4, "
								" StmtCharges.BillStatementNote, StmtCharges.LineItemStatementNote "
								"FROM ");
				   part2.Format("/*begin StmtCharges. StmtCharges gathers all the information you need for any charge*/ "
								"/*this query will give you the ChargeID, PatientID, UserDefinedID, Type, Generates a Total Charge,*/ "
								"/*date, insurance charge, first name, last name, middle name, address1, address2, city,*/ "
								"/*state, zip, phone, fax, provID, Doctor's name, billID, BillDate, BillDescription, Birthdate,*/ "
								"/*diagCodes, and whichCodes field*/  "
								"(SELECT  LineItemT.ID, PatientsT.UserDefinedID as PatientID, LineItemT.PatientID as PatID, LineItemT.Type, "
								"Total =  "
								"	CASE "
								"	   WHEN Sum(ChargeRespT.Amount) is Null then "
								"		0 "
								"	   ELSE  "
								"		Sum(ChargeRespT.Amount) "
								"	   END, Description = %s, "
								"LineItemT.Date,   "
								"Insurance =  "
								"	Sum(CASE  "
								"		WHEN ChargeRespT.InsuredPartyID is  not NULL AND ChargeRespT.InsuredPartyID <> -1  "
								"			then  ChargeRespT.Amount "
								"			else 0 "
								"	END), "
								"PersonT.Last, PersonT.Middle, PersonT.First, PersonT.Address1, PersonT.Address2, "
								"PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Name PracName, LocationsT.Address1 PracAddress1, LocationsT.Address2 as PracAddress2, "
								"LocationsT.City PracCity, LocationsT.State PracState, LocationsT.Zip as PracZip, LocationsT.Phone as PracPhone, "
								"LocationsT.Fax as PracFax,  "
								"ProvidersT.PersonID as ProvID, (%s WHERE PersonT.ID = TransProvPersonT.ID) as DocName, "
								" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
								" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
								"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward, "
								"BillsT.ID as BillID, BillsT.Date as BillDate, BillsT.Description as BillDescription, PersonT.BirthDate, \r\n"
								"ICD9T1.CodeNumber AS ICD9Code1, ICD9T2.CodeNumber AS ICD9Code2, ICD9T3.CodeNumber AS ICD9Code3, ICD9T4.CodeNumber AS ICD9Code4, \r\n"
								"ICD10T1.CodeNumber AS ICD10Code1, ICD10T2.CodeNumber AS ICD10Code2, ICD10T3.CodeNumber AS ICD10Code3, ICD10T4.CodeNumber AS ICD10Code4, \r\n"
								"WhichCodesQ.WhichCodes9, WhichCodesQ.WhichCodes10, WhichCodesQ.WhichCodesBoth, \r\n"
								"PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
								" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
								" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2)  AS FullChargeNoTax,   "
								" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate) - 1) AS ChargeTax1, "
								" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 0 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate2) - 1) AS ChargeTax2, "
								" Min(ChargesT.TaxRate) AS TaxRate1, Min(ChargesT.TaxRate2) AS TaxRate2, "
								" (%s WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 As Prepayment,  "
								"TransProvPersonT.First AS TransProvFirst, TransProvPersonT.Middle AS TransProvMiddle, TransProvPersonT.Last As TransProvLast, TransProvPersonT.Address1 As TransProvAdd1, TransProvPersonT.Address2 As TransProvAdd2, TransProvPersonT.City As TransProvCity, TransProvPersonT.State As TransProvState, TransProvPersonT.Zip AS TransProvZip, TransProvPersonT.ID AS TransProvID, ChargesT.Quantity, "
								" TotalPercentOff as PercentOff, TotalDiscount as DiscountAmt, "
								" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDesc, "
								" TransProvidersT.NPI as TransProviderNPI, CPTCodeT.Code as CPTCode, ChargesT.CPTModifier as CPTModifier1, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = ChargesT.ID) AS LineItemStatementNote "

								" FROM LineItemT LEFT OUTER JOIN "
								"    PatientsT LEFT OUTER JOIN "
								"    ProvidersT ON  ", strChargeDescription, strHeaderProv, strTransProv);
				   part3.Format("    PatientsT.MainPhysician = ProvidersT.PersonID LEFT OUTER JOIN "
								"    PersonT PersonT1 ON  "
								"    ProvidersT.PersonID = PersonT1.ID LEFT OUTER JOIN "
								"    LocationsT RIGHT OUTER JOIN "
								"    PersonT ON LocationsT.ID = PersonT.Location ON  "
								"    PatientsT.PersonID = PersonT.ID ON  "
								"    LineItemT.PatientID = PatientsT.PersonID LEFT OUTER JOIN "
								"    BillsT RIGHT OUTER JOIN "
								"    ServiceT LEFT OUTER JOIN "
								" CPTCodeT ON  "
								"    ServiceT.ID = CPTCodeT.ID RIGHT OUTER JOIN "
								"    ChargesT ON ServiceT.ID = ChargesT.ServiceID ON  "
								"    BillsT.ID = ChargesT.BillID LEFT OUTER JOIN "
								"    ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID ON  "
								"    LineItemT.ID = ChargesT.ID "
								"	LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewCharge ON ChargesT.ID = LineItemCorrectionsT_NewCharge.NewLineItemID \r\n"
								"	LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID \r\n"
							
								" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
								" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
											
								" LEFT JOIN  \r\n"
								" (SELECT ChargesT.ID as ChargeID, "
								" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
								" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
								" STUFF((SELECT ', ' +  \r\n "
								" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
									 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
									 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
								" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
								" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
								" FROM ChargesT \r\n "
								" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "
								
								" LEFT JOIN PersonT TransProvPersonT ON ChargesT.DoctorsProviders = TransProvPersonT.ID "
								" LEFT JOIN ProvidersT TransProvidersT ON TransProvPersonT.ID = TransProvidersT.PersonID "
								" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
								" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
								"WHERE (PatientsT.PersonID =  ? ) AND (%s >= ? ) AND (%s <=  ? ) AND (LineItemT.Deleted = 0) "
								" AND BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL "
								" AND (BillsT.Deleted = 0) AND (LineItemT.Type = 10) %s %s "
								"GROUP BY LineItemT.ID, PatientsT.UserDefinedID, LineItemT.PatientID, LineItemT.Type, "
								"CPTCodeT.Code, LineItemT.Description, LineItemCorrectionsT_NewCharge.NewLineItemID, "
								"LineItemT.Date,  "
								"PersonT.Last, PersonT.Middle, PersonT.First, PersonT.Address1, PersonT.Address2, "
								"PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Name, LocationsT.Address1, LocationsT.Address2, "
								"LocationsT.City, LocationsT.State, LocationsT.Zip, LocationsT.Phone, "
								"LocationsT.Fax, ProvidersT.PersonID, PersonT1.Last,PersonT1.First, PersonT1.Middle, PersonT1.Title, "
								"BillsT.ID, BillsT.Date, BillsT.Description, PersonT.BirthDate, \r\n"
								"ICD9T1.CodeNumber, ICD9T2.CodeNumber, ICD9T3.CodeNumber, ICD9T4.CodeNumber, \r\n"
								"ICD10T1.CodeNumber, ICD10T2.CodeNumber, ICD10T3.CodeNumber, ICD10T4.CodeNumber, \r\n"
								"WhichCodesQ.WhichCodes9, WhichCodesQ.WhichCodes10, WhichCodesQ.WhichCodesBoth, \r\n"
								"PersonT.Location, LineItemT.LocationID, [ProvidersT].[Fed Employer ID], ChargesT.DoctorsProviders, "
								" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
								"PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PersonT1.ID, "
								"TransProvPersonT.First, TransProvPersonT.Middle, TransProvPersonT.Last, TransProvPersonT.Address1, TransProvPersonT.Address2, TransProvPersonT.City, TransProvPersonT.State, TransProvPersonT.Zip, TransProvPersonT.ID , ChargesT.Quantity, TotalPercentOff, TotalDiscount, ChargesT.ID, TransProvidersT.NPI, CPTCodeT.Code, LineItemCorrectionsT_NewCharge.NewLineItemID, "
								" ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4 "
								") as StmtCharges "
								"/*end StmtCharges*/ "
								"/*End Union Charges*/ "
								"UNION ", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT.Date", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT.Date", strLoc, strBillID);
				
				   part4.Format("/*Begin Union Payments*/ "
								"SELECT StmtPays.ID, StmtPays.PatientID, StmtPays.PatID, StmtPays.Type, StmtPays.UnAppliedAmount,  "
								"StmtPays.Description, StmtPays.Date, StmtPays.Insurance, StmtPays.Last, StmtPays.First, StmtPays.Middle, "
								"StmtPays.Address1, StmtPays.Address2, StmtPays.City, StmtPays.State, StmtPays.Zip, StmtPays.PatForward, "
								"StmtPays.PatComma, StmtPays.DocName, StmtPays.DocAddress1, StmtPays.DocAddress2, StmtPays.DocCity, StmtPays.DocState, StmtPays.DocZip, "
								"StmtPays.PracName, StmtPays.PracAddress1, StmtPays.PracAddress2, "
								"StmtPays.PracCity, StmtPays.PracState, StmtPays.PracZip, StmtPays.PracPhone, StmtPays.PracFax, "
								"StmtPays.ProvID, StmtPays.BillID, StmtPays.BillDate, StmtPays.BillDescription, StmtPays.BirthDate, \r\n"
								"StmtPays.ICD9Code1, StmtPays.ICD9Code2, StmtPays.ICD9Code3, StmtPays.ICD9Code4, \r\n"
								"StmtPays.ICD10Code1, StmtPays.ICD10Code2, StmtPays.ICD10Code3, StmtPays.ICD10Code4, \r\n"
								"StmtPays.WhichCodes9, StmtPays.WhichCodes10, StmtPays.WhichCodesBoth, \r\n"
								"StmtPays.Location, 2 as StatementType, -2 as GroupFixID, StmtPays.LocationFixID, StmtPays.ProvTaxID, StmtPays.TransProv, StmtPays.Prepayment, "
								"StmtPays.ProvEIN, StmtPays.ProvLicense, StmtPays.ProvUPIN, StmtPays.ProvDEA, StmtPays.ProvBCBS, StmtPays.ProvMedicare, StmtPays.ProvMedicaid, StmtPays.ProvWorkersComp, StmtPays.ProvOtherID, StmtPays.ProvOtherIDDesc, StmtPays.HomePhone AS PatPhone, "
								"  0 AS FullChargeNoTax, 0 AS ChargeTax1, 0 AS ChargeTax2, 0 AS TaxRate1, 0 As TaxRate2, "
								"StmtPays.TransProvAdd1, StmtPays.TransProvAdd2, StmtPays.TransProvCity, StmtPays.TransProvState, StmtPays.TransProvZip, StmtPays.TransProvFirst, StmtPays.TransProvMiddle, StmtPays.TransProvLast, StmtPays.TransProvID, 0 as Quantity, "
								" StmtPays.CCType, StmtPays.CCNumber, StmtPays.CheckNo, StmtPays.BankNo, StmtPays.CheckAcctNo, "
								" StmtPays.CCHoldersName, StmtPays.CCExpDate, StmtPays.CCAuthNo, StmtPays.BankRoutingNum, " 
								" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, StmtPays.TransProviderNPI, '' as CPTCode, "
								" '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, " 
								" StmtPays.BillStatementNote, "
								" StmtPays.LineItemStatementNote "
								"FROM ( "
								"/*StmtPays is the query that gets all the information you need for payments, this includes:*/ "
								"/*PaymentID, Insurance, Last Name, First Name, Middle Name, Address1, Address2,*/  "
								"SELECT LineItemT.ID, Insurance=   "
								"	CASE "
								"	   WHEN MIN([PaymentsT].[InsuredPartyID])>0 then "
								"		CASE "
								"		   WHEN [LineItemT].[Type]= 3 then "
								"			MIN([LineItemT].[Amount])-Sum( "
								"			CASE "
								"			   WHEN [AppliesT].[Amount] is NULL then 0 "
								"			   ELSE [AppliesT].[Amount] "
								"			END) "
								"		   ELSE Min([LineItemT].[Amount])-Sum( ");
				   part5.Format("			CASE  "
								"			    WHEN [AppliesT].[Amount] is NULL THEN 0 "
								"			    ELSE [AppliesT].[Amount] "
								"			END) "
								"		   END "
								"	   ELSE 0 "
								"	END, "
								"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, "
								"PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Address1 as PracAddress1, LocationsT.Address2 as PracAddress2, "
								"LocationsT.Name as PracName, LocationsT.City as PracCity, LocationsT.State as PracState, LocationsT.Zip as PracZip, "
								"LocationsT.Phone as PracPhone, LocationsT.Fax as PracFax, PatientsT.UserDefinedID as PatientID, PatientsT.PersonID as PatID, "
								"ProvidersT.PersonID as ProvID,  "
								" (%s WHERE PersonT.ID = TransProvPersonT.ID) as DocName,  "
								" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
								" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
								"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, "
								"PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward, LineItemT.Type, "
								"UnAppliedAmount =  "
								"	CASE "
								"	   WHEN [LineItemT].[Type]=3 then "
								"		MIN([LineItemT].[Amount])-Sum( "
								"		CASE  "
								"		   WHEN [AppliesT].[Amount] is Null then 0 "
								"		   ELSE [AppliesT].[Amount] "
								"		END) "
								"	   ELSE Min([LineItemT].[Amount])-Sum( "
								"		CASE  "
								"		   WHEN [AppliesT].[Amount] is Null then 0 "
								"		   ELSE [AppliesT].[Amount] "
								"		END) "
								"	   END, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
								"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
								"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
								"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
								" ELSE LineItemT.Description END) AS Description,  "
								"LineItemT.Date, LineItemT.ID as BillID, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
								"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
								"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
								"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
								"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
								"LineItemT.Date as BillDate, PersonT.BirthDate, \r\n"
								"'' as ICD9Code1, '' as ICD9Code2, '' AS ICD9Code3, '' AS ICD9Code4, \r\n"
								"'' as ICD10Code1, '' as ICD10Code2, '' AS ICD10Code3, '' AS ICD10Code4, \r\n"
								"'' AS WhichCodes9, '' AS WhichCodes10, '' AS WhichCodesBoth, \r\n"
								"PersonT.Location, LineItemT.LocationID AS LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
								" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
								"  (%s WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, PaymentsT.Prepayment As Prepayment, TransProvPersonT.First AS TransProvFirst, TransProvPersonT.Middle AS TransProvMiddle, TransProvPersonT.Last As TransProvLast, TransProvPersonT.Address1 As TransProvAdd1, TransProvPersonT.Address2 As TransProvAdd2, TransProvPersonT.City As TransProvCity, TransProvPersonT.State As TransProvState, TransProvPersonT.Zip AS TransProvZip, TransProvPersonT.ID AS TransProvID, "
								" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
								" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum,  " 
								" TransProvidersT.NPI as TransProviderNPI, "
								" '' AS BillStatementNote, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
								, strHeaderProv, strTransProv);
				 part6.Format(  " FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
								" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
								" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
								" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
								" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
								" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID  "
								" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
								" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID  "
								" LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
								"LEFT JOIN PersonT TransProvPersonT ON PaymentsT.ProviderID = TransProvPersonT.ID "
								" LEFT JOIN ProvidersT TransProvidersT ON TransProvPersonT.ID = TransProvidersT.PersonID "
								" LEFT JOIN LineItemCorrectionsT ON PaymentsT.ID = LineItemCorrectionsT.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT VoidT ON PaymentsT.ID = VoidT.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
								"WHERE (PatientsT.PersonID = ? ) AND (LineItemT.Date >= ? ) AND (LineItemT.Date <= ? ) "
								"  AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL "
								" AND (LineItemT.Type < 10) AND (LineItemT.Deleted = 0)  %s  %s %s " 
								+ GetStatementChargebackString("ChargebacksPayments") + " "
								+ GetStatementChargebackString("ChargebacksAdjustments") + " "
								"GROUP BY LineItemT.ID,   "
								"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, "
								"PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Address1, LocationsT.Address2, "
								"LocationsT.Name, LocationsT.City, LocationsT.State, LocationsT.Zip, "
								"LocationsT.Phone, LocationsT.Fax, PatientsT.UserDefinedID, PatientsT.PersonID, "
								"ProvidersT.PersonID,  "
								"LineItemT.Type, LineItemT.Description, LineItemT.Date,  "
								" LineItemCorrectionsT_NewPay.NewLineItemID, "
								" PersonT1.Last, PersonT1.Middle, PersonT1.Title, PersonT1.First, PersonT.BirthDate, PersonT.Location, LineItemT.LocationID, [ProvidersT].[Fed Employer ID], PaymentsT.ProviderID, "
								" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
								"PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PaymentsT.Prepayment, PersonT1.ID, "
								" TransProvPersonT.First, TransProvPersonT.Middle, TransProvPersonT.Last, TransProvPersonT.Address1, TransProvPersonT.Address2, TransProvPersonT.City, TransProvPersonT.State, TransProvPersonT.Zip, TransProvPersonT.ID,  "
								" CreditCardNamesT.CardName, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
								" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, TransProvidersT.NPI, PaymentsT.ID " 
								" HAVING ((Min(LineItemT.Amount) = 0 AND Min(AppliesT.Amount) IS NULL) OR (CASE WHEN "
								" [LineItemT].[Type] = 3 THEN "
								"  - 1 * MIN([LineItemT].[Amount])  "
								"  + SUM(CASE WHEN [AppliesT].[Amount] IS "
								"  NULL  THEN 0 ELSE [AppliesT].[Amount] END)  "
								" ELSE MIN([LineItemT].[Amount])  "
								" - SUM(CASE WHEN [AppliesT].[Amount] IS "
								" NULL  THEN 0 ELSE [AppliesT].[Amount] END ) "
								" END <> 0)) ) AS StmtPays "
								"UNION ", strLoc, strHideUnAppliedPrepayments, strNoUnAppPays);
				part7.Format(   "/* Begin StmtApplies, this is all the information about applied payments, it it a union query unioning payments applied to payments*/  "
								"/*and payments applied to charges*/ "
								"SELECT StmtApplies.ChargeID, StmtApplies.PatientID, StmtApplies.PatID, StmtApplies.Type, StmtApplies.ApplyAmount,  "
								"StmtApplies.Description, StmtApplies.Date, StmtApplies.Insurance, StmtApplies.Last, StmtApplies.First, StmtApplies.Middle, "
								"StmtApplies.Address1, StmtApplies.Address2, StmtApplies.City, StmtApplies.State, StmtApplies.Zip, StmtApplies.PatForward, "
								"StmtApplies.PatComma, StmtApplies.DocName, StmtApplies.DocAddress1, StmtApplies.DocAddress2, StmtApplies.DocCity, StmtApplies.DocState, StmtApplies.DocZip, "
								"StmtApplies.PracName, StmtApplies.PracAddress1, StmtApplies.PracAddress2, "
								"StmtApplies.PracCity, StmtApplies.PracState, StmtApplies.PracZip, StmtApplies.PracPhone, StmtApplies.PracFax, "
								"StmtApplies.ProvID, StmtApplies.BillID, StmtApplies.BillDate, StmtApplies.BillDescription, StmtApplies.BirthDate, \r\n"
								"StmtApplies.ICD9Code1, StmtApplies.ICD9Code2, StmtApplies.ICD9Code3, StmtApplies.ICD9Code4, \r\n"
								"StmtApplies.ICD10Code1, StmtApplies.ICD10Code2, StmtApplies.ICD10Code3, StmtApplies.ICD10Code4, \r\n"
								"StmtApplies.WhichCodes9, StmtApplies.WhichCodes10, StmtApplies.WhichCodesBoth, \r\n"
								"StmtApplies.Location, 3 as StatementType, StmtApplies.PaymentID as GroupFixID, StmtApplies.LocationFixID, StmtApplies.ProvTaxID, StmtApplies.TransProv, StmtApplies.PrePayment,  "
								"StmtApplies.ProvEIN, StmtApplies.ProvLicense, StmtApplies.ProvUPIN, StmtApplies.ProvDEA, StmtApplies.ProvBCBS, StmtApplies.ProvMedicare, StmtApplies.ProvMedicaid, StmtApplies.ProvWorkersComp, StmtApplies.ProvOtherID, StmtApplies.ProvOtherIDDesc, StmtApplies.HomePhone AS PatPhone, "
								"  0 AS FullChargeNoTax, 0 AS ChargeTax1, 0 AS ChargeTax2, 0 AS TaxRate1, 0 As TaxRate2, "
								"StmtApplies.TransProvAdd1, StmtApplies.TransProvAdd2, StmtApplies.TransProvCity, StmtApplies.TransProvState, StmtApplies.TransProvZip, StmtApplies.TransProvFirst, StmtApplies.TransProvMiddle, StmtApplies.TransProvLast, StmtApplies.TransProvID, 0 As Quantity, "
								" StmtApplies.CCType, StmtApplies.CCNumber, StmtApplies.CheckNo, StmtApplies.BankNo, StmtApplies.CheckAcctNo, "
								" StmtApplies.CCHoldersName, StmtApplies.CCExpDate, StmtApplies.CCAuthNo, StmtApplies.BankRoutingNum, " 
								" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, StmtApplies.TransProviderNPI, '' as CPTCode, " 
								" '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, " 
								" StmtApplies.BillStatementNote, StmtApplies.LineItemStatementNote "
								"FROM  "
								"/*Payments applied to charges*/ "
								"(SELECT * FROM (SELECT AppliesT.DestID as ChargeID, AppliesT.ID as PaymentID, PatientsT.UserDefinedID as PatientID, PatientsT.PersonID as PatID, LineItemT1.Type, "
								"AppliesT.Amount as ApplyAmount, LineItemT1.Date, LineItemT1.InputDate, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
								"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
								"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
								"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
								" ELSE LineItemT1.Description END) AS Description,  "
								"Insurance =  "
								"	CASE "
								"		WHEN [PaymentsT].[InsuredPartyID] > 0 then "
								"			[AppliesT].[Amount] "
								"		ELSE 0 "
								"	END, "
								"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.CIty, PersonT.State, PersonT.Zip, PersonT.HomePhone, "
								"LocationsT.Name as PracName, LocationsT.Address1 as PracAddress1, LocationsT.Address2 as PracAddress2, LocationsT.City as PracCity, "
								"LocationsT.State as PracState, LocationsT.Zip as PracZip, LocationsT.Phone as PracPhone, LocationsT.Fax as PracFax, "
								"PatientsT.SuppressStatement, ProvidersT.PersonID as ProvID, (%s WHERE PersonT.ID = TransProvPersonT.ID) as DocName, "
								" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
								" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, ", strHeaderProv);
				 part8.Format(  "PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward,  "
								"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, "
								"BillsT.ID as BillID, BillsT.Date as BillDate, BillsT.Description as BillDescription, PersonT.BirthDate, \r\n"
								"ICD9T1.CodeNumber AS ICD9Code1, ICD9T2.CodeNumber AS ICD9Code2, ICD9T3.CodeNumber AS ICD9Code3, ICD9T4.CodeNumber AS ICD9Code4, \r\n"
								"ICD10T1.CodeNumber AS ICD10Code1, ICD10T2.CodeNumber AS ICD10Code2, ICD10T3.CodeNumber AS ICD10Code3, ICD10T4.CodeNumber AS ICD10Code4, \r\n"
								"WhichCodesQ.WhichCodes9, WhichCodesQ.WhichCodes10, WhichCodesQ.WhichCodesBoth, \r\n"
								"PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
								" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
								" (%s WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 As Prepayment,  "
								"TransProvPersonT.First AS TransProvFirst, TransProvPersonT.Middle AS TransProvMiddle, TransProvPersonT.Last As TransProvLast, TransProvPersonT.Address1 As TransProvAdd1, TransProvPersonT.Address2 As TransProvAdd2, TransProvPersonT.City As TransProvCity, TransProvPersonT.State As TransProvState, TransProvPersonT.Zip AS TransProvZip, TransProvPersonT.ID AS TransProvID, "
								" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
								" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum,  TransProvidersT.NPI as TransProviderNPI,  " 
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
								/*"FROM PersonT LEFT OUTER JOIN "
								"    LocationsT ON  "
								"    PersonT.Location = LocationsT.ID RIGHT OUTER JOIN "
								"    LineItemT LineItemT1 RIGHT OUTER JOIN "
								"    PaymentsT ON  "
								"    LineItemT1.ID = PaymentsT.ID "
								"    LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
								"    RIGHT OUTER JOIN "
								"    AppliesT ON  "
								"    PaymentsT.ID = AppliesT.SourceID LEFT OUTER JOIN "
								"    LineItemT RIGHT OUTER JOIN "
								"    ChargesT LEFT OUTER JOIN "
								"    BillsT LEFT OUTER JOIN "
								"    PatientsT ON BillsT.PatientID = PatientsT.PersonID ON  "
								"    ChargesT.BillID = BillsT.ID ON LineItemT.ID = ChargesT.ID ON  "
								"    AppliesT.DestID = ChargesT.ID ON  "
								"    PersonT.ID = PatientsT.PersonID LEFT OUTER JOIN "
								"    PersonT PersonT1 RIGHT OUTER JOIN "
								"    ProvidersT ON PersonT1.ID = ProvidersT.PersonID ON  "
								"    PatientsT.MainPhysician = ProvidersT.PersonID "
								" LEFT JOIN DiagCodes DiagCodes1 ON BillsT.Diag1ID = DiagCodes1.ID LEFT JOIN DiagCodes DiagCodes2 ON BillsT.Diag2ID = DiagCodes2.ID LEFT JOIN DiagCodes DiagCodes3 ON BillsT.Diag3ID = DiagCodes3.ID LEFT JOIN DiagCodes DiagCodes4 ON BillsT.Diag4ID = DiagCodes4.ID "
								"LEFT JOIN PersonT TransProvPersonT ON ChargesT.DoctorsProviders = TransProvPersonT.ID "
								*/
								" FROM LineItemT LineItemT1 LEFT JOIN PaymentsT ON LineItemT1.ID = PaymentsT.ID "
								" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
								" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
								" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
								" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
								" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
								" LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
								" LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
								" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
								" LEFT JOIN PatientsT ON LineItemT1.PatientID = PatientsT.PersonID "
								" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
								" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
								" LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID "
								" LEFT JOIN PersonT PersonT1 on ProvidersT.PersonID = PersonT1.ID \r\n"
								
								" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
								" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
								" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
								" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
											
								" LEFT JOIN  \r\n"
								" (SELECT ChargesT.ID as ChargeID, "
								" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
								" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
								" STUFF((SELECT ', ' +  \r\n "
								" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
									 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
									 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
								" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
								" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
								" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
								" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
								" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
								" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
								" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
								" FROM ChargesT \r\n "
								" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "

								" LEFT JOIN PersonT TransProvPersonT ON ChargesT.DoctorsProviders = TransProvPersonT.ID "
								" LEFT JOIN ProvidersT TransProvidersT ON TransProvPersonT.ID = TransProvidersT.PersonID "
								" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
								" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON PaymentsT.ID = LineItemCorrections2T.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT Void2T ON PaymentsT.ID = Void2T.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
								"WHERE (PatientsT.PersonID = ? ) AND (%s >= ? ) AND (%s <= ? ) AND (LineItemT1.Deleted = 0) AND (LineItemT.Deleted = 0) "
								" AND BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL "
								" AND (BillsT.Deleted = 0) AND (AppliesT.PointsToPayments = 0) %s %s " 
									+ GetStatementChargebackString("ChargebacksPayments") + " "
									+ GetStatementChargebackString("ChargebacksAdjustments") + " "
								" ) AS StatementDataAppliesCharges "
								"UNION "
								"/*Payments applied to payments*/ "
								"SELECT * FROM (SELECT AppliesT.DestID AS ChargeID,  AppliesT.ID as PaymentID, "
								"    PatientsT.UserDefinedID AS PatientID,  "
								"    PatientsT.PersonID AS PatID, LineItemT1.Type,  "
								"    AppliesT.Amount AS ApplyAmount, LineItemT1.Date,  "
								"    LineItemT1.InputDate, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
								"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
								"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
								"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
								" ELSE LineItemT1.Description END) AS Description,  "
								"    CASE WHEN (PaymentsT.InsuredPartyID) > 0 THEN AppliesT.Amount ELSE 0 END AS Insurance, PersonT.Last, PersonT.First,  "
								"    PersonT.Middle, PersonT.Address1, PersonT.Address2,  "
								"    PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, "
								"    LocationsT.Name AS PracName,  "
								"    LocationsT.Address1 AS PracAddress1,  ", strTransProv, m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT1.Date", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT1.Date", strLoc, strBillID);
				   part9.Format("    LocationsT.Address2 AS PracAddress2,  "
								"    LocationsT.City AS PracCity, LocationsT.State AS PracState,  "
								"    LocationsT.Zip AS PracZip, LocationsT.Phone AS PracPhone,  "
								"    LocationsT.Fax AS PracFax, PatientsT.SuppressStatement,  "
								"    ProvidersT.PersonID AS ProvID,  "
								"    (%s WHERE PersonT.ID = TransProvPersonT.ID) AS DocName, "
								" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
								" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
								"     PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS PatForward, "
								"     PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatComma, "
								" LineItemT.ID as BillID, LineItemT.Date as BillDate, "
								" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
								"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
								"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
								"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
								"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
								"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
								" PersonT.BirthDate, \r\n"
								" '' as ICD9Code1, '' as ICD9Code2, '' as ICD9Code3, '' as ICD9Code4, \r\n"
								" '' as ICD10Code1, '' as ICD10Code2, '' as ICD10Code3, '' as ICD10Code4, \r\n"
								" '' as WhichCodes9, '' as WhichCodes10, '' as WhichCodesBoth, \r\n"
								" PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
								" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
								" (%s WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, 0 As PrePayment,  "
								" TransProvPersonT.First AS TransProvFirst, TransProvPersonT.Middle AS TransProvMiddle, TransProvPersonT.Last As TransProvLast, TransProvPersonT.Address1 As TransProvAdd1, TransProvPersonT.Address2 As TransProvAdd2, TransProvPersonT.City As TransProvCity, TransProvPersonT.State As TransProvState, TransProvPersonT.Zip AS TransProvZip, TransProvPersonT.ID AS TransProvID, "
								" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
								" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum,  TransProvidersT.NPI as TransProviderNPI,  " 
								" '' AS BillStatementNote, "
								" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
								"FROM PersonT PersonT1 RIGHT OUTER JOIN "
								"    ProvidersT ON  "
								"    PersonT1.ID = ProvidersT.PersonID RIGHT OUTER JOIN "
								"    PersonT LEFT OUTER JOIN "
								"    LocationsT ON  "
								"    PersonT.Location = LocationsT.ID RIGHT OUTER JOIN "
								"    LineItemT LineItemT1 LEFT OUTER JOIN "
								"    PatientsT ON LineItemT1.PatientID = PatientsT.PersonID ON  "
								"    PersonT.ID = PatientsT.PersonID ON  "
								"    ProvidersT.PersonID = PatientsT.MainPhysician RIGHT OUTER JOIN "
								"    PaymentsT ON  "
								"    LineItemT1.ID = PaymentsT.ID "
								"	LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID "
								"	LEFT JOIN ChargebacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID "
								"	RIGHT OUTER JOIN "
								"    LineItemT RIGHT OUTER JOIN "
								"    AppliesT ON LineItemT.ID = AppliesT.DestID ON  "
								"    PaymentsT.ID = AppliesT.SourceID "
								"	LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID "
								"	LEFT JOIN ChargebacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID "
								"    LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
								"	 LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
								"LEFT JOIN PersonT TransProvPersonT ON PaymentsT.ProviderID = TransProvPersonT.ID "
								" LEFT JOIN ProvidersT TransProvidersT ON TransProvPersonT.ID = TransProvidersT.PersonID "
								" LEFT JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT VoidT ON LineItemT.ID = VoidT.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON LineItemT1.ID = LineItemCorrections2T.OriginalLineItemID "
								" LEFT JOIN LineItemCorrectionsT Void2T ON LineItemT1.ID = Void2T.VoidingLineItemID "
								" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
								"WHERE (PatientsT.PersonID = ? ) AND (%s >= ? ) AND (%s <= ? ) AND (LineItemT1.Deleted = 0) "
								" AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL "
								" AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 1) %s %s "
								+ GetStatementChargebackString("ChargebacksPayments1") + " " 
								+ GetStatementChargebackString("ChargebacksAdjustments1") + " "
								+ GetStatementChargebackString("ChargebacksPayments") + " " 
								+ GetStatementChargebackString("ChargebacksAdjustments") + " "
								" ) AS StatementDataAppliesPays) AS StmtApplies) as StatementAllData "
								"/*this ends StmtApplies and StatementAllData*/ "
								"LEFT JOIN ", strHeaderProv, strTransProv, m_pReport->nDateFilter == 2 ? "LineItemT.Date" : "LineItemT1.Date", m_pReport->nDateFilter == 2 ? "LineItemT.Date" : "LineItemT1.Date", strLoc, strNoUnAppPays);
				  part10a.Format("/* this is a query that returns the time and date of the next appointment for the patient and the patientID so that it cam be linked to the main query*/ "
								"(SELECT AppointmentsT.Date AS AppDate, AppointmentsT.StartTime, AppointmentsT.PatientID as PatID "
								"FROM AppointmentsT  "
								"WHERE (AppointmentsT.Date > GetDate()) AND (AppointmentsT.PatientID = ? ) AND (AppointmentsT.Status <> 4)) AS NextApp ON NextApp.PatID = StatementAllData.PatID LEFT JOIN LineItemT on LineItemT.ID = StatementAllData.ID "
								"LEFT JOIN "
								"  (SELECT CASE WHEN Thirty IS NULL THEN 0 ELSE Thirty END AS Thirty, CASE WHEN Sixty IS NULL THEN 0 ELSE SIXTY END AS Sixty, CASE WHEN Ninety IS NULL THEN 0 ELSE NINETY END AS Ninety,  "
								"  CASE WHEN NINETYPLUS  IS NULL THEN 0 ELSE NinetyPlus END AS NinetyPlus, PatientID, ProviderID  "
								"  FROM(  "
								"  SELECT PatAR.PatientID, PatAR.ProviderID,  Sum(PatAR.Thirty) AS Thirty, Sum(PatAR.Sixty) AS Sixty, Sum(PatAR.Ninety) AS Ninety, Sum(PatAR.NinetyPlus) AS NinetyPlus  "
								"  FROM  "
								"  (SELECT Sum((CASE WHEN Thirty.ChargeAmount IS NULL THEN 0 ELSE Thirty.ChargeAmount END) - (CASE WHEN Thirty.PPAmount IS NULL THEN 0 ELSE Thirty.PPAmount END)) AS Thirty, 0 AS Sixty, 0 As Ninety, 0 AS NinetyPlus, PatientID, ProviderID   "
								"  FROM   "
								"  ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, ProviderID FROM    "
								"  (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, ChargesT.DoctorsProviders as ProviderID FROM ChargeRespDetailT LEFT JOIN  "
								"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID   "
								"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
								"   LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd,-30, getDate()))  AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (PatientID = ? ) "
								"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, ChargesT.DoctorsProviders) As Charges   "
								"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM   "
								"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID  "
								"   GROUP BY PatientID, ProviderID)  "
								"   UNION   "
								"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, PaymentsT.ProviderID   "
								"   FROM   "
								"   LineItemT LEFT JOIN    "
								"   (SELECT DestID, Sum(Amount) as Amount   "
								"   FROM AppliesT   "
								"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID    "
								"   WHERE (InsuredPartyID = -1)   "
								" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
								"   LEFT JOIN  "
								" 	(SELECT SourceID, Sum(Amount) AS Amount   "
								"   FROM AppliesT   "
								"  	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID   "
								"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd,-30, getDate())) AND (PaymentsT.InsuredPartyID = -1) AND (PatientID =  ? ) %s "
								"   GROUP BY LineItemT.PatientID, PaymentsT.ProviderID)) AS Thirty    "
								"   GROUP BY Thirty.PatientID, Thirty.ProviderID   "
								"   UNION    "
								"   SELECT 0 AS Thirty, Sum((CASE WHEN Sixty.ChargeAmount IS NULL THEN 0 ELSE Sixty.ChargeAmount END) - (CASE WHEN Sixty.PPAmount IS NULL THEN 0 ELSE Sixty.PPAmount END)) AS Sixty, 0 AS Ninety, 0 AS NinetyPlus, PatientID, ProviderID    "
								"   FROM   "
								"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, ProviderID FROM    "
								"  	(SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, ChargesT.DoctorsProviders AS ProviderID FROM ChargeRespDetailT LEFT JOIN   "
								"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID   "
								" 	LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
								"	LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
								" 	WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date <= DATEADD(dd, -30, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (PatientID = ? ) "
								" 	GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, ChargesT.DoctorsProviders) As Charges   "
								"   LEFT JOIN    "
								" 	(SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM   "
								" 	ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays   "
								"   ON Charges.DetailID = Pays.DetailID  "
								"   GROUP BY PatientID, ProviderID)  "
								"   UNION   "
								"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, PaymentsT.ProviderID  "
								"   FROM   "
								"   LineItemT LEFT JOIN   "
								"   (SELECT DestID, Sum(Amount) as Amount   "
								"   	FROM AppliesT    "
								" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID    "
								"  	WHERE (InsuredPartyID = -1)   "
								" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID   "
								"   LEFT JOIN  "
								" 	(SELECT SourceID, Sum(Amount) AS Amount   "
								"  	FROM AppliesT   "
								" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID   "
								"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd, -60, getDate())) AND (LineItemT.Date <= DATEADD(dd, -30, getDate())) AND (PaymentsT.InsuredPartyID = -1) AND (PatientID = ? ) %s "
								"   GROUP BY LineItemT.PatientID, PaymentsT.ProviderID)) AS Sixty   "
								"   GROUP BY Sixty.PatientID, Sixty.ProviderID ", strHideUnAppliedPrepayments, strHideUnAppliedPrepayments);
								part10b.Format("   UNION   "
								"   SELECT 0 AS Thirty, 0 AS Sixty, Sum((CASE WHEN Ninety.ChargeAmount IS NULL THEN 0 ELSE Ninety.ChargeAmount END) - (CASE WHEN Ninety.PPAmount IS NULL THEN 0 ELSE Ninety.PPAmount END)) AS Ninety, 0 AS NinetyPlus, PatientID, ProviderID   "
								"   FROM  "
								"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, ProviderID FROM   "
								"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, ChargesT.DoctorsProviders AS ProviderID FROM ChargeRespDetailT LEFT JOIN  "
								"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
								"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
								"   LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date >= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1) AND (PatientID =  ? ) "
								"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, ChargesT.DoctorsProviders) As Charges  "
								"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM   "
								"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID  "
								"   GROUP BY PatientID, ProviderID)  "
								"   UNION   "
								"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, PaymentsT.ProviderID  "
								"   FROM   "
								"   LineItemT LEFT JOIN   "
								"  	(SELECT DestID, Sum(Amount) as Amount   "
								"  	FROM AppliesT   "
								"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
								"  	WHERE (InsuredPartyID = -1)  "
								"  	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
								"   LEFT JOIN  "
								"  	(SELECT SourceID, Sum(Amount) AS Amount  "
								" 	FROM AppliesT  "
								" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
								"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd, -60, getDate())) AND (LineItemT.Date >= DATEADD(dd, -90, getDate())) AND (PaymentsT.InsuredPartyID = -1) AND (PatientID = ? ) %s "
								"   GROUP BY LineItemT.PatientID, PaymentsT.ProviderID)) AS Ninety  "
								"   GROUP BY Ninety.PatientID, Ninety.ProviderID  "
								"    UNION    "
								"   SELECT 0 AS Thirty, 0 AS Sixty, 0 AS Ninety, Sum((CASE WHEN NinetyPlus.ChargeAmount IS NULL THEN 0 ELSE NinetyPlus.ChargeAmount END) - (CASE WHEN NinetyPlus.PPAmount IS NULL THEN 0 ELSE NinetyPlus.PPAmount END)) AS NinetyPlus, PatientID, ProviderID   "
								"   FROM   "
								"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, ProviderID FROM   "
								"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, ChargesT.DoctorsProviders As ProviderID FROM ChargeRespDetailT LEFT JOIN  "
								"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID   "
								"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID   "
								"   LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1) AND (PatientID = ? ) "
								"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, ChargesT.DoctorsProviders) As Charges   "
								"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
								"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID   "
								"   GROUP BY PatientID, ProviderID)   "
								"    UNION    "
								"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, PaymentsT.ProviderID "
								"   FROM   "
								"   LineItemT LEFT JOIN   "
								"  	(SELECT DestID, Sum(Amount) as Amount  "
								" 	FROM AppliesT   "
								" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
								"  	WHERE (InsuredPartyID = -1)   "
								" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID   "
								"    LEFT JOIN   "
								" 	(SELECT SourceID, Sum(Amount) AS Amount   "
								" 	FROM AppliesT   "
								" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
								"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
								"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd,-90, getDate())) AND (PaymentsT.InsuredPartyID = -1)  AND (PatientID = ? ) %s "
								"   GROUP BY LineItemT.PatientID, PaymentsT.ProviderID)) AS NinetyPlus  "
							    "   GROUP BY NinetyPlus.PatientID, NinetyPlus.ProviderID) PatAR   "
								"   GROUP BY PatAR.PatientID, PatAR.ProviderID) AS PatientAR) AS StatementAR ON StatementAllData.PatID = StatementAR.PatientID "
								"   AND StatementAllData.TransProvID = StatementAR.ProviderID "
								"Group By StatementAllData.ID, StatementAllData.PatientId, StatementAllData.PatID, StatementAllData.Type, StatementAllData.Total, "
								"StatementAllData.Date, StatementAllData.Description, StatementAllData.Insurance, StatementAllData.Last, "
								"StatementAllData.First, StatementAllData.Middle, StatementAllData.Address1, StatementAllData.Address2, StatementAllData.City, "
								"StatementAllData.State, StatementAllData.Zip, StatementAllData.PatForward, StatementAllData.PatComma, StatementAllData.DocName, "
								"StatementAllData.DocAddress1, StatementAllData.DocAddress2, StatementAllData.DocCity, StatementAllData.DocState, StatementAllData.DocZip, "
								"StatementAllData.PracName, StatementAllData.PracAddress1, StatementAllData.PracAddress2, StatementAllData.PracCity, StatementAllData.PracState, "
								"StatementAllData.PracZip, StatementAllData.PracPhone, StatementAllData.PracFax, StatementAllData.BillId, StatementAllData.BillDate, "
								"StatementAllData.BillDescription, StatementAllData.Birthdate, \r\n"
								"StatementAllData.ICD9Code1, StatementAllData.ICD9Code2, StatementAllData.ICD9Code3, StatementAllData.ICD9Code4, \r\n"
								"StatementAllData.ICD10Code1, StatementAllData.ICD10Code2, StatementAllData.ICD10Code3, StatementAllData.ICD10Code4, \r\n"
								"StatementAllData.WhichCodes9,StatementAllData.WhichCodes10, StatementAllData.WhichCodesBoth, \r\n"
								"StatementAllData.DocAddress1, StatementAllData.DocAddress2, StatementAllData.DocCity, StatementAllData.DocState, StatementAllData.DocZip, "
								"StatementAllData.ProvId, StatementAllData.Location, StatementAllData.Date, LineItemT.Date, StatementAllData.StatementType, StatementAllData.GroupFixID, StatementAllData.LocationFixID, StatementAllData.ProvTaxID, StatementAllData.TransProv, StatementAllData.PrePayment, "
								"StatementAllData.ProvEIN, StatementAllData.ProvLicense, StatementAllData.ProvUPIN, StatementAllData.ProvDEA, StatementAllData.ProvBCBS, StatementAllData.ProvMedicare, StatementAllData.ProvMedicaid, StatementAllData.ProvWorkersComp, StatementAllData.ProvOtherID, StatementAllData.ProvOtherIDDesc, StatementAR.Thirty, StatementAR.Sixty, StatementAR.Ninety, StatementAR.NinetyPlus, StatementAllData.PatPhone, "
								" StatementAllData.FullChargeNoTax, StatementAllData.ChargeTax1, StatementAllData.ChargeTax2, StatementAllData.TaxRate1, StatementAllData.TaxRate2, StatementAllData.TransProvAdd1, StatementAllData.TransProvAdd2, StatementAllData.TransProvCity, StatementAllData.TransProvState, StatementAllData.TransProvZip, StatementAllData.TransProvFirst, StatementAllData.TransProvMiddle, StatementAllData.TransProvLast, StatementAllData.TransProvID, StatementAllData.Quantity, "
								"  StatementAllData.CCType, StatementAllData.CCNumber, StatementAllData.CheckNo, StatementAllData.BankNo, StatementAllData.CheckAcctNo, "
								" StatementAllData.CCHoldersName, StatementAllData.CCExpDate, StatementAllData.CCAuthNo, StatementAllData.BankRoutingNum, StatementAllData.PercentOff, StatementAllData.DiscountAmt, StatementAllData.DiscountCategoryDesc, StatementAllData.TransProviderNPI, StatementAllData.CPTCode, "
								" StatementAllData.CPTModifier1, StatementAllData.CPTModifier2, StatementAllData.CPTModifier3, StatementAllData.CPTModifier4, StatementAllData.BillStatementNote, StatementAllData.LineItemStatementNote  ) as StatementSubQ "
								"LEFT OUTER JOIN  "
								"/*this query returns all the patient information that the statement needs for a patient.  Eventually the patient demographics in the above queries*/ "
								"/*needs to be phased out and all of the information can come from this query*/ "
								"/*Begin StatementEndQ*/ "
								"(Select *  "
								"FROM "
								"/*this is a patient info query*/ "
								"(SELECT PersonT1.First + ' ' + PersonT1.Middle + ' ' + PersonT1.Last + ' ' + PersonT1.Title "
								"     AS DocName, PersonT1.ID as ProvID, PatientsT.SuppressStatement,  "
								"    PatientsT.PersonID AS PatID, PatientsT.PrimaryRespPartyID AS PrimaryRespID, PatientsT.StatementNote,  "
								"  PatCoordT.First as PatCoordFirst, PatCoordT.Middle as PatCoordMiddle, PatCoordT.Last as PatCoordLast, ProvidersT.NPI as ProviderNPI "								
								"FROM PatientsT LEFT OUTER JOIN "
								"    PersonT PersonT1 ON  "
								"    PatientsT.MainPhysician = PersonT1.ID LEFT OUTER JOIN "
								"    PersonT ON PatientsT.PersonID = PersonT.ID "
								"	 LEFT JOIN ProvidersT ON PersonT1.ID = ProvidersT.PersonID "
								"    LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
								"WHERE (PatientsT.PersonID = ? ))  "
								"AS PatInfo  "
								"/*end PatInfo*/ "
								"LEFT OUTER JOIN "
								"/*this query returns primary insurance information*/ "
								"(SELECT InsuranceCoT.Name as PriInsCo, "
								"	PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS "
								"     PriGuarForward,  "
								"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PriGuarComma, ", strHideUnAppliedPrepayments, strHideUnAppliedPrepayments);
				part11.Format(  "     PersonT.First AS PriInsFirst,  "
								"    PersonT.Middle AS PriInsMiddle,  "
								"    PersonT.Last AS PriInsLast, PatientsT.PersonID as PersonID "
								"FROM InsuranceCoT RIGHT OUTER JOIN "
								"    PersonT RIGHT OUTER JOIN "
								"    InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID ON  "
								"    InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID RIGHT "
								"     OUTER JOIN "
								"    PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
								"WHERE (PatientsT.PersonID = ? ) AND (InsuredPartyT.RespTypeID = 1)) AS  PriInsInfo  "
								"ON PatInfo.PatID = PriInsInfo.PersonID "
								"/*End PriInsInfo*/ "
								"LEFT Outer JOIN "
								"/*This query returns secondary insurance information*/ "
								"(SELECT InsuranceCoT.Name as SecInsCo, "
								"	PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS "
								"     SecGuarForward,  "
								"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS SecGuarComma, "
								"     PersonT.First AS SecInsFirst,  "
								"    PersonT.Middle AS SecInsMiddle,  "
								"    PersonT.Last AS SecInsLast, PatientsT.PersonID as PersID "
								"FROM InsuranceCoT RIGHT OUTER JOIN "
								"    PersonT RIGHT OUTER JOIN "
								"    InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID ON  "
								"    InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID RIGHT "
								"     OUTER JOIN "
								"    PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
								" WHERE (PatientsT.PersonID = ? ) AND (InsuredPartyT.RespTypeID = 2)) AS SecInsInfo "
								" /*end SecInsInfo*/ "
								" ON PatInfo.PatID = SecInsInfo.PersID  "
								" Left Outer Join "
								" (SELECT ResponsiblePartyT.PersonID as RespID, ResponsiblePartyT.PatientID AS RespPatID, First as RespFirst, Middle as RespMiddle, Last as RespLast, Address1 as RespAdd1, Address2 as RespAdd2,  "
								" City as RespCity, State as RespState, Zip as RespZip "
								" FROM PersonT Inner Join ResponsiblePartyT ON PersonT.ID = ResponsiblePartyT.PersonID LEFT JOIN PatientsT ON ResponsiblePartyT.PatientID = PatientsT.PersonID @RespFilter) AS ResPartyT  "
								" ON PatInfo.PatID = ResPartyT.RespPatID) AS StatementEndQ ON StatementSubQ.PatID = StatementEndQ.PatID "
								" LEFT JOIN "
								"(SELECT * FROM [%s]) AS StatementLastPatPayInfoQ " 
								" ON CASE WHEN StatementSubQ.TransProvID IS NULL THEN -1 ELSE StatementSubQ.TransProvID END = "
								" CASE WHEN StatementLastPatPayInfoQ.ProvID IS NULL THEN -1 ELSE StatementLastPatPayInfoQ.ProvID END "
								" AND StatementSubQ.PatID = StatementLastPatPayInfoQ.PatID "
								" AND StatementLastPatPayInfoQ.IsInsurance = 0 "
								" LEFT JOIN "
								"(SELECT * FROM [%s]) AS StatementLastInsPayInfoQ " 
								" ON CASE WHEN StatementSubQ.TransProvID IS NULL THEN -1 ELSE StatementSubQ.TransProvID END =  "
								" CASE WHEN StatementLastInsPayInfoQ.ProvID IS NULL THEN -1 ELSE StatementLastInsPayInfoQ.ProvID END  "
								" AND StatementSubQ.PatID = StatementLastInsPayInfoQ.PatID "
								" AND StatementLastInsPayInfoQ.IsInsurance = 1 "
								"/*end StatementEndQ*/", strTempTableName, strTempTableName);

						// (a.walling 2006-10-24 13:12) - PLID 16059 - Multiple Responsible parties
						part11.Replace("@RespFilter", strRespFilter);

						return _T(part1 + part2 + part3 + part4 + part5 + part6 + part7 + part8 + part9 + part10a + part10b + part11);
						}
						 break;




					 default:
							return "";
					break;
		}
}



CString CStatementSqlBuilder::GetLastProviderPaymentInformationSql(CString strPatTempTable, long nPatientID) {
	long nReportID = m_pReport->nID;
	switch (nReportID) {

		//by provider statements
		case 483:
		case 484:
		case 485:
		case 486:
				
			{
				// (j.gruber 2008-07-01 17:40) - PLID 29553 - make a temp table

				//make a filter for the individual reports 
				CString strPatientFilter;
				if (nReportID == 483 || nReportID == 485) {
					if (!strPatTempTable.IsEmpty()) {
						strPatientFilter.Format(" AND LineItemT.PatientID IN (SELECT ID FROM [%s])", strPatTempTable);
					}
				}
				else {
					strPatientFilter.Format("AND LineItemT.PatientID = {INT} ", nPatientID);
				}

					
						
				CString strTableName;
				strTableName.Format("#TempStatement%lu", GetTickCount());
				
				// (a.walling 2009-09-08 14:02) - PLID 35178 - Use the snapshot connection
				ExecuteSql(GetConnection(), "CREATE TABLE %s (PatID int, ProvID int, LastPayDate datetime, LastPayAmt money, IsInsurance bit) ", strTableName);

				CString strInnerQuery;

				// (j.gruber 2011-07-05 11:03) - PLID 44831 - leave in originals since we want the actual payment, not the adjustment
				//TES 7/17/2014 - PLID 62935 - Added code to hide chargebacks when requested
				// (r.gonet 2015-05-05 14:38) - PLID 65870 - Exclude Gift Certificate Refunds
				// (r.goldschmidt 2015-11-10 16:35) - PLID 65568 - Exclude voided payments
				// (c.haag 2016-02-11) - PLID 68236 - Split single Chargebacks joins into two joins to avoid producing super slow query plans
				strInnerQuery.Format(
					" SELECT MaxDateQ.PatID, MaxTDate as LastPayDate, Max(SumofAmount) as LastPayAmt, MaxDateQ.ProvID, MaxDateQ.IsInsurance FROM  "
					"   (SELECT PatID, Max(TDate) as MaxTDate, ProvID, IsInsurance "
					"   FROM ( "
					"   SELECT  StmtLastPayDateQ.ID                                      , "
					"  		 StmtLastPayDateQ.PatientID AS PatID                      , "
					"  		 StmtLastPayDateQ.ProvID AS ProvID                        , "
					"  		 StmtLastPayDateQ.TDate  AS TDate                         , "
					"  		 SUM(StmtLastPayDateQ.Amount) AS SumOfAmount              , "
					"  		 StmtLastPayDateQ.LocID       AS LocID					 ,        "
					"  		 CASE WHEN StmtLastPayDateQ.InsuredPartyID = -1 then 0 else 1 END as IsInsurance "
					"   FROM ( "
					"  		 SELECT * "
					"  		 FROM "
					"  				 (SELECT LineItemT.ID       , "
					"  						 LineItemT.PatientID, "
					"  						 Amount = "
					"  						 CASE "
					"  								  WHEN [_PartiallyAppliedPaysQ].[ID] IS NULL "
					"  								  THEN "
					"  										 CASE "
					"  												WHEN [LineItemT_1].[ID] IS NULL "
					"  												THEN [LineItemT].[Amount] "
					"  												ELSE [AppliesT].[Amount] "
					"  										END "
					"  								ELSE [AppliesT].[Amount] "
					"  						END, "
					"  						ProvID = "
					"  						CASE "
					"  								WHEN [DoctorsProviders] IS NULL "
					"  								THEN [ProviderID] "
					"  								ELSE CASE WHEN [DoctorsProviders] = -1 THEN NULL ELSE  [DoctorsProviders] END "
					"  						END                                                                   , "
					"  						'Full'              AS RandomText                                     , "
					"  						LineItemT.PatientID AS PatID                                          , "
					"  						LineItemT.Date                                             AS TDate   , "
					"  						AppliesT.ID               AS ApplyID                                  , "
					"  						LineItemT.ID              AS LineID                                   , "
					"  						CASE "
					"  								WHEN LineItemT_1.LocationID IS NULL "
					"  								THEN LineItemT.LocationID "
					"  								ELSE LineItemT_1.LocationID "
					"  						END             AS LocID, "
					"  						PaymentsT.InsuredPartyID "
					"  			  FROM (((((((LineItemT "
					"  						LEFT JOIN PaymentsT "
					"  						ON      LineItemT.ID = PaymentsT.ID) "
					"  						LEFT JOIN "
					"  								(SELECT LineItemT_1.ID                  , "
					"  										LineItemT_1.Amount   AS PayAmt  , "
					"  										SUM(AppliesT.Amount) AS ApplyAmt, "
					
					"  										MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount]) AS Total, "
					"  										LineItemT_1.PatientID "
					"  								FROM    LineItemT AS LineItemT_1 "
					"  										LEFT JOIN (PaymentsT "
					"  												LEFT JOIN (AppliesT "
					"  														LEFT JOIN LineItemT "
					"  														ON      AppliesT.DestID = LineItemT.ID) "
					"  												ON      PaymentsT.ID            = AppliesT.SourceID "
					"												LEFT JOIN ChargeBacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"												LEFT JOIN ChargeBacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID) "
					"  										ON      LineItemT_1.ID                  = PaymentsT.ID "
					"  								WHERE (((LineItemT_1.Deleted)                   =0)) "
					"  									AND (PaymentsT.PayMethod                   NOT IN (4,10)) "
					"									AND ChargebacksPayments.ID Is Null "
					"									AND ChargebacksAdjustments.ID Is Null "
					"									%s												 "
					"  								GROUP BY LineItemT_1.ID   , "
					"  										LineItemT_1.Amount, "
					"  										LineItemT_1.PatientID "
					"  								HAVING (((LineItemT_1.ID)                                       IS NOT NULL) "
					"  									AND ((MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount])) <> 0)) "
					"  								) AS _PartiallyAppliedPaysQ ON LineItemT.ID                      = [_PartiallyAppliedPaysQ].ID) "
					"  						LEFT JOIN AppliesT "
					"  						ON      PaymentsT.ID = AppliesT.SourceID) "
					"						LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"						LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"  						LEFT JOIN LineItemT AS LineItemT_1 "
					"  						ON      AppliesT.DestID = LineItemT_1.ID) "
					"  						LEFT JOIN ChargesT "
					"  						ON      LineItemT_1.ID = ChargesT.ID)                         "
					"						LEFT JOIN LineItemCorrectionsT OriginalLineItemsT "
					"						ON		LineItemT.ID = OriginalLineItemsT.OriginalLineItemID) "
					"						LEFT JOIN LineItemCorrectionsT VoidingLineItemsT "
					"						ON		LineItemT.ID = VoidingLineItemsT.VoidingLineItemID) "
					"  				WHERE (((PaymentsT.ID)                    IS NOT NULL) "
					"  					AND ((LineItemT.Deleted)               =0) "
					"  					AND ((LineItemT.Type)                  =1)) "
					"  					AND (PaymentsT.PayMethod              NOT IN (4,10)) "
					"					AND ChargebacksPayments.ID Is Null "
					"					AND ChargebacksAdjustments.ID Is Null "
					"					AND OriginalLineItemsT.OriginalLineItemID IS NULL "
					"					AND VoidingLineItemsT.VoidingLineItemID IS NULL "
					"					%s											"
					"  				) AS _StmtMaxPayInfoFullQ "
					"           "
					"  		UNION "
					"           "
					"  		SELECT  * "
					"  		FROM "
					"  				(SELECT [_PartiallyAppliedPaysQ].ID                                           , "
					"  						LineItemT.PatientID                                                   , "
					"  						[_PartiallyAppliedPaysQ].Total AS Amount                              , "
					"  						PaymentsT.ProviderID           AS ProvID                              , "
					"  						'Part'                         AS RandomText                          , "
					"  						LineItemT.PatientID            AS PatID                               , "
					"  						LineItemT.Date                                             AS TDate   , "
					"  						0                         AS ApplyID                                  , "
					"  						LineItemT.ID              AS LineID                                   , "
					"  						LineItemT.LocationID      AS LocID                                    , "
					"  						PaymentsT.InsuredPartyID "
					"  				FROM (( "
					"  						(SELECT LineItemT_1.ID                  , "
					"  								LineItemT_1.Amount   AS PayAmt  , "
					"  								SUM(AppliesT.Amount) AS ApplyAmt, "
					
					"  								MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount]) AS Total, "
					"  								LineItemT_1.PatientID "
					"  						FROM    LineItemT AS LineItemT_1 "
					"  								LEFT JOIN (PaymentsT "
					"  										LEFT JOIN (AppliesT "
					"  												LEFT JOIN LineItemT "
					"  												ON      AppliesT.DestID = LineItemT.ID) "
					"  										ON      PaymentsT.ID            = AppliesT.SourceID) "
					"										LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"										LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"  								ON      LineItemT_1.ID                  = PaymentsT.ID "
					"  						WHERE (((LineItemT_1.Deleted)                   =0)) "
					"  							AND (PaymentsT.PayMethod                   NOT IN (4,10)) "
					"							AND ChargebacksPayments.ID Is Null "
					"							AND ChargebacksAdjustments.ID Is Null "
					"							%s												 "
					"  						GROUP BY LineItemT_1.ID   , "
					"  								LineItemT_1.Amount, "
					"  								LineItemT_1.PatientID "
					"  						HAVING (((LineItemT_1.ID)                                       IS NOT NULL) "
					"  							AND ((MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount])) <> 0)) "
					"  						) AS _PartiallyAppliedPaysQ "
					"  						INNER JOIN LineItemT "
					"  						ON      [_PartiallyAppliedPaysQ].ID  = LineItemT.ID) "
					"  						INNER JOIN PaymentsT "
					"  						ON      LineItemT.ID = PaymentsT.ID "
					"						LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"						LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"						LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
					"						LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID) "
					"                           "
					"  				WHERE (((LineItemT.Deleted)                =0) "
					"  					AND ((LineItemT.Type)                  =1)) "
					"  					AND (PaymentsT.PayMethod              NOT IN (4,10)) "
					"					AND ChargebacksPayments.ID Is Null "
					"					AND ChargebacksAdjustments.ID Is Null "
					"					AND OriginalLineItemsT.OriginalLineItemID IS NULL "
					"					AND VoidingLineItemsT.VoidingLineItemID IS NULL "
					"					%s											"
					"  				) AS _StmtMaxPayInfoPartQ "
					"  		) AS StmtLastPayDateQ        "
					"  GROUP BY StmtLastPayDateQ.ID                          , "
					"  		StmtLastPayDateQ.PatientID                    , "
					"  		StmtLastPayDateQ.ProvID                       , "
					"  		StmtLastPayDateQ.TDate                        , "
					"  		StmtLastPayDateQ.InsuredPartyID                , "
					"  		StmtLastPayDateQ.LocID                         "
					"  )SubQ "
					"  GROUP BY PatID, ProvID, IsInsurance) AS MaxDateQ "
					"  LEFT JOIN (SELECT  StmtLastPayDateQ.ID                                      , "
					"  		StmtLastPayDateQ.PatientID AS PatID                      , "
					"  		StmtLastPayDateQ.ProvID AS ProvID                        , "
					"  		StmtLastPayDateQ.TDate  AS TDate                         , "
					"  		SUM(StmtLastPayDateQ.Amount) AS SumOfAmount              , "
					"  		StmtLastPayDateQ.LocID       AS LocID					 ,        "
					"  		CASE WHEN StmtLastPayDateQ.InsuredPartyID = -1 then 0 else 1 END as IsInsurance "
					"  FROM ( "
					"  		SELECT * "
					"  		FROM "
					"  				(SELECT LineItemT.ID       , "
					"  						LineItemT.PatientID, "
					"  						Amount = "
					"  						CASE "
					"  								WHEN [_PartiallyAppliedPaysQ].[ID] IS NULL "
					"  								THEN "
					"  										CASE "
					"  												WHEN [LineItemT_1].[ID] IS NULL "
					"  												THEN [LineItemT].[Amount] "
					"  												ELSE [AppliesT].[Amount] "
					"  										END "
					"  								ELSE [AppliesT].[Amount] "
					"  						END, "
					"  						ProvID = "
					"  						CASE "
					"  								WHEN [DoctorsProviders] IS NULL "
					"  								THEN [ProviderID] "
					"  								ELSE CASE WHEN [DoctorsProviders] = -1 THEN NULL ELSE [DoctorsProviders] END "
					"  						END                                                                   , "
					"  						'Full'              AS RandomText                                     , "
					"  						LineItemT.PatientID AS PatID                                          , "
					"  						LineItemT.Date                                             AS TDate   , "
					"  						AppliesT.ID               AS ApplyID                                  , "
					"  						LineItemT.ID              AS LineID                                   , "
					"  						CASE "
					"  								WHEN LineItemT_1.LocationID IS NULL "
					"  								THEN LineItemT.LocationID "
					"  								ELSE LineItemT_1.LocationID "
					"  						END             AS LocID, "
					"  						PaymentsT.InsuredPartyID "
					"  			  FROM (((((LineItemT "
					"  						LEFT JOIN PaymentsT "
					"  						ON      LineItemT.ID = PaymentsT.ID) "
					"  						LEFT JOIN "
					"  								(SELECT LineItemT_1.ID                  , "
					"  										LineItemT_1.Amount   AS PayAmt  , "
					"  										SUM(AppliesT.Amount) AS ApplyAmt, "
					
					"  										MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount]) AS Total, "
					"  										LineItemT_1.PatientID "
					"  								FROM    LineItemT AS LineItemT_1 "
					"  										LEFT JOIN (PaymentsT "
					"  												LEFT JOIN (AppliesT "
					"  														LEFT JOIN LineItemT "
					"  														ON      AppliesT.DestID = LineItemT.ID) "
					"  												ON      PaymentsT.ID            = AppliesT.SourceID) "
					"												LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"												LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"  										ON      LineItemT_1.ID                  = PaymentsT.ID "
					"  								WHERE (((LineItemT_1.Deleted)                   =0)) "
					"  									AND (PaymentsT.PayMethod                   NOT IN (4,10)) "
					"									AND ChargebacksPayments.ID Is Null "
					"									AND ChargebacksAdjustments.ID Is Null "
					"									%s							 					"
					"  								GROUP BY LineItemT_1.ID   , "
					"  										LineItemT_1.Amount, "
					"  										LineItemT_1.PatientID "
					"  								HAVING (((LineItemT_1.ID)                                       IS NOT NULL) "
					"  									AND ((MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount])) <> 0)) "
					"  								) AS _PartiallyAppliedPaysQ ON LineItemT.ID                      = [_PartiallyAppliedPaysQ].ID) "
					"  						LEFT JOIN AppliesT "
					"  						ON      PaymentsT.ID = AppliesT.SourceID "
					"						LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"						LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"						LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
					"						LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID) "
					"  						LEFT JOIN LineItemT AS LineItemT_1 "
					"  						ON      AppliesT.DestID = LineItemT_1.ID) "
					"  						LEFT JOIN ChargesT "
					"  						ON      LineItemT_1.ID = ChargesT.ID)                         "
					"  				WHERE (((PaymentsT.ID)                    IS NOT NULL) "
					"  					AND ((LineItemT.Deleted)               =0) "
					"  					AND ((LineItemT.Type)                  =1)) "
					"  					AND (PaymentsT.PayMethod              NOT IN (4,10)) "
					"					AND ChargebacksPayments.ID Is Null "
					"					AND ChargebacksAdjustments.ID Is Null "
					"					AND OriginalLineItemsT.OriginalLineItemID IS NULL "
					"					AND VoidingLineItemsT.VoidingLineItemID IS NULL "
					"					%s											"
					"  				) AS _StmtMaxPayInfoFullQ "
					"           "
					"  		UNION "
					"           "
					"  		SELECT  * "
					"  		FROM "
					"  				(SELECT [_PartiallyAppliedPaysQ].ID                                           , "
					"  						LineItemT.PatientID                                                   , "
					"  						[_PartiallyAppliedPaysQ].Total AS Amount                              , "
					"  						PaymentsT.ProviderID           AS ProvID                              , "
					"  						'Part'                         AS RandomText                          , "
					"  						LineItemT.PatientID            AS PatID                               , "
					"  						LineItemT.Date                                             AS TDate   , "
					"  						0                         AS ApplyID                                  , "
					"  						LineItemT.ID              AS LineID                                   , "
					"  						LineItemT.LocationID      AS LocID                                    , "
					"  						PaymentsT.InsuredPartyID "
					"  				FROM (( "
					"  						(SELECT LineItemT_1.ID                  , "
					"  								LineItemT_1.Amount   AS PayAmt  , "
					"  								SUM(AppliesT.Amount) AS ApplyAmt, "
					
					"  								MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount]) AS Total, "
					"  								LineItemT_1.PatientID "
					"  						FROM    LineItemT AS LineItemT_1 "
					"  								LEFT JOIN (PaymentsT "
					"  										LEFT JOIN (AppliesT "
					"  														LEFT JOIN LineItemT "
					"  														ON      AppliesT.DestID = LineItemT.ID) "
					"  												ON      PaymentsT.ID            = AppliesT.SourceID) "
					"												LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"												LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"  										ON      LineItemT_1.ID                  = PaymentsT.ID "
					"  								WHERE (((LineItemT_1.Deleted)                   =0)) "
					"  									AND (PaymentsT.PayMethod                   NOT IN (4,10)) "
					"									AND ChargebacksPayments.ID Is Null "
					"									AND ChargebacksAdjustments.ID Is Null "
					"									%s												 "
					"  								GROUP BY LineItemT_1.ID   , "
					"  										LineItemT_1.Amount, "
					"  								LineItemT_1.PatientID "
					"  						HAVING (((LineItemT_1.ID)                                       IS NOT NULL) "
					"  							AND ((MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount])) <> 0)) "
					"  						) AS _PartiallyAppliedPaysQ "
					"  						INNER JOIN LineItemT "
					"  						ON      [_PartiallyAppliedPaysQ].ID  = LineItemT.ID) "
					"  						INNER JOIN PaymentsT "
					"  						ON      LineItemT.ID = PaymentsT.ID "
					"						LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"						LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"						LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
					"						LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID) "
					"                           "
					"  				WHERE (((LineItemT.Deleted)                =0) "
					"  					AND ((LineItemT.Type)                  =1)) "
					"  					AND (PaymentsT.PayMethod              NOT IN (4,10)) "
					"					AND ChargebacksPayments.ID Is Null "
					"					AND ChargebacksAdjustments.ID Is Null "
					"					AND OriginalLineItemsT.OriginalLineItemID IS NULL "
					"					AND VoidingLineItemsT.VoidingLineItemID IS NULL "
					"					%s											"
					"  				) AS _StmtMaxPayInfoPartQ "
					"  		) AS StmtLastPayDateQ        "
					"  GROUP BY StmtLastPayDateQ.ID                          , "
					"  		StmtLastPayDateQ.PatientID                    , "
					"  		StmtLastPayDateQ.ProvID                       , "
					"  		StmtLastPayDateQ.TDate                        , "
					"  		StmtLastPayDateQ.InsuredPartyID                , "
					"  		StmtLastPayDateQ.LocID       ) OtherInfoQ "
					"  ON MaxDateQ.PatID = OtherInfoQ.PatID  "
					"  AND MaxDateQ.MaxTDate = OtherInfoQ.TDate "
					"  AND CASE WHEN MaxDateQ.ProvID IS NULL THEN -1 ELSE MaxDateQ.ProvID END = CASE WHEN OtherInfoQ.ProvID IS NULL THEN -1 ELSE OtherInfoQ.ProvID END "
					"  AND MaxDateQ.IsInsurance = OtherInfoQ.IsInsurance "
					"  GROUP BY MaxDateQ.PatID, MaxTDate, MaxDateQ.ProvID, MaxDateQ.IsInsurance "
					  ,strPatientFilter, strPatientFilter, strPatientFilter, strPatientFilter,
					  strPatientFilter, strPatientFilter, strPatientFilter, strPatientFilter);

					CString strQuery;
					strQuery.Format("INSERT INTO %s (PatID, ProvID, LastPayDate, LastPayAmt, IsInsurance) "
					" SELECT PatID, ProvID, LastPayDate, LastPayAmt, IsInsurance FROM (%s) StmtTempQ ", strTableName, strInnerQuery);
#ifdef _DEBUG	
	MsgBox(strQuery);
#endif

				// (a.walling 2009-09-08 14:02) - PLID 35178 - Use the snapshot connection
				if (nReportID == 483 || nReportID == 485 ) {
					ExecuteSql(GetConnection(), strQuery);
				}
				else {
					ExecuteParamSql(GetConnection(), strQuery, nPatientID, nPatientID, nPatientID, nPatientID, 
												   nPatientID, nPatientID, nPatientID, nPatientID);
				}

					

				return strTableName;

			}
		break;

	default: return "";
	break;

	}


}

CString CStatementSqlBuilder::GetLastLocationPaymentInformationSql(CString strPatTempTable, long nPatientID) {
	long nReportID = m_pReport->nID;

	switch (nReportID) {

		//by location
		case 337:
		case 355:
		case 435:
		case 437:
		case 338: //Individual Patient Statement By Location
		case 356:
		
			{
				// (j.gruber 2008-07-01 17:40) - PLID 30322 - make a temp table

				//make a filter for the individual reports 
				CString strPatientFilter, strPatientFilter1;
				if (nReportID == 337 || nReportID == 355 || nReportID == 435 || nReportID == 437) {
					if (!strPatTempTable.IsEmpty()) {
						strPatientFilter.Format(" AND LineItemT.PatientID IN (SELECT ID FROM [%s])", strPatTempTable);
						//(s.dhole 9/11/2015 4:09 PM ) - PLID 66975 added second filter
						strPatientFilter1.Format(" AND LineItemT_1.PatientID IN (SELECT ID FROM [%s])", strPatTempTable);
					}
				}
				else {
					strPatientFilter.Format("AND LineItemT.PatientID = {INT} ", nPatientID);
					//(s.dhole 9/11/2015 4:09 PM ) - PLID 66975 added second filter
					strPatientFilter1.Format("AND LineItemT_1.PatientID = {INT} ", nPatientID);
				}

					
						
				CString strTableName;
				strTableName.Format("#TempStatement%lu", GetTickCount());
				
				// (a.walling 2009-09-08 14:02) - PLID 35178 - Use the snapshot connection
				ExecuteSql(GetConnection(), "CREATE TABLE %s (PatID int, LocID int, LastPayDate datetime, LastPayAmount money, IsInsurance bit) ", strTableName);

				CString strInnerQuery;
                /* (a.levy 2013-11-04 10:43) - PLID  59198 - Removed ProviderID from Group By clause and subQuery that 
                leaves out a payment when a provider is null from the total*/
				// (j.gruber 2011-07-05 11:03) - PLID 44831 - leave in originals since we want the actual payment, not the adjustment
				//TES 7/17/2014 - PLID 62564 - Added code to always hide chargebacks (even if they are not being hidden, we don't want them to count as the "last payment")
				// (r.gonet 2015-05-05 14:38) - PLID 65870 - Exclude Gift Certificate Refunds
				// (r.goldschmidt 2015-11-10 16:35) - PLID 65568 - Exclude voided payments
				// (c.haag 2016-02-11) - PLID 68236 - Split single Chargebacks joins into two joins to avoid producing super slow query plans
				strInnerQuery.Format(
					" SELECT MaxDateQ.PatID, MaxTDate as LastPayDate, Max(SumofAmount) as LastPayAmount, MaxDateQ.LocID, MaxDateQ.IsInsurance FROM  "
					"   (SELECT PatID, Max(TDate) as MaxTDate, LocID, IsInsurance "
					"   FROM ( "
					"   SELECT  StmtLastPayDateQ.ID                                      , "
					"  		 StmtLastPayDateQ.PatientID AS PatID                      , "
					"  		 StmtLastPayDateQ.ProvID AS ProvID                        , "
					"  		 StmtLastPayDateQ.TDate  AS TDate                         , "
					"  		 SUM(StmtLastPayDateQ.Amount) AS SumOfAmount              , "
					"  		 StmtLastPayDateQ.LocID       AS LocID					 ,        "
					"  		 CASE WHEN StmtLastPayDateQ.InsuredPartyID = -1 then 0 else 1 END as IsInsurance "
					"   FROM ( "
					"  		 SELECT * "
					"  		 FROM "
					"  				 (SELECT LineItemT.ID       , "
					"  						 LineItemT.PatientID, "
					"  						 Amount = "
					"  						 CASE "
					"  								  WHEN [_PartiallyAppliedPaysQ].[ID] IS NULL "
					"  								  THEN "
					"  										 CASE "
					"  												WHEN [LineItemT_1].[ID] IS NULL "
					"  												THEN [LineItemT].[Amount] "
					"  												ELSE [AppliesT].[Amount] "
					"  										END "
					"  								ELSE [AppliesT].[Amount] "
					"  						END, "
					"  						ProvID = "
					"  						CASE "
					"  								WHEN [DoctorsProviders] IS NULL "
					"  								THEN [ProviderID] "
					"  								ELSE [DoctorsProviders] "
					"  						END                                                                   , "
					"  						'Full'              AS RandomText                                     , "
					"  						LineItemT.PatientID AS PatID                                          , "
					"  						LineItemT.Date                                             AS TDate   , "
					"  						AppliesT.ID               AS ApplyID                                  , "
					"  						LineItemT.ID              AS LineID                                   , "
					"  						CASE "
					"  								WHEN LineItemT_1.LocationID IS NULL "
					"  								THEN LineItemT.LocationID "
					"  								ELSE LineItemT_1.LocationID "
					"  						END             AS LocID, "
					"  						PaymentsT.InsuredPartyID "
					"  			  FROM (((((((LineItemT "
					"  						LEFT JOIN PaymentsT "
					"  						ON      LineItemT.ID = PaymentsT.ID) "
					"  						LEFT JOIN "
					"  								(SELECT LineItemT_1.ID                  , "
					"  										LineItemT_1.Amount   AS PayAmt  , "
					"  										SUM(AppliesT.Amount) AS ApplyAmt, "
					
					"  										MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount]) AS Total, "
					"  										LineItemT_1.PatientID "
					"  								FROM    LineItemT AS LineItemT_1 "
					"  										LEFT JOIN (PaymentsT "
					"  												LEFT JOIN (AppliesT "
					"  												LEFT JOIN LineItemT "
					"  												ON      AppliesT.DestID = LineItemT.ID) "
					"  										ON      PaymentsT.ID            = AppliesT.SourceID) "
					"										LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"										LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"  								ON      LineItemT_1.ID                  = PaymentsT.ID "
					"  						WHERE (((LineItemT_1.Deleted)                   =0)) "
					"  							AND (PaymentsT.PayMethod                   NOT IN (4,10)) "
					"							AND ChargebacksPayments.ID Is Null "
					"							AND ChargebacksAdjustments.ID Is Null "
					"							%s												 "
					"  						GROUP BY LineItemT_1.ID   , "
					"  								LineItemT_1.Amount, "
					"  										LineItemT_1.PatientID "
					"  								HAVING (((LineItemT_1.ID)                                       IS NOT NULL) "
					"  									AND ((MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount])) <> 0)) "
					"  								) AS _PartiallyAppliedPaysQ ON LineItemT.ID                      = [_PartiallyAppliedPaysQ].ID) "
					"  						LEFT JOIN AppliesT "
					"  						ON      PaymentsT.ID = AppliesT.SourceID "
					"						LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"						LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID) "
					"  						LEFT JOIN LineItemT AS LineItemT_1 "
					"  						ON      AppliesT.DestID = LineItemT_1.ID) "
					"  						LEFT JOIN ChargesT "
					"  						ON      LineItemT_1.ID = ChargesT.ID)                         "
					"						LEFT JOIN LineItemCorrectionsT OriginalLineItemsT "
					"						ON		LineItemT.ID = OriginalLineItemsT.OriginalLineItemID) "
					"						LEFT JOIN LineItemCorrectionsT VoidingLineItemsT "
					"						ON		LineItemT.ID = VoidingLineItemsT.VoidingLineItemID) "
					"  				WHERE (((PaymentsT.ID)                    IS NOT NULL) "
					"  					AND ((LineItemT.Deleted)               =0) "
					"  					AND ((LineItemT.Type)                  =1)) "
					"  					AND (PaymentsT.PayMethod              NOT IN (4,10)) "
					"					AND ChargebacksPayments.ID Is Null "
					"					AND ChargebacksAdjustments.ID Is Null "
					"					AND OriginalLineItemsT.OriginalLineItemID IS NULL "
					"					AND VoidingLineItemsT.VoidingLineItemID IS NULL "
					"					%s											"
					"  				) AS _StmtMaxPayInfoFullQ "
					"           "
					"  		UNION "
					"           "
					"  		SELECT  * "
					"  		FROM "
					"  				(SELECT [_PartiallyAppliedPaysQ].ID                                           , "
					"  						LineItemT.PatientID                                                   , "
					"  						[_PartiallyAppliedPaysQ].Total AS Amount                              , "
					"  						PaymentsT.ProviderID           AS ProvID                              , "
					"  						'Part'                         AS RandomText                          , "
					"  						LineItemT.PatientID            AS PatID                               , "
					"  						LineItemT.Date                                             AS TDate   , "
					"  						0                         AS ApplyID                                  , "
					"  						LineItemT.ID              AS LineID                                   , "
					"  						LineItemT.LocationID      AS LocID                                    , "
					"  						PaymentsT.InsuredPartyID "
					"  				FROM (( "
					"  						(SELECT LineItemT_1.ID                  , "
					"  								LineItemT_1.Amount   AS PayAmt  , "
					"  								SUM(AppliesT.Amount) AS ApplyAmt, "
					
					"  								MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount]) AS Total, "
					"  								LineItemT_1.PatientID "
					"  						FROM    LineItemT AS LineItemT_1 "
					"  								LEFT JOIN (PaymentsT "
					"  										LEFT JOIN (AppliesT "
					"  												LEFT JOIN LineItemT "
					"  												ON      AppliesT.DestID = LineItemT.ID) "
					"  										ON      PaymentsT.ID            = AppliesT.SourceID "
					"										LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"										LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID) "
					"  								ON      LineItemT_1.ID                  = PaymentsT.ID "
					"  						WHERE (((LineItemT_1.Deleted)                   =0)) "
					"  							AND (PaymentsT.PayMethod                   NOT IN (4,10)) "
					"							AND ChargebacksPayments.ID Is Null "
					"							AND ChargebacksAdjustments.ID Is Null "
					"							%s												 "
					"  						GROUP BY LineItemT_1.ID   , "
					"  								LineItemT_1.Amount, "
					"  								LineItemT_1.PatientID "
					"  						HAVING (((LineItemT_1.ID)                                       IS NOT NULL) "
					"  							AND ((MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount])) <> 0)) "
					"  						) AS _PartiallyAppliedPaysQ "
					"  						INNER JOIN LineItemT "
					"  						ON      [_PartiallyAppliedPaysQ].ID  = LineItemT.ID) "
					"  						INNER JOIN PaymentsT "
					"  						ON      LineItemT.ID = PaymentsT.ID"
					"						LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"						LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"						LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
					"						LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID) "
					"                           "
					"  				WHERE (((LineItemT.Deleted)                =0) "
					"  					AND ((LineItemT.Type)                  =1)) "
					"  					AND (PaymentsT.PayMethod              NOT IN (4,10)) "
					"					AND ChargebacksPayments.ID Is Null "
					"					AND ChargebacksAdjustments.ID Is Null "
					"					AND OriginalLineItemsT.OriginalLineItemID IS NULL "
					"					AND VoidingLineItemsT.VoidingLineItemID IS NULL "
					"					%s											"
					"  				) AS _StmtMaxPayInfoPartQ "
					"  		) AS StmtLastPayDateQ        "
					"  GROUP BY StmtLastPayDateQ.ID                          , "
					"  		StmtLastPayDateQ.PatientID                    , "
					"  		StmtLastPayDateQ.ProvID                       , "
					"  		StmtLastPayDateQ.TDate                        , "
					"  		StmtLastPayDateQ.InsuredPartyID                , "
					"  		StmtLastPayDateQ.LocID                         "
					"  )SubQ "
					"  GROUP BY PatID, LocID, IsInsurance) AS MaxDateQ "
					"  LEFT JOIN (SELECT  StmtLastPayDateQ.ID                                      , "
					"  		StmtLastPayDateQ.PatientID AS PatID                      , "
					"  		StmtLastPayDateQ.TDate  AS TDate                         , "
					"  		SUM(StmtLastPayDateQ.Amount) AS SumOfAmount              , "
					"  		StmtLastPayDateQ.LocID       AS LocID					 ,        "
					"  		CASE WHEN StmtLastPayDateQ.InsuredPartyID = -1 then 0 else 1 END as IsInsurance "
					"  FROM ( "
					"  		SELECT * "
					"  		FROM "
					"  				(SELECT LineItemT.ID       , "
					"  						LineItemT.PatientID, "
					"  						Amount = "
					"  						CASE "
					"  								WHEN [_PartiallyAppliedPaysQ].[ID] IS NULL "
					"  								THEN "
					"  										CASE "
					"  												WHEN [LineItemT_1].[ID] IS NULL "
					"  												THEN [LineItemT].[Amount] "
					"  												ELSE [AppliesT].[Amount] "
					"  										END "
					"  								ELSE [AppliesT].[Amount] "
					"  						END, "
					"  						ProvID = "
					"  						CASE "
					"  								WHEN [DoctorsProviders] IS NULL "
					"  								THEN [ProviderID] "
					"  								ELSE [DoctorsProviders] "
					"  						END                                                                   , "
					"  						'Full'              AS RandomText                                     , "
					"  						LineItemT.PatientID AS PatID                                          , "
					"  						LineItemT.Date                                             AS TDate   , "
					"  						AppliesT.ID               AS ApplyID                                  , "
					"  						LineItemT.ID              AS LineID                                   , "
					"  						CASE "
					"  								WHEN LineItemT_1.LocationID IS NULL "
					"  								THEN LineItemT.LocationID "
					"  								ELSE LineItemT_1.LocationID "
					"  						END             AS LocID, "
					"  						PaymentsT.InsuredPartyID "
					"  			  FROM (((((LineItemT "
					"  						LEFT JOIN PaymentsT "
					"  						ON      LineItemT.ID = PaymentsT.ID) "
					"  						LEFT JOIN "
					"  								(SELECT LineItemT_1.ID                  , "
					"  										LineItemT_1.Amount   AS PayAmt  , "
					"  										SUM(AppliesT.Amount) AS ApplyAmt, "
					
					"  										MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount]) AS Total, "
					"  										LineItemT_1.PatientID "
					"  								FROM    LineItemT AS LineItemT_1 "
					"  										LEFT JOIN (PaymentsT "
					"  												LEFT JOIN (AppliesT "
					"  														LEFT JOIN LineItemT "
					"  														ON      AppliesT.DestID = LineItemT.ID) "
					"  												ON      PaymentsT.ID            = AppliesT.SourceID "
					"												LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"												LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID) "
					"  										ON      LineItemT_1.ID                  = PaymentsT.ID "
					"  								WHERE (((LineItemT_1.Deleted)                   =0)) "
					"  									AND (PaymentsT.PayMethod                   NOT IN (4,10)) "
					"									AND ChargebacksPayments.ID Is Null "
					"									AND ChargebacksAdjustments.ID Is Null "
					"									%s							 					"
					"  								GROUP BY LineItemT_1.ID   , "
					"  										LineItemT_1.Amount, "
					"  										LineItemT_1.PatientID "
					"  								HAVING (((LineItemT_1.ID)                                       IS NOT NULL) "
					"  									AND ((MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount])) <> 0)) "
					"  								) AS _PartiallyAppliedPaysQ ON LineItemT.ID                      = [_PartiallyAppliedPaysQ].ID) "
					"  						LEFT JOIN AppliesT "
					"  						ON      PaymentsT.ID = AppliesT.SourceID "
					"						LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"						LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"						LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
					"						LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID) "
					"  						LEFT JOIN LineItemT AS LineItemT_1 "
					"  						ON      AppliesT.DestID = LineItemT_1.ID) "
					"  						LEFT JOIN ChargesT "
					"  						ON      LineItemT_1.ID = ChargesT.ID)                         "
					"  				WHERE (((PaymentsT.ID)                    IS NOT NULL) "
					"  					AND ((LineItemT.Deleted)               =0) "
					"  					AND ((LineItemT.Type)                  =1)) "
					"  					AND (PaymentsT.PayMethod              NOT IN (4,10)) "
					"					AND ChargebacksPayments.ID Is Null "
					"					AND ChargebacksAdjustments.ID Is Null "
					"					AND OriginalLineItemsT.OriginalLineItemID IS NULL "
					"					AND VoidingLineItemsT.VoidingLineItemID IS NULL "
					"					%s											"
					"  				) AS _StmtMaxPayInfoFullQ "
					"           "
					"  		UNION "
					"           "
					"  		SELECT  * "
					"  		FROM "
					"  				(SELECT [_PartiallyAppliedPaysQ].ID                                           , "
					"  						LineItemT.PatientID                                                   , "
					"  						[_PartiallyAppliedPaysQ].Total AS Amount                              , "
					"  						PaymentsT.ProviderID           AS ProvID                              , "
					"  						'Part'                         AS RandomText                          , "
					"  						LineItemT.PatientID            AS PatID                               , "
					"  						LineItemT.Date                                             AS TDate   , "
					"  						0                         AS ApplyID                                  , "
					"  						LineItemT.ID              AS LineID                                   , "
					"  						LineItemT.LocationID      AS LocID                                    , "
					"  						PaymentsT.InsuredPartyID "
					"  				FROM (( "
					"  						(SELECT LineItemT_1.ID                  , "
					"  								LineItemT_1.Amount   AS PayAmt  , "
					"  								SUM(AppliesT.Amount) AS ApplyAmt, "
					
					"  								MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount]) AS Total, "
					"  								LineItemT_1.PatientID "
					"  						FROM    LineItemT AS LineItemT_1 "
					"  								LEFT JOIN (PaymentsT "
					"  										LEFT JOIN (AppliesT "
					"  												LEFT JOIN LineItemT "
					"  												ON      AppliesT.DestID = LineItemT.ID) "
					"  										ON      PaymentsT.ID            = AppliesT.SourceID "
					"										LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"										LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID) "
					"  								ON      LineItemT_1.ID                  = PaymentsT.ID "
					"  						WHERE (((LineItemT_1.Deleted)                   =0)) "
					"  							AND (PaymentsT.PayMethod                   NOT IN (4,10)) "
					"							AND ChargebacksPayments.ID Is Null "
					"							AND ChargebacksAdjustments.ID Is Null "
					"							%s												 "
					"  						GROUP BY LineItemT_1.ID   , "
					"  								LineItemT_1.Amount, "
					"  								LineItemT_1.PatientID "
					"  						HAVING (((LineItemT_1.ID)                                       IS NOT NULL) "
					"  							AND ((MIN([LineItemT_1].[Amount])-SUM([AppliesT].[Amount])) <> 0)) "
					"  						) AS _PartiallyAppliedPaysQ "
					"  						INNER JOIN LineItemT "
					"  						ON      [_PartiallyAppliedPaysQ].ID  = LineItemT.ID) "
					"  						INNER JOIN PaymentsT "
					"  						ON      LineItemT.ID = PaymentsT.ID "
					"						LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
					"						LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
					"						LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
					"						LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID) "
					"                           "
					"  				WHERE (((LineItemT.Deleted)                =0) "
					"  					AND ((LineItemT.Type)                  =1)) "
					"  					AND (PaymentsT.PayMethod              NOT IN (4,10)) "
					"					AND ChargebacksPayments.ID Is Null "
					"					AND ChargebacksAdjustments.ID Is Null "
					"					AND OriginalLineItemsT.OriginalLineItemID IS NULL "
					"					AND VoidingLineItemsT.VoidingLineItemID IS NULL "
					"					%s											"
					"  				) AS _StmtMaxPayInfoPartQ "
					"  		) AS StmtLastPayDateQ        "
					"  GROUP BY StmtLastPayDateQ.ID                          , "
					"  		StmtLastPayDateQ.PatientID                    , "
					"  		StmtLastPayDateQ.TDate                        , "
					"  		StmtLastPayDateQ.InsuredPartyID                , "
					"  		StmtLastPayDateQ.LocID       ) OtherInfoQ "
					"  ON MaxDateQ.PatID = OtherInfoQ.PatID  "
					"  AND MaxDateQ.MaxTDate = OtherInfoQ.TDate "
					"  AND MaxDateQ.LocID = OtherInfoQ.LocID "
					"  AND MaxDateQ.IsInsurance = OtherInfoQ.IsInsurance "
					"  GROUP BY MaxDateQ.PatID, MaxTDate, MaxDateQ.LocID, MaxDateQ.IsInsurance "
					, strPatientFilter1, strPatientFilter, strPatientFilter1, strPatientFilter,
					strPatientFilter1, strPatientFilter, strPatientFilter1, strPatientFilter); //(s.dhole 9/11/2015 4:09 PM ) - PLID 66975 added second filter

					CString strQuery;
					strQuery.Format("INSERT INTO %s (PatID, LocID, LastPayDate, LastPayAmount, IsInsurance) "
					" SELECT PatID, LocID, LastPayDate, LastPayAmount, IsInsurance FROM (%s) StmtTempQ ", strTableName, strInnerQuery);
#ifdef _DEBUG	
	MsgBox(strQuery);
#endif

				// (a.walling 2009-09-08 14:02) - PLID 35178 - Use the snapshot connection
				if (nReportID == 337 || nReportID == 355 || nReportID == 435 || nReportID == 437) {
					ExecuteSql(GetConnection(), strQuery);
				}
				else {
					ExecuteParamSql(GetConnection(), strQuery, nPatientID, nPatientID, nPatientID, nPatientID, 
											  nPatientID, nPatientID, nPatientID, nPatientID);
				}

					

				return strTableName;

			}
		break;


	



		default:
			return "";
		break;
	}


}

CString CStatementSqlBuilder::GetStatementByLocationSql(CString strRespFilter) {

	switch (m_pReport->nID) {

		case 337 :
		case 355:
		case 435: //by last send date
		case 437: //by last send date 7.0
		
			{
			long nTransFormat, nHeaderFormat;
			nTransFormat = GetRemotePropertyInt("SttmntTransProvFormat", 0, 0, "<None>");
			nHeaderFormat = GetRemotePropertyInt("SttmntHeaderProvFormat", 0, 0, "<None>");
			CString strTransProv = GetStatementProvString(nTransFormat);
			CString strHeaderProv = GetStatementProvString(nHeaderFormat);
			CString strHideUnAppliedPrepayments = GetStatementUnAppliedPrePaymentsString();
			CString strChargeDescription = GetStatementChargeDescription();

			//PLID 19451: create the temp table to filter with
			CString strInnerFilter = "", strInnerFilterApp = "";
			CString strTmpTable;
			if (!m_pReport->GetFilter(m_nSubLevel, m_nSubRepNum).IsEmpty())  {
				strTmpTable = CreateStatementFilterTable();
				strInnerFilter.Format(" AND LineItemT.PatientID IN (SELECT ID FROM %s) ", strTmpTable);
				strInnerFilterApp.Format(" AND LineItemT1.PatientID IN (SELECT ID FROM %s) ", strTmpTable);

				//if we got here and have no tmp table, that means we got an error in the last function
				if (strTmpTable.IsEmpty()) {
					return "";
				}
			}
			else {
				//we have no filters, so running the extra part isn't going to help us, so just skip it
			}

			// (j.gruber 2008-07-01 16:16) - PLID 30322 - make a table variable
			CString strTempTableName;
			strTempTableName = GetLastLocationPaymentInformationSql(strTmpTable, m_pReport->nPatient);


			// (j.gruber 2007-05-01 17:11) - PLID 25745 - only show last 4 digits of cc number
			// (j.gruber 2007-05-15 09:08) - PLID 25987 - take out credit card expiration dates
			// (j.gruber 2007-05-30 12:26) - PLID 25978 - adding discount fields and discount categories
			// (e.lally 2007-07-13) PLID 26670 - Updated all references to PaymentPlansT. CCType with link to CardName, aliased as CCType where applicable.
			// (j.gruber 2008-06-09 11:21) - PLID 30323 - changed the statement by location to only look at the last sent from each location
			// (j.gruber 2008-07-02 15:26) - PLID 30322 - made the last payment amounts and dates pull per location
			// (j.jones 2008-09-05 10:24) - PLID 30288 - supported MailSentNotesT
			// (j.gruber 2009-11-05 17:40) - PLID 36217 - added provider NPI
			// (j.gruber 2009-11-25 12:29) - PLID 36430 - added cpt code
			// (j.gruber 2009-12-24 13:02) - PLID 17122 - added CPT Modifier
			// (j.gruber 2010-06-14 15:54) - PLID 36484 - billing notes
			// (j.gruber 2011-07-05 11:03) - PLID 44831 - take out originals and voids
			// (a.wilson 2012-2-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
			// (j.gruber 2014-03-04 11:13) - PLID 61155 - update for ICD10, diag and whichcodes redo
			//TES 7/17/2014 - PLID 62935 - Added code to hide chargebacks when requested
			// (j.jones 2015-03-09 09:48) - PLID 64937 - if the description begins with 'Corrected Charge',
			// 'Corrected Payment', etc., strip that off
			// (c.haag 2016-02-11) - PLID 68236 - Split single Chargebacks joins into two joins to avoid producing super slow query plans
			CString part1, part2, part3, part4, part5, part6, part7, part8, part8b, part9, part10, part11, part12,  part13, part14,part15, part16, part17;
			part1 = _T(" SELECT StatementSubQ.ID, StatementSubQ.PatientID, StatementSubQ.PatID as PatID, StatementSubQ.Type, StatementSubQ.Total, StatementSubQ.Description, StatementSubQ.Date as TDate,  "
			" StatementSubQ.Insurance, StatementSubQ.Last, StatementSubQ.First, StatementSubQ.Middle, StatementSubQ.Address1, StatementSubQ.Address2, "
			" StatementSubQ.City, StatementSubQ.State, StatementSubQ.Zip, StatementSubQ.PatForward, StatementSubQ.PatComma, StatementSubQ.DocName, StatementSubQ.DocAddress1, StatementSubQ.DocAddress2, StatementSubQ.DocCity, StatementSubQ.DocState, StatementSubQ.DocZip, StatementSubQ.ProvTaxID, StatementLocQ.PracName, StatementLocQ.PracAddress1, "
			" StatementLocQ.PracAddress2, StatementLocQ.PracCity, StatementLocQ.PracState, StatementLocQ.PracZip, StatementLocQ.PracPhone, StatementLocQ.PracFax, StatementSubQ.ProvID as ProvID2, StatementSubQ.BillId, "
			" StatementSubQ.BillDate as BillDate, StatementSubQ.BillDescription, StatementSubQ.Birthdate, \r\n "
			" StatementSubQ.ICD9Code1, StatementSubQ.ICD9Code2, StatementSubQ.ICD9Code3, StatementSubQ.ICD9Code4, \r\n"
			" StatementSubQ.ICD10Code1, StatementSubQ.ICD10Code2, StatementSubQ.ICD10Code3, StatementSubQ.ICD10Code4, \r\n"
			" StatementSubQ.WhichCodes9, StatementSubQ.WhichCodes10, StatementSubQ.WhichCodesBoth, \r\n"
			" StatementSubQ.Location, StatementSubQ.StatementType, StatementSubQ.GroupFixID, StatementSubQ.Appdate, StatementSubQ.StartTime, StatementSubQ.ARDate, StatementSubQ.Age,  "
			" StatementSubQ.TransProv, StatementSubQ.Prepayment, StatementSubQ.Quantity, StatementSubQ.Thirty, StatementSubQ.Sixty, StatementSubQ.Ninety, StatementSubQ.NinetyPlus, "
			" StatementSubQ.ProvEIN, StatementSubQ.ProvLicense, StatementSubQ.ProvUPIN, StatementSubQ.ProvDEA, StatementSubQ.ProvBCBS, StatementSubQ.ProvMedicare, StatementSubQ.ProvMedicaid, StatementSubQ.ProvWorkersComp, StatementSubQ.ProvOtherID, StatementSubQ.ProvOtherIDDesc, "
			" StatementEndQ.DocName as DocName2, StatementEndQ.ProvID as ProvID, StatementEndQ.SuppressStatement, StatementEndQ.PatID as PatID2, StatementEndQ.StatementNote, StatementEndQ.PriInsCo, StatementEndQ.PriGuarForward, StatementEndQ.PriGuarComma, "
			" StatementEndQ.PriInsFirst, StatementEndQ.PriInsMiddle, StatementEndQ.PriInsLast, StatementEndQ.PersonID, StatementEndQ.SecInsCo, StatementEndQ.SecGuarForward, StatementEndQ.SecGuarComma, StatementEndQ.SecInsfirst, StatementEndQ.SecInsMiddle, "
			" StatementEndQ.SecInsLast, StatementEndQ.PersID, "
			" StatementSubQ.LocationFixID AS LocID,  "
			" StatementEndQ.RespID, StatementEndQ.RespFirst, StatementEndQ.RespMiddle, StatementEndQ.RespLast, StatementEndQ.RespAdd1, StatementEndQ.RespAdd2, StatementEndQ.RespCity, "
			" StatementEndQ.RespState, StatementEndQ.RespZip, "
			" (SELECT Max(Date) FROM MailSent INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
			"	WHERE (MailSentNotesT.Note Like '%%Patient Statement%%Printed%%' OR MailSentNotesT.Note Like '%%Patient Statement%%Run%%' OR MailSentNotesT.Note Like '%%E-Statement%%Exported%%') AND PersonID = StatementSubQ.PatID) AS LastSentDate, StatementSubQ.PatPhone, "
			" StatementSubQ.FullChargeNoTax, StatementSubQ.ChargeTax1, StatementSubQ.ChargeTax2, StatementSubQ.TaxRate1, StatementSubQ.TaxRate2, "
			" (SELECT COUNT(*) FROM ResponsiblePartyT WHERE PatientID = StatementSubQ.PatID) AS RespPartyCount, "			
			" StatementLastPatPayInfoQ.LastPayDate as LastPatientPaymentDate, StatementLastInsPayInfoQ.LastPayDate as LastInsurancePaymentDate, "
			" StatementLastPatPayInfoQ.LastPayAmount as LastPatientPaymentAmount, StatementLastInsPayInfoQ.LastPayAmount as LastInsurancePaymentAmount, "
			" StatementSubQ.CCType, CASE WHEN Len(StatementSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(StatementSubQ.CCNumber, 4) END as CCNumber, StatementSubQ.CheckNo, StatementSubQ.BankNo, StatementSubQ.CheckAcctNo, "
			" StatementSubQ.CCHoldersName, Convert(DateTime, NULL) AS CCExpDate, StatementSubQ.CCAuthNo, StatementSubQ.BankRoutingNum, " 
			" StatementEndQ.PatCoordFirst, StatementEndQ.PatCoordMiddle, StatementEndQ.PatCoordLast, "
			" StatementSubQ.PercentOff, StatementSubQ.DiscountAmt, StatementSubQ.DiscountCategoryDesc, StatementEndQ.ProviderNPI, StatementSubQ.CPTCode, "
			" StatementSubQ.CPTModifier1, StatementSubQ.CPTModifier2, StatementSubQ.CPTModifier3, StatementSubQ.CPTModifier4, "
			" StatementSubQ.BillStatementNote, StatementSubQ.LineItemStatementNote, "
			" CASE WHEN StatementSubQ.BillStatementNote = '' OR StatementSubQ.BillStatementNote IS NULL THEN 1 ELSE 0 END as SuppressBillStatementNote, "
			" CASE WHEN StatementSubQ.LineItemStatementNote = '' OR StatementSubQ.LineItemStatementNote IS NULL  THEN 1 ELSE 0 END as SuppressLineItemStatementNote "
			"FROM (SELECT StatementAllData.*, MIN(NextApp.AppDate)  "
			"  AS AppDate, MIN(NextApp.StartTime) AS StartTime,  "
			"  LineItemT.Date AS ARDate,  "
			"  CASE WHEN StatementAR.Thirty IS NULL THEN 0 ELSE StatementAR.Thirty END AS Thirty, "
			"  CASE WHEN StatementAR.Sixty IS NULL THEN 0 ELSE StatementAR.Sixty END AS Sixty, "
			"  CASE WHEN StatementAR.Ninety IS NULL THEN 0 ELSE StatementAR.Ninety END AS Ninety, "
			"  CASE WHEN StatementAR.NinetyPlus IS NULL THEN 0 ELSE StatementAR.NinetyPlus END AS NinetyPlus, "
			"  Age = CASE WHEN StatementAllData.BirthDate IS NULL "
			"   THEN - 1 ELSE DATEDIFF(YYYY, StatementAllData.Birthdate, GetDate()) -  "
			"		CASE WHEN MONTH(StatementAllData.Birthdate) > MONTH(GetDate()) OR (MONTH(StatementAllData.Birthdate) = MONTH(GetDate()) AND DAY(StatementAllData.Birthdate) > DAY(GetDate())) "
			"			THEN 1 ELSE 0 END "
			"  END "
			"  FROM (SELECT StmtCharges.ID, StmtCharges.PatientID,  "
			"  StmtCharges.PatID, StmtCharges.Type,  "
			"  StmtCharges.Total, StmtCharges.Description,  "
			"  StmtCharges.Date, StmtCharges.Insurance,  "
			"  StmtCharges.Last, StmtCharges.First,  "
			"  StmtCharges.Middle, StmtCharges.Address1,  "
			"  StmtCharges.Address2, StmtCharges.City,  "
			"  StmtCharges.State, StmtCharges.Zip,  "
			"  StmtCharges.PatForward,  "
			"  StmtCharges.PatComma,  "
			"  StmtCharges.DocName,  "
			"  StmtCharges.DocAddress1, "
			"  StmtCharges.DocAddress2, "
			"  StmtCharges.DocCity, "
			"  StmtCharges.DocState, "
			"  StmtCharges.DocZip, "
			"  StmtCharges.PracName,  "
			"  StmtCharges.PracAddress1,  "
			"  StmtCharges.PracAddress2,  "
			"  StmtCharges.PracCity, StmtCharges.PracState,  "
			"  StmtCharges.PracZip, StmtCharges.PracPhone,  "
			"  StmtCharges.PracFax, StmtCharges.ProvID,  "
			"  StmtCharges.BillID, StmtCharges.BillDate,  "
			"  StmtCharges.BillDescription,  "
			"  StmtCharges.BirthDate, \r\n "
			"  StmtCharges.ICD9Code1,  \r\n "
			"  StmtCharges.ICD9Code2,  \r\n"
			"  StmtCharges.ICD9Code3,  \r\n"
			"  StmtCharges.ICD9Code4,  \r\n"
			"  StmtCharges.ICD10Code1,  \r\n "
			"  StmtCharges.ICD10Code2,  \r\n"
			"  StmtCharges.ICD10Code3,  \r\n"
			"  StmtCharges.ICD10Code4,  \r\n"
			"  StmtCharges.WhichCodes9,  \r\n"
			"  StmtCharges.WhichCodes10,  \r\n"
			"  StmtCharges.WhichCodesBoth,  \r\n"
			"  StmtCharges.Location, "
			"  1 as StatementType, -1 as GroupFixID, "
			"  StmtCharges.LocationFixID, "
			"  StmtCharges.ProvTaxID,  "
			"  StmtCharges.TransProv, StmtCharges.Prepayment,  "
			"  StmtCharges.ProvEIN, StmtCharges.ProvLicense, StmtCharges.ProvUPIN, StmtCharges.ProvDEA, StmtCharges.ProvBCBS, StmtCharges.ProvMedicare, StmtCharges.ProvMedicaid, StmtCharges.ProvWorkersComp, StmtCharges.ProvOtherID, StmtCharges.ProvOtherIDDesc, StmtCharges.HomePhone AS PatPhone, "
			"  StmtCharges.FullChargeNoTax, StmtCharges.ChargeTax1, StmtCharges.ChargeTax2, StmtCharges.TaxRate1, StmtCharges.TaxRate2, StmtCharges.Quantity, "
			" '' as CCType, '' AS CCNumber,'' AS CheckNo, '' AS BankNo,  '' AS CheckAcctNo, "
			" '' AS CCHoldersName, NULL AS CCExpDate, '' AS CCAuthNo, '' AS BankRoutingNum, "
			"  StmtCharges.PercentOff, StmtCharges.DiscountAmt, StmtCharges.DiscountCategoryDesc, StmtCharges.CPTCode, "
			" StmtCharges.CPTModifier1, StmtCharges.CPTModifier2, StmtCharges.CPTModifier3, StmtCharges.CPTModifier4, "
			" StmtCharges.BillStatementNote, StmtCharges.LineItemStatementNote ");
			part2.Format("  FROM (SELECT LineItemT.ID,  "
			" PatientsT.UserDefinedID AS PatientID,  "
			" LineItemT.PatientID AS PatID,  "
			" LineItemT.Type,  "
			" Total = CASE WHEN SUM(ChargeRespT.Amount) "
			"  IS NULL  "
			" THEN 0 ELSE SUM(ChargeRespT.Amount) "
			"  END,  "
			" Description = %s,"
			" LineItemT.Date,  "
			" Insurance = SUM(CASE WHEN ChargeRespT.InsuredPartyID "
			"  IS NOT NULL AND ChargeRespT.InsuredPartyID <> -1  "
			" THEN ChargeRespT.Amount ELSE 0 END), "
			"  PersonT.Last, PersonT.Middle,  "
			" PersonT.First, PersonT.Address1,  "
			" PersonT.Address2, PersonT.City,  "
			" PersonT.State, PersonT.Zip, PersonT.HomePhone,  "
			" LocationsT.Name PracName,  "
			" LocationsT.Address1 PracAddress1,  "
			" LocationsT.Address2 AS PracAddress2,  "
			" LocationsT.City PracCity,  "
			" LocationsT.State PracState,  "
			" LocationsT.Zip AS PracZip,  "
			" LocationsT.Phone AS PracPhone,  "
			" LocationsT.Fax AS PracFax,  "
			" ProvidersT.PersonID AS ProvID,  "
			" (%s WHERE PersonT.ID = PersonT1.ID)  AS DocName,  "
			" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
			" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
			" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
			"  AS PatComma,  "
			" PersonT.First + ' ' + PersonT.Middle + ' ' + "
			"  PersonT.Last AS PatForward,  "
			" BillsT.ID AS BillID,  "
			" BillsT.Date AS BillDate,  "
			" BillsT.Description AS BillDescription,  "
			" PersonT.BirthDate, \r\n"
			" ICD9T1.CodeNumber AS ICD9Code1,  \r\n"
			" ICD9T2.CodeNumber AS ICD9Code2,  \r\n"
			" ICD9T3.CodeNumber AS ICD9Code3,  \r\n"
			" ICD9T4.CodeNumber AS ICD9Code4,  \r\n"
			" ICD10T1.CodeNumber AS ICD10Code1,  \r\n"
			" ICD10T2.CodeNumber AS ICD10Code2,  \r\n"
			" ICD10T3.CodeNumber AS ICD10Code3,  \r\n"
			" ICD10T4.CodeNumber AS ICD10Code4,  \r\n"
			" WhichCodesQ.WhichCodes9,  \r\n"
			" WhichCodesQ.WhichCodes10,  \r\n"
			" WhichCodesQ.WhichCodesBoth,  \r\n"
			" PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
			" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
			" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2)  AS FullChargeNoTax,   "
			" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate) - 1) AS ChargeTax1, "
			" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate2) - 1) AS ChargeTax2, "
			" Min(ChargesT.TaxRate) AS TaxRate1, Min(ChargesT.TaxRate2) AS TaxRate2, "
			" (%s  WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 As Prepayment, ChargesT.Quantity, "
			" TotalPercentOff as PercentOff, TotalDiscount as DiscountAmt, "
			" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDesc, CPTCodeT.Code as CPTCode, ChargesT.CPTModifier as CPTModifier1, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, "
			" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
			" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = ChargesT.ID) AS LineItemStatementNote "
			, strChargeDescription, strHeaderProv, strTransProv);
			part3.Format(" FROM LineItemT LEFT OUTER JOIN "
			" PatientsT LEFT OUTER JOIN "
			" ProvidersT ON  "
			" PatientsT.MainPhysician = ProvidersT.PersonID "
			"  LEFT OUTER JOIN "
			" PersonT PersonT1 ON  "
			" ProvidersT.PersonID = PersonT1.ID LEFT OUTER "
			"  JOIN "
			" LocationsT RIGHT OUTER JOIN "
			" PersonT ON  "
			" LocationsT.ID = PersonT.Location ON  "
			" PatientsT.PersonID = PersonT.ID ON  "
			" LineItemT.PatientID = PatientsT.PersonID LEFT "
			"  OUTER JOIN "
			" BillsT RIGHT OUTER JOIN "
			" ServiceT LEFT OUTER JOIN "
			" CPTCodeT ON  "
			" ServiceT.ID = CPTCodeT.ID RIGHT OUTER "
			"  JOIN "
			" ChargesT ON  "
			" ServiceT.ID = ChargesT.ServiceID ON  "
			" BillsT.ID = ChargesT.BillID LEFT OUTER JOIN "
			" ChargeRespT ON  "
			" ChargesT.ID = ChargeRespT.ChargeID ON "
			"  LineItemT.ID = ChargesT.ID "
			" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewCharge ON ChargesT.ID = LineItemCorrectionsT_NewCharge.NewLineItemID \r\n"
			"  LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID  \r\n "

			" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
			" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
						
			" LEFT JOIN  \r\n"
			" (SELECT ChargesT.ID as ChargeID, "
			" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
			" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
			" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
			" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
			" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
			" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
			" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
			" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
			" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
			" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
			" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
			" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
			" STUFF((SELECT ', ' +  \r\n "
			" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
				 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
				 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
			" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
			" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
			" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
			" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
			" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
			" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
			" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
			" FROM ChargesT \r\n "
			" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "

			" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
			" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
			" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
			" WHERE (PatientsT.PersonID > 0) AND  "
			" (LineItemT.Deleted = 0) AND  "
			" (BillsT.Deleted = 0) AND  "
			" BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND "
			" (LineItemT.Type = 10) "
			" %s	"
			" GROUP BY LineItemT.ID,  "
			" PatientsT.UserDefinedID,  "
			" LineItemT.PatientID, LineItemT.Type,  "
			" CPTCodeT.Code, LineItemT.Description, LineItemCorrectionsT_NewCharge.NewLineItemID,  "
			" LineItemT.Date, PersonT.Last,  "
			" PersonT.Middle, PersonT.First,  "
			" PersonT.Address1, PersonT.Address2,  "
			" PersonT.City, PersonT.State,  "
			" PersonT.Zip, PersonT.HomePhone, LocationsT.Name,  "
			" LocationsT.Address1,  "
			" LocationsT.Address2, LocationsT.City,  "
			" LocationsT.State, LocationsT.Zip,  "
			" LocationsT.Phone, LocationsT.Fax,  "
			" ProvidersT.PersonID, PersonT1.Last,  "
			" PersonT1.First, PersonT1.Middle, PersonT1.Title,   "
			" BillsT.ID, BillsT.Date, BillsT.Description,  "
			" PersonT.BirthDate, \r\n "
			" ICD9T1.CodeNumber,  \r\n "
			" ICD9T2.CodeNumber,  \r\n"
			" ICD9T3.CodeNumber,  \r\n"
			" ICD9T4.CodeNumber,  \r\n"
			" ICD10T1.CodeNumber,  \r\n "
			" ICD10T2.CodeNumber,  \r\n"
			" ICD10T3.CodeNumber,  \r\n"
			" ICD10T4.CodeNumber,  \r\n"
			" WhichCodesQ.WhichCodes9,  \r\n"
			" WhichCodesQ.WhichCodes10,  \r\n"
			" WhichCodesQ.WhichCodesBoth,  \r\n"
			" PersonT.Location, LineItemT.LocationID, [ProvidersT].[Fed Employer ID], ChargesT.DoctorsProviders, "
			" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
			" PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PersonT1.ID, ChargesT.Quantity, TotalPercentOff, TotalDiscount, ChargesT.ID, CPTCodeT.Code, LineItemCorrectionsT_NewCharge.NewLineItemID, "
			" ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4 "
			")  "
			"  AS StmtCharges ", strInnerFilter);
			part4 = _T("UNION "
			" SELECT StmtPays.ID, StmtPays.PatientID,  "
			"  StmtPays.PatID, StmtPays.Type,  "
			"  StmtPays.UnAppliedAmount,  "
			"  StmtPays.Description, StmtPays.Date,  "
			"  StmtPays.Insurance, StmtPays.Last,  "
			"  StmtPays.First, StmtPays.Middle,  "
			"  StmtPays.Address1, StmtPays.Address2,  "
			"  StmtPays.City, StmtPays.State, StmtPays.Zip,  "
			"  StmtPays.PatForward, StmtPays.PatComma,  "
			"  StmtPays.DocName, "
			"  StmtPays.DocAddress1, StmtPays.DocAddress2, StmtPays.DocCity, StmtPays.DocState, StmtPays.DocZip, "
			"  StmtPays.PracName,  "
			"  StmtPays.PracAddress1,  "
			"  StmtPays.PracAddress2, StmtPays.PracCity,  "
			"  StmtPays.PracState, StmtPays.PracZip,  "
			"  StmtPays.PracPhone, StmtPays.PracFax,  "
			"  StmtPays.ProvID, StmtPays.BillID,  "
			"  StmtPays.BillDate, StmtPays.BillDescription,  "
			"  StmtPays.BirthDate, \r\n "
			"  StmtPays.ICD9Code1,  \r\n"
			"  StmtPays.ICD9Code2, \r\n"
			"  StmtPays.ICD9Code3,  \r\n"
			"  StmtPays.ICD9Code4, \r\n"
			"  StmtPays.ICD10Code1,  \r\n"
			"  StmtPays.ICD10Code2, \r\n"
			"  StmtPays.ICD10Code3,  \r\n"
			"  StmtPays.ICD10Code4, \r\n"
			"  StmtPays.WhichCodes9,  \r\n"
			"  StmtPays.WhichCodes10,  \r\n"
			"  StmtPays.WhichCodesBoth,  \r\n"
			"  StmtPays.Location, "
			"  2 as StatementType, -2 as GroupFixID,  "
			"  StmtPays.LocationFixID, "
			"  StmtPays.ProvTaxID,  "
			"  StmtPays.TransProv, StmtPays.Prepayment,  "
			"  StmtPays.ProvEIN, StmtPays.ProvLicense, StmtPays.ProvUPIN, StmtPays.ProvDEA, StmtPays.ProvBCBS, StmtPays.ProvMedicare, StmtPays.ProvMedicaid, StmtPays.ProvWorkersComp, StmtPays.ProvOtherID, StmtPays.ProvOtherIDDesc, StmtPays.HomePhone AS PatPhone, " 	
			"  0 As FullChargeNoTax, 0 AS ChargeTax1, 0 As ChargeTax2, 0 As TaxRate1, 0 As TaxRate2, 0 As Quantity, "
			" StmtPays.CCType, StmtPays.CCNumber, StmtPays.CheckNo, StmtPays.BankNo, StmtPays.CheckAcctNo, "
			" StmtPays.CCHoldersName, StmtPays.CCExpDate, StmtPays.CCAuthNo, StmtPays.BankRoutingNum, " 
			" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, '' as CPTCode, "
			" '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, " 
			" StmtPays.BillStatementNote, "
			" StmtPays.LineItemStatementNote "
			" FROM (SELECT LineItemT.ID,  "
			" Insurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) "
			"  > 0 THEN CASE WHEN [LineItemT].[Type] "
			"  = 3 THEN MIN([LineItemT].[Amount])  "
			" - SUM(CASE WHEN [AppliesT].[Amount] IS "
			"  NULL  "
			" THEN 0 ELSE [AppliesT].[Amount] END)  "
			" ELSE MIN([LineItemT].[Amount])  "
			" - SUM(CASE WHEN [AppliesT].[Amount] IS "
			"  NULL  "
			" THEN 0 ELSE [AppliesT].[Amount] END)  "
			" END ELSE 0 END, PersonT.Last,  "
			" PersonT.First, PersonT.Middle,  "
			" PersonT.Address1, PersonT.Address2,  "
			" PersonT.City, PersonT.State,  "
			" PersonT.Zip, PersonT.HomePhone, "
			" LocationsT.Address1 AS PracAddress1,  "
			" LocationsT.Address2 AS PracAddress2,  "
			" LocationsT.Name AS PracName,  "
			" LocationsT.City AS PracCity,  "
			" LocationsT.State AS PracState,  "
			" LocationsT.Zip AS PracZip,  "
			" LocationsT.Phone AS PracPhone,  "
			" LocationsT.Fax AS PracFax,  ");
			part5.Format(" PatientsT.UserDefinedID AS PatientID,  "
			" PatientsT.PersonID AS PatID,  "
			" ProvidersT.PersonID AS ProvID,  "
			" (%s WHERE PersonT.ID = PersonT1.ID) AS DocName,  "
			" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
			" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
			" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
			"  AS PatComma,  "
			" PersonT.First + ' ' + PersonT.Middle + ' ' + "
			"  PersonT.Last AS PatForward,  "
			" LineItemT.Type,  "
			" UnAppliedAmount = CASE WHEN [LineItemT].[Type] "
			"  = 3 THEN MIN([LineItemT].[Amount])  "
			" - SUM(CASE WHEN [AppliesT].[Amount] IS "
			"  NULL  "
			" THEN 0 ELSE [AppliesT].[Amount] END)  "
			" ELSE MIN([LineItemT].[Amount])  "
			" - SUM(CASE WHEN [AppliesT].[Amount] IS "
			"  NULL  "
			" THEN 0 ELSE [AppliesT].[Amount] END)  "
			" END, "
			" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
			"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
			"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
			"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
			" ELSE LineItemT.Description END) AS Description, "
			" LineItemT.Date, LineItemT.ID AS BillID,  "
			" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
			"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
			"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
			"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
			"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
			" LineItemT.Date AS BillDate,  "
			" PersonT.BirthDate, \r\n"
			" '' AS ICD9Code1,  \r\n"
			" '' AS ICD9Code2, \r\n"
			" '' AS ICD9Code3,  \r\n"
			" '' AS ICD9Code4, \r\n"
			" '' AS ICD10Code1,  \r\n"
			" '' AS ICD10Code2, \r\n"
			" '' AS ICD10Code3,  \r\n"
			" '' AS ICD10Code4, \r\n"
			" '' AS WhichCodes9,  \r\n"
			" '' AS WhichCodes10,  \r\n"
			" '' AS WhichCodesBoth,  \r\n"
			" PersonT.Location, "
			" LineItemT.LocationID as LocationFixID, "
			" [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
			" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
			" (%s WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, PaymentsT.Prepayment As Prepayment, "
			" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
			" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
			" '' AS BillStatementNote, "
			" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
			" FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
			" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
			" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID  "
			" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID  "
			" LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
			" LEFT JOIN LineItemCorrectionsT ON PaymentsT.ID = LineItemCorrectionsT.OriginalLineItemID "
			" LEFT JOIN LineItemCorrectionsT VoidT ON PaymentsT.ID = VoidT.VoidingLineItemID "
			" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
			" WHERE (LineItemT.Type < 10) AND  "
			" (LineItemT.Deleted = 0) AND  "
			"  LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND "
			" (PatientsT.PersonID > 0) %s  %s " 
				+ GetStatementChargebackString("ChargebacksPayments")
				+ GetStatementChargebackString("ChargebacksAdjustments")
				, strHeaderProv, strTransProv, strHideUnAppliedPrepayments, strInnerFilter);
			part6 = _T(	" GROUP BY LineItemT.ID, PersonT.Last,  "
			" PersonT.First, PersonT.Middle,  "
			" PersonT.Address1, PersonT.Address2,  "
			" PersonT.City, PersonT.State,  "
			" PersonT.Zip, PersonT.HomePhone, LocationsT.Address1,  "
			" LocationsT.Address2, LocationsT.Name,  "
			" LocationsT.City, LocationsT.State,  "
			" LocationsT.Zip, LocationsT.Phone,  "
			" LocationsT.Fax,  "
			" PatientsT.UserDefinedID,  "
			" PatientsT.PersonID,  "
			" ProvidersT.PersonID, LineItemT.Type,  "
			" LineItemT.Description, LineItemT.Date,  "
			" LineItemCorrectionsT_NewPay.NewLineItemID, "
			" PersonT1.Last, PersonT1.Middle,  PersonT1.Title, "
			" PersonT1.First, PersonT.BirthDate,  "
			" PersonT.Location, "
			" LineItemT.LocationID, "
			" [ProvidersT].[Fed Employer ID], "
			" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
			"  PaymentsT.ProviderID, "
			"PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PaymentsT.Prepayment, PersonT1.ID, "
			" CreditCardNamesT.CardName, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
			" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, PaymentsT.ID " 
			" HAVING ((Min(LineItemT.Amount) = 0 AND Min(AppliesT.Amount) IS NULL) OR (CASE WHEN "
			" [LineItemT].[Type] = 3 THEN "
			"  - 1 * MIN([LineItemT].[Amount])  "
			"  + SUM(CASE WHEN [AppliesT].[Amount] IS "
			"  NULL  THEN 0 ELSE [AppliesT].[Amount] END)  "
			" ELSE MIN([LineItemT].[Amount])  "
			" - SUM(CASE WHEN [AppliesT].[Amount] IS "
			" NULL  THEN 0 ELSE [AppliesT].[Amount] END ) "
			" END <> 0)) ) AS StmtPays "
			" UNION ");
			part7 =  _T("  SELECT StmtApplies.ChargeID,  "
			"  StmtApplies.PatientID, StmtApplies.PatID,  "
			"  StmtApplies.Type, StmtApplies.ApplyAmount,  "
			"  StmtApplies.Description, StmtApplies.Date,  "
			"  StmtApplies.Insurance, StmtApplies.Last,  "
			"  StmtApplies.First, StmtApplies.Middle,  "
			"  StmtApplies.Address1, StmtApplies.Address2,  "
			"  StmtApplies.City, StmtApplies.State,  "
			"  StmtApplies.Zip, StmtApplies.PatForward,  "
			"  StmtApplies.PatComma, StmtApplies.DocName,  "
			"  StmtApplies.DocAddress1, StmtApplies.DocAddress2, StmtApplies.DocCity, "
			"  StmtApplies.DocState, StmtApplies.DocZip, "
			"  StmtApplies.PracName,  "
			"  StmtApplies.PracAddress1,  "
			"  StmtApplies.PracAddress2,  "
			"  StmtApplies.PracCity, StmtApplies.PracState,  "
			"  StmtApplies.PracZip, StmtApplies.PracPhone,  "
			"  StmtApplies.PracFax, StmtApplies.ProvID,  "
			"  StmtApplies.BillID, StmtApplies.BillDate,  "
			"  StmtApplies.BillDescription,  "
			"  StmtApplies.BirthDate, \r\n"
			"  StmtApplies.ICD9Code1,  \r\n"
			"  StmtApplies.ICD9Code2,  \r\n"
			"  StmtApplies.ICD9Code3,  \r\n"
			"  StmtApplies.ICD9Code4,  \r\n"
			"  StmtApplies.ICD10Code1,  \r\n"
			"  StmtApplies.ICD10Code2,  \r\n"
			"  StmtApplies.ICD10Code3,  \r\n"
			"  StmtApplies.ICD10Code4,  \r\n"
			"  StmtApplies.WhichCodes9,  \r\n"
			"  StmtApplies.WhichCodes10,  \r\n"
			"  StmtApplies.WhichCodesBoth,  \r\n"
			"  StmtApplies.Location,  "
			"  3 as StatementType, StmtApplies.PaymentID as GroupFixID, "
			"  StmtApplies.LocationFixID, "
			"  StmtApplies.ProvTaxID, "
			"  StmtApplies.TransProv, StmtApplies.Prepayment,   "
			"  StmtApplies.ProvEIN, StmtApplies.ProvLicense, StmtApplies.ProvUPIN, StmtApplies.ProvDEA, StmtApplies.ProvBCBS, StmtApplies.ProvMedicare, StmtApplies.ProvMedicaid, StmtApplies.ProvWorkersComp, StmtApplies.ProvOtherID, StmtApplies.ProvOtherIDDesc, StmtApplies.HomePhone AS PatPhone,  "
			"  0 As FullChargeNoTax, 0 As ChargeTax1, 0 As ChargeTax2, 0 As TaxRate1, 0 As TaxRate2, 0 AS Quantity, "
			" StmtApplies.CCType, StmtApplies.CCNumber, StmtApplies.CheckNo, StmtApplies.BankNo, StmtApplies.CheckAcctNo, "
			" StmtApplies.CCHoldersName, StmtApplies.CCExpDate, StmtApplies.CCAuthNo, StmtApplies.BankRoutingNum, " 
			" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, '' as CPTCode, "
			" '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, "
			" StmtApplies.BillStatementNote, StmtApplies.LineItemStatementNote "
			" FROM (SELECT * "
			" FROM (SELECT AppliesT.DestID AS ChargeID,  "
			"   AppliesT.ID AS PaymentID, "
			"   PatientsT.UserDefinedID AS PatientID, "
			"   PatientsT.PersonID AS PatID,  "
			"   LineItemT1.Type,  "
			"   AppliesT.Amount AS ApplyAmount,  "
			"   LineItemT1.Date,  "
			"   LineItemT1.InputDate,  "
			" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
			"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
			"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
			"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
			" ELSE LineItemT1.Description END) AS Description,  "
			"   Insurance = CASE WHEN [PaymentsT].[InsuredPartyID] "
			"   > 0 THEN [AppliesT].[Amount] ELSE "
			"   0 END, PersonT.Last,  "
			"   PersonT.First, PersonT.Middle,  "
			"   PersonT.Address1,  "
			"   PersonT.Address2, PersonT.CIty,  "
			"   PersonT.State, PersonT.Zip, PersonT.HomePhone, "
			"   LocationsT.Name AS PracName,  "
			"   LocationsT.Address1 AS PracAddress1, "
			"   LocationsT.Address2 AS PracAddress2, ");
			part8.Format(" LocationsT.City AS PracCity,  "
			"   LocationsT.State AS PracState,  "
			"   LocationsT.Zip AS PracZip,  "
			"   LocationsT.Phone AS PracPhone,  "
			"   LocationsT.Fax AS PracFax,  "
			"   PatientsT.SuppressStatement,  "
			"   ProvidersT.PersonID AS ProvID,  "
			"   (%s WHERE PersonT.ID = PersonT1.ID) AS DocName, "
			" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
			" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
			"   PersonT.First + ' ' + PersonT.Middle "
			"   + ' ' + PersonT.Last AS PatForward, "
			"   PersonT.Last + ', ' + PersonT.First "
			"   + ' ' + PersonT.Middle AS PatComma, "
			"   BillsT.ID AS BillID,  "
			"   BillsT.Date AS BillDate,  "
			"   BillsT.Description AS BillDescription, "
			"   PersonT.BirthDate,  \r\n"
			"   ICD9T1.CodeNumber AS ICD9Code1,  \r\n"
			"	ICD9T2.CodeNumber AS ICD9Code2,  \r\n"
			"	ICD9T3.CodeNumber AS ICD9Code3,  \r\n"
			"	ICD9T4.CodeNumber AS ICD9Code4,  \r\n"
			"   ICD10T1.CodeNumber AS ICD10Code1,  \r\n"
			"	ICD10T2.CodeNumber AS ICD10Code2,  \r\n"
			"	ICD10T3.CodeNumber AS ICD10Code3,  \r\n"
			"	ICD10T4.CodeNumber AS ICD10Code4,  \r\n"
			"   WhichCodesQ.WhichCodes9,  \r\n"
			"   WhichCodesQ.WhichCodes10,  \r\n"
			"   WhichCodesQ.WhichCodesBoth,  \r\n"
			"   PersonT.Location, "
			"   LineItemT.LocationID as LocationFixID, "
			"   [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
			" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
			"   (%s WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 as Prepayment, "
			" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
			" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
			" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
			" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
			"  FROM PersonT LEFT OUTER JOIN "
			"   LocationsT ON  "
			"   PersonT.Location = LocationsT.ID RIGHT "
			"   OUTER JOIN "
			"   LineItemT LineItemT1 RIGHT OUTER "
			"   JOIN "
			"   PaymentsT ON  "
			"   LineItemT1.ID = PaymentsT.ID "
			"   LEFT JOIN PaymentPlansT ON PaymentsT.Id = PaymentPlansT.ID "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"	LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
			"	LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID "
			"   RIGHT "
			"   OUTER JOIN "
			"   AppliesT ON  "
			"   PaymentsT.ID = AppliesT.SourceID ", strHeaderProv, strTransProv);
			part8b.Format("   LEFT OUTER JOIN "
			"   LineItemT RIGHT OUTER JOIN "
			"   ChargesT LEFT OUTER JOIN "
			"   BillsT LEFT OUTER JOIN "
			"   PatientsT ON  "
			"   BillsT.PatientID = PatientsT.PersonID "
			"   ON  "
			"   ChargesT.BillID = BillsT.ID ON  "
			"   LineItemT.ID = ChargesT.ID ON  "
			"   AppliesT.DestID = ChargesT.ID ON "
			"   PersonT.ID = PatientsT.PersonID \r\n"

			" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
			" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
						
			" LEFT JOIN  \r\n"
			" (SELECT ChargesT.ID as ChargeID, "
			" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
			" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
			" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
			" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
			" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
			" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
			" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
			" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
			" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
			" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
			" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
			" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
			" STUFF((SELECT ', ' +  \r\n "
			" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
				 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
				 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
			" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
			" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
			" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
			" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
			" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
			" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
			" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
			" FROM ChargesT \r\n "
			" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "
			
			"	LEFT OUTER JOIN "
			"   PersonT PersonT1 RIGHT OUTER JOIN "
			"   ProvidersT ON  "
			"   PersonT1.ID = ProvidersT.PersonID "
			"   ON  "
			"   PatientsT.MainPhysician = ProvidersT.PersonID "
			" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
			" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
			" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
			" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON PaymentsT.ID = LineItemCorrections2T.OriginalLineItemID "
			" LEFT JOIN LineItemCorrectionsT Void2T ON PaymentsT.ID = Void2T.VoidingLineItemID "
			" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
			"  WHERE (LineItemT1.Deleted = 0) AND  "
			"   (LineItemT.Deleted = 0) AND  "
			"   BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL AND "
			"   (BillsT.Deleted = 0) AND  "
			"   (AppliesT.PointsToPayments = 0)  "
			"   AND (PatientsT.PersonID > 0) %s " 
				+ GetStatementChargebackString("ChargebacksPayments") + " "
				+ GetStatementChargebackString("ChargebacksAdjustments") + " "
			" )  "
			" AS StatementDataAppliesCharges ", strInnerFilter);
			part9.Format("UNION "
			" SELECT * "
			" FROM (SELECT AppliesT.DestID AS ChargeID,  "
			"   AppliesT.ID AS PaymentID, "
			"   PatientsT.UserDefinedID AS PatientID, "
			"   PatientsT.PersonID AS PatID,  "
			"   LineItemT1.Type,  "
			"   AppliesT.Amount AS ApplyAmount,  "
			"   LineItemT1.Date,  "
			"   LineItemT1.InputDate,  "
			" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
			"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
			"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
			"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
			" ELSE LineItemT1.Description END) AS Description,  "
			"   CASE WHEN PaymentsT.InsuredPartyID "
			"   > 0 THEN AppliesT.Amount ELSE 0 "
			"   END AS Insurance,  "
			"   PersonT.Last, PersonT.First,  "
			"   PersonT.Middle,  "
			"   PersonT.Address1,  "
			"   PersonT.Address2, PersonT.City,  "
			"   PersonT.State, PersonT.Zip, PersonT.HomePhone,  "
			"   LocationsT.Name AS PracName,  "
			"   LocationsT.Address1 AS PracAddress1, "
			"   LocationsT.Address2 AS PracAddress2, "
			"   LocationsT.City AS PracCity,  "
			"   LocationsT.State AS PracState,  "
			"   LocationsT.Zip AS PracZip,  "
			"   LocationsT.Phone AS PracPhone,  "
			"   LocationsT.Fax AS PracFax,  "
			"   PatientsT.SuppressStatement,  "
			"   ProvidersT.PersonID AS ProvID,  "
			"   (%s WHERE PersonT.ID = PersonT1.ID) AS DocName, "
			" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
			" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
			"   PersonT.First + ' ' + PersonT.Middle "
			"   + ' ' + PersonT.Last AS PatForward, "
			"   PersonT.Last + ', ' + PersonT.First "
			"   + ' ' + PersonT.Middle AS PatComma, "
			"   LineItemT.ID AS BillID,  "
			"   LineItemT.Date AS BillDate,  "
			" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
			"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
			"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
			"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
			"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
			"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
			"   PersonT.BirthDate,  \r\n"
			"   '' AS ICD9Code1,  \r\n"
			"   '' AS ICD9Code2,  \r\n"
			"   '' AS ICD9Code3,  \r\n"
			"   '' AS ICD9Code4,  \r\n"
			"   '' AS ICD10Code1,  \r\n"
			"   '' AS ICD10Code2,  \r\n"
			"   '' AS ICD10Code3,  \r\n"
			"   '' AS ICD10Code4,  \r\n"
			"   '' AS WhichCodes9,  \r\n"
			"   '' AS WhichCodes10,  \r\n"
			"   '' AS WhichCodesBoth,  \r\n"
			"   PersonT.Location, "
			"   LineItemT.LocationID as LocationFixID, "
			"  [ProvidersT].[Fed Employer ID] AS ProvTaxID,  "
			" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
			"  (%s  WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, 0 As Prepayment, "
			" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
			" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
			" '' AS BillStatementNote, "
			" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
			"  FROM PersonT PersonT1 RIGHT OUTER "
			"   JOIN "
			"   ProvidersT ON  "
			"   PersonT1.ID = ProvidersT.PersonID "
			"   RIGHT OUTER JOIN "
			"   PersonT LEFT OUTER JOIN "
			"   LocationsT ON  "
			"   PersonT.Location = LocationsT.ID RIGHT "
			"   OUTER JOIN "
			"   LineItemT LineItemT1 LEFT OUTER "
			"   JOIN "
			"   PatientsT ON  ",  strHeaderProv, strTransProv);
			part10.Format("   LineItemT1.PatientID = PatientsT.PersonID "
			"   ON  "
			"   PersonT.ID = PatientsT.PersonID ON "
			"   ProvidersT.PersonID = PatientsT.MainPhysician "
			"   RIGHT OUTER JOIN "
			"   PaymentsT ON  "
			"   LineItemT1.ID = PaymentsT.ID "
			"	LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID "
			"	LEFT JOIN ChargebacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID "
			"	RIGHT OUTER JOIN "
			"   LineItemT RIGHT OUTER JOIN "
			"   AppliesT ON  "
			"   LineItemT.ID = AppliesT.DestID ON "
			"   PaymentsT.ID = AppliesT.SourceID "
			"   LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"	LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID "
			"	LEFT JOIN ChargebacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID "
			" LEFT JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
			" LEFT JOIN LineItemCorrectionsT VoidT ON LineItemT.ID = VoidT.VoidingLineItemID "
			" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON LineItemT1.ID = LineItemCorrections2T.OriginalLineItemID "
			" LEFT JOIN LineItemCorrectionsT Void2T ON LineItemT1.ID = Void2T.VoidingLineItemID "	
			" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
			"  WHERE (LineItemT1.Deleted = 0) AND  "
			"   (LineItemT1.Deleted = 0) AND  "
			"   LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL AND "
			"   (AppliesT.PointsToPayments = 1)  "
			"   AND (PatientsT.PersonID > 0) %s " 
				+ GetStatementChargebackString("ChargebacksPayments1") + " " 
				+ GetStatementChargebackString("ChargebacksAdjustments1") + " "
				+ GetStatementChargebackString("ChargebacksPayments") + " " 
				+ GetStatementChargebackString("ChargebacksAdjustments") + " "
			" )  "
			" AS StatementDataAppliesPays)  "
			"  AS StmtApplies) AS StatementAllData LEFT JOIN "
			"   (SELECT AppointmentsT.Date AS AppDate,  "
			"   AppointmentsT.StartTime,  "
			"   AppointmentsT.PatientID AS PatID "
			" FROM AppointmentsT "
			" WHERE (((AppointmentsT.Date) > GetDate()) AND  "
			"   (AppointmentsT.PatientID > 0) AND  "
			"   ((AppointmentsT.Status) <> 4))) AS NextApp ON  "
			"  NextApp.PatID = StatementAllData.PatID LEFT JOIN "
			"  LineItemT ON LineItemT.ID = StatementAllData.ID LEFT  "
			"  JOIN "
				"  (SELECT CASE WHEN Thirty IS NULL THEN 0 ELSE Thirty END AS Thirty, CASE WHEN Sixty IS NULL THEN 0 ELSE SIXTY END AS Sixty, CASE WHEN Ninety IS NULL THEN 0 ELSE NINETY END AS Ninety, "
			"  CASE WHEN NINETYPLUS  IS NULL THEN 0 ELSE NinetyPlus END AS NinetyPlus, PatientID, LocationID "
			"  FROM( "
			"  SELECT PatAR.PatientID, PatAR.LocationID,  Sum(PatAR.Thirty) AS Thirty, Sum(PatAR.Sixty) AS Sixty, Sum(PatAR.Ninety) AS Ninety, Sum(PatAR.NinetyPlus) AS NinetyPlus "
			"  FROM "
			"  (SELECT Sum((CASE WHEN Thirty.ChargeAmount IS NULL THEN 0 ELSE Thirty.ChargeAmount END) - (CASE WHEN Thirty.PPAmount IS NULL THEN 0 ELSE Thirty.PPAmount END)) AS Thirty, 0 AS Sixty, 0 As Ninety, 0 AS NinetyPlus, PatientID, LocationID  "
			"  FROM  "
			"  ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, LocationID FROM   "
			"  (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, LineItemT.LocationID FROM ChargeRespDetailT LEFT JOIN "
			"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
			"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
			"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd,-30, getDate()))  AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1) %s "
			"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, LineItemT.LocationID) As Charges  "
			"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
			"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID "
			"   GROUP BY PatientID, LocationID) "
			"   UNION  "
			"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, LineItemT.LocationID  "
			"   FROM  "
			"   LineItemT LEFT JOIN   "
			"   (SELECT DestID, Sum(Amount) as Amount  "
			"   FROM AppliesT  "
			"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
			"   WHERE (InsuredPartyID = -1)  "
			" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID "
			"   LEFT JOIN "
			" 	(SELECT SourceID, Sum(Amount) AS Amount  "
			"   FROM AppliesT  "
			"  	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
			"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
			"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd,-30, getDate())) AND (PaymentsT.InsuredPartyID = -1)  %s %s "
			"   GROUP BY LineItemT.PatientID, LineItemT.LocationID)) AS Thirty   "
			"   GROUP BY Thirty.PatientID, Thirty.LocationID  "
			"   UNION   "
			"   SELECT 0 AS Thirty, Sum((CASE WHEN Sixty.ChargeAmount IS NULL THEN 0 ELSE Sixty.ChargeAmount END) - (CASE WHEN Sixty.PPAmount IS NULL THEN 0 ELSE Sixty.PPAmount END)) AS Sixty, 0 AS Ninety, 0 AS NinetyPlus, PatientID, LocationID   "
			"   FROM  "
			"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, LocationID FROM   "
			"  	(SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, LineItemT.LocationID FROM ChargeRespDetailT LEFT JOIN  "
			"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
			" 	LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
			" 	WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date <= DATEADD(dd, -30, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1) %s "
			" 	GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, LineItemT.LocationID) As Charges  "
			"   LEFT JOIN   "
			" 	(SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
			" 	ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays  "
			"   ON Charges.DetailID = Pays.DetailID "
			"   GROUP BY PatientID, LocationID) "
			"   UNION  "
			"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, LineItemT.LocationID "
			"   FROM  "
			"   LineItemT LEFT JOIN  "
			"   (SELECT DestID, Sum(Amount) as Amount  "
			"   	FROM AppliesT   "
			" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
			"  	WHERE (InsuredPartyID = -1)  "
			" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
			"   LEFT JOIN "
			" 	(SELECT SourceID, Sum(Amount) AS Amount  "
			"  	FROM AppliesT  "
			" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
			"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd, -60, getDate())) AND (LineItemT.Date <= DATEADD(dd, -30, getDate())) AND (PaymentsT.InsuredPartyID = -1)  %s %s "
			"   GROUP BY LineItemT.PatientID, LineItemT.LocationID)) AS Sixty  "
			"   GROUP BY Sixty.PatientID, Sixty.LocationID "
			"   UNION  "
			"   SELECT 0 AS Thirty, 0 AS Sixty, Sum((CASE WHEN Ninety.ChargeAmount IS NULL THEN 0 ELSE Ninety.ChargeAmount END) - (CASE WHEN Ninety.PPAmount IS NULL THEN 0 ELSE Ninety.PPAmount END)) AS Ninety, 0 AS NinetyPlus, PatientID, LocationID  "
			"   FROM "
			"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, LocationID FROM  "
			"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, LineItemT.LocationID FROM ChargeRespDetailT LEFT JOIN "
			"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID "
			"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
			"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date >= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1) %s "
			"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, LineItemT.LocationID) As Charges "
			"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
			"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID "
			"   GROUP BY PatientID, LocationID) "
			"   UNION  "
			"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, LineItemT.LocationID "
			"   FROM  "
			"   LineItemT LEFT JOIN  "
			"  	(SELECT DestID, Sum(Amount) as Amount  "
			"  	FROM AppliesT  "
			"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"  	WHERE (InsuredPartyID = -1) "
			"  	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID "
			"   LEFT JOIN "
			"  	(SELECT SourceID, Sum(Amount) AS Amount "
			" 	FROM AppliesT "
			" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID "
			"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
			"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd, -60, getDate())) AND (LineItemT.Date >= DATEADD(dd, -90, getDate())) AND (PaymentsT.InsuredPartyID = -1)  %s %s "
			"   GROUP BY LineItemT.PatientID, LineItemT.LocationID)) AS Ninety "
			"   GROUP BY Ninety.PatientID, Ninety.LocationID "
			"    UNION   "
			"   SELECT 0 AS Thirty, 0 AS Sixty, 0 AS Ninety, Sum((CASE WHEN NinetyPlus.ChargeAmount IS NULL THEN 0 ELSE NinetyPlus.ChargeAmount END) - (CASE WHEN NinetyPlus.PPAmount IS NULL THEN 0 ELSE NinetyPlus.PPAmount END)) AS NinetyPlus, PatientID, LocationID  "
			"   FROM  "
			"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, LocationID FROM  "
			"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, LineItemT.LocationID FROM ChargeRespDetailT LEFT JOIN "
			"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
			"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
			"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1) %s "
			"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, LineItemT.LocationID) As Charges  "
			"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM "
			"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID  "
			"   GROUP BY PatientID, LocationID)  "
			"    UNION   "
			"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, LineItemT.LocationID  "
			"   FROM  "
			"   LineItemT LEFT JOIN  "
			"  	(SELECT DestID, Sum(Amount) as Amount "
			" 	FROM AppliesT  "
			" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"  	WHERE (InsuredPartyID = -1)  "
			" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
			"    LEFT JOIN  "
			" 	(SELECT SourceID, Sum(Amount) AS Amount  "
			" 	FROM AppliesT  "
			" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID "
			"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd,-90, getDate())) AND (PaymentsT.InsuredPartyID = -1) %s %s "
			"   GROUP BY LineItemT.PatientID, LineItemT.LocationID)) AS NinetyPlus "
			"   GROUP BY NinetyPlus.PatientID, NinetyPlus.LocationID) PatAR  "
			"   GROUP BY PatAR.PatientID, PatAR.LocationID) AS PatientAR) AS StatementAR ON StatementAllData.PatID = StatementAR.PatientID "
			"   AND StatementAllData.LocationFixID = StatementAR.LocationID "
			"  GROUP BY StatementAllData.ID, StatementAllData.PatientID,  "
			"  StatementAllData.PatID, StatementAllData.Type,  ", strInnerFilterApp, strInnerFilter, strHideUnAppliedPrepayments,  strInnerFilter, strInnerFilter, strHideUnAppliedPrepayments, strInnerFilter, strInnerFilter, strHideUnAppliedPrepayments, strInnerFilter, strInnerFilter, strHideUnAppliedPrepayments, strInnerFilter);
			part11 = _T("  StatementAllData.Total, StatementAllData.Description,  "
			"  StatementAllData.Date, StatementAllData.Insurance,  "
			"  StatementAllData.Last, StatementAllData.First,  "
			"  StatementAllData.Middle, StatementAllData.Address1,  "
			"  StatementAllData.Address2, StatementAllData.City,  "
			"  StatementAllData.State, StatementAllData.Zip,  "
			"  StatementAllData.PatForward,  "
			"  StatementAllData.PatComma,  "
			"  StatementAllData.DocName,  "
			"  StatementAllData.DocAddress1, "
			"  StatementAllData.DocAddress2, "
			"  StatementAllData.DocCity, "
			"  StatementAllData.DocState, "
			"  StatementAllData.DocZip, "
			"  StatementAllData.PracName,  "
			"  StatementAllData.PracAddress1,  "
			"  StatementAllData.PracAddress2,  "
			"  StatementAllData.PracCity, StatementAllData.PracState,  "
			"  StatementAllData.PracZip, StatementAllData.PracPhone,  "
			"  StatementAllData.PracFax, StatementAllData.ProvID,  "
			"  StatementAllData.BillID, StatementAllData.BillDate,  "
			"  StatementAllData.BillDescription,  "
			"  StatementAllData.BirthDate, LineItemT.Date,  \r\n"
			"  StatementAllData.ICD9Code1,  \r\n"
			"  StatementAllData.ICD9Code2,  \r\n"
			"  StatementAllData.ICD9Code3,  \r\n"
			"  StatementAllData.ICD9Code4,  \r\n"
			"  StatementAllData.ICD10Code1,  \r\n"
			"  StatementAllData.ICD10Code2,  \r\n"
			"  StatementAllData.ICD10Code3,  \r\n"
			"  StatementAllData.ICD10Code4,  \r\n"			
			"  StatementAllData.WhichCodes9,  \r\n"
			"  StatementAllData.WhichCodes10,  \r\n"
			"  StatementAllData.WhichCodesBoth,  \r\n"
			"  StatementAllData.Location,  "
			"  StatementAR.Thirty, "
			"  StatementAR.Sixty, "
			"  StatementAR.Ninety, "
			"  StatementAR.NinetyPlus, "
			"  StatementAllData.GroupFixID, StatementAllData.StatementType, StatementAllData.LocationFixID,  "
			"  StatementAllData.DocAddress1, StatementAllData.DocAddress2, StatementAllData.DocCity, StatementAllData.DocState, StatementAllData.DocZip, "
			"  StatementAllData.ProvTaxID, StatementAllData.TransProv, StatementAllData.Prepayment, "
			"  StatementAllData.ProvEIN, StatementAllData.ProvLicense, StatementAllData.ProvUPIN, StatementAllData.ProvDEA, StatementAllData.ProvBCBS, StatementAllData.ProvMedicare, StatementAllData.ProvMedicaid, StatementAllData.ProvWorkersComp, StatementAllData.ProvOtherID, StatementAllData.ProvOtherIDDesc, StatementAllData.PatPhone, "
			"  StatementAllData.FullChargeNoTax, StatementAllData.ChargeTax1, StatementAllData.ChargeTax2, StatementAllData.TaxRate1, StatementAllData.TaxRate2, StatementAllData.Quantity, "
			"  StatementAllData.CCType, StatementAllData.CCNumber, StatementAllData.CheckNo, StatementAllData.BankNo, StatementAllData.CheckAcctNo, "
			" StatementAllData.CCHoldersName, StatementAllData.CCExpDate, StatementAllData.CCAuthNo, StatementAllData.BankRoutingNum, "
			" StatementAllData.PercentOff, StatementAllData.DiscountAmt, StatementAllData.DiscountCategoryDesc, StatementAllData.CPTCode, "
			" StatementAllData.CPTModifier1, StatementAllData.CPTModifier2,StatementAllData.CPTModifier3, StatementAllData.CPTModifier4, StatementAllData.BillStatementNote, StatementAllData.LineItemStatementNote  )  "
			" AS StatementSubQ LEFT OUTER JOIN "
			" (SELECT * "
			"  FROM (SELECT PersonT1.First + ' ' + PersonT1.Middle + ' ' + PersonT1.Last + ' ' + PersonT1.Title ");
			part12 = _T("  AS DocName, PersonT1.ID AS ProvID,  "
			"  PatientsT.SuppressStatement,  "
			"  PatientsT.PersonID AS PatID,  "
			"  PatientsT.PrimaryRespPartyID AS PrimaryRespID, "
			"  PatientsT.StatementNote, "
			"  PatCoordT.First as PatCoordFirst, PatCoordT.Middle as PatCoordMiddle, PatCoordT.Last as PatCoordLast, ProvidersT.NPI as ProviderNPI "
			" FROM PatientsT LEFT OUTER JOIN "
			"  PersonT PersonT1 ON  "
			"  PatientsT.MainPhysician = PersonT1.ID LEFT OUTER "
			"   JOIN "
			"  PersonT ON PatientsT.PersonID = PersonT.ID "
			"  LEFT JOIN ProvidersT ON PersonT1.ID = ProvidersT.PersonID "
			"  LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID " 
			" )  "
			"   AS PatInfo LEFT OUTER JOIN "
			" (SELECT InsuranceCoT.Name AS PriInsCo,  "
			" PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last "
			" AS PriGuarForward,  "
			" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
			" AS PriGuarComma,  "
			" PersonT.First AS PriInsFirst,  "
			" PersonT.Middle AS PriInsMiddle,  "
			" PersonT.Last AS PriInsLast,  "
			" PatientsT.PersonID AS PersonID "
			"  FROM InsuranceCoT RIGHT OUTER JOIN "
			" PersonT RIGHT OUTER JOIN "
			" InsuredPartyT ON  "
			" PersonT.ID = InsuredPartyT.PersonID ON  "
			" InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			" RIGHT OUTER JOIN "
			" PatientsT ON  "
			" InsuredPartyT.PatientID = PatientsT.PersonID "
			"  WHERE InsuredPartyT.RespTypeID = 1)  "
			"   AS PriInsInfo ON  "
			"   PatInfo.PatID = PriInsInfo.PersonID LEFT OUTER JOIN "
			" (SELECT InsuranceCoT.Name AS SecInsCo,  "
			" PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last "
			" AS SecGuarForward,  "
			" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
			" AS SecGuarComma,  "
			" PersonT.First AS SecInsFirst,  "
			" PersonT.Middle AS SecInsMiddle,  "
			" PersonT.Last AS SecInsLast,  "
			" PatientsT.PersonID AS PersID "
			"  FROM InsuranceCoT RIGHT OUTER JOIN "
			" PersonT RIGHT OUTER JOIN "
			" InsuredPartyT ON  "
			" PersonT.ID = InsuredPartyT.PersonID ON  "
			" InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			" RIGHT OUTER JOIN "
			" PatientsT ON  "
			" InsuredPartyT.PatientID = PatientsT.PersonID "
			"  WHERE InsuredPartyT.RespTypeID = 2)  "
			"   AS SecInsInfo ON PatInfo.PatID = SecInsInfo.PersID "
			"  LEFT OUTER JOIN "
			" (SELECT ResponsiblePartyT.PersonID as RespID, ResponsiblePartyT.PatientID AS RespPatID, First as RespFirst, Middle as RespMiddle, Last as RespLast, Address1 as RespAdd1, Address2 as RespAdd2,  "
			" City as RespCity, State as RespState, Zip as RespZip "
			" FROM PersonT Inner Join ResponsiblePartyT ON PersonT.ID = ResponsiblePartyT.PersonID LEFT JOIN PatientsT ON ResponsiblePartyT.PatientID = PatientsT.PersonID @RespFilter) AS ResPartyT  "
			" ON PatInfo.PatID = ResPartyT.RespPatID)  "
			" AS StatementEndQ ON  "
			" StatementSubQ.PatID = StatementEndQ.PatID "
			" LEFT OUTER JOIN (SELECT ID, Name AS PracName, Address1 AS PracAddress1, Address2 AS PracAddress2, City AS PracCity, State AS PracState, Zip AS PracZip, Phone As PracPhone, Fax AS PracFax FROM LocationsT) AS StatementLocQ ON "
			" StatementSubQ.LocationFixID = StatementLocQ.ID ");
			part13.Format(" LEFT JOIN "
			"(SELECT * FROM [%s]) AS StatementLastPatPayInfoQ " 
			" ON StatementSubQ.LocationFixID = StatementLastPatPayInfoQ.LocID "
			" AND StatementSubQ.PatID = StatementLastPatPayInfoQ.PatID "
			" AND StatementLastPatPayInfoQ.IsInsurance = 0 "
			" LEFT JOIN "
			"(SELECT * FROM [%s]) AS StatementLastInsPayInfoQ " 
			" ON StatementSubQ.LocationFixID = StatementLastInsPayInfoQ.LocID "
			" AND StatementSubQ.PatID = StatementLastInsPayInfoQ.PatID "
			" AND StatementLastInsPayInfoQ.IsInsurance = 1 "			
			"WHERE (StatementEndQ.SuppressStatement = 0) ", strTempTableName, strTempTableName);

			// (a.walling 2006-10-24 13:12) - PLID 16059 - Multiple Responsible parties
			part12.Replace("@RespFilter", strRespFilter);

			CString strSql = part1 + part2 + part3 + part4 + part5 + part6 + part7 + part8 + part8b + part9 + part10 + part11 + part12 + part13;

			//now we have to add the other filters that we haven't already filtered on
			CString strMoreFilters;
			AddPartToClause(strMoreFilters, m_pReport->GetLocationFilter(m_nSubLevel, m_nSubRepNum));
			AddPartToClause(strMoreFilters, m_pReport->GetDateFilter(m_nSubLevel, m_nSubRepNum));
			AddPartToClause(strMoreFilters, m_pReport->GetExternalFilter(m_nSubLevel, m_nSubRepNum));

			strMoreFilters.TrimLeft();
			strMoreFilters.TrimRight();
			
			if (!strMoreFilters.IsEmpty()) {
				m_pReport->ConvertCrystalToSql(strSql, strMoreFilters);
				AddFilter(strSql, strMoreFilters, TRUE);
			}		
			return strSql;
		}
		break;

		case 338: //Individual Patient Statement By Location
		case 356:
			{



            CString strLoc;
			
			if (m_pReport->nLocation == -1) {
							strLoc = " ";
			}
			else {
				strLoc.Format("AND (LineItemT.LocationID  = %li)", m_pReport->nLocation);
			}


			CString strBillID, strNoUnAppPays;
			if (m_pReport->nExtraID == -1) {
				strBillID = " ";
				strNoUnAppPays = " ";
			}
			else {
				strBillID.Format(" AND (BillsT.ID = %li) ", m_pReport->nExtraID);
				strNoUnAppPays = " AND (1 = 0) ";
			}

			long nTransFormat, nHeaderFormat;
			nTransFormat = GetRemotePropertyInt("SttmntTransProvFormat", 0, 0, "<None>");
			nHeaderFormat = GetRemotePropertyInt("SttmntHeaderProvFormat", 0, 0, "<None>");
			CString strTransProv = GetStatementProvString(nTransFormat);
			CString strHeaderProv = GetStatementProvString(nHeaderFormat);
		   CString strHideUnAppliedPrepayments = GetStatementUnAppliedPrePaymentsString();
		   CString strChargeDescription = GetStatementChargeDescription();
						
			// (j.gruber 2008-07-01 16:16) - PLID 30322 - make a table variable
			CString strTempTableName;
			strTempTableName = GetLastLocationPaymentInformationSql("", m_pReport->nPatient);

		   // (j.gruber 2007-05-01 17:13) - PLID 25745 - only show the last 4 digits of the cc number
		   // (j.gruber 2007-05-15 09:08) - PLID 25987 - take out credit card expiration dates
		   // (j.gruber 2007-05-29 17:21) - PLID 26096 - change the joins of the top part of the applies query
		   // (j.gruber 2007-06-29 09:37) - PLID 26501 - parameterized the individual statement
		   // (e.lally 2007-07-13) PLID 26670 - Updated all references to PaymentPlansT. CCType with link to CardName, aliased as CCType where applicable.
		   // (j.gruber 2008-06-09 11:24) - PLID 30323 - made the last sent date only applicable to the current location			
			// (j.gruber 2008-07-02 15:26) - PLID 30322 - made the last payment amounts and dates pull per location
			// (j.gruber 2008-07-03 15:27) - PLID 27499 - corrected the filter on the payments applied to charges query and the payments applied to payment query
			// (j.jones 2008-09-23 17:15) - PLID 30288 - supported MailSentNotesT
			// (j.gruber 2009-11-05 17:42) - PLID 36217 - added providerNPI
			// (j.gruber 2009-11-25 12:31) - PLID 36430 - added CPTCode
			// (j.gruber 2009-12-24 13:10) - PLID 17122 - added CPTModifiers
			// (j.gruber 2010-06-14 15:59) - PLID 36484 - added billing notes
			// (j.gruber 2011-07-05 11:09) - PLID 44831 - take out originals and voids
			// (a.wilson 2012-2-24) PLID 48380 - Removed ':' from query to fix compatibility change errors.
			// (j.gruber 2014-03-04 15:04) - PLID 61167 - update for ICD10, diag and whichcodes refactoring
			//TES 7/17/2014 - PLID 62935 - Added code to hide chargebacks when requested
			// (r.goldschmidt 2014-09-25 14:22) - PLID 62851 - Need to account for changes to statement history note
			// (j.jones 2015-03-09 09:48) - PLID 64937 - if the description begins with 'Corrected Charge',
			// 'Corrected Payment', etc., strip that off
			CString part1, part2, part3, part4, part5, part6, part7, part8, part9, part10, part11;
			        part1.Format(" SELECT StatementSubQ.ID, StatementSubQ.PatientID, StatementSubQ.PatID as PatID, StatementSubQ.Type, StatementSubQ.Total, StatementSubQ.Description, StatementSubQ.Date as TDate,  "
					" StatementSubQ.Insurance, StatementSubQ.Last, StatementSubQ.First, StatementSubQ.Middle, StatementSubQ.Address1, StatementSubQ.Address2, "
					" StatementSubQ.City, StatementSubQ.State, StatementSubQ.Zip, StatementSubQ.PatForward, StatementSubQ.PatComma, StatementSubQ.DocName, StatementSubQ.DocAddress1, StatementSubQ.DocAddress2, StatementSubQ.DocCity, StatementSubQ.DocState, StatementSubQ.DocZip,  "
					" StatementSubQ.ProvTaxID,  StatementLocQ.PracName, StatementLocQ.PracAddress1, "
					" StatementLocQ.PracAddress2, StatementLocQ.PracCity, StatementLocQ.PracState, StatementLocQ.PracZip, StatementLocQ.PracPhone, StatementLocQ.PracFax, StatementSubQ.ProvID as ProvID2, StatementSubQ.BillId, "
					" StatementSubQ.BillDate, StatementSubQ.BillDescription, StatementSubQ.Birthdate, \r\n"
					" StatementSubQ.ICD9Code1, StatementSubQ.ICD9Code2, StatementSubQ.ICD9Code3, StatementSubQ.ICD9Code4, \r\n "
					" StatementSubQ.ICD10Code1, StatementSubQ.ICD10Code2, StatementSubQ.ICD10Code3, StatementSubQ.ICD10Code4, \r\n "
					" StatementSubQ.WhichCodes9, StatementSubQ.WhichCodes10, StatementSubQ.WhichCodesBoth, "
					" StatementSubQ.Location, StatementSubQ.StatementType, StatementSubQ.GroupFixID, StatementSubQ.Appdate, StatementSubQ.StartTime, StatementSubQ.ARDate, StatementSubQ.Age,  "
					" StatementSubQ.TransProv, StatementSubQ.PrePayment, StatementSubQ.Quantity, StatementSubQ.Thirty, StatementSubQ.Sixty, StatementSubQ.Ninety, StatementSubQ.NinetyPlus, "
					" StatementSubQ.ProvEIN, StatementSubQ.ProvLicense, StatementSubQ.ProvUPIN, StatementSubQ.ProvDEA, StatementSubQ.ProvBCBS, StatementSubQ.ProvMedicare, StatementSubQ.ProvMedicaid, StatementSubQ.ProvWorkersComp, StatementSubQ.ProvOtherID, StatementSubQ.ProvOtherIDDesc, "
					" StatementEndQ.DocName as DocName2, StatementEndQ.ProvID as ProvID, StatementEndQ.SuppressStatement, StatementEndQ.PatID as PatID2, StatementEndQ.StatementNote, StatementEndQ.PriInsCo, StatementEndQ.PriGuarForward, StatementEndQ.PriGuarComma, "
					" StatementEndQ.PriInsFirst, StatementEndQ.PriInsMiddle, StatementEndQ.PriInsLast, StatementEndQ.PersonID, StatementEndQ.SecInsCo, StatementEndQ.SecGuarForward, StatementEndQ.SecGuarComma, StatementEndQ.SecInsfirst, StatementEndQ.SecInsMiddle, "
					" StatementEndQ.SecInsLast, StatementEndQ.PersID, "
					" StatementSubQ.LocationFixID AS LocID, StatementEndQ.RespID, StatementEndQ.RespFirst, StatementEndQ.RespMiddle, StatementEndQ.RespLast, StatementEndQ.RespAdd1, StatementEndQ.RespAdd2, StatementEndQ.RespCity, "
					" StatementEndQ.RespState, StatementEndQ.RespZip, "
					" (SELECT Max(Date) FROM MailSent INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
						"WHERE ((MailSentNotesT.Note Like '%%Patient Statement%%Printed%%For ' + StatementLocQ.PracName + '%%') OR (MailSentNotesT.Note Like '%%Patient Statement%%Run%%For ' + StatementLocQ.PracName + '%%') OR (MailSentNotesT.Note Like '%%E-Statement%%Exported%%')) AND PersonID = StatementSubQ.PatID) AS LastSentDate, "
					" StatementSubQ.PatPhone,   "
					" StatementSubQ.FullChargeNoTax, StatementSubQ.ChargeTax1, StatementSubQ.ChargeTax2, StatementSubQ.TaxRate1, StatementSubQ.TaxRate2, "
					" (SELECT COUNT(*) FROM ResponsiblePartyT WHERE PatientID = StatementSubQ.PatID) AS RespPartyCount, "
					" StatementLastPatPayInfoQ.LastPayDate as LastPatientPaymentDate, StatementLastInsPayInfoQ.LastPayDate as LastInsurancePaymentDate, "
					" StatementLastPatPayInfoQ.LastPayAmount as LastPatientPaymentAmount, StatementLastInsPayInfoQ.LastPayAmount as LastInsurancePaymentAmount, "
					" StatementSubQ.CCType, CASE WHEN Len(StatementSubQ.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(StatementSubQ.CCNumber, 4) END as CCNumber, StatementSubQ.CheckNo, StatementSubQ.BankNo, StatementSubQ.CheckAcctNo, "
					" StatementSubQ.CCHoldersName, Convert(datetime, NULL) AS CCExpDate, StatementSubQ.CCAuthNo, StatementSubQ.BankRoutingNum, " 
					" StatementEndQ.PatCoordFirst, StatementEndQ.PatCoordMiddle, StatementEndQ.PatCoordLast, "
					" StatementSubQ.PercentOff, StatementSubQ.DiscountAmt, StatementSubQ.DiscountCategoryDesc, StatementEndQ.ProviderNPI, StatementSubQ.CPTCode, "
					" StatementSubQ.CPTModifier1, StatementSubQ.CPTModifier2, StatementSubQ.CPTModifier3, StatementSubQ.CPTModifier4, "
					" StatementSubQ.BillStatementNote, StatementSubQ.LineItemStatementNote, "
					" CASE WHEN StatementSubQ.BillStatementNote = '' OR StatementSubQ.BillStatementNote IS NULL THEN 1 ELSE 0 END as SuppressBillStatementNote, "
					" CASE WHEN StatementSubQ.LineItemStatementNote = '' OR StatementSubQ.LineItemStatementNote IS NULL  THEN 1 ELSE 0 END as SuppressLineItemStatementNote "
							"FROM "
							"/*Begin StatementSubQ*/ "
							"/*StatementSubQ brings it all together*/  "
							"(SELECT StatementAllData.*,  Min(NextApp.AppDate)as AppDate, Min(NextApp.StartTime) as StartTime, LineItemT.Date as ARDate,  "
							" Age =   "
							"       CASE "
							"	  WHEN StatementAllData.BirthDate Is Null then "
							"		-1 "
							"	  ELSE  "
							"		DATEDIFF(YYYY, StatementAllData.Birthdate, GetDate()) - "
							"			CASE WHEN MONTH(StatementAllData.Birthdate) > MONTH(GetDate()) OR (MONTH(StatementAllData.Birthdate) = MONTH(GetDate()) AND DAY(StatementAllData.Birthdate) > DAY(GetDate())) "
							"				THEN 1 ELSE 0 END "
							"       END, "
							"  CASE WHEN StatementAR.Thirty IS NULL THEN 0 ELSE StatementAR.Thirty END AS Thirty, "
							"  CASE WHEN StatementAR.Sixty IS NULL THEN 0 ELSE StatementAR.Sixty END AS Sixty, "
							"  CASE WHEN StatementAR.Ninety IS NULL THEN 0 ELSE StatementAR.Ninety END AS Ninety, "
							"  CASE WHEN StatementAR.NinetyPlus IS NULL THEN 0 ELSE StatementAR.NinetyPlus END AS NinetyPlus "
							"FROM "
							"/*Begin StatementAllData*/ "
							"/*StatementAllData is the big union query that unions all the separate queries together*/ "
							"/*the separate queries being Charges, Payments, Applied Charges and Payments*/ "
							"( "
							"/*Begin Union Charges*/ "
							"SELECT StmtCharges.ID, StmtCharges.PatientID, StmtCharges.PatID, StmtCharges.Type, StmtCharges.Total,  "
							"StmtCharges.Description, StmtCharges.Date, StmtCharges.Insurance, StmtCharges.Last, StmtCharges.First, StmtCharges.Middle, "
							"StmtCharges.Address1, StmtCharges.Address2, StmtCharges.City, StmtCharges.State, StmtCharges.Zip, StmtCharges. PatForward, "
							"StmtCharges.PatComma, StmtCharges.DocName, StmtCharges.DocAddress1, StmtCharges.DocAddress2, StmtCharges.DocCity, StmtCharges.DocState, StmtCharges.DocZip,  "
							"StmtCharges.PracName, StmtCharges.PracAddress1, StmtCharges.PracAddress2, "
							"StmtCharges.PracCity, StmtCharges.PracState, StmtCharges.PracZip, StmtCharges.PracPhone, StmtCharges.PracFax, "
							"StmtCharges.ProvID, StmtCharges.BillID, StmtCharges.BillDate, StmtCharges.BillDescription, StmtCharges.BirthDate, \r\n"
							"StmtCharges.ICD9Code1, StmtCharges.ICD9Code2, StmtCharges.ICD9Code3, StmtCharges.ICD9Code4, \r\n"
							"StmtCharges.ICD10Code1, StmtCharges.ICD10Code2, StmtCharges.ICD10Code3, StmtCharges.ICD10Code4, \r\n"
							"StmtCharges.WhichCodes9, StmtCharges.WhichCodes10, StmtCharges.WhichCodesBoth, \r\n"
							"StmtCharges.Location, 1 as StatementType, -1 as GroupFixID, StmtCharges.LocationFixID, StmtCharges.ProvTaxID, StmtCharges.TransProv, StmtCharges.PrePayment,  "
							"StmtCharges.ProvEIN, StmtCharges.ProvLicense, StmtCharges.ProvUPIN, StmtCharges.ProvDEA, StmtCharges.ProvBCBS, StmtCharges.ProvMedicare, StmtCharges.ProvMedicaid, StmtCharges.ProvWorkersComp, StmtCharges.ProvOtherID, StmtCharges.ProvOtherIDDesc, StmtCharges.HomePhone AS PatPhone, "
							"  StmtCharges.FullChargeNoTax, StmtCharges.ChargeTax1, StmtCharges.ChargeTax2, StmtCharges.TaxRate1, StmtCharges.TaxRate2, StmtCharges.Quantity, "
							" '' as CCType, '' AS CCNumber,'' AS CheckNo, '' AS BankNo,  '' AS CheckAcctNo, "
							" '' AS CCHoldersName, NULL AS CCExpDate, '' AS CCAuthNo, '' AS BankRoutingNum, " 
							"  StmtCharges.PercentOff, StmtCharges.DiscountAmt, StmtCharges.DiscountCategoryDesc, StmtCharges.CPTCode, "
							" StmtCharges.CPTModifier1, StmtCharges.CPTModifier2, StmtCharges.CPTModifier3, StmtCharges.CPTModifier4, "
							" StmtCharges.BillStatementNote, StmtCharges.LineItemStatementNote "
							"FROM ");
			   part2.Format("/*begin StmtCharges. StmtCharges gathers all the information you need for any charge*/ "
							"/*this query will give you the ChargeID, PatientID, UserDefinedID, Type, Generates a Total Charge,*/ "
							"/*date, insurance charge, first name, last name, middle name, address1, address2, city,*/ "
							"/*state, zip, phone, fax, provID, Doctor's name, billID, BillDate, BillDescription, Birthdate,*/ "
							"/*diagCodes, and whichCodes field*/  "
							"(SELECT  LineItemT.ID, PatientsT.UserDefinedID as PatientID, LineItemT.PatientID as PatID, LineItemT.Type, "
							"Total =  "
							"	CASE "
							"	   WHEN Sum(ChargeRespT.Amount) is Null then "
							"		0 "
							"	   ELSE  "
							"		Sum(ChargeRespT.Amount) "
							"	   END, Description = %s, "
							"LineItemT.Date,   "
							"Insurance =  "
							"	Sum(CASE  "
							"		WHEN ChargeRespT.InsuredPartyID is  not NULL AND ChargeRespT.InsuredPartyID <> -1  "
							"			then  ChargeRespT.Amount "
							"			else 0 "
							"	END), "
							"PersonT.Last, PersonT.Middle, PersonT.First, PersonT.Address1, PersonT.Address2, "
							"PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Name PracName, LocationsT.Address1 PracAddress1, LocationsT.Address2 as PracAddress2, "
							"LocationsT.City PracCity, LocationsT.State PracState, LocationsT.Zip as PracZip, LocationsT.Phone as PracPhone, "
							"LocationsT.Fax as PracFax,  "
							"ProvidersT.PersonID as ProvID, (%s WHERE PersonT.ID = PersonT1.ID) as DocName, "
							" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
							" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
							"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward, "
							"BillsT.ID as BillID, BillsT.Date as BillDate, BillsT.Description as BillDescription, PersonT.BirthDate, \r\n"
							"ICD9T1.CodeNumber AS ICD9Code1, ICD9T2.CodeNumber AS ICD9Code2, ICD9T3.CodeNumber AS ICD9Code3, ICD9T4.CodeNumber AS ICD9Code4, \r\n"
							"ICD10T1.CodeNumber AS ICD10Code1, ICD10T2.CodeNumber AS ICD10Code2, ICD10T3.CodeNumber AS ICD10Code3, ICD10T4.CodeNumber AS ICD10Code4, \r\n"
							"WhichCodesQ.WhichCodes9, WhichCodesQ.WhichCodes10, WhichCodesQ.WhichCodesBoth, \r\n"
							"PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
							" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
							" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2)  AS FullChargeNoTax,   "
							" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate) - 1) AS ChargeTax1, "
							" Round(Convert(money,(((Min(LineItemT.[Amount])*Min([Quantity])*(CASE WHEN(Min(CPTMultiplier1) Is Null) THEN 1 ELSE Min(CPTMultiplier1) END)*(CASE WHEN Min(CPTMultiplier2) Is Null THEN 1 ELSE Min(CPTMultiplier2) END)*(CASE WHEN Min(CPTMultiplier3) Is Null THEN 1 ELSE Min(CPTMultiplier3) END)*(CASE WHEN Min(CPTMultiplier4) Is Null THEN 1 ELSE Min(CPTMultiplier4) END)* (CASE WHEN(Min([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,Min([TotalPercentOff])))/100) END)-(CASE WHEN Min([TotalDiscount]) Is Null THEN 0 ELSE Min([TotalDiscount]) END))))), 2) * Min((ChargesT.TaxRate2) - 1) AS ChargeTax2, "
							" Min(ChargesT.TaxRate) AS TaxRate1, Min(ChargesT.TaxRate2) AS TaxRate2, "
							" (%s WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 AS Prepayment, ChargesT.Quantity,  "
							" TotalPercentOff as PercentOff, TotalDiscount as DiscountAmt, "
							" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDesc, CPTCodeT.Code as CPTCode, "
							" ChargesT.CPTModifier as CPTModifier1, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, "
							" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
							" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = ChargesT.ID) AS LineItemStatementNote "
							"FROM LineItemT LEFT OUTER JOIN "
							"    PatientsT LEFT OUTER JOIN "
							"    ProvidersT ON  ", strChargeDescription, strHeaderProv, strTransProv);
			   part3.Format("    PatientsT.MainPhysician = ProvidersT.PersonID LEFT OUTER JOIN "
							"    PersonT PersonT1 ON  "
							"    ProvidersT.PersonID = PersonT1.ID LEFT OUTER JOIN "
							"    LocationsT RIGHT OUTER JOIN "
							"    PersonT ON LocationsT.ID = PersonT.Location ON  "
							"    PatientsT.PersonID = PersonT.ID ON  "
							"    LineItemT.PatientID = PatientsT.PersonID LEFT OUTER JOIN "
							"    BillsT RIGHT OUTER JOIN "
							"    ServiceT LEFT OUTER JOIN "
							" CPTCodeT ON  "
							"    ServiceT.ID = CPTCodeT.ID RIGHT OUTER JOIN "
							"    ChargesT ON ServiceT.ID = ChargesT.ServiceID ON  "
							"    BillsT.ID = ChargesT.BillID LEFT OUTER JOIN "
							"    ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID ON  "
							"    LineItemT.ID = ChargesT.ID "
							"	LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewCharge ON ChargesT.ID = LineItemCorrectionsT_NewCharge.NewLineItemID \r\n"
							"	LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID \r\n"
							
							" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
							" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
							" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
							" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
							" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
							" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
							" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
							" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
							" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
										
							" LEFT JOIN  \r\n"
							" (SELECT ChargesT.ID as ChargeID, "
							" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
							" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
							" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
							" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
							" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
							" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
							" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
							" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
							" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
							" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
							" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
							" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
							" STUFF((SELECT ', ' +  \r\n "
							" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
								 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
								 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
							" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
							" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
							" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
							" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
							" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
							" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
							" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
							" FROM ChargesT \r\n "
							" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "
							
							" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
							" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
							" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
							"WHERE (PatientsT.PersonID = ? ) AND (%s >= ? ) AND (%s <= ? ) AND (LineItemT.Deleted = 0) AND (BillsT.Deleted = 0) "
							" AND BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL "
							" AND (LineItemT.Type = 10) %s  %s "
							"GROUP BY LineItemT.ID, PatientsT.UserDefinedID, LineItemT.PatientID, LineItemT.Type, "
							"CPTCodeT.Code, LineItemT.Description, LineItemCorrectionsT_NewCharge.NewLineItemID, "
							"LineItemT.Date,  "
							"PersonT.Last, PersonT.Middle, PersonT.First, PersonT.Address1, PersonT.Address2, "
							"PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Name, LocationsT.Address1, LocationsT.Address2, "
							"LocationsT.City, LocationsT.State, LocationsT.Zip, LocationsT.Phone, "
							"LocationsT.Fax, ProvidersT.PersonID, PersonT1.Last,PersonT1.First, PersonT1.Middle, PersonT1.Title, "
							"BillsT.ID, BillsT.Date, BillsT.Description, PersonT.BirthDate, \r\n"
							"ICD9T1.CodeNumber, ICD9T2.CodeNumber, ICD9T3.CodeNumber, ICD9T4.CodeNumber, \r\n"
							"ICD10T1.CodeNumber, ICD10T2.CodeNumber, ICD10T3.CodeNumber, ICD10T4.CodeNumber, \r\n"
							"WhichCodesQ.WhichCodes9, WhichCodesQ.WhichCodes10, WhichCodesQ.WhichCodesBoth, \r\n"
							"PersonT.Location, LineItemT.LocationID, [ProvidersT].[Fed Employer ID], ChargesT.DoctorsProviders, "
							" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
							"PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PersonT1.ID, ChargesT.Quantity, TotalPercentOff, TotalDiscount, ChargesT.ID, CPTCodeT.Code, "
							" ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4 ) as StmtCharges "
							"/*end StmtCharges*/ "
							"/*End Union Charges*/ "
							"UNION ", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT.Date", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT.Date", strLoc, strBillID);
			
			   part4.Format("/*Begin Union Payments*/ "
							"SELECT StmtPays.ID, StmtPays.PatientID, StmtPays.PatID, StmtPays.Type, StmtPays.UnAppliedAmount,  "
							"StmtPays.Description, StmtPays.Date, StmtPays.Insurance, StmtPays.Last, StmtPays.First, StmtPays.Middle, "
							"StmtPays.Address1, StmtPays.Address2, StmtPays.City, StmtPays.State, StmtPays.Zip, StmtPays.PatForward, "
							"StmtPays.PatComma, StmtPays.DocName, StmtPays.DocAddress1, StmtPays.DocAddress2, StmtPays.DocCity, StmtPays.DocState, StmtPays.DocZip, "
							"StmtPays.PracName, StmtPays.PracAddress1, StmtPays.PracAddress2, "
							"StmtPays.PracCity, StmtPays.PracState, StmtPays.PracZip, StmtPays.PracPhone, StmtPays.PracFax, "
							"StmtPays.ProvID, StmtPays.BillID, StmtPays.BillDate, StmtPays.BillDescription, StmtPays.BirthDate, \r\n"
							"StmtPays.ICD9Code1, StmtPays.ICD9Code2, StmtPays.ICD9Code3, StmtPays.ICD9Code4, \r\n"
							"StmtPays.ICD10Code1, StmtPays.ICD10Code2, StmtPays.ICD10Code3, StmtPays.ICD10Code4, \r\n"
							"StmtPays.WhichCodes9, StmtPays.WhichCodes10, StmtPays.WhichCodesBoth,\r\n"
							"StmtPays.Location, 2 as StatementType, -2 as GroupFixID, StmtPays.LocationFixID, StmtPays.ProvTaxID, StmtPays.TransProv, StmtPays.PrePayment, "
							"StmtPays.ProvEIN, StmtPays.ProvLicense, StmtPays.ProvUPIN, StmtPays.ProvDEA, StmtPays.ProvBCBS, StmtPays.ProvMedicare, StmtPays.ProvMedicaid, StmtPays.ProvWorkersComp, StmtPays.ProvOtherID, StmtPays.ProvOtherIDDesc, StmtPays.HomePhone AS PatPhone, "
							"  0 AS FullChargeNoTax, 0 AS ChargeTax1, 0 AS ChargeTax2, 0 AS TaxRate1, 0 As TaxRate2, 0 As Quantity, "
							" StmtPays.CCType, StmtPays.CCNumber, StmtPays.CheckNo, StmtPays.BankNo, StmtPays.CheckAcctNo, "
						    " StmtPays.CCHoldersName, StmtPays.CCExpDate, StmtPays.CCAuthNo, StmtPays.BankRoutingNum, " 
							" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, '' as CPTCode, "
							" '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, " 
							" StmtPays.BillStatementNote, "
							" StmtPays.LineItemStatementNote "
							"FROM ( "
							"/*StmtPays is the query that gets all the information you need for payments, this includes:*/ "
							"/*PaymentID, Insurance, Last Name, First Name, Middle Name, Address1, Address2,*/  "
							"SELECT LineItemT.ID, Insurance=   "
							"	CASE "
							"	   WHEN MIN([PaymentsT].[InsuredPartyID])>0 then "
							"		CASE "
							"		   WHEN [LineItemT].[Type]= 3 then "
							"			MIN([LineItemT].[Amount])-Sum( "
							"			CASE "
							"			   WHEN [AppliesT].[Amount] is NULL then 0 "
							"			   ELSE [AppliesT].[Amount] "
							"			END) "
							"		   ELSE Min([LineItemT].[Amount])-Sum( ");
			   part5.Format("			CASE  "
							"			    WHEN [AppliesT].[Amount] is NULL THEN 0 "
							"			    ELSE [AppliesT].[Amount] "
							"			END) "
							"		   END "
							"	   ELSE 0 "
							"	END, "
							"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, "
							"PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Address1 as PracAddress1, LocationsT.Address2 as PracAddress2, "
							"LocationsT.Name as PracName, LocationsT.City as PracCity, LocationsT.State as PracState, LocationsT.Zip as PracZip, "
							"LocationsT.Phone as PracPhone, LocationsT.Fax as PracFax, PatientsT.UserDefinedID as PatientID, PatientsT.PersonID as PatID, "
							"ProvidersT.PersonID as ProvID,  "
							"(%s WHERE PersonT.ID = PersonT1.ID) as DocName,  "
							" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
							" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
							"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, "
							"PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward, LineItemT.Type, "
							"UnAppliedAmount =  "
							"	CASE "
							"	   WHEN [LineItemT].[Type]=3 then "
							"		MIN([LineItemT].[Amount])-Sum( "
							"		CASE  "
							"		   WHEN [AppliesT].[Amount] is Null then 0 "
							"		   ELSE [AppliesT].[Amount] "
							"		END) "
							"	   ELSE Min([LineItemT].[Amount])-Sum( "
							"		CASE  "
							"		   WHEN [AppliesT].[Amount] is Null then 0 "
							"		   ELSE [AppliesT].[Amount] "
							"		END) "
							"	   END, "
							" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
							"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
							"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
							"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
							" ELSE LineItemT.Description END) AS Description,  "
							"LineItemT.Date, LineItemT.ID as BillID, "
							" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
							"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
							"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
							"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
							"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
							"LineItemT.Date as BillDate, PersonT.BirthDate, \r\n"
							"'' as ICD9Code1, '' as ICD9Code2, '' AS ICD9Code3, '' AS ICD9Code4, \r\n"
							"'' as ICD10Code1, '' as ICD10Code2, '' AS ICD10Code3, '' AS ICD10Code4, \r\n"
							"'' AS WhichCodes9, '' AS WhichCodes10, '' AS WhichCodesBoth, \r\n"
							"PersonT.Location, LineItemT.LocationID AS LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
							" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
							" (%s  WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, PaymentsT.Prepayment AS Prepayment, "
							" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
							" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
							" '' AS BillStatementNote, "
							" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
							, strHeaderProv, strTransProv);
			 part6.Format(  " FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
							" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
							" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
							" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
							" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID"
							" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID  "
							" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
							" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID  "
							" LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID "
							" LEFT JOIN LineItemCorrectionsT ON PaymentsT.ID = LineItemCorrectionsT.OriginalLineItemID "
							" LEFT JOIN LineItemCorrectionsT VoidT ON PaymentsT.ID = VoidT.VoidingLineItemID "
							" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
							"WHERE (PatientsT.PersonID = ? ) AND (LineItemT.Date >= ? ) AND (LineItemT.Date <= ? ) "
							"  AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL "
							" AND (LineItemT.Type < 10) AND (LineItemT.Deleted = 0) %s %s %s " 
							+ GetStatementChargebackString("ChargebacksPayments") + " "
							+ GetStatementChargebackString("ChargebacksAdjustments") + " "
							"GROUP BY LineItemT.ID,   "
							"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, "
							"PersonT.State, PersonT.Zip, PersonT.HomePhone, LocationsT.Address1, LocationsT.Address2, "
							"LocationsT.Name, LocationsT.City, LocationsT.State, LocationsT.Zip, "
							"LocationsT.Phone, LocationsT.Fax, PatientsT.UserDefinedID, PatientsT.PersonID, "
							"ProvidersT.PersonID,  "
							"LineItemT.Type, LineItemT.Description, LineItemT.Date,  "
							" LineItemCorrectionsT_NewPay.NewLineItemID, "
							" PersonT1.Last, PersonT1.Middle, PersonT1.Title, PersonT1.First, PersonT.BirthDate, PersonT.Location, LineItemT.LocationID, [ProvidersT].[Fed Employer ID], PaymentsT.ProviderID, "
							" ProvidersT.EIN, ProvidersT.License, ProvidersT.UPIN, ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.[Other ID Description], "
							"PersonT1.Address1, PersonT1.Address2,  PersonT1.City, PersonT1.State, PersonT1.Zip, PaymentsT.Prepayment, PersonT1.ID, "
							" CreditCardNamesT.CardName, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
							" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, PaymentsT.ID " 
							" HAVING ((Min(LineItemT.Amount) = 0 AND Min(AppliesT.Amount) IS NULL) OR (CASE WHEN "
							" [LineItemT].[Type] = 3 THEN "
							"  - 1 * MIN([LineItemT].[Amount])  "
							"  + SUM(CASE WHEN [AppliesT].[Amount] IS "
							"  NULL  THEN 0 ELSE [AppliesT].[Amount] END)  "
							" ELSE MIN([LineItemT].[Amount])  "
							" - SUM(CASE WHEN [AppliesT].[Amount] IS "
							" NULL  THEN 0 ELSE [AppliesT].[Amount] END ) "
							" END <> 0)) ) AS StmtPays "
							"UNION ", strLoc,strHideUnAppliedPrepayments, strNoUnAppPays);
			part7.Format(   "/* Begin StmtApplies, this is all the information about applied payments, it it a union query unioning payments applied to payments*/  "
							"/*and payments applied to charges*/ "
							"SELECT StmtApplies.ChargeID, StmtApplies.PatientID, StmtApplies.PatID, StmtApplies.Type, StmtApplies.ApplyAmount,  "
							"StmtApplies.Description, StmtApplies.Date, StmtApplies.Insurance, StmtApplies.Last, StmtApplies.First, StmtApplies.Middle, "
							"StmtApplies.Address1, StmtApplies.Address2, StmtApplies.City, StmtApplies.State, StmtApplies.Zip, StmtApplies.PatForward, "
							"StmtApplies.PatComma, StmtApplies.DocName, StmtApplies.DocAddress1, StmtApplies.DocAddress2, StmtApplies.DocCity, StmtApplies.DocState, StmtApplies.DocZip, "
							"StmtApplies.PracName, StmtApplies.PracAddress1, StmtApplies.PracAddress2, "
							"StmtApplies.PracCity, StmtApplies.PracState, StmtApplies.PracZip, StmtApplies.PracPhone, StmtApplies.PracFax, "
							"StmtApplies.ProvID, StmtApplies.BillID, StmtApplies.BillDate, StmtApplies.BillDescription, StmtApplies.BirthDate, \r\n"
							"StmtApplies.ICD9Code1, StmtApplies.ICD9Code2, StmtApplies.ICD9Code3, StmtApplies.ICD9Code4, \r\n "
							"StmtApplies.ICD10Code1, StmtApplies.ICD10Code2, StmtApplies.ICD10Code3, StmtApplies.ICD10Code4, \r\n "
							"StmtApplies.WhichCodes9, StmtApplies.WhichCodes10, StmtApplies.WhichCodesBoth, \r\n" 
							"StmtApplies.Location, 3 as StatementType, StmtApplies.PaymentID as GroupFixID, StmtApplies.LocationFixID, StmtApplies.ProvTaxID, StmtApplies.TransProv, StmtApplies.PrePayment, "
							"StmtApplies.ProvEIN, StmtApplies.ProvLicense, StmtApplies.ProvUPIN, StmtApplies.ProvDEA, StmtApplies.ProvBCBS, StmtApplies.ProvMedicare, StmtApplies.ProvMedicaid, StmtApplies.ProvWorkersComp, StmtApplies.ProvOtherID, StmtApplies.ProvOtherIDDesc, StmtApplies.HomePhone AS PatPhone, "
							"  0 AS FullChargeNoTax, 0 AS ChargeTax1, 0 AS ChargeTax2, 0 AS TaxRate1, 0 As TaxRate2, 0 As Quantity, "
							" StmtApplies.CCType, StmtApplies.CCNumber, StmtApplies.CheckNo, StmtApplies.BankNo, StmtApplies.CheckAcctNo, "
							" StmtApplies.CCHoldersName, StmtApplies.CCExpDate, StmtApplies.CCAuthNo, StmtApplies.BankRoutingNum, " 
							" 0 as PercentOff, Convert(money, 0) as DiscountAmt, '' as DiscountCategoryDesc, '' as CPTCode,  " 
							" '' as CPTModifier1, '' as CPTModifier2, '' as CPTModifier3, '' as CPTModifier4, " 
							" StmtApplies.BillStatementNote, StmtApplies.LineItemStatementNote "
							"FROM  "
							"/*Payments applied to charges*/ "
							"(SELECT * FROM (SELECT AppliesT.DestID as ChargeID, AppliesT.ID as PaymentID, PatientsT.UserDefinedID as PatientID, PatientsT.PersonID as PatID, LineItemT1.Type, "
							"AppliesT.Amount as ApplyAmount, LineItemT1.Date, LineItemT1.InputDate, "
							" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
							"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
							"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
							"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
							" ELSE LineItemT1.Description END) AS Description,  "
							"Insurance =  "
							"	CASE "
							"		WHEN [PaymentsT].[InsuredPartyID] > 0 then "
							"			[AppliesT].[Amount] "
							"		ELSE 0 "
							"	END, "
							"PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.CIty, PersonT.State, PersonT.Zip, PersonT.HomePhone, "
							"LocationsT.Name as PracName, LocationsT.Address1 as PracAddress1, LocationsT.Address2 as PracAddress2, LocationsT.City as PracCity, "
							"LocationsT.State as PracState, LocationsT.Zip as PracZip, LocationsT.Phone as PracPhone, LocationsT.Fax as PracFax, "
							"PatientsT.SuppressStatement, ProvidersT.PersonID as ProvID, (%s WHERE PersonT.ID = PersonT1.ID) as DocName, "
							" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
							" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, ", strHeaderProv);
			 part8.Format(  "PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last as PatForward,  "
							"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatComma, "
							"BillsT.ID as BillID, BillsT.Date as BillDate, BillsT.Description as BillDescription, PersonT.BirthDate, \r\n"
							"ICD9T1.CodeNumber AS ICD9Code1, ICD9T2.CodeNumber AS ICD9Code2, ICD9T3.CodeNumber AS ICD9Code3, ICD9T4.CodeNumber AS ICD9Code4, \r\n"
							"ICD10T1.CodeNumber AS ICD10Code1, ICD10T2.CodeNumber AS ICD10Code2, ICD10T3.CodeNumber AS ICD10Code3, ICD10T4.CodeNumber AS ICD10Code4, \r\n"
							"WhichCodesQ.WhichCodes9, WhichCodesQ.WhichCodes10, WhichCodesQ.WhichCodesBoth, \r\n"
							"PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
							" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
							"(%s  WHERE PersonT.ID = ChargesT.DoctorsProviders) AS TransProv, 0 As Prepayment, "
							" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
							" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
							" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.BillID = BillsT.ID) AS BillStatementNote, "
							" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
							/*"FROM PersonT LEFT OUTER JOIN "
							"    LocationsT ON  "
							"    PersonT.Location = LocationsT.ID RIGHT OUTER JOIN "
							"    LineItemT LineItemT1 RIGHT OUTER JOIN "
							"    PaymentsT ON  "
							"    LineItemT1.ID = PaymentsT.ID "
							"    LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
							"    RIGHT OUTER JOIN "
							"    AppliesT ON  "
							"    PaymentsT.ID = AppliesT.SourceID LEFT OUTER JOIN "
							"    LineItemT RIGHT OUTER JOIN "
							"    ChargesT LEFT OUTER JOIN "
							"    BillsT LEFT OUTER JOIN "
							"    PatientsT ON BillsT.PatientID = PatientsT.PersonID ON  "
							"    ChargesT.BillID = BillsT.ID ON LineItemT.ID = ChargesT.ID ON  "
							"    AppliesT.DestID = ChargesT.ID ON  "
							"    PersonT.ID = PatientsT.PersonID LEFT OUTER JOIN "
							"    PersonT PersonT1 RIGHT OUTER JOIN "
							"    ProvidersT ON PersonT1.ID = ProvidersT.PersonID ON  "
							"    PatientsT.MainPhysician = ProvidersT.PersonID "
							" LEFT JOIN DiagCodes DiagCodes1 ON BillsT.Diag1ID = DiagCodes1.ID LEFT JOIN DiagCodes DiagCodes2 ON BillsT.Diag2ID = DiagCodes2.ID LEFT JOIN DiagCodes DiagCodes3 ON BillsT.Diag3ID = DiagCodes3.ID LEFT JOIN DiagCodes DiagCodes4 ON BillsT.Diag4ID = DiagCodes4.ID "
							*/
							" FROM LineItemT LineItemT1 LEFT JOIN PaymentsT ON LineItemT1.ID = PaymentsT.ID "
							" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
							" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
							" LEFT JOIN ChargebacksT ChargebacksPayments ON PaymentsT.ID = ChargebacksPayments.PaymentID "
							" LEFT JOIN ChargebacksT ChargebacksAdjustments ON PaymentsT.ID = ChargebacksAdjustments.AdjustmentID"
							" LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
							" LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
							" LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
							" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
							" LEFT JOIN PatientsT ON LineItemT1.PatientID = PatientsT.PersonID "
							" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
							" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
							" LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID "
							" LEFT JOIN PersonT PersonT1 on ProvidersT.PersonID = PersonT1.ID \r\n"

							" LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n"				
							" LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n"
							" LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n"
							" LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n"
							" LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n"
							" LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
							" LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n"
							" LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n"
							" LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n"
										
							" LEFT JOIN  \r\n"
							" (SELECT ChargesT.ID as ChargeID, "
							" STUFF((SELECT ', ' + ICD9T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT \r\n "
							" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
							" INNER JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
							" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
							" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
							" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes9, \r\n "
							" STUFF((SELECT ', ' + ICD10T.CodeNumber FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
							" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
							" INNER JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
							" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
							" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
							" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '') as WhichCodes10, \r\n "
							" STUFF((SELECT ', ' +  \r\n "
							" CASE WHEN ICD9T.ID IS NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber  \r\n "
								 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NULL THEN ICD9T.CodeNumber \r\n "
								 " WHEN ICD9T.ID IS NOT NULL AND ICD10T.ID IS NOT NULL THEN ICD10T.CodeNumber + ' (' + ICD9T.CodeNumber + ')' END  \r\n "
							" FROM ChargeWhichCodesT INNER JOIN BillDiagCodeT  \r\n "
							" ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID \r\n "
							" LEFT JOIN DiagCodes ICD9T ON BillDiagCodeT.ICD9DiagID = ICD9T.ID \r\n "
							" LEFT JOIN DiagCodes ICD10T ON BillDiagCodeT.ICD10DiagID = ICD10T.ID \r\n "
							" WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
							" ORDER BY BillDiagCodeT.OrderIndex ASC \r\n "
							" FOR XML PATH(''), TYPE).value('/', 'NVarChar(max)'), 1,2, '')  as WhichCodesBoth \r\n "
							" FROM ChargesT \r\n "
							" ) WhichCodesQ ON ChargesT.ID = WhichCodesQ.ChargeID \r\n "
							
							" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
							" LEFT JOIN LineItemCorrectionsT ON ChargesT.ID = LineItemCorrectionsT.OriginalLineItemID "
							" LEFT JOIN LineItemCorrectionsT VoidT ON ChargesT.ID = VoidT.VoidingLineItemID "
							" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON PaymentsT.ID = LineItemCorrections2T.OriginalLineItemID "
							" LEFT JOIN LineItemCorrectionsT Void2T ON PaymentsT.ID = Void2T.VoidingLineItemID "
							" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
							"WHERE (PatientsT.PersonID = ? ) AND (%s >= ? ) AND (%s <= ? ) AND (LineItemT1.Deleted = 0) AND (LineItemT.Deleted = 0) "
							" AND BillCorrectionsT.ID IS NULL AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL "
							" AND (BillsT.Deleted = 0) AND (AppliesT.PointsToPayments = 0) %s %s " 
							+ GetStatementChargebackString("ChargebacksPayments") + " "
							+ GetStatementChargebackString("ChargebacksAdjustments") + " "
							") AS StatementDataAppliesCharges "
							"UNION "
							"/*Payments applied to payments*/ "
							"SELECT * FROM (SELECT AppliesT.DestID AS ChargeID,  AppliesT.ID as PaymentID, "
							"    PatientsT.UserDefinedID AS PatientID,  "
							"    PatientsT.PersonID AS PatID, LineItemT1.Type,  "
							"    AppliesT.Amount AS ApplyAmount, LineItemT1.Date,  "
							"    LineItemT1.InputDate, "
							" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 1 "
							"	AND Left(LineItemT1.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Payment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 2 "
							"	AND Left(LineItemT1.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Adjustment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT1.Type = 3 "
							"	AND Left(LineItemT1.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT1.Description, Len(LineItemT1.Description)-Len('Corrected Refund'))) "
							" ELSE LineItemT1.Description END) AS Description,  "
							"    CASE WHEN (PaymentsT.InsuredPartyID) > 0 THEN AppliesT.Amount ELSE 0 END AS Insurance, PersonT.Last, PersonT.First,  "
							"    PersonT.Middle, PersonT.Address1, PersonT.Address2,  "
							"    PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, "
							"    LocationsT.Name AS PracName,  "
							"    LocationsT.Address1 AS PracAddress1,  ", strTransProv, m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT1.Date", m_pReport->nDateFilter == 2 ? "BillsT.Date" : "LineItemT1.Date", strLoc, strBillID);
			   part9.Format("    LocationsT.Address2 AS PracAddress2,  "
							"    LocationsT.City AS PracCity, LocationsT.State AS PracState,  "
							"    LocationsT.Zip AS PracZip, LocationsT.Phone AS PracPhone,  "
							"    LocationsT.Fax AS PracFax, PatientsT.SuppressStatement,  "
							"    ProvidersT.PersonID AS ProvID,  "
							"    (%s WHERE PersonT.ID = PersonT1.ID) AS DocName, "
							" PersonT1.Address1 As DocAddress1, PersonT1.Address2 AS DocAddress2, "
							" PersonT1.City AS DocCity, PersonT1.State AS DocState, PersonT1.Zip AS DocZip, "
							"     PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS PatForward, "
							"     PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatComma, "
							"LineItemT.ID as BillID, LineItemT.Date as BillDate, "
							" (CASE WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
							"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
							"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
							"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
							"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
							"ELSE LineItemT.Description END) AS BillDescription, "		//no idea why this is called BillDescription
							"PersonT.BirthDate, \r\n"
							"'' as ICD9Code1, '' as ICD9Code2, '' as ICD9Code3, '' as ICD9Code4, \r\n"
							"'' as ICD10Code1, '' as ICD10Code2, '' as ICD10Code3, '' as ICD10Code4, \r\n"
							"'' as WhichCodes9, '' as WhichCodes10, '' as WhichCodesBoth, \r\n "
							"PersonT.Location, LineItemT.LocationID as LocationFixID, [ProvidersT].[Fed Employer ID] AS ProvTaxID, "
							" ProvidersT.EIN AS ProvEIN, ProvidersT.License AS ProvLicense, ProvidersT.UPIN AS ProvUPIN, ProvidersT.[DEA Number] AS ProvDEA, ProvidersT.[BCBS Number] AS ProvBCBS, ProvidersT.[Medicare Number] AS ProvMedicare, ProvidersT.[Medicaid Number] AS ProvMedicaid, ProvidersT.[Workers Comp Number] AS ProvWorkersComp, ProvidersT.[Other ID Number] AS ProvOtherID, ProvidersT.[Other ID Description] AS ProvOtherIDDesc, "
							"  (%s WHERE PersonT.ID = PaymentsT.ProviderID) AS TransProv, 0 As Prepayment,  "
							" CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, "
							" PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, " 
							" '' AS BillStatementNote, "
							" (SELECT Left(Note, 1000) FROM Notes WHERE ShowOnStatement = 1 AND Notes.LineItemID = PaymentsT.ID) AS LineItemStatementNote "
							"FROM PersonT PersonT1 RIGHT OUTER JOIN "
							"    ProvidersT ON  "
							"    PersonT1.ID = ProvidersT.PersonID RIGHT OUTER JOIN "
							"    PersonT LEFT OUTER JOIN "
							"    LocationsT ON  "
							"    PersonT.Location = LocationsT.ID RIGHT OUTER JOIN "
							"    LineItemT LineItemT1 LEFT OUTER JOIN "
							"    PatientsT ON LineItemT1.PatientID = PatientsT.PersonID ON  "
							"    PersonT.ID = PatientsT.PersonID ON  "
							"    ProvidersT.PersonID = PatientsT.MainPhysician RIGHT OUTER JOIN "
							"    PaymentsT ON  "
							"    LineItemT1.ID = PaymentsT.ID "
							"    LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
							"	 LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
							"	 LEFT JOIN ChargebacksT ChargebacksPayments1 ON PaymentsT.ID = ChargebacksPayments1.PaymentID "
							"	 LEFT JOIN ChargebacksT ChargebacksAdjustments1 ON PaymentsT.ID = ChargebacksAdjustments1.AdjustmentID "
							"    RIGHT OUTER JOIN "
							"    LineItemT RIGHT OUTER JOIN "
							"    AppliesT ON LineItemT.ID = AppliesT.DestID ON  "
							"    PaymentsT.ID = AppliesT.SourceID "
							"	 LEFT JOIN ChargebacksT ChargebacksPayments ON LineItemT.ID = ChargebacksPayments.PaymentID "
							"	 LEFT JOIN ChargebacksT ChargebacksAdjustments ON LineItemT.ID = ChargebacksAdjustments.AdjustmentID "
							" LEFT JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
							" LEFT JOIN LineItemCorrectionsT VoidT ON LineItemT.ID = VoidT.VoidingLineItemID "
							" LEFT JOIN LineItemCorrectionsT LineItemCorrections2T ON LineItemT1.ID = LineItemCorrections2T.OriginalLineItemID "
							" LEFT JOIN LineItemCorrectionsT Void2T ON LineItemT1.ID = Void2T.VoidingLineItemID "				
							" LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
							"WHERE (PatientsT.PersonID = ? ) AND (%s >= ? ) AND (%s <= ? ) AND (LineItemT1.Deleted = 0) "
							" AND LineItemCorrectionsT.ID IS NULL AND VoidT.ID IS NULL AND LineItemCorrections2T.ID IS NULL AND Void2T.ID IS NULL "
							+ GetStatementChargebackString("ChargebacksPayments1") + " " 
							+ GetStatementChargebackString("ChargebacksAdjustments1") + " "
							+ GetStatementChargebackString("ChargebacksPayments") + " " 
							+ GetStatementChargebackString("ChargebacksAdjustments") + " "
							" AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 1) %s %s ) AS StatementDataAppliesPays) AS StmtApplies) as StatementAllData "
							"/*this ends StmtApplies and StatementAllData*/ "
							"LEFT JOIN ", strHeaderProv, strTransProv, m_pReport->nDateFilter == 2 ? "LineItemT.Date" : "LineItemT1.Date", m_pReport->nDateFilter == 2 ? "LineItemT.Date" : "LineItemT1.Date", strLoc, strNoUnAppPays);
			  part10.Format("/* this is a query that returns the time and date of the next appointment for the patient and the patientID so that it cam be linked to the main query*/ "
							"(SELECT AppointmentsT.Date AS AppDate, AppointmentsT.StartTime, AppointmentsT.PatientID as PatID "
							"FROM AppointmentsT  "
							"WHERE (AppointmentsT.Date > GetDate()) AND (AppointmentsT.PatientID = ? ) AND (AppointmentsT.Status <> 4)) AS NextApp ON NextApp.PatID = StatementAllData.PatID LEFT JOIN LineItemT on LineItemT.ID = StatementAllData.ID "
							" LEFT JOIN "
							"  (SELECT CASE WHEN Thirty IS NULL THEN 0 ELSE Thirty END AS Thirty, CASE WHEN Sixty IS NULL THEN 0 ELSE SIXTY END AS Sixty, CASE WHEN Ninety IS NULL THEN 0 ELSE NINETY END AS Ninety, "
							"  CASE WHEN NINETYPLUS  IS NULL THEN 0 ELSE NinetyPlus END AS NinetyPlus, PatientID, LocationID "
							"  FROM( "
							"  SELECT PatAR.PatientID, PatAR.LocationID, Sum(PatAR.Thirty) AS Thirty, Sum(PatAR.Sixty) AS Sixty, Sum(PatAR.Ninety) AS Ninety, Sum(PatAR.NinetyPlus) AS NinetyPlus "
							"  FROM "
							"  (SELECT Sum((CASE WHEN Thirty.ChargeAmount IS NULL THEN 0 ELSE Thirty.ChargeAmount END) - (CASE WHEN Thirty.PPAmount IS NULL THEN 0 ELSE Thirty.PPAmount END)) AS Thirty, 0 AS Sixty, 0 As Ninety, 0 AS NinetyPlus, PatientID, LocationID  "
							"  FROM  "
							"  ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, LocationID FROM   "
							"  (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, LineItemT.LocationID FROM ChargeRespDetailT LEFT JOIN "
							"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
							"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
							"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd,-30, getDate()))  AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (PatientID = ? ) "
							"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, LineItemT.LocationID) As Charges  "
							"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
							"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID "
							"   GROUP BY PatientID, LocationID) "
							"   UNION  "
							"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, LineItemT.LocationID  "
							"   FROM  "
							"   LineItemT LEFT JOIN   "
							"   (SELECT DestID, Sum(Amount) as Amount  "
							"   FROM AppliesT  "
							"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
							"   WHERE (InsuredPartyID = -1)  "
							" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID "
							"   LEFT JOIN "
							" 	(SELECT SourceID, Sum(Amount) AS Amount  "
							"   FROM AppliesT  "
							"  	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
							"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
							"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd,-30, getDate())) AND (PaymentsT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) %s "
							"   GROUP BY LineItemT.PatientID, LineItemT.LocationID)) AS Thirty   "
							"   GROUP BY Thirty.PatientID, Thirty.LocationID  "
							"   UNION   "
							"   SELECT 0 AS Thirty, Sum((CASE WHEN Sixty.ChargeAmount IS NULL THEN 0 ELSE Sixty.ChargeAmount END) - (CASE WHEN Sixty.PPAmount IS NULL THEN 0 ELSE Sixty.PPAmount END)) AS Sixty, 0 AS Ninety, 0 AS NinetyPlus, PatientID, LocationID   "
							"   FROM  "
							"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, LocationID FROM   "
							"  	(SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, LineItemT.LocationID FROM ChargeRespDetailT LEFT JOIN  "
							"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
							" 	LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID  "
							" 	WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date >= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date <= DATEADD(dd, -30, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) "
							" 	GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, LineItemT.LocationID) As Charges  "
							"   LEFT JOIN   "
							" 	(SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
							" 	ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays  "
							"   ON Charges.DetailID = Pays.DetailID "
							"   GROUP BY PatientID, LocationID) "
							"   UNION  "
							"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, LineItemT.LocationID "
							"   FROM  "
							"   LineItemT LEFT JOIN  "
							"   (SELECT DestID, Sum(Amount) as Amount  "
							"   	FROM AppliesT   "
							" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID   "
							"  	WHERE (InsuredPartyID = -1)  "
							" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
							"   LEFT JOIN "
							" 	(SELECT SourceID, Sum(Amount) AS Amount  "
							"  	FROM AppliesT  "
							" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID  "
							"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
							"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date >= DATEADD(dd, -60, getDate())) AND (LineItemT.Date <= DATEADD(dd, -30, getDate())) AND (PaymentsT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) %s "
							"   GROUP BY LineItemT.PatientID, LineItemT.LocationID)) AS Sixty  "
							"   GROUP BY Sixty.PatientID, Sixty.LocationID "
							"   UNION  "
							"   SELECT 0 AS Thirty, 0 AS Sixty, Sum((CASE WHEN Ninety.ChargeAmount IS NULL THEN 0 ELSE Ninety.ChargeAmount END) - (CASE WHEN Ninety.PPAmount IS NULL THEN 0 ELSE Ninety.PPAmount END)) AS Ninety, 0 AS NinetyPlus, PatientID, LocationID  "
							"   FROM "
							"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, LocationID FROM  "
							"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, LineItemT.LocationID FROM ChargeRespDetailT LEFT JOIN "
							"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID "
							"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
							"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -60, getDate())) AND (ChargeRespDetailT.Date >= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) "
							"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, LineItemT.LocationID) As Charges "
							"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM  "
							"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID "
							"   GROUP BY PatientID, LocationID) "
							"   UNION  "
							"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, LineItemT.LocationID "
							"   FROM  "
							"   LineItemT LEFT JOIN  "
							"  	(SELECT DestID, Sum(Amount) as Amount  "
							"  	FROM AppliesT  "
							"  	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
							"  	WHERE (InsuredPartyID = -1) "
							"  	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID "
							"   LEFT JOIN "
							"  	(SELECT SourceID, Sum(Amount) AS Amount "
							" 	FROM AppliesT "
							" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID "
							"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
							"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd, -60, getDate())) AND (LineItemT.Date >= DATEADD(dd, -90, getDate())) AND (PaymentsT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) %s "
							"   GROUP BY LineItemT.PatientID, LineItemT.LocationID)) AS Ninety "
							"   GROUP BY Ninety.PatientID, Ninety.LocationID "
							"    UNION   "
							"   SELECT 0 AS Thirty, 0 AS Sixty, 0 AS Ninety, Sum((CASE WHEN NinetyPlus.ChargeAmount IS NULL THEN 0 ELSE NinetyPlus.ChargeAmount END) - (CASE WHEN NinetyPlus.PPAmount IS NULL THEN 0 ELSE NinetyPlus.PPAmount END)) AS NinetyPlus, PatientID, LocationID  "
							"   FROM  "
							"   ((SELECT Sum(ChargeAmt - CASE WHEN PayAmt IS NULL THEN 0 ELSE PayAmt END) AS ChargeAmount, 0 AS PPAmount,  PatientID, LocationID FROM  "
							"   (SELECT Sum(ChargeRespDetailT.Amount) AS ChargeAmt, LineItemT.PatientID, ChargeRespDetailT.ID AS DetailID, LineItemT.LocationID FROM ChargeRespDetailT LEFT JOIN "
							"   ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID  "
							"   LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
							"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 10) AND (ChargeRespDetailT.Date <= DATEADD(dd, -90, getDate())) AND (ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1)  AND (LineItemT.PatientID = ? ) "
							"   GROUP BY LineItemT.PatientID, ChargeRespDetailT.ID, LineItemT.LocationID) As Charges  "
							"   LEFT JOIN (SELECT Sum(ApplyDetailsT.Amount) AS PayAmt, ApplyDetailsT.DetailID FROM "
							"   ApplyDetailsT GROUP BY ApplyDetailsT.DetailID) AS Pays ON Charges.DetailID = Pays.DetailID  "
							"   GROUP BY PatientID, LocationID)  "
							"    UNION   "
							"   (SELECT 0 AS ChargeAmount, SUM(LineItemT.Amount - CASE WHEN ApplyQ.Amount IS NULL THEN 0 ELSE ApplyQ.Amount END + CASE WHEN PPayQ.Amount IS NULL THEN 0 ELSE PPayQ.Amount END), LineItemT.PatientID, LocationID  "
							"   FROM  "
							"   LineItemT LEFT JOIN  "
							"  	(SELECT DestID, Sum(Amount) as Amount "
							" 	FROM AppliesT  "
							" 	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
							"  	WHERE (InsuredPartyID = -1)  "
							" 	GROUP BY DestID) AS PPayQ ON LineItemT.ID = PPayQ.DestID  "
							"    LEFT JOIN  "
							" 	(SELECT SourceID, Sum(Amount) AS Amount  "
							" 	FROM AppliesT  "
							" 	GROUP BY SourceID)AS ApplyQ ON LineItemT.ID = ApplyQ.SourceID "
							"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
							"   WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) AND (LineItemT.Date <= DATEADD(dd,-90, getDate())) AND (PaymentsT.InsuredPartyID = -1) AND (LineItemT.PatientID = ? ) %s "
							"   GROUP BY LineItemT.PatientID, LineItemT.LocationID)) AS NinetyPlus "
							"   GROUP BY NinetyPlus.PatientID, NinetyPlus.LocationID) PatAR  "
							"   GROUP BY PatAR.PatientID, PatAR.LocationID) AS PatientAR) AS StatementAR ON StatementAllData.PatID = StatementAR.PatientID "
							"	AND StatementAllData.LocationFixID = StatementAR.LocationID "
							"Group By StatementAllData.ID, StatementAllData.PatientId, StatementAllData.PatID, StatementAllData.Type, StatementAllData.Total, "
							"StatementAllData.Date, StatementAllData.Description, StatementAllData.Insurance, StatementAllData.Last, "
							"StatementAllData.First, StatementAllData.Middle, StatementAllData.Address1, StatementAllData.Address2, StatementAllData.City, "
							"StatementAllData.State, StatementAllData.Zip, StatementAllData.PatForward, StatementAllData.PatComma, StatementAllData.DocName, "
							"StatementAllData.DocAddress1, StatementAllData.DocAddress2, StatementAllData.DocCity, StatementAllData.DocState, StatementAllData.DocZip, "
							"StatementAllData.PracName, StatementAllData.PracAddress1, StatementAllData.PracAddress2, StatementAllData.PracCity, StatementAllData.PracState, "
							"StatementAllData.PracZip, StatementAllData.PracPhone, StatementAllData.PracFax, StatementAllData.BillId, StatementAllData.BillDate, "
							"StatementAllData.BillDescription, StatementAllData.Birthdate, \r\n"
							"StatementAllData.ICD9Code1, StatementAllData.ICD9Code2, StatementAllData.ICD9Code3, StatementAllData.ICD9Code4, "
							"StatementAllData.ICD10Code1, StatementAllData.ICD10Code2, StatementAllData.ICD10Code3, StatementAllData.ICD10Code4, "
							"StatementAllData.WhichCodes9, StatementAllData.WhichCodes10, StatementAllData.WhichCodesBoth, \r\n "
							"StatementAllData.DocAddress1, StatementAllData.DocAddress2, StatementAllData.DocCity, StatementAllData.DocState, StatementAllData.DocZip, "
							"StatementAllData.ProvId, StatementAllData.Location, StatementAllData.Date, LineItemT.Date, StatementAllData.StatementType, StatementAllData.GroupFixID, StatementAllData.LocationFixID, StatementAllData.ProvTaxID, StatementAllData.TransProv, StatementAllData.PrePayment,  "
							"StatementAllData.ProvEIN, StatementAllData.ProvLicense, StatementAllData.ProvUPIN, StatementAllData.ProvDEA, StatementAllData.ProvBCBS, StatementAllData.ProvMedicare, StatementAllData.ProvMedicaid, StatementAllData.ProvWorkersComp, StatementAllData.ProvOtherID, StatementAllData.ProvOtherIDDesc, StatementAR.Thirty, StatementAR.Sixty, StatementAR.Ninety, StatementAR.NinetyPlus, StatementAllData.PatPhone, "
							"  StatementAllData.FullChargeNoTax, StatementAllData.ChargeTax1, StatementAllData.ChargeTax2, StatementAllData.TaxRate1, StatementAllData.TaxRate2, StatementAllData.Quantity, "
							"  StatementAllData.CCType, StatementAllData.CCNumber, StatementAllData.CheckNo, StatementAllData.BankNo, StatementAllData.CheckAcctNo, "
							" StatementAllData.CCHoldersName, StatementAllData.CCExpDate, StatementAllData.CCAuthNo, StatementAllData.BankRoutingNum, "
							" StatementAllData.PercentOff, StatementAllData.DiscountAmt, StatementAllData.DiscountCategoryDesc, StatementAllData.CPTCode, "
							" StatementAllData.CPtModifier1, StatementAllData.CPtModifier2, StatementAllData.CPtModifier3, StatementAllData.CPtModifier4,  StatementAllData.BillStatementNote, StatementAllData.LineItemStatementNote) as StatementSubQ "
							"LEFT OUTER JOIN  "
							"/*this query returns all the patient information that the statement needs for a patient.  Eventually the patient demographics in the above queries*/ "
							"/*needs to be phased out and all of the information can come from this query*/ "
							"/*Begin StatementEndQ*/ "
							"(Select *  "
							"FROM "
							"/*this is a patient info query*/ "
							"(SELECT PersonT1.First + ' ' + PersonT1.Middle + ' ' + PersonT1.Last + ' ' + PersonT1.Title "
							"     AS DocName, PersonT1.ID as ProvID, PatientsT.SuppressStatement,  "
							"    PatientsT.PersonID AS PatID, PatientsT.PrimaryRespPartyID AS PrimaryRespID, PatientsT.StatementNote,  "
							"  PatCoordT.First as PatCoordFirst, PatCoordT.Middle as PatCoordMiddle, PatCoordT.Last as PatCoordLast, ProvidersT.NPI as ProviderNPI "
							"FROM PatientsT LEFT OUTER JOIN "
							"    PersonT PersonT1 ON  "
							"    PatientsT.MainPhysician = PersonT1.ID LEFT OUTER JOIN "
							"    PersonT ON PatientsT.PersonID = PersonT.ID "
							"	 LEFT JOIN ProvidersT ON PersonT1.ID = ProvidersT.PersonId "
							"    LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
							"WHERE (PatientsT.PersonID = ? ))  "
							"AS PatInfo  "
							"/*end PatInfo*/ "
							"LEFT OUTER JOIN "
							"/*this query returns primary insurance information*/ "
							"(SELECT InsuranceCoT.Name as PriInsCo, "
							"	PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS "
							"     PriGuarForward,  "
							"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PriGuarComma, ", strHideUnAppliedPrepayments, strHideUnAppliedPrepayments, strHideUnAppliedPrepayments,  strHideUnAppliedPrepayments);
			part11.Format(  "     PersonT.First AS PriInsFirst,  "
							"    PersonT.Middle AS PriInsMiddle,  "
							"    PersonT.Last AS PriInsLast, PatientsT.PersonID as PersonID "
							"FROM InsuranceCoT RIGHT OUTER JOIN "
							"    PersonT RIGHT OUTER JOIN "
							"    InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID ON  "
							"    InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID RIGHT "
							"     OUTER JOIN "
							"    PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
							"WHERE (PatientsT.PersonID = ? ) AND (InsuredPartyT.RespTypeID = 1)) AS  PriInsInfo  "
							"ON PatInfo.PatID = PriInsInfo.PersonID "
							"/*End PriInsInfo*/ "
							"LEFT Outer JOIN "
							"/*This query returns secondary insurance information*/ "
							"(SELECT InsuranceCoT.Name as SecInsCo, "
							"	PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS "
							"     SecGuarForward,  "
							"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS SecGuarComma, "
							"     PersonT.First AS SecInsFirst,  "
							"    PersonT.Middle AS SecInsMiddle,  "
							"    PersonT.Last AS SecInsLast, PatientsT.PersonID as PersID "
							"FROM InsuranceCoT RIGHT OUTER JOIN "
							"    PersonT RIGHT OUTER JOIN "
							"    InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID ON  "
							"    InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID RIGHT "
							"     OUTER JOIN "
							"    PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
							" WHERE (PatientsT.PersonID = ? ) AND (InsuredPartyT.RespTypeID = 2)) AS SecInsInfo "
							" /*end SecInsInfo*/ "
							" ON PatInfo.PatID = SecInsInfo.PersID  "
							" Left Outer Join "
							" (SELECT ResponsiblePartyT.PersonID as RespID, ResponsiblePartyT.PatientID AS RespPatID, First as RespFirst, Middle as RespMiddle, Last as RespLast, Address1 as RespAdd1, Address2 as RespAdd2,  "
							" City as RespCity, State as RespState, Zip as RespZip "
							" FROM PersonT Inner Join ResponsiblePartyT ON PersonT.ID = ResponsiblePartyT.PersonID LEFT JOIN PatientsT ON ResponsiblePartyT.PatientID = PatientsT.PersonID @RespFilter) AS ResPartyT  "
							" ON PatInfo.PatID = ResPartyT.RespPatID) AS StatementEndQ ON StatementSubQ.PatID = StatementEndQ.PatID "
							" LEFT OUTER JOIN (SELECT ID, Name AS PracName, Address1 AS PracAddress1, Address2 AS PracAddress2, City AS PracCity, State AS PracState, Zip AS PracZip, Phone As PracPhone, Fax AS PracFax FROM LocationsT) AS StatementLocQ ON "
							" StatementSubQ.LocationFixID = StatementLocQ.ID "
							" LEFT JOIN "
							"(SELECT * FROM [%s]) AS StatementLastPatPayInfoQ " 
							" ON StatementSubQ.LocationFixID = StatementLastPatPayInfoQ.LocID "
							" AND StatementSubQ.PatID = StatementLastPatPayInfoQ.PatID "
							" AND StatementLastPatPayInfoQ.IsInsurance = 0 "
							" LEFT JOIN "
							"(SELECT * FROM [%s]) AS StatementLastInsPayInfoQ " 
							" ON StatementSubQ.LocationFixID = StatementLastInsPayInfoQ.LocID "
							" AND StatementSubQ.PatID = StatementLastInsPayInfoQ.PatID "
							" AND StatementLastInsPayInfoQ.IsInsurance = 1 "
							"/*end StatementEndQ*/", strTempTableName, strTempTableName);

					// (a.walling 2006-10-24 13:12) - PLID 16059 - Multiple Responsible parties
					part11.Replace("@RespFilter", strRespFilter);
						
					return _T(part1 + part2 + part3 + part4 + part5 + part6 + part7 + part8 + part9 + part10 + part11);
					 break;
				 }

		default:
			return "";
		break;
	}
}

CString CStatementSqlBuilder::GetStatementProvString(long nTransProvFormat)
{

	//get how they want the provider name formatted
	CString strTransProv;
	switch (nTransProvFormat)
	{

	case 0:
		//First Last Title
		strTransProv.Format("SELECT First + ' ' + Last + ' ' + Title From PersonT");
		break;

	case 1:
		//First Middle Last Title
		strTransProv.Format("SELECT First + ' ' +  Middle + ' ' + Last + ' ' + Title From PersonT");
		break;

	case 2:
		//Last Title
		strTransProv.Format("SELECT Last + ' ' + Title From PersonT");
		break;

	case 3:
		//Last
		strTransProv.Format("SELECT Last From PersonT");
		break;

	case 4:
		//Prefix Last
		strTransProv.Format("SELECT CASE WHEN Prefix IS NULL THEN '' ELSE Prefix + ' ' END + Last From PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID");
		break;

	case 5:
		//Prefix First Last 
		strTransProv.Format("SELECT CASE WHEN Prefix IS NULL THEN '' ELSE Prefix  + ' ' END + First + ' ' + Last From PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID");
		break;

	case 6:
		//Last, Title
		strTransProv.Format("SELECT Last + ', ' + Title From PersonT");
		break;

	case 7:
		//First MI Last Title
		strTransProv.Format("SELECT First + ' ' + CASE WHEN Middle = '' THEN '' ELSE Left(Middle, 1) + ' ' END + Last + ' ' + Title FROM PersonT");
		break;

	case 8:
		strTransProv.Format("SELECT CASE WHEN Prefix IS NULL THEN '' ELSE Prefix  + ' ' END + First + ' ' + CASE WHEN Middle = '' THEN '' ELSE Left(Middle, 1) + ' ' END + Last From PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID");
		break;

	case 9:
		strTransProv.Format("SELECT CASE WHEN Prefix IS NULL THEN '' ELSE Prefix  + ' ' END + First + ' ' + CASE WHEN Middle = '' THEN '' ELSE Left(Middle, 1) + ' ' END + Last + ' ' + Title From PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID");
		break;
		//that's all I can think of at the moment
	default:
		//First Last Title
		strTransProv.Format("SELECT First + ' ' + Last + ' ' + Title From PersonT");
		break;
	}

	return strTransProv;
}

CString CStatementSqlBuilder::GetStatementChargeDescription()
{

	CString strReturn;

	// (j.jones 2015-03-06 08:24) - PLID 64937 - if the description begins with 'Corrected Charge',
	// strip that off
	CString strLineItemDescription = " (CASE WHEN LineItemCorrectionsT_NewCharge.NewLineItemID Is Not Null AND Left(LineItemT.Description, Len('Corrected Charge')) = 'Corrected Charge' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Charge'))) ELSE LineItemT.Description END) ";

	if (GetRemotePropertyInt("SttmntHideCPTCodes", 0, 0, "<None>"))
	{

		return strLineItemDescription;

	}
	else
	{

		return FormatString(" CASE WHEN CPTCodeT.Code IS NULL THEN %s ELSE CPTCodeT.Code + ' - ' + %s  END ", strLineItemDescription, strLineItemDescription);
	}


}

CString CStatementSqlBuilder::GetStatementUnAppliedPrePaymentsString()
{

	CString strReturn;

	long nHideUnAppliedPrePays = GetRemotePropertyInt("SttmntHidePrePayments");

	if (nHideUnAppliedPrePays)
	{

		strReturn = " AND (PaymentsT.Prepayment = 0 OR PaymentsT.ID IN (SELECT DestID FROM AppliesT)) ";
	}
	else
	{
		strReturn = "";
	}

	return strReturn;
}

//TES 7/17/2014 - PLID 62565 - Added; in some cases two differently aliased chargeback tables both need to be suppressed, so this function requires an input.
CString CStatementSqlBuilder::GetStatementChargebackString(const CString &strChargebacksTAlias)
{

	CString strReturn;

	//TES 7/17/2014 - PLID 62565 - Check the setting. If we don't have the Vision Payments license, use the default of 1
	long nHideChargeBacks = 1;
	if (g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent))
	{
		nHideChargeBacks = GetRemotePropertyInt("SttmntHideChargebacks", 1);
	}

	if (nHideChargeBacks)
	{

		strReturn = " AND (" + strChargebacksTAlias + ".ID Is Null) ";
	}
	else
	{
		strReturn = "";
	}

	return strReturn;
}
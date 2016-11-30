// Rewards.cpp: implementation of the Rewards namespace.
//
//////////////////////////////////////////////////////////////////////

// (a.walling 2007-05-21 14:24) - PLID 20838 - Namespace of utility functions for applying and voiding reward points

#include "stdafx.h"
#include "practice.h"
#include "Rewards.h"
#include "AuditTrail.h"
#include <InternationalUtils.h>

using namespace ADODB;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


void Rewards::Initialize()
{
	// (a.wetta 2007-05-23 16:33) - PLID 25960 - Make sure they have the NexSpa license
	if (!(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)))
		return;

	// at some point, it may be useful to do something else here.
	// for now just refresh the cache.
	Refresh();
}

void Rewards::Refresh()
{
	// bulk cache properties
	g_propManager.CachePropertiesInBulk("Rewards", propNumber,
		"(Username = '<None>') AND ("
		"Name = 'Bill_PointsPerBill' OR "
		"Name = 'Bill_Dollars' OR "
		"Name = 'Bill_Points' OR "
		"Name = 'Charge_Dollars' OR "
		"Name = 'Charge_Points' OR "
		"Name = 'Charge_UsePriceWhenNoPoints' OR "
		"Name = 'RefBill_PointsPerBill' OR "
		"Name = 'RefBill_Dollars' OR "
		"Name = 'RefBill_Points' OR "
		"Name = 'RefBill_RefBillPoints' OR "
		"Name = 'RefBill_BillPoints' OR "
		"Name = 'Ref_PointsPerReferral' "
		")",
		_Q(GetCurrentUserName()));
}

long Rewards::GetReferredByPatientID(long nPatientID)
{
	// (a.walling 2007-10-24 10:40) - PLID 26079 - Changed to use parameterized queries
	_RecordsetPtr prs = CreateParamRecordset("SELECT ReferringPatientID FROM PatientsT WHERE PersonID = {INT}", nPatientID);
	
	if (prs->eof) {
		return -1;
	} else {
		return AdoFldLong(prs, "ReferringPatientID", -1);
	}
}

COleCurrency Rewards::GetTotalPoints(long nPatientID)
{
	// (a.walling 2007-10-24 10:40) - PLID 26079 - Changed to use parameterized queries
	CParamSqlBatch sqlBatch;
	AddTotalPointSqlToBatch(sqlBatch, nPatientID);
	_RecordsetPtr prs = sqlBatch.CreateRecordsetNoTransaction(GetRemoteData());

	return AdoFldCurrency(prs, "TotalPoints", m_cyZero);
}

void Rewards::AddTotalPointSqlToBatch(IN OUT CParamSqlBatch &sqlBatch, long nPatientID)
{
	sqlBatch.Add("SELECT SUM(Points) AS TotalPoints FROM RewardHistoryT WHERE PatientID = {INT} AND Deleted = 0", nPatientID);
}

COleCurrency Rewards::GetBillPoints(long nPatientID, long nBillID)
{
	// (a.walling 2007-10-24 10:40) - PLID 26079 - Changed to use parameterized queries
	_RecordsetPtr prs = CreateParamRecordset("SELECT SUM(Points) AS BillPoints FROM RewardHistoryT WHERE Source >= 0 AND PatientID = {INT} AND BillID = {INT} AND Deleted = 0",
		nPatientID, nBillID);

	return AdoFldCurrency(prs, "BillPoints", m_cyZero);
}

////////////////////////
// Apply reward points

COleCurrency Rewards::UpdateBillAll(long nPatientID, long nBillID)
{
	return ApplyBillAll(nPatientID, nBillID, TRUE);
}

COleCurrency Rewards::ApplyBillAll(long nPatientID, long nBillID, BOOL bUpdate /* = FALSE */)
{
	// (a.wetta 2007-05-23 16:33) - PLID 25960 - Make sure they have the NexSpa license
	if (!(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)))
		return m_cyZero;

	COleCurrency cyPoints_Bill;
	COleCurrency cyPoints_BillTotal;
	COleCurrency cyPoints_ChargeTotal;

	// 1. Get Points per Bill
	cyPoints_Bill = GetBillPoints();
	
	// 2. Get Points per Bill Dollars
	// 3. Get Points per Charge
	// only accumulate price based points when service points is null or zero
	// (a.walling 2007-10-24 12:38) - PLID 26079 - Use our param utils rather than Command objects
	// (j.gruber 2010-07-29 12:05) - PLID 31147 - discoutn rewards
	_RecordsetPtr prs = CreateParamRecordset(
		" SET NOCOUNT ON; \r\n "
		"\r\n "
		" DECLARE @nBillID INT; \r\n "
		" SET @nBillID = {INT}; \r\n "
		" DECLARE @nPref INT \r\n "
		" SET @nPref = {INT}; \r\n "
		" DECLARE @cyChargeAmt money; \r\n "
		" DECLARE @cyDiscAmt money; \r\n "
		" DECLARE @cyPoints money; \r\n "
		" DECLARE @cyPointsToUse money; \r\n "
		" DECLARE @flQuantity float; \r\n "
		" SET @cyPointsToUse = 0; \r\n "
		" DECLARE @nChargeID INT \r\n "
		"  \r\n "
		" IF @nPref = 1 BEGIN	 \r\n "
		" 	DECLARE rsCharges CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR  \r\n "
		" 	SELECT ChargesT.ID, ChargesT.Quantity, Coalesce(ServiceT.Points,0) as Points, \r\n "
		" 	Round(Convert(money,((((LineItemT.[Amount])*([Quantity])*(CASE WHEN((CPTMultiplier1) Is Null) THEN 1 ELSE (CPTMultiplier1) END)*(CASE WHEN (CPTMultiplier2) Is Null THEN 1 ELSE (CPTMultiplier2) END)*(CASE WHEN (CPTMultiplier3) Is Null THEN 1 ELSE (CPTMultiplier3) END)*(CASE WHEN (CPTMultiplier4) Is Null THEN 1 ELSE (CPTMultiplier4) END)* (CASE WHEN(([TotalPercentOff]) Is Null) THEN 1 ELSE ((100-Convert(float,([TotalPercentOff])))/100) END)-(CASE WHEN ([TotalDiscount]) Is Null THEN 0 ELSE ([TotalDiscount]) END))))), 2)  AS FullChargeNoTax,  \r\n "
		" 	Round(Convert(money,((((LineItemT.[Amount])*([Quantity])*(CASE WHEN((CPTMultiplier1) Is Null) THEN 1 ELSE (CPTMultiplier1) END)*(CASE WHEN (CPTMultiplier2) Is Null THEN 1 ELSE (CPTMultiplier2) END)*(CASE WHEN (CPTMultiplier3) Is Null THEN 1 ELSE (CPTMultiplier3) END)*(CASE WHEN (CPTMultiplier4) Is Null THEN 1 ELSE (CPTMultiplier4) END))))), 2)  AS FullChargeNoTaxDisc \r\n "
		" 	FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID \r\n "		
		"	INNER JOIN (SELECT * FROM ChargeRespT WHERE InsuredPartyID IS NULL AND Amount > CONVERT(money,0)) ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
		" 	LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID \r\n "
		" 	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID \r\n "
		" 	WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = @nBillID \r\n "
		" 	OPEN rsCharges  \r\n "
		" 	FETCH FROM rsCharges INTO @nChargeID, @flQuantity, @cyPoints, @cyDiscAmt, @cyChargeAmt \r\n "
		" 	WHILE @@FETCH_STATUS = 0 BEGIN  		 \r\n "
		" 		if (@cyDiscAmt = @cyChargeAmt) BEGIN \r\n "
		" 			SET @cyPointsToUse = @cyPointsToUse + (@cyPoints * @flQuantity) \r\n "
		" 		END \r\n "
		" 		ELSE BEGIN \r\n "
		" 			DECLARE @dblMultiplier float \r\n "
		" 			SET @dblMultiplier =  @cyDiscAmt/@cyChargeAmt \r\n "
		" 			--now multiply that by the points \r\n "
		" 			SET @cyPointsToUse = @cyPointsToUse + ((@cyPoints * @dblMultiplier) * @flQuantity) \r\n "
		" 		END \r\n "
		" 	FETCH FROM rsCharges INTO @nChargeID, @flQuantity, @cyPoints, @cyDiscAmt, @cyChargeAmt \r\n "
		" 	END \r\n "
		" END \r\n "
		" ELSE  \r\n "
		" BEGIN \r\n "
		" 	SET @cyPointsToUse = (SELECT Sum(COALESCE(ServiceT.Points,0)) FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID \r\n "
		" 					LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID \r\n "
		" 					WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = @nBillID); \r\n "
		" END \r\n "
		" SET NOCOUNT OFF; \r\n "
		" SELECT SUM(ChargeRespT.Amount) AS BillTotal,  \r\n "
		" 		@cyPointsToUse AS PointTotal,  \r\n "
		" 		SUM(CASE   \r\n "
		" 			WHEN ServiceT.Points IS NULL OR ServiceT.Points = 0 THEN ChargeRespT.Amount  \r\n "
		" 			ELSE 0  \r\n "
		" 			END) AS PointAdjustedBillTotal,  \r\n "
		" 		MAX(ChargesT.BillID) AS BillID  \r\n "
		" 		FROM   ChargeRespT  \r\n "
		" 		INNER JOIN ChargesT  \r\n "
		" 			ON ChargeRespT.ChargeID = ChargesT.ID  \r\n "
		" 		INNER JOIN LineItemT  \r\n "
		" 			ON ChargesT.ID = LineItemT.ID  \r\n "
		" 		LEFT JOIN ServiceT  \r\n "
		" 			ON ChargesT.ServiceID = ServiceT.ID  \r\n "
		" 		WHERE  LineItemT.Deleted = 0  \r\n "
		" 		AND ChargeRespT.InsuredPartyID IS NULL   \r\n "
		" 		AND ChargesT.BillID = @nBillID" , nBillID, GetRemotePropertyInt("RewardCalcDiscountInPoints", 0, 0, "<None>", true));
		

	COleCurrency cyBillTotal;
	COleCurrency cyPointAdjustedBillTotal;

	BOOL bPatBill = TRUE;

	if (prs->eof) {
		// (a.walling 2007-10-24 16:06) - PLID 26079 - No charges that are pat resp.
		// (although since we are selecting sums we should just get a row of nulls)
		bPatBill = FALSE;

		// (a.walling 2007-10-25 15:39) - PLID 26079 - Actually, don't even apply the per-bill
		// points if there are no pat resp charges. We can accomplish this two ways. I'm using 
		// both here for clarity.
		cyPoints_Bill = m_cyZero;
	} else {
		if (AdoFldLong(prs, "BillID", -1) == -1) {
			// (a.walling 2007-10-24 16:06) - PLID 26079 - No charges that are pat resp.
			bPatBill = FALSE;
			cyPoints_Bill = m_cyZero;
		}
		cyBillTotal = AdoFldCurrency(prs, "BillTotal", m_cyZero);
		if (cyBillTotal == m_cyZero) {
			// bill has no pat resp charges
			bPatBill = FALSE;
			cyPoints_Bill = m_cyZero;
		}
		cyPointAdjustedBillTotal = AdoFldCurrency(prs, "PointAdjustedBillTotal", m_cyZero);
		COleCurrency cyBillPointTotal = AdoFldCurrency(prs, "PointTotal", m_cyZero);
		
		if (!GetUsePriceWhenNoPoints()) {
			// if we are accumulating regardless, then the PointAdjustedBillTotal just needs to be equal to the bill total.
			cyPointAdjustedBillTotal = cyBillTotal;
		}

		cyPoints_BillTotal = GetPointsPerBillDollars(cyBillTotal);
		cyPoints_ChargeTotal = GetPointsPerChargeDollars(cyPointAdjustedBillTotal);
		cyPoints_ChargeTotal += cyBillPointTotal;
	}

	COleCurrency cyPoints_Total = cyPoints_Bill + cyPoints_BillTotal + cyPoints_ChargeTotal;

	/*
	if (cyPoints_Bill == m_cyZero)
		cyPoints_Bill.SetStatus(COleCurrency::null);
	if (cyPoints_BillTotal == m_cyZero)
		cyPoints_BillTotal.SetStatus(COleCurrency::null);
	if (cyPoints_ChargeTotal == m_cyZero)
		cyPoints_ChargeTotal.SetStatus(COleCurrency::null);
	*/
	
	long nReferredByPatientID = GetReferredByPatientID(nPatientID);

	CString strSql = BeginSqlBatch();

	if (bUpdate) {
		COleCurrency cyExist_Bill, cyExist_BillTotal, cyExist_ChargeTotal, cyExist_RefBillDollars, cyExist_RefBillPoints;
		cyExist_Bill.SetStatus(COleCurrency::null);
		cyExist_BillTotal.SetStatus(COleCurrency::null);
		cyExist_ChargeTotal.SetStatus(COleCurrency::null);
		cyExist_RefBillDollars.SetStatus(COleCurrency::null);
		cyExist_RefBillPoints.SetStatus(COleCurrency::null);

		// (a.walling 2007-10-24 10:40) - PLID 26079 - Changed to use parameterized queries
		_RecordsetPtr prs = CreateParamRecordset("SELECT Source, Points FROM RewardHistoryT "
			"WHERE BillID = {INT} AND Deleted = 0 AND (PatientID = {INT} AND Source IN ({INT}, {INT}, {INT})) "
			"OR (RefPatientID = {INT} AND Source IN ({INT}, {INT}))",
			nBillID, nPatientID, ersPerBill, ersPerBillDollars, ersPerBillCharges,
			nPatientID, ersPerRefBillDollars, ersPerRefBillPoints);

		if (prs->eof) {
			bUpdate = FALSE;
		} else {

			while (!prs->eof) {
				ERewardSource ers = (ERewardSource)AdoFldLong(prs, "Source");

				switch (ers) {
					case ersPerBill:
						cyExist_Bill += AdoFldCurrency(prs, "Points");
						break;
					case ersPerBillDollars:
						cyExist_BillTotal += AdoFldCurrency(prs, "Points");
						break;
					case ersPerBillCharges:
						cyExist_ChargeTotal += AdoFldCurrency(prs, "Points");
						break;
					case ersPerRefBillDollars:
						cyExist_RefBillDollars += AdoFldCurrency(prs, "Points");
						break;
					case ersPerRefBillPoints:
						cyExist_RefBillPoints += AdoFldCurrency(prs, "Points");
						break;					
					default:
						ASSERT(FALSE);
						break;
				}

				prs->MoveNext();
			}

			if (cyPoints_Bill != cyExist_Bill) {
				AddStatementToSqlBatch(strSql, 
				"UPDATE RewardHistoryT SET Deleted = %li, DeletedDate = GetDate() WHERE "
				"PatientID = %li AND Source = %li AND BillID = %li AND Deleted = 0", erdrChanged, nPatientID, ersPerBill, nBillID);

				// (a.walling 2007-10-25 15:41) - PLID 26079 - Don't apply points if only insurance charges
				if ((cyPoints_Bill > m_cyZero) && bPatBill) {
					AddStatementToSqlBatch(strSql, 
					"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, Points) "
					"VALUES (%li, %li, %s, %li, %s)", nPatientID, ersPerBill, FormatCurrencyForSql(m_cyZero), nBillID, FormatCurrencyForSql(cyPoints_Bill));
				}
			}
			
			if (cyPoints_BillTotal != cyExist_BillTotal) {
				AddStatementToSqlBatch(strSql, 
				"UPDATE RewardHistoryT SET Deleted = %li, DeletedDate = GetDate() WHERE "
				"PatientID = %li AND Source = %li AND BillID = %li AND Deleted = 0", erdrChanged, nPatientID, ersPerBillDollars, nBillID);

				if (cyPoints_BillTotal > m_cyZero) {
					AddStatementToSqlBatch(strSql, 
					"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, Points) "
					"VALUES (%li, %li, %s, %li, %s)", nPatientID, ersPerBillDollars, FormatCurrencyForSql(cyBillTotal), nBillID, FormatCurrencyForSql(cyPoints_BillTotal));
				}
			}
			
			if (cyPoints_ChargeTotal != cyExist_ChargeTotal) {
				AddStatementToSqlBatch(strSql, 
				"UPDATE RewardHistoryT SET Deleted = %li, DeletedDate = GetDate() WHERE "
				"PatientID = %li AND Source = %li AND BillID = %li AND Deleted = 0", erdrChanged, nPatientID, ersPerBillCharges, nBillID);

				if (cyPoints_ChargeTotal > m_cyZero) {
					AddStatementToSqlBatch(strSql, 
					"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, Points) "
					"VALUES (%li, %li, %s, %li, %s)", nPatientID, ersPerBillCharges, FormatCurrencyForSql(cyPointAdjustedBillTotal), nBillID, FormatCurrencyForSql(cyPoints_ChargeTotal));
				}
			}
			
			// 4. Handle referred patient bill points
			{				
				COleCurrency cyPoints_RefBillDollars = GetPointsPerRefBillDollars(cyBillTotal);
				COleCurrency cyPoints_RefBillPoints = GetPointsPerRefBillPoints(cyPoints_Total);
			
				
				if (cyPoints_RefBillDollars != cyExist_RefBillDollars) {
					AddStatementToSqlBatch(strSql, 
					"UPDATE RewardHistoryT SET Deleted = %li, DeletedDate = GetDate() WHERE "
					"RefPatientID = %li AND Source = %li AND BillID = %li AND Deleted = 0", erdrChanged, nPatientID, ersPerRefBillDollars, nBillID);

					// (a.walling 2007-10-24 11:34) - PLID 26079 - don't award points for bills that have no patient responsibility
					if (bPatBill && nReferredByPatientID > -1 && cyPoints_RefBillDollars > m_cyZero) {
						AddStatementToSqlBatch(strSql, 
						"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, RefPatientID, Points) "
						"VALUES (%li, %li, %s, %li, %li, %s)", nReferredByPatientID, ersPerRefBillDollars, FormatCurrencyForSql(cyBillTotal), nBillID, nPatientID, FormatCurrencyForSql(cyPoints_RefBillDollars));
					}
				}
				
				if (cyPoints_RefBillPoints != cyExist_RefBillPoints) {
					AddStatementToSqlBatch(strSql, 
					"UPDATE RewardHistoryT SET Deleted = %li, DeletedDate = GetDate() WHERE "
					"RefPatientID = %li AND Source = %li AND BillID = %li AND Deleted = 0", erdrChanged, nPatientID, ersPerRefBillPoints, nBillID);

					// (a.walling 2007-10-24 11:34) - PLID 26079 - don't award points for bills that have no patient responsibility
					if (bPatBill && nReferredByPatientID > -1 && cyPoints_RefBillPoints > m_cyZero) {
						AddStatementToSqlBatch(strSql, 
						"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, RefPatientID, Points) "
						"VALUES (%li, %li, %s, %li, %li, %s)", nReferredByPatientID, ersPerRefBillPoints, FormatCurrencyForSql(cyPoints_Total), nBillID, nPatientID, FormatCurrencyForSql(cyPoints_RefBillPoints));
					}
				}	
			}	
		}
	}
	
	if (!bUpdate) {
		// (a.walling 2007-10-25 15:41) - PLID 26079 - Don't apply points if only insurance charges
		if ((cyPoints_Bill > m_cyZero) && bPatBill) {
			AddStatementToSqlBatch(strSql, 
			"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, Points) "
			"VALUES (%li, %li, %s, %li, %s)", nPatientID, ersPerBill, FormatCurrencyForSql(m_cyZero), nBillID, FormatCurrencyForSql(cyPoints_Bill));
		}
		
		if (cyPoints_BillTotal > m_cyZero) {
			AddStatementToSqlBatch(strSql, 
			"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, Points) "
			"VALUES (%li, %li, %s, %li, %s)", nPatientID, ersPerBillDollars, FormatCurrencyForSql(cyBillTotal), nBillID, FormatCurrencyForSql(cyPoints_BillTotal));
		}
		
		if (cyPoints_ChargeTotal > m_cyZero) {
			AddStatementToSqlBatch(strSql, 
			"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, Points) "
			"VALUES (%li, %li, %s, %li, %s)", nPatientID, ersPerBillCharges, FormatCurrencyForSql(cyPointAdjustedBillTotal), nBillID, FormatCurrencyForSql(cyPoints_ChargeTotal));
		}
							
		// 4. Handle referred patient bill points
		{
			if (bPatBill) { // (a.walling 2007-10-24 11:34) - PLID 26079 - don't award points for bills that have no patient responsibility
				if (nReferredByPatientID >= 0) {
					// this patient was referred by a valid patient.
					
					COleCurrency cyPoints_RefBill = GetRefBillPoints();
					COleCurrency cyPoints_RefBillDollars = GetPointsPerRefBillDollars(cyBillTotal);
					COleCurrency cyPoints_RefBillPoints = GetPointsPerRefBillPoints(cyPoints_Total);
					
					if (cyPoints_RefBill > m_cyZero) {
						AddStatementToSqlBatch(strSql, 
						"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, RefPatientID, Points) "
						"VALUES (%li, %li, %s, %li, %li, %s)", nReferredByPatientID, ersPerRefBill, FormatCurrencyForSql(m_cyZero), nBillID, nPatientID, FormatCurrencyForSql(cyPoints_RefBill));
					}
					
					if (cyPoints_RefBillDollars > m_cyZero) {
						AddStatementToSqlBatch(strSql, 
						"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, RefPatientID, Points) "
						"VALUES (%li, %li, %s, %li, %li, %s)", nReferredByPatientID, ersPerRefBillDollars, FormatCurrencyForSql(cyBillTotal), nBillID, nPatientID, FormatCurrencyForSql(cyPoints_RefBillDollars));
					}
					
					if (cyPoints_RefBillPoints > m_cyZero) {
						AddStatementToSqlBatch(strSql, 
						"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, BillID, RefPatientID, Points) "
						"VALUES (%li, %li, %s, %li, %li, %s)", nReferredByPatientID, ersPerRefBillPoints, FormatCurrencyForSql(cyPoints_Total), nBillID, nPatientID, FormatCurrencyForSql(cyPoints_RefBillPoints));
					}			
				}	
			}
		}
	}
	
	if (strSql.GetLength() > 0)
		ExecuteSqlBatch(strSql);
	
	return cyPoints_Total;
}


// points are applied to the ReferredByPatient, not the RefPatient
COleCurrency Rewards::ApplyRefPatient(long nReferredPatientID)
{
	// (a.wetta 2007-05-23 16:33) - PLID 25960 - Make sure they have the NexSpa license
	if (!(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)))
		return m_cyZero;

	COleCurrency cyPointsPerReferral = GetPointsPerReferral();
	
	if (cyPointsPerReferral > m_cyZero) {
		// throws contraint exception if there is no referring patient, since NULL cannot be in the PatientID field.
		// (a.walling 2007-10-24 10:40) - PLID 26079 - Changed to use parameterized queries
		_CommandPtr pCmd = OpenParamQuery(
			"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, RefPatientID, Points) "
			"VALUES((SELECT ReferringPatientID FROM PatientsT WHERE PersonID = ?), ?, ?, ?, ?)");
		AddParameterLong(pCmd, "nReferredPatientID", nReferredPatientID);
		AddParameterLong(pCmd, "ersPerRefPatient", ersPerRefPatient);
		AddParameter(pCmd, "cyZero", ADODB::adCurrency, ADODB::adParamInput, 8, _variant_t(m_cyZero));
		AddParameterLong(pCmd, "nReferredPatientID", nReferredPatientID);
		AddParameter(pCmd, "cyPointsPerReferral", ADODB::adCurrency, ADODB::adParamInput, 8, _variant_t(cyPointsPerReferral));

		CreateRecordset(pCmd);
	}
	
	return cyPointsPerReferral;
}

COleCurrency Rewards::ApplyRefPatient(long nReferredPatientID, long nPatientID)
{
	COleCurrency cyPointsPerReferral = GetPointsPerReferral();
	
	if (cyPointsPerReferral > m_cyZero) {
		// throws contraint exception if there is no referring patient, since NULL cannot be in the PatientID field.
		// (a.walling 2007-10-24 10:40) - PLID 26079 - Changed to use parameterized queries
		_CommandPtr pCmd = OpenParamQuery(
			"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, RefPatientID, Points) "
			"VALUES(?, ?, ?, ?, ?)");

		AddParameterLong(pCmd, "nPatientID", nPatientID);
		AddParameterLong(pCmd, "ersPerRefPatient", ersPerRefPatient);
		AddParameter(pCmd, "cyZero", ADODB::adCurrency, ADODB::adParamInput, 8, _variant_t(m_cyZero));
		AddParameterLong(pCmd, "nReferredPatientID", nReferredPatientID);
		AddParameter(pCmd, "cyPointsPerReferral", ADODB::adCurrency, ADODB::adParamInput, 8, _variant_t(cyPointsPerReferral));

		CreateRecordset(pCmd);
	}
	
	return cyPointsPerReferral;
}

////////////////////////
// Unapply reward points

void Rewards::UnapplyBillAll(long nBillID, ERewardsDeletedReason erdrReason)
{
	// (a.walling 2007-10-24 10:40) - PLID 26079 - Changed to use parameterized queries
	ExecuteParamSql(
		"UPDATE RewardHistoryT SET Deleted = {INT}, DeletedDate = GetDate() "
		"WHERE Deleted = 0 AND BillID = {INT} "
		"AND Source in ({INT}, {INT}, {INT}, {INT}, {INT}, {INT})",
		erdrReason, nBillID, ersPerBill, ersPerBillDollars, ersPerBillCharges, ersPerRefBill, ersPerRefBillDollars, ersPerRefBillPoints);
}

// points are unapplied from the ReferredByPatient
void Rewards::UnapplyRefPatient(long nPatientID, ERewardsDeletedReason erdrReason)
{
	// a patient can be referred by only one patient.
	// (a.walling 2007-10-24 10:40) - PLID 26079 - Changed to use parameterized queries
	ExecuteParamSql(
		"UPDATE RewardHistoryT SET Deleted = {INT}, DeletedDate = GetDate() "
		"WHERE Deleted = 0 AND Source = {INT} AND RefPatientID = {INT}",
		erdrReason, ersPerRefPatient, nPatientID);
}

////////////////////////

BOOL Rewards::GetUsePriceWhenNoPoints()
{
	return GetRemotePropertyInt("Charge_UsePriceWhenNoPoints", 0, 0, "<None>", false);	
}

COleCurrency Rewards::GetBillPoints()
{
	return COleCurrency(GetRemotePropertyInt("Bill_PointsPerBill", 0, 0, "<None>", false), 0);	
}

COleCurrency Rewards::GetRefBillPoints()
{
	return COleCurrency(GetRemotePropertyInt("RefBill_PointsPerBill", 0, 0, "<None>", false), 0);	
}

COleCurrency Rewards::GetPointsPerBillDollars(const COleCurrency &cyValue)
{
	long nPoints = GetRemotePropertyInt("Bill_Points", 0, 0, "<None>", false);	
	long nDollars = GetRemotePropertyInt("Bill_Dollars", 0, 0, "<None>", false);	

	return nDollars == 0 ? m_cyZero : (cyValue * nPoints) / nDollars;
}

COleCurrency Rewards::GetPointsPerRefBillDollars(const COleCurrency &cyValue)
{
	long nPoints = GetRemotePropertyInt("RefBill_Points", 0, 0, "<None>", false);	
	long nDollars = GetRemotePropertyInt("RefBill_Dollars", 0, 0, "<None>", false);	

	return nDollars == 0 ? m_cyZero : (cyValue * nPoints) / nDollars;
}

COleCurrency Rewards::GetPointsPerRefBillPoints(const COleCurrency &cyValue)
{
	long nRefBillPoints = GetRemotePropertyInt("RefBill_RefBillPoints", 0, 0, "<None>", false);	
	long nBillPoints = GetRemotePropertyInt("RefBill_BillPoints", 0, 0, "<None>", false);	

	return nRefBillPoints == 0 ? m_cyZero : (cyValue * nBillPoints) / nRefBillPoints;
}

COleCurrency Rewards::GetPointsPerChargeDollars(const COleCurrency &cyValue)
{
	long nPoints = GetRemotePropertyInt("Charge_Points", 0, 0, "<None>", false);
	long nDollars = GetRemotePropertyInt("Charge_Dollars", 0, 0, "<None>", false);	

	return nDollars == 0 ? m_cyZero : (cyValue * nPoints) / nDollars;
}

COleCurrency Rewards::GetPointsPerReferral()
{
	return COleCurrency(GetRemotePropertyInt("Ref_PointsPerReferral", 0, 0, "<None>", false), 0);	
}

// (z.manning 2010-07-20 14:07) - PLID 30127
void Rewards::ManuallyAdjustRewardPoints(const long nPatientID, const COleCurrency cyOldPoints, const COleCurrency cyNewPoints)
{
	if(cyOldPoints == cyNewPoints) {
		// (z.manning 2010-08-03 17:16) - PLID 30127 - Nothing changed so no need to do anything
		return;
	}

	COleCurrency cyAdjust = cyNewPoints - cyOldPoints;
	ExecuteParamSql(
		"INSERT INTO RewardHistoryT(PatientID, Source, SourceValue, Points) \r\n"
		"VALUES ({INT}, {INT}, {VT_CY}, {VT_CY}) \r\n"
		, nPatientID, ersManualAdjustment, _variant_t(m_cyZero), _variant_t(cyAdjust));

	// (z.manning 2010-07-20 14:40) - PLID 30127 - Audit
	CString strOld = FormatCurrencyForInterface(cyOldPoints, FALSE);
	CString strNew = "Manually changed to: " + FormatCurrencyForInterface(cyNewPoints, FALSE);
	AuditEvent(nPatientID, GetExistingPatientName(nPatientID), BeginNewAuditEvent(), aeiPatientRewardPoints, nPatientID, strOld, strNew, aepMedium, aetChanged);
}
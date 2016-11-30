////////////////
// DRT 8/6/03 - GetSqlFinancial() function from ReportInfoCallback
//		Also includes GetSqlAR() and GetSqlStatement()
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "DateTimeUtils.h"
#include "InvUtils.h"
#include "GlobalFinancialUtils.h"
#include "ReportAdo.h"
#include "NxSystemUtilitiesLib\MiscSystemUtils.h"
#include "MsgBox.h"
#include <NxSystemUtilitiesLib/NxThread.h>
#include "StatementSql.h"

/* AllFinancialQ Version History
TES 8/23/2004 - Copied here from MonthlyFinancialQ, referenced in all the Financial Activity reports plus Financial Summary.
TES 8/24/2004 - Added sections for GCSold and GCRedeemed; filtered GC charges out of the BILL and CHARGE sections.
TES 9/20/2004 - NOTE: The Amt field is used for Bills, Charges, Gift Certificates Sold, and
Prepayments Applied.  All other fields use the Amount field.  Please don't ask me to justify or
explain this.
TES 3/8/2005  - Changed the BILL section to use the LineItemT.LocationID (as it always should have).  Also added a new
PAYAPPLY section (**which uses Amt**) which handles discrepancies caused by the fact that payments'
locations magically change when you apply them to a bill with a different location.
TES 4/29/2005 - Modified any payment sections that were using the charge provider; that is all handled now using the
same PAYAPPLY section that takes care of the location discrepancies.
TES 4/29/2005 - Per Burk's demands, added field for InsResp, if 0 it's patient responsibility, otherwise insurance.
TES 4/29/2005 - Also per Burk's demands, added field for PayCategory.
TES 5/2/2005  - Those previous changes necessitated changing all the UNIONs to UNION ALLs.
TES 5/13/2005 - Sigh.  Undid a bunch of previous changes.  The payments are going back to using the provider/location of
their associated charges, and the PAYAPPLY section has been renamed to TRANSFER, and a new column,
TransferredFromDate, which is NULL for all types other than TRANSFER, but is the payment date for the
TRANSFER type.
DRT 5/18/2005 - APPPRE only - Changed the location filter to be the destination in all cases (chg if pay->chg, pay2 if pay->pay2)
TES 10/6/2006 - PLID 22678 - Fixed bug where if a payment was applied to an item with no provider, it would use the payment's
provider, not the destination item's provider.
ZM 10/31/2006 - PLID 23298 - Added totals for tax1 and tax2.
ADW 11/13/2006 - PLID 23209 - Added the relevant Code, CodeName, ServiceID, and Code Category to prepayments as applied to charges.
DRT 3/29/2007 - PLID 25414 - Made some big display changes, but not much to the functionality.  I broke down the "1 big ugly query"
into individual queries for each UNION'd section.  This makes it a lot clearer to read, and if you need to
make changes, you now need only look for the specific section in question.  While doing this, I also made
it immensely easier to actually read these queries.  I finally got rid of all the old access junk of extra
parenthesis, some of the really weird RIGHT JOIN attempts, and found a number of useless WHERE clause filters
that weren't actually doing anything.  While doing all this, I made a few optimizations to each individual
piece of the total query.  I have now put a version history on each of those sections as well, and I noted
exactly what changes I made in each.
EML 06/19/2007 - PLID 26347 - Added the PreTaxChargeAmt field to the charges section and empty values to the rest of the unioned sections
EML 06/19/2007 - PLID 26347 - Added the Total tax fields, and PreTaxChargeAmt field to the bills section, replacing the zero amounts.
d.moore 2007-10-02 - PLID 25166 - Added in fields for discount, percent off, and total discount
// (j.gruber 2008-02-22 18:02) - PLID 29166 - added inline filters
TES 7/9/2008 - PLID 29580 - Made it so that applies are treated as having happened on AppliesT.InputDate, not on the later
of the two dates involved in the apply.
// (j.gruber 2009-04-01 12:22) - PLID 33358 - updated discount structure
TES 10/13/2009 - PLID 35662 - Restored the old apply date calculation for everything but the input date
// (j.gruber 2010-01-07 11:26) - PLID 36747 - optimized the bills query and charges querywith regards to the discount structure


*/

// (c.haag 2016-02-18) - PLID 68460 - Added reportID so we can figure out which connection to create temp tables for
// (c.haag 2016-03-14) - PLID 68592 - strUserNameFilter is now strExternalFilter. It was always this way; and poorly labeled and mangled.
const CString GetAllFinancialQ(int reportID, CString strLocFilter = "", CString strProvFilter = "", CString strPatientFilter = "", CString strBillDateFilter = "", CString strServiceDateFilter = "", CString strInputDateFilter = "", CString strExternalFilter = "");
const CString GetAllFinancialQ_Insurance(int reportID, CString strLocFilter = "", CString strProvFilter = "", CString strPatientFilter = "", CString strBillDateFilter = "", CString strServiceDateFilter = "", CString strInputDateFilter = "", CString strExternalFilter = "");
// (r.gonet 2016-03-14 19:21) - PLID 68570 - Returns a SQL statement which selects the patient's balance and other related fields for reports.
const CString GetPatientBalanceQ(int reportID);
// (j.gruber 2010-03-11 11:34) - PLID 37713 - split this out
const CString GetTransferFinType(CString strLineItemFilter);

#define ALL_FINANCIAL_Q(reportID)		+ GetAllFinancialQ(reportID)+
#define ALL_FINANCIAL_Q_FILTERED(reportID, strLocFilter, strProvFilter, strPatientFilter, strBillDateFilter, strServiceDateFilter, strInputDateFilter, strUserNameFilter) + GetAllFinancialQ(reportID, strLocFilter, strProvFilter, strPatientFilter, strBillDateFilter, strServiceDateFilter, strInputDateFilter, strExternalFilter)+

// (c.haag 2016-02-18) - PLID 68460 - Appends SQL to a where clause
void AppendToWhereClause(CString& strWhereClause, const CString& strSql)
{
	if (!strSql.IsEmpty())
	{
		if (strWhereClause.IsEmpty())
		{
			strWhereClause = FormatString("WHERE (%s)", strSql);
		}
		else
		{
			strWhereClause += FormatString(" AND (%s)", strSql);
		}
	}
}

// (c.haag 2016-03-14) - PLID 68592 - Filter to pass into GetAllFinancialQFragmentAsTempTableInsert
class CAllFinancialQFragmentFilter
{
public:
	CString m_strBillDateFilter;
	CString m_strServiceDateFilter;
	CString m_strInputDateFilter;
	CString m_strInputUserFilter;
	// (j.jones 2016-03-15 15:10) - PLID 68598 - added location filter
	CString m_strLocationFilter;
};

// (c.haag 2016-02-18) - PLID 68460 - Converts a fragment of AllFinancialQ into one that inserts into a temp table, and filters it based on filters that would be applied to the parent query.
// (c.haag 2016-03-14) - PLID 68592 - We now pass in all the filters in CAllFinancialQFragmentFilter
const CString GetAllFinancialQFragmentAsTempTableInsert(const CString& strSubQuery, const CString& strTempT, const CAllFinancialQFragmentFilter& fragmentFilter)
{
	CString strWhere;
	AppendToWhereClause(strWhere, fragmentFilter.m_strBillDateFilter);
	AppendToWhereClause(strWhere, fragmentFilter.m_strServiceDateFilter);
	AppendToWhereClause(strWhere, fragmentFilter.m_strInputDateFilter);
	AppendToWhereClause(strWhere, fragmentFilter.m_strInputUserFilter);
	// (j.jones 2016-03-15 15:10) - PLID 68598 - added location filter
	AppendToWhereClause(strWhere, fragmentFilter.m_strLocationFilter);

	CString strFinalSubQuery = FormatString(R"(
INSERT INTO [%s]
	SELECT * FROM (%s)
	TempTableInsertSubQ
	%s)"
		, strTempT
		, strSubQuery
		, strWhere);

	return strFinalSubQuery;
}

// (c.haag 2016-02-18) - PLID 68460 - Added reportID so we can figure out which connection to create temp tables for
// (c.haag 2016-03-14) - PLID 68592 - strUserNameFilter is now strExternalFilter. It was always this way; and poorly labeled and mangled.
const CString GetAllFinancialQ(int reportID, CString strLocFilter /*=""*/, CString strProvFilter /*=""*/, CString strPatientFilter /*=""*/, CString strBillDateFilter /*=""*/, CString strServiceDateFilter /*=""*/, CString strInputDateFilter /*=""*/, CString strExternalFilter /*=""*/)
{

	//fill in all the variable tables
	CString strLineItemFilter, strBillFilter, strChargeFilter, strPayFilter,
		strPersonFilter, strUserFilter, strLineItemDateFilter, strGCBillFilter,
		strLineItemLocationFilter;
	CString strTemp;
	// (c.haag 2016-03-14) - PLID 68592 - Added bHasSupportedExtendedFilter. When true, it means strExternalFilter is not empty,
	// that we recognize the field it tries to filter on and that we will filter on it within a call to GetAllFinancialQFragmentAsTempTableInsert
	// (j.jones 2016-03-15 15:10) - PLID 68598 - added location filter
	BOOL bHasBillDate, bHasServiceDate, bHasInputDate, bHasSupportedExtendedFilter, bHasLocationFilter;

	// (c.haag 2016-03-14) - PLID 68592 - Filter to pass into the GetAllFinancialQFragmentAsTempTableInsert
	CAllFinancialQFragmentFilter fragmentFilter;

	bHasBillDate = FALSE;
	bHasServiceDate = FALSE;
	bHasInputDate = FALSE;
	bHasSupportedExtendedFilter = FALSE;
	bHasLocationFilter = FALSE;


	strLineItemFilter = " SELECT * FROM LineItemT WHERE (DELETED = 0)";
	strBillFilter = " SELECT * FROM BillsT WHERE (DELETED = 0)";
	strGCBillFilter = " SELECT * FROM BillsT WHERE (DELETED = 0)";
	strChargeFilter = " SELECT * FROM ChargesT WHERE (1=1)";
	strPayFilter = " SELECT * FROM PaymentsT WHERE (1=1)";
	strLineItemDateFilter = " ";
	strLineItemLocationFilter = "";

	if (!strLocFilter.IsEmpty()) {

		// (j.jones 2016-03-15 15:10) - PLID 68598 - added location filter
		bHasLocationFilter = TRUE;
		fragmentFilter.m_strLocationFilter = strLocFilter;

		strLocFilter.Replace("LocID", "LocationID");

		//(e.lally 2008-10-10) PLID 31421 - Support filtering on multiple locations
		strLineItemLocationFilter.Format(" AND %s", strLocFilter);
	}

	if (!strProvFilter.IsEmpty()) {
		strTemp.Format("%s AND DoctorsProviders IN (%s)", strChargeFilter, strProvFilter);
		strChargeFilter = strTemp;

		strTemp.Format("%s AND ProviderID IN (%s)", strPayFilter, strProvFilter);
		strPayFilter = strTemp;

	}

	if (!strPatientFilter.IsEmpty()) {
		strTemp.Format("%s AND %s", strLineItemFilter, strPatientFilter);
		strLineItemFilter = strTemp;
	}

	if (!strBillDateFilter.IsEmpty()) {
		bHasBillDate = TRUE;

		CString strTemp2;
		strTemp = strBillDateFilter;
		fragmentFilter.m_strBillDateFilter = strTemp; // (c.haag 2016-03-14) - PLID 68592 - We also want to filter at the fragment level
		strTemp.Replace("BDate", "BillsT.Date");
		strTemp2.Format("%s AND %s ", strBillFilter, strTemp);
		strBillFilter = strTemp2;

		strTemp = strBillDateFilter;
		strTemp.Replace("BDate", "LineItemT.Date");
		strTemp2.Format("%s AND %s ", strLineItemDateFilter, strTemp);
		strLineItemDateFilter = strTemp2;
	}

	if (!strServiceDateFilter.IsEmpty()) {
		bHasServiceDate = TRUE;

		CString strTemp2;
		strTemp = strServiceDateFilter;
		fragmentFilter.m_strServiceDateFilter = strTemp; // (c.haag 2016-03-14) - PLID 68592 - We also want to filter at the fragment level
		strTemp.Replace("TDate", "BillsT.Date");
		strTemp2.Format("%s AND %s ", strGCBillFilter, strTemp);
		strGCBillFilter = strTemp2;

		strTemp = strServiceDateFilter;
		strTemp.Replace("TDate", "LineItemT.Date");
		strTemp2.Format("%s AND %s ", strLineItemDateFilter, strTemp);
		strLineItemDateFilter = strTemp2;
	}


	if (!strInputDateFilter.IsEmpty()) {
		bHasInputDate = TRUE;

		CString strTemp2;
		strTemp = strInputDateFilter;
		// (c.haag 2016-03-14) - PLID 68592 - We also want to filter at the fragment level
		fragmentFilter.m_strInputDateFilter = strTemp;
		fragmentFilter.m_strInputDateFilter.Replace("IDate", "InputDate");
		strTemp.Replace("IDate", "LineItemT.InputDate");
		strTemp2.Format("%s AND %s ", strLineItemDateFilter, strTemp);
		strLineItemDateFilter = strTemp2;
	}

	if (!strExternalFilter.IsEmpty())
	{
		// (c.haag 2016-03-14) - PLID 68592 - See if we support this external filter, and if we do then apply it to
		// the line item filter. In the future we may support more kinds of external filters and alter filters other than
		// strLineItemFilter. As far as I can tell, an extended filter will only filter one field at a time.
		const CString strExtendedInputNameFilter = "{DailyFinancialQ.InputName} IN";

		if (-1 != strExternalFilter.Find(strExtendedInputNameFilter))
		{
			// The external filter is an InputName filter
			bHasSupportedExtendedFilter = TRUE;
			strExternalFilter.Replace(strExtendedInputNameFilter, "");
			fragmentFilter.m_strInputUserFilter = FormatString("InputName IN (%s) ", strExternalFilter);
			strLineItemFilter += FormatString(" AND LineItemT.InputName IN (%s) ", strExternalFilter);
		}
		else
		{
			// We don't know how to apply this external filter to optimize the temp table generation
		}
	}



	//Billed information.
	/*	Version History
	DRT 3/28/2007 - PLID 25414 - Cleaned up the formatting just to make it all easier to read.  I tried a few optimizations, but
	I think this query is actually as fast as possible.  I removed 2 references in the FROM clause to CPTModifierT because
	(a) They were unnecessary joins, and (b) we have 4 now and it was confusing.
	DRT 6/13/2008 - PLID 30393 - Fixed subqueries in join for DiscountCategory and Coupons.  Had to clarify some ambiguous PercentOff values with it.
	// (j.gruber 2009-03-25 15:48) - PLID 33358 - updated discount structure
	// (j.gruber 2010-01-07 11:30) - PLID 36747
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-01-12 16:48) - PLID 47507 - for a non-CPT code, use the serviceT.ID instead of ProductT.ID in case of finance charges
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	*/
	CString strBill;
	strBill.Format("SELECT 'BILL' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type,  \r\n"
		" [LineItemT].[Amount], LineItemT.Description, LineItemT.Date, 	LineItemT.InputDate, LineItemT.InputName, LineItemT.Deleted,  \r\n"
		" LineItemT.DeleteDate, LineItemT.DeletedBy, BillsT.Date AS BDate, 	LineItemT.Date AS TDate, 0 AS PrePayment, Amt = ChargeRespT.Amount,  \r\n"
		" ChargesT.DoctorsProviders AS ProvID, 'Bill' AS RText, 	0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, LocationsT.Name AS  \r\n"
		" Location, 	CASE WHEN CPTCodeT.ID IS NULL THEN convert(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS Code, CPTCodeT.RVU AS RVU, ServiceT.Name AS CodeName, 	 \r\n"
		" ChargesT.Quantity, ServiceT.Category, CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName, ChargesT.ServiceID, 	 \r\n"
		" CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN 0 ELSE 1 END AS InsResp, 	'' AS PayCategory, NULL AS TransferredFromTDate, NULL AS  \r\n"
		" TransferredFromIDate, 	Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE  \r\n"
		" CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE  \r\n"
		" CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN \r\n"
		" ((TotalDiscountsQ.TotalPercentOff) Is Null) THEN 1 ELSE ((100-Convert(float, \r\n"
		" (TotalDiscountsQ.TotalPercentOff)))/100) END)-(CASE WHEN  \r\n"
		" (TotalDiscountsQ.TotalDiscount) Is Null THEN 0 ELSE  \r\n"
		" (TotalDiscountsQ.TotalDiscount)  \r\n"
		" END))*(ChargesT.TaxRate-1)))),2) AS TotalTax1,  	 \r\n"
		" Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE  \r\n"
		" WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN  \r\n"
		" CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN(( \r\n"
		" TotalDiscountsQ.TotalPercentOff) Is Null) THEN 1 ELSE ((100-Convert(float,( \r\n"
		" TotalDiscountsQ.TotalPercentOff)))/100) END)-(CASE WHEN ( \r\n"
		" TotalDiscountsQ.TotalDiscount) Is Null THEN 0 ELSE ( \r\n"
		" TotalDiscountsQ.TotalDiscount) END))*(ChargesT.TaxRate2-1)))),2) AS TotalTax2,  	Convert(money, Round(convert(money,  \r\n"
		" Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is  \r\n"
		" Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1  \r\n"
		" ELSE CPTMultiplier4 END)*(CASE WHEN ( \r\n"
		" TotalDiscountsQ.TotalPercentOff) Is Null THEN 1  \r\n"
		" ELSE ((100-Convert(float,( \r\n"
		" TotalDiscountsQ.TotalPercentOff)))/100) END)-(CASE WHEN  \r\n"
		" (TotalDiscountsQ.TotalDiscount) Is Null THEN 0 ELSE ( \r\n"
		" TotalDiscountsQ.TotalDiscount) END))))),2)) AS PreTaxChargeAmt,      \r\n"
		" (Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2  \r\n"
		" Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN  \r\n"
		" 1 ELSE CPTMultiplier4 END))), 2)-Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1  \r\n"
		" END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE  \r\n"
		" WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN ( \r\n"
		" TotalDiscountsQ.TotalPercentOff) Is Null THEN 1 ELSE ((100-Convert(float,( \r\n"
		" TotalDiscountsQ.TotalPercentOff)))/100) END)-(CASE WHEN ( \r\n"
		" TotalDiscountsQ.TotalDiscount) Is Null THEN 0  \r\n"
		" ELSE ( \r\n"
		" TotalDiscountsQ.TotalDiscount) END))), 2)) AS TotalDiscount, 	CASE WHEN  \r\n"
		" (TotalDiscountsQ.TotalPercentOff) IS NULL THEN 0 ELSE ( \r\n"
		" TotalDiscountsQ.TotalPercentOff) END AS PercentDiscount,    CASE WHEN ( \r\n"
		" TotalDiscountsQ.TotalDiscount) IS NULL THEN CONVERT(money, 0) ELSE ( \r\n"
		" TotalDiscountsQ.TotalDiscount) END AS DollarDiscount, 	 \r\n"
		" coalesce(TotalDiscountsQ.DiscountCategory, '') as DiscountCategoryDescription, \r\n"
		" ChargesT.ClaimProviderID, \r\n"
		" Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2) as ChargeAmtBeforeDiscount \r\n"
		"\r\n\r\n"
		" FROM (%s %s %s) LineItemT 	LEFT JOIN ( %s ) ChargesT ON LineItemT.ID = ChargesT.ID 	 \r\n"
		" LEFT JOIN ( %s) BillsT ON ChargesT.BillID = BillsT.ID 	LEFT JOIN  \r\n"
		" (SELECT ChargeID, TotalPercentOff, TotalDiscount,  \r\n"
		" (CASE WHEN CountID = 1 THEN  \r\n"
		" 	Coalesce(CASE WHEN TotalDiscountsSubQ.DiscountCategoryID IS NULL OR TotalDiscountsSubQ.DiscountCategoryID = -3 THEN 'No Category'         \r\n"
		" 		  ELSE CASE WHEN TotalDiscountsSubQ.DiscountCategoryID = -1 THEN TotalDiscountsSubQ.CustomDiscountDesc  \r\n"
		" 					ELSE CASE WHEN TotalDiscountsSubQ.DiscountCategoryID = -2 THEN COALESCE((SELECT Description FROM CouponsT WHERE ID = TotalDiscountsSubQ.CouponID), 'Coupon - Unspecified')  \r\n"
		" 							  ELSE COALESCE((SELECT Description FROM DiscountCategoriesT WHERE ID = TotalDiscountsSubQ.DiscountCategoryID), 'No Category') END  \r\n"
		" 					END  \r\n"
		" 		  END, '') \r\n"
		" 	ELSE dbo.GetChargeDiscountList(ChargeID) END) as DiscountCategory  \r\n"
		"    FROM  \r\n"
		" (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount, Count(ID) as CountID, Min(DiscountCategoryID) as DiscountCategoryID, Min(CouponID) as CouponID, Min(CustomDiscountDesc) as CustomDiscountDesc FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsSubQ) TotalDiscountsQ ON ChargesT.ID =  \r\n"
		" TotalDiscountsQ.ChargeID 	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID 	LEFT JOIN ServiceT ON ChargesT.ServiceID =  \r\n"
		" ServiceT.ID 	LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID 	LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID 	LEFT JOIN  \r\n"
		" LocationsT ON LineItemT.LocationID = LocationsT.ID 	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID  WHERE LineItemT.Type = 10  \r\n"
		" AND LineItemT.Deleted = 0 AND ChargesT.ServiceID NOT IN (SELECT ServiceID FROM GCTypesT)  \r\n", strLineItemFilter,
		bHasBillDate ? "" : strLineItemDateFilter,
		strLineItemLocationFilter,
		strChargeFilter, strBillFilter);



	//Gift Certificates Sold
	/*	Version History
	DRT 3/28/2007 - PLID 25414 - Cleaned up formatting to make it easier to read.  No optimizations can be made.  Removed 2 references
	to CPTModifierT because they are (a) unnecessary and (b) we have 4 now, and 2 here was confusing.
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	*/
	//this uses BillsT.Date as the serviceDate, so we need to account for that.
	CString strGCSold;
	strGCSold.Format("SELECT 'GCSOLD' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, [LineItemT].[Amount], LineItemT.Description, LineItemT.Date,  \r\n"
		"	LineItemT.InputDate, LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, BillsT.Date AS BDate, BillsT.Date AS TDate, \r\n"
		"	0 AS PrePayment, Amt = ChargeRespT.Amount, ChargesT.DoctorsProviders AS ProvID, 'Bill' AS RText, 0 AS ApplyID, LineItemT.ID AS LineID,  \r\n"
		"	LineItemT.LocationID AS LocID, LocationsT.Name AS Location, NULL AS Code, NULL AS RVU, '' AS CodeName, ChargesT.Quantity, ServiceT.Category,  \r\n"
		"	CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName, ChargesT.ServiceID,  \r\n"
		"	CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN 0 ELSE 1 END AS InsResp, '' AS PayCategory, NULL AS TransferredFromTDate,  \r\n"
		"	NULL AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, \r\n"
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, \r\n"
		"	ChargesT.ClaimProviderID, \r\n"
		"   0 as ChargeAmtBeforeDiscount \r\n"
		"\r\n\r\n\r\n"
		"FROM (%s %s %s) LineItemT \r\n"
		"	LEFT JOIN (%s) ChargesT  ON LineItemT.ID = ChargesT.ID \r\n"
		"	LEFT JOIN (%s) BillsT  ON ChargesT.BillID = BillsT.ID \r\n"
		"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID \r\n"
		"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID \r\n"
		"	INNER JOIN GCTypesT ON ServiceT.ID = GCTypesT.ServiceID \r\n"
		"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n"
		"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID \r\n"
		" \r\n"
		"WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 \r\n",
		strLineItemFilter,
		bHasInputDate ? strLineItemDateFilter : "",
		strLineItemLocationFilter,
		strChargeFilter,
		bHasServiceDate ? strGCBillFilter : strBillFilter);



	//Charges
	/*	Version History
	DRT 3/28/2007 - PLID 25414 - Cleaned up formatting to make it easier to read.  No optimizations can be made to this query.  Removed 2
	references to CPTModifierT because they are (a) unnecessary and (b) we have 4 total now, and 2 here is confusing.
	DRT 6/13/2008 - PLID 30393 - Fixed subqueries in join for DiscountCategory and Coupons.  Had to clarify some ambiguous PercentOff values with it.
	// (j.gruber 2009-04-01 12:25) - PLID 33358 - updated discount structure
	// (j.gruber 2010-01-07 11:32) - PLID 36747 - optimized the discount structure
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-01-12 16:49) - PLID 47507 - for non-CPT Codes, use serviceT.ID instead of ProductT.ID in case of other things like finance charges
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	*/
	CString strCharges;
	strCharges.Format("SELECT 'CHARGE' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, \r\n"
		" LineItemT.Amount, LineItemT.Description, LineItemT.Date,  	LineItemT.InputDate, LineItemT.InputName, LineItemT.Deleted,  \r\n"
		" LineItemT.DeleteDate, LineItemT.DeletedBy, LineItemT.Date AS BDate,  	LineItemT.Date AS TDate, 0 AS PrePayment, ChargeRespT.Amount AS Amt,  \r\n"
		" ChargesT.DoctorsProviders AS ProvID, 'Charge' AS RText,  	0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID,  \r\n"
		" LocationsT.Name AS Location,  	CASE WHEN CPTCodeT.ID IS NULL THEN convert(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS Code, CPTCodeT.RVU AS RVU, ServiceT.Name  \r\n"
		" AS CodeName,  	ChargesT.Quantity, ServiceT.Category, CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName,  \r\n"
		" ChargesT.ServiceID,  	CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN 0 ELSE 1 END AS InsResp, '' AS PayCategory, NULL AS  \r\n"
		" TransferredFromTDate, NULL AS TransferredFromIDate,  	Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE  \r\n"
		" WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN  \r\n"
		" CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE  \r\n"
		" WHEN(TotalPercentOff Is Null) THEN 1 ELSE ((100-Convert(float,TotalPercentOff))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE  \r\n"
		" [TotalDiscount] END))*(ChargesT.TaxRate-1)))),2) AS TotalTax1,  	Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE  \r\n"
		" WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN  \r\n"
		" CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE  \r\n"
		" WHEN(TotalPercentOff Is Null) THEN 1 ELSE ((100-Convert(float,TotalPercentOff))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE  \r\n"
		" [TotalDiscount] END))*(ChargesT.TaxRate2-1)))),2) AS TotalTax2,  	Convert(money, Round(convert(money,  \r\n"
		" Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is  \r\n"
		" Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1  \r\n"
		" ELSE CPTMultiplier4 END)*(CASE WHEN TotalPercentOff Is Null THEN 1 ELSE ((100-Convert(float,TotalPercentOff))/100) END)-(CASE WHEN  \r\n"
		" [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))),2)) AS PreTaxChargeAmt,      \r\n"
		" (Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2  \r\n"
		" Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN  \r\n"
		" 1 ELSE CPTMultiplier4 END))), 2)-Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1  \r\n"
		" END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE  \r\n"
		" WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN TotalPercentOff Is Null THEN 1 ELSE  \r\n"
		" ((100-Convert(float,TotalPercentOff))/100) END) -(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))), 2)) AS TotalDiscount, 	 \r\n"
		" TotalPercentOff AS PercentDiscount, TotalDiscount AS DollarDiscount, coalesce(TotalDiscountsQ.DiscountCategory, '') as DiscountCategory, \r\n"
		" ChargesT.ClaimProviderID, \r\n"
		" Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2) as ChargeAmtBeforeDiscount \r\n"
		"\r\n"
		"\r\n"
		" FROM ( %s %s %s  ) LineItemT	INNER JOIN ( %s ) ChargesT  ON  \r\n"
		" LineItemT.ID = ChargesT.ID 	 \r\n"
		" LEFT JOIN (SELECT ChargeID, TotalPercentOff, TotalDiscount,  \r\n"
		" (CASE WHEN CountID = 1 THEN  \r\n"
		" 	Coalesce(CASE WHEN TotalDiscountsSubQ.DiscountCategoryID IS NULL OR TotalDiscountsSubQ.DiscountCategoryID = -3 THEN 'No Category'         \r\n"
		" 		  ELSE CASE WHEN TotalDiscountsSubQ.DiscountCategoryID = -1 THEN TotalDiscountsSubQ.CustomDiscountDesc  \r\n"
		" 					ELSE CASE WHEN TotalDiscountsSubQ.DiscountCategoryID = -2 THEN COALESCE((SELECT Description FROM CouponsT WHERE ID = TotalDiscountsSubQ.CouponID), 'Coupon - Unspecified')  \r\n"
		" 							  ELSE COALESCE((SELECT Description FROM DiscountCategoriesT WHERE ID = TotalDiscountsSubQ.DiscountCategoryID), 'No Category') END  \r\n"
		" 					END  \r\n"
		" 		  END, '') \r\n"
		" 	ELSE dbo.GetChargeDiscountList(ChargeID) END) as DiscountCategory  \r\n"
		"    FROM  \r\n"
		" (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount, Count(ID) as CountID, Min(DiscountCategoryID) as DiscountCategoryID, Min(CouponID) as CouponID, Min(CustomDiscountDesc) as CustomDiscountDesc FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsSubQ) TotalDiscountsQ ON ChargesT.ID =  \r\n"
		" TotalDiscountsQ.ChargeID  \r\n"
		" LEFT JOIN ChargeRespT ON  \r\n"
		" ChargesT.ID = ChargeRespT.ChargeID 	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID 	LEFT JOIN CPTCodeT ON ServiceT.ID =  \r\n"
		" CPTCodeT.ID 	LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID 	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID 	LEFT  \r\n"
		" JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID  WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND ChargesT.ServiceID NOT IN  \r\n"
		" (SELECT ServiceID FROM GCTypesT)  \r\n",
		strLineItemFilter,
		bHasBillDate ? "" : strLineItemDateFilter,
		strLineItemLocationFilter,
		strChargeFilter);



	//Payments
	/*	Version History
	DRT 3/28/2007 - PLID 25414 - Cleaned up formatting significantly.  Applied a few minor changes to be more consistent, including
	changing all <alias> = <case> to <case> AS <alias> format, changing a LEFT JOIN + WHERE clause filter into an INNER JOIN.  I
	cannot find any optimizations to be made.  Removed the MIN(LineItemT_1.Amount) from the _PartiallyAppliedPaysQ subquery.  We're
	already GROUP BY LineItemT_1.Amount, so doing MIN is useless, there will never be more than 1 record.  This resulted in a 10%
	performance boost on that subquery, as reported by the execution plan.  I also removed the HAVING LineItemT_1.Amount IS NOT NULL,
	as this is the base of the query, and cannot possibly be NULL.
	Failed attempt optimization:  I attempted to change the _PartiallyAppliedPaysQ so that the applies were grouped in to a subquery, and
	the entire thing did not need a GROUP BY.  This however turned out to be less efficient.
	(e.lally 2007-08-03) PLID 25116 - Replaced refunds with a breakdown of refunds as cash, check, or credit
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	// (r.gonet 2015-05-05 14:38) - PLID 65903 - Added PayMethod 10 - Gift Certificate Refunds
	*/
	CString strPays;
	strPays.Format("SELECT  convert(nvarchar(40), \r\n"
		"		CASE WHEN LineItemT.Type = 1 THEN \r\n"
		"		CASE WHEN PaymentsT.PayMethod = 1 THEN 'CASH' \r\n"
		"		     WHEN PaymentsT.PayMethod = 2 THEN 'CHECK' \r\n"
		"		     WHEN PaymentsT.PayMethod = 3 THEN 'CREDIT' \r\n"
		"		     WHEN PaymentsT.PayMethod = 4 THEN 'GCREDEEMED' END \r\n"
		"	     WHEN LineItemT.Type = 2 THEN 'ADJUSTMENT' \r\n"
		"	     WHEN LineItemT.Type = 3 THEN \r\n"
		"			CASE WHEN PaymentsT.PayMethod = 7 THEN 'RefundCash' \r\n"
		"				 WHEN PaymentsT.PayMethod = 8 THEN 'RefundCheck' \r\n"
		"				 WHEN PaymentsT.PayMethod = 9 THEN 'RefundCredit' \r\n"
		"				 WHEN PaymentsT.PayMethod = 10 THEN 'RefundGC' END \r\n"
		"	END) AS FinType, \r\n"
		"	LineItemT.ID, LineItemT.PatientID, LineItemT.Type, \r\n"
		"	CASE WHEN _PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount END ELSE AppliesT.Amount END AS Amount, \r\n"
		"	LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, \r\n"
		"	LineItemT.Date AS BDate, LineItemT.Date AS TDate, PaymentsT.PrePayment, LineItemT.Amount AS Amt, \r\n"
		"	CASE WHEN ChargesT.ID Is Null THEN CASE WHEN PayDestT.ID Is Null THEN PaymentsT.ProviderID ELSE PayDestT.ProviderID END ELSE [DoctorsProviders] END AS ProvID, \r\n"
		"	'Full' AS RText, AppliesT.ID AS ApplyID, LineItemT.ID AS LineID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, \r\n"
		"	CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, \r\n"
		"	CASE WHEN CPTCodeT.Code IS NULL THEN CONVERT(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS Code, CPTCodeT.RVU AS RVU, ServiceT.Name AS CodeName, 0 AS Quantity, ServiceT.Category, \r\n"
		"	CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName, ServiceT.ID AS ServiceID, CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, \r\n"
		"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, NULL AS TransferredFromTDate, NULL AS TransferredFromIDate, \r\n"
		"	0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, \r\n"
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, \r\n"
		"	ChargesT.ClaimProviderID AS ClaimProviderID, 0 as chargeAmtBeforeDiscount \r\n"
		"\r\n\r\n\r\n"
		"FROM (%s %s) LineItemT \r\n"
		"	INNER JOIN PaymentsT  ON LineItemT.ID = PaymentsT.ID \r\n"
		"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID \r\n"
		"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n"
		"	LEFT JOIN \r\n"
		"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, SUM(AppliesT.Amount) AS ApplyAmt, \r\n"
		"			LineItemT_1.Amount - SUM(AppliesT.Amount) AS Total, LineItemT_1.PatientID \r\n"
		"	 \r\n"
		"		FROM LineItemT AS LineItemT_1 \r\n"
		"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID \r\n"
		"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID \r\n"
		"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID \r\n"
		"	 \r\n"
		"		WHERE LineItemT_1.Deleted = 0 AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT_1.GiftID, -1)) \r\n"
		"	 \r\n"
		"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID \r\n"
		"	 \r\n"
		"		HAVING (LineItemT_1.Amount - SUM(AppliesT.Amount)) <> 0 \r\n"
		"		) _PartiallyAppliedPaysQ \r\n"
		"	ON LineItemT.ID = _PartiallyAppliedPaysQ.ID \r\n"
		"	LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID \r\n"
		"	LEFT JOIN LineItemT LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID \r\n"
		"	LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID \r\n"
		"	LEFT JOIN PaymentsT AS PayDestT ON LineItemT_1.ID = PayDestT.ID \r\n"
		"	LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID \r\n"
		"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID \r\n"
		"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID \r\n"
		"	LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID \r\n"
		" \r\n"
		"WHERE LineItemT.Deleted = 0 AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT.GiftID, -1)) \r\n",
		strLineItemFilter, strLineItemDateFilter);



	//Partial Payments
	/*	Version History
	DRT 3/29/2007 - 25414 - Cleaned up formatting.  Removed MIN aggregate from the _PartiallyAppliedPaysQ subquery, as we already
	group on LineItemT_1.Amount, thus MIN can have no effect.  Removed HAVING LineItemT_1.ID IS NOT NULL, as it is the base
	of the whole query.
	(e.lally 2007-08-03) PLID 25116 - Replaced refunds with a breakdown of refunds as cash, check, or credit
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	// (r.gonet 2015-05-05 14:38) - PLID 65903 - Added PayMethod 10 - Gift Certificate Refunds
	*/
	CString strPartialPays;
	strPartialPays.Format("SELECT  CASE WHEN LineItemT.Type = 1 THEN \r\n"
		"		CASE WHEN PaymentsT.PayMethod = 1 THEN 'CASH' \r\n"
		"		     WHEN PaymentsT.PayMethod = 2 THEN 'CHECK' \r\n"
		"		     WHEN PaymentsT.PayMethod = 3 THEN 'CREDIT' \r\n"
		"		     WHEN PaymentsT.PayMethod = 4 THEN 'GCREDEEMED' END \r\n"
		"	     WHEN LineItemT.Type = 2 THEN 'ADJUSTMENT' \r\n"
		"	     WHEN LineItemT.Type = 3 THEN \r\n"
		"			CASE WHEN PaymentsT.PayMethod = 7 THEN 'RefundCash' \r\n"
		"				 WHEN PaymentsT.PayMethod = 8 THEN 'RefundCheck' \r\n"
		"				 WHEN PaymentsT.PayMethod = 9 THEN 'RefundCredit' \r\n"
		"				 WHEN PaymentsT.PayMethod = 10 THEN 'RefundGift' END \r\n"
		"	END AS FinType, \r\n"
		"	_PartiallyAppliedPaysQ.ID, LineItemT.PatientID, LineItemT.Type, _PartiallyAppliedPaysQ.Total AS Amount, LineItemT.Description, \r\n"
		"	LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, \r\n"
		"	LineItemT.Date AS BDate, LineItemT.Date AS TDate, PaymentsT.PrePayment, LineItemT.Amount AS Amt, PaymentsT.ProviderID AS ProvID, \r\n"
		"	'Part' AS RText, 0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, NULL AS Code, NULL AS RVU, \r\n"
		"	NULL AS CodeName, 0 AS Quantity, NULL AS Category, '' AS CatName, NULL AS ServiceID, \r\n"
		"	CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, \r\n"
		"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, \r\n"
		"	NULL AS TransferredFromTDate, NULL AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, \r\n"
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, NULL AS ClaimProviderID, 0 as chargeAmtBeforeDiscount \r\n"
		"\r\n\r\n "
		"FROM (%s %s %s) LineItemT \r\n"
		"	INNER JOIN (%s) PaymentsT  ON LineItemT.ID = PaymentsT.ID \r\n"
		"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID \r\n"
		"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n"
		"	LEFT JOIN \r\n"
		"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt, \r\n"
		"			LineItemT_1.Amount - Sum(AppliesT.Amount) AS Total, LineItemT_1.PatientID \r\n"
		"	 \r\n"
		"		FROM LineItemT AS LineItemT_1 \r\n"
		"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID \r\n"
		"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID \r\n"
		"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID \r\n"
		"	 \r\n"
		"		WHERE LineItemT_1.Deleted = 0 \r\n"
		"	 \r\n"
		"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID \r\n"
		"	 \r\n"
		"		HAVING (LineItemT_1.Amount - Sum(AppliesT.Amount)) <> 0 \r\n"
		"		) AS _PartiallyAppliedPaysQ \r\n"
		"	ON LineItemT.ID = _PartiallyAppliedPaysQ.ID \r\n"
		" \r\n"
		"WHERE _PartiallyAppliedPaysQ.ID Is Not Null AND LineItemT.Deleted = 0 \r\n",
		strLineItemFilter, strLineItemDateFilter, strLineItemLocationFilter, strPayFilter);


	//Prepayments
	/*	Version History
	DRT 3/29/2007 - PLID 25414 - Cleaned up formatting.  Removed MIN from subquery, it was unnecessary since we already group on Amount.  Removed
	subquery reference to HAVING LineItemT_1.ID IS NOT NULL and main query LineItemT.ID IS NOT NULL, since both of those are the base
	tables of their queries, they cannot possibly be NULL.
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	*/
	CString strPrepays;
	strPrepays.Format("SELECT 'PREPAYMENT' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, \r\n"
		"	CASE WHEN _PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount END ELSE AppliesT.Amount END AS Amount, \r\n"
		"	LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, \r\n"
		"	LineItemT.DeletedBy, LineItemT.Date AS BDate, LineItemT.Date AS TDate, PaymentsT.PrePayment, LineItemT.Amount AS Amt, \r\n"
		"	CASE WHEN ChargesT.ID Is Null THEN CASE WHEN PayDestT.ID Is Null THEN PaymentsT.ProviderID ELSE PayDestT.ProviderID END ELSE DoctorsProviders END AS ProvID, \r\n"
		"	'Full' AS RText, AppliesT.ID AS ApplyID, LineItemT.ID AS LineID, \r\n"
		"	CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, \r\n"
		"	CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, \r\n"
		"	CASE WHEN CPTCodeT.Code IS NULL THEN CONVERT(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS Code, CPTCodeT.RVU AS RVU, ServiceT.Name AS CodeName, \r\n"
		"	0 AS Quantity, ServiceT.Category, CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName, ServiceT.ID AS ServiceID, \r\n"
		"	CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, \r\n"
		"	NULL AS TransferredFromTDate, NULL AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, \r\n"
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, \r\n"
		"	ChargesT.ClaimProviderID AS ClaimProviderID, 0 as chargeAmtBeforeDiscount \r\n"
		"\r\n\r\n\r\n"
		"FROM (%s %s) LineItemT  \r\n"
		"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n"
		"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID \r\n"
		"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n"
		"	LEFT JOIN \r\n"
		"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, SUM(AppliesT.Amount) AS ApplyAmt, \r\n"
		"			LineItemT_1.Amount - SUM(AppliesT.Amount) AS Total, LineItemT_1.PatientID \r\n"
		" \r\n"
		"		FROM LineItemT AS LineItemT_1 \r\n"
		"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID \r\n"
		"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID \r\n"
		"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID \r\n"
		" \r\n"
		"		WHERE LineItemT_1.Deleted = 0 \r\n"
		" \r\n"
		"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID \r\n"
		" \r\n"
		"		HAVING (LineItemT_1.Amount - SUM(AppliesT.Amount)) <> 0 \r\n"
		"		) _PartiallyAppliedPaysQ \r\n"
		" \r\n"
		"	ON LineItemT.ID = _PartiallyAppliedPaysQ.ID \r\n"
		"	LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID \r\n"
		"	LEFT JOIN LineItemT LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID \r\n"
		"	LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID \r\n"
		"	LEFT JOIN PaymentsT AS PayDestT ON LineItemT_1.ID = PayDestT.ID \r\n"
		"	LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID \r\n"
		"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID \r\n"
		"	LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID \r\n"
		"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID \r\n"
		" \r\n"
		"WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 1 \r\n",
		strLineItemFilter, strLineItemDateFilter);


	//Partially applied prepayments
	/*	Version History
	DRT 3/29/2007 - PLID 25414 - Cleaned up formatting.  Had to change the FROM to get rid of a RIGHT JOIN.  Removed MIN aggregate from
	_PartiallyAppliedPaysQ, it is not needed since we group by the amount.  Removed HAVING LineItemT_1.ID IS NOT NULL, as it is
	the base of the subquery.
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	*/
	CString strPartialPrepays;
	strPartialPrepays.Format("SELECT 'PREPAYMENT' AS FinType, _PartiallyAppliedPaysQ.ID, LineItemT.PatientID, LineItemT.Type, _PartiallyAppliedPaysQ.Total AS Amount, \r\n"
		"	LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, \r\n"
		"	LineItemT.DeletedBy, LineItemT.Date AS BDate, LineItemT.Date AS TDate, PaymentsT.PrePayment, LineItemT.Amount AS Amt, \r\n"
		"	PaymentsT.ProviderID AS ProvID, 'Part' AS RText, 0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, \r\n"
		"	LocationsT.Name AS Location, NULL AS Code, NULL AS RVU, NULL AS CodeName, 0 AS Quantity, NULL AS Category, '' AS CatName, NULL AS ServiceID, \r\n"
		"	CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, \r\n"
		"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, \r\n"
		"	NULL AS TransferredFromTDate, NULL AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, \r\n"
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription,  NULL AS ClaimProviderID, 0 as chargeAmtBeforeDiscount \r\n"
		"\r\n\r\n\r\n"
		"FROM (%s %s %s) LineItemT \r\n"
		"	INNER JOIN (%s) PaymentsT  ON LineItemT.ID = PaymentsT.ID \r\n"
		"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID \r\n"
		"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n"
		"	LEFT JOIN \r\n"
		"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt, \r\n"
		"			LineItemT_1.Amount - Sum(AppliesT.Amount) AS Total, LineItemT_1.PatientID \r\n"
		"	 \r\n"
		"		FROM LineItemT AS LineItemT_1  \r\n"
		"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID \r\n"
		"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID \r\n"
		"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID \r\n"
		"	 \r\n"
		"		WHERE LineItemT_1.Deleted = 0 \r\n"
		"	 \r\n"
		"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID \r\n"
		"	 \r\n"
		"		HAVING (LineItemT_1.Amount - Sum(AppliesT.Amount)) <> 0 \r\n"
		"		) AS _PartiallyAppliedPaysQ  \r\n"
		"	ON LineItemT.ID = _PartiallyAppliedPaysQ.ID \r\n"
		" \r\n"
		"WHERE _PartiallyAppliedPaysQ.ID Is Not Null AND PaymentsT.Prepayment = 1 AND LineItemT.Deleted = 0 \r\n",
		strLineItemFilter, strLineItemDateFilter, strLineItemLocationFilter, strPayFilter);


	//Applied Prepayments
	/*	Version History
	DRT 3/29/2007 - PLID 25414 - Cleaned up formatting.  Fixed a bug that said "LEFT JOIN ServiceT ON ServiceT.ID = ChargesT.ServiceID".  I
	was unable to find any optimizations to be made.
	TES 7/9/2008 - PLID 29580 - Made it so that applies are treated as having happened on AppliesT.InputDate, not on the later
	of the two dates involved in the apply.
	TES 10/13/2009 - PLID 35662 - Restored the old apply date calculation for everything but the input date
	(c.haag 2010-01-19 17:03) - PLID 36643 - I did not change this query; but if you change it, you may also need to change strAppliedToPrepays
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	// (z.manning 2013-06-03 15:44) - PLID 56977 - Fixed this to not include voided prepayments that are not involved in
	// any other applies.
	// (b.eyers 2016-03-10) - PLID 68542 - Fixed so refund's voided applies aren't hidden
	*/
	CString strPrepaysApplied;
	strPrepaysApplied.Format("SELECT 'APPPRE' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Amount, LineItemT.Description, LineItemT.Date, \r\n"
		"	AppliesSubQ.InputDate, \r\n"
		"	LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, \r\n"
		"	CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS BDate, \r\n"
		"	CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS TDate, \r\n"
		"	0 AS Prepayment, Round(Convert(money,AppliesSubQ.Amount),2) AS Amt, AppliesSubQ.ProvID AS ProvID, 'APPPRE' AS RText, \r\n"
		"	AppliesSubQ.ApplyID AS ApplyID, LineItemT.ID AS LineID, AppliesSubQ.LocID AS LocID, LocationsT.Name AS Location, AppliesSubQ.Code AS Code,  \r\n"
		"	AppliesSubQ.RVU AS RVU, AppliesSubQ.CodeName AS CodeName, 0 AS Quantity, AppliesSubQ.Category AS Category, AppliesSubQ.CatName AS CatName, \r\n"
		"	AppliesSubQ.ServiceID AS ServiceID, CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, \r\n"
		"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, \r\n"
		"	NULL AS TransferredFromTDate, NULL AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, \r\n"
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, AppliesSubQ.ClaimProviderID, 0 as chargeAmtBeforeDiscount \r\n"
		"\r\n\r\n\r\n"
		"FROM (%s) LineItemT  \r\n"
		"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n"
		"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID \r\n"
		"	LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID \r\n"
		"	INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"
		"	LEFT JOIN PersonT PersonT1 ON PaymentsT.ProviderID = PersonT1.ID \r\n"
		"	LEFT JOIN \r\n"
		"		/*DRT - select all charges which have a prepayment applied to them (prepay is source)*/ \r\n"
		"		(SELECT AppliesT.ID AS ApplyID, SourceID, AppliesT.Amount AS Amount, BillsT.Description, BillsT.Date, \r\n"
		"			AppliesT.InputDate, ChargesT.DoctorsProviders AS ProvID, LineItemT.LocationID AS LocID, \r\n"
		"			CASE WHEN CPTCodeT.Code IS NULL THEN CONVERT(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS Code, CPTCodeT.RVU AS RVU, \r\n"
		"			ServiceT.Name AS CodeName, ServiceT.Category, ServiceT.ID AS ServiceID, \r\n"
		"			CASE WHEN CategoriesT.ID IS NULL THEN '' ELSE CategoriesT.Name END AS CatName, ChargesT.ClaimProviderID \r\n"
		"	 \r\n"
		"		FROM AppliesT  \r\n"
		"			INNER JOIN  ChargesT ON AppliesT.DestID = ChargesT.ID \r\n"
		"			INNER JOIN  LineItemT ON ChargesT.ID = LineItemT.ID\r\n"
		"			INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID \r\n"
		"			INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID \r\n"
		"			LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID \r\n"
		"			LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID \r\n"
		"			LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID \r\n"
		"		 \r\n"
		"		WHERE PaymentsT.Prepayment = 1 \r\n"
		"	 \r\n"
		"		UNION ALL \r\n"
		"		/*DRT - select all items applied to a prepayment */ \r\n"
		"		SELECT AppliesT.ID AS ApplyID, SourceID, AppliesT.Amount * -1 AS Amount, LineItemT2.Description, LineItemT2.Date, AppliesT.InputDate, \r\n"
		"			PaymentsT.ProviderID AS ProvID, LineItemT2.LocationID AS LocID, NULL AS Code, NULL AS RVU, NULL AS CodeName, NULL AS Category, '' AS CatName, NULL AS ServiceID, \r\n"
		"			NULL AS ClaimProviderID \r\n"
		"	 \r\n"
		"		FROM AppliesT \r\n"
		"			INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID \r\n"
		"			INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID \r\n"
		"			INNER JOIN LineItemT LineItemT2 ON PaymentsT.ID = LineItemT2.ID \r\n"
		"			LEFT JOIN LineItemCorrectionsT ON AppliesT.SourceID = LineItemCorrectionsT.VoidingLineItemID AND AppliesT.DestID = LineItemCorrectionsT.OriginalLineItemID \r\n"
		"	 \r\n"
		"		WHERE PaymentsT.Prepayment = 1 AND LineItemCorrectionsT.ID IS NULL \r\n"
		"		) AppliesSubQ \r\n"
		"	ON LineItemT.ID = AppliesSubQ.SourceID \r\n"
		"	LEFT JOIN LocationsT ON AppliesSubQ.LocID = LocationsT.ID \r\n"
		" \r\n"
		"WHERE AppliesSubQ.Amount <> 0 \r\n",
		strLineItemFilter);

	//Applied to Prepayments
	/*	Version History
	(c.haag 2010-01-19 17:03) - PLID 36643 - Initial implementation. This query is identical to Applied Prepayments
	(a.k.a. strPrepaysApplied), except that it only has line items applied TO prepayments.
	// (j.gruber 2010-04-28 11:07) - PLID 38399 - fixed sql error on Sql 2000
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	// (z.manning 2013-06-03 15:44) - PLID 56977 - Fixed this to not include voided prepayments that are not involved in
	// any other applies.
	// (b.eyers 2016-03-10) - PLID 68542 - Fixed so refund's voided applies aren't hidden
	*/
	CString strAppliedToPrepays;
	strAppliedToPrepays.Format("SELECT 'APPTOPRE' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Amount, LineItemT.Description, LineItemT.Date, \r\n"
		"	AppliesSubQ.InputDate, \r\n"
		"	LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, \r\n"
		"	CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS BDate, \r\n"
		"	CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS TDate, \r\n"
		"	0 AS Prepayment, Round(Convert(money,AppliesSubQ.Amount),2) AS Amt, AppliesSubQ.ProvID AS ProvID, 'APPTOPRE' AS RText, \r\n"
		"	AppliesSubQ.ApplyID AS ApplyID, LineItemT.ID AS LineID, AppliesSubQ.LocID AS LocID, LocationsT.Name AS Location, CONVERT(nVarChar, AppliesSubQ.Code) AS Code,  \r\n"
		"	AppliesSubQ.RVU AS RVU, CONVERT(nVarChar, AppliesSubQ.CodeName) AS CodeName, 0 AS Quantity, AppliesSubQ.Category AS Category, CONVERT(nVarChar, AppliesSubQ.CatName) AS CatName, \r\n"
		"	AppliesSubQ.ServiceID AS ServiceID, CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, \r\n"
		"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, \r\n"
		"	NULL AS TransferredFromTDate, NULL AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, \r\n"
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, \r\n"
		"	AppliesSubQ.ClaimProviderID, 0 as chargeAmtBeforeDiscount \r\n"
		"\r\n\r\n\r\n"
		"FROM (%s) LineItemT  \r\n"
		"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n"
		"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID \r\n"
		"	LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID \r\n"
		"	INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"
		"	LEFT JOIN PersonT PersonT1 ON PaymentsT.ProviderID = PersonT1.ID \r\n"
		"	LEFT JOIN \r\n"
		"		/*CAH - select all items applied to a prepayment */ \r\n"
		"		(SELECT AppliesT.ID AS ApplyID, SourceID, AppliesT.Amount * -1 AS Amount, LineItemT2.Description, LineItemT2.Date, AppliesT.InputDate, \r\n"
		"			PaymentsT.ProviderID AS ProvID, LineItemT2.LocationID AS LocID, NULL AS Code, NULL AS RVU, NULL AS CodeName, NULL AS Category, '' AS CatName, NULL AS ServiceID, \r\n"
		"			NULL AS ClaimProviderID \r\n"
		"	 \r\n"
		"		FROM AppliesT \r\n"
		"			INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID \r\n"
		"			INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID \r\n"
		"			INNER JOIN LineItemT LineItemT2 ON PaymentsT.ID = LineItemT2.ID \r\n"
		"			LEFT JOIN LineItemCorrectionsT ON AppliesT.SourceID = LineItemCorrectionsT.VoidingLineItemID AND AppliesT.DestID = LineItemCorrectionsT.OriginalLineItemID \r\n"
		"	 \r\n"
		"		WHERE PaymentsT.Prepayment = 1 AND LineItemCorrectionsT.ID IS NULL \r\n"
		"		) AppliesSubQ \r\n"
		"	ON LineItemT.ID = AppliesSubQ.SourceID \r\n"
		"	LEFT JOIN LocationsT ON AppliesSubQ.LocID = LocationsT.ID \r\n"
		" \r\n"
		"WHERE AppliesSubQ.Amount <> 0 \r\n",
		strLineItemFilter);

	// (j.gruber 2010-03-11 09:44) - PLID 37713 - move transfer types to their own function


	//Applied Payments
	// (j.gruber 2007-08-28 12:12) - PLID 25190 - added this to be able to calculate payments that were applied in this area, the above wasn't working because it returned when there was no payment
	//basically just copied from applied prepayments and changed it to only use just payments
	//TES 7/9/2008 - PLID 29580 - Made it so that applies are treated as having happened on AppliesT.InputDate, not on the later
	// of the two dates involved in the apply.
	//TES 10/13/2009 - PLID 35662 - Restored the old apply date calculation for everything but the input date
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount

	CString strAppliedPays;
	strAppliedPays.Format("/*Applied Payments */ \r\n"
		" SELECT 'APPLIEDPAYS' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Amount, LineItemT.Description, LineItemT.Date,  \r\n"
		" 				AppliesSubQ.InputDate,  \r\n"
		" 				LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy,  \r\n"
		" 				CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS BDate,  \r\n"
		" 				CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS TDate,  \r\n"
		" 				0 AS Prepayment, Round(Convert(money,AppliesSubQ.Amount),2) AS Amt, AppliesSubQ.ProvID AS ProvID, 'APPPRE' AS RText,  \r\n"
		" 				AppliesSubQ.ApplyID AS ApplyID, LineItemT.ID AS LineID, AppliesSubQ.LocID AS LocID, LocationsT.Name AS Location, AppliesSubQ.Code AS Code,   \r\n"
		" 				AppliesSubQ.RVU AS RVU, AppliesSubQ.CodeName AS CodeName, 0 AS Quantity, AppliesSubQ.Category AS Category, AppliesSubQ.CatName AS CatName,  \r\n"
		" 				AppliesSubQ.ServiceID AS ServiceID, CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp,  \r\n"
		" 				CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory,  \r\n"
		" 				NULL AS TransferredFromTDate, NULL AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt,  \r\n"
		"				0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, AppliesSubQ.ClaimProviderID, 0 as chargeAmtBeforeDiscount \r\n"
		"\r\n\r\n\r\n"
		" FROM (%s) LineItemT \r\n"
		" 				INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  \r\n"
		" 				LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID  \r\n"
		" 				LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID  \r\n"
		" 				INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n"
		" 				LEFT JOIN PersonT PersonT1 ON PaymentsT.ProviderID = PersonT1.ID  \r\n"
		" 				LEFT JOIN  \r\n"
		" 					/*DRT - select all charges which have a prepayment applied to them (prepay is source)*/  \r\n"
		" 					(SELECT AppliesT.ID AS ApplyID, SourceID, AppliesT.Amount AS Amount, BillsT.Description, BillsT.Date,  \r\n"
		"						AppliesT.InputDate, ChargesT.DoctorsProviders AS ProvID, LineItemT.LocationID AS LocID,  \r\n"
		" 						CASE WHEN CPTCodeT.Code IS NULL THEN CONVERT(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS Code,  CPTCodeT.RVU AS RVU,\r\n"
		" 						ServiceT.Name AS CodeName, ServiceT.Category, ServiceT.ID AS ServiceID, \r\n"
		" \r\n"
		" 						CASE WHEN CategoriesT.ID IS NULL THEN '' ELSE CategoriesT.Name END AS CatName,  \r\n"
		"						ChargesT.ClaimProviderID \r\n"
		" 				 \r\n"
		" 					FROM AppliesT   \r\n"
		" 						INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID  \r\n"
		" 						INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  \r\n"
		" 						INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID  \r\n"
		" 						INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  \r\n"
		" 						LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  \r\n"
		" 						LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID  \r\n"
		" 						LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID  \r\n"
		"			 		WHERE PaymentsT.Prepayment = 0  \r\n"
		" 				  \r\n"
		" 					UNION ALL  \r\n"
		" 					/*DRT - select all items applied to a prepayment */  \r\n"
		" 					SELECT AppliesT.ID AS ApplyID, SourceID, AppliesT.Amount * -1 AS Amount, LineItemT2.Description, LineItemT2.Date, AppliesT.InputDate,  \r\n"
		" 						PaymentsT.ProviderID AS ProvID, LineItemT2.LocationID AS LocID, NULL AS Code, NULL AS RVU, NULL AS CodeName, NULL AS Category, '' AS CatName, NULL AS ServiceID, NULL AS ClaimProviderID  \r\n"
		" 				  \r\n"
		"					FROM AppliesT  \r\n"
		" 						INNER JOIN (%s) LineItemT ON AppliesT.SourceID = LineItemT.ID  \r\n"
		" 						INNER JOIN (%s) PaymentsT ON AppliesT.DestID = PaymentsT.ID  \r\n"
		" 						INNER JOIN LineItemT LineItemT2 ON PaymentsT.ID = LineItemT2.ID  \r\n"
		" 				  \r\n"
		" 					WHERE PaymentsT.Prepayment = 0  \r\n"
		" 					) AppliesSubQ  \r\n"
		" 				ON LineItemT.ID = AppliesSubQ.SourceID  \r\n"
		" 				LEFT JOIN LocationsT ON AppliesSubQ.LocID = LocationsT.ID  \r\n"
		" 			 \r\n"
		" 			WHERE AppliesSubQ.Amount <> 0 \r\n",
		strLineItemFilter,
		strLineItemFilter, strPayFilter);


	//Now add up all of our sections into one big happy query.
	// (j.gruber 2010-03-11 09:46) - PLID 37713 - changed the transfer types to be called from their own function
	// (c.haag 2016-02-18) - PLID 68460 - Write all the records to the temp table and return the temp table.
	// If we're not doing any filtering, just defer to the old UNION's since we can potentially flood the temp table
	// with millions and millions of records. This is also a possibility if filtering on large date ranges, in which case
	// the user can opt to disable using a temp table for this report through ConfigRT. At the time of this comment
	// the only reports that could even use a temp table are "Financial Activity - Daily" and 
	// "Financial Activity - Daily For Logged On User." In the future we could look into expanding this optimization
	// into other reports and with more filters that directly affect AllFinancialQ. If this optimization fails for a client,
	// you can disable it through inserting a record in ConfigRT with the name SupportTempAllFinQReport.
	// (c.haag 2016-03-14) - PLID 68592 - Added support for input date and line item filter
	// (j.jones 2016-03-15 15:10) - PLID 68598 - added location filter
	if ((!bHasBillDate && !bHasServiceDate && !bHasInputDate && !bHasSupportedExtendedFilter && !bHasLocationFilter)
		|| (GetRemotePropertyInt("SupportTempAllFinQReport", 1, reportID, "<None>", false) != 1)
		)
	{
		return strBill + "UNION ALL \r\n" + strGCSold + "UNION ALL \r\n" + strCharges +
			"UNION ALL " + strPays + "UNION ALL " + strPartialPays + "UNION ALL \r\n" +
			strPrepays + "UNION ALL \r\n" + strPartialPrepays + "UNION ALL \r\n" +
			strPrepaysApplied + "UNION ALL \r\n" +
			GetTransferFinType(strLineItemFilter) +
			" UNION ALL \r\n" + strAppliedPays +
			" UNION ALL \r\n" + strAppliedToPrepays;
	}
	else
	{
		CString strTempT;
		CString strGUID = NewPlainUUID();
		strTempT.Format("#TempAllFinancialQ%s", strGUID);

		// (c.haag 2016-03-01) - PLID 68460 - This is the SQL statement batch for creating and populating the
		// temp table. Note it is not parameterized because the query fragments are not parameterized. Doing so
		// would require major reporting changes beyond the scope of this optimization that we can maybe do
		// another time.
		// (r.goldschmidt 2016-04-27 14:35) - NX-100264 - fix incorrect nvarchar size limits (dbo.GetChargeDiscountList(ChargesT.ID) returns an nvarchar(1000))
		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, R"(
IF OBJECT_ID('tempdb..%s') IS NOT NULL
	DROP TABLE %s;

						CREATE TABLE %s (
	FinType NVARCHAR(255)
	,ID INT
	,PatientID INT
	,Type INT
	,Amount MONEY
	,Description NVARCHAR(255)
	,Date DATETIME
	,InputDate DATETIME
	,InputName NVARCHAR(50)
	,Deleted BIT
	,DeleteDate DATETIME
	,DeletedBy NVARCHAR(50)
	,BDate DATETIME
	,TDate DATETIME
	,PrePayment BIT
	,Amt MONEY
	,ProvID INT
	,RText NVARCHAR(255)
	,ApplyID INT
	,LineID INT
	,LocID INT
	,Location NVARCHAR(255)
	,Code NVARCHAR(255)
	,RVU FLOAT
	,CodeName NVARCHAR(255)
	,Quantity FLOAT
	,Category INT
	,CatName NVARCHAR(255)
	,ServiceID INT
	,InsResp INT
	,PayCategory NVARCHAR(255)
	,TransferredFromTDate DATETIME
	,TransferredFromIDate DATETIME
	,TotalTax1 MONEY
	,TotalTax2 MONEY
	,PreTaxChargeAmt MONEY
	,TotalDiscount MONEY
	,PercentDiscount INT
	,DollarDiscount MONEY
	,DiscountCategoryDescription NVARCHAR(1000)
	,ClaimProviderID INT
	,ChargeAmtBeforeDiscount MONEY);

						CREATE CLUSTERED INDEX NXI_TempAllFinancialQ%s_ID ON %s (ID);
CREATE NONCLUSTERED INDEX NXI_TempAllFinancialQ%s_PatientID ON %s (PatientID);

)"
, strTempT, strTempT, strTempT
, strGUID, strTempT
, strGUID, strTempT
);

		// Now add the individual components
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strBill, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strGCSold, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strCharges, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strPays, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strPartialPays, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strPrepays, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strPartialPrepays, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strPrepaysApplied, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(GetTransferFinType(strLineItemFilter), strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strAppliedPays, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strAppliedToPrepays, strTempT, fragmentFilter));
		//(s.tullis 2016-05-18 17:41) - NX-100491
		// Create and populate the temp table
		ADODB::_ConnectionPtr pCon = CReportInfo::IsServerSide(reportID, true) ? GetRemoteDataReportSnapshot() : GetRemoteDataSnapshot();
		NxAdo::PushMaxRecordsWarningLimit pmr(1000000); // Debug only: We expect well over 100 records to be inserted into the temp table. I raised the warning cap to 1M.
														// (c.haag 2016-03-01) - PLID 68565 - Ten minute timeout by default
		long nCommandTimeout = GetRemotePropertyInt("TempAllFinQTimeout", 600, reportID, "<None>", false);
		CIncreaseCommandTimeout cict(pCon, nCommandTimeout);
		// (c.haag 2016-03-14) - PLID 68592 - Show a message box because the query is too big to get from the Watch window
#ifdef _DEBUG
		CMsgBox dlg(NULL);
		dlg.msg = strSqlBatch;
		dlg.DoModal();
#endif

		ExecuteSql(pCon, "%s", strSqlBatch);

		// Return the temp table content
		return FormatString("SELECT * FROM [%s]", strTempT);
	}
}

// (r.gonet 2016-03-14 09:49) - PLID 68570 - Created a separate version for the Financial Activity Daily By Ins Co report because it can't
// use the normal GetAllFinancialQ function. The component queries are significantly different.
const CString GetAllFinancialQ_Insurance(int reportID, CString strLocFilter /*=""*/, CString strProvFilter /*=""*/, CString strPatientFilter /*=""*/, CString strBillDateFilter /*=""*/, CString strServiceDateFilter /*=""*/, CString strInputDateFilter /*=""*/, CString strExternalFilter /*=""*/)
{

	//fill in all the variable tables
	CString strLineItemFilter, strBillFilter, strChargeFilter, strPayFilter,
		strPersonFilter, strUserFilter, strLineItemDateFilter, strGCBillFilter, strLineItemLocationFilter;
	CString strTemp;
	// (j.jones 2016-03-15 15:10) - PLID 68598 - added location filter
	BOOL bHasBillDate, bHasServiceDate, bHasInputDate, bHasSupportedExtendedFilter, bHasLocationFilter;

	// (c.haag 2016-03-14) - PLID 68592 - Filter to pass into the GetAllFinancialQFragmentAsTempTableInsert
	CAllFinancialQFragmentFilter fragmentFilter;

	bHasBillDate = FALSE;
	bHasServiceDate = FALSE;
	bHasInputDate = FALSE;
	bHasSupportedExtendedFilter = FALSE;
	bHasLocationFilter = FALSE;

	strLineItemFilter = " SELECT * FROM LineItemT WHERE (DELETED = 0)";
	// (r.gonet 2016-03-14 14:36) - PLID 68570 - Added the DELETED = 0 check because it doesn't make sense to return deleted bills and that's
	// what GetAllFinancialQ does.
	strBillFilter = " SELECT * FROM BillsT WHERE (DELETED = 0)";
	strGCBillFilter = " SELECT * FROM BillsT WHERE (DELETED = 0)";
	strChargeFilter = " SELECT * FROM ChargesT WHERE (1=1)";
	strPayFilter = " SELECT * FROM PaymentsT WHERE (1=1)";
	strLineItemDateFilter = " ";
	strLineItemLocationFilter = "";

	if (!strLocFilter.IsEmpty()) {

		// (j.jones 2016-03-15 15:10) - PLID 68598 - added location filter
		bHasLocationFilter = TRUE;
		fragmentFilter.m_strLocationFilter = strLocFilter;

		strLocFilter.Replace("LocID", "LocationID");

		//(e.lally 2008-10-10) PLID 31421 - Support filtering on multiple locations
		strLineItemLocationFilter.Format(" AND %s", strLocFilter);
	}

	if (!strProvFilter.IsEmpty()) {
		strTemp.Format("%s AND DoctorsProviders IN (%s)", strChargeFilter, strProvFilter);
		strChargeFilter = strTemp;

		strTemp.Format("%s AND ProviderID IN (%s)", strPayFilter, strProvFilter);
		strPayFilter = strTemp;

	}

	if (!strPatientFilter.IsEmpty()) {
		CString strTemp2 = strPatientFilter;
		strTemp2.Replace("DailyFinancialInsCoQ.PatID", "LineItemT.PatientID");

		strTemp.Format("%s AND %s", strLineItemFilter, strTemp2);
		strLineItemFilter = strTemp;
	}

	if (!strBillDateFilter.IsEmpty()) {
		bHasBillDate = TRUE;

		CString strTemp2;
		strTemp = strBillDateFilter;
		fragmentFilter.m_strBillDateFilter = strTemp; // (c.haag 2016-03-14) - PLID 68592 - We also want to filter at the fragment level
		strTemp.Replace("BDate", "BillsT.Date");
		strTemp2.Format("%s AND %s ", strBillFilter, strTemp);
		strBillFilter = strTemp2;

		strTemp = strBillDateFilter;
		strTemp.Replace("BDate", "LineItemT.Date");
		strTemp2.Format("%s AND %s ", strLineItemDateFilter, strTemp);
		strLineItemDateFilter = strTemp2;
	}

	if (!strServiceDateFilter.IsEmpty()) {
		bHasServiceDate = TRUE;

		CString strTemp2;
		strTemp = strServiceDateFilter;
		fragmentFilter.m_strServiceDateFilter = strTemp; // (c.haag 2016-03-14) - PLID 68592 - We also want to filter at the fragment level
		strTemp.Replace("TDate", "BillsT.Date");
		strTemp2.Format("%s AND %s ", strGCBillFilter, strTemp);
		strGCBillFilter = strTemp2;

		strTemp = strServiceDateFilter;
		strTemp.Replace("TDate", "LineItemT.Date");
		strTemp2.Format("%s AND %s ", strLineItemDateFilter, strTemp);
		strLineItemDateFilter = strTemp2;
	}


	if (!strInputDateFilter.IsEmpty()) {
		bHasInputDate = TRUE;

		CString strTemp2;
		strTemp = strInputDateFilter;
		// (c.haag 2016-03-14) - PLID 68592 - We also want to filter at the fragment level
		fragmentFilter.m_strInputDateFilter = strTemp;
		fragmentFilter.m_strInputDateFilter.Replace("IDate", "InputDate");
		strTemp.Replace("IDate", "LineItemT.InputDate");
		strTemp2.Format("%s AND %s ", strLineItemDateFilter, strTemp);
		strLineItemDateFilter = strTemp2;
	}


	//Billed information.
	/*	Version History
	// (r.gonet 2016-03-14 14:36) - PLID 68570 - Moved to GetAllFinancialQ_Insurance from Report 468's query.
	*/
	CString strBill = FormatString(R"(
SELECT 
	'BILL' AS FinType,    
	LineItemT.ID, 
	LineItemT.PatientID, 
	LineItemT.Type,     
				[LineItemT].[Amount],     
	LineItemT.Description, 
	LineItemT.Date,     
	LineItemT.InputDate, 
	LineItemT.InputName,     
	LineItemT.Deleted, 
	LineItemT.DeleteDate,     
				LineItemT.DeletedBy,    
	BillsT.Date AS BDate,    
	LineItemT.Date AS TDate, 
	0 AS PrePayment,    
	Amt = ChargeRespT.Amount,    
	ChargesT.DoctorsProviders AS ProvID,    
	'Bill' AS RText,    
	0 AS ApplyID,    
	LineItemT.ID AS LineID,   
	LineItemT.LocationID AS LocID,   
	LocationsT.Name AS Location,   
	InsuranceCoT.PersonID AS InsCoID, 
	InsuranceCoT.Name AS InsCoName, 
	TotalDiscount AS DollarDiscount, 
	TotalPercentOff as PercentOff, 
	CONVERT(money,[LineItemT].[Amount]-ROUND((([LineItemT].[Amount]*(CASE WHEN TotalPercentOff Is Null THEN 1 ELSE ((100-Convert(float,TotalPercentOff))/100)END))- TotalDiscount),2)) AS TotalDiscount, 
	dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription,
	InsuranceCoT.FinancialClassID, 
	FinancialClassT.Name AS FinancialClass, 
	ChargesT.ClaimProviderID 
FROM 
(
	(%s %s %s) LineItemT 
	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID 
	LEFT JOIN 
	(
		(%s) ChargesT 
		LEFT JOIN (%s) BillsT ON ChargesT.BillID = BillsT.ID
	) ON LineItemT.ID = ChargesT.ID
)  
LEFT JOIN 
(
	SELECT 
		ChargeID, 
		SUM(Percentoff) as TotalPercentOff, 
		Sum(Discount) As TotalDiscount 
	FROM ChargeDiscountsT 
	WHERE DELETED = 0 
	GROUP BY ChargeID
) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID 
INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID 
INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID 
INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID 
LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID 
WHERE 
	LineItemT.Type = 10 
	AND LineItemT.Deleted =0 
	AND ChargeRespT.InsuredPartyID > -1 
AND ChargesT.ServiceID NOT IN (SELECT ServiceID FROM GCTypesT) 
)",
strLineItemFilter,
bHasBillDate ? "" : strLineItemDateFilter,
strLineItemLocationFilter,
strChargeFilter, strBillFilter);

	//Gift Certificates Sold
	/*	Version History
	// (r.gonet 2016-03-14 14:36) - PLID 68570 - Moved to GetAllFinancialQ_Insurance from Report 468's query.
	*/
	CString strGCSold = FormatString(R"(
SELECT 
	'GCSOLD' AS FinType,    
	LineItemT.ID, 
	LineItemT.PatientID, 
	LineItemT.Type,     
				[LineItemT].[Amount],     
	LineItemT.Description, 
	LineItemT.Date,     
	LineItemT.InputDate, 
	LineItemT.InputName,     
	LineItemT.Deleted, 
	LineItemT.DeleteDate,     
				LineItemT.DeletedBy,    
	BillsT.Date AS BDate,    
	LineItemT.Date AS TDate, 
	0 AS PrePayment,    
	Amt = ChargeRespT.Amount,    
	ChargesT.DoctorsProviders AS ProvID,    
	'Bill' AS RText,    
	0 AS ApplyID,    
	LineItemT.ID AS LineID,   
	LineItemT.LocationID AS LocID,   
	LocationsT.Name AS Location,   
	InsuranceCoT.PersonID AS InsCoID, 
	InsuranceCoT.Name AS InsCoName, 
	0 AS DollarDiscount, 
	0 AS PercentOff, 
	0 AS TotalDiscount, 
	'' AS DiscountCategoryDescription,
	InsuranceCoT.FinancialClassID, 
	FinancialClassT.Name AS FinancialClass,
	ChargesT.ClaimProviderID 
FROM 
(
	(%s %s %s) LineItemT 
	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID 
	LEFT JOIN 
	(
		(%s) ChargesT 
		LEFT JOIN (%s) BillsT ON ChargesT.BillID = BillsT.ID
	) ON LineItemT.ID = ChargesT.ID
)  
INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID 
INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID 
INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID 
LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID 
WHERE 
	LineItemT.Type = 10 
	AND LineItemT.Deleted = 0 
	AND ChargeRespT.InsuredPartyID > -1  
AND ChargesT.ServiceID IN (SELECT ServiceID FROM GCTypesT) 
)",
strLineItemFilter,
bHasInputDate ? strLineItemDateFilter : "",
strLineItemLocationFilter,
strChargeFilter,
bHasServiceDate ? strGCBillFilter : strBillFilter);

	//Payments
	/*	Version History
	// (r.gonet 2016-03-14 14:36) - PLID 68570 - Moved to GetAllFinancialQ_Insurance from Report 468's query.
	*/
	CString strPays = FormatString(R"(
SELECT 
	FinType = 
		CASE WHEN LineItemT.Type = 1 THEN 
			CASE WHEN [PaymentsT].[PayMethod] = 1 THEN 'CASH' 
			WHEN [PaymentsT].[PayMethod] = 2 THEN 'CHECK'   
			WHEN [PaymentsT].[PayMethod] = 3 THEN 'CREDIT' 
			WHEN [PaymentsT].[PayMethod] = 4 THEN 'GCREDEEMED' 
			END 
	  WHEN LineItemT.Type = 2 THEN 'ADJUSTMENT' 
	  WHEN LineItemT.Type = 3 THEN 
		CASE WHEN PaymentsT.PayMethod = 7 THEN 'RefundCash' 
		WHEN PaymentsT.PayMethod = 8 THEN 'RefundCheck'   
		WHEN PaymentsT.PayMethod = 9 THEN 'RefundCredit'  
			WHEN PaymentsT.PayMethod = 10 THEN 'RefundGC' 
			END
		END, 
	LineItemT.ID, 
	LineItemT.PatientID, 
	LineItemT.Type,    
	Amount = 
		CASE WHEN _PartiallyAppliedPaysQ.ID Is Null THEN   
			CASE WHEN LineItemT_1.ID Is Null THEN 
				[LineItemT].[Amount]   
			ELSE 
				[AppliesT].[Amount] 
			END 
		ELSE 
			[AppliesT].[Amount] 
		END,    
	LineItemT.Description, 
	LineItemT.Date,    
	LineItemT.InputDate, 
	LineItemT.InputName,    
	LineItemT.Deleted, 
	LineItemT.DeleteDate,    
	LineItemT.DeletedBy,   
	LineItemT.Date AS BDate, 
	LineItemT.Date AS TDate, 
	PaymentsT.PrePayment,    
	LineItemT.Amount AS Amt,    
	ProvID = 
		CASE WHEN [DoctorsProviders] Is Null THEN 
			[ProviderID] 
		ELSE 
			[DoctorsProviders] 
		END,    
	'Full' AS RText, 
	AppliesT.ID AS ApplyID,    
	LineItemT.ID AS LineID,   
	CASE WHEN LineItemT_1.ID Is Null THEN 
		LineItemT.LocationID 
	ELSE 
		LineItemT_1.LocationID 
	END AS LocID,   
	LocationsT.Name,   
	InsuranceCoT.PersonID AS InsCoID, 
	InsuranceCoT.Name AS InsName, 
	0 AS DollarDiscount, 
	0 AS PercentOff, 
	0 AS TotalDiscount, 
	'' AS DiscountCategoryDescription,
	InsuranceCoT.FinancialClassID, 
	FinancialClassT.Name AS FinancialClass,
	ChargesT.ClaimProviderID 
FROM 
(
	(%s %s) LineItemT 
	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID
) 
LEFT OUTER JOIN PaymentsT 
INNER JOIN InsuredPartyT 
INNER JOIN InsuranceCoT 
	ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID 
	ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID 
	ON LineItemT.ID = PaymentsT.ID 
LEFT OUTER JOIN   
(
	SELECT 
		LineItemT_1.ID, 
		LineItemT_1.Amount AS PayAmt,    
           SUM(AppliesT.Amount) AS ApplyAmt,    
		MIN([LineItemT_1].[Amount]) - SUM([AppliesT].[Amount]) AS Total,    
           LineItemT_1.PatientID   
	FROM LineItemT AS LineItemT_1 
	LEFT JOIN   
	(
		PaymentsT 
		LEFT JOIN   
		(
			AppliesT 
			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID
		) ON PaymentsT.ID = AppliesT.SourceID
	) ON LineItemT_1.ID = PaymentsT.ID   
	WHERE 
		LineItemT_1.Deleted = 0
		AND 
		(
			PaymentsT.PayMethod <> 10 
			OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT_1.GiftID, -1)
		)   
	GROUP BY 
		LineItemT_1.ID, 
		LineItemT_1.Amount,    
           LineItemT_1.PatientID   
	HAVING 
		LineItemT_1.ID IS NOT NULL 
		AND MIN([LineItemT_1].[Amount]) - SUM([AppliesT].[Amount]) <> 0
) _PartiallyAppliedPaysQ ON LineItemT.ID = _PartiallyAppliedPaysQ.ID 
LEFT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID 
LEFT OUTER JOIN LineItemT LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID 
LEFT OUTER JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID 
	LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID 
WHERE 
	LineItemT.ID IS NOT NULL 
	AND LineItemT.Deleted = 0 
	AND PaymentsT.ID IS NOT NULL 
	AND 
	(
		PaymentsT.PayMethod <> 10 
		OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT.GiftID, -1)
	)
)",
strLineItemFilter, strLineItemDateFilter);

	//Partial Payments
	/*	Version History
	// (r.gonet 2016-03-14 14:36) - PLID 68570 - Moved to GetAllFinancialQ_Insurance from Report 468's query.
	*/
	CString strPartialPays = FormatString(R"(
SELECT 
	FinType =    
		CASE WHEN LineItemT.Type = 1 THEN   
			CASE WHEN [PaymentsT].[PayMethod] = 1 THEN 'CASH'   
			WHEN [PaymentsT].[PayMethod] = 2 THEN 'CHECK'   
			WHEN [PaymentsT].[PayMethod] = 3 THEN 'CREDIT'   
			WHEN [PaymentsT].[PayMethod] = 4 THEN 'GCREDEEMED' 
			End   
		WHEN LineItemT.Type = 2 THEN 'ADJUSTMENT'   
		WHEN LineItemT.Type = 3	THEN 
			CASE WHEN PaymentsT.PayMethod = 7 THEN 'RefundCash'   
			WHEN PaymentsT.PayMethod = 8 THEN 'RefundCheck'   
			WHEN PaymentsT.PayMethod = 9 THEN 'RefundCredit'   
			WHEN PaymentsT.PayMethod = 10 THEN 'RefundGC'  
			End   
		End,    
	[_PartiallyAppliedPaysQ].ID, 
	LineItemT.PatientID, 
	LineItemT.Type, 
	[_PartiallyAppliedPaysQ].Total AS Amount, 
	LineItemT.Description, 
	LineItemT.Date, 
	LineItemT.InputDate, 
	LineItemT.InputName, 
	LineItemT.Deleted, 
	LineItemT.DeleteDate, 
	LineItemT.DeletedBy, 
	LineItemT.Date AS BDate, 
	LineItemT.Date AS TDate, 
	PaymentsT.PrePayment, 
	LineItemT.Amount AS Amt, 
	PaymentsT.ProviderID AS ProvID, 
	'Part' AS RText, 
	0 AS ApplyID, 
	LineItemT.ID AS LineID, 
	LineItemT.LocationID AS LocID, 
	LocationsT.Name AS Location,  
	InsuranceCoT.PersonID AS InsCoID, 
	InsuranceCoT.Name AS InsName, 
	0 AS DollarDiscount, 
	0 AS PercentOff, 
	0 AS TotalDiscount, 
	'' AS DiscountCategoryDescription,
	InsuranceCoT.FinancialClassID, FinancialClassT.Name AS FinancialClass,
	NULL AS ClaimProviderID 
FROM 
(
	(
		(
			SELECT 
				LineItemT_1.ID, 
				LineItemT_1.Amount AS PayAmt,    
Sum(AppliesT.Amount) AS ApplyAmt,    
				/*First a Amount*/
				Min([LineItemT_1].[Amount]) - Sum([AppliesT].[Amount]) AS Total,    
				LineItemT_1.PatientID    
			FROM LineItemT AS LineItemT_1 
			LEFT JOIN 
			(
				PaymentsT 
				LEFT JOIN 
				(
					AppliesT 
					LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID
				) ON PaymentsT.ID = AppliesT.SourceID
			) ON LineItemT_1.ID = PaymentsT.ID   
			WHERE 
				LineItemT_1.Deleted = 0 
				AND 
				(
					PaymentsT.PayMethod <> 10 
					OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT_1.GiftID, -1)
				)  
			GROUP BY 
				LineItemT_1.ID, 
				LineItemT_1.Amount, 
				LineItemT_1.PatientID    
			HAVING 
				LineItemT_1.ID is not Null 
				AND MIN([LineItemT_1].[Amount]) - Sum([AppliesT].[Amount]) <> 0
		) AS _PartiallyAppliedPaysQ 
		RIGHT OUTER JOIN 
		(
			(%s %s %s) LineItemT 
			LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID
		) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID
	) 
	INNER JOIN (%s) PaymentsT 
	INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID 
	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ON LineItemT.ID = PaymentsT.ID
)   
LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID 
WHERE 
	[_PartiallyAppliedPaysQ].ID Is Not Null 
	AND LineItemT.Deleted = 0 
	AND 
	(
		PaymentsT.PayMethod <> 10 
		OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT.GiftID, -1)
	)  
/* Modified 1/24/03 - Now shows ALL things that have been prepayments, not things that are currently prepayments */ 
)",
strLineItemFilter, strLineItemDateFilter, strLineItemLocationFilter, 
strPayFilter);

	//Now add up all of our sections into one big happy query.
	// (r.gonet 2016-03-28 00:45) - PLID 68570 - Write all the records to the temp table and return the temp table.
	// If we're not doing any filtering, just defer to the old UNION's since we can potentially flood the temp table
	// with millions and millions of records. This is also a possibility if filtering on large date ranges, in which case
	// the user can opt to disable using a temp table for this report through ConfigRT. If this optimization fails for a client,
	// you can disable it through inserting a record in ConfigRT with the name SupportTempAllFinQReport.
	if ((!bHasBillDate && !bHasServiceDate && !bHasInputDate && !bHasSupportedExtendedFilter && !bHasLocationFilter)
		|| (GetRemotePropertyInt("SupportTempAllFinQReport", 1, reportID, "<None>", false) != 1)
		) {
		return
			strBill + "\r\n"
			+ "UNION ALL \r\n"
			+ strGCSold + "\r\n"
			+ "UNION ALL \r\n"
			+ strPays + "\r\n"
			+ "UNION ALL \r\n"
			+ strPartialPays;
	} else {
		CString strTempT;
		CString strGUID = NewPlainUUID();
		strTempT.Format("#TempAllFinancialQ%s", strGUID);

		// (r.gonet 2016-03-28 00:45) - PLID 68570 - This is the SQL statement batch for creating and populating the
		// temp table. Note it is not parameterized because the query fragments are not parameterized. Doing so
		// would require major reporting changes beyond the scope of this optimization that we can maybe do
		// another time.
		// (r.goldschmidt 2016-04-27 14:35) - NX-100264 - fix incorrect nvarchar size limits (dbo.GetChargeDiscountList(ChargesT.ID) returns an nvarchar(1000))
		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, R"(
IF OBJECT_ID('tempdb..%s') IS NOT NULL
	DROP TABLE %s;

						CREATE TABLE %s (
	FinType NVARCHAR(255)
	,ID INT
	,PatientID INT
	,Type INT
	,Amount MONEY
	,Description NVARCHAR(255)
	,Date DATETIME
	,InputDate DATETIME
	,InputName NVARCHAR(50)
	,Deleted BIT
	,DeleteDate DATETIME
	,DeletedBy NVARCHAR(50)
	,BDate DATETIME
	,TDate DATETIME
	,PrePayment BIT
	,Amt MONEY
	,ProvID INT
	,RText NVARCHAR(255)
	,ApplyID INT
	,LineID INT
	,LocID INT
	,Location NVARCHAR(255)
	,InsCoID INT
	,InsCoName NVARCHAR(255)
	,DollarDiscount MONEY
	,PercentOff INT
	,TotalDiscount MONEY
	,DiscountCategoryDescription NVARCHAR(1000)
	,FinancialClassID INT
	,FinancialClass NVARCHAR(255)
	,ClaimProviderID INT
);

CREATE CLUSTERED INDEX NXI_TempAllFinancialQ%s_ID ON %s (ID);
CREATE NONCLUSTERED INDEX NXI_TempAllFinancialQ%s_PatientID ON %s (PatientID);

)"
, strTempT, strTempT, strTempT
, strGUID, strTempT
, strGUID, strTempT
);

		// Now add the individual components
		// We now pass in all the filters in CAllFinancialQFragmentFilter
		CAllFinancialQFragmentFilter fragmentFilter;
		fragmentFilter.m_strBillDateFilter = strBillDateFilter;
		fragmentFilter.m_strServiceDateFilter = strServiceDateFilter;
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strBill, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strGCSold, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strPays, strTempT, fragmentFilter));
		AddStatementToSqlBatch(strSqlBatch, "%s", GetAllFinancialQFragmentAsTempTableInsert(strPartialPays, strTempT, fragmentFilter));
		//(s.tullis 2016-05-18 17:41) - NX-100491
		// Create and populate the temp table
		ADODB::_ConnectionPtr pCon = CReportInfo::IsServerSide(reportID, true) ? GetRemoteDataReportSnapshot() : GetRemoteDataSnapshot();
		NxAdo::PushMaxRecordsWarningLimit pmr(1000000); // Debug only: We expect well over 100 records to be inserted into the temp table. I raised the warning cap to 1M.
														// (r.gonet 2016-03-28 00:45) - PLID 68570 - Ten minute timeout by default
		long nCommandTimeout = GetRemotePropertyInt("TempAllFinQTimeout", 600, reportID, "<None>", false);
		CIncreaseCommandTimeout cict(pCon, nCommandTimeout);
		// (r.gonet 2016-03-28 00:45) - PLID 68570 - Show a message box because the query is too big to get from the Watch window
#ifdef _DEBUG
		CMsgBox dlg(NULL);
		dlg.msg = strSqlBatch;
		dlg.DoModal();
#endif

		ExecuteSql(pCon, "%s", strSqlBatch);

		// Return the temp table content
		return FormatString("SELECT * FROM [%s]", strTempT);
	}
}

// (r.gonet 2016-03-14 18:58) - PLID 68570 - Gets the patient balance subquery shared by several financial reports.
const CString GetPatientBalanceQ(int reportID)
{
	// (r.gonet 2016-03-14 18:58) - PLID 68570 - I tried this out in its own temp table but that led to slightly worse performance
	// so I stuck with a query. I think it is valuable to keep this in its own function. The same subquery is used in several financial reports.
	// I also improved some parts of the query, some for small performance gains and some for readability and understandability:
	// - Removed the provider ID and name. They were incorrect before anyway, using the patient's middle name in the doctorname. Keep this subquery focused.
	// - Replaced overly-complicated CASE WHENs in the select list with COALESCE's to be more readable.
	// - Simplified some join nestings where the nesting was unnecessary.
	// - Removed a couple redundant columns from the GROUP BY clauses.
	// - Moved the HAVING Type < 4 for prepays into the WHERE clause and specified out the types in order to avoid an unbounded range and to be more selective earlier in the plan.
	// - Changed a couple LEFT JOINs into INNER JOINs where the join column was non-nullable and constrained by a foreign key.
	CString strBalance = R"(
SELECT 
	PatientsT.PersonID AS ID,
	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,
	COALESCE(ChargesQ.ChargeAmount, CAST(0 AS MONEY)) AS Charges,
	COALESCE(PaymentsQ.SumOfAmount, CAST(0 AS MONEY)) AS Payments,
	COALESCE(PrePaymentsQ.SumOfAmount, CAST(0 AS MONEY)) AS PrePayments,
	COALESCE(ChargesQ.ChargeAmount, CAST(0 AS MONEY)) - COALESCE(PaymentsQ.SumOfAmount, CAST(0 AS MONEY)) + COALESCE(PrePaymentsQ.SumOfAmount, CAST(0 AS MONEY)) AS AccountBal
FROM PatientsT
INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID
LEFT JOIN
(
	SELECT 
		LineItemT.PatientID,    
		Sum(ChargeRespT.Amount) AS ChargeAmount
	FROM LineItemT 
	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID 
	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID 
	INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID    
	INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID
	WHERE 
		LineItemT.Type = 10
		AND LineItemT.Deleted = 0
	GROUP BY LineItemT.PatientID
) ChargesQ ON ChargesQ.PatientID = PatientsT.PersonID
LEFT JOIN
(
	SELECT 
		PatientsT.PersonID AS ID,
		Sum(LineItemT.Amount) AS SumOfAmount
	FROM LineItemT 
	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID
	INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID
	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID
	WHERE 
		LineItemT.Deleted = 0
		AND LineItemT.Type IN (1, 2, 3) 
	GROUP BY PatientsT.PersonID
) PaymentsQ ON PaymentsQ.ID = PatientsT.PersonID
LEFT JOIN
(
	SELECT 
		ID,
		Sum(Amount) AS SumofAmount	  
	FROM  
	(
		SELECT
			PatientsT.PersonID AS ID,
			(
				CASE WHEN PrepayAppliedToQ.ID IS NULL THEN     
					CASE WHEN PrepayAppliesQ.ID IS NULL THEN 
						MAX(LineItemT.Amount) 
					ELSE 
						MAX(LineItemT.Amount - PrepayAppliesQ.Amount) 
					END 
				ELSE    
					MAX
					(
						CASE WHEN PrepayAppliesQ.ID IS NULL THEN 
							LineItemT.Amount - PrepayAppliedToQ.Amount 
						ELSE 
							LineItemT.Amount - PrepayAppliesQ.Amount - PrepayAppliedToQ.Amount 
						END
					) 
				END
			) AS Amount  
		FROM LineItemT
		INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID
		INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID
		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID
		LEFT JOIN AppliesT AS AppliesT_1 ON PaymentsT.ID = AppliesT_1.SourceID
		LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID  
		LEFT JOIN  
		/* This will total everything applied to a prepayment */ 
		( 
			SELECT 
				SUM(AppliesT.Amount * -1) AS Amount, 
				AppliesT.DestID AS ID  
			FROM LineItemT 
			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  
			INNER JOIN AppliesT ON LineItemT.ID = AppliesT.DestID  
			WHERE 
				LineItemT.Deleted = 0
				AND PaymentsT.Prepayment = 1  
			GROUP BY AppliesT.DestID  
		) PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID  
		LEFT JOIN  
		/* This will total everything that the prepayment is applied to */    
		(
			SELECT 
				SUM(AppliesT.Amount) AS Amount, 
				AppliesT.SourceID AS ID    
			FROM LineItemT 
			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    
			INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID    
			WHERE 
				LineItemT.Deleted = 0 
				AND PaymentsT.Prepayment = 1  
			GROUP BY AppliesT.SourceID    
		) PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID    
		/*end totalling applies to prepays */    
		WHERE 
			LineItemT.Deleted = 0 
			AND LineItemT.Type IN (1, 2, 3) 
			AND PaymentsT.PrePayment = 1
		GROUP BY 
			PatientsT.PersonID, 
			LineItemT.ID, 
			PrepayAppliedToQ.ID, 
			PrepayAppliesQ.ID
	) SubQ
	GROUP BY ID
) PrePaymentsQ ON PrePaymentsQ.ID = PatientsT.PersonID
)";

	return strBalance;
}

// (j.gruber 2010-03-11 11:34) - PLID 37713 - split transfers out
const CString GetTransferFinType(CString strLineItemFilter) {


	//Payments Applied (a)
	/*	Version History
	TES 3/8/2005 - When payments are applied to bills of a different location, it leads to their "location" changing.
	We will have two records, a positive payment amount for the bill's location, and a negative one for the payments location.
	TES 4/28/2005 - This also works for the provider.
	TES 5/13/2005 - This all still works the same way, but now it has a TransferredFromTDate field and
	TransferredFromIDate field, so that this data will be ignored unless the credit was transferred from a date
	earlier than the date range being filtered on.
	DRT 3/29/2007 - PLID 25414 - Fixed formatting.  Made a significant improvement in the subquery by removing the "DestID IN (...)"
	with a join to the referenced table and then just checking a field.
	TES 7/9/2008 - PLID 29580 - Made it so that applies are treated as having happened on AppliesT.InputDate, not on the later
	of the two dates involved in the apply.
	TES 10/13/2009 - PLID 35662 - Restored the old apply date calculation for everything but the input date
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	// (r.farnworth 2013-08-01) - PLID 45417 - Added Categories and CatName for Transers so that they would be grouped in reports
	*/
	CString strPaysAppliedPartA;
	strPaysAppliedPartA.Format("/*Payments Applied (a)*/ "
		"SELECT 'TRANSFER' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Amount, LineItemT.Description, LineItemT.Date, "
		"	AppliesSubQ.InputDate, "
		"	LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, "
		"	CASE WHEN LineItemT.Date > AppliesSubQ.BDate THEN LineItemT.Date ELSE AppliesSubQ.BDate END AS BDate, "
		"	CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS TDate, "
		"	0 AS Prepayment, Round(Convert(money,AppliesSubQ.Amount),2) AS Amt, AppliesSubQ.ProvID AS ProvID, 'PAYAPPLY' AS RText, "
		"	AppliesSubQ.ApplyID AS ApplyID, LineItemT.ID AS LineID, AppliesSubQ.LocID AS LocID, LocationsT.Name AS Location, "
		"	NULL AS Code, NULL AS RVU, NULL AS CodeName, 0 AS Quantity, AppliesSubQ.Category AS Category, AppliesSubQ.CatName AS CatName, NULL AS ServiceID, "
		"	CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, "
		"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, "
		"	LineItemT.Date AS TransferredFromTDate, LineItemT.InputDate AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, "
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, "
		"	AppliesSubQ.ClaimProviderID, 0 as chargeAmtBeforeDiscount "
		"\r\n\r\n"
		"FROM (%s) LineItemT "
		"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
		"	INNER JOIN "
		"	(SELECT AppliesT.ID AS ApplyID, SourceID, ServiceT.Category, "
		"		CASE WHEN CategoriesT.Name IS NULL THEN '' ELSE CategoriesT.Name END AS CatName, "
		"		CASE WHEN PayDestT.Prepayment = 1 THEN AppliesT.Amount ELSE AppliesT.Amount END AS Amount, "
		"		CASE WHEN BillsT.Description Is Null THEN LineItemT.Description ELSE BillsT.Description END AS Description, "
		"		CASE WHEN BillsT.Date IS Null THEN LineItemT.Date ELSE BillsT.Date END AS BDate, LineItemT.Date, AppliesT.InputDate, "
		"		CASE WHEN ChargesT.ID Is Null THEN PaymentsT.ProviderID ELSE ChargesT.DoctorsProviders END AS ProvID, "
		"		LineItemT.LocationID AS LocID, ChargesT.ClaimProviderID "
		" "
		"	FROM AppliesT "
		"		INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
		"		LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"		LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"		LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
		"		LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"		LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"		LEFT JOIN PaymentsT PayDestT ON AppliesT.DestID = PayDestT.ID "
		"	) AppliesSubQ  "
		"	ON LineItemT.ID = AppliesSubQ.SourceID "
		"	LEFT JOIN LocationsT ON AppliesSubQ.LocID = LocationsT.ID "
		" "
		"WHERE PaymentsT.PrePayment = 0",
		strLineItemFilter);

	//Payments Applied (b)
	/*	Version History
	DRT 3/29/2007 - PLID 25414 - Fixed formatting.  Made a significant improvement in the subquery by removing the "DestID IN (...)"
	with a join to the referenced table and then just checking a field.
	TES 7/9/2008 - PLID 29580 - Made it so that applies are treated as having happened on AppliesT.InputDate, not on the later
	of the two dates involved in the apply.
	TES 10/13/2009 - PLID 35662 - Restored the old apply date calculation for everything but the input date
	// (a.wilson 2011-9-30) PLID 44199 - added the RVU field and all union queries
	//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
	// (j.gruber 2012-08-13 10:59) - PLID 32726 - added charge before discount
	// (r.farnworth 2013-08-01) - PLID 45417 - Added Categories and CatName for Transers so that they would be grouped in reports
	*/
	CString strPaysAppliedPartB;
	strPaysAppliedPartB.Format("/*Payments Applied (b)*/ "
		"SELECT 'TRANSFER' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Amount, LineItemT.Description, LineItemT.Date, "
		"	AppliesSubQ.InputDate AS InputDate, "
		"	LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, "
		"	CASE WHEN LineItemT.Date > AppliesSubQ.BDate THEN LineItemT.Date ELSE AppliesSubQ.BDate END AS BDate, "
		"	CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS TDate, "
		"	0 AS Prepayment, -1 * Round(Convert(money,AppliesSubQ.Amount),2) AS Amt, PaymentsT.ProviderID AS ProvID, 'PAYAPPLY' AS RText, "
		"	AppliesSubQ.ApplyID AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, NULL AS Code, NULL AS RVU, NULL AS CodeName, "
		"	0 AS Quantity, AppliesSubQ.Category AS Category, AppliesSubQ.CatName AS CatName, NULL AS ServiceID, CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, "
		"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, "
		"	LineItemT.Date AS TransferredFromTDate, LineItemT.InputDate AS TransferredFromIDate, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS PreTaxChargeAmt, "
		"	0 AS TotalDiscount, 0 AS PercentDiscount, 0 AS DollarDiscount, '' AS DiscountCategoryDescription, "
		"	NULL AS ClaimProviderID, 0 as chargeAmtBeforeDiscount "
		" "
		"FROM (%s) LineItemT "
		"	INNER JOIN  PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
		"	INNER JOIN "
		"		(SELECT AppliesT.ID AS ApplyID, SourceID, ServiceT.Category, "
		"			CASE WHEN CategoriesT.Name IS NULL THEN '' ELSE CategoriesT.Name END AS CatName, "
		"			CASE WHEN PayDestT.PrePayment = 1 THEN Appliest.amount ELSE AppliesT.Amount END AS Amount, "
		"			CASE WHEN BillsT.Description Is Null THEN LineItemT.Description ELSE BillsT.Description END AS Description, "
		"			CASE WHEN BillsT.Date IS Null THEN LineItemT.Date ELSE BillsT.Date END AS BDate, LineItemT.Date, AppliesT.InputDate, "
		"			CASE WHEN ChargesT.ID Is Null THEN PaymentsT.ProviderID ELSE ChargesT.DoctorsProviders END AS ProvID, LineItemT.LocationID AS LocID "
		"		FROM AppliesT "
		"			INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
		"			LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		"			LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"			LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
		"			LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"			LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"			LEFT JOIN PaymentsT PayDestT ON AppliesT.DestID = PayDestT.ID "
		"		) AppliesSubQ "
		"	ON LineItemT.ID = AppliesSubQ.SourceID "
		"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
		" "
		"WHERE PaymentsT.PrePayment = 0",
		strLineItemFilter);

	return strPaysAppliedPartA + " UNION ALL " + strPaysAppliedPartB;

}

CString CReportInfo::GetSqlFinancial(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;
	COleDateTime dtNext;
	COleDateTimeSpan  OneDay(1, 0, 0, 0);

	// (f.dinatale 2010-10-15) - PLID 40876 - SSN Masking Permissions
	BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
	BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);

	switch (nID) {

	case 572:

		//Unbatched Claims


		//the basis for query is pretty much lifted directly from Billing Followup
		//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values
		return _T(
			FormatString(
				"SELECT BillID, PatientID AS PatID, UserDefinedID, Bal AS Balance, dbo.GetBillTotal(BillID) AS BillTotal, TC AS TotalCharges, TP AS TotalPayments, InsuredPartyID, InsCoName AS InsuranceCompany, InsuranceCoID AS InsCoID, RespTypeName, LastDate AS LastSentDate, PatName, BillDate AS TDate, BillInputDate AS IDate, "
				"LastTracerDate, LastPaymentDate, BillDescription, LocationID AS LocID, CASE WHEN LastDate IS NULL THEN 1 ELSE 2 END AS HasBeenSent "
				"FROM "
				"(SELECT TotalsPerChargeQ.BillID, TotalsPerChargeQ.PatientID, PatientsT.UserDefinedID, Sum(TotalCharges - TotalPays) AS Bal, Sum(TotalCharges) AS TC, Sum(TotalPays) AS TP, "
				"TotalsPerChargeQ.InsuredPartyID, InsuranceCoT.PersonID AS InsuranceCoID, InsuranceCoT.Name AS InsCoName, RespTypeT.TypeName AS RespTypeName, InsuranceCoT.HCFASetupGroupID, HistoryQ.LastDate, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, TotalsPerChargeQ.BillDate, TotalsPerChargeQ.BillInputDate, ClaimHistoryTracerQ.LastTracerDate, TotalsPerChargeQ.BillInputDate AS InputDate, "
				"LastPaymentQ.LastPaymentDate, TotalsPerChargeQ.BillDescription, TotalsPerChargeQ.LocationID "
				"FROM "
				"(SELECT BillsT.ID AS BillID, BillsT.Description AS BillDescription, RespQ.LocationID, BillsT.PatientID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, BillsT.Date AS BillDate, Sum(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END) AS TotalPays, BillsT.InputDate AS BillInputDate "
				"FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN "
				"	(SELECT ChargeRespT.ID, ChargeID, Sum(ChargeRespT.Amount) AS TotalCharges, LineItemT.LocationID, CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END AS InsuredPartyID "
				"	FROM ChargeRespT LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				"	AND ChargeRespT.InsuredPartyID Is Not Null "
				"	GROUP BY ChargeRespT.ID, ChargeID, LineItemT.LocationID, InsuredPartyID "
				"	) RespQ "
				"ON ChargesT.ID = RespQ.ChargeID "
				"LEFT JOIN "
				"	(SELECT AppliesT.RespID, Sum(AppliesT.Amount) AS Amount FROM PaymentsT "
				"	INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"	WHERE LineItemT.Deleted = 0 AND PaymentsT.InsuredPartyID Is Not Null "
				"	GROUP BY AppliesT.RespID "
				"	) PaysQ "
				"ON RespQ.ID = PaysQ.RespID "
				"GROUP BY BillsT.ID, BillsT.Description, BillsT.PatientID, RespQ.ChargeID, RespQ.LocationID, RespQ.TotalCharges, RespQ.InsuredPartyID, BillsT.Date, BillsT.InputDate "
				") TotalsPerChargeQ "
				"LEFT JOIN InsuredPartyT ON TotalsPerChargeQ.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN PersonT ON TotalsPerChargeQ.PatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN "
				"	(SELECT Max(Date) AS LastDate, ClaimHistoryT.BillID, InsuredPartyID "
				"	FROM ClaimHistoryT "
				"	WHERE SendType >= %li "
				"	GROUP BY ClaimHistoryT.BillID, InsuredPartyID "
				"	) HistoryQ "
				"ON TotalsPerChargeQ.BillID = HistoryQ.BillID AND TotalsPerChargeQ.InsuredPartyID = HistoryQ.InsuredPartyID "
				"LEFT JOIN "
				"	(SELECT BillID, Max(Date) AS LastTracerDate, InsuredPartyID FROM ClaimHistoryT WHERE SendType = %li GROUP BY BillID, InsuredPartyID "
				"	) AS ClaimHistoryTracerQ "
				"ON TotalsPerChargeQ.BillID = ClaimHistoryTracerQ.BillID AND TotalsPerChargeQ.InsuredPartyID = ClaimHistoryTracerQ.InsuredPartyID "
				"LEFT JOIN "
				"	(SELECT Max(LineItemT.Date) AS LastPaymentDate, PaymentsT.InsuredPartyID, ChargesT.BillID "
				"	FROM PaymentsT "
				"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"	INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"	INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
				"	INNER JOIN LineItemT LineItemT2 ON ChargesT.ID = LineItemT2.ID "
				"	WHERE LineItemT.Deleted = 0 AND LineItemT2.Deleted = 0 "
				"	GROUP BY PaymentsT.InsuredPartyID, ChargesT.BillID "
				"	) LastPaymentQ "
				"ON TotalsPerChargeQ.BillID = LastPaymentQ.BillID AND TotalsPerChargeQ.InsuredPartyID = LastPaymentQ.InsuredPartyID "
				"GROUP BY TotalsPerChargeQ.BillID, TotalsPerChargeQ.BillDescription, TotalsPerChargeQ.LocationID, TotalsPerChargeQ.PatientID, PatientsT.UserDefinedID, "
				"TotalsPerChargeQ.InsuredPartyID, InsuranceCoT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, InsuranceCoT.HCFASetupGroupID, HistoryQ.LastDate, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, TotalsPerChargeQ.BillDate, "
				"ClaimHistoryTracerQ.LastTracerDate, TotalsPerChargeQ.BillInputDate, LastPaymentQ.LastPaymentDate "
				") Q "
				"WHERE Bal <> 0 AND BillID NOT IN (SELECT BillID FROM HCFATrackT)",
				ClaimSendType::Electronic, ClaimSendType::TracerLetter));
		break;

	case 537: {

		//Electronic Remittance

		/* Version History

		JMJ 8/3/2006 - PLID 21602 - ensured that deleted batch payments and deleted charges don't show up
		// (j.jones 2008-12-16 09:52) - PLID 32317 - added birthdate, OHIP health card number, and OHIP
		// version code to the query
		// (j.jones 2009-07-06 17:15) - PLID 34776 - supported OriginalAmount
		// (j.jones 2009-09-25 11:55) - PLID 34453 - supported BatchPaymentOHIPMessagesT
		// (j.jones 2011-01-20 17:04) - PLID 42173 - added allowables
		// (j.jones 2011-03-18 10:12) - PLID 42157 - added EOBAllowable, and made the FeeSchedAllowable pull from ERemittanceHistoryT
		*/

		// (j.jones 2008-12-16 10:02) - PLID 32317 - find out what custom fields to use for the OHIP health number and version code
		long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
		long nVersionCodeCustomField = GetRemotePropertyInt("OHIP_VersionCodeCustomField", 2, 0, "<None>", true);

		switch (nSubLevel) {
		case 1: {
			//show BatchPaymentOHIPMessagesT in a subreport

			CString str = "SELECT BatchPaymentsT.ID AS BatchPaymentID, BatchPaymentsT.Date AS TDate, "
				"BatchPaymentsT.ProviderID AS ProvID, BatchPaymentsT.Location AS LocID, BatchPaymentsT.InsuranceCoID AS InsCoID, "
				"BatchPaymentOHIPMessagesT.Date AS TransactionDate, "
				"BatchPaymentOHIPMessagesT.Amount AS TransactionAmount, "
				"BatchPaymentOHIPMessagesT.ReasonCode, "
				"BatchPaymentOHIPMessagesT.Reason, "
				"BatchPaymentOHIPMessagesT.Message "
				"FROM BatchPaymentsT "
				"INNER JOIN BatchPaymentOHIPMessagesT ON BatchPaymentsT.ID = BatchPaymentOHIPMessagesT.BatchPaymentID "
				"WHERE BatchPaymentsT.Deleted = 0 AND BatchPaymentsT.Type = 1";
			return str;
			break;
		}

		case 0:
		default: {
			//main report

			CString str;
			str.Format("SELECT ERemittanceHistoryT.ID, ERemittanceHistoryT.EOBID, ERemittanceHistoryT.PostingDate AS TDate, "
				"ERemittanceHistoryT.BatchPayID, ERemittanceHistoryT.PatientID AS PatID, ERemittanceHistoryT.InsuredPartyID, "
				"ERemittanceHistoryT.ChargeID, ERemittanceHistoryT.ChargeAmount, ERemittanceHistoryT.PaymentAmount, "
				"ERemittanceHistoryT.AdjustmentAmount, ERemittanceHistoryT.PatResp, ERemittanceHistoryT.PatApplies, "
				"ERemittanceHistoryT.PatBalance, ERemittanceHistoryT.InsBalance, ERemittanceHistoryT.ShiftType, "
				"BatchPaymentsT.Amount AS BatchPayAmount, BatchPaymentsT.ProviderID AS ProvID, BatchPaymentsT.Location AS LocID, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
				"PersonProviderT.Last + ', ' + PersonProviderT.First + ' ' + PersonProviderT.Middle AS ProvName, "
				"LocationsT.Name AS LocName, InsuranceCoT.Name AS InsCoName, InsuranceCoT.PersonID AS InsCoID, "
				"ChargesT.ItemCode, LineItemT.Date AS ServiceDate, PersonT.BirthDate, "
				"OHIPHealthCardCustomT.TextParam AS OHIPHealthCardNum, OHIPVersionCodeCustomT.TextParam AS OHIPVersionCode, "
				"BatchPaymentsT.OriginalAmount, "
				"(SELECT Count(*) FROM BatchPaymentOHIPMessagesT WHERE BatchPaymentOHIPMessagesT.BatchPaymentID = BatchPaymentsT.ID) AS CountTransactionMessages, "
				"ERemittanceHistoryT.FeeSchedAllowable AS FeeSchedAllowedAmount, "
				"ERemittanceHistoryT.EOBAllowable "
				"FROM ERemittanceHistoryT "
				"INNER JOIN BatchPaymentsT ON ERemittanceHistoryT.BatchPayID = BatchPaymentsT.ID "
				"INNER JOIN PersonT ON ERemittanceHistoryT.PatientID = PersonT.ID "
				"INNER JOIN ChargesT ON ERemittanceHistoryT.ChargeID = ChargesT.ID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN PersonT PersonProviderT ON BatchPaymentsT.ProviderID = PersonProviderT.ID "
				"LEFT JOIN LocationsT ON BatchPaymentsT.Location = LocationsT.ID "
				"LEFT JOIN InsuredPartyT ON ERemittanceHistoryT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = %li) AS OHIPHealthCardCustomT ON PersonT.ID = OHIPHealthCardCustomT.PersonID "
				"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = %li) AS OHIPVersionCodeCustomT ON PersonT.ID = OHIPVersionCodeCustomT.PersonID "
				"WHERE BatchPaymentsT.Deleted = 0 AND LineItemT.Deleted = 0 ", nHealthNumberCustomField, nVersionCodeCustomField);

			return str;
			break;
		}
		}
	}
			  break;

	case 136:
	{
		//Deposits Made
		/*	Version History
		DRT 3/11/2004 - PLID 11382 - Added tips.
		JMM 5/18/2006 - PLID 12412 - Changed Tips so that they only show separately when there is a different payment type
		JMM 11/3/2005 - PLID 13548 - Added refunds
		JMJ 10/11/2006 - PLID 22955 - Supported Adjusted Batch Payments by ignoring them
		// (j.gruber 2007-05-02 10:49) - PLID 25876 - changed the alias in repoinfocallback to stop the assertions
		// (j.jones 2011-09-15 16:48) - PLID 45202 - we now hide void and corrected payments, only showing the originals
		// (j.jones 2012-04-19 14:23) - PLID 48032 - changed to use PaymentsT.BatchPaymentID
		// (b.spivey, July 24, 2012) - PLID 44450 - added input date.
		// (r.gonet 2015-05-01 14:44) - PLID 65870 - Do NOT support gift certificate refunds here. They are not deposited since
		// they already have the money.
		*/

		CString strRefunds, strBatchRefunds, str;
		if (GetRemotePropertyInt("BankingIncludeRefunds", 1, 0, GetCurrentUserName(), true)) {
			strRefunds = "OR (LineItemT.Type = 3 AND PaymentsT.PayMethod IN (7,8,9))";
			strBatchRefunds = "Type <> 2 AND ";
		}
		else {
			strRefunds = "";
			strBatchRefunds = "Type = 1 AND ";
		}
		str.Format("SELECT DepositsQ.PatID AS PatID, DepositsQ.PatName, DepositsQ.ProvID AS ProvID, DepositsQ.DocName, "
			"DepositsQ.Amount, DepositsQ.TDate AS TDate, DepositsQ.Date AS Date, DepositsQ.Description, DepositsQ.PayMethod, "
			"DepositsQ.PatientID, DepositsQ.LocID AS LocID, DepositsQ.Location, DepositsQ.InputDate "
			"FROM ("
			"SELECT "
			/*Payment Info*/
			"LineItemT.PatientID AS PatID, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS PatName, "
			"ProvidersT.PersonID AS ProvID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
			"LineItemT.Amount + (SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END FROM PaymentTipsT WHERE PaymentID = PaymentsT.ID AND PayMethod = PaymentsT.PayMethod) AS Amount, "
			"PaymentsT.DepositDate AS TDate, "
			"LineItemT.Date, LineItemT.Description, "
			"PaymentsT.PayMethod, "
			"PatientsT.UserDefinedID AS PatientID, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location, "
			"LineItemT.InputDate AS InputDate "
			"FROM PersonT PersonT1 INNER JOIN "
			"PatientsT ON PersonT1.ID = PatientsT.PersonID INNER JOIN "
			"(LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN "
			"PaymentsT ON LineItemT.ID = PaymentsT.ID ON "
			"PatientsT.PersonID = LineItemT.PatientID LEFT OUTER JOIN "
			"PersonT INNER JOIN "
			"ProvidersT ON PersonT.ID = ProvidersT.PersonID ON "
			"PaymentsT.ProviderID = ProvidersT.PersonID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			"WHERE (PaymentsT.Deposited = 1) AND (LineItemT.Type = 1 %s) "
			"AND CorrectedLineItemsT.NewLineItemID Is Null AND VoidingLineItemsT.VoidingLineItemID Is Null "
			"AND (LineItemT.Deleted = 0) AND LineItemT.Amount <> 0 AND PaymentsT.BatchPaymentID Is Null AND PaymentsT.PayMethod NOT IN (4,10) "
			"UNION ALL SELECT "
			/*BatchPaymentInfo */
			"NULL AS PatID, "
			"InsuranceCoT.Name AS PatName, "
			"ProviderID AS ProvID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, "
			"CASE WHEN Type <> 1 THEN -1 * Amount ELSE Amount END AS Amount, "
			"DepositDate AS TDate, "
			"Date, Description, "
			"CASE WHEN Type = 1 THEN 2 WHEN Type = 3 THEN 8 ELSE 0 END AS PayMethod, "
			"NULL AS PatientID, "
			"BatchPaymentsT.Location AS LocID, "
			"LocationsT.Name AS Location, "
			"NULL AS InputDate "
			"FROM BatchPaymentsT INNER JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN LocationsT ON BatchPaymentsT.Location = LocationsT.ID "
			"LEFT JOIN PersonT ON BatchPaymentsT.ProviderID = PersonT.ID WHERE Deposited = 1 AND Deleted = 0 AND %s Amount <> 0 "
			"/*Tips*/"
			"UNION ALL SELECT PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle + '  (Tip)' AS PatName, "
			"PersonProv.ID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS DocName, "
			"PaymentTipsT.Amount, PaymentTipsT.DepositDate AS TDate, LineItemT.Date, 'Tip' AS Description, "
			"PaymentTipsT.PayMethod, "
			"PatientsT.UserDefinedID, LocationsT.ID AS LocID, LocationsT.Name AS Location, LineItemT.InputDate AS InputDate "
			"FROM PaymentTipsT INNER JOIN PaymentsT ON PaymentTipsT.PaymentID = PaymentsT.ID "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			"WHERE PaymentTipsT.Deposited = 1 AND Deleted = 0 AND PaymentsT.PayMethod <> PaymentTipsT.PayMethod "
			"AND CorrectedLineItemsT.NewLineItemID Is Null AND VoidingLineItemsT.VoidingLineItemID Is Null AND PaymentsT.PayMethod NOT IN (4,10) "
			") AS DepositsQ", strRefunds, strBatchRefunds);
		return str;
	}
	break;


	case 137:
	{

		//Financial Summary
		/*	Version History
		1/24/03 - DRT - Made a significant change to the way we show prepayments (again).  Now this report shows (in the prepayments field), ALL prepayments,
		even if they have been applied or had things applied to them.  Then we calculate another field of that info (the applies to/from prepays), and
		add that in.  So the final calculation of Net Patient Receivables is now:  net pays + adjustments - prepays + applied prepays = receivables.
		This change is also being applied to all reports with AllFinancialQ (the ___ Financial Activity series)

		3/2/04 - TES - Copied the changes I made to Monthly Financial the other day here, so that I could add the AR
		progression for PLID 6903
		4/8/2004 - DRT - PLID 11816 - Replaced a LineItemT.* with appropriate fields
		8/25/2004 - TES - Moved AllFinancialQ to a #define.
		3/8/2005 - TES - Added BDate, and started filtering on it (as we always should have been!)
		4/29/2005 - TES - Referenced new InsResp field.
		4/29/2005 - TES - Also referenced new PayCategory field.
		5/13/05 - TES - Referenced new TransferredFromTDate and TransferredFromIDate fields.
		5/19/2005 - DRT - Removed LocationID from AllFinancialQ.  Had to reverify ttx file.
		//(e.lally 2007-08-03) PLID 25116 - Made FinType 40 characters
		(d.moore 2007-09-18) - PLID 25166 - Added fields for discounts and discount categories.
		// (j.gruber 2010-03-11 09:29) - PLID 37713 - made a hidden preference to calculate the AR

		*/


		long nSumWithQuery = GetRemotePropertyInt("FinancialSummaryQueryAR", 0, 0, "<None>");
		CString strTableName;

		// (j.gruber 2010-05-18 12:35) - PLID 38467 - handle the APPTOPRE

		if (nSumWithQuery != 0) {
			//they want to get it from the query and not have the report calculate it
			CString strLineItemFilter;
			CString strPayApplyTotal, strFuturePayApplyTotal;

			COleDateTime dtFrom, dtTo;
			if (nDateRange == -1) {
				dtFrom.SetDate(1800, 01, 01);
				dtTo.SetDate(2020, 12, 31);
			}
			else {
				dtFrom = DateFrom;
				dtTo = DateTo;
			}

			COleDateTimeSpan dtOneDay(1, 0, 0, 0);
			CString strLocationFilter;
			if (nLocation > 0) {
				strLocationFilter.Format(" AND FinalQ.LocID = %li", nLocation);
			}
			else if (nLocation == -2) {
				strLocationFilter.Format(" AND (FinalQ.LocID IS NULL)");
			}
			else if (nLocation == -3) {

				strLocationFilter.Format(" AND FinalQ.LocID IN");

				CString strPart;
				CString strIDs = "";

				for (int i = 0; i < m_dwLocations.GetSize(); i++) {
					strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
					strIDs += strPart;
				}

				strIDs = "(" + strIDs.Left(strIDs.GetLength() - 2) + ")";
				strLocationFilter += strIDs;
			}

			//provider
			CString strProvFilter;
			if (nProvider > 0) {
				strProvFilter.Format(" AND FinalQ.ProvID = %li", nProvider);
			}
			else if (nProvider == -2) {
				strProvFilter.Format(" AND (FinalQ.ProvID IS NULL OR FinalQ.ProvID = -1) ");
			}
			else if (nProvider == -3) {
				CString strAns;
				CString strPart;
				for (int i = 0; i < m_dwProviders.GetSize(); i++) {
					strPart.Format("%li, ", (long)m_dwProviders.GetAt(i));
					strAns += strPart;
				}
				strProvFilter.Format(" AND FinalQ.ProvID IN (%s)", strAns.Left(strAns.GetLength() - 2));
			}

			CString strDateField;
			if (nDateFilter == 1) {
				//serviceDate 
				strDateField = "Date";
			}
			else {
				strDateField = "InputDate";
			}

			strLineItemFilter.Format(" SELECT * FROM LineItemT WHERE DELETED = 0 AND "
				" %s < CONVERT(datetime, '%s')  "
				, strDateField, FormatDateTimeForSql(dtFrom));

			strPayApplyTotal.Format(" WHERE (BDate < DateAdd(d ,1,CONVERT(datetime, '%s')) AND "
				" BDate >= CONVERT(dateTime, '%s')) ", FormatDateTimeForSql(dtTo), FormatDateTimeForSql(dtFrom));

			strFuturePayApplyTotal.Format(" WHERE BDate > DateAdd(d,1, CONVERT(dateTime, '%s')) ", FormatDateTimeForSql(dtTo));

			CString strEndFilter;

			//for the AR, don't use any date
			if (!strProvFilter.IsEmpty()) {
				strEndFilter = strProvFilter;
			}

			if (!strLocationFilter.IsEmpty()) {
				if (strEndFilter.IsEmpty()) {
					strEndFilter = strLocationFilter;
				}
				else {
					strEndFilter += " AND " + strLocationFilter;
				}
			}

			CString strPatientFilter;
			CString strPatFilter;
			if (bUseGroup) {
				CString str, strFilter = DefGetGroupFilter(nSubLevel, nSubRepNum);

				//we want to strip off the "(<something> IN " part of it
				long nIn = strFilter.Find("IN");
				strFilter = strFilter.Right(strFilter.GetLength() - nIn - 3);

				strPatientFilter.Format(" AND (FinalQ.PatientID IN %s", strFilter);
				strPatFilter.Format(" AND (FinalQ.PatientID IN %s", strFilter);
			}
			else if (bUseFilter) {

				CString str, strFilter = DefGetFilter(nSubLevel, nSubRepNum);

				//we want to strip off the "(<something> IN " part of it
				long nIn = strFilter.Find("IN");

				strFilter = strFilter.Right(strFilter.GetLength() - nIn - 3);

				strPatientFilter.Format(" AND (FinalQ.PatientID IN %s", strFilter);
				strPatFilter.Format(" AND (FinalQ.PatID IN %s", strFilter);

			}

			CString strTemp;
			
			strTemp.Format("WHERE FinalQ.FinType NOT IN ('CHARGE', 'TRANSFER', 'APPLIEDPAYS', 'APPTOPRE') AND %s < DateAdd(d, 1, CONVERT(DateTime, '%s')) %s %s %s ",
			strDateFilterField == "IDate" ? "InputDate" : strDateFilterField, FormatDateTimeForSql(dtTo), strLocationFilter, strProvFilter, strPatientFilter);

			CString strStartingSql = " SELECT CASE WHEN FinalQ.FinType = 'BILL' OR FinalQ.FinType = 'GCSOLD' then FinalQ.Amt "
				" 	   ELSE  "
				" 			CASE WHEN FinalQ.FinType = 'APPPRE' then (-1 * FinalQ.Amt) "
				"				 ELSE CASE WHEN FinalQ.FinType = 'CHARGE' or FinalQ.FinType = 'TRANSFER' then 0.00 "
				" 						   ELSE CASE WHEN FinalQ.FinType = 'PREPAYMENT' then FinalQ.Amount "
				" 									 ELSE CASE WHEN FinalQ.FinType = 'APPLIEDPAYS' OR FinalQ.FinType = 'APPTOPRE' then 0 "
				" ELSE -1 * FinalQ.Amount "
				" 										  END "
				" 								END "
				" 					  END "
				" 		    END "
				" 		END as Amount, ProvID as ProvID, LocID as LocID, InputDate AS IDate FROM ( "
				ALL_FINANCIAL_Q(nID) " ) FinalQ " + strTemp;

			CString strEndingSql =
				" SELECT CASE WHEN FinalQ.FinType = 'BILL' OR FinalQ.FinType = 'GCSOLD' then FinalQ.Amt \r\n"
				" 	   ELSE  \r\n"
				" 			CASE WHEN FinalQ.FinType = 'APPPRE' then (-1 * FinalQ.Amt) \r\n"
				"				 ELSE CASE WHEN FinalQ.FinType = 'CHARGE' or FinalQ.FinType = 'TRANSFER' then 0.00 \r\n"
				" 						   ELSE CASE WHEN FinalQ.FinType = 'PREPAYMENT' then FinalQ.Amount \r\n"
				" 									 ELSE CASE WHEN FinalQ.FinType = 'APPLIEDPAYS' or FinalQ.FinType = 'APPTOPRE' then 0 \r\n"
				" ELSE -1 * FinalQ.Amount \r\n"
				" 										  END \r\n"
				" 								END \r\n"
				" 					  END \r\n"
				" 		    END \r\n"
				" 		END as Amount, ProvID as ProvID, LocID as LocID, InputDate AS IDate FROM ( \r\n"
				ALL_FINANCIAL_Q(nID) " ) FinalQ " + strTemp;


			CString strPayApplySql, strFuturePayApplySql;
			strPayApplySql = " SELECT Amt, ProvID as ProvID, LocID as LocID, InputDate AS IDate FROM ( "
				+ GetTransferFinType(strLineItemFilter) + ") FinalQ " + strPayApplyTotal + strLocationFilter + strProvFilter + strPatientFilter;

			strFuturePayApplySql = " SELECT Amt, ProvID as ProvID, LocID as LocID, InputDate AS IDate FROM ( "
				+ GetTransferFinType(strLineItemFilter) + ") FinalQ " + strFuturePayApplyTotal + strLocationFilter + strProvFilter + strPatientFilter;


			//insert them into the database			
			strTableName.Format("#TempFinancialSum%lu", GetTickCount());

			// (c.haag 2016-03-14) - PLID 68592 - Make this readable and debuggable
			CString strSql = FormatString(""
				" CREATE TABLE %s (StartingAR money, EndingAR money, PayApplyTotal money, FuturePayApplyTotal money)  "
				" DECLARE @cyStartingAR money \r\n "
				" DECLARE @cyEndingAR money \r\n "
				" SET @cyStartingAR = (SELECT Sum(Amount) FROM (" + strStartingSql + ") ARQ) \r\n "
				" SET @cyEndingAR = (SELECT Sum(AMOUNT) FROM (" + strEndingSql + ") ARFQ) \r\n "
				" DECLARE @cyPayApplyTotal money \r\n "
				" SET @cyPayApplyTotal =  (SELECT Sum(Amt) FROM (" + strPayApplySql + " ) Q ) \r\n "
				" DECLARE @cyFuturePayApplyTotal money \r\n "
				" SET @cyFuturePayApplyTotal =  (SELECT Sum(Amt) FROM (" + strFuturePayApplySql + " ) Q ) \r\n "
				" INSERT INTO %s (StartingAR, EndingAR, PayApplytotal, FuturePayApplyTotal) VALUES "
				" (COALESCE(@cyStartingAR, Convert(money, 0)), COALESCE(@cyEndingAR, Convert(money, 0)), "
				" COALESCE(@cyPayApplyTotal, Convert(money, 0)), COALESCE(@cyFuturePayApplyTotal, Convert(money, 0)))", strTableName, strTableName);
#ifdef _DEBUG
			CMsgBox dlg(NULL);
			dlg.msg = strSql;
			dlg.DoModal();
#endif

			ExecuteSqlStd(CReportInfo::IsServerSide(nID, true) ? GetRemoteDataReportSnapshot() : GetRemoteDataSnapshot(), strSql);
		}


		CString strReturn;

		strReturn.Format("SELECT AllFinancialQ.FinType, AllFinancialQ.ID, AllFinancialQ.PatientID,  "
			"AllFinancialQ.Type, AllFinancialQ.Amount, AllFinancialQ.Description, AllFinancialQ.Date,  "
			"AllFinancialQ.InputDate AS IDate, AllFinancialQ.InputName, AllFinancialQ.Deleted,  "
			"AllFinancialQ.DeleteDate, AllFinancialQ.DeletedBy, "
			"AllFinancialQ.TDate AS TDate, AllFinancialQ.PrePayment, AllFinancialQ.Amt,  "
			"AllFinancialQ.ProvID AS ProvID, AllFinancialQ.RText, AllFinancialQ.ApplyID,  "
			"AllFinancialQ.LineID, AllFinancialQ.LocID AS LocID, AllFinancialQ.Location, "
			"AllFinancialQ.BDate AS BDate, AllFinancialQ.PatientID AS PatID, AllFinancialQ.InsResp, "
			"AllFinancialQ.PayCategory, "
			"AllFinancialQ.TransferredFromTDate, "
			"AllFinancialQ.TransferredFromIDate, "
			"AllFinancialQ.TotalDiscount, "
			"AllFinancialQ.PercentDiscount, "
			"AllFinancialQ.DollarDiscount, "
			"AllFinancialQ.DiscountCategoryDescription, "
			" %s as StartingAR, \r\n "
			" %s as EndingAR, \r\n "
			" %s as PayApplyTotal, \r\n "
			" %s as FuturePayApplyTotal \r\n "
			"FROM "
			"("
			"%s "
			") AS AllFinancialQ",
			nSumWithQuery == 0 ? " CONVERT(money, 0) " : " (SELECT StartingAR FROM " + strTableName + " ) ",
			nSumWithQuery == 0 ? " CONVERT(money, 0) " : " (SELECT EndingAR FROM " + strTableName + " ) ",
			nSumWithQuery == 0 ? " CONVERT(money, 0) " : " (SELECT PayApplyTotal FROM " + strTableName + " ) ",
			nSumWithQuery == 0 ? " CONVERT(money, 0) " : " (SELECT FuturePayApplyTotal FROM " + strTableName + " ) ",
			GetAllFinancialQ(nID));

		return strReturn;

	}
	break;


	case 148:
	{
		//Tax Totals
		/* - Revision History
		TS 5/2/2003: Made sure that all currency fields were converted to money, not left as floats or whatever.
		DRT 8/30/2005 - PLID 15996 - Included the optional data from preferences to show refunded or adjusted items on this report.
		DRT 1/6/2006 - PLID 18721 - Changed the UNION to UNION ALL in both of the optional additions to the query.  If these were on,
		the report was removing any duplicate charges.
		JMJ 2/3/2006 - PLID 19139 - The Returned/Adjusted products were showing the patient's default provider name,
		not the charge provider name
		(a.walling 2007-03-21 12:17) - PLID 25189 - filter on refund date (linepays.date), not ReturnedProductsT.DateReturned.
		(e.lally 2007-06-25) PLID 26329 - Surrounded the Type field values with a conversion to a 50 character string so that the
		report verification does not vary depending on your current preferences since the values are hardcoded strings.
		// (j.jones 2008-06-03 14:28) - PLID 29928 - we now only include ReturnedProductsT records that are not for deleted charges
		// (j.gruber 2009-03-25 16:15) - PLID 33359 updated for new structure
		*/
		CString strBase;
		strBase.Format("SELECT Type, TotalAmt, PreTax, TotalTax1, TotalTax2, Description, ItemCode, TDate AS TDate, ProvID AS ProvID, UserDefinedID, "
			"	PatID AS PatID, PatName, DocName, LocID AS LocID, Location "
			"FROM "
			"	 (SELECT CONVERT(NVARCHAR(50), 'Taxed Charges') AS Type, dbo.GetChargeTotal(ChargesT.ID) AS TotalAmt,  "
			"	 Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(1)))),2) AS PreTax, "
			"	 Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2) AS TotalTax1, "
			"	 Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2) AS TotalTax2, "
			"	 LineItemT.Description, ChargesT.ItemCode, LineItemT.Date AS TDate, ChargesT.DoctorsProviders AS ProvID,  "
			"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName, LineItemT.LocationID AS LocID, LocationsT.Name AS Location "
			"FROM ChargesT LEFT OUTER JOIN "
			"    PersonT PersonT1 ON  "
			"    ChargesT.DoctorsProviders = PersonT1.ID LEFT OUTER JOIN "
			"    PatientsT LEFT OUTER JOIN "
			"    PersonT ON  "
			"    PatientsT.PersonID = PersonT.ID RIGHT OUTER JOIN "
			"    (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PatientsT.PersonID = LineItemT.PatientID ON  "
			"    ChargesT.ID = LineItemT.ID "
			"WHERE (LineItemT.Type = 10) AND (LineItemT.Deleted = 0) AND ("
			"    Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2) <> 0 OR "
			"    Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2) <> 0)");


		//DRT 8/29/2005 - PLID 15996 - Include returned items or adjusted items if they have chosen the preferences to do so.
		if (GetRemotePropertyInt("Rpt_ReturnUseTaxTotals", 1, 0, "<None>", true)) {
			CString strReturn;

			//If we have chosen to use return data on the tax totals report
			if (GetRemotePropertyInt("Rpt_UseReturnProduct", 0, 0, "<None>", true)) {
				//If we have chosen to use the Return Product feature to specify our data
				strReturn.Format("\r\nUNION ALL SELECT CONVERT(NVARCHAR(50),'Returned Products') AS Type, LinePays.Amount * -1 AS AdjAmount,  "
					"LinePays.Amount * -1 AS PreTax, "
					"Round(convert(money,((LineCharges.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1))),2) "
					"* LinePays.Amount / dbo.GetChargeTotal(ChargesT.ID) * -1 AS TotalTax1,  "
					"Round(convert(money,((LineCharges.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1))),2) "
					"* LinePays.Amount / dbo.GetChargeTotal(ChargesT.ID) * -1 AS TotalTax2,  "
					"LinePays.Description, ChargesT.ItemCode, LinePays.Date AS TDate, ChargesT.DoctorsProviders AS ProvID, "
					"PatientsT.UserDefinedID,  "
					"PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
					"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS DocName, LocationsT.ID AS LocID, LocationsT.Name AS LocName "
					" "
					"FROM ReturnedProductsT "
					"INNER JOIN ChargesT ON ReturnedProductsT.ChargeID = ChargesT.ID "
					"INNER JOIN LineItemT LineCharges ON ChargesT.ID = LineCharges.ID "
					"LEFT JOIN PaymentsT ON ReturnedProductsT.FinAdjID = PaymentsT.ID "
					"LEFT JOIN LineItemT LinePays ON PaymentsT.ID = LinePays.ID "
					"LEFT JOIN PersonT ON LinePays.PatientID = PersonT.ID "
					"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN PersonT PersonProv ON ChargesT.DoctorsProviders = PersonProv.ID "
					"LEFT JOIN LocationsT ON LineCharges.LocationID = LocationsT.ID "
					"WHERE LineCharges.Deleted = 0 AND LinePays.ID Is Not Null AND LinePays.Deleted = 0 AND ("
					"Round(convert(money, Convert(money,((LineCharges.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2) <> 0 OR "
					"Round(convert(money, Convert(money,((LineCharges.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2) <> 0)");
			}
			// (d.thompson 2009-01-05) - PLID 32612 - Remove RetAdj in favor of !RetProd
			else {
				//If we have chosen to use applied adjustments to specify our data
				strReturn.Format("\r\nUNION ALL SELECT CONVERT(NVARCHAR(50),'Applied Adjustments') AS Type, AppliesT.Amount * -1 AS AppliedAmount, "
					"AppliesT.Amount * -1 AS PreTax, "
					"Round(convert(money, ((LineCharges.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1))),2) "
					"* AppliesT.Amount / dbo.GetChargeTotal(ChargesT.ID) * -1 AS TotalTax1,  "
					"Round(convert(money, ((LineCharges.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1))),2) "
					"* AppliesT.Amount / dbo.GetChargeTotal(ChargesT.ID) * -1 AS TotalTax2,  "
					"LinePays.Description, ChargesT.ItemCode,  "
					"AppliesT.InputDate AS TDate, ChargesT.DoctorsProviders AS ProvID,  "
					"PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
					"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS DocName, LocationsT.ID AS LocID, LocationsT.Name AS LocName "
					"FROM ChargesT  "
					"INNER JOIN LineItemT LineCharges ON ChargesT.ID = LineCharges.ID "
					"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
					"INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
					"INNER JOIN LineItemT LinePays ON PaymentsT.ID = LinePays.ID "
					"LEFT JOIN PersonT ON LineCharges.PatientID = PersonT.ID "
					"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN PersonT PersonProv ON ChargesT.DoctorsProviders = PersonProv.ID "
					"LEFT JOIN LocationsT ON LineCharges.LocationID = LocationsT.ID "
					"WHERE LinePays.Type = 2 AND LinePays.Deleted = 0 AND ("
					"Round(convert(money, Convert(money,((LineCharges.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2) <> 0 OR "
					"Round(convert(money, Convert(money,((LineCharges.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2) <> 0)");
			}

			strBase += strReturn;
		}

		strBase += ") SubQ ";

		return _T(strBase);
	}	break;
	case 149:
		//UnApplied Credits
		/*Version History
		TES 9/8/2004 - PLID 14021 - Included gift certificates.
		(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType
		TES 2/13/2009 - PLID 23266 - Added Adjustments in, and changed the LineItemTyp to differentiate
		between regular payments and prepayments (they use some arbitrary negative numbers to make the
		filter list order properly, and not conflict with anything).
		*/
		return _T("SELECT PatientsT.PersonID AS PatID,  "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
			"LineItemT.Date AS TDate,  "
			"PaymentPlansT.CheckNo,  "
			"CreditCardNamesT.CardName AS CCType, "
			"PayType = CASE "
			"	WHEN [Type]=1 "
			"	THEN CASE "
			"		WHEN [PayMethod]=1 "
			"		THEN 'Cash' "
			"		ELSE CASE "
			"			WHEN [PayMethod]=2 "
			"			THEN 'Check' "
			"			WHEN [PayMethod]=4 "
			"			THEN 'Gift Certificate' "
			"			ELSE 'Credit Card' "
			"			End "
			"		End "
			"	ELSE CASE "
			"		WHEN [Type]=2 "
			"		THEN 'Adjustment' "
			"		ELSE 'Refund' "
			"		End "
			"	End,  "
			"LineItemT.Description,  "
			"[_UnappliedCreditsSubQ].Amount, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location, PaymentsT.ProviderID AS ProvID, CASE WHEN Type = 1 THEN CASE WHEN PrePayment = 1 THEN -20 ELSE -21 END ELSE Type END AS LineItemType "
			"FROM (SELECT [_UnappliedCreditsSubQ1].ID,  "
			"Min([_UnappliedCreditsSubQ1].[Amount])+Sum(CASE WHEN [AppliesT].[Amount] Is Null THEN 0 ELSE [AppliesT].[Amount] End) AS Amount,  "
			"[_UnappliedCreditsSubQ1].PatientID "
			"FROM (SELECT PaymentsT.ID,  "
			"Min([LineItemT].[Amount])-Sum(CASE WHEN [AppliesT].[Amount] Is Null THEN 0 ELSE [AppliesT].[Amount] End) AS Amount,  "
			"LineItemT.PatientID "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"GROUP BY PaymentsT.ID, LineItemT.PatientID "
			"HAVING (((Min([LineItemT].[Amount])-Sum(CASE WHEN [AppliesT].[Amount] Is Null THEN 0 ELSE [AppliesT].[Amount] End))<>0)) "
			") AS _UnappliedCreditsSubQ1 LEFT JOIN AppliesT ON [_UnappliedCreditsSubQ1].ID = AppliesT.DestID "
			"GROUP BY [_UnappliedCreditsSubQ1].ID, [_UnappliedCreditsSubQ1].PatientID "
			"HAVING (((Min([_UnappliedCreditsSubQ1].[Amount])+Sum(CASE WHEN [AppliesT].[Amount] Is Null THEN 0 ELSE [AppliesT].[Amount] End))<>0)) "
			") AS _UnappliedCreditsSubQ LEFT JOIN ((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) RIGHT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) ON [_UnappliedCreditsSubQ].ID = PaymentsT.ID "
			"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"WHERE ((LineItemT.Deleted)=0) "
			"");
		break;

	case 152:
		//Patient Balances By Provider
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"Payments = CASE   "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [_PatPaysQ].[SumOfAmount]   "
			"	End,    "
			"Charges = CASE   "
			"	WHEN [ChargeAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [ChargeAmount]   "
			"	End, "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS DocName,    "
			"PatientsT.PersonID AS PatID,    "
			"PatientsT.UserDefinedID,   "
			"(CASE   "
			"	WHEN [ChargeAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [ChargeAmount]   "
			"	End)-   "
			"(CASE   "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [_PatPaysQ].[SumOfAmount]   "
			"	End)+   "
			"(CASE   "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]   "
			"	End) AS AccountBal,    "
			"ProvidersT.PersonID AS ProvID,    "
			"PrePayments = CASE   "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]   "
			"	End,   "
			"PersonT.Location AS LocID,   "
			"LocationsT.Name AS Location   "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN  "
			"/* Pat PaysQ */ "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"Sum(LineItemT.Amount) AS SumOfAmount,    "
			"PatientsT.PersonID AS ID   "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID   "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3)) AND (PaymentsT.InsuredPartyID < 0) "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID   "
			") AS _PatPaysQ  "
			"/* End Pat PaysQ */ "
			"ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN  "
			"/* Pat ChargesQ */ "
			"(SELECT LineItemT.PatientID,   "
			"Sum(ChargeRespT.Amount) AS ChargeAmount, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName   "
			"FROM ((LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number) "
			"INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID   "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0) AND (ChargeRespT.InsuredPartyID Is Null))   "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle   "
			") AS _PatChargesQ  "
			"/* End Pat ChargesQ */ "
			"ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN   "
			"  "
			"/* Prepays */  "
			"(SELECT FullName, Sum(Amount) AS SumOfAmount, ID   "
			"FROM  "
			"(   "
			"SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN (LineItemT.Amount) ELSE (LineItemT.Amount - PrepayAppliesQ.Amount) END)   "
			"ELSE   "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) AS Amount,    "
			"PatientsT.PersonID AS ID   "
			"FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)   "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID))   "
			"LEFT JOIN   "
			"/* This will total everything applied to a prepayment */   "
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID   "
			"FROM   "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID   "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID   "
			"WHERE (LineItemT.Deleted = 0) "
			"GROUP BY AppliesT.DestID   "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID   "
			"LEFT JOIN   "
			"/* This will total everything that the prepayment is applied to */   "
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID   "
			"FROM   "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID   "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID   "
			"WHERE LineItemT.Deleted = 0   "
			"GROUP BY AppliesT.SourceID   "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID   "
			"WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1) AND (PersonT.ID > 0) AND (PaymentsT.InsuredPartyID < 0) "
			"AND (((LineItemT.Type)<4))   "
			") AS PrepaySubQ  "
			"GROUP BY FullName, ID)  "
			" AS _PatPrePaysQ  "
			"/* End Prepays */  "
			" ON PatientsT.PersonID = [_PatPrePaysQ].ID  "
			"WHERE PatientsT.PersonID <> -25 AND PatientsT.CurrentStatus <> 4 ");
		break;
	case 603:
		// Financial Activity - Today's Service Date
		// (d.moore 2007-07-25) - PLID 22487 - Same content as 'Financial Activity - Daily' but the report is
		//  limited to just the current date.
	case 580:
		//Daily Sales (By Charge Category)
	case 153:
		//Financial Activity - Daily
	case 154:
		//Financial Activity - Monthly
	case 155:
		//Financial Activity - Yearly
	case 624:
		//Daily User Activity
	case 660:
		// (z.manning 2009-04-02 12:35) - PLID 33170 - Daily User Activity - Detailed
	case 719:
		// (j.dinatale 2011-11-08 11:12) - PLID 45658 - Financial Activity - Daily for Logged in user
		return GetSqlFinancialActivity(nSubLevel, nSubRepNum);
		break;
	case 156:
		//Performance Graph
		/*Version History
		TES 9/8/2004 - PLID 14021 - Filtered out gift certificates
		(r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT GraphSubQ.Amt, GraphSubQ.ProvID AS ProvID, GraphSubQ.TDate AS TDate, "
			"GraphSubQ.TransType, GraphSubQ.ID, GraphSubQ.LocID AS LocID, GraphSubQ.Location "
			"FROM ("
			"SELECT * FROM (SELECT LineItemT.Amount AS Amt,  "
			"PaymentsT.ProviderID AS ProvID,  "
			"LineItemT.Date AS TDate,  "
			"TransType = CASE "
			"	WHEN [Type]=2 "
			"	THEN 'Adjustment' "
			"	ELSE CASE "
			"		WHEN [Type]=3 "
			"		THEN 'Refund' "
			"		ELSE 'Payment' "
			"		End "
			"	End,  "
			"LineItemT.ID, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location "
			"FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE (((LineItemT.Deleted)=0)) AND PaymentsT.PayMethod NOT IN (4,10) "
			") AS PerformancePaymentsQ "
			"UNION SELECT * FROM (SELECT Sum(dbo.GetChargeTotal(LineItemT.ID)) AS Amt,  "
			"ProvID = Min(CASE "
			"	WHEN [DoctorsProviders] Is Null "
			"	THEN -1 "
			"	ELSE [DoctorsProviders] "
			"	End), "
			"LineItemT.Date AS TDate,  "
			"'Charges' AS Transtype,  "
			"LineItemT.ID, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location "
			"FROM (ChargesT INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
			"WHERE ((LineItemT.Deleted)=0)  AND ChargesT.ServiceID NOT IN (SELECT ServiceID FROM GCTypesT) "
			"GROUP BY LineItemT.Date, LineItemT.ID, LineItemT.Type, LineItemT.LocationID, LocationsT.Name "
			"HAVING (((LineItemT.Type)=10)) "
			") AS PerformanceGraphChargesSubQ "
			") AS GraphSubQ");
		break;
	case 157:
		//Performance Analysis
		/*Version History
		TES 9/8/2004 - PLID 14021 - Filtered out gift certificates
		(r.gonet 2015-05-05 14:38) - PLID 66302 - Exclude Gift Certificate Refunds
		*/
		return _T("SELECT PerfAdjSubQ.PatCharges, "
			"PerfAdjSubQ.InsCharges, "
			"PerfAdjSubQ.PatPayments, "
			"PerfAdjSubQ.InsPayments, "
			"PerfAdjSubQ.PatAdj, "
			"PerfAdjSubQ.InsAdj, "
			"PerfAdjSubQ.ProvID AS ProvID, "
			"PerfAdjSubQ.TDate AS TDate, "
			"PerfAdjSubQ.ID, "
			"PerfAdjSubQ.LocID AS LocID, "
			"PerfAdjSubQ.Location "
			"FROM (SELECT Sum(CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN ChargeRespT.Amount ELSE 0 End) AS PatCharges,  "
			"Sum (CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN 0 ELSE ChargeRespT.Amount End) AS InsCharges,  "
			"0 AS PatPayments,  "
			"0 AS InsPayments,  "
			"0 AS PatAdj,  "
			"0 AS InsAdj,  "
			"ChargesT.DoctorsProviders AS ProvID,  "
			"BillsT.Date AS TDate,  "
			"LineItemT.ID, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location "
			"FROM (((ChargesT LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID) INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON ChargesT.ID = LineItemT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number) LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0) AND ChargesT.ServiceID NOT IN (SELECT ServiceID FROM GCTypesT)) "
			"GROUP BY ChargesT.DoctorsProviders, BillsT.Date, LineItemT.ID, LineItemT.LocationID, LocationsT.Name "
			" "
			"UNION "
			"select ALL * from (SELECT 0 AS PatCharges,  "
			"0 AS InsCharges,  "
			"PatPayments = CASE "
			"	WHEN [InsuredPartyID]=-1 "
			"	THEN [Amount] "
			"	ELSE 0 "
			"	End,  "
			"InsPayments = CASE "
			"	WHEN [InsuredPartyID]=-1 "
			"	THEN 0 "
			"	ELSE [Amount] "
			"	End, "
			"0 AS PatAdj,  "
			"0 AS InsAdj,  "
			"PaymentsT.ProviderID AS ProvID,  "
			"LineItemT.Date AS TDate,  "
			"LineItemT.ID, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location "
			"FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE (((LineItemT.Type)=1 Or (LineItemT.Type)=3) AND ((LineItemT.Deleted)=0) AND PaymentsT.PayMethod NOT IN (4,10)) "
			") AS _PerformancePaymentsAnalysisQ "
			"UNION select ALL * from (SELECT 0 AS PatCharges,  "
			"0 AS InsCharges,  "
			"0 AS PatPayments,  "
			"0 AS InsPayments,  "
			"PatAdj = CASE "
			"	WHEN [InsuredPartyID]=-1 "
			"	THEN [Amount] "
			"	ELSE 0 "
			"	End,  "
			"InsAdj = CASE "
			"	WHEN [InsuredPartyID]=-1 "
			"	THEN 0 "
			"	ELSE [Amount] "
			"	End,  "
			"PaymentsT.ProviderID AS ProvID,  "
			"LineItemT.Date AS TDate,  "
			"LineItemT.ID, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location "
			"FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE (((LineItemT.Type)=2) AND ((LineItemT.Deleted)=0)) "
			") AS _PerformanceAdjustmentsAnalysisQ) AS PerfAdjSubQ");
		break;

	case 186:
		//Patient Balances by Patient Type
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,    "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"GroupTypes.GroupName,    "
			"ProvidersT.PersonID AS ProvID,    "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS ProvName,    "
			"PersonT.FirstContactDate AS Date,    "
			"GroupTypes.TypeIndex AS TypeIndex,    "
			"[_PatBalQ].AccountBal AS AccountBal,    "
			"[_PatBalbyTypeSubQ].MaxOfDate,    "
			"LineItemT.Amount,    "
			"[_PatBalbyTypeSubQ].PayID,   "
			"PersonT.Location AS LocID,   "
			"LocationsT.Name AS Location   "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT.ID) LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"Payments = CASE   "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [_PatPaysQ].[SumOfAmount]   "
			"	End,    "
			"Charges = CASE   "
			"	WHEN [ChargeAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [ChargeAmount]   "
			"	End,    "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,    "
			"PatientsT.PersonID AS ID,    "
			"(CASE   "
			"	WHEN [ChargeAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [ChargeAmount]   "
			"	End)-   "
			"(CASE   "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [_PatPaysQ].[SumOfAmount]   "
			"	End)+   "
			"(CASE   "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]   "
			"	End) AS AccountBal,    "
			"ProvidersT.PersonID AS ProvID,    "
			"PrePayments = CASE   "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null   "
			"	THEN 0   "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]   "
			"	End   "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN  "
			"/* PatPaysQ*/ "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"Sum(LineItemT.Amount) AS SumOfAmount,     "
			"PatientsT.PersonID AS ID    "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3)) AND (PaymentsT.InsuredPartyID < 0)  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID    "
			") AS _PatPaysQ "
			"/* End PatPaysQ */ "
			" ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN  "
			"/* PatChargesQ */ "
			"(SELECT LineItemT.PatientID,    "
			"Sum(ChargeRespT.Amount) AS ChargeAmount,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName    "
			"FROM ((LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number)  "
			"INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0) AND (ChargeRespT.InsuredPartyID Is Null))    "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle    "
			") AS _PatChargesQ  "
			"/* PatChargesQ */ "
			"ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN    "
			"/* Prepays */  "
			"(SELECT FullName, Sum(Amount) AS SumOfAmount, ID    "
			"FROM   "
			"(    "
			"SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN     "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN (LineItemT.Amount) ELSE (LineItemT.Amount - PrepayAppliesQ.Amount) END)    "
			"ELSE    "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) AS Amount,     "
			"PatientsT.PersonID AS ID    "
			"FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)    "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID))    "
			"LEFT JOIN    "
			"/* This will total everything applied to a prepayment */    "
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID    "
			"FROM    "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID    "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID    "
			"WHERE (LineItemT.Deleted = 0)  "
			"GROUP BY AppliesT.DestID    "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID    "
			"LEFT JOIN    "
			"/* This will total everything that the prepayment is applied to */    "
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID    "
			"FROM    "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID    "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID    "
			"WHERE LineItemT.Deleted = 0    "
			"GROUP BY AppliesT.SourceID    "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID    "
			"WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1) AND (PersonT.ID > 0) AND (PaymentsT.InsuredPartyID < 0)  "
			"AND (((LineItemT.Type)<4))    "
			") AS PrepaySubQ  "
			"GROUP BY FullName, ID)  "
			" AS _PatPrePaysQ  "
			"/* End Prepays */  "
			"ON PatientsT.PersonID = [_PatPrePaysQ].ID   "
			") AS _PatBalQ ON PatientsT.PersonID = [_PatBalQ].ID) LEFT JOIN ((SELECT PatientsT.PersonID AS PatID,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
			"[_PatBalbyTypeSubQ2].MaxOfDate,    "
			"Max(LineItemT.ID) AS PayID   "
			"FROM ((PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) LEFT JOIN (SELECT PatientsT.PersonID AS PatID,    "
			"Max(LineItemT.Date) AS MaxOfDate   "
			"FROM PatientsT LEFT JOIN LineItemT ON PatientsT.PersonID = LineItemT.PatientID   "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3 Or (LineItemT.Type) Is Null))   "
			"GROUP BY PatientsT.PersonID   "
			"HAVING (((PatientsT.PersonID)>0))   "
			") AS _PatBalbyTypeSubQ2 ON PatientsT.PersonID = [_PatBalbyTypeSubQ2].PatID) LEFT JOIN LineItemT ON [_PatBalbyTypeSubQ2].PatID = LineItemT.PatientID   "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3 Or (LineItemT.Type) Is Null))   "
			"GROUP BY PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, [_PatBalbyTypeSubQ2].MaxOfDate   "
			"HAVING (((PatientsT.PersonID)>0))   "
			") AS _PatBalbyTypeSubQ LEFT JOIN LineItemT ON [_PatBalbyTypeSubQ].PayID = LineItemT.ID) ON PatientsT.PersonID = [_PatBalbyTypeSubQ].PatID   "
			"GROUP BY PatientsT.UserDefinedID, PatientsT.CurrentStatus, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, GroupTypes.GroupName, ProvidersT.PersonID, PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle, PersonT.FirstContactDate, GroupTypes.TypeIndex, [_PatBalQ].AccountBal, [_PatBalbyTypeSubQ].MaxOfDate, LineItemT.Amount, [_PatBalbyTypeSubQ].PayID, PersonT.Location, LocationsT.Name   "
			"HAVING (((PatientsT.UserDefinedID)>0 AND PatientsT.CurrentStatus <> 4)) ");
		break;
	case 187:
		//Daily Batch Report  TODO--Update the queries for this.
		switch (nSubLevel) {
		case 1:
			switch (nSubRepNum) {
				//Insurance Pays
				/* Version History
				(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType
				(e.lally 2008-10-27) PLID 31817 - Added Apply source and destination IDs
				(e.lally 2008-11-14) PLID 32038 - Added charge "ItemCode" for consistency amongst subreports
				*/
			case 0:
				return _T("SELECT SubQ.PaymentID, SubQ.InputDate AS IDate, SubQ.ProvID AS ProvID, SubQ.TDate AS TDate, "
					"SubQ.PatID AS PatID, SubQ.UserDefinedID, SubQ.PatName, SubQ.Description, SubQ.ProvName, "
					"SubQ.PayAmount, SubQ.AppliedAmount, SubQ.CardName AS CCType, SubQ.CheckNo, SubQ.InputName AS UserName, SubQ.PayMethod, "
					"SubQ.InsuredPartyID, SubQ.InsCoName, SubQ.ChargeID, SubQ.ChargeDesc, SubQ.ChargeTotal, "
					"SubQ.ApplyDate, SubQ.LocID AS LocID, SubQ.Location, SubQ.SourceID, SubQ.DestID, SubQ.ItemCode "
					"FROM "
					"(SELECT PaymentsT.ID AS PaymentID,  "
					"LineItemT.InputDate,  "
					"ProvidersT.PersonID AS ProvID,  "
					"LineItemT.Date AS TDate,  "
					"LineItemT.PatientID AS PatID, "
					"PatientsT.UserDefinedID,  "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
					"LineItemT.Description,  "
					"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS ProvName,  "
					"LineItemT.Amount AS PayAmount,  "
					"Sum(AppliesT.Amount) AS AppliedAmount,  "
					"CreditCardNamesT.CardName,  "
					"PaymentPlansT.CheckNo,  "
					"LineItemT.InputName,  "
					"PaymentsT.PayMethod,  "
					"PaymentsT.InsuredPartyID,  "
					"InsuranceCoT.Name AS InsCoName,  "
					"ChargesT.ID AS ChargeID,  "
					"LineItemT_1.Description AS ChargeDesc,  "
					"dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal,  "
					"LineItemT_1.Date AS ApplyDate, "
					"LineItemT.LocationID AS LocID, "
					"LocationsT.Name AS Location, "
					"AppliesT.SourceID, "
					"AppliesT.DestID, "
					"ChargesT.ItemCode "
					""
					"FROM ((((((((((PaymentsT LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PaymentsT.ID = LineItemT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN (ProvidersT INNER JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) ON PaymentsT.ProviderID = ProvidersT.PersonID)) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID) LEFT JOIN InsuranceCot ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN LineItemT AS LineItemT_1 ON ChargesT.ID = LineItemT_1.ID "
					"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
					"WHERE (((LineItemT.Deleted)=0) AND ((AppliesT.PointsToPayments)=0)) "
					"GROUP BY PaymentsT.ID, LineItemT.InputDate, ProvidersT.PersonID, LineItemT.Date, LineItemT.PatientID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.Description, PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle, LineItemT.Amount, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, LineItemT.InputName, PaymentsT.PayMethod, PaymentsT.InsuredPartyID, InsuranceCoT.Name, ChargesT.ID, LineItemT_1.Description, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT.LocationID, LocationsT.Name, "
					"AppliesT.SourceID, AppliesT.DestID, ChargesT.ItemCode "
					"HAVING (((PaymentsT.PayMethod)=1 Or (PaymentsT.PayMethod)=2 Or (PaymentsT.PayMethod)=3) AND ((PaymentsT.InsuredPartyID)>0 And (PaymentsT.InsuredPartyID) Is Not Null)) "
					" "
					"UNION SELECT PaymentsT.ID AS PaymentID,  "
					"LineItemT.InputDate,  "
					"ProvidersT.PersonID,  "
					"LineItemT.Date AS TDate,  "
					"LineItemT.PatientID AS PatID,  "
					"PatientsT.UserDefinedID,  "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
					"LineItemT.Description,  "
					"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS ProvName,  "
					"LineItemT.Amount AS PayAmount,  "
					"Sum(AppliesT.Amount) AS AppliedAmount,  "
					"CreditCardNamesT.CardName,  "
					"PaymentPlansT.CheckNo,  "
					"LineItemT.InputName,  "
					"PaymentsT.PayMethod,  "
					"PaymentsT.InsuredPartyID,  "
					"InsuranceCoT.Name AS InsCoName,  "
					"PaymentsT.ID AS ChargeID,  "
					"LineItemT_1.Description AS ChargeDesc,  "
					"LineItemT_1.Amount AS ChargeTotal,  "
					"LineItemT_1.Date AS ApplyDate, "
					"LineItemT.LocationID AS LocID, "
					"LocationsT.Name AS Location, "
					"AppliesT.SourceID, "
					"AppliesT.DestID, "
					"NULL as ItemCode "
					"FROM ((((((((PaymentsT LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PaymentsT.ID = LineItemT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN (ProvidersT INNER JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) ON PaymentsT.ProviderID = ProvidersT.PersonID)) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID) LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN (PaymentsT AS PaymentsT_1 LEFT JOIN LineItemT AS LineItemT_1 ON PaymentsT_1.ID = LineItemT_1.ID) ON AppliesT.DestID = PaymentsT_1.ID "
					"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
					"WHERE (((LineItemT.Deleted)=0) AND ((AppliesT.PointsToPayments)=1)) "
					"GROUP BY PaymentsT.ID, LineItemT.InputDate, ProvidersT.PersonID, LineItemT.Date, LineItemT.PatientID, PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, LineItemT.Description, PersonT_1.Last, PersonT_1.First, PersonT_1.Middle, LineItemT.Amount, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, LineItemT.InputName, PaymentsT.PayMethod, PaymentsT.InsuredPartyID, InsuranceCoT.Name, PaymentsT.ID, LineItemT_1.Description, LineItemT_1.Amount, LineItemT_1.Date, LineItemT.LocationID, LocationsT.Name, "
					"AppliesT.SourceID, AppliesT.DestID "
					"HAVING (((PaymentsT.PayMethod)=1 Or (PaymentsT.PayMethod)=2 Or (PaymentsT.PayMethod)=3) AND ((PaymentsT.InsuredPartyID)>0 And (PaymentsT.InsuredPartyID) Is Not Null)) "
					") AS SubQ");
				break;
			case 1:
				//Unapplied Pays
				/* Rewritten 5/15/02 - DRT - This was horrible and still some junk modified from Access generation.  I rewrote it, and simplified it greatly.  Pretty much all we need to do is subtract the Payment amount minus the sum
				of everything it is applied to (AppliesT.Amount).
				TES 6/18/04 - The WHERE clause was calculating the balance as payment amount - apply amount, rather than
				payment amount - sum(apply amount).  I fixed it.
				(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType
				*/
				return _T("SELECT "
					"PaymentsT.ID, LineItemT.InputDate AS IDate /* payment input date */, PaymentsT.ProviderID AS ProvID, LineItemT.Date AS TDate /* payment date */, LineItemT.PatientID AS PatID, "
					"PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, (CASE WHEN LineItemT.Type = 1 THEN 'Payment - ' WHEN LineItemT.Type = 2 THEN 'Adjustment - ' WHEN LineItemT.Type = 3 THEN 'Refund - ' END) + LineItemT.Description AS PayDesc, "
					"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName, "
					"(LineItemT.Amount - Sum(CASE WHEN AppliesT.Amount IS NULL THEN 0 ELSE AppliesT.Amount END)) AS PayAmount, "
					"CreditCardNamesT.CardName AS CCType, PaymentPlansT.CheckNo, LineItemT.InputName AS UserName, PaymentsT.PayMethod, "
					"LineItemT.Type, LineItemT.InputDate AS IDate, PaymentsT.InsuredPartyID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location "
					"FROM "
					"LineItemT "
					"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
					"LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
					"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"LEFT JOIN PaymentPlanst ON PaymentsT.ID = PaymentPlansT.ID "
					"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
					"LEFT JOIN PersonT PersonT1 ON PaymentsT.ProviderID = PersonT1.ID "
					"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID  /* Payment! */ "
					"LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
					"LEFT JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
					"WHERE LineItemT.Type <= 3 AND LineItemT.Deleted = 0 "
					"GROUP BY PaymentsT.ID, LineItemT.InputDate/* payment input date */, PaymentsT.ProviderID, LineItemT.Date /* payment date */, LineItemT.PatientID, "
					"PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.Description, "
					"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, "
					"LineItemT.Amount, "
					"CreditCardNamesT.CardName, PaymentPlansT.CheckNo, LineItemT.InputName, PaymentsT.PayMethod, "
					"LineItemT.Type, LineItemT.InputDate, PaymentsT.InsuredPartyID, LineItemT.LocationID, LocationsT.Name "
					"HAVING LineItemT.Amount - (CASE WHEN Sum(AppliesT.Amount) IS NULL THEN 0 ELSE Sum(AppliesT.Amount) END) <> 0 ");
				break;
			case 2:
				//Applied Adjustments and Refunds
				/*	Version History
				DRT 8/10/2005 - PLID 16523 - Fixed the InputName field, which was showing the Inputting User of the destination when
				an adjustment or refund was applied.  It now always displays the user who input that specific item.
				(e.lally 2008-10-27) PLID 31817 - Added Apply source and destination IDs
				*/
				return _T("SELECT SubQ.PaymentID, SubQ.IDate AS IDate, SubQ.ProvID AS ProvID, SubQ.TDate AS TDate,  "
					"SubQ.PatID AS PatID, SubQ.UserDefinedID, SubQ.PatName, SubQ.Description, SubQ.ProvName,  "
					"SubQ.Amount, SubQ.PayMethod, SubQ.InputName AS UserName, SubQ.ApplyAmount, SubQ.Type, SubQ.ApplyDate,  "
					"SubQ.ChargeAmount, SubQ.ItemCode, SubQ.ID, SubQ.LocID AS LocID, SubQ.Location, SubQ.ApplyToDesc, "
					"SubQ.SourceID, SubQ.DestID "
					"FROM  "
					"(SELECT PaymentsT.ID AS PaymentID,   "
					"LineItemT.InputDate AS IDate,   "
					"CASE WHEN DoctorsProviders Is Null THEN ProviderID ELSE DoctorsProviders End AS ProvID,   "
					"LineItemT.Date AS TDate,   "
					"PatientsT.PersonID AS PatID,   "
					"PatientsT.UserDefinedID,  "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,   "
					"LineItemT.Description,   "
					"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,   "
					"LineItemT.Amount,   "
					"PaymentsT.PayMethod,   "
					"LineItemT.InputName AS InputName,   "
					"AppliesT.Amount AS ApplyAmount,   "
					"LineItemT.Type,   "
					"LineItemT_2.Date AS ApplyDate,   "
					"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,   "
					"ChargesT.ItemCode,   "
					"AppliesT.ID,  "
					"LineItemT.LocationID AS LocID,  "
					"LocationsT.Name AS Location, LineItemT_2.Description AS ApplyToDesc, "
					"AppliesT.SourceID, "
					"AppliesT.DestID "
					"FROM ((((((PaymentsT RIGHT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN  "
					"LineItemT LineItemT_2 ON AppliesT.DestID = LineItemT_2.ID LEFT JOIN (ChargesT LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) ON LineItemT_2.ID = ChargesT.ID) "
					"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN (LineItemT  "
					"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PaymentsT.ID = LineItemT.ID)  "
					"LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  "
					"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID)  "
					"WHERE (((AppliesT.PointsToPayments)=0) AND LineItemT.Deleted = 0)  "
					"GROUP BY LineItemT_2.Description, PaymentsT.ID, LineItemT.InputDate, CASE WHEN DoctorsProviders Is Null THEN ProviderID ELSE DoctorsProviders End, LineItemT.Date, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.Description, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, LineItemT.Amount, PaymentsT.PayMethod, LineItemT.InputName, AppliesT.Amount, LineItemT.Type, LineItemT_2.Date, dbo.GetChargeTotal(ChargesT.ID), ChargesT.ItemCode, AppliesT.ID, LineItemT.LocationID, LocationsT.Name,  "
					"AppliesT.SourceID, AppliesT.DestID "
					"HAVING (((AppliesT.Amount) Is Not Null) AND ((LineItemT.Type)=2 Or (LineItemT.Type)=3))  "
					"  "
					"UNION SELECT  PaymentsT.ID AS PaymentID,   "
					"LineItemT.InputDate AS IDate,   "
					"CASE WHEN DoctorsProviders Is Null THEN ProviderID ELSE DoctorsProviders End AS ProvID,   "
					"LineItemT.Date AS TDate,   "
					"PatientsT.PersonID AS PatID,  "
					"PatientsT.UserDefinedID,   "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,   "
					"LineItemT.Description,   "
					"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName,   "
					"LineItemT.Amount,   "
					"PaymentsT.PayMethod,   "
					"LineItemT.InputName, "
					"AppliesT.Amount AS ApplyAmount,   "
					"LineItemT.Type,   "
					"LineItemT_2.Date AS ApplyDate,   "
					"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,   "
					"ChargesT.ItemCode,   "
					"AppliesT.ID,  "
					"LineItemT.LocationID AS LocID,  "
					"LocationsT.Name AS Location, LineItemT_2.Description AS ApplyToDesc,  "
					"AppliesT.SourceID, "
					"AppliesT.DestID "
					" "
					"FROM ((((((PaymentsT RIGHT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)  "
					"LEFT JOIN LineItemT LineItemT_2 ON AppliesT.DestID = LineItemT_2.ID LEFT JOIN (ChargesT  "
					"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) ON AppliesT.DestID = ChargesT.ID)  "
					"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN (LineItemT  "
					"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PaymentsT.ID = LineItemT.ID)  "
					"LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) "
					"LEFT JOIN PersonT PersonT1 ON ProvidersT.PersonID = PersonT1.ID)  "
					" "
					"WHERE (((AppliesT.PointsToPayments)=1) AND LineItemT.Deleted = 0)  "
					"GROUP BY LineItemT_2.Description, PaymentsT.ID, LineItemT.InputDate, CASE WHEN DoctorsProviders Is Null THEN ProviderID ELSE DoctorsProviders End, LineItemT.Date, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.Description, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, LineItemT.Amount, PaymentsT.PayMethod, LineItemT.InputName, AppliesT.Amount, LineItemT.Type, LineItemT_2.Date, dbo.GetChargeTotal(ChargesT.ID), ChargesT.ItemCode, AppliesT.ID, LineItemT.LocationID, LocationsT.Name,  "
					"AppliesT.SourceID, AppliesT.DestID "
					"HAVING (((AppliesT.Amount) Is Not Null) AND ((LineItemT.Type)=2 Or (LineItemT.Type)=3))  "
					") AS SubQ ");
				break;
			case 3:
				//Charges
				// (j.gruber 2008-02-19 13:28) - PLID 28940 - Changed the LocID to be the LineItem LocationID, not the POS
				return _T("SELECT BillsT.ID AS BillID,  "
					"ChargesT.ID AS ChargeID,  "
					"UsersT.UserName AS BillLogin,  "
					"ProvidersT.PersonID AS ProvID,  "
					"LineItemT.InputName AS UserName,  "
					"BillsT.Date AS BillDate,  "
					"LineItemT.Date AS TDate,  "
					"BillsT.PatientID AS PatID, "
					"PatientsT.UserDefinedID AS PatientID,  "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
					"ChargesT.ItemCode,  "
					"BillsT.Description AS BillNote,  "
					"LineItemT.Description,  "
					"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS ProvName,  "
					"ChargesT.Quantity,  "
					"Sum(dbo.GetChargeTotal(ChargesT.ID)) AS LineItemCost,  "
					"LineItemT.InputDate AS IDate,  "
					"LineItemT.InputDate AS ChargeInputDate,  "
					"BillsT.EntryType, "
					"BillsT.Location AS POSID, "
					"POST.Name AS POSName, "
					"LineItemT.LocationID AS LocID, "
					"LocationsT.Name as LocationName "
					"FROM ((((((BillsT LEFT JOIN LocationsT POST ON BillsT.Location = POST.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON BillsT.PatientID = PatientsT.PersonID) LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID) LEFT JOIN (ProvidersT INNER JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifiert.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number)) LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN UsersT ON BillsT.InputName = UsersT.PersonID "
					" LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
					"WHERE (((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) "
					"GROUP BY BillsT.ID, ChargesT.ID, UsersT.UserName, ProvidersT.PersonID, LineItemT.InputName, BillsT.Date, LineItemT.Date, BillsT.PatientID, PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, ChargesT.ItemCode, BillsT.Description, LineItemT.Description, PersonT_1.Last, PersonT_1.First, PersonT_1.Middle, ChargesT.Quantity, BillsT.InputDate, LineItemT.InputDate, BillsT.EntryType, BillsT.Location, POST.Name, LineItemT.LocationID, LocationsT.Name "
					"HAVING (((BillsT.EntryType)=1)) "
					"");
				break;
			default:
				return _T("");
				break;
			}
			break;
		default:
			//Main report query
			/*Version History
			(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType
			(e.lally 2008-10-27) PLID 31817 - Added Apply source and destination IDs
			*/
			return _T("SELECT PaymentsT.ID,  "
				"LineItemT.InputDate AS IDate,  "
				"PaymentsT.ProviderID AS ProvID,  "
				"LineItemT.Date AS TDate,  "
				"LineItemT.PatientID AS PatID, "
				"PatientsT.UserDefinedID,  "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
				"LineItemT.Description AS Notes,  "
				"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS DocName,  "
				"LineItemT.Amount AS PayAmount,  "
				"CreditCardNamesT.CardName AS CCType,  "
				"PaymentPlansT.CheckNo,  "
				"LineItemT.InputName AS UserName,  "
				"PaymentsT.PayMethod,  "
				"LineItemT.Type,  "
				"LineItemT_1.Date,  "
				"LineItemT_1.Description AS ItemDesc,  "
				"ChargesT.ItemCode,  "
				"dbo.GetChargeTotal(ChargesT.ID) AS LineTotal,  "
				"LineItemT_1.Amount AS ChargeAmount,  "
				"LineItemT_1.Date AS ApplyDate,  "
				"AppliesT.Amount AS ApplyAmount, "
				"LineItemT.LocationID AS LocID, "
				"LocationsT.Name AS Location, "
				"AppliesT.SourceID, "
				"AppliesT.DestID "
				"FROM ((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) RIGHT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID) LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID) LEFT JOIN LineItemT AS LineItemT_1 ON ChargesT.ID = LineItemT_1.ID LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
				"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				"WHERE (((LineItemT.Type)=1) AND ((ChargesT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((PaymentsT.InsuredPartyID)<=0)) "
				"");
			break;
		}
		break;

	case 197:
	{
		//Printed Superbills
		/*	Version History
		DRT 6/19/03 - Removed references to AptPurposeID, which is obsolete.
		TES 3/4/04 - Removed references to AppointmentsT.ResourceID, which is also obsolete.
		TES 8/23/05 - Changed Resource filtering.
		*/
		CString sql = "SELECT PrintedSuperBillsT.PrintedOn AS TDate, AppointmentsT.Date AS ApptDate, AppointmentsT.Notes,  "
			"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, AptTypeT.Name AS PurposeSet,  "
			"    AppointmentsT.StartTime AS StartTime,  "
			"    AppointmentsT.EndTime AS EndTime,  "
			"    dbo.GetResourceString(AppointmentsT.ID) AS Resource, "
			"-1 AS ResourceID, "
			"    PrintedSuperBillsT.SavedID AS SuperBillID,  "
			"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
			"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"AppointmentsT.LocationID AS LocID, "
			"LocationsT.Name AS Location "
			"FROM (AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID) LEFT OUTER JOIN "
			"    AptTypeT ON "
			"    AppointmentsT.AptTypeID = AptTypeT.ID    "
			"    RIGHT OUTER JOIN PrintedSuperBillsT ON  "
			"    AppointmentsT.ID = PrintedSuperBillsT.ReservationID LEFT OUTER "
			"     JOIN "
			"    PatientsT LEFT OUTER JOIN "
			"    PersonT ON PatientsT.PersonID = PersonT.ID ON  "
			"    PrintedSuperBillsT.PatientID = PatientsT.PersonID "
			"WHERE (AppointmentsT.Status <> 4 Or AppointmentsT.Status Is Null)";

		return _T(sql);
	}
	break;


	case 203:
	{
		//Capitation
		/*	Version History
		// (b.cardillo 2015-11-30 15:18) - PLID 67656 - Account for charge date relative to fee schedule effective date
		*/
		CString sql = _T(
			"SELECT (SELECT PatientsT.UserDefinedID FROM PatientsT WHERE PatientsT.PersonID = LineItemT.PatientID) AS UserDefinedID, "
			"   LineItemT.PatientID AS PatID,  "
			"   ChargeRespT.Amount AS LineAmount,  "
			"   LineItemT.Date AS TDate, "
			"   LineItemT.InputDate AS IDate,  "
			"   ChargesT.ItemCode AS Code,  "
			"   ChargesT.ItemSubCode AS SubCode, "
			"   (SELECT MultiFeeItemsT.FeeGroupID FROM MultiFeeGroupsT INNER JOIN MultiFeeItemsT ON MultiFeeGroupsT.ID = MultiFeeItemsT.FeeGroupID INNER JOIN MultiFeeLocationsT ON MultiFeeItemsT.FeeGroupID = MultiFeeLocationsT.FeeGroupID WHERE (MultiFeeGroupsT.EffectiveFromDate IS NULL OR (LineItemT.Date >= MultiFeeGroupsT.EffectiveFromDate AND (MultiFeeGroupsT.EffectiveToDate IS NULL OR LineItemT.Date <= MultiFeeGroupsT.EffectiveToDate))) AND MultiFeeItemsT.ServiceID = ChargesT.ServiceID AND MultiFeeItemsT.FeeGroupID IN (SELECT FeeGroupID FROM MultiFeeInsuranceT WHERE InsuranceCoID = (SELECT InsuredPartyT.InsuranceCoID FROM InsuredPartyT WHERE InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID)) AND MultiFeeItemsT.FeeGroupID IN (SELECT FeeGroupID FROM MultiFeeProvidersT WHERE ProviderID = ChargesT.DoctorsProviders) @LocID GROUP BY MultiFeeItemsT.FeeGroupID) AS FeeGroupID,  "
			"   (SELECT MultiFeeGroupsT.Name FROM MultiFeeGroupsT INNER JOIN MultiFeeItemsT ON MultiFeeGroupsT.ID = MultiFeeItemsT.FeeGroupID INNER JOIN MultiFeeLocationsT ON MultiFeeItemsT.FeeGroupID = MultiFeeLocationsT.FeeGroupID WHERE (MultiFeeGroupsT.EffectiveFromDate IS NULL OR (LineItemT.Date >= MultiFeeGroupsT.EffectiveFromDate AND (MultiFeeGroupsT.EffectiveToDate IS NULL OR LineItemT.Date <= MultiFeeGroupsT.EffectiveToDate))) AND MultiFeeItemsT.ServiceID = ChargesT.ServiceID AND MultiFeeItemsT.FeeGroupID IN (SELECT FeeGroupID FROM MultiFeeInsuranceT WHERE InsuranceCoID = (SELECT InsuredPartyT.InsuranceCoID FROM InsuredPartyT WHERE InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID)) AND MultiFeeItemsT.FeeGroupID IN (SELECT FeeGroupID FROM MultiFeeProvidersT WHERE ProviderID = ChargesT.DoctorsProviders) @LocID GROUP BY MultiFeeGroupsT.NAME) AS FeeGroupName,  "
			"   (SELECT ServiceT.Price FROM ServiceT WHERE ServiceT.ID = ChargesT.ServiceID) AS StandardFee, "
			"   convert(money, (SELECT ServiceT.Price FROM ServiceT WHERE ServiceT.ID = ChargesT.ServiceID) * ChargesT.Quantity * (ChargesT.TaxRate+ChargesT.TaxRate2-1)*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)) AS StandardTotal, "
			"   ChargesT.Quantity,  "
			"   ChargesT.TaxRate,  "
			"   ChargesT.TaxRate2,  "
			"   (SELECT InsuredPartyT.InsuranceCoID FROM InsuredPartyT WHERE InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID) AS InsCoID,  "
			"   (SELECT InsuranceCoT.Name FROM InsuranceCoT WHERE InsuranceCoT.PersonID = (SELECT (SELECT InsuredPartyT.InsuranceCoID FROM InsuredPartyT WHERE InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID))) As InsCo,  "
			"   ChargesT.DoctorsProviders AS ProvID,  "
			"   (SELECT ServiceT.Name FROM ServiceT WHERE ServiceT.ID = ChargesT.ServiceID) AS ItemDesc,  "
			"   ChargesT.ServiceID AS CPTID,  "
			"   (SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE PersonT.ID = LineItemT.PatientID) AS PatName,  "
			"   (SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE PersonT.ID = ChargesT.DoctorsProviders) AS DocName,  "
			"   LineItemT.LocationID AS LocID, "
			"   (SELECT LocationsT.Name FROM LocationsT WHERE LocationsT.ID = LineItemT.LocationID) AS Location "
			"FROM ChargeRespT INNER JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			);

		// (r.farnworth 2014-05-12 09:18) - PLID 45308 - Error from report Capitation.
		CString strLocation;
		//CString strLocJoin;
		if (nLocation == -2) {
			strLocation = "AND MultiFeeLocationsT.LocationID IS NULL ";
		}
		else if (nLocation == -3) {
			strLocation.Format("AND MultiFeeLocationsT.LocationID IN (");
			CString strPart;
			for (int i = 0; i < m_dwLocations.GetSize(); i++) {
				strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
				strLocation += strPart;
			}
			strLocation = strLocation.Left(strLocation.GetLength() - 2) + ")";
		}
		else if (nLocation != -1) {
			strLocation.Format("AND MultiFeeLocationsT.LocationID = %i", nLocation);
		}
		sql.Replace("@LocID", strLocation);

		return sql;

		break;
	}


	/*		I don't know why there are 2 versions of this report! (case 252)
	case 262:
	//Insurance Procedures
	return _T("SELECT (CASE WHEN LineItemT.Type = 1 THEN LineItemT.Amount ELSE 0 END) AS PayAmt,  "
	"(CASE WHEN LineItemT.Type = 2 THEN LineItemT.Amount ELSE 0 END) AS AdjAmt,  "
	"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, ProcedureT.Name AS ProcName, LineItemT1.Date AS TDate,  "
	"LineItemT1.InputDate AS IDate, LineItemT.PatientID AS PatID, LineItemT1.LocationID AS LocID, ChargesT.DoctorsProviders AS ProvID, ProcedureT.ID AS ProcID  "
	"FROM "
	"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
	"	LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
	"	LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
	"	INNER JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
	"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
	"	LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
	"	LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
	"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID"
	" "
	"WHERE (AppliesT.SourceID Is Not Null) AND (PaymentsT.InsuredPartyID > 0) AND (LineItemT.Deleted = 0) AND (ProcedureT.ID Is Not Null)");
	break;
	*/

	case 265:
		//Account Balances By Provider
		// (j.gruber 2009-03-25 16:45) - PLID 33359 - updated discount structure
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		// (r.goldschmidt 2016-02-08 13:12) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"Payments = CASE  "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPaysQ].[SumOfAmount]  "
			"	End,   "
			"Charges = CASE  "
			"	WHEN [ChargeAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [ChargeAmount]  "
			"	End,   "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS DocName,   "
			"PatientsT.PersonID AS PatID,   "
			"PatientsT.UserDefinedID,  "
			"(CASE  "
			"	WHEN [ChargeAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [ChargeAmount]  "
			"	End)-  "
			"(CASE  "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPaysQ].[SumOfAmount]  "
			"	End)+  "
			"(CASE  "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]  "
			"	End) AS AccountBal,   "
			"ProvidersT.PersonID AS ProvID,   "
			"PrePayments = CASE  "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]  "
			"	End,  "
			"PersonT.Location AS LocID,  "
			"LocationsT.Name AS Location  "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"Sum(LineItemT.Amount) AS SumOfAmount,   "
			"PatientsT.PersonID AS ID  "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID  "
			") AS _PatPaysQ ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN (SELECT LineItemT.PatientID,  "
			"Sum(ChargeRespT.Amount) AS ChargeAmount,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName  "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))  "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle  "
			") AS _PatChargesQ ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN  "
			" "
			"/* Prepays */ "
			"(SELECT FullName, Sum(Amount) AS SumOfAmount, ID  "
			"FROM "
			"(  "
			"SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN   "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN (LineItemT.Amount) ELSE (LineItemT.Amount - PrepayAppliesQ.Amount) END)  "
			"ELSE  "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) AS Amount,   "
			"PatientsT.PersonID AS ID  "
			"FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID))  "
			"LEFT JOIN  "
			"/* This will total everything applied to a prepayment */  "
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID  "
			"FROM  "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID  "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID  "
			"WHERE (LineItemT.Deleted = 0)  "
			"GROUP BY AppliesT.DestID  "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID  "
			"LEFT JOIN  "
			"/* This will total everything that the prepayment is applied to */  "
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID  "
			"FROM  "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID  "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID  "
			"WHERE LineItemT.Deleted = 0  "
			"GROUP BY AppliesT.SourceID  "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID  "
			"WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1) AND (PersonT.ID > 0) "
			"AND (((LineItemT.Type)<4))  "
			") AS PrepaySubQ "
			"GROUP BY FullName, ID) "
			" AS _PatPrePaysQ "
			"/* End Prepays */ "
			" ON PatientsT.PersonID = [_PatPrePaysQ].ID "
			"WHERE PatientsT.PersonID <> -25 AND PatientsT.CurrentStatus <> 4 ");
		break;

	case 266:
		//Account Balances by Patient Type
		// (j.gruber 2009-03-25 16:47) - PLID 33359 - updated discount structure
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		// (r.goldschmidt 2016-02-08 13:13) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"GroupTypes.GroupName,   "
			"ProvidersT.PersonID AS ProvID,   "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS ProvName,   "
			"PersonT.FirstContactDate AS Date,   "
			"GroupTypes.TypeIndex AS TypeIndex,   "
			"[_PatBalQ].AccountBal AS AccountBal,   "
			"[_PatBalbyTypeSubQ].MaxOfDate,   "
			"LineItemT.Amount,   "
			"[_PatBalbyTypeSubQ].PayID,  "
			"PersonT.Location AS LocID,  "
			"LocationsT.Name AS Location  "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT.ID) LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"Payments = CASE  "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPaysQ].[SumOfAmount]  "
			"	End,   "
			"Charges = CASE  "
			"	WHEN [ChargeAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [ChargeAmount]  "
			"	End,   "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,   "
			"PatientsT.PersonID AS ID,   "
			"(CASE  "
			"	WHEN [ChargeAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [ChargeAmount]  "
			"	End)-  "
			"(CASE  "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPaysQ].[SumOfAmount]  "
			"	End)+  "
			"(CASE  "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]  "
			"	End) AS AccountBal,   "
			"ProvidersT.PersonID AS ProvID,   "
			"PrePayments = CASE  "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]  "
			"	End  "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"Sum(LineItemT.Amount) AS SumOfAmount,   "
			"PatientsT.PersonID AS ID  "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID  "
			") AS _PatPaysQ ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN (SELECT LineItemT.PatientID,  "
			"Sum(ChargeRespT.Amount) AS ChargeAmount,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName  "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))  "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle  "
			") AS _PatChargesQ ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN   "
			"/* Prepays */ "
			"(SELECT FullName, Sum(Amount) AS SumOfAmount, ID  "
			"FROM "
			"(  "
			"SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN   "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN (LineItemT.Amount) ELSE (LineItemT.Amount - PrepayAppliesQ.Amount) END)  "
			"ELSE  "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) AS Amount,   "
			"PatientsT.PersonID AS ID  "
			"FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID))  "
			"LEFT JOIN  "
			"/* This will total everything applied to a prepayment */  "
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID  "
			"FROM  "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID  "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID  "
			"WHERE (LineItemT.Deleted = 0)  "
			"GROUP BY AppliesT.DestID  "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID  "
			"LEFT JOIN  "
			" "
			"/* This will total everything that the prepayment is applied to */  "
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID  "
			"FROM  "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID  "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID  "
			"WHERE LineItemT.Deleted = 0  "
			"GROUP BY AppliesT.SourceID  "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID  "
			" "
			"WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1) AND (PersonT.ID > 0) "
			"AND (((LineItemT.Type)<4))  "
			") AS PrepaySubQ "
			"GROUP BY FullName, ID) "
			" AS _PatPrePaysQ "
			"/* End Prepays */ "
			"ON PatientsT.PersonID = [_PatPrePaysQ].ID  "
			") AS _PatBalQ ON PatientsT.PersonID = [_PatBalQ].ID) LEFT JOIN ((SELECT PatientsT.PersonID AS PatID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"[_PatBalbyTypeSubQ2].MaxOfDate,   "
			"Max(LineItemT.ID) AS PayID  "
			"FROM ((PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) LEFT JOIN (SELECT PatientsT.PersonID AS PatID,   "
			"Max(LineItemT.Date) AS MaxOfDate  "
			"FROM PatientsT LEFT JOIN LineItemT ON PatientsT.PersonID = LineItemT.PatientID  "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3 Or (LineItemT.Type) Is Null))  "
			"GROUP BY PatientsT.PersonID  "
			"HAVING (((PatientsT.PersonID)>0))  "
			") AS _PatBalbyTypeSubQ2 ON PatientsT.PersonID = [_PatBalbyTypeSubQ2].PatID) LEFT JOIN LineItemT ON [_PatBalbyTypeSubQ2].PatID = LineItemT.PatientID  "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3 Or (LineItemT.Type) Is Null))  "
			"GROUP BY PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, [_PatBalbyTypeSubQ2].MaxOfDate  "
			"HAVING (((PatientsT.PersonID)>0))  "
			") AS _PatBalbyTypeSubQ LEFT JOIN LineItemT ON [_PatBalbyTypeSubQ].PayID = LineItemT.ID) ON PatientsT.PersonID = [_PatBalbyTypeSubQ].PatID  "
			"GROUP BY PatientsT.UserDefinedID, PatientsT.CurrentStatus, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, GroupTypes.GroupName, ProvidersT.PersonID, PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle, PersonT.FirstContactDate, GroupTypes.TypeIndex, [_PatBalQ].AccountBal, [_PatBalbyTypeSubQ].MaxOfDate, LineItemT.Amount, [_PatBalbyTypeSubQ].PayID, PersonT.Location, LocationsT.Name  "
			"HAVING (((PatientsT.UserDefinedID)>0 AND PatientsT.CurrentStatus <> 4))");
		break;

	case 458:
		//Account Balances By Provider (w/PrePays) 
		/*
		Version History
		-TES 10/30/03: Created
		// (j.gruber 2009-03-25 16:51) - PLID 33359 - update discount structure
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		// (r.goldschmidt 2016-02-08 14:43) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
		*/
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"Payments = CASE  "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPaysQ].[SumOfAmount]  "
			"	End,   "
			"Charges = CASE  "
			"	WHEN [ChargeAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [ChargeAmount]  "
			"	End,   "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS DocName,   "
			"PatientsT.PersonID AS PatID,   "
			"PatientsT.UserDefinedID,  "
			"(CASE  "
			"	WHEN [ChargeAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [ChargeAmount]  "
			"	End)-  "
			"(CASE  "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPaysQ].[SumOfAmount]  "
			"	End) AS AccountBal,   "
			"ProvidersT.PersonID AS ProvID,   "
			"PersonT.Location AS LocID,  "
			"LocationsT.Name AS Location  "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"Sum(LineItemT.Amount) AS SumOfAmount,   "
			"PatientsT.PersonID AS ID  "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID  "
			") AS _PatPaysQ ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN (SELECT LineItemT.PatientID,  "
			"Sum(ChargeRespT.Amount) AS ChargeAmount,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName  "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))  "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle  "
			") AS _PatChargesQ ON PatientsT.PersonID = [_PatChargesQ].PatientID) "
			"WHERE PatientsT.PersonID <> -25 AND PatientsT.CurrentStatus <> 4");
		break;

	case 459:
		//Account Balances by Patient Type (w/PrePays)
		/*
		Version History
		-TES 10/30/03: Created
		// (j.gruber 2009-03-25 16:53) - PLID 33359 - updated discount structure
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		// (r.goldschmidt 2016-02-08 14:44) - PLID 68058 - Financial reports that run dbo.GetChargeTotal will time out
		*/
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"GroupTypes.GroupName,   "
			"ProvidersT.PersonID AS ProvID,   "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS ProvName,   "
			"PersonT.FirstContactDate AS Date,   "
			"GroupTypes.TypeIndex AS TypeIndex,   "
			"[_PatBalQ].AccountBal AS AccountBal,   "
			"[_PatBalbyTypeSubQ].MaxOfDate,   "
			"LineItemT.Amount,   "
			"[_PatBalbyTypeSubQ].PayID,  "
			"PersonT.Location AS LocID,  "
			"LocationsT.Name AS Location  "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT.ID) LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"Payments = CASE  "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPaysQ].[SumOfAmount]  "
			"	End,   "
			"Charges = CASE  "
			"	WHEN [ChargeAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [ChargeAmount]  "
			"	End,   "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,   "
			"PatientsT.PersonID AS ID,   "
			"(CASE  "
			"	WHEN [ChargeAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [ChargeAmount]  "
			"	End)-  "
			"(CASE  "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
			"	THEN 0  "
			"	ELSE [_PatPaysQ].[SumOfAmount]  "
			"	End) AS AccountBal "
			"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID) LEFT JOIN (SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"Sum(LineItemT.Amount) AS SumOfAmount,   "
			"PatientsT.PersonID AS ID  "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID  "
			") AS _PatPaysQ ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN (SELECT LineItemT.PatientID,  "
			"Sum(ChargeRespT.Amount) AS ChargeAmount,   "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName  "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID  "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))  "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle  "
			") AS _PatChargesQ ON PatientsT.PersonID = [_PatChargesQ].PatientID) "
			") AS _PatBalQ ON PatientsT.PersonID = [_PatBalQ].ID) LEFT JOIN ((SELECT PatientsT.PersonID AS PatID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"[_PatBalbyTypeSubQ2].MaxOfDate,   "
			"Max(LineItemT.ID) AS PayID  "
			"FROM ((PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) LEFT JOIN (SELECT PatientsT.PersonID AS PatID,   "
			"Max(LineItemT.Date) AS MaxOfDate  "
			"FROM PatientsT LEFT JOIN LineItemT ON PatientsT.PersonID = LineItemT.PatientID  "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3 Or (LineItemT.Type) Is Null))  "
			"GROUP BY PatientsT.PersonID  "
			"HAVING (((PatientsT.PersonID)>0))  "
			") AS _PatBalbyTypeSubQ2 ON PatientsT.PersonID = [_PatBalbyTypeSubQ2].PatID) LEFT JOIN LineItemT ON [_PatBalbyTypeSubQ2].PatID = LineItemT.PatientID  "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3 Or (LineItemT.Type) Is Null))  "
			"GROUP BY PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, [_PatBalbyTypeSubQ2].MaxOfDate  "
			"HAVING (((PatientsT.PersonID)>0))  "
			") AS _PatBalbyTypeSubQ LEFT JOIN LineItemT ON [_PatBalbyTypeSubQ].PayID = LineItemT.ID) ON PatientsT.PersonID = [_PatBalbyTypeSubQ].PatID  "
			"GROUP BY PatientsT.UserDefinedID, PatientsT.CurrentStatus, PatientsT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, GroupTypes.GroupName, ProvidersT.PersonID, PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle, PersonT.FirstContactDate, GroupTypes.TypeIndex, [_PatBalQ].AccountBal, [_PatBalbyTypeSubQ].MaxOfDate, LineItemT.Amount, [_PatBalbyTypeSubQ].PayID, PersonT.Location, LocationsT.Name  "
			"HAVING (((PatientsT.UserDefinedID)>0 AND PatientsT.CurrentStatus <> 4))");
		break;

	case 268:
		//Inventory Sales
		/*
		Revision History:
		- TES 12/5/02: Changed the query to not include quotes (!)
		- TES 1/21/03: Changed the query to not filter out sales with no applies.
		- TES 1/27/04: Added an "AS PatID" so we could enable the "Use Filter" option.
		// (j.gruber 2009-03-25 16:54) - PLID 33359 - updated discount structure

		*/
		return _T("SELECT  "
			"LineItemT1.ID, LineItemT1.PatientID AS PatID, ServiceT.ID AS ItemID, ServiceT.Name AS ItemName,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
			//Tax amount is (everything else) * (taxrate - 1)
			"Round(convert(money, Convert(money,((LineItemT1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate-1)))),2) AS Tax1Amount, "
			"Round(convert(money, Convert(money,((LineItemT1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate2-1)))),2) AS Tax2Amount, "
			"Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS PayAmt,   "
			"Sum(CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS AdjAmt,   "
			"Sum(CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END) AS RefAmt,   "
			"LineItemT1.Date AS TDate, LineItemT1.InputDate AS IDate, ServiceT.Price AS UnitCost, LineItemT1.LocationID AS LocID, ChargesT.Quantity,  "
			"PatientsT.EmployeeID AS CoordID, CASE WHEN CoordPersonT.ID Is Null THEN '' ELSE CoordPersonT.Last + ', ' + CoordPersonT.First + ' ' + CoordPersonT.Middle END AS Coordinator, "
			"ChargesT.TaxRate, ChargesT.TaxRate2 "
			"FROM  "
			"ChargesT INNER JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
			"LEFT JOIN PatientsT ON LineItemT1.PatientID = PatientsT.PersonID "
			"LEFT JOIN PersonT CoordPersonT ON PatientsT.EmployeeID = CoordPersonT.ID "
			"	INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID  "
			"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
			"	LEFT JOIN AppliesT ON LineItemT1.ID = AppliesT.DestID "
			"	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
			"	LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
			"WHERE LineItemT1.Deleted = 0 AND LineItemT1.Type = 10 AND (LineItemT.Deleted Is Null OR LineItemT.Deleted = 0)"
			"GROUP BY  "
			"LineItemT1.ID, ChargesT.ID, LineItemT1.PatientID, ServiceT.ID, ServiceT.Name,  "
			"LineItemT1.Amount, Quantity, ChargesT.TaxRate, ChargesT.TaxRate2, CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4,  "
			"LineItemT1.Date, LineItemT1.InputDate, ServiceT.Price, LineItemT1.LocationID, ChargesT.Quantity, PatientsT.EmployeeID, CoordPersonT.ID, CoordPersonT.First, CoordPersonT.Middle, CoordPersonT.Last, ChargesT.TaxRate, ChargesT.TaxRate2");
		break;

	case 269:
		//Inventory Sales by Provider
		/*
		Revision History:
		- TES 12/5/02: Changed the query to not include quotes (!)
		- TES 1/21/03: Changed the query to not filter out sales with no applies.
		- TES 1/27/04: Added an "AS PatID" so we could enable the "Use Filter" option.
		// (j.gruber 2009-03-25 16:58) - PLID 33359 - updated discount structure
		// (b.savon 2012-06-06 10:26) - PLID 50755 - Filter out deleted bills and handle financial corrections
		*/
		return _T("SELECT  "
			"LineItemT1.ID, LineItemT1.PatientID AS PatID, ServiceT.ID AS ItemID, ServiceT.Name AS ItemName,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  "
			//Tax amount is (everything else) * (taxrate - 1)
			"Round(convert(money, Convert(money,((LineItemT1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate-1)))),2) AS Tax1Amount, "
			"Round(convert(money, Convert(money,((LineItemT1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate2-1)))),2) AS Tax2Amount, "
			"Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS PayAmt,   "
			"Sum(CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS AdjAmt,   "
			"LineItemT1.Date AS TDate, LineItemT1.InputDate AS IDate, ServiceT.Price AS UnitCost, LineItemT1.LocationID AS LocID,  "
			"PersonT.ID AS ProvID, "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS ProvName, ChargesT.Quantity, ChargesT.TaxRate, ChargesT.TaxRate2, "
			"MultiSupplierT.SupplierID AS SupplierID "
			"FROM  "
			"ChargesT INNER JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
			"	INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID  "
			"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
			"	LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"	LEFT JOIN AppliesT ON LineItemT1.ID = AppliesT.DestID "
			"	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
			"	LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
			"	LEFT JOIN PersonT ON ChargesT.DoctorsProviders = PersonT.ID  "
			"	INNER JOIN (   "
			"			SELECT DISTINCT BillsT.ID AS BillID   "
			"			FROM BillsT   "
			"			LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID   "
			"			INNER JOIN (   "
			"				SELECT DISTINCT BillID   "
			"				FROM   "
			"				LineItemT   "
			"				INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID   "
			"				WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10  "
			"			) ChargesSubQ ON BillsT.ID = ChargesSubQ.BillID   "
			"			WHERE   "
			"			BillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL   "
			"			AND BillsT.EntryType = 1   "
			"		) AS ValidBillsQ ON ChargesT.BillID = ValidBillsQ.BillID "
			"WHERE LineItemT1.Deleted = 0 AND LineItemT1.Type = 10 AND (LineItemT.Deleted Is Null OR LineItemT.Deleted = 0) "
			"GROUP BY  "
			"LineItemT1.ID, ChargesT.ID, LineItemT1.PatientID, ServiceT.ID, ServiceT.Name,  "
			"LineItemT1.Amount, Quantity, ChargesT.TaxRate, ChargesT.TaxRate2, CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4,  "
			"LineItemT1.Date, LineItemT1.InputDate, ServiceT.Price, LineItemT1.LocationID,  "
			"PersonT.ID, "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END, ChargesT.Quantity,ChargesT.TaxRate, ChargesT.TaxRate2, MultiSupplierT.SupplierID ");
		break;

	case 270:
		//Inventory Sales by Category
		/*
		Revision History:
		- TES 12/5/02: Changed the query to not include quotes (!) or deleted charges (!)
		- TES 1/21/03: Changed the query to not filter out sales with no applies.
		- TES 1/27/04: Added PatientID so we could enable the "Use Filter" option.
		// (b.spivey, August 20, 2014) - PLID 60797 - Corrected the service date/input date fields to pull from the charge, not the payment.
		*/
		return _T("SELECT  "
			"AppliesT.ID, ServiceT.ID AS ItemID, ServiceT.Name AS ItemName, AppliesT.Amount,   "
			"LineItemT1.Date AS TDate, LineItemT1.InputDate AS IDate, PersonT.ID AS ProvID,   "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS ProvName,  "
			"CASE WHEN LineItemT.Type = 2 THEN 'Adjustment' ELSE "
			"     CASE WHEN PaymentsT.PayMethod = 1 THEN 'Cash'   "
			"          WHEN PaymentsT.PayMethod = 2 THEN 'Check'   "
			"          WHEN PaymentsT.PayMethod = 3 THEN 'Credit' END "
			"END AS PayMethod,   "
			"CategoriesT.Name AS Category, CategoriesT.ID AS CategoryID, ServiceT.Price AS UnitCost, LineItemT1.LocationID AS LocID,  "
			"ChargesT.Quantity, LineItemT.PatientID AS PatID  "
			"FROM  "
			"ChargesT INNER JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID  "
			"	LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number  "
			"	INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID   "
			"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID   "
			"	LEFT JOIN AppliesT ON LineItemT1.ID = AppliesT.DestID  "
			"	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"	LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID   "
			"	LEFT JOIN PersonT ON ChargesT.DoctorsProviders = PersonT.ID   "
			"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID  "
			"WHERE (LineItemT.Deleted Is Null OR LineItemT.Deleted = 0) AND LineItemT1.Type = 10 AND LineItemT1.Deleted = 0 ");
		break;

		/*			return _T("SELECT  "
		"LineItemT1.ID, LineItemT1.PatientID, ServiceT.ID AS ItemID, ServiceT.Name AS ItemName,  "
		"Round(Convert(money,(CASE WHEN [LineItemT1].[Amount] Is Null THEN 0 ELSE [LineItemT1].[Amount] End)*[Quantity]*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)*(CASE WHEN [Multiplier] Is Null THEN 1 ELSE Multiplier End)),2) AS ChargeAmount,  "
		"(AppliesT.Amount) AS PayAmt,   "
		"LineItemT1.Date AS TDate, LineItemT1.InputDate AS IDate, ServiceT.Price AS UnitCost, LineItemT1.LocationID AS LocID,  "
		"PersonT.ID AS ProvID, "
		"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS ProvName,  "
		"CategoriesT.Name AS Category, CategoriesT.ID AS CategoryID,  "
		"CASE WHEN PaymentsT.PayMethod = 1 THEN 'Cash'   "
		"WHEN PaymentsT.PayMethod = 2 THEN 'Check'   "
		"WHEN PaymentsT.PayMethod = 3 THEN 'Credit' END AS PayMethod "
		"FROM  "
		"ChargesT INNER JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
		"	LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
		"	INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID  "
		"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
		"	LEFT JOIN AppliesT ON LineItemT1.ID = AppliesT.DestID "
		"	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
		"	LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
		"	LEFT JOIN PersonT ON ChargesT.DoctorsProviders = PersonT.ID  "
		"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
		"WHERE LineItemT1.Deleted = 0 "
		"GROUP BY  "
		"LineItemT1.ID, LineItemT1.PatientID, ServiceT.ID, ServiceT.Name,  "
		"Round(Convert(money,(CASE WHEN [LineItemT1].[Amount] Is Null THEN 0 ELSE [LineItemT1].[Amount] End)*[Quantity]*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)*(CASE WHEN [Multiplier] Is Null THEN 1 ELSE Multiplier End)),2),  "
		"LineItemT1.Date, LineItemT1.InputDate, ServiceT.Price, LineItemT1.LocationID,  "
		"PersonT.ID, "
		"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END,  "
		"CategoriesT.Name, CategoriesT.ID, CASE WHEN PaymentsT.PayMethod = 1 THEN 'Cash'   "
		"WHEN PaymentsT.PayMethod = 2 THEN 'Check'   "
		"WHEN PaymentsT.PayMethod = 3 THEN 'Credit' END");
		break;*/

	case 271:
		//Inventory Sales Graph
		/*
		Revision History:
		- TES 12/5/02: Changed the query to not include quotes (!) or deleted charges (!)
		- TES 1/21/03: Changed the query to not filter out sales with no applies.
		*/
		return _T("SELECT "
			"Left(convert(varchar, (LineItemT.Date)), 3) AS Month,  "
			"AppliesT.ID, ServiceT.ID AS ItemID, ServiceT.Name AS ItemName, AppliesT.Amount,  "
			"LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, PersonT.ID AS ProvID,  "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS ProvName, "
			"CASE WHEN PaymentsT.PayMethod = 1 THEN 'Cash'  "
			"     WHEN PaymentsT.PayMethod = 2 THEN 'Check'  "
			"     WHEN PaymentsT.PayMethod = 3 THEN 'Credit' END AS PayMethod,  "
			"CategoriesT.Name AS Category, CategoriesT.ID AS CategoryID, ServiceT.Price AS UnitCost, "
			"Year(LineItemT.Date) AS TYear, LineItemT.LocationID AS LocID "
			"FROM "
			"PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"	INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"	INNER JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID "
			"	INNER JOIN ChargesT ON LineItemT1.ID = ChargesT.ID "
			"	INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
			"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"	LEFT JOIN PersonT ON ChargesT.DoctorsProviders = PersonT.ID "
			"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"WHERE (LineItemT.Deleted Is Null OR LineItemT.Deleted = 0) AND LineItemT1.Type = 10 AND LineItemT1.Deleted = 0");
		break;
	case 284://Procedure Correlation by Cause
			 /*
			 Revision History:
			 - TES 12/19/02: Changed the query to not include quotes
			 (e.lally 2008-09-30) PLID 31422 - Added support for multiple locations
			 */
	{
		CString sql = _T(
			"SELECT Sub1.CauseID AS CauseID, Cause, Effect, "
			" "
			"(	SELECT Count(ChargesT1.ID)AS Charges  "
			"	FROM ChargesT AS ChargesT1 "
			"	INNER JOIN ServiceT AS ServiceT1 ON ChargesT1.ServiceID = ServiceT1.ID "
			"	INNER JOIN ProcedureT AS ProcedureT1 ON ServiceT1.ProcedureID = ProcedureT1.ID "
			"	INNER JOIN LineItemT AS LineItemT1 ON ChargesT1.ID = LineItemT1.ID "
			" "
			"	WHERE ProcedureT1.ID = EffectID AND EXISTS  "
			"	(	SELECT ChargesT2.ID  "
			"		FROM ChargesT AS ChargesT2  "
			"		INNER JOIN LineItemT AS LineItemT2 ON ChargesT2.ID = LineItemT2.ID "
			"		INNER JOIN ServiceT AS ServiceT2 ON ChargesT2.ServiceID = ServiceT2.ID "
			"		INNER JOIN ProcedureT AS ProcedureT2 ON ServiceT2.ProcedureID = ProcedureT2.ID "
			"		INNER JOIN BillsT ON ChargesT2.BillID = BillsT.ID "
			"		WHERE LineItemT2.PatientID = LineItemT1.PatientID  "
			"		AND ProcedureT2.ID = CauseID "
			"		AND LineItemT2.Date < LineItemT1.Date "
			"		AND LineItemT2.Date >= @DateFrom "
			"		AND LineItemT2.Date <= @DateTo "
			"		AND LineItemT2.Deleted <> 1 AND LineItemT2.Type = 10 "
			"		@ChargesT2.ProvID"
			"		@LocID "
			"	) AND LineItemT1.Deleted <> 1 AND LineItemT1.Type = 10 "
			") AS EffectCount, "
			" "
			"(	SELECT SUM (AppliesT1.Amount)AS Total "
			"	FROM ChargesT AS ChargesT1 "
			"	INNER JOIN ServiceT AS ServiceT1 ON ChargesT1.ServiceID = ServiceT1.ID "
			"	INNER JOIN ProcedureT AS ProcedureT1 ON ServiceT1.ProcedureID = ProcedureT1.ID "
			"	INNER JOIN LineItemT AS LineItemT1 ON ChargesT1.ID = LineItemT1.ID "
			"	INNER JOIN AppliesT AS AppliesT1 ON ChargesT1.ID = AppliesT1.DestID "
			"	INNER JOIN LineItemT AS PayLinesT ON AppliesT1.SourceID = PayLinesT.ID "
			" "
			"	WHERE ProcedureT1.ID = EffectID AND EXISTS  "
			"	(	SELECT ChargesT2.ID  "
			"		FROM ChargesT AS ChargesT2  "
			"		INNER JOIN LineItemT AS LineItemT2 ON ChargesT2.ID = LineItemT2.ID "
			"		INNER JOIN ServiceT AS ServiceT2 ON ChargesT2.ServiceID = ServiceT2.ID "
			"		INNER JOIN ProcedureT AS ProcedureT2 ON ServiceT2.ProcedureID = ProcedureT2.ID "
			"		INNER JOIN BillsT ON ChargesT2.BillID = BillsT.ID "
			"		WHERE LineItemT2.PatientID = LineItemT1.PatientID  "
			"		AND ProcedureT2.ID = CauseID "
			"		AND LineItemT2.Date < LineItemT1.Date "
			"		AND LineItemT2.Date >= @DateFrom "
			"		AND LineItemT2.Date <= @DateTo "
			"		AND LineItemT2.Deleted <> 1  AND LineItemT2.Type = 10 "
			"		@LocID "
			"		@ChargesT2.ProvID "
			"	) AND LineItemT1.Deleted <> 1 AND LineItemT1.Type = 10 AND (PayLinesT.Type = 1 OR PayLinesT.Type = 3) "
			") AS Total "
			" "
			"FROM  "
			" "
			"( "
			"	SELECT ProcedureT1.Name AS Cause, ProcedureT1.ID AS CauseID, ProcedureT2.Name AS Effect, ProcedureT2.ID AS EffectID "
			" "
			"	FROM LineItemT AS LineItemT1 "
			"	INNER JOIN LineItemT AS LineItemT2 ON LineItemT1.PatientID = LineItemT2.PatientID "
			"	INNER JOIN ChargesT AS ChargesT1 ON LineItemT1.ID = ChargesT1.ID "
			"	INNER JOIN ChargesT AS ChargesT2 ON LineItemT2.ID = ChargesT2.ID "
			"	INNER JOIN ServiceT AS ServiceT1 ON ChargesT1.ServiceID = ServiceT1.ID "
			"	INNER JOIN ServiceT AS ServiceT2 ON ChargesT2.ServiceID = ServiceT2.ID "
			"	INNER JOIN ProcedureT AS ProcedureT1 ON ServiceT1.ProcedureID = ProcedureT1.ID "
			"	INNER JOIN ProcedureT AS ProcedureT2 ON ServiceT2.ProcedureID = ProcedureT2.ID "
			"	INNER JOIN BillsT ON ChargesT1.BillID = BillsT.ID "
			" "
			"	WHERE LineItemT1.Date < LineItemT2.Date "
			"		AND LineItemT1.Deleted <> 1 AND LineItemT2.Deleted <> 1 "
			"       AND LineItemT1.Type = 10 AND LineItemT2.Type = 10 "
			"		AND LineItemT1.Date >= @DateFrom "
			"		AND LineItemT1.Date <= @DateTo "
			"		@ChargesT1.ProvID "
			"		@LocID "
			" "
			"	GROUP BY ProcedureT1.Name, ProcedureT1.ID, ProcedureT2.Name, ProcedureT2.ID "
			") AS Sub1 "
			);

		CString strProvider1, strProvider2;
		if (nProvider > 0) {
			strProvider1.Format("AND ChargesT1.DoctorsProviders = %i", nProvider);
			strProvider2.Format("AND ChargesT2.DoctorsProviders = %i", nProvider);
		}
		else if (nProvider == -2) {
			strProvider1.Format("AND (ChargesT1.DoctorsProviders Is Null OR ChargesT1.DoctorsProviders = -1)");
			strProvider2.Format("AND (ChargesT2.DoctorsProviders Is Null OR ChargesT2.DoctorsProviders = -1)");
		}
		else if (nProvider == -3) {
			strProvider1.Format("AND ChargesT1.DoctorsProviders IN (");
			strProvider2.Format("AND ChargesT2.DoctorsProviders IN (");
			CString strPart;
			for (int i = 0; i < m_dwProviders.GetSize(); i++) {
				strPart.Format("%li, ", (long)m_dwProviders.GetAt(i));
				strProvider1 += strPart;
				strProvider2 += strPart;
			}
			strProvider1 = strProvider1.Left(strProvider1.GetLength() - 2) + ")";
			strProvider2 = strProvider2.Left(strProvider2.GetLength() - 2) + ")";
		}
		sql.Replace("@ChargesT1.ProvID", strProvider1);
		sql.Replace("@ChargesT2.ProvID", strProvider2);

		CString strLocation;
		//(e.lally 2008-09-30) PLID 31422 - Added support for multiple locations
		if (nLocation == -2) {
			//This will always be false as the data structure stands does not allow for null
			strLocation = "AND Location IS NULL ";
		}
		else if (nLocation == -3) {
			strLocation.Format("AND Location IN (");
			CString strPart;
			for (int i = 0; i < m_dwLocations.GetSize(); i++) {
				strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
				strLocation += strPart;
			}
			strLocation = strLocation.Left(strLocation.GetLength() - 2) + ")";
		}
		else if (nLocation != -1) {
			strLocation.Format("AND Location = %i", nLocation);
		}
		sql.Replace("@LocID", strLocation);

		if (nDateRange != -1)
		{
			sql.Replace("@DateFrom", DateFrom.Format("'%m/%d/%Y'"));
			sql.Replace("@DateTo", DateTo.Format("'%m/%d/%Y'"));
		}
		else
		{
			sql.Replace("@DateFrom", "'2/10/1878'");
			sql.Replace("@DateTo", "'2/10/2178'");
		}

		return sql;
	}


	case 297:	//Financial Activity - Daily (by Charge Code)
		return GetSqlFinancialActivity(nSubLevel, nSubRepNum);
		break;

	case 299:
		//Daily Posting Totals
		/*	Version History
		2/4/03 DRT - I made a small change to the way the union works so it will pick up charges of the same amount (I really can't believe we are STILL having
		these problems).  This query really sucks, and I'd like to re-write it at some point (preferably soon).  It took me 30 minutes to figure out what
		was going on, the naming is horrible, the joins are hard to read, and there are no comments.
		9/8/04 TES - Broke out Gift Certificate charges and payments, they don't actually show on the report unless you
		edit it.
		DRT - 4/18/2005 - Several changes
		PLID 16276 - Changed so that it reads the batch payment deposited flag.  Previously it was reading the flag on the child payment,
		which (due to another bug - PLID 16280), was basically a random value depending what the office had done.  That value should always be
		ignored, regardless of its status.
		PLID 16281 - Fixed a bug in which the UNION'd queries in the _PaymentsByReferralSourcePartQ were not in the same order.
		Reworked the indentation of the query.  Not as well as my ranting note on 2/4/2003 suggested, but a little ways there.  Also removed
		a silly "return _T("");" which was after the "return _T(<query here>);" code.
		// (z.manning, 04/25/2007) - PLID 25444 - Added service category ID and name fields.
		TES - 9/17/2007 - PLID 27411 - Fixed a bug where unapplied payment amounts would get duplicated.
		// (j.jones 2009-06-09 15:46) - PLID 34549 - ensured batch payment info. is not loaded for adjustments
		// (r.gonet 2015-09-17) - PLID 67067 - Corrected a bug where the pay method was being selected as NULL for non-payment type line items, leading to refunds always reporting as 0.00 on the Daily Posting Totals report.
		*/
		return _T("SELECT TotalSubQ.ID, TotalSubQ.Type, TotalSubQ.Amount, TotalSubQ.Prov AS ProvID, TotalSubQ.PayMethod, TotalSubQ.Date AS TDate, TotalSubQ.InputDate AS IDate,  "
			"CASE WHEN PersonT.ID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS ProvName, Deposited, PersonT.ID AS PatID,  "
			"TotalSubQ.LocationID AS LocID, CategoryID, COALESCE(CategoryName, '{No Category}') AS CategoryName  "
			"FROM  "
			"	(SELECT LineItemT.ID, CASE WHEN ServiceID IN (SELECT ServiceID FROM GCTypesT) THEN 'GCSOLD' ELSE 'CHARGE' END AS Type,  "
			"	dbo.GetChargeTotal(ChargesT.ID) AS Amount, ChargesT.DoctorsProviders AS Prov, NULL AS Paymethod, LineItemT.Date,  "
			"	LineItemT.InputDate, 0 AS Deposited, LineItemT.LocationID, CategoriesT.ID AS CategoryID, CategoriesT.Name AS CategoryName  "
			"	FROM "
			"	BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number  "
			"	LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
			"   LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID  "
			"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.EntryType = 1  "
			" "
			"	UNION  "
			" "
			"	/*Begin blatant copying out of payments reports*/ "
			"	SELECT PaymentsByReferralSourceSubQ.ID,  "
			"	CASE WHEN PaymentsByReferralSourceSubQ.Type = 1 THEN 'PAYMENT'   "
			"	WHEN PaymentsByReferralSourceSubQ.Type = 2 THEN 'ADJUSTMENT'   "
			"	WHEN PaymentsByReferralSourceSubQ.Type = 3 THEN 'REFUND' END AS Type,  "
			"	Sum(PaymentsByReferralSourceSubQ.Amount) AS Amount,   "
			"	PaymentsByReferralSourceSubQ.ProvID AS Prov,   "
			"	CASE WHEN PaymentsByReferralSourceSubQ.Type IN (1,3) THEN  "
			"			CASE WHEN PaymentsByReferralSourceSubQ.PayMethod IN (1,7) THEN 'Cash'   "
			"				WHEN PaymentsByReferralSourceSubQ.PayMethod IN (2,8) THEN 'Check'   "
			"				WHEN PaymentsByReferralSourceSubQ.Paymethod IN (3,9) THEN 'Credit'  "
			"				WHEN PaymentsByReferralSourceSubQ.PayMethod IN (4,10) THEN 'Gift Certificate'  "
			"			END  "
			"		END AS PayMethod, "
			"	PaymentsByReferralSourceSubQ.TDate AS Date,   "
			"	PaymentsByReferralSourceSubQ.IDate AS InputDate,   "
			"	PaymentsByReferralSourceSubQ.Deposited,   "
			"	PaymentsByReferralSourceSubQ.LocationID,  "
			"	PaymentsByReferralSourceSubQ.CategoryID,  "
			"	PaymentsByReferralSourceSubQ.CategoryName "
			"	FROM  "
			"		(SELECT * FROM  "
			"			(SELECT LineItemT.ID, LineItemT.Type,   "
			"			Amount = CASE WHEN [_PartiallyAppliedPaysQ].[ID] Is Null THEN CASE WHEN [LineItemT_1].[ID] Is Null  "
			"				THEN [LineItemT].[Amount] ELSE [AppliesT].[Amount] End  "
			"				ELSE  [AppliesT].[Amount] End,   "
			"			ProvID = CASE WHEN [DoctorsProviders] Is Null THEN PaymentsT.ProviderID ELSE [DoctorsProviders] End,   "
			"			'Full' AS RandomText,  LineItemT.InputDate AS IDate,  LineItemT.Date AS TDate,  PaymentsT.PayMethod,   "
			"			CASE WHEN PaymentsT.BatchPaymentID IS NULL OR LineItemT.Type = 2 THEN PaymentsT.Deposited ELSE BatchPaymentsT.Deposited END AS Deposited, "
			"			LineItemT.LocationID, CategoriesT.ID AS CategoryID, CategoriesT.Name AS CategoryName  "
			"			FROM (((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID)  "
			"			LEFT JOIN  "
			"				(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  Sum(AppliesT.Amount) AS ApplyAmt,   "
			"				/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  LineItemT_1.PatientID   "
			"				FROM LineItemT AS LineItemT_1 LEFT JOIN  "
			"				(PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID  "
			"				WHERE (((LineItemT_1.Deleted)=0))  "
			"				GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"				HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"				) AS _PartiallyAppliedPaysQ  "
			"			ON LineItemT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)  "
			"			LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID)  "
			"			LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  "
			"			LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID  "
			"			LEFT JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID "
			"			LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"			LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"			WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0))  "
			"			) AS _PaymentsByReferralSourceFullQ  "
			"		UNION ALL  "
			"		SELECT * FROM  "
			"			(SELECT [_PartiallyAppliedPaysQ].ID,  LineItemT.Type,  [_PartiallyAppliedPaysQ].Total AS Amount,   "
			"			ProvID = PaymentsT.ProviderID,  'Part' AS RandomText,  LineItemT.InputDate AS IDate,  LineItemT.Date AS TDate,   "
			"			PaymentsT.PayMethod, CASE WHEN PaymentsT.BatchPaymentID IS NULL OR LineItemT.Type = 2 THEN PaymentsT.Deposited ELSE BatchPaymentsT.Deposited END AS Deposited, "
			"			LineItemT.LocationID, NULL AS CategoryID, convert(nvarchar,NULL) AS CategoryName "
			"			FROM ((( "
			"				(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  Sum(AppliesT.Amount) AS ApplyAmt,   "
			"				/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  LineItemT_1.PatientID "
			"				FROM LineItemT AS LineItemT_1  "
			"				LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  "
			"				ON LineItemT_1.ID = PaymentsT.ID  "
			"				WHERE (((LineItemT_1.Deleted)=0))  "
			"				GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			"				HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
			"				) AS _PartiallyAppliedPaysQ  "
			"			INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID)  "
			"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)  "
			"			ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID  "
			"			LEFT JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID "
			//TES 9/17/2007 - PLID 27411 - This query is for the unapplied amount, so don't link through AppliesT.
			/*"			LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"			LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
			"			LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"			LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "*/
			"			WHERE LineItemT.Deleted = 0  "
			"			) AS _PaymentsByReferralSourcePartQ  "
			"		) AS PaymentsByReferralSourceSubQ  "
			"	LEFT JOIN (ProvidersT LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) ON PaymentsByReferralSourceSubQ.ProvID = ProvidersT.PersonID  "
			"	GROUP BY PaymentsByReferralSourceSubQ.ID, PaymentsByReferralSourceSubQ.Type, PaymentsByReferralSourceSubQ.ProvID, PaymentsByReferralSourceSubQ.PayMethod,  "
			"	PaymentsByReferralSourceSubQ.TDate,  PaymentsByReferralSourceSubQ.IDate, PaymentsByReferralSourceSubQ.Deposited, PaymentsByReferralSourceSubQ.LocationID,  "
			"	PaymentsByReferralSourceSubQ.CategoryID, PaymentsByReferralSourceSubQ.CategoryName "
			"	/*end blatant copying out of payments reports*/ "
			"	) TotalSubQ  "
			"LEFT JOIN PersonT ON TotalSubQ.Prov = PersonT.ID");
		break;

	case 325: //Financial Activity - Daily (by Charge Category)
		return GetSqlFinancialActivity(nSubLevel, nSubRepNum);

	case 333:
		//Prepayments Applied
		/*	Version History
		1/24/03 - DRT - The SubQuery was correctly pulling out all items that were applied TO a refund, but the filter outside that was filtering
		on just Prepayment = 1, which, obviously, anything that is applied to a prepayment is not itself a prepayment.  I moved the Prepayment = 1
		filter inside each union of the subquery, in the correct place.
		8/25/04 - TES - The query was failing to reverse the sign of applies to prepayments
		5/18/05 - DRT - Changed the location filter to be the destination in all cases (chg if pay->chg, pay2 if pay->pay2)
		8/01/06 - TES - Changed the provider filter to be the destination in all cases (chg if pay->chg, pay2 if pay->pay2)
		TES 7/9/2008 - PLID 29580 - Added an ApplyDate, which will be the default filter, now that we actually use that
		date on all the financial reports.
		TES 10/13/2009 - PLID 35662 - Restored the old Service Date filter (the query didn't change).
		(j.jones 2015-10-07 15:42) - PLID 66865 - corrections can cause a voided prepayment to be applied
		to an original prepayment, and this code did not account for prepays applied to prepays
		*/
		return _T("/* Applied PrepaysQ */"
			"SELECT LineItemT.ID, LineItemT.PatientID AS PatID, CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS TDate, "
			"CASE WHEN LineItemT.InputDate > AppliesSubQ.InputDate THEN LineItemT.InputDate ELSE AppliesSubQ.InputDate END AS IDate, PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First AS PatName, AppliesSubQ.Amount, AppliesSubQ.Description AS BillDesc, LineItemT.Description AS PayDesc, "
			"AppliesSubQ.ProviderID AS ProvID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName, AppliesSubQ.LocationID AS LocID, "
			"AppliesSubQ.ApplyDate AS ApplyDate "
			""
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN "
			"(  /* select all charges which have a prepayment applied to them (prepay is source) */"
			"SELECT SourceID, Sum(AppliesT.Amount) AS Amount, BillsT.Description, BillsT.Date, LineItemT.InputDate, LineItemT.LocationID, ChargesT.DoctorsProviders AS ProviderID, "
			"AppliesT.InputDate AS ApplyDate "
			"FROM AppliesT INNER JOIN (ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID) ON AppliesT.DestID = ChargesT.ID INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
			"WHERE PaymentsT.Prepayment = 1 "
			"GROUP BY AppliesT.SourceID, BillsT.ID, BillsT.Description, BillsT.Date, LineItemT.InputDate, LineItemT.LocationID, ChargesT.DoctorsProviders, AppliesT.InputDate "
			""
			"UNION ALL "
			"/* select all payments which have a prepayment applied to them (prepay is source) */"
			"/* this is only likely to happen with voided prepayments */"
			"SELECT SourceID, AppliesT.Amount, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.LocationID, PaymentsT.ProviderID, "
			"	AppliesT.InputDate AS ApplyDate "
			"FROM AppliesT "
			"INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
			"INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID "
			"INNER JOIN PaymentsT PaymentsT2 ON AppliesT.SourceID = PaymentsT2.ID "
			"INNER JOIN LineItemT LineItemT2 ON PaymentsT.ID = LineItemT2.ID "
			"WHERE PaymentsT2.Prepayment = 1 "
			""
			"UNION ALL "
			"/* select all items applied to a prepayment */"
			"SELECT SourceID, -1*AppliesT.Amount, LineItemT2.Description, LineItemT2.Date, LineItemT2.InputDate, LineItemT2.LocationID, PaymentsT.ProviderID, "
			"	AppliesT.InputDate AS ApplyDate "
			"FROM AppliesT "
			"INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID "
			"INNER JOIN LineItemT LineItemT2 ON PaymentsT.ID = LineItemT2.ID "
			"WHERE PaymentsT.Prepayment = 1 "
			""
			") AppliesSubQ "
			""
			"ON LineItemT.ID = AppliesSubQ.SourceID LEFT JOIN (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) ON LineItemT.PatientID = PersonT.ID "
			"LEFT JOIN PersonT PersonT1 ON AppliesSubQ.ProviderID = PersonT1.ID "
			"WHERE AppliesSubQ.Amount <> 0");
		break;

	case 335:
		//Outstanding Insurance Balances
		//  For Dr. Miller
		//		Detailed report will show Insurance company as group 1, patient as group 2, then all detail information (date, amt, cpt code, etc)
		//			it will then summary the total outstanding balance for each patient and each insurance company, and sort descending on all
		//			amounts (similar to AR by amount).  
		//		Summary report will show JUST insurance company and total balance, sorting descending again
		//		Also a filter that lets you specify to only include charges that are over 30 days old, over 60 days old, etc
		//			so that new charges aren't including in the listing (there's no need to call an insurance company over a charge that was just submitted yesterday)
		/*	Version History
		1/28/03 - Fixed the special GetExtraFilter work, it was always filtering on TDate, even for input date attempts.  Made it use the new structure.
		JMM - 8/3/2006 - PLID 21661 - made the insurance phone number pull from the default insurance contact
		// (j.gruber 2010-08-04 13:10) - PLID 39951 - added BillID
		// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
		// (j.jones 2011-04-06 09:42) - PLID 37717 - added a ton of data to this report, including
		// claim sent dates and more balance information
		*/

		strSQL = "SELECT "
			"ChargesT.ID AS ChargeID, PersonT.ID AS PatID, "
			"LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
			"ChargesT.DoctorsProviders AS ProvID, LineItemT.LocationID AS LocID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"PatientsT.UserDefinedID, PersonT.BirthDate, "
			"dbo.MaskSSN(PersonT.SocialSecurity, ";

		strSQL += ((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1");

		//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values
		strSQL += FormatString(") AS SocialSecurity, "

			"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, "
			"InsuranceContactsQ.WorkPhone AS InsurancePhone, InsuranceContactsQ.Extension AS InsurancePhoneExt, "
			"InsuranceContactsQ.ContactName AS InsuranceContactName, "
			"InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum, InsurancePlansT.PlanName, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmt, ChargeRespT.Amount AS InsResp, "
			"Coalesce(TotalInsAppliesQ.TotalApplied, 0) AS TotalInsAppliedAmount, "
			"Coalesce(TotalInsAppliesQ.AppliedPays, 0) AS InsAppliedPays, "
			"Coalesce(TotalInsAppliesQ.AppliedAdjs, 0) AS InsAppliedAdjs, "
			"ChargesT.BillID, BillsT.Date AS BillDate, BillsT.InputDate AS BillInputDate, "
			"BillsT.Description AS BillDesc, LineItemT.Description AS ChargeDesc, "
			"ChargesT.ItemCode, ChargesT.ItemSubCode, ChargesT.Quantity, "
			"ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, "
			"DATEDIFF(day, LineItemT.Date, GetDate()) AS AgeOfCharge, "
			/* > 30 (1), > 60 (2), > 90 (3), > 120 (4)*/ 	//for filtering
			"CASE WHEN DATEDIFF(day, LineItemT.Date, GetDate()) > 30 THEN 1 "
			"     WHEN DATEDIFF(day, LineItemT.Date, GetDate()) > 60 THEN 2 "
			"     WHEN DATEDIFF(day, LineItemT.Date, GetDate()) > 90 THEN 3 "
			"     WHEN DATEDIFF(day, LineItemT.Date, GetDate()) > 120 THEN 4 END AS AgedDaysFilter, "

			"ClaimHistoryQ.FirstDateSent, ClaimHistoryQ.LastDateSent, Coalesce(ClaimHistoryQ.TimesSent, 0) AS CountOfSubmissions "
			" "
			"FROM PersonT "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"INNER JOIN LineItemT ON PersonT.ID = LineItemT.PatientID "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN PersonT InsPersonT ON InsuranceCoT.PersonID = InsPersonT.ID "
			"LEFT JOIN (SELECT InsuranceContactsT.PersonID, "
			"	PersonT.WorkPhone, PersonT.Extension, "
			"	PersonT.Last + 	CASE WHEN PersonT.Last <> '' THEN ', ' ELSE ' ' END + PersonT.First + ' ' + PersonT.Middle AS ContactName "
			"	FROM InsuranceContactsT "
			"	INNER JOIN PersonT ON InsuranceContactsT.PersonID = PersonT.ID "
			") AS InsuranceContactsQ ON InsuredPartyT.InsuranceContactID = InsuranceContactsQ.PersonID "
			"LEFT JOIN ( "
			"	SELECT AppliesT.DestID, AppliesT.RespID, "
			"	Sum(AppliesT.Amount) AS TotalApplied, "
			"	Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS AppliedPays, "
			"	Sum(CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS AppliedAdjs "
			"	FROM AppliesT "
			"	LEFT JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
			"	WHERE AppliesT.PointsToPayments = 0 AND LineItemT.Deleted = 0 "
			"	GROUP BY AppliesT.DestID, AppliesT.RespID "
			") TotalInsAppliesQ ON ChargeRespT.ChargeID = TotalInsAppliesQ.DestID AND ChargeRespT.ID = TotalInsAppliesQ.RespID "
			"LEFT JOIN ( "
			"	SELECT BillID, InsuredPartyID, "
			"	Min(Date) AS FirstDateSent, Max(Date) AS LastDateSent, "
			"	Count(ID) AS TimesSent "
			"	FROM ClaimHistoryT "
			"	WHERE SendType >= %li " //(r.wilson 10/2/2012) plid 53082 - this line was "   WHERE SendType >= 0 "
			"	GROUP BY BillID, InsuredPartyID "
			") AS ClaimHistoryQ ON BillsT.ID = ClaimHistoryQ.BillID AND InsuredPartyT.PersonID = ClaimHistoryQ.InsuredPartyID "

			"WHERE LineItemT.Deleted = 0 AND (ChargeRespT.Amount - Coalesce(TotalInsAppliesQ.TotalApplied, 0)) > 0",
			ClaimSendType::Electronic);
		return _T(strSQL);
		break;

	case 340: //Aged Payment Activity (shows charges and how long it took to pay them).
		return _T("SELECT  "
			"LineItemT.ID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, LineItemT.Date AS TDate,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS Amount,  "
			"dbo.GetChargeTotal(ChargesT.ID) - (CASE WHEN (SELECT Sum(CASE WHEN Amount Is Null THEN 0 ELSE Amount END) FROM AppliesT WHERE DestID = LineItemT.ID) Is Null THEN 0 ELSE (SELECT Sum(CASE WHEN Amount Is Null THEN 0 ELSE Amount END) FROM AppliesT WHERE DestID = LineItemT.ID) END) AS RemainingBalance, "
			"AppliesT.Amount AS AppliedAmount, DATEDIFF(day, LineItemT.Date, LinePays.Date) AS DaysLater,  "
			"CASE WHEN AppliesT.RespID Is Null THEN '' ELSE CASE WHEN AppliesT.RespID IN (SELECT ID FROM ChargeRespT WHERE InsuredPartyID IS Not Null) THEN 'Insurance' ELSE 'Patient' END END AS Responsibility,  "
			"ServiceT.Name, CPTCodeT.Code, PersonT.ID AS PatID, PatientsT.UserDefinedID, LineItemT.LocationID AS LocID, ChargesT.DoctorsProviders AS ProvID "
			"FROM (LineItemT INNER JOIN ((ChargesT LEFT JOIN (ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID) ON ChargesT.ServiceID = ServiceT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number)  "
			"ON LineItemT.ID = ChargesT.ID LEFT JOIN (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) ON LineItemT.PatientID = PersonT.ID  "
			"LEFT JOIN PersonT PersonTProv ON ChargesT.DoctorsProviders = PersonTProv.ID LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) "
			"LEFT JOIN (AppliesT INNER JOIN (SELECT * FROM LineItemT WHERE Type = 1) LinePays ON AppliesT.SourceID = LinePays.ID INNER JOIN PaymentsT ON LinePays.ID = PaymentsT.ID) "
			"ON LineItemT.ID = AppliesT.DestID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10");
		break;

	case 342: //Claim History
			  /*	Revision History
			  - JMJ 4/6/2005: Added support for showing which charges were sent
			  TES 3/13/2007 - PLID 24993 - In ReportInfoCallback.cpp as well as in this query, I updated the Extended filter
			  to show "UB Form" instead of "UB92 Form"
			  // (j.jones 2010-02-04 11:59) - PLID 37113 - supported the NY Medicaid form
			  // (d.singleton 2011-10-04 11:14) - PLID 42719 -  added ClaimTotal, the claim history report should total the claim total sent that day vs the bill total
			  */
			  //(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values
			  //(r.wilson 2/12/2013) pl 40712 - Added SentWithoutPrint to the query
		return _T(
			FormatString(
				"SELECT "
				"ClaimHistoryT.ID, BillsT.ID AS BillID, BillsT.Description, PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
				"ClaimHistoryT.Date AS TDate, BillsT.Date AS BillDate, (CASE WHEN InsuranceCoT.Name Is NULL THEN '<No Insurance Company>' ELSE InsuranceCoT.Name END) AS InsCoName, InsuranceCoT.PersonID AS InsCoID, "
				"(CASE WHEN SendType = %li THEN 'Electronic' WHEN SendType = %li THEN 'HCFA Form' WHEN SendType = %li THEN 'UB Form' WHEN SendType = %li THEN 'ADA Form' WHEN SendType = %li THEN 'IDPA Form' WHEN SendType = %li THEN 'NYWC Form' WHEN SendType = %li THEN 'MICR Form' WHEN SendType = %li THEN 'NY Medicaid Form' WHEN SendType = %li THEN 'Tracer Letter' WHEN SendType = %li THEN 'Marked Sent Without Exporting'  ELSE '<Unspecified>' END) AS SendStyle, ClaimHistoryT.SendType AS SendType, "
				"ClaimHistoryT.UserName, BillsT.Location AS LocID, ClaimHistoryT.ClearingHouse, Coalesce(dbo.GetBillTotal(BillsT.ID),0) AS Total, "
				"(SELECT Count(ChargeID) FROM ClaimHistoryDetailsT WHERE ClaimHistoryID = ClaimHistoryT.ID) AS CountOfChargesSent, "
				"(SELECT Count(ChargesT.ID) FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 AND ChargesT.BillID = ClaimHistoryT.BillID) AS CountOfCharges ,"
				"(SELECT SUM(dbo.GetChargeTotal(ChargeID)) FROM ClaimHistoryDetailsT WHERE ClaimHistoryID = ClaimHistoryT.ID) As ClaimTotal "
				"FROM ClaimHistoryT INNER JOIN BillsT ON ClaimHistoryT.BillID = BillsT.ID "
				"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN InsuredPartyT ON ClaimHistoryT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1",
				ClaimSendType::Electronic, ClaimSendType::HCFA, ClaimSendType::UB, ClaimSendType::ADA, /*4*/ ClaimSendType::IDPA,
				ClaimSendType::NYWC, ClaimSendType::MICR, ClaimSendType::NYMedicaid, ClaimSendType::TracerLetter, ClaimSendType::SentWithoutPrint
				));
		break;

	case 360: //Discount Analysis
			  /*	Revision History
			  - TES 12/19/02: Fixed to not include charges from quotes, and put in "AS ServiceID so the external filter wouldn't crash".
			  DRT 4/10/2006 - PLID 11734 - Replaced ProcCode.
			  (e.lally 2007-07-18) PLID 26131 - Added charge category and parent charge category
			  (d.moore 2007-08-24) - PLID 27161 - I changed the way discounts were calculated to exclusively use
			  the Discount and Percent off fields instead of comparing the lineitem amount to the price for the
			  cpt service code.
			  (d.moore 2007-08-24) - PLID 25166 - Added a fields for discount, percent off, discount total, and discount category.
			  // (j.gruber 2009-03-25 17:00) - PLID 33823 - updated discount structure
			  */
		return _T(
			"SELECT "
			"PatientsT.PersonID AS PatID, "
			"LineItemT.Date AS TDate, "
			"LineItemT.InputDate AS IDate, "
			"ChargesT.DoctorsProviders AS ProvID, "
			"LineItemT.LocationID AS LocID, "
			"(CASE WHEN CPTCodeT.ID IS NOT NULL THEN ItemCode ELSE '' END) AS CPTCode, "
			"ServiceT.Name AS ServiceName, "
			"(Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2)) as AmtBeforeDiscount, "
			"ChargesT.ServiceID AS ServiceID, "
			"Round(Convert(money, LineItemT.Amount * Quantity), 2) OrigAmount, "
			" dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
			"ChargeDiscountsT.PercentOff, "
			"ChargeDiscountsT.Discount, "
			"Round(Convert(money, LineItemT.Amount * Quantity), 2) - "
			"Round(Convert(money, LineItemT.Amount * Quantity "
			"* (CASE WHEN(ChargeDiscountsT.[PercentOff] Is Null) THEN 1 "
			"ELSE ((100-Convert(float,ChargeDiscountsT.[PercentOff]))/100) END) "
			"-(CASE WHEN ChargeDiscountsT.[Discount] Is Null THEN 0 "
			"ELSE ChargeDiscountsT.[Discount] END)),2) AS TotalDiscount, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"Quantity, "
			"(CASE WHEN DoctorsT.Last Is Null THEN '<No Provider>' "
			"ELSE DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle END) AS DocName, "
			"CategoriesT.Name AS ChargeCategory, "
			"ParentCat.Name AS ParentChargeCategory, "
			"CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL "
			"THEN '' "
			"ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 "
			"THEN ChargeDiscountsT.CustomDiscountDesc "
			"ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 "
			"THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) "
			"ELSE (SELECT Description "
			"FROM DiscountCategoriesT "
			"WHERE ID = ChargeDiscountsT.DiscountCategoryID) "
			"END "
			"END "
			"END AS DiscountCategoryDescription,  "
			" ChargesT.ID AS ChargeID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT AS DoctorsT ON ChargesT.DoctorsProviders = DoctorsT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"LEFT JOIN CategoriesT ParentCat ON CategoriesT.Parent = ParentCat.ID "
			"WHERE LineItemT.Deleted = 0 "
			"AND BillsT.Deleted = 0 "
			"AND BillsT.EntryType = 1 "
			"AND LineItemT.Type = 10 "
			" AND ChargeDiscountsT.Deleted = 0");
		break;

	case 550: //Discount Analysis By Procedure
			  /*	Revision History
			  - JMJ 2/17/05: Created (based on Discount Analysis)
			  DRT 4/10/2006 - PLID 11734 - Replaced ProcCode.
			  (d.moore 2007-08-24) - PLID 27161 - I changed the way discounts were calculated to exclusively use
			  the Discount and Percent off fields instead of comparing the lineitem amount to the price for the
			  cpt service code.
			  (d.moore 2007-08-24) - PLID 25166 - Added a field for the discount category/description, discount,
			  percent off, and discount total.
			  // (j.gruber 2009-03-25 17:07) - PLID 33823 - updated discount structure
			  */
		return _T(
			"SELECT "
			"PatientsT.PersonID AS PatID, "
			"LineItemT.Date AS TDate, "
			"LineItemT.InputDate AS IDate, "
			"ChargesT.DoctorsProviders AS ProvID, "
			"LineItemT.LocationID AS LocID, "
			"(Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2)) as AmtBeforeDiscount, "
			"(CASE WHEN CPTCodeT.ID IS NOT NULL THEN ItemCode ELSE '' END) AS CPTCode, "
			"ServiceT.Name AS ServiceName, "
			"ChargesT.ServiceID AS ServiceID, "
			" dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
			"Round(Convert(money,LineItemT.Amount * Quantity),2) AS OrigAmount, "
			"ChargeDiscountsT.[PercentOff], "
			"ChargeDiscountsT.[Discount], "
			"Round(Convert(money,LineItemT.Amount * Quantity),2) - "
			"Round(Convert(money, LineItemT.Amount * Quantity "
			"* (CASE WHEN(ChargeDiscountsT.[PercentOff] Is Null) THEN 1 "
			"ELSE ((100-Convert(float,ChargeDiscountsT.[PercentOff]))/100) END) "
			"-(CASE WHEN ChargeDiscountsT.[Discount] Is Null THEN 0 "
			"ELSE ChargeDiscountsT.[Discount] END)),2) AS TotalDiscount, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"Quantity, "
			"(CASE WHEN DoctorsT.Last Is Null THEN '<No Provider>' ELSE DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle END) AS DocName, "
			"ServiceT.ProcedureID AS ProcID, "
			"(CASE WHEN ProcedureT.Name Is Null THEN '<No Procedure>' ELSE ProcedureT.Name END) AS ProcedureName, "
			"CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL "
			"THEN '' "
			"ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 "
			"THEN ChargeDiscountsT.CustomDiscountDesc "
			"ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 "
			"THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) "
			"ELSE (SELECT Description "
			"FROM DiscountCategoriesT "
			"WHERE ID = ChargeDiscountsT.DiscountCategoryID) "
			"END "
			"END "
			"END AS DiscountCategoryDescription, "
			"ChargesT.ID as ChargeID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT AS DoctorsT ON ChargesT.DoctorsProviders = DoctorsT.ID "
			"LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
			"WHERE LineItemT.Deleted = 0 "
			"AND BillsT.Deleted = 0 "
			"AND BillsT.EntryType = 1 "
			"AND LineItemT.Type = 10 "
			" AND ChargeDiscountsT.Deleted = 0");
		break;

	case 613: //Discount Analysis By Category
			  /*	Revision History
			  (d.moore 2007-09-12) - PLID 25166 - Report created
			  (e.lally 2008-01-22) PLID 28682 - Fixed error when using external filter
			  TES 5/15/2008 - PLID 28917 - Fixed the DiscountCategoryID to pull the CouponID where appropriate, so it
			  can filter on them.  Also changed the ADD_REPORT struct to include Coupons in the external filter.
			  // (j.gruber 2009-03-25 17:10) - PLID 33823 - updated discount structure
			  */
		return _T(
			"SELECT "
			"PatientsT.PersonID AS PatID, "
			"LineItemT.Date AS TDate, "
			"LineItemT.InputDate AS IDate, "
			"ChargesT.DoctorsProviders AS ProvID, "
			"LineItemT.LocationID AS LocID, "
			"(CASE WHEN CPTCodeT.ID IS NOT NULL THEN ItemCode ELSE '' END) AS CPTCode, "
			"ServiceT.Name AS ServiceName, "
			"ChargesT.ServiceID AS ServiceID, "
			"(Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)),2)) as AmtBeforeDiscount, "
			" dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
			"Round(Convert(money,LineItemT.Amount * Quantity),2) AS OrigAmount, "
			"ChargeDiscountsT.[PercentOff], "
			"ChargeDiscountsT.[Discount], "
			"Round(Convert(money,LineItemT.Amount * Quantity),2) - "
			"Round(Convert(money, LineItemT.Amount * Quantity "
			"* (CASE WHEN(ChargeDiscountsT.[PercentOff] Is Null) THEN 1 "
			"ELSE ((100-Convert(float,ChargeDiscountsT.[PercentOff]))/100) END) "
			"-(CASE WHEN ChargeDiscountsT.[Discount] Is Null THEN 0 "
			"ELSE ChargeDiscountsT.[Discount] END)),2) AS TotalDiscount, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"Quantity, "
			"(CASE WHEN DoctorsT.Last Is Null THEN '<No Provider>' ELSE DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle END) AS DocName, "
			"ServiceT.ProcedureID AS ProcID, "
			"(CASE WHEN ProcedureT.Name Is Null THEN '<No Procedure>' ELSE ProcedureT.Name END) AS ProcedureName, "
			"CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (-1 * ChargeDiscountsT.CouponID) ELSE ChargeDiscountsT.DiscountCategoryID END AS DiscountCategoryID, "
			"CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL "
			"THEN '<No Discount Category>' "
			"ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 "
			"THEN ChargeDiscountsT.CustomDiscountDesc "
			"ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 "
			"THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) "
			"ELSE (SELECT Description "
			"FROM DiscountCategoriesT "
			"WHERE ID = ChargeDiscountsT.DiscountCategoryID) "
			"END "
			"END "
			"END AS DiscountCategoryDescription, "
			"ChargesT.ID AS ChargeID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT AS DoctorsT ON ChargesT.DoctorsProviders = DoctorsT.ID "
			"LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
			"WHERE LineItemT.Deleted = 0 "
			"AND BillsT.Deleted = 0 "
			"AND BillsT.EntryType = 1 "
			"AND LineItemT.Type = 10 "
			" AND ChargeDiscountsT.Deleted = 0");
		break;

	case 369:
	{
		//Financial Activity by Superbill ID
		/*	Version History
		DRT 2/14/03 - Created - based off the Daily Financial Activity report
		DRT 4/22/03 - Removed the GetDateFilter() code, it was just causing problems and was not always functioning correctly.
		DRT 4/8/2004 - PLID 11816 - Replaced a LineItemT.* with appropriate fields
		(e.lally 2007-08-07) PLID 25116 - Replaced Refunds with cash/check/credit refund breakdowns in the subqueries
		(d.thompson 2009-03-04) - PLID 33119 - Added a GROUP BY element of LineID to the DFSQ subquery.  Our previous
		grouping did not ensure uniqueness, and it was possible to filter out legitimate records, if you had 100%
		identical (including to-the-second dates) records.  However, I do not believe it was possible to actually
		notice a problem with the report as it exists now, I'm only future-proofing in case more fields are added.
		// (j.gruber 2009-03-25 17:13) - PLID 33359 - updated discount structure
		// (j.jones 2011-07-27 08:53) - PLID 44708 - fixed so only one field is cast AS ProvID
		// (r.gonet 2015-05-20 19:51) - PLID 65903 - Added RefundGC to the select list.
		*/
		switch (nSubLevel) {
			//daily financial copy
		case 1:	//for the subreport
		case 2:	//used for totals
		{
			CString str1 = "SELECT DFSQ.FullName, DFSQ.UserDefinedID, DFSQ.PatID AS PatID, DFSQ.BDate AS BDate, DFSQ.TDate AS TDate, DFSQ.IDate AS IDate, DFSQ.DocName, DFSQ.DoctorID AS ProvID, DFSQ.ADJUSTMENT,  "
				"DFSQ.BILL AS BILL, DFSQ.CASH, DFSQ.CHECKAMT AS 'CHECK', DFSQ.CREDIT, DFSQ.GCSOLD, DFSQ.GCREDEEMED, DFSQ.RefundGC, DFSQ.AccountBal, DFSQ.LocID AS LocID, DFSQ.Location, "
				"DFSQ.InputName AS InputName, DFSQ.SuperbillID, DFSQ.SuperbillPrintDate AS PrintDate, DFSQ.ApptDate AS ApptDate "
				"FROM  "
				"	(SELECT  DailyFinancialSubQ.FullName,  PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, DailyFinancialSubQ.BDate AS BDate, DailyFinancialSubQ.TDate AS TDate,  "
				"	DailyFinancialSubQ.IDate AS IDate, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName, DailyFinancialSubQ.DoctorID,  "
				"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'ADJUSTMENT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS ADJUSTMENT,  "
				"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS BILL,  "
				"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CASH' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CASH,  "
				"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CHECK' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CHECKAMT,  "
				"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CREDIT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CREDIT,  "
				"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCSOLD' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCSOLD,  "
				"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCREDEEMED' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCREDEEMED,  "
				"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundGC' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundGC,  "
				"	convert(money, PatBalQ.AccountBal) AS AccountBal, DailyFinancialSubQ.LocID, LocationsT.Name AS Location, DailyFinancialSubQ.InputName, DailyFinancialSubQ.SuperbillID,  "
				"	DailyFinancialSubQ.SuperbillPrintDate, DailyFinancialSubQ.ApptDate  "
				"	FROM (  "
				"	/* Patient Balance */    "
				"		(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,      "
				"		Payments = CASE     "
				"			WHEN [_PatPaysQ].[SumOfAmount] Is Null     "
				"			THEN 0     "
				"			ELSE [_PatPaysQ].[SumOfAmount]     "
				"			End,      "
				"		Charges = CASE     "
				"			WHEN [ChargeAmount] Is Null     "
				"			THEN 0     "
				"			ELSE [ChargeAmount]     "
				"			End,      "
				"		PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,      "
				"		PatientsT.PersonID AS ID,      "
				"		(CASE     "
				"			WHEN [ChargeAmount] Is Null     "
				"			THEN 0     "
				"			ELSE [ChargeAmount]     "
				"			End)-     "
				"		(CASE  "
				"			WHEN [_PatPaysQ].[SumOfAmount] Is Null  "
				"			THEN 0  "
				"			ELSE [_PatPaysQ].[SumOfAmount]  "
				"			End)+  "
				"		(CASE  "
				"			WHEN [_PatPrePaysQ].[SumOfAmount] Is Null  "
				"			THEN 0  "
				"			ELSE [_PatPrePaysQ].[SumOfAmount]  "
				"			End) AS AccountBal,  "
				"		ProvidersT.PersonID AS DoctorID,  "
				"		PrePayments = CASE  "
				"			WHEN [_PatPrePaysQ].[SumOfAmount] Is Null  "
				"			THEN 0  "
				"			ELSE [_PatPrePaysQ].[SumOfAmount]  "
				"			End  "
				" "
				"FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)    "
				"			ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID)    "
				"			LEFT JOIN    "
				"			/*Patient Payments*/  "
				"			(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,      "
				"			Sum(LineItemT.Amount) AS SumOfAmount,      "
				"			PatientsT.PersonID AS ID     "
				"			FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON    "
				"			PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID     "
				"			WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))     "
				"			GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID     "
				"			) AS _PatPaysQ    "
				"			/* End Patient Payments */   "
				"			ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN    "
				// (j.politis 2015-07-30 15:07) - PLID 66741 - We need to fix how we round tax 1 and tax 2
				// (r.goldschmidt 2016-02-04 14:23) - PLID 68022 - Financial reports that run dbo.GetChargeTotal will time out (daily financial reports)
				"			/* Patient Charges */   "
				"			(SELECT LineItemT.PatientID,     "
				"			Sum(ChargeRespT.Amount) AS ChargeAmount,      "
				"			PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName     "
				"			FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"			LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
				"			INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID     "
				"			WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))     "
				"			GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle     "
				"			) AS _PatChargesQ   "
				"			/* End Patient Charges */   "
				"			 ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN    "
				"			/* Prepays*/   "
				"			(SELECT FullName, Sum(Amount) AS SumofAmount, ID   "
				"			FROM   "
				"			(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,      "
				"			((CASE WHEN PrepayAppliedToQ.ID IS NULL THEN      "
				"			    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN MAX(LineItemT.Amount) ELSE MAX(LineItemT.Amount - PrepayAppliesQ.Amount) END)     "
				"			ELSE     "
				"			    MAX(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END)) AS Amount,   "
				"			PatientsT.PersonID AS ID   "
				"			FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)    "
				"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN AppliesT AS AppliesT_1 ON PaymentsT.ID = AppliesT_1.SourceID)   "
				"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID   "
				"			LEFT JOIN   "
				"			/* This will total everything applied to a prepayment */  "
				"			( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID   "
				"			FROM   "
				"			LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
				"			INNER JOIN AppliesT ON LineItemT.ID = AppliesT.DestID   "
				"			WHERE (LineItemT.Deleted = 0) and PaymentsT.Prepayment = 1   "
				"			GROUP BY AppliesT.DestID   "
				"			) PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID   "
				"			LEFT JOIN   "
				"			 "
				"/* This will total everything that the prepayment is applied to */     "
				"			( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID     "
				"			FROM    "
				"			LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID     "
				"			INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID     "
				"			WHERE LineItemT.Deleted = 0 and PaymentsT.Prepayment = 1   "
				"			GROUP BY AppliesT.SourceID     "
				"			) PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID     "
				"			/*end totalling applies to prepays */     "
				"			WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1)   "
				"			GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID, LineItemT.Type, LineItemT.ID, PrepayAppliedToQ.ID, PrepayAppliesQ.ID, LineItemT.Description   "
				"			HAVING (((LineItemT.Type)<4))   "
				"			) AS PrePaysQ   "
				"			GROUP BY FullName, ID) AS _PatPrePaysQ   "
				"			/* End Prepays */  "
				"			ON PatientsT.PersonID = [_PatPrePaysQ].ID     "
				"			) AS PatBalQ    "
				"			/* End Patient Balance */    "
				"			 INNER JOIN   ";

			//	(c.haag 2010-01-19 17:07) - PLID 36643 - Added "FinType IN ('APPPRE', 'APPTOPRE')". This is for consistency 
			// with other queries in calculating SumOfAmt. The APPTOPRE field is not actually used in the Financial Activity by
			// Superbill ID report file at this time.
			// (r.gonet 2015-05-05 14:38) - PLID 65903 - Added PayMethod 10 - Gift Certificate Refunds
			CString str2 = "			/* All Financial Query */  "
				"			(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    "
				"			PatientsT.PersonID,    "
				"			SumOfAmt = CASE    "
				"				WHEN AllFinancialQ.[Type]=10 OR FinType = 'PREPAYMENT' OR FinType IN ('APPPRE', 'APPTOPRE')  "
				"				THEN AllFinancialQ.[Amt]   "
				"				ELSE AllFinancialQ.[Amount]   "
				"				End,    "
				"			AllFinancialQ.FinType,    "
				"			AllFinancialQ.BDate,  "
				"			AllFinancialQ.TDate,    "
				"			AllFinancialQ.DoctorID,    "
				"			AllFinancialQ.InputDate AS IDate,    "
				"			AllFinancialQ.Description,    "
				"			AllFinancialQ.ApplyID,    "
				"			AllFinancialQ.LineID,   "
				"			AllFinancialQ.LocID,   "
				"			AllFinancialQ.PrePayment,   "
				"			AllFinancialQ.InputName,  "
				"			AllFinancialQ.SuperbillID, AllFinancialQ.SuperbillPrintDate, AllFinancialQ.ApptDate "
				"			FROM ((/*AllFinancialQ*/   "
				"			SELECT 'BILL' AS FinType,     "
				"						LineItemT.ID, LineItemT.PatientID, LineItemT.Type,     "
				"				[LineItemT].[Amount],     "
				"				LineItemT.Description, LineItemT.Date,     "
				"				LineItemT.InputDate, LineItemT.InputName,     "
				"				LineItemT.Deleted, LineItemT.DeleteDate,     "
				"				LineItemT.DeletedBy,  "
				"				BillsT.Date AS BDate,     "
				"			   LineItemT.Date AS TDate,  "
				"				0 AS PrePayment,     "
				"				Amt = dbo.GetChargeTotal(ChargesT.ID),     "
				"				ChargesT.DoctorsProviders AS DoctorID,     "
				"				'Bill' AS RText,     "
				"				0 AS ApplyID,     "
				"				LineItemT.ID AS LineID,    "
				"			LineItemT.LocationID AS LocID,    "
				"			LocationsT.Name AS Location, ChargesT.SuperbillID, PrintedSuperbillsT.PrintedOn AS SuperbillPrintDate, AppointmentsT.Date AS ApptDate "
				"			FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID LEFT JOIN (ChargesT LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID) ON LineItemT.ID = ChargesT.ID) LEFT JOIN [CPTModifierT] ON ChargesT.CPTModifier = [CPTModifierT].Number  LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number  "
				"				LEFT JOIN PrintedSuperbillsT ON ChargesT.SuperbillID = PrintedSuperbillsT.SavedID "
				"				LEFT JOIN AppointmentsT ON PrintedSuperbillsT.ReservationID = AppointmentsT.ID "
				"			WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))    "
				"			AND ChargesT.ServiceID NOT IN (SELECT ServiceID FROM GCTypesT) "
				"		UNION SELECT 'GCSOLD' AS FinType,     "
				"						LineItemT.ID, LineItemT.PatientID, LineItemT.Type,     "
				"				[LineItemT].[Amount],     "
				"				LineItemT.Description, LineItemT.Date,     "
				"				LineItemT.InputDate, LineItemT.InputName,     "
				"				LineItemT.Deleted, LineItemT.DeleteDate,     "
				"				LineItemT.DeletedBy,  "
				"				BillsT.Date AS BDate,     "
				"			   LineItemT.Date AS TDate,  "
				"				0 AS PrePayment,     "
				"				Amt = dbo.GetChargeTotal(ChargesT.ID),     "
				"				ChargesT.DoctorsProviders AS DoctorID,     "
				"				'Bill' AS RText,     "
				"				0 AS ApplyID,     "
				"				LineItemT.ID AS LineID,    "
				"			LineItemT.LocationID AS LocID,    "
				"			LocationsT.Name AS Location, ChargesT.SuperbillID, PrintedSuperbillsT.PrintedOn AS SuperbillPrintDate, AppointmentsT.Date AS ApptDate "
				"			FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID LEFT JOIN (ChargesT LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID) ON LineItemT.ID = ChargesT.ID) LEFT JOIN [CPTModifierT] ON ChargesT.CPTModifier = [CPTModifierT].Number  LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number  "
				"				LEFT JOIN PrintedSuperbillsT ON ChargesT.SuperbillID = PrintedSuperbillsT.SavedID "
				"				LEFT JOIN AppointmentsT ON PrintedSuperbillsT.ReservationID = AppointmentsT.ID "
				"			WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))    "
				"			AND ChargesT.ServiceID IN (SELECT ServiceID FROM GCTypesT) "
				"			 "
				"		UNION SELECT FinType = CASE WHEN LineItemT.Type = 1 THEN CASE WHEN    "
				"			     [PaymentsT].[PayMethod] = 1 THEN 'CASH' "
				"				 WHEN [PaymentsT].[PayMethod] = 2 THEN 'CHECK'    "
				"			     WHEN [PaymentsT].[PayMethod] = 3 THEN 'CREDIT' "
				"				 WHEN [PaymentsT].[PayMethod] = 4 THEN 'GCREDEEMED' END "
				"				 WHEN LineItemT.Type = 2 THEN 'ADJUSTMENT' "
				"				 WHEN LineItemT.Type = 3 THEN "
				"					CASE WHEN PaymentsT.Paymethod = 7 THEN 'RefundCash'    "
				"					WHEN PaymentsT.Paymethod = 8 THEN 'RefundCheck'    "
				"					WHEN PaymentsT.Paymethod = 9 THEN 'RefundCredit'   "
				"					WHEN PaymentsT.Paymethod = 10 THEN 'RefundGC'  END "
				"			     END, LineItemT.ID, LineItemT.PatientID, LineItemT.Type,     "
				"			     Amount = CASE WHEN _PartiallyAppliedPaysQ.ID Is Null THEN    "
				"				CASE WHEN LineItemT_1.ID Is Null THEN [LineItemT].[Amount]    "
				"				ELSE [AppliesT].[Amount] END ELSE [AppliesT].[Amount] END,     "
				"			    LineItemT.Description, LineItemT.Date,     "
				"			    LineItemT.InputDate AS IDate, LineItemT.InputName,     "
				"			    LineItemT.Deleted, LineItemT.DeleteDate,     "
				"			    LineItemT.DeletedBy,    "
				"			LineItemT.Date AS BDate,  "
				"			    LineItemT.Date AS TDate, PaymentsT.PrePayment,     "
				"			    LineItemT.Amount AS Amt,     "
				"			    DoctorID = CASE WHEN [DoctorsProviders] Is Null     "
				"			    THEN [ProviderID] ELSE [DoctorsProviders] END,     "
				"			    'Full' AS RText, AppliesT.ID AS ApplyID,     "
				"			    LineItemT.ID AS LineID,    "
				"			CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,    "
				"			LocationsT.Name, ChargesT.SuperbillID, PrintedSuperbillsT.PrintedOn AS SuperbillPrintDate, AppointmentsT.Date AS ApptDate "
				"			FROM (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT OUTER JOIN    "
				"			    PaymentsT ON     "
				"			    LineItemT.ID = PaymentsT.ID LEFT OUTER JOIN    "
				"			        (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,     "
				"			           SUM(AppliesT.Amount) AS ApplyAmt,     "
				"			           MIN([LineItemT_1].[Amount])     "
				"			           - SUM([AppliesT].[Amount]) AS Total,     "
				"			           LineItemT_1.PatientID    "
				"			      FROM LineItemT AS LineItemT_1 LEFT JOIN    "
				"			           (PaymentsT LEFT JOIN    "
				"			           (AppliesT LEFT JOIN    "
				"			           LineItemT ON AppliesT.DestID = LineItemT.ID) ON     "
				"			           PaymentsT.ID = AppliesT.SourceID) ON     "
				"			           LineItemT_1.ID = PaymentsT.ID    "
				"			      WHERE (((LineItemT_1.Deleted) = 0)) AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT_1.GiftID, -1))    "
				"			      GROUP BY LineItemT_1.ID, LineItemT_1.Amount,     "
				"			           LineItemT_1.PatientID    "
				"			      HAVING (((LineItemT_1.ID) IS NOT NULL) AND     "
				"			           ((MIN([LineItemT_1].[Amount])     "
				"			           - SUM([AppliesT].[Amount])) <> 0)))     "
				"			    _PartiallyAppliedPaysQ ON     "
				"			    LineItemT.ID = _PartiallyAppliedPaysQ.ID LEFT OUTER JOIN    "
				"			    AppliesT ON     "
				"			    PaymentsT.ID = AppliesT.SourceID LEFT OUTER JOIN    "
				"			    LineItemT LineItemT_1 ON     "
				"			    AppliesT.DestID = LineItemT_1.ID LEFT OUTER JOIN    "
				"			    ChargesT ON LineItemT_1.ID = ChargesT.ID LEFT JOIN PrintedSuperbillsT ON ChargesT.SuperbillID = PrintedSuperbillsT.SavedID "
				"				LEFT JOIN AppointmentsT ON PrintedSuperbillsT.ReservationID = AppointmentsT.ID "
				"			WHERE (LineItemT.ID IS NOT NULL) AND (LineItemT.Deleted = 0) AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT.GiftID, -1))     "
				"			    AND (PaymentsT.ID IS NOT NULL)    ";

			CString str3 = "			UNION SELECT FinType =  "
				"					CASE  "
				"					WHEN LineItemT.Type=1 THEN  "
				"						CASE  "
				"						WHEN [PaymentsT].[PayMethod]=1  "
				"						THEN 'CASH'  "
				"						WHEN [PaymentsT].[PayMethod]=2  "
				"						THEN 'CHECK'  "
				"						WHEN [PaymentsT].[PayMethod]=3  "
				"						THEN 'CREDIT'  "
				"						WHEN [PaymentsT].[PayMethod]=4  "
				"						THEN 'GCREDEEMED'  "
				"						End  "
				"					WHEN LineItemT.Type=2  "
				"					THEN 'ADJUSTMENT'  "
				"					WHEN LineItemT.Type=3  THEN"
				"						CASE "
				"						WHEN PaymentsT.PayMethod=7  "
				"						THEN 'RefundCash'  "
				"						WHEN PaymentsT.PayMethod=8  "
				"						THEN 'RefundCheck'  "
				"						WHEN PaymentsT.PayMethod=9  "
				"						THEN 'RefundCredit'  "
				"						WHEN PaymentsT.PayMethod=10 "
				"						THEN 'RefundGC'  "
				"						End  "
				"					End,  "
				"			[_PartiallyAppliedPaysQ].ID, LineItemT.PatientID, LineItemT.Type, [_PartiallyAppliedPaysQ].Total AS Amount, LineItemT.Description, LineItemT.Date,  "
				"			LineItemT.InputDate AS IDate, LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, LineItemT.Date AS BDate,  "
				"			LineItemT.Date AS TDate, PaymentsT.PrePayment, LineItemT.Amount AS Amt, PaymentsT.ProviderID AS DoctorID, 'Part' AS RText, 0 AS ApplyID, LineItemT.ID AS LineID,  "
				"			LineItemT.LocationID AS LocID, LocationsT.Name AS Location, NULL AS SuperbillID, NULL AS SuperbillPrintDate, NULL AS ApptDate "
				" "
				"			FROM (((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,  "
				"			Sum(AppliesT.Amount) AS ApplyAmt,  "
				"			/*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,  "
				"			LineItemT_1.PatientID  "
				"			FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID  "
				"			WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT_1.GiftID, -1))  "
				"			GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
				"			HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))  "
				"			)  "
				"			 AS _PartiallyAppliedPaysQ RIGHT OUTER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineItemT.ID)  "
				"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID)  "
				"			WHERE ((([_PartiallyAppliedPaysQ].ID) Is Not Null) AND ((LineItemT.Deleted)=0)) AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT.GiftID, -1))  "
				"			/* Modified 1/24/03 - Now shows ALL things that have been prepayments, not things that are currently prepayments */ "
				" ";

			//removed "PREPAY" and "APPRE" sections
			CString str4 =
				"			 "
				"			) AS AllFinancialQ   "
				"			/* End Financial Query */  "
				"			LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON AllFinancialQ.PatientID = PatientsT.PersonID) LEFT JOIN LineItemT ON AllFinancialQ.ID = LineItemT.ID   "
				"			) AS DailyFinancialSubQ ON PatBalQ.ID = DailyFinancialSubQ.PersonID LEFT JOIN PatientsT ON DailyFinancialSubQ.PersonID = PatientsT.PersonID) LEFT JOIN PersonT ON DailyFinancialSubQ.DoctorID = PersonT.ID LEFT JOIN LocationsT ON DailyFinancialSubQ.LocID = LocationsT.ID   "
				"			GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, DailyFinancialSubQ.DoctorID, DailyFinancialSubQ.FullName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, DailyFinancialSubQ.BDate, DailyFinancialSubQ.TDate, DailyFinancialSubQ.IDate, PatBalQ.AccountBal, DailyFinancialSubQ.LocID, LocationsT.Name, DailyFinancialSubQ.InputName, DailyFinancialSubQ.SuperbillID, DailyFinancialSubQ.SuperbillPrintDate, DailyFinancialSubQ.ApptDate, DailyFinancialSubQ.LineID) AS DFSQ "
				"			WHERE SuperbillID IS NOT NULL AND SuperbillID > 0";
			return _T(str1 + str2 + str3 + str4);
		}

		default:
		{
			//main query
			CString str = "SELECT PrintedOn AS PrintDate, PrintedSuperbillsT.PatientID AS PatID, ReservationID, SavedID, Void, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				"AS Name, AppointmentsT.Date AS ApptDate FROM PrintedSuperbillsT LEFT JOIN PersonT ON PrintedSuperbillsT.PatientID = PersonT.ID "
				"LEFT JOIN AppointmentsT ON PrintedSuperbillsT.ReservationID = AppointmentsT.ID "
				"WHERE SavedID > 0";

			return _T(str);
		}
		break;
		}
	}
	break;
		
	case 386:
		//Printed Statements
		// (j.gruber 2008-06-09 08:38) - PLID 29398 - added "run" to the filter
		// (j.jones 2008-09-05 10:19) - PLID 30288 - supported MailSentNotesT
		return _T("SELECT MailSent.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"MailSentNotesT.Note, MailSent.Selection, MailSent.Sender, MailSent.Date AS TDate, MailSent.Location AS LocID, PatientsT.MainPhysician AS ProvID  "
			"FROM MailSent "
			"INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
			"LEFT JOIN PersonT ON MailSent.PersonID = PersonT.ID  "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE MailSentNotesT.Note Like '%Patient Statement%Printed%' OR MailSentNotesT.Note Like '%Patient Statement%Run%' OR MailSentNotesT.Note Like '%E-Statement %Exported%'");
		break;

	case 426:
		//Inventory Sales Profit
		/*	Version History
		DRT 8/1/03 - Created (PLID 9053)
		TES 2/18/2008 - PLID 28952 - Took the adjustments out of the Applied Amount, and made a new Applied
		Adjustments field.
		// (j.gruber 2009-03-25 17:15) - PLID 33359 - updated discount structure
		// (b.savon 2012-06-06 10:26) - PLID 50755 - Filter out deleted bills and handle financial corrections
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		*/
		return _T("SELECT "
			"PatientsT.UserDefinedID, PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargedAmountWithTax, "
			"Round(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))) "
			"	)),2) AS ChargedAmountNoTax,  "
			"Round(Convert(money, "
			"	((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)) + "
			"	((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)) "
			"	),2) AS TaxAmt,  "
			"ChargesT.Quantity, LastCostPerUU AS LastCost, "
			"Round(Convert(money,LastCostPerUU * ChargesT.Quantity),2) AS CostToPractice,  "
			"CASE WHEN AppliedPaysQ.AppliedToID IS NULL THEN 0 ELSE AppliedPaysQ.ApplyAmt END AS AppliedAmount, CASE WHEN AppliedAdjsQ.AppliedToID IS NULL THEN 0 ELSE AppliedAdjsQ.ApplyAmt END AS AppliedAdjustments, ServiceT.Name AS ProductName, LineChargesT.LocationID AS LocID,  "
			"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, LineChargesT.Date AS TDate, LineChargesT.InputDate AS IDate, "
			"PersonCoord.Last + ', ' + PersonCoord.First + ' ' + PersonCoord.Middle AS CoordName, "
			" "
			"/*IDs for Filters*/ "
			"ServiceT.ID AS ServiceID, CategoriesT.ID AS CategoryID, MultiSupplierT.SupplierID AS SupplierID, PersonCoord.ID AS CoordID,  "
			"PersonProv.ID AS ProvID,  "
			"/*For Custom Fields*/ "
			"ProductT.Notes AS ProductNotes, ProductT.UnitDesc AS ProductDescription,  "
			"PersonSupT.Company AS SupplierName, CategoriesT.Name AS CategoryName, LocationsT.Name AS LocName, ChargesT.TaxRate, ChargesT.TaxRate2 "
			"FROM "
			"ChargesT INNER JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID "
			"LEFT JOIN PersonT PersonProv ON ChargesT.DoctorsProviders = PersonProv.ID "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN PersonT PersonCoord ON ChargesT.PatCoordID = PersonCoord.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID "
			"LEFT JOIN  "
			"	(SELECT ChargesT.ID AS AppliedToID, Sum(AppliesT.Amount) AS ApplyAmt "
			"	FROM "
			"	ChargesT INNER JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID "
			"	INNER JOIN AppliesT ON LineChargesT.ID = AppliesT.DestID "
			"	INNER JOIN LineItemT LinePaysT ON AppliesT.SourceID = LinePaysT.ID "
			"	WHERE LineChargesT.Type = 10 AND LineChargesT.Deleted = 0 "
			"   AND LinePaysT.Type = 1 "
			"	GROUP BY ChargesT.ID "
			"	) AppliedPaysQ "
			"ON ChargesT.ID = AppliedPaysQ.AppliedToID "
			"LEFT JOIN  "
			"	(SELECT ChargesT.ID AS AppliedToID, Sum(AppliesT.Amount) AS ApplyAmt "
			"	FROM "
			"	ChargesT INNER JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID "
			"	INNER JOIN AppliesT ON LineChargesT.ID = AppliesT.DestID "
			"	INNER JOIN LineItemT LinePaysT ON AppliesT.SourceID = LinePaysT.ID "
			"	WHERE LineChargesT.Type = 10 AND LineChargesT.Deleted = 0 "
			"   AND LinePaysT.Type = 2 "
			"	GROUP BY ChargesT.ID "
			"	) AppliedAdjsQ "
			"ON ChargesT.ID = AppliedAdjsQ.AppliedToID "
			"INNER JOIN (   "
			"			SELECT DISTINCT BillsT.ID AS BillID   "
			"			FROM BillsT   "
			"			LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID   "
			"			INNER JOIN (   "
			"				SELECT DISTINCT BillID   "
			"				FROM   "
			"				LineItemT   "
			"				INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID   "
			"				WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10  "
			"			) ChargesSubQ ON BillsT.ID = ChargesSubQ.BillID   "
			"			WHERE   "
			"			BillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL   "
			"			AND BillsT.EntryType = 1   "
			"		) AS ValidBillsQ ON ChargesT.BillID = ValidBillsQ.BillID "
			"LEFT JOIN PersonT ON LineChargesT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT PersonSupT ON MultiSupplierT.SupplierID = PersonSupT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"WHERE LineChargesT.Deleted = 0 AND LineChargesT.Type = 10 ");
		break;

	case 439:
		//Inventory Sales by Supplier
		/*
		Revision History:
		- TES 8/8/03: Created, copied off Inventory Sales
		- TES 1/27/04 - Added "AS PatID" so we could Use Filter.
		- TES 2/18/03 - Added ProductTotalsQ with number in stock and on order.  Also modified the joins so that
		all products appear, regardless of whether or not they were sold.
		// (j.jones 2007-11-28 11:25) - PLID 28196 - completed Allocations now decrement from Inventory
		// (j.jones 2007-12-17 11:41) - PLID 27988 - billed allocations do not decrement from inventory
		//(e.lally 2008-10-07) PLID 31420 - Added support for multiple locations
		// (j.gruber 2009-03-25 17:20) - PLID 33359 - updated for discount structure
		*/
		if (nLocation != -1) {
			CString strLocationFilter;
			//(e.lally 2008-10-07) PLID 31420 - Add support for multiple locations
			if (nLocation == -2) {
				strLocationFilter = " IS NULL ";
			}
			else if (nLocation == -3) {
				strLocationFilter.Format(" IN (");
				CString strPart;
				for (int i = 0; i < m_dwLocations.GetSize(); i++) {
					strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
					strLocationFilter += strPart;
				}
				strLocationFilter = strLocationFilter.Left(strLocationFilter.GetLength() - 2) + ") ";
			}
			else {
				strLocationFilter.Format(" = %li ", nLocation);
			}

			CString strSql;
			// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
			strSql.Format("SELECT  "
				"LineItemT1.ID, LineItemT1.PatientID AS PatID, ServiceT.ID AS ItemID, ServiceT.Name AS ItemName,  "
				"COALESCE(dbo.GetChargeTotal(ChargesT.ID),convert(money,0)) AS ChargeAmount,  "
				"COALESCE(Round(Convert(money, ((LineItemT1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate-1)) "
				"	),2),convert(money,0)) AS Tax1Amount, "
				"COALESCE(Round(Convert(money, ((LineItemT1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate2-1)) "
				"	),2),convert(money,0)) AS Tax2Amount, "
				"Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS PayAmt,   "
				"Sum(CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS AdjAmt,   "
				"Sum(CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END) AS RefAmt,   "
				"LineItemT1.Date AS TDate, LineItemT1.InputDate AS IDate, ServiceT.Price AS UnitCost, LineItemT1.LocationID AS LocID, COALESCE(ChargesT.Quantity,0) AS Quantity,  "
				"PatientsT.EmployeeID AS CoordID, CASE WHEN CoordPersonT.ID Is Null THEN '' ELSE CoordPersonT.Last + ', ' + CoordPersonT.First + ' ' + CoordPersonT.Middle END AS Coordinator, "
				"ChargesT.TaxRate, ChargesT.TaxRate2, "
				"MultiSupplierT.SupplierID AS SupplierID, PersonSupplier.Company AS SupplierName, COALESCE(ProductTotalsQ.NumInStock,0) AS NumInStock, COALESCE(ProductTotalsQ.NumOnOrder,0) AS NumOnOrder "
				"FROM  "
				"ProductT LEFT JOIN ChargesT ON ProductT.ID = ChargesT.ServiceID LEFT JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"LEFT JOIN PatientsT ON LineItemT1.PatientID = PatientsT.PersonID "
				"LEFT JOIN PersonT CoordPersonT ON PatientsT.EmployeeID = CoordPersonT.ID "
				"	LEFT JOIN PersonT PersonSupplier ON MultiSupplierT.SupplierID = PersonSupplier.ID "
				"	LEFT JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
				"	LEFT JOIN AppliesT ON LineItemT1.ID = AppliesT.DestID "
				"	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
				"	LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
				"LEFT JOIN (SELECT ServiceT.ID, "
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
				"THEN 0 ELSE TransferFromSubQ.TransferFromQty END) AS NumInStock, CASE WHEN OnOrderSubQ.QuantityOrdered Is Null THEN 0 ELSE OnOrderSubQ.QuantityOrdered END AS NumOnOrder "
				"FROM ServiceT INNER JOIN  "
				"ProductT ON ServiceT.ID = ProductT.ID LEFT OUTER JOIN  "
				"/*Items Charged */  "
				// (j.dinatale 2011-11-07 09:52) - PLID 46226 - need to exclude voiding and original line items of financial corrections
				"				(SELECT ServiceID, SUM(CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS ChargeQuantity   "
				"  				FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
				"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "

				// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
				"LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
				"	INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
				"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
				"	WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li "
				"	GROUP BY ChargeID) AS AllocationInfoQ ON ChargesT.ID = AllocationInfoQ.ChargeID "
				"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Type = 10 AND LineItemT.LocationID %s "
				"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
				"AND (BillsT.ID NOT IN (SELECT BillID FROM BilledCaseHistoriesT) OR ChargesT.ServiceID NOT IN "
				"(SELECT ItemID FROM CaseHistoryDetailsT "
				"INNER JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID "
				"WHERE BilledCaseHistoriesT.BillID = BillsT.ID "
				"AND ItemType = -1)) "
				"GROUP BY ServiceID) ChargeSubQ ON   "
				"ProductT.ID = ChargeSubQ.ServiceID LEFT OUTER JOIN  "
				"/* Items received */  "
				" 				(SELECT ProductID, SUM(QuantityOrdered)    "
				"       				AS QuantityOrdered   "
				"  				FROM OrderDetailsT   "
				"  				WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived IS NOT NULL) AND OrderDetailsT.OrderID In (SELECT ID FROM OrderT WHERE LocationID %s)  "
				"  				GROUP BY ProductID "
				") ReceivedSubQ ON   "
				"ProductT.ID = ReceivedSubQ.ProductID  "
				"LEFT OUTER JOIN  "
				"/* Items on order*/  "
				" 				(SELECT ProductID, SUM(QuantityOrdered)    "
				"       				AS QuantityOrdered   "
				"  				FROM OrderDetailsT   "
				"  				WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived IS NULL) AND OrderDetailsT.OrderID IN (SELECT ID FROM OrderT WHERE LocationID %s)  "
				"  				GROUP BY ProductID "
				") OnOrderSubQ ON   "
				"ProductT.ID = OnOrderSubQ.ProductID  "
				"LEFT OUTER JOIN  "
				"/* Items Adjusted */   "
				" 				(SELECT ProductID, SUM(Quantity) AS AdjQuantity   "
				"  				FROM ProductAdjustmentsT  "
				"WHERE ProductAdjustmentsT.LocationID %s  "
				"  				GROUP BY ProductID) AdjSubQ ON   "
				"ProductT.ID = AdjSubQ.ProductID LEFT OUTER JOIN  "
				"/* Items On Completed Case Histories */   "
				" 				(SELECT ItemID, SUM(Quantity) AS CaseHistQuantity   "
				"  				FROM CaseHistoryDetailsT INNER JOIN "
				"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
				"WHERE ItemType = -1 AND CompletedDate Is Not Null AND CaseHistoryT.LocationID %s "
				"GROUP BY ItemID) CaseHistSubQ ON   "
				"ProductT.ID = CaseHistSubQ.ItemID "
				"/*Items Transferred To This Location*/ "
				"LEFT OUTER JOIN "
				"(SELECT SUM(Amount) AS TransferToQty, ProductID  "
				"					FROM ProductLocationTransfersT  "
				"					WHERE DestLocationID %s  "
				"					GROUP BY ProductID "
				") AS TransferToSubQ ON ProductT.ID = TransferToSubQ.ProductID "
				"/*Items Transferred From This Location*/ "
				"LEFT OUTER JOIN "
				"(SELECT SUM(Amount) AS TransferFromQty, ProductID  "
				"					FROM ProductLocationTransfersT  "
				"					WHERE SourceLocationID %s  "
				"					GROUP BY ProductID "
				") AS TransferFromSubQ ON ProductT.ID = TransferFromSubQ.ProductID "
				"LEFT OUTER JOIN ( "
				"SELECT Sum(Quantity) AS UsedAllocationQty, ProductID "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"WHERE PatientInvAllocationsT.Status = %li "
				"AND PatientInvAllocationDetailsT.Status = %li "
				"AND LocationID %s "
				"GROUP BY ProductID "
				") AS CompletedAllocationsQ ON ProductT.ID = CompletedAllocationsQ.ProductID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"LEFT JOIN PersonT ON PersonT.ID = MultiSupplierT.SupplierID "
				//(e.lally 2008-10-07)PLID 31420 - Doing a join on ProductLocationInfoT here will duplicate our totals per location, we can make sure the ProductID fits the 
				//criteria just like the all locations query does.
				"WHERE ProductT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE ProductLocationInfoT.LocationID %s AND ProductLocationInfoT.TrackableStatus = 2 AND ServiceT.Active = 1) "
				") ProductTotalsQ ON ProductT.ID = ProductTotalsQ.ID "
				//(e.lally 2008-10-07)PLID 31420 Doing a join on ProductLocationInfoT here will duplicate our totals per location, we can make sure the ProductID fits the 
				//criteria just like the all locations query does.
				"WHERE (LineItemT1.ID IS Null OR (LineItemT1.Deleted = 0 AND LineItemT1.Type = 10)) AND (LineItemT.Deleted Is Null OR LineItemT.Deleted = 0) "
				"AND ProductT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE (Billable = 1 OR TrackableStatus = 2) AND ProductLocationInfoT.LocationID %s) "
				"				GROUP BY  "
				"LineItemT1.ID, ChargesT.ID, LineItemT1.PatientID, ServiceT.ID, ServiceT.Name,  "
				"LineItemT1.Amount, Quantity, ChargesT.TaxRate, ChargesT.TaxRate2, CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4, "
				"LineItemT1.Date, LineItemT1.InputDate, ServiceT.Price, LineItemT1.LocationID, ChargesT.Quantity, PatientsT.EmployeeID, CoordPersonT.ID, CoordPersonT.First, CoordPersonT.Middle, CoordPersonT.Last, ChargesT.TaxRate, ChargesT.TaxRate2, MultiSupplierT.SupplierID, PersonSupplier.Company, ProductTotalsQ.NumInStock, ProductTotalsQ.NumOnOrder",
				InvUtils::iadsUsed, InvUtils::iasCompleted, strLocationFilter, strLocationFilter, strLocationFilter, strLocationFilter, strLocationFilter, strLocationFilter, strLocationFilter, InvUtils::iasCompleted, InvUtils::iadsUsed, strLocationFilter, strLocationFilter, strLocationFilter);
			return _T(strSql);

		}
		else {
			CString strSql;
			// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
			strSql.Format("SELECT  "
				"LineItemT1.ID, LineItemT1.PatientID AS PatID, ServiceT.ID AS ItemID, ServiceT.Name AS ItemName,  "
				"COALESCE(dbo.GetChargeTotal(ChargesT.ID),convert(money,0)) AS ChargeAmount,  "
				"COALESCE(Round(Convert(money, ((LineItemT1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate-1)) "
				"	),2),convert(money,0)) AS Tax1Amount, "
				"COALESCE(Round(Convert(money, ((LineItemT1.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(TaxRate2-1)) "
				"	),2),convert(money,0)) AS Tax2Amount, "
				"Sum(CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS PayAmt,   "
				"Sum(CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS AdjAmt,   "
				"Sum(CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END) AS RefAmt,   "
				"LineItemT1.Date AS TDate, LineItemT1.InputDate AS IDate, ServiceT.Price AS UnitCost, LineItemT1.LocationID AS LocID, COALESCE(ChargesT.Quantity,0) AS Quantity,  "
				"PatientsT.EmployeeID AS CoordID, CASE WHEN CoordPersonT.ID Is Null THEN '' ELSE CoordPersonT.Last + ', ' + CoordPersonT.First + ' ' + CoordPersonT.Middle END AS Coordinator, "
				"ChargesT.TaxRate, ChargesT.TaxRate2, "
				"MultiSupplierT.SupplierID AS SupplierID, PersonSupplier.Company AS SupplierName, COALESCE(ProductTotalsQ.NumInStock,0) AS NumInStock, COALESCE(ProductTotalsQ.NumOnOrder,0) AS NumOnOrder "
				"FROM  "
				"ProductT LEFT JOIN ChargesT ON ProductT.ID = ChargesT.ServiceID LEFT JOIN LineItemT LineItemT1 ON ChargesT.ID = LineItemT1.ID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"LEFT JOIN PatientsT ON LineItemT1.PatientID = PatientsT.PersonID "
				"LEFT JOIN PersonT CoordPersonT ON PatientsT.EmployeeID = CoordPersonT.ID "
				"	LEFT JOIN PersonT PersonSupplier ON MultiSupplierT.SupplierID = PersonSupplier.ID  "
				"	LEFT JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
				"	LEFT JOIN AppliesT ON LineItemT1.ID = AppliesT.DestID "
				"	LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
				"	LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
				"LEFT JOIN (SELECT ServiceT.ID,  "
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
				"THEN 0 ELSE TransferFromSubQ.TransferFromQty END) AS NumInStock, "
				"CASE WHEN OnOrderSubQ.QuantityOrdered Is Null THEN 0 ELSE OnOrderSubQ.QuantityOrdered END AS NumOnOrder "
				"FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT OUTER JOIN  "
				"/*Items Charged */  "
				// (j.dinatale 2011-11-07 09:52) - PLID 46226 - need to exclude voiding and original line items of financial corrections
				"  				(SELECT ServiceID, SUM(CASE WHEN ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) < 0 THEN 0 ELSE ChargesT.Quantity - Coalesce(AllocationInfoQ.Quantity,0) END) AS ChargeQuantity   "
				"  				FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID "
				"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "

				// (j.jones 2009-08-06 09:54) - PLID 35120 - supported BilledCaseHistoriesT
				"LEFT JOIN (SELECT ChargeID, Sum(PatientInvAllocationDetailsT.Quantity) AS Quantity FROM ChargedAllocationDetailsT "
				"	INNER JOIN PatientInvAllocationDetailsT ON ChargedAllocationDetailsT.AllocationDetailID = PatientInvAllocationDetailsT.ID "
				"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
				"	WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li "
				"	GROUP BY ChargeID) AS AllocationInfoQ ON ChargesT.ID = AllocationInfoQ.ChargeID "
				"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Type = 10 "
				"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "
				"AND (BillsT.ID NOT IN (SELECT BillID FROM BilledCaseHistoriesT) OR ChargesT.ServiceID NOT IN "
				"(SELECT ItemID FROM CaseHistoryDetailsT "
				"INNER JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID "
				"WHERE BilledCaseHistoriesT.BillID = BillsT.ID "
				"AND ItemType = -1)) "
				"GROUP BY ServiceID) ChargeSubQ ON   "
				"ProductT.ID = ChargeSubQ.ServiceID LEFT OUTER JOIN  "
				"/* Items received */  "
				" 				(SELECT ProductID, SUM(QuantityOrdered)    "
				"       				AS QuantityOrdered   "
				"  				FROM OrderDetailsT   "
				"  				WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived IS NOT NULL)  "
				"  				GROUP BY ProductID "
				") ReceivedSubQ ON   "
				"ProductT.ID = ReceivedSubQ.ProductID  "
				" "
				"LEFT OUTER JOIN  "
				"/* Items on order*/  "
				" 				(SELECT ProductID, SUM(QuantityOrdered)    "
				"       				AS QuantityOrdered   "
				"  				FROM OrderDetailsT   "
				"  				WHERE (OrderDetailsT.Deleted = 0) AND (DateReceived IS NULL)  "
				"  				GROUP BY ProductID "
				") OnOrderSubQ ON   "
				"ProductT.ID = OnOrderSubQ.ProductID  "
				" "
				"LEFT OUTER JOIN  "
				"/* Items Adjusted */   "
				" 				(SELECT ProductID, SUM(Quantity) AS AdjQuantity   "
				"  				FROM ProductAdjustmentsT   "
				"  				GROUP BY ProductID) AdjSubQ ON   "
				"ProductT.ID = AdjSubQ.ProductID LEFT OUTER JOIN  "
				"/* Items On Completed Case Histories */   "
				" 				(SELECT ItemID, SUM(Quantity) AS CaseHistQuantity   "
				"  				FROM CaseHistoryDetailsT INNER JOIN "
				"CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
				"WHERE ItemType = -1 AND CompletedDate Is Not Null "
				"GROUP BY ItemID) CaseHistSubQ ON   "
				"ProductT.ID = CaseHistSubQ.ItemID "
				"/*Items Transferred To This Location*/ "
				"LEFT OUTER JOIN "
				"(SELECT SUM(Amount) AS TransferToQty, ProductID  "
				"					FROM ProductLocationTransfersT  "
				"					GROUP BY ProductID "
				") AS TransferToSubQ ON ProductT.ID = TransferToSubQ.ProductID "
				"/*Items Transferred From This Location*/ "
				"LEFT OUTER JOIN "
				"(SELECT SUM(Amount) AS TransferFromQty, ProductID  "
				"					FROM ProductLocationTransfersT  "
				"					GROUP BY ProductID "
				") AS TransferFromSubQ ON ProductT.ID = TransferFromSubQ.ProductID "
				"LEFT OUTER JOIN ( "
				"SELECT Sum(Quantity) AS UsedAllocationQty, ProductID "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
				"WHERE PatientInvAllocationsT.Status = %li "
				"AND PatientInvAllocationDetailsT.Status = %li "
				"GROUP BY ProductID "
				") AS CompletedAllocationsQ ON ProductT.ID = CompletedAllocationsQ.ProductID "
				"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"LEFT JOIN PersonT ON PersonT.ID = MultiSupplierT.SupplierID "
				"WHERE (ProductT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE Billable = 1 OR TrackableStatus = 2)) AND ServiceT.Active = 1) ProductTotalsQ ON ProductT.ID = ProductTotalsQ.ID "
				"WHERE (LineItemT1.ID IS Null OR (LineItemT1.Deleted = 0 AND LineItemT1.Type = 10)) AND (LineItemT.Deleted Is Null OR LineItemT.Deleted = 0) "
				"	GROUP BY  "
				"LineItemT1.ID, ChargesT.ID, LineItemT1.PatientID, ServiceT.ID, ServiceT.Name,  "
				"LineItemT1.Amount, Quantity, ChargesT.TaxRate, ChargesT.TaxRate2, CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4, "
				"LineItemT1.Date, LineItemT1.InputDate, ServiceT.Price, LineItemT1.LocationID, ChargesT.Quantity, PatientsT.EmployeeID, CoordPersonT.ID, CoordPersonT.First, CoordPersonT.Middle, CoordPersonT.Last, ChargesT.TaxRate, ChargesT.TaxRate2, MultiSupplierT.SupplierID, PersonSupplier.Company, ProductTotalsQ.NumInStock, ProductTotalsQ.NumOnOrder",
				InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iasCompleted, InvUtils::iadsUsed);

			return _T(strSql);
		}
		break;

	case 446:
		//Patient Financial History
		/*	Version History
		DRT 9/10/03 - Created.  Based off the Patient Financial History (PP) report.  The only difference I made
		was to comment out all sections that limited it to 1 patient.  I changed the report file a bit as well,
		but it still uses the same physical file as the PP report.  There is special parameter code in reports.cpp
		for this report.
		// (j.gruber 2007-05-29 14:37) - PLID 25979 - added discount and discount category information
		// (j.gruber 2009-04-02 11:00) - PLID 33814 - update discount structure
		// (j.gruber 2011-09-09 11:08) - PLID 45411 - fix voided pays not showing
		*/
	{
		strSQL = _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"   PatientsT.UserDefinedID,  "
			"   FinHistorySubQ.TopID,  "
			"   FinHistorySubQ.MiddleID,  "
			"   FinHistorySubQ.ItemLevel, "
			"   FinHistorySubQ.PatID AS PatID, "
			"   FinHistorySubQ.TDate AS TDate,  "
			"   FinHistorySubQ.Description,  "
			"   Coalesce(FinHistorySubQ.ChargeAmount, 0) AS ChargeAmount, "
			"   Coalesce(FinHistorySubQ.PayAmount, 0) AS PayAmount,  "
			"   Coalesce(FinHistorySubQ.AdjAmount, 0) AS AdjAmount,  "
			"   Coalesce(FinHistorySubQ.RefAmount, 0) AS RefAmount, "
			"   FinHistorySubQ.Type, "
			"   FinHistorySubQ.TopDate AS TopDate,  "
			"   FinHistorySubQ.ProvId, "
			"	FinHistorySubQ.ProvFirst, "
			"   FinHistorySubq.ProvMiddle, "
			"	FinHistorySubq.ProvLast,		"
			"	FinHistorySubQ.ProvTitle, PersonT.ID AS PatID, "
			"   FinHistorySubQ.PercentOff as PercentOff, "
			"   FinHistorySubQ.DiscountAmt as DiscountAmt, "
			"   FinHistorySubQ.DiscountCategoryDescription "

			"FROM (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) INNER JOIN ( "
			" "
			"   /*TS: TopID: ID of the top-level lineitem with which this is associated "
			"         MiddleID:  ID of the second-level LineItem (charge) with which this is associated (-1 for top level) "
			"         ItemLevel: Level of this item (1, 2, or 3)*/ "
			"    "
			"   /*Top level items (Bills, and payments with unapplied amounts remaining*/ "
			"    "
			"   /*Payments, Adjustments and Refunds with unapplied amounts remaining*/ "
			"   SELECT LineItemT.ID AS TopID,  "
			"      -1 AS MiddleID,  "
			"      1 AS ItemLevel,  "
			"      LineItemT.PatientID AS PatID,  "
			"      LineItemT.Date AS TDate,  "
			"      LineItemT.Date AS TopDate, "
			"      CASE WHEN LineItemT.Type = 1 THEN 'Payment - ' ELSE CASE WHEN LineItemT.Type = 2 THEN 'Adjustment - ' ELSE 'Refund - ' END END + LineItemT.Description AS Description,  "
			"      Convert(money, 0) AS ChargeAmount,  "
			"      CASE WHEN LineItemT.Type = 1 THEN LineItemT.Amount - CASE WHEN AppliesFromSubQ.Amount Is Null THEN 0 ELSE AppliesFromSubQ.Amount END ELSE 0 END AS PayAmount, "
			"      CASE WHEN LineItemT.Type = 2 THEN LineItemT.Amount - CASE WHEN AppliesFromSubQ.Amount Is Null THEN 0 ELSE AppliesFromSubQ.Amount END ELSE 0 END AS AdjAmount, "
			"      CASE WHEN LineItemT.Type = 3 THEN LineItemT.Amount - CASE WHEN AppliesFromSubQ.Amount Is Null THEN 0 ELSE AppliesFromSubQ.Amount END ELSE 0 END AS RefAmount, "
			"      LineItemT.Type,	"
			"	   Providerst.ID AS ProvID,	"
			"	   ProvidersT.First AS ProvFirst,  "
			"	   ProvidersT.Middle AS ProvMiddle, "
			"	   ProvidersT.last AS ProvLast,	"
			"	   ProvidersT.Title AS ProvTitle, "
			"	   LineItemT.ID As GroupFixId,  "
			"      0 as PercentOff, "
			"      Convert(money, 0) AS DiscountAmt, "
			"      '' AS DiscountCategoryDescription "
			"   FROM LineItemT LEFT JOIN ( "
			"         SELECT Sum(AppliesT.Amount) AS Amount,  "
			"            AppliesT.SourceID  "
			"         FROM AppliesT  "
			"         GROUP BY AppliesT.SourceID) AS AppliesFromSubQ ON LineItemT.ID = AppliesFromSubQ.SourceID "
			"	Left Join PaymentsT ON LineItemT.ID = PaymentsT.ID Left Join(SELECT ID, First, Last, Middle, Title  FROM PersonT WHERE ID IN (SELECT PersonID FROM ProvidersT)) AS ProvidersT ON  "
			"	PaymentsT.ProviderID = ProvidersT.ID  "
			"   WHERE /*LineItemT.PatientID = @PatientID@ AND*/ (LineItemT.Amount = 0 OR (LineItemT.Amount - CASE WHEN AppliesFromSubQ.Amount Is Null THEN 0 ELSE AppliesFromSubQ.Amount END <> 0)) AND (LineItemT.Type = 1 OR LineItemT.Type = 2 OR LineItemT.Type = 3) AND LineItemT.Deleted = 0 "
			"    "
			"   /*Bills, Bills, Bills*/ "
			"   UNION SELECT BillsT.ID AS TopID,  "
			"      -1 AS MiddleID,  "
			"      1 AS ItemLevel,  "
			"      BillsT.PatientID AS PatID,  "
			"      BillsT.Date AS TDate, "
			"      BillsT.Date AS TopDate, "
			"      'BILL - ' + BillsT.Description AS Description, "
			"      0 AS ChargeAmount, "
			"      0 AS PayAmount,  "
			"      0 AS AdjAmount, "
			"      0 AS RefAmount, "
			"      0 AS Type, "
			"	   0 AS ProvID,	"
			"	   '' AS ProvFirst,  "
			"	   '' AS ProvMiddle, "
			"	   '' AS ProvLast,	"
			"	   '' AS ProvTitle,  "
			"	   BillsT.Id AS GroupFixID,  "
			"      0 as PercentOff, "
			"      Convert(money, 0) AS DiscountAmt, "
			"      '' AS DiscountCategoryDescription "
			"   FROM BillsT "
			"   WHERE /*BillsT.PatientID = @PatientID@ AND*/ BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
			"    "
			"   /*Second level:  Charges, and refunds/adjustments applied to payments*/ "
			"    "
			"   /*Charges*/ "
			"   UNION SELECT ChargesT.BillID AS TopID, "
			"      LineItemT.ID AS MiddleID,  "
			"      2 AS ItemLevel, "
			"      LineItemT.PatientID AS PatID, "
			"      LineItemT.Date AS TDate, "
			"      (SELECT Date FROM BillsT WHERE ID = ChargesT.BillID) AS TopDate, "
			"      '   Charge - (' + ChargesT.ItemCode + ') ' + LineItemT.Description AS Description,  "
			"      dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,   "
			"      0 AS PayAmount, "
			"      0 AS AdjAmount,  "
			"      0 AS RefAmount, "
			"      LineItemT.Type, "
			"	   Providerst.ID AS ProvID,	"
			"	   ProvidersT.First AS ProvFirst,  "
			"	   ProvidersT.Middle AS ProvMiddle, "
			"	   ProvidersT.last AS ProvLast,	"
			"	   ProvidersT.Title AS ProvTitle, "
			"	   LineItemT.ID AS GroupFixID,  "
			"      TotalPercentOff, "
			"      TotalDiscount, "
			"	   dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription "

			"   FROM LineItemT INNER JOIN (ChargesT LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) ON LineItemT.ID = ChargesT.ID "
			"	LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
			"	LEFT JOIN (SELECT ID, first, Middle, Last, Title FROM PersonT WHERE ID IN (SELECT ID FROM ProvidersT)) AS ProvidersT ON    "
			"	ChargesT.DoctorsProviders = ProvidersT.ID  "
			"   WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 /*AND LineItemT.PatientID = @PatientID@*/ "
			"    "
			"   /*Refunds/Adjustments applied to payments*/ "
			"   UNION SELECT AppliesT.DestID AS TopID, "
			"      LineItemT.ID AS MiddleID, "
			"      2 AS ItemLevel,  "
			"      LineItemT.PatientID AS PatID, "
			"      LineItemT.Date AS TDate,  "
			"      (SELECT Date FROM LineItemT PayItems WHERE PayItems.ID = AppliesT.DestID) AS TopDate, "
			"      CASE WHEN LineItemT.Type = 1 THEN '   Payment - ' WHEN LineItemT.Type = 2 THEN '   Adjustment - ' ELSE '   Refund - ' END + LineItemT.Description AS Description,  "
			"      0 AS ChargeAmount,  "
			"	   CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END AS PayAmount,  "
			"      CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END AS AdjAmount,  "
			"      CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END AS RefAmount, "
			"      LineItemT.Type, "
			"	   Providerst.ID AS ProvID,	"
			"	   ProvidersT.First AS ProvFirst,  "
			"	   ProvidersT.Middle AS ProvMiddle, "
			"	   ProvidersT.last AS ProvLast,	"
			"	   ProvidersT.Title AS ProvTitle, "
			"	   AppliesT.ID As FixGroupID,  "
			"      0 as PercentOff, "
			"      Convert(money, 0) AS DiscountAmt, "
			"      '' AS DiscountCategoryDescription "
			"   FROM LineItemT INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"   LEFT JOIN (SELECT ID, First, Middle, Last, Title FROM PersonT WHERE ID IN (SELECT ID FROM ProvidersT)) AS ProvidersT ON   "
			"   PaymentsT.ProviderId = ProvidersT.ID   "
			"   WHERE (LineItemT.Type IN (1,2,3)) AND LineItemT.Deleted = 0 AND AppliesT.PointsToPayments = 1 /*AND LineItemT.PatientID = @PatientID@*/ "
			"       "
			"   /*Third Level:  Payments/Refunds/Adjustments applied to Charges*/ "
			"   UNION SELECT ChargesT.BillID AS TopID,  "
			"      ChargesT.ID AS MiddleID,  "
			"      3 AS ItemLevel, "
			"      LineItemT.PatientID AS PatID, "
			"      LineItemT.Date AS TDate, "
			"      (SELECT Date FROM BillsT WHERE BillsT.ID = ChargesT.BillID) AS TopDate, "
			"      '      ' + CASE WHEN InsuredPartyT.RespTypeID = 1 THEN '[Primary]' ELSE CASE WHEN InsuredPartyT.RespTypeID = 2 THEN '[Secondary]' ELSE CASE WHEN InsuredPartyT.RespTypeID = -1 THEN '[Inactive]' ELSE '' END END END + CASE WHEN LineItemT.Type = 1 THEN 'Payment - ' ELSE CASE WHEN LineItemT.Type = 2 THEN 'Adjustment - ' ELSE 'Refund - ' END END + LineItemT.Description AS Description, "
			"      0 AS ChargeAmount, "
			"      CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END AS PayAmount,  "
			"      CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END AS AdjAmount,  "
			"      CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END AS RefAmount, "
			"      LineItemT.Type, "
			"	   Providerst.ID AS ProvID,	"
			"	   ProvidersT.First AS ProvFirst,  "
			"	   ProvidersT.Middle AS ProvMiddle, "
			"	   ProvidersT.last AS ProvLast,	"
			"	   ProvidersT.Title AS ProvTitle, "
			"	   AppliesT.ID As GroupFixId,  "
			"      0 as PercentOff, "
			"      Convert(money, 0) AS DiscountAmt, "
			"      '' AS DiscountCategoryDescription "
			"   FROM ((LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN (AppliesT INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID) ON LineItemT.ID = AppliesT.SourceID "
			"	LEFT JOIN (SELECT ID, First, Middle, Last, Title FROM PErsonT WHERE ID IN (SELECT ID From ProvidersT)) AS ProvidersT "
			"	ON PaymentsT.ProviderID = ProvidersT.ID  "
			"   WHERE LineItemT.Deleted = 0 AND AppliesT.PointsToPayments = 0 /*AND LineItemT.PatientID = @PatientID@*/ "
			") AS FinHistorySubQ ON PersonT.ID = FinHistorySubQ.PatID");
		return strSQL;
	}
	break;

	case 447: {
		//Historical Deposit Slip
		/*	Version History
		DRT 9/11/03 - Created.  This is very similar to the deposit slip report, but it is historical and allows you to see
		exactly what was printed in previous reports.  It groups by date/time, so each individual printout will have its
		own page.
		DRT 4/19/2004 - PLID 11382 - Added tip information.
		JMM 5/18/2004 - PLID 12412 - Changed Tips to only separate them when they have a different payment type than the payment
		JMM 11/3/2005 - PLID 13548 - Added Refunds
		JMJ 10/11/2006 - PLID 22955 - Supported Adjusted Batch Payments by ignoring them
		// (j.gruber 2007-05-01 17:17) - PLID 25745 - only show the last 4 digits of the ccnumber
		(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType
		(d.thompson 2009-07-08) - PLID 17140 - Added PayCategoryID and PayCategoryName
		// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
		// (j.gruber 2010-10-22 14:56) - PLID 31187 - made it show cash tips correctly
		// (j.jones 2011-09-15 16:48) - PLID 45202 - we now hide void and corrected payments, only showing the originals
		// (j.jones 2012-04-19 14:23) - PLID 48032 - changed to use PaymentsT.BatchPaymentID
		// (b.spivey, July 24, 2012) - PLID 44450 - Added InputDate
		// (r.gonet 2015-05-01 14:59) - PLID 65870 - Do not include refunded gift certificates.
		*/

		CString str;
		CString strRefunds, strBatchRefunds;
		if (GetRemotePropertyInt("BankingIncludeRefunds", 1, 0, GetCurrentUserName(), true)) {
			strRefunds = "OR (LineItemT.Type = 3 AND PaymentsT.PayMethod IN (7,8,9))";
			strBatchRefunds = "Type <> 2 AND ";
		}
		else {
			strRefunds = "";
			strBatchRefunds = "Type = 1 AND ";
		}

		BOOL bIncludeTips = GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true);

		str.Format("SELECT  "
			"PatID, UserDefinedID, PatName, CheckNo, CCType, CASE WHEN Len(CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(CCNumber, 4) END as CCNumber, Amount, PayMethod,  "
			"BankName, BankRoutingNum, CheckAcctNo, SocialSecurity, TDate AS TDate, PayCategoryID AS PayCategoryID,  "
			"PayCategoryName, InputDate "
			"FROM "
			"	(SELECT  "
			"	/*Payment Info*/  "
			"	PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"	PaymentPlansT.CheckNo, CreditCardNamesT.CardName AS CCType, PaymentPlansT.CCNumber, "
			"   LineItemT.Amount + (SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END FROM PaymentTipsT WHERE PaymentID = PaymentsT.ID AND PaymentTipsT.PayMethod = PaymentsT.PayMethod AND PaymentTipsT.Paymethod <> 1) AS Amount, "
			"	PaymentsT.PayMethod,  "
			"	PaymentPlansT.BankNo AS BankName, PaymentPlansT.BankRoutingNum, PaymentPlansT.CheckAcctNo,  "
			"	dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, PaymentsT.DepositDate AS TDate, "
			"	PaymentsT.PaymentGroupID AS PayCategoryID, PaymentGroupsT.GroupName AS PayCategoryName, "
			"	LineItemT.InputDate AS InputDate	"
			"	FROM  "
			"	PatientsT LEFT JOIN LineItemT ON  "
			"	PatientsT.PersonID = LineItemT.PatientID INNER JOIN PaymentsT ON  "
			"	LineItemT.ID = PaymentsT.ID LEFT JOIN PaymentPlansT ON  "
			"	PaymentsT.ID = PaymentPlansT.ID "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"	LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"	WHERE (PatientsT.PersonID > 0) AND (PaymentsT.Deposited = 1) AND  "
			"	(LineItemT.Deleted = 0) AND (LineItemT.Type = 1 %s) AND Amount <> 0 "
			"	AND CorrectedLineItemsT.NewLineItemID Is Null AND VoidingLineItemsT.VoidingLineItemID Is Null "
			"	AND PaymentsT.BatchPaymentID Is Null AND PaymentsT.PayMethod NOT IN (4,10) "
			"	 "
			"	UNION ALL SELECT  "
			"	/*BatchPaymentInfo */  "
			"	NULL AS PatID, NULL AS UserDefinedID, Name, CheckNo, NULL AS CCType, NULL AS CCNumber, CASE WHEN Type <> 1 THEN -1 * Amount ELSE Amount END, CASE WHEN Type = 1 THEN 2 WHEN Type = 3 THEN 8 ELSE 0 END AS PayMethod,  "
			"	BankName, BankRoutingNum, CheckAcctNo, NULL AS SocialSecurity, DepositDate AS TDate, "
			"	BatchPaymentsT.PayCatID AS PayCategoryID, PaymentGroupsT.GroupName AS PayCategoryName, NULL AS InputDate "
			"	FROM BatchPaymentsT INNER JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID  "
			"	LEFT JOIN PaymentGroupsT ON BatchPaymentsT.PayCatID = PaymentGroupsT.ID "
			"	WHERE Deleted = 0 AND Deposited = 1 AND %s Amount <> 0 "
			"	UNION ALL  "
			"	SELECT /*Tips*/PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle + '  (Tip)' AS PatName,  "
			"	NULL AS CheckNo, NULL AS CCType, NULL AS CCNumber, PaymentTipsT.Amount, "
			"	PaymentTipsT.PayMethod, "
			"	NULL AS BankName, "
			"	NULL AS BankRoutingNum, NULL AS CheckAcctNo, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, PaymentTipsT.DepositDate, "
			"	NULL AS PayCategoryID, NULL AS PayCategoryName, LineItemT.InputDate AS InputDate "
			"	FROM PaymentTipsT INNER JOIN PaymentsT ON PaymentTipsT.PaymentID = PaymentsT.ID  "
			"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
			"	INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID  "
			"	INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
			"	LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID  "
			"	LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"	LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			"	WHERE PaymentTipsT.Deposited = 1 AND Deleted = 0 "
			"	AND CorrectedLineItemsT.NewLineItemID Is Null AND VoidingLineItemsT.VoidingLineItemID Is Null "
			"	AND (PaymentTipsT.Paymethod = 1 OR PaymentsT.PayMethod <> PaymentTipsT.PayMethod) %s "
			"	) UnionQ ", ((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"), strRefunds, strBatchRefunds,
			((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
			bIncludeTips ? "" : " AND PaymentTipsT.ID = -1"
			);
		return str;
	}
			  break;

	case 454:
		//Payer Mix by Charges
		/*	Version History
		DRT 10/20/2003 - PLID 7716 - Created.
		TES 7/27/2004 - PLID 13704 - Changed to sum the charge resp amount rather than the charge amount; the old way
		was causing charges to get counted multiple times.
		*/
		return _T("SELECT InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, LineItemT.LocationID AS LocID,  "
			"LocationsT.Name AS LocName, PersonT.ID AS ProvID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProvName,  "
			"ChargeRespT.Amount AS ChgAmount,  "
			"LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, BillsT.Date AS BillDate "
			"FROM "
			"BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT ON ChargesT.DoctorsProviders = PersonT.ID "
			"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number  "
			"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND InsuranceCoT.PersonID IS NOT NULL");
		break;

	case 455:
		//Payer Mix by Payments
		/*	Version History
		DRT 10/20/2003 - PLID 7716 - Created.
		*/
		return _T("SELECT InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, LineItemT.LocationID AS LocID,  "
			"LocationsT.Name AS LocName, PersonT.ID AS ProvID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProvName,  "
			"LineItemT.Amount AS PayAmt, LineItemT.Date AS TDate, LineItemT.InputDate AS IDate "
			"FROM "
			"PaymentsT "
			"LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT ON PaymentsT.ProviderID = PersonT.ID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 AND InsuranceCoT.PersonID IS NOT NULL");
		break;

	case 456:
		//Payer Mix by Patient
		/*	Version History
		DRT 10/20/2003 - PLID 7716 - Created.
		//j.gruber 2009-12-17 14:53 - PLID 33632 - fixed the provider filter
		*/
		return _T("SELECT InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, LocationsT.ID AS LocID,  "
			"LocationsT.Name AS LocName, PersonProv.ID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName,  "
			"PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PersonT.FirstContactDate AS FCDate "
			"FROM "
			"InsuredPartyT LEFT JOIN PersonT ON InsuredPartyT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"LEFT JOIN PersonT PersonProv ON PatientsT.MainPhysician = PersonProv.ID");
		break;

	case 460:
		//Payments Refunded/Adjusted Off
		/*	Version History
		DRT 11/5/2003 - PLID 9863 - Created.  Covers a gap between items applied (payments by cpt category) and things unapplied
		(unapplied credits) by showing items that have been refunded/adjusted off.  ONLY shows payments, not adj/refunds.
		*/
		return _T("SELECT LinePaysT.ID, LinePaysT.Type, LinePaysT.Description, LinePaysT.Date AS Date, LinePaysT.InputDate AS IDate,  "
			"LinePaysT.LocationID AS LocID, PaymentsT.ProviderID AS ProvID, LinePaysT.Amount AS OriginalAmt,  "
			"RemainingQ.RemovedAmt, CASE WHEN AppliedToChgQ.AppliedToChgAmt IS NULL THEN 0 ELSE AppliedToChgQ.AppliedToChgAmt END AS AppliedToChgAmt,  "
			"LinePaysT.Amount + RemovedAmt - CASE WHEN AppliedToChgQ.AppliedToChgAmt IS NULL THEN 0 ELSE AppliedToChgQ.AppliedToChgAmt END AS CurrentBalance,  "
			"LocationsT.Name AS LocName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID "
			"FROM "
			"LineItemT LinePaysT  "
			"INNER JOIN PaymentsT ON LinePaysT.ID = PaymentsT.ID "
			"/* Determine the Amount that has been adjusted/refunded off.  All items must be in here. */ "
			"INNER JOIN  "
			"	(SELECT LineItemT.ID AS PayID, Sum(AppliesT.Amount) AS RemovedAmt, LineItemT.Type "
			"	FROM AppliesT INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID "
			"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"	LEFT JOIN LineItemT LineSource ON AppliesT.SourceID = LineSource.ID "
			"	WHERE LineItemT.Deleted = 0 "
			"	GROUP BY LineItemT.ID, LineItemT.Type "
			"	) RemainingQ "
			"ON LinePaysT.ID = RemainingQ.PayID "
			"/* Determine the Amount that has been applied to charges.  Items are not required to be in here.*/ "
			"LEFT JOIN  "
			"	(SELECT LineItemT.ID AS PayID, Sum(AppliesT.Amount) AS AppliedToChgAmt "
			"	FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"	INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
			"	WHERE LineItemT.Deleted = 0 "
			"	GROUP BY LineItemT.ID "
			"	) AppliedToChgQ "
			"ON LinePaysT.ID = AppliedToChgQ.PayID "
			"LEFT JOIN LocationsT ON LinePaysT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT ON LinePaysT.PatientID = PersonT.ID "
			"LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE LinePaysT.Deleted = 0 AND LinePaysT.Type = 1 ");
		break;


	case 462:
		//Collection Rate By Category
		/*Version History
		JMM - 11/11/03 Created PLID 6553
		DRT 7/1/2005 - PLID 15573 - Added Location filter.  This filter is on the CHARGE location, and totals all
		payments and adjustments applied, regardless of their location status.
		// (s.tullis 2016-03-08 16:01) - PLID 68489 - Fixed an issue where UNION was combining Unique payments
		// (s.tullis 2016-03-09 09:43) - PLID 68491 - Collection By Ins Co and Collection by Service Category reports factor in gift certificate payments and charges into the report calculation
		*/
		return _T("SELECT CASE WHEN ServiceT.Category IS NULL THEN 0 ELSE ServiceT.Category END AS CategoryID,  "
			"CASE WHEN CategoriesT.Name IS NULL THEN 'No Category' ELSE CategoriesT.Name END AS CatName,  "
			"SUM(ChargesSubQ.ChargeAmount) AS SumCharges, SUM(PaySubQ.SumPays) AS SumPayments,  "
			"SUM(PaySubQ.SumAdjs) AS SumAdjustments, ChargesSubQ.DoctorsProviders As ProvID,  "
			"ChargesSubQ.Date As TDate, ChargesSubQ.InputDate As IDate, ChargesSubQ.LocationID AS LocID "
			"FROM  "
			"	(SELECT SUM(ChargeRespT.Amount) AS ChargeAmount, ChargesT.ID, ChargesT.ServiceID, LineItemT.PatientID, LineItemT.Date,  "
			"	LineItemT.InputDate, ChargesT.DoctorsProviders, LineItemT.LocationID "
			"	FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			"	WHERE LineItemT.Deleted = 0 AND Type = 10 AND LineItemT.GiftID IS NULL "
			"	Group By ChargesT.ID, ChargesT.ServiceID, LineItemT.PatientID, LineItemT.Date, LineItemT.InputDate,  "
			"	ChargesT.DoctorsProviders, LineItemT.LocationID "
			"	) AS ChargesSubQ  "
			"LEFT JOIN   "
			"	(SELECT ChargeID, Sum(PayAmount) as SumPays, Sum(AdjAmount) as SumAdjs  "
			"	FROM  "
			"		(SELECT AppliesT.DestID AS ChargeID, AppliesT.Amount as PayAmount, 0 As AdjAmount  "
			"		FROM   "
			"		AppliesT Left JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"		LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID   "
			"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1  "
			"		UNION ALL  "
			"		SELECT AppliesT.DestID AS ChargeID, 0 as PayAmount,  AppliesT.Amount as AdjAmount  "
			"		FROM  AppliesT Left JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"		LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID   "
			"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
			"		) PaySubSubQ  "
			"	Group By ChargeID "
			"	) PaySubQ  ON ChargesSubQ.ID = PaySubQ.ChargeID  "
			"LEFT JOIN ServiceT ON ChargesSubQ.ServiceID = ServiceT.ID  "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID  "
			"GROUP BY CASE WHEN ServiceT.Category IS NULL THEN 0 ELSE ServiceT.Category END,  "
			"CASE WHEN CategoriesT.Name IS NULL THEN 'No Category' ELSE CategoriesT.Name END,  "
			"ChargesSubQ.DoctorsProviders, ChargesSubQ.Date, ChargesSubQ.InputDate, ChargesSubQ.LocationID");
		break;

	case 463:

		//Collection Rate By Service Code
		/* Version History
		JMM - 11/11/03 - Created PLID 6553
		DRT 7/1/2005 - PLID 15573 - Added Location filter.  This filter is on the CHARGE location, and totals all
		payments and adjustments applied, regardless of their location status.
		// (z.manning, 05/10/2007) - PLID 25490 - Rearranged the joinings of CPTCodeT and ServiceT slightly
		// such that inventory items will no longer be included.
		// (s.tullis 2016-03-08 16:01) - PLID 68489 -Fixed an issue where UNION was combining Unique payments
		*/
		return _T("SELECT CptCodeT.Code, CPTCodeT.SubCode, ServiceT.Name, SUM(ChargesSubQ.ChargeAmount) AS SumCharges,  "
			"SUM(PaySubQ.SumPays) AS SumPayments, SUM(PaySubQ.SumAdjs) AS SumAdjustments,   "
			"ChargesSubQ.DoctorsProviders As ProvID, ChargesSubQ.Date As TDate, ChargesSubQ.InputDate As IDate,  "
			"ServiceT.ID AS CPTID, ChargesSubQ.LocationID AS LocID "
			"FROM  "
			"	(SELECT SUM(ChargeRespT.Amount) AS ChargeAmount, ChargesT.ID, ChargesT.ServiceID, LineItemT.PatientID, LineItemT.Date, LineItemT.InputDate,  "
			"	ChargesT.DoctorsProviders, LineItemT.LocationID "
			"	FROM   "
			"	LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			"	WHERE LineItemT.Deleted = 0 AND Type = 10  "
			"	Group By ChargesT.ID, ChargesT.ServiceID, LineItemT.PatientID, LineItemT.Date, LineItemT.InputDate,  "
			"	ChargesT.DoctorsProviders, LineItemT.LocationID "
			"	) AS ChargesSubQ  "
			"LEFT JOIN   "
			"	(SELECT ChargeID, Sum(PayAmount) as SumPays, Sum(AdjAmount) as SumAdjs  "
			"	FROM  "
			"		(SELECT AppliesT.DestID AS ChargeID, AppliesT.Amount as PayAmount, 0 As AdjAmount  "
			"		FROM  AppliesT Left JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"		LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID   "
			"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1  "
			"		UNION ALL  "
			"		SELECT AppliesT.DestID AS ChargeID, 0 as PayAmount,  AppliesT.Amount as AdjAmount FROM   "
			"		AppliesT Left JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"		LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID    "
			"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
			"		) PaySubSubQ   "
			"	Group By ChargeID "
			"	) PaySubQ   "
			"ON ChargesSubQ.ID = PaySubQ.ChargeID   "
			"LEFT JOIN CPTCodeT ON ChargesSubQ.ServiceID = CPTCodeT.ID  "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID  "
			"GROUP By CPTCodeT.Code, ServiceT.Name, CPTCodeT.Subcode, ChargesSubQ.DoctorsProviders,  "
			"ChargesSubQ.Date, ChargesSubQ.InputDate, ServiceT.ID, ChargesSubQ.LocationID");
		break;

	case 464:

		//Collection Rate By Ins Co
		/* Version History
		JMM - 11/11/03
		DRT 7/1/2005 - PLID 15573 - Added Location filter.  This filter is on the CHARGE location, and totals all
		payments and adjustments applied, regardless of their location status.
		// (s.tullis 2016-03-08 16:01) - PLID 68489 - Fixed an issue where UNION was combining Unique payments
		// (s.tullis 2016-03-09 09:43) - PLID 68491 - Collection By Ins Co and Collection by Service Category reports factor in gift certificate payments and charges into the report calculation
		*/
		return _T("SELECT Sum(ChargeRespT.Amount) As ChargeAmount, SUm(CASE WHEN SumPays IS NULL THEN 0 ELSE PaySubQ.SumPays END) AS PayAmount,  "
			"SUM(CASE WHEN SumAdjs IS NULL THEN 0 ELSE PaySubQ.SumAdjs END) AS AdjAmount,  "
			"CASE WHEN InsuranceCoT.PersonID IS NULL THEN -1 ELSE InsuranceCoT.PersonID END AS InsCoID,  "
			"CASE WHEN InsuranceCoT.Name IS NULL THEN 'Patient Responsibility' ELSE InsuranceCoT.Name END AS RespName,  "
			"ChargesT.DoctorsProviders AS ProvID, LineItemT.Date AS TDate, LineItemT.InputDate As IDate, LineItemT.LocationID AS LocID "
			"FROM LineItemT   "
			"LEFT JOIN ChargesT ON LineItemT.Id = ChargesT.ID  "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			"LEFT JOIN   "
			"	(SELECT ChargeID, Sum(PayAmount) as SumPays, Sum(AdjAmount) as SumAdjs, RespID  "
			"	FROM  "
			"		(SELECT AppliesT.DestID AS ChargeID, AppliesT.Amount as PayAmount, 0 As AdjAmount, AppliesT.RespID AS RespID FROM   "
			"		AppliesT Left JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"		LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID   "
			"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1  "
			"		UNION ALL  "
			"		SELECT AppliesT.DestID AS ChargeID, 0 as PayAmount,  AppliesT.Amount as AdjAmount, AppliesT.RespID  "
			"		FROM AppliesT Left JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
			"		LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID   "
			"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
			"		) PaySubSubQ  "
			"	Group By ChargeID, RespID "
			"	) PaySubQ "
			"ON ChargeRespT.ID = PaySubQ.RespID  "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID  "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID  "
			"WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND LineItemT.GiftID IS NULL  "
			"AND ChargeRespT.Amount <> 0  "
			"GROUP BY InsuranceCoT.PersonID, InsuranceCoT.Name, ChargesT.DoctorsProviders, LineItemT.InputDate, LineItemT.Date,  "
			"LineItemT.LocationID");
		break;

	case 468:  //Financial Activity - Daily (by Ins Co)
		return GetSqlFinancialActivity(nSubLevel, nSubRepNum);
		break;

	case 473://Service Codes by Billed Amount
			 /* Version History
			 TES 2/9/2004 - Created.
			 */
		return _T("SELECT ServiceT.ID, CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name, "
			"Sum(ChargesT.Quantity) AS QuantityBilled, Sum(ChargeRespQ.Amount) AS AmountBilled, "
			"Sum(CASE WHEN AppliesQ.Amount IS Null THEN 0 ELSE AppliesQ.Amount END) AS AmountApplied, ServiceT.Category AS CatFilterID "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"INNER JOIN (SELECT ChargeID, Sum(Amount) AS Amount FROM ChargeRespT GROUP BY ChargeID) AS ChargeRespQ "
			"ON ChargesT.ID = ChargeRespQ.ChargeID LEFT JOIN "
			"(SELECT DestID, Sum(Amount) AS Amount FROM AppliesT GROUP BY DestID) AppliesQ ON LineItemT.ID = AppliesQ.DestID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
			"GROUP BY ServiceT.ID, CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name, ServiceT.Category");
		break;

	case 474://Billing Notes
			 /* Version History
			 TES 2/11/2004 - Created
			 TES 2/19/2004 - Fixed to use our retarded way of recording the bill's location.
			 */
		return _T("SELECT Notes.PersonID AS PatID, PatientsT.UserDefinedID, Notes.Date AS Date, Notes.Note, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, Notes.Category AS NoteCatID, "
			//Type-specific stuff
			"CASE WHEN Notes.LineItemID IS Null THEN PatientsT.MainPhysician ELSE "
			"   CASE WHEN PaymentsT.ID Is Not NULL THEN PaymentsT.ProviderID ELSE "
			"      ChargesT.DoctorsProviders "
			"   END "
			"END AS ProvID, CASE WHEN Notes.LineItemID Is Null THEN LineBillsT.LocationID ELSE LineItemT.LocationID END AS LocID, "
			"CASE WHEN Notes.LineItemID IS Null THEN BillsT.Date ELSE LineItemT.Date END AS TDate, "
			"CASE WHEN Notes.LineItemID IS Null THEN BillsT.Description ELSE LineItemT.Description END AS Description, "
			"CASE WHEN Notes.LineItemID IS Null THEN convert(nvarchar,BillsT.ID)+'B' ELSE convert(nvarchar,LineItemT.ID) END AS ItemIdentifier, "
			"CASE WHEN Notes.LineItemID Is Null THEN 0 ELSE LineItemT.Type END AS TypeID, "
			"NoteCatsF.Description AS Category "
			"FROM Notes LEFT JOIN LineItemT ON Notes.LineItemID = LineItemT.ID "
			"LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID LEFT JOIN PaymentsT ON LineITemT.ID = PaymentsT.ID "
			"LEFT JOIN BillsT LEFT JOIN (SELECT Min(ID) AS ID, BillID FROM ChargesT GROUP BY BillID) BillChargesQ ON BillsT.ID = BillChargesQ.BillID INNER JOIN LineItemT LineBillsT ON BillChargesQ.ID = LineBillsT.ID ON Notes.BillID = BillsT.ID "
			"INNER JOIN PersonT ON Notes.PersonID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN NoteCatsF ON Notes.Category = NoteCatsF.ID "
			"WHERE Notes.LineItemID Is Not Null OR Notes.BillID Is Not Null");
		break;

	case 475:
		//Mismatched Apply Locations
		/*
		DRT 2/12/2004 - PLID 10060 - Created.
		TES 2/13/2004 - Moved to Financial tab.
		*/
		return _T("SELECT PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"LinePaysT.Type AS AppliedType, LinePaysT.Amount AS AppliedTotal, LinePaysT.Description AS AppliedDesc, LinePaysT.Date AS AppliedDate,  "
			"LinePaysT.InputDate AS AppliedIDate, LocPayT.Name AS AppliedLocName,  "
			"LineChargesT.Type AS ApplyToType,  "
			"CASE WHEN LineChargesT.Type = 10 THEN dbo.GetChargeTotal(LineChargesT.ID) ELSE LineChargesT.Amount END AS ApplyToTotal,  "
			"LineChargesT.Description AS ApplyToDesc, LineChargesT.Date AS ApplyToDate,  "
			"LineChargesT.InputDate AS ApplyToIDate, LocChgT.Name AS ApplyToLocName,  "
			"AppliesT.InputDate AS DateOfApply, AppliesT.Amount AS AmountOfApply "
			" "
			"FROM "
			"LineItemT LinePaysT  "
			"LEFT JOIN AppliesT ON LinePaysT.ID = AppliesT.SourceID "
			"LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID "
			"LEFT JOIN PersonT ON LinePaysT.PatientID = PersonT.ID "
			"LEFT JOIN LocationsT LocPayT ON LinePaysT.LocationID = LocPayT.ID "
			"LEFT JOIN LocationsT LocChgT ON LineChargesT.LocationID = LocChgT.ID "
			" "
			"WHERE LinePaysT.LocationID <> LineChargesT.LocationID "
			"AND LinePaysT.Deleted = 0 AND LineChargesT.Deleted = 0");
		break;

	case 482:
		//Mismatched Apply Providers
		/*
		TES 2/27/2004 - PLID 11136 - Created (guess what it's based on!).
		*/
		return _T("SELECT PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"LinePaysT.Type AS AppliedType, LinePaysT.Amount AS AppliedTotal, LinePaysT.Description AS AppliedDesc, LinePaysT.Date AS AppliedDate,  "
			"LinePaysT.InputDate AS AppliedIDate, PayDoc.Last + ', ' + PayDoc.First + ' ' + PayDoc.Middle AS AppliedProvName,  "
			"LineChargesT.Type AS ApplyToType,  "
			"CASE WHEN LineChargesT.Type = 10 THEN dbo.GetChargeTotal(LineChargesT.ID) ELSE LineChargesT.Amount END AS ApplyToTotal,  "
			"LineChargesT.Description AS ApplyToDesc, LineChargesT.Date AS ApplyToDate,  "
			"LineChargesT.InputDate AS ApplyToIDate, ChargeDoc.Last + ', ' + ChargeDoc.First + ' ' + ChargeDoc.Middle AS ApplyToProvName,  "
			"AppliesT.InputDate AS DateOfApply, AppliesT.Amount AS AmountOfApply "
			" "
			"FROM "
			"LineItemT LinePaysT  LEFT JOIN PaymentsT ON LinePaysT.ID = PaymentsT.ID "
			"LEFT JOIN PersonT PayDoc ON PaymentsT.ProviderID = PayDoc.ID "
			"LEFT JOIN AppliesT ON LinePaysT.ID = AppliesT.SourceID "
			"LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID "
			"LEFT JOIN (SELECT ID, DoctorsProviders FROM ChargesT UNION SELECT ID, ProviderID FROM PaymentsT) AS LineProvsQ "
			"ON LineChargesT.ID = LineProvsQ.ID LEFT JOIN PersonT ChargeDoc ON LineProvsQ.DoctorsProviders = ChargeDoc.ID "
			"LEFT JOIN PersonT ON LinePaysT.PatientID = PersonT.ID "
			"LEFT JOIN LocationsT LocPayT ON LinePaysT.LocationID = LocPayT.ID "
			"LEFT JOIN LocationsT LocChgT ON LineChargesT.LocationID = LocChgT.ID "
			" "
			"WHERE PaymentsT.ProviderID <> LineProvsQ.DoctorsProviders "
			"AND LinePaysT.Deleted = 0 AND LineChargesT.Deleted = 0");
		break;

	case 488:
		//Applies Across Date
		/*
		TES 3/10/2004 - Created, PLID 10495
		*/
	{
		CString strSql;
		strSql.Format("SELECT PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"LinePaysT.Type AS AppliedType, LinePaysT.Amount AS AppliedTotal, LinePaysT.Description AS AppliedDesc, LinePaysT.Date AS AppliedDate,  "
			"LinePaysT.InputDate AS AppliedIDate, LocPayT.Name AS AppliedLocName,  "
			"LineChargesT.Type AS ApplyToType,  "
			"CASE WHEN LineChargesT.Type = 10 THEN dbo.GetChargeTotal(LineChargesT.ID) ELSE LineChargesT.Amount END AS ApplyToTotal,  "
			"LineChargesT.Description AS ApplyToDesc, LineChargesT.Date AS ApplyToDate,  "
			"LineChargesT.InputDate AS ApplyToIDate, LocChgT.Name AS ApplyToLocName,  "
			"AppliesT.InputDate AS DateOfApply, AppliesT.Amount AS AmountOfApply "
			" "
			"FROM "
			"LineItemT LinePaysT  "
			"LEFT JOIN AppliesT ON LinePaysT.ID = AppliesT.SourceID "
			"LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID "
			"LEFT JOIN PersonT ON LinePaysT.PatientID = PersonT.ID "
			"LEFT JOIN LocationsT LocPayT ON LinePaysT.LocationID = LocPayT.ID "
			"LEFT JOIN LocationsT LocChgT ON LineChargesT.LocationID = LocChgT.ID "
			" "
			"WHERE ((LinePaysT.%s < '%s' AND LineChargesT.%s >= '%s') OR (LinePaysT.%s >= '%s' AND LineChargesT.%s < '%s')) "
			"AND LinePaysT.Deleted = 0 AND LineChargesT.Deleted = 0", strDateFilterField, _Q(FormatDateTimeForSql(DateTo, dtoDate)),
			strDateFilterField, _Q(FormatDateTimeForSql(DateTo, dtoDate)), strDateFilterField, _Q(FormatDateTimeForSql(DateTo, dtoDate)),
			strDateFilterField, _Q(FormatDateTimeForSql(DateTo, dtoDate)));
		return _T(strSql);
	}
	break;

	/*
	case 491:
	//Provider Commissions (Payments)
	// (j.jones 2009-11-17 11:16) - PLID 36326 - moved report to GetProviderCommissionsPaymentsRecordset() so we can custom build the recordset
	break;
	*/

	/*
	case 544:
	//Provider Commissions (Charges)
	// (j.jones 2009-11-17 11:16) - PLID 36326 - moved report to GetProviderCommissionsChargesRecordset() so we can custom build the recordset
	break;
	*/

	case 499:
		//Outstanding Gift Certificate Balances
		/*	Version History
		DRT 4/19/2004 - PLID 11689 - Created.  Shows all gift certificates with an open balance.
		ADW 4/12/06 PLID 16347 - Removed voided gift certificates from the report
		(a.walling 2007-03-22 15:48) - PLID 25113 - Turned the query into GiftQ for external and extended filters
		// (j.jones 2010-02-04 16:02) - PLID 36500 - removed outer query to fix filtering assertions
		// (j.dinatale 2011-09-30 15:05) - PLID 45773 - Take into account a voided gift certificates (via financial corrections)
		TES 3/20/2015 - PLID 65074 - Include tip amounts in the total spent
		// (j.jones 2015-04-27 09:12) - PLID 65388 - moved the value/balance calculations to a shared function,
		// which added support for the new Value field, Refunds, and GC Transfers
		*/
		return _T("SELECT GiftCertificatesT.ID, GiftCertificatesT.GiftID, "
			"PurchT.Last + ', ' + PurchT.First + ' ' + PurchT.Middle AS PurchasedBy,  "
			"ServiceT.ID AS ServiceID, ServiceT.Name AS TypeName, PurchT.ID AS PatID, RecT.ID AS ReceivedByID,  "
			"RecT.Last + ', ' + RecT.First + ' ' + RecT.Middle AS ReceivedBy, "
			"GCBalanceQ.TotalValue AS AmtPurch, "
			"GCBalanceQ.AmtUsed AS AmtSpent, "
			"GCBalanceQ.Balance,  "
			"GiftCertificatesT.PurchaseDate AS PurchDate, GiftCertificatesT.ExpDate AS ExpDate, "
			"LocationsT.ID AS LocID, LocationsT.Name AS LocName "
			"FROM GiftCertificatesT "
			"INNER JOIN (" + GetGiftCertificateValueQuery() + ") AS GCBalanceQ ON GiftCertificatesT.ID = GCBalanceQ.ID "
			"LEFT JOIN PersonT PurchT ON GiftCertificatesT.PurchasedBy = PurchT.ID  "
			"LEFT JOIN PersonT RecT ON GiftCertificatesT.ReceivedBy = RecT.ID  "
			"LEFT JOIN GCTypesT ON GiftCertificatesT.DefaultTypeID = GCTypesT.ServiceID  "
			"LEFT JOIN ServiceT ON GCTypesT.ServiceID = ServiceT.ID "
			"LEFT JOIN LocationsT ON GiftCertificatesT.LocationID = LocationsT.ID "
			"WHERE GCBalanceQ.Balance <> 0 AND GiftCertificatesT.Voided = 0 ");
		break;

	case 501:
	{
		//Closed Cash Drawers
		/*	Version History
		DRT 4/21/2004 - PLID 11696 - Created.  Shows all cash drawers which have been closed out.  NexSpa Only.
		DRT 4/26/2004 - PLID 12097 - Changed the query to include tips.
		DRT 2/15/2005 - PLID 15614 - Changed query to read the "CashReceived" field.  It now calculates this as the amount brought in,
		and looks at the change given due to that (as cash).  Then the report now totals up each value at the end.
		DRT 2/15/2005 - PLID 15644 - Forced inclusion of only types 1, 2, 3, 4, 7 - previously it was thought impossible to
		put items in a draw which were not of those types, but due to a bug in the payment dialog, it was.
		DRT 6/29/2005 - PLID 16010 - Made changes to allow refunds of check and charge type.  Reports changed as well.
		DRT 10/3/2005 - PLID 17759 - Removed the ORDER BY clause, it was causing the Create Merge Group option to fail.  I moved the
		sort fields into the query and made the report do the sorting
		JMM 8/4/06 - Added a print preview version
		(a.walling 2006-11-28 10:52) - PLID 22659 - For tips: AmtRecieved is the Tip Amount change given is always 0.
		TES 3/30/2015 - PLID 65180 - Updated to support tip refunds
		(r.gonet 2015-05-05 14:38) - PLID 66356 - Added PayMethod 10 - Gift Certificate Refunds
		TES 7/9/2015 - PLID 66532 - Changed UNIONs to UNION ALLs
		*/
		CString strSql;
		strSql.Format("SELECT DrawerID AS DrawerID, Amount, PatID AS PatID, PatName, DrawerName,   "
			"DateOpen AS DateOpen, DateClosed AS DateClosed, PayMethod, PayMethodText,   "
			"OpenAmt, CloseCash, CloseCheck, CloseCharge, CloseGift, UserDefinedID,   "
			"PaymentDate, PayInputDate, PayDesc, ProvID AS ProvID, ProvName,   "
			"LocID AS LocID, LocName, AmtReceived, ChangeGiven, LineItemID "
			"FROM   "
			"	(SELECT LineItemT.DrawerID, LineItemT.Amount, PersonT.ID AS PatID,   "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, CashDrawersT.Name AS DrawerName,  "
			"	CashDrawersT.DateOpen, CashDrawersT.DateClosed, PaymentsT.PayMethod,   "
			"	CASE WHEN PaymentsT.PayMethod = 1 THEN 'Cash'   "
			"		WHEN PaymentsT.PayMethod = 2 THEN 'Check'  "
			"		WHEN PaymentsT.PayMethod = 3 THEN 'Charge'  "
			"		WHEN PaymentsT.PayMethod = 4 THEN 'Gift Certificate' "
			"		WHEN PaymentsT.PayMethod = 7 THEN 'Cash Refund' "
			"		WHEN PaymentsT.PayMethod = 8 THEN 'Check Refund' "
			"		WHEN PaymentsT.PayMethod = 9 THEN 'Charge Refund' "
			"		WHEN PaymentsT.PayMethod = 10 THEN 'Gift Cert Refund' "
			"		END AS PayMethodText,  "
			"	OpenAmt, CloseCash, CloseCheck, CloseCharge, CloseGift, PatientsT.UserDefinedID,   "
			"	LineItemT.Date AS PaymentDate, LineItemT.InputDate AS PayInputDate,  "
			"	CASE WHEN PaymentsT.PrePayment = 1 THEN '[Prepay] - ' ELSE '' END + LineItemT.Description AS PayDesc,   "
			"	PersonProv.ID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName,  "
			"	LocationsT.ID AS LocID, LocationsT.Name AS LocName, "
			"	CASE WHEN PaymentsT.PayMethod = 4 THEN LineItemT.Amount  WHEN LineItemT.Type <> 3 THEN PaymentsT.CashReceived ELSE convert(money, 0) END AS AmtReceived, "
			"	CASE WHEN PaymentsT.PayMethod = 4 THEN convert(money, 0) WHEN LineItemT.Type <> 3 THEN LineItemT.Amount - PaymentsT.CashReceived ELSE LineItemT.Amount END AS ChangeGiven, "
			"	LineItemT.ID AS LineItemID "
			"	FROM LineItemT  "
			"	LEFT JOIN CashDrawersT ON LineItemT.DrawerID = CashDrawersT.ID  "
			"	LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID  "
			"	LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
			"	LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"	LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID  "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
			"	WHERE LineItemT.Deleted = 0 AND DrawerID IS NOT NULL AND CashDrawersT.DateClosed IS NOT NULL AND "
			"	(LineItemT.Type = 1 OR LineItemT.Type = 2 OR LineItemT.Type = 3 OR LineItemT.Type = 4 OR LineItemT.Type = 7) "
			"  "
			"	UNION ALL "
			"	SELECT ID, OpenAmt, NULL, NULL, Name, DateOpen, DateClosed, -1, '<Opening Amount>', OpenAmt, CloseCash, CloseCheck, CloseCharge,   "
			"	CloseGift, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, OpenAmt, NULL, -1  "
			"	FROM CashDrawersT  "
			"	WHERE DateClosed IS NOT NULL  "
			" "
			"	UNION ALL "
			"	SELECT PaymentTipsT.DrawerID, PaymentTipsT.Amount, PersonT.ID AS PatID,   "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, CashDrawersT.Name AS DrawerName,  "
			"	CashDrawersT.DateOpen, CashDrawersT.DateClosed, "
			"	PaymentTipsT.PayMethod, "
			"	CASE WHEN PaymentTipsT.PayMethod = 1 THEN 'Cash'   "
			"		WHEN PaymentTipsT.PayMethod = 2 THEN 'Check'  "
			"		WHEN PaymentTipsT.PayMethod = 3 THEN 'Charge'  "
			"		WHEN PaymentTipsT.PayMethod = 4 THEN 'Gift Certificate' "
			"		WHEN PaymentTipsT.PayMethod = 7 THEN 'Cash Refund' "
			"		WHEN PaymentTipsT.PayMethod = 8 THEN 'Check Refund' "
			"		WHEN PaymentTipsT.PayMethod = 9 THEN 'Charge Refund' "
			"		WHEN PaymentTipsT.PayMethod = 10 THEN 'Gift Cert Refund' "
			"		END AS PayMethodText,  "

			"	OpenAmt, CloseCash, CloseCheck, CloseCharge, CloseGift, PatientsT.UserDefinedID,   "
			"	LineItemT.Date AS PaymentDate, LineItemT.InputDate AS PayInputDate,  "
			"	'Tip' AS PayDesc,   "
			"	PersonProv.ID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName,  "
			"	LocationsT.ID AS LocID, LocationsT.Name AS LocName, "
			"	CASE WHEN LineItemT.Type = 3 THEN convert(money,0) ELSE PaymentTipsT.Amount END AS AmtReceived, "
			"	CASE WHEN LineItemT.Type = 3 THEN PaymentTipsT.Amount ELSE convert(money,0) END AS ChangeGiven, "
			"	LineItemT.ID AS LineItemID "
			"	FROM PaymentTipsT "
			"	LEFT JOIN LineItemT ON PaymentTipsT.PaymentID = LineItemT.ID "
			"	LEFT JOIN CashDrawersT ON PaymentTipsT.DrawerID = CashDrawersT.ID  "
			"	LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID  "
			"	LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
			"	LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"	LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID  "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
			"	WHERE LineItemT.Deleted = 0 AND PaymentTipsT.DrawerID IS NOT NULL AND CashDrawersT.DateClosed IS NOT NULL  "
			"	) SubQ ");

		return _T(strSql);

	}
	break;


	case 503:
		//Open Cash Drawers
		/*	Version History
		DRT 4/21/2004 - PLID 11696 - Created.  Allows us to see all currently open cash drawers and how much is in them.
		DRT 4/26/2004 - PLID 12097 - Changed the query to include tips.
		DRT 2/15/2005 - PLID 15613 - Changed query to read the "CashReceived" field.  It now calculates this as the amount brought in,
		and looks at the change given due to that (as cash).  Then the report now totals up each value at the end.
		DRT 2/15/2005 - PLID 15644 - Forced inclusion of only types 1, 2, 3, 4, 7 - previously it was thought impossible to
		put items in a drawer which were not of those types, but due to a bug in the payment dialog, it was.
		DRT 6/29/2005 - PLID 16010 - Made changes to allow refunds of check and charge type.  Reports changed as well.
		DRT 10/3/2005 - PLID 17760 - Removed the ORDER BY clause to the report file so the create merge group function works.
		DRT 9/22/2006 - PLID 22659 - For tips, "Received" is now always the tip amount, and change given is now always 0.
		// (j.gruber 2010-10-28 11:31) - PLID 41002 - added Payment Input Name
		// (j.gruber 2011-01-25 11:01) - PLID 41002 - changed the opening cash drawer username to not be blank
		TES 3/30/2015 - PLID 65180 - Updated to support tip refunds
		(r.gonet 2015-05-05 14:38) - PLID 66356 - Added PayMethod 10 - Gift Certificate Refunds
		TES 7/9/2015 - PLID 66532 - Changed UNIONs to UNION ALLs
		*/
		return _T("SELECT DrawerID AS DrawerID, Amount AS ChargeAmount, PatID AS PatID, PatName, DrawerName,   "
			"DateOpen AS DateOpen, DateClosed AS DateClosed, PayMethod, PayMethodText,   "
			"OpenAmt, CloseCash, CloseCheck, CloseCharge, CloseGift, UserDefinedID,   "
			"PaymentDate, PayInputDate, PayDesc, ProvID AS ProvID, ProvName,   "
			"LocID AS LocID, LocName, ReceivedAmount, ChangeGiven, LineItemID, PayInputName as PayInputName "
			"FROM   "
			"	(SELECT LineItemT.DrawerID, LineItemT.Amount, PersonT.ID AS PatID,   "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, CashDrawersT.Name AS DrawerName,  "
			"	CashDrawersT.DateOpen, CashDrawersT.DateClosed, PaymentsT.PayMethod,   "
			"	CASE WHEN PaymentsT.PayMethod = 1 THEN 'Cash'   "
			"		WHEN PaymentsT.PayMethod = 2 THEN 'Check'  "
			"		WHEN PaymentsT.PayMethod = 3 THEN 'Charge'  "
			"		WHEN PaymentsT.PayMethod = 4 THEN 'Gift Certificate' "
			"		WHEN PaymentsT.PayMethod = 7 THEN 'Cash Refund' "
			"		WHEN PaymentsT.PayMethod = 8 THEN 'Check Refund' "
			"		WHEN PaymentsT.PayMethod = 9 THEN 'Charge Refund' "
			"		WHEN PaymentsT.PayMethod = 10 THEN 'Gift Cert Refund' "
			"	END AS PayMethodText,  "
			"	OpenAmt, CloseCash, CloseCheck, CloseCharge, CloseGift, PatientsT.UserDefinedID,   "
			"	LineItemT.Date AS PaymentDate, LineItemT.InputDate AS PayInputDate,  "
			"	CASE WHEN PaymentsT.PrePayment = 1 THEN '[Prepay] - ' ELSE '' END + LineItemT.Description AS PayDesc,  "
			"	PersonProv.ID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName,  "
			"	LocationsT.ID AS LocID, LocationsT.Name AS LocName, "
			"	CASE WHEN PaymentsT.PayMethod = 4 THEN LineItemT.Amount  WHEN LineItemT.Type <> 3 THEN PaymentsT.CashReceived ELSE convert(money, 0) END AS ReceivedAmount, "
			"	CASE WHEN PaymentsT.PayMethod = 4 THEN convert(money, 0) WHEN LineItemT.Type <> 3 THEN LineItemT.Amount - PaymentsT.CashReceived ELSE LineItemT.Amount END AS ChangeGiven, "
			"	LineItemT.InputDate, LineItemT.ID AS LineItemID, LineItemT.InputName as PayInputName "
			"	FROM LineItemT  "
			"	LEFT JOIN CashDrawersT ON LineItemT.DrawerID = CashDrawersT.ID  "
			"	LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID  "
			"	LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
			"	LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"	LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID  "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
			"	WHERE LineItemT.Deleted = 0 AND DrawerID IS NOT NULL AND CashDrawersT.DateClosed IS NULL AND "
			"	(LineItemT.Type = 1 OR LineItemT.Type = 2 OR LineItemT.Type = 3 OR LineItemT.Type = 4 OR LineItemT.Type = 7) "
			" "
			"	UNION ALL "
			"	SELECT ID, OpenAmt, NULL, NULL, Name, DateOpen, DateClosed, -1, '<Opening Amount>', OpenAmt, CloseCash, CloseCheck, CloseCharge,   "
			"	CloseGift, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, OpenAmt, NULL, DateOpen, -1 AS LineItemID, '<Opening_Cash_Amt>' as PayInputName "
			"	FROM CashDrawersT  "
			"	WHERE DateClosed IS NULL  "
			"  "
			"	UNION ALL "
			"	SELECT PaymentTipsT.DrawerID, PaymentTipsT.Amount AS ChargeAmount, PersonT.ID AS PatID,    "
			"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, CashDrawersT.Name AS DrawerName,   "
			"	CashDrawersT.DateOpen, CashDrawersT.DateClosed, "
			"	PaymentTipsT.PayMethod, "
			"	CASE WHEN PaymentTipsT.PayMethod = 1 THEN 'Cash'   "
			"		WHEN PaymentTipsT.PayMethod = 2 THEN 'Check'  "
			"		WHEN PaymentTipsT.PayMethod = 3 THEN 'Charge'  "
			"		WHEN PaymentTipsT.PayMethod = 4 THEN 'Gift Certificate' "
			"		WHEN PaymentTipsT.PayMethod = 7 THEN 'Cash Refund' "
			"		WHEN PaymentTipsT.PayMethod = 8 THEN 'Check Refund' "
			"		WHEN PaymentTipsT.PayMethod = 9 THEN 'Charge Refund' "
			"		WHEN PaymentTipsT.PayMethod = 10 THEN 'Gift Cert Refund' "
			"		END AS PayMethodText,  "
			"	OpenAmt, CloseCash, CloseCheck, CloseCharge, CloseGift, PatientsT.UserDefinedID,    "
			"	LineItemT.Date AS PaymentDate, LineItemT.InputDate AS PayInputDate,   "
			"	'Tip' AS PayDesc,    "
			"	PersonProv.ID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName,   "
			"	LocationsT.ID AS LocID, LocationsT.Name AS LocName, "
			"	CASE WHEN LineItemT.Type = 3 THEN convert(money,0) ELSE PaymentTipsT.Amount END AS ReceivedAmount,  "
			"	CASE WHEN LineItemT.Type = 3 THEN PaymentTipsT.Amount ELSE 0 END AS ChangeGiven, "
			"	LineItemT.InputDate, LineItemT.ID AS LineItemID, LineItemT.InputName as PayInputName "
			"	FROM PaymentTipsT  "
			"	LEFT JOIN LineItemT ON PaymentTipsT.PaymentID = LineItemT.ID  "
			"	LEFT JOIN CashDrawersT ON PaymentTipsT.DrawerID = CashDrawersT.ID   "
			"	LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID   "
			"	LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
			"	LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
			"	LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID   "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID   "
			"	WHERE LineItemT.Deleted = 0 AND PaymentTipsT.DrawerID IS NOT NULL AND CashDrawersT.DateClosed IS NULL   "
			"	) SubQ ");
		break;

	case 554:
		//Insurance Vs. Self-Pay Percentages By Provider
		/* Version History
		JMM- 7/1/2005 - Created
		JMM - 5-19-2006 - its not filtering out quotes
		*/
		return _T("SELECT  DoctorsProviders as ProvID, PersonT.First as DocFirst, PersonT.Last As DocLast, PersonT.Title as DocTitle, Sum(InsAmount) as InsAmount, Sum(PatAmount) AS PatAmount, Sum(TotalAmt) as TotalAmount, Q.Date as TDate, Q.InputDate as IDate FROM ( "
			" 	SELECT ChargesT.DoctorsProviders, Sum(InsAmount) as InsAmount, 0 as PatAmount, 0 as TotalAmt, LineItemT.Date, LineItemT.InputDate FROM  "
			" 	(SELECT ChargeID, Sum(ChargeRespT.Amount) as InsAmount  "
			" 	From ChargeRespT  "
			" 	WHERE ChargeID IN (SELECT ChargeID FROM ChargeRespT WHERE InsuredPartyID IS NOT NULL AND Amount <> 0)  "
			" 	Group By ChargeID  "
			" 	) InsuranceRespT "
			" 	LEFT JOIN ChargesT ON InsuranceRespT.ChargeID = ChargesT.ID "
			" 	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	WHERE LineItemT.Deleted = 0 and LineItemT.Type = 10 "
			" 	GROUP BY ChargesT.DoctorsProviders, LineItemT.Date, LineItemT.InputDate "
			" UNION "
			" SELECT ChargesT.DoctorsProviders, 0, Sum(PatAmount) as PatAmount, 0 as TotalAmt, LineItemT.Date, LineItemT.InputDate FROM  "
			" 	(SELECT ChargeID, Sum(ChargeRespT.Amount) as PatAmount  "
			" 	From ChargeRespT  "
			" 	WHERE ChargeID IN (SELECT ChargeID FROM ChargeRespT WHERE InsuredPartyID IS NULL AND Amount <> 0 AND ChargeID NOT IN (SELECT ChargeID FROM ChargeRespT WHERE InsuredPartyID IS NOT NULL AND Amount <> 0)) "
			" 	Group By ChargeID  "
			" 	) InsuranceRespT "
			" 	LEFT JOIN ChargesT ON InsuranceRespT.ChargeID = ChargesT.ID "
			" 	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			" 	WHERE LineItemT.Deleted = 0 and LineItemT.Type = 10 "
			" 	GROUP BY ChargesT.DoctorsProviders, LineItemT.Date, LineItemT.InputDate "
			" UNION  "
			" 	SELECT ChargesT.DoctorsProviders, 0, 0, Sum(AllAmount) AS TotalAmt, LineItemT.Date, LineItemT.InputDate FROM  "
			" 	(SELECT ChargeID, Sum(ChargeRespT.Amount) AS AllAmount "
			" 	FROM ChargeRespT GROUP BY ChargeID) ChargeRespT LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID "
			" 	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			" 	WHERE LineItemT.Deleted = 0  and LineItemT.Type = 10 "
			" 	GROUP BY ChargesT.DoctorsProviders, LineItemT.Date, LineItemT.InputDate "
			" )Q "
			" LEFT JOIN ProvidersT ON Q.DoctorsProviders = ProvidersT.PersonID "
			" LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
			" GROUP BY DoctorsProviders, PErsonT.First, PersonT.Last, PersonT.Title, Q.Date, Q.InputDate");
		break;

	case 557:
		//Charges with Applies Split by Resp
		/*Version History
		8/8/2005 - JMM PLID 14865  - Created
		11/09/2006 - e.lally PLID 22941 - Added some payment and bill information
		2009-06-02 - v.arth PLID 33985 - Added the diagnosis codes and the modifier codes
		2009-06-04 - v.arth PLID 33985 - Added report 667
		// (j.gruber 2009-07-13 17:17) - PLID 33985 - added modifier descriptions and fixed the diag codes to be the code, not the ID
		// (j.gruber 2010-06-09 14:54) - PLID 39065 - added primary ref source, ref phys, and ref pat
		// (a.levy 2013-12-06 15:35) - PLID 59936 - added Tertiary amount & party.
		// (d.thompson 2014-03-21) - PLID 61352 - Added ICD-10 fields
		*/
		return _T(
			" DECLARE @nRespTypeFinder INT "
			" SET @nRespTypeFinder = ((SELECT ID FROM RespTypeT WHERE Priority = 3)); "
			" SELECT ChargesT.ID As ChargeID, DoctorsProviders AS ProvID, Sum(ChargeRespT.Amount) AS ChargeAmount,  LineCharges.LocationID AS LocID,  "
			" LineCharges.PatientID AS PatID, PersonT.First, PersonT.Middle, PersonT.Last,  PatientsT.UserDefinedID, ServiceT.ID as ServiceID,  "
			" CASE WHEN CPTCodeT.ID IS NULL THEN '' ELSE CPTCodeT.Code END AS CPTCode, ServiceT.Name as ServiceName,  LineCharges.Date AS ChargeDate,  "
			" CASE WHEN LinePays.Date IS NULL THEN Convert(datetime, '1899-12-30') ELSE CAST(Convert(nVarChar, LinePays.Date, 23) as datetime) END AS PayDate, LineCharges.InputDate AS ChargeIDate,  "
			" CASE WHEN LinePays.Date IS NULL THEN Convert(datetime, '1899-12-30') ELSE CAST(Convert(nVarChar, LinePays.InputDate, 23) as datetime) END AS PayIDate,   "
			" ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, ProvT.Title as ProvTitle,    "
			" SUM (CASE WHEN ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1 THEN ChargeRespT.Amount  	  ELSE 0       END) AS PatAmount,   "
			" SUM (CASE WHEN InsuredPartyT.RespTypeID = 1 THEN ChargeRespT.Amount  	  ELSE 0       END) AS PriAmount,   "
			" SUM (CASE WHEN InsuredPartyT.RespTypeID = 2 THEN ChargeRespT.Amount  	  ELSE 0       END) AS SecAmount,   "
			" CASE WHEN @nRespTypeFinder IS NOT NULL THEN "
			" SUM (CASE WHEN InsuredPartyT.RespTypeID = @nRespTypeFinder THEN ChargeRespT.Amount ELSE 0 END) "
			" ELSE NULL END AS  TertAmount, "
			" CASE WHEN AppliesT.Amount IS NULL THEN 0 ELSE CASE WHEN LinePays.Type = 1 THEN AppliesT.Amount ELSE 0 END END AS AppliedPay,   "
			" PaymentsT.InsuredPartyID,  "
			" (SELECT PersonID FROM InsuredPartyT WHERE InsuredPartyT.PatientID = PatientsT.PersonID AND RespTypeID = 1) AS PriIns, "
			" (SELECT PersonID FROM InsuredPartyT WHERE InsuredPartyT.PatientID = PatientsT.PersonID AND RespTypeID = 2) AS SecIns, "
			" CASE WHEN @nRespTypeFinder IS NOT NULL THEN "
			" (SELECT PersonID FROM InsuredPartyT WHERE InsuredPartyT.PatientID = PatientsT.PersonID AND RespTypeID = @nRespTypeFinder) "
			" ELSE NULL END AS TertiaryIns, "
			" CASE WHEN PaymentsT.ID IS NOT NULL THEN CASE WHEN LinePays.Type = 1 THEN CASE WHEN PaymentsT.InsuredPartyID = -1 THEN AppliesT.Amount ELSE 0 END ELSE 0 END ELSE 0 END AS PatPays, "
			" CASE WHEN AppliesT.Amount IS NULL THEN 0  	ELSE CASE WHEN LinePays.Type = 2 THEN AppliesT.Amount   ELSE 0   	END END AS Adjustments,   "
			" PaymentsT.Paymethod, "
			" (SELECT SUM(Amount) FROM AppliesT WHERE DestID = ChargesT.ID AND SourceID IN(SELECT ID FROM LineItemT WHERE Type = 1)GROUP BY DestID) AS AppliedPayAmt, "
			" (SELECT SUM(Amount) FROM AppliesT WHERE DestID = ChargesT.ID AND SourceID IN(SELECT ID FROM LineItemT WHERE Type = 2) GROUP BY DestID) AS AppliedAdjAmt, "
			" LinePays.Description AS PayDescription, PaymentsT.ID AS PaymentID, "
			" (SELECT Name FROM InsuranceCoT WHERE InsuranceCoT.PersonID = (SELECT InsuranceCoID FROM InsuredPartyT WHERE InsuredPartyT.PersonID = PaymentsT.InsuredPartyID)) AS InsuranceCoName, "
			" BillsT.ID AS BillID, BillsT.Description AS BillDesc, BillsT.Date AS BillDate, ChargesT.CPTModifier AS Modifier1, ChargesT.CPTModifier2 AS Modifier2, "
			" ChargesT.CPTModifier3 AS Modifier3, ChargesT.CPTModifier4 AS Modifier4, "
			"ICD9T1.CodeNumber as ICD9Code1, \r\n "
			"ICD9T2.CodeNumber as ICD9Code2, \r\n "
			"ICD9T3.CodeNumber as ICD9Code3, \r\n "
			"ICD9T4.CodeNumber as ICD9Code4, \r\n "
			"ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"ICD10T2.CodeNumber as ICD10Code2, \r\n "
			"ICD10T3.CodeNumber as ICD10Code3, \r\n "
			"ICD10T4.CodeNumber as ICD10Code4, \r\n "
			" CPTModifier1T.Note as CPTModifier1Note, CPTModifier2T.Note as CPTModifier2Note, CPTModifier3T.Note as CPTModifier3Note, CPTModifier4T.Note as CPTModifier4Note, "
			" ReferralSourceT.Name as PriRefSourceName, PatientsT.ReferralID as PriRefID, "
			" RefPhysPersonT.Id as RefPhysPersonID, RefPhysPersonT.First as RefPhysFirst, RefPhysPersonT.Middle as RefPhysMiddle, RefPhysPersonT.Last as RefPhysLast, RefPhysPersonT.Title as RefPhysTitle, "
			" RefPatPersonT.Id as RefPatPersonID, RefPatPersonT.First as RefPatFirst, RefPatPersonT.Middle as RefPatMiddle, RefPatPersonT.Last as RefPatLast, RefPatPersonT.Title as RefPatTitle "


			" FROM ChargesT INNER JOIN LineItemT LineCharges ON ChargesT.ID = LineCharges.ID   LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
			" LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			" LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID  "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID   "
			" LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID   "
			" LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  LEFT JOIN LineItemT LinePays ON PaymentsT.ID = LinePays.ID   "
			" LEFT JOIN PatientsT ON LineCharges.PatientID = PatientsT.PersonID  LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
			" LEFT JOIN PersonT ProvT ON ChargesT.DoctorsProviders = ProvT.ID   "
			" LEFT JOIN CPTModifierT CPTModifier1T ON ChargesT.CptModifier = CPTModifier1T.Number "
			" LEFT JOIN CPTModifierT CPTModifier2T ON ChargesT.CptModifier2 = CPTModifier2T.Number "
			" LEFT JOIN CPTModifierT CPTModifier3T ON ChargesT.CptModifier3 = CPTModifier3T.Number "
			" LEFT JOIN CPTModifierT CPTModifier4T ON ChargesT.CptModifier4 = CPTModifier4T.Number "
			"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
			"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
			" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			" LEFT JOIN PersonT RefPhysPersonT ON PatientsT.DefaultReferringPhyID = RefPhysPersonT.ID "
			" LEFT JOIN PersonT RefPatPersonT ON PatientsT.ReferringPatientID = RefPatPersonT.ID "

			" WHERE (LineCharges.Deleted = 0 AND LinePays.Deleted = 0) "

			" GROUP BY ChargesT.ID, ChargesT.DoctorsProviders, LineCharges.LocationID, LineCharges.PatientID,  PersonT.First, PersonT.Middle, PersonT.Last,  "
			" PatientsT.UserDefinedID, ServiceT.ID, CPTCodeT.Code,  ServiceT.Name, CPTCodeT.ID, LineCharges.Date, LineCharges.InputDate,   "
			" ProvT.First, ProvT.Middle, ProvT.Last, ProvT.Title, LinePays.Date, LinePays.InputDate, LinePays.Type, AppliesT.Amount,  "
			" PaymentsT.InsuredPartyID, PatientsT.PersonID, PaymentsT.ID, PaymentsT.Paymethod, AppliesT.SourceID, LinePays.Description, "
			" BillsT.ID, BillsT.Description, BillsT.Date, ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, "
			" ChargesT.CPTModifier4, "
			"ICD9T1.CodeNumber, \r\n "
			"ICD9T2.CodeNumber, \r\n "
			"ICD9T3.CodeNumber, \r\n "
			"ICD9T4.CodeNumber, \r\n "
			"ICD10T1.CodeNumber, \r\n "
			"ICD10T2.CodeNumber, \r\n "
			"ICD10T3.CodeNumber, \r\n "
			"ICD10T4.CodeNumber, \r\n "
			" CPTModifier1T.Note, CPTModifier2T.Note, CPTModifier3T.Note, CPTModifier4T.Note, "
			" ReferralSourceT.Name, PatientsT.ReferralID, "
			" RefPhysPersonT.Id, RefPhysPersonT.First, RefPhysPersonT.Middle, RefPhysPersonT.Last, RefPhysPersonT.Title, "
			" RefPatPersonT.Id, RefPatPersonT.First, RefPatPersonT.Middle, RefPatPersonT.Last, RefPatPersonT.Title"
			);
		break;

	case 558:
		/*Version History - Charges Breakdown by Provider
		8/8/2005 - JMM PLID 14850 - Created
		10/31/2005 - JMM PLID 18105 - fixed so it doesn't show quotes
		*/
		return _T("SELECT PersonT.First, PersonT.Middle, PersonT.Last, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID AS PatientID, "
			" BillsT.Location AS POSID, POST.Name as POSName, LineChargesT.Date as TDate,  LineChargesT.InputDate AS IDate, "
			" BillsT.ID AS BillID, ChargesT.Quantity, ChargesT.ID AS ChargeID, LineChargesT.LocationID AS LocID, "
			" ChargeRespT.Amount AS ChargeAmt, "
			" SUM(CASE WHEN AppliesT.ID IS NOT NULL THEN AppliesT.Amount ELSE 0 END) AS PayAmt, "
			" CASE WHEN ServiceT.Anesthesia <> 0 THEN 1 ELSE 0 END AS IsAnes, "
			" CASE WHEN ServiceT.FacilityFee <> 0 THEN 1 ELSE 0 END AS IsFacFee, "
			" ServiceT.Name, CPTCodeT.Code, CPTCodeT.Subcode, ProductT.LastCost, "
			" ProvT.First AS ProvFirst, ProvT.Middle AS ProvMiddle, ProvT.Last as ProvLast, "
			" ProvT.Title as ProvTitle, ProvT.ID AS ProvID, ServiceT.Category AS CatID, CategoriesT.Name as CatName,  "
			" BillsT.Date "
			" FROM ChargesT INNER JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID "
			" LEFT JOIN PatientsT ON LineChargesT.PatientID = PatientsT.PersonID "
			" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			" LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			" LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			" LEFT JOIN AppliesT ON ChargeRespT.ID = AppliesT.RespID "
			" LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
			" LEFT JOIN LineItemT LinePaysT ON PaymentsT.ID = LinePaysT.ID "
			" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" LEFT JOIN PersonT ProvT ON ChargesT.DoctorsProviders = ProvT.ID "
			" LEFT JOIN LocationsT POST ON BillsT.Location = POST.ID "
			" LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			" WHERE LineChargesT.Deleted = 0 AND (LinePaysT.Deleted = 0 OR LinePaysT.Deleted IS NULL) AND BillsT.Deleted = 0 "
			" AND LinechargesT.Type = 10 AND BillsT.EntryType = 1 "
			" GROUP BY BillsT.ID, PersonT.First, PersonT.Middle, PersonT.Last, PatientsT.PersonId, PatientsT.UserDefinedID, BillsT.Location,  "
			" POST.Name, LineChargesT.Date, ServiceT.Anesthesia, ServiceT.FacilityFee, ServiceT.Name, CPTCodeT.Code, CPTCodeT.SubCode, ProductT.LastCost, "
			" ProvT.First, ProvT.Middle, ProvT.Last, ProvT.Title, ProvT.ID, LineChargesT.InputDate, BillsT.ID, ChargeRespT.Amount, ChargeRespT.ID, ChargesT.ID, ChargesT.Quantity, ServiceT.Category, CategoriesT.Name, LineChargesT.LocationID, BillsT.Date");
		break;


	case 560:
		//Returned Products
		/*	Version History
		DRT 10/10/2005 - PLID 17844 - Created
		// (j.jones 2008-06-03 14:28) - PLID 29928 - we now only include ReturnedProductsT records that are not for deleted charges
		*/
		return _T("SELECT ReturnedProductsT.ID AS ReturnID, DateReturned AS DateReturned, QtyReturned, UsersT.Username AS ReturnedByUser, "
			"/*Charge info*/ "
			"LineCharges.Date AS ChargeDate, LineCharges.InputDate AS ChargeIDate, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal, ChargesT.Quantity AS QtyCharged, "
			"LineCharges.Description AS ChargeDesc, LocationsT.ID AS LocID, LocationsT.Name AS LocName,  "
			"/*Fin Adjustment Info*/ "
			"LineAdj.Date AS FinAdjDate, LineAdj.InputDate AS FinAdjIDate, LineAdj.Amount AS FinAdjAmount, LineAdj.Description AS FinAdjDesc, "
			"/*Refund Info*/ "
			"LineRef.Date AS RefDate, LineRef.InputDate AS RefIDate, LineRef.Amount AS RefAmount, LineRef.Description AS RefDesc, "
			"/*Inv Adjustment Info*/ "
			"ProductAdjustmentsT.Date AS ProdAdjDate, ProductAdjustmentsT.Amount AS ProdAdjAmount, "
			"/*Patient Info*/ "
			"PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"/*Provider Info*/ "
			"ChargesT.DoctorsProviders AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
			"/*Product Info*/ "
			"ServiceT.ID AS ProductID, ServiceT.Name AS ProductName "
			"FROM "
			"ReturnedProductsT "
			"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineAdj ON ReturnedProductsT.FinAdjID = LineAdj.ID "
			"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineRef ON ReturnedProductsT.FinRefundID = LineRef.ID "
			"LEFT JOIN ProductAdjustmentsT ON ReturnedProductsT.InvAdjID = ProductAdjustmentsT.ID "
			"INNER JOIN ChargesT ON ReturnedProductsT.ChargeID = ChargesT.ID "
			"INNER JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineCharges ON ChargesT.ID = LineCharges.ID "
			"LEFT JOIN UsersT ON ReturnedProductsT.UserID = UsersT.PersonID "
			"LEFT JOIN LocationsT ON LineCharges.LocationID = LocationsT.ID "
			"INNER JOIN PersonT ON LineCharges.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT PersonProv ON ChargesT.DoctorsProviders = PersonProv.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID");
		break;

	case 561:
		//Returned Product Basic Commission Summary
		/*	Version History
		DRT 10/10/2005 - PLID 17840 - Created.
		// (a.wetta 2007-04-20 14:25) - PLID 24872 - Added advanced commission rules to the calculation of a provider's commission
		// (j.jones 2008-05-02 12:24) - PLID 27817 - fixed divide by zero errors if a charge was $0.00
		// (j.jones 2008-06-03 14:28) - PLID 29928 - we now only include ReturnedProductsT records that are not for deleted charges
		// (j.gruber 2009-03-26 09:06) - PLID 33359 - updated discount structure
		// (j.jones 2009-11-20 16:35) - PLID 36386 - renamed to reflect this only uses Basic commissions, and filtered
		// the report only on providers that use Basic commissions, and rules for Basic commissions
		// (j.gruber 2012-12-18 15:25) - PLID 53242  - changed shopfee structure
		*/
		return _T("select ID, ChargeAmount, NoShopFeeAmount, NoTaxTotal, NoTaxNoShopFeeAmount, PatID AS PatID, UserDefinedID, PatName, ProvID AS ProvID, ProvName, ServiceID AS ServiceID, "
			"ServiceName, Code, CommissionBasePercentage, DateReturned AS DateReturned, ChargeDate AS ChargeDate, ChargeIDate AS ChargeIDate, CategoryName, LocID AS LocID, LocName, CommissionRulePercentage, "
			"CommissionRule, convert(money, NoShopFeeAmount * ((CASE WHEN CommissionRulePercentage is not NULL THEN CommissionRulePercentage ELSE CommissionBasePercentage END)/100)) AS CommissionAmount, "
			"convert(money, NoTaxNoShopFeeAmount * ((CASE WHEN CommissionRulePercentage is not NULL THEN CommissionRulePercentage ELSE CommissionBasePercentage END)/100)) AS NoTaxCommissionAmount "
			"from "
			"(select * , "
			/* Get the commission percentage for this charge*/
			"(select top 1 CommissionRulesT.Percentage "
			"from CommissionRulesLinkT "
			"inner join CommissionRulesT on CommissionRulesLinkT.RuleID = CommissionRulesT.ID "
			/* We only want a rule that is valid for the time period of a particular charge*/
			"WHERE CommissionRulesT.IsTieredCommission = 0 "
			"AND convert(datetime, convert(nvarchar, CommissionRulesT.StartDate, 101)) <= ChargeInfoQ.ChargeDate and "
			"convert(datetime, convert(nvarchar, CommissionRulesT.EndDate, 101)) >= ChargeInfoQ.ChargeDate and "
			"coalesce((select sum(dbo.GetChargeTotal(ChargesT.ID)) "
			"from ChargesT "
			"left join LineItemT on ChargesT.ID = LineItemT.ID "
			"where LineItemT.Type = 10 and LineItemT.Deleted = 0 and "
			"convert(datetime, convert(nvarchar, LineItemT.Date, 101)) >= convert(datetime, convert(nvarchar, CommissionRulesT.BasedOnStartDate, 101)) and "
			"convert(datetime, convert(nvarchar, LineItemT.Date, 101)) <= convert(datetime, convert(nvarchar, CommissionRulesT.BasedOnEndDate, 101)) and "
			" ServiceID  = CommissionRulesLinkT.ServiceID AND "
			"DoctorsProviders = CommissionRulesLinkT.ProvID), 0) >= MoneyThreshold and ProvID = ChargeInfoQ.ProvID and ServiceID = ChargeInfoQ.ServiceID "
			"order by MoneyThreshold desc) AS CommissionRulePercentage, "
			/* Get the name of the commission rule that applies to this charge*/
			"(select top 1 CommissionRulesT.Name "
			"from CommissionRulesLinkT "
			"INNER join CommissionRulesT on CommissionRulesLinkT.RuleID = CommissionRulesT.ID "
			/*We only want a rule that is valid for the time period of a particular charge*/
			"WHERE CommissionRulesT.IsTieredCommission = 0 "
			"AND convert(datetime, convert(nvarchar, CommissionRulesT.StartDate, 101)) <= ChargeInfoQ.ChargeDate and "
			"convert(datetime, convert(nvarchar, CommissionRulesT.EndDate, 101)) >= ChargeInfoQ.ChargeDate and "
			"coalesce((select sum(dbo.GetChargeTotal(ChargesT.ID)) "
			"from ChargesT "
			"left join LineItemT on ChargesT.ID = LineItemT.ID "
			"where LineItemT.Type = 10 and LineItemT.Deleted = 0 and "
			"convert(datetime, convert(nvarchar, LineItemT.Date, 101)) >= convert(datetime, convert(nvarchar, CommissionRulesT.BasedOnStartDate, 101)) and "
			"convert(datetime, convert(nvarchar, LineItemT.Date, 101)) <= convert(datetime, convert(nvarchar, CommissionRulesT.BasedOnEndDate, 101)) and "
			" ServiceID  = CommissionRulesLinkT.ServiceID AND "
			"DoctorsProviders = CommissionRulesLinkT.ProvID), 0) >= MoneyThreshold and ProvID = ChargeInfoQ.ProvID and ServiceID = ChargeInfoQ.ServiceID "
			"order by MoneyThreshold desc) AS CommissionRule "
			"from "
			"(SELECT LinePays.ID, LinePays.Amount * -1 AS ChargeAmount, "
			"CASE WHEN dbo.GetChargeTotal(ChargesT.ID) <> Convert(money, 0) THEN "
			"	(Round(convert(money, (dbo.GetChargeTotal(ChargesT.ID) - (ServiceLocationInfoT.ShopFee * ChargesT.Quantity))), 2)) "
			"	* LinePays.Amount / dbo.GetChargeTotal(ChargesT.ID) * -1 "
			"ELSE Convert(money, 0) END "
			"AS NoShopFeeAmount, "
			"CASE WHEN dbo.GetChargeTotal(ChargesT.ID) <> Convert(money, 0) THEN "
			"	Round(Convert(money,((([LineCharges].[Amount]*[Quantity]* "
			"		(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*  "
			"		(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*  "
			"		(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*  "
			"		(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*  "
			"	  (CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2)  "
			"	* LinePays.Amount "
			"	/ dbo.GetChargeTotal(ChargesT.ID) * -1 "
			"ELSE Convert(money, 0) END "
			"AS NoTaxTotal, "
			"CASE WHEN dbo.GetChargeTotal(ChargesT.ID) <> Convert(money, 0) THEN "
			"	(Round(convert(money, (  "
			"		Round(Convert(money,((([LineCharges].[Amount]*[Quantity]*  "
			"		(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*  "
			"		(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*  "
			"		(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*  "
			"		(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*  "
			"		(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))))),2)  "
			"	 - (ServiceLocationInfoT.ShopFee * ChargesT.Quantity))), 2)) "
			"	* LinePays.Amount / dbo.GetChargeTotal(ChargesT.ID) * -1 "
			"ELSE Convert(money, 0) END "
			"AS NoTaxNoShopFeeAmount, "
			"PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"ProvidersT.PersonID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
			"ServiceT.ID AS ServiceID, ServiceT.Name AS ServiceName, "
			"CASE WHEN CPTCodeT.ID IS NULL THEN '' ELSE CPTCodeT.Code END AS Code, CommissionT.Percentage AS CommissionBasePercentage, "
			"ReturnedProductsT.DateReturned AS DateReturned, LineCharges.Date AS ChargeDate, LineCharges.InputDate AS ChargeIDate, "
			"CategoriesT.Name AS CategoryName, LocationsT.ID AS LocID, LocationsT.Name AS LocName "
			" "
			"FROM ReturnedProductsT "
			"LEFT JOIN PaymentsT ON ReturnedProductsT.FinAdjID = PaymentsT.ID "
			"LEFT JOIN LineItemT LinePays ON PaymentsT.ID = LinePays.ID "
			"INNER JOIN ChargesT ON ReturnedProductsT.ChargeID = ChargesT.ID "
			"INNER JOIN LineItemT LineCharges ON ChargesT.ID = LineCharges.ID "
			" "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			" LEFT JOIN ServiceLocationInfoT ON ServiceT.ID = ServiceLocationInfoT.ServiceID "
			" AND LineCharges.LocationID = ServiceLocationInfoT.LocationID "
			"LEFT JOIN CommissionT ON ChargesT.ServiceID = CommissionT.ServiceID AND ChargesT.DoctorsProviders = CommissionT.ProvID "
			"LEFT JOIN PersonT ON LinePays.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID   "
			"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID   "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID  "
			"LEFT JOIN LocationsT ON LineCharges.LocationID = LocationsT.ID  "
			"INNER JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			"LEFT JOIN PersonT PersonProv ON ProvidersT.PersonID = PersonProv.ID "
			" "
			"WHERE ProvidersT.UseTieredCommissions = 0 "
			"AND LineCharges.Deleted = 0 AND LinePays.ID Is Not Null AND LinePays.Type = 2 AND LinePays.Deleted = 0) ChargeInfoQ ) CommissionQ "
			"where CommissionBasePercentage IS NOT NULL or CommissionRulePercentage is not null");
		break;


	case 562:
		/* Payment Analysis By Primary Insurance Company by Service Code
		Version History
		JMM - 10/31/2005 - Created
		(e.lally 2007-07-11) PLID 26591 - Replaced CCType with link to CardName, aliased as CCType. This change does not affect the
		field definition of the report.
		// (j.jones 2010-02-04 16:05) - PLID 36500 - re-aliased some of these fields for proper filtering
		// (r.goldschmidt 2014-01-28 13:44) - PLID 60404 - Should display location name for a charge associated with payment (previously, displayed location name for payment)
		*/
		return _T("SELECT ProvID AS ProvID, DocName, TDate AS TDate, Sum(ApplyAmount) AS ApplyAmt, "
			"IDate AS IDate, ItemCode, CPTID, ChargeAmount, Quantity, ItemDesc, LocID AS LocID, Location, InsuranceCoID AS InsuranceCoID, InsName, "
			"PatID AS PatID "
			" FROM ( "
			" SELECT AdjustmentsByCPTCodeFullQ.ProvID AS ProvID,   "
			" 		PersonT.Last + ', ' + PersonT.First AS DocName,   "
			" 		AdjustmentsByCPTCodeFullQ.TDate AS TDate,   "
			" 		AdjustmentsByCPTCodeFullQ.ApplyAmount,   "
			" 		AdjustmentsByCPTCodeFullQ.PatID AS PatID,  "
			" 		AdjustmentsByCPTCodeFullQ.UserDefinedID,   "
			" 		AdjustmentsByCPTCodeFullQ.IDate AS IDate,   "
			" 		AdjustmentsByCPTCodeFullQ.Type,  "
			" 		AdjustmentsByCPTCodeFullQ.ItemCode,  "
			" 		AdjustmentsByCPTCodeFullQ.CPTID AS CPTID,  "
			" 		AdjustmentsByCPTCodeFullQ.ChargeAmount,  "
			" 		AdjustmentsByCPTCodeFullQ.Quantity,  "
			" 		AdjustmentsByCPTCodeFullQ.PayAmt AS PayAmount,  "
			" 		AdjustmentsByCPTCodeFullQ.ServiceDate,  "
			" 		AdjustmentsByCPTCodeFullQ.ItemDesc,  "
			" 		AdjustmentsByCPTCodeFullQ.LocID AS LocID,  "
			" 		AdjustmentsByCPTCodeFullQ.Location, "
			" 		AdjustmentsByCPTCodeFullQ.InsuranceCoID, "
			" 		AdjustmentsByCPTCodeFullQ.InsName, "
			" 		AdjustmentsByCPTCodeFullQ.ChargeID "
			" 		FROM ((((SELECT LineItemT.ID,   "
			" 		ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,   "
			" 		CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID,   "
			" 		PatientsT.PersonID AS PatID,   "
			" 		PatientsT.UserDefinedID,  "
			" 		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			" 		LineItemT_1.InputDate AS IDate,   "
			" 		LineItemT_1.Date AS TDate,   "
			" 		PaymentsT.PayMethod,   "
			" 		LineItemT.Description,   "
			" 		ChargesT.ItemCode AS FirstOfItemCode,   "
			" 		LineItemT.Amount AS PayAmt,   "
			" 		dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,   "
			" 		ChargesT.Quantity,  "
			" 		LineItemT_1.Date AS ServiceDate,   "
			" 		LineItemT_1.Description AS ItemDesc,   "
			" 		CreditCardNamesT.CardName AS CCType,   "
			" 		PaymentPlansT.CheckNo,   "
			" 		ChargesT.Category,   "
			" 		AppliesT.ID AS ApplyID,   "
			" 		LineItemT.ID AS LineID,   "
			" 		LineItemT.Type,  "
			" 		ChargesT.ItemCode,  "
			" 		CPTCodeT.ID AS CPTID,  "
			" 		ChargesT.ID AS ChargeID, "
			" 		InsuredPartyT.InsuranceCoID,  "
			" 		InsuranceCoT.Name as InsName, "
			" 		CASE WHEN LineItemT_1.LocationID IS Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID,  "
			" 		CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location  "
			" 		FROM (((((((((LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,   "
			" 		Sum(AppliesT.Amount) AS ApplyAmt,   "
			" 		/*First of LineItemT_1*/MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,   "
			" 		LineItemT_1.PatientID   "
			" 		FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID  "
			" 		WHERE (((LineItemT_1.Deleted)=0))  "
			" 		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID  "
			" 		HAVING (((LineItemT_1.ID) Is Not Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]))<>0))  "
			" 		) AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID) LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID) LEFT JOIN (ChargesT LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID) ON LineItemT_1.ID = ChargesT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID  "
			"		LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			" 		LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
			" 		LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			" 		WHERE (((PaymentsT.ID) Is Not Null) AND ((LineItemT.Deleted)=0) AND ((LineItemT_1.ID) Is Not Null) AND ((AppliesT.PointsToPayments)=0)) AND CPTCodeT.ID IS NOT NULL  "
			" 		AND PaymentsT.InsuredPartyID IS NOT NULL AND PaymentsT.InsuredPartyID <> -1 AND InsuredPartyT.RespTypeID = 1 "
			" 		GROUP BY LineItemT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End, CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End, PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, LineItemT.InputDate, LineItemT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode, LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineItemT_1.Date, LineItemT_1.Description, CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ChargesT.Category, AppliesT.ID, LineItemT.ID, LineItemT.Type, ChargesT.ItemCode, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END, CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END, CPTCodeT.ID, ChargesT.Quantity, "
			" 		InsuredPartyT.InsuranceCoID, InsuranceCoT.Name, ChargesT.ID, lineItemt_1.InputDate "
			" 		HAVING (((LineItemT.Type)=1))  "
			" 		) AS AdjustmentsByCPTCodeFullQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID  "
			" 		FROM CategoriesT  "
			" 		WHERE (((CategoriesT.Parent)=0))  "
			" 		) AS CategoriesQ ON AdjustmentsByCPTCodeFullQ.Category = CategoriesQ.CategoryID) LEFT JOIN PersonT ON AdjustmentsByCPTCodeFullQ.ProvID = PersonT.ID)) "
			" ) PayerbyInsServiceQ Group By ChargeID, ProvID, DocName, TDate, IDate, ItemCode, CPTID, ChargeAmount, Quantity, ItemDesc, LocID, Location, InsuranceCoID, InsName, PatID");
		break;

	case 563:
		/* Adjustments To Prior Receivables
		Version History
		TES - 11/11/2005 - PLID 15933 - Created, copied just the PAYAPPLY section out of ALL_FINANCIAL_Q
		TES 7/9/2008 - PLID 29580 - We now use AppliesT.InputDate as the relevant date, not the later of the two
		dates involved, which is what we used to use.
		TES 10/13/2009 - PLID 35662 - Restored the old apply date calculation for everything but the input date

		*/

	{
		CString strFromDate = nDateRange == -1 ? "1900-01-01" : FormatDateTimeForSql(DateFrom);
		CString strToDate = nDateRange == -1 ? "5000-12-31" : FormatDateTimeForSql(DateTo + OneDay);
		CString strDate = nDateFilter == 1 ? "BDate" : "InputDate";
		CString strTransferDate = nDateFilter == 1 ? "TransferredFromTDate" : "TransferredFromIDate";

		CString strSql;
		strSql.Format("SELECT TransfersQ.ID, TransfersQ.PatientID AS PatID, TransfersQ.Type, -1*TransfersQ.Amt AS Amt, TransfersQ.Description, TransfersQ.Date, "
			"TransfersQ.InputDate AS IDate, TransfersQ.InputName, TransfersQ.TDate AS TDate, TransfersQ.ProvID AS ProvID, "
			"TransfersQ.LocID AS LocID, TransfersQ.Location, TransfersQ.TransferredFromTDate, TransfersQ.TransferredFromIDate, "
			"PersonPat.Last + ', ' + PersonPat.First + ' ' + PersonPat.Middle AS PatName, PatientsT.UserDefinedID, "
			"CASE WHEN PersonProv.ID Is Null THEN '<None>' ELSE PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle END AS ProvName, "
			"CASE WHEN PersonTransferProv.ID Is Null THEN '<None>' ELSE PersonTransferProv.Last + ', ' + PersonTransferProv.First + ' ' + PersonTransferProv.Middle END AS TransferProviderName, "
			"CASE WHEN TransferLocation.ID Is Null THEN '<None>' ELSE TransferLocation.Name END AS TransferLocation, "
			"TransfersQ.TransferDescription, TransfersQ.SourceTDate, TransfersQ.SourceIDate, TransfersQ.DestTDate, TransfersQ.DestIDate "
			"FROM ("
			"/*Payments Applied (a)*/"
			"SELECT FinType = 'TRANSFER', LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Amount, AppliesSubQ.Description, LineItemT.Date, "
			"AppliesSubQ.InputDate AS InputDate, "
			"LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, "
			"CASE WHEN LineItemT.Date > AppliesSubQ.BDate THEN LineItemT.Date ELSE AppliesSubQ.BDate END AS BDate, "
			"CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS TDate, "
			"0 AS Prepayment, "
			"Round(Convert(money,AppliesSubQ.Amount),2) AS Amt, "
			"AppliesSubQ.ProvID AS ProvID, "
			"'PAYAPPLY' AS RText, "
			"AppliesSubQ.ApplyID AS ApplyID, "
			"LineItemT.ID AS LineID, "
			"AppliesSubQ.LocID AS LocID, "
			"LocationsT.Name AS Location, "
			"NULL AS Code, NULL AS CodeName, 0 AS Quantity, NULL AS Category, '' AS CatName, NULL AS ServiceID, "
			"CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, "
			"CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, "
			"LineItemT.Date AS TransferredFromTDate, "
			"LineItemT.InputDate AS TransferredFromIDate, "
			"PaymentsT.ProviderID AS TransferProvID, "
			"LineItemT.LocationID AS TransferLocID, "
			"LineItemT.Description AS TransferDescription, "
			"LineItemT.Date AS SourceTDate, "
			"LineItemT.InputDate AS SourceIDate, "
			"AppliesSubQ.BDate AS DestTDate, "
			"AppliesSubQ.InputDate AS DestIDate "
			""
			"FROM LineItemT INNER JOIN PaymentsT LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID ON LineItemT.ID = PaymentsT.ID INNER JOIN "
			"(SELECT AppliesT.ID AS ApplyID, SourceID, CASE WHEN DestID IN (SELECT ID FROM PaymentsT WHERE PrePayment = 1) THEN AppliesT.Amount ELSE AppliesT.Amount END AS Amount, "
			"CASE WHEN BillsT.Description Is Null THEN LineItemT.Description ELSE BillsT.Description END AS Description, "
			"CASE WHEN BillsT.Date IS Null THEN LineItemT.Date ELSE BillsT.Date END AS BDate, LineItemT.Date, AppliesT.InputDate, "
			"CASE WHEN ChargesT.ID Is Null THEN PaymentsT.ProviderID ELSE ChargesT.DoctorsProviders END AS ProvID, "
			"LineItemT.LocationID AS LocID "
			"FROM AppliesT INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			") AppliesSubQ "
			""
			"ON LineItemT.ID = AppliesSubQ.SourceID LEFT JOIN LocationsT ON AppliesSubQ.LocID = LocationsT.ID "
			"WHERE PaymentsT.PrePayment = 0 "
			""
			""
			"UNION ALL "
			"/*Payments Applied (b)*/"
			"SELECT FinType = 'TRANSFER', LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Amount, LineItemT.Description, LineItemT.Date, "
			"AppliesSubQ.InputDate AS InputDate, "
			"LineItemT.InputName, LineItemT.Deleted, LineItemT.DeleteDate, LineItemT.DeletedBy, "
			"CASE WHEN LineItemT.Date > AppliesSubQ.BDate THEN LineItemT.Date ELSE AppliesSubQ.BDate END AS BDate, "
			"CASE WHEN LineItemT.Date > AppliesSubQ.Date THEN LineItemT.Date ELSE AppliesSubQ.Date END AS TDate, "
			"0 AS Prepayment, "
			"-1*Round(Convert(money,AppliesSubQ.Amount),2) AS Amt, "
			"PaymentsT.ProviderID AS ProvID, "
			"'PAYAPPLY' AS RText, "
			"AppliesSubQ.ApplyID AS ApplyID, "
			"LineItemT.ID AS LineID, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location, "
			"NULL AS Code, NULL AS CodeName, 0 AS Quantity, NULL AS Category, '' AS CatName, NULL AS ServiceID, "
			"CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, "
			"CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, "
			"LineItemT.Date AS TransferredFromTDate, "
			"LineItemT.InputDate AS TransferredFromIDate, "
			"AppliesSubQ.ProvID AS TransferProvID, "
			"AppliesSubQ.LocID AS TransferLocID, "
			"AppliesSubQ.Description AS TransferDescription, "
			"AppliesSubQ.BDate AS SourceTDate, "
			"AppliesSubQ.InputDate AS SourceIDate, "
			"LineItemT.Date AS DestTDate, "
			"LineItemT.InputDate AS DestIDate "
			""
			"FROM LineItemT INNER JOIN PaymentsT LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID ON LineItemT.ID = PaymentsT.ID INNER JOIN "
			"(SELECT AppliesT.ID AS ApplyID, SourceID, CASE WHEN DestID IN (SELECT ID FROM PaymentsT WHERE PrePayment = 1) THEN Appliest.amount ELSE AppliesT.Amount END AS Amount, "
			"CASE WHEN BillsT.Description Is Null THEN LineItemT.Description ELSE BillsT.Description END AS Description, "
			"CASE WHEN BillsT.Date IS Null THEN LineItemT.Date ELSE BillsT.Date END AS BDate, LineItemT.Date, AppliesT.InputDate, "
			"CASE WHEN ChargesT.ID Is Null THEN PaymentsT.ProviderID ELSE ChargesT.DoctorsProviders END AS ProvID, "
			"LineItemT.LocationID AS LocID "
			"FROM AppliesT INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			") AppliesSubQ "
			""
			"ON LineItemT.ID = AppliesSubQ.SourceID LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"WHERE PaymentsT.PrePayment = 0 "
			") AS TransfersQ "
			"INNER JOIN PersonT PersonPat INNER JOIN PatientsT ON PersonPat.ID = PatientsT.PersonID "
			"ON TransfersQ.PatientID = PersonPat.ID "
			"LEFT JOIN PersonT PersonProv ON TransfersQ.ProvID = PersonProv.ID "
			"LEFT JOIN PersonT PersonTransferProv ON TransfersQ.TransferProvID = PersonTransferProv.ID "
			"LEFT JOIN LocationsT TransferLocation ON TransfersQ.TransferLocID = TransferLocation.ID "
			"WHERE (TransfersQ.%s < '%s' AND TransfersQ.%s >= '%s') AND (TransfersQ.%s >= '%s' OR TransfersQ.%s < '%s')",
			strDate, strToDate, strDate, strFromDate, strTransferDate, strToDate, strTransferDate, strFromDate);
		return strSql;
	}
	break;

	case 586:
		//Bills with Applies Split by Resp
		/*Version History
		// (j.gruber 2007-03-30 09:39) - PLID 25365 - created
		// (j.gruber 2010-06-09 14:55) - PLID 39065 - added primary ref source, ref phys, and ref pat
		*/
		return _T(" SELECT ChargesT.ID As ChargeID, DoctorsProviders AS ProvID, Sum(ChargeRespT.Amount) AS ChargeAmount,  LineCharges.LocationID AS LocID,  "
			" LineCharges.PatientID AS PatID, PersonT.First, PersonT.Middle, PersonT.Last,  PatientsT.UserDefinedID, ServiceT.ID as ServiceID,  "
			" CASE WHEN CPTCodeT.ID IS NULL THEN '' ELSE CPTCodeT.Code END AS CPTCode, ServiceT.Name as ServiceName,  LineCharges.Date AS ChargeDate,  "
			" CASE WHEN LinePays.Date IS NULL THEN Convert(datetime, '1899-12-30') ELSE CAST(Convert(nVarChar, LinePays.Date, 23) as datetime) END AS PayDate, LineCharges.InputDate AS ChargeIDate,  "
			" CASE WHEN LinePays.Date IS NULL THEN Convert(datetime, '1899-12-30') ELSE CAST(Convert(nVarChar, LinePays.InputDate, 23) as datetime) END AS PayIDate,   "
			" ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, ProvT.Title as ProvTitle,    "
			" SUM (CASE WHEN ChargeRespT.InsuredPartyID IS NULL OR ChargeRespT.InsuredPartyID = -1 THEN ChargeRespT.Amount  	  ELSE 0       END) AS PatAmount,   "
			" SUM (CASE WHEN InsuredPartyT.RespTypeID = 1 THEN ChargeRespT.Amount  	  ELSE 0       END) AS PriAmount,   "
			" SUM (CASE WHEN InsuredPartyT.RespTypeID = 2 THEN ChargeRespT.Amount  	  ELSE 0       END) AS SecAmount,   "
			" CASE WHEN AppliesT.Amount IS NULL THEN 0 ELSE CASE WHEN LinePays.Type = 1 THEN AppliesT.Amount ELSE 0 END END AS AppliedPay,   "
			" PaymentsT.InsuredPartyID,  "
			" (SELECT PersonID FROM InsuredPartyT WHERE InsuredPartyT.PatientID = PatientsT.PersonID AND RespTypeID = 1) AS PriIns, "
			" (SELECT PersonID FROM InsuredPartyT WHERE InsuredPartyT.PatientID = PatientsT.PersonID AND RespTypeID = 2) AS SecIns, "
			" CASE WHEN PaymentsT.ID IS NOT NULL THEN CASE WHEN LinePays.Type = 1 THEN CASE WHEN PaymentsT.InsuredPartyID = -1 THEN AppliesT.Amount ELSE 0 END ELSE 0 END ELSE 0 END AS PatPays, "
			" CASE WHEN AppliesT.Amount IS NULL THEN 0  	ELSE CASE WHEN LinePays.Type = 2 THEN AppliesT.Amount   ELSE 0   	END END AS Adjustments,   "
			" PaymentsT.Paymethod, "
			" (SELECT SUM(Amount) FROM AppliesT WHERE DestID IN (SELECT ID FROM ChargesT WHERE ChargesT.BillID = BillsT.ID) AND SourceID IN(SELECT ID FROM LineItemT WHERE Type = 1 AND DELETED = 0) ) AS AppliedPayAmt, "
			" (SELECT SUM(Amount) FROM AppliesT WHERE DestID IN (SELECT ID FROM ChargesT WHERE ChargesT.BillID = BillsT.ID) AND SourceID IN(SELECT ID FROM LineItemT WHERE Type = 2 AND DELETED = 0) ) AS AppliedAdjAmt, "
			" LinePays.Description AS PayDescription, PaymentsT.ID AS PaymentID, "
			" (SELECT Name FROM InsuranceCoT WHERE InsuranceCoT.PersonID = (SELECT InsuranceCoID FROM InsuredPartyT WHERE InsuredPartyT.PersonID = PaymentsT.InsuredPartyID)) AS InsuranceCoName, "
			" BillsT.ID AS BillID, BillsT.Description AS BillDesc, BillsT.Date AS BillDate, dbo.GetBillTotal(BillsT.ID) As BillTotal, "
			" ReferralSourceT.Name as PriRefSourceName, PatientsT.ReferralID as PriRefID, "
			" RefPhysPersonT.Id as RefPhysPersonID, RefPhysPersonT.First as RefPhysFirst, RefPhysPersonT.Middle as RefPhysMiddle, RefPhysPersonT.Last as RefPhysLast, RefPhysPersonT.Title as RefPhysTitle, "
			" RefPatPersonT.Id as RefPatPersonID, RefPatPersonT.First as RefPatFirst, RefPatPersonT.Middle as RefPatMiddle, RefPatPersonT.Last as RefPatLast, RefPatPersonT.Title as RefPatTitle "


			" FROM ChargesT INNER JOIN LineItemT LineCharges ON ChargesT.ID = LineCharges.ID   LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
			" LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID  "
			" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			" LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID  "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID   "
			" LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID   "
			" LEFT JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  LEFT JOIN LineItemT LinePays ON PaymentsT.ID = LinePays.ID   "
			" LEFT JOIN PatientsT ON LineCharges.PatientID = PatientsT.PersonID  LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
			" LEFT JOIN PersonT ProvT ON ChargesT.DoctorsProviders = ProvT.ID   "
			" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			" LEFT JOIN PersonT RefPhysPersonT ON PatientsT.DefaultReferringPhyID = RefPhysPersonT.ID "
			" LEFT JOIN PersonT RefPatPersonT ON PatientsT.ReferringPatientID = RefPatPersonT.ID "

			" WHERE (LineCharges.Deleted = 0 AND (LinePays.Deleted IS NULL OR LinePays.Deleted = 0)) "
			" AND BillID IN (SELECT BillID FROM ChargesT WHERE ID IN (SELECT DestID FROM AppliesT)) "

			" GROUP BY ChargesT.ID, ChargesT.DoctorsProviders, LineCharges.LocationID, LineCharges.PatientID,  PersonT.First, PersonT.Middle, PersonT.Last,  "
			" PatientsT.UserDefinedID, ServiceT.ID, CPTCodeT.Code,  ServiceT.Name, CPTCodeT.ID, LineCharges.Date, LineCharges.InputDate,   "
			" ProvT.First, ProvT.Middle, ProvT.Last, ProvT.Title, LinePays.Date, LinePays.InputDate, LinePays.Type, AppliesT.Amount,  "
			" PaymentsT.InsuredPartyID, PatientsT.PersonID, PaymentsT.ID, PaymentsT.Paymethod, AppliesT.SourceID, LinePays.Description, "
			" BillsT.ID, BillsT.Description, BillsT.Date, "
			" ReferralSourceT.Name, PatientsT.ReferralID, "
			" RefPhysPersonT.Id, RefPhysPersonT.First, RefPhysPersonT.Middle, RefPhysPersonT.Last, RefPhysPersonT.Title, "
			" RefPatPersonT.Id, RefPatPersonT.First, RefPatPersonT.Middle, RefPatPersonT.Last, RefPatPersonT.Title"
			);
		break;

	case 594: // 
			  // Patient Reward History
			  /*Version History
			  // (a.walling 2007-06-06 11:03) - PLID 25976 - Created
			  */
		return _T(
			"SELECT RewardHistoryT.PatientID AS PatID, "
			"PersonT.First, "
			"PersonT.Middle, "
			"PersonT.Last, "
			"PatientsT.UserDefinedID AS PatientID, "
			"RewardHistoryT.ModifiedDate AS Date, "
			"RewardHistoryT.DeletedDate, "
			"RewardHistoryT.Source, "
			"CASE  "
			"WHEN RewardHistoryT.Source = -1 THEN 'Redeemed by Charge' "
			"WHEN RewardHistoryT.Source = 0 THEN 'Per Bill' "
			"WHEN RewardHistoryT.Source = 1 THEN 'Per Bill Amount' "
			"WHEN RewardHistoryT.Source = 2 THEN 'Per Bill Charges' "
			"WHEN RewardHistoryT.Source = 3 THEN 'Referred Patient' "
			"WHEN RewardHistoryT.Source = 4 THEN 'Referred Patient Bill' "
			"WHEN RewardHistoryT.Source = 5 THEN 'Referred Patient Bill Amount' "
			"WHEN RewardHistoryT.Source = 6 THEN 'Referred Patient Bill Points' "
			"END AS SourceReason, "
			"RewardHistoryT.Deleted, "
			"CASE  "
			"WHEN RewardHistoryT.Deleted = 0 THEN '' "
			"WHEN RewardHistoryT.Deleted = 1 THEN 'Removed' "
			"WHEN RewardHistoryT.Deleted = 2 THEN 'Changed' "
			"END AS DeletedReason, "
			"RewardHistoryT.Points, "
			"CASE WHEN RewardHistoryT.Points > 0 AND RewardHistoryT.Deleted = 0 THEN RewardHistoryT.Points ELSE 0 END AS PointsAccumulated, "
			"CASE WHEN RewardHistoryT.Points < 0 AND RewardHistoryT.Deleted = 0 THEN ABS(RewardHistoryT.Points) ELSE 0 END AS PointsRedeemed, "
			"RefPersonT.First AS RefFirst, "
			"RefPersonT.Middle AS RefMiddle, "
			"RefPersonT.Last AS RefLast, "
			"RefPatientsT.UserDefinedID AS RefPatientID, "
			"BillsT.Description "
			"FROM   RewardHistoryT "
			"LEFT JOIN PersonT "
			"ON RewardHistoryT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT "
			"ON RewardHistoryT.PatientID = PatientsT.PersonID "
			"LEFT JOIN PersonT RefPersonT "
			"ON RewardHistoryT.RefPatientID = RefPersonT.ID "
			"LEFT JOIN PatientsT RefPatientsT "
			"ON RewardHistoryT.RefPatientID = RefPatientsT.PersonID "
			"LEFT JOIN BillsT "
			"ON RewardHistoryT.BillID = BillsT.ID ");
		//"ORDER BY Date" // (a.walling 2007-09-24 10:14) - PLID 25976 - The report should handle all ordering
		break;

	case 595:
		// Patient Reward Points
		/* Version History
		// (a.walling 2007-06-06 12:44) - PLID 25976 - Created
		// (a.walling 2007-09-25 17:18) - PLID 25976 - Moved to Financial
		*/
		return _T(
			"SELECT PersonT.First, "
			"PersonT.Middle,  "
			"PersonT.Last,  "
			"PatientsT.UserDefinedID AS PatientID,  "
			"PatientsT.PersonID      AS PatID,  "
			"PatientsT.MainPhysician AS ProvID,  "
			"PersonT.Location        AS LocID,  "
			"LocationsT.Name         AS LocationName,  "
			"PhysT.First             AS ProvFirst,  "
			"PhysT.Middle            AS ProvMiddle,  "
			"PhysT.Last              AS ProvLast,  "
			"SUM(Points)             AS Points  "
			"FROM PersonT  "
			"INNER JOIN PatientsT  "
			"ON PersonT.ID = PatientsT.PersonID  "
			"INNER JOIN RewardHistoryT  "
			"ON PatientsT.PersonID = RewardHistoryT.PatientID  "
			"LEFT JOIN LocationsT  "
			"ON PersonT.Location = LocationsT.ID  "
			"LEFT JOIN PersonT PhysT  "
			"ON PatientsT.MainPhysician = PhysT.ID  "
			"WHERE RewardHistoryT.Deleted = 0  "
			"GROUP BY PersonT.First,  "
			"PersonT.Middle,  "
			"PersonT.Last,  "
			"PatientsT.UserDefinedID,  "
			"PatientsT.PersonID,  "
			"PatientsT.MainPhysician,  "
			"PersonT.Location,  "
			"LocationsT.Name,  "
			"PhysT.First,  "
			"PhysT.Middle,  "
			"PhysT.Last  ");
		break;

		// (j.jones 2010-08-27 13:11) - PLID 40244 - moved Credit Card Batch Processing to ReportInfoPreview


	case 611: // Charges Billed Below Standard Fee
			  /*	Revision History
			  - TES 12/19/02: Fixed to not include charges from quotes, and put in "AS ServiceID so the external filter wouldn't crash".
			  DRT 4/10/2006 - PLID 11734 - Replaced ProcCode.
			  (e.lally 2007-07-18) PLID 26131 - Added charge category and parent charge category
			  // (d.moore 2007-08-23) - PLID 27164 - This was originally named Discount Analysis. That report
			  //  needed to be modified, so this report was created just in case any client needs it.
			  // (j.gruber 2009-03-26 09:08) - PLID 3359 - updated discount structure
			  */
		return _T("SELECT PatientsT.PersonID AS PatID, LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
			"ChargesT.DoctorsProviders AS ProvID, LineItemT.LocationID AS LocID, "
			"(CASE WHEN CPTCodeT.ID IS NOT NULL THEN ItemCode ELSE '' END) AS CPTCode, "
			"ServiceT.Name AS ServiceName, ChargesT.ServiceID AS ServiceID, Round(Convert(money, LineItemT.Amount * Quantity * (CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END)),2) AS ChargeAmount, "
			"Round(Convert(money,ServiceT.Price * Quantity),2) AS OrigAmount, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, Quantity, "
			"(CASE WHEN DoctorsT.Last Is Null THEN '<No Provider>' ELSE DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle END) AS DocName, "
			"CategoriesT.Name AS ChargeCategory, ParentCat.Name AS ParentChargeCategory "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT AS DoctorsT ON ChargesT.DoctorsProviders = DoctorsT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"LEFT JOIN CategoriesT ParentCat ON CategoriesT.Parent = ParentCat.ID "
			"WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND LineItemT.Type = 10 AND Round(Convert(money, LineItemT.Amount * Quantity * (CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END)),2) < Round(Convert(money,ServiceT.Price * Quantity),2) ");
		break;

	case 612: // Charges Billed Below Standard Fee By Procedure
			  /*	Revision History
			  - JMJ 2/17/05: Created (based on Discount Analysis)
			  DRT 4/10/2006 - PLID 11734 - Replaced ProcCode.
			  // (d.moore 2007-08-23) - PLID 27164 - This was originally named Discount Analysis By Procedure. That report
			  //  needed to be modified, so this report was created just in case any client needs it.
			  // (j.gruber 2009-03-26 09:10) - PLID 33359 - updated discount structure
			  */
		return _T("SELECT PatientsT.PersonID AS PatID, LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
			"ChargesT.DoctorsProviders AS ProvID, LineItemT.LocationID AS LocID, "
			"(CASE WHEN CPTCodeT.ID IS NOT NULL THEN ItemCode ELSE '' END) AS CPTCode, "
			"ServiceT.Name AS ServiceName, ChargesT.ServiceID AS ServiceID, Round(Convert(money, LineItemT.Amount * Quantity * (CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END)),2) AS ChargeAmount, "
			"Round(Convert(money,ServiceT.Price * Quantity),2) AS OrigAmount, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, Quantity, "
			"(CASE WHEN DoctorsT.Last Is Null THEN '<No Provider>' ELSE DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle END) AS DocName, "
			"ServiceT.ProcedureID AS ProcID, (CASE WHEN ProcedureT.Name Is Null THEN '<No Procedure>' ELSE ProcedureT.Name END) AS ProcedureName "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT AS DoctorsT ON ChargesT.DoctorsProviders = DoctorsT.ID "
			"LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
			"WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND LineItemT.Type = 10 AND Round(Convert(money, LineItemT.Amount * Quantity * (CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END)),2) < Round(Convert(money,ServiceT.Price * Quantity),2) ");
		break;

	case 631: // EMNs With Charges
			  /*	Revision History
			  // (j.jones 2008-07-03 13:09) - PLID 18534 - created
			  // (j.jones 2011-03-28 15:36) - PLID 42575 - ensured that we hid
			  // non-billable CPT codes
			  // (j.jones 2011-07-05 17:32) - PLID 44432 - supported custom statuses
			  // (j.gruber 2014-03-12 15:45) - PLID 61353 - Update for ICD-10
			  */
		switch (nSubLevel) {
		case 1:
			//these subreports need to join EMRMasterT and expose the filterable field names
			//so that any filters applied to the main query can also apply to these subqueries,
			//such that the subqueries return only the data needed for the report as a whole,
			//as opposed to all charges and all diag codes
			switch (nSubRepNum) {
			case 1:	//EMN Charges						
				return _T("SELECT EMRID, ServiceT.Name AS ServiceName, "
					"CPTCodeT.Code AS ServiceCode, CPTCodeT.SubCode AS ServiceSubCode, "
					"EMRChargesT.Description AS ChargeDescription, EMRChargesT.UnitCost, EMRChargesT.Quantity, "
					"Round(Convert(money, EMRChargesT.Quantity * EMRChargesT.UnitCost), 2) AS LineTotal, "
					"CASE WHEN EMRChargesT.ID IN (SELECT EMRChargeID FROM EMRQuotedChargesT INNER JOIN LineItemT ON EMRQuotedChargesT.ChargeID = LineItemT.ID WHERE LineItemT.Deleted = 0) THEN 'Yes' ELSE 'No' END AS HasChargeBeenQuoted, "
					"EMRMasterT.PatientID AS PatID, EMRMasterT.Date AS Date, EMRMasterT.LocationID AS LocID, "
					"Convert(bit, CASE WHEN EMRMasterT.ID IN (SELECT EMNID FROM BilledEMNsT WHERE BillID IN (SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) THEN 1 ELSE 0 END) AS HasEMNBeenBilled "
					"FROM EMRChargesT "
					"INNER JOIN EMRMasterT ON EMRChargesT.EMRID = EMRMasterT.ID "
					"INNER JOIN ServiceT ON EMRChargesT.ServiceID = ServiceT.ID "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"LEFT JOIN EMNBillsSentToHL7T ON EMRMasterT.ID = EMNBillsSentToHL7T.EMNID "
					"WHERE EMRChargesT.Deleted = 0 AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
					"AND EMNBillsSentToHL7T.EMNID IS NULL ");
				break;
			default://EMN Diag Codes
				return _T("SELECT EMRDiagCodesT.EMRID, \r\n"
					"ICD9T.CodeNumber as ICD9Code, \r\n"
					"ICD10T.CodeNumber as ICD10Code, \r\n"
					"ICD9T.CodeDesc as ICD9CodeDesc, \r\n"
					"ICD10T.CodeDesc as ICD10CodeDesc, \r\n"
					"EMRDiagCodesT.OrderIndex, "
					"EMRMasterT.PatientID AS PatID, EMRMasterT.Date AS Date, EMRMasterT.LocationID AS LocID, "
					"Convert(bit, CASE WHEN EMRMasterT.ID IN (SELECT EMNID FROM BilledEMNsT WHERE BillID IN (SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) THEN 1 ELSE 0 END) AS HasEMNBeenBilled "
					"FROM EMRDiagCodesT "
					"INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID "
					"LEFT JOIN DiagCodes ICD9T ON EMRDiagCodesT.DiagCodeID = ICD9T.ID \r\n"
					"LEFT JOIN DiagCodes ICD10T ON EMRDiagCodesT.DiagCodeID_ICD10 = ICD10T.ID \r\n"
					"WHERE EMRDiagCodesT.Deleted = 0 "
					"ORDER BY EMRDiagCodesT.OrderIndex ");
				break;
			}
			break;
		default://Main report
			return _T("SELECT EMRMasterT.PatientID AS PatID, PatientsT.UserDefinedID, EMRMasterT.ID AS EMNID, EMRMasterT.Date AS Date, "
				"EMRMasterT.LocationID AS LocID, LocationsT.Name AS LocName, "
				"dbo.GetEMNProviderList(EMRMasterT.ID) AS ProvList, "
				"dbo.GetEMNSecondaryProviderList(EMRMasterT.ID) AS SecondaryProvList, "
				"dbo.GetEMRString(EMRMasterT.ID) AS ProcedureList, "
				"EMRStatusListT.Name AS Status, "
				"EMRMasterT.Description AS EMNDescription, EMRGroupsT.Description AS EMRDescription, "
				"Last + ', ' + First + ' ' + Middle AS PatientName, GroupTypes.GroupName AS PatientType, "
				"EMRMasterT.PatientLast + ', ' + EMRMasterT.PatientFirst + ' ' + EMRMasterT.PatientMiddle AS EMNPatientName, "
				"EMRMasterT.PatientAge AS EMNPatientAge, CASE WHEN EMRMasterT.PatientGender = 1 THEN 'Male' WHEN EMRMasterT.PatientGender = 2 THEN 'Female' ELSE '' END AS EMNPatientGender, "
				"Convert(bit, CASE WHEN EMRMasterT.ID IN (SELECT EMNID FROM BilledEMNsT WHERE BillID IN (SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) THEN 1 ELSE 0 END) AS HasEMNBeenBilled "
				"FROM EMRMasterT "
				"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
				"INNER JOIN PersonT ON EMRMasterT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				"LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
				"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
				"WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.ID IN ("
				"	SELECT EMRID "
				"	FROM EMRChargesT "
				"	LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
				"	LEFT JOIN EMNBillsSentToHL7T ON EMRChargesT.EMRID = EMNBillsSentToHL7T.EMNID "
				"	WHERE Deleted = 0 AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1 "
				"	AND EMNBillsSentToHL7T.EMNID IS NULL) "
				")");
			break;
		}

	case 682: {	//Unapplied PrePayments

				// (j.jones 2009-09-28 10:58) - PLID 33413 - created

		CString strApplyFilter = "";
		//if we're filtering on a date range, ensure we only include applies that occurred
		//before the end of that date range (it doesn't matter if our filter is service or
		//input date, applies only have input date)
		if (nDateRange > 0 && DateTo.GetStatus() != COleDateTime::invalid) {
			strApplyFilter.Format(" AND AppliesT.InputDate < DateAdd(day, 1, '%s')", FormatDateTimeForSql(DateTo));
		}

		CString strSql;
		strSql.Format("SELECT PatientsT.UserDefinedID as PatientID, PatientsT.PersonID AS PatID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"LineItemT.Description, LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
			"LineItemT.Amount, "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN "
			"	(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END) "
			"ELSE "
			"	(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) AS Balance, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName, "
			"PersonT1.ID AS ProvID, "
			"LocationsT.Name AS Location, "
			"LocationsT.ID AS LocID "
			"FROM PaymentsT INNER JOIN "
			"	(LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PaymentsT.ID = LineItemT.ID INNER JOIN "
			"	PatientsT ON "
			"	LineItemT.PatientID = PatientsT.PersonID INNER JOIN "
			"	PersonT ON PatientsT.PersonID = PersonT.ID LEFT JOIN "
			"	PersonT PersonT1 ON "
			"	PaymentsT.ProviderID = PersonT1.ID LEFT OUTER JOIN "
			"	AppliesT AppliesT1 ON "
			"	LineItemT.ID = AppliesT1.DestID LEFT OUTER JOIN "
			"	AppliesT ON "
			"	LineItemT.ID = AppliesT.SourceID LEFT OUTER JOIN "
			"	PaymentPlansT ON "
			"	PaymentsT.ID = PaymentPlansT.ID "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"LEFT JOIN "
			/* This will total everything applied to a prepayment */
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID "
			"FROM "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID "
			"WHERE (LineItemT.Deleted = 0) %s "
			"GROUP BY AppliesT.DestID "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID "
			"LEFT JOIN "
			/* This will total everything that the prepayment is applied to */
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID "
			"FROM "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID "
			"WHERE LineItemT.Deleted = 0 %s "
			"GROUP BY AppliesT.SourceID "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID "
			""
			"WHERE (LineItemT.Deleted = 0) "
			"AND (PaymentsT.Prepayment = 1) "
			"AND "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN "
			"	(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END) "
			"ELSE "
			"	(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) <> 0 "
			"GROUP BY LineItemT.ID, PatientsT.UserDefinedID, PatientsT.PersonID, "
			"PaymentsT.ProviderID, PaymentsT.PayMethod, "
			"LineItemT.Description, "
			"LineItemT.Amount, "
			"(CASE WHEN PrepayAppliedToQ.ID IS NULL THEN "
			"	(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END) "
			"ELSE "
			"	(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END), "
			"LineItemT.Date, PaymentPlansT.CheckNo, "
			"CreditCardNamesT.CardName, LineItemT.InputDate, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
			"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle, "
			"PersonT1.ID, "
			"LocationsT.Name, "
			"LocationsT.ID", strApplyFilter, strApplyFilter);

		return _T(strSql);
		break;
	}

	case 684: //Claim Submission Percentages By Responsibility

			  // (j.jones 2009-10-07 11:52) - PLID 35776 - created
			  //(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values

		return _T(
			FormatString(
				"SELECT ClaimHistoryQ.ID, PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First +  ' ' + PersonT.Middle AS PatName, "
				"PatientsT.UserDefinedID, BillsT.ID AS BillID, BillsT.Date AS TDate, BillsT.InputDate AS IDate, BillsT.Description, "
				"dbo.GetBillTotal(BillsT.ID) AS BillTotal, ClaimHistoryQ.LastSentDate AS LastSentDate, "
				"CASE WHEN ClaimHistoryQ.SendType = %li THEN 'Electronic' ELSE 'Paper' END AS MethodSent, "
				"RespTypeT.ID AS RespTypeID, RespTypeT.TypeName AS RespType, InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, "
				"BillLocationQ.LocationID AS LocID "
				"FROM BillsT "
				"INNER JOIN (SELECT BillID, Max(LocationID) AS LocationID "
				"	FROM LineItemT "
				"	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"	WHERE Deleted = 0 "
				"	GROUP BY BillID "
				"	) AS BillLocationQ ON BillsT.ID = BillLocationQ.BillID "
				"INNER JOIN (SELECT Max(ID) AS ID, BillID, InsuredPartyID, SendType, Max(Date) AS LastSentDate "
				"	FROM ClaimHistoryT "
				"	WHERE SendType <> %li "
				"	GROUP BY BillID, InsuredPartyID, SendType "
				"	) AS ClaimHistoryQ ON BillsT.ID = ClaimHistoryQ.BillID "
				"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"INNER JOIN InsuredPartyT ON ClaimHistoryQ.InsuredPartyID = InsuredPartyT.PersonID "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1",
				ClaimSendType::Electronic, ClaimSendType::TracerLetter));
		break;


	case 695: //Collection Percentages by Place of Service
			  // (j.gruber 2010-07-14 11:49) - PLID 37356 - created for
			  // (j.gruber 2010-12-06 13:50) - PLID 41630 - added RespType Info
			  // (j.jones 2015-02-24 09:37) - PLID 64103 - added RespTypeT.CategoryPlacement
		return _T("SELECT PatientsT.PersonID AS PatID,  "
			"PatientsT.UserDefinedID as PatientID,  "
			"PersonT.Zip,  "
			"LineItemT.Date AS TDate, "
			"BillsT.Date AS BDate,  "
			"ChargesT.ID as ChargeID,  "
			"ChargesT.ItemCode,  "
			"LineItemT.Description,  "
			"PersonProvT.Last + ', ' + PersonProvT.First + ' ' + PersonProvT.Middle AS ProvName,  "
			"ChargesT.Quantity,  "
			"ChargeRespT.Amount AS RespAmount,  "
			"ChargesT.BillID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"ProvidersT.PersonID AS ProvID,  "
			"LineItemT.Amount AS LineItemAmt,  "
			"CategoriesT.ID AS CatID,  "
			"CategoriesT.Name AS CatName,  "
			"BillsT.Location AS ServiceLocation, "
			"POST.Name AS POService, "
			"PersonT.City,  "
			"PersonT.State,  "
			"LineItemT.InputDate AS IDate, "
			"LineItemT.LocationID AS LocID, "
			"LocationsT.Name AS Location, "
			"ChargesT.ServiceID AS CPTID, "
			"IsNull(ApplyPaysQ.Amount, Convert(money, 0)) AS AppliedPays, "
			"IsNull(ApplyAdjsQ.Amount, Convert(money, 0)) as AppliedAdjs, "
			" InsuranceCoT.Name as InsCoName, "
			" InsuredPartyT.RespTypeID, "
			" RespTypeT.TypeName, RespTypeT.Priority, RespTypeT.CategoryType as RespCat, RespTypeT.CategoryPlacement "
			"FROM ChargesT LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			" LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			" LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			" LEFT JOIN PersonT PersonProvT ON ProvidersT.PersonID = PersonProvT.ID "
			" INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			" LEFT JOIN LocationsT AS POST ON BillsT.Location = POST.ID "
			" LEFT JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
			" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			" LEFT JOIN (SELECT IChargeRespDetailT.ChargeRespID, Sum(IApplyDetailsT.Amount) as Amount "
			" FROM ApplyDetailsT IApplyDetailsT "
			" LEFT JOIN ChargeRespDetailT IChargeRespDetailT ON IApplyDetailsT.DetailID = IChargeRespDetailT.ID "
			" INNER JOIN AppliesT IAppliesT ON IApplyDetailsT.ApplyID = IAppliesT.ID "
			" INNER JOIN (SELECT * FROM LineItemT WHERE Deleted = 0 and Type = 1) LinePays "
			" ON IAppliesT.SourceID = LinePays.ID GROUP BY IChargeRespDetailT.ChargeRespID) ApplyPaysQ ON ApplyPaysQ.ChargeRespID = ChargeRespT.ID "
			" LEFT JOIN (SELECT IChargeRespDetailT.ChargeRespID, Sum(IApplyDetailsT.Amount) as Amount "
			" FROM ApplyDetailsT IApplyDetailsT "
			" LEFT JOIN ChargeRespDetailT IChargeRespDetailT ON IApplyDetailsT.DetailID = IChargeRespDetailT.ID "
			" INNER JOIN AppliesT IAppliesT ON IApplyDetailsT.ApplyID = IAppliesT.ID "
			" INNER JOIN (SELECT * FROM LineItemT WHERE Deleted = 0 and Type = 2) LineAdjs "
			" ON IAppliesT.SourceID = LineAdjs.ID GROUP BY IChargeRespDetailT.ChargeRespID) ApplyAdjsQ ON ApplyAdjsQ.ChargeRespID = ChargeRespT.ID "
			"WHERE (((BillsT.EntryType)=1) AND ((BillsT.Deleted)=0) AND ((LineItemT.Deleted)=0)) AND GCTypesT.ServiceID IS NULL ");
		break;

	case 703: //Fully Adjusted Insurance Responsibilities

			  // (j.jones 2011-03-11 09:57) - PLID 41787 - created

		return _T("SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' '+ PersonT.Middle AS PatientName, "
			"BillsT.Date AS BillDate, LineItemT.Date AS TDate, "
			"LineItemT.InputDate AS IDate, "
			"ProviderPersonT.ID AS ProvID, "
			"ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' '+ ProviderPersonT.Middle AS ProvName, "
			"LocationsT.ID AS LocID, "
			"LocationsT.Name AS LocName, "
			"BillsT.Description AS BillDescription, "
			" LineItemT.Description AS ChargeDescription, "
			"ChargesT.ItemCode AS ChargeCode, "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal, "
			"ChargeRespT.Amount AS RespTotal, "
			"AppliedAdjsQ.TotalAppliedToResp AS TotalAdjusted, "
			"InsuranceCoT.PersonID AS InsCoID, "
			"InsuranceCoT.Name AS InsuranceCompany, "
			"RespTypeT.ID AS RespTypeID, "
			"RespTypeT.TypeName AS Responsibility, "
			"ChargeRespT.ID AS ChargeRespID, "
			"AdjLineItemT.Date AS AdjServiceDate, AdjLineItemT.InputDate AS AdjInputDate, "
			"AdjProviderPersonT.Last + ', ' + AdjProviderPersonT.First + ' '+ AdjProviderPersonT.Middle AS AdjProvName, "
			"AdjLocationsT.Name AS AdjLocName, "
			"AdjLineItemT.Description AS AdjDescription, "
			"AdjLineItemT.Amount AS AdjAmount, "
			"AppliesT.Amount AS AdjAppliedAmount, "
			"PaymentGroupsT.GroupName AS AdjCategory, "
			"AdjustmentGroupCodesT.Code AS GroupCode, "
			"Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
			"AdjustmentReasonCodesT.Code AS ReasonCode, "
			"Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT ProviderPersonT ON ChargesT.DoctorsProviders = ProviderPersonT.ID "
			"INNER JOIN (SELECT Sum(AppliesT.Amount) AS TotalAppliedToResp, AppliesT.RespID "
			"FROM LineItemT  "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID	 "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
			"AND PaymentsT.InsuredPartyID Is Not Null AND PaymentsT.InsuredPartyID > 0 "
			"GROUP BY AppliesT.RespID) AS AppliedAdjsQ ON ChargeRespT.ID = AppliedAdjsQ.RespID "
			"INNER JOIN AppliesT ON AppliedAdjsQ.RespID = AppliesT.RespID "
			"INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
			"INNER JOIN LineItemT AdjLineItemT ON PaymentsT.ID = AdjLineItemT.ID "
			"INNER JOIN LocationsT AdjLocationsT ON AdjLineItemT.LocationID = AdjLocationsT.ID "
			"LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"LEFT JOIN PersonT AdjProviderPersonT ON PaymentsT.ProviderID = AdjProviderPersonT.ID "
			"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
			"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
			"WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND AdjLineItemT.Deleted = 0 "
			"AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
			"AND AdjLineItemT.Type = 2 "
			"AND AppliedAdjsQ.TotalAppliedToResp = ChargeRespT.Amount");
		break;

	case 711: /*slurpee*/
			  //Charges Fully Adjusted By E-Remittance

			  // (j.jones 2011-07-15 11:16) - PLID 44582 - created

		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"PatientsT.UserDefinedID, "
			"PatientsT.PersonID AS PatID, "
			"BillsT.Date AS BillDate, "
			"BillsT.ID AS BillID, "
			"LineItemTCharges.Date AS ChargeDate, "
			"BatchPaymentsT.Date AS TDate, "
			"BatchPaymentsT.InputDate AS IDate, "
			"ChargesT.ID AS ChargeID, "
			"ChargesT.ItemCode AS Code, "
			"LineItemTCharges.Description AS ChargeDescription, "
			"dbo.GetChargeTotal(LineItemTCharges.ID) AS ChargeAmount, "
			"AppliesQ.TotalApplied, AppliesT.Amount AS AppliedAmount, "
			"LineItemT.Amount AS AdjustmentAmount, LineItemT.Date AS AdjustmentDate, LineItemT.InputDate AS AdjCreatedDate, "
			"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsuranceCompany, "
			"LineItemT.Description AS AdjustmentDescription, "
			"ProviderPersonT.ID AS ProvID, "
			"ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' '+ ProviderPersonT.Middle AS ProvName, "
			"LocationsT.ID AS LocID, "
			"LocationsT.Name AS LocName, "
			"PaymentGroupsT.GroupName AS AdjCategory, "
			"AdjustmentGroupCodesT.Code AS GroupCode, "
			"Convert(nvarchar(4000), AdjustmentGroupCodesT.Description) AS GroupCodeDesc, "
			"AdjustmentReasonCodesT.Code AS ReasonCode, "
			"Convert(nvarchar(4000), AdjustmentReasonCodesT.Description) AS ReasonCodeDesc "
			"FROM LineItemT LineItemTCharges "
			"INNER JOIN ChargesT ON LineItemTCharges.ID = ChargesT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN PatientsT ON LineItemTCharges.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			//calculate the total amount adjusted by one batch payment,
			//but only for EOBs that actually paid zero
			"INNER JOIN (SELECT Sum(AppliesT.Amount) AS TotalApplied, AppliesT.DestID, "
			"	PaymentsT.InsuredPartyID, BatchPaymentsT.ID AS BatchPayID "
			"	FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	INNER JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID "
			"	INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"	INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
			"	INNER JOIN (SELECT BatchPayID, ChargeID FROM ERemittanceHistoryT "
			"		WHERE PaymentAmount = Convert(money,0) "
			"		GROUP BY BatchPayID, ChargeID "
			"	) AS ERemitQ ON BatchPaymentsT.ID = ERemitQ.BatchPayID AND ChargesT.ID = ERemitQ.ChargeID "
			"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
			"	GROUP BY AppliesT.DestID, PaymentsT.InsuredPartyID, BatchPaymentsT.ID "
			") AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "
			"INNER JOIN InsuredPartyT ON AppliesQ.InsuredPartyID = InsuredPartyT.PersonID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
			"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"INNER JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID "
			"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN PersonT ProviderPersonT ON ChargesT.DoctorsProviders = ProviderPersonT.ID "
			"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
			"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
			"LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			//filter only on EOBs that paid zero
			"INNER JOIN (SELECT BatchPayID, ChargeID FROM ERemittanceHistoryT "
			"	WHERE PaymentAmount = Convert(money,0) "
			"	GROUP BY BatchPayID, ChargeID "
			") AS ERemitQ ON BatchPaymentsT.ID = ERemitQ.BatchPayID AND ChargesT.ID = ERemitQ.ChargeID "
			"WHERE LineItemT.Deleted = 0 "
			"AND LineItemT.Type = 2 AND LineItemTCharges.Deleted = 0 "
			"AND AppliesQ.BatchPayID = BatchPaymentsT.ID "
			"AND AppliesQ.TotalApplied = dbo.GetChargeTotal(LineItemTCharges.ID)");

		break;

	case 713:
		// (j.dinatale 2011-12-01 11:57) - PLID 46716 - LineItemTypeText changed to OrigLineItemTypeText
		// (j.dinatale 2011-07-11 17:21) - PLID 44807
		// (c.haag 2016-02-22) - PLID 68257 - Changed UNIONs to UNION ALLs because like records would be unintentionally grouped together
		return _T(
			"SELECT FinancialCorrectionsQ.ID AS ID, PatID AS PatID, Description, OrigLineItemType AS OrigLineItemType, Amount, LocID AS LocID, \r\n"
			"			ProvID AS ProvID, IsVoidingLineItem, IsNewLineItem, ParentLineItemID, TDate AS TDate, IDate AS IDate,  \r\n"
			"			COALESCE(LocationsT.Name, '') AS LocName, COALESCE(ProvQ.Last, '') + ', ' + COALESCE(ProvQ.First, '') + ' ' + COALESCE(ProvQ.Middle, '') AS ProvName,  \r\n"
			"			COALESCE(PatientQ.Last, '') + ', ' + COALESCE(PatientQ.First, '') + ' ' + COALESCE(PatientQ.Middle, '') AS PatientName,  \r\n"
			"			CASE  \r\n"
			"			WHEN OrigLineItemType = 1 THEN 'Payment' \r\n"
			"			WHEN OrigLineItemType = 2 THEN 'Adjustment' \r\n"
			"			WHEN OrigLineItemType = 3 THEN 'Refund' \r\n"
			"			WHEN OrigLineItemType = 10 THEN 'Charge' \r\n"
			"			END AS LineItemTypeText \r\n"
			"						FROM (  \r\n"
			// Unapplied and partially applied (gives the remaining balance on the partially applied line item)
			"							SELECT LineItemT.ID, LineItemT.PatientID AS PatID, LineItemT.Description,  \r\n"
			"							COALESCE(OrigLineItemsForVoidT.Type, OrigLineItemsForNewT.Type) AS OrigLineItemType,   \r\n"
			"							COALESCE(LineItemT.Amount, 0) - SUM(COALESCE(AppliesT.Amount, 0)) AS Amount,  \r\n"
			"							LineItemT.LocationID AS LocID, PaymentsT.ProviderID AS ProvID,   \r\n"
			"							CASE WHEN VoidingLineItemT.ID IS NOT NULL THEN 1 ELSE 0 END AS IsVoidingLineItem,   \r\n"
			"							CASE WHEN NewLineItemT.ID IS NOT NULL THEN 1 ELSE 0  END AS IsNewLineItem, \r\n"
			"							LineItemT.ID AS ParentLineItemID, dbo.AsDateNoTime(LineItemT.Date) AS TDate,  \r\n"
			"							dbo.AsDateNoTime(LineItemT.InputDate) AS IDate  \r\n"
			"							FROM  \r\n"
			"							LineItemT  \r\n"
			"							LEFT JOIN LineItemCorrectionsT VoidingLineItemT ON LineItemT.ID = VoidingLineItemT.VoidingLineItemID  \r\n"
			"							LEFT JOIN LineItemCorrectionsT NewLineItemT ON LineItemT.ID = NewLineItemT.NewLineItemID  \r\n"
			"							LEFT JOIN LineItemT OrigLineItemsForNewT ON NewLineItemT.OriginalLineItemID = OrigLineItemsForNewT.ID \r\n"
			"							LEFT JOIN LineItemT OrigLineItemsForVoidT ON VoidingLineItemT.OriginalLineItemID = OrigLineItemsForVoidT.ID \r\n"
			"							LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID  \r\n"
			"							INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n"
			"							WHERE (VoidingLineItemT.ID Is Not Null or NewLineItemT.ID IS Not Null) \r\n"
			"							AND LineItemT.Deleted = 0 \r\n"
			"							GROUP BY LineItemT.ID, LineItemT.PatientID, LineItemT.Description, COALESCE(OrigLineItemsForVoidT.Type, OrigLineItemsForNewT.Type), LineItemT.Amount, LineItemT.LocationID, PaymentsT.ProviderID, VoidingLineItemT.ID, NewLineItemT.ID, \r\n"
			"							LineItemT.Date, LineItemT.InputDate \r\n"
			"							HAVING COALESCE(LineItemT.Amount, 0) - SUM(COALESCE(AppliesT.Amount, 0)) <> 0 \r\n"
			"						UNION ALL \r\n"
			// gives the portions of the line item that is applied 
			"							SELECT LineItemT.ID, LineItemT.PatientID AS PatID, LineItemT.Description, \r\n"
			"							COALESCE(OrigLineItemsForVoidT.Type, OrigLineItemsForNewT.Type) AS OrigLineItemType, \r\n"
			"							AppliesT.Amount, LineItemT.LocationID AS LocID, \r\n"
			"							COALESCE(ChargesT.DoctorsProviders, PaymentsT.ProviderID) AS ProvID, \r\n"
			"							CASE WHEN VoidingLineItemT.ID IS NOT NULL THEN 1 ELSE 0 END AS IsVoidingLineItem, \r\n"
			"							CASE WHEN NewLineItemT.ID IS NOT NULL THEN 1 ELSE 0 END AS IsNewLineItem, \r\n"
			"							CASE WHEN EXISTS(SELECT 1 FROM LineItemCorrectionsT WHERE NewLineItemID = AppliesT.DestID OR VoidingLineItemID = AppliesT.DestID) \r\n"
			"							THEN AppliesT.DestID ELSE LineItemT.ID END AS ParentLineItemID, dbo.AsDateNoTime(LineItemT.Date) AS TDate, \r\n"
			"							dbo.AsDateNoTime(LineItemT.InputDate) AS IDate \r\n"
			"							FROM \r\n"
			"							LineItemT \r\n"
			"							LEFT JOIN LineItemCorrectionsT VoidingLineItemT ON LineItemT.ID = VoidingLineItemT.VoidingLineItemID \r\n"
			"							LEFT JOIN LineItemCorrectionsT NewLineItemT ON LineItemT.ID = NewLineItemT.NewLineItemID \r\n"
			"							LEFT JOIN LineItemT OrigLineItemsForNewT ON NewLineItemT.OriginalLineItemID = OrigLineItemsForNewT.ID \r\n"
			"							LEFT JOIN LineItemT OrigLineItemsForVoidT ON VoidingLineItemT.OriginalLineItemID = OrigLineItemsForVoidT.ID \r\n"
			"							INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID \r\n"
			"							LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID \r\n"
			"							LEFT JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID \r\n"
			"							WHERE (VoidingLineItemT.ID Is Not Null or NewLineItemT.ID IS Not Null) \r\n"
			"							AND LineItemT.Deleted = 0 \r\n"
			"						UNION ALL \r\n"
			// All corrected charges 
			"							SELECT LineItemT.ID, LineItemT.PatientID AS PatID, LineItemT.Description, \r\n"
			"							COALESCE(OrigLineItemsForVoidT.Type, OrigLineItemsForNewT.Type) AS OrigLineItemType, \r\n"
			"							dbo.GetChargeTotal(LineItemT.ID) AS Amount, LineItemT.LocationID AS LocID, ChargesT.DoctorsProviders AS ProvID, \r\n"
			"							CASE WHEN VoidingLineItemT.ID IS NOT NULL THEN 1 ELSE 0 END AS IsVoidingLineItem, \r\n"
			"							CASE WHEN NewLineItemT.ID IS NOT NULL THEN 1 ELSE 0 END AS IsNewLineItem, \r\n"
			"							LineItemT.ID AS ParentLineItemID, dbo.AsDateNoTime(LineItemT.Date) AS TDate, \r\n"
			"							dbo.AsDateNoTime(LineItemT.InputDate) AS IDate \r\n"
			"							FROM \r\n"
			"							LineItemT \r\n"
			"							LEFT JOIN LineItemCorrectionsT VoidingLineItemT ON LineItemT.ID = VoidingLineItemT.VoidingLineItemID \r\n"
			"							LEFT JOIN LineItemCorrectionsT NewLineItemT ON LineItemT.ID = NewLineItemT.NewLineItemID \r\n"
			"							LEFT JOIN LineItemT OrigLineItemsForNewT ON NewLineItemT.OriginalLineItemID = OrigLineItemsForNewT.ID \r\n"
			"							LEFT JOIN LineItemT OrigLineItemsForVoidT ON VoidingLineItemT.OriginalLineItemID = OrigLineItemsForVoidT.ID \r\n"
			"							INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID \r\n"
			"							WHERE (VoidingLineItemT.ID Is Not Null or NewLineItemT.ID IS Not Null) \r\n"
			"							AND LineItemT.Deleted = 0 \r\n"
			"						) FinancialCorrectionsQ \r\n"
			"					LEFT JOIN LocationsT ON LocID = LocationsT.ID \r\n"
			"					LEFT JOIN PersonT ProvQ ON ProvID = ProvQ.ID \r\n"
			"					LEFT JOIN PersonT PatientQ ON PatID = PatientQ.ID \r\n"
			);
		break;

	case 716:
		/*Version History
		// (j.gruber 2011-10-24 12:57) - PLID 45362 - created
		// (r.gonet 2015-05-05 14:38) - PLID 65317 - Exclude Gift Certificate Refunds
		*/
		return _T(" SELECT InfoQ.ProvID AS ProvID,   \r\n "
			" InfoQ.PayID, InfoQ.ChargeID, InfoQ.BillID, \r\n "
			" InfoQ.FullName,   \r\n "
			" PersonT.Last + ', ' + PersonT.First AS DocName,   \r\n "
			" InfoQ.ChargeDate AS ChargeDate,   \r\n "
			" InfoQ.PayMethod AS PayMethod,   \r\n "
			" InfoQ.CCType,   \r\n "
			" InfoQ.CheckNo,   \r\n "
			" InfoQ.Description,   \r\n "
			" InfoQ.ApplyAmount,   \r\n "
			" CategoriesQ.Category,   \r\n "
			" CategoriesQ.SubCategory,   \r\n "
			" CategoriesQ.ParentID AS ParentID,   \r\n "
			" InfoQ.PatID AS PatID,  \r\n "
			" InfoQ.UserDefinedID,   \r\n "
			" InfoQ.ChargeIDate AS ChargeIDate,   \r\n "
			" InfoQ.Type,  \r\n "
			" InfoQ.ServiceCode,  \r\n "
			" InfoQ.ServiceID AS ServiceID,  \r\n "
			" InfoQ.ChargeAmount,  \r\n "
			" InfoQ.PayAmt AS PayAmount, 		 \r\n "
			" InfoQ.ItemDesc,  \r\n "
			" InfoQ.LocID AS LocID,  \r\n "
			" InfoQ.Location,  \r\n "
			" InfoQ.PaymentDate as PayDate,  \r\n "
			" InfoQ.PayIDate as PayIDate,  \r\n "
			" InfoQ.BDate AS BillDate, \r\n "
			" InfoQ.BillDescription, \r\n "
			" InfoQ.ApplyDate as ApplyDate, \r\n "
			" InfoQ.ResptypeCategory, \r\n "
			" InfoQ.respTypePriority, \r\n "
			" InfoQ.RespName, \r\n "
			" InfoQ.AffiliatePhysID as AffiliatePhysID, InfoQ.AffiliatePhysAmount,  \r\n "
			" InfoQ.AffiliateNote, InfoQ.StatusID as StatusID,  \r\n "
			" InfoQ.AffiliateStatusName, \r\n "
			" InfoQ.AffiliatePhysStatusDate, InfoQ.AffiliateStatusType, \r\n "
			" InfoQ.AffiliateFirst, InfoQ.AffiliateLast, InfoQ.AffiliateMiddle,  \r\n "
			" InfoQ.AffiliateTitle, InfoQ.AffiliateAddress1, InfoQ.AffiliateAddress2,   \r\n "
			" InfoQ.AffiliateCity, InfoQ.AffiliateZip, InfoQ.AffiliateHPhone,  \r\n "
			" InfoQ.AffiliateWorkPhone, InfoQ.AffiliateNPI,  \r\n "
			" InfoQ.AffiliateCompany, InfoQ.AffiliateAccountID, \r\n "
			" InfoQ.AffiliateCustom1FieldName, InfoQ.AffiliateCustom1Data, \r\n "
			" InfoQ.AffiliateCustom2FieldName, InfoQ.AffiliateCustom2Data, \r\n "
			" InfoQ.AffiliateCustom3FieldName, InfoQ.AffiliateCustom3Data, \r\n "
			" InfoQ.AffiliateCustom4FieldName, InfoQ.AffiliateCustom4Data \r\n "
			" FROM ((((SELECT LineItemT.ID as PayID, LineChargesT.ID as ChargeID, BillsT.ID as BillID,   \r\n "
			" ApplyAmount = CASE WHEN PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineChargesT.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount End ELSE AppliesT.Amount End,   \r\n "
			" CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End AS ProvID,   \r\n "
			" PatientsT.PersonID AS PatID,   \r\n "
			" PatientsT.UserDefinedID,  \r\n "
			" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   \r\n "
			" LineChargesT.InputDate AS ChargeIDate,   \r\n "
			" LineChargesT.Date AS ChargeDate,  \r\n "
			" LineItemT.Date AS PaymentDate,  \r\n "
			" LineItemT.InputDate as PayIDate, \r\n "
			" PaymentsT.PayMethod,   \r\n "
			" LineItemT.Description,   \r\n "
			" CASE WHEN CPTCodeT.ID IS NULL THEN convert(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS ServiceCode, 	 \r\n "
			" LineItemT.Amount AS PayAmt,   \r\n "
			" dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,  		 \r\n "
			" LineChargesT.Description AS ItemDesc,   \r\n "
			" CreditCardNamesT.CardName AS CCType,   \r\n "
			" PaymentPlansT.CheckNo,   \r\n "
			" ServiceT.Category,   \r\n "
			" AppliesT.ID AS ApplyID,   \r\n "
			" LineItemT.ID AS LineID,   \r\n "
			" LineItemT.Type, 		 \r\n "
			" ServiceT.ID AS ServiceID, \r\n "
			" CASE WHEN LineChargesT.LocationID Is Null THEN LineItemT.LocationID ELSE LineChargesT.LocationID END AS LocID,  \r\n "
			" LocationsT.Name AS Location,  \r\n "
			" BillsT.Date AS BDate, BillsT.Description as BillDescription, AppliesT.InputDate as ApplyDate, \r\n "
			" RespTypeT.CategoryType as RespTypeCategory, RespTypeT.TypeName as RespName, \r\n "
			" RespTypeT.Priority as RespTypePriority, \r\n "
			" BillsT.AffiliatePhysID as AffiliatePhysID, BillsT.AffiliatePhysAmount, BillsT.AffiliateNote, \r\n "
			" BillsT.AffiliateStatusID as StatusID, BillAffiliateStatusT.Name as AffiliateStatusName, \r\n "
			" BillAffiliateStatusHistoryT.Date as AffiliatePhysStatusDate, BillAffiliateStatusT.Type as AffiliateStatusType,   \r\n "
			"    AffPersonT.First As AffiliateFirst, AffPersonT.Last As AffiliateLast, AffPersonT.Middle As AffiliateMiddle, AffPersonT.Title As AffiliateTitle, \r\n "
			" AffPersonT.Address1 As AffiliateAddress1, AffPersonT.Address2 As AffiliateAddress2,  AffPersonT.City As AffiliateCity, \r\n "
			" AffPersonT.Zip As AffiliateZip, AffPersonT.HomePhone As AffiliateHPhone, AffPersonT.WorkPhone As AffiliateWorkPhone, AffiliatesT.NPI as AffiliateNPI,  \r\n "
			" AffPersonT.Company as AffiliateCompany, AffPersonT.CompanyID As AffiliateAccountID, \r\n "
			" CustomContact1.Name as AffiliateCustom1FieldName, CustomContact1.TextParam as AffiliateCustom1Data, \r\n "
			" CustomContact2.Name as AffiliateCustom2FieldName, CustomContact2.TextParam as AffiliateCustom2Data, \r\n "
			" CustomContact3.Name as AffiliateCustom3FieldName, CustomContact3.TextParam as AffiliateCustom3Data, \r\n "
			" CustomContact4.Name as AffiliateCustom4FieldName, CustomContact4.TextParam as AffiliateCustom4Data \r\n "
			" 			   \r\n "
			" FROM  \r\n "
			" (SELECT InnerLineItemT.* FROM LineItemT InnerLineItemT \r\n "
			" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON InnerLineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID \r\n "
			" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON InnerLineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID \r\n "
			" WHERE InnerLineItemT.DELETED = 0 AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID IS NULL AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID IS NULL \r\n "
			" )  LineChargesT LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID \r\n "
			" LEFT JOIN (SELECT InnerBillsT.* FROM BillsT InnerBillsT LEFT JOIN BillCorrectionsT ON InnerBillsT.ID = BillCorrectionsT.OriginalBillID WHERE InnerBillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL) BillsT ON ChargesT.BillID = BillsT.ID \r\n "
			" LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID \r\n "
			" LEFT JOIN ( SELECT InnerLineItemT.* FROM LineItemT InnerLineItemT \r\n "
			" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaysQ ON InnerLineItemT.ID = LineItemCorrections_OriginalPaysQ.OriginalLineItemID \r\n "
			" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaysQ ON InnerLineItemT.ID = LineItemCorrections_VoidingPaysQ.VoidingLineItemID \r\n "
			" WHERE InnerLineItemT.DELETED = 0 AND InnerLineItemT.Type = 1 AND LineItemCorrections_OriginalPaysQ.OriginalLineItemID IS NULL AND LineItemCorrections_VoidingPaysQ.VoidingLineItemID IS NULL \r\n "
			" ) LineItemT ON AppliesT.SourceID = LineItemT.ID  \r\n "
			" LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n "
			" LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  \r\n "
			" LEFT JOIN  \r\n "
			" ( \r\n "
			" 	SELECT InnerLinePaysT.ID, InnerLinePaysT.Amount AS PayAmt,   \r\n "
			" 	Sum(AppliesT.Amount) AS ApplyAmt,   \r\n "
			" 	/*First of LineItemT_1*/MIN(InnerLinePaysT.[Amount])-Sum([AppliesT].[Amount]) AS Total,   \r\n "
			" 	InnerLinePaysT.PatientID   \r\n "
			" 	FROM LineItemT AS InnerLinePaysT LEFT JOIN (PaymentsT LEFT JOIN  \r\n "
			" 	(AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)  \r\n "
			" 	ON InnerLinePaysT.ID = PaymentsT.ID  \r\n "
			" 	WHERE (((InnerLinePaysT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))  \r\n "
			" 	GROUP BY InnerLinePaysT.ID, InnerLinePaysT.Amount, InnerLinePaysT.PatientID  \r\n "
			" 	HAVING (((InnerLinePaysT.ID) Is Not Null) AND ((MIN(InnerLinePaysT.[Amount])-Sum([AppliesT].[Amount]))<>0))  \r\n "
			" ) AS PartiallyAppliedPaysQ ON LineItemT.ID = PartiallyAppliedPaysQ.ID		 \r\n "
			" INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  \r\n "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID  \r\n "
			" LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID \r\n "
			" LEFT JOIN PatientsT ON LineChargesT.PatientID = PatientsT.PersonID \r\n "
			" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID				 \r\n "
			" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID  \r\n "
			" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID  \r\n "
			" LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID \r\n "
			" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID \r\n "
			" LEFT JOIN BillAffiliateStatusT ON BillsT.AffiliateStatusID = BillAffiliateStatusT.ID \r\n "
			" LEFT JOIN BillAffiliateStatusHistoryT ON BillsT.ID = BillAffiliateStatusHistoryT.BillID  \r\n "
			" AND BillsT.AffiliateStatusID = BillAffiliateStatusHistoryT.StatusID \r\n "
			" LEFT JOIN ReferringPhysT AffiliatesT ON BillsT.AffiliatePhysID = AffiliatesT.PersonID \r\n "
			" LEFT JOIN PersonT AffPersonT ON AffiliatesT.PersonID = AffPersonT.ID \r\n "
			" LEFT JOIN (SELECT PersonID, TextParam, Name FROM CustomFieldDataT INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID WHERE FieldID = 6) AS CustomContact1 ON AffPersonT.ID = CustomContact1.PersonID \r\n "
			" LEFT JOIN (SELECT PersonID, TextParam, Name FROM CustomFieldDataT INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID WHERE FieldID = 7) AS CustomContact2 ON AffPersonT.ID = CustomContact2.PersonID  \r\n "
			" LEFT JOIN (SELECT PersonID, TextParam, Name FROM CustomFieldDataT INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID WHERE FieldID = 8) AS CustomContact3 ON AffPersonT.ID = CustomContact3.PersonID  \r\n "
			" LEFT JOIN (SELECT PersonID, TextParam, Name FROM CustomFieldDataT INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID WHERE FieldID = 9) AS CustomContact4 ON AffPersonT.ID = CustomContact4.PersonID  \r\n "
			" WHERE (LineItemT.ID IS NULL OR LineItemT.Deleted=0) AND (LineItemT.ID IS NULL OR LineItemT.Type = 1) AND (LineChargesT.Deleted = 0)  \r\n "
			" AND (AppliesT.ID IS NULL OR AppliesT.PointsToPayments = 0)  \r\n "
			" AND (PaymentsT.ID IS NULL OR PaymentsT.PayMethod NOT IN (4,10)) AND BillAffiliateStatusT.Type IS NOT NULL  \r\n "
			" AND BillsT.Deleted = 0 \r\n "
			" GROUP BY LineItemT.ID, LineChargesT.ID, BillsT.ID, CASE WHEN PartiallyAppliedPaysQ.ID Is Null  \r\n "
			" THEN CASE WHEN LineChargesT.ID Is Null THEN LineItemT.Amount  \r\n "
			" ELSE AppliesT.Amount End ELSE AppliesT.Amount End,  \r\n "
			" CASE WHEN DoctorsProviders Is Null THEN PaymentsT.ProviderID ELSE DoctorsProviders End,  \r\n "
			" PatientsT.PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  \r\n "
			" LineChargesT.InputDate, LineChargesT.Date, PaymentsT.PayMethod, LineItemT.Description, ChargesT.ItemCode,  \r\n "
			" LineItemT.Amount, dbo.GetChargeTotal(ChargesT.ID), LineChargesT.Date, LineChargesT.Description, \r\n "
			" CreditCardNamesT.CardName, PaymentPlansT.CheckNo, ServiceT.Category, AppliesT.ID, LineItemT.ID,  \r\n "
			" LineItemT.Type, ChargesT.ItemCode, BillsT.Description, \r\n "
			" CASE WHEN LineChargesT.LocationID Is Null THEN LineItemT.LocationID ELSE LineChargesT.LocationID END,  \r\n "
			" LocationsT.Name, CPTCodeT.ID, LineItemT.Date, ServiceT.ID, CPTCodeT.Code, BillsT.Date, \r\n "
			" RespTypeT.CategoryType, RespTypeT.Priority, \r\n "
			" BillsT.AffiliatePhysID, BillsT.AffiliatePhysAmount, BillsT.AffiliateNote, \r\n "
			" BillsT.AffiliateStatusID, BillAffiliateStatusT.Name, \r\n "
			" BillAffiliateStatusHistoryT.Date, BillAffiliateStatusT.Type, \r\n "
			" AffPersonT.First, AffPersonT.Last, AffPersonT.Middle, AffPersonT.Title, \r\n "
			" AffPersonT.Address1, AffPersonT.Address2,  AffPersonT.City, \r\n "
			" AffPersonT.Zip, AffPersonT.HomePhone, AffPersonT.WorkPhone, AffiliatesT.NPI,  \r\n "
			" AffPersonT.Company, AffPersonT.CompanyID, AppliesT.InputDate, LineItemT.InputDate, \r\n "
			" CustomContact1.Name, CustomContact1.TextParam, \r\n "
			" CustomContact2.Name, CustomContact2.TextParam, \r\n "
			" CustomContact3.Name, CustomContact3.TextParam, \r\n "
			" CustomContact4.Name, CustomContact4.TextParam,RespTypeT.TypeName  \r\n "
			" ) AS InfoQ LEFT JOIN (SELECT CategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, '' AS SubCategory, CategoriesT.ID AS ParentID  \r\n "
			" FROM CategoriesT  \r\n "
			" WHERE (((CategoriesT.Parent)=0))  \r\n "
			" UNION SELECT SubCategoriesT.ID AS CategoryID, CategoriesT.Name AS Category, SubCategoriesT.Name AS SubCategory, SubCategoriesT.Parent AS ParentID  \r\n "
			" FROM CategoriesT RIGHT JOIN CategoriesT AS SubCategoriesT ON CategoriesT.ID = SubCategoriesT.Parent  \r\n "
			" WHERE (((SubCategoriesT.Parent)<>0))  \r\n "
			" ) AS CategoriesQ ON InfoQ.Category = CategoriesQ.CategoryID)  \r\n "
			" LEFT JOIN PersonT ON InfoQ.ProvID = PersonT.ID)) \r\n "
			);
		break;

	case 721:
		//Inventory Sales Profit - Ins. Vs. Self Pay
		/*	Version History
		TES 1/3/2012 - PLID 47295 - Created
		*/
		// (j.politis 2015-08-18 14:49) - PLID 66741 - We need to fix how we round tax 1 and tax 2 by using dbo.GetChargeTotal(ChargesT.ID)
		return _T("SELECT "
			"PatientsT.UserDefinedID, PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"dbo.GetChargeTotal(ChargesT.ID) AS ChargedAmountWithTax, "
			"Round(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))) "
			"	)),2) AS ChargedAmountNoTax,  "
			"Round(Convert(money, "
			"	((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)) + "
			"	((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)) "
			"	),2) AS TaxAmt,  "
			"ChargesT.Quantity, LastCostPerUU AS LastCost, "
			"Round(Convert(money,LastCostPerUU * ChargesT.Quantity),2) AS CostToPractice,  "
			"CASE WHEN AppliedPaysQ.AppliedToID IS NULL THEN 0 ELSE AppliedPaysQ.ApplyAmt END AS AppliedAmount, CASE WHEN AppliedAdjsQ.AppliedToID IS NULL THEN 0 ELSE AppliedAdjsQ.ApplyAmt END AS AppliedAdjustments, ServiceT.Name AS ProductName, LineChargesT.LocationID AS LocID,  "
			"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, LineChargesT.Date AS TDate, LineChargesT.InputDate AS IDate, "
			"PersonCoord.Last + ', ' + PersonCoord.First + ' ' + PersonCoord.Middle AS CoordName, "
			" "
			"/*IDs for Filters*/ "
			"ServiceT.ID AS ServiceID, CategoriesT.ID AS CategoryID, MultiSupplierT.SupplierID AS SupplierID, PersonCoord.ID AS CoordID,  "
			"PersonProv.ID AS ProvID,  "
			"/*For Custom Fields*/ "
			"ProductT.Notes AS ProductNotes, ProductT.UnitDesc AS ProductDescription,  "
			"PersonSupT.Company AS SupplierName, CategoriesT.Name AS CategoryName, LocationsT.Name AS LocName, ChargesT.TaxRate, ChargesT.TaxRate2, "
			"COALESCE(InsuranceCoT.PersonID,-1) AS InsCoID, COALESCE(InsuranceCoT.Name, ' <Self Pay> ') AS InsCoName "
			"FROM "
			"ChargesT INNER JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID "
			"LEFT JOIN (SELECT ChargeRespT.ChargeID, Min(CASE WHEN Priority = -1 THEN 99 ELSE Priority END) AS Priority "
			"	FROM ChargeRespT LEFT JOIN (InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID) "
			"	ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID WHERE ChargeRespT.Amount <> 0 GROUP BY ChargeID) AS ChargeRespPriorityQ "
			"ON ChargesT.ID = ChargeRespPriorityQ.ChargeID "
			"LEFT JOIN (InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID) "
			"	ON InsuredPartyT.PatientID = LineChargesT.PatientID AND "
			"	CASE WHEN RespTypeT.Priority = -1 THEN 99 ELSE RespTypeT.Priority END = ChargeRespPriorityQ.Priority "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN PersonT PersonProv ON ChargesT.DoctorsProviders = PersonProv.ID "
			"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN PersonT PersonCoord ON ChargesT.PatCoordID = PersonCoord.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
			"LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID "
			"LEFT JOIN  "
			"	(SELECT ChargesT.ID AS AppliedToID, Sum(AppliesT.Amount) AS ApplyAmt "
			"	FROM "
			"	ChargesT INNER JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID "
			"	INNER JOIN AppliesT ON LineChargesT.ID = AppliesT.DestID "
			"	INNER JOIN LineItemT LinePaysT ON AppliesT.SourceID = LinePaysT.ID "
			"	WHERE LineChargesT.Type = 10 AND LineChargesT.Deleted = 0 "
			"   AND LinePaysT.Type = 1 "
			"	GROUP BY ChargesT.ID "
			"	) AppliedPaysQ "
			"ON ChargesT.ID = AppliedPaysQ.AppliedToID "
			"LEFT JOIN  "
			"	(SELECT ChargesT.ID AS AppliedToID, Sum(AppliesT.Amount) AS ApplyAmt "
			"	FROM "
			"	ChargesT INNER JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID "
			"	INNER JOIN AppliesT ON LineChargesT.ID = AppliesT.DestID "
			"	INNER JOIN LineItemT LinePaysT ON AppliesT.SourceID = LinePaysT.ID "
			"	WHERE LineChargesT.Type = 10 AND LineChargesT.Deleted = 0 "
			"   AND LinePaysT.Type = 2 "
			"	GROUP BY ChargesT.ID "
			"	) AppliedAdjsQ "
			"ON ChargesT.ID = AppliedAdjsQ.AppliedToID "
			"LEFT JOIN PersonT ON LineChargesT.PatientID = PersonT.ID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT PersonSupT ON MultiSupplierT.SupplierID = PersonSupT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"WHERE LineChargesT.Deleted = 0 AND LineChargesT.Type = 10 ");
		break;

		// (b.spivey, March 19, 2012) - PLID 48696 - Request report. 
	case 723:
		// E-Eligibility Request Report 
		/*	Version History
		// (b.spivey, March 30, 2012) - PLID 48696 -  Created.
		*/
	{
		CString strSql =
			"SELECT "
			"PatientsT.UserDefinedID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"PatientsT.PersonID AS PatID, "
			"EligibilityRequestsT.ID AS RequestID, "
			"InsuranceCoT.Name AS InsCoName, "
			"RespTypeT.TypeName AS RespTypeName, "
			"PersonProvidersT.Last + ', ' + PersonProvidersT.First + ' ' + PersonProvidersT.Middle AS ProvName, "
			"PersonProvidersT.ID AS ProvID, "
			"EligibilityRequestsT.LastSentDate AS LastSentDate, "
			"EligibilityRequestsT.CreateDate AS CreatedDate, "
			"ResponseQ.DateReceived, "
			"ResponseQ.ID AS ResponseID, "
			"ResponseQ.Response, ResponseQ.HasDetails, "
			"DetailsQ.DetailID AS ResponseDetailID, "
			"DetailsQ.Authorized, "
			"DetailsQ.ServiceType AS TypeOfService, "
			"DetailsQ.CoverageLevel, "
			"(DetailsQ.BenefitType + (CASE WHEN DetailsQ.TimePeriod <> '' THEN ': ' + DetailsQ.TimePeriod ELSE '' END)) AS FullBenefit, "
			"DetailsQ.CoverageDesc, DetailsQ.ExtraMessage, "
			"DetailsQ.Amount AS CurrencyAmount, DetailsQ.Percentage, "
			"DetailsQ.QuantityType, DetailsQ.Quantity, "
			"DetailsQ.InNetwork, DetailsQ.InsuranceType, "
			"DetailsQ.BenefitID, DetailsQ.BenefitExcluded, "
			"DetailsQ.CoverageID, DetailsQ.CoverageExcluded, "
			"DetailsQ.ServiceID, DetailsQ.ServiceExcluded "
			""
			"FROM EligibilityRequestsT "
			"LEFT JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
			"LEFT JOIN ProvidersT ON EligibilityRequestsT.ProviderID = ProvidersT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN PersonT PersonProvidersT ON ProvidersT.PersonID = PersonProvidersT.ID "
			"LEFT JOIN "
			"( "
			"	SELECT ID, RequestID, DateReceived, Response, "
			"	Convert(bit, CASE WHEN EXISTS (SELECT TOP 1 EligibilityResponseDetailsT.ID "
			"	FROM EligibilityResponseDetailsT "
			"	WHERE EligibilityResponseDetailsT.ResponseID = EligibilityResponsesT.ID) THEN 1 ELSE 0 END) AS HasDetails "
			"	FROM EligibilityResponsesT "
			") ResponseQ ON ResponseQ.RequestID = EligibilityRequestsT.ID "
			"LEFT JOIN "
			"( "
			"	SELECT EligibilityResponseDetailsT.ID AS DetailID, "
			"	ResponseID, "
			"	BenefitTypeRefID, BenefitTypeQ.Description AS BenefitType, "
			"	CoverageLevelRefID, CoverageLevelQ.Description AS CoverageLevel, "
			"	ServiceTypeRefID, ServiceTypeQ.Description AS ServiceType, "
			"	InsuranceType, CoverageDesc, TimePeriod, "
			"	Amount, Percentage, "
			"	QuantityType, Quantity, "
			"	Authorized, InNetwork, "
			"	ServiceCode, Modifiers, "
			"	ExtraMessage, ServiceCodeRangeEnd, "
			"	BenefitTypeQ.FilterExcluded AS BenefitExcluded, CoverageLevelQ.FilterExcluded AS CoverageExcluded, "
			"	ServiceTypeQ.FilterExcluded AS ServiceExcluded, BenefitTypeQ.ID AS BenefitID, "
			"	CoverageLevelQ.ID AS CoverageID, ServiceTypeQ.ID AS ServiceID "
			"	FROM EligibilityResponseDetailsT "
			"	LEFT JOIN (SELECT ID, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 1) BenefitTypeQ ON EligibilityResponseDetailsT.BenefitTypeRefID = BenefitTypeQ.ID "
			"	LEFT JOIN (SELECT ID, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 2) CoverageLevelQ ON EligibilityResponseDetailsT.CoverageLevelRefID = CoverageLevelQ.ID "
			"	LEFT JOIN (SELECT ID, Description, FilterExcluded FROM EligibilityDataReferenceT WHERE ListType = 3) ServiceTypeQ ON EligibilityResponseDetailsT.ServiceTypeRefID = ServiceTypeQ.ID "
			") DetailsQ ON DetailsQ.ResponseID = ResponseQ.ID ";

		// (b.spivey, March 30, 2012) - PLID 48696 - Did it come from the print preview? If no, use default where clause.
		if (GetExtraValue().IsEmpty()) {
			strSql += _T("WHERE	 "
				"( "
				"	ResponseQ.ID IN "
				"	( "
				"	SELECT ResponseID "
				"	FROM "
				"		( "
				"			SELECT MAX(RepT.ID) AS ResponseID, ReqT.ID "
				"			FROM EligibilityRequestsT ReqT "
				"			LEFT JOIN EligibilityResponsesT RepT ON ReqT.ID = RepT.RequestID "
				"			WHERE RepT.ID IS NOT NULL "
				"			GROUP BY ReqT.ID "
				"		) LatestResponsesQ "
				"	) "
				"	OR ResponseQ.ID IS NULL "
				") ");
		}
		else {
			// (b.spivey, March 30, 2012) - PLID 48696 - Came from print preview, append new where clause. 
			//	 The print preview is run from this function: CEligibilityRequestDetailDlg::OnBnClickedBtnPrintPreview()
			strSql += GetExtraValue();
		}

		// (b.spivey, March 30, 2012) - PLID 48696 - We have to SubQuery this because of the 
		//	 Date/Provider/Patient filters throwing exceptions and assertions all over the place.
		CString strFinalSql = "SELECT "
			"EligReqQ.UserDefinedID AS PatientID, "
			"EligReqQ.PatName AS PatientName, "
			"EligReqQ.PatID AS PatID, "
			"EligReqQ.RequestID, "
			"EligReqQ.InsCoName AS InsuranceCompanyName, "
			"EligReqQ.RespTypeName, "
			"EligReqQ.ProvName AS ProviderName, "
			"EligReqQ.ProvID AS ProvID, "
			"EligReqQ.LastSentDate AS LastSentDate, "
			"EligReqQ.CreatedDate AS CreatedDate, "
			"EligReqQ.DateReceived AS DateReceived, "
			"EligReqQ.ResponseID, "
			"EligReqQ.Response AS ResponseText, EligReqQ.HasDetails, "
			"EligReqQ.ResponseDetailID, "
			"EligReqQ.Authorized, "
			"EligReqQ.TypeOfService, "
			"EligReqQ.CoverageLevel, "
			"EligReqQ.FullBenefit, "
			"EligReqQ.CoverageDesc, EligReqQ.ExtraMessage, "
			"EligReqQ.CurrencyAmount AS CurrencyValue, EligReqQ.Percentage AS PercentageValue, "
			"EligReqQ.QuantityType, EligReqQ.Quantity AS QuantityValue, "
			"EligReqQ.InNetwork, EligReqQ.InsuranceType, "
			"EligReqQ.BenefitID, EligReqQ.BenefitExcluded, "
			"EligReqQ.CoverageID, EligReqQ.CoverageExcluded, "
			"EligReqQ.ServiceID, EligReqQ.ServiceExcluded "
			"FROM ( " + strSql + ") EligReqQ ";

		return _T(strFinalSql);
	}
	break;

	default:
		return _T("");
		break;
	}
}

//DRT 4/30/03 - Comiler heap limit reached, had to move something out of GetSqlFinancial, so the AR reports 
//		get the boot.
CString CReportInfo::GetSqlAR(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;
	COleDateTime dtNext;
	COleDateTimeSpan OneDay(1, 0, 0, 0);

	//TES 7/9/2008 - PLID 29580 - For all AR reports, we want to disregard any applies which happened after the "As Of:" date.
	//TES 10/13/2009 - PLID 35662 - Only do this for InputDate filtering.
	CString strAppliesT;
	// (j.gruber 2011-12-16 15:54) - PLID 36465 -- use payment input date
	if (strDateFilterField == "IDate"
		|| (nID == 170 && nDateFilter == 3) //AR by Insurance
		|| (nID == 385 && nDateFilter == 3) //AR by Insurance Co Office
		|| (nID == 391 && nDateFilter == 3)  //AR split by Resp
		|| (nID == 659 && nDateFilter == 3) //AR by Fin Class		
		|| (nID == 388 && nDateFilter == 3) //AR by patient resp
		) {
		strAppliesT.Format("(SELECT * FROM AppliesT WHERE InputDate < '%s') AS AppliesT", (DateTo + OneDay).Format("%Y-%m-%d"));
	}
	else {
		strAppliesT = "AppliesT";
	}

	switch (nID) {

	case 151://Aged Receivables
	case 177://AR By Amount
	case 238://AR By Patient
	case 715: //AR by Category // (j.gruber 2014-10-16) - PLID 63899 - moved AR by amount, patient, and category up here to support Input Date

			  /* Version History
			  TES 11/7/2003: - Simplified the payments parts of the ar queries to resolve an issue with payments that
			  were multiply applied from and to.
			  DRT 4/22/2004 - PLID 11816 - Fixed an issue with 2 select *'s
			  TES 9/20/2004 - PLID 14152 - Fixed bug with calculating applies.
			  DRT 4/10/2006 - PLID 11734 - Removed ChargesT.ProcCode, it was not being used.
			  // (j.gruber 2008-07-01 09:46) - PLID 26389 - added last payment info fields
			  TES 7/9/2008 - PLID 29580 - Filtered out applies which happened after the "As Of:" date.
			  // (j.gruber 2009-03-24 17:57) - PLID 33359 - took out the discount fields since they weren't used in the main query
			  // (j.gruber 2009-04-23 16:58) - PLID 34064 - replace dbo.GetChargeTotal
			  // (j.gruber 2010-01-28 15:07) - PLID 36899 - filtered out deleted bills
			  // (j.jones 2010-04-07 17:29) - PLID 15224 - removed ChargesT.EMG, obsolete field
			  TES 4/7/2011 - PLID 33741 - Added PlaceOfServiceID, PlaceOfService
			  // (j.gruber 2011-10-07 13:38) - PLID 45745 - added category
			  // (a.walling 2014-04-09 09:14) - PLID 61694 - removed WhichCodes
			  // (j.gruber 2014-10-16) - PLID 63899 - supported Input Date
			  */
	{
		BOOL bUseInputDate = FALSE;
		if (nDateFilter == 2) {
			bUseInputDate = TRUE;
		}

		//This is if they're running on Input Date; if they aren't, we won't break,
		//and it will be handled with AR by amount and by patient.
		strArSql = "SELECT AgedLineItemsQ.Expr1, AgedLineItemsQ.Diff AS Diff, AgedLineItemsQ.PatID AS PatID,  \r\n"
			"AgedLineItemsQ.UserDefinedID, AgedLineItemsQ.FullName, AgedLineItemsQ.DocName, AgedLineItemsQ.Description,   \r\n"
			"AgedLineItemsQ.TDate AS TDate, AgedLineItemsQ.IDate AS IDate, AgedLineItemsQ.ServiceDate AS ServiceDate, AgedLineItemsQ.ProvID AS ProvID,   \r\n"
			"AgedLineItemsQ.ItemCode, AgedLineItemsQ.Quantity, AgedLineItemsQ.LocID AS LocID, AgedLineItemsQ.Location,   \r\n"
			"AgedLineItemsQ.POSID AS PlaceOfServiceID, AgedLineItemsQ.PlaceOfService,  \r\n"
			" LastPayQ.LastDate as LastPayDate, LastPayQ.Description as LastPayDesc,  \r\n"
			" LastPayQ.RespType as LastPayRespType, LastPayQ.InputDate As LastPayInputDate, LastPayQ.Amount as LastPayAmount,  \r\n"
			" AgedLineItemsQ.CategoryID as CategoryID, AgedLineItemsQ.CategoryName  \r\n"
			"FROM  \r\n"
			"(  \r\n"
			"SELECT * FROM  \r\n"
			"(  \r\n"
			"SELECT AgedChargesQ.ID AS Expr1,    \r\n"
			"AgedChargesQ.[Amount]-[SumOfAmount] AS Diff,    \r\n"
			"AgedChargesQ.PatID,    \r\n"
			"AgedChargesQ.UserDefinedID,   \r\n"
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    \r\n"
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS DocName,    \r\n"
			"LineItemT.Description,    \r\n"
			"BillsT.Date AS TDate,    \r\n"
			"LineItemT.InputDate AS IDate,  \r\n"
			"LineItemT.Date AS ServiceDate,    \r\n"
			"ProvidersT.PersonID AS ProvID,    \r\n"
			"ChargesT.ItemCode,    \r\n"
			"ChargesT.Quantity,   \r\n"
			"AgedChargesQ.LocID,   \r\n"
			"AgedChargesQ.Location,   \r\n"
			"AgedChargesQ.POSID,  \r\n"
			"AgedChargesQ.PlaceOfService,  \r\n"
			"CategoriesT.ID as CategoryID, CategoriesT.Name as CategoryName  \r\n"
			"FROM (((((  \r\n"
			"(  \r\n"
			"SELECT (SELECT Sum(Amount) FROM ChargeRespT ChargeRespInnerT WHERE ChargeRespInnerT.ChargeID = LineItemT.ID) AS Amount,    \r\n"
			"Sum(CASE WHEN AgedAppliesQ.Amount Is Null THEN 0 ELSE AgedAppliesQ.Amount End) AS SumOfAmount,    \r\n"
			"LineItemT.ID,    \r\n"
			"PatientsT.PersonID AS PatID,   \r\n"
			"PatientsT.UserDefinedID,   \r\n"
			"LineItemT.LocationID AS LocID,   \r\n"
			"LocationsT.Name AS Location,  "
			"Location_POS.ID AS POSID,  \r\n"
			"Location_POS.Name AS PlaceOfService  \r\n"
			"FROM (((  \r\n"
			"(  \r\n"
			"SELECT ChargesT.ID, ChargesT.BillID, ChargesT.ServiceID, ChargesT.ItemCode, ChargesT.ItemSubCode, ChargesT.Category,  \r\n"
			"ChargesT.SubCategory, ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.TaxRate, ChargesT.Quantity, ChargesT.DoctorsProviders,  \r\n"
			"ChargesT.ServiceDateFrom, ChargesT.ServiceDateTo, PlaceOfServiceCodesT.PlaceCodes, ChargesT.ServiceType, ChargesT.EPSDT, ChargesT.COB,  \r\n"
			"ChargesT.OthrBillFee, ChargesT.LineID, ChargesT.TaxRate2, ChargesT.SuperbillID, ChargesT.PatCoordID, ChargesT.CPTModifier3,  \r\n"
			"ChargesT.CPTModifier4  \r\n"
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID   \r\n"
			"INNER JOIN (SELECT ID, Date FROM BillsT WHERE DELETED = 0) BillsT ON ChargesT.BillID = BillsT.ID \r\n"
			"LEFT JOIN PlaceofServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID  \r\n"
			"WHERE (@AgedChargeSubQ_Date < @ReportDateTo))   \r\n"
			") AS AgedChargesSubQ  \r\n"
			"LEFT JOIN BillsT ON AgedChargesSubQ.BillID = BillsT.ID  \r\n"
			"LEFT JOIN LocationsT Location_POS ON BillsT.Location = Location_POS.ID  \r\n"
			"LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON AgedChargesSubQ.ID = LineItemT.ID)  \r\n"
			"LEFT JOIN  \r\n"
			"(  \r\n"
			"SELECT AppliesT.ID, AppliesT.SourceID, AppliesT.DestID, AppliesT.RespID, AppliesT.Amount, AppliesT.PointsToPayments,  \r\n"
			"AppliesT.InputDate, AppliesT.InputName   \r\n"
			"FROM (((" + strAppliesT + " INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID)  \r\n"
			"INNER JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID)  \r\n"
			"INNER JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)  \r\n"
			"INNER JOIN (SELECT * FROM BillsT WHERE DELETED = 0) BillsT ON ChargesT.BillID = BillsT.ID   \r\n"
			"WHERE @AgedAppliesQ_Date1 < @ReportDateTo AND @AgedAppliesQ_Date2 < @ReportDateTo   \r\n"
			") AS AgedAppliesQ ON LineItemT.ID = AgedAppliesQ.DestID  \r\n"
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  \r\n"
			"LEFT JOIN CPTModifierT ON AgedChargesSubQ.CPTModifier = CPTModifierT.Number  \r\n"
			"LEFT JOIN CPTModifierT CPTModifierT2 ON AgedChargesSubQ.CPTModifier2 = CPTModifierT2.Number   \r\n"
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=10))   \r\n"
			"GROUP BY LineItemT.ID, PatientsT.PersonID, PatientsT.UserDefinedID, LineItemT.LocationID, LocationsT.Name, Location_POS.ID, Location_POS.Name   \r\n"
			") AS AgedChargesQ LEFT JOIN LineItemT ON AgedChargesQ.ID = LineItemT.ID)  \r\n"
			"LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID)  \r\n"
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  \r\n"
			"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID)  \r\n"
			"LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID   \r\n"
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  \r\n"
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID  \r\n"
			"WHERE (((AgedChargesQ.[Amount]-[SumOfAmount])>=0.01 Or (AgedChargesQ.[Amount]-[SumOfAmount])<=-0.01))   \r\n"
			") AS AgedChargesUnfinishedQ   \r\n"
			"UNION  \r\n"
			"SELECT * FROM  \r\n"
			"(  \r\n"
			"SELECT AgedPayAppliesSubQ.*,    \r\n"
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,    \r\n"
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT_1.Middle AS DocName,   \r\n"
			"LineItemT.Description,    \r\n"
			"LineItemT.Date AS TDate,    \r\n"
			"LineItemT.InputDate AS IDate,  \r\n"
			"LineItemT.Date AS ServiceDate,    \r\n"
			"ProvidersT.PersonID AS ProvID,    \r\n"
			"'' AS ItemCode,    \r\n"
			"'' AS Quantity,   \r\n"
			"LineItemT.LocationID AS LocID,   \r\n"
			"LocationsT.Name AS Location,   \r\n"
			"LineItemT.LocationID AS POSID,  \r\n"
			"LocationsT.Name AS PlaceOfService,  \r\n"
			"-1 as CategoryID, '' as CategoryName  \r\n"
			"FROM ((((  \r\n"
			"(  \r\n"
			"SELECT LineItemT.ID AS DestID,  \r\n"
			"-1 * (Amount + CASE WHEN (SELECT Sum(Amount) FROM " + strAppliesT + " WHERE DestID = LineItemT.ID AND SourceID IN (SELECT ID FROM LineItemT WHERE @AgedPayAppliesSubQ_Date1 < @ReportDateTo)) Is Null THEN 0 ELSE (SELECT Sum(Amount) FROM " + strAppliesT + " WHERE DestID = LineItemT.ID AND SourceID IN (SELECT ID FROM LineItemT WHERE LineItemT.Deleted = 0 AND @AgedPayAppliesSubQ_Date1 < @ReportDateTo)) END  \r\n"
			"- CASE WHEN (SELECT Sum(Amount) FROM " + strAppliesT + " WHERE SourceID = LineItemT.ID AND DestID IN (SELECT LineItemT.ID FROM LineItemT LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID LEFT JOIN (SELECT ID, Date FROM BillsT WHERE BillsT.Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID WHERE LineItemT.Deleted = 0 AND @AgedPayAppliesSubQ_Date2 < @ReportDateTo)) Is Null then 0  \r\n"
			" ELSE (SELECT Sum(Amount) FROM " + strAppliesT + " WHERE SourceID = LineItemT.ID AND DestID IN (SELECT LineItemT.ID FROM LineItemT LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID LEFT JOIN (SELECT ID, Date FROM BillsT WHERE BillsT.Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID WHERE LineItemT.Deleted = 0 AND @AgedPayAppliesSubQ_Date2 < @ReportDateTo)) END) AS Remaining,  \r\n"
			"PatientID, UserDefinedID  \r\n"
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID  \r\n"
			"WHERE Type IN(1,2,3) AND Deleted = 0 AND PrePayment = 0  \r\n"
			"AND Amount + CASE WHEN (SELECT Sum(Amount) FROM " + strAppliesT + " WHERE DestID = LineItemT.ID AND SourceID IN (SELECT ID FROM LineItemT WHERE @AgedPayAppliesSubQ_Date1 < @ReportDateTo)) Is Null THEN 0 ELSE (SELECT Sum(Amount) FROM " + strAppliesT + " WHERE DestID = LineItemT.ID AND SourceID IN (SELECT ID FROM LineItemT WHERE @AgedPayAppliesSubQ_Date1 < @ReportDateTo)) END  \r\n"
			"- CASE WHEN (SELECT Sum(Amount) FROM " + strAppliesT + " WHERE SourceID = LineItemT.ID AND DestID IN (SELECT LineItemT.ID FROM LineItemT LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID LEFT JOIN (SELECT ID, Date FROM BillsT WHERE BillsT.Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID WHERE LineItemT.Deleted = 0 AND @AgedPayAppliesSubQ_Date2 < @ReportDateTo)) Is Null THEN 0  \r\n"
			"ELSE (SELECT Sum(Amount) FROM " + strAppliesT + " WHERE SourceID = LineItemT.ID AND DestID IN (SELECT LineItemT.ID FROM LineItemT LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID LEFT JOIN (SELECT ID, Date FROM BillsT WHERE BillsT.Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID WHERE LineItemT.Deleted = 0 AND @AgedPayAppliesSubQ_Date2 < @ReportDateTo)) END <> 0   \r\n"
			") AS AgedPayAppliesSubQ LEFT JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON AgedPayAppliesSubQ.DestID = LineItemT.ID)  \r\n"
			"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID)  \r\n"
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)  \r\n"
			"LEFT JOIN ProvidersT ON PaymentsT.ProviderID = ProvidersT.PersonID)  \r\n"
			"LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID   \r\n"
			") AS AgedPayAppliesUnfinishedQ   \r\n"
			") AS AgedLineItemsQ   \r\n"
			" LEFT JOIN  \r\n"
			"(  \r\n"
			"SELECT MaxDateQ.LastDate, OtherInfoQ.PatientID, OtherInfoQ.Description, OtherInfoQ.RespType,  \r\n"
			"OtherInfoQ.InputDate, OtherInfoQ.Amount  \r\n"
			"FROM   \r\n"
			"(  \r\n"
			"SELECT Max(Date) as LastDate, LineItemT.PatientID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  \r\n"
			" WHERE Type = 1 AND DELETED = 0  \r\n"
			" GROUP BY LineItemT.PatientID  \r\n"
			") AS MaxDateQ  \r\n"
			"INNER JOIN   \r\n"
			"(  \r\n"
			"SELECT Max(LineItemT.ID) as ID, LineItemT.PatientID, LineItemT.Date FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  \r\n"
			"WHERE Type = 1 AND DELETED = 0  \r\n"
			"GROUP BY LineItemT.PatientID, LineItemT.Date  \r\n"
			") IDQ  \r\n"
			" ON MaxDateQ.LastDate = IDQ.Date AND MaxDateQ.PatientID = IDQ.PatientID  \r\n"
			" LEFT JOIN   \r\n"
			"(  \r\n"
			"SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.Amount,  \r\n"
			" CASE WHEN InsuredPartyID IS NULL OR InsuredPartyID = -1 then 'Patient' ELSE InsuranceCoT.Name END AS RespType  \r\n"
			" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  \r\n"
			" LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID  \r\n"
			" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID  \r\n"
			" WHERE Type = 1 AND DELETED = 0  \r\n"
			") AS OtherInfoQ  \r\n"
			" ON IDQ.PatientID = OtherInfoQ.PatientID AND IDQ.ID = OtherInfoQ.ID  \r\n"
			") LastPayQ ON AgedLineItemsQ.PatID = LastPayQ.PatientID  \r\n"
			"WHERE (@AgedLineItemsQ_Date < @ReportDateTo)  ";

		if (bUseInputDate)
		{
			strArSql.Replace("@AgedChargeSubQ_Date", " LineItemT.InputDate ");
			strArSql.Replace("@AgedAppliesQ_Date1", " LineItemT.InputDate ");
			strArSql.Replace("@AgedAppliesQ_Date2", " BillsT.InputDate ");
			strArSql.Replace("@AgedPayAppliesSubQ_Date1", "LineItemT.InputDate ");
			strArSql.Replace("@AgedPayAppliesSubQ_Date2", " LineItemT.InputDate ");
			strArSql.Replace("@AgedLineItemsQ_Date", " AgedLineItemsQ.IDate ");
		}
		else
		{
			strArSql.Replace("@AgedChargeSubQ_Date", " BillsT.Date ");
			strArSql.Replace("@AgedAppliesQ_Date1", " LineItemT.Date ");
			strArSql.Replace("@AgedAppliesQ_Date2", " (CASE WHEN PointsToPayments=1 THEN LineChargesT.Date ELSE BillsT.Date END) ");
			strArSql.Replace("@AgedPayAppliesSubQ_Date1", " LineItemT.Date ");
			strArSql.Replace("@AgedPayAppliesSubQ_Date2", " (CASE WHEN BillsT.Date Is Null THEN LineItemT.Date ELSE BillsT.Date END) ");
			strArSql.Replace("@AgedLineItemsQ_Date", " AgedLineItemsQ.TDate ");
		}


		dtNext = DateTo + OneDay;
		strArSql.Replace("@ReportDateTo", "'" + _Q(FormatDateTimeForSql(dtNext)) + "'");
		return _T(strArSql);
	}	//end case
	break;



	case 170:
	case 385:
	case 659:
		//Aged Receivables By Insurance Co
		//Aged Receivables By Insurance Co Office
		//Aged Receivables By Financial Class
		/*	Version History
		DRT 4/4/03 - Added AR by Ins Co Office report to use this same query
		DRT 4/10/03 - Fixed a bug with insurance prepayments (which never really happen anyways, but should be handled)
		TES 6/6/03 - Added filter on assigment date.
		DRT 7/16/03 - This report had 2 UserDefinedID fields.  Due to some current issues with making ttx files, I can't remove the fields and
		fix it the "proper" way, so I renamed the 2nd one to UserDefID.  TODO:  We need to take this out in the future, so if you're
		making a new ttx file, please remove it.
		TES 3/5/04 - Added subreports for the billing notes.
		DRT 4/24/2004 - PLID 11816 - Fixed an issue with select *'s
		DRT 7/19/2004 - PLID 13443 - I changed the nDateFilter == 1 query to use '12/30/1899' as the date instead of NULL.  If you make 2 queries
		here with 2 different types for a field, it's impossible to make the ttx files correctly when trying to edit the report, and so 1 of
		them is always wrong.
		TES 9/20/2004 - PLID 14152 - Fixed problem where it was using charge rather than bill date.
		TES 7/8/2005 - PLID 16129 - Added LineNoteID and BillNoteID for use in optimization
		TES 10/27/2005 - PLID 18109 - It stopped filtering on the date at some point, so I put that back in.
		TES 1/10/2006 - PLID 18776 = Fixed calculation of DiagList field (was returning blank if you had 3 diag codes).
		ADW 9/15/2006 - PLID 21792 - Added ServiceDate (ie, charge date) to the query and reports. TDate is the Bill Date.
		EML 11/01/2006 - PLID 23200 - Had to change one of the joins for the AppliesFromQ sub query to account for
		multiple charge resp detail records. I then added a union to add back in the refunds since the new join could no longer include those records
		without a major performance hit.
		// (j.gruber 2008-07-01 10:06) - PLID 26389 - added last pay info
		TES 7/9/2008 - PLID 29580 - Filtered out applies which happened after the "As Of:" date.
		(d.thompson 2009-03-18) - PLID 33171 - Added responsibility type fields
		// (z.manning 2009-03-25 11:54) - PLID 19120 - Added financial class
		// (j.gruber 2010-01-28 15:07) - PLID 36899 - filtered out deleted bills
		// (r.gonet 2010-08-13 14:05) - PLID 40111 - Replaced slow correlated subqueries with faster uncorrelated subqueries
		TES 4/7/2011 - PLID 33741 - Added PlaceOfServiceID, PlaceOfService
		// (j.gruber 2011-12-21 13:36) - PLID 36465 - support payment input date as a filter
		// (b.spivey March 31st, 2014) PLID 61441 - Updated for ICD-10
		// (b.spivey April 10th, 2014) PLID 61441 - Fixed group by clause.
		// (j.gruber 2014-10-16) - PLID 63901 - supported by Input Date
		*/
		switch (nSubLevel) {
		case 1:
			switch (nSubRepNum) {
			case 1://Line Item Notes.
				return _T("SELECT LineItemID AS LineID, Date, Note FROM Notes WHERE LineItemID Is Not Null");
				break;
			default://Bill notes.
				return _T("SELECT BillID, Date, Note FROM Notes WHERE BillID Is Not Null");
				break;
			}
			break;
		default://Main report
			if (nDateFilter == 2 || nDateFilter == 3) {//Based on assignment date.

													   // (j.gruber 2011-12-22 13:35) - PLID 36465 - switch based off payment date used date
				CString strPayDate;
				if (nDateFilter == 2) {
					strPayDate = "LineItemT.Date";
				}
				else if (nDateFilter == 3) {
					strPayDate = "LineItemT.InputDate";
				}

				// (a.walling 2014-11-17 13:09) - PLID 64145 - join with BillDiagCodes outside the inner unioned subquery
				// (r.gonet 2015-11-24 11:20) - PLID 67620 - Replaced a UNION of applies with a UNION ALL. The set-unioning behavior was
				// causing an apply to be combined with another, different apply because they had the same source ID and amount, but different
				// RespIDs.
				CString strSql = _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
					"LineItemT.Description, convert(money,ARInsLineItemsQ.Bal) AS Diff, ARInsLineItemsQ.ProvID AS ProvID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName,    "
					"InsuranceCoT.Name AS InsName, PersonT2.Address1 AS InsAdd1, PersonT2.Address2 AS InsAdd2, PersonT2.City AS InsCity, PersonT2.State AS InsState, PersonT2.Zip AS InsZip,  "
					"InsContactPerson.WorkPhone AS InsPhone, PersonT.BirthDate, InsuredPartyT.IDForInsurance AS PolicyNum, ARInsLineItemsQ.TDate AS TDate, ARInsLineItemsQ.IDate AS IDate, ARInsLineItemsQ.BillDesc,  "
					"InsuranceCoT.PersonID AS InsCoID, PatientsT.UserDefinedID AS UserDefID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
					"ARInsLineItemsQ.POSID AS PlaceOfServiceID, ARInsLineItemsQ.PlaceOfService, "
					"CASE WHEN InsContactPerson.ID IS NULL THEN '' ELSE InsContactPerson.Last + ', ' + InsContactPerson.First + ' ' + InsContactPerson.Middle END AS InsContactName, "
					"InsPartyPerson.Last + ', ' + InsPartyPerson.First + ' ' + InsPartyPerson.Middle AS InsPartyName, InsuredPartyT.RelationToPatient, "
					"InsuredPartyT.PolicyGroupNum, ArInsLineItemsQ.PriorAuthNum, ArInsLineItemsQ.Code, "
					"	ICD9T1.CodeNumber as ICD9Code1, \r\n "
					"	ICD9T2.CodeNumber as ICD9Code2, \r\n "
					"	ICD9T3.CodeNumber as ICD9Code3, \r\n "
					"	ICD9T4.CodeNumber as ICD9Code4, \r\n "
					"	ICD10T1.CodeNumber as ICD10Code1, \r\n "
					"	ICD10T2.CodeNumber as ICD10Code2, \r\n "
					"	ICD10T3.CodeNumber as ICD10Code3, \r\n "
					"	ICD10T4.CodeNumber as ICD10Code4, \r\n "
					"ArInsLineItemsQ.AssignDate AS ADate, InsPartyPerson.Note AS InsNotes, "
					"ArInsLineItemsQ.ID AS LineID, ArInsLineItemsQ.BillID, BillNoteID, LineNoteID, "
					"ArInsLineItemsQ.ServiceDate AS ServiceDate, "
					"LastPayQ.LastDate as LastPayDate, LastPayQ.Description as LastPayDesc, LastPayQ.RespType as LastPayRespType, LastPayQ.InputDate As LastPayInputDate, LastPayQ.Amount as LastPayAmount, "
					"RespTypeT.Priority AS RespTypePriority, RespTypeT.TypeName AS RespTypeName, "
					"FinancialClassT.ID AS FinClassID, FinancialClassT.Name AS FinClass "
					"FROM PatientsT INNER JOIN InsuredPartyT LEFT JOIN PersonT InsContactPerson ON InsuredPartyT.InsuranceContactID = InsContactPerson.ID "
					"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID)  "
					"INNER JOIN  "
					"	/*Begin ARLineItemsQ*/ "
					"	(SELECT *  "
					"	FROM  "
					"		(SELECT LineItemT.ID, ChargeRespT.InsuredPartyID, ChargeRespDetailT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) AS Bal, "
					"		BillsT.Date AS TDate, LineItemT.InputDate as IDate, BillsT.Description AS BillDesc, LineItemT.PatientID, ChargesT.DoctorsProviders AS ProvID,  "
					"		BillsT.PriorAuthNum, CPTCodeT.Code,  "
					"		ChargeRespDetailT.Date AS AssignDate, "
					"       BillsT.ID AS BillID, "
					"		BillNoteID, LineNoteID, "
					"		LineItemT.Date AS ServiceDate, "
					"		Location_POS.ID AS POSID, Location_POS.Name AS PlaceOfService "
					"		FROM (ChargeRespT LEFT JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID) LEFT OUTER JOIN  "
					"			(SELECT AppliesT.ID, SourceID, DestID, RespID, ApplyDetailsT.Amount, ApplyDetailsT.DetailID, PaymentsT.InsuredPartyID AS InsuredPartyID, LineItemT1.PatientID AS PatID "
					"			FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN (" + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID) INNER JOIN  "
					"			PaymentsT ON AppliesT.SourceID = PaymentsT.ID INNER JOIN  "
					"			LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID ON  "
					"			ChargesT.ID = AppliesT.DestID "
					"			WHERE (PaymentsT.InsuredPartyID > 0) AND (AppliesT.PointsToPayments = 0) AND (LineItemT.Deleted = 0) AND LineItemT1.Date < @ReportDateTo) AS ARInsAppliesQ  "
					"		ON ChargeRespDetailT.ID = ARInsAppliesQ.DetailID LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
					"		LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT Location_POS ON BillsT.Location = Location_POS.ID LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
					"		LEFT JOIN "
					"			(SELECT MIN(ID) AS LineNoteID, LineItemID "
					"			FROM Notes "
					"			WHERE LineItemID IS NOT NULL "
					"			GROUP BY LineItemID) NotesSQ1 "
					"		ON LineItemT.ID = LineItemID "
					"		LEFT JOIN "
					"			(SELECT MIN(ID) AS BillNoteID, BillID "
					"			FROM Notes "
					"			WHERE BillID IS NOT NULL "
					"			GROUP BY BillID "
					"		) NotesSQ2 "
					"		ON BillsT.ID = NotesSQ2.BillID "
					"		WHERE ChargeRespT.InsuredPartyID Is Not Null AND LineItemT.Deleted = 0  "
					"		GROUP BY ChargeRespT.ID, ChargeRespDetailT.Amount, LineItemT.ID, ChargeRespT.InsuredPartyID, BillsT.Date, LineItemT.Inputdate, BillsT.Description, LineItemT.PatientID, ChargesT.DoctorsProviders,  "
					"		BillsT.PriorAuthNum, CPTCodeT.Code, "
					"		ChargeRespDetailT.Date, BillsT.ID, LineItemT.Date, "
					"		NotesSQ1.LineNoteID, NotesSQ2.BillNoteID, Location_POS.ID, Location_POS.Name "
					"		HAVING ChargeRespDetailT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) != 0) AS ARInsChargesUnfinishedQ    "
					" "
					"		UNION  "
					"		SELECT * FROM  "
					"			(SELECT LineItemT.ID, PaymentsT.InsuredPartyID, -1*(Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End)+  "
					"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END)) AS Bal, LineItemT.Date AS TDate, LineItemT.InputDate as IDate, '' AS BillDesc, LineItemT.PatientID,  "
					"			PaymentsT.ProviderID AS ProvID, '' AS PriorAuthNum, '' AS Code, "
					"			" + strPayDate + " AS AssignDate, "
					"           -1 AS BillID, "
					"			NULL AS BillNoteID, "
					"			LineNoteID, "
					"			LineItemT.Date AS ServiceDate, "
					"			LocationsT.ID AS POSID, LocationsT.Name AS PlaceOfService "
					"			FROM  "
					"				((SELECT AppliesT.SourceID, Sum(ApplyDetailsT.Amount) AS Amount  "
					"				FROM " + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.SourceID = PaymentsT.ID INNER JOIN (LineItemT LineDestT LEFT JOIN ChargeRespT "
					"					INNER JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID ON LineDestT.ID = ChargeRespT.ChargeID) ON ApplyDetailsT.DetailID = ChargeRespDetailT.ID "
					"				WHERE (PaymentsT.InsuredPartyID > 0) AND (LineItemT.Date < @ReportDateTo AND (CASE WHEN ChargeRespDetailT.Date IS Null THEN LineDestT.Date ELSE ChargeRespDetailT.Date END) < @ReportDateTo)    "
					"				GROUP BY AppliesT.SourceID) /*AS AppliesFromQ*/  "
					"				UNION ALL "
					"				(SELECT AppliesT.SourceID, Sum(ApplyDetailsT.Amount) AS Amount  "
					"				FROM " + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID INNER JOIN LineItemT LineDestT2 "
					"				ON (AppliesT.DestID = LineDestT2.ID AND (AppliesT.RespID Is Null)) "
					"				GROUP BY AppliesT.SourceID)) AS AppliesFromQ  "
					"			RIGHT JOIN ( "
					"				(SELECT AppliesT.DestID, Sum(ApplyDetailsT.Amount) AS Amount  "
					"				FROM (" + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID) INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.DestID = PaymentsT.ID INNER JOIN LineItemT LineSourceT ON AppliesT.SourceID = LineSourceT.ID "
					"				WHERE (PaymentsT.InsuredPartyID > 0) AND (LineItemT.Date < @ReportDateTo AND LineSourceT.Date < @ReportDateTo) "
					"				GROUP BY AppliesT.DestID) AS AppliesToQ  "
					"			RIGHT JOIN PaymentsT ON AppliesToQ.DestID = PaymentsT.ID) ON AppliesFromQ.SourceID = PaymentsT.ID  "
					"			INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
					"			LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
					"			LEFT JOIN "
					"				(SELECT MIN(ID) AS LineNoteID, LineItemID "
					"				FROM Notes "
					"				WHERE LineItemID IS NOT NULL "
					"				GROUP BY LineItemID) NotesSQ1 "
					"			ON LineItemT.ID = LineItemID "
					"			WHERE PaymentsT.InsuredPartyID > 0 AND LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 0 "
					"			GROUP BY LineItemT.ID, PaymentsT.InsuredPartyID, " + strPayDate + ", LineItemT.Date, LineItemT.InputDate, LineItemT.PatientID, LineItemT.Amount, PaymentsT.ProviderID,  "
					"			NotesSQ1.LineNoteID, LocationsT.ID, LocationsT.Name "
					"			HAVING Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End) +  "
					"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END) != 0) AS ARInsPaysUnfinishedQ "
					"	) AS ARInsLineItemsQ  "
					"	/* End ARInsLineItemsQ */ "
					"ON LineItemT.ID = ARInsLineItemsQ.ID ON InsuredPartyT.PersonID = ARInsLineItemsQ.InsuredPartyID  "
					"INNER JOIN (InsuranceCoT LEFT JOIN PersonT PersonT2 ON InsuranceCoT.PersonID = PersonT2.ID) ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID  "
					"LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID "
					"INNER JOIN PersonT ON ARInsLineItemsQ.PatientID = PersonT.ID ON PatientsT.PersonID = PersonT.ID LEFT JOIN PersonT PersonT1 ON ARInsLineItemsQ.ProvID = PersonT1.ID "
					"LEFT JOIN PersonT InsPartyPerson ON InsuredPartyT.PersonID = InsPartyPerson.ID "
					"	LEFT JOIN BillDiagCodeFlat4V ON ARInsLineItemsQ.BillID = BillDiagCodeFlat4V.BillID AND ARInsLineItemsQ.BillID <> -1 \r\n "
					"	LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
					"	LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
					" LEFT JOIN (SELECT MaxDateQ.LastDate, OtherInfoQ.PatientID, OtherInfoQ.Description, OtherInfoQ.RespType, "
					" OtherInfoQ.InputDate, OtherInfoQ.Amount "
					" FROM  "
					" (SELECT Max(Date) as LastDate, LineItemT.PatientID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" WHERE Type = 1 AND DELETED = 0 "
					" GROUP BY LineItemT.PatientID) AS MaxDateQ "
					" INNER JOIN  "
					" (SELECT Max(LineItemT.ID) as ID, LineItemT.PatientID, LineItemT.Date FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" WHERE Type = 1 AND DELETED = 0 "
					" GROUP BY LineItemT.PatientID, LineItemT.Date) IDQ "
					" ON MaxDateQ.LastDate = IDQ.Date AND MaxDateQ.PatientID = IDQ.PatientID "
					" LEFT JOIN  "
					" (SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.Amount, "
					" CASE WHEN InsuredPartyID IS NULL OR InsuredPartyID = -1 then 'Patient' ELSE InsuranceCoT.Name END AS RespType "
					" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID "
					" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					" LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID "
					" WHERE Type = 1 AND DELETED = 0) AS OtherInfoQ "
					" ON IDQ.PatientID = OtherInfoQ.PatientID "
					" AND IDQ.ID = OtherInfoQ.ID) LastPayQ "
					" ON ARInsLineItemsQ.PatientID = LastPayQ.PatientID "
					"WHERE ArInsLineItemsQ.AssignDate < @ReportDateTo");
				dtNext = DateTo + OneDay;
				strSql.Replace("@ReportDateTo", "'" + _Q(FormatDateTimeForSql(dtNext)) + "'");
				return strSql;
			}
			else {

				// (a.walling 2014-11-17 13:09) - PLID 64145 - join with BillDiagCodes outside the inner unioned subquery
				CString strSql = _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
					"LineItemT.Description, convert(money,ARInsLineItemsQ.Bal) AS Diff, ARInsLineItemsQ.ProvID AS ProvID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName,    "
					"InsuranceCoT.Name AS InsName, PersonT2.Address1 AS InsAdd1, PersonT2.Address2 AS InsAdd2, PersonT2.City AS InsCity, PersonT2.State AS InsState, PersonT2.Zip AS InsZip,  "
					"InsContactPerson.WorkPhone AS InsPhone, PersonT.BirthDate, InsuredPartyT.IDForInsurance AS PolicyNum, ARInsLineItemsQ.TDate AS TDate, ARInsLineItemsQ.IDate AS IDate, ARInsLineItemsQ.BillDesc,  "
					"InsuranceCoT.PersonID AS InsCoID, PatientsT.UserDefinedID AS UserDefID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
					"ArInsLineItemsQ.POSID AS PlaceOfServiceID, ArInsLineItemsQ.PlaceOfService, "
					"CASE WHEN InsContactPerson.ID IS NULL THEN '' ELSE InsContactPerson.Last + ', ' + InsContactPerson.First + ' ' + InsContactPerson.Middle END AS InsContactName, "
					"InsPartyPerson.Last + ', ' + InsPartyPerson.First + ' ' + InsPartyPerson.Middle AS InsPartyName, InsuredPartyT.RelationToPatient, "
					"InsuredPartyT.PolicyGroupNum, ArInsLineItemsQ.PriorAuthNum, ArInsLineItemsQ.Code, "
					"	ICD9T1.CodeNumber as ICD9Code1, \r\n "
					"	ICD9T2.CodeNumber as ICD9Code2, \r\n "
					"	ICD9T3.CodeNumber as ICD9Code3, \r\n "
					"	ICD9T4.CodeNumber as ICD9Code4, \r\n "
					"	ICD10T1.CodeNumber as ICD10Code1, \r\n "
					"	ICD10T2.CodeNumber as ICD10Code2, \r\n "
					"	ICD10T3.CodeNumber as ICD10Code3, \r\n "
					"	ICD10T4.CodeNumber as ICD10Code4, \r\n "
					"convert(datetime, '12/30/1899') AS ADate, InsPartyPerson.Note AS InsNotes, "
					"ArInsLineItemsQ.ID AS LineID, ArInsLineItemsQ.BillID, BillNoteID, LineNoteID, "
					"ArInsLineItemsQ.ServiceDate AS ServiceDate, "
					"LastPayQ.LastDate as LastPayDate, LastPayQ.Description as LastPayDesc, LastPayQ.RespType as LastPayRespType, LastPayQ.InputDate As LastPayInputDate, LastPayQ.Amount as LastPayAmount, "
					"RespTypeT.Priority AS RespTypePriority, RespTypeT.TypeName AS RespTypeName, "
					"FinancialClassT.ID AS FinClassID, FinancialClassT.Name AS FinClass "
					"FROM PatientsT INNER JOIN InsuredPartyT LEFT JOIN PersonT InsContactPerson ON InsuredPartyT.InsuranceContactID = InsContactPerson.ID "
					"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID)  "
					"INNER JOIN  "
					"	/*Begin ARLineItemsQ*/ "
					"	(SELECT *  "
					"	FROM  "
					"		(SELECT LineItemT.ID, ChargeRespT.InsuredPartyID, ChargeRespT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) AS Bal, "
					"		BillsT.Date AS TDate, LineItemT.InputDate as IDate, BillsT.Description AS BillDesc, LineItemT.PatientID, ChargesT.DoctorsProviders AS ProvID,  "
					"		BillsT.PriorAuthNum, CPTCodeT.Code,  "
					"       BillsT.ID AS BillID, "
					"		BillNoteID, LineNoteID, "
					"		LineItemT.Date AS ServiceDate, "
					"		Location_POS.ID AS POSID, Location_POS.Name AS PlaceOfService "
					"		FROM ChargeRespT LEFT OUTER JOIN  "
					"			(SELECT AppliesT.ID, AppliesT.SourceID, AppliesT.DestID, AppliesT.RespID, AppliesT.Amount, AppliesT.PointsToPayments, AppliesT.InputDate, AppliesT.InputName, PaymentsT.InsuredPartyID AS InsuredPartyID, LineItemT1.PatientID AS PatID "
					"			FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN " + strAppliesT + " INNER JOIN  "
					"			PaymentsT ON AppliesT.SourceID = PaymentsT.ID INNER JOIN  "
					"			LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID ON  "
					"			ChargesT.ID = AppliesT.DestID "
					"			WHERE (PaymentsT.InsuredPartyID > 0) AND (AppliesT.PointsToPayments = 0) AND (LineItemT.Deleted = 0) AND LineItemT1.@DateFilter < @ReportDateTo) AS ARInsAppliesQ  "
					"		ON ChargeRespT.ID = ARInsAppliesQ.RespID LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
					"		LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT Location_POS ON BillsT.Location = Location_POS.ID LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
					"		LEFT JOIN "
					"			(SELECT MIN(ID) AS LineNoteID, LineItemID "
					"			FROM Notes "
					"			WHERE LineItemID IS NOT NULL "
					"			GROUP BY LineItemID "
					"		) NotesSQ1 "
					"		ON LineItemT.ID = LineItemID "
					"		LEFT JOIN "
					"			(SELECT MIN(ID) AS BillNoteID, BillID "
					"			FROM Notes "
					"			WHERE BillID IS NOT NULL "
					"			GROUP BY BillID "
					"		) NotesSQ2 "
					"		ON BillsT.ID = NotesSQ2.BillID "
					"		WHERE ChargeRespT.InsuredPartyID Is Not Null AND LineItemT.Deleted = 0  "
					"		GROUP BY ChargeRespT.ID, ChargeRespT.Amount, LineItemT.ID, ChargeRespT.InsuredPartyID, BillsT.Date, LineItemT.Inputdate, BillsT.Description, LineItemT.PatientID, ChargesT.DoctorsProviders,  "
					"		BillsT.PriorAuthNum, CPTCodeT.Code, "
					"		BillsT.ID, LineItemT.Date, "
					"		NotesSQ1.LineNoteID, NotesSQ2.BillNoteID, Location_POS.ID, Location_POS.Name "
					"		HAVING ChargeRespT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) != 0) AS ARInsChargesUnfinishedQ    "
					" "
					"		UNION  "
					"		SELECT * FROM  "
					"			(SELECT LineItemT.ID, PaymentsT.InsuredPartyID, -1*(Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End)+  "
					"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END)) AS Bal, LineItemT.Date AS TDate, LineItemT.InputDate as IDate, '' AS BillDesc, LineItemT.PatientID,  "
					"			PaymentsT.ProviderID AS ProvID, '' AS PriorAuthNum, '' AS Code, "
					"           -1 AS BillID, "
					"			NULL AS BillNoteID, "
					"			LineNoteID, "
					"			LineItemT.Date AS ServiceDate, "
					"			LocationsT.ID AS POSID, LocationsT.Name AS PlaceOfService "
					"			FROM  "
					"				(SELECT AppliesT.SourceID, Sum(AppliesT.Amount) AS Amount  "
					"				FROM " + strAppliesT + " INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.SourceID = PaymentsT.ID INNER JOIN LineItemT LineDestT ON AppliesT.DestID = LineDestT.ID "
					"				LEFT JOIN ChargesT ON LineDestT.ID = ChargesT.ID LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID "
					"				WHERE (PaymentsT.InsuredPartyID > 0) AND (LineItemT.@DateFilter < @ReportDateTo AND CASE WHEN BillsT.Date Is Null THEN LineDestT.@DateFilter ELSE BillsT.@DateFilter END < @ReportDateTo)   "
					"				GROUP BY AppliesT.SourceID) AS AppliesFromQ  "
					"			RIGHT JOIN ( "
					"				(SELECT AppliesT.DestID, Sum(AppliesT.Amount) AS Amount  "
					"				FROM " + strAppliesT + " INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.DestID = PaymentsT.ID INNER JOIN LineItemT LineSourceT ON AppliesT.SourceID = LineSourceT.ID "
					"				WHERE (PaymentsT.InsuredPartyID > 0) AND (LineItemT.@DateFilter < @ReportDateTo AND LineSourceT.@DateFilter < @ReportDateTo) "
					"				GROUP BY AppliesT.DestID) AS AppliesToQ  "
					"			RIGHT JOIN PaymentsT ON AppliesToQ.DestID = PaymentsT.ID) ON AppliesFromQ.SourceID = PaymentsT.ID  "
					"			INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
					"			LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
					"			LEFT JOIN "
					"				(SELECT MIN(ID) AS LineNoteID, LineItemID "
					"				FROM Notes "
					"				WHERE LineItemID IS NOT NULL "
					"				GROUP BY LineItemID) NotesSQ1 "
					"			ON LineItemT.ID = LineItemID "
					"			WHERE PaymentsT.InsuredPartyID > 0 AND LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 0 "
					"			GROUP BY LineItemT.ID, PaymentsT.InsuredPartyID, LineItemT.Date, LineItemT.InputDate, LineItemT.PatientID, LineItemT.Amount, PaymentsT.ProviderID,  "
					"			NotesSQ1.LineNoteID, LocationsT.ID, LocationsT.Name "
					"			HAVING Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End) +  "
					"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END) != 0) AS ARInsPaysUnfinishedQ "
					"	) AS ARInsLineItemsQ  "
					"	/* End ARInsLineItemsQ */ "
					"ON LineItemT.ID = ARInsLineItemsQ.ID ON InsuredPartyT.PersonID = ARInsLineItemsQ.InsuredPartyID  "
					"INNER JOIN (InsuranceCoT LEFT JOIN PersonT PersonT2 ON InsuranceCoT.PersonID = PersonT2.ID) ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID  "
					"LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID "
					"INNER JOIN PersonT ON ARInsLineItemsQ.PatientID = PersonT.ID ON PatientsT.PersonID = PersonT.ID LEFT JOIN PersonT PersonT1 ON ARInsLineItemsQ.ProvID = PersonT1.ID "
					"LEFT JOIN PersonT InsPartyPerson ON InsuredPartyT.PersonID = InsPartyPerson.ID "
					"	LEFT JOIN BillDiagCodeFlat4V ON ARInsLineItemsQ.BillID = BillDiagCodeFlat4V.BillID AND ARInsLineItemsQ.BillID <> -1 \r\n "
					"	LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
					"	LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
					" LEFT JOIN (SELECT MaxDateQ.LastDate, OtherInfoQ.PatientID, OtherInfoQ.Description, OtherInfoQ.RespType, "
					" OtherInfoQ.InputDate, OtherInfoQ.Amount "
					" FROM  "
					" (SELECT Max(Date) as LastDate, LineItemT.PatientID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" WHERE Type = 1 AND DELETED = 0 "
					" GROUP BY LineItemT.PatientID) AS MaxDateQ "
					" INNER JOIN  "
					" (SELECT Max(LineItemT.ID) as ID, LineItemT.PatientID, LineItemT.Date FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" WHERE Type = 1 AND DELETED = 0 "
					" GROUP BY LineItemT.PatientID, LineItemT.Date) IDQ "
					" ON MaxDateQ.LastDate = IDQ.Date AND MaxDateQ.PatientID = IDQ.PatientID "
					" LEFT JOIN  "
					" (SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.Amount, "
					" CASE WHEN InsuredPartyID IS NULL OR InsuredPartyID = -1 then 'Patient' ELSE InsuranceCoT.Name END AS RespType "
					" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID "
					" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					" LEFT JOIN FinancialClassT ON InsuranceCoT.FinancialClassID = FinancialClassT.ID "
					" WHERE Type = 1 AND DELETED = 0) AS OtherInfoQ "
					" ON IDQ.PatientID = OtherInfoQ.PatientID "
					" AND IDQ.ID = OtherInfoQ.ID) LastPayQ "
					" ON ARInsLineItemsQ.PatientID = LastPayQ.PatientID "
					"WHERE ARInsLineItemsQ.@DateFilter2 < @ReportDateTo");
				dtNext = DateTo + OneDay;
				strSql.Replace("@ReportDateTo", "'" + _Q(FormatDateTimeForSql(dtNext)) + "'");

				// (j.gruber 2014-10-16) - PLID 63901 - Added IDate field
				if (nDateFilter == 1)
				{
					//date
					strSql.Replace("@DateFilter2", "TDate");
					strSql.Replace("@DateFilter", "Date");
				}
				else if (nDateFilter == 4)
				{
					//inputdate
					strSql.Replace("@DateFilter2", "IDate");
					strSql.Replace("@DateFilter", "InputDate");


				}
				return strSql;
			}
			break;
		}
		break;

	case 390:
		//Aged Receivables By Insurance Co by Claim Date
		/*	Version History
		DRT 4/22/2004 - PLID 11816 - Fixed issues with select *'s
		// (j.gruber 2008-07-01 10:34) - PLID 26389 - added last pay info fields
		TES 7/9/2008 - PLID 29580 - Filtered out applies which happened after the "As Of:" date.
		(d.thompson 2009-03-18) - PLID 33171 - Added resp type fields
		// (j.gruber 2010-01-28 15:07) - PLID 36899 - filtered out deleted bills
		TES 4/7/2011 - PLID 33741 - Added PlaceOfServiceID, PlaceOfService
		//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values
		// (d.thompson 2014-03-28) - PLID 61445 - Added fields for ICD-10.  Previously this query generated a string
		//	that was ICD9-1, ICD9-2, etc.  I've changed this to just output the 8 fields (ic9 1-4 and icd10 1-4), and
		//	put a formula in the report to do the aggregration of display.
		// (a.walling 2014-11-17 13:09) - PLID 64145 - join with BillDiagCodes outside the inner unioned subquery
		*/
		return _T(
			FormatString(
				"SELECT PatientsT.UserDefinedID,  "
				"PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
				"LineItemT.Description, convert(money,ARInsLineItemsQ.Bal) AS Diff, ARInsLineItemsQ.ProvID AS ProvID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName,     "
				"InsuranceCoT.Name AS InsName,  "
				"PersonT2.Address1 AS InsAdd1, PersonT2.Address2 AS InsAdd2, PersonT2.City AS InsCity, PersonT2.State AS InsState, PersonT2.Zip AS InsZip,   "
				"InsContactPerson.WorkPhone AS InsPhone, PersonT.BirthDate, InsuredPartyT.IDForInsurance AS PolicyNum,  "
				"ARInsLineItemsQ.TDate AS TDate, ARInsLineItemsQ.BillDesc,   "
				"InsuranceCoT.PersonID AS InsCoID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  "
				"ARInsLineItemsQ.POSID AS PlaceOfServiceID, ARInsLineItemsQ.PlaceOfService, "
				"CASE WHEN InsContactPerson.ID IS NULL THEN '' ELSE InsContactPerson.Last + ', ' + InsContactPerson.First + ' ' + InsContactPerson.Middle END AS InsContactName,  "
				"InsPartyPerson.Last + ', ' + InsPartyPerson.First + ' ' + InsPartyPerson.Middle AS InsPartyName, InsuredPartyT.RelationToPatient,  "
				"InsuredPartyT.PolicyGroupNum, ArInsLineItemsQ.PriorAuthNum, ArInsLineItemsQ.Code, "
				"	ICD9T1.CodeNumber as ICD9Code1, \r\n "
				"	ICD9T2.CodeNumber as ICD9Code2, \r\n "
				"	ICD9T3.CodeNumber as ICD9Code3, \r\n "
				"	ICD9T4.CodeNumber as ICD9Code4, \r\n "
				"	ICD10T1.CodeNumber as ICD10Code1, \r\n "
				"	ICD10T2.CodeNumber as ICD10Code2, \r\n "
				"	ICD10T3.CodeNumber as ICD10Code3, \r\n "
				"	ICD10T4.CodeNumber as ICD10Code4, \r\n "
				"ArInsLineItemsQ.ClaimDate AS ClaimDate, "
				"InsPartyPerson.Note AS InsNotes, "
				"LastPayQ.LastDate as LastPayDate, LastPayQ.Description as LastPayDesc, LastPayQ.RespType as LastPayRespType, LastPayQ.InputDate As LastPayInputDate, LastPayQ.Amount as LastPayAmount, "
				"RespTypeT.Priority AS RespTypePriority, RespTypeT.TypeName AS RespTypeName "
				" "
				"FROM PatientsT INNER JOIN InsuredPartyT LEFT JOIN PersonT InsContactPerson ON InsuredPartyT.InsuranceContactID = InsContactPerson.ID  "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"INNER JOIN (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID)   "
				" "
				"INNER JOIN   "
				"	/*Begin ARLineItemsQ*/  "
				"	(SELECT *   "
				"	FROM   "
				"		(SELECT LineItemT.ID, ChargeRespT.InsuredPartyID, ChargeRespT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) AS Bal,  "
				"		BillsT.Date AS TDate, BillsT.ID AS BillID, BillsT.Description AS BillDesc, LineItemT.PatientID, ChargesT.DoctorsProviders AS ProvID,   "
				"		BillsT.PriorAuthNum, CPTCodeT.Code,   "
				"		ClaimDateQ.LastDate AS ClaimDate, "
				"		Location_POS.ID AS POSID, Location_POS.Name AS PlaceOfService "
				"		FROM ChargeRespT LEFT OUTER JOIN   "
				"			(SELECT AppliesT.ID, AppliesT.SourceID, AppliesT.DestID, AppliesT.RespID, AppliesT.Amount, AppliesT.PointsToPayments, AppliesT.InputDate, AppliesT.InputName, PaymentsT.InsuredPartyID AS InsuredPartyID, LineItemT1.PatientID AS PatID  "
				"			FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN " + strAppliesT + " INNER JOIN   "
				"			PaymentsT ON AppliesT.SourceID = PaymentsT.ID INNER JOIN   "
				"			LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID ON   "
				"			ChargesT.ID = AppliesT.DestID  "
				"			WHERE (PaymentsT.InsuredPartyID > 0) AND (AppliesT.PointsToPayments = 0) AND (LineItemT.Deleted = 0)) AS ARInsAppliesQ   "
				"		ON ChargeRespT.ID = ARInsAppliesQ.RespID LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID   "
				"		LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT Location_POS ON BillsT.Location = Location_POS.ID LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID  "
				"		LEFT JOIN (SELECT BillID, InsuredPartyID, Max(Date) AS LastDate from ClaimHistoryT WHERE SendType <> %li GROUP BY BillID, InsuredPartyID) ClaimDateQ ON  "
				"		BillsT.ID = ClaimDateQ.BillID AND ChargeRespT.InsuredpartyID = ClaimDateQ.InsuredPartyID "
				"		WHERE ChargeRespT.InsuredPartyID Is Not Null AND LineItemT.Deleted = 0   "
				"		GROUP BY ChargeRespT.ID, ChargeRespT.Amount, LineItemT.ID, ChargeRespT.InsuredPartyID, BillsT.Date, BillsT.ID, BillsT.Description, LineItemT.PatientID, ChargesT.DoctorsProviders,   "
				"		BillsT.PriorAuthNum, CPTCodeT.Code, "
				"		ClaimDateQ.LastDate, "
				"		Location_POS.ID, Location_POS.Name "
				"		HAVING ChargeRespT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) != 0) AS ARInsChargesUnfinishedQ  "
				"  "
				"		UNION   "
				"		SELECT * FROM   "
				"			(SELECT LineItemT.ID, PaymentsT.InsuredPartyID, -1*(Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End)+   "
				"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END)) AS Bal, LineItemT.Date AS TDate, -1 AS BillID, '' AS BillDesc, LineItemT.PatientID,   "
				"			PaymentsT.ProviderID AS ProvID, '' AS PriorAuthNum, '' AS Code, "
				"			NULL AS ClaimDate, "
				"			LocationsT.ID AS POSID, LocationsT.Name AS PlaceOfService "
				"			FROM   "
				"				(SELECT AppliesT.SourceID, Sum(AppliesT.Amount) AS Amount   "
				"				FROM " + strAppliesT + " INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID  "
				"				WHERE (PaymentsT.InsuredPartyID > 0)     "
				"				GROUP BY AppliesT.SourceID) AS AppliesFromQ   "
				"			RIGHT JOIN (  "
				"				(SELECT AppliesT.DestID, Sum(AppliesT.Amount) AS Amount   "
				"				FROM " + strAppliesT + " INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID   "
				"				WHERE (PaymentsT.InsuredPartyID > 0)   "
				"				GROUP BY AppliesT.DestID) AS AppliesToQ   "
				"			RIGHT JOIN PaymentsT ON AppliesToQ.DestID = PaymentsT.ID) ON AppliesFromQ.SourceID = PaymentsT.ID   "
				"			INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID   "
				"			LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"			WHERE PaymentsT.InsuredPartyID > 0 AND LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 0  "
				"			GROUP BY LineItemT.ID, PaymentsT.InsuredPartyID, LineItemT.Date, LineItemT.PatientID, LineItemT.Amount, PaymentsT.ProviderID, LocationsT.ID, LocationsT.Name   "
				"			HAVING Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End) +   "
				"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END) != 0) AS ARInsPaysUnfinishedQ  "
				"	) AS ARInsLineItemsQ   "
				"	/* End ARInsLineItemsQ */  "
				"ON LineItemT.ID = ARInsLineItemsQ.ID ON InsuredPartyT.PersonID = ARInsLineItemsQ.InsuredPartyID   "
				"	LEFT JOIN BillDiagCodeFlat4V ON ARInsLineItemsQ.BillID = BillDiagCodeFlat4V.BillID AND ARInsLineItemsQ.BillID <> -1 \r\n "
				"	LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
				"	LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
				"INNER JOIN (InsuranceCoT LEFT JOIN PersonT PersonT2 ON InsuranceCoT.PersonID = PersonT2.ID) ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID   "
				"INNER JOIN PersonT ON ARInsLineItemsQ.PatientID = PersonT.ID ON PatientsT.PersonID = PersonT.ID LEFT JOIN PersonT PersonT1 ON ARInsLineItemsQ.ProvID = PersonT1.ID  "
				"LEFT JOIN PersonT InsPartyPerson ON InsuredPartyT.PersonID = InsPartyPerson.ID "
				" LEFT JOIN (SELECT MaxDateQ.LastDate, OtherInfoQ.PatientID, OtherInfoQ.Description, OtherInfoQ.RespType, "
				" OtherInfoQ.InputDate, OtherInfoQ.Amount "
				" FROM  "
				" (SELECT Max(Date) as LastDate, LineItemT.PatientID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" WHERE Type = 1 AND DELETED = 0 "
				" GROUP BY LineItemT.PatientID) AS MaxDateQ "
				" INNER JOIN  "
				" (SELECT Max(LineItemT.ID) as ID, LineItemT.PatientID, LineItemT.Date FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" WHERE Type = 1 AND DELETED = 0 "
				" GROUP BY LineItemT.PatientID, LineItemT.Date) IDQ "
				" ON MaxDateQ.LastDate = IDQ.Date AND MaxDateQ.PatientID = IDQ.PatientID "
				" LEFT JOIN  "
				" (SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.Amount, "
				" CASE WHEN InsuredPartyID IS NULL OR InsuredPartyID = -1 then 'Patient' ELSE InsuranceCoT.Name END AS RespType "
				" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID "
				" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				" WHERE Type = 1 AND DELETED = 0) AS OtherInfoQ "
				" ON IDQ.PatientID = OtherInfoQ.PatientID "
				" AND IDQ.ID = OtherInfoQ.ID) LastPayQ "
				" ON ARInsLineItemsQ.PatientID = LastPayQ.PatientID "
				" "
				"WHERE ClaimDate IS NOT NULL", ClaimSendType::TracerLetter));
		break;

	case 388:
		//Aged Receivables By Patient Responsibility
		/*	Version History
		DRT 4/10/03 - Added report - shows ONLY Patient responsibilty, no insurance information at all, aged like the others
		TES 6/6/03 - Added option to filter on "assignment" date.
		DRT 7/16/03 - This report had 2 UserDefinedID fields.  Due to some current issues with making ttx files, I can't remove the fields and
		fix it the "proper" way, so I renamed the 2nd one to UserDefID.  TODO:  We need to take this out in the future, so if you're
		making a new ttx file, please remove it.
		DRT 4/22/2004 - PLID 11816 - Fixed some bad select *'s
		DRT 7/19/2004 - PLID 13443 - I changed the nDateFilter == 1 query to use '12/30/1899' as the date instead of NULL.  If you make 2 queries
		here with 2 different types for a field, it's impossible to make the ttx files correctly when trying to edit the report, and so 1 of
		them is always wrong.
		TES 7/26/2004 - PLID 13656 - Made it so the Assignment date version filters out quotes(!)
		TES 9/20/2004 - PLID 14152 - Fixed problem where it was using charge rather than bill date.
		// (j.gruber 2008-07-01 11:00) - PLID 26389 - added last pay info
		TES 7/9/2008 - PLID 29580 - Filtered out applies which happened after the "As Of:" date.
		// (j.gruber 2010-01-28 15:07) - PLID 36899 - filtered out deleted bills
		TES 4/7/2011 - PLID 33741 - Added PlaceOfServiceID, PlaceOfService
		// (j.gruber 2011-12-22 13:39) - PLID 46565 - support payment input date filter
		// (j.luckoski 2013-03-18 14:05) - PLID 55579 - Added Date of service (lineitemt.date)
		// (d.thompson 2014-03-28) - PLID 61446 - Added fields for ICD-10.  Previously this query generated a string
		//	that was ICD9-1, ICD9-2, etc.  I've changed this to just output the 8 fields (ic9 1-4 and icd10 1-4), and
		//	put a formula in the report to do the aggregration of display.
		// (j.gruber 2014-10-16) - PLID 63902 - Added Input date
		// (a.walling 2014-11-17 13:09) - PLID 64145 - join with BillDiagCodes outside the inner unioned subquery
		*/
		if (nDateFilter == 2 || nDateFilter == 3) {//Assignment date
			CString strPayDate;
			if (nDateFilter == 2) {
				strPayDate = "LineItemT.Date";
			}
			else if (nDateFilter == 3) {
				strPayDate = "LineItemT.InputDate";
			}
			// (a.walling 2014-11-17 13:09) - PLID 64145 - join with BillDiagCodes outside the inner unioned subquery
			CString strSql = _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
				"	LineItemT.Description, convert(money,ARInsLineItemsQ.Bal) AS Diff, ARInsLineItemsQ.ProvID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS DocName, "
				"	PersonT.BirthDate, ARInsLineItemsQ.TDate AS TDate, ARInsLineItemsQ.IDate AS IDate, ARInsLineItemsQ.BillDesc, "
				"	PatientsT.UserDefinedID AS UserDefID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
				"	ARInsLineItemsQ.POSID AS PlaceOfServiceID, ARInsLineItemsQ.PlaceOfService, "
				"	ArInsLineItemsQ.PriorAuthNum, ArInsLineItemsQ.Code, "
				"	ICD9T1.CodeNumber as ICD9Code1, \r\n "
				"	ICD9T2.CodeNumber as ICD9Code2, \r\n "
				"	ICD9T3.CodeNumber as ICD9Code3, \r\n "
				"	ICD9T4.CodeNumber as ICD9Code4, \r\n "
				"	ICD10T1.CodeNumber as ICD10Code1, \r\n "
				"	ICD10T2.CodeNumber as ICD10Code2, \r\n "
				"	ICD10T3.CodeNumber as ICD10Code3, \r\n "
				"	ICD10T4.CodeNumber as ICD10Code4, \r\n "
				"   ArInsLineItemsQ.AssignDate AS ADate, "
				" LastPayQ.LastDate as LastPayDate, LastPayQ.Description as LastPayDesc, LastPayQ.RespType as LastPayRespType, LastPayQ.InputDate As LastPayInputDate, LastPayQ.Amount as LastPayAmount, LineItemT.Date as DateOfService "
				" "
				"	FROM "
				"	/*Begin ARLineItemsQ*/ "
				"	(SELECT *  "
				"	FROM  "
				"		(SELECT LineItemT.ID, ChargeRespT.InsuredPartyID, ChargeRespDetailT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) AS Bal, "
				"		BillsT.Date AS TDate, LineItemT.InputDate as IDate, BillsT.ID AS BillID, BillsT.Description AS BillDesc, LineItemT.PatientID, ChargesT.DoctorsProviders AS ProvID,  "
				"		BillsT.PriorAuthNum, CPTCodeT.Code,  "
				"		ChargeRespDetailT.Date AS AssignDate, "
				"		Location_POS.ID AS POSID, Location_POS.Name AS PlaceOfService "
				"		FROM (ChargeRespT LEFT JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID) LEFT OUTER JOIN  "
				"			(SELECT AppliesT.ID, SourceID, DestID, RespID, ApplyDetailsT.Amount, ApplyDetailsT.DetailID, PaymentsT.InsuredPartyID AS InsuredPartyID, LineItemT1.PatientID AS PatID "
				"			FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN (" + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID) INNER JOIN  "
				"			PaymentsT ON AppliesT.SourceID = PaymentsT.ID INNER JOIN  "
				"			LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID ON  "
				"			ChargesT.ID = AppliesT.DestID "
				"			WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) AND (AppliesT.PointsToPayments = 0) AND (LineItemT.Deleted = 0) AND LineItemT1.Date < @ReportDateTo) AS ARInsAppliesQ  "
				"		ON ChargeRespDetailT.ID = ARInsAppliesQ.DetailID LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
				"		LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT Location_POS ON BillsT.Location = Location_POS.ID LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"		WHERE LineItemT.Type = 10 AND (ChargeRespT.InsuredPartyID Is Null OR ChargeRespT.InsuredPartyID = -1) AND LineItemT.Deleted = 0  "
				"		GROUP BY ChargeRespT.ID, ChargeRespDetailT.Amount, LineItemT.ID, ChargeRespT.InsuredPartyID, BillsT.Date, LineItemT.InputDate, BillsT.ID, BillsT.Description, LineItemT.PatientID, ChargesT.DoctorsProviders,  "
				"		BillsT.PriorAuthNum, CPTCodeT.Code, "
				"		ChargeRespDetailT.Date, Location_POS.ID, Location_POS.Name "
				"		HAVING ChargeRespDetailT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) != 0) AS ARInsChargesUnfinishedQ    "
				" "
				"		UNION  "
				"		SELECT * FROM  "
				"			(SELECT LineItemT.ID, PaymentsT.InsuredPartyID, -1*(Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End)+  "
				"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END)) AS Bal, LineItemT.Date AS TDate, LineItemT.InputDate as IDate, -1 AS BillID, '' AS BillDesc, LineItemT.PatientID,  "
				"			PaymentsT.ProviderID AS ProvID, '' AS PriorAuthNum, '' AS Code, "
				+ strPayDate + " AS AssignDate, "
				"			LocationsT.ID AS POSID, LocationsT.Name AS PlaceOfService "
				"			FROM  "
				"				(SELECT AppliesT.SourceID, Sum(ApplyDetailsT.Amount) AS Amount  "
				"				FROM " + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.SourceID = PaymentsT.ID LEFT JOIN ChargeRespDetailT ON ApplyDetailsT.DetailID = ChargeRespDetailT.ID LEFT JOIN LineItemT LineDestT ON AppliesT.DestID = LineDestT.ID "
				"				WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) AND (LineItemT.Date < @ReportDateTo AND (CASE WHEN ChargeRespDetailT.Date IS Null THEN LineDestT.Date ELSE ChargeRespDetailT.Date END) < @ReportDateTo)    "
				"				GROUP BY AppliesT.SourceID) AS AppliesFromQ  "
				"			RIGHT JOIN ( "
				"				(SELECT AppliesT.DestID, Sum(ApplyDetailsT.Amount) AS Amount  "
				"				FROM (" + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID) INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.DestID = PaymentsT.ID INNER JOIN LineItemT LineSourceT ON AppliesT.SourceID = LineSourceT.ID "
				"				WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) AND (LineItemT.Date < @ReportDateTo AND LineSourceT.Date < @ReportDateTo) "
				"				GROUP BY AppliesT.DestID) AS AppliesToQ  "
				"			RIGHT JOIN PaymentsT ON AppliesToQ.DestID = PaymentsT.ID) ON AppliesFromQ.SourceID = PaymentsT.ID  "
				"			INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
				"			LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"			WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) AND LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 0 "
				"			GROUP BY LineItemT.ID, PaymentsT.InsuredPartyID, " + strPayDate + ", LineItemT.Date, LineItemT.InputDate, LineItemT.PatientID, LineItemT.Amount, PaymentsT.ProviderID, LocationsT.ID, LocationsT.Name  "
				"			HAVING Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End) +  "
				"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END) != 0) AS ARInsPaysUnfinishedQ "
				"	) AS ARInsLineItemsQ "
				"	/* End ARInsLineItemsQ */ "
				"	LEFT JOIN PatientsT ON ARInsLineItemsQ.PatientID = PatientsT.PersonID "
				"	LEFT JOIN LineItemT ON ArInsLineItemsQ.ID = LineItemT.ID "
				"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"	LEFT JOIN PersonT PersonProv ON ARInsLineItemsQ.ProvID = PersonProv.ID "
				"	LEFT JOIN BillDiagCodeFlat4V ON ARInsLineItemsQ.BillID = BillDiagCodeFlat4V.BillID AND ARInsLineItemsQ.BillID <> -1 \r\n "
				"	LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
				"	LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
				" LEFT JOIN (SELECT MaxDateQ.LastDate, OtherInfoQ.PatientID, OtherInfoQ.Description, OtherInfoQ.RespType, "
				" OtherInfoQ.InputDate, OtherInfoQ.Amount "
				" FROM  "
				" (SELECT Max(Date) as LastDate, LineItemT.PatientID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" WHERE Type = 1 AND DELETED = 0 "
				" GROUP BY LineItemT.PatientID) AS MaxDateQ "
				" INNER JOIN  "
				" (SELECT Max(LineItemT.ID) as ID, LineItemT.PatientID, LineItemT.Date FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" WHERE Type = 1 AND DELETED = 0 "
				" GROUP BY LineItemT.PatientID, LineItemT.Date) IDQ "
				" ON MaxDateQ.LastDate = IDQ.Date AND MaxDateQ.PatientID = IDQ.PatientID "
				" LEFT JOIN  "
				" (SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.Amount, "
				" CASE WHEN InsuredPartyID IS NULL OR InsuredPartyID = -1 then 'Patient' ELSE InsuranceCoT.Name END AS RespType "
				" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID "
				" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				" WHERE Type = 1 AND DELETED = 0) AS OtherInfoQ "
				" ON IDQ.PatientID = OtherInfoQ.PatientID "
				" AND IDQ.ID = OtherInfoQ.ID) LastPayQ "
				" ON ARInsLineItemsQ.PatientID = LastPayQ.PatientID "
				"");
			dtNext = DateTo + OneDay;
			strSql.Replace("@ReportDateTo", "'" + _Q(FormatDateTimeForSql(dtNext)) + "'");
			return strSql;
		}
		else {
			// (a.walling 2014-11-17 13:09) - PLID 64145 - join with BillDiagCodes outside the inner unioned subquery
			CString strSql = _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
				"	LineItemT.Description, convert(money,ARInsLineItemsQ.Bal) AS Diff, ARInsLineItemsQ.ProvID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS DocName, "
				"	PersonT.BirthDate, ARInsLineItemsQ.TDate AS TDate, ARInsLineItemsQ.IDate AS IDate, ARInsLineItemsQ.BillDesc, "
				"	PatientsT.UserDefinedID AS UserDefID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
				"	ARInsLineItemsQ.POSID AS PlaceOfServiceID, ARInsLineItemsQ.PlaceOfService, "
				"	ArInsLineItemsQ.PriorAuthNum, ArInsLineItemsQ.Code, "
				"	ICD9T1.CodeNumber as ICD9Code1, \r\n "
				"	ICD9T2.CodeNumber as ICD9Code2, \r\n "
				"	ICD9T3.CodeNumber as ICD9Code3, \r\n "
				"	ICD9T4.CodeNumber as ICD9Code4, \r\n "
				"	ICD10T1.CodeNumber as ICD10Code1, \r\n "
				"	ICD10T2.CodeNumber as ICD10Code2, \r\n "
				"	ICD10T3.CodeNumber as ICD10Code3, \r\n "
				"	ICD10T4.CodeNumber as ICD10Code4, \r\n "
				"   convert(datetime, '12/30/1899') AS ADate, "
				" LastPayQ.LastDate as LastPayDate, LastPayQ.Description as LastPayDesc, LastPayQ.RespType as LastPayRespType, LastPayQ.InputDate As LastPayInputDate, LastPayQ.Amount as LastPayAmount, LineItemT.Date as DateOfService "
				" "
				"	FROM "
				"	/*Begin ARLineItemsQ*/ "
				"	(SELECT * "
				"	FROM "
				"		(SELECT LineItemT.ID, ChargeRespT.InsuredPartyID, ChargeRespT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) AS Bal, "
				"		BillsT.Date AS TDate, LineItemT.InputDate as IDate, BillsT.ID AS BillID, BillsT.Description AS BillDesc, LineItemT.PatientID, ChargesT.DoctorsProviders AS ProvID, "
				"		BillsT.PriorAuthNum, CPTCodeT.Code, "
				"		Location_POS.ID AS POSID, Location_POS.Name AS PlaceOfService "
				"		FROM ChargeRespT LEFT OUTER JOIN "
				"			(SELECT AppliesT.ID, AppliesT.SourceID, AppliesT.DestID, AppliesT.RespID, AppliesT.Amount, AppliesT.PointsToPayments, AppliesT.InputDate, AppliesT.InputName, PaymentsT.InsuredPartyID AS InsuredPartyID, LineItemT1.PatientID AS PatID "
				"			FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN " + strAppliesT + " INNER JOIN  "
				"			PaymentsT ON AppliesT.SourceID = PaymentsT.ID INNER JOIN  "
				"			LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID ON  "
				"			ChargesT.ID = AppliesT.DestID "
				"			WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) AND (AppliesT.PointsToPayments = 0) AND (LineItemT.Deleted = 0) AND LineItemT1.@DateFilter < @ReportDateTo) AS ARInsAppliesQ "
				"		ON ChargeRespT.ID = ARInsAppliesQ.RespID LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"		LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT Location_POS ON BillsT.Location = Location_POS.ID LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"		WHERE (ChargeRespT.InsuredPartyID Is Null OR ChargeRespT.InsuredPartyID = -1) AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				"		GROUP BY ChargeRespT.ID, ChargeRespT.Amount, LineItemT.ID, ChargeRespT.InsuredPartyID, BillsT.Date, LineItemT.InputDate, BillsT.ID, BillsT.Description, LineItemT.PatientID, ChargesT.DoctorsProviders, "
				"		BillsT.PriorAuthNum, CPTCodeT.Code, "
				"		Location_POS.ID, Location_POS.Name "
				"		HAVING ChargeRespT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) != 0) AS ARInsChargesUnfinishedQ "
				" "
				"		UNION "
				"		SELECT * FROM "
				"			(SELECT LineItemT.ID, PaymentsT.InsuredPartyID, -1*(Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End)+ "
				"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END)) AS Bal, LineItemT.Date AS TDate, LineItemT.InputDate as IDate, -1 AS BillID, '' AS BillDesc, LineItemT.PatientID, "
				"			PaymentsT.ProviderID AS ProvID, '' AS PriorAuthNum, '' AS Code, "
				"			LocationsT.ID AS POSID, LocationsT.Name AS PlaceOfService "
				"			FROM "
				"				(SELECT AppliesT.SourceID, Sum(AppliesT.Amount) AS Amount  "
				"				FROM " + strAppliesT + " INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.SourceID = PaymentsT.ID INNER JOIN LineItemT LineDestT ON AppliesT.DestID = LineDestT.ID "
				"				LEFT JOIN ChargesT ON LineDestT.ID = ChargesT.ID LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID "
				"				WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) AND (LineItemT.@DateFilter < @ReportDateTo AND CASE WHEN BillsT.@DateFilter Is Null THEN LineDestT.@DateFilter ELSE BillsT.@DateFilter END < @ReportDateTo)   "
				"				GROUP BY AppliesT.SourceID) AS AppliesFromQ "
				"			RIGHT JOIN ( "
				"				(SELECT AppliesT.DestID, Sum(AppliesT.Amount) AS Amount  "
				"				FROM " + strAppliesT + " INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.DestID = PaymentsT.ID INNER JOIN LineItemT LineSourceT ON AppliesT.SourceID = LineSourceT.ID "
				"				WHERE (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) AND (LineItemT.@DateFilter < @ReportDateTo AND LineSourceT.@DateFilter < @ReportDateTo) "
				"				GROUP BY AppliesT.DestID) AS AppliesToQ "
				"			RIGHT JOIN PaymentsT ON AppliesToQ.DestID = PaymentsT.ID) ON AppliesFromQ.SourceID = PaymentsT.ID "
				"			INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"			LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"			WHERE (PaymentsT.InsuredPartyID is null OR PaymentsT.InsuredPartyID = -1) AND LineItemT.Deleted = 0 AND LineItemT.Type <= 3 AND PaymentsT.Prepayment = 0 "
				"			GROUP BY LineItemT.ID, PaymentsT.InsuredPartyID, LineItemT.Date, LineItemT.InputDate, LineItemT.PatientID, LineItemT.Amount, PaymentsT.ProviderID, LocationsT.ID, LocationsT.Name "
				"			HAVING Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End) + "
				"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END) != 0) AS ARInsPaysUnfinishedQ "
				"	) AS ARInsLineItemsQ "
				"	/* End ARInsLineItemsQ */ "
				"	LEFT JOIN PatientsT ON ARInsLineItemsQ.PatientID = PatientsT.PersonID "
				"	LEFT JOIN LineItemT ON ArInsLineItemsQ.ID = LineItemT.ID "
				"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"	LEFT JOIN PersonT PersonProv ON ARInsLineItemsQ.ProvID = PersonProv.ID "
				"	LEFT JOIN BillDiagCodeFlat4V ON ARInsLineItemsQ.BillID = BillDiagCodeFlat4V.BillID AND ARInsLineItemsQ.BillID <> -1 \r\n "
				"	LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
				"	LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
				"	LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
				"	LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
				" LEFT JOIN (SELECT MaxDateQ.LastDate, OtherInfoQ.PatientID, OtherInfoQ.Description, OtherInfoQ.RespType, "
				" OtherInfoQ.InputDate, OtherInfoQ.Amount "
				" FROM  "
				" (SELECT Max(Date) as LastDate, LineItemT.PatientID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" WHERE Type = 1 AND DELETED = 0 "
				" GROUP BY LineItemT.PatientID) AS MaxDateQ "
				" INNER JOIN  "
				" (SELECT Max(LineItemT.ID) as ID, LineItemT.PatientID, LineItemT.Date FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" WHERE Type = 1 AND DELETED = 0 "
				" GROUP BY LineItemT.PatientID, LineItemT.Date) IDQ "
				" ON MaxDateQ.LastDate = IDQ.Date AND MaxDateQ.PatientID = IDQ.PatientID "
				" LEFT JOIN  "
				" (SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.Amount, "
				" CASE WHEN InsuredPartyID IS NULL OR InsuredPartyID = -1 then 'Patient' ELSE InsuranceCoT.Name END AS RespType "
				" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				" LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID "
				" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				" WHERE Type = 1 AND DELETED = 0) AS OtherInfoQ "
				" ON IDQ.PatientID = OtherInfoQ.PatientID "
				" AND IDQ.ID = OtherInfoQ.ID) LastPayQ "
				" ON ARInsLineItemsQ.PatientID = LastPayQ.PatientID "
				"");

			// (j.gruber 2014-10-16) - PLID 63902 - Added IDate field
			if (nDateFilter == 1)
			{
				//date						
				strSql.Replace("@DateFilter", "Date");
			}
			else if (nDateFilter == 4)
			{
				//inputdate						
				strSql.Replace("@DateFilter", "InputDate");
			}

			dtNext = DateTo + OneDay;
			strSql.Replace("@ReportDateTo", "'" + _Q(FormatDateTimeForSql(dtNext)) + "'");
			return strSql;
		}
		break;

	case 391:
		//Aged Receivables Split By Responsibility
		/*	Version History
		DRT 5/2/03 - Very similar to AR by Pat Resp, but without a lot of insured party filtering, and a few extra fields and tables selected.
		TES 6/6/03 - Added ability to filter on assignment date.
		DRT 7/16/03 - This report had 2 UserDefinedID fields.  Due to some current issues with making ttx files, I can't remove the fields and
		fix it the "proper" way, so I renamed the 2nd one to UserDefID.  TODO:  We need to take this out in the future, so if you're
		making a new ttx file, please remove it.
		TES 9/24/03: Please note that the .rpt files have formulas that depend on patient charges having an InsName field equal to "<Pat Resp>", so don't change that field unless you're willing to go through all the .rpt files.
		DRT 4/22/2004 - PLID 11816 - Fixed some bad select *'s
		DRT 7/19/2004 - PLID 13443 - I changed the nDateFilter == 1 query to use '12/30/1899' as the date instead of NULL.  If you make 2 queries
		here with 2 different types for a field, it's impossible to make the ttx files correctly when trying to edit the report, and so 1 of
		them is always wrong.
		TES 7/27/2004 - PLID 13669 - Assignment date was duplicating payment amounts when applied to a charge with multiple
		ChargeRespDetails
		TES 9/20/2004 - PLID 14152 - Fixed problem where it was using charge rather than bill date.
		TES 5/2/2005 - PLID 16118 - Added the billing notes.
		TES 5/2/2005 - PLID 16269 - Made the .ttx files manual because the Notes were coming over as String rather than Memo.
		// (j.gruber 2008-07-01 11:01) - PLID 26389 - added last pay info fields
		TES 7/9/2008 - PLID 29580 - Filtered out applies which happened after the "As Of:" date.
		(d.thompson 2009-03-18) - PLID 33171 - Added resp type fields
		// (j.gruber 2010-01-28 15:07) - PLID 36899 - filtered out deleted bills
		TES 4/7/2011 - PLID 33741 - Added PlaceOfServiceID, PlaceOfService
		// (j.gruber 2011-12-16 15:47) - PLID 36465 - made a assignment date / pay input date possibility
		// (d.thompson 2014-03-31) - PLID 61447 - Added ICD-10 fields
		// (j.gruber 2014-10-16) - PLID 63900 - Added Input Date
		*/
		switch (nSubLevel) {
		case 1:
			switch (nSubRepNum) {
			case 1://Line Item Notes.
				return _T("SELECT LineItemID AS LineID, Date, convert(ntext,Note) AS Note FROM Notes WHERE LineItemID Is Not Null");
				break;
			default://Bill notes.
				return _T("SELECT BillID, Date, convert(ntext,Note) AS Note FROM Notes WHERE BillID Is Not Null");
				break;
			}
			break;
		default:
			// (j.gruber 2011-12-16 15:48) - PLID 36465 - 2 assingment dates for charges now
			if (nDateFilter == 2 || nDateFilter == 3) {//Assignment date

				CString strPayDate;
				if (nDateFilter == 2) {
					strPayDate = "LineItemT.Date";
				}
				else if (nDateFilter == 3) {
					strPayDate = "LineItemT.InputDate";
				}

				// (a.walling 2014-11-17 13:09) - PLID 64145 - join with BillDiagCodes outside the inner unioned subquery
				CString strSql = _T("SELECT ARInsLineItemsQ.ID AS LineID, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  \r\n"
					"	LineItemT.Description, convert(money,ARInsLineItemsQ.Bal) AS Diff, ARInsLineItemsQ.ProvID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS DocName,  \r\n"
					"	PersonT.BirthDate, ARInsLineItemsQ.TDate AS TDate, ARInsLineItemsQ.IDate AS IDate, ARInsLineItemsQ.BillID, ARInsLineItemsQ.BillDesc,  \r\n"
					"	PatientsT.UserDefinedID AS UserDefID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  \r\n"
					"	ArInsLineItemsQ.POSID AS PlaceOfServiceID, ArInsLineItemsQ.PlaceOfService, \r\n"
					"	ArInsLineItemsQ.PriorAuthNum, ArInsLineItemsQ.Code, "
					"	ICD9T1.CodeNumber as ICD9Code1, \r\n "
					"	ICD9T2.CodeNumber as ICD9Code2, \r\n "
					"	ICD9T3.CodeNumber as ICD9Code3, \r\n "
					"	ICD9T4.CodeNumber as ICD9Code4, \r\n "
					"	ICD10T1.CodeNumber as ICD10Code1, \r\n "
					"	ICD10T2.CodeNumber as ICD10Code2, \r\n "
					"	ICD10T3.CodeNumber as ICD10Code3, \r\n "
					"	ICD10T4.CodeNumber as ICD10Code4, \r\n "
					"	CASE WHEN InsuranceCoT.PersonID IS NULL THEN '<Patient Resp>' ELSE InsuranceCoT.Name END AS InsName, \r\n"
					"   ArInsLineItemsQ.AssignDate AS ADate, BillNoteID, LineNoteID, \r\n"
					" LastPayQ.LastDate as LastPayDate, LastPayQ.Description as LastPayDesc, LastPayQ.RespType as LastPayRespType, LastPayQ.InputDate As LastPayInputDate, LastPayQ.Amount as LastPayAmount, \r\n"
					"   RespTypeT.Priority AS RespTypePriority, RespTypeT.TypeName AS RespTypeName \r\n"
					"  \r\n"
					"	FROM  \r\n"
					"	/*Begin ARInsLineItemsQ*/  \r\n"
					"	(SELECT *  \r\n"
					"	FROM  \r\n"
					"		(SELECT LineItemT.ID, ChargeRespT.InsuredPartyID, ChargeRespDetailT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) AS Bal, \r\n"
					"		BillsT.Date AS TDate, LineItemT.InputDate as IDate, BillsT.ID AS BillID, BillsT.Description AS BillDesc, LineItemT.PatientID, ChargesT.DoctorsProviders AS ProvID,  \r\n"
					"		BillsT.PriorAuthNum, CPTCodeT.Code,  \r\n"
					"		ChargeRespDetailT.Date AS AssignDate,  \r\n"
					"		(SELECT TOP 1 ID FROM Notes WHERE BillID = BillsT.ID) AS BillNoteID, (SELECT TOP 1 ID FROM Notes WHERE LineItemID = LineItemT.ID) AS LineNoteID, \r\n"
					"		Location_POS.ID AS POSID, Location_POS.Name AS PlaceOfService \r\n"
					"		FROM (ChargeRespT LEFT JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID) LEFT OUTER JOIN  \r\n"
					"			(SELECT AppliesT.ID, SourceID, DestID, RespID, ApplyDetailsT.Amount, ApplyDetailsT.DetailID, PaymentsT.InsuredPartyID AS InsuredPartyID, LineItemT1.PatientID AS PatID \r\n"
					"			FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN (" + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID) INNER JOIN  \r\n"
					"			PaymentsT ON AppliesT.SourceID = PaymentsT.ID INNER JOIN  \r\n"
					"			LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID ON  \r\n"
					"			ChargesT.ID = AppliesT.DestID \r\n"
					"			WHERE (AppliesT.PointsToPayments = 0) AND (LineItemT.Deleted = 0) AND LineItemT1.Date < @ReportDateTo) AS ARInsAppliesQ  \r\n"
					"		ON ChargeRespDetailT.ID = ARInsAppliesQ.DetailID LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  \r\n"
					"		LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT Location_POS ON BillsT.Location = Location_POS.ID LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID \r\n"
					"		WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND LineItemT.Type = 10 \r\n"
					"		GROUP BY ChargeRespT.ID, ChargeRespDetailT.Amount, LineItemT.ID, ChargeRespT.InsuredPartyID, BillsT.Date, LineItemT.InputDate, BillsT.ID, BillsT.Description, LineItemT.PatientID, ChargesT.DoctorsProviders,  \r\n"
					"		BillsT.PriorAuthNum, CPTCodeT.Code, "
					"		ChargeRespDetailT.Date, Location_POS.ID, Location_POS.Name \r\n"
					"		HAVING ChargeRespDetailT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) != 0) AS ARInsChargesUnfinishedQ    \r\n"
					" \r\n"
					"		UNION  \r\n"
					"		SELECT * FROM  \r\n"
					"			(SELECT LineItemT.ID, PaymentsT.InsuredPartyID, -1*(Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End)+  \r\n"
					"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END)) AS Bal, LineItemT.Date AS TDate, LineItemT.InputDate as IDate, -1 AS BillID, '' AS BillDesc, LineItemT.PatientID,  \r\n"
					"			PaymentsT.ProviderID AS ProvID, '' AS PriorAuthNum, '' AS Code, "
					"			" + strPayDate + " AS AssignDate, \r\n"
					"			NULL AS BillNoteID, (SELECT TOP 1 ID FROM Notes WHERE LineItemID = LineItemT.ID) AS LineNoteID, \r\n"
					"			LocationsT.ID AS POSID, LocationsT.Name AS PlaceOfService \r\n"
					"			FROM  \r\n"
					"				(SELECT AppliesT.SourceID, Sum(ApplyDetailsT.Amount) AS Amount  \r\n"
					"				FROM " + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.SourceID = PaymentsT.ID LEFT JOIN ChargeRespDetailT ON ApplyDetailsT.DetailID = ChargeRespDetailT.ID LEFT JOIN LineItemT LineDestT ON AppliesT.DestID = LineDestT.ID \r\n"
					"				WHERE (LineItemT.Date < @ReportDateTo AND (CASE WHEN ChargeRespDetailT.Date IS Null THEN LineDestT.Date ELSE ChargeRespDetailT.Date END) < @ReportDateTo)    \r\n"
					"				GROUP BY AppliesT.SourceID) AS AppliesFromQ  \r\n"
					"			RIGHT JOIN ( \r\n"
					"				(SELECT AppliesT.DestID, Sum(ApplyDetailsT.Amount) AS Amount  \r\n"
					"				FROM (" + strAppliesT + " LEFT JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID) INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.DestID = PaymentsT.ID INNER JOIN LineItemT LineSourceT ON AppliesT.SourceID = LineSourceT.ID \r\n"
					"				WHERE (LineItemT.Date < @ReportDateTo AND LineSourceT.Date < @ReportDateTo) \r\n"
					"				GROUP BY AppliesT.DestID) AS AppliesToQ  \r\n"
					"			RIGHT JOIN PaymentsT ON AppliesToQ.DestID = PaymentsT.ID) ON AppliesFromQ.SourceID = PaymentsT.ID  \r\n"
					"			INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  \r\n"
					"			LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n"
					"			WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 0 \r\n"
					"			GROUP BY LineItemT.ID, PaymentsT.InsuredPartyID, " + strPayDate + ", LineItemT.Date, LineItemT.InputDate, LineItemT.PatientID, LineItemT.Amount, PaymentsT.ProviderID, LocationsT.ID, LocationsT.Name \r\n"
					"			HAVING Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End) +  \r\n"
					"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END) != 0) AS ARInsPaysUnfinishedQ \r\n"
					"	) AS ARInsLineItemsQ  \r\n"
					"	/* End ARInsLineItemsQ */  \r\n"
					"	LEFT JOIN PatientsT ON ARInsLineItemsQ.PatientID = PatientsT.PersonID  \r\n"
					"	LEFT JOIN LineItemT ON ArInsLineItemsQ.ID = LineItemT.ID  \r\n"
					"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  \r\n"
					"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  \r\n"
					"	LEFT JOIN PersonT PersonProv ON ARInsLineItemsQ.ProvID = PersonProv.ID \r\n"
					"	LEFT JOIN InsuredPartyT ON ARInsLineItemsQ.InsuredPartyID = InsuredPartyT.PersonID \r\n"
					"   LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID \r\n"
					"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID \r\n"
					"	LEFT JOIN BillDiagCodeFlat4V ON ARInsLineItemsQ.BillID = BillDiagCodeFlat4V.BillID AND ARInsLineItemsQ.BillID <> -1 \r\n "
					"	LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
					"	LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
					" LEFT JOIN (SELECT MaxDateQ.LastDate, OtherInfoQ.PatientID, OtherInfoQ.Description, OtherInfoQ.RespType, \r\n"
					" OtherInfoQ.InputDate, OtherInfoQ.Amount \r\n"
					" FROM  \r\n"
					" (SELECT Max(Date) as LastDate, LineItemT.PatientID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID \r\n"
					" WHERE Type = 1 AND DELETED = 0 \r\n"
					" GROUP BY LineItemT.PatientID) AS MaxDateQ \r\n"
					" INNER JOIN  \r\n"
					" (SELECT Max(LineItemT.ID) as ID, LineItemT.PatientID, LineItemT.Date FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID \r\n"
					" WHERE Type = 1 AND DELETED = 0 \r\n"
					" GROUP BY LineItemT.PatientID, LineItemT.Date) IDQ \r\n"
					" ON MaxDateQ.LastDate = IDQ.Date AND MaxDateQ.PatientID = IDQ.PatientID \r\n"
					" LEFT JOIN  \r\n"
					" (SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.Amount, \r\n"
					" CASE WHEN InsuredPartyID IS NULL OR InsuredPartyID = -1 then 'Patient' ELSE InsuranceCoT.Name END AS RespType \r\n"
					" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID \r\n"
					" LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID \r\n"
					" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID \r\n"
					" WHERE Type = 1 AND DELETED = 0) AS OtherInfoQ \r\n"
					" ON IDQ.PatientID = OtherInfoQ.PatientID \r\n"
					" AND IDQ.ID = OtherInfoQ.ID) LastPayQ \r\n"
					" ON ARInsLineItemsQ.PatientID = LastPayQ.PatientID ");
				dtNext = DateTo + OneDay;
				strSql.Replace("@ReportDateTo", "'" + _Q(FormatDateTimeForSql(dtNext)) + "'");
				return strSql;
			}
			else {
				// (a.walling 2014-11-17 13:09) - PLID 64145 - join with BillDiagCodes outside the inner unioned subquery
				CString strSql = _T("SELECT ARInsLineItemsQ.ID AS LineID, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
					"	LineItemT.Description, convert(money,ARInsLineItemsQ.Bal) AS Diff, ARInsLineItemsQ.ProvID AS ProvID, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS DocName,  "
					"	PersonT.BirthDate, ARInsLineItemsQ.TDate AS TDate, ARInsLineItemsQ.IDate AS IDate, ARInsLineItemsQ.BillID, ARInsLineItemsQ.BillDesc,  "
					"	PatientsT.UserDefinedID AS UserDefID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  "
					"	ARInsLineItemsQ.POSID AS PlaceOfServiceID, ARInsLineItemsQ.PlaceOfService, "
					"	ArInsLineItemsQ.PriorAuthNum, ArInsLineItemsQ.Code, "
					"	ICD9T1.CodeNumber as ICD9Code1, \r\n "
					"	ICD9T2.CodeNumber as ICD9Code2, \r\n "
					"	ICD9T3.CodeNumber as ICD9Code3, \r\n "
					"	ICD9T4.CodeNumber as ICD9Code4, \r\n "
					"	ICD10T1.CodeNumber as ICD10Code1, \r\n "
					"	ICD10T2.CodeNumber as ICD10Code2, \r\n "
					"	ICD10T3.CodeNumber as ICD10Code3, \r\n "
					"	ICD10T4.CodeNumber as ICD10Code4, \r\n "
					"	CASE WHEN InsuranceCoT.PersonID IS NULL THEN '<Patient Resp>' ELSE InsuranceCoT.Name END AS InsName, "
					"   convert(datetime, '12/30/1899') AS ADate, BillNoteID, LineNoteID, "
					" LastPayQ.LastDate as LastPayDate, LastPayQ.Description as LastPayDesc, LastPayQ.RespType as LastPayRespType, LastPayQ.InputDate As LastPayInputDate, LastPayQ.Amount as LastPayAmount, "
					"   RespTypeT.Priority AS RespTypePriority, RespTypeT.TypeName AS RespTypeName "
					"	FROM  "
					"	/*Begin ARInsLineItemsQ*/  "
					"	(SELECT *  "
					"	FROM  "
					"		(SELECT LineItemT.ID, ChargeRespT.InsuredPartyID, ChargeRespT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) AS Bal,  "
					"		BillsT.Date AS TDate, LineItemT.InputDate as IDate, BillsT.ID AS BillID, BillsT.Description AS BillDesc, LineItemT.PatientID, ChargesT.DoctorsProviders AS ProvID,  "
					"		BillsT.PriorAuthNum, CPTCodeT.Code,  "
					"		(SELECT TOP 1 ID FROM Notes WHERE BillID = BillsT.ID) AS BillNoteID, (SELECT TOP 1 ID FROM Notes WHERE LineItemID = LineItemT.ID) AS LineNoteID, "
					"		Location_POS.ID AS POSID, Location_POS.Name AS PlaceOfService "
					"		FROM ChargeRespT LEFT OUTER JOIN  "
					"			(SELECT AppliesT.ID, AppliesT.SourceID, AppliesT.DestID, AppliesT.RespID, AppliesT.Amount, AppliesT.PointsToPayments, AppliesT.InputDate, AppliesT.InputName, PaymentsT.InsuredPartyID AS InsuredPartyID, LineItemT1.PatientID AS PatID "
					"			FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN " + strAppliesT + " INNER JOIN  "
					"			PaymentsT ON AppliesT.SourceID = PaymentsT.ID INNER JOIN  "
					"			LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID ON  "
					"			ChargesT.ID = AppliesT.DestID "
					"			WHERE (AppliesT.PointsToPayments = 0) AND (LineItemT.Deleted = 0) AND LineItemT1.@DateFilter < @ReportDateTo) AS ARInsAppliesQ  "
					"		ON ChargeRespT.ID = ARInsAppliesQ.RespID LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
					"		LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID LEFT JOIN LocationsT Location_POS ON BillsT.Location = Location_POS.ID LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID  "
					"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10  "
					"		GROUP BY ChargeRespT.ID, ChargeRespT.Amount, LineItemT.ID, ChargeRespT.InsuredPartyID, BillsT.Date, LineItemT.InputDate, BillsT.ID, BillsT.Description, LineItemT.PatientID, ChargesT.DoctorsProviders,  "
					"		BillsT.PriorAuthNum, CPTCodeT.Code, "
					"		Location_POS.ID, Location_POS.Name  "
					"		HAVING ChargeRespT.Amount - Sum(CASE WHEN ARInsAppliesQ.Amount IS Null THEN 0 ELSE ARInsAppliesQ.Amount End) != 0 "
					"		) AS ARInsChargesUnfinishedQ  "
					" "
					"		UNION  "
					"		SELECT * FROM  "
					"			(SELECT LineItemT.ID, PaymentsT.InsuredPartyID, -1*(Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End)+  "
					"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END)) AS Bal, LineItemT.Date AS TDate, LineItemT.InputDate as IDate, -1 AS BillID, '' AS BillDesc, LineItemT.PatientID,  "
					"			PaymentsT.ProviderID AS ProvID, '' AS PriorAuthNum, '' AS Code, "
					"			NULL AS BillNoteID, (SELECT TOP 1 ID FROM Notes WHERE LineItemID = LineItemT.ID) AS LineNoteID, "
					"			LocationsT.ID AS POSID, LocationsT.Name AS PlaceOfService "
					"			FROM  "
					"				(SELECT AppliesT.SourceID, Sum(AppliesT.Amount) AS Amount  "
					"				FROM " + strAppliesT + " INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.SourceID = PaymentsT.ID INNER JOIN LineItemT LineDestT ON AppliesT.DestID = LineDestT.ID "
					"				LEFT JOIN ChargesT ON LineDestT.ID = ChargesT.ID LEFT JOIN (SELECT * FROM BillsT WHERE Deleted = 0) BillsT ON ChargesT.BillID = BillsT.ID "
					"				WHERE (LineItemT.@DateFilter < @ReportDateTo AND CASE WHEN BillsT.Date Is Null THEN LineDestT.@DateFilter ELSE BillsT.@DateFilter END < @ReportDateTo)   "
					"				GROUP BY AppliesT.SourceID) AS AppliesFromQ  "
					"			RIGHT JOIN ( "
					"				(SELECT AppliesT.DestID, Sum(AppliesT.Amount) AS Amount  "
					"				FROM " + strAppliesT + " INNER JOIN (PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID) ON AppliesT.DestID = PaymentsT.ID INNER JOIN LineItemT LineSourceT ON AppliesT.SourceID = LineSourceT.ID "
					"				WHERE (LineItemT.@DateFilter < @ReportDateTo AND LineSourceT.@DateFilter < @ReportDateTo) "
					"				GROUP BY AppliesT.DestID) AS AppliesToQ  "
					"			RIGHT JOIN PaymentsT ON AppliesToQ.DestID = PaymentsT.ID) ON AppliesFromQ.SourceID = PaymentsT.ID  "
					"			INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
					"			LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
					"			WHERE LineItemT.Deleted = 0 AND LineItemT.Type <= 3 AND PaymentsT.Prepayment = 0  "
					"			GROUP BY LineItemT.ID, PaymentsT.InsuredPartyID, LineItemT.Date, LineItemT.InputDate, LineItemT.PatientID, LineItemT.Amount, PaymentsT.ProviderID, LocationsT.ID, LocationsT.Name  "
					"			HAVING Min(LineItemT.Amount) - Sum(CASE WHEN AppliesFromQ.Amount Is Null THEN 0 ELSE AppliesFromQ.Amount End) +  "
					"			Sum(CASE WHEN AppliesToQ.Amount Is Null THEN 0 ELSE AppliesToQ.Amount END) != 0 "
					"			) AS ARInsPaysUnfinishedQ  "
					"	) AS ARInsLineItemsQ  "
					"	/* End ARInsLineItemsQ */  "
					"	LEFT JOIN PatientsT ON ARInsLineItemsQ.PatientID = PatientsT.PersonID  "
					"	LEFT JOIN LineItemT ON ArInsLineItemsQ.ID = LineItemT.ID  "
					"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID  "
					"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
					"	LEFT JOIN PersonT PersonProv ON ARInsLineItemsQ.ProvID = PersonProv.ID "
					"	LEFT JOIN InsuredPartyT ON ARInsLineItemsQ.InsuredPartyID = InsuredPartyT.PersonID "
					"   LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"	LEFT JOIN BillDiagCodeFlat4V ON ARInsLineItemsQ.BillID = BillDiagCodeFlat4V.BillID AND ARInsLineItemsQ.BillID <> -1 \r\n "
					"	LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
					"	LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
					"	LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
					"	LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "
					" LEFT JOIN (SELECT MaxDateQ.LastDate, OtherInfoQ.PatientID, OtherInfoQ.Description, OtherInfoQ.RespType, "
					" OtherInfoQ.InputDate, OtherInfoQ.Amount "
					" FROM  "
					" (SELECT Max(Date) as LastDate, LineItemT.PatientID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" WHERE Type = 1 AND DELETED = 0 "
					" GROUP BY LineItemT.PatientID) AS MaxDateQ "
					" INNER JOIN  "
					" (SELECT Max(LineItemT.ID) as ID, LineItemT.PatientID, LineItemT.Date FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" WHERE Type = 1 AND DELETED = 0 "
					" GROUP BY LineItemT.PatientID, LineItemT.Date) IDQ "
					" ON MaxDateQ.LastDate = IDQ.Date AND MaxDateQ.PatientID = IDQ.PatientID "
					" LEFT JOIN  "
					" (SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.Amount, "
					" CASE WHEN InsuredPartyID IS NULL OR InsuredPartyID = -1 then 'Patient' ELSE InsuranceCoT.Name END AS RespType "
					" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					" LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID "
					" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					" WHERE Type = 1 AND DELETED = 0) AS OtherInfoQ "
					" ON IDQ.PatientID = OtherInfoQ.PatientID "
					" AND IDQ.ID = OtherInfoQ.ID) LastPayQ "
					" ON ARInsLineItemsQ.PatientID = LastPayQ.PatientID "
					);

				// (j.gruber 2014-10-16) - PLID 63900 - Added IDate field
				if (nDateFilter == 1)
				{
					//date						
					strSql.Replace("@DateFilter", "Date");
				}
				else if (nDateFilter == 4)
				{
					//inputdate						
					strSql.Replace("@DateFilter", "InputDate");
				}

				dtNext = DateTo + OneDay;
				strSql.Replace("@ReportDateTo", "'" + _Q(FormatDateTimeForSql(dtNext)) + "'");
				return strSql;
			}
		}

	default:
		return _T("");
		break;
	}
}

// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, return a CComplexReportQuery object
CComplexReportQuery CReportInfo::GetSqlStatement(ADODB::_ConnectionPtr pConnection, long nSubLevel, long nSubRepNum) const
{
	CReportInfo *pReport = (CReportInfo*)this;

	CStatementSqlBuilder builder(pConnection, pReport, nSubLevel, nSubRepNum, FALSE);
	return builder.GetStatementSql();
}

//TES 2/18/04 - Reached the freakin' compiler heap limit again, once again we need to break out into new function.
CString CReportInfo::GetSqlFinancialActivity(long nSubLevel, long nSubRepNum) const
{
	// (c.haag 2016-03-10) - PLID 68565 - Calculate the filters regardless of report so that they can all be passed into 
	// AllFinancialQ  for more efficient query generation. Notice we're not filtering on report ID here; this is because I don't
	// want developers to have to change the filter when adding a new report. They almost surely won't realize that they have
	// to; and when their report is released, it will be slow if it uses AllFinancialQ.
	// (c.haag 2016-03-14) - PLID 68592 - Deprecated strUserNameFilter because it's misleading and now handled in GetAllFinancialQ
	CString strLocFilter, strProvFilter, strPatientFilter, strBillDateFilter, strServiceDateFilter, strInputDateFilter;// , strUserNameFilter;

	//(e.lally 2008-10-10) PLID 31421 - Support filtering on multiple locations
	if (nLocation > 0) {
		strLocFilter.Format(" LocID = %li ", nLocation);
	}
	else if (nLocation == -2) {
		strLocFilter = " LocID IS NULL ";
	}
	else if (nLocation == -3) {
		strLocFilter.Format(" LocID IN (");
		CString strPart;
		for (int i = 0; i < m_dwLocations.GetSize(); i++) {
			strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
			strLocFilter += strPart;
		}
		strLocFilter = strLocFilter.Left(strLocFilter.GetLength() - 2) + ") ";
	}

	if (nProvider != -1) {

		for (int i = 0; i < m_dwProviders.GetSize(); i++) {
			strProvFilter += AsString((long)m_dwProviders.GetAt(i)) + ",";
		}

		//take off the comma
		strProvFilter.TrimRight(",");
	}

	//Patient will include the patientID,  and any filters or groups
	if (nPatient != -1) {
		strPatientFilter.Format(" PatientID = %li ", nPatient);

		if (bUseFilter) {
			CString strTemp = strFilterString;
			strTemp.Replace("DailyFinancialQ.PatID", "LineItemT.PatientID");

			CString strTemp2;
			strTemp2.Format("(%s) AND (%s)", strPatientFilter, strTemp);
			strPatientFilter = strTemp2;
		}
		else if (bUseGroup) {
			CString strTemp = strGroupFilter;
			strTemp.Replace("DailyFinancialQ.PatID", "LineItemT.PatientID");

			CString strTemp2;
			strTemp2.Format("((%s) AND (%s))", strPatientFilter, strTemp);
			strPatientFilter = strTemp2;
		}
	}
	else {

		if (bUseFilter) {
			CString strTemp = strFilterString;
			strTemp.Replace("DailyFinancialQ.PatID", "LineItemT.PatientID");

			strPatientFilter.Format("(%s)", strTemp);

		}
		else if (bUseGroup) {
			CString strTemp = strGroupFilter;
			strTemp.Replace("DailyFinancialQ.PatID", "LineItemT.PatientID");

			strPatientFilter.Format("(%s)", strTemp);
		}
	}

	//DateFilters
	if (nDateRange != -1) {
		if (strDateFilterField == "BDate") {
			strBillDateFilter.Format("( BDate  >= '%s' AND  BDate < DateAdd(day, 1, '%s'))", FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo));
		}
		else if (strDateFilterField == "TDate") {
			strServiceDateFilter.Format("( TDate  >= '%s' AND  TDate < DateAdd(day, 1, '%s'))", FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo));
		}
		else if (strDateFilterField == "IDate") {
			strInputDateFilter.Format("( IDate  >= '%s' AND  IDate < DateAdd(day, 1, '%s'))", FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo));
		}
	}

	//external filter
	// (c.haag 2016-03-14) - PLID 68592 - Deprecated this nonsense code because strUserNameFilter is just the
	// external filter if "{DailyFinancialQ.InputName} IN" is absent.
	//strUserNameFilter = strExternalFilter;
	//strUserNameFilter.Replace("{DailyFinancialQ.InputName} IN", "");

	switch (nID) {
	case 603: // Financial Activity - Today's Service Date
			  /*  Version History
			  (d.moore 2007-07-25) - PLID 22487 - Same content as 'Financial Activity - Daily' but the report is
			  limited to just the current date.
			  (e.lally 2007-08-03) PLID 25116 - Replaced refunds with a breakdown of refunds as cash, check, or credit
			  */
	case 719:  // (j.dinatale 2011-11-08 11:13) - PLID 45658 - Financial Activity - Daily for logged in user
	case 153:
	{
		//Financial Activity - Daily
		/*	Version History
		1/30/03 - DRT - Made the report editable.
		1/24/03 - DRT - Made a significant change to the way we show prepayments (again).  Now this report shows (in the prepayments field), ALL prepayments,
		even if they have been applied or had things applied to them.  Then we calculate another field of that info (the applies to/from prepays), and
		add that in.  So the final calculation of Net Patient Receivables is now:  net pays + adjustments - prepays + applied prepays = receivables.
		This change is also being applied to all reports with AllFinancialQ (the ___ Financial Activity series)
		2/27/04 - TES - Changed it so that it uses the charge location for applied payments, just like the provider.
		DRT 4/8/2004 - PLID 11816 - Replaced a LineItemT.* with appropriate fields
		TES 8/26/2004 - Used the #define for AllFinancialQ
		TES 5/16/2005 - AllFinancialQ now uses Amount for the Prepayments Received field.
		DRT 5/19/2005 - Removed LocationID field, it was unnecessary and confusing, and some reports were filtering on it
		improperly.
		(e.lally 2007-08-03) PLID 25116 - Replaced refunds with a breakdown of refunds as cash, check, or credit
		// (j.gruber 2007-08-29 09:36) - PLID 25190 - added payments applied field for calidora
		(d.moore 2007-09-18) - PLID 25166 - Added fields for discounts and discount categories.
		// (j.gruber 2008-02-22 18:03) - PLID 29166 - added inline filters in daily financial activity
		(d.thompson 2009-03-04) - PLID 33119 - Added a GROUP BY element of LineID to the DFSQ subquery.  Our previous
		grouping did not ensure uniqueness, and it was possible to filter out legitimate records, if you had 100%
		identical (including to-the-second dates) records.
		// (j.gruber 2009-04-01 12:33) - PLID 33358 - update discount structure
		//	(c.haag 2010-01-19 17:07) - PLID 36643 - Added APPTOPREPAY (Which equals prepayments minus line items applied to
		// those prepayments)
		(e.lally 2011-11-10) PLID 45425 - Added the linked claim provider for the ProvID
		(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
		// (d.thompson 2013-06-13) - PLID 56360 - Moved InsResp (which already existed in the deepest bowels of AllFinancialQ) and exposed it to the top level of the reports.
		// (j.gruber 2014-03-10 12:26) - PLID 58526 - fix discounts to only show per bill, expose fintype, and ChargeAmtBeforeDiscount fields, also added \r\ns
		*/

		// (r.gonet 2015-05-05 14:38) - PLID 65904 - Added Gift Certificate Refunds
		CString str1 = "SELECT DFSQ.FullName, DFSQ.UserDefinedID, DFSQ.PatID AS PatID, DFSQ.BDate AS BDate, DFSQ.TDate AS TDate, \r\n "
			"DFSQ.IDate AS IDate, DFSQ.DocName, DFSQ.ProvID AS ProvID, DFSQ.ADJUSTMENT, DFSQ.BILL AS BILL, \r\n "
			"DFSQ.CASH, DFSQ.CHECKAMT AS 'CHECK', DFSQ.CREDIT, DFSQ.PrePayment, DFSQ.APPPREPAY, DFSQ.APPTOPREPAY, \r\n "
			"DFSQ.RefundCash, DFSQ.RefundCheck, DFSQ.RefundCredit, DFSQ.RefundGC, DFSQ.GCSOLD, DFSQ.GCREDEEMED, DFSQ.PaysApplied, \r\n "
			"DFSQ.AccountBal, DFSQ.LocID AS LocID, DFSQ.Location, @SUPPRESS@ AS SUPPRESS, DFSQ.InputName AS InputName, \r\n "
			"DFSQ.TotalDiscount, DFSQ.PercentDiscount, DFSQ.DollarDiscount, DFSQ.DiscountCategoryDescription, \r\n "
			"DFSQ.ClaimProviderID, DFSQ.ClaimProvider, DFSQ.InsResp, DFSQ.FinType, DFSQ.ChargeAmtBeforeDiscount \r\n "
			"\r\n "
			"FROM \r\n "
			"(SELECT  DailyFinancialSubQ.FullName,  \r\n "
			"PatientsT.UserDefinedID,  \r\n "
			"PatientsT.PersonID AS PatID,  \r\n "
			"DailyFinancialSubQ.BDate AS BDate,  \r\n "
			"DailyFinancialSubQ.TDate AS TDate,  \r\n "
			"DailyFinancialSubQ.IDate AS IDate,  \r\n "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  \r\n "
			"DailyFinancialSubQ.ProvID AS ProvID,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'ADJUSTMENT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS ADJUSTMENT,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS BILL,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CASH' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CASH,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CHECK' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CHECKAMT, \r\n  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CREDIT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CREDIT,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType =  'PrePayment' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS PREPAYMENT,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType =  'APPPRE' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS APPPREPAY,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCash' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCash,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCheck' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCheck,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCredit' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCredit,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundGC' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundGC,  \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCSOLD' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCSOLD, \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCREDEEMED' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCREDEEMED, \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'APPLIEDPAYS' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS PaysApplied, \r\n "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'APPTOPRE' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS APPTOPREPAY, \r\n "
			"convert(money, PatBalQ.AccountBal) AS AccountBal,  \r\n "
			"DailyFinancialSubQ.LocID,  \r\n "
			"LocationsT.Name AS Location, DailyFinancialSubQ.InputName,  \r\n "
			" CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.TotalDiscount ELSE 0 END AS TotalDiscount,  \r\n "
			" CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.PercentDiscount ELSE 0 END AS PercentDiscount , \r\n "
			" CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.DollarDiscount ELSE 0 END AS DollarDiscount, \r\n "
			" CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.DiscountCategoryDescription ELSE '' END AS DiscountCategoryDescription, \r\n "
			" ClaimProviderPersonT.ID AS ClaimProviderID, ClaimProviderPersonT.Last + ', ' + ClaimProviderPersonT.First + ' ' + ClaimProviderPersonT.Middle AS ClaimProvider, DailyFinancialSubQ.InsResp, \r\n "
			" DailyFinancialSubQ.FinType, \r\n "
			" CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.ChargeAmtBeforeDiscount  ELSE 0 END AS ChargeAmtBeforeDiscount  \r\n "
			"FROM ( \r\n "
			"/* Patient Balance */   \r\n "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     \r\n "
			"Payments = CASE    \r\n "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null    \r\n "
			"	THEN 0    \r\n "
			"	ELSE [_PatPaysQ].[SumOfAmount]    \r\n "
			"	End,     \r\n "
			"Charges = CASE  \r\n  "
			"	WHEN [ChargeAmount] Is Null    \r\n "
			"	THEN 0    \r\n "
			"	ELSE [ChargeAmount]    \r\n "
			"	End,     \r\n "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,     \r\n "
			"PatientsT.PersonID AS ID,     \r\n "
			"(CASE    \r\n "
			"	WHEN [ChargeAmount] Is Null    \r\n "
			"	THEN 0    \r\n "
			"	ELSE [ChargeAmount]    \r\n "
			"	End)-    \r\n "
			"(CASE    \r\n "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null    \r\n "
			"	THEN 0    \r\n "
			"	ELSE [_PatPaysQ].[SumOfAmount]    \r\n "
			"	End)+    \r\n "
			"(CASE    \r\n "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null    \r\n "
			"	THEN 0    \r\n "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]    \r\n "
			"	End) AS AccountBal,     \r\n "
			"ProvidersT.PersonID AS ProvID,     \r\n "
			"PrePayments = CASE    \r\n "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null    \r\n "
			"	THEN 0    \r\n "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]    \r\n "
			"	End    \r\n ";


		if (bExtended && saExtraValues.GetSize())
			str1.Replace("@SUPPRESS@", "1");
		else
			str1.Replace("@SUPPRESS@", "0");

		CString str2 = "FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)   \r\n "
			"ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID)   \r\n "
			"LEFT JOIN   \r\n "
			"/*Patient Payments*/ \r\n "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     \r\n "
			"Sum(LineItemT.Amount) AS SumOfAmount,     \r\n "
			"PatientsT.PersonID AS ID    \r\n "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON   \r\n "
			"PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    \r\n "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))    \r\n "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID    \r\n "
			") AS _PatPaysQ   \r\n "
			"/* End Patient Payments */  \r\n "
			"ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN   \r\n "
			"/* Patient Charges */  \r\n "
			// (j.politis 2015-07-30 15:07) - PLID 66741 - We need to fix how we round tax 1 and tax 2
			// (r.goldschmidt 2016-02-04 14:22) - PLID 68022 - Financial reports that run dbo.GetChargeTotal will time out (daily financial reports)
			"(SELECT LineItemT.PatientID,    \r\n "
			"	Sum(ChargeRespT.Amount) AS ChargeAmount,     \r\n"
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName    \r\n "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID \r\n "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID \r\n "
			"INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    \r\n "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))    \r\n "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle    \r\n "
			") AS _PatChargesQ  \r\n "
			"/* End Patient Charges */  \r\n "
			" ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN   \r\n "
			"/* Prepays*/  \r\n "
			"(SELECT FullName, Sum(Amount) AS SumofAmount, ID  \r\n "
			"FROM  \r\n "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     \r\n "
			"((CASE WHEN PrepayAppliedToQ.ID IS NULL THEN     \r\n "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN MAX(LineItemT.Amount) ELSE MAX(LineItemT.Amount - PrepayAppliesQ.Amount) END)    \r\n "
			"ELSE    \r\n "
			"    MAX(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END)) AS Amount,  \r\n "
			"PatientsT.PersonID AS ID  \r\n "
			"FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)   \r\n "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN AppliesT AS AppliesT_1 ON PaymentsT.ID = AppliesT_1.SourceID)  \r\n "
			"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID  \r\n "
			"LEFT JOIN  \r\n "
			"/* This will total everything applied to a prepayment */ \r\n "
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID  \r\n "
			"FROM  \r\n "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  \r\n "
			"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.DestID  \r\n "
			"WHERE (LineItemT.Deleted = 0) and PaymentsT.Prepayment = 1 \r\n "
			"GROUP BY AppliesT.DestID  \r\n "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID  \r\n "
			"LEFT JOIN  \r\n ";

		CString str3 = "/* This will total everything that the prepayment is applied to */    \r\n "
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID    \r\n "
			"FROM   \r\n "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    \r\n "
			"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID    \r\n "
			"WHERE LineItemT.Deleted = 0 and PaymentsT.Prepayment = 1  \r\n "
			"GROUP BY AppliesT.SourceID    \r\n "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID    \r\n "
			"/*end totalling applies to prepays */    \r\n "
			"WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1)  \r\n "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID, LineItemT.Type, LineItemT.ID, PrepayAppliedToQ.ID, PrepayAppliesQ.ID, LineItemT.Description  \r\n "
			"HAVING (((LineItemT.Type)<4))  \r\n "
			") AS PrePaysQ  \r\n "
			"GROUP BY FullName, ID) AS _PatPrePaysQ  \r\n "
			"/* End Prepays */ \r\n "
			"ON PatientsT.PersonID = [_PatPrePaysQ].ID    \r\n "
			") AS PatBalQ   \r\n "
			"/* End Patient Balance */   \r\n "
			" INNER JOIN  \r\n "
			"/* All Financial Query */ \r\n "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   \r\n "
			"PatientsT.PersonID,   \r\n "
			"SumOfAmt = CASE   \r\n "
			"	WHEN AllFinancialQ.[Type]=10 OR FinType IN ('APPPRE', 'APPTOPRE') \r\n "
			"	THEN AllFinancialQ.[Amt]  \r\n "
			"	ELSE AllFinancialQ.[Amount]  \r\n "
			"	End,   \r\n "
			"AllFinancialQ.FinType,   \r\n "
			"AllFinancialQ.BDate, \r\n "
			"AllFinancialQ.TDate,   \r\n "
			"AllFinancialQ.ProvID,   \r\n "
			"AllFinancialQ.InputDate AS IDate,   \r\n "
			"AllFinancialQ.Description,   \r\n "
			"AllFinancialQ.ApplyID,   \r\n "
			"AllFinancialQ.LineID,  \r\n "
			"AllFinancialQ.LocID,  \r\n "
			"AllFinancialQ.PrePayment, \r\n "
			"AllFinancialQ.InputName, \r\n "
			"AllFinancialQ.TotalDiscount, \r\n "
			"AllFinancialQ.PercentDiscount, \r\n "
			"AllFinancialQ.DollarDiscount, \r\n "
			"AllFinancialQ.DiscountCategoryDescription, \r\n "
			"AllFinancialQ.ClaimProviderID, AllFinancialQ.InsResp, AllFinancialQ.ChargeAmtBeforeDiscount \r\n "
			"FROM ((\r\n "
			// (c.haag 2016-03-14) - PLID 68592 - Don't pass in strUserNameFilter; pass in strExternalFilter
			ALL_FINANCIAL_Q_FILTERED(nID, strLocFilter, strProvFilter, strPatientFilter, strBillDateFilter, strServiceDateFilter, strInputDateFilter, strExternalFilter)
			") AS AllFinancialQ  \r\n "
			"/* End Financial Query */ \r\n "
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON AllFinancialQ.PatientID = PatientsT.PersonID) LEFT JOIN LineItemT ON AllFinancialQ.ID = LineItemT.ID  \r\n "
			") AS DailyFinancialSubQ ON PatBalQ.ID = DailyFinancialSubQ.PersonID \r\n "
			"LEFT JOIN PatientsT ON DailyFinancialSubQ.PersonID = PatientsT.PersonID) \r\n "
			"LEFT JOIN PersonT ON DailyFinancialSubQ.ProvID = PersonT.ID \r\n "
			"LEFT JOIN LocationsT ON DailyFinancialSubQ.LocID = LocationsT.ID  \r\n "
			"LEFT JOIN ProvidersT ON DailyFinancialSubQ.ProvID = ProvidersT.PersonID\r\n "
			"LEFT JOIN PersonT ClaimProviderPersonT ON COALESCE(DailyFinancialSubQ.ClaimProviderID, ProvidersT.ClaimProviderID) = ClaimProviderPersonT.ID \r\n "
			"GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, DailyFinancialSubQ.ProvID, DailyFinancialSubQ.FullName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, DailyFinancialSubQ.BDate, DailyFinancialSubQ.TDate, DailyFinancialSubQ.IDate, PatBalQ.AccountBal, DailyFinancialSubQ.LocID, LocationsT.Name, DailyFinancialSubQ.InputName, \r\n "
			"DailyFinancialSubQ.TotalDiscount, DailyFinancialSubQ.PercentDiscount, DailyFinancialSubQ.DollarDiscount, DailyFinancialSubQ.DiscountCategoryDescription, DailyFinancialSubQ.LineID \r\n \r\n "
			", ClaimProviderPersonT.ID, ClaimProviderPersonT.Last + ', ' + ClaimProviderPersonT.First + ' ' + ClaimProviderPersonT.Middle, DailyFinancialSubQ.InsResp, DailyFinancialSubQ.FinType, DailyFinancialSubQ.ChargeAmtBeforeDiscount \r\n \r\n "
			") AS DFSQ \r\n ";
		return _T(str1 + str2 + str3);
	}
	break;

	//Statement


	case 154:
		//Financial Activity - Monthly
		/*	Version History
		1/30/03 - DRT - Made the report editable.
		1/24/03 - TES - Made a significant change to the way we show prepayments (again).  Now this report shows (in the prepayments field), ALL prepayments,
		even if they have been applied or had things applied to them.  Then we calculate another field of that info (the applies to/from prepays), and
		add that in.  So the final calculation of Net Patient Receivables is now:  net pays + adjustments - prepays + applied prepays = receivables.
		This change is also being applied to all reports with AllFinancialQ (the ___ Financial Activity series)
		2/27/04 - TES - Changed it so that it uses the charge location for applied payments, just like the provider.
		DRT 4/8/2004 - PLID 11816 - Replaced a LineItemT.* with appropriate fields
		8/25/2004 - TES - Moved AllFinancialQ to a #define.
		3/11/2005 - TES - Changed to use bill date instead of service date.
		5/16/2005 - TES - Added references to new TransferredFromIDate and TransferredFromTDate fields.
		//(e.lally 2007-08-03) PLID 25116 - Made FinType 40 characters
		// (r.gonet 04/24/2013) - PLID 56171 - Added the charge service date
		// (d.thompson 2013-06-13) - PLID 56360 - Moved InsResp (which already existed in the deepest bowels of AllFinancialQ) and exposed it to the top level of the reports.
		TES 7/30/2015 - PLID 66486 - Fixed TMonth/TYear/TDay/TMonthYear to use TDate instead of Date
		// (j.jones 2016-05-10 09:02) - NX-100501 - changed the MonthYear filter to be YYYYMM, not MMYYYY
		*/
		return _T("SELECT Month(AllFinancialQ.[BDate]) AS Month, Month(AllFinancialQ.InputDate) AS IMonth, Month(AllFinancialQ.TDate) AS TMonth,   "
			"    Year(AllFinancialQ.BDate) AS Year, Year(AllFinancialQ.InputDate) AS IYear, Year(AllFinancialQ.TDate) AS TYear,   "
			"	 Day(AllFinancialQ.BDate) AS Day, Day(AllFinancialQ.InputDate) AS IDay, Day(AllFinancialQ.TDate) AS TDay,   "
			"    AllFinancialQ.FinType AS FinType,   "
			"    AllFinancialQ.ProvID AS ProvID,   "
			"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProvName,  "
			"     AllFinancialQ.ID AS FINID, AllFinancialQ.Amt,   "
			"    AllFinancialQ.Amount, AllFinancialQ.BDate, AllFinancialQ.InputDate AS IDate,  "
			"	 YEAR(AllFinancialQ.BDate)*100 + MONTH(AllFinancialQ.BDate) AS MonthYear, "
			"	 YEAR(AllFinancialQ.InputDate)*100 + MONTH(AllFinancialQ.InputDate) AS IMonthYear, "
			"    AllFinancialQ.LocID AS LocID,   "
			"    LocationsT.Name AS Location, AllFinancialQ.PrePayment,  "
			"	 AllFinancialQ.TransferredFromIDate, AllFinancialQ.TransferredFromTDate, "
			"	 YEAR(AllFinancialQ.TDate)*100 + MONTH(AllFinancialQ.TDate) AS TMonthYear, "
			"	AllFinancialQ.InsResp	"
			"FROM (  "
			ALL_FINANCIAL_Q(nID)
			") AS AllFinancialQ LEFT OUTER JOIN  "
			"    ProvidersT ON   "
			"    AllFinancialQ.ProvID = ProvidersT.PersonID LEFT JOIN  "
			"    PersonT ON ProvidersT.PersonID = PersonT.ID LEFT JOIN  "
			"    LocationsT ON AllFinancialQ.LocID = LocationsT.ID");
		break;
	case 155:
		//Financial Activity - Yearly
		/*	Version History
		1/30/03 - DRT - Made the report editable.
		1/24/03 - DRT - Made a significant change to the way we show prepayments (again).  Now this report shows (in the prepayments field), ALL prepayments,
		even if they have been applied or had things applied to them.  Then we calculate another field of that info (the applies to/from prepays), and
		add that in.  So the final calculation of Net Patient Receivables is now:  net pays + adjustments - prepays + applied prepays = receivables.
		This change is also being applied to all reports with AllFinancialQ (the ___ Financial Activity series)
		2/27/04 - TES - Changed it so that it uses the charge location for applied payments, just like the provider.
		4/8/2004 - DRT - PLID 11816 - Replaced a LineItemT.* with appropriate fields
		8/25/2004 - TES - Moved AllFinancialQ to a #define.
		3/10/2005 - TES - Changed to use bill date
		//(e.lally 2007-08-03) PLID 25116 - Made FinType 40 characters
		// (d.thompson 2013-06-13) - PLID 56360 - Moved InsResp (which already existed in the deepest bowels of AllFinancialQ) and exposed it to the top level of the reports.
		//TES 10/10/2013 - PLID 56392 - Added Input Date and Service Date/Payment Date filters
		// (z.manning 2015-11-13 09:49) - PLID 66486 - Use TDate for the service date
		*/
		return _T(R"(
SELECT Month(AllFinancialQ.BDate) AS Month
	, Year(AllFinancialQ.BDate) AS TYear
	, Day(AllFinancialQ.BDate) AS Day
	, AllFinancialQ.FinType AS FinType
	, AllFinancialQ.ProvID AS ProvID
	, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProvName
	, AllFinancialQ.ID AS FINID
	, AllFinancialQ.Amt
	, AllFinancialQ.Amount
	, AllFinancialQ.BDate
	, AllFinancialQ.PrePayment
	, AllFinancialQ.LocID AS LocID
	, LocationsT.NAME AS Location
	, AllFinancialQ.InsResp
	, Month(AllFinancialQ.InputDate) AS IMonth
	, Year(AllFinancialQ.InputDate) AS IYear
	, Day(AllFinancialQ.InputDate) AS IDay
	, Month(AllFinancialQ.TDate) AS ServiceMonth
	, Year(AllFinancialQ.TDate) AS ServiceYear
	, Day(AllFinancialQ.TDate) AS ServiceDay
FROM (
)" ALL_FINANCIAL_Q(nID) R"(
	) AS AllFinancialQ 
LEFT JOIN ProvidersT ON AllFinancialQ.ProvID = ProvidersT.PersonID
LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID
LEFT JOIN LocationsT ON AllFinancialQ.LocID = LocationsT.ID
)");
		break;

	case 325:
	{
		//Financial Activity - Daily (by Charge Category)
		/*	Version History
		DRT 12/19/2003 - PLID 10483 - Fixed an issue where a mysterious null category was showing up on the report above 'No Category'
		DRT 4/8/2004 - PLID 11816 - Replaced a LineItemT.* with appropriate fields
		8/25/2004 - TES - Moved AllFinancialQ to a #define.
		8/25/2005 - TES - Moved from GetSqlFinancial to GetSqlFinancialActivity
		TES 3/10/2005 - Changed to filter on Bill date like it claims to.
		TES 5/16/2005 - AllFinancialQ now uses Amount for the Prepayments Received column.
		DRT 5/19/2005 - Removed LocationID field, it was unnecessary and confusing, and some reports were filtering on it
		improperly.
		(a.walling 2006-11-02 10:37) - PLID 23209 - Include applied prepayments to dailyfin by code / category
		(e.lally 2007-06-19) PLID 26347 - Formatted query to be more readable (first checkin)
		-Added PreTaxChargeAmt, TotalTax1, TotalTax2 fields
		(e.lally 2007-08-03) PLID 25116 - Replaced refunds with a breakdown of refunds as cash, check, or credit
		(d.moore 2007-09-18) - PLID 25166 - Added fields for discounts and discount categories.
		(d.thompson 2009-03-04) - PLID 33119 - The DFSQ GROUP BY clause was not unique, causing fully duplicated rows
		to be dropped from the query (required to-the-second dates to be the same).  Added a GROUP BY of LineID.
		// (j.gruber 2009-04-01 12:32) - PLID 33358 - updated new discount structure
		//	(c.haag 2010-01-19 17:07) - PLID 36643 - Added APPTOPREPAY (Which equals prepayments minus line items applied to
		// those prepayments)
		(e.lally 2011-11-10) PLID 45425 - Added the linked claim provider for the ProvID
		(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
		// (d.thompson 2012-06-19) - PLID 50833 - This now follows the preference to automatically include subcategories when the parent category is chosen.
		// (r.gonet 2015-05-05 14:38) - PLID 65901 - Added Gift Certificate Refunds
		*/
		CString str1 = "SELECT DFSQ.FullName, DFSQ.UserDefinedID, DFSQ.PatID AS PatID, DFSQ.TDate AS TDate, "
			"DFSQ.IDate AS IDate, DFSQ.DocName, DFSQ.ProvID AS ProvID, DFSQ.ADJUSTMENT, DFSQ.BILL AS BILL, "
			"DFSQ.CASH, DFSQ.CHECKAMT AS 'CHECK', DFSQ.CREDIT, DFSQ.PrePayment, DFSQ.APPPREPAY, DFSQ.APPTOPREPAY, "
			"DFSQ.RefundCash, DFSQ.RefundCheck, DFSQ.RefundCredit, DFSQ.RefundGC, "
			"DFSQ.GCSOLD, DFSQ.GCREDEEMED, DFSQ.AccountBal, DFSQ.LocID AS LocID, "
			"DFSQ.Location, @SUPPRESS@ AS SUPPRESS, DFSQ.Code, DFSQ.RVU, DFSQ.CodeName, DFSQ.FinType, "
			"DFSQ.Quantity, DFSQ.Category AS Category, DFSQ.CatName, DFSQ.BDate AS BDate, "
			"DFSQ.PreTaxChargeAmt, DFSQ.TotalTax1, DFSQ.TotalTax2, "
			"DFSQ.TotalDiscount, DFSQ.PercentDiscount, DFSQ.DollarDiscount, DFSQ.DiscountCategoryDescription, "
			"DFSQ.ClaimProviderID, DFSQ.ClaimProvider "
			"\r\n"
			"FROM "
			"	/* Begin DFSQ */ "
			"	(SELECT  DailyFinancialSubQ.FullName,  "
			"	PatientsT.UserDefinedID,  "
			"	PatientsT.PersonID AS PatID,  "
			"	DailyFinancialSubQ.TDate AS TDate,  "
			"	DailyFinancialSubQ.IDate AS IDate,  "
			"	CASE WHEN DailyFinancialSubQ.ProvID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS DocName,  "
			"	DailyFinancialSubQ.ProvID AS ProvID,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'ADJUSTMENT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS ADJUSTMENT,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS BILL,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CASH' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CASH,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CHECK' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CHECKAMT,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CREDIT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CREDIT,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType =  'PrePayment' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS PREPAYMENT,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType =  'APPPRE' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS APPPREPAY,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCash' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCash,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCheck' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCheck,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCredit' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCredit,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundGC' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundGC,  "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCSOLD' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCSOLD, "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCREDEEMED' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCREDEEMED, "
			"	convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'APPTOPRE' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS APPTOPREPAY, "
			"	convert(money, PatBalQ.AccountBal) AS AccountBal,  "
			"	DailyFinancialSubQ.LocID,  "
			"	LocationsT.Name AS Location, DailyFinancialSubQ.Code, DailyFinancialSubQ.RVU, DailyFinancialSubQ.CodeName, DailyFinancialSubQ.FinType,  "
			"	DailyFinancialSubQ.Quantity, DailyFinancialSubQ.Category, DailyFinancialSubQ.CatName, DailyFinancialSubQ.BDate, "
			"	convert(money, (CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.PreTaxChargeAmt ELSE 0 End)) AS PreTaxChargeAmt, "
			"	convert(money, (CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.TotalTax1 ELSE 0 End)) AS TotalTax1, "
			"	convert(money, (CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.TotalTax2 ELSE 0 End)) AS TotalTax2, "
			"	DailyFinancialSubQ.TotalDiscount, DailyFinancialSubQ.PercentDiscount, "
			"	DailyFinancialSubQ.DollarDiscount, DailyFinancialSubQ.DiscountCategoryDescription "
			"	, ClaimProviderPersonT.ID AS ClaimProviderID, ClaimProviderPersonT.Last + ', ' + ClaimProviderPersonT.First + ' ' + ClaimProviderPersonT.Middle AS ClaimProvider \r\n"
			"\r\n"
			"	FROM ( "
			"	/* Patient Balance */   "
			"		(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"		Payments = CASE    "
			"			WHEN [_PatPaysQ].[SumOfAmount] Is Null    "
			"			THEN 0    "
			"			ELSE [_PatPaysQ].[SumOfAmount]    "
			"			End,     "
			"		Charges = CASE    "
			"			WHEN [ChargeAmount] Is Null    "
			"			THEN 0    "
			"			ELSE [ChargeAmount]    "
			"			End,     "
			"		PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,     "
			"		PatientsT.PersonID AS ID,     "
			"		(CASE    "
			"			WHEN [ChargeAmount] Is Null    "
			"			THEN 0    "
			"			ELSE [ChargeAmount]    "
			"			End)-    "
			"		(CASE    "
			"			WHEN [_PatPaysQ].[SumOfAmount] Is Null    "
			"			THEN 0    "
			"			ELSE [_PatPaysQ].[SumOfAmount]    "
			"			End)+    "
			"		(CASE    "
			"			WHEN [_PatPrePaysQ].[SumOfAmount] Is Null    "
			"			THEN 0    "
			"			ELSE [_PatPrePaysQ].[SumOfAmount]    "
			"			End) AS AccountBal,     "
			"		ProvidersT.PersonID AS ProvID,     "
			"		PrePayments = CASE    "
			"			WHEN [_PatPrePaysQ].[SumOfAmount] Is Null    "
			"			THEN 0    "
			"			ELSE [_PatPrePaysQ].[SumOfAmount]    "
			"			End    ";


		if (bExtended && saExtraValues.GetSize())
			str1.Replace("@SUPPRESS@", "1");
		else
			str1.Replace("@SUPPRESS@", "0");

		CString str2 =
			"		FROM "
			"		/*Begin Patient Balance from clause */ "
			"			(ProvidersT "
			"			RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON ProvidersT.PersonID = PatientsT.[MainPhysician] "
			"			LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID   "
			"			LEFT JOIN   "
			"			/*Patient Payments*/ "
			"				(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"					Sum(LineItemT.Amount) AS SumOfAmount,     "
			"					PatientsT.PersonID AS ID    "
			"					FROM LineItemT "
			"					INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"					INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID "
			"					WHERE (LineItemT.Deleted=0 AND (LineItemT.Type=1 Or LineItemT.Type=2 Or LineItemT.Type=3))    "
			"					GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID    "
			"				) AS _PatPaysQ "
			"			ON PatientsT.PersonID = [_PatPaysQ].ID "
			"			/* End Patient Payments */  "

			"			LEFT JOIN   "
			// (j.politis 2015-07-30 15:07) - PLID 66741 - We need to fix how we round tax 1 and tax 2
			// (r.goldschmidt 2016-02-04 14:21) - PLID 68022 - Financial reports that run dbo.GetChargeTotal will time out (daily financial reports)
			"			/* Patient Charges */  "
			"				(SELECT LineItemT.PatientID,    "
			"				Sum(ChargeRespT.Amount) AS ChargeAmount,     "
			"				PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName    "
			"				FROM LineItemT "
			"				INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"				LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"				INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    "
			"				WHERE LineItemT.Type=10 AND LineItemT.Deleted=0 "
			"				GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle    "
			"				) AS _PatChargesQ  "
			"			 ON PatientsT.PersonID = [_PatChargesQ].PatientID) "
			"			/* End Patient Charges */  "

			"			LEFT JOIN   "
			"			/* Prepays*/  "
			"				(SELECT FullName, Sum(Amount) AS SumofAmount, ID  "
			"				FROM  "
			"					(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"					((CASE WHEN PrepayAppliedToQ.ID IS NULL THEN     "
			"						(CASE WHEN PrepayAppliesQ.ID IS NULL THEN MAX(LineItemT.Amount) ELSE MAX(LineItemT.Amount - PrepayAppliesQ.Amount) END)    "
			"						ELSE    "
			"						MAX(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END)) AS Amount,  "
			"					PatientsT.PersonID AS ID  "
			"					FROM LineItemT "
			"					INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID   "
			"					INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"					LEFT JOIN AppliesT AS AppliesT_1 ON PaymentsT.ID = AppliesT_1.SourceID  "
			"					LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID  "
			"					LEFT JOIN  "
			"				/* This will total everything applied to a prepayment */ "
			"						(SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID  "
			"						FROM  LineItemT "
			"						INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"						INNER JOIN AppliesT ON LineItemT.ID = AppliesT.DestID  "
			"						WHERE (LineItemT.Deleted = 0) and PaymentsT.Prepayment = 1  "
			"						GROUP BY AppliesT.DestID  "
			"						) PrepayAppliedToQ "
			"					ON LineItemT.ID = PrepayAppliedToQ.ID  "
			"					LEFT JOIN  ";

		CString str3 =
			"				/* This will total everything that the prepayment is applied to */    "
			"						(SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID    "
			"						FROM   LineItemT "
			"						INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
			"						INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID    "
			"						WHERE LineItemT.Deleted = 0 and PaymentsT.Prepayment = 1  "
			"						GROUP BY AppliesT.SourceID    "
			"						) PrepayAppliesQ "
			"					ON LineItemT.ID = PrepayAppliesQ.ID    "

			"				/*end totalling applies to prepays */    "
			"					WHERE LineItemT.Deleted = 0 AND PaymentsT.PrePayment = 1  "
			"					GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
			"						PatientsT.PersonID, LineItemT.Type, LineItemT.ID, PrepayAppliedToQ.ID, "
			"						PrepayAppliesQ.ID, LineItemT.Description  "
			"					HAVING LineItemT.Type<4  "
			"					) AS PrePaysQ  "
			"				GROUP BY FullName, ID "
			"				) AS _PatPrePaysQ  "
			"			ON PatientsT.PersonID = [_PatPrePaysQ].ID    "
			"			/* End Prepays */ "

			"			) AS PatBalQ   "
			"		/* End Patient Balance from clause */   "

			"		INNER JOIN  "
			"		/* All Financial Query */ "
			"			(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"				PatientsT.PersonID,   "
			"				SumOfAmt = CASE   "
			"					WHEN AllFinancialQ.[Type]=10 OR AllFinancialQ.FinType IN ('APPPRE', 'APPTOPRE') "
			"					THEN AllFinancialQ.[Amt]  "
			"					ELSE AllFinancialQ.[Amount]  "
			"					End,   "
			"			AllFinancialQ.FinType,   "
			"			AllFinancialQ.TDate,   "
			"			AllFinancialQ.ProvID,   "
			"			AllFinancialQ.InputDate AS IDate,   "
			"			AllFinancialQ.Description,   "
			"			AllFinancialQ.ApplyID,   "
			"			AllFinancialQ.LineID,  "
			"			AllFinancialQ.LocID,  "
			"			AllFinancialQ.PrePayment, AllFinancialQ.Code, AllFinancialQ.RVU, AllFinancialQ.CodeName,  "
			"			AllFinancialQ.Quantity, AllFinancialQ.Category, AllFinancialQ.CatName, AllFinancialQ.BDate, "
			"			AllFinancialQ.PreTaxChargeAmt, AllFinancialQ.TotalTax1, AllFinancialQ.TotalTax2, "
			"			AllFinancialQ.TotalDiscount, AllFinancialQ.PercentDiscount, "
			"			AllFinancialQ.DollarDiscount, AllFinancialQ.DiscountCategoryDescription, AllFinancialQ.ClaimProviderID "
			"			FROM "
			"				( "
			"					("
			// (c.haag 2016-03-10) - PLID 68565 - Include all the filters so that we can optimize AllFinancialQ. You may be asking yourself why we aren't including the other fields -- well i'll tell you why.
			// It's because the other fields aren't what you think they are. strUserNameFilter doesn't actually have usernames for example; it's actually the "extra" report filter (the dropdown).
			// (c.haag 2016-03-14) - PLID 68592 - Added strInputDateFilter and strExternalFilter
			//ALL_FINANCIAL_Q(nID)
			ALL_FINANCIAL_Q_FILTERED(nID, "", "", "", strBillDateFilter, strServiceDateFilter, strInputDateFilter, strExternalFilter)
			"					) AS AllFinancialQ  "

			"					LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) "
			"					ON AllFinancialQ.PatientID = PatientsT.PersonID"
			"				) "

			"			LEFT JOIN LineItemT ON AllFinancialQ.ID = LineItemT.ID  "
			"			) AS DailyFinancialSubQ "
			"		ON PatBalQ.ID = DailyFinancialSubQ.PersonID "
			"		/* End Financial Query */ "


			"		LEFT JOIN PatientsT ON DailyFinancialSubQ.PersonID = PatientsT.PersonID"
			"		) "
			"	/* End Patient Balance */ "

			"	LEFT JOIN PersonT ON DailyFinancialSubQ.ProvID = PersonT.ID "
			"	LEFT JOIN LocationsT ON DailyFinancialSubQ.LocID = LocationsT.ID  "
			"	LEFT JOIN ProvidersT ON DailyFinancialSubQ.ProvID = ProvidersT.PersonID\r\n"
			"	LEFT JOIN PersonT ClaimProviderPersonT ON COALESCE(DailyFinancialSubQ.ClaimProviderID, ProvidersT.ClaimProviderID) = ClaimProviderPersonT.ID \r\n"

			"	GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, DailyFinancialSubQ.ProvID, "
			"		DailyFinancialSubQ.FullName, "
			"		CASE WHEN DailyFinancialSubQ.ProvID IS NULL THEN 'No Provider' "
			"			ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END, "
			"		DailyFinancialSubQ.TDate, DailyFinancialSubQ.IDate, PatBalQ.AccountBal, "
			"		DailyFinancialSubQ.LocID, LocationsT.Name, DailyFinancialSubQ.Code, DailyFinancialSubQ.RVU,"
			"		DailyFinancialSubQ.CodeName, DailyFinancialSubQ.FinType, "
			"		DailyFinancialSubQ.Quantity, DailyFinancialSubQ.Category, "
			"		DailyFinancialSubQ.CatName, DailyFinancialSubQ.BDate, "
			"		DailyFinancialSubQ.TotalDiscount, DailyFinancialSubQ.PercentDiscount, "
			"		DailyFinancialSubQ.DollarDiscount, DailyFinancialSubQ.DiscountCategoryDescription, "
			"		DailyFinancialSubQ.PreTaxChargeAmt, DailyFinancialSubQ.TotalTax1, DailyFinancialSubQ.TotalTax2, DailyFinancialSubQ.LineID "
			"		, ClaimProviderPersonT.ID, ClaimProviderPersonT.Last + ', ' + ClaimProviderPersonT.First + ' ' + ClaimProviderPersonT.Middle \r\n"
			"	) AS DFSQ "
			"	/* End DFSQ */";
		return _T(str1 + str2 + str3);
	}
	break;

	case 468:
		//Financial Activity - Daily (by Ins Co)
		/* Version History
		TES 1/30/2004 - Created, copied from the original Financial Activity - Daily
		DRT 4/8/2004 - PLID 11816 - Replaced a LineItemT.* with appropriate fields
		TES 8/26/2004 - Moved here from GetSqlFinancial
		DRT 5/19/2005 - Removed LocationID field, it was unnecessary and confusing, and some reports were filtering on it
		improperly.
		(e.lally 2007-08-03) PLID 25116 - Replaced refunds with a breakdown of refunds as cash, check, or credit
		(d.moore 2007-09-18) - PLID 25166 - Added fields for discounts and discount categories.
		(d.thompson 2009-03-04) - PLID 33119 - Added a GROUP BY element of LineID to the DFSQ subquery.  Our previous
		grouping did not ensure uniqueness, and it was possible to filter out legitimate records, if you had 100%
		identical (including to-the-second dates) records.
		// (j.gruber 2009-04-01 12:35) - PLID 33358 - updated discount structure
		//(e.lally 2011-11-08) PLID 45541 - Added financial class
		//(e.lally 2011-11-10) PLID 45425 - Added the linked claim provider for the ProvID
		//(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
		// (r.gonet 2015-05-05 14:38) - PLID 65902 - Added Gift Certificate Refunds
		// (r.gonet 2016-03-14 14:36) - PLID 68570 - Moved the AllFinancialQ subquery into its own function in order to use a temp table optimization.
		// - Replaced the string literals with one raw string literal so linebreaks are preserved.
		// - Moved the patient balance subquery into its own function, since it is used all over the place in reports and can stand to be optimized a bit.
		// - Added some indents to the report query.
		// - Made the report ordering expilict since a parallel plan, if one happened to be chosen, could result in the GROUP BY being implemented as
		// a hash aggregate operation rather than with a sort operation, leading to a different results order.
		*/
	{
		CString str = FormatString(R"(
SELECT DFSQ.FullName, DFSQ.UserDefinedID, DFSQ.PatID AS PatID, DFSQ.BDate AS BDate, DFSQ.TDate AS TDate, 
	DFSQ.IDate AS IDate, DFSQ.DocName, DFSQ.ProvID AS ProvID, DFSQ.ADJUSTMENT, DFSQ.BILL AS BILL, 
	DFSQ.CASH, DFSQ.CHECKAMT AS 'CHECK', DFSQ.CREDIT, DFSQ.RefundCash, DFSQ.RefundCheck, DFSQ.RefundCredit, DFSQ.RefundGC, 
	DFSQ.GCSOLD, DFSQ.GCREDEEMED, DFSQ.AccountBal, 
	DFSQ.LocID AS LocID, DFSQ.Location, @SUPPRESS@ AS SUPPRESS, DFSQ.InputName AS InputName, DFSQ.InsCoID AS InsCoID, DFSQ.InsCoName, 
	DFSQ.DollarDiscount, DFSQ.PercentOff, DFSQ.TotalDiscount, DFSQ.DiscountCategoryDescription, 
	DFSQ.FinancialClassID, DFSQ.FinancialClass, DFSQ.ClaimProviderID, DFSQ.ClaimProvider
FROM 
(
	SELECT
		DailyFinancialSubQ.FullName,  
		PatientsT.UserDefinedID,  
		PatientsT.PersonID AS PatID,  
		DailyFinancialSubQ.BDate AS BDate,  
		DailyFinancialSubQ.TDate AS TDate,  
		DailyFinancialSubQ.IDate AS IDate,  
		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName,  
		DailyFinancialSubQ.ProvID AS ProvID,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'ADJUSTMENT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS ADJUSTMENT,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS BILL,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CASH' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CASH,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CHECK' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CHECKAMT,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CREDIT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CREDIT,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCash' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCash,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCheck' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCheck,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCredit' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCredit,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundGC' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundGC,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCSOLD' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCSOLD,  
		convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCREDEEMED' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCREDEEMED,  
		convert(money, PatBalQ.AccountBal) AS AccountBal,  
		DailyFinancialSubQ.LocID,  
		LocationsT.Name AS Location, DailyFinancialSubQ.InputName,  
		DailyFinancialSubQ.InsCoID, DailyFinancialSubQ.InsCoName, 
		DailyFinancialSubQ.DollarDiscount, DailyFinancialSubQ.PercentOff, 
		DailyFinancialSubQ.TotalDiscount, DailyFinancialSubQ.DiscountCategoryDescription,
		DailyFinancialSubQ.FinancialClassID, DailyFinancialSubQ.FinancialClass ,
		ClaimProviderPersonT.ID AS ClaimProviderID, ClaimProviderPersonT.Last + ', ' + ClaimProviderPersonT.First + ' ' + ClaimProviderPersonT.Middle AS ClaimProvider,
		DailyFinancialSubQ.LineID
	FROM 
	( 
		/* Patient Balance */   
		(
			%s
		) AS PatBalQ   
		/* End Patient Balance */
		INNER JOIN  
		/* All Financial Query */ 
		(
			SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   
				PatientsT.PersonID,   
				CASE WHEN AllFinancialQ.[Type] = 10 OR FinType = 'PREPAYMENT' OR FinType IN ('APPPRE', 'APPTOPRE') THEN 
					AllFinancialQ.[Amt]  
				ELSE 
					AllFinancialQ.[Amount]  
				End AS SumOfAmt,   
				AllFinancialQ.FinType,   
				AllFinancialQ.BDate, 
				AllFinancialQ.TDate,   
				AllFinancialQ.ProvID,   
				AllFinancialQ.InputDate AS IDate,   
				AllFinancialQ.Description,   
				AllFinancialQ.ApplyID,   
				AllFinancialQ.LineID,  
				AllFinancialQ.LocID,  
				AllFinancialQ.PrePayment,  
				AllFinancialQ.InputName, 
				AllFinancialQ.InsCoID, 
				AllFinancialQ.InsCoName, 
				AllFinancialQ.DollarDiscount, 
				AllFinancialQ.PercentOff, 
				AllFinancialQ.TotalDiscount, 
				AllFinancialQ.DiscountCategoryDescription,
				AllFinancialQ.FinancialClassID, AllFinancialQ.FinancialClass,
				AllFinancialQ.ClaimProviderID 	
			FROM 
			(
				%s
			) AS AllFinancialQ  
			/* End Financial Query */ 
			LEFT JOIN 
			(
				PatientsT 
				INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID
			) ON AllFinancialQ.PatientID = PatientsT.PersonID 
			LEFT JOIN LineItemT ON AllFinancialQ.ID = LineItemT.ID  
		) AS DailyFinancialSubQ ON PatBalQ.ID = DailyFinancialSubQ.PersonID 
		LEFT JOIN PatientsT ON DailyFinancialSubQ.PersonID = PatientsT.PersonID
	) 
	LEFT JOIN PersonT ON DailyFinancialSubQ.ProvID = PersonT.ID 
	LEFT JOIN LocationsT ON DailyFinancialSubQ.LocID = LocationsT.ID 
	LEFT JOIN ProvidersT ON DailyFinancialSubQ.ProvID = ProvidersT.PersonID
	LEFT JOIN PersonT ClaimProviderPersonT ON COALESCE(DailyFinancialSubQ.ClaimProviderID, ProvidersT.ClaimProviderID) = ClaimProviderPersonT.ID 
	GROUP BY 
		PatientsT.UserDefinedID, 
		PatientsT.PersonID, 
		DailyFinancialSubQ.ProvID, 
		DailyFinancialSubQ.FullName, 
		PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, 
		DailyFinancialSubQ.BDate, 
		DailyFinancialSubQ.TDate, 
		DailyFinancialSubQ.IDate, 
		PatBalQ.AccountBal, 
		DailyFinancialSubQ.LocID, 
		LocationsT.Name, 
		DailyFinancialSubQ.InputName, 
		DailyFinancialSubQ.InsCoID, 
		DailyFinancialSubQ.InsCoName, 
		DailyFinancialSubQ.DollarDiscount, 
		DailyFinancialSubQ.PercentOff, 
		DailyFinancialSubQ.TotalDiscount, 
		DailyFinancialSubQ.DiscountCategoryDescription, 
		DailyFinancialSubQ.LineID, 
		DailyFinancialSubQ.FinancialClassID, 
		DailyFinancialSubQ.FinancialClass,
		ClaimProviderPersonT.ID, 
		ClaimProviderPersonT.Last + ', ' + ClaimProviderPersonT.First + ' ' + ClaimProviderPersonT.Middle 
) AS DFSQ 
ORDER BY 
	DFSQ.UserDefinedID ASC,
	DFSQ.ProvID ASC,
	DFSQ.BDate ASC,
	DFSQ.TDate ASC, 
	DFSQ.IDate ASC, 
	DFSQ.AccountBal ASC, 
	DFSQ.LocID ASC,
	DFSQ.InputName ASC, 
	DFSQ.InsCoID ASC, 
	DFSQ.DollarDiscount ASC, 
	DFSQ.PercentOff ASC, 
	DFSQ.TotalDiscount ASC, 
	DFSQ.DiscountCategoryDescription ASC, 
	DFSQ.LineID ASC, 
	DFSQ.FinancialClassID ASC, 
	DFSQ.ClaimProviderID ASC
)",
GetPatientBalanceQ(nID),
GetAllFinancialQ_Insurance(nID, strLocFilter, strProvFilter, strPatientFilter, strBillDateFilter, strServiceDateFilter, strInputDateFilter, strExternalFilter));

		if (bExtended && saExtraValues.GetSize()) {
			str.Replace("@SUPPRESS@", "1");
			} else {
			str.Replace("@SUPPRESS@", "0");
		}

		return _T(str);
	}
	break;

	case 297:
	{
		//Financial Activity - Daily (by Charge Code)
		/*
		Revision History:
		- DRT - 11/25/02 - Added ability to filter on inventory items as well as cpt codes.  Both have always
		shown in the report, but previously you were only able to filter on cpt codes.
		DRT 4/8/2004 - PLID 11816 - Replaced a LineItemT.* with appropriate fields
		8/25/2004 - TES - Moved AllFinancialQ to a #define.
		8/26/2004 - TES - Moved from GetSqlFinancial
		TES 3/10/2005 - Changed to filter on the Bill date like it says it does.
		TES 5/16/2005 - AllFinancialQ now uses Amount for the Prepayments Received column.
		DRT 5/19/2005 - Removed LocationID field, it was unnecessary and confusing, and some reports were filtering on it
		improperly.
		(a.walling 2006-11-02 10:38) - PLID 23209 - Include applied prepayments to daily fin by code / category
		(a.walling 2007-02-14 15:55) - PLID 23209 - Include category to daily fin by code
		(e.lally 2007-08-03) PLID 25116 - Replaced refunds with a breakdown of refunds as cash, check, or credit
		(d.moore 2007-09-18) - PLID 25166 - Added fields for discounts and discount categories.
		(d.thompson 2009-03-04) - PLID 33119 - Added a GROUP BY element of LineID to the DFSQ subquery.  Our previous
		grouping did not ensure uniqueness, and it was possible to filter out legitimate records, if you had 100%
		identical (including to-the-second dates) records.
		// (j.gruber 2009-04-01 12:41) - PLID 33358 - updated discount structure
		//	(c.haag 2010-01-19 17:07) - PLID 36643 - Added APPTOPREPAY (Which equals prepayments minus line items applied to
		// those prepayments)
		(e.lally 2011-11-10) PLID 45425 - Added the linked claim provider for the ProvID
		(e.lally 2011-12-20) PLID 47113 - Changed ClaimProvider to pull from the charge claim provider override first, then if none exists use the listed provider's default claim provider
		// (r.gonet 2015-05-05 14:38) - PLID 65901 - Added Gift Certificate Refunds
		*/
		CString str1 = "SELECT DFSQ.FullName, DFSQ.UserDefinedID, DFSQ.PatID AS PatID, DFSQ.TDate AS TDate, "
			"DFSQ.IDate AS IDate, DFSQ.DocName, DFSQ.ProvID AS ProvID, DFSQ.ADJUSTMENT, DFSQ.BILL AS BILL, "
			"DFSQ.CASH, DFSQ.CHECKAMT AS 'CHECK', DFSQ.CREDIT, DFSQ.PrePayment, DFSQ.APPPREPAY, DFSQ.APPTOPREPAY, "
			"DFSQ.RefundCash, DFSQ.RefundCheck, DFSQ.RefundCredit, DFSQ.RefundGC, DFSQ.GCREDEEMED, DFSQ.GCSOLD, DFSQ.AccountBal, "
			"DFSQ.LocID AS LocID, DFSQ.Location, @SUPPRESS@ AS SUPPRESS, DFSQ.Code, DFSQ.CodeName, DFSQ.Category AS Category, DFSQ.CatName AS CategoryName, DFSQ.FinType, DFSQ.Quantity, DFSQ.InputName AS InputName, DFSQ.ServiceID AS CPTID, DFSQ.BDate AS BDate, "
			"DFSQ.TotalDiscount, DFSQ.PercentDiscount, DFSQ.DollarDiscount, DFSQ.DiscountCategoryDescription, "
			"DFSQ.ClaimProviderID, DFSQ.ClaimProvider "
			"\r\n"
			"FROM "
			"(SELECT  DailyFinancialSubQ.FullName,  "
			"PatientsT.UserDefinedID,  "
			"PatientsT.PersonID AS PatID,  "
			"DailyFinancialSubQ.TDate AS TDate,  "
			"DailyFinancialSubQ.IDate AS IDate,  "
			"CASE WHEN DailyFinancialSubQ.ProvID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS DocName,  "
			"DailyFinancialSubQ.ProvID AS ProvID,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'ADJUSTMENT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS ADJUSTMENT,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS BILL,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CASH' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CASH,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CHECK' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CHECKAMT,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CREDIT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CREDIT,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType =  'PrePayment' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS PREPAYMENT,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType =  'APPPRE' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS APPPREPAY,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCash' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCash,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCheck' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCheck,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCredit' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCredit,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundGC' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundGC,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCSOLD' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCSOLD,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCREDEEMED' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCREDEEMED,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'APPTOPRE' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS APPTOPREPAY, "
			"convert(money, PatBalQ.AccountBal) AS AccountBal,  "
			"DailyFinancialSubQ.LocID,  "
			"LocationsT.Name AS Location, DailyFinancialSubQ.Code, DailyFinancialSubQ.CodeName, DailyFinancialSubQ.Category, DailyFinancialSubQ.CatName, DailyFinancialSubQ.FinType,  "
			"DailyFinancialSubQ.Quantity, DailyFinancialSubQ.InputName, DailyFinancialSubQ.ServiceID, DailyFinancialSubQ.BDate, "
			"DailyFinancialSubQ.TotalDiscount, DailyFinancialSubQ.PercentDiscount, DailyFinancialSubQ.DollarDiscount, DailyFinancialSubQ.DiscountCategoryDescription "
			", ClaimProviderPersonT.ID AS ClaimProviderID, ClaimProviderPersonT.Last + ', ' + ClaimProviderPersonT.First + ' ' + ClaimProviderPersonT.Middle AS ClaimProvider \r\n"
			"FROM ( "
			"/* Patient Balance */   "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"Payments = CASE    "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [_PatPaysQ].[SumOfAmount]    "
			"	End,     "
			"Charges = CASE    "
			"	WHEN [ChargeAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [ChargeAmount]    "
			"	End,     "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,     "
			"PatientsT.PersonID AS ID,     "
			"(CASE    "
			"	WHEN [ChargeAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [ChargeAmount]    "
			"	End)-    "
			"(CASE    "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [_PatPaysQ].[SumOfAmount]    "
			"	End)+    "
			"(CASE    "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]    "
			"	End) AS AccountBal,     "
			"ProvidersT.PersonID AS ProvID,     "
			"PrePayments = CASE    "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]    "
			"	End    ";


		if (bExtended && saExtraValues.GetSize())
			str1.Replace("@SUPPRESS@", "1");
		else
			str1.Replace("@SUPPRESS@", "0");

		CString str2 = "FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)   "
			"ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID)   "
			"LEFT JOIN   "
			"/*Patient Payments*/ "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"Sum(LineItemT.Amount) AS SumOfAmount,     "
			"PatientsT.PersonID AS ID    "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON   "
			"PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))    "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID    "
			") AS _PatPaysQ   "
			"/* End Patient Payments */  "
			"ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN   "
			// (j.politis 2015-07-30 15:07) - PLID 66741 - We need to fix how we round tax 1 and tax 2
			// (r.goldschmidt 2016-02-04 14:18) - PLID 68022 - Financial reports that run dbo.GetChargeTotal will time out (daily financial reports)
			"/* Patient Charges */  "
			"(SELECT LineItemT.PatientID,    "
			"Sum(ChargeRespT.Amount) AS ChargeAmount,     "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName    "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))    "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle    "
			") AS _PatChargesQ  "
			"/* End Patient Charges */  "
			" ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN   "
			"/* Prepays*/  "
			"(SELECT FullName, Sum(Amount) AS SumofAmount, ID  "
			"FROM  "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"((CASE WHEN PrepayAppliedToQ.ID IS NULL THEN     "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN MAX(LineItemT.Amount) ELSE MAX(LineItemT.Amount - PrepayAppliesQ.Amount) END)    "
			"ELSE    "
			"    MAX(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END)) AS Amount,  "
			"PatientsT.PersonID AS ID  "
			"FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)   "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN AppliesT AS AppliesT_1 ON PaymentsT.ID = AppliesT_1.SourceID)  "
			"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID  "
			"LEFT JOIN  "
			"/* This will total everything applied to a prepayment */ "
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID  "
			"FROM  "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.DestID  "
			"WHERE (LineItemT.Deleted = 0) and PaymentsT.Prepayment = 1  "
			"GROUP BY AppliesT.DestID  "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID  "
			"LEFT JOIN  ";

		CString str3 = "/* This will total everything that the prepayment is applied to */    "
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID    "
			"FROM   "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
			"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID    "
			"WHERE LineItemT.Deleted = 0 and PaymentsT.Prepayment = 1  "
			"GROUP BY AppliesT.SourceID    "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID    "
			"/*end totalling applies to prepays */    "
			"WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1)  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID, LineItemT.Type, LineItemT.ID, PrepayAppliedToQ.ID, PrepayAppliesQ.ID, LineItemT.Description  "
			"HAVING (((LineItemT.Type)<4))  "
			") AS PrePaysQ  "
			"GROUP BY FullName, ID) AS _PatPrePaysQ  "
			"/* End Prepays */ "
			"ON PatientsT.PersonID = [_PatPrePaysQ].ID    "
			") AS PatBalQ   "
			"/* End Patient Balance */   "
			" INNER JOIN  "
			"/* All Financial Query */ "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"PatientsT.PersonID,   "
			"SumOfAmt = CASE   "
			"	WHEN AllFinancialQ.[Type]=10 OR AllFinancialQ.FinType IN ('APPPRE', 'APPTOPRE') "
			"	THEN AllFinancialQ.[Amt]  "
			"	ELSE AllFinancialQ.[Amount]  "
			"	End,   "
			"AllFinancialQ.FinType,   "
			"AllFinancialQ.TDate,   "
			"AllFinancialQ.ProvID,   "
			"AllFinancialQ.InputDate AS IDate,   "
			"AllFinancialQ.Description,   "
			"AllFinancialQ.ApplyID,   "
			"AllFinancialQ.LineID,  "
			"AllFinancialQ.LocID,  "
			"AllFinancialQ.PrePayment, AllFinancialQ.Code, AllFinancialQ.CodeName, AllFinancialQ.Category, AllFinancialQ.CatName, "
			"AllFinancialQ.Quantity, AllFinancialQ.InputName, AllFinancialQ.ServiceID, AllFinancialQ.BDate, "
			"AllFinancialQ.TotalDiscount, AllFinancialQ.PercentDiscount, AllFinancialQ.DollarDiscount, AllFinancialQ.DiscountCategoryDescription "
			", AllFinancialQ.ClaimProviderID "
			"\r\n"
			"FROM ((  "
			// (c.haag 2016-03-10) - PLID 68565 - Include all the filters so that we can optimize AllFinancialQ. You may be asking yourself why we aren't including the other fields -- well i'll tell you why.
			// It's because the other fields aren't what you think they are. strUserNameFilter doesn't actually have usernames for example; it's actually the "extra" report filter (the dropdown).
			// (c.haag 2016-03-14) - PLID 68592 - Added strInputDateFilter and strExternalFilter
			//ALL_FINANCIAL_Q(nID)
			ALL_FINANCIAL_Q_FILTERED(nID, "", "", "", strBillDateFilter, strServiceDateFilter, strInputDateFilter, strExternalFilter)
			") AS AllFinancialQ  "
			"/* End Financial Query */ "
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON AllFinancialQ.PatientID = PatientsT.PersonID) LEFT JOIN LineItemT ON AllFinancialQ.ID = LineItemT.ID  \r\n"
			") AS DailyFinancialSubQ ON PatBalQ.ID = DailyFinancialSubQ.PersonID \r\n"
			"LEFT JOIN PatientsT ON DailyFinancialSubQ.PersonID = PatientsT.PersonID) \r\n"
			"LEFT JOIN PersonT ON DailyFinancialSubQ.ProvID = PersonT.ID \r\n"
			"LEFT JOIN LocationsT ON DailyFinancialSubQ.LocID = LocationsT.ID  \r\n"
			"LEFT JOIN ProvidersT ON DailyFinancialSubQ.ProvID = ProvidersT.PersonID\r\n"
			"LEFT JOIN PersonT ClaimProviderPersonT ON COALESCE(DailyFinancialSubQ.ClaimProviderID, ProvidersT.ClaimProviderID) = ClaimProviderPersonT.ID \r\n"

			"GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, DailyFinancialSubQ.ProvID, DailyFinancialSubQ.FullName, CASE WHEN DailyFinancialSubQ.ProvID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END, DailyFinancialSubQ.TDate, DailyFinancialSubQ.IDate, PatBalQ.AccountBal, DailyFinancialSubQ.LocID, LocationsT.Name, DailyFinancialSubQ.Code, DailyFinancialSubQ.CodeName, DailyFinancialSubQ.Category, DailyFinancialSubQ.CatName, DailyFinancialSubQ.FinType, DailyFinancialSubQ.Quantity, DailyFinancialSubQ.InputName, DailyFinancialSubQ.ServiceID, DailyFinancialSubQ.BDate, "
			"DailyFinancialSubQ.TotalDiscount, DailyFinancialSubQ.PercentDiscount, DailyFinancialSubQ.DollarDiscount, DailyFinancialSubQ.DiscountCategoryDescription, DailyFinancialSubQ.LineID "
			", ClaimProviderPersonT.ID, ClaimProviderPersonT.Last + ', ' + ClaimProviderPersonT.First + ' ' + ClaimProviderPersonT.Middle \r\n"
			") AS DFSQ ";
		return _T(str1 + str2 + str3);
	}
	break;



	case 580:
	{
		// Daily Sales (By Charge Category)
		/*	Version History
		// (z.manning, 10/31/2006) - PLID 22462 - Created. (copied from Financial Activity - Daily (by Charge Category))
		// (z.manning, 10/31/2006) - PLID 22462 - Added tax1, tax2, and applied prepayments.
		//(e.lally 2007-08-03) PLID 25116 - Replaced refunds with a breakdown of refunds as cash, check, or credit
		// (z.manning, 03/05/2008) - PLID 28998 - Fixed the tax calculations in this report.
		(d.thompson 2009-03-04) - PLID 33119 - The DFSQ GROUP BY clause was not unique, causing fully duplicated rows
		to be dropped from the query (required to-the-second dates to be the same).  Added a GROUP BY of LineID.
		// (j.gruber 2009-03-26 09:15) - PLID 33359 - changed discount structure
		//	(c.haag 2010-01-19 17:07) - PLID 36643 - Added APPTOPREPAY (Which equals prepayments minus line items applied to
		// those prepayments)
		// (d.thompson 2012-06-19) - PLID 50834 - This now follows the preference to automatically include subcategories when the parent category is chosen.
		// (j.gruber 2012-08-10 10:21) - PLID 32726 - add charges before discount and discount amt
		// (r.gonet 2015-05-05 14:38) - PLID 65902 - Added Gift Certificate Refunds
		*/
		CString str1 = "SELECT DFSQ.FullName, DFSQ.UserDefinedID, DFSQ.PatID AS PatID, DFSQ.TDate AS TDate, "
			"DFSQ.IDate AS IDate, DFSQ.DocName, DFSQ.ProvID AS ProvID, DFSQ.ADJUSTMENT, DFSQ.BILL AS BILL, "
			"DFSQ.CASH, DFSQ.CHECKAMT AS 'CHECK', DFSQ.CREDIT, DFSQ.PrePayment, DFSQ.APPPREPAY, DFSQ.APPTOPREPAY, "
			"DFSQ.RefundCash, DFSQ.RefundCheck, DFSQ.RefundCredit, DFSQ.RefundGC, DFSQ.GCSOLD, DFSQ.GCREDEEMED, DFSQ.AccountBal, "
			"DFSQ.LocID AS LocID, DFSQ.Location, @SUPPRESS@ AS SUPPRESS, DFSQ.Code, DFSQ.CodeName, DFSQ.FinType, DFSQ.Quantity, DFSQ.Category AS Category, DFSQ.CatName, DFSQ.BDate AS BDate, "
			"DFSQ.TotalTax1, DFSQ.TotalTax2, DFSQ.PreTaxChargeAmt, DFSQ.ChargeAmtBeforeDiscount, DFSQ.TotalDiscount "
			"FROM "
			"(SELECT  DailyFinancialSubQ.FullName,  "
			"PatientsT.UserDefinedID,  "
			"PatientsT.PersonID AS PatID,  "
			"DailyFinancialSubQ.TDate AS TDate,  "
			"DailyFinancialSubQ.IDate AS IDate,  "
			"CASE WHEN DailyFinancialSubQ.ProvID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS DocName,  "
			"DailyFinancialSubQ.ProvID AS ProvID,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'ADJUSTMENT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS ADJUSTMENT,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS BILL,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CASH' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CASH,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CHECK' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CHECKAMT,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'CREDIT' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS CREDIT,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType =  'PrePayment' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS PREPAYMENT,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType =  'APPPRE' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS APPPREPAY,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCash' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCash,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCheck' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCheck,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundCredit' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundCredit,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'RefundGC' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS RefundGC,  "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCSOLD' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCSOLD, "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'GCREDEEMED' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS GCREDEEMED, "
			"convert(money, Sum(CASE WHEN DailyFinancialSubQ.FinType = 'APPTOPRE' THEN DailyFinancialSubQ.SumOfAmt ELSE 0 End)) AS APPTOPREPAY, "
			"convert(money, PatBalQ.AccountBal) AS AccountBal,  "
			"DailyFinancialSubQ.LocID,  "
			"LocationsT.Name AS Location, DailyFinancialSubQ.Code, DailyFinancialSubQ.CodeName, DailyFinancialSubQ.FinType,  "
			"DailyFinancialSubQ.Quantity, DailyFinancialSubQ.Category, DailyFinancialSubQ.CatName, DailyFinancialSubQ.BDate, "
			"convert(money, (CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.PreTaxChargeAmt ELSE 0 End)) AS PreTaxChargeAmt, "
			"convert(money, (CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.TotalTax1 ELSE 0 End)) AS TotalTax1, "
			"convert(money, (CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.TotalTax2 ELSE 0 End)) AS TotalTax2, "
			"convert(money, (CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.ChargeAmtBeforeDiscount ELSE 0 End)) AS ChargeAmtBeforeDiscount, "
			"convert(money, (CASE WHEN DailyFinancialSubQ.FinType = 'BILL' THEN DailyFinancialSubQ.TotalDiscount ELSE 0 End)) AS TotalDiscount "
			"FROM ( "
			"/* Patient Balance */   "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"Payments = CASE    "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [_PatPaysQ].[SumOfAmount]    "
			"	End,     "
			"Charges = CASE    "
			"	WHEN [ChargeAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [ChargeAmount]    "
			"	End,     "
			"PersonT_1.Last + ', ' + PersonT_1.First + ' ' + PersonT.Middle AS DocName,     "
			"PatientsT.PersonID AS ID,     "
			"(CASE    "
			"	WHEN [ChargeAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [ChargeAmount]    "
			"	End)-    "
			"(CASE    "
			"	WHEN [_PatPaysQ].[SumOfAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [_PatPaysQ].[SumOfAmount]    "
			"	End)+    "
			"(CASE    "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]    "
			"	End) AS AccountBal,     "
			"ProvidersT.PersonID AS ProvID,     "
			"PrePayments = CASE    "
			"	WHEN [_PatPrePaysQ].[SumOfAmount] Is Null    "
			"	THEN 0    "
			"	ELSE [_PatPrePaysQ].[SumOfAmount]    "
			"	End    ";


		if (bExtended && saExtraValues.GetSize())
			str1.Replace("@SUPPRESS@", "1");
		else
			str1.Replace("@SUPPRESS@", "0");

		CString str2 = "FROM ((((ProvidersT RIGHT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)   "
			"ON ProvidersT.PersonID = PatientsT.[MainPhysician]) LEFT JOIN PersonT PersonT_1 ON ProvidersT.PersonID = PersonT_1.ID)   "
			"LEFT JOIN   "
			"/*Patient Payments*/ "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"Sum(LineItemT.Amount) AS SumOfAmount,     "
			"PatientsT.PersonID AS ID    "
			"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (PatientsT INNER JOIN PersonT ON   "
			"PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    "
			"WHERE (((LineItemT.Deleted)=0) AND ((LineItemT.Type)=1 Or (LineItemT.Type)=2 Or (LineItemT.Type)=3))    "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID    "
			") AS _PatPaysQ   "
			"/* End Patient Payments */  "
			"ON PatientsT.PersonID = [_PatPaysQ].ID) LEFT JOIN   "
			// (j.politis 2015-07-30 15:07) - PLID 66741 - We need to fix how we round tax 1 and tax 2
			// (r.goldschmidt 2016-02-04 14:17) - PLID 68022 - Financial reports that run dbo.GetChargeTotal will time out (daily financial reports)
			"/* Patient Charges */  "
			"(SELECT LineItemT.PatientID,    "
			"Sum(ChargeRespT.Amount) AS ChargeAmount,     "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName    "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"INNER JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID    "
			"WHERE (((LineItemT.Type)=10) AND ((LineItemT.Deleted)=0))    "
			"GROUP BY LineItemT.PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle    "
			") AS _PatChargesQ  "
			"/* End Patient Charges */  "
			" ON PatientsT.PersonID = [_PatChargesQ].PatientID) LEFT JOIN   "
			"/* Prepays*/  "
			"(SELECT FullName, Sum(Amount) AS SumofAmount, ID  "
			"FROM  "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,     "
			"((CASE WHEN PrepayAppliedToQ.ID IS NULL THEN     "
			"    (CASE WHEN PrepayAppliesQ.ID IS NULL THEN MAX(LineItemT.Amount) ELSE MAX(LineItemT.Amount - PrepayAppliesQ.Amount) END)    "
			"ELSE    "
			"    MAX(CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END)) AS Amount,  "
			"PatientsT.PersonID AS ID  "
			"FROM (((LineItemT INNER JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineItemT.PatientID = PatientsT.PersonID)   "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN AppliesT AS AppliesT_1 ON PaymentsT.ID = AppliesT_1.SourceID)  "
			"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID  "
			"LEFT JOIN  "
			"/* This will total everything applied to a prepayment */ "
			"( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID  "
			"FROM  "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.DestID  "
			"WHERE (LineItemT.Deleted = 0) and PaymentsT.Prepayment = 1  "
			"GROUP BY AppliesT.DestID  "
			") PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID  "
			"LEFT JOIN  ";

		CString str3 = "/* This will total everything that the prepayment is applied to */    "
			"( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID    "
			"FROM   "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID    "
			"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID    "
			"WHERE LineItemT.Deleted = 0 and PaymentsT.Prepayment = 1  "
			"GROUP BY AppliesT.SourceID    "
			") PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID    "
			"/*end totalling applies to prepays */    "
			"WHERE (LineItemT.Deleted = 0) AND (PaymentsT.PrePayment = 1)  "
			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.PersonID, LineItemT.Type, LineItemT.ID, PrepayAppliedToQ.ID, PrepayAppliesQ.ID, LineItemT.Description  "
			"HAVING (((LineItemT.Type)<4))  "
			") AS PrePaysQ  "
			"GROUP BY FullName, ID) AS _PatPrePaysQ  "
			"/* End Prepays */ "
			"ON PatientsT.PersonID = [_PatPrePaysQ].ID    "
			") AS PatBalQ   "
			"/* End Patient Balance */   "
			" INNER JOIN  "
			"/* All Financial Query */ "
			"(SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,   "
			"PatientsT.PersonID,   "
			"SumOfAmt = CASE   "
			"	WHEN AllFinancialQ.[Type]=10 OR AllFinancialQ.FinType IN ('APPPRE', 'APPTOPRE') "
			"	THEN AllFinancialQ.[Amt]  "
			"	ELSE AllFinancialQ.[Amount]  "
			"	End,   "
			"AllFinancialQ.FinType,   "
			"AllFinancialQ.TDate,   "
			"AllFinancialQ.ProvID,   "
			"AllFinancialQ.InputDate AS IDate,   "
			"AllFinancialQ.Description,   "
			"AllFinancialQ.ApplyID,   "
			"AllFinancialQ.LineID,  "
			"AllFinancialQ.LocID,  "
			"AllFinancialQ.PrePayment, AllFinancialQ.Code, AllFinancialQ.CodeName,  "
			"AllFinancialQ.Quantity, AllFinancialQ.Category, AllFinancialQ.CatName, AllFinancialQ.BDate, "
			"AllFinancialQ.TotalTax1, AllFinancialQ.TotalTax2, AllFinancialQ.PreTaxChargeAmt, AllfinancialQ.ChargeAmtBeforeDiscount, AllFinancialQ.TotalDiscount "
			"FROM (("
			// (c.haag 2016-03-10) - PLID 68565 - Include all the filters so that we can optimize AllFinancialQ. You may be asking yourself why we aren't including the other fields -- well i'll tell you why.
			// It's because the other fields aren't what you think they are. strUserNameFilter doesn't actually have usernames for example; it's actually the "extra" report filter (the dropdown).
			// (c.haag 2016-03-14) - PLID 68592 - Added strInputDateFilter and strExternalFilter
			//ALL_FINANCIAL_Q(nID)
			ALL_FINANCIAL_Q_FILTERED(nID, "", "", "", strBillDateFilter, strServiceDateFilter, strInputDateFilter, strExternalFilter)
			") AS AllFinancialQ  "
			"/* End Financial Query */ "
			"LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON AllFinancialQ.PatientID = PatientsT.PersonID) LEFT JOIN LineItemT ON AllFinancialQ.ID = LineItemT.ID  "
			") AS DailyFinancialSubQ ON PatBalQ.ID = DailyFinancialSubQ.PersonID LEFT JOIN PatientsT ON DailyFinancialSubQ.PersonID = PatientsT.PersonID) LEFT JOIN PersonT ON DailyFinancialSubQ.ProvID = PersonT.ID LEFT JOIN LocationsT ON DailyFinancialSubQ.LocID = LocationsT.ID  "
			"GROUP BY PatientsT.UserDefinedID, PatientsT.PersonID, DailyFinancialSubQ.ProvID, DailyFinancialSubQ.FullName, CASE WHEN DailyFinancialSubQ.ProvID IS NULL THEN 'No Provider' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END, DailyFinancialSubQ.TDate, DailyFinancialSubQ.IDate, PatBalQ.AccountBal, DailyFinancialSubQ.LocID, LocationsT.Name, DailyFinancialSubQ.Code, DailyFinancialSubQ.CodeName, DailyFinancialSubQ.FinType, DailyFinancialSubQ.Quantity, DailyFinancialSubQ.Category, DailyFinancialSubQ.CatName, DailyFinancialSubQ.BDate, DailyFinancialSubQ.PreTaxChargeAmt, DailyFinancialSubQ.TotalTax1, DailyFinancialSubQ.TotalTax2, DailyFinancialSubQ.LineID, DailyFinancialSubQ.ChargeAmtBeforeDiscount, DailyFinancialSubQ.TotalDiscount) AS DFSQ ";
		return _T(str1 + str2 + str3);
	}
	break;

	case 624:
	case 660:
		//Daily User Activity
		//Daily User Activity - Detailed
		/*	Version History
		DRT 2/22/2008 - PLID 29113 - Created.  This is the same as the first 8 columns in the 'Financial Activity - Daily' report.  It's
		purpose is to show the bulk of the same info, but to do so without a ton of extra processing (balances, weird applies, etc).
		Please do not add a lot of advanced stuff to this report in the future, it must remain "lean and mean", to operate quickly
		in a very busy environment.
		// (z.manning 2009-04-02 12:38) - PLID 33170 - Daily User Activity - Detailed now uses this query too
		TES 2/15/2010 - PLID 37309 - Added LineID to the GROUP BY to make sure that identical charges would be kept separate
		// (j.gruber 2010-05-18 12:04) - PLID 38429 - added lineitemID to the main report
		// (c.haag 2011-03-28) - PLID 42403 - Added SuperbillID, Insurance companies, Quantity, Diagnosis codes
		// (z.manning 2012-11-14 15:02) - PLID 53748 - Added ApplyID to group by clause to make sure the same payment
		// applied to multiple charges (with the same apply amount) show up.
		// (j.jones 2013-05-08 11:10) - PLID 55066 - added Calls, Alberta Claim Number, and Referring Physician NPI
		// (r.farnworth 2013-07-25 10:38) - PLID 56745 - Added Check # and Credit Card Authorization
		// (s.dhole 2013-08-30 14:40) - PLID 57762 Added PreTaxAndDiscountAmt , TotalDiscountAmt
		// (d.lange 2013-09-10 10:34) - PLID 58492 - Added Alberta HLINK preference check, which removes the join on ClaimHistoryDetailsT
		// when the preference is disabled
		// (j.gruber 2014-03-27 14:49) - PLID 61448 - updated for ICD-10
		// (c.haag 2015-02-25) - PLID 65009 - Optimized for ICD-10 by moving the ICD and WhichCodes joins to the outer query
		*/
	{
		long nAlbertaHLINK = GetRemotePropertyInt("UseAlbertaHLINK", 0, 0, "<None>", true);

		CString strQuery;
		strQuery =
			"SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, \r\n"
			"PatientsT.UserDefinedID,  \r\n"
			"PatientsT.PersonID AS PatID,  \r\n"
			"BDate AS BDate,  \r\n"
			"TDate AS TDate,  \r\n"
			"ActivityQ.InputDate AS IDate,  \r\n"
			"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS DocName,  \r\n"
			"ProvID AS ProvID,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'ADJUSTMENT' THEN Amt ELSE 0 End)) AS ADJUSTMENT,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'BILL' THEN Amt ELSE 0 End)) AS BILL,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'CASH' THEN Amt ELSE 0 End)) AS CASH,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'CHECK' THEN Amt ELSE 0 End)) AS CHECKAMT,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'CREDIT' THEN Amt ELSE 0 End)) AS CREDIT,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'PrePayment' THEN Amt ELSE 0 End)) AS PREPAYMENT,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'RefundCash' THEN Amt ELSE 0 End)) AS RefundCash,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'RefundCheck' THEN Amt ELSE 0 End)) AS RefundCheck,  \r\n"
			"convert(money, Sum(CASE WHEN FinType = 'RefundCredit' THEN Amt ELSE 0 End)) AS RefundCredit,  \r\n"
			"LocID AS LocID, ActivityQ.Location, ActivityQ.InputName AS InputName \r\n"
			", ActivityQ.Description, ActivityQ.FinType, ActivityQ.Amt AS Amount, ActivityQ.Code \r\n"
			", CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4 \r\n"
			", PlaceOfServiceID, PlaceOfService, POSCode, PayCategory, InsResp, CardName \r\n"
			", TotalTax1, TotalTax2, IsProduct, ActivityQ.ID as LineItemID \r\n"
			", ActivityQ.SuperBillID AS SuperBillID \r\n"
			", ActivityQ.PrimaryInsCo AS PrimaryInsCo \r\n"
			", ActivityQ.SecondaryInsCo AS SecondaryInsCo \r\n"
			", ActivityQ.Quantity AS Quantity, \r\n"

			"ICD9T1.CodeNumber as ICD9Code1, \r\n "
			"ICD9T2.CodeNumber as ICD9Code2, \r\n "
			"ICD9T3.CodeNumber as ICD9Code3, \r\n "
			"ICD9T4.CodeNumber as ICD9Code4, \r\n "
			"ICD9T5.CodeNumber as ICD9Code5, \r\n "
			"ICD9T6.CodeNumber as ICD9Code6, \r\n "
			"ICD9T7.CodeNumber as ICD9Code7, \r\n "
			"ICD9T8.CodeNumber as ICD9Code8, \r\n "
			"ICD9T9.CodeNumber as ICD9Code9, \r\n "
			"ICD9T10.CodeNumber as ICD9Code10, \r\n "

			"ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"ICD10T2.CodeNumber as ICD10Code2, \r\n "
			"ICD10T3.CodeNumber as ICD10Code3, \r\n "
			"ICD10T4.CodeNumber as ICD10Code4, \r\n "
			"ICD10T5.CodeNumber as ICD10Code5, \r\n "
			"ICD10T6.CodeNumber as ICD10Code6, \r\n "
			"ICD10T7.CodeNumber as ICD10Code7, \r\n "
			"ICD10T8.CodeNumber as ICD10Code8, \r\n "
			"ICD10T9.CodeNumber as ICD10Code9, \r\n "
			"ICD10T10.CodeNumber as ICD10Code10, \r\n "

			"ChargeWhichCodesListV.WhichCodes9, \r\n"
			"ChargeWhichCodesListV.WhichCodes10, \r\n"
			"ChargeWhichCodesListV.WhichCodesBoth \r\n"

			", ActivityQ.Calls \r\n"
			", ActivityQ.AlbertaClaimNumber \r\n"
			", ActivityQ.ReferringPhysicianNPI \r\n"
			", ActivityQ.CheckNo \r\n"
			", ActivityQ.CCAuthNo \r\n"
			",convert(money,  Sum(ActivityQ.PreTaxAndDiscountLineAmt)) AS PreTaxAndDiscountAmt\r\n"
			",convert(money,  Sum(ActivityQ.DiscountAmt)) AS TotalDiscountAmt\r\n"
			"FROM (\r\n"
			"SELECT 'BILL' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Description,  "
			"	LineItemT.InputDate, LineItemT.InputName, BillsT.Date AS BDate, "
			"	LineItemT.Date AS TDate, 0 AS PrePayment, Amt = ChargeRespT.Amount, ChargesT.DoctorsProviders AS ProvID, 'Bill' AS RText, "
			"	0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, "
			"	CASE WHEN CPTCodeT.ID IS NULL THEN convert(nvarchar, ProductT.ID) ELSE CPTCodeT.Code END AS Code, ServiceT.Name AS CodeName, "
			"	ChargesT.Quantity, ServiceT.Category, CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName, ChargesT.ServiceID, "
			"	CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN 0 ELSE 1 END AS InsResp, "
			"	'' AS PayCategory, CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4, "
			"	POS.ID AS PlaceOfServiceID, POS.Name AS PlaceOfService, PlaceOfServiceCodesT.PlaceCodes AS POSCode, "
			"	'' AS CardName, CASE WHEN ProductT.ID IS NOT NULL THEN 1 ELSE 0 END AS IsProduct, "
			"	Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2) AS TotalTax1,  "
			"	Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2) AS TotalTax2,  "
			"	ChargesT.SuperBillID, "
			"	(SELECT InsuranceCoT.Name FROM ChargeRespT LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID LEFT JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE ChargeRespT.ChargeID = ChargesT.ID AND InsuredPartyT.RespTypeID = 1) AS PrimaryInsCo, "
			"	(SELECT InsuranceCoT.Name FROM ChargeRespT LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID LEFT JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE ChargeRespT.ChargeID = ChargesT.ID AND InsuredPartyT.RespTypeID = 2) AS SecondaryInsCo, "
			"	ChargesT.ID AS ChargeID, ChargesT.BillID AS BillID, "

			" ChargesT.Calls, ";
		strQuery += (nAlbertaHLINK == 0 ? "'' AS AlbertaClaimNumber, " : "AlbertaTransNumQ.AlbertaTransNum AS AlbertaClaimNumber, ");
		strQuery +=
			"	ReferringPhysT.NPI AS ReferringPhysicianNPI, "
			"	'' AS CheckNo, '' AS CCAuthNo, "
			"   Round(Convert(money,(((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)* (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))))),2) AS PreTaxAndDiscountLineAmt, "
			"   (Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2  Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN  1 ELSE CPTMultiplier4 END))), 2)-Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1  END)* "
			"   (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN (TotalDiscountsQ.TotalPercentOff) Is Null THEN 1 ELSE ((100-Convert(float,(TotalDiscountsQ.TotalPercentOff)))/100) END)-(CASE WHEN ( TotalDiscountsQ.TotalDiscount) Is Null THEN 0 ELSE (TotalDiscountsQ.TotalDiscount) END))), 2))  AS DiscountAmt "
			"FROM LineItemT "
			"	LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"	LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"	LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"	LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"	LEFT JOIN LocationsT POS ON BillsT.Location = POS.ID "
			"	LEFT JOIN PlaceOfServiceCodesT ON POS.POSID = PlaceOfServiceCodesT.ID "
			"	LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
			"   LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID ";
		if (nAlbertaHLINK == 1) {
			strQuery +=
				"	LEFT JOIN (SELECT Max(ID) AS NewestID, ChargeID FROM ClaimHistoryDetailsT "
				"		GROUP BY ChargeID) AS MostRecentClaimHistoryDetailQ ON ChargesT.ID = MostRecentClaimHistoryDetailQ.ChargeID "
				"	LEFT JOIN (SELECT ID, AlbertaTransNum FROM ClaimHistoryDetailsT) AS AlbertaTransNumQ ON MostRecentClaimHistoryDetailQ.NewestID = AlbertaTransNumQ.ID ";
		}
		strQuery +=
			"WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND ChargesT.ServiceID NOT IN (SELECT ServiceID FROM GCTypesT) "
			""
			"\r\n\r\n"
			""
			"UNION ALL SELECT 'CHARGE' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Description,   "
			"	LineItemT.InputDate, LineItemT.InputName, LineItemT.Date AS BDate,  "
			"	LineItemT.Date AS TDate, 0 AS PrePayment, ChargeRespT.Amount AS Amt, ChargesT.DoctorsProviders AS ProvID, 'Charge' AS RText,  "
			"	0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location,  "
			"	CASE WHEN CPTCodeT.ID IS NULL THEN convert(nvarchar, ProductT.ID) ELSE CPTCodeT.Code END AS Code, ServiceT.Name AS CodeName,  "
			"	ChargesT.Quantity, ServiceT.Category, CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName, ChargesT.ServiceID,  "
			"	CASE WHEN ChargeRespT.InsuredPartyID Is Null THEN 0 ELSE 1 END AS InsResp, '' AS PayCategory, "
			"	CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4, "
			"	POS.ID AS PlaceOfServiceID, POS.Name AS PlaceOfService, PlaceOfServiceCodesT.PlaceCodes AS POSCode, "
			"	'' AS CardName, CASE WHEN ProductT.ID IS NOT NULL THEN 1 ELSE 0 END AS IsProduct, "
			"	Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2) AS TotalTax1,  "
			"	Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2) AS TotalTax2, "
			"	ChargesT.SuperBillID, "
			"	(SELECT InsuranceCoT.Name FROM ChargeRespT LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID LEFT JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE ChargeRespT.ChargeID = ChargesT.ID AND InsuredPartyT.RespTypeID = 1) AS PrimaryInsCo, "
			"	(SELECT InsuranceCoT.Name FROM ChargeRespT LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID LEFT JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE ChargeRespT.ChargeID = ChargesT.ID AND InsuredPartyT.RespTypeID = 2) AS SecondaryInsCo, "
			"	ChargesT.ID AS ChargeID, ChargesT.BillID AS BillID, "

			" ChargesT.Calls, ";
		strQuery += (nAlbertaHLINK == 0 ? "'' AS AlbertaClaimNumber, " : "AlbertaTransNumQ.AlbertaTransNum AS AlbertaClaimNumber, ");
		strQuery +=
			"	ReferringPhysT.NPI AS ReferringPhysicianNPI, "
			"	'' AS CheckNo, '' AS CCAuthNo, "
			"   Round(Convert(money,(((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)* (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))))),2) AS PreTaxAndDiscountAmt, "
			"   (Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2  Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN  1 ELSE CPTMultiplier4 END))), 2)-Round(convert(money,(LineItemT.[Amount]*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1  END)* "
			"   (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN (TotalDiscountsQ.TotalPercentOff) Is Null THEN 1 ELSE ((100-Convert(float,(TotalDiscountsQ.TotalPercentOff)))/100) END)-(CASE WHEN ( TotalDiscountsQ.TotalDiscount) Is Null THEN 0 ELSE (TotalDiscountsQ.TotalDiscount) END))), 2))  AS DiscountAmt "
			"FROM LineItemT "
			"	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"	LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"	LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"	LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"	LEFT JOIN LocationsT POS ON BillsT.Location = POS.ID "
			"	LEFT JOIN PlaceOfServiceCodesT ON POS.POSID = PlaceOfServiceCodesT.ID "
			"	LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
			"   LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID ";
		if (nAlbertaHLINK == 1) {
			strQuery +=
				"	LEFT JOIN (SELECT Max(ID) AS NewestID, ChargeID FROM ClaimHistoryDetailsT "
				"		GROUP BY ChargeID) AS MostRecentClaimHistoryDetailQ ON ChargesT.ID = MostRecentClaimHistoryDetailQ.ChargeID "
				"	LEFT JOIN (SELECT ID, AlbertaTransNum FROM ClaimHistoryDetailsT) AS AlbertaTransNumQ ON MostRecentClaimHistoryDetailQ.NewestID = AlbertaTransNumQ.ID ";
		}
		// (r.gonet 2015-05-05 14:38) - PLID 65902 - Added PayMethod 10 - Gift Certificate Refunds
		strQuery +=
			"WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND ChargesT.ServiceID NOT IN (SELECT ServiceID FROM GCTypesT) "
			""
			"\r\n\r\n"
			""
			"UNION ALL SELECT  convert(nvarchar(40), "
			"		CASE WHEN LineItemT.Type = 1 THEN "
			"		CASE WHEN PaymentsT.PayMethod = 1 THEN 'CASH' "
			"		     WHEN PaymentsT.PayMethod = 2 THEN 'CHECK' "
			"		     WHEN PaymentsT.PayMethod = 3 THEN 'CREDIT' "
			"		     WHEN PaymentsT.PayMethod = 4 THEN 'GCREDEEMED' END "
			"	     WHEN LineItemT.Type = 2 THEN 'ADJUSTMENT' "
			"	     WHEN LineItemT.Type = 3 THEN "
			"			CASE WHEN PaymentsT.PayMethod = 7 THEN 'RefundCash' "
			"				 WHEN PaymentsT.PayMethod = 8 THEN 'RefundCheck' "
			"				 WHEN PaymentsT.PayMethod = 9 THEN 'RefundCredit' "
			"				 WHEN PaymentsT.PayMethod = 10 THEN 'RefundGC' END "
			"	END) AS FinType, "
			"	LineItemT.ID, LineItemT.PatientID, LineItemT.Type, "
			"	LineItemT.Description,  LineItemT.InputDate AS IDate, LineItemT.InputName,  "
			"	LineItemT.Date AS BDate, LineItemT.Date AS TDate, PaymentsT.PrePayment,  "
			"	CASE WHEN _PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount END ELSE AppliesT.Amount END AS Amt, "
			"	CASE WHEN ChargesT.ID Is Null THEN CASE WHEN PayDestT.ID Is Null THEN PaymentsT.ProviderID ELSE PayDestT.ProviderID END ELSE [DoctorsProviders] END AS ProvID, "
			"	'Full' AS RText, AppliesT.ID AS ApplyID, LineItemT.ID AS LineID, CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
			"	CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
			"	CASE WHEN CPTCodeT.Code IS NULL THEN CONVERT(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS Code, ServiceT.Name AS CodeName, 0 AS Quantity, ServiceT.Category, "
			"	CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName, ServiceT.ID AS ServiceID, CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, "
			"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, "
			"	'' AS CPTModifier, '' AS CPTModifier2, '' AS CPTModifier3, '' AS CPTModifier4, "
			"	POS.ID AS PlaceOfServiceID, POS.Name AS PlaceOfService, PlaceOfServiceCodesT.PlaceCodes AS POSCode, "
			"	CardName, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS IsProduct, "
			"	ChargesT.SuperBillID, "
			"	(SELECT InsuranceCoT.Name FROM ChargeRespT LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID LEFT JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE ChargeRespT.ChargeID = ChargesT.ID AND InsuredPartyT.RespTypeID = 1) AS PrimaryInsCo, "
			"	(SELECT InsuranceCoT.Name FROM ChargeRespT LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID LEFT JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE ChargeRespT.ChargeID = ChargesT.ID AND InsuredPartyT.RespTypeID = 2) AS SecondaryInsCo, "
			"	ChargesT.ID AS ChargeID, ChargesT.BillID AS BillID, "

			" ChargesT.Calls, ";
		strQuery += (nAlbertaHLINK == 0 ? "'' AS AlbertaClaimNumber, " : "AlbertaTransNumQ.AlbertaTransNum AS AlbertaClaimNumber, ");
		strQuery +=
			"	ReferringPhysT.NPI AS ReferringPhysicianNPI, "
			"	PaymentPlansT.CheckNo, PaymentPlansT.CCAuthNo, "
			"   0 AS PreTaxAndDiscountLineAmt, "
			"   0  AS DiscountAmt "
			"FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"	LEFT JOIN "
			"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, SUM(AppliesT.Amount) AS ApplyAmt, "
			"			LineItemT_1.Amount - SUM(AppliesT.Amount) AS Total, LineItemT_1.PatientID "
			"	 "
			"		FROM LineItemT AS LineItemT_1 "
			"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
			"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
			"	 "
			"		WHERE LineItemT_1.Deleted = 0 AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT_1.GiftID, -1)) "
			"	 "
			"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
			"	 "
			"		HAVING (LineItemT_1.Amount - SUM(AppliesT.Amount)) <> 0 "
			"		) _PartiallyAppliedPaysQ "
			"	ON LineItemT.ID = _PartiallyAppliedPaysQ.ID "
			"	LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"	LEFT JOIN LineItemT LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID "
			"	LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID "
			"	LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"	LEFT JOIN PaymentsT AS PayDestT ON LineItemT_1.ID = PayDestT.ID "
			"	LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID "
			"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"	LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"	LEFT JOIN LocationsT POS ON BillsT.Location = POS.ID "
			"	LEFT JOIN PlaceOfServiceCodesT ON POS.POSID = PlaceOfServiceCodesT.ID "
			"	LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID ";
		if (nAlbertaHLINK == 1) {
			strQuery +=
				"	LEFT JOIN (SELECT Max(ID) AS NewestID, ChargeID FROM ClaimHistoryDetailsT "
				"		GROUP BY ChargeID) AS MostRecentClaimHistoryDetailQ ON ChargesT.ID = MostRecentClaimHistoryDetailQ.ChargeID "
				"	LEFT JOIN (SELECT ID, AlbertaTransNum FROM ClaimHistoryDetailsT) AS AlbertaTransNumQ ON MostRecentClaimHistoryDetailQ.NewestID = AlbertaTransNumQ.ID ";
		}
		// (r.gonet 2015-05-05 14:38) - PLID 65902 - Added PayMethod 10 - Gift Certificate Refunds
		strQuery +=
			"WHERE LineItemT.Deleted = 0 AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT.GiftID, -1)) "
			""
			"\r\n\r\n"
			""
			"UNION ALL SELECT  CASE WHEN LineItemT.Type = 1 THEN "
			"		CASE WHEN PaymentsT.PayMethod = 1 THEN 'CASH' "
			"		     WHEN PaymentsT.PayMethod = 2 THEN 'CHECK' "
			"		     WHEN PaymentsT.PayMethod = 3 THEN 'CREDIT' "
			"		     WHEN PaymentsT.PayMethod = 4 THEN 'GCREDEEMED' END "
			"	     WHEN LineItemT.Type = 2 THEN 'ADJUSTMENT' "
			"	     WHEN LineItemT.Type = 3 THEN "
			"			CASE WHEN PaymentsT.PayMethod = 7 THEN 'RefundCash' "
			"				 WHEN PaymentsT.PayMethod = 8 THEN 'RefundCheck' "
			"				 WHEN PaymentsT.PayMethod = 9 THEN 'RefundCredit' "
			"				 WHEN PaymentsT.PayMethod = 10 THEN 'RefundGC' END "
			"	END AS FinType, "
			"	_PartiallyAppliedPaysQ.ID, LineItemT.PatientID, LineItemT.Type, LineItemT.Description, "
			"	 LineItemT.InputDate AS IDate, LineItemT.InputName,  "
			"	LineItemT.Date AS BDate, LineItemT.Date AS TDate, PaymentsT.PrePayment, _PartiallyAppliedPaysQ.Total AS Amt, PaymentsT.ProviderID AS ProvID, "
			"	'Part' AS RText, 0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, LocationsT.Name AS Location, NULL AS Code, "
			"	NULL AS CodeName, 0 AS Quantity, NULL AS Category, '' AS CatName, NULL AS ServiceID, "
			"	CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, "
			"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, "
			"	'' AS CPTModifier, '' AS CPTModifier2, '' AS CPTModifier3, '' AS CPTModifier4, "
			"	NULL AS PlaceOfServiceID, '' AS PlaceOfService, NULL AS POSCode, "
			"	CardName, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS IsProduct, "
			"	0 AS SuperBillID, "
			"	'' AS PrimaryInsCo, "
			"	'' AS SecondaryInsCo, "
			"	NULL AS ChargeID, NULL AS BillID, "

			"	NULL AS Calls, '' AS AlbertaClaimNumber, '' AS ReferringPhysicianNPI, "
			"	PaymentPlansT.CheckNo, PaymentPlansT.CCAuthNo, "
			"   0 AS PreTaxAndDiscountLineAmt, "
			"   0 AS DiscountAmt "
			" "
			"FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"	LEFT JOIN "
			"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt, "
			"			LineItemT_1.Amount - Sum(AppliesT.Amount) AS Total, LineItemT_1.PatientID "
			"	 "
			"		FROM LineItemT AS LineItemT_1 "
			"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
			"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
			"	 "
			"		WHERE LineItemT_1.Deleted = 0 AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT_1.GiftID, -1)) "
			"	 "
			"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
			"	 "
			"		HAVING (LineItemT_1.Amount - Sum(AppliesT.Amount)) <> 0 "
			"		) AS _PartiallyAppliedPaysQ "
			"	ON LineItemT.ID = _PartiallyAppliedPaysQ.ID "
			" "
			"WHERE _PartiallyAppliedPaysQ.ID Is Not Null AND LineItemT.Deleted = 0 AND (PaymentsT.PayMethod <> 10 OR ISNULL(PaymentsT.RefundedFromGiftID, -1) = ISNULL(LineItemT.GiftID, -1)) "
			""
			"\r\n\r\n"
			""
			"UNION ALL SELECT 'PREPAYMENT' AS FinType, LineItemT.ID, LineItemT.PatientID, LineItemT.Type, "
			"	LineItemT.Description,  LineItemT.InputDate AS IDate, LineItemT.InputName, LineItemT.Date AS BDate, LineItemT.Date AS TDate, PaymentsT.PrePayment, "
			"	CASE WHEN _PartiallyAppliedPaysQ.ID Is Null THEN CASE WHEN LineItemT_1.ID Is Null THEN LineItemT.Amount ELSE AppliesT.Amount END ELSE AppliesT.Amount END AS Amt, "
			"	CASE WHEN ChargesT.ID Is Null THEN CASE WHEN PayDestT.ID Is Null THEN PaymentsT.ProviderID ELSE PayDestT.ProviderID END ELSE DoctorsProviders END AS ProvID, "
			"	'Full' AS RText, AppliesT.ID AS ApplyID, LineItemT.ID AS LineID, "
			"	CASE WHEN LineItemT_1.LocationID Is Null THEN LineItemT.LocationID ELSE LineItemT_1.LocationID END AS LocID, "
			"	CASE WHEN LineItemT_1.LocationID Is Null THEN LocationsT.Name ELSE ChargeLoc.Name END AS Location, "
			"	CASE WHEN CPTCodeT.Code IS NULL THEN CONVERT(nvarchar, ServiceT.ID) ELSE CPTCodeT.Code END AS Code, ServiceT.Name AS CodeName, "
			"	0 AS Quantity, ServiceT.Category, CASE WHEN CategoriesT.ID Is Null THEN '' ELSE CategoriesT.Name END AS CatName, ServiceT.ID AS ServiceID, "
			"	CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, "
			"	'' AS CPTModifier, '' AS CPTModifier2, '' AS CPTModifier3, '' AS CPTModifier4, "
			"	POS.ID AS PlaceOfServiceID, POS.Name AS PlaceOfService, PlaceOfServiceCodesT.PlaceCodes AS POSCode, "
			"	CardName, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS IsProduct, "
			"	ChargesT.SuperBillID, "
			"	(SELECT InsuranceCoT.Name FROM ChargeRespT LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID LEFT JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE ChargeRespT.ChargeID = ChargesT.ID AND InsuredPartyT.RespTypeID = 1) AS PrimaryInsCo, "
			"	(SELECT InsuranceCoT.Name FROM ChargeRespT LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID LEFT JOIN InsuranceCoT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE ChargeRespT.ChargeID = ChargesT.ID AND InsuredPartyT.RespTypeID = 2) AS SecondaryInsCo, "
			"	ChargesT.ID AS ChargeID, ChargesT.BillID AS BillID, "

			"	ChargesT.Calls, ";
		strQuery += (nAlbertaHLINK == 0 ? "'' AS AlbertaClaimNumber, " : "AlbertaTransNumQ.AlbertaTransNum AS AlbertaClaimNumber, ");
		strQuery +=
			"	ReferringPhysT.NPI AS ReferringPhysicianNPI, "
			"	PaymentPlansT.CheckNo, PaymentPlansT.CCAuthNo, "
			"   0 AS PreTaxAndDiscountLineAmt, "
			"   0 AS DiscountAmt "
			"FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"	LEFT JOIN "
			"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, SUM(AppliesT.Amount) AS ApplyAmt, "
			"			LineItemT_1.Amount - SUM(AppliesT.Amount) AS Total, LineItemT_1.PatientID "
			" "
			"		FROM LineItemT AS LineItemT_1 "
			"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
			"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
			" "
			"		WHERE LineItemT_1.Deleted = 0 "
			" "
			"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
			" "
			"		HAVING (LineItemT_1.Amount - SUM(AppliesT.Amount)) <> 0 "
			"		) _PartiallyAppliedPaysQ "
			" "
			"	ON LineItemT.ID = _PartiallyAppliedPaysQ.ID "
			"	LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"	LEFT JOIN LineItemT LineItemT_1 ON AppliesT.DestID = LineItemT_1.ID "
			"	LEFT JOIN ChargesT ON LineItemT_1.ID = ChargesT.ID "
			"	LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "

			"	LEFT JOIN PaymentsT AS PayDestT ON LineItemT_1.ID = PayDestT.ID "
			"	LEFT JOIN LocationsT AS ChargeLoc ON LineItemT_1.LocationID = ChargeLoc.ID "
			"	LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"	LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"	LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"	LEFT JOIN LocationsT POS ON BillsT.Location = POS.ID "
			"	LEFT JOIN PlaceOfServiceCodesT ON POS.POSID = PlaceOfServiceCodesT.ID "
			"	LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID ";
		if (nAlbertaHLINK == 1) {
			strQuery +=
				"	LEFT JOIN (SELECT Max(ID) AS NewestID, ChargeID FROM ClaimHistoryDetailsT "
				"		GROUP BY ChargeID) AS MostRecentClaimHistoryDetailQ ON ChargesT.ID = MostRecentClaimHistoryDetailQ.ChargeID "
				"	LEFT JOIN (SELECT ID, AlbertaTransNum FROM ClaimHistoryDetailsT) AS AlbertaTransNumQ ON MostRecentClaimHistoryDetailQ.NewestID = AlbertaTransNumQ.ID ";
		}
		strQuery +=
			" "
			"WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 1 "
			""
			"\r\n\r\n"
			""
			"UNION ALL SELECT 'PREPAYMENT' AS FinType, _PartiallyAppliedPaysQ.ID, LineItemT.PatientID, LineItemT.Type,  "
			"	LineItemT.Description,  LineItemT.InputDate AS IDate, LineItemT.InputName, LineItemT.Date AS BDate,  "
			"	LineItemT.Date AS TDate, PaymentsT.PrePayment, "
			"	_PartiallyAppliedPaysQ.Total AS Amt, "
			"	PaymentsT.ProviderID AS ProvID, 'Part' AS RText, 0 AS ApplyID, LineItemT.ID AS LineID, LineItemT.LocationID AS LocID, "
			"	LocationsT.Name AS Location, NULL AS Code, NULL AS CodeName, 0 AS Quantity, NULL AS Category, '' AS CatName, NULL AS ServiceID, "
			"	CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END AS InsResp, "
			"	CASE WHEN PaymentGroupsT.GroupName Is Null THEN '' ELSE PaymentGroupsT.GroupName END AS PayCategory, "
			"	'' AS CPTModifier, '' AS CPTModifier2, '' AS CPTModifier3, '' AS CPTModifier4, "
			"	NULL AS PlaceOfServiceID, '' AS PlaceOfService, NULL AS POSCode, "
			"	CardName, 0 AS TotalTax1, 0 AS TotalTax2, 0 AS IsProduct, "
			"	0 AS SuperBillID, "
			"	'' AS PrimaryInsCo, "
			"	'' AS SecondaryInsCo, "
			"	NULL AS ChargeID, NULL AS BillID, "

			"	NULL AS Calls, '' AS AlbertaClaimNumber, '' AS ReferringPhysicianNPI, "
			"	PaymentPlansT.CheckNo, PaymentPlansT.CCAuthNo, "
			"   0 AS PreTaxAndDiscountLineAmt, "
			"   0 AS DiscountAmt "
			" "
			"FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"	LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"	LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
			"	LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"	LEFT JOIN "
			"		(SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt, Sum(AppliesT.Amount) AS ApplyAmt, "
			"			LineItemT_1.Amount - Sum(AppliesT.Amount) AS Total, LineItemT_1.PatientID "
			"	 "
			"		FROM LineItemT AS LineItemT_1  "
			"			LEFT JOIN PaymentsT ON LineItemT_1.ID = PaymentsT.ID "
			"			LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"			LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
			"	 "
			"		WHERE LineItemT_1.Deleted = 0 "
			"	 "
			"		GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID "
			"	 "
			"		HAVING (LineItemT_1.Amount - Sum(AppliesT.Amount)) <> 0 "
			"		) AS _PartiallyAppliedPaysQ  "
			"	ON LineItemT.ID = _PartiallyAppliedPaysQ.ID "
			" "
			"WHERE _PartiallyAppliedPaysQ.ID Is Not Null AND PaymentsT.Prepayment = 1 AND LineItemT.Deleted = 0 \r\n"
			") ActivityQ \r\n"
			"\r\n"
			"LEFT JOIN PatientsT ON ActivityQ.PatientID = PatientsT.PersonID \r\n"
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID \r\n"
			"LEFT JOIN PersonT PersonProv ON ActivityQ.ProvID = PersonProv.ID \r\n"

			"LEFT JOIN BillDiagCodeFlat12V ON ActivityQ.BillID = BillDiagCodeFlat12V.BillID \r\n "
			"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat12V.ICD9Diag1ID = ICD9T1.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat12V.ICD9Diag2ID = ICD9T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat12V.ICD9Diag3ID = ICD9T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat12V.ICD9Diag4ID = ICD9T4.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T5 ON BillDiagCodeFlat12V.ICD9Diag5ID = ICD9T5.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T6 ON BillDiagCodeFlat12V.ICD9Diag6ID = ICD9T6.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T7 ON BillDiagCodeFlat12V.ICD9Diag7ID = ICD9T7.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T8 ON BillDiagCodeFlat12V.ICD9Diag8ID = ICD9T8.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T9 ON BillDiagCodeFlat12V.ICD9Diag9ID = ICD9T9.ID \r\n "
			"LEFT JOIN DiagCodes ICD9T10 ON BillDiagCodeFlat12V.ICD9Diag10ID = ICD9T10.ID \r\n "

			"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat12V.ICD10Diag1ID = ICD10T1.ID \r\n"
			"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat12V.ICD10Diag2ID = ICD10T2.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat12V.ICD10Diag3ID = ICD10T3.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat12V.ICD10Diag4ID = ICD10T4.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T5 ON BillDiagCodeFlat12V.ICD10Diag5ID = ICD10T5.ID \r\n"
			"LEFT JOIN DiagCodes ICD10T6 ON BillDiagCodeFlat12V.ICD10Diag6ID = ICD10T6.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T7 ON BillDiagCodeFlat12V.ICD10Diag7ID = ICD10T7.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T8 ON BillDiagCodeFlat12V.ICD10Diag8ID = ICD10T8.ID \r\n "
			"LEFT JOIN DiagCodes ICD10T9 ON BillDiagCodeFlat12V.ICD10Diag9ID = ICD10T9.ID \r\n"
			"LEFT JOIN DiagCodes ICD10T10 ON BillDiagCodeFlat12V.ICD10Diag10ID = ICD10T10.ID \r\n "

			"LEFT JOIN ChargeWhichCodesListV ON ActivityQ.ChargeID = ChargeWhichCodesListV.ChargeID \r\n "

			"GROUP BY PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.UserDefinedID, PatientsT.PersonID, \r\n"
			"BDate, TDate, ActivityQ.InputDate, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle, \r\n"
			"ProvID, LocID, ActivityQ.Location, ActivityQ.InputName, ActivityQ.Description, ActivityQ.FinType, \r\n"
			"ActivityQ.Amt, ActivityQ.Code, CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4, \r\n"
			"PlaceOfServiceID, PlaceOfService, POSCode, PayCategory, InsResp, CardName, TotalTax1, TotalTax2, IsProduct, LineID, ActivityQ.ID, \r\n"
			"ActivityQ.SuperBillID, ActivityQ.PrimaryInsCo, ActivityQ.SecondaryInsCo, \r\n"
			"ActivityQ.Quantity, "

			"ICD9T1.CodeNumber, \r\n "
			"ICD9T2.CodeNumber, \r\n "
			"ICD9T3.CodeNumber, \r\n "
			"ICD9T4.CodeNumber, \r\n "
			"ICD9T5.CodeNumber, \r\n "
			"ICD9T6.CodeNumber, \r\n "
			"ICD9T7.CodeNumber, \r\n "
			"ICD9T8.CodeNumber, \r\n "
			"ICD9T9.CodeNumber, \r\n "
			"ICD9T10.CodeNumber, \r\n "

			"ICD10T1.CodeNumber, \r\n "
			"ICD10T2.CodeNumber, \r\n "
			"ICD10T3.CodeNumber, \r\n "
			"ICD10T4.CodeNumber, \r\n "
			"ICD10T5.CodeNumber, \r\n "
			"ICD10T6.CodeNumber, \r\n "
			"ICD10T7.CodeNumber, \r\n "
			"ICD10T8.CodeNumber, \r\n "
			"ICD10T9.CodeNumber, \r\n "
			"ICD10T10.CodeNumber, \r\n "

			"ChargeWhichCodesListV.WhichCodes9, \r\n"
			"ChargeWhichCodesListV.WhichCodes10, \r\n"
			"ChargeWhichCodesListV.WhichCodesBoth, \r\n"

			"ActivityQ.ApplyID, ActivityQ.Calls, ActivityQ.AlbertaClaimNumber, ActivityQ.ReferringPhysicianNPI, \r\n"
			"ActivityQ.CheckNo, ActivityQ.CCAuthNo, \r\n"
			"ActivityQ.ChargeID, ActivityQ.BillID \r\n"
			;

		return _T(strQuery);
	}
	break;

	default:
		ASSERT(FALSE);
		return _T("");
		break;
	}
}
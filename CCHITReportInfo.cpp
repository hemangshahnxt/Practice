// (d.thompson 2010-01-18) - PLID 36927 - Created
#include "stdafx.h"
#include "CCHITReportInfo.h"
#include "CCHITReportMeasures.h"
#include "CCHITReportInfoListing.h"
#include "DateTimeUtils.h"
using namespace ADODB;
using namespace CCHITReportMeasures;


CCCHITReportInfo::CCCHITReportInfo(void)
{
	m_bHasCalculated = false;
	m_crctConfigType = crctNone;
	// (j.jones 2011-04-21 10:48) - PLID 43285 - added flag for whether the query allows use of the snapshot connection
	m_bCanUseSnapshot = true;
	// (j.gruber 2011-10-27 16:33) - PLID 46160 
	m_bUseDefaultDenominator = FALSE;
	//(e.lally 2012-03-21) PLID 48707
	m_nPercentToPass = -1;
	m_nInternalID = -1; //(e.lally 2012-04-03) PLID 48264
	m_bPatientAgeTempTableDeclared = false;
}

//(e.lally 2012-04-24) PLID 48266 - T-SQL variables
CString CCCHITReportInfo::GetFilterDeclarations()
{
	return ""
		"DECLARE @DateFrom datetime;\r\n"
		"DECLARE @DateTo datetime;\r\n"
		"DECLARE @EMRDataGroupID int;\r\n"
		"DECLARE @EMRMasterID int;\r\n"
		"DECLARE @EMRMasterID2 int;\r\n"
		"DECLARE @MailSentCatID int; \r\n "
		"DECLARE @4BusDays datetime; \r\n"
		"DECLARE @2BusDays datetime; \r\n"
		"DECLARE @DateToUse datetime; \r\n";
}

// (r.gonet 06/12/2013) - PLID 55151 - Returns SQL that performs necessary initialization before the numerator and denominator can be run.
// (c.haag 2015-08-31) - PLID 65056 - We can now apply filters to the initialization sql
CString CCCHITReportInfo::GetInitializationSql(COleDateTime dtFrom, COleDateTime dtTo, CString strProviderList, CString strLocationList, CString strExclusionsList, BOOL bExcludeSecondaries, long nPatientID)
{
	CString strInitializationSql;
	if(m_nInternalID == eMUSmokingStatus ||
		m_nInternalID == eMUHeight ||
		m_nInternalID == eMUWeight ||
		m_nInternalID == eMUBloodPressure ||
		m_nInternalID == eMUHeightWeightBP ||
		m_nInternalID == eMUHeightWeight ||
		m_nInternalID == eMUStage2BloodPressure || //TES 10/16/2013 - PLID 59016
		m_nInternalID == eMUStage2HeightWeightBP || //TES 10/16/2013 - PLID 59006
		m_nInternalID == eMUStage2SmokingStatus) //TES 10/16/2013 - PLID 59007
	{
		if(strInitializationSql != "") {
			strInitializationSql += "\r\n";
		}
		// (r.gonet 06/12/2013) - PLID 55151 - These reports use the PatientAge column from EMR. 
		// But converting that column to an integer is intolerably slow. Do it in batch beforehand.
		strInitializationSql += GetPatientAgeTempTableSql();
	}

	if (m_nInternalID == eMUTimelyClinicalSummary)
	{
		// (c.haag 2015-09-11) - PLID 65056 - These reports require temp tables for more maintainable
		// timely clinical summary calculations
		if (strInitializationSql != "") {
			strInitializationSql += "\r\n";
		}
		// (c.haag 2015-09-11) - PLID 66976 - We now pass in the code filter
		CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES_v3", MU_13_DEFAULT_CODES, 0, "<None>", true);
		strInitializationSql += GetClinicalSummaryTempTablesSql(strCodes, 3 /* business days */);
	}

	if (m_nInternalID == eMUStage2ClinicalSummary)
	{
		// (c.haag 2015-09-11) - PLID 65056 - These reports require temp tables for more maintainable
		// timely clinical summary calculations
		if (strInitializationSql != "") {
			strInitializationSql += "\r\n";
		}
		// (c.haag 2015-09-11) - PLID 66976 - We now pass in the code filter
		CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES_v3", MU_13_DEFAULT_CODES, 0, "<None>", true);
		strInitializationSql += GetClinicalSummaryTempTablesSql(strCodes, 1 /* business day */);
	}

	// (j.jones 2014-11-07 11:38) - PLID 63983 - added ability for additional SQL declarations,
	// for SQL member variables, temp. tables, etc.
	if (!m_strInitializationSQL.IsEmpty()) {
		strInitializationSql += m_strInitializationSQL;
		strInitializationSql += "\r\n";
	}

	return ApplyFilters(strInitializationSql, dtFrom, dtTo, strProviderList, strLocationList, strExclusionsList, bExcludeSecondaries, nPatientID);
}

// (c.haag 2015-09-11) - PLID 65056 - Returns the SQL for dropping a temp table
// (c.haag 2015-09-11) - PLID 66976 - Now returns a SQL fragment
CSqlFragment CCCHITReportInfo::GetDropTempTableSql(const CString& strTempTableName)
{
	if (!strTempTableName.IsEmpty())
	{
		return CSqlFragment(R"(
SET NOCOUNT ON;
DROP TABLE {SQL};
SET NOCOUNT OFF;
		)"
		, CSqlFragment(strTempTableName));
	}
	else
	{
		return "";
	}
}

// (r.gonet 06/12/2013) - PLID 55151 - Returns SQL that performs necessary cleanup after the numerator and denominator have been run.
// (c.haag 2015-09-11) - PLID 66976 - Now returns a SQL fragment
CSqlFragment CCCHITReportInfo::GetCleanupSql()
{
	CSqlFragment sqlCleanup;
	if(m_bPatientAgeTempTableDeclared) {
		// (r.gonet 06/12/2013) - PLID 55151 - If we created the patient age temporary table, then drop it here.
		sqlCleanup += GetDropTempTableSql(m_strPatientAgeTempTableName);
	}

	// (c.haag 2015-09-11) - PLID 65056 - Drop the temporary clinical summary tables if they exist
	sqlCleanup += GetDropTempTableSql(m_strClinicalSummaryNumeratorTempTableName);
	sqlCleanup += GetDropTempTableSql(m_strClinicalSummaryDenominatorTempTableName);

	return sqlCleanup;
}

// (j.gruber 2011-11-09 10:13) - PLID 45689 - moved to its own function
//(e.lally 2012-02-24) PLID 48268 - Added nPatientID to filter on a single patient
CString CCCHITReportInfo::ApplyFilters(CString strSql, COleDateTime dtFrom, COleDateTime dtTo, CString strProviderList, CString strLocationList, CString strExclusionsList, BOOL bExcludeSecondaries, long nPatientID)
{

	//Need to prepend the date setup
	//(e.lally 2012-04-24) PLID 48266 - Moved declarations into separate function
	CString strDates = FormatString(
		"SET @DateFrom = '%s';\r\n"
		"SET @DateTo = DATEADD(dd, 1, '%s');\r\n", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));

	//TES 7/24/2013 - PLID 57603 - The Timely Clinical Summary report needs a variable set for 4 business days prior to the end date.
	if(m_nInternalID == eMUTimelyClinicalSummary) {
		strDates += " SET @4BusDays = (SELECT DateAdd(day, -1*(CASE WHEN DateName(dw, @DateTo) = 'Sunday' THEN 5 ELSE "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Saturday' THEN 4 ELSE  "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Friday' THEN 4 ELSE "
		"	  6 END END END), @DateTo));\r\n " ;
	}

	// (r.farnworth 2013-10-15 17:15) - PLID 59573 - Implement MU.CORE.08 for Stage 2
	if(m_nInternalID == eMUStage2ClinicalSummary) {
		strDates += " SET @2BusDays = (SELECT DateAdd(day, -1*(CASE WHEN DateName(dw, @DateTo) = 'Sunday' THEN 3 ELSE "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Monday' THEN 4 ELSE  "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Tuesday' THEN 4 ELSE "
		"	  2 END END END), @DateTo));\r\n ";
	}
	
	// (j.gruber 2011-05-16 16:44) - PLID 43676	
	// (j.gruber 2011-10-25 09:59) - PLID 44755 - added charge filter
	// (j.gruber 2011-11-07 13:03) - PLID 45365 - Exclusions
	//(e.lally 2012-02-24) PLID 48268 - Added strPatientIDFilter to filter on a single patient
	
	CString strProvFilter, strLabProvFilter, strPrescripProvFilter, strPatientProvFilter, strChargeProvFilter, strPatientIDFilter, strInnerProvFilter;
	if (strProviderList == "") {
		//all
		// (j.gruber 2011-09-27 10:08) - PLID 45616 - exclude secondaries
		if (bExcludeSecondaries) {
			strProvFilter = " AND (EMRMasterT.ID NOT IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE DELETED = 0)) ";
			// (d.singleton 2014-09-18 10:52) - PLID 63457 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Detailed
			strInnerProvFilter = " AND (EMRInnerT.ID NOT IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE DELETED = 0)) ";
		}
		else {
			strProvFilter = "";
			strInnerProvFilter = "";
		}
	}
	else {
		// (j.gruber 2011-09-27 10:12) - PLID 45616
		if (bExcludeSecondaries) {
			//emr has no secondary providers
			strProvFilter.Format(" AND (EMRMasterT.ID IN (SELECT EMRID FROM EMRProvidersT WHERE ProviderID IN %s AND DELETED = 0) "
				" AND EMRMasterT.ID NOT IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE DELETED = 0))", strProviderList);
			// (d.singleton 2014-09-18 10:52) - PLID 63457 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Detailed
			strInnerProvFilter.Format(" AND (EMRInnerT.ID IN (SELECT EMRID FROM EMRProvidersT WHERE ProviderID IN %s AND DELETED = 0) "
				" AND EMRInnerT.ID NOT IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE DELETED = 0))", strProviderList);

		}
		else {
			strProvFilter.Format(" AND (EMRMasterT.ID IN (SELECT EMRID FROM EMRProvidersT WHERE ProviderID IN %s AND DELETED = 0) "
				" OR EMRMasterT.ID IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE ProviderID IN %s AND DELETED = 0))", strProviderList, strProviderList);
			// (d.singleton 2014-09-18 10:52) - PLID 63457 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Detailed
			strInnerProvFilter.Format(" AND (EMRInnerT.ID IN (SELECT EMRID FROM EMRProvidersT WHERE ProviderID IN %s AND DELETED = 0) "
				" OR EMRInnerT.ID IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE ProviderID IN %s AND DELETED = 0))", strProviderList, strProviderList);

		}

		strPrescripProvFilter.Format(" AND PatientMedications.ProviderID IN %s ", strProviderList);

		strLabProvFilter.Format(" AND LabsT.ID IN (SELECT LabID FROM LabMultiProviderT WHERE ProviderID IN %s )", strProviderList);

		strPatientProvFilter.Format(" AND PatientsT.MainPhysician IN %s ", strProviderList);
		strChargeProvFilter.Format( " AND ChargesT.DoctorsProviders IN %s ", strProviderList);
	}

// (j.gruber 2011-05-18 10:51) - PLID 43758
	// (j.gruber 2011-10-25 10:00) - PLID 44755 - added charge loctionat
	CString strLocFilter, strLabLocFilter, strPrescripLocFilter, strPatientLocFilter, strChargeLocFilter;
	if (!strLocationList.IsEmpty()) {		
	
		strLocFilter.Format(" AND EMRMasterT.LocationID IN %s ", strLocationList);

		strPrescripLocFilter.Format(" AND PatientMedications.LocationID IN %s ", strLocationList);

		strLabLocFilter.Format(" AND LabsT.LocationID IN %s ", strLocationList);

		strPatientLocFilter.Format(" AND PersonT.Location IN %s ", strLocationList);

		strChargeLocFilter.Format(" AND LineItemT.LocationID IN %s ", strLocationList);
	}

	CString strExcludedFilter;
	if (!strExclusionsList.IsEmpty()) {
		strExcludedFilter.Format(" AND (EMRMasterT.TemplateID IS NULL OR EMRMasterT.TemplateID NOT IN %s )", strExclusionsList);
	}

	//(e.lally 2012-02-24) PLID 48268 - Added ability to filter on a single patient
	if(nPatientID != -1){
		strPatientIDFilter.Format(" AND PatientsT.PersonID = %li ", nPatientID);
	}

	strSql.Replace("[ProvFilter]", strProvFilter);
	// (d.singleton 2014-09-18 10:41) - PLID 63457 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Detailed
	strSql.Replace("[InnerProvFilter]", strInnerProvFilter);

	strSql.Replace("[ProvPrescripFilter]", strPrescripProvFilter);
	strSql.Replace("[LabProvFilter]", strLabProvFilter);	
	strSql.Replace("[PatientProvFilter]", strPatientProvFilter);
	
	// (j.gruber 2011-10-25 10:02) - PLID 44755
	strSql.Replace("[ProvChargeFilter]", strChargeProvFilter);

	// (j.gruber 2011-05-18 10:53) - PLID 43758
	strSql.Replace("[LocFilter]", strLocFilter);
	strSql.Replace("[LocPrescripFilter]", strPrescripLocFilter);
	strSql.Replace("[LabLocFilter]", strLabLocFilter);
	strSql.Replace("[PatientLocFilter]", strPatientLocFilter);
	
	// (j.gruber 2011-10-25 10:02) - PLID 44755
	strSql.Replace("[LocChargeFilter]", strChargeLocFilter);
	
	// (j.gruber 2011-11-07 13:07) - PLID 45365
	strSql.Replace("[ExclusionsFilter]", strExcludedFilter);

	//(e.lally 2012-02-24) PLID 48268 - Added filter for a single patient
	int iPatFiltReplaces = strSql.Replace("[PatientIDFilter]", strPatientIDFilter);
	//We expect all report queries to have a patient filter, so double check that that is still the case
	// (c.haag 2015-09-01) - PLID 65056 - We still expect all report queries to have a patient filter, 
	// but we now take in sql that may have already had it applied previously
	// to the clinical summary
	//ASSERT(iPatFiltReplaces != 0);
	
	//If this is a configurable item, add in the EMRDataGroupID
	// (j.gruber 2010-09-10 14:28) - PLID 40487 - changed to a type
	//(e.lally 2012-04-24) PLID 48266 - Moved sql declarations into separate function
	CString strEMRConfig;
	switch (m_crctConfigType) {
		case crctEMRDataGroup:
			// (j.gruber 2011-11-04 13:20) - PLID 45692 - user internal name
			strEMRConfig.Format("SET @EMRDataGroupID = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n", m_strInternalName);
		break;	

		case crctEMRItem:
			// (j.gruber 2011-11-04 13:20) - PLID 45692 - user internal name
			strEMRConfig.Format("SET @EMRMasterID = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n", m_strInternalName);
		break;

		// (j.gruber 2010-09-14 11:34) - PLID 40514 - allow for multiple selections
		case crctEMRMultiItems:
			// (j.gruber 2011-11-04 13:20) - PLID 45692 - user internal name
			strEMRConfig.Format(
				"SET @EMRMasterID = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
				"SET @EMRMasterID2 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s2');\r\n"
				, m_strInternalName, m_strInternalName);
		break;

		// (j.gruber 2010-09-21 13:49) - PLID 40617 - mailsent category
		case crctMailsentCat:
			// (j.gruber 2011-11-04 13:20) - PLID 45692 - user internal name
			strEMRConfig.Format(" SET @MailSentCatID = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
				, m_strInternalName);
		break;

		// (j.jones 2014-02-05 10:23) - PLID 60651 - we now allow sliders in addition to list & table items
		case crctEMRDataGroupOrSlider:
			//these are mutually exclusive, if both exist (they should not), prefer the MasterID,
			//because if this were to ever happen, the configure interface would select the slider
			strEMRConfig.Format("SET @EMRDataGroupID = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s'); \r\n"
				"SET @EMRMasterID = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s2'); \r\n"
				"IF (IsNull(@EMRDataGroupID,-1) <> -1 AND IsNull(@EMRMasterID,-1) <> -1) \r\n"
				"BEGIN \r\n"
				"	SET @EMRDataGroupID = -1; \r\n "
				"END \r\n",
				m_strInternalName, m_strInternalName);
		break;
	}

	return strDates + strEMRConfig + strSql;

}

// (j.gruber 2011-05-16 16:43) - PLID 43676 - added provider list as parameter
// (j.gruber 2011-05-18 10:51) - PLID 43758 - sdded location list
// (j.gruber 2011-09-27 10:07) - PLID 45616 - exclude secondaries
// (j.gruber 2011-10-27 16:51) - PLID 46160 - support default denoms
// (j.gruber 2011-11-01 11:50) - PLID 46222 - needs to take a connection
// (j.gruber 2011-11-08 09:23) - PLID 45365 - add exclusions
//(e.lally 2012-02-24) PLID 48268 - Added nPatientID to filter on a single patient
void CCCHITReportInfo::Calculate(_ConnectionPtr pCon, COleDateTime dtFrom, COleDateTime dtTo, CString strProviderList, CString strLocationList, CString strExclusionsList, BOOL bExcludeSecondaries, long nDefaultDenominator  /*= -1*/, long nPatientID /* = -1 */)
{
	//If either are empty, blow up
	if(m_strNumSql.IsEmpty() || m_strDenomSql.IsEmpty()) {
		AfxThrowNxException("Numerator and denominator SQL not found, cannot calculate report data.");
	}

	// (j.gruber 2011-11-03 16:04) - PLID 46222 - now that we are using element at instead of get at we need to use a copy of the string here
	//otherwise subsequent runs won't work
	// (r.gonet 06/12/2013) - PLID 55151 - Added initialization and cleanup sql.
	CString tempDeclarations, strInitializationSql, strCleanupSql, tempNum, tempDenom;	

	//(e.lally 2012-04-24) PLID 48266 - Get all the declarations at once so they aren't duplicated in the two queries
	tempDeclarations = GetFilterDeclarations();
	// (r.gonet 06/12/2013) - PLID 55151 - Initialize the initialization sql and cleanup sql.
	// (c.haag 2015-08-31) - PLID 65056 - Apply filters to the initialization sql
	strInitializationSql = GetInitializationSql(dtFrom, dtTo, strProviderList, strLocationList, strExclusionsList, bExcludeSecondaries, nPatientID);
	strCleanupSql = GetCleanupSql().Flatten();

	//(e.lally 2012-02-24) PLID 48268 - Added nPatientID to filter on a single patient
	tempNum = ApplyFilters(m_strNumSql, dtFrom, dtTo, strProviderList, strLocationList, strExclusionsList, bExcludeSecondaries, nPatientID);
	tempDenom = ApplyFilters(m_strDenomSql, dtFrom, dtTo, strProviderList, strLocationList, strExclusionsList, bExcludeSecondaries, nPatientID);

	// (j.gruber 2011-11-01 11:50) - PLID 46219 - since we are using a thread now, the connection has to be passed in
	// (j.jones 2011-04-21 10:48) - PLID 43285 - figure out which connection we can use
	/*_ConnectionPtr pCon = NULL;
	if(!m_bCanUseSnapshot) {
		//this report cannot use the snapshot connection (likely because it references FirstDataBank),
		//so use the regular connection
		pCon = GetRemoteData();
	}
	else {
		//use the snapshot connection
		pCon = GetRemoteDataSnapshot();
	}*/

	// (j.jones 2011-04-21 10:48) - PLID 43285 - increase the timeout on the connection
	// (j.jones 2016-05-26 16:06) - PLID-68563 - now increased to be infinite
	CIncreaseCommandTimeout cict(pCon, 0);

	//Get the numerator first
	// (j.jones 2011-04-21 10:48) - PLID 43285 - use our calculated connection, usually the snapshot connection	
	//(e.lally 2012-04-24) PLID 48266 - Combine the numerator and denominator into one roundtrip
	// (r.gonet 06/12/2013) - PLID 55151 - Added initialization and cleanup sql
	CString strSql = tempDeclarations + "\r\n" + strInitializationSql + "\r\n" + tempNum + "\r\n" + tempDenom + "\r\n" + strCleanupSql;
	_RecordsetPtr prs = CreateRecordsetStd(pCon, strSql);
	try
	{
		if (prs->eof) {
			//The query has to return *something*
			AfxThrowNxException("Numerator calculation returned no results!");
		}
		else if (prs->GetRecordCount() > 1) {
			//The query cannot return more than 1 record.
			AfxThrowNxException("Numerator calculation returned too many results!");
		}
		else {
			m_nNumerator = AdoFldLong(prs, "NumeratorValue");
		}
	}
	catch (...)
	{
		// (c.haag 2015-08-31) - PLID 65056 - Handy place for a breakpoint if a query ever breaks
		throw;
	}

	//Get the denominator next
	// (j.jones 2011-04-21 10:48) - PLID 43285 - use our calculated connection, usually the snapshot connection
	// (j.gruber 2011-05-16 16:44) - PLID 43676 - add providers
	// (j.gruber 2011-10-27 16:52) - PLID 46160 - only do this if we don't have a default
	if (nDefaultDenominator == -1) {
		//(e.lally 2012-04-24) PLID 48266 - Combined queries into one roundtrip
		prs = prs->NextRecordset(NULL);
		if(prs->eof) {
			//The query has to return *something*
			AfxThrowNxException("Denominator calculation returned no results!");
		}
		else if(prs->GetRecordCount() > 1) {
			//The query cannot return more than 1 record.
			AfxThrowNxException("Denominator calculation returned too many results!");
		}
		else {
			m_nDenominator = AdoFldLong(prs, "DenominatorValue");
		}
	}
	else {
		m_nDenominator = nDefaultDenominator;
	}

	//success, set our flag
	m_bHasCalculated = true;
}

double CCCHITReportInfo::GetPercentage()
{
	//You must calculate before getting the percentage
	if(!m_bHasCalculated) {
		AfxThrowNxException("Cannot get percentage without calculating the values first.");
	}

	//Check for divide-by-zero
	if(m_nDenominator == 0) {
		return 0.0;
	}

	//Simple enough, numerator over denominator, * 100 to be a percent
	return ((double)m_nNumerator / (double)m_nDenominator) * 100.0;
}

long CCCHITReportInfo::GetNumerator()
{
	//You must calculate before getting the data
	if(!m_bHasCalculated) {
		AfxThrowNxException("Cannot get numerator without calculating the values first.");
	}

	return m_nNumerator;
}

long CCCHITReportInfo::GetDenominator()
{
	//You must calculate before getting the data
	if(!m_bHasCalculated) {
		AfxThrowNxException("Cannot get denominator without calculating the values first.");
	}

	return m_nDenominator;
}

void CCCHITReportInfo::SetHelpText(CString strGeneral, CString strNum, CString strDenom)
{
	m_strHelpGeneral = strGeneral;
	m_strHelpNum = strNum;
	m_strHelpDenom = strDenom;
}

CString CCCHITReportInfo::GetHelpGeneral()
{
	return m_strHelpGeneral;
}

CString CCCHITReportInfo::GetHelpNum()
{
	return m_strHelpNum;
}

CString CCCHITReportInfo::GetHelpDenom()
{
	return m_strHelpDenom;
}

// (j.gruber 2010-09-10 14:24) - PLID 40487 - changed to be a type
void CCCHITReportInfo::SetConfigureType(CCHITReportConfigType crctType)
{
	m_crctConfigType = crctType;
}

// (j.gruber 2010-09-10 14:24) - PLID 40487 - changed to be a type
CCHITReportConfigType CCCHITReportInfo::GetConfigureType()
{
	return m_crctConfigType;
}

// (j.gruber 2011-05-16 16:45) - PLID 43694 - added report type
void CCCHITReportInfo::SetReportType(CCHITReportType crtType)
{
	m_crtType = crtType;
}

// (j.gruber 2011-05-16 16:45) - PLID 43694 - added report type
CCHITReportType CCCHITReportInfo::GetReportType()
{
	return m_crtType;
}

// (j.gruber 2011-11-01 09:43) - PLID 46160
//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
void CCCHITReportInfo::SetDefaultDenominatorSql()
{
	m_bUseDefaultDenominator = true;
	m_strDenomSql = "/*DEFAULT*/ "
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0  "
		" 		 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] )  "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ";	
}

// (j.gruber 2011-11-08 11:03) - PLID 45689
void CCCHITReportInfo::SetReportInfo(CString strSelect, CString strNumFrom, CString strDenomFrom)
{
	if (!strSelect.IsEmpty() ) {
		m_strReportSelect = strSelect;
	}
	else {
		//set the default
		m_strReportSelect = " SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  ";
	}
	

	if (!strNumFrom.IsEmpty() ) {
		m_strReportNumFrom = strNumFrom;
	}
	else {
		//we really don't have a default numerator 
		ASSERT(FALSE);
	}

	if (!strDenomFrom.IsEmpty() ) {
		m_strReportDenomFrom = strDenomFrom;
	}
	else {
		//set our default
		//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
		m_strReportDenomFrom = " FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ";
	}
}

// (r.gonet 06/12/2013) - PLID 55151 - Retrieves a unique name for the patient age temp table.
CString CCCHITReportInfo::EnsurePatientAgeTempTableName()
{
	if(!m_bPatientAgeTempTableDeclared) {
		m_bPatientAgeTempTableDeclared = true;
		m_strPatientAgeTempTableName.Format("#PatientAgeT_%s", NewUUID(true));
	}

	return m_strPatientAgeTempTableName;
}

// (r.gonet 06/12/2013) - PLID 55151 - Retrieves the SQL that creates and fills the patient age temp table.
CString CCCHITReportInfo::GetPatientAgeTempTableSql()
{
	if(!m_bPatientAgeTempTableDeclared || m_strPatientAgeTempTableName == "") {
		EnsurePatientAgeTempTableName();
	}

	CString strSql = FormatString(
		"CREATE TABLE %s \r\n"
		"( \r\n"
		"	EMRID INT NOT NULL, \r\n"
		"	PatientAgeNumber INT \r\n"
		") \r\n"
		"\r\n",
		m_strPatientAgeTempTableName);

	strSql += FormatString(
		"CREATE NONCLUSTERED INDEX IX_%s_EMRID ON %s(EMRID); \r\n",
		m_strPatientAgeTempTableName.Mid(1), m_strPatientAgeTempTableName);

	strSql += FormatString(
		"CREATE CLUSTERED INDEX IX_%s_PatientAgeNumber ON %s(PatientAgeNumber); \r\n"
		"\r\n",
		m_strPatientAgeTempTableName.Mid(1), m_strPatientAgeTempTableName);

	// (r.gonet 06/12/2013) - PLID 55151 - We used to use IsNumeric(EMRMasterT.PatientAge) directly in the numerator and denominator queries.
	// This was too slow so now we compute the numeric patient age beforehand.
	strSql += FormatString(
		"INSERT INTO %s (EMRID, PatientAgeNumber) \r\n"
		"SELECT EMRMasterT.ID, \r\n"
		"	CASE WHEN EMRMasterT.PatientAge IS NULL \r\n"
		"	THEN NULL \r\n"
		"	WHEN IsNumeric(EMRMasterT.PatientAge + 'e0') = 1 \r\n"
		"	THEN CAST(EmrMasterT.PatientAge AS INT) \r\n"
		"	ELSE 0 END AS PatientAgeNumber \r\n"
		"FROM EMRMasterT \r\n"
		"\r\n",
		m_strPatientAgeTempTableName);

	strSql = 
		"SET NOCOUNT ON; \r\n"
		+ strSql +
		"SET NOCOUNT OFF; \r\n";


	return strSql;
}

// (c.haag 2015-09-11) - PLID 65056 - Ensures we have temp tables names used in clinical summary calculations
void CCCHITReportInfo::EnsureClinicalSummaryTempTableNames()
{
	if (m_strClinicalSummaryNumeratorTempTableName.IsEmpty()) 
	{
		m_strClinicalSummaryNumeratorTempTableName.Format("#ClinicalSummaryNumerator_%s", NewUUID(true));
	}
	if (m_strClinicalSummaryDenominatorTempTableName.IsEmpty())
	{
		m_strClinicalSummaryDenominatorTempTableName.Format("#ClinicalSummaryDenominator_%s", NewUUID(true));
	}
}

// (c.haag 2015-09-11) - PLID 66976 - Add and modify the result of this function to a query to perform MU timely clinical summary calculations.
// The caller is expected to replace the square-bracketed fields with data or {SQL} parameters, and to generate a CSqlFragment object with it.
CString CCCHITReportInfo::GetUnparameterizedClinicalSummaryTempTablesSql()
{
	// Ensure we have the temp table names
	EnsureClinicalSummaryTempTableNames();

	// This creates a temp table of all unique values that make up the denominator of a clinical summary measure.
	// To calculate the denominator we do the following:
	//
	// 1. Find all EMN's with 99xxx visit EMR charges
	// 2. Among all the EMN's we found, we count every unique combination of patient ID, date, primary and second provider as one "point"
	// 3. Find all bills with 99xxx visit charges
	// 4. Among all the bills we found, we count every unique combination of patient ID, date, and provider as one "point"
	// 5. We combine the results from 2 and 4 (charges have no secondary providers so they are treated as null) into a table variable
	//		named strTableVariableName. A combination that exists in both 2 and 4 will only appear once in strTableVariableName.
	// 6. Sort the results by date so that we have a predictable table ordering. This is helpful later when we later try to match up
	//		clinical summaries to the 99xxx visits. If multiple clinical summaries on different dates match with a 99xxx visit, we can
	//		always choose the first 99xxx visit in chronological order.
	//
	CString strSql = R"(
SET NOCOUNT ON

CREATE TABLE {SQL} (ID INT NOT NULL IDENTITY(1,1), PatientID INT NOT NULL, NumeratorID INT, Date DATETIME NOT NULL, PrimaryProviderID INT, SecondaryProviderID INT)
INSERT INTO {SQL} (PatientID, [Date], PrimaryProviderID, SecondaryProviderID)
	SELECT PatientID, [Date], PrimaryProviderID, SecondaryProviderID
	FROM
	(
		-- Select unique entries based on EMR patient, EMR charge date, EMR provider and EMR secondary provider
		SELECT DISTINCT
			EMRMasterT.PatientID,
			EMRMasterT.Date,
			EmrPrimaryProviders.ProviderID AS PrimaryProviderID,
			EMRSecondaryProviders.ProviderID AS SecondaryProviderID
		FROM EMRMasterT 
		INNER JOIN PatientsT ON PatientsT.PersonID = EMRMasterT.PatientID
		INNER JOIN EMRChargesT ON EMRMasterT.ID = EMRChargesT.EMRID 
		INNER JOIN ServiceT ON EMRChargesT.ServiceID = ServiceT.ID 
		INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID 
		LEFT JOIN EmrProvidersT EmrPrimaryProviders ON EmrPrimaryProviders.EmrID = EMRMasterT.ID AND EmrPrimaryProviders.Deleted = CONVERT(BIT,0)
		LEFT JOIN EmrSecondaryProvidersT EmrSecondaryProviders ON EmrSecondaryProviders.EmrID = EMRMasterT.ID AND EmrSecondaryProviders.Deleted = CONVERT(BIT,0)
		WHERE CPTCodeT.Code IN ({SQL}) 
			AND EMRChargesT.Deleted = 0 
			AND EMRMasterT.Deleted = 0 
			AND PatientsT.CurrentStatus <> 4
			AND EMRMasterT.PatientID > 0 
			AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo
			[PatientIDFilter] [ProvFilter] [LocFilter] [ExclusionsFilter]

		UNION

		-- Select unique entries based on charge patient, charge date, and charge provider such that those values match
		-- with any existing EMN
		SELECT DISTINCT
			EMRMasterQ.PatientID,
			EMRMasterQ.Date,
			EMRMasterQ.PrimaryProviderID,
			EMRMasterQ.SecondaryProviderID
		FROM LineItemT 
		INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID 
		INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID 
		INNER JOIN PatientsT ON PatientsT.PersonID = LineItemT.PatientID
		CROSS APPLY
		(
			SELECT TOP 1 EMRMasterT.ID, EMRMasterT.PatientID, EMRMasterT.Date, EMRProvidersT.ProviderID AS PrimaryProviderID, EMRSecondaryProvidersT.ProviderID AS SecondaryProviderID
			FROM EMRMasterT
			INNER JOIN EMRProvidersT ON EMRProvidersT.EmrID = EMRMasterT.ID AND EMRProvidersT.Deleted = CONVERT(BIT,0)
			LEFT JOIN EMRSecondaryProvidersT ON EMRSecondaryProvidersT.EmrID = EMRMasterT.ID AND EMRSecondaryProvidersT.Deleted = CONVERT(BIT,0)
			WHERE EMRMasterT.Deleted = 0
				AND LineItemT.PatientID = EMRMasterT.PatientID
				AND LineItemT.Date = EMRMasterT.Date 
				AND ChargesT.DoctorsProviders = EMRProvidersT.ProviderID
				[PatientIDFilter] [ProvFilter] [LocFilter] [ExclusionsFilter]
			ORDER BY EMRMasterT.ID
		)  EMRMasterQ
		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID 
		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID 
		WHERE (VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL) 
			AND LineItemT.Type = 10
			AND LineItemT.Deleted = 0 
			AND CPTCodeT.Code IN ({SQL}) 
			AND PatientsT.CurrentStatus <> 4
			AND LineItemT.Date >= @DateFrom AND LineItemT.Date < @DateTo
			AND EMRMasterQ.ID IS NOT NULL
			[PatientIDFilter] [ProvChargeFilter] [LocChargeFilter]
	)
	SubQ
	ORDER BY SubQ.[Date]

SET NOCOUNT OFF
)";

	// This creates a temp table of all unique values that make up the numerator clinical summary measure.
	// To calculate the numerator we do the following:
	//
	// 1. Find all EMN's with a clinical summary detail or merged clinical summary in its history tab. Each one is a numerator value.
	// 2. For every denominator in the denominator table variable, look for the first 99xxx visit that matches the patient, date 
	//		range, primary and secondary providers. Then "assign" that numerator value to the denominator value.
	// 3. Delete any numerator values without a denominator value.
	//
	// As a rule, the numerator must never exceed the denominator in any circumstance. This is why we have ID's in the denominator
	// and denominator table variables -- to ensure a (0 or 1)-to-1 mapping. If a denominator matches with multiple numerators, or
	// in other words a 99xxx visit matches with multiple clinical summaries, we choose the oldest first.
	//
	// Other rules:
	// - We only support filtering on on one and only one primary provider
	// - The merge EMN must always take place on the same day of or after the 99xxx EMN
	// - If multiple clinical summaries satisfy a 99xxx visit, the numerator will only increase by 1
	// - If one clinical summary satisfies multiple 99xxx visits, the numerator will increase by the number of 99xxx visits
	//
	strSql += R"(
SET NOCOUNT ON

-- Create and fill a table with numerator candidates
CREATE TABLE {SQL} (ID INT NOT NULL IDENTITY(1,1), PatientID INT NOT NULL, Date DATETIME NOT NULL, PrimaryProviderID INT, SecondaryProviderID INT)
INSERT INTO {SQL} (PatientID, [Date], PrimaryProviderID, SecondaryProviderID)

	-- Select unique entries based on EMR patient, EMR charge date, EMR provider and EMR secondary provider. An EMN
	-- is considered to have a clinical summary if it has a specific EMR detail or a specific kind of MailSent item
	SELECT DISTINCT
		EMRMasterT.PatientID
		,EMRMasterT.Date
		,EmrPrimaryProviders.ProviderID AS PrimaryProviderID
		,EmrSecondaryProviders.ProviderID AS SecondaryProviderID
	FROM EMRMasterT
	INNER JOIN PatientsT ON PatientsT.PersonID = EMRMasterT.PatientID
	LEFT JOIN EmrProvidersT EmrPrimaryProviders ON EmrPrimaryProviders.EmrID = EMRMasterT.ID AND EmrPrimaryProviders.Deleted = CONVERT(BIT,0)
	LEFT JOIN EMRSecondaryProvidersT EmrSecondaryProviders ON EmrSecondaryProviders.EmrID = EMRMasterT.ID AND EmrSecondaryProviders.Deleted = CONVERT(BIT,0)
	LEFT JOIN EMRDetailsT ON EMRDetailsT.EMRID = EMRMasterT.ID 
	LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID 
	WHERE 
		EMRMasterT.Deleted = 0 
		AND PatientsT.CurrentStatus <> 4
		AND EMRMasterT.PatientID > 0 
		AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo
		AND EMRDetailsT.Deleted = 0 
		[PatientIDFilter] [ProvFilter] [LocFilter] [ExclusionsFilter]
		AND 
		( 
			( 
				( 
					EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT}) AND EMRDetailsT.ID IN 
					(
						SELECT EMRSelectT.EMRDetailID FROM EMRSelectT 
						INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID 
						INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID	 
						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID
					) 
				)
				OR 
				(
					EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN 
					(
						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT 
						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID 
						INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 
						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID
					) 
				)
			) 
			OR EMRMasterT.ID IN ( SELECT EMNID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField = 2) 
		) 
	ORDER BY EMRMasterT.Date
		
-- Now assign numerator values to each denominator value where possible
UPDATE DenominatorQ SET NumeratorID = SubQ.NumeratorID
FROM {SQL} DenominatorQ
INNER JOIN
(
	SELECT InnerDenominatorQ.ID AS DenominatorID, MIN(InnerNumeratorQ.ID) AS NumeratorID
	FROM {SQL} InnerDenominatorQ
	INNER JOIN {SQL} InnerNumeratorQ
		-- Match numerator on denominator patientID
		ON InnerNumeratorQ.PatientID = InnerDenominatorQ.PatientID

		-- Match numerator on denominator date range
		AND 
			DATEDIFF(d, InnerDenominatorQ.Date, InnerNumeratorQ.Date) - DATEDIFF(wk, InnerDenominatorQ.Date, InnerNumeratorQ.Date) * 2  
			- CASE  
				WHEN DATENAME(dw, InnerDenominatorQ.Date) <> 'Saturday' AND DATENAME(dw, InnerNumeratorQ.Date) = 'Saturday'  
					THEN 1  
				WHEN DATENAME(dw, InnerDenominatorQ.Date) = 'Saturday' AND DATENAME(dw, InnerNumeratorQ.Date) <> 'Saturday'  
					THEN -1  
				ELSE 0  
			END >= 0 
		AND 
			DATEDIFF(d, InnerDenominatorQ.Date, InnerNumeratorQ.Date) - DATEDIFF(wk, InnerDenominatorQ.Date, InnerNumeratorQ.Date) * 2  
			- CASE  
				WHEN DATENAME(dw, InnerDenominatorQ.Date) <> 'Saturday' AND DATENAME(dw, InnerNumeratorQ.Date) = 'Saturday'  
					THEN 1  
				WHEN DATENAME(dw, InnerDenominatorQ.Date) = 'Saturday' AND DATENAME(dw, InnerNumeratorQ.Date) <> 'Saturday'  
					THEN -1  
				ELSE 0  
			END <= {INT}
		AND InnerDenominatorQ.Date > 
		( 
			SELECT DateAdd(day, - 1 * ( 
				CASE 
					WHEN DateName(dw, InnerNumeratorQ.Date) = 'Sunday' 
						THEN 5 
					ELSE CASE 
							WHEN DateName(dw,  InnerNumeratorQ.Date) = 'Saturday' 
								THEN 4 
							ELSE CASE 
									WHEN DateName(dw,  InnerNumeratorQ.Date) = 'Friday' 
										THEN 4 
									ELSE 6 
									END 
							END 
					END 
				)
				,  InnerNumeratorQ.Date
			) 
		) 
		-- Match numerator on denominator primary provider
		AND ISNULL(InnerNumeratorQ.PrimaryProviderID,-1) = ISNULL(InnerDenominatorQ.PrimaryProviderID,-1)

		-- Match numerator on denominator secondary provider with this exception: If the denominator element
		-- (99xxx visit) has a primary provider and no secondary provider, then a numerator element (clinical summary)
		-- can have a primary provider and any secondary provider
		AND 
		(
			( ISNULL(InnerNumeratorQ.SecondaryProviderID, -1) = ISNULL(InnerDenominatorQ.SecondaryProviderID, -1) )
			OR 
			( InnerNumeratorQ.SecondaryProviderID IS NOT NULL AND InnerDenominatorQ.SecondaryProviderID IS NULL )
		)

	GROUP BY InnerDenominatorQ.ID
) 
SubQ ON SubQ.DenominatorID = DenominatorQ.ID

-- If you have two denominator values that only differ by secondary provider, it's entirely possible to have
-- them both claim the same numerator. Look for, and null out those cases.
UPDATE DenominatorQ SET NumeratorID = NULL
FROM {SQL} DenominatorQ
WHERE 
	NumeratorID IS NOT NULL
	AND NumeratorID IN (SELECT NumeratorID FROM {SQL} WHERE NumeratorID IS NOT NULL GROUP BY NumeratorID HAVING COUNT(*) > 1)
AND SecondaryProviderID IS NULL

-- Clear out any numerator values that don't actually correspond to a denominator value
DELETE FROM {SQL} WHERE ID NOT IN (SELECT ISNULL(NumeratorID,-1) FROM {SQL})

SET NOCOUNT OFF
)";

	return strSql;
}

// (c.haag 2015-09-11) - PLID 65056 - Add the result of this function to a query to perform MU timely clinical summary calculations
// Takes in the number of business days as a parameter since different reports have different metrics
// Only one temp table may exist per CCHITReportInfo object, so choose your business days wisely
// (c.haag 2015-09-11) - PLID 66976 - We now require the caller to pass in the code filter as well
CString CCCHITReportInfo::GetClinicalSummaryTempTablesSql(const CString& strCodes, long nBusinessDays)
{
	// (c.haag 2015-09-11) - PLID 66976 - Get the SQL that contains both SQL fragment parameters as well as special
	// parameters (such as "[LocFilter]") that will be replaced by other functions before the query is run. You may think
	// "Well this is really strange, why not just use SQL strings instead of fragments from start to end?" The answer is
	// that GetUnparameterizedClinicalSummaryTempTablesSql is shared by the detailed report which runs on fragments
	// and the summary report does not support fragments.
	CString strSql = GetUnparameterizedClinicalSummaryTempTablesSql();

	CSqlFragment fragment = CSqlFragment(strSql

		// Denominator values
		, CSqlFragment(m_strClinicalSummaryDenominatorTempTableName), CSqlFragment(m_strClinicalSummaryDenominatorTempTableName)
		, CSqlFragment(strCodes), CSqlFragment(strCodes)

		// Numerator values
		, CSqlFragment(m_strClinicalSummaryNumeratorTempTableName), CSqlFragment(m_strClinicalSummaryNumeratorTempTableName)
		, eitSingleList, eitMultiList, eitTable

		, CSqlFragment(m_strClinicalSummaryDenominatorTempTableName), CSqlFragment(m_strClinicalSummaryDenominatorTempTableName)
		, CSqlFragment(m_strClinicalSummaryNumeratorTempTableName)
		, nBusinessDays

		, CSqlFragment(m_strClinicalSummaryDenominatorTempTableName), CSqlFragment(m_strClinicalSummaryDenominatorTempTableName)

		, CSqlFragment(m_strClinicalSummaryNumeratorTempTableName), CSqlFragment(m_strClinicalSummaryDenominatorTempTableName)
		);

	return fragment.Flatten();
}


// (c.haag 2015-09-11) - PLID 65056 - Returns the timely clinical summary numerator temp table name
CString CCCHITReportInfo::GetClinicalSummaryNumeratorTempTableName()
{
	EnsureClinicalSummaryTempTableNames();
	return m_strClinicalSummaryNumeratorTempTableName;
}

// (c.haag 2015-09-11) - PLID 65056 - Returns the timely clinical summary denominator temp table name
CString CCCHITReportInfo::GetClinicalSummaryDenominatorTempTableName()
{
	EnsureClinicalSummaryTempTableNames();
	return m_strClinicalSummaryDenominatorTempTableName;
}
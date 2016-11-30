#include "stdafx.h"
#include "MUMeasure_CORE_13.h"
#include "CCHITReportInfoListing.h"
#include "CCHITReportInfo.h"

// (j.dinatale 2012-10-24 14:40) - PLID 53505 - Clinical Summaries

CMUMeasure_CORE_13::CMUMeasure_CORE_13(void)
{
}

CMUMeasure_CORE_13::~CMUMeasure_CORE_13(void)
{
}

CSqlFragment CMUMeasure_CORE_13::GetMeasureSql()
{
	// (c.haag 2015-09-11) - PLID 66976 - This is the only difference between MUMeasure_CORE_13 and MUMeasure2_CORE_08's
	// call to GetMeasureSql() other than the table variable name.
	int nBusinessDays = 3;

	// (c.haag 2015-09-11) - PLID 66976 - The clinical summary numerator and denominator queries are now in CCHITReportInfo
	CCCHITReportInfo riReport;
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES", MU_13_DEFAULT_CODES, 0, "<None>", true);

	//TES 7/24/2013 - PLID 57603 - Updated to check for EMNs up to 3 business days after the qualifying service code
	//(s.dhole 7/25/2014 10:49 AM ) - PLID 63037 Change query to return only those Emn Which hase Code or same date as bill using Codes(99XXX) 
	// (b.savon 2014-05-06 10:17) - PLID 62038 - This is really the EMRInfoMasterID, not the DataGroupID
	// (s.dhole 2014-06-3 07:53) - PLID 62274 - exclude voide charges
	// (d.singleton 2014-09-05 10:05) - PLID 63468 - MU Core 13 (Clinical Summary) - Bills not filtering on Provider - Detailed
	//(s.dhole 9/26/2014 8:55 AM ) - PLID 63765 update query with common  function
	// (c.haag 2015-09-11) - PLID 66976 - We now use code from CCCHITReportInfo to generate the report SQL.
	CString strClinicalSummaryTempTables = riReport.GetUnparameterizedClinicalSummaryTempTablesSql();

	// PatientIDFilter is for individual patient filtering, which we have no concept of here
	strClinicalSummaryTempTables.Replace("[PatientIDFilter]", "");
	// Set up fragment EMR filters
	strClinicalSummaryTempTables.Replace("[ProvFilter]", "{SQL}"); // EMR providers
	strClinicalSummaryTempTables.Replace("[LocFilter]", "{SQL}"); // EMR location
	strClinicalSummaryTempTables.Replace("[ExclusionsFilter]", "{SQL}"); // EMR exclusions
	// Set up fragment charge filters
	strClinicalSummaryTempTables.Replace("[ProvChargeFilter]", "{SQL}"); // Provider charges
	strClinicalSummaryTempTables.Replace("[LocChargeFilter]", "{SQL}"); // Charge locations

	CSqlFragment sqlNum = riReport.GetClinicalSummaryNumeratorTempTableName();
	CSqlFragment sqlDenom = riReport.GetClinicalSummaryDenominatorTempTableName();
	CSqlFragment sqlCleanup = riReport.GetCleanupSql();
	CSqlFragment sqlClinicalSummaryTempTables(strClinicalSummaryTempTables
		// Denominator values
		, sqlDenom, sqlDenom
		, CSqlFragment(strCodes)
		// Denominator EMR filters
		, m_filterPatients.GenerateEMRProviderFilter(), m_filterPatients.GenerateEMRLocationFilter(), m_filterPatients.GenerateEMRTemplateExclusions()
		, m_filterPatients.GenerateEMRProviderFilter(), m_filterPatients.GenerateEMRLocationFilter(), m_filterPatients.GenerateEMRTemplateExclusions()
		// Denominator values
		, CSqlFragment(strCodes)
		// Denominator charge filters
		, m_filterPatients.GenerateProviderFilter("ChargesT.DoctorsProviders"), m_filterPatients.GenerateLocationFilter("LineItemT.LocationID")

		// Numerator values
		, sqlNum, sqlNum
		// Numerator EMR filters
		, m_filterPatients.GenerateEMRProviderFilter(), m_filterPatients.GenerateEMRLocationFilter(), m_filterPatients.GenerateEMRTemplateExclusions()
		// Numerator constants
		, eitSingleList, eitMultiList, eitTable

		// Denominator update values
		, sqlDenom, sqlDenom, sqlNum
		, nBusinessDays

		// Denominator update values
		, sqlDenom, sqlDenom

		// Numerator clearing out values
		, sqlNum, sqlDenom
		);


	CString strSql = R"(
SET NOCOUNT ON; 	

DECLARE @DateFrom DATETIME, @DateTo DATETIME
SET @DateFrom = {OLEDATETIME}; 
SET @DateTo = DATEADD(dd, 1, {OLEDATETIME}); 

DECLARE @EMRMasterID INT; 
SET @EMRMasterID = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); 

{SQL}

SET NOCOUNT ON;
DECLARE @Core13Data TABLE 
( 
	PersonID INT, 
	Qualified INT 
); 
		
INSERT INTO @Core13Data 
	SELECT PatientID,
		CASE WHEN NumeratorID IS NULL THEN 0 ELSE 1 END
FROM {SQL}

SET NOCOUNT OFF; 
SELECT PersonID, SUM(Qualified) AS Qualified, COUNT(*) AS Total FROM @Core13Data GROUP BY PersonID 

{SQL}
)";

	CSqlFragment sqlResult = CSqlFragment(strSql,
		m_filterMURange.m_dtFromDate,
		m_filterMURange.m_dtToDate,
		("CCHITReportInfo_" + GetInternalName()),
		sqlClinicalSummaryTempTables,
		sqlDenom,
		sqlCleanup
		);

	return sqlResult;
}


MU::MeasureData CMUMeasure_CORE_13::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core13Data;
	Core13Data.MeasureID = MU::MU_CORE_13;
	Core13Data.nDenominator = 0;
	Core13Data.nNumerator = 0;
	Core13Data.dblRequiredPercent = GetRequirePrecent();
	Core13Data.strFullName = GetFullName();
	Core13Data.strShortName = GetShortName();
	Core13Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo ClinSummary = MU::DataPointInfo("Clin. Summary", MU::RawMeasure);

	
	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);

			personData.nNumerator = AdoFldLong(prsDetails, "Qualified", 0);
			personData.nDenominator = AdoFldLong(prsDetails, "Total", 0);

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::RawMeasure;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li/%li", personData.nNumerator, personData.nDenominator);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			ClinSummary.nNumerator += personData.nNumerator;
			ClinSummary.nDenominator += personData.nDenominator;

			Core13Data.MeasureInfo.push_back(personData);
			Core13Data.nDenominator += personData.nDenominator;
			Core13Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core13Data.DataPointInfo.push_back(ClinSummary);	
 
	return Core13Data;
}
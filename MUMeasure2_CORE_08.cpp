#include "stdafx.h"
#include "MUMeasure2_CORE_08.h"
#include "CCHITReportInfoListing.h"
#include "CCHITReportInfo.h"

// (r.farnworth 2013-10-15 15:01) - PLID 59573 - MU.CORE.08

CMUMeasure2_CORE_08::CMUMeasure2_CORE_08(void)
{
}

CMUMeasure2_CORE_08::~CMUMeasure2_CORE_08(void)
{
}

CSqlFragment CMUMeasure2_CORE_08::GetMeasureSql()
{
	// (c.haag 2015-09-11) - PLID 66976 - This is the only difference between MUMeasure_CORE_13 and MUMeasure2_CORE_08's
	// call to GetMeasureSql() other than the table variable name.
	int nBusinessDays = 1;

	// (c.haag 2015-09-11) - PLID 66976 - The clinical summary numerator and denominator queries are in CCHITReportInfo
	CCCHITReportInfo riReport;
	// (r.farnworth 2013-10-15 16:55) - PLID 59573 - We will likely use the same codes since the measures are so similar
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES", MU_13_DEFAULT_CODES, 0, "<None>", true);

	// (r.farnworth 2013-10-15 15:06) - PLID 59573 - Created and copied from MUMeasure_CORE_13
	// (b.savon 2014-05-06 10:01) - PLID 59573 - This is really the EMRInfoMasterID, not the DataGroupID
	// (b.savon 2014-05-06 10:01) - PLID 59573 - This needs to compare the *actual* visit dates; not the reporting period date
	// (s.dhole 2014-06-04 07:03) - PLID 62293 - Exclude voide charges
	// (d.singleton 2014-09-11 17:04) - PLID 63467 -  MU2 Core 8 (clinical summaries) - Bills not filtering on Provider - Detailed
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
DECLARE @Core08Data TABLE 
( 
	PersonID INT, 
	Qualified INT 
); 
		
INSERT INTO @Core08Data 
	SELECT PatientID,
		CASE WHEN NumeratorID IS NULL THEN 0 ELSE 1 END
FROM {SQL}

SET NOCOUNT OFF; 
SELECT PersonID, SUM(Qualified) AS Qualified, COUNT(*) AS Total FROM @Core08Data GROUP BY PersonID 

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

MU::MeasureData CMUMeasure2_CORE_08::GetMeasureInfo()
{
	// (r.farnworth 2013-10-15 15:06) - PLID 59573 - Created and copied from MUMeasure_CORE_13
	// create a measure data
	MU::MeasureData Core08Data;
	Core08Data.MeasureID = MU::MU2_CORE_08;
	Core08Data.nDenominator = 0;
	Core08Data.nNumerator = 0;
	Core08Data.dblRequiredPercent = GetRequirePrecent();
	Core08Data.strFullName = GetFullName();
	Core08Data.strShortName = GetShortName();
	Core08Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo ClinSummary = MU::DataPointInfo("Clin. Summary", MU::RawMeasure);
	
	// create recordset
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

			Core08Data.MeasureInfo.push_back(personData);
			Core08Data.nDenominator += personData.nDenominator;
			Core08Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core08Data.DataPointInfo.push_back(ClinSummary);	
 
	return Core08Data;
}

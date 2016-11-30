#include "stdafx.h"
#include "MUMeasure2_MENU_02.h"
#include "CCHITReportInfoListing.h" // (s.dhole 2014-05-20 14:56) - PLID 59581 
// (r.farnworth 2013-10-29 14:56) - PLID 59581 - Implement Stage 2 MU.MENU.02

CMUMeasure2_MENU_02::CMUMeasure2_MENU_02(void)
{
}

CMUMeasure2_MENU_02::~CMUMeasure2_MENU_02(void)
{
}

// (s.dhole 2014-05-20 14:56) - PLID 59581 update SQL to code to supprot void charges and filters
// (d.singleton 2014-09-11 16:40) - PLID 63459 - MU2 Menu 2 (Progress Notes) - Bills not filtering on Provider - Detailed
CSqlFragment CMUMeasure2_MENU_02::GetMeasureSql()
{
	
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES", MU_13_DEFAULT_CODES, 0, "<None>", true);
	//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 This query return denominator 
	//  Return all emns who have provider filter  and  visit codes  OR Those emn who have provider filter and Emn date matches with Bill(Same provider filter) with Visit code
	CSqlFragment strCommonDenominatorSQL = GetClinicalSummaryCommonDenominatorSQL(strCodes);

	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "
		"DECLARE @CORE02_Data table (\r\n"
		" PersonID int, NumeratorValue int, DenominatorValue int \r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE02_Data\r\n"
		"SELECT PatientID, "
		" Sum(COALESCE(NumeratorValue, 0)) as NumeratorValue, Sum(COALESCE(DenominatorValue, 0)) as DenominatorValue \r\n "	
		"FROM\r\n"
	);

	CSqlFragment f(str);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	f += CSqlFragment(		
		" INNER JOIN  ( "
		" SELECT PersonT.ID AS PatientID, Count(*) AS NumeratorValue, 0 AS DenominatorValue "
		" FROM PersonT "
		" INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" INNER JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID "
		" INNER JOIN {SQL} ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date "
		" WHERE EMRMasterT.ID IN "
		" ( "
		"	SELECT EMRMasterT.ID FROM EMRMasterT INNER JOIN EMRTopicsT ON EMRTopicsT.EMRID = EMRMasterT.ID "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	{SQL} "
		"	AND (EMRTopicsT.ShowIfEmpty = 1 OR EXISTS (SELECT * FROM EMRDetailsT WHERE EMRDetailsT.EMRTopicID = EMRTopicsT.ID)) "
		" ) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 "
		" GROUP BY PersonT.ID "
 		" UNION "
		" SELECT PersonT.ID AS PatientID, 0 AS NumeratorValue, Count(*) AS DenominatorValue "
		" FROM PersonT "
		" INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" INNER JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID "
		" INNER JOIN {SQL} ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date "
		" WHERE EMRMasterT.ID IN "
		" (SELECT EMRMasterT.ID FROM EMRMasterT "
		" WHERE EMRMasterT.Deleted = 0 "
		" {SQL} "
		" {SQL} ) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 "
		" GROUP BY PersonT.ID "
		" ) CountQ ON MUPatientEMRBaseQ.PersonID = CountQ.PatientID "
		" GROUP BY CountQ.PatientID \r\n ",
		strCommonDenominatorSQL,
		m_filterMURange.GenerateEMRFilter_ForPatientBaseMeasures(),
		strCommonDenominatorSQL,
		m_filterMURange.GenerateDateFilter("EMRMasterT.Date"),
		m_filterMURange.GenerateLocationFilter("EMRMasterT.LocationID")
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT * FROM @CORE02_Data \r\n ");

	return f;
	
}

MU::MeasureData CMUMeasure2_MENU_02::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu02Data;
	Menu02Data.MeasureID = MU::MU2_MENU_02;
	Menu02Data.nDenominator = 0;
	Menu02Data.nNumerator = 0;
	Menu02Data.dblRequiredPercent = GetRequirePrecent();
	Menu02Data.strFullName = GetFullName();
	Menu02Data.strShortName = GetShortName();
	Menu02Data.strInternalName = GetInternalName();

	// possible data points
	// (s.dhole 2014-05-20 14:56) - PLID 59581 
	MU::DataPointInfo Measure = MU::DataPointInfo("Elec. Note", MU::RawMeasure);
	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	_variant_t varMeasure = g_cvarNull;	

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			long nNum = AdoFldLong(prsDetails, "NumeratorValue");
			long nDenom = AdoFldLong(prsDetails, "DenominatorValue");
			
			personData.nNumerator = nNum > 0 ? 1 : 0;
			personData.nDenominator = nDenom > 0 ? 1 : 0;

			Measure.nDenominator += personData.nDenominator;
			Measure.nNumerator += personData.nNumerator;
			
			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::RawMeasure;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%s", nNum > 0 ? "Yes" : "");
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			Menu02Data.MeasureInfo.push_back(personData);
			Menu02Data.nDenominator += personData.nDenominator;
			Menu02Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Menu02Data.DataPointInfo.push_back(Measure);	
 
	return Menu02Data;

}
#include "stdafx.h"
#include "MUMeasure2_CORE_17.h"
// (r.farnworth 2013-10-25 12:27) - PLID 59580 - Implement MU.CORE.17 for Stage 2

CMUMeasure2_CORE_17::CMUMeasure2_CORE_17(void)
{
}

CMUMeasure2_CORE_17::~CMUMeasure2_CORE_17(void)
{
}

CSqlFragment CMUMeasure2_CORE_17::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "
		"DECLARE @CORE17Data table (\r\n" /*+ */
		"	PersonID int, Denominator INT, Numerator INT \r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE17Data\r\n"
		"SELECT MUAllPatientsQ.PersonID, "
		" CASE WHEN IncludedPatsQ.PersonID IS NULL THEN 0 ELSE 1 END as Denominator, \r\n "	
		" CASE WHEN NumQ.PersonID IS NULL THEN 0 ELSE 1 END as Numerator \r\n "	
		"FROM (\r\n"
		);

	CSqlFragment f(str);

	f += GetAllMUPatients();

	//first let's join to the denominator
	f += CSqlFragment(	
		" ) MUAllPatientsQ \r\n "
		" INNER JOIN  ( "
		" SELECT PersonT.ID as PersonID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.Deleted = 0 AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 "
		" ) IncludedPatsQ ON MUAllPatientsQ.PersonID = IncludedPatsQ.PersonID \r\n"
		"    \r\n "
	);

	//now the numerator
	f += CSqlFragment (
		" LEFT JOIN ( \r\n "
		" SELECT PersonT.ID as PersonID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "	
		" INNER JOIN Notes on Notes.PersonID = PersonT.ID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.Deleted = 0 AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 "
		" AND Notes.IsPatientCreated = 1 {SQL} "
		" ) NumQ ON IncludedPatsQ.PersonID = NumQ.PersonID "
		, m_filterMURange.GenerateDateFilter("Notes.Date")
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT PersonID, Max(Numerator) as Numerator, Max(Denominator) as Denominator FROM @CORE17Data GROUP BY PersonID \r\n ");

	CString str1 = f.Flatten();

	return f;
}

MU::MeasureData CMUMeasure2_CORE_17::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core17Data;
	Core17Data.MeasureID = MU::MU2_CORE_17;
	Core17Data.nDenominator = 0;
	Core17Data.nNumerator = 0;
	Core17Data.dblRequiredPercent = GetRequirePrecent();
	Core17Data.strFullName = GetFullName();
	Core17Data.strShortName = GetShortName();
	Core17Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Measure = MU::DataPointInfo("Messaging", MU::RawMeasure);
	
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
			long nNum = AdoFldLong(prsDetails, "Numerator");
			long nDenom = AdoFldLong(prsDetails, "Denominator");
			
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
			
			Core17Data.MeasureInfo.push_back(personData);
			Core17Data.nDenominator += personData.nDenominator;
			Core17Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core17Data.DataPointInfo.push_back(Measure);	
 
	return Core17Data;
}

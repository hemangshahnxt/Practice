#include "stdafx.h"
#include "MUMeasure_CORE_06.h"
// (j.gruber 2012-10-25 09:40) - PLID 53528
CMUMeasure_CORE_06::CMUMeasure_CORE_06(void)
{
}

CMUMeasure_CORE_06::~CMUMeasure_CORE_06(void)
{
}

CSqlFragment CMUMeasure_CORE_06::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	

		"DECLARE @CORE06Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, AllergyCount int \r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE06Data \r\n"
		"SELECT MUPatientEMRBaseQ.PersonID, "
		" AllergyQ.AllergyCount \r\n "	
		"FROM \r\n"
	);

	CSqlFragment f(str);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	// (j.jones 2012-11-05 10:24) - PLID 53564 - added support for PatientsT.HasNoAllergies, which indicates that they have no allergies (who would have guessed?!?)
	f += CSqlFragment(		
		" LEFT JOIN  ( "
		"SELECT PersonT.ID AS PersonID, Count(*) as AllergyCount FROM PersonT "
		" INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN (SELECT PersonID FROM PatientAllergyT WHERE Discontinued = 0) AS PatientAllergyQ ON PersonT.ID = PatientAllergyQ.PersonID  "
		" WHERE (PatientAllergyQ.PersonID Is Not Null OR PatientsT.HasNoAllergies = 1) "
		" GROUP BY PersonT.ID"
		" ) AllergyQ ON MUPatientEMRBaseQ.PersonID = AllergyQ.PersonID "		
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT PersonID, Min(AllergyCount) as AllergyCount FROM @CORE06Data GROUP BY PersonID \r\n ");

	return f;
}

MU::MeasureData CMUMeasure_CORE_06::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core06Data;
	Core06Data.MeasureID = MU::MU_CORE_06;
	Core06Data.nDenominator = 0;
	Core06Data.nNumerator = 0;
	Core06Data.dblRequiredPercent = GetRequirePrecent();
	Core06Data.strFullName = GetFullName();
	Core06Data.strShortName = GetShortName();
	Core06Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Allergy = MU::DataPointInfo("Allergy", MU::RawMeasure);	

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			long nAllergyCount = AdoFldLong(prsDetails, "AllergyCount", 0);
			
			personData.nNumerator = nAllergyCount > 0? 1 : 0;
			personData.nDenominator = 1;

			Allergy.nDenominator += personData.nDenominator;
			Allergy.nNumerator += personData.nNumerator;
			
			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::RawMeasure;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li", nAllergyCount);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			Core06Data.MeasureInfo.push_back(personData);
			Core06Data.nDenominator += personData.nDenominator;
			Core06Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core06Data.DataPointInfo.push_back(Allergy);	
 
	return Core06Data;
}
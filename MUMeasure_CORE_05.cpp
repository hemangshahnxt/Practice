#include "stdafx.h"
#include "MUMeasure_CORE_05.h"
// (j.gruber 2012-10-25 10:12) - PLID 53529
CMUMeasure_CORE_05::CMUMeasure_CORE_05(void)
{
}

CMUMeasure_CORE_05::~CMUMeasure_CORE_05(void)
{
}

CSqlFragment CMUMeasure_CORE_05::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	

		"DECLARE @CORE05Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, MedCount int \r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE05Data \r\n"
		"SELECT MUPatientEMRBaseQ.PersonID, "
		" MedsQ.MedCount \r\n "	
		"FROM \r\n"
	);

	CSqlFragment f(str);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	// (j.jones 2012-11-05 09:57) - PLID 53563 - added support for PatientsT.HasNoMeds, which indicates that they have no current meds
	f += CSqlFragment(		
		" LEFT JOIN  ( "
		" SELECT PersonT.ID as PersonID, Count(*) as MedCount FROM PersonT "
		" INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN (SELECT PatientID FROM CurrentPatientMedsT WHERE Discontinued = 0) AS CurrentPatientMedsQ ON PersonT.ID = CurrentPatientMedsQ.PatientID  "
		" WHERE (CurrentPatientMedsQ.PatientID Is Not Null OR PatientsT.HasNoMeds = 1) "
		" GROUP BY PersonT.ID"
		" ) MedsQ ON MUPatientEMRBaseQ.PersonID = MedsQ.PersonID "		
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT PersonID, Min(MedCount) as MedCount FROM @CORE05Data GROUP BY PersonID \r\n ");

	return f;
}

MU::MeasureData CMUMeasure_CORE_05::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core05Data;
	Core05Data.MeasureID = MU::MU_CORE_05;
	Core05Data.nDenominator = 0;
	Core05Data.nNumerator = 0;
	Core05Data.dblRequiredPercent = GetRequirePrecent();
	Core05Data.strFullName = GetFullName();
	Core05Data.strShortName = GetShortName();
	Core05Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Med = MU::DataPointInfo("Meds", MU::RawMeasure);	

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			long nMedCount = AdoFldLong(prsDetails, "MedCount", 0);
			
			personData.nNumerator = nMedCount > 0? 1 : 0;
			personData.nDenominator = 1;

			Med.nDenominator += personData.nDenominator;
			Med.nNumerator += personData.nNumerator;
			
			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::RawMeasure;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li", nMedCount);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			Core05Data.MeasureInfo.push_back(personData);
			Core05Data.nDenominator += personData.nDenominator;
			Core05Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core05Data.DataPointInfo.push_back(Med);	
 
	return Core05Data;
}
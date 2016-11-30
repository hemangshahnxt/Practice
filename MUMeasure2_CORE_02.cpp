#include "stdafx.h"
#include "MUMeasure2_CORE_02.h"
#include "PrescriptionUtilsNonAPI.h"
// (r.farnworth 2013-11-18 12:30) - PLID 59566 - Implement Detailed Reporting for MU.CORE.02 for Stage 2

using namespace ADODB;

CMUMeasure2_CORE_02::CMUMeasure2_CORE_02(void)
{
}

CMUMeasure2_CORE_02::~CMUMeasure2_CORE_02(void)
{
}

CSqlFragment CMUMeasure2_CORE_02::GetMeasureSql()
{
	// (j.jones 2016-02-05 10:54) - PLID 67981 - added DispensedInHouse as a valid status,
	// and switched the hardcoded statuses to use the enums
	return CSqlFragment(
		"SET NOCOUNT ON "
		"DECLARE @PatientMeds TABLE "
		"( "
		"	PersonID INT, "
		"	IsElectronicMed INT "
		");"

		"INSERT INTO @PatientMeds "
		"SELECT "
		"PatientMedications.PatientID AS PersonID, "
		"CASE "
		"	WHEN ( "
		"	(NewCropGUID IS NOT NULL AND FinalDestinationType IN (2,3,4) AND NewCropFormularyChecked = 1) "
		"	OR (NewCropGUID IS NULL AND QueueStatus IN ({CONST_INT}, {CONST_INT}, {CONST_INT}, {CONST_INT}) AND SureScriptsEligibilityDetailID IS NOT NULL ) "
		"	) "
		"		THEN 1 "
		"	ELSE 0 "
		"END AS IsElectronicMed "
		"FROM "
		"PatientMedications "
		"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"INNER JOIN ("
		"	SELECT DISTINCT PersonID FROM "
		"	({SQL}) SubQ "
		") PatientFilterQ ON PatientMedications.PatientID = PatientFilterQ.PersonID "
		"WHERE "
		"PatientMedications.Deleted = 0 "
		"AND PatientMedications.Discontinued = 0 "
		//(s.dhole 8/1/2014 3:23 PM ) - PLID  63046 -  Exlude void prescriptions 
		" AND PatientMedications.QueueStatus <>14 "
		"{SQL} "
		"AND RTRIM(LTRIM(DEASchedule)) = ''; "
		""
		"SET NOCOUNT OFF "
		""
		"SELECT PersonID, "
		"COUNT(*) AS TotalMeds, "
		"COALESCE(SUM(IsElectronicMed), 0) AS ElectronicMeds  "
		"FROM "
		"@PatientMeds "
		"GROUP BY PersonID;",
		(long)PrescriptionQueueStatus::pqseTransmitSuccess, (long)PrescriptionQueueStatus::pqseTransmitPending, (long)PrescriptionQueueStatus::pqseFaxed, (long)PrescriptionQueueStatus::pqseDispensedInHouse,
		GetAllMUPatients(MU::UnionType::ePrescribe),
		m_filterMURange.GenerateFieldFilter("PatientMedications.PrescriptionDate", "PatientMedications.ProviderID", "PatientMedications.LocationID")
	);
}

MU::MeasureData CMUMeasure2_CORE_02::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core02Data;
	Core02Data.MeasureID = MU::MU2_CORE_02;
	Core02Data.nDenominator = 0;
	Core02Data.nNumerator = 0;
	Core02Data.dblRequiredPercent = GetRequirePrecent();
	Core02Data.strFullName = GetFullName();
	Core02Data.strShortName = GetShortName();
	Core02Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo eRx = MU::DataPointInfo("eRx", MU::eRx);

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			personData.nNumerator = AdoFldLong(prsDetails, "ElectronicMeds", 0);
			personData.nDenominator = AdoFldLong(prsDetails, "TotalMeds", 0);

			eRx.nDenominator += personData.nDenominator;
			eRx.nNumerator += personData.nNumerator;

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::eRx;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li/%li", personData.nNumerator, personData.nDenominator);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);

			Core02Data.MeasureInfo.push_back(personData);
			Core02Data.nDenominator += personData.nDenominator;
			Core02Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core02Data.DataPointInfo.push_back(eRx);
 
	return Core02Data;
}

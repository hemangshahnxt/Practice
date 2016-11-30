#include "stdafx.h"
#include "MUMeasure_CORE_04.h"
#include "PrescriptionUtilsNonAPI.h"

// (j.dinatale 2012-10-24 14:38) - PLID 53507 - eRx measures

using namespace ADODB;

CMUMeasure_CORE_04::CMUMeasure_CORE_04(void)
{
}

CMUMeasure_CORE_04::~CMUMeasure_CORE_04(void)
{
}

CSqlFragment CMUMeasure_CORE_04::GetMeasureSql()
{
	// TODO: Fix GetPatientBaseQuery because it is definitely wrong...
	// (r.farnworth 2014-05-06 10:51) - PLID 61907 - Fix Meaningful Use Detailed Reporting measure for MU.CORE.04 for Stage 1
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
		"	(NewCropGUID IS NOT NULL AND FinalDestinationType IN (2,3,4)) "
		"	OR (NewCropGUID IS NULL AND QueueStatus IN ({CONST_INT}, {CONST_INT}, {CONST_INT}, {CONST_INT})) "
		"	) "
		"		THEN 1 "
		"	ELSE 0 "
		"END AS IsElectronicMed "
		"FROM "
		"PatientMedications "
		"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"INNER JOIN ("
		"	SELECT DISTINCT PersonID FROM "
		// patients seen filter
		"	({SQL}) SubQ "
		") PatientFilterQ ON PatientMedications.PatientID = PatientFilterQ.PersonID "
		"WHERE "
		"PatientMedications.Deleted = 0 "
		"AND PatientMedications.Discontinued = 0 "
		"{SQL} "
		"AND DEASchedule NOT IN ('II', 'III', 'IV', 'V'); "
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

MU::MeasureData CMUMeasure_CORE_04::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core04Data;
	Core04Data.MeasureID = MU::MU_CORE_04;
	Core04Data.nDenominator = 0;
	Core04Data.nNumerator = 0;
	Core04Data.dblRequiredPercent = GetRequirePrecent();
	Core04Data.strFullName = GetFullName();
	Core04Data.strShortName = GetShortName();
	Core04Data.strInternalName = GetInternalName();

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

			Core04Data.MeasureInfo.push_back(personData);
			Core04Data.nDenominator += personData.nDenominator;
			Core04Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core04Data.DataPointInfo.push_back(eRx);
 
	return Core04Data;
}
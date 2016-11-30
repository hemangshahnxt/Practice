#include "stdafx.h"
#include "MUMeasure2_CORE_01A.h"
// (r.farnworth 2013-11-18 12:22) - PLID 59563 - Implement Detailed Report for MU.CORE.01.A for Stage 2
// (r.farnworth 2014-04-30 15:32) - PLID 59563 - 11400 Clean-up

using namespace ADODB;

CMUMeasure2_CORE_01A::CMUMeasure2_CORE_01A(void)
{
}

CMUMeasure2_CORE_01A::~CMUMeasure2_CORE_01A(void)
{
}

CSqlFragment CMUMeasure2_CORE_01A::GetMeasureSql()
{
	// (r.farnworth 2014-06-04 16:37) - PLID 62325 - Query changes to better sync with new base query
	return CSqlFragment(
		"SET NOCOUNT ON "
		" DECLARE @MailSentCatID_MU_CORE_01A INT; \r\n"
		" SET @MailSentCatID_MU_CORE_01A = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING});\r\n"
		"DECLARE @PatientMeds TABLE "
		"( "
		"PersonID INT,"
		"HistoryScripts INT, "
		"MedicationScripts INT "
		"); "
		"" 
		"INSERT INTO @PatientMeds "
		"SELECT "
		"Q.PersonID, "
		"PrescriptionH AS HistoryScripts, "
		"PrescriptionM AS MedicationScripts "
		"FROM( "
		"SELECT Count(*) AS PrescriptionM, 0 AS PrescriptionH, PatientID AS PersonID FROM PatientMedications "
		"WHERE DELETED = 0 AND Discontinued = 0 "
		"{SQL} {SQL} "
		"AND PatientMedications.PatientID IN(SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0) "
		"GROUP BY PatientID "
		"UNION "
		"SELECT 0 AS PrescriptionM, Count(*) AS PrescriptionH, PersonID FROM MailSent "
		"		INNER JOIN ( "
		"			SELECT PersonT.ID AS PatientID "
		"			FROM "
		"			PersonT "
		"			INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"			WHERE PersonT.ID > 0 AND PatientsT.CurrentStatus <> 4 "
		"			{SQL} "
		"			{SQL} "
		"			{SQL} "
		"			{SQL} "
		"		) PatFilterQ ON MailSent.PersonID = PatFilterQ.PatientID "
		"		WHERE "
		"		MailSent.CategoryID = @MailSentCatID_MU_CORE_01A "
		"		{SQL} "
		"		{SQL} "
		"GROUP BY PersonID "
		")Q "
		"INNER JOIN ("
		"	SELECT DISTINCT PersonID FROM "
		// patients seen filter
		"	({SQL}) SubQ "
		") PatientFilterQ ON Q.PersonID = PatientFilterQ.PersonID "
		""
		" SET NOCOUNT OFF "
		""
		" SELECT "
		" PersonID, "
		" COALESCE(SUM(HistoryScripts), 0) + COALESCE(SUM(MedicationScripts), 0) AS TotalMeds, "
		" COALESCE(SUM(MedicationScripts), 0) AS ElectronicMeds "
		" FROM "
		" @PatientMeds "
		" GROUP BY PersonID ",
		"CCHITReportInfo_" + GetInternalName(),
		m_filterMURange.GenerateFieldFilter("PatientMedications.PrescriptionDate", "PatientMedications.ProviderID", "PatientMedications.LocationID"),
		m_filterPatients.GenerateFieldFilter("PatientMedications.PrescriptionDate", "PatientMedications.ProviderID", "PatientMedications.LocationID"),
		m_filterMURange.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterMURange.GenerateLocationFilter("PersonT.Location"),
		m_filterPatients.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterPatients.GenerateLocationFilter("PersonT.Location"),
		m_filterMURange.GenerateDateFilter("MailSent.ServiceDate"),
		m_filterPatients.GenerateDateFilter("MailSent.ServiceDate"),
		GetAllMUPatients(MU::UnionType::CPOEMedicationsStage2)
		);
}


MU::MeasureData CMUMeasure2_CORE_01A::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core01A_Data;
	Core01A_Data.MeasureID = MU::MU2_CORE_01A;
	Core01A_Data.nDenominator = 0;
	Core01A_Data.nNumerator = 0;
	Core01A_Data.dblRequiredPercent = GetRequirePrecent();
	Core01A_Data.strFullName = GetFullName();
	Core01A_Data.strShortName = GetShortName();
	Core01A_Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo CPOE = MU::DataPointInfo("CPOE(Medication)", MU::CPOE);

	// create recordset
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if (prsDetails){
		while (!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			personData.nNumerator = AdoFldLong(prsDetails, "ElectronicMeds", 0);
			personData.nDenominator = AdoFldLong(prsDetails, "TotalMeds", 0);

			CPOE.nDenominator += personData.nDenominator;
			CPOE.nNumerator += personData.nNumerator;

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::CPOE;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li/%li", personData.nNumerator, personData.nDenominator);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);

			Core01A_Data.MeasureInfo.push_back(personData);
			Core01A_Data.nDenominator += personData.nDenominator;
			Core01A_Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core01A_Data.DataPointInfo.push_back(CPOE);

	return Core01A_Data;
}
#include "stdafx.h"
#include "MUMeasure2_CORE_01C.h"
// (r.farnworth 2013-11-18 12:28) - PLID 59565 - Implement Detailed Reporting for MU.CORE.01.C for Stage 2

CMUMeasure2_CORE_01C::CMUMeasure2_CORE_01C(void)
{
}

CMUMeasure2_CORE_01C::~CMUMeasure2_CORE_01C(void)
{
}

CSqlFragment CMUMeasure2_CORE_01C::GetMeasureSql()
{
	// (r.farnworth 2014-06-04 16:37) - PLID 62325 - Query changes to better sync with new base query
	return CSqlFragment(
		"SET NOCOUNT ON "
		" DECLARE @MailSentCatID_MU_CORE_01C INT; \r\n"
		" SET @MailSentCatID_MU_CORE_01C = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING});\r\n"
		"DECLARE @PatientLabs TABLE "
		"( "
		"PersonID INT,"
		"HistoryLabs INT, "
		"PracticeLabs INT "
		"); "
		""
		"INSERT INTO @PatientLabs "
		"SELECT "
		"Q.PersonID, "
		"LabsH AS HistoryLabs, "
		"LabsP AS PracticeLabs "
		"FROM( "
		"SELECT Count(*) AS LabsP, 0 AS LabsH, PatientID AS PersonID FROM LabsT "
		"INNER JOIN LabMultiProviderT ON LabsT.ID = LabMultiProviderT.LabID "
		"WHERE DELETED = 0 AND Discontinued = 0 AND Type IN (1,2,3) "
		"{SQL} {SQL} {SQL} {SQL} {SQL} {SQL} "
		"AND LabsT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0) "
		"GROUP BY PatientID "
		"UNION ALL "
		"SELECT 0 AS LabsP, Count(*) AS LabsH, PersonID FROM MailSent "
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
		"		MailSent.CategoryID = @MailSentCatID_MU_CORE_01C "
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
		" COALESCE(SUM(HistoryLabs), 0) + COALESCE(SUM(PracticeLabs), 0) AS TotalLabs, "
		" COALESCE(SUM(PracticeLabs), 0) AS ElectronicLabs "
		" FROM "
		" @PatientLabs "
		" GROUP BY PersonID ",
		"CCHITReportInfo_" + GetInternalName(),
		m_filterMURange.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterMURange.GenerateDateFilter("LabsT.InputDate"),
		m_filterMURange.GenerateLocationFilter("LabsT.LocationID"),
		m_filterPatients.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterPatients.GenerateDateFilter("LabsT.InputDate"),
		m_filterPatients.GenerateLocationFilter("LabsT.LocationID"),
		m_filterMURange.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterMURange.GenerateLocationFilter("PersonT.Location"),
		m_filterPatients.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterPatients.GenerateLocationFilter("PersonT.Location"),
		m_filterMURange.GenerateDateFilter("MailSent.ServiceDate"),
		m_filterPatients.GenerateDateFilter("MailSent.ServiceDate"),
		GetAllMUPatients(MU::UnionType::CPOELaboratories)
		);
}


MU::MeasureData CMUMeasure2_CORE_01C::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core01C_Data;
	Core01C_Data.MeasureID = MU::MU2_CORE_01C;
	Core01C_Data.nDenominator = 0;
	Core01C_Data.nNumerator = 0;
	Core01C_Data.dblRequiredPercent = GetRequirePrecent();
	Core01C_Data.strFullName = GetFullName();
	Core01C_Data.strShortName = GetShortName();
	Core01C_Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo CPOE = MU::DataPointInfo("CPOE(Laboratory)", MU::CPOE);	
	
	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			personData.nNumerator = AdoFldLong(prsDetails, "ElectronicLabs", 0);
			personData.nDenominator = AdoFldLong(prsDetails, "TotalLabs", 0);

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::CPOE;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li/%li", personData.nNumerator, personData.nDenominator);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);

			CPOE.nNumerator += personData.nNumerator;
			CPOE.nDenominator += personData.nDenominator;

			Core01C_Data.MeasureInfo.push_back(personData);
			Core01C_Data.nDenominator += personData.nDenominator;
			Core01C_Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core01C_Data.DataPointInfo.push_back(CPOE);
	return Core01C_Data;
}
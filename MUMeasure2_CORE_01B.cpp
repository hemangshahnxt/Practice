#include "stdafx.h"
#include "MUMeasure2_CORE_01B.h"
// (r.farnworth 2013-11-18 12:26) - PLID 59564 - Implement Detailed Reporting for MU.CORE.01.B for Stage 2

CMUMeasure2_CORE_01B::CMUMeasure2_CORE_01B(void)
{
}

CMUMeasure2_CORE_01B::~CMUMeasure2_CORE_01B(void)
{
}

CSqlFragment CMUMeasure2_CORE_01B::GetMeasureSql()
{
	// (r.farnworth 2014-06-04 16:37) - PLID 62325 - Query changes to better sync with new base query
	return CSqlFragment(
		"SET NOCOUNT ON "
		" DECLARE @MailSentCatID_MU_CORE_01B INT; \r\n"
		" SET @MailSentCatID_MU_CORE_01B = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING});\r\n"
		"DECLARE @PatientRads TABLE "
		"( "
		"PersonID INT,"
		"HistoryRadiology INT, "
		"RadiologyLabs INT "
		"); "
		""
		"INSERT INTO @PatientRads "
		"SELECT "
		"Q.PersonID, "
		"RadiologyH AS HistoryRadiology, "
		"RadiologyL AS RadiologyLabs "
		"FROM( "
		"SELECT Count(*) AS RadiologyL, 0 AS RadiologyH, PatientID AS PersonID FROM LabsT "
		"INNER JOIN LabMultiProviderT ON LabsT.ID = LabMultiProviderT.LabID "
		"WHERE DELETED = 0 AND Discontinued = 0 AND Type = 4 "
		"{SQL} {SQL} {SQL} {SQL} {SQL} {SQL} "
		"AND LabsT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0) "
		"GROUP BY PatientID "
		"UNION ALL "
		"SELECT 0 AS RadiologyL, Count(*) AS RadiologyH, PersonID FROM MailSent "
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
		"		MailSent.CategoryID = @MailSentCatID_MU_CORE_01B "
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
		" COALESCE(SUM(HistoryRadiology), 0) + COALESCE(SUM(RadiologyLabs), 0) AS TotalRads, "
		" COALESCE(SUM(RadiologyLabs), 0) AS ElectronicRads "
		" FROM "
		" @PatientRads "
		" GROUP BY PersonID ",
		"CCHITReportInfo_" + GetInternalName(),
		m_filterMURange.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterMURange.GenerateDateFilter("LabsT.BiopsyDate"),
		m_filterMURange.GenerateLocationFilter("LabsT.LocationID"),
		m_filterPatients.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterPatients.GenerateDateFilter("LabsT.BiopsyDate"),
		m_filterPatients.GenerateLocationFilter("LabsT.LocationID"),
		m_filterMURange.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterMURange.GenerateLocationFilter("PersonT.Location"),
		m_filterPatients.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterPatients.GenerateLocationFilter("PersonT.Location"),
		m_filterMURange.GenerateDateFilter("MailSent.ServiceDate"),
		m_filterPatients.GenerateDateFilter("MailSent.ServiceDate"),
		GetAllMUPatients(MU::UnionType::CPOERadiologies)
		);
}


MU::MeasureData CMUMeasure2_CORE_01B::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core01BData;
	Core01BData.MeasureID = MU::MU2_CORE_01B;
	Core01BData.nDenominator = 0;
	Core01BData.nNumerator = 0;
	Core01BData.dblRequiredPercent = GetRequirePrecent();
	Core01BData.strFullName = GetFullName();
	Core01BData.strShortName = GetShortName();
	Core01BData.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo CPOE = MU::DataPointInfo("CPOE(Radiology)", MU::CPOE);	

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			personData.nNumerator =AdoFldLong(prsDetails, "ElectronicRads");
			personData.nDenominator =  AdoFldLong(prsDetails, "TotalRads");

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

			Core01BData.MeasureInfo.push_back(personData);
			Core01BData.nDenominator += personData.nDenominator;
			Core01BData.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core01BData.DataPointInfo.push_back(CPOE);	
 
	return Core01BData;
}
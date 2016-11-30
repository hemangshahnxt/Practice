#include "stdafx.h"
#include "MUMeasure2_MENU_03.h"
// (b.savon 2014-05-14 11:17) - PLID 59582 - Implement Detailed Reporting for MU.MENU.03 for Stage 2

CMUMeasure2_MENU_03::CMUMeasure2_MENU_03(void)
{
}

CMUMeasure2_MENU_03::~CMUMeasure2_MENU_03(void)
{
}

CSqlFragment CMUMeasure2_MENU_03::GetMeasureSql()
{
	
	CString str;
	str.Format(
		" SET NOCOUNT ON;  "	
		" DECLARE @MailSentCatID_MU2_MENU_03 INT;  "
		" SET @MailSentCatID_MU2_MENU_03 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');"
		"DECLARE @Menu03_Data table (" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, LabsCount INT, TotalCount INT "
		" ); "

		"INSERT INTO @Menu03_Data "
		"SELECT MUAllPatientsQ.PersonID, "
		" LabsQ.LabsCount, LabsQ.TotalCount  "	
		"FROM (",
		GetInternalName()	);

	CSqlFragment f(str);

	f += GetAllMUPatients(MU::UnionType::ImagingResults);

	//Then join the interesting stuff from this measure...
	// (r.farnworth 2014-06-04 16:37) - PLID 62325 - Query changes to better sync with new base query
	f += CSqlFragment(
		" ) MUAllPatientsQ "
		" INNER JOIN  ( "
		"SELECT PatientID as PersonID, Sum(LabsCount) as LabsCount, Sum(TotalCount) as TotalCount FROM ( "
		" SELECT PatientID, Count(*) AS LabsCount, 0 as TotalCount, 1 as Type  "
		"   FROM LabsT  "
		"	INNER JOIN ( "
		"		SELECT DISTINCT LabID FROM LabMultiProviderT WHERE (1=1) {SQL} {SQL}"
		"	) LabProvQ ON LabsT.ID = LabProvQ.LabID "
		"	INNER JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
		"	INNER JOIN MailSent ON LabResultsT.MailID = MailSent.MailID "
		"   WHERE   "
		"	LabsT.Deleted = 0 AND LabsT.Discontinued = 0 AND LabsT.Type = 4  "
		"	{SQL} {SQL}"
		"	{SQL} {SQL}"
		"	AND LabsT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0 ) "
		"   GROUP BY PatientID  "
		"	UNION ALL "

		"	SELECT PatientID, "
		"	0 AS LabsCount, "
		"	Count(*) AS TotalCount, "
		"	1 AS TYPE "
		"	FROM LabsT "
		"	INNER JOIN ( "
		"		SELECT DISTINCT LabID FROM LabMultiProviderT WHERE(1 = 1) {SQL} {SQL}"
		"	) LabProvQ ON LabsT.ID = LabProvQ.LabID "
		"	WHERE LabsT.Deleted = 0 "
		"	AND LabsT.Discontinued = 0 "
		"	AND LabsT.TYPE = 4 "
		"   {SQL} {SQL}"
		"   {SQL} {SQL}"
		"	AND LabsT.PatientID IN "
		"	(SELECT PersonID "
		"	FROM PatientsT "
		"	WHERE CurrentStatus <> 4 "
		"	AND PersonID > 0) "
		"	GROUP BY PatientID "
		"	UNION ALL "

		"	SELECT Mailsent.PersonID AS PatientID, "
		"	0 AS LabsCount, "
		"	Count(*) AS TotalCount, "
		"	1 AS TYPE "
		"		FROM MailSent "
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
		"		MailSent.CategoryID = @MailSentCatID_MU2_MENU_03 "
		"		{SQL} "
		"		{SQL} "
		"	GROUP BY Mailsent.PersonID "
		"  )SubQ GROUP BY SubQ.PatientID  "
		" ) LabsQ ON MUAllPatientsQ.PersonID = LabsQ.PersonID "
		"     ",
		m_filterMURange.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterPatients.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterMURange.GenerateDateFilter("LabsT.BiopsyDate"),
		m_filterPatients.GenerateDateFilter("LabsT.BiopsyDate"),
		m_filterMURange.GenerateLocationFilter("LabsT.LocationID"),
		m_filterPatients.GenerateLocationFilter("LabsT.LocationID"),
		m_filterMURange.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterPatients.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterMURange.GenerateDateFilter("LabsT.BiopsyDate"),
		m_filterPatients.GenerateDateFilter("LabsT.BiopsyDate"),
		m_filterMURange.GenerateLocationFilter("LabsT.LocationID"),
		m_filterPatients.GenerateLocationFilter("LabsT.LocationID"),
		m_filterMURange.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterPatients.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterMURange.GenerateLocationFilter("PersonT.Location"),
		m_filterPatients.GenerateLocationFilter("PersonT.Location"),
		m_filterMURange.GenerateDateFilter("MailSent.ServiceDate"),
		m_filterPatients.GenerateDateFilter("MailSent.ServiceDate")
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; "
		""
		"SELECT * FROM @Menu03_Data  ");

	CString str1 = f.Flatten();
	return f;
}


MU::MeasureData CMUMeasure2_MENU_03::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu03Data;
	Menu03Data.MeasureID = MU::MU2_MENU_03;
	Menu03Data.nDenominator = 0;
	Menu03Data.nNumerator = 0;
	Menu03Data.dblRequiredPercent = GetRequirePrecent();
	Menu03Data.strFullName = GetFullName();
	Menu03Data.strShortName = GetShortName();
	Menu03Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo ImageResults = MU::DataPointInfo("Imaging Results", MU::CPOE);	

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);

			personData.nNumerator = AdoFldLong(prsDetails, "LabsCount", 0);
			personData.nDenominator = AdoFldLong(prsDetails, "TotalCount", 0);

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::CPOE;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li/%li", personData.nNumerator, personData.nDenominator);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			ImageResults.nNumerator += personData.nNumerator;
			ImageResults.nDenominator += personData.nDenominator;

			Menu03Data.MeasureInfo.push_back(personData);
			Menu03Data.nDenominator += personData.nDenominator;
			Menu03Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Menu03Data.DataPointInfo.push_back(ImageResults);	
 
	return Menu03Data;
}

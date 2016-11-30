#include "stdafx.h"
#include "MUMeasure_MENU_02.h"
// (j.gruber 2012-10-25 09:16) - PLID 53524
CMUMeasure_MENU_02::CMUMeasure_MENU_02(void)
{
}

CMUMeasure_MENU_02::~CMUMeasure_MENU_02(void)
{
}

CSqlFragment CMUMeasure_MENU_02::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	
		" DECLARE @MailsentCatID_MU_MENU_02 INT; \r\n "
		" SET @MailsentCatID_MU_MENU_02 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
		"DECLARE @Menu02Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, MetLabCount INT, LabCount INT \r\n"
		");\r\n\r\n"

		"INSERT INTO @Menu02Data \r\n"
		"SELECT MUAllPatientsQ.PersonID, "
		" LabsQ.MetLabCount, LabsQ.LabCount \r\n "	
		"FROM (\r\n",
		GetInternalName()	);

	CSqlFragment f(str);

	f += GetAllMUPatients(MU::UnionType::LabResults);

	//Then join the interesting stuff from this measure...
	// (r.farnworth 2014-04-21 13:48) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (b.savon 2014-05-07 09:28) - PLID 62051 - Fix Stage 1  - Clinical Lab Results Summary and Detailed Report so
	// that the denominator includes all labs; not only the ones with a Flag set or a first character numeric value
	// Added a UNION to count the *actual* labs in the numerator
	// (b.savon 2014-05-07 10:50) - PLID 62064 - Trim leading/trailing spaces for LabResultsT.Value
	// (r.farnworth 2014-06-04 16:37) - PLID 62325 - Query changes to better sync with new base query
	f += CSqlFragment(
		" ) MUAllPatientsQ "
		" INNER JOIN  ( "
		"SELECT PatientID as PersonID, Sum(MetLabs) as MetLabCount, Sum(LabCount) as LabCount FROM \r\n "
		" ( SELECT PatientID, Count(*) AS MetLabs,\r\n "
		"  0 as LabCount, 1 as Type \r\n "
		"  FROM LabsT \r\n "
		"	INNER JOIN ( "
		"		SELECT DISTINCT LabID FROM LabMultiProviderT WHERE (1=1) {SQL} {SQL} "
		"	) LabProvQ ON LabsT.ID = LabProvQ.LabID "
		"   WHERE  \r\n "
		"   LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 "
		"	 {SQL} {SQL} "		
		"	 AND (LabResultsT.FlagID IS NOT NULL "
		"	 OR CASE WHEN isnumeric(convert(nvarchar, Left(CONVERT(nVarChar(255), LTRIM(RTRIM(LabResultsT.Value))), 1)) + 'e0') <> 0 THEN 1 ELSE 0 END = 1) "
		") "
		"   AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0 \r\n "
		"	{SQL} {SQL} \r\n"
		"  AND LabsT.LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) \r\n "
		"   GROUP BY PatientID \r\n "
		"   UNION  \r\n "
		"SELECT PatientID, 0 AS MetLabs, \r\n "
		"  Count(*) as LabCount, 1 as Type \r\n "
		"  FROM LabsT \r\n "
		"	INNER JOIN ( "
		"		SELECT DISTINCT LabID FROM LabMultiProviderT WHERE (1=1) {SQL} {SQL} "
		"	) LabProvQ ON LabsT.ID = LabProvQ.LabID "
		"   WHERE  \r\n "
		"   LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 "
		"    AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0 \r\n "
		"	 {SQL} {SQL} "
		") "
		"	{SQL} {SQL} \r\n"
		"  AND LabsT.LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) \r\n "
		"   GROUP BY PatientID \r\n "
		"   UNION  \r\n "
		"   SELECT Mailsent.PersonID, 0 as MetLabCount, Count(*) as LabCount, 2 as Type "
		"	FROM MailSent \r\n "
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
		"		MailSent.CategoryID = @MailsentCatID_MU_MENU_02 \r\n "
		"		{SQL} "
		"		{SQL} "
		"   GROUP BY MailSent.PersonID \r\n "
		"  )SubQ GROUP BY SubQ.PatientID \r\n "
		" ) LabsQ ON MUAllPatientsQ.PersonID = LabsQ.PersonID \r\n"
		"    \r\n ",
		m_filterMURange.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterPatients.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterMURange.GenerateDateFilter("LabResultsT.DateReceived"),
		m_filterPatients.GenerateDateFilter("LabResultsT.DateReceived"),
		m_filterMURange.GenerateLocationFilter("LabsT.LocationID"),
		m_filterPatients.GenerateLocationFilter("LabsT.LocationID"),
		m_filterMURange.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterPatients.GenerateProviderFilter("LabMultiProviderT.ProviderID"),
		m_filterMURange.GenerateDateFilter("LabsT.BiopsyDate"),
		m_filterPatients.GenerateDateFilter("LabsT.BiopsyDate"),
		m_filterMURange.GenerateLocationFilter("LabsT.LocationID"),
		m_filterPatients.GenerateLocationFilter("LabsT.LocationID"),
		m_filterMURange.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterMURange.GenerateLocationFilter("PersonT.Location"),
		m_filterPatients.GenerateProviderFilter("PatientsT.MainPhysician"),
		m_filterPatients.GenerateLocationFilter("PersonT.Location"),
		m_filterMURange.GenerateDateFilter("MailSent.ServiceDate"),
		m_filterPatients.GenerateDateFilter("MailSent.ServiceDate")
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT * FROM @Menu02Data \r\n ");

	CString str1 = f.Flatten();
	return f;
}

MU::MeasureData CMUMeasure_MENU_02::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu02Data;
	Menu02Data.MeasureID = MU::MU_MENU_02;
	Menu02Data.nDenominator = 0;
	Menu02Data.nNumerator = 0;
	Menu02Data.dblRequiredPercent = GetRequirePrecent();
	Menu02Data.strFullName = GetFullName();
	Menu02Data.strShortName = GetShortName();
	Menu02Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Labs = MU::DataPointInfo("Labs", MU::Labs);

	CString strSql = GetMeasureSql().Flatten();
	
	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			personData.nNumerator = AdoFldLong(prsDetails, "MetLabCount", 0);
			personData.nDenominator = AdoFldLong(prsDetails, "LabCount", 0);

			Labs.nDenominator += personData.nDenominator;
			Labs.nNumerator += personData.nNumerator;

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::Labs;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li/%li", personData.nNumerator, personData.nDenominator);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);

			Menu02Data.MeasureInfo.push_back(personData);
			Menu02Data.nDenominator += personData.nDenominator;
			Menu02Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Menu02Data.DataPointInfo.push_back(Labs);
	return Menu02Data;
}
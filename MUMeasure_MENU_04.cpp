#include "stdafx.h"
#include "MUMeasure_MENU_04.h"
// (j.gruber 2012-10-25 09:14) - PLID 53523

CMUMeasure_MENU_04::CMUMeasure_MENU_04(void)
{
}

CMUMeasure_MENU_04::~CMUMeasure_MENU_04(void)
{
}
//(s.dhole 9/4/2014 4:32 PM ) - PLID 62794 Added additional union to display remindersent data 
CSqlFragment CMUMeasure_MENU_04::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	
		" DECLARE @EMRDataGroupID_MU_MENU_04 INT; \r\n "
		" SET @EMRDataGroupID_MU_MENU_04 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
		"DECLARE @04Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, Denominator INT, Numerator INT \r\n"
		");\r\n\r\n"

		"INSERT INTO @04Data \r\n"
		"SELECT MUAllPatientsQ.PersonID, "
		" CASE WHEN IncludedPatsQ.PersonID IS NULL THEN 0 ELSE 1 END as Denominator, \r\n "	
		" CASE WHEN NumQ.PatientID IS NULL THEN 0 ELSE 1 END as Numerator \r\n "	
		"FROM ( \r\n",
		GetInternalName()	);

	CSqlFragment f(str);

	f += GetAllMUPatients(MU::UnionType::RemindersStage1);
	//(s.dhole 9/4/2014 4:32 PM ) - PLID 62794 rollback union to display remindersent data 
	//first let's join to the denominator
	f += CSqlFragment(	
		" ) MUAllPatientsQ \r\n "
		" INNER JOIN  ( "
		" SELECT PersonT.ID as PersonID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" WHERE PersonT.ID IN  " 
		" (SELECT PatientID FROM EMRMasterT WHERE Deleted = 0 {SQL}  "
		" ) "
		" AND CurrentStatus <> 4 AND PersonID > 0 "
		" AND ((CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, {OLEDATETIME}) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH({OLEDATETIME}) OR (MONTH(PersonT.Birthdate) = MONTH({OLEDATETIME})  "
		" AND DAY(PersonT.Birthdate) > DAY({OLEDATETIME}))  "
		" THEN 1 ELSE 0 END END) <= 5 OR " 
		" (CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, {OLEDATETIME}) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH({OLEDATETIME}) OR (MONTH(PersonT.Birthdate) = MONTH({OLEDATETIME})  "
		" AND DAY(PersonT.Birthdate) > DAY({OLEDATETIME}))  "
		" THEN 1 ELSE 0 END END) >= 65) "
		" ) IncludedPatsQ ON MUAllPatientsQ.PersonID = IncludedPatsQ.PersonID \r\n"
		"    \r\n "
		, m_filterMURange.GenerateEMRFilter_ForPatientBaseMeasures(FALSE)
		, m_filterMURange.m_dtFromDate
		, m_filterMURange.m_dtFromDate
		, m_filterMURange.m_dtFromDate
		, m_filterMURange.m_dtFromDate
		, m_filterMURange.m_dtToDate
		, m_filterMURange.m_dtToDate
		, m_filterMURange.m_dtToDate
		, m_filterMURange.m_dtToDate		
	);

	//now the numerator
	f += CSqlFragment (
		" LEFT JOIN ( \r\n "
		" SELECT PatientID FROM EMRMasterT \r\n"
		" WHERE EMRMasterT.DELETED = 0 {SQL}  \r\n"		
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"			(EMRInfoT.DataType IN ({INT}, {INT}) AND EMRDetailsT.ID IN ("
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRDataGroupID_MU_MENU_04) "
		"			)"
		"		OR "
		"			(EMRInfoT.DataType = {INT} AND EMRDetailsT.ID IN ("
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRDataGroupID_MU_MENU_04) "
		"			)\r\n"
		"		)\r\n"
		"	)\r\n"	
		// (s.tullis 2015-06-22 15:27) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
		"  UNION SELECT PatientID FROM PatientRemindersSentT WHERE ( PatientRemindersSentT.Deleted = 0 AND dbo.AsDateNoTime(ReminderDate) >= {OLEDATETIME}  AND dbo.AsDateNoTime(ReminderDate) < DATEADD(dd, 1, dbo.AsDateNoTime({OLEDATETIME}))) "
		" ) NumQ ON IncludedPatsQ.PersonID = NumQ.PatientID " 
		, m_filterMURange.GenerateEMRFilter_ForPatientBaseMeasures()
		, eitSingleList, eitMultiList, eitTable
		, m_filterMURange.m_dtFromDate
		, m_filterMURange.m_dtToDate
		);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT PersonID, Max(Numerator) as Numerator, Max(Denominator) as Denominator FROM @04Data GROUP BY PersonID \r\n ");

	return f;
}

MU::MeasureData CMUMeasure_MENU_04::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu04Data;
	Menu04Data.MeasureID = MU::MU_MENU_04;
	Menu04Data.nDenominator = 0;
	Menu04Data.nNumerator = 0;
	Menu04Data.dblRequiredPercent = GetRequirePrecent();
	Menu04Data.strFullName = GetFullName();
	Menu04Data.strShortName = GetShortName();
	Menu04Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Measure = MU::DataPointInfo("Reminder", MU::RawMeasure);
	
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
			
			Menu04Data.MeasureInfo.push_back(personData);
			Menu04Data.nDenominator += personData.nDenominator;
			Menu04Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Menu04Data.DataPointInfo.push_back(Measure);	
 
	return Menu04Data;
}
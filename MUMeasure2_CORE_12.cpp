#include "stdafx.h"
#include "MUMeasure2_CORE_12.h"
#include "CCHITReportInfoListing.h"
// (r.farnworth 2013-10-17 10:42) - PLID 59575 - MU.CORE.12

CMUMeasure2_CORE_12::CMUMeasure2_CORE_12(void)
{
}

CMUMeasure2_CORE_12::~CMUMeasure2_CORE_12(void)
{
}

CSqlFragment CMUMeasure2_CORE_12::GetMeasureSql()
{
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES", MU_13_DEFAULT_CODES, 0, "<None>", true);
	//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 This query return denominator 
	//  Return all emns who have provider filter  and  visit codes  OR Those emn who have provider filter and Emn date matches with Bill(Same provider filter) with Visit code
	CSqlFragment strCommonDenominatorSQL = GetClinicalSummaryCommonDenominatorSQL(strCodes);
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	
		" DECLARE @EMRDataGroupID_MU_CORE_12 INT; \r\n "
		" SET @EMRDataGroupID_MU_CORE_12 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
		"DECLARE @12Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, Denominator INT, Numerator INT \r\n"
		");\r\n\r\n"

		"INSERT INTO @12Data \r\n"
		"SELECT MUAllPatientsQ.PersonID, "
		" CASE WHEN IncludedPatsQ.PatientID IS NULL THEN 0 ELSE 1 END as Denominator, \r\n "	
		" CASE WHEN NumQ.PatientID IS NULL THEN 0 ELSE 1 END as Numerator \r\n "	
		"FROM ( \r\n",
		GetInternalName()	);

	CSqlFragment f(str);

	f += GetAllMUPatients(MU::UnionType::RemindersStage2);

	//first let's join to the denominator
	// (r.farnworth 2014-05-19 08:10) - PLID 59575 - Implement Detailed Reporting for MU.CORE.12 for Stage 2
	// (d.singleton 2014-09-05 11:05) - PLID 63457 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Detailed
	//(s.dhole 9/8/2014 12:14 PM ) - PLID 62795 Added remindersent union
	f += CSqlFragment(	
		" ) MUAllPatientsQ \r\n "
		" INNER JOIN  ( "
		"	SELECT Q.PatientID "
		"	FROM( "
		"		SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID "
		"		FROM EMRMasterT "
		"		INNER JOIN {SQL} ON EMRMasterT.PatientID = ChargeFilterQ.PatientID "
		"			AND ChargeFilterQ.DATE = EmrMasterT.DATE "
		"		WHERE EMRMasterT.Deleted = 0 {SQL} {SQL} "
		"	 ) Q "
		"	INNER JOIN ( "
		"		SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID "
		"		 FROM EMRMasterT "
		"		INNER JOIN {SQL} ON EMRMasterT.PatientID = ChargeFilterQ.PatientID "
		"			AND ChargeFilterQ.DATE = EmrMasterT.DATE "
		"		WHERE EMRMasterT.Deleted = 0 {SQL} {SQL} "
		"	) N ON Q.PatientID = N.PatientID "
		"	WHERE Q.DATE >= DATEADD(year, -2, {OLEDATETIME}) "
		"	AND Q.DATE < {OLEDATETIME} "
		"	AND  N.DATE >= DATEADD(year, -2, {OLEDATETIME}) "
		"	AND N.DATE < {OLEDATETIME} "
		"	AND Q.ChargeID <> N.ChargeID "
		"	AND Q.ID <> N.ID "
		" ) IncludedPatsQ ON MUAllPatientsQ.PersonID = IncludedPatsQ.PatientID \r\n"
		"    \r\n "
		, strCommonDenominatorSQL
		, m_filterPatients.GenerateEMRFilter_ForPatientBaseMeasures(FALSE)
		, m_filterMURange.GenerateEMRFilter_ForPatientBaseMeasures(FALSE)
		,strCommonDenominatorSQL
		, m_filterPatients.GenerateEMRFilter_ForPatientBaseMeasures(FALSE)
		, m_filterMURange.GenerateEMRFilter_ForPatientBaseMeasures(FALSE)
		, m_filterMURange.m_dtFromDate, m_filterMURange.m_dtFromDate
		, m_filterMURange.m_dtFromDate, m_filterMURange.m_dtFromDate
		
	);


	//now the numerator
	f += CSqlFragment (
		" LEFT JOIN ( \r\n "
		" SELECT EMRMasterT.PatientID FROM EMRMasterT \r\n"
		" WHERE EMRMasterT.DELETED = 0 {SQL}  \r\n"		
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"			(EMRInfoT.DataType IN ({INT}, {INT}) AND EMRDetailsT.ID IN ("
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"				WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID_MU_CORE_12) "
		"			)"
		"		OR "
		"			(EMRInfoT.DataType = {INT} AND EMRDetailsT.ID IN ("
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"				WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID_MU_CORE_12 OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID_MU_CORE_12) "
		"			)\r\n"
		"		)\r\n"
		"	)\r\n"
		// (s.tullis 2015-06-22 15:45) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
		"  UNION SELECT PatientID FROM PatientRemindersSentT WHERE ( PatientRemindersSentT.Deleted = 0 AND dbo.AsDateNoTime(ReminderDate) >= {OLEDATETIME}  AND dbo.AsDateNoTime(ReminderDate) < DATEADD(dd, 1, dbo.AsDateNoTime({OLEDATETIME}))) "
		" ) NumQ ON IncludedPatsQ.PatientID = NumQ.PatientID "
		, m_filterMURange.GenerateEMRFilter_ForPatientBaseMeasures()
		, eitSingleList, eitMultiList, eitTable
		, m_filterMURange.m_dtFromDate
		, m_filterMURange.m_dtToDate
		);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT PersonID, Max(Numerator) as Numerator, Max(Denominator) as Denominator FROM @12Data GROUP BY PersonID \r\n ");

	
	return f;
}

MU::MeasureData CMUMeasure2_CORE_12::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core12Data;
	Core12Data.MeasureID = MU::MU2_CORE_12;
	Core12Data.nDenominator = 0;
	Core12Data.nNumerator = 0;
	Core12Data.dblRequiredPercent = GetRequirePrecent();
	Core12Data.strFullName = GetFullName();
	Core12Data.strShortName = GetShortName();
	Core12Data.strInternalName = GetInternalName();

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
			
			Core12Data.MeasureInfo.push_back(personData);
			Core12Data.nDenominator += personData.nDenominator;
			Core12Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core12Data.DataPointInfo.push_back(Measure);	
 
	return Core12Data;
}

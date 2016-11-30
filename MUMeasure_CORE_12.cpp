#include "stdafx.h"
#include "MUMeasure_CORE_12.h"

// (j.dinatale 2012-10-24 14:39) - PLID 53506 - Electronic health info

CMUMeasure_CORE_12::CMUMeasure_CORE_12(void)
{
}

CMUMeasure_CORE_12::~CMUMeasure_CORE_12(void)
{
}

CSqlFragment CMUMeasure_CORE_12::GetMeasureSql()
{
	return CSqlFragment(
		"SET NOCOUNT ON; "

		"DECLARE @EMRMasterID_MU_CORE_12 INT;  "
		"SET @EMRMasterID_MU_CORE_12 = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING});  "

		"DECLARE @EMRMasterID2_MU_CORE_12 INT;  "
		"SET @EMRMasterID2_MU_CORE_12 = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING});  "

		"DECLARE @dtDateTo DATETIME; "
		"SET @dtDateTo = DATEADD(dd, 1, dbo.AsDateNoTime({OLEDATETIME})); "

		"DECLARE @4BusDays DATETIME; "
		"SET @4BusDays = (SELECT DateAdd(day, -1*(CASE WHEN DateName(dw, @dtDateTo) = 'Sunday' THEN 5 ELSE "
		"	CASE WHEN DateName(dw, @dtDateTo) = 'Saturday' THEN 4 ELSE  "
		"	CASE WHEN DateName(dw, @dtDateTo) = 'Friday' THEN 4 ELSE "
		"	6 END END END), @dtDateTo)); " 	

		"DECLARE @CORE12Data TABLE  "
		"(  "
		"	PersonID INT,  "
		"	Qualified INT  "
		");  "

		"INSERT INTO @CORE12Data  "
		"SELECT SubQ.PatientID AS PersonID,  "
		"CASE WHEN (BusinessDays IS NOT NULL AND BusinessDays >= 0 AND BusinessDays < 4) THEN 1 ELSE 0 END AS Qualified  "
		"FROM (  "
		"	SELECT EMRRequestQ.PatientID,  "
		"	DATEDIFF(d, EMRRequestQ.DATE, EMRProvidedQ.DATE) - DATEDIFF(wk, EMRRequestQ.DATE, EMRProvidedQ.DATE) * 2  "
		"	- CASE  "
		"		WHEN DATENAME(dw, EMRRequestQ.DATE) <> 'Saturday' AND DATENAME(dw, EMRProvidedQ.DATE) = 'Saturday'  "
		"			THEN 1  "
		"		WHEN DATENAME(dw, EMRRequestQ.DATE) = 'Saturday' AND DATENAME(dw, EMRProvidedQ.DATE) <> 'Saturday'  "
		"			THEN -1  "
		"		ELSE 0  "
		"	END AS BusinessDays  "
		"	FROM (  "
		"		SELECT PatientID, Date  "
		"		FROM  "
		"		EMRMasterT  "
		"		INNER JOIN (  "
		"			SELECT EMRDetailsT.EMRID AS EMRID  "
		"			FROM EMRDetailsT  "
		"			INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"			WHERE EMRDetailsT.Deleted = 0  "
		"			AND (  "
		"				( "
		"					EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT})  "
		"					AND EMRDetailsT.ID IN (  "
		"						SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"						INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"						INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID  "
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_CORE_12 "
		"					)  "
		"				)  "
		"				OR  "
		"				( "
		"					EMRInfoT.DataType = {CONST_INT}  "
		"					AND EMRDetailsT.ID IN (  "
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"						INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID  "
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_CORE_12 "
		"					)  "
		"				)  "
		"			)  "
		"		) EMRPotentialQ ON EMRMasterT.ID = EMRPotentialQ.EMRID  "
		"		WHERE  "
		"		EMRMasterT.Deleted = 0  "
		"		{SQL} "
		"		{SQL} "
		"		AND EMRMasterT.Date >= dbo.AsDateNoTime({OLEDATETIME}) AND EMRMasterT.Date < @4BusDays  "
		"	) EMRRequestQ  "
		"	LEFT JOIN (  "
		"		SELECT PatientID, DATE  "
		"		FROM EMRMasterT  "
		"		INNER JOIN (  "
		"			SELECT EMRDetailsT.EMRID AS EMRID  "
		"			FROM EMRDetailsT  "
		"			INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"			WHERE EMRDetailsT.Deleted = 0  "
		"			AND (  "
		"				( "
		"					EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT}) AND EMRDetailsT.ID IN (  "
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID  "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_CORE_12)  "
		"				)  "
		"				OR  "
		"				( "
		"					EMRInfoT.DataType = {CONST_INT}  "
		"					AND EMRDetailsT.ID IN (  "
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID  "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_CORE_12)  "
		"				)  "
		"			)  "
		"		) EMRQualifiedQ ON EMRMasterT.ID = EMRQualifiedQ.EMRID  "
		"		WHERE EMRMasterT.Deleted = 0  "
		"		{SQL}  "
		"	) EMRProvidedQ ON EMRRequestQ.PatientID = EMRProvidedQ.PatientID  "
		") SubQ "
		"INNER JOIN (  "
		"	{SQL}  "
		") PatSubQ ON SubQ.PatientID = PatSubQ.PersonID ; "

		"SET NOCOUNT OFF;  "

		"SELECT MAX(Qualified) AS Qualified, PersonID FROM @CORE12Data GROUP BY PersonID; ", 
		("CCHITReportInfo_" + GetInternalName()), ("CCHITReportInfo_" + GetInternalName() + "2"),
		m_filterMURange.m_dtToDate, eitSingleList, eitMultiList, eitTable,
		m_filterMURange.GenerateEMRLocationFilter(),
		m_filterMURange.GenerateEMRProviderFilter(),
		m_filterMURange.m_dtFromDate, eitSingleList, eitMultiList, eitTable,
		m_filterMURange.GenerateEMRFilter_ForPatientBaseMeasures(),
		GetAllMUPatients()
	);
}

MU::MeasureData CMUMeasure_CORE_12::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core12Data;
	Core12Data.MeasureID = MU::MU_CORE_12;
	Core12Data.nDenominator = 0;
	Core12Data.nNumerator = 0;
	Core12Data.dblRequiredPercent = GetRequirePrecent();
	Core12Data.strFullName = GetFullName();
	Core12Data.strShortName = GetShortName();
	Core12Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo ElectAccess = MU::DataPointInfo("Request Elec.", MU::RawMeasure);

	// create recordset
	CNxAdoConnection pConn = CreateThreadConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);

			personData.nNumerator = AdoFldLong(prsDetails, "Qualified", 0);
			personData.nDenominator = 1;

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::RawMeasure;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			if(personData.nNumerator){
				dataPoint.strVisibleData = "Yes";
			}
			personData.DataPoints.push_back(dataPoint);
			
			ElectAccess.nNumerator += personData.nNumerator;
			ElectAccess.nDenominator += personData.nDenominator;

			Core12Data.MeasureInfo.push_back(personData);
			Core12Data.nDenominator += personData.nDenominator;
			Core12Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core12Data.DataPointInfo.push_back(ElectAccess);	
 
	return Core12Data;
}
#include "stdafx.h"
#include "MUMeasure2_CORE_14.h"

// (b.savon 2014-05-06 12:09) - PLID 59577 - Implement MU.CORE.14 for Stage 2

CMUMeasure2_CORE_14::CMUMeasure2_CORE_14(void)
{
}

CMUMeasure2_CORE_14::~CMUMeasure2_CORE_14(void)
{
}

// (s.dhole 2014-05-16 11:09) - PLID 62136 - Added Reconciliation data and fix report to show emn count not patient count
CSqlFragment CMUMeasure2_CORE_14::GetMeasureSql()
{
	return CSqlFragment(
		"SET NOCOUNT ON; "	

		"DECLARE @EMRMasterID_MU_CORE_14 INT; "
		"SET @EMRMasterID_MU_CORE_14 = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); "

		"DECLARE @EMRMasterID2_MU_CORE_14 INT; "
		"SET @EMRMasterID2_MU_CORE_14 = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); "

		"DECLARE @Core14Data TABLE "
		"( "
		" PersonID INT, "
		" Qualified INT "
		"); "
		
		"INSERT INTO @Core14Data "
		"SELECT "
		"EMRQ.PersonID AS PersonID, "
		"CASE WHEN EMRQ.EMRID IS NOT NULL THEN 1 ELSE 0 END AS Qualified "
		"FROM "
		"( "
		"	SELECT "
		"	MUPatientEMRBaseQ.PersonID AS PersonID, "
		"	MUPatientEMRBaseQ.EMRID as  EMRID_D ,Case WHEN PatientReconciliationQ.PatientID IS NOT  NULL THEN MUPatientEMRBaseQ.EMRID ELSE  EMRQualifiedQ.EMRID END EMRID "
		"	FROM "
		"	{SQL} "
		"  LEFT JOIN 	( SELECT PatientID FROM PatientReconciliationT WHERE ReconciliationDate >= dbo.AsDateNoTime({OLEDATETIME}) AND ReconciliationDate < DATEADD(dd, 1, dbo.AsDateNoTime({OLEDATETIME}))) AS PatientReconciliationQ "
		"  ON  PatientReconciliationQ.PatientID = MUPatientEMRBaseQ.PersonID "
		"	INNER JOIN ( "
			"	SELECT EMRDetailsT.EMRID AS EMRID "
			"	FROM EMRDetailsT "
			"	INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"	WHERE EMRDetailsT.Deleted = 0 "
			"	AND ( "
				"	(EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT}) AND EMRDetailsT.ID IN ( "
					"	SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
					"	INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
					"	INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
					"	WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_CORE_14) "
				"	) "
			"	OR "
				"	(EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN ( "
					"	SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
					"	INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
					"	INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
					"	INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
					"	WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_CORE_14) "
				"	) "
			"	) "
			"	GROUP BY EMRDetailsT.EMRID "
		"	) EMRPotentialQ ON MUPatientEMRBaseQ.EMRID = EMRPotentialQ.EMRID "
		"	LEFT JOIN ( "
			"	SELECT EMRDetailsT.EMRID AS EMRID, EMRMasterT.PatientID "
			"	FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"	INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"	WHERE EMRDetailsT.Deleted = 0 "
			"	AND ( "
			"	(EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT}) AND EMRDetailsT.ID IN ( "
					"	SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
					"	INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
					"	INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
					"	WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_CORE_14) "
				"	) "
			"	OR "
				"	(EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN ( "
					"	SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
					"	INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
					"	INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
					"	INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
					"	WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_CORE_14) "
				"	) "
			"	) "
			"	GROUP BY EMRDetailsT.EMRID, EMRMasterT.PatientID "
		"	) EMRQualifiedQ ON MUPatientEMRBaseQ.PersonID = EMRQualifiedQ.PatientID "
		"	GROUP BY MUPatientEMRBaseQ.PersonID,  MUPatientEMRBaseQ.EMRID , EMRQualifiedQ.EMRID ,PatientReconciliationQ.PatientID"
		") EMRQ "
		"SET NOCOUNT OFF; "
		"SELECT COUNT(*) AS Total, SUM(Qualified) AS Qualified, PersonID FROM @Core14Data GROUP BY PersonID; ",
		("CCHITReportInfo_" + GetInternalName()), ("CCHITReportInfo_" + GetInternalName() + "2"),
		GetPatientEMRBaseQuery(),
		m_filterMURange.m_dtFromDate, m_filterMURange.m_dtToDate,
		eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable
	);
}

MU::MeasureData CMUMeasure2_CORE_14::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core14Data;
	Core14Data.MeasureID = MU::MU2_CORE_14;
	Core14Data.nDenominator = 0;
	Core14Data.nNumerator = 0;
	Core14Data.dblRequiredPercent = GetRequirePrecent();
	Core14Data.strFullName = GetFullName();
	Core14Data.strShortName = GetShortName();
	Core14Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo MedRecon = MU::DataPointInfo("Med. Recon.", MU::RawMeasure);	

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);

			personData.nNumerator = AdoFldLong(prsDetails, "Qualified", 0);
			personData.nDenominator = AdoFldLong(prsDetails, "Total", 0);

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::RawMeasure;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li/%li", personData.nNumerator, personData.nDenominator);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			MedRecon.nNumerator += personData.nNumerator;
			MedRecon.nDenominator += personData.nDenominator;

			Core14Data.MeasureInfo.push_back(personData);
			Core14Data.nDenominator += personData.nDenominator;
			Core14Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core14Data.DataPointInfo.push_back(MedRecon);	
 
	return Core14Data;
}
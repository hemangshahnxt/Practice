#include "stdafx.h"
#include "MUMeasure_MENU_07.h"

// (j.dinatale 2012-10-24 14:43) - PLID 53500 - Medication Reconciliation

CMUMeasure_MENU_07::CMUMeasure_MENU_07(void)
{
}

CMUMeasure_MENU_07::~CMUMeasure_MENU_07(void)
{
}

// (s.dhole 2014-05-16 11:09) - PLID 62135 - Added Reconciliation data and fix report to show emn count not patient count
CSqlFragment CMUMeasure_MENU_07::GetMeasureSql()
{
	return CSqlFragment(
		"SET NOCOUNT ON; "

		"DECLARE @EMRMasterID_MU_MENU_07 INT; "
		"SET @EMRMasterID_MU_MENU_07 = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); "

		"DECLARE @EMRMasterID2_MU_MENU_07 INT; "
		"SET @EMRMasterID2_MU_MENU_07 = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); "

		"DECLARE @MENU07Data TABLE "
		"( "
		" PersonID INT, "
		" Qualified INT "
		"); "

		"INSERT INTO @MENU07Data "
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
		"  LEFT JOIN 	( SELECT PatientID FROM PatientReconciliationT WHERE ReconciliationDate >=  dbo.AsDateNoTime({OLEDATETIME}) AND ReconciliationDate < DATEADD(dd, 1, dbo.AsDateNoTime({OLEDATETIME}))) AS PatientReconciliationQ "
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
		"	WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_MENU_07) "
		"	) "
		"	OR "
		"	(EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN ( "
		"	SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"	INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"	INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"	INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
		"	WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_MENU_07) "
		"	) "
		"	) "
		"	GROUP BY EMRDetailsT.EMRID "
		"	) EMRPotentialQ ON MUPatientEMRBaseQ.EMRID = EMRPotentialQ.EMRID "
		"	LEFT JOIN ( "
		/// Numerator
		// (r.farnworth 2014-05-21 15:15) - PLID 62222 - Was joining the base query on EMRID when it needed to look at PersonID
		"	SELECT EMRDetailsT.EMRID AS EMRID, EMRMasterT.PatientID "
		"	FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"	INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"	WHERE EMRDetailsT.Deleted = 0 "
		"	AND ( "
		"	(EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT}) AND EMRDetailsT.ID IN ( "
		"	SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"	INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"	INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
		"	WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_MENU_07) "
		"	) "
		"	OR "
		"	(EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN ( "
		"	SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"	INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"	INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"	INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
		"	WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_MENU_07) "
		"	) "
		"	) "
		"	GROUP BY EMRDetailsT.EMRID, EMRMasterT.PatientID "
		//(s.dhole 8/1/2014 4:19 PM ) - PLID  63040 Should be EmrID not patientid
		"	) EMRQualifiedQ ON MUPatientEMRBaseQ.EMRID = EMRQualifiedQ.EMRID "
		"	GROUP BY MUPatientEMRBaseQ.PersonID, MUPatientEMRBaseQ.EMRID  ,EMRQualifiedQ.EMRID ,PatientReconciliationQ.PatientID"
		") EMRQ "
		"SET NOCOUNT OFF; "
		"SELECT COUNT(*) AS Total, SUM(Qualified) AS Qualified, PersonID FROM @MENU07Data GROUP BY PersonID; ",
			("CCHITReportInfo_" + GetInternalName()), ("CCHITReportInfo_" + GetInternalName() + "2"),
		GetPatientEMRBaseQuery(),
		m_filterMURange.m_dtFromDate, m_filterMURange.m_dtToDate,
		eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable
	);
}

MU::MeasureData CMUMeasure_MENU_07::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu07Data;
	Menu07Data.MeasureID = MU::MU_MENU_07;
	Menu07Data.nDenominator = 0;
	Menu07Data.nNumerator = 0;
	Menu07Data.dblRequiredPercent = GetRequirePrecent();
	Menu07Data.strFullName = GetFullName();
	Menu07Data.strShortName = GetShortName();
	Menu07Data.strInternalName = GetInternalName();

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

			Menu07Data.MeasureInfo.push_back(personData);
			Menu07Data.nDenominator += personData.nDenominator;
			Menu07Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Menu07Data.DataPointInfo.push_back(MedRecon);	
 
	return Menu07Data;
}
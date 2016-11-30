#include "stdafx.h"
#include "MUMeasure_MENU_08.h"

// (j.dinatale 2012-10-24 14:44) - PLID 53499 - Transition of Care

CMUMeasure_MENU_08::CMUMeasure_MENU_08(void)
{
}

CMUMeasure_MENU_08::~CMUMeasure_MENU_08(void)
{
}

CSqlFragment CMUMeasure_MENU_08::GetMeasureSql()
{
	// (r.farnworth 2014-05-14 08:14) - PLID 62133 - Transitions of Care Stage 1 Menu 7 should automatically count in the numerator when a user merges a summary of care for a transitioning patient.
	return CSqlFragment(
		"SET NOCOUNT ON; "	

		"DECLARE @EMRMasterID_MU_MENU_08 INT; "
		"SET @EMRMasterID_MU_MENU_08 = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); "

		"DECLARE @EMRMasterID2_MU_MENU_08 INT; "
		"SET @EMRMasterID2_MU_MENU_08 = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); "

		"DECLARE @MENU08Data TABLE "
		"( "
		" PersonID INT, "
		" Qualified INT "
		"); "
		
		"INSERT INTO @MENU08Data "
		"SELECT "
		"MUPatientEMRBaseQ.PersonID AS PersonID, "
		"CASE WHEN EMRQualifiedQ.PatientID IS NOT NULL OR MailSubQ.PersonID IS NOT NULL THEN 1 ELSE 0 END AS Qualified "
		"FROM "
		"{SQL} "
		"INNER JOIN ( "
			"SELECT EMRDetailsT.EMRID AS EMRID "
			"FROM EMRDetailsT "
			"INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"WHERE EMRDetailsT.Deleted = 0 "
			"AND ( "
				"(EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT}) AND EMRDetailsT.ID IN ( "
				"SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
				"INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
				"INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
				"WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_MENU_08) "
				") "
			"OR "
				"(EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN ( "
				"SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
				"INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
				"INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
				"INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
				"WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_MENU_08) "
				") "
			") "
			"GROUP BY EMRDetailsT.EMRID "
		") EMRPotentialQ ON MUPatientEMRBaseQ.EMRID = EMRPotentialQ.EMRID "
		"LEFT JOIN ( "
			"SELECT PatientID FROM EMRMasterT "
			"INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"WHERE EMRDetailsT.Deleted = 0 "
			"AND ( "
			"(EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT}) AND EMRDetailsT.ID IN ( "
				"SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
				"INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
				"INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
				"WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_MENU_08) "
				") "
			"OR "
				"(EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN ( "
				"SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
				"INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
				"INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
				"INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
				"WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_MENU_08) "
				") "
			") "
			"GROUP BY EMRMasterT.PatientID "
		") EMRQualifiedQ ON MUPatientEMRBaseQ.PersonID = EMRQualifiedQ.PatientID "
		"LEFT JOIN ( "
		"	SELECT PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField = 1 "
		" ) MailSubQ ON MUPatientEMRBaseQ.PersonID = MailSubQ.PersonID "
		"SET NOCOUNT OFF; "
		"SELECT COUNT(*) AS Total, SUM(Qualified) AS Qualified, PersonID FROM @MENU08Data GROUP BY PersonID; ",
		("CCHITReportInfo_" + GetInternalName()), ("CCHITReportInfo_" + GetInternalName() + "2"),
		GetPatientEMRBaseQuery(),
		eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable
	);
}

MU::MeasureData CMUMeasure_MENU_08::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu08Data;
	Menu08Data.MeasureID = MU::MU_MENU_08;
	Menu08Data.nDenominator = 0;
	Menu08Data.nNumerator = 0;
	Menu08Data.dblRequiredPercent = GetRequirePrecent();
	Menu08Data.strFullName = GetFullName();
	Menu08Data.strShortName = GetShortName();
	Menu08Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo TranOfCare = MU::DataPointInfo("Tran. of Care", MU::RawMeasure);

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
			
			TranOfCare.nNumerator += personData.nNumerator;
			TranOfCare.nDenominator += personData.nDenominator;

			Menu08Data.MeasureInfo.push_back(personData);
			Menu08Data.nDenominator += personData.nDenominator;
			Menu08Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Menu08Data.DataPointInfo.push_back(TranOfCare);	
 
	return Menu08Data;
}
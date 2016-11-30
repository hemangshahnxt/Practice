#include "stdafx.h"
#include "MUMeasure2_CORE_15B.h"
#include "CCHITReportInfoListing.h"
// (r.farnworth 2014-05-09 10:08) - PLID 59579 - Implement Detailed Reporting for MU.CORE.15.B for Stage 2



CMUMeasure2_CORE_15B::CMUMeasure2_CORE_15B()
{
}


CMUMeasure2_CORE_15B::~CMUMeasure2_CORE_15B()
{
}

CSqlFragment CMUMeasure2_CORE_15B::GetMeasureSql()
{
	return CSqlFragment(
		"SET NOCOUNT ON; "

		"DECLARE @EMRMasterID_MU_CORE_15B INT; "
		"SET @EMRMasterID_MU_CORE_15B = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); "

		"DECLARE @EMRMasterID2_MU_CORE_15B INT; "
		"SET @EMRMasterID2_MU_CORE_15B = (SELECT IntParam FROM ConfigRT WHERE Name = {STRING}); "

		"DECLARE @CORE_15BData TABLE "
		"( "
		" PersonID INT, "
		" Qualified INT "
		"); "

		"INSERT INTO @CORE_15BData "
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
		"WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_CORE_15B) "
		") "
		"OR "
		"(EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN ( "
		"SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
		"WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_CORE_15B) "
		") "
		") "
		"GROUP BY EMRDetailsT.EMRID "
		") EMRPotentialQ ON MUPatientEMRBaseQ.EMRID = EMRPotentialQ.EMRID "
		"LEFT JOIN ( "
		//(s.dhole 8/1/2014 4:27 PM ) - PLID 63068  we can join emr id not patientid  which causing to return multiple rows
		"SELECT PatientID ,EMRMasterT.ID AS EMRID  FROM EMRMasterT "
		"INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"WHERE EMRDetailsT.Deleted = 0 "
		"AND ( "
		"(EMRInfoT.DataType IN ({CONST_INT}, {CONST_INT}) AND EMRDetailsT.ID IN ( "
		"SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
		"WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_CORE_15B) "
		") "
		"OR "
		"(EMRInfoT.DataType = {CONST_INT} AND EMRDetailsT.ID IN ( "
		"SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID "
		"WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2_MU_CORE_15B) "
		") "
		") "
		"GROUP BY EMRMasterT.PatientID ,EMRMasterT.ID "
		") EMRQualifiedQ ON MUPatientEMRBaseQ.EMRID = EMRQualifiedQ.EMRID "
		"LEFT JOIN ( "
		"	SELECT PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND MailSent.MailID IN (SELECT MailSentID From DirectAttachmentT) "
		" ) MailSubQ ON MUPatientEMRBaseQ.PersonID = MailSubQ.PersonID "
		"SET NOCOUNT OFF; "
		"SELECT COUNT(*) AS Total, SUM(Qualified) AS Qualified, PersonID FROM @CORE_15BData GROUP BY PersonID; ",
		("CCHITReportInfo_" + GetInternalName()), ("CCHITReportInfo_" + GetInternalName() + "2"),
		//(s.dhole 8/1/2014 4:27 PM ) - PLID 63068  
		GetPatientEMRIDBaseQuery(),
		eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable
		);
}

MU::MeasureData CMUMeasure2_CORE_15B::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core15BData;
	Core15BData.MeasureID = MU::MU2_CORE_15B;
	Core15BData.nDenominator = 0;
	Core15BData.nNumerator = 0;
	Core15BData.dblRequiredPercent = GetRequirePrecent();
	Core15BData.strFullName = GetFullName();
	Core15BData.strShortName = GetShortName();
	Core15BData.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo TranOfCare = MU::DataPointInfo("SoC (Transmitted)", MU::RawMeasure);

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if (prsDetails){
		while (!prsDetails->eof){
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

			Core15BData.MeasureInfo.push_back(personData);
			Core15BData.nDenominator += personData.nDenominator;
			Core15BData.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core15BData.DataPointInfo.push_back(TranOfCare);

	return Core15BData;
}
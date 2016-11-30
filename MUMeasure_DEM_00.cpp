#include "stdafx.h"
#include "MUMeasure_DEM_00.h"

// (j.dinatale 2012-10-24 14:34) - PLID 53504 - Measure to provide our base demographic info and other misc info

using namespace ADODB;

CMUMeasure_DEM_00::CMUMeasure_DEM_00(MU::Stage eMeaningfulUseStage)
{
	m_eMeaningfulUseStage = eMeaningfulUseStage;
}

CMUMeasure_DEM_00::~CMUMeasure_DEM_00(void)
{
}

CSqlFragment CMUMeasure_DEM_00::GetMeasureSql()
{
	return CSqlFragment(
		"SELECT "
		"PatQ.UserDefinedID, "
		"MUAllPatientsQ.PersonID, "
		"PersonQ.Last, "
		"RecentVisitQ.LocationName AS RecentLoc, "
		"RecentVisitQ.Date AS RecentDate "
		"FROM "
		"("
		"	{SQL} "
		") MUAllPatientsQ "
		"INNER JOIN ( "
		"	SELECT PatientsT.PersonID, PatientsT.UserDefinedID FROM PatientsT "
		") PatQ ON MUAllPatientsQ.PersonID = PatQ.PersonID "
		"LEFT JOIN ( "
		"	SELECT ID AS PersonID, Last FROM PersonT "
		") PersonQ ON MUAllPatientsQ.PersonID = PersonQ.PersonID "
		"LEFT JOIN ( "
		"	SELECT "
		"	PatientID, "
		"	LocationsT.Name AS LocationName, "
		"	Date "
		"	FROM ( "
		"		SELECT EMRMasterT.PatientID, "
		"		EMRMasterT.LocationID, "
		"		EMRMasterT.Date, "
		"		ROW_NUMBER() "
		"		OVER ( "
		"			PARTITION BY EMRMasterT.PatientID "
		"			ORDER BY EMRMasterT.Date DESC, EMRMasterT.InputDate DESC "
		"		) AS Ranking "
		"		FROM EMRMasterT "
		"		WHERE EMRMasterT.Deleted = 0 "
		"	) MostRecentQ "
		"	LEFT JOIN LocationsT ON MostRecentQ.LocationID = LocationsT.ID "
		"	WHERE Ranking = 1 "
		") RecentVisitQ ON MUAllPatientsQ.PersonID = RecentVisitQ.PatientID ",
		// (r.farnworth 2014-06-04 16:45) - PLID 62325 - Check for Stage 1 or 2. If neither (impossible), default to Stage 2. 
		// Extra if is unnecessary now but will be useful if Stage 3 ever happens.
		(m_eMeaningfulUseStage == MU::Stage1) ? GetAllMUPatients(MU::UnionType::AllStage1) :
		(m_eMeaningfulUseStage == MU::Stage2) ? GetAllMUPatients(MU::UnionType::AllStage2) : GetAllMUPatients(MU::UnionType::AllStage2)
		);
}

MU::MeasureData CMUMeasure_DEM_00::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core00Data;
	Core00Data.MeasureID = MU::MU_DEM_00;
	Core00Data.nDenominator = 0;
	Core00Data.nNumerator = 0;
	Core00Data.dblRequiredPercent = GetRequirePrecent();
	Core00Data.strFullName = GetFullName();
	Core00Data.strShortName = GetShortName();
	Core00Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo RecDate = MU::DataPointInfo("Last Visit", MU::EMNDate);
	MU::DataPointInfo RecLoc = MU::DataPointInfo("Recent Loc.", MU::Location);
	MU::DataPointInfo PatID = MU::DataPointInfo("Patient ID", MU::PatientID);
	MU::DataPointInfo Last = MU::DataPointInfo("Last", MU::LastName);

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);

			MU::DataPoint dpDate;
			dpDate.DataType = MU::EMNDate;
			dpDate.strVisibleData = FormatDateTimeForInterface(AdoFldDateTime(prsDetails, "RecentDate", g_cdtNull), 0, dtoDate);
			personData.DataPoints.push_back(dpDate);

			MU::DataPoint dpLocation;
			dpLocation.DataType = MU::Location;
			dpLocation.strVisibleData = AdoFldString(prsDetails, "RecentLoc", "");
			personData.DataPoints.push_back(dpLocation);

			MU::DataPoint dpUserDefinedID;
			dpUserDefinedID.DataType = MU::PatientID;
			dpUserDefinedID.strVisibleData = AsString(AdoFldLong(prsDetails, "UserDefinedID", -1));
			personData.DataPoints.push_back(dpUserDefinedID);

			MU::DataPoint dpLast;
			dpLast.DataType = MU::LastName;
			dpLast.strVisibleData = AdoFldString(prsDetails, "Last", "");
			personData.DataPoints.push_back(dpLast);

			Core00Data.MeasureInfo.push_back(personData);
			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core00Data.DataPointInfo.push_back(PatID);
	Core00Data.DataPointInfo.push_back(Last);
	Core00Data.DataPointInfo.push_back(RecDate);
	Core00Data.DataPointInfo.push_back(RecLoc);
 
	return Core00Data;
}
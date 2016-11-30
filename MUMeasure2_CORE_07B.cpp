#include "stdafx.h"
#include "MUMeasure2_CORE_07B.h"
#include "CCHITReportInfoListing.h"
// (s.dhole 2014-05-06 16:22) - PLID 59572 - Implement Detailed Reporting for MU.CORE.07.B for Stage 2


CMUMeasure2_CORE_07B::CMUMeasure2_CORE_07B(void)
{
}

CMUMeasure2_CORE_07B::~CMUMeasure2_CORE_07B(void)
{
}

CSqlFragment CMUMeasure2_CORE_07B::GetMeasureSql()
{
	// (r.gonet 2015-02-18 13:04) - PLID 64437 - We now join the NexWebCcdaAccessHistoryT onto the MailSent record and from there get the PersonID
	// rather than joining NexWebCcdaAccessHistoryT to the NexWebLoginInfoT on the username column. This is because we now keep access history 
	// instead of deleting it when a NexWeb username is deleted.
	return CSqlFragment(
		"SET NOCOUNT ON; "

		"DECLARE @CORE07BData TABLE "
		"( "
		" PersonID INT, "
		" Qualified INT "
		"); "

		"INSERT INTO @CORE07BData "
		"SELECT DataQ.PersonID,  "
		"HasAccessDoc AS Qualified "
		"FROM ( "
		"	SELECT Q.PersonID, CASE WHEN NexWebAccessQ.PersonID IS NULL THEN 0 ELSE 1 END AS HasAccessDoc "
		"	FROM "
		"	( "
		"		SELECT MUPatientEMRBaseQ.PersonID, MUPatientEMRBaseQ.Date "
		"		FROM "
		"		{SQL} "
		"	) Q "
		"	LEFT JOIN( "
		"	SELECT DISTINCT MailSent.PersonID FROM MailSent "
		"	INNER JOIN NexWebCcdaAccessHistoryT ON NexWebCcdaAccessHistoryT.MailSentMailID = MailSent.MailID "
		"	WHERE NexWebCcdaAccessHistoryT.AccessType IN (-1, -2, -3) "
		"	) NexWebAccessQ ON Q.PersonID = NexWebAccessQ.PersonID "
		") DataQ "
		"SET NOCOUNT OFF; "
		"SELECT PersonID, MAX(Qualified) AS Qualified FROM @CORE07BData GROUP BY PersonID ",
		GetPatientEMRBaseQuery()
		);
}

MU::MeasureData CMUMeasure2_CORE_07B::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu07AData;
	Menu07AData.MeasureID = MU::MU2_CORE_07B;
	Menu07AData.nDenominator = 0;
	Menu07AData.nNumerator = 0;
	Menu07AData.dblRequiredPercent = GetRequirePrecent();
	Menu07AData.strFullName = GetFullName();
	Menu07AData.strShortName = GetShortName();
	Menu07AData.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo ElectAccess = MU::DataPointInfo("Accessed Elec.", MU::RawMeasure);

	// create recordset
		CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if (prsDetails){
		while (!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			personData.nNumerator = AdoFldLong(prsDetails, "Qualified", 0);
			personData.nDenominator = 1;
			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::RawMeasure;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			if (personData.nNumerator){
				dataPoint.strVisibleData = "Yes";
			}
			personData.DataPoints.push_back(dataPoint);

			ElectAccess.nNumerator += personData.nNumerator;
			ElectAccess.nDenominator += personData.nDenominator;

			Menu07AData.MeasureInfo.push_back(personData);
			Menu07AData.nDenominator += personData.nDenominator;
			Menu07AData.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Menu07AData.DataPointInfo.push_back(ElectAccess);

	return Menu07AData;
}
#include "stdafx.h"
#include "MUMeasure_MENU_05.h"

// (j.dinatale 2012-10-24 14:41) - PLID 53501 - Timely Electronic Access

CMUMeasure_MENU_05::CMUMeasure_MENU_05(void)
{
}

CMUMeasure_MENU_05::~CMUMeasure_MENU_05(void)
{
}

CSqlFragment CMUMeasure_MENU_05::GetMeasureSql()
{
	return CSqlFragment(
		"SET NOCOUNT ON; "	

		"DECLARE @MENU05Data TABLE "
		"( "
		" PersonID INT, "
		" Qualified INT "
		"); "

		"INSERT INTO @MENU05Data "
		"SELECT DataQ.PersonID,  "
		// (r.farnworth 2014-05-19 11:15) - PLID 62188 - Patients Electronic Access Stage 1 Core 11 need to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		"CASE WHEN BusinessDays IS NOT NULL AND BusinessDays <= 4 AND MergeDays IS NOT NULL AND MergeDays <= 4 "
		"	THEN 1 "
		"ELSE 0 END AS Qualified "
		"FROM ( "
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the InitialSecurityCodeCreationDate since SecurityCodeCreationDate is cleared out when the patient creates a username with the security code.
		"	SELECT Q.PersonID, "
		"	DATEDIFF(d, Q.DATE, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) "
		"	- DATEDIFF(wk, Q.DATE, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) * 2 - CASE  "
		"		WHEN DATENAME(dw, Q.DATE) <> 'Saturday' "
		"			AND DATENAME(dw, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) = 'Saturday' "
		"			THEN 1 "
		"		WHEN DATENAME(dw, Q.DATE) = 'Saturday' "
		"			AND DATENAME(dw, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) <> 'Saturday' "
		"			THEN - 1 "
		"		ELSE 0 "
		"	END AS BusinessDays, "
		// (r.farnworth 2014-05-19 11:15) - PLID 62188 - Patients Electronic Access Stage 1 Core 11 need to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		"	DATEDIFF(d, Q.DATE, MergeDocsT.MergeDate) "
		"	- DATEDIFF(wk, Q.DATE, MergeDocsT.MergeDate) * 2 - CASE  "
		"		WHEN DATENAME(dw, Q.DATE) <> 'Saturday' "
		"			AND DATENAME(dw, MergeDocsT.MergeDate) = 'Saturday' "
		"			THEN 1 "
		"		WHEN DATENAME(dw, Q.DATE) = 'Saturday' "
		"			AND DATENAME(dw, MergeDocsT.MergeDate) <> 'Saturday' "
		"			THEN - 1 "
		"		ELSE 0 "
		"	END AS MergeDays "
		"	FROM "
		"	( "
		"		SELECT MUPatientEMRBaseQ.PersonID, MUPatientEMRBaseQ.Date "
		"		FROM "
		"		{SQL} "
		"	) Q "
		"	LEFT JOIN ( "
		"		SELECT Min(CreatedDate) AS MinDate "
		"			,PersonID "
		"		FROM NexwebLoginInfoT "
		"		WHERE Enabled = 1 "
		"		GROUP BY PersonID "
		"	) NexWebLoginT ON Q.PersonID = NexwebLoginT.PersonID "
		// (r.farnworth 2014-05-06 11:35) - PLID 61910 - Fix Meaningful Use Detailed and Summary Reporting measure for MU.MENU.05 for Stage 1
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the first date a security code was ever created for the patient rather than the current security code creation date.
		// This ensures we are using the date when the patient first had access to the portal since SecurityCode and SecurityCodeCreationDate are nulled out when the patient creates a login from the security code.
		"	LEFT JOIN ( "
		"		SELECT InitialSecurityCodeCreationDate, PersonID FROM PatientsT WHERE CurrentStatus <> 4 "
		"		AND PersonID > 0 AND SecurityCode IS NOT NULL "
		"	) SecurityCodeT ON Q.PersonID = SecurityCodeT.PersonID  "
		// (r.farnworth 2014-05-19 11:16) - PLID 62188 - Patients Electronic Access Stage 1 Core 11 need to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		"	LEFT JOIN ( "
		"	   SELECT Min(Date) as MergeDate, PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField IN (1,2) GROUP BY PersonID "
		"	) MergeDocsT ON Q.PersonID = MergeDocsT.PersonID "
		") DataQ "
		"SET NOCOUNT OFF; "
		"SELECT PersonID, MAX(Qualified) AS Qualified FROM @MENU05Data GROUP BY PersonID ",
		GetPatientEMRBaseQuery()
	);
}

MU::MeasureData CMUMeasure_MENU_05::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu05Data;
	Menu05Data.MeasureID = MU::MU_MENU_05;
	Menu05Data.nDenominator = 0;
	Menu05Data.nNumerator = 0;
	Menu05Data.dblRequiredPercent = GetRequirePrecent();
	Menu05Data.strFullName = GetFullName();
	Menu05Data.strShortName = GetShortName();
	Menu05Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo ElectAccess = MU::DataPointInfo("Elect. Access", MU::RawMeasure);	

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

			Menu05Data.MeasureInfo.push_back(personData);
			Menu05Data.nDenominator += personData.nDenominator;
			Menu05Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Menu05Data.DataPointInfo.push_back(ElectAccess);	
 
	return Menu05Data;
}
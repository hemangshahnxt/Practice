#include "stdafx.h"
#include "MUMeasure_CORE_01.h"

// (j.gruber 2012-10-25 09:11) - PLID 53531
CMUMeasure_CORE_01::CMUMeasure_CORE_01(void)
{
}

CMUMeasure_CORE_01::~CMUMeasure_CORE_01(void)
{
}

CSqlFragment CMUMeasure_CORE_01::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "
		" DECLARE @MailSentCatID_MU_CORE_01 INT; \r\n"
		" SET @MailSentCatID_MU_CORE_01 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"

		"DECLARE @CORE01Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, CountScripts int, CountHistory int \r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE01Data\r\n"
		"SELECT PersonID, "
		" Sum(COALESCE(ScriptCount, 0)) as ScriptCount, Sum(COALESCE(HistoryCount, 0)) as HistoryCount \r\n "	
		"FROM\r\n", GetInternalName()
	);

	CSqlFragment f(str);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	f += CSqlFragment(		
		"INNER JOIN  ( "
		" 	SELECT PatientID, Count(*) AS ScriptCount, 0 as HistoryCount "
		" 	FROM PatientMedications  "
		" 	WHERE PatientMedications.Deleted = 0 "
		" 	GROUP BY PatientMedications.PatientID "
		" 	UNION "
		"	SELECT Mailsent.PersonID AS PatientID, 0 as ScriptCount, Count(*) as HistoryCount "
		" 	FROM Mailsent WHERE MailSent.CategoryID =  @MailSentCatID_MU_CORE_01 "
		" 	GROUP BY MailSent.PersonID "
		" ) CountQ ON MUPatientEMRBaseQ.PersonID = CountQ.PatientID "
		" GROUP BY MUPatientEMRBaseQ.PersonID \r\n "
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT * FROM @CORE01Data \r\n ");

	return f;
}

MU::MeasureData CMUMeasure_CORE_01::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core01Data;
	Core01Data.MeasureID = MU::MU_CORE_01;	
	Core01Data.nDenominator = 0;
	Core01Data.nNumerator = 0;
	Core01Data.dblRequiredPercent = GetRequirePrecent();
	Core01Data.strFullName = GetFullName();
	Core01Data.strShortName = GetShortName();
	Core01Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo CPOE = MU::DataPointInfo("CPOE", MU::CPOE);	

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			long nCountScripts = AdoFldLong(prsDetails, "CountScripts");
			long nCountHistory = AdoFldLong(prsDetails, "CountHistory");
			
			personData.nNumerator = nCountScripts > 0 ? 1 : 0;
			personData.nDenominator = nCountScripts > 0 || nCountHistory > 0 ? 1 : 0;

			CPOE.nDenominator += personData.nDenominator;
			CPOE.nNumerator += personData.nNumerator;
			
			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::CPOE;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%li", nCountScripts);
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			Core01Data.MeasureInfo.push_back(personData);
			Core01Data.nDenominator += personData.nDenominator;
			Core01Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core01Data.DataPointInfo.push_back(CPOE);	
 
	return Core01Data;
}
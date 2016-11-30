#include "stdafx.h"
#include "MUMeasure_CORE_03.h"
// (j.gruber 2012-10-25 10:14) - PLID 53530
CMUMeasure_CORE_03::CMUMeasure_CORE_03(void)
{
}

CMUMeasure_CORE_03::~CMUMeasure_CORE_03(void)
{
}

CSqlFragment CMUMeasure_CORE_03::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	

		"DECLARE @CORE03Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, HasMeasure nVarchar(5) null \r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE03Data \r\n"
		"SELECT MUPatientEMRBaseQ.PersonID, "
		" CASE WHEN DiagPersonQ.PersonID IS NOT NULL then 'Yes' ELSE NULL END as HasMeasure \r\n "	
		"FROM \r\n"
	);

	CSqlFragment f(str);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	// (r.gonet 03/03/2013) - PLID 61120 - The measure now looks at ICD-10 codes as well.
	f += CSqlFragment(		
		" LEFT JOIN  ( "
		"	SELECT ID as PersonID FROM PersonT  "
		" 	WHERE ID IN (SELECT PersonID FROM PatientsT WHERE DefaultDiagID1 IS NOT NULL OR DefaultDiagID2 IS NOT NULL OR DefaultDiagID3 IS NOT NULL OR DefaultDiagID4 IS NOT NULL OR DefaultICD10DiagID1 IS NOT NULL OR DefaultICD10DiagID2 IS NOT NULL OR DefaultICD10DiagID3 IS NOT NULL OR DefaultICD10DiagID4 IS NOT NULL)   "
		" 		OR ID IN (SELECT PatientID FROM BillsT WHERE DELETED = 0 AND BillsT.ID IN (SELECT BillDiagCodeT.BillID FROM BillDiagCodeT WHERE BillDiagCodeT.ICD9DiagID IS NOT NULL OR BillDiagCodeT.ICD10DiagID IS NOT NULL))  "
		" 		OR ID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0 AND ID IN (SELECT EMRID FROM EMRDiagCodesT WHERE DELETED = 0 AND (DiagCodeID IS NOT NULL OR DiagCodeID_ICD10 IS NOT NULL)))   "
		" 		OR ID IN (SELECT PatientID FROM EMRProblemsT WHERE DELETED = 0 AND (DiagCodeID IS NOT NULL OR DiagCodeID_ICD10 IS NOT NULL))   "
		" ) DiagPersonQ ON MUPatientEMRBaseQ.PersonID = DiagPersonQ.PersonID "		
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT PersonID, Min(HasMeasure) as HasMeasure FROM @CORE03Data GROUP BY PersonID \r\n ");

	return f;
}

MU::MeasureData CMUMeasure_CORE_03::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core03Data;
	Core03Data.MeasureID = MU::MU_CORE_03;
	Core03Data.nDenominator = 0;
	Core03Data.nNumerator = 0;
	Core03Data.dblRequiredPercent = GetRequirePrecent();
	Core03Data.strFullName = GetFullName();
	Core03Data.strShortName = GetShortName();
	Core03Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Diag = MU::DataPointInfo("DX", MU::RawMeasure);	

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			_variant_t varMeasure = prsDetails->Fields->Item["HasMeasure"]->Value;
			
			personData.nNumerator = varMeasure.vt == VT_BSTR ? 1 : 0;
			personData.nDenominator = 1;

			Diag.nDenominator += personData.nDenominator;
			Diag.nNumerator += personData.nNumerator;
			
			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::RawMeasure;
			dataPoint.nDenominator = personData.nDenominator;
			dataPoint.nNumerator = personData.nNumerator;
			CString strVisibleData;
			strVisibleData.Format("%s", VarString(varMeasure, ""));
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			Core03Data.MeasureInfo.push_back(personData);
			Core03Data.nDenominator += personData.nDenominator;
			Core03Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core03Data.DataPointInfo.push_back(Diag);	
 
	return Core03Data;
}
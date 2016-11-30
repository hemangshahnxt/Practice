#include "stdafx.h"
#include "MUMeasure2_CORE_13.h"
#include "CCHITReportInfoListing.h"
// (r.farnworth 2013-10-16 14:03) - PLID 59576 - Created, copied from MU.MENU.06
CMUMeasure2_CORE_13::CMUMeasure2_CORE_13(void)
{
}

CMUMeasure2_CORE_13::~CMUMeasure2_CORE_13(void)
{
}

CSqlFragment CMUMeasure2_CORE_13::GetMeasureSql()
{
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES", MU_13_DEFAULT_CODES, 0, "<None>", true);


	//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 This query return denominator 
	//  Return all emns who have provider filter  and  visit codes  OR Those emn who have provider filter and Emn date matches with Bill(Same provider filter) with Visit code
	CSqlFragment strCommonDenominatorSQL = GetClinicalSummaryCommonDenominatorSQL(strCodes);


	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	
		" DECLARE @EMRDataGroupID_MU_CORE_13 INT; \r\n "
		" SET @EMRDataGroupID_MU_CORE_13 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
		"DECLARE @Core13Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, HasMeasure nvarchar(5) \r\n"
		");\r\n\r\n"

		"INSERT INTO @Core13Data \r\n"
		"SELECT MUPatientEMRBaseQ.PersonID, "
		" CASE WHEN SubQ.EMRID IS NOT NULL OR EducationResourceAccessT.PatientID IS NOT NULL THEN 'Yes' ELSE NULL END as HasMeasure \r\n "
		"FROM \r\n",
		GetInternalName()	);

	CSqlFragment f(str);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	// (r.farnworth 2014-05-14 16:03) - PLID 62138 - Patient Education Resources Stage 2 Core 13 should automatically count in the numerator when a user clicks on any of our informational buttons or hyperlinks
	// (s.dhole 2014-06-04 07:03) - PLID 62293 - Exclude voide charges
	// (d.singleton 2014-09-11 16:02) - PLID 63455 - MU2 Core 13 (Education) - Bills not filtering on Provider - Detailed
	//(s.dhole 9/26/2014 8:55 AM ) - PLID 63765 update query with common  function
	f += CSqlFragment(
		" INNER JOIN {SQL} ON MUPatientEMRBaseQ.PersonID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = MUPatientEMRBaseQ.Date "
		" LEFT JOIN  ( "
		" SELECT EMRMasterT.ID as EMRID, EMRMasterT.PatientID  \r\n" 
		" FROM EMRMasterT \r\n "
		" WHERE EMRMasterT.ID IN ( \r\n "
		" 		SELECT EMRDetailsT.EMRID FROM EMRDetailsT  \r\n "
		" 				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  \r\n "
		" 				WHERE EMRDetailsT.Deleted = 0 		 \r\n "
		" 				AND ( \r\n "
		" 					(EMRInfoT.DataType IN ({INT}, {INT}) AND EMRDetailsT.ID IN ( \r\n "
		" 						SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  \r\n "
		" 						INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  \r\n "
		" 						WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID_MU_CORE_13)  \r\n "
		" 					) \r\n "
		" 				OR 	 \r\n "
		" 					(EMRInfoT.DataType = {INT} AND EMRDetailsT.ID IN ( \r\n "
		" 						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  \r\n "
		" 						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  \r\n "
		" 						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  \r\n "
		" 						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID_MU_CORE_13 OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID_MU_CORE_13)  \r\n "
		" 					) \r\n "
		" 				) \r\n "
		"	) \r\n "
		" ) SubQ ON MUPatientEMRBaseQ.EMRID = SubQ.EMRID \r\n"
		" LEFT JOIN EducationResourceAccessT ON MUPatientEMRBaseQ.PersonID = EducationResourceAccessT.PatientID "
		"    \r\n ", strCommonDenominatorSQL,
		eitSingleList, eitMultiList, eitTable
		
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT * FROM @Core13Data ORDER BY PersonID \r\n ");

	
	return f;
}

MU::MeasureData CMUMeasure2_CORE_13::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core13Data;
	Core13Data.MeasureID = MU::MU2_CORE_13;
	Core13Data.nDenominator = 0;
	Core13Data.nNumerator = 0;
	Core13Data.dblRequiredPercent = GetRequirePrecent();
	Core13Data.strFullName = GetFullName();
	Core13Data.strShortName = GetShortName();
	Core13Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Measure = MU::DataPointInfo("Ed. Resources", MU::RawMeasure);
	
	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	long nCurrentPersonID;
	long nOldPersonID = -1;

	_variant_t varMeasure = g_cvarNull;	

	// fill our measure data
	if(prsDetails){
		if (!prsDetails->eof) {
			nOldPersonID = AdoFldLong(prsDetails, "PersonID");

			while(!prsDetails->eof){

				//now loop
				nCurrentPersonID = AdoFldLong(prsDetails, "PersonID");

				if (nCurrentPersonID != nOldPersonID) {

					MU::DataPoint dataPointMeasure;
					dataPointMeasure.DataType = MU::RawMeasure;
					dataPointMeasure.nDenominator = 1;				

					//we know this person counts for all the denominators, so increment those
					Measure.nDenominator++;

					//we hit a new person, so let's save our information for the old one
					// (j.armen 2013-08-02 16:30) - PLID 57804 - Declare new PersonMeasureData object
					MU::PersonMeasureData personData;

					if (varMeasure.vt != VT_NULL) {
						Measure.nNumerator++;
						dataPointMeasure.nNumerator = 1;
						personData.nNumerator = 1;
						dataPointMeasure.strVisibleData = VarString(varMeasure);
					}
					else {
						dataPointMeasure.nNumerator = 0;
						personData.nNumerator = 0;
					}

					//now that we have all the values, add them to the patient
					personData.nPersonID = nOldPersonID;	
					personData.DataPoints.push_back(dataPointMeasure);				

					Core13Data.MeasureInfo.push_back(personData);
					Core13Data.nDenominator++;
					Core13Data.nNumerator += personData.nNumerator;

					//now reset all the variables
					nOldPersonID = nCurrentPersonID;
					varMeasure = g_cvarNull;				
				}	


				_variant_t varTemp = prsDetails->Fields->Item["HasMeasure"]->Value;
				if (varTemp.vt != VT_NULL) {
					varMeasure = varTemp;
				}		

				prsDetails->MoveNext();
			}

			//now we have to add the last one
			MU::DataPoint dataPointMeasure;
			dataPointMeasure.DataType = MU::RawMeasure;
			dataPointMeasure.nDenominator = 1;				

			//we know this person counts for all the denominators, so increment those
			Measure.nDenominator++;

			//we hit a new person, so let's save our information for the old one
			// (j.armen 2013-08-02 16:30) - PLID 57804 - Declare new PersonMeasureData object
			MU::PersonMeasureData personData;

			if (varMeasure.vt != VT_NULL) {
				Measure.nNumerator++;
				dataPointMeasure.nNumerator = 1;
				personData.nNumerator = 1;
				dataPointMeasure.strVisibleData = VarString(varMeasure);
			}
			else {
				dataPointMeasure.nNumerator = 0;
				personData.nNumerator = 0;
			}

			//now that we have all the values, add them to the patient
			personData.nPersonID = nOldPersonID;	
			personData.DataPoints.push_back(dataPointMeasure);				

			Core13Data.MeasureInfo.push_back(personData);
			Core13Data.nDenominator++;
			Core13Data.nNumerator += personData.nNumerator;
		}
	}

	// add our data point info to our vector
	Core13Data.DataPointInfo.push_back(Measure);	
 
	return Core13Data;
}
#include "stdafx.h"
#include "MUMeasure_MENU_06.h"
// (j.gruber 2012-10-25 09:08) - PLID 53522

CMUMeasure_MENU_06::CMUMeasure_MENU_06(void)
{
}

CMUMeasure_MENU_06::~CMUMeasure_MENU_06(void)
{
}

CSqlFragment CMUMeasure_MENU_06::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	
		" DECLARE @EMRDataGroupID_MU_MENU_06 INT; \r\n "
		" SET @EMRDataGroupID_MU_MENU_06 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
		"DECLARE @Menu06Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, HasMeasure nvarchar(5) \r\n"
		");\r\n\r\n"

		"INSERT INTO @Menu06Data \r\n"
		"SELECT MUPatientEMRBaseQ.PersonID, "
		" CASE WHEN SubQ.EMRID IS NOT NULL OR EducationResourceAccessT.PatientID IS NOT NULL THEN 'Yes' ELSE NULL END as HasMeasure \r\n "	
		"FROM \r\n",
		GetInternalName()	);

	CSqlFragment f(str);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	// (r.farnworth 2014-05-14 15:06) - PLID 62137 - Patient Education Resources Stage 1 Menu 6 should automatically count in the numerator when a user clicks
	// on any of our informational buttons or hyperlinks.
	// (r.farnworth 2014-05-21 15:15) - PLID 62222 - Was joining the base query on EMRID when it needed to look at PersonID
	f += CSqlFragment(		
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
		" 						WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID_MU_MENU_06)  \r\n "
		" 					) \r\n "
		" 				OR 	 \r\n "
		" 					(EMRInfoT.DataType = {INT} AND EMRDetailsT.ID IN ( \r\n "
		" 						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  \r\n "
		" 						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  \r\n "
		" 						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  \r\n "
		" 						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID_MU_MENU_06 OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID_MU_MENU_06)  \r\n "
		" 					) \r\n "
		" 				) \r\n "
		"	) \r\n "
		//(s.dhole 8/1/2014 3:31 PM ) - PLID 63088 Should be EmrID not patientid
		" ) SubQ ON MUPatientEMRBaseQ.EMRID = SubQ.EMRID \r\n"
		" LEFT JOIN EducationResourceAccessT ON MUPatientEMRBaseQ.PersonID = EducationResourceAccessT.PatientID "
		"    \r\n ", eitSingleList, eitMultiList, eitTable
		
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT * FROM @Menu06Data ORDER BY PersonID \r\n ");

	CString str1 = f.Flatten();
	return f;
}

MU::MeasureData CMUMeasure_MENU_06::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu06Data;
	Menu06Data.MeasureID = MU::MU_MENU_06;
	Menu06Data.nDenominator = 0;
	Menu06Data.nNumerator = 0;
	Menu06Data.dblRequiredPercent = GetRequirePrecent();
	Menu06Data.strFullName = GetFullName();
	Menu06Data.strShortName = GetShortName();
	Menu06Data.strInternalName = GetInternalName();

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

					Menu06Data.MeasureInfo.push_back(personData);
					Menu06Data.nDenominator++;
					Menu06Data.nNumerator += personData.nNumerator;

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

			Menu06Data.MeasureInfo.push_back(personData);
			Menu06Data.nDenominator++;
			Menu06Data.nNumerator += personData.nNumerator;
		}
	}

	// add our data point info to our vector
	Menu06Data.DataPointInfo.push_back(Measure);	
 
	return Menu06Data;
}
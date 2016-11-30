#include "stdafx.h"
#include "MUMeasure2_MENU_04.h"
//TES 10/16/2013 - PLID 59583 - MU.MENU.04

CMUMeasure2_MENU_04::CMUMeasure2_MENU_04(void)
{
}

CMUMeasure2_MENU_04::~CMUMeasure2_MENU_04(void)
{
}

CSqlFragment CMUMeasure2_MENU_04::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	
		" DECLARE @EMRMasterID_MU_MENU_04 INT; \r\n "
		" SET @EMRMasterID_MU_MENU_04 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
		"DECLARE @MENU04Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, Data nVarChar(4000) \r\n"
		");\r\n\r\n"

		"INSERT INTO @MENU04Data \r\n"
		"SELECT MUPatientEMRBaseQ.PersonID, "
		" FamilyHistoryQ.Data \r\n "	
		"FROM \r\n",
		GetInternalName()	);

	CSqlFragment f(str);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	f += CSqlFragment(				
		"  LEFT JOIN   \r\n "
		"  (SELECT EMRDetailsT.EMRID, EMRMasterT.PatientID, CASE WHEN EMRInfoT.DataType IN ({INT},{INT}) THEN EMRSelectDataT.Data WHEN EMRInfoT.DataType = {INT} THEN EMRTableDataT.Data ELSE NULL END as Data  \r\n "
		"  FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"  INNER JOIN EMRInfoT ON EmrDetailsT.EmrInfoID = EMRInfoT.ID   \r\n "
		"  LEFT JOIN (SELECT EMRSelectT.EMRDetailID, EMRDataT.Data   \r\n "
		"  	  FROM	EMRSelectT INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  \r\n "
		" 	  INNER JOIN EMRInfoT InnerEMRInfoT ON EMRDataT.EMRInfoID = InnerEMRInfoT.ID\r\n "
		"  	  WHERE InnerEMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_MENU_04\r\n "
		"  	) EMRSelectDataT ON EMRDetailsT.ID = EMRSelectDataT.EMRDetailID  \r\n "
		"  	LEFT JOIN   \r\n "
		"  	(SELECT EMRDetailTableDataT.EMRDetailID, EMRDetailTableDataT.Data  \r\n "
		"  		FROM EMRDetailTableDataT	   \r\n "
		"  		INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID   \r\n "
		"  		INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID   \r\n "
		" 		INNER JOIN EMRInfoT InnerEMRInfoT ON EMRDataT_X.EMRInfoID = InnerEMRInfoT.ID\r\n "
		"  		WHERE InnerEMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_MENU_04 \r\n "
		"  	) EMRTableDataT ON EMRDetailsT.ID = EMRTableDataT.EMRDetailID  \r\n "
		"  	WHERE EMRDetailsT.Deleted = 0 AND (EMRSelectDataT.Data IS NOT NULL OR EMRTableDataT.Data IS NOT NULL)  \r\n "		
		" ) FamilyHistoryQ ON MUPatientEMRBaseQ.PersonID = FamilyHistoryQ.PatientID"
		"    \r\n "
		,  eitSingleList, eitMultiList, eitTable
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT * FROM @MENU04Data ORDER BY PersonID \r\n ");

	CString str1 = f.Flatten();

	return f;
}

MU::MeasureData CMUMeasure2_MENU_04::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Menu04Data;
	Menu04Data.MeasureID = MU::MU2_MENU_04;
	Menu04Data.nDenominator = 0;
	Menu04Data.nNumerator = 0;
	Menu04Data.dblRequiredPercent = GetRequirePrecent();
	Menu04Data.strFullName = GetFullName();
	Menu04Data.strShortName = GetShortName();
	Menu04Data.strInternalName = GetInternalName();

	// possible data points
	// (s.dhole 2014-05-19 10:30) - PLID 59583 
	MU::DataPointInfo FamilyHistory = MU::DataPointInfo("FHx", MU::FamilyHistory);
	
	// create recordset
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	long nCurrentPersonID;
	long nOldPersonID = -1;

	_variant_t varFamilyHistory = g_cvarNull;	

	// fill our measure data
	if(prsDetails){
		if (!prsDetails->eof) {
			nOldPersonID = AdoFldLong(prsDetails, "PersonID");

			while(!prsDetails->eof){

				//now loop
				nCurrentPersonID = AdoFldLong(prsDetails, "PersonID");

				if (nCurrentPersonID != nOldPersonID) {

					MU::DataPoint dataPointFamilyHistory;
					dataPointFamilyHistory.DataType = MU::FamilyHistory;
					dataPointFamilyHistory.nDenominator = 1;				

					//we know this person counts for all the denominators, so increment those
					FamilyHistory.nDenominator++;

					//we hit a new person, so let's save our information for the old one
					MU::PersonMeasureData personData;

					if (varFamilyHistory.vt != VT_NULL) {
						FamilyHistory.nNumerator++;
						dataPointFamilyHistory.nNumerator = 1;
						personData.nNumerator = 1;
						dataPointFamilyHistory.strVisibleData = VarString(varFamilyHistory);
					}
					else {
						dataPointFamilyHistory.nNumerator = 0;
						personData.nNumerator = 0;
					}

					//now that we have all the values, add them to the patient
					personData.nPersonID = nOldPersonID;	
					personData.DataPoints.push_back(dataPointFamilyHistory);				

					Menu04Data.MeasureInfo.push_back(personData);
					Menu04Data.nDenominator++;
					Menu04Data.nNumerator += personData.nNumerator;

					//now reset all the variables
					nOldPersonID = nCurrentPersonID;
					varFamilyHistory = g_cvarNull;				
				}	


				_variant_t varTemp = prsDetails->Fields->Item["Data"]->Value;
				if (varTemp.vt != VT_NULL) {
					//varFamilyHistory = varTemp;
					// (s.dhole 2014-05-19 10:30) - PLID 59583  Diplay yes if patient exust in numerator and denominator
					varFamilyHistory = _variant_t("Yes");
				}		

				prsDetails->MoveNext();
			}

			//now we have to add the last one
			MU::DataPoint dataPointFamilyHistory;
			dataPointFamilyHistory.DataType = MU::FamilyHistory;
			dataPointFamilyHistory.nDenominator = 1;				

			//we know this person counts for all the denominators, so increment those
			FamilyHistory.nDenominator++;

			//we hit a new person, so let's save our information for the old one
			// (j.armen 2013-08-02 16:30) - PLID 57804 - Declare new PersonMeasureData object
			MU::PersonMeasureData personData;

			if (varFamilyHistory.vt != VT_NULL) {
				FamilyHistory.nNumerator++;
				dataPointFamilyHistory.nNumerator = 1;
				personData.nNumerator = 1;
				dataPointFamilyHistory.strVisibleData = VarString(varFamilyHistory);
			}
			else {
				dataPointFamilyHistory.nNumerator = 0;
				personData.nNumerator = 0;
			}

			//now that we have all the values, add them to the patient
			personData.nPersonID = nOldPersonID;	
			personData.DataPoints.push_back(dataPointFamilyHistory);				

			Menu04Data.MeasureInfo.push_back(personData);
			Menu04Data.nDenominator++;
			Menu04Data.nNumerator += personData.nNumerator;
		}
	}

	// add our data point info to our vector
	Menu04Data.DataPointInfo.push_back(FamilyHistory);	
 
	return Menu04Data;
}
#include "stdafx.h"
#include "MUMeasure_CORE_09.h"
// (j.gruber 2012-10-25 09:19) - PLID 53526
CMUMeasure_CORE_09::CMUMeasure_CORE_09(void)
{
}

CMUMeasure_CORE_09::~CMUMeasure_CORE_09(void)
{
}

CSqlFragment CMUMeasure_CORE_09::GetMeasureSql()
{
	CString str;
	str.Format(
		" SET NOCOUNT ON; \r\n "	
		" DECLARE @EMRMasterID_MU_CORE_09 INT; \r\n "
		" SET @EMRMasterID_MU_CORE_09 = (SELECT IntParam FROM ConfigRT WHERE Name = 'CCHITReportInfo_%s');\r\n"
		"DECLARE @CORE09Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		" PersonID int, Data nVarChar(4000) \r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE09Data \r\n"
		"SELECT MUPatientEMRBaseQ.PersonID, "
		" SmokingQ.Data \r\n "	
		"FROM \r\n",
		GetInternalName()	);

	CSqlFragment f(str);

	//(s.dhole 8/1/2014 3:49 PM ) - PLID  63044 return EMRID 
	f += GetPatientEMRIDBaseQuery();

	// (r.farnworth 2014-05-21 15:15) - PLID 62222 - Was joining the base query on EMRID when it needed to look at PersonID
	//Then join the interesting stuff from this measure...
	//(s.dhole 8/1/2014 3:49 PM ) - PLID  63044 Numerator was always calculating all emns from all providers , It should calculate  only those emn which return from filter
	f += CSqlFragment(
		"  LEFT JOIN   \r\n "
		"  (SELECT EMRDetailsT.EMRID, EMRMasterT.PatientID, CASE WHEN EMRInfoT.DataType IN ({INT},{INT}) THEN EMRSelectDataT.Data WHEN EMRInfoT.DataType = {INT} THEN EMRTableDataT.Data ELSE NULL END as Data  \r\n "
		"  FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"  INNER JOIN EMRInfoT ON EmrDetailsT.EmrInfoID = EMRInfoT.ID   \r\n "
		"  LEFT JOIN (SELECT EMRSelectT.EMRDetailID, EMRDataT.Data   \r\n "
		"  	  FROM	EMRSelectT INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  \r\n "
		" 	  INNER JOIN EMRInfoT InnerEMRInfoT ON EMRDataT.EMRInfoID = InnerEMRInfoT.ID\r\n "
		"  	  WHERE InnerEMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_CORE_09\r\n "
		"  	) EMRSelectDataT ON EMRDetailsT.ID = EMRSelectDataT.EMRDetailID  \r\n "
		"  	LEFT JOIN   \r\n "
		"  	(SELECT EMRDetailTableDataT.EMRDetailID, EMRDetailTableDataT.Data  \r\n "
		"  		FROM EMRDetailTableDataT	   \r\n "
		"  		INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID   \r\n "
		"  		INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID   \r\n "
		" 		INNER JOIN EMRInfoT InnerEMRInfoT ON EMRDataT_X.EMRInfoID = InnerEMRInfoT.ID\r\n "
		"  		WHERE InnerEMRInfoT.EMRInfoMasterID = @EMRMasterID_MU_CORE_09 \r\n "
		"  	) EMRTableDataT ON EMRDetailsT.ID = EMRTableDataT.EMRDetailID  \r\n "
		"  	WHERE EMRDetailsT.Deleted = 0 AND (EMRSelectDataT.Data IS NOT NULL OR EMRTableDataT.Data IS NOT NULL)  \r\n "
		//(s.dhole 8/1/2014 3:49 PM ) - PLID  63044  
		" ) SmokingQ ON  MUPatientEMRBaseQ.EMRID =SmokingQ.EMRID "
		"    \r\n "
		"  WHERE CASE WHEN MUPatientEMRBaseQ.PatientAge IS NULL THEN NULL ELSE CASE WHEN MUPatientEMRBaseQ.PatientAge = '' THEN NULL ELSE CASE WHEN isnumeric(convert(nvarchar, MUPatientEMRBaseQ.PatientAge) + 'e0') <> 0 THEN MUPatientEMRBaseQ.PatientAge else 0 END END END >= 13   \r\n "
		, eitSingleList, eitMultiList, eitTable
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF; \r\n"
		""
		"SELECT * FROM @CORE09Data ORDER BY PersonID \r\n ");

	CString str1 = f.Flatten();

	return f;
}

MU::MeasureData CMUMeasure_CORE_09::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core09Data;
	Core09Data.MeasureID = MU::MU_CORE_09;
	Core09Data.nDenominator = 0;
	Core09Data.nNumerator = 0;
	Core09Data.dblRequiredPercent = GetRequirePrecent();
	Core09Data.strFullName = GetFullName();
	Core09Data.strShortName = GetShortName();
	Core09Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Smoking = MU::DataPointInfo("Smoking Status", MU::SmokingStatus);
	
	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	long nCurrentPersonID;
	long nOldPersonID = -1;

	_variant_t varSmoking = g_cvarNull;	

	// fill our measure data
	if(prsDetails){
		if (!prsDetails->eof) {
			nOldPersonID = AdoFldLong(prsDetails, "PersonID");

			while(!prsDetails->eof){

				//now loop
				nCurrentPersonID = AdoFldLong(prsDetails, "PersonID");

				if (nCurrentPersonID != nOldPersonID) {

					MU::DataPoint dataPointSmoking;
					dataPointSmoking.DataType = MU::SmokingStatus;
					dataPointSmoking.nDenominator = 1;				

					//we know this person counts for all the denominators, so increment those
					Smoking.nDenominator++;

					//we hit a new person, so let's save our information for the old one
					// (j.armen 2013-08-02 16:30) - PLID 57804 - Declare new PersonMeasureData object
					MU::PersonMeasureData personData;

					if (varSmoking.vt != VT_NULL) {
						Smoking.nNumerator++;
						dataPointSmoking.nNumerator = 1;
						personData.nNumerator = 1;
						dataPointSmoking.strVisibleData = VarString(varSmoking);
					}
					else {
						dataPointSmoking.nNumerator = 0;
						personData.nNumerator = 0;
					}

					//now that we have all the values, add them to the patient
					personData.nPersonID = nOldPersonID;	
					personData.DataPoints.push_back(dataPointSmoking);				

					Core09Data.MeasureInfo.push_back(personData);
					Core09Data.nDenominator++;
					Core09Data.nNumerator += personData.nNumerator;

					//now reset all the variables
					nOldPersonID = nCurrentPersonID;
					varSmoking = g_cvarNull;				
				}	


				_variant_t varTemp = prsDetails->Fields->Item["Data"]->Value;
				if (varTemp.vt != VT_NULL) {
					varSmoking = varTemp;
				}		

				prsDetails->MoveNext();
			}

			//now we have to add the last one
			MU::DataPoint dataPointSmoking;
			dataPointSmoking.DataType = MU::SmokingStatus;
			dataPointSmoking.nDenominator = 1;				

			//we know this person counts for all the denominators, so increment those
			Smoking.nDenominator++;

			//we hit a new person, so let's save our information for the old one
			// (j.armen 2013-08-02 16:30) - PLID 57804 - Declare new PersonMeasureData object
			MU::PersonMeasureData personData;

			if (varSmoking.vt != VT_NULL) {
				Smoking.nNumerator++;
				dataPointSmoking.nNumerator = 1;
				personData.nNumerator = 1;
				dataPointSmoking.strVisibleData = VarString(varSmoking);
			}
			else {
				dataPointSmoking.nNumerator = 0;
				personData.nNumerator = 0;
			}

			//now that we have all the values, add them to the patient
			personData.nPersonID = nOldPersonID;	
			personData.DataPoints.push_back(dataPointSmoking);				

			Core09Data.MeasureInfo.push_back(personData);
			Core09Data.nDenominator++;
			Core09Data.nNumerator += personData.nNumerator;
		}
	}

	// add our data point info to our vector
	Core09Data.DataPointInfo.push_back(Smoking);	
 
	return Core09Data;
}
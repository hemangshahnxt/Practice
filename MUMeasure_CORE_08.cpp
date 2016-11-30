#include "stdafx.h"
#include "MUMeasure_CORE_08.h"
// (j.gruber 2012-10-25 09:17) - PLID 53525
CMUMeasure_CORE_08::CMUMeasure_CORE_08(void)
{
}

CMUMeasure_CORE_08::~CMUMeasure_CORE_08(void)
{
}

CSqlFragment CMUMeasure_CORE_08::GetMeasureSql()
{
	//TES 12/19/2013 - PLID 59972 - Added PatientAge to the output, took out the age filtering (Height and Weight don't filter on it, but Blood Pressure does)
	// (j.jones 2014-11-06 11:52) - PLID 64088 - cached ConfigRT properties in order to improve query plans
	
	CSqlFragment f(
		" SET NOCOUNT ON; \r\n "

		"DECLARE @cchitPatientsWithHeightRecorded int; "
		"SET @cchitPatientsWithHeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Height recorded') \r\n"

		"DECLARE @cchitPatientsWithHeightRecorded2 int; "
		"SET @cchitPatientsWithHeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Height recorded2') \r\n"

		"DECLARE @cchitPatientsWithWeightRecorded int; "
		"SET @cchitPatientsWithWeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Weight recorded') \r\n"

		"DECLARE @cchitPatientsWithWeightRecorded2 int; "
		"SET @cchitPatientsWithWeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Weight recorded2') \r\n"

		"DECLARE @cchitPatientsWithBloodPressureRecorded int; "
		"SET @cchitPatientsWithBloodPressureRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Blood Pressure recorded') \r\n"

		"DECLARE @cchitPatientsWithBloodPressureRecorded2 int; "
		"SET @cchitPatientsWithBloodPressureRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Blood Pressure recorded2') \r\n"

		"DECLARE @CORE08Data table (\r\n" 		
		" PersonID int, Height nVarChar(255), Weight nVarChar(255), BP nVarChar(255), PatientAge int\r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE08Data\r\n"
		"SELECT PersonID as PersonID, " 
		" HeightQ.Data as Weight, \r\n "
		" WeightQ.Data as Height, \r\n "
		" BPQ.Data as BP, \r\n "
		"CASE WHEN MUPatientEMRBaseQ.PatientAge IS NULL THEN NULL ELSE CASE WHEN MUPatientEMRBaseQ.PatientAge = '' THEN NULL ELSE CASE WHEN isnumeric(convert(nvarchar, MUPatientEMRBaseQ.PatientAge) + 'e0') <> 0 THEN MUPatientEMRBaseQ.PatientAge else 0 END END END AS PatientAge \r\n"
		"FROM\r\n"
	);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	// (j.jones 2014-02-05 13:47) - PLID 60660 - this measure can optionally have a slider configured
	// (b.savon 2014-05-20 15:36) - PLID 62213 - Fix the MUS1 Core 8 detailed report to look at historical emn's; not just the emn's within the patients seen reporting period
	//(s.dhole 3/16/2015 3:06 PM ) - PLID 63705  replace  EMRDetailTableDropDownDataQ with EMRDetailTableDropDownDataNoFilterQ  to improve SQl Time out
	f += CSqlFragment(		
		" LEFT JOIN  \r\n "
		" (SELECT EMRDetailsT.EMRID, CASE WHEN EMRInfoT.DataType IN (2,3) THEN EMRSelectDataT.Data "
		"	WHEN EMRInfoT.DataType = 5 THEN EMRSliderDataT.Data \r\n "
		"	WHEN EMRInfoT.DataType = 7 THEN EMRTableDataT.Data ELSE NULL END as Data, EMRMasterT.PatientID \r\n "
		" FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID INNER JOIN EMRInfoT ON EmrDetailsT.EmrInfoID = EMRInfoT.ID  \r\n "
		" LEFT JOIN (SELECT EMRSelectT.EMRDetailID, EMRDataT.Data  \r\n "
		" 	  FROM	EMRSelectT INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID \r\n "
		" 	  WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) \r\n "
		" 	) EMRSelectDataT ON EMRDetailsT.ID = EMRSelectDataT.EMRDetailID \r\n "
		" 	LEFT JOIN  \r\n "
		" 	(SELECT EMRDetailTableDataT.EMRDetailID, CASE WHEN EMRDataT_Y.ListType = {INT} THEN DropDownDataQ.Data ELSE EMRDetailTableDataT.Data END as DATA \r\n "
		" 		FROM EMRDetailTableDataT	  \r\n "
		" 		INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  \r\n "
		" 		INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  \r\n "
		"		LEFT JOIN ( SELECT OuterQ.EMRDetailTableDataID,  STUFF((SELECT ', ' + EMRTableDropDownInfoT.Data FROM EMRDetailTableDropDownDataNoFilterQ Q INNER JOIN EMRTableDropDownInfoT ON Q.DropDownInfoID = EMRTableDropDownInfoT.ID WHERE Q.EMRDetailTableDataID = OuterQ.EMRDetailTableDataID FOR XML PATH(''), TYPE).value('/', 'NVARCHAR(MAX)'), 1,2,'') as Data  FROM EMRDetailTableDropDownDataNoFilterQ OuterQ) DropDownDataQ ON EMRDetailTableDataT.ID = DropDownDataQ.EmrDetailTableDataID \r\n "
		" 		WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) \r\n "
		" 	) EMRTableDataT ON EMRDetailsT.ID = EMRTableDataT.EMRDetailID \r\n "
		"	LEFT JOIN \r\n"
		"	(SELECT EMRDetailsT.ID AS EMRDetailID, Convert(nvarchar, EMRDetailsT.SliderValue) AS Data\r\n "
		"	FROM EMRDetailsT \r\n"
		"	INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"	WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithWeightRecorded2) \r\n"
		"	AND EMRDetailsT.SliderValue Is Not Null \r\n"
		" 	) EMRSliderDataT ON EMRDetailsT.ID = EMRSliderDataT.EMRDetailID \r\n "
		" 	WHERE EMRDetailsT.Deleted = 0 AND (EMRSelectDataT.Data IS NOT NULL OR EMRTableDataT.Data IS NOT NULL OR EMRSliderDataT.Data IS NOT NULL) \r\n "
		" ) WeightQ ON MUPatientEMRBaseQ.PersonID = WeightQ.PatientID \r\n "
		" LEFT JOIN \r\n "
		" (SELECT EMRDetailsT.EMRID, CASE WHEN EMRInfoT.DataType IN (2,3) THEN EMRSelectDataT.Data "
		"	WHEN EMRInfoT.DataType = 5 THEN EMRSliderDataT.Data \r\n "
		"	WHEN EMRInfoT.DataType = 7 THEN EMRTableDataT.Data ELSE NULL END as Data, EMRMasterT.PatientID \r\n "
		" FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID INNER JOIN EMRInfoT ON EmrDetailsT.EmrInfoID = EMRInfoT.ID  \r\n "
		" LEFT JOIN (SELECT EMRSelectT.EMRDetailID, EMRDataT.Data  \r\n "
		" 	  FROM	EMRSelectT INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID \r\n "
		" 	  WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) \r\n "
		" 	) EMRSelectDataT ON EMRDetailsT.ID = EMRSelectDataT.EMRDetailID \r\n "
		" 	LEFT JOIN  \r\n "
		" 	(SELECT EMRDetailTableDataT.EMRDetailID, CASE WHEN EMRDataT_Y.ListType = {INT} THEN DropDownDataQ.Data ELSE EMRDetailTableDataT.Data END as DATA \r\n "
		" 		FROM EMRDetailTableDataT	  \r\n "
		" 		INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  \r\n "
		" 		INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  \r\n "
		"		LEFT JOIN ( SELECT OuterQ.EMRDetailTableDataID,  STUFF((SELECT ', ' + EMRTableDropDownInfoT.Data FROM EMRDetailTableDropDownDataNoFilterQ Q INNER JOIN EMRTableDropDownInfoT ON Q.DropDownInfoID = EMRTableDropDownInfoT.ID WHERE Q.EMRDetailTableDataID = OuterQ.EMRDetailTableDataID FOR XML PATH(''), TYPE).value('/', 'NVARCHAR(MAX)'), 1,2,'') as Data  FROM EMRDetailTableDropDownDataNoFilterQ OuterQ) DropDownDataQ ON EMRDetailTableDataT.ID = DropDownDataQ.EmrDetailTableDataID \r\n "
		" 		WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) \r\n "
		" 	) EMRTableDataT ON EMRDetailsT.ID = EMRTableDataT.EMRDetailID \r\n "
		"	LEFT JOIN \r\n"
		"	(SELECT EMRDetailsT.ID AS EMRDetailID, Convert(nvarchar, EMRDetailsT.SliderValue) AS Data\r\n "
		"	FROM EMRDetailsT \r\n"
		"	INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"	WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithHeightRecorded2) \r\n"
		"	AND EMRDetailsT.SliderValue Is Not Null \r\n"
		" 	) EMRSliderDataT ON EMRDetailsT.ID = EMRSliderDataT.EMRDetailID \r\n "
		" 	WHERE EMRDetailsT.Deleted = 0 AND (EMRSelectDataT.Data IS NOT NULL OR EMRTableDataT.Data IS NOT NULL OR EMRSliderDataT.Data IS NOT NULL) \r\n "
		" ) HeightQ ON MUPatientEMRBaseQ.PersonID = HeightQ.PatientID \r\n "
		" LEFT JOIN \r\n "
		" (SELECT EMRDetailsT.EMRID, CASE WHEN EMRInfoT.DataType IN (2,3) THEN EMRSelectDataT.Data "
		"	WHEN EMRInfoT.DataType = 5 THEN EMRSliderDataT.Data \r\n "
		"	WHEN EMRInfoT.DataType = 7 THEN EMRTableDataT.Data ELSE NULL END as Data, EMRMasterT.PatientID \r\n "
		" FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID INNER JOIN EMRInfoT ON EmrDetailsT.EmrInfoID = EMRInfoT.ID  \r\n "
		" LEFT JOIN (SELECT EMRSelectT.EMRDetailID, EMRDataT.Data  \r\n "
		" 	  FROM	EMRSelectT INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID \r\n "
		" 	  WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithBloodPressureRecorded) \r\n "
		" 	) EMRSelectDataT ON EMRDetailsT.ID = EMRSelectDataT.EMRDetailID \r\n "
		" 	LEFT JOIN  \r\n "
		" 	(SELECT EMRDetailTableDataT.EMRDetailID, CASE WHEN EMRDataT_Y.ListType = {INT} THEN DropDownDataQ.Data ELSE EMRDetailTableDataT.Data END as DATA \r\n "
		" 		FROM EMRDetailTableDataT	  \r\n "
		" 		INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  \r\n "
		" 		INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  \r\n "
		"		LEFT JOIN ( SELECT OuterQ.EMRDetailTableDataID,  STUFF((SELECT ', ' + EMRTableDropDownInfoT.Data FROM EMRDetailTableDropDownDataNoFilterQ Q INNER JOIN EMRTableDropDownInfoT ON Q.DropDownInfoID = EMRTableDropDownInfoT.ID WHERE Q.EMRDetailTableDataID = OuterQ.EMRDetailTableDataID FOR XML PATH(''), TYPE).value('/', 'NVARCHAR(MAX)'), 1,2,'') as Data  FROM EMRDetailTableDropDownDataNoFilterQ OuterQ) DropDownDataQ ON EMRDetailTableDataT.ID = DropDownDataQ.EmrDetailTableDataID \r\n "
		" 		WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithBloodPressureRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithBloodPressureRecorded) \r\n "
		" 	) EMRTableDataT ON EMRDetailsT.ID = EMRTableDataT.EMRDetailID \r\n "
		"	LEFT JOIN \r\n"
		"	(SELECT EMRDetailsT.ID AS EMRDetailID, Convert(nvarchar, EMRDetailsT.SliderValue) AS Data\r\n "
		"	FROM EMRDetailsT \r\n"
		"	INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"	WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithBloodPressureRecorded2) \r\n"
		"	AND EMRDetailsT.SliderValue Is Not Null \r\n"
		" 	) EMRSliderDataT ON EMRDetailsT.ID = EMRSliderDataT.EMRDetailID \r\n "
		" 	WHERE EMRDetailsT.Deleted = 0 AND (EMRSelectDataT.Data IS NOT NULL OR EMRTableDataT.Data IS NOT NULL OR EMRSliderDataT.Data IS NOT NULL) \r\n "
		" ) BPQ ON MUPatientEMRBaseQ.PersonID = BPQ.PatientID \r\n "
		"  \r\n "
		"  \r\n "
		, LIST_TYPE_DROPDOWN, LIST_TYPE_DROPDOWN, LIST_TYPE_DROPDOWN
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF "
		""
		"SELECT * FROM @CORE08Data ORDER BY PersonID \r\n ");
		
		
	CString str1 = f.Flatten();

	return f;
}

MU::MeasureData CMUMeasure_CORE_08::GetMeasureInfo()
{
	// create a measure data
	MU::MeasureData Core08Data;
	Core08Data.MeasureID = MU::MU_CORE_08;
	Core08Data.nDenominator = 0;
	Core08Data.nNumerator = 0;
	Core08Data.dblRequiredPercent = GetRequirePrecent();
	Core08Data.strFullName = GetFullName();
	Core08Data.strShortName = GetShortName();
	Core08Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo Height = MU::DataPointInfo("Height", MU::Height);
	MU::DataPointInfo Weight = MU::DataPointInfo("Weight", MU::Weight);
	MU::DataPointInfo BP = MU::DataPointInfo("BP", MU::BloodPressure);	

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	CIncreaseCommandTimeout cict(pConn, 600);
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	long nCurrentPersonID;
	long nOldPersonID = -1;

	_variant_t varHeight = g_cvarNull;
	_variant_t varWeight = g_cvarNull;
	_variant_t varBP = g_cvarNull;
	_variant_t varPatientAge = g_cvarNull;

	// fill our measure data
	if(prsDetails){
		if (!prsDetails->eof) {
			nOldPersonID = AdoFldLong(prsDetails, "PersonID");

			while(!prsDetails->eof){

				//now loop
				nCurrentPersonID = AdoFldLong(prsDetails, "PersonID");

				if (nCurrentPersonID != nOldPersonID) {

					MU::DataPoint dataPointHeight;
					dataPointHeight.DataType = MU::Height;
					dataPointHeight.nDenominator = 1;

					MU::DataPoint dataPointWeight;
					dataPointWeight.DataType = MU::Weight;
					dataPointWeight.nDenominator = 1;

					//TES 12/19/2013 - PLID 59972 - We only look at blood pressure for patients older than 3
					BOOL bIncludeBP = (varPatientAge.vt != VT_NULL && VarLong(varPatientAge) >= 3);

					MU::DataPoint dataPointBP;
					dataPointBP.DataType = MU::BloodPressure;
					dataPointBP.nDenominator = 1;

					//we know this person counts for all the denominators, so increment those
					Height.nDenominator++;
					Weight.nDenominator++;
					if(bIncludeBP) {
						BP.nDenominator++;
					}

					//we hit a new person, so let's save our information for the old one
					if (varHeight.vt != VT_NULL) {
						Height.nNumerator++;
						dataPointHeight.nNumerator = 1;
						dataPointHeight.strVisibleData = VarString(varHeight);
					}
					else {
						dataPointHeight.nNumerator = 0;
					}
					if (varWeight.vt != VT_NULL) {
						Weight.nNumerator++;
						dataPointWeight.nNumerator = 1;
						dataPointWeight.strVisibleData = VarString(varWeight);
					}
					else {
						dataPointWeight.nNumerator = 0;
					}
					//TES 12/19/2013 - PLID 59972 - We only look at blood pressure for patients older than 3
					if(bIncludeBP) {
						if (varBP.vt != VT_NULL) {
							BP.nNumerator++;
							dataPointBP.nNumerator = 1;
							dataPointBP.strVisibleData = VarString(varBP);
						}
						else {
							dataPointBP.nNumerator = 0;
						}
					}

					//now that we have all the values, add them to the patient
					// (j.armen 2013-08-02 16:30) - PLID 57804 - Declare new PersonMeasureData object
					MU::PersonMeasureData personData;
					personData.nPersonID = nOldPersonID;	
					personData.DataPoints.push_back(dataPointHeight);
					personData.DataPoints.push_back(dataPointWeight);
					//TES 12/19/2013 - PLID 59972 - We only look at blood pressure for patients older than 3
					if(bIncludeBP) {
						personData.DataPoints.push_back(dataPointBP);
					}

					Core08Data.MeasureInfo.push_back(personData);
					Core08Data.nDenominator++;
					Core08Data.nNumerator += personData.nNumerator;

					//now reset all the variables
					nOldPersonID = nCurrentPersonID;
					varHeight = g_cvarNull;
					varWeight = g_cvarNull;
					varBP = g_cvarNull;
					varPatientAge = g_cvarNull;
				}	


				_variant_t varTemp = prsDetails->Fields->Item["Height"]->Value;
				if (varTemp.vt != VT_NULL) {
					varHeight = varTemp;
				}

				varTemp = prsDetails->Fields->Item["Weight"]->Value;
				if (varTemp.vt != VT_NULL) {
					varWeight = varTemp;
				}

				varTemp = prsDetails->Fields->Item["BP"]->Value;
				if (varTemp.vt != VT_NULL) {
					varBP = varTemp;
				}

				varTemp = prsDetails->Fields->Item["PatientAge"]->Value;
				if (varTemp.vt != VT_NULL) {
					varPatientAge = varTemp;
				}

				prsDetails->MoveNext();
			}

			//now we have to add the last one
			MU::DataPoint dataPointHeight;
			dataPointHeight.DataType = MU::Height;
			dataPointHeight.nDenominator = 1;

			MU::DataPoint dataPointWeight;
			dataPointWeight.DataType = MU::Weight;
			dataPointWeight.nDenominator = 1;

			//TES 12/19/2013 - PLID 59972 - We only look at blood pressure for patients older than 3
			BOOL bIncludeBP = (varPatientAge.vt != VT_NULL && VarLong(varPatientAge) >= 3);
			MU::DataPoint dataPointBP;
			dataPointBP.DataType = MU::BloodPressure;
			dataPointBP.nDenominator = 1;

			//we know this person counts for all the denominators, so increment those
			Height.nDenominator++;
			Weight.nDenominator++;
			if(bIncludeBP) {
				BP.nDenominator++;
			}

			//we hit a new person, so let's save our information for the old one
			if (varHeight.vt != VT_NULL) {
				Height.nNumerator++;
				dataPointHeight.nNumerator = 1;
				dataPointHeight.strVisibleData = VarString(varHeight);
			}
			else {
				dataPointHeight.nNumerator = 0;
			}
			if (varWeight.vt != VT_NULL) {
				Weight.nNumerator++;
				// (b.savon 2014-05-02 13:05) - PLID 62022 - Looks like copy paste fail; this should be numerator
				dataPointWeight.nNumerator = 1;
				dataPointWeight.strVisibleData = VarString(varWeight);
			}
			else {
				dataPointWeight.nNumerator = 0;
			}
			//TES 12/19/2013 - PLID 59972 - We only look at blood pressure for patients older than 3
			if(bIncludeBP) {
				if (varBP.vt != VT_NULL) {
					BP.nNumerator++;
					// (b.savon 2014-05-02 13:05) - PLID 62022 - Looks like copy paste fail; this should be numerator
					dataPointBP.nNumerator = 1;
					dataPointBP.strVisibleData = VarString(varBP);
				}
				else {
					dataPointBP.nNumerator = 0;
				}
			}

			//now that we have all the values, add them to the patient
			// (j.armen 2013-08-02 16:30) - PLID 57804 - Declare new PersonMeasureData object
			MU::PersonMeasureData personData;
			personData.nPersonID = nOldPersonID;	
			personData.DataPoints.push_back(dataPointHeight);
			personData.DataPoints.push_back(dataPointWeight);
			//TES 12/19/2013 - PLID 59972 - We only look at blood pressure for patients older than 3
			if(bIncludeBP) {
				personData.DataPoints.push_back(dataPointBP);
			}

			Core08Data.MeasureInfo.push_back(personData);
			Core08Data.nDenominator++;
			Core08Data.nNumerator += personData.nNumerator;
		}
	}

	// add our data point info to our vector
	Core08Data.DataPointInfo.push_back(Height);
	Core08Data.DataPointInfo.push_back(Weight);
	Core08Data.DataPointInfo.push_back(BP);	
 
	return Core08Data;
}
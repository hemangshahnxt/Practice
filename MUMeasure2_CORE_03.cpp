#include "stdafx.h"
#include "MUMeasure2_CORE_03.h"
// (b.savon 2014-05-02 11:41) - PLID 59567 - MU.CORE.03

CMUMeasure2_CORE_03::CMUMeasure2_CORE_03(void)
{
}

CMUMeasure2_CORE_03::~CMUMeasure2_CORE_03(void)
{
}

CSqlFragment CMUMeasure2_CORE_03::GetMeasureSql()
{
	// (b.savon 2014-05-02 11:41) - PLID 59567 - MU.CORE.03 - Created, copied from stage one MU.CORE.07
	CSqlFragment f(
		" SET NOCOUNT ON; \r\n "
		"DECLARE @CORE03Data table (\r\n" /*+ */
		//These are the common demographics fields all measures have
		/*GetCommonDemographicsTableDefs() + "\r\n" +*/
		//These are the interesting data points from this particular measure
		// (b.spivey, May 22, 2013) - PLID 56869 - Changes the raceID to a "has race" bit value. 
		// (b.spivey, June 12, 2013) - PLID 56869 - nvarchar(255) to max!
		"	PersonID int, Gender int, GenderName nvarchar(50), LanguageID int NULL, Language nvarchar(100), HasRace bit NULL, Race nvarchar(max), EthnicityID int NULL, Ethnicity nvarchar(255), Birthdate datetime \r\n"
		");\r\n\r\n"

		"INSERT INTO @CORE03Data\r\n"
		"SELECT PersonT.ID as PersonID, " /*+ GetCommonDemographicsFieldNames() + "\r\n" +*/
		" Gender, CASE WHEN PersonT.Gender = 1 THEN 'Male' WHEN PersonT.Gender = 2 THEN 'Female' END as GenderName, "
		"LanguageT.ID, LanguageT.Name, "
		"PatientRaceQ.HasRace, PatientRaceQ.RaceName, "
		"EthnicityT.ID, EthnicityT.Name, PersonT.Birthdate \r\n"
		"FROM\r\n"
	);

	f += GetPatientEMRBaseQuery();

	//Then join the interesting stuff from this measure...
	f += CSqlFragment(
		//TODO:  Bleh, the main patient subquery already has all this PersonT stuff, so I can't load PersonT again.  alias?
		"INNER JOIN (SELECT ID, LanguageID, Ethnicity, Gender, BirthDate FROM PersonT) PersonT ON MUPatientEMRBaseQ.PersonID = PersonT.ID\r\n"
		"LEFT JOIN LanguageT ON PersonT.LanguageID = LanguageT.ID\r\n"
		// (b.spivey, May 22, 2013) - PLID 56869 - Pull the list of race names since the old structure is dead. 
		"LEFT JOIN ( "
		"		SELECT PersonPat.ID AS PatientPersonID, LEFT(RacePat.Name, LEN(RacePat.Name) -1) AS RaceName, CASE WHEN RacePat.Name IS NOT NULL THEN 1 ELSE 0 END AS HasRace "
		"		FROM PersonT PersonPat "
		"		CROSS APPLY  "
		"		 (  "
		"			SELECT ( " 
	 	"				SELECT RT.Name + ', '  "
	 	"				FROM PersonRaceT PRT  "
	 	"				INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
	 	"				WHERE PRT.PersonID = PersonPat.ID  "
	 	"				FOR XML PATH(''), TYPE  "
		"			).value('/','nvarchar(max)') "
		"		 ) RacePat (Name)  "
		"	) PatientRaceQ ON PatientRaceQ.PatientPersonID = PersonT.ID "
		"LEFT JOIN EthnicityT ON PersonT.Ethnicity = EthnicityT.ID\r\n"
	);

	f += CSqlFragment(
		"SET NOCOUNT OFF "
		""
		"SELECT PersonID, Gender, GenderName, LanguageID, Language, HasRace, Race, EthnicityId, Ethnicity, Birthdate FROM @CORE03Data \r\n "
		" GROUP BY PersonID, Gender, GenderName, LanguageID, Language, HasRace, Race, EthnicityId, Ethnicity, Birthdate ");
		

	return f;
}

MU::MeasureData CMUMeasure2_CORE_03::GetMeasureInfo()
{
	// (b.savon 2014-05-02 11:41) - PLID 59567 - MU.CORE.03 - Created, copied from stage one MU.CORE.07

	// create a measure data
	MU::MeasureData Core03Data;
	Core03Data.MeasureID = MU::MU2_CORE_03;
	Core03Data.nDenominator = 0;
	Core03Data.nNumerator = 0;
	Core03Data.dblRequiredPercent = GetRequirePrecent();
	Core03Data.strFullName = GetFullName();
	Core03Data.strShortName = GetShortName();
	Core03Data.strInternalName = GetInternalName();

	// possible data points
	MU::DataPointInfo lang = MU::DataPointInfo("Language", MU::Language);
	MU::DataPointInfo Gender = MU::DataPointInfo("Gender", MU::Gender);
	MU::DataPointInfo Race = MU::DataPointInfo("Race", MU::Race);
	MU::DataPointInfo Ethnicity = MU::DataPointInfo("Ethnicity", MU::Ethnicity);
	MU::DataPointInfo BirthDate = MU::DataPointInfo("BirthDate", MU::BirthDate);

	// create recordset
	// (j.armen 2013-08-02 14:55) - PLID 57803 - Use snapshot isolation
	CNxAdoConnection pConn = CreateThreadSnapshotConnection();
	ADODB::_RecordsetPtr prsDetails = CreateParamRecordset(pConn, GetMeasureSql());

	// fill our measure data
	if(prsDetails){
		while(!prsDetails->eof){
			MU::PersonMeasureData personData;
			personData.nPersonID = AdoFldLong(prsDetails, "PersonID", -1);
			long nLangID = AdoFldLong(prsDetails, "LanguageID", -1);
			bool bHasRace = !!AdoFldBool(prsDetails, "HasRace", FALSE);
			long nEthID = AdoFldLong(prsDetails, "EthnicityID", -1);
			// (b.savon 2014-05-02 11:42) - PLID 59567 - Unfilled Gender is 0
			long nGender = AdoFldLong(prsDetails, "Gender", 0);			
			
			COleDateTime dtInvalid;
			dtInvalid.SetDate(1800,12,31);
			COleDateTime dtBirthDate = AdoFldDateTime(prsDetails, "BirthDate", dtInvalid);	
			// (b.savon 2014-05-02 11:42) - PLID 59567 - Unfilled Gender is 0
			personData.nNumerator = nLangID == -1 || !bHasRace ||nEthID == -1 || nGender == 0 || dtBirthDate == dtInvalid ? 0 : 1;
			personData.nDenominator = 1;

			lang.nDenominator += personData.nDenominator;
			lang.nNumerator += nLangID == -1 ? 0 : 1;

			// (b.savon 2014-05-02 11:42) - PLID 59567 - Unfilled Gender is 0
			Gender.nDenominator += personData.nDenominator;
			Gender.nNumerator += nGender == 0 ? 0 : 1;

			Race.nDenominator += personData.nDenominator;
			// (b.spivey, May 22, 2013) - PLID 56869 - If they have race, increase the numerator. 
			Race.nNumerator += (bHasRace ? 1 : 0);

			Ethnicity.nDenominator += personData.nDenominator;
			Ethnicity.nNumerator += nEthID == -1 ? 0 : 1;

			BirthDate.nDenominator += personData.nDenominator;
			BirthDate.nNumerator  += dtBirthDate == dtInvalid ? 0 : 1;

			MU::DataPoint dataPoint;
			dataPoint.DataType = MU::Language;
			dataPoint.nDenominator = 1;
			dataPoint.nNumerator = nLangID == -1 ? 0 : 1;
			CString strVisibleData;
			strVisibleData.Format("%s", AdoFldString(prsDetails, "Language", ""));
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);
			
			dataPoint.DataType = MU::Ethnicity;
			dataPoint.nDenominator = 1;
			dataPoint.nNumerator = nEthID == -1 ? 0 : 1;
			strVisibleData = "";
			strVisibleData.Format("%s", AdoFldString(prsDetails, "Ethnicity", ""));
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);

			dataPoint.DataType = MU::Gender;
			dataPoint.nDenominator = 1;
			// (b.savon 2014-05-02 11:42) - PLID 59567 - Unfilled Gender is 0
			dataPoint.nNumerator = nGender == 0 ? 0 : 1;
			strVisibleData = "";
			strVisibleData.Format("%s", AdoFldString(prsDetails, "GenderName", ""));
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);

			dataPoint.DataType = MU::Race;
			dataPoint.nDenominator = 1;
			// (b.spivey, May 22, 2013) - PLID 56869 - increase the numerator if they have a race.
			dataPoint.nNumerator = (bHasRace ? 1 : 0);
			strVisibleData = "";
			strVisibleData.Format("%s", AdoFldString(prsDetails, "Race", ""));
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);

			dataPoint.DataType = MU::BirthDate;
			dataPoint.nDenominator = 1;
			dataPoint.nNumerator = dtBirthDate == dtInvalid ? 0 : 1;
			strVisibleData = "";
			if (dtBirthDate != dtInvalid) {
				strVisibleData.Format("%s", FormatDateTimeForInterface(dtBirthDate));
			}
			dataPoint.strVisibleData = strVisibleData;
			personData.DataPoints.push_back(dataPoint);

			Core03Data.MeasureInfo.push_back(personData);
			Core03Data.nDenominator += personData.nDenominator;
			Core03Data.nNumerator += personData.nNumerator;

			prsDetails->MoveNext();
		}
	}

	// add our data point info to our vector
	Core03Data.DataPointInfo.push_back(lang);
	Core03Data.DataPointInfo.push_back(Race);
	Core03Data.DataPointInfo.push_back(Ethnicity);
	Core03Data.DataPointInfo.push_back(Gender);
	Core03Data.DataPointInfo.push_back(BirthDate);
 
	return Core03Data;
}

#include "stdafx.h"
#include "ImportValidationUtils.h"
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
#include "CachedData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// (r.farnworth 2015-03-27 11:34) - PLID 65163 - Return the error message of a validation failure
// (b.savon 2015-04-10 07:40) - PLID 65223 - Added record type so that validation messages are dictated by requirements per type
CString LookupErrorMessage(const CString &strColumnTitle, int nMaxLength, bool bIncludeLength, ImportRecordType irtRecordType)
{
	CString strErrorMessage;

	if (strColumnTitle == GetImportFieldHeader(ifnIgnore))
	{
		strErrorMessage = "This column header has been assigned to another column which may not contain invalid information.";
		return strErrorMessage;
	}

	
	strErrorMessage.Format("The %s field: \r\n \r\n", strColumnTitle);

	// (b.savon 2015-04-02 09:13) - PLID 65236 - Add ifnRecallTemplateName
	// (r.farnworth 2015-04-06 14:50) - PLID 65168 - Add ifnInsCoName
	// (r.farnworth 2015-04-29 11:15) - PLID 65525 - Added all boolean values
	// (r.goldschmidt 2016-02-11 15:12) - PLID 68163 - add race preferred name
	if (strColumnTitle == GetImportFieldHeader(ifnUserUsername)
		|| strColumnTitle == GetImportFieldHeader(ifnResourceName)
		|| strColumnTitle == GetImportFieldHeader(ifnProductName)
		|| strColumnTitle == GetImportFieldHeader(ifnServiceCode)
		|| strColumnTitle == GetImportFieldHeader(ifnLastName)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomPatientID)
		|| strColumnTitle == GetImportFieldHeader(ifnRecallTemplateName)
		|| strColumnTitle == GetImportFieldHeader(ifnInsCoName)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsEvent)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsCancelled)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsConfirmed)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsNoShow)
		|| strColumnTitle == GetImportFieldHeader(ifnUserPatientCoord)
		|| strColumnTitle == GetImportFieldHeader(ifnOtherContactNurse)
		|| strColumnTitle == GetImportFieldHeader(ifnOtherContactAnesthesiologist)
		|| strColumnTitle == GetImportFieldHeader(ifnTaxable1)
		|| strColumnTitle == GetImportFieldHeader(ifnTaxable2)
		|| strColumnTitle == GetImportFieldHeader(ifnProductTaxable1)
		|| strColumnTitle == GetImportFieldHeader(ifnProductTaxable2)
		|| strColumnTitle == GetImportFieldHeader(ifnProductBillable)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox1)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox2)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox3)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox4)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox5)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox6)
		|| strColumnTitle == GetImportFieldHeader(ifnPatientNoteDateTime)
		|| strColumnTitle == GetImportFieldHeader(ifnRacePreferredName))
	{
		strErrorMessage += "Cannot be blank \r\n";
	}

	// (r.farnworth 2015-04-06 15:24) - PLID 65169 - Added ifnConversionID
	// (r.goldschmidt 2016-02-11 15:12) - PLID 68163 - add race preferred name
	if (strColumnTitle == GetImportFieldHeader(ifnUserUsername)
		|| (strColumnTitle == GetImportFieldHeader(ifnResourceName) && irtRecordType == irtResources)
		|| strColumnTitle == GetImportFieldHeader(ifnProductName)
		|| strColumnTitle == GetImportFieldHeader(ifnServiceCode)
		|| strColumnTitle == GetImportFieldHeader(ifnPatientID)
		|| strColumnTitle == GetImportFieldHeader(ifnProductBarcode)
		|| strColumnTitle == GetImportFieldHeader(ifnConversionID)
		|| strColumnTitle == GetImportFieldHeader(ifnRacePreferredName))
	{
		strErrorMessage += "Cannot already exist in the database \r\n";
	}

	// (r.farnworth 2015-04-06 15:24) - PLID 65169 - Added ifnConversionID
	//(s.dhole 4/13/2015 8:18 AM ) - PLID 65226  Exlcude PatientNotes
	//(s.dhole 4/28/2015 9:05 AM ) - PLID 65194 Exclude InsuredParties
	// (r.goldschmidt 2016-02-11 15:12) - PLID 68163 - add race preferred name
	if ((irtRecordType != irtPatientNotes && irtRecordType != irtInsuredParties) && (strColumnTitle == GetImportFieldHeader(ifnUserUsername)
		|| (strColumnTitle == GetImportFieldHeader(ifnResourceName) && irtRecordType == irtResources)
		|| strColumnTitle == GetImportFieldHeader(ifnProductName)
		|| strColumnTitle == GetImportFieldHeader(ifnServiceCode)
		|| strColumnTitle == GetImportFieldHeader(ifnPatientID)
		|| strColumnTitle == GetImportFieldHeader(ifnProductBarcode)
		|| strColumnTitle == GetImportFieldHeader(ifnConversionID)
		|| strColumnTitle == GetImportFieldHeader(ifnRacePreferredName)))
	{
		strErrorMessage += "Cannot exist elsewhere in the import file \r\n";
	}

	//(s.dhole 4/29/2015 3:55 PM ) - PLID 
	if (strColumnTitle == GetImportFieldHeader(ifnInsuredPartyRespTypeID))
	{
		strErrorMessage += "Cannot exist elsewhere in the import file or database with link to unique patient record\r\n";
	}
	 

	if (strColumnTitle == GetImportFieldHeader(ifnProductOnHand)
		|| strColumnTitle == GetImportFieldHeader(ifnProductPrice)
		|| strColumnTitle == GetImportFieldHeader(ifnServicePrice))
	{
		strErrorMessage += "Cannot be less than 0 \r\n";
	}

	if (strColumnTitle == GetImportFieldHeader(ifnBirthdate))
	{
		strErrorMessage += "Cannot be greater than today's date \r\n";
	}

	//(s.dhole 5/6/2015 9:50 AM ) - PLID 65755
	if (strColumnTitle == GetImportFieldHeader(ifnInsuredInactiveDate))
	{
			strErrorMessage += "Cannot be less than or equal today's date of active insurance\r\n";
	}


	// (b.savon 2015-04-02 09:13) - PLID 65236 - Add ifnRecallTemplateName
	// (r.farnworth 2015-04-06 16:01) - PLID 65189 - Added Ins. Co. PayerIDs
	// (b.savon 2015-04-10 13:00) - PLID 65221 - Add Appointment Purpose
	// (b.savon 2015-04-10 13:01) - PLID 65220 - Add Appointment Type
	//(s.dhole 4/7/2015 10:07 AM ) - PLID 65229 Added ifnInsuranceCompanyConversionID,ifnInsuranceCompanyName
	if (strColumnTitle == GetImportFieldHeader(ifnRace))
	{
		strErrorMessage += "Must already exist in the Race List in General 2. Values are matched to entries in that list by name, CDCID, or hierarchical code.\r\n";
	}

	// (r.goldschmidt 2016-02-11 15:13) - PLID 68163 - Added race cdc code
	if (strColumnTitle == GetImportFieldHeader(ifnEthnicity)
		|| strColumnTitle == GetImportFieldHeader(ifnLanguage)
		|| strColumnTitle == GetImportFieldHeader(ifnLocation)
		|| strColumnTitle == GetImportFieldHeader(ifnReferralSourceName)
		|| strColumnTitle == GetImportFieldHeader(ifnRecallTemplateName)
		|| strColumnTitle == GetImportFieldHeader(ifnHCFAPayerID)
		|| strColumnTitle == GetImportFieldHeader(ifnUBPayerID)
		|| strColumnTitle == GetImportFieldHeader(ifnEligibilityPayerID)
		|| (strColumnTitle == GetImportFieldHeader(ifnResourceName) && irtRecordType == irtAppointments)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentPurpose)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentType)
		|| strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyConversionID)
		|| strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyName)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomPatientID)
		|| strColumnTitle == GetImportFieldHeader(ifnInsuredPartyRespTypeID)
		|| strColumnTitle == GetImportFieldHeader(ifnRaceCDCCode)
		)
	{
		strErrorMessage += "Must already exist in the database \r\n";
	}

	// (b.savon 2015-04-02 09:13) - PLID 65236 - Add ifnRecallTemplateName
	//(s.dhole 4/7/2015 3:55 PM ) - PLID 65230 Added ifnPatientNoteCategory

	//(s.dhole 4/15/2015 2:30 PM ) - PLID 65195 Add InsureedParty   Resp type
	//(s.dhole 4/7/2015 10:07 AM ) - PLID 65229 Added ifnInsuranceCompanyConversionID	ifnInsuranceCompanyName

	// (b.savon 2015-03-26 13:26) - PLID 65151 - Add ifnReferringPhysicianName
	// (b.savon 2015-03-26 14:23) - PLID 65152 - Add ifnPrimaryCarePhysicianName 

	if (strColumnTitle == GetImportFieldHeader(ifnCustomPatientID)
		|| strColumnTitle == GetImportFieldHeader(ifnProviderName)
		|| strColumnTitle == GetImportFieldHeader(ifnRecallTemplateName)
		|| strColumnTitle == GetImportFieldHeader(ifnPatientNoteCategory)
		|| strColumnTitle == GetImportFieldHeader(ifnInsuredPartyRespTypeID)
		|| strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyConversionID)
		|| strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyName)
		|| strColumnTitle == GetImportFieldHeader(ifnReferringPhysicianName)
		|| strColumnTitle == GetImportFieldHeader(ifnPrimaryCarePhysicianName))

	{
		strErrorMessage += "Must match unique data already existing in the database \r\n";
	}

	if (strColumnTitle == GetImportFieldHeader(ifnPatientCurrentStatus))
	{
		strErrorMessage += "Must be either 'Patient', 'Prospect', or 'Patient Prospect' \r\n";
	}

	//(s.dhole 4/30/2015 9:05 AM ) - PLID 65229 Note Priority error
	if (strColumnTitle == GetImportFieldHeader(ifnPatientNotePriority))
	{
		strErrorMessage += "Must be either 'High', 'Medium', 'Low', '1', '2' , '3'  or empty \r\n";
	}
	
	


	// (b.savon 2015-03-31 10:58) - PLID 65235 - Add Recall Date
	//(s.dhole 4/7/2015 10:16 AM ) - PLID 65227 Added Note date
	// (b.savon 2015-04-07 15:20) - PLID 65218 - Add Appointment Date
	// (b.savon 2015-04-10 13:26) - PLID 65219 - Add Appointment object Start Time, End Time
	//(s.dhole 4/28/2015 9:07 AM ) - PLID 65195  added InsuredEffectiveDate,InsuredInactiveDate
	if (strColumnTitle == GetImportFieldHeader(ifnFirstContactDate) || 
		strColumnTitle == GetImportFieldHeader(ifnRecallDate) || 
		strColumnTitle == GetImportFieldHeader(ifnAppointmentDate) ||
		strColumnTitle == GetImportFieldHeader(ifnAppointmentStartTime) ||
		strColumnTitle == GetImportFieldHeader(ifnAppointmentEndTime) ||
		strColumnTitle == GetImportFieldHeader(ifnPatientNoteDateTime) ||
		strColumnTitle == GetImportFieldHeader(ifnInsuredEffectiveDate) ||
		strColumnTitle == GetImportFieldHeader(ifnInsuredInactiveDate) ||
		strColumnTitle == GetImportFieldHeader(ifnBirthdate) ||
		strColumnTitle == GetImportFieldHeader(ifnUserDateOfHire) ||
		strColumnTitle == GetImportFieldHeader(ifnFirstContactDate) ||
		strColumnTitle == GetImportFieldHeader(ifnCustomDate1) ||
		strColumnTitle == GetImportFieldHeader(ifnCustomDate2) ||
		strColumnTitle == GetImportFieldHeader(ifnCustomDate3) ||
		strColumnTitle == GetImportFieldHeader(ifnCustomDate4))

	{
		strErrorMessage += "Must contain a valid date \r\n";
	}

	if (strColumnTitle == GetImportFieldHeader(ifnMaritalStatus))
	{
		strErrorMessage += "Must be either 'Single', 'Married', 'Other', 'S', 'M', or 'O' \r\n";
	}

	//(s.dhole 4/28/2015 9:33 AM ) - PLID 65755 Added ifnInsuredCopay
	if (strColumnTitle == GetImportFieldHeader(ifnProductLastCost)
		|| strColumnTitle == GetImportFieldHeader(ifnProductOnHand)
		|| strColumnTitle == GetImportFieldHeader(ifnProductPrice)
		|| strColumnTitle == GetImportFieldHeader(ifnInsuredCopay)
		|| strColumnTitle == GetImportFieldHeader(ifnServicePrice)
		) 
	{
		strErrorMessage += "Must be a valid number \r\n";
	}

	//(s.dhole 4/7/2015 10:07 AM ) - PLID 65229 validate insure p[arty relation
	if (strColumnTitle == GetImportFieldHeader(ifnInsuredPartyRelation))
	{
		strErrorMessage += "Must be either 'Self', 'Child', 'Spouse', 'Other', 'Unknown', 'Employee', 'Organ Donor', 'Cadaver Donor' or 'Life Partner' \r\n";
	}

	// (b.savon 2015-04-13 09:02) - PLID 65219 - Appt requirements
	//(s.dhole 4/28/2015 9:33 AM ) - PLID 65755 Added InsuredCopayPercent
	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentDuration) ||
		strColumnTitle == GetImportFieldHeader(ifnInsuredCopayPercent)){
		strErrorMessage += "Must be a valid integer greater than zero \r\n";
	}

	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentStartTime) ||
		strColumnTitle == GetImportFieldHeader(ifnAppointmentEndTime) ||
		strColumnTitle == GetImportFieldHeader(ifnAppointmentDuration)){
		strErrorMessage += "Must have a 'Start Time' with an 'End Time' or 'Duration' and span within the same calendar day \r\n";
		strErrorMessage += "The 'Start Time' must be before the 'End Time' \r\n";
	}

	// (r.farnworth 2015-04-29 11:15) - PLID 65525 - Boolean validation
	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentIsEvent)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsCancelled)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsConfirmed)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsNoShow)
		|| strColumnTitle == GetImportFieldHeader(ifnUserPatientCoord)
		|| strColumnTitle == GetImportFieldHeader(ifnOtherContactNurse)
		|| strColumnTitle == GetImportFieldHeader(ifnOtherContactAnesthesiologist)
		|| strColumnTitle == GetImportFieldHeader(ifnTaxable1)
		|| strColumnTitle == GetImportFieldHeader(ifnTaxable2)
		|| strColumnTitle == GetImportFieldHeader(ifnProductTaxable1)
		|| strColumnTitle == GetImportFieldHeader(ifnProductTaxable2)
		|| strColumnTitle == GetImportFieldHeader(ifnProductBillable)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox1)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox2)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox3)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox4)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox5)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox6)){
		strErrorMessage += "Must be either 'T', 'True', 'Y', 'Yes', or '1' for a value of Yes \r\n";
		strErrorMessage += "or 'F', 'False', 'N', 'No', or '0' for a value of No \r\n";
	}

	// (r.goldschmidt 2016-03-15 17:11) - PLID 67976 - add validation warning for Appointment Purpose
	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentPurpose)) {
		strErrorMessage += "Must be prepended to note if Appointment Type is forced to \"Conversion\" \r\n";
	}

	// (r.goldschmidt 2016-03-16 13:04) - PLID 67976 - add validation warning for Appointment Type
	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentType)) {
		strErrorMessage += "Must be prepended to note if Appointment Type is forced to \"Conversion\" \r\n";
	}

	// (r.farnworth 2015-03-30 12:23) - PLID 65163 - All String fields can fail due to exceeding their maximum length
	if (bIncludeLength) {
		CString strAppendage;
		strAppendage.Format("Cannot exceed more than %li characters \r\n", nMaxLength);
		strErrorMessage += strAppendage;
	}

	// (r.farnworth 2015-04-07 10:10) - PLID 65246 - For Import objects that support Patient Mapping ID, add a dropdown list to allow the user to select the field to use to map the patient and support the map during import
	if (strColumnTitle == GetImportFieldHeader(ifnCustomPatientID)) {
		strErrorMessage += "\r\n"
			"If User Defined ID is selected in the Patient ID Mapping dropdown, the field also: \r\n \r\n"
			"Must be greater than 0 or exactly -25 \r\n"
			"Must be less than 2,147,483,647 \r\n"
			"Cannot contain letters \r\n";
	}
	return strErrorMessage;
}

// (b.savon 2015-03-20 07:22) - PLID 65153 - Add validation for Race
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateRace(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
		-----------------------
		Race Name Must match name, be blank, CDCID code, or hierarchical code. (See RaceCodesT)
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnRace)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Prepare query
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Changed for use with caching framework.
		static const CString strValidationQuery = R"(
SELECT RaceT.Name FROM RaceT 
UNION SELECT RaceCodesT.CDCID FROM RaceT INNER JOIN RaceCodesT ON RaceT.RaceCodeID = RaceCodesT.ID 
UNION SELECT RaceCodesT.HierarchicalCode FROM RaceT INNER JOIN RaceCodesT ON RaceT.RaceCodeID = RaceCodesT.ID 
UNION SELECT RaceCodesT.OfficialRaceName FROM RaceT INNER JOIN RaceCodesT ON RaceT.RaceCodeID = RaceCodesT.ID
		)";

		//Check if this data is in the database
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!cache.Exists_QueryValue(strValidationQuery, strData)){
			return false;
		}

	}

	return true;
}

// (r.goldschmidt 2016-02-08 18:55) - PLID 68163 - validate Race CDC Code
bool ValidateRaceCDCCode(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Race CDC Code must match existing CDCID in RaceCodesT
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnRaceCDCCode)) {

		//Is this field blank -- We allow blank (to indicate that the race name is not matched to a CDC Race Code)
		if (strData.IsEmpty()) {
			return true;
		}

		//Prepare query
		static const CString strValidationQuery = R"(SELECT RaceCodesT.CDCID FROM RaceCodesT)";

		//Check if this data is in the database
		if (!cache.Exists_QueryValue(strValidationQuery, strData)) {
			return false;
		}

	}

	return true;
}

// (r.goldschmidt 2016-02-08 18:55) - PLID 68163 - validate Race Preferred Name
bool ValidateRacePreferredName(CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn)
{
	/*	Validation Requirements
	-----------------------
	Race Preferred Name can't already exist
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnRacePreferredName)) {

		//Is this field blank -- We do not allow blank 
		if (strData.IsEmpty()) {
			return false;
		}

		//Check if this race name exists elsewhere in our table. This could take a bit of processing depending on the number of rows
		// The list has us already (1), so check for multiple (>1) to detect dups.
		if (cache.Count_DatalistValueInColumn(pdl, nColumn, strData) > 1) {
			return false;
		}

		//Prepare query
		static const CString strValidationQuery = R"(SELECT RaceT.Name FROM RaceT)";

		//Check if this data is in the database
		if (cache.Exists_QueryValue(strValidationQuery, strData)) {
			return false;
		}

	}

	return true;
}

// (b.savon 2015-03-20 14:54) - PLID 65154 - Add validation for Ethnicity
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateEthnicity(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Ethnicity name must match name, be blank, CDCID code, or hierarchical code. (See EthnicityCodesT)
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnEthnicity)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Prepare query
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Changed for use with caching framework.
		static const CString strValidationQuery = R"(
SELECT EthnicityT.Name FROM EthnicityT
UNION SELECT EthnicityCodesT.CDCID FROM EthnicityCodesT 
UNION SELECT EthnicityCodesT.HierarchicalCode FROM EthnicityCodesT 
UNION SELECT EthnicityCodesT.OfficialEthnicityName FROM EthnicityCodesT
		)";

		//Check if this data is in the database
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!cache.Exists_QueryValue(strValidationQuery, strData)){
			return false;
		}

	}

	return true;
}

// (b.savon 2015-03-23 07:32) - PLID 65155 - Add validation for Language
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateLanguage(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Language must match name, be blank, or match LanguageCode, or LanguageCodeAlpha3 (See LanguageCodesT)
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnLanguage)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Prepare query
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Changed for use with caching framework.
		static const CString strValidationQuery = R"(
SELECT LanguageT.Name FROM LanguageT
UNION SELECT LanguageCodesT.OfficialName FROM LanguageCodesT 
UNION SELECT LanguageCodesT.LanguageCode FROM LanguageCodesT 
UNION SELECT LanguageCodesT.LanguageCodeAlpha3 FROM LanguageCodesT
		)";

		//Check if this data is in the database
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!cache.Exists_QueryValue(strValidationQuery, strData)){
			return false;
		}

	}

	return true;
}

// (b.savon 2015-03-23 08:13) - PLID 65156 - Add validation for Patients Current Status
bool ValidatePatientCurrentStatus(const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Current Status must be blank or one of “Patient”, “Prospect”, or “Patient Prospect”.  If current status is blank, import the patient by following the NewPatientDefault preference.
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnPatientCurrentStatus)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Check if this data is valid
		if (strData.CompareNoCase("Patient") == 0){
			return true;
		}

		if (strData.CompareNoCase("Prospect") == 0){
			return true;
		}

		if (strData.CompareNoCase("Patient Prospect") == 0){
			return true;
		}

		// It doesn't match any of the approved values and is non-empty.  Fail.
		return false;
	}

	return true;
}

// (b.savon 2015-03-23 10:18) - PLID 65157 - Add validation for Location
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateLocation(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Location must match name or be blank. Validation fails if a name is provided that doesn’t exist.
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnLocation)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Prepare query
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Changed for use with caching framework.
		static const CString strValidationQuery = R"(
SELECT	Name
FROM	LocationsT 
WHERE	Name <> ''
		)";

		//Check if this data is in the database
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!cache.Exists_QueryValue(strValidationQuery, strData)){
			return false;
		}

	}

	return true;
}

// (b.savon 2015-03-23 11:28) - PLID 65158 - Add validation for Referral Source Name
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateReferralSource(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Referral Name must match name or be blank. Validation fails if a name is provided that doesn’t exist.
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnReferralSourceName)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Prepare query
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Changed for use with caching framework.
		static const CString strValidationQuery = R"(
SELECT	Name
FROM	ReferralSourceT 
WHERE	Name <> ''
		)";

		//Check if this data is in the database
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!cache.Exists_QueryValue(strValidationQuery, strData)){
			return false;
		}

	}

	return true;
}

// (b.savon 2015-03-23 13:00) - PLID 65159 - Add validation for First Contact Date
bool ValidateFirstContactDate(CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	First Contact Date -- if it is blank, import the patient with the first contact date of today.
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnFirstContactDate)) {

		//Is this field blank -- Set it to today's date
		if (strData.IsEmpty()){
			strData = FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate);
			return true;
		}

		//Check if this is a valid date
		COleDateTime dt = ParseDateTime(strData, false); // (v.maida 2015-04-08 16:20) - PLID 65423 - Use updated ParseDateTime() function, which should now handle ISO-8601 formatted dates.

		if (dt.GetStatus() != COleDateTime::valid) {
			return false;
		}

	}

	return true;
}



// (b.savon 2015-03-23 15:55) - PLID 65161 - Add validation for Marital Status
bool ValidateMaritalStatus(const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Marital Status must blank or be one of: S/M/O or Single/Married/Other
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnMaritalStatus)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Check if this data is valid
		if (strData.CompareNoCase("Single") == 0 || strData.CompareNoCase("S") == 0){
			return true;
		}

		if (strData.CompareNoCase("Married") == 0 || strData.CompareNoCase("M") == 0){
			return true;
		}

		if (strData.CompareNoCase("Other") == 0 || strData.CompareNoCase("O") == 0){
			return true;
		}

		return false;
	}

	return true;
}

// (b.savon 2015-03-24 08:15) - PLID 65160 - Modify validation Patient Last Name
bool ValidatePatientLastName(const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Do not allow import of any patient with a blank last name
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnLastName)) {

		//Is this field blank -- We don't allow blank
		if (strData.IsEmpty()){
			return false;
		}
	}

	return true;
}

// (r.farnworth 2015-03-16 16:18) - PLID 65200 - Add validation for Resource object
// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
bool ValidateResourceName(CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn, bool bCanExist /*= false*/)
{
	if (strColumnTitle == GetImportFieldHeader(ifnResourceName)) {

		//check for empty names
		if (strData.IsEmpty()){
			return false;
		}

		//Check if this resource exists elsewhere in our table. This could take a bit of processing depending on the number of rows
		if (!bCanExist){
			// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
			// (b.cardillo 2015-05-10 10:55) - PLID 65951 - The list has us already (1), so check for multiple (>1) to detect dups.
			if (cache.Count_DatalistValueInColumn(pdl, nColumn, strData) > 1) {
				return false;
			}
		}

		//Check if this resource exists in the database
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (cache.Exists_QueryValue("SELECT Item FROM ResourceT", strData)){ 
			if (bCanExist) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			if (bCanExist) {
				return false;
			}
		}
	}

	return true;
}

// (r.farnworth 2015-03-16 16:18) - PLID 65240 - Add validation for Product object
// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
bool ValidateProductName(CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn)
{
	if (strColumnTitle == GetImportFieldHeader(ifnProductName)) {

		//check for empty names
		if (strData.IsEmpty()){
			return false;
		}

		//Check if this product exists elsewhere in our table. This could take a bit of processing depending on the number of rows
		// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
		// (b.cardillo 2015-05-10 10:55) - PLID 65951 - The list has us already (1), so check for multiple (>1) to detect dups.
		if (cache.Count_DatalistValueInColumn(pdl, nColumn, strData) > 1) {
			return false;
		}

		//Check if this product exists in the database
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (cache.Exists_QueryValue("SELECT Name FROM ServiceT WHERE Name <> ''", strData)) {
			return false;
		}
	}

	return true;
}

// (r.farnworth 2015-03-20 11:34) - PLID 65241 - Add validation for Products object -- Price field and ensure valid values are saved to data.
bool ValidateProductPrice(const CString &strData, const CString &strColumnTitle)
{
	if (strColumnTitle == GetImportFieldHeader(ifnProductPrice)) {
		COleCurrency cy = ParseCurrencyFromInterface(strData);

		if (cy.GetStatus() != COleCurrency::valid || cy < COleCurrency(0, 0)) {
			return false;
		}
	}

	return true;
}

// (r.farnworth 2015-03-20 11:55) - PLID 65242 - Add validation for Products object -- Barcode field and ensure valid values are saved to data.
// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
bool ValidateProductBarcode(CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn)
{
	if (strColumnTitle == GetImportFieldHeader(ifnProductBarcode)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//check if this barcode already exists in the table
		// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
		// (b.cardillo 2015-05-10 10:55) - PLID 65951 - The list has us already (1), so check for multiple (>1) to detect dups.
		if (cache.Count_DatalistValueInColumn(pdl, nColumn, strData) > 1) {
			return false;
		}

		//Check if this barcode exists in the database
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (cache.Exists_QueryValue("SELECT Barcode FROM ServiceT WHERE Barcode <> '' \r\n"
			"UNION \r\n"
			"SELECT Barcode FROM CouponsT WHERE Barcode <> '' \r\n", strData)) {
			return false;
		}
	}

	return true;
}

// (r.farnworth 2015-03-20 14:14) - PLID 65244 - Add validation for Products object -- On Hand Amount field and ensure valid values are saved to data.
bool ValidateProductOnHand(const CString &strData, const CString &strColumnTitle)
{
	if (strColumnTitle == GetImportFieldHeader(ifnProductOnHand)) {

		//On Hand Amounts must be >= 0
		long nOnHandAmt = atol(strData);
		if (nOnHandAmt < 0) {
			return false;
		}
	}

	return true;
}

// (b.savon 2015-03-25 13:26) - PLID 65150 - Add validation for Patients import -- Provider Name
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidatePatientProvider(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Provider Name must match First and Last or be blank. Validation fails if a name is provided that doesn’t exist or if there are multiple providers returned
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnProviderName)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Prepare query
		//   The caching framework does not allow for duplicate values, so we'll handle that in the query.
		//   If an entry has duplicates, we will omit it from the results altogether, so then later we only
		//   need to check for the presence of a value to know that it is unique.
		static const CString strValidationQuery = R"(
;WITH P
     AS (SELECT PersonT.First,
                PersonT.Last
         FROM ProvidersT
              INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID
         GROUP BY PersonT.First,
                  PersonT.Last
         HAVING COUNT(*) = 1),
     FullNames
     AS (
     SELECT LTRIM(RTRIM(P.First))+' '+LTRIM(RTRIM(P.Last)) AS Val
     FROM P
     UNION ALL
     SELECT LTRIM(RTRIM(P.Last))+', '+LTRIM(RTRIM(P.First)) AS Val
     FROM P)
     SELECT Val
     FROM FullNames
     GROUP BY Val
     HAVING COUNT(*) = 1;
)";
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!cache.Exists_QueryValue(strValidationQuery, strData)) {
			// Return false if the name doesn't exist (which will be the case if it's not an existing name, or if there are duplicates. See query above)
			return false;
		}

	}

	return true;
}

// (b.savon 2015-03-26 13:26) - PLID 65151 - Add validation for Patients import -- Referring Physician Name
// (b.savon 2015-03-26 14:23) - PLID 65152 - Add validation for Patients import -- PCP Name 
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidatePatientReferringOrPCPName(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Referring Phsician and PCP Name must match First and Last or be blank. Validation fails if a name is provided that doesn’t exist or if there are multiple providers returned
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnReferringPhysicianName) || strColumnTitle == GetImportFieldHeader(ifnPrimaryCarePhysicianName)) {

		//Is this field blank -- We allow blank
		if (strData.IsEmpty()) {
			return true;
		}

		// Prepare query.
		//   The caching framework does not allow for duplicate values, so we'll handle that in the query.
		//   If an entry has duplicates, we will omit it from the results altogether, so then later we only
		//   need to check for the presence of a value to know that it is unique.
		static const CString strValidationQuery = R"(
;WITH R
     AS (SELECT PersonT.First,
                PersonT.Last
         FROM ReferringPhysT
              INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID
         GROUP BY PersonT.First,
                  PersonT.Last
         HAVING COUNT(*) = 1),
     FullNames
     AS (
     SELECT LTRIM(RTRIM(R.First))+' '+LTRIM(RTRIM(R.Last)) AS Val
     FROM R
     UNION ALL
     SELECT LTRIM(RTRIM(R.Last))+', '+LTRIM(RTRIM(R.First)) AS Val
     FROM R)
     SELECT Val
     FROM FullNames
     GROUP BY Val
     HAVING COUNT(*) = 1;
)";

		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!cache.Exists_QueryValue(strValidationQuery, strData)) {
			// Return false if the name doesn't exist (which will be the case if it's not an existing name, or if there are duplicates. See query above)
			return false;
		}
	}

	return true;
}

// (b.savon 2015-03-31 11:04) - PLID 65235 - Add validation for Recalls object -- Recall date
bool ValidateRecallDate(CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Recall Date -- if it is blank or invalid, fail
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnRecallDate)) {

		//Is this field blank -- We don't allow blank
		if (strData.IsEmpty()){
			return false;
		}

		//Check if this is a valid date
		COleDateTime dt = ParseDateTime(strData, false); // (v.maida 2015-04-08 16:20) - PLID 65423 - Use updated ParseDateTime() function, which should now handle ISO-8601 formatted dates.

		if (dt.GetStatus() != COleDateTime::valid) {
			return false;
		}

	}

	return true;
}

// (b.savon 2015-04-02 09:11) - PLID 65236 - Add validation for Recalls object -- Template Name
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateRecallTemplateName(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Recall Date -- if it is blank or invalid, fail
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnRecallTemplateName)) {

		//Is this field blank -- We don't allow blank
		if (strData.IsEmpty()){
			return false;
		}

		//Prepare query
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		static const CString strValidationQuery = R"(SELECT Name FROM RecallTemplateT GROUP BY Name HAVING COUNT(*) = 1)";

		if (!cache.Exists_QueryValue(strValidationQuery, strData)) {
			// Return false if the template doesn't exist or there are multiple for that name
			return false;
		}

	}

	return true;
}

// (r.farnworth 2015-04-06 14:50) - PLID 65168 - Add validation for Insurance Company object -- Insurance Company Name field and ensure valid values are saved to data.
bool ValidateInsCoName(const CString &strData, const CString &strColumnTitle)
{
	if (strColumnTitle == GetImportFieldHeader(ifnInsCoName)) {
		//Must Not be Blank
		if (strData.IsEmpty()){
			return false;
		}
	}

	return true;
}

// (r.farnworth 2015-04-06 15:06) - PLID 65169 - Add validation for Insurance Company object -- Insurance Company Conversion ID field and ensure valid values are saved to data.
// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
bool ValidateConversionID(CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn)
{
	if (strColumnTitle == GetImportFieldHeader(ifnConversionID) ) {

		//Not Required, but if provided must be unique in data and the file
		if (!strData.IsEmpty()){
			// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
			// (b.cardillo 2015-05-10 10:55) - PLID 65951 - The list has us already (1), so check for multiple (>1) to detect dups.
			if (cache.Count_DatalistValueInColumn(pdl, nColumn, strData) > 1) {
				return false;
			}

			// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
			if (cache.Exists_QueryValue("SELECT ConversionID FROM InsuranceCoT WHERE ConversionID <> ''", strData)){
				return false;
			}
		}
		else {
			return true;
		}

	}

	return true;
}

// (r.farnworth 2015-04-06 16:04) - PLID 65189 - Add validation for Insurance Company object -- Default HCFA PayerID, Default UB PayerID, and Eligbility PayerID fields and ensure valid values are saved to data.
// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
bool ValidatePayerIDs(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	//The logic of the validation should be made modular so all of these PayerID checks can use the same routine.
	if (strColumnTitle == GetImportFieldHeader(ifnHCFAPayerID) || strColumnTitle == GetImportFieldHeader(ifnUBPayerID) || strColumnTitle == GetImportFieldHeader(ifnEligibilityPayerID)) {

		// (r.farnworth 2015-05-04 16:17) - PLID 65189 - Allow blank
		if (strData.IsEmpty())
		{
			return true;
		}

		//Must match available data in EBillingInsCoIDs 
		//Check if this data is in the database
		// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
		if (!cache.Exists_QueryValue("SELECT EBillingID From EBillingInsCoIDs", strData)){ 
			return false;
		}
	}

	return true;
}



//(s.dhole 4/7/2015 8:38 AM ) - PLID 65229 check if priority is valid
bool ValidatePatientPriority(const CString &strData, const CString &strColumnTitle)
{
	bool bReturn = true;
	if (strColumnTitle == GetImportFieldHeader(ifnPatientNotePriority))
	{
		// allow blank
		if (strData.IsEmpty())
		{
			bReturn = true;
		}		
		else if (strColumnTitle == GetImportFieldHeader(ifnPatientNotePriority)) {
			if (((strData.CompareNoCase("Low") == 0 || strData.CompareNoCase("Medium") == 0 || strData.CompareNoCase("High") == 0)) ||
				((atol(strData) > 0 && atol(strData) <= 3)))
			{
				bReturn = true;
			}
			else
			{
				bReturn = false;
			}
		}
	}
	return bReturn;
}


//(s.dhole 4/7/2015 8:51 AM ) - PLID 65227 This function  will check valid date field , if date is empty then set todays date as date
bool ValidateDate(int nColumnFiled, CString &strData, const CString &strColumnTitle)
{

	if (strColumnTitle == GetImportFieldHeader((ImportFieldNumber)nColumnFiled)) {
		if (strData.IsEmpty()){
			// do not allow blank
			return false;
		}
		//Check if this is a valid date
		COleDateTime dt = ParseDateTime(strData, false); // (v.maida 2015-04-08 16:20) - PLID 65423 - Use updated ParseDateTime() function, which should now handle ISO-8601 formatted dates.

		if (dt.GetStatus() != COleDateTime::valid) {
			return false;
		}
	}
	return true;
}


//(s.dhole 4/7/2015 3:49 PM ) - PLID 65228 Check valid note category
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateNoteCategory(CCachedData &cache, const CString &strData, const CString &strColumnTitle)
{
	// allow emty note category
	if (strData.IsEmpty())
	{
		return true;
	}

	if (strColumnTitle == GetImportFieldHeader(ifnPatientNoteCategory))  {
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		if (!cache.Exists_QueryValue("SELECT Description From NoteCatsF WHERE Description <> ''", strData)) { 
			return false;
		}
	}
	return true;
}


// (b.savon 2015-04-07 14:59) - PLID 65218 - Add validation for Appointment Date
bool ValidateAppointmentDate(const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Date -- Must never be blank
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentDate)) {

		//We don't allow blank
		if (strData.IsEmpty()){
			return false;
		}

		//Check if this is a valid date
		COleDateTime dt = ParseDateTime(strData, false);

		if (dt.GetStatus() != COleDateTime::valid) {
			return false;
		}

	}

	return true;
}

// (b.savon 2015-04-07 15:24) - PLID 65220 - Add validation for Appointment Type
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
// (r.goldschmidt 2016-01-26 19:42) - PLID 67976 - allow for forcing to conversion
// (r.goldschmidt 2016-03-15 12:34) - PLID 67976 - allow for sticking into note only
// (r.goldschmidt 2016-03-16 13:00) - PLID 67976 - .. now disallow when forced to conversion, this column has been set, and prepend to note is not set
bool ValidateAppointmentType(CCachedData &cache, const CString &strData, const CString &strColumnTitle, bool bForceToConversion /* = false */, bool bForceToNotes /* = false */)
{
	/*	Validation Requirements
	-----------------------
		Appointment Type - Must either me blank or match a type in the system exactly
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentType)) {

		//Are we forcing into note instead of data -- allowed
		if (bForceToNotes) {
			return true;
		}
		//Are we forcing to type of conversion when selected type isn't going into note -- disallowed
		else if (bForceToConversion) {
			return false;
		}

		// (r.goldschmidt 2016-03-18 16:10) - PLID 67976 - rearrange to allow bForceToConversion to return false even when strData is empty
		//Is this field blank -- We allow blank
		if (strData.IsEmpty()) {
			return true;
		}

		//Prepare query
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		static const CString strValidationQuery = R"(SELECT Name FROM AptTypeT GROUP BY Name HAVING COUNT(*) = 1)";
		
		
		if (!cache.Exists_QueryValue(strValidationQuery, strData)) {
			// Return false if the type doesn't exist or there are multiple for that name
			return false;
		}
	}

	return true;
}

// (b.savon 2015-04-10 08:52) - PLID 65221 - Add validation for Appointment Purpose
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
// (r.goldschmidt 2016-01-26 19:42) - PLID 67976 - allow for forcing to conversion
// (r.goldschmidt 2016-03-15 12:34) - PLID 67976 - disallow when conversion is forced but not notes
bool ValidateAppointmentPurpose(CCachedData &cache, const CString &strData, const CString &strColumnTitle, bool bForceToConversion /* = false */, bool bForceToNotes /* = false */)
{
	/*	Validation Requirements
	-----------------------
		Appointment Purpose - Must either me blank or match a purpose in the system exactly
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentPurpose)) {

		//Are we forcing to notes -- allowed
		if (bForceToNotes) {
			return true;
		}
		else if (bForceToConversion) { // can't have appt type set to conversion and also try to match to appt purpose
			return false;
		}

		// (r.goldschmidt 2016-03-18 16:10) - PLID 67976 - rearrange to allow bForceToConversion to return false even when strData is empty
		//Is this field blank -- We allow blank
		if (strData.IsEmpty()) {
			return true;
		}

		//Prepare query
		// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
		static const CString strValidationQuery = R"(SELECT Name FROM AptPurposeT GROUP BY Name HAVING COUNT(*) = 1)";


		if (!cache.Exists_QueryValue(strValidationQuery, strData)) {
			// Return false if the purpose doesn't exist or there are multiple for that name
			return false;
		}
	}

	return true;
}

// (b.savon 2015-04-10 13:02) - PLID 65219 - Add validation Appointment Start/End Time
bool ValidateAppointmentStartEndTime(const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	Start Time -- This is field level validation; group validation happens elsewhere.  That said, this can be blank but if there is something provided, it must be a valid time
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentStartTime) ||
		strColumnTitle == GetImportFieldHeader(ifnAppointmentEndTime)) {

		//We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Check if this is a valid date
		COleDateTime dt = ParseDateTime(strData, false);

		if (dt.GetStatus() != COleDateTime::valid) {
			return false;
		}

	}

	return true;
}


//(s.dhole 4/15/2015 2:25 PM ) - PLID 65196 make sure it has expected values
bool ValidateInsuredPartyRelations(const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	'Self', 'Child', 'Spouse', 'Other', 'Unknown', 'Employee', 'Organ Donor', 'Cadaver Donor' or 'Life Partner'
	*/
	if (strColumnTitle == GetImportFieldHeader(ifnInsuredPartyRelation)) {
		if (strData.CompareNoCase("Self") == 0 || strData.CompareNoCase("Child") == 0 || strData.CompareNoCase("Spouse") == 0 ||
			strData.CompareNoCase("Other") == 0 || strData.CompareNoCase("Unknown") == 0 || strData.CompareNoCase("Employee") == 0 ||
			strData.CompareNoCase("Organ Donor") == 0 || strData.CompareNoCase("Cadaver Donor") == 0 || strData.CompareNoCase("Life Partner") == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}



//(s.dhole 4/15/2015 2:23 PM ) - PLID 65195
bool ValidateInsuredPartyRespoType(const CString &strData, const CString &strColumnTitle, const NXDATALIST2Lib::IRowSettingsPtr &pRow, NXDATALIST2Lib::_DNxDataListPtr &pRecordList, CSqlFragment &PatientSql)
{
	/*	Validation Requirements
	-----------------------
	step 1- We allow blank , If not blank then execute following steps
	step 2- check datalist for any duplicat RespoType for selected patient, Except Inactive
	step 3- Check if Respo type is exist in database
	step 4- if there is no duplicate Respo Type in datalist, then check database for any existing record , Ignore Inactive
	*/
	if (strColumnTitle == GetImportFieldHeader(ifnInsuredPartyRespTypeID)) {
		//We allow blank, it is inactive isurancwe
		
		if (strData.IsEmpty() ){
			return true;
		}
		if (strData.CompareNoCase("-1")==0){
			return true;
		}

		long nRespoTypeId = atol(strData);
		if (strData.CompareNoCase(FormatString("%li", nRespoTypeId)) != 0)
		{
			return false;
		}

		//// There is no patitnt id found yet
		if (PatientSql->m_strSql.IsEmpty())
		{
			return false;
		}
		
		
		// find out which column is use for patient id and Respo type
		short nColumnPatietnID = -1;
		short nColumnInsuredPartyRespTypeID = -1;
		for (int i = 0, nColumnCount = pRecordList->GetColumnCount(); i < nColumnCount; i++){
			CString strColumnHeader = VarString(pRecordList->GetColumn(i)->GetColumnTitle(), "");
			if (strColumnHeader == GetImportFieldHeader(ifnCustomPatientID))
			{
				nColumnPatietnID = i;
			}
			else if (strColumnTitle == strColumnHeader)
			{
				nColumnInsuredPartyRespTypeID = i;
			}
			else
			{
				// nothing
			}
		}

	

		// check if there is duplicate in datalist for same patient
		CString strPatientID = VarString(pRow->GetValue(nColumnPatietnID), "");
		CString strInitalRepTypeValue = VarString(pRow->GetValue(nColumnInsuredPartyRespTypeID), "");
		// Step 1 , Check if there is another row has same patitn id,  
		//FindByColumn - this function search all rows in circular pattern , so if it won't find any record in forward rows then will start from bigining
		//step 2 -  will check if row poiter is same as existing one, this can be happen if there is no other record with same patient,  There is no duplicate respo typ for same patient
		//step 3 -  will check if row poiter is not same then search next prow for patitn recor till we get same row or null
		NXDATALIST2Lib::IRowSettingsPtr pSearchRow = pRecordList->FindByColumn(nColumnPatietnID, _bstr_t(strPatientID), pRow->GetNextRow(), FALSE);
		while (pSearchRow)
		{
			CString strNewRepTypeValue = VarString(pSearchRow->GetValue(nColumnInsuredPartyRespTypeID), "");
			// if we do have same RepTypeValue value on another row fro same patitnt then  we have duplicates
			if (strNewRepTypeValue.CollateNoCase(strInitalRepTypeValue) == 0 && pRow != pSearchRow)
			{
				return false;
			}
			else if (pRow == pSearchRow)
			{ 
				break;
			}
			else
			{
				pSearchRow = pRecordList->FindByColumn(nColumnPatietnID, _bstr_t(strPatientID), pSearchRow->GetNextRow(), FALSE);
			}
		}

		// if we are this stage then we do not have duplicate record in data list 
		//now check in database if Respo type is exist for same patient
		CString strValidationQuery = R"(SELECT ISNULL((SELECT COUNT(PersonID)  FROM  InsuredPartyT INNER JOIN RespTypeT  ON InsuredPartyT.RespTypeID = RespTypeT.ID  WHERE PatientID IN ({SQL}) AND RespTypeT.Priority ={INT}) ,0) AS IsRespoTypeInUse , ISNULL((Select count(ID) from RespTypeT WHERE Priority ={INT} ),0) AS IsRespoTypeExist )";
		ADODB::_RecordsetPtr prs = CreateParamRecordset(strValidationQuery, PatientSql, nRespoTypeId, nRespoTypeId);
		if (prs->eof)
		{
			return false;
		}
		else if (AdoFldLong(prs, "IsRespoTypeInUse") != 0 || AdoFldLong(prs, "IsRespoTypeExist") == 0)
		{
			return false;
		}
		
	}
	return true;
}




//(s.dhole 4/15/2015 2:23 PM ) - PLID 65193 
// if both conversion id and insurance name are selected then validate only on conversionID
// if conversion id is missing and insurance name is selected , then validate insurance name and should match  name with existng db Insurance name , should match  1:1
// if both are missing then return false
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateInsuredPartyInsuComp(CCachedData &cache, const CString &strData, const CString &strColumnTitle, const NXDATALIST2Lib::IRowSettingsPtr &pRow, NXDATALIST2Lib::_DNxDataListPtr &pRecordList)
{
	
	if (strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyConversionID) || strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyName)) {
		// check if both column are selected

		NXDATALIST2Lib::IRowSettingsPtr pSearchRow = pRecordList->FindByColumn(ifnInsuredPartyRespTypeID, _bstr_t(strData), pRecordList->GetFirstRow(), FALSE);
		short nColumnPatietnID = -1;
		short nColumnInsuredPartyRespTypeID = -1;
		bool bISConversionIDColExist, bISInsuCompNameColExist ;
		bISConversionIDColExist = bISInsuCompNameColExist = false;
		// check if conversion id or insurance name are selected
		for (int i = 0, nColumnCount = pRecordList->GetColumnCount(); i < nColumnCount; i++){
			CString strColumnHeader = VarString(pRecordList->GetColumn(i)->GetColumnTitle(), "");
			if (strColumnHeader == GetImportFieldHeader(ifnInsuranceCompanyConversionID))
			{
				//now check if column has data
				CString strData = "";
				_variant_t varData = pRow->GetValue(i);
				if (varData.vt != VT_EMPTY){
					strData = VarString(varData, "");

					//Cleanup the string to remove leading and trailing white spaces
					strData.TrimLeft();
					strData.TrimRight();
				}
				if (!strData.IsEmpty())
				{
				bISConversionIDColExist =true;
				}
			}
			else if (strColumnHeader == GetImportFieldHeader(ifnInsuranceCompanyName))
			{
				//now check if column has data
				CString strData = "";
				_variant_t varData = pRow->GetValue(i);
				if (varData.vt != VT_EMPTY){
					strData = VarString(varData, "");

					//Cleanup the string to remove leading and trailing white spaces
					strData.TrimLeft();
					strData.TrimRight();
				}
				if (!strData.IsEmpty())
				{
					bISInsuCompNameColExist = true;
				}
			}
			else if (bISConversionIDColExist && bISInsuCompNameColExist)
			{
				break;
			}
			else
			{
				// nothing
			}
		}
		// if both are missing then return false
		if (!bISConversionIDColExist && !bISInsuCompNameColExist)
		{
			return false;
		}
		else if (bISConversionIDColExist && strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyName))
		{
			//Skip Company name validation, since Conversion id is exist and will validate on conversion id
			return true;
		}
		else if (bISConversionIDColExist   && strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyConversionID))
		{
			// can't be empty
			if (strData.IsEmpty() && !bISInsuCompNameColExist)
			{
				return false;
			}
			
			// ifnInsuranceCompanyConversionID Validation
			// ifnInsuranceCompanyName Validation
			// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
			static const CString strValidationQuery = R"(SELECT ConversionID FROM InsuranceCoT GROUP BY ConversionID HAVING COUNT(*) = 1)";
			
			return cache.Exists_QueryValue(strValidationQuery, strData);
		}
		else if (bISInsuCompNameColExist && strColumnTitle == GetImportFieldHeader(ifnInsuranceCompanyName))
		{
			// can't be empty
			if (strData.IsEmpty() && !bISConversionIDColExist)
			{
				return false;
			}
			// ifnInsuranceCompanyName Validation
			// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
			static const CString strValidationQuery = R"(SELECT Name FROM InsuranceCoT GROUP BY Name HAVING COUNT(*) = 1)";

			return cache.Exists_QueryValue(strValidationQuery, strData);
		}
		
	}
	return true;
}

// (r.farnworth 2015-04-29 11:01) - PLID 65525 - Ensure boolean values are validated before the preview screen silently changes them to false if they are invalid
bool ValidateBooleanValue(const CString &strData, const CString &strColumnTitle)
{
	// Check for any of the boolean/checkbox fields
	if (strColumnTitle == GetImportFieldHeader(ifnAppointmentIsEvent) 
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsCancelled)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsConfirmed)
		|| strColumnTitle == GetImportFieldHeader(ifnAppointmentIsNoShow)
		|| strColumnTitle == GetImportFieldHeader(ifnUserPatientCoord)
		|| strColumnTitle == GetImportFieldHeader(ifnOtherContactNurse)
		|| strColumnTitle == GetImportFieldHeader(ifnOtherContactAnesthesiologist)
		|| strColumnTitle == GetImportFieldHeader(ifnTaxable1)
		|| strColumnTitle == GetImportFieldHeader(ifnTaxable2)
		|| strColumnTitle == GetImportFieldHeader(ifnProductTaxable1)
		|| strColumnTitle == GetImportFieldHeader(ifnProductTaxable2)
		|| strColumnTitle == GetImportFieldHeader(ifnProductBillable)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox1)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox2)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox3)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox4)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox5)
		|| strColumnTitle == GetImportFieldHeader(ifnCustomCheckbox6))
	{

		//We don't support empty
		if (strData.IsEmpty())
		{
			return false;
		}

		//Check for the values that we support for booleans
		if (strData.CompareNoCase("T") == 0 || strData.CompareNoCase("TRUE") == 0 || strData.CompareNoCase("Y") == 0 || strData.CompareNoCase("YES") == 0 || strData.CompareNoCase("1") == 0
			|| strData.CompareNoCase("F") == 0 || strData.CompareNoCase("FALSE") == 0 || strData.CompareNoCase("N") == 0 || strData.CompareNoCase("NO") == 0 || strData.CompareNoCase("0") == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

//(s.dhole 5/1/2015 2:17 PM ) - PLID 65755 
bool ValidateCopayPercent(const CString &strData, const CString &strColumnTitle)
{
	if (strColumnTitle == GetImportFieldHeader(ifnInsuredCopayPercent)) {

		//On Hand Amounts must be >= 0
		long nValue = atol(strData);
		if (nValue < 0) {
			return false;
		}
	}

	return true;
}



//(s.dhole 5/5/2015 4:03 PM ) - PLID 65755
bool ValidateBirthDate(const CString &strData, const CString &strColumnTitle)
{
	/*	Validation Requirements
	-----------------------
	we do not llow any future dae as birth date
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnBirthdate) ) {

		//We allow blank
		if (strData.IsEmpty()){
			return true;
		}
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		//Check if this is a valid date
		COleDateTime dt = ParseDateTime(strData, false);

		if (dt.GetStatus() != COleDateTime::valid) {
			return false;

		}
		else if (dt > dtNow) {
			return false;
		}

	}

	return true;
}

//(s.dhole 5/5/2015 4:03 PM ) - PLID 65755
bool ValidateInsuredPartyInActiveDate(const CString &strData, const CString &strColumnTitle, const NXDATALIST2Lib::IRowSettingsPtr &pRow, NXDATALIST2Lib::_DNxDataListPtr &pRecordList)
{

	/*	Validation Requirements
	-----------------------
	we do not llow any past inactive date active insurance party
	*/

	if (strColumnTitle == GetImportFieldHeader(ifnInsuredInactiveDate)) {

		//find out if isurence is mark active

		//We allow blank
		if (strData.IsEmpty()){
			return true;
		}

		//Check if this is a valid date
		COleDateTime dt = ParseDateTime(strData, false);

		if (dt.GetStatus() != COleDateTime::valid) {
			return false;

		}
		bool bIsActiveInsured = false;
		for (int i = 0, nColumnCount = pRecordList->GetColumnCount(); i < nColumnCount; i++){
			CString strColumnHeader = VarString(pRecordList->GetColumn(i)->GetColumnTitle(), "");
			if (strColumnHeader == GetImportFieldHeader(ifnInsuredPartyRespTypeID))
			{
				//now check if column has data
				CString strDataTemp = "";
				_variant_t varData = pRow->GetValue(i);
				if (varData.vt != VT_EMPTY){
					strDataTemp = VarString(varData, "");

					//Cleanup the string to remove leading and trailing white spaces
					strDataTemp.TrimLeft();
					strDataTemp.TrimRight();
				}
				if ((!strDataTemp.IsEmpty()) && (strDataTemp.CompareNoCase("-1") != 0)){
					bIsActiveInsured = true;
				}
				break;
			}
		}

		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		COleDateTime dtNewInactiveDate = dt;
		dtToday.SetDateTime(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay(), 0, 0, 0);
		dtNewInactiveDate.SetDateTime(dtNewInactiveDate.GetYear(), dtNewInactiveDate.GetMonth(), dtNewInactiveDate.GetDay(), 0, 0, 0);

		if (dtNewInactiveDate <= dtToday && bIsActiveInsured) {
			return false;
		}
	}
	return true;
}





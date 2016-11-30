//ImportUtils.h
#ifndef IMPORTUTILS_H
#define IMPORTUTILS_H

#pragma once



enum ImportRecordType
{
	irtPatients,
	irtProviders,
	irtReferringPhysicians,
	irtUsers,
	irtSuppliers,
	irtOtherContacts,
	irtMediNotes,
	irtServiceCodes,	// (j.jones 2010-03-30 08:52) - PLID 16717
	irtResources,		// (r.farnworth 2015-03-16 14:11) - PLID 65197
	irtProducts,		// (r.farnworth 2015-03-19 09:13) - PLID 65238
	irtInsuranceCos,	// (r.farnworth 2015-04-01 12:49) - PLID 65166
	// (r.farnworth 2015-03-23 11:49) - PLID 65246 - Added for the sake of this item, when these objects are added, change the associated PLID to match.
	irtInsuredParties,//(s.dhole 4/13/2015 3:07 PM ) - PLID 65190   - Add a new import type, Insure Party, to the import utility
	irtRecalls,
	irtPatientNotes,
	irtAppointments,
	irtRaces, // (r.goldschmidt 2016-02-09 14:52) - PLID 68163
};

enum ImportFileType
{
	iftCharacterSeparated,
};

const OLE_COLOR ocWhite(RGB(255, 255, 255));
const OLE_COLOR ocNavyBlue(RGB(49, 106, 197));
const OLE_COLOR ocBlack(RGB(0, 0, 0));
const OLE_COLOR ocDarkRed(RGB(255, 100, 100));
const OLE_COLOR ocLightRed(RGB(255, 185, 185));
const OLE_COLOR ocDarkGreen(RGB(149, 253, 150));
const OLE_COLOR ocLightGreen(RGB(214, 255, 214));
const OLE_COLOR ocDarkOrange(RGB(255, 175, 96));
const OLE_COLOR ocLightOrange(RGB(250, 200, 145));


struct SpecialChar {
	CString strSourceChar;
	CString strReplaceChar;
};


//(e.lally 2007-06-08) PLID 26262 - Add Provider fields
//(e.lally 2007-06-11) PLID 10320 - Add Ref Phys fields
//(e.lally 2007-06-11) PLID 26274 - Add Supplier fields
//(e.lally 2007-06-12) PLID 26273 - Add User fields
//(e.lally 2007-06-12) PLID 26275 - Add Other contact fields
enum ImportFieldNumber
{
	ifnIgnore =-1,
	ifnLastName = 0,
	ifnFirstName,
	ifnMiddleName,
	ifnPatientID,
	ifnAddress1,
	ifnAddress2,
	ifnCity,
	ifnState,
	ifnZip,
	ifnSocialSecurity,
	ifnHomePhone,
	ifnBirthdate,
	ifnGender,
	ifnWorkPhone,
	ifnWorkExt,
	ifnCellPhone,
	ifnPager,
	ifnOtherPhone,
	ifnFax,
	ifnEmail,
	ifnTitle,
	ifnCompany,
	ifnAccount,
	ifnProviderNPI,
	ifnProviderFederalEmpNumber,
	ifnProviderWorkersCompNumber,
	ifnProviderMedicaidNumber,
	ifnProviderLicenseNumber,
	ifnProviderBCBSNumber,
	ifnProviderTaxonomyCode,
	ifnProviderUPIN,
	ifnProviderMedicare,
	ifnProviderOtherID,
	ifnProviderDEANumber,
	ifnRefPhysNPI,
	ifnRefPhysFederalEmpNumber,
	ifnRefPhysWorkersCompNumber,
	ifnRefPhysMedicaidNumber,
	ifnRefPhysLicenseNumber,
	ifnRefPhysBCBSNumber,
	ifnRefPhysTaxonomyCode,
	ifnRefPhysUPIN,
	ifnRefPhysMedicare,
	ifnRefPhysOtherID,
	ifnRefPhysDEANumber,
	ifnRefPhysID,
	ifnSupplierPayMethod,
	ifnUserUsername,
	ifnUserNationalEmpNum,
	ifnUserDateOfHire,
	ifnUserPatientCoord,
	ifnOtherContactNurse,
	ifnOtherContactAnesthesiologist,
	// (j.jones 2010-04-02 17:55) - PLID 16717 - supported service codes
	ifnServiceName,
	ifnServicePrice,
	ifnTaxable1,
	ifnTaxable2,
	ifnBarcode,
	ifnServiceCode,
	ifnServiceSubCode,
	ifnRVU,
	ifnGlobalPeriod,
	ifnResourceName, // (r.farnworth 2015-03-16 14:32) - PLID 65198
	// (r.farnworth 2015-03-19 09:37) - PLID 65239
	ifnProductName,
	ifnProductDescription,
	ifnProductPrice,
	ifnProductLastCost,
	ifnProductTaxable1,
	ifnProductTaxable2,
	ifnProductBarcode,
	ifnProductBillable,
	ifnProductOnHand,
	// (j.gruber 2010-08-03 10:22) - PLID 39944 drop prompt for copay
	//ifnPromptForCopay,
	// (b.savon 2015-03-19 12:38) - PLID 65144 - Add new fields to patient object for the import utility
	ifnLocation,
	ifnProviderName,
	ifnEthnicity,
	ifnLanguage,
	ifnRace,
	ifnReferringPhysicianName,
	ifnPrimaryCarePhysicianName,
	ifnPatientCurrentStatus,
	ifnFirstContactDate,
	ifnReferralSourceName,
	ifnEmergencyContactFirstName,
	ifnEmergencyContactLastName,
	ifnEmergencyContactRelation,
	ifnEmergencyContactHomePhone,
	ifnEmergencyContactWorkPhone,
	ifnWarningMessage,
	ifnNote,
	ifnGen1Custom1,
	ifnGen1Custom2,
	ifnGen1Custom3,
	ifnGen1Custom4,
	ifnCustomText1,
	ifnCustomText2,
	ifnCustomText3,
	ifnCustomText4,
	ifnCustomText5,
	ifnCustomText6,
	ifnCustomText7,
	ifnCustomText8,
	ifnCustomText9,
	ifnCustomText10,
	ifnCustomText11,
	ifnCustomText12,
	ifnCustomNote,
	ifnCustomCheckbox1,
	ifnCustomCheckbox2,
	ifnCustomCheckbox3,
	ifnCustomCheckbox4,
	ifnCustomCheckbox5,
	ifnCustomCheckbox6,
	ifnCustomDate1,
	ifnCustomDate2,
	ifnCustomDate3,
	ifnCustomDate4,
	ifnMaritalStatus,
	// (r.farnworth 2015-04-01 09:48) - PLID 65246 - We need a string PatientID for mapping to the custom fields
	ifnCustomPatientID,
	// (b.savon 2015-03-30 17:13) - PLID 65233 - Create fields for the Recalls object for the import utility.
	ifnRecallDate,
	ifnRecallTemplateName,
	// (r.farnworth 2015-04-01 14:56) - PLID 65167 - Create fields for the insurance companies object for the import utility
	ifnConversionID,
	ifnInsCoName,
	ifnContactFirst,
	ifnContactLast,
	ifnContactTitle,
	ifnContactPhone,
	ifnContactFax,
	ifnContactNote,
	ifnHCFAPayerID,
	ifnUBPayerID,
	ifnEligibilityPayerID,
	//(s.dhole 4/8/2015 1:45 PM ) - PLID 65230
	ifnPatientNoteCategory,
	ifnPatientNoteText,
	ifnPatientNoteDateTime,
	ifnPatientNotePriority,

	// (b.savon 2015-04-06 09:10) - PLID 65216 - Create fields for the Appointment object for the import utility.
	ifnAppointmentDate,
	ifnAppointmentIsEvent,
	ifnAppointmentStartTime,
	ifnAppointmentEndTime,
	ifnAppointmentDuration,
	ifnAppointmentType,
	ifnAppointmentPurpose,
	ifnAppointmentNotes,
	ifnAppointmentIsConfirmed,
	ifnAppointmentIsCancelled,
	ifnAppointmentIsNoShow,
	//(s.dhole 4/14/2015 9:52 AM ) - PLID 65191 Added Insured Party fields
	ifnInsuranceCompanyName,
	ifnInsuredEmployer,
	ifnInsuredInsuranceID,
	ifnInsuredGroupNo,//(s.dhole 5/6/2015 10:55 AM ) - PLID 65755
	ifnInsuredCopay,
	ifnInsuredCopayPercent,
	ifnInsuredEffectiveDate, 
	ifnInsuredInactiveDate,
	ifnInsuranceCompanyConversionID,
	ifnInsuredPartyRespTypeID,
	ifnInsuredPartyRelation,
	ifnRacePreferredName, // (r.goldschmidt 2016-02-09 14:59) - PLID 68163
	ifnRaceCDCCode,// (r.goldschmidt 2016-02-09 14:59) - PLID 68163

	// *****All new field numbers must have corresponding entries 
	//in the GetImportFieldHeader and GetImportFieldDataField functions ******

};

long GetGenderValueFromString(CString strInput);
CString GetGenderAsTextFromString(CString strInput);

CString GetImportFieldHeader(ImportFieldNumber ifnField);
CString GetImportFieldDataField(ImportFieldNumber ifnField);
int GetMaxFieldLength(ImportFieldNumber ifnField);
BOOL DoesColumnDataExist(NXDATALIST2Lib::IRowSettingsPtr pRow, const CString &strData, const short nCol); // (r.farnworth 2015-03-17 15:36) - PLID 65197

CString FormatPhoneForSql(CString strPhone);

CString GetBooleanYesNoTextFromString(CString strInput);
long GetSqlValueFromBoolString(CString strInput);

// (b.savon 2015-03-24 11:15) - PLID 65250 - Migrate the CapsFix algorithm from DCS tools
void FixCaps(CString &strValue);
// (b.savon 2015-03-24 12:37) - PLID 65251 - Create a utility function that returns if the supplied field is eligible for caps fix algorithm
bool IsEligibleCapsFixField(const CString &strField);
//(s.dhole 4/7/2015 8:36 AM ) - PLID 65229 Get note Prority
long GetNotePriorityFromString(const CString &strInput);

#endif
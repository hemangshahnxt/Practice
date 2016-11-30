#pragma once

// (j.jones 2009-02-12 09:42) - PLID 33163 - created

#ifndef NEWCROPUTILS_H
#define NEWCROPUTILS_H

// (j.dinatale 2010-10-04) - PLID 40604 - Needed for FormatForNewCrop and GetXMLElementValuePair_NewCrop
#include <NewCropSOAPFunctions.h>

#import "msxml.tlb"

// (j.jones 2013-04-16 17:00) - PLID 56275 - the NewCropRequestedPageType enum is now in the API

// (j.jones 2009-02-25 14:54) - PLID 33232 - Response Code enum
enum NewCropResponseCodeType {

	ncrctInvalid = -1,
	ncrctAccept = 1,
	ncrctDeny,
	ncrctUnableToProcess,
	ncrctUndetermined,
};

// (j.jones 2009-02-25 14:54) - PLID 33232 - Response Deny Code enum
enum NewCropResponseDenyCodeType {

	ncrdctInvalid = -1,
	ncrdctPatientUnknownToThePrescriber = 1,
	ncrdctPatientNeverUnderPrescriberCare,
	ncrdctPatientNoLongerUnderPrescriberCare,
	ncrdctPatientHasRequestedRefillTooSoon,
	ncrdctMedicationNeverPrescribedForThePatient,
	ncrdctPatientShouldContactPrescriberFirst,
	ncrdctRefillNotAppropriate,
	ncrdctPatientHasPickedUpPrescription,
	ncrdctPatientHasPickedUpPartialFillOfPrescription,
	ncrdctPatientHasNotPickedUpPrescriptionDrugReturnedToStock,
	ncrdctChangeNotAppropriate,
	ncrdctPatientNeedsAppointment,
	ncrdctPrescriberNotAssociatedWithThisPracticeOrLocation,
	ncrdctNoAttemptWillBeMadeToObtainPriorAuthorization,
	ncrdctDeniedNewPrescriptionToFollow,
};

// (j.gruber 2009-03-31 11:42) - PLID 33328 enum for user statii
//there are only 2 right now
// (j.jones 2009-08-18 16:23) - PLID 35203 - supported Midlevel Prescriber
//DO NOT CHANGE - THESE ARE IN DATA
enum NewCropUserTypes {
	ncutNone = 0,
	ncutLicensedPrescriber = 1,
	ncutStaff_Nurse = 2,
	ncutMidlevelProvider = 3,
};

// (j.jones 2011-06-17 09:38) - PLID 41709 - enum for ProvidersT.NewCropRole,
// stored in data, and cannot be changed
enum NewCropProviderRoles {
	ncprNone = -1,					//a -1 is not saved in data
	ncprLicensedPrescriber = 1,		//a licensed prescriber can be used as a supervising provider
	ncprMidlevelProvider = 2,
};

// (j.jones 2009-02-18 17:16) - PLID 33163 - web-browser requests

// (j.jones 2009-02-27 09:53) - PLID 33163 - this function builds the full XML necessary to login
// to a paient's account, with a destination of the ComposeRx page
// (j.gruber 2009-05-12 12:56) - PLID 29906 - Added diagcodes
// (j.gruber 2009-05-15 14:00) - PLID 28541 - added ability to exclude footer
// (j.gruber 2009-05-15 17:34) - PLID 28541 - added ability to specify route page
// (j.gruber 2009-06-08 09:59) - PLID 34515  - added user information and role
// (j.jones 2009-08-25 09:03) - PLID 35203 - split Linked Provider and Supervising Provider information
// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
// (j.jones 2010-06-03 15:04) - PLID 38994 - supported sending the insurance company names
// (j.jones 2013-01-03 16:24) - PLID 49624 - this function is now obsolete, its logic is now in the API
/*
BOOL GenerateLoginToPatientAccountRequest(OUT CString &strXmlDocument,
						long nPatientID, long nUserDefinedID,
						CString strPatientLast, CString strPatientFirst, CString strPatientMiddle,
						CString strPatientPrefix, CString strPatientSuffix,
						CString strPatientHomePhone, COleDateTime dtPatientBirthDate, long nPatientGender,
						CString strPatientAddress1, CString strPatientAddress2,
						CString strPatientCity, CString strPatientState, CString strPatientZip,
						CString strPatientCountry,
						CString strPatientPrimaryInsuranceCoName, CString strPatientSecondaryInsuranceCoName,
						long nLinkedProviderID, CString strLinkedProviderLast, CString strLinkedProviderFirst, CString strLinkedProviderMiddle,
						CString strLinkedProviderPrefix, CString strLinkedProviderSuffix,
						CString strLinkedProviderDEANumber, CString strLinkedProviderUPIN, CString strLinkedProviderState, CString strLinkedProviderLicenseNumber,
						CString strLinkedProviderNPI,
						long nSupervisingProviderID, CString strSupervisingProviderLast, CString strSupervisingProviderFirst, CString strSupervisingProviderMiddle,
						CString strSupervisingProviderPrefix, CString strSupervisingProviderSuffix,
						CString strSupervisingProviderDEANumber, CString strSupervisingProviderUPIN, CString strSupervisingProviderState, CString strSupervisingProviderLicenseNumber,
						CString strSupervisingProviderNPI,
						CString strAccountName,
						CString strAccountAddress1, CString strAccountAddress2,
						CString strAccountCity, CString strAccountState, CString strAccountZip,
						CString strAccountCountry, CString strAccountPhone, CString strAccountFax,
						long nLocationID, CString strLocationName,
						CString strLocationAddress1, CString strLocationAddress2,
						CString strLocationCity, CString strLocationState, CString strLocationZip,
						CString strLocationCountry, CString strLocationPhone, CString strLocationFax,
						CString strAllDiagCodesXML, NewCropUserTypes ncuTypeID, 
						CString strUserID, CString strUserFirst, CString strUserMiddle, CString strUserLast, 
						CString strUserPrefix, CString strUserSuffix,
						BOOL bAddNcScriptFooter = TRUE,
						NewCropRequestedPageType = ncrptCompose);
*/

// (j.jones 2009-03-04 17:00) - PLID 33345 - this function builds the full XML necessary to login
// to a paient's account, and submit a given prescription to be edited on the ComposeRx page
/* temporarily removed
BOOL GenerateFullOutsidePrescriptionRequest(OUT CString &strXmlDocument,
						long nPatientID, long nUserDefinedID,
						CString strPatientLast, CString strPatientFirst, CString strPatientMiddle,
						CString strPatientHomePhone, COleDateTime dtPatientBirthDate, long nPatientGender,
						CString strPatientAddress1, CString strPatientAddress2,
						CString strPatientCity, CString strPatientState, CString strPatientZip,
						CString strPatientCountry,
						long nPrescriptionID, CString strNewCropPharmacyID,
						CString strPharmacyPhone, CString strPharmacyFax,
						COleDateTime dtPrescriptionDate,
						CString strDoctorFullName, CString strDrugName, CString strQuantity,
						CString strRefillsAllowed, CString strPatientExplanation,
						CString strPharmacistNote, BOOL bAllowSubstitutions,
						CString strDrugStrengthWithUOM,
						CString strDrugDosageForm, CString strDrugRoute,
						long nProviderID, CString strProviderLast, CString strProviderFirst, CString strProviderMiddle,
						CString strDEANumber, CString strUPIN, CString strProviderState, CString strLicenseNumber,
						CString strProviderNPI,
						CString strAccountName,
						CString strAccountAddress1, CString strAccountAddress2,
						CString strAccountCity, CString strAccountState, CString strAccountZip,
						CString strAccountCountry, CString strAccountPhone, CString strAccountFax,
						long nLocationID, CString strLocationName,
						CString strLocationAddress1, CString strLocationAddress2,
						CString strLocationCity, CString strLocationState, CString strLocationZip,
						CString strLocationCountry, CString strLocationPhone, CString strLocationFax);
*/

// (j.jones 2009-02-25 10:16) - PLID 33232 - added ability to generate a renewal
//this function builds the full XML necessary to login to a paient's account, and respond to a renewal request
// (j.gruber 2009-05-15 13:53) - PLID 28541 - reinstated with some changes
BOOL GenerateFullPrescriptionRenewalResponse(OUT CString &strXmlDocument,
						CString strPatientLoginXML,
						CString strRenewalRequestIdentifier, NewCropResponseCodeType ncrctResponseCode = ncrctUndetermined,
						CString strRefillCount = "", NewCropResponseDenyCodeType ncrdctDenyCode = ncrdctInvalid,
						CString strMessageToPharmacist = "");

void GenerateNCScriptHeader(OUT CString &strXmlDocument);
void GenerateNCScriptFooter(OUT CString &strXmlDocument);

// (j.jones 2013-01-03 16:24) - PLID 49624 - these functions are all now obsolete, their logic is now in the API
/*
// (j.jones 2009-02-18 17:26) - PLID 33163 - web-browser request components
BOOL GenerateAddress(CString strAddressName, OUT CString &strXmlDocument,
					 CString strAddress1, CString strAddress2,
					 CString strCity, CString strState, CString strZip,
					 CString strCountry);
BOOL GenerateCredentials(OUT CString &strXmlDocument);
// (j.gruber 2009-06-08 11:03) - PLID 34515 - added staff/nurse
BOOL GenerateUserRole(OUT CString &strXmlDocument, NewCropUserTypes ncuTypeID);
BOOL GenerateDestination(OUT CString &strXmlDocument, NewCropRequestedPageType ncrptPageType);
BOOL GenerateAccount(OUT CString &strXmlDocument, CString strAccountName,
					  CString strAddress1, CString strAddress2,
					  CString strCity, CString strState, CString strZip,
					  CString strCountry, CString strPhone, CString strFax);
BOOL GenerateLocation(OUT CString &strXmlDocument, long nLocationID, CString strLocationName,
					  CString strAddress1, CString strAddress2,
					  CString strCity, CString strState, CString strZip,
					  CString strCountry, CString strPhone, CString strFax);

// (j.jones 2009-08-18 16:23) - PLID 35203 - I added support for Midlevel Prescriber, which uses the same XML
// output as Licensed Prescriber, except that DEA number is not required for Midlevel Prescribers.
// So I renamed this function appropriately and made it more modular.
// Also added bIsSupervising for better context-sensitive warnings when sending a supervising provider XML segment.
// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
BOOL GeneratePrescriber(OUT CString &strXmlDocument, NewCropUserTypes ncutSendAsTypeID,
						BOOL bIsSupervising, long nProviderID,
						CString strLast, CString strFirst, CString strMiddle,
						CString strPrefix, CString strSuffix,
						CString strDEANumber, CString strUPIN, CString strState, CString strLicenseNumber,
						CString strNPI);

// (j.gruber 2009-06-08 10:12) - PLID 34515 - add Staff
// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
BOOL GenerateStaff_Nurse(OUT CString &strXmlDocument, CString strUserID, CString strUserFirst, CString strUserMiddle, CString strUserLast,
						 CString strUserPrefix, CString strUserSuffix);		

// (j.gruber 2009-05-12 13:09) - PLID 29906 - added diagcodes
// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
// (j.jones 2010-06-03 15:04) - PLID 38994 - supported sending the insurance company names
BOOL GeneratePatient(OUT CString &strXmlDocument, long nPatientID, long nUserDefinedID,
					 CString strLast, CString strFirst, CString strMiddle,
					 CString strPrefix, CString strSuffix,
					 CString strHomePhone, COleDateTime dtBirthDate, long nGender,
					 CString strAddress1, CString strAddress2,
					 CString strCity, CString strState, CString strZip,
					 CString strCountry, CString strAddDiagCodesXML,
					 CString strPrimaryInsuranceCoName, CString strSecondaryInsuranceCoName);
*/


// (j.jones 2009-03-04 17:00) - PLID 33345 - support the ExternalOverrideDrug element
/* temporarily removed
BOOL GenerateExternalOverrideDrug(OUT CString &strXmlDocument, CString strDrugName,
								  CString strDrugStrengthWithUOM, CString strDrugDosageForm,
								  CString strDrugRoute);
// (j.jones 2009-03-04 17:00) - PLID 33345 - support the OutsidePrescription element
/* temporarily removed
BOOL GenerateOutsidePrescription(OUT CString &strXmlDocument, long nPrescriptionID,
								 CString strNewCropPharmacyID, CString strPharmacyPhone, CString strPharmacyFax,
								 COleDateTime dtPrescriptionDate,
								 CString strDoctorFullName, CString strDrugName, CString strQuantity,
								 CString strRefillsAllowed, CString strPatientExplanation,
								 CString strPharmacistNote, BOOL bAllowSubstitutions,
								 CString strDrugStrengthWithUOM, CString strDrugDosageForm, CString strDrugRoute);
*/
// (j.jones 2009-02-25 10:16) - PLID 33232 - added ability to generate a renewal
// (j.gruber 2009-05-15 14:07) - PLID 28541 - back in
BOOL GeneratePrescriptionRenewalResponse(OUT CString &strXmlDocument,
										 CString strRenewalRequestIdentifier, NewCropResponseCodeType ncrctResponseCode = ncrctUndetermined,
										 CString strRefillCount = "", NewCropResponseDenyCodeType ncrdctDenyCode = ncrdctInvalid,
										 CString strMessageToPharmacist = "");


// (j.dinatale 2010-10-04) - PLID 40604 - Moved to NewCropSOAPFunctions static lib
//FormatForNewCrop will take in a value and strip out characters NewCrop does not support
//CString FormatForNewCrop(CString strValue);

// (j.dinatale 2010-10-04) - PLID 40604 - Moved to NewCropSOAPFunctions static lib
//GetXMLElementValuePair_NewCrop will build an XML pair of <strElement>strValue</strElement>,
//and will format strValue to be NewCrop-friendly unless bIsValueXML is TRUE, in which case
//we are merely wrapping existing XML in a new strElement
//CString GetXMLElementValuePair_NewCrop(CString strElement, CString strValue, BOOL bIsValueXML = FALSE);

// (j.jones 2009-03-04 17:00) - PLID 33345 - root function for creating NewCrop XML
// from a given Prescription ID, to submit as an OutsidePrescription
/* temporarily removed
BOOL GenerateNewCropPrescriptionXML(OUT CString &strXmlDocument, long nPrescriptionID);
*/

// (j.jones 2009-02-27 09:56) - PLID 33163 - root function for creating NewCrop XML
// from a given Patient ID, to log in to that patient's account (submitting no new info.)
// (j.gruber 2009-05-15 14:02) - PLID 28541 - added ability to exclude nxscriptfooter
// (j.gruber 2009-05-15 17:37) - PLID 28541 - added ability to change request page
// (j.gruber 2009-06-08 09:45) - PLID 34515 - added role
// (j.jones 2009-08-25 09:03) - PLID 35203 - split LinkedProviderID and SupervisingProviderID
// (j.jones 2011-06-17 11:03) - PLID 41709 - split LinkedProviderID into LicensedPrescriberID and MidlevelProviderID
// (j.jones 2013-01-03 16:24) - PLID 49624 - this function is now obsolete, its logic is now in the API
/*
BOOL GenerateNewCropPatientLoginXML(OUT CString &strXmlDocument, long nPatientID, long nLicensedPrescriberID, long nMidlevelProviderID, long nSupervisingProviderID,
									long nLocationID, NewCropUserTypes ncuTypeID, BOOL bAddNcScriptFooter = TRUE, 
									NewCropRequestedPageType ncrptPage = ncrptCompose);
*/

// (j.gruber 2009-05-12 12:58) - PLID 29906 - added diagnosis codes
// (j.jones 2013-01-03 16:24) - PLID 49624 - this function is now obsolete, its logic is now in the API
/*
BOOL GenerateNewCropDiagCodeXML(OUT CString &strXMLDiagCode, CString strDiagCode, CString strDiagCodeDesc);
*/

// (j.jones 2009-02-27 14:59) - PLID 33163 - added function to get the RxEntry URL for production or pre-production
CString GetNewCropRxEntryURL();

// (j.gruber 2009-02-27 15:46) - PLID 33273 - function to validate new crop ncScript and NcStandard
// (a.walling 2009-04-07 13:20) - PLID 33306 - const CString reference
BOOL ValidateNewCropXMLSchemas(const CString &strXML);

// (j.jones 2009-03-04 10:12) - PLID 33163 - function to validate that a state code is a US state or territory
BOOL VerifyIsNewCropValidState(CString strState);

// (j.jones 2009-12-18 11:21) - PLID 36391 - added validation for a person's prefix or suffix (PersonT.Title)
BOOL VerifyIsNewCropValidNamePrefix(CString strPrefix);
BOOL VerifyIsNewCropValidNameSuffix(CString strSuffix);

// (j.jones 2009-08-18 17:37) - PLID 35203 - added simple function to determine whether a user is prescriber type
BOOL IsNewCropPrescriberRole(NewCropUserTypes ncuTypeID);

// (j.jones 2009-08-25 08:34) - PLID 35203 - added function to determined whether a user is a "supervised" type,
// in that it can have a supervising provider
BOOL IsNewCropSupervisedRole(NewCropUserTypes ncuTypeID);

// (j.gruber 2009-03-31 11:42) - PLID 33328 - function to check status
// (j.gruber 2009-06-08 10:35) - PLID 34515 - added role
BOOL CheckEPrescribingStatus(NewCropUserTypes &ncuTypeID, long nUserID, CString strUserName);

// (j.jones 2010-06-09 11:09) - PLID 39013 - gets the production status, which most of the time
// is hard coded on for clients, off for NexTech, but can be overridden
BOOL GetNewCropIsProduction();

// (j.jones 2011-06-20 09:20) - PLID 44127 - EnsureUniqueDEANumberForProvider will optionally take in
// a provider ID, and if they have a licensed prescriber NewCrop role it will check to see
// if any other licensed prescriber has the same DEA. Returns TRUE if the provider's DEA is unique.
// Can optionally take in a -1 provider ID to find all duplicate DEA numbers for licensed prescribers.
BOOL EnsureUniqueDEANumber(IN OUT CString &strInvalidProviders, long nProviderID = -1);

#endif	//NEWCROPUTILS_H
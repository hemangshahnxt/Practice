// ImportUtils.cpp: implementation of the ImportUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImportUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


long GetGenderValueFromString(CString strInput)
{
	if( strInput.CompareNoCase("M")==0 || 
		strInput.CompareNoCase("MALE") == 0 ||
		strInput == "1"){
		return 1;
	}
	else if(strInput.CompareNoCase("F")==0 ||
		strInput.CompareNoCase("FEMALE") == 0 ||
		strInput == "2"){
		return 2;
	}
	else{
		return 0; //unset
	}
}

CString GetGenderAsTextFromString(CString strInput)
{
	if( strInput.CompareNoCase("M")==0 || 
		strInput.CompareNoCase("MALE") == 0 ||
		strInput == "1"){
		return "Male";
	}
	else if(strInput.CompareNoCase("F")==0 ||
		strInput.CompareNoCase("FEMALE") == 0 ||
		strInput == "2"){
		return "Female";
	}
	else{
		return ""; //unset
	}
}


CString GetImportFieldHeader(ImportFieldNumber ifnField)
{
	CString strFieldHeader;

	switch (ifnField){
	default:
	case ifnIgnore:
		strFieldHeader = "< Ignore >"; break;
	case ifnLastName:
		strFieldHeader = "Last Name"; break;
	case ifnFirstName:
		strFieldHeader = "First Name"; break;
	case ifnMiddleName:
		strFieldHeader = "Middle Name"; break;
	case ifnPatientID:
		strFieldHeader = "Patient ID"; break;
	case ifnAddress1:
		strFieldHeader = "Address 1"; break;
	case ifnAddress2:
		strFieldHeader = "Address 2"; break;
	case ifnCity:
		strFieldHeader = "City"; break;
	case ifnState:
		strFieldHeader = "State"; break;
	case ifnZip:
		strFieldHeader = "Zip"; break;
	case ifnSocialSecurity:
		strFieldHeader = "Social Security"; break;
	case ifnHomePhone:
		strFieldHeader = "Home Phone"; break;
	case ifnBirthdate:
		strFieldHeader = "Birthdate"; break;
	case ifnGender:
		strFieldHeader = "Gender"; break;
	case ifnWorkPhone:
		strFieldHeader = "Work Phone"; break;
	case ifnWorkExt:
		strFieldHeader = "Work Extension"; break;
	case ifnCellPhone:
		strFieldHeader = "Cell Phone"; break;
	case ifnPager:
		strFieldHeader = "Pager"; break;
	case ifnOtherPhone:
		strFieldHeader = "Other Phone"; break;
	case ifnFax:
		strFieldHeader = "Fax"; break;
	case ifnEmail:
		strFieldHeader = "Email"; break;
	case ifnTitle:
		strFieldHeader = "Title"; break;
	case ifnCompany:
		strFieldHeader = "Company Name"; break;
	case ifnAccount:
		strFieldHeader = "Account Number"; break;
	//(e.lally 2007-06-08) PLID 26262 - Add Provider fields
	//(e.lally 2007-06-11) PLID 10320 - Add referring phys headers, for now, we can reuse the provider text 
	case ifnProviderNPI:
	case ifnRefPhysNPI:
		strFieldHeader = "NPI"; break;
	case ifnProviderFederalEmpNumber:
	case ifnRefPhysFederalEmpNumber:
		strFieldHeader = "Federal Employer ID"; break;
	case ifnProviderWorkersCompNumber:
	case ifnRefPhysWorkersCompNumber:
		strFieldHeader = "Workers Compensation ID"; break;
	case ifnProviderMedicaidNumber:
	case ifnRefPhysMedicaidNumber:
		strFieldHeader = "Medicaid ID"; break;
	case ifnProviderLicenseNumber:
	case ifnRefPhysLicenseNumber:
		strFieldHeader = "License Number"; break;
	case ifnProviderBCBSNumber:
	case ifnRefPhysBCBSNumber:
		strFieldHeader = "BCBS ID"; break;
	case ifnProviderTaxonomyCode:
	case ifnRefPhysTaxonomyCode:
		strFieldHeader = "Taxonomy Code"; break;
	case ifnProviderUPIN:
	case ifnRefPhysUPIN:
		strFieldHeader = "UPIN"; break;
	case ifnProviderMedicare:
	case ifnRefPhysMedicare:
		strFieldHeader = "Medicare ID"; break;
	case ifnProviderOtherID:
	case ifnRefPhysOtherID:
		strFieldHeader = "Other ID"; break;
	case ifnProviderDEANumber:
	case ifnRefPhysDEANumber:
		strFieldHeader = "DEA Number"; break;
	case ifnRefPhysID:
		strFieldHeader = "Referring Physician ID"; break;
	//(e.lally 2007-06-11) PLID 26274 - Add supplier headers
	case ifnSupplierPayMethod:
		strFieldHeader = "Payment Method"; break;
	//(e.lally 2007-06-12) PLID 26273 - Add user headers
	case  ifnUserUsername:
		strFieldHeader = "User Name"; break;
	case ifnUserNationalEmpNum:
		strFieldHeader = "National Emp #"; break;
	case ifnUserDateOfHire:
		strFieldHeader = "Date Of Hire"; break;
	case ifnUserPatientCoord:
		strFieldHeader = "Patient Coordinator"; break;
	//(e.lally 2007-06-12) PLID 26275 - Add other contact headers
	case ifnOtherContactNurse:
		strFieldHeader = "Nurse"; break;
	case ifnOtherContactAnesthesiologist:
		strFieldHeader = "Anesthesiologist"; break;
	// (j.jones 2010-04-02 17:55) - PLID 16717 - supported service codes
	case ifnServiceName:
		strFieldHeader = "Description"; break;
	case ifnServicePrice:
		strFieldHeader = "Standard Fee"; break;
	case ifnTaxable1:
		strFieldHeader = "Taxable (Tax 1)"; break;
	case ifnTaxable2:
		strFieldHeader = "Taxable (Tax 2)"; break;
	case ifnBarcode:
		strFieldHeader = "Barcode"; break;
	case ifnServiceCode:
		strFieldHeader = "Service Code"; break;
	case ifnServiceSubCode:
		strFieldHeader = "Sub Code"; break;
	case ifnRVU:
		strFieldHeader = "R.V.U."; break;
	case ifnGlobalPeriod:
		strFieldHeader = "Global Period (Days)"; break;
	case ifnResourceName:
		strFieldHeader = "Resource Name"; break; // (r.farnworth 2015-03-16 14:36) - PLID 65198
	// (r.farnworth 2015-03-19 12:06) - PLID 65239 - Create fields for the Products object for the import utility.
	case ifnProductName:
		strFieldHeader = "Product Name"; break;
	case ifnProductDescription:
		strFieldHeader = "Unit Description"; break;
	case ifnProductPrice:
		strFieldHeader = "Price"; break;
	case ifnProductLastCost:
		strFieldHeader = "Last Cost"; break;
	case ifnProductTaxable1:
		strFieldHeader = "Taxable 1"; break;
	case ifnProductTaxable2:
		strFieldHeader = "Taxable 2"; break;
	case ifnProductBarcode:
		strFieldHeader = "Barcode"; break;
	case ifnProductBillable:
		strFieldHeader = "Billable"; break;
	case ifnProductOnHand:
		strFieldHeader = "On Hand"; break;
	// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
	/*case ifnPromptForCopay:
		strFieldHeader = "Prompt For CoPay"; break;*/
	// (b.savon 2015-03-19 12:38) - PLID 65144 - Add new fields to patient object for the import utility
	case ifnLocation:
		strFieldHeader = "Location Name"; break;
	case ifnProviderName:
		strFieldHeader = "Provider Name"; break;
	case ifnEthnicity:
		strFieldHeader = "Ethnicity"; break;
	case ifnLanguage:
		strFieldHeader = "Language"; break;
	case ifnRace:
		strFieldHeader = "Race"; break;
	case ifnReferringPhysicianName:
		strFieldHeader = "Referring Phys Name"; break;
	case ifnPrimaryCarePhysicianName:
		strFieldHeader = "PCP Name"; break;
	case ifnPatientCurrentStatus:
		strFieldHeader = "Patient Current Status"; break;
	case ifnFirstContactDate:
		strFieldHeader = "First Contact Date"; break;
	case ifnReferralSourceName:
		strFieldHeader = "Referral Source Name"; break;
	case ifnEmergencyContactFirstName:
		strFieldHeader = "Emergency Contact First Name"; break;
	case ifnEmergencyContactLastName:
		strFieldHeader = "Emergency Contact Last Name"; break;
	case ifnEmergencyContactRelation:
		strFieldHeader = "Emergency Contact Relation"; break;
	case ifnEmergencyContactHomePhone:
		strFieldHeader = "Emergency Contact Home Phone"; break;
	case ifnEmergencyContactWorkPhone:
		strFieldHeader = "Emergency Contact Work Phone"; break;
	case ifnWarningMessage:
		strFieldHeader = "Warning Message"; break;
	case ifnNote:
		strFieldHeader = "Note"; break;
	case ifnGen1Custom1:
		strFieldHeader = "Gen 1 - Custom 1"; break;
	case ifnGen1Custom2:
		strFieldHeader = "Gen 1 - Custom 2"; break;
	case ifnGen1Custom3:
		strFieldHeader = "Gen 1 - Custom 3"; break;
	case ifnGen1Custom4:
		strFieldHeader = "Gen 1 - Custom 4"; break;
	case ifnCustomText1:
		strFieldHeader = "Custom Text 1"; break;
	case ifnCustomText2:
		strFieldHeader = "Custom Text 2"; break;
	case ifnCustomText3:
		strFieldHeader = "Custom Text 3"; break;
	case ifnCustomText4:
		strFieldHeader = "Custom Text 4"; break;
	case ifnCustomText5:
		strFieldHeader = "Custom Text 5"; break;
	case ifnCustomText6:
		strFieldHeader = "Custom Text 6"; break;
	case ifnCustomText7:
		strFieldHeader = "Custom Text 7"; break;
	case ifnCustomText8:
		strFieldHeader = "Custom Text 8"; break;
	case ifnCustomText9:
		strFieldHeader = "Custom Text 9"; break;
	case ifnCustomText10:
		strFieldHeader = "Custom Text 10"; break;
	case ifnCustomText11:
		strFieldHeader = "Custom Text 11"; break;
	case ifnCustomText12:
		strFieldHeader = "Custom Text 12"; break;
	case ifnCustomNote:
		strFieldHeader = "Custom Note"; break;
	case ifnCustomCheckbox1:
		strFieldHeader = "Custom Checkbox 1"; break;
	case ifnCustomCheckbox2:
		strFieldHeader = "Custom Checkbox 2"; break;
	case ifnCustomCheckbox3:
		strFieldHeader = "Custom Checkbox 3"; break;
	case ifnCustomCheckbox4:
		strFieldHeader = "Custom Checkbox 4"; break;
	case ifnCustomCheckbox5:
		strFieldHeader = "Custom Checkbox 5"; break;
	case ifnCustomCheckbox6:
		strFieldHeader = "Custom Checkbox 6"; break;
	case ifnCustomDate1:
		strFieldHeader = "Custom Date 1"; break;
	case ifnCustomDate2:
		strFieldHeader = "Custom Date 2"; break;
	case ifnCustomDate3:
		strFieldHeader = "Custom Date 3"; break;
	case ifnCustomDate4:
		strFieldHeader = "Custom Date 4"; break;
	case ifnMaritalStatus:
		strFieldHeader = "Marital Status"; break;
	// (b.savon 2015-03-30 17:35) - PLID 65233 - Create fields for the Recalls object for the import utility.
	case ifnRecallDate:
		strFieldHeader = "Recall Date"; break;
	case ifnRecallTemplateName:
		strFieldHeader = "Recall Template"; break;
	case ifnCustomPatientID: // (r.farnworth 2015-04-01 10:28) - PLID 65246
		strFieldHeader = "Patient Mapping ID"; break;
	// (r.farnworth 2015-04-01 17:28) - PLID 65167 - Create fields for the insurance companies object for the import utility
	case ifnConversionID:
		strFieldHeader = "Conversion ID"; break;
	case ifnInsCoName:
		strFieldHeader = "Insurance Co. Name"; break;
	case ifnContactFirst:
		strFieldHeader = "Contact First Name"; break;
	case ifnContactLast:
		strFieldHeader = "Contact Last Name"; break;
	case ifnContactTitle:
		strFieldHeader = "Contact Title"; break;
	case ifnContactPhone:
		strFieldHeader = "Contact Phone"; break;
	case ifnContactFax:
		strFieldHeader = "Contact Fax"; break;
	case ifnContactNote:
		strFieldHeader = "Contact Note"; break;
	case ifnHCFAPayerID:
		strFieldHeader = "HCFA Payer ID"; break;
	case ifnUBPayerID:
		strFieldHeader = "UB Payer ID"; break;
	case ifnEligibilityPayerID:
		strFieldHeader = "Eligibility Payer ID"; break;

		//(s.dhole 4/8/2015 1:28 PM ) - PLID 65224  Fields header we required for Patient notes
	case ifnPatientNoteCategory: 
		strFieldHeader = "Note Category"; break;
	case ifnPatientNoteText:
		strFieldHeader = "Note"; break;
	case ifnPatientNoteDateTime:
		strFieldHeader = "Note Date"; break;
	case ifnPatientNotePriority:
		strFieldHeader = "Note Priority"; break;
	// (b.savon 2015-04-06 09:15) - PLID 65216 - Create fields for the Appointment object for the import utility.
	case ifnAppointmentDate:
		strFieldHeader = "Appointment Date"; break;
	case ifnAppointmentIsEvent:
		strFieldHeader = "Appointment Event"; break;
	case ifnAppointmentStartTime:
		strFieldHeader = "Appointment Start Time"; break;
	case ifnAppointmentEndTime:
		strFieldHeader = "Appointment End Time"; break;
	case ifnAppointmentDuration:
		strFieldHeader = "Appointment Duration"; break;
	case ifnAppointmentType:
		strFieldHeader = "Appointment Type"; break;
	case ifnAppointmentPurpose:
		strFieldHeader = "Appointment Purpose"; break;
	case ifnAppointmentNotes:
		strFieldHeader = "Appointment Notes"; break;
	case ifnAppointmentIsConfirmed:
		strFieldHeader = "Appointment Confirmed"; break;
	case ifnAppointmentIsCancelled:
		strFieldHeader = "Appointment Cancelled"; break;
	case ifnAppointmentIsNoShow:
		strFieldHeader = "Appointment No Show"; break;
	//(s.dhole 4/14/2015 9:52 AM ) - PLID 65191 Added Insured Party fields header
	case ifnInsuranceCompanyConversionID:
		strFieldHeader = "Insurance Conversion ID"; break;
	case ifnInsuranceCompanyName:
		strFieldHeader = "Insurance Company Name"; break;
	case ifnInsuredEmployer:
		strFieldHeader  = "Employer"; break;
	case ifnInsuredInsuranceID:
		strFieldHeader  = "Insurance ID #"; break;
	case ifnInsuredGroupNo://(s.dhole 5/6/2015 10:55 AM ) - PLID 65755
		strFieldHeader = "Group #"; break;
	case ifnInsuredCopay:
		strFieldHeader  = "Copay $"; break;
	case ifnInsuredCopayPercent:
		strFieldHeader  = "Copay %"; break;
	case ifnInsuredEffectiveDate:
		strFieldHeader  = "Effective Date"; break;
	case ifnInsuredInactiveDate:
		strFieldHeader  = "Inactive Date"; break;
	case ifnInsuredPartyRespTypeID:
		strFieldHeader = "Responsibility Type ID"; break;
	case ifnInsuredPartyRelation:
		strFieldHeader = "Patient Relationship to the Insured Party"; break;
	// (r.goldschmidt 2016-02-09 14:59) - PLID 68163 - Add fields for new import type, Races
	case ifnRacePreferredName:
		strFieldHeader = "Race Preferred Name"; break;
	case ifnRaceCDCCode:
		strFieldHeader = "Race CDC Code"; break;
	}
	return strFieldHeader;
}

CString GetImportFieldDataField(ImportFieldNumber ifnField)
{
	CString strFieldDef;

	switch (ifnField){
	case ifnLastName:
		strFieldDef = "Last"; break;
	case ifnFirstName:
		strFieldDef = "First"; break;
	case ifnMiddleName:
		strFieldDef = "Middle"; break;
	case ifnPatientID:
		strFieldDef = "UserdefinedID"; break;
	case ifnAddress1:
		strFieldDef = "Address1"; break;
	case ifnAddress2:
		strFieldDef = "Address2"; break;
	case ifnCity:
		strFieldDef = "City"; break;
	case ifnState:
		strFieldDef = "State"; break;
	case ifnZip:
		strFieldDef = "Zip"; break;
	case ifnSocialSecurity:
		strFieldDef = "SocialSecurity"; break;
	case ifnHomePhone:
		strFieldDef = "HomePhone"; break;
	case ifnBirthdate:
		strFieldDef = "Birthdate"; break;
	case ifnGender:
		strFieldDef = "Gender"; break;
	case ifnWorkPhone:
		strFieldDef = "WorkPhone"; break;
	case ifnWorkExt:
		strFieldDef = "Extension"; break;
	case ifnCellPhone:
		strFieldDef = "CellPhone"; break;
	case ifnPager:
		strFieldDef = "Pager"; break;
	case ifnOtherPhone:
		strFieldDef = "OtherPhone"; break;
	case ifnFax:
		strFieldDef = "Fax"; break;
	case ifnEmail:
		strFieldDef = "Email"; break;
	case ifnTitle:
		strFieldDef = "Title"; break;
	case ifnCompany:
		strFieldDef = "Company"; break;
	case ifnAccount:
		strFieldDef = "CompanyID"; break;
	//(e.lally 2007-06-08) PLID 26262 - Add Provider fields
	case ifnProviderNPI:
		strFieldDef = "NPI"; break;
	case ifnProviderFederalEmpNumber:
		strFieldDef = "[Fed Employer ID]"; break;
	case ifnProviderWorkersCompNumber:
		strFieldDef = "[Workers Comp Number]"; break;
	case ifnProviderMedicaidNumber:
		strFieldDef = "[Medicaid Number]"; break;
	case ifnProviderLicenseNumber:
		strFieldDef = "License"; break;
	case ifnProviderBCBSNumber:
		strFieldDef = "[BCBS Number]"; break;
	case ifnProviderTaxonomyCode:
		strFieldDef = "TaxonomyCode"; break;
	case ifnProviderUPIN:
		strFieldDef = "UPIN"; break;
	case ifnProviderMedicare:
		strFieldDef = "[Medicare Number]"; break;
	case ifnProviderOtherID:
		strFieldDef = "[Other ID Number]"; break;
	case ifnProviderDEANumber:
		strFieldDef = "[DEA Number]"; break;
	//(e.lally 2007-06-11) PLID 10320 - Add Referring Physician fields
	case ifnRefPhysNPI:
		strFieldDef = "NPI"; break;
	case ifnRefPhysFederalEmpNumber:
		strFieldDef = "FedEmployerID"; break;
	case ifnRefPhysWorkersCompNumber:
		strFieldDef = "WorkersCompNumber"; break;
	case ifnRefPhysMedicaidNumber:
		strFieldDef = "MedicaidNumber"; break;
	case ifnRefPhysLicenseNumber:
		strFieldDef = "License"; break;
	case ifnRefPhysBCBSNumber:
		strFieldDef = "BlueShieldID"; break;
	case ifnRefPhysTaxonomyCode:
		strFieldDef = "TaxonomyCode"; break;
	case ifnRefPhysUPIN:
		strFieldDef = "UPIN"; break;
	case ifnRefPhysMedicare:
		strFieldDef = "MedicareNumber"; break;
	case ifnRefPhysOtherID:
		strFieldDef = "OtherIDNumber"; break;
	case ifnRefPhysDEANumber:
		strFieldDef = "DEANumber"; break;
	case ifnRefPhysID:
		strFieldDef = "ReferringPhyID"; break;
	//(e.lally 2007-06-11) PLID 26274 - Add supplier fields
	case ifnSupplierPayMethod:
		strFieldDef = "CCNumber"; break;
	//(e.lally 2007-06-12) PLID 26273 - Add user fields
	case  ifnUserUsername:
		strFieldDef = "UserName"; break;
	case ifnUserNationalEmpNum:
		strFieldDef = "NationalEmplNumber"; break;
	case ifnUserDateOfHire:
		strFieldDef = "DateOfHire"; break;
	case ifnUserPatientCoord:
		strFieldDef = "PatientCoordinator"; break;
	//(e.lally 2007-06-12) PLID 26275 - Add other contact fields
	case ifnOtherContactNurse:
		strFieldDef = "Nurse"; break;
	case ifnOtherContactAnesthesiologist:
		strFieldDef = "Anesthesiologist"; break;
	// (j.jones 2010-04-02 17:55) - PLID 16717 - supported service codes
	case ifnServiceName:
		strFieldDef = "Name"; break;
	case ifnServicePrice:
		strFieldDef = "Price"; break;
	case ifnTaxable1:
		strFieldDef = "Taxable1"; break;
	case ifnTaxable2:
		strFieldDef = "Taxable2"; break;
	case ifnBarcode:
		strFieldDef = "Barcode"; break;
	case ifnServiceCode:
		strFieldDef = "Code"; break;
	case ifnServiceSubCode:
		strFieldDef = "Subcode"; break;
	case ifnRVU:
		strFieldDef = "RVU"; break;
	case ifnGlobalPeriod:
		strFieldDef = "GlobalPeriod"; break;
	case ifnResourceName:
		strFieldDef = "Item"; break; // (r.farnworth 2015-03-16 14:36) - PLID 65198
	// (r.farnworth 2015-03-19 12:06) - PLID 65239 - Create fields for the Products object for the import utility.
	case ifnProductName:
		strFieldDef = "Name"; break;
	case ifnProductDescription:
		strFieldDef = "UnitDesc"; break;
	case ifnProductPrice:
		strFieldDef = "Price"; break;
	case ifnProductLastCost:
		strFieldDef = "LastCost"; break;
	case ifnProductTaxable1:
		strFieldDef = "Taxable1"; break;
	case ifnProductTaxable2:
		strFieldDef = "Taxable2"; break;
	case ifnProductBarcode:
		strFieldDef = "Barcode"; break;
	case ifnProductBillable:
		strFieldDef = "Billable"; break;
	case ifnProductOnHand:
		strFieldDef = "Quantity"; break;
		// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
	/*case ifnPromptForCopay:
		strFieldDef = "PromptForCoPay"; break;*/
	// (b.savon 2015-03-19 12:38) - PLID 65144 - Add new fields to patient object for the import utility
	case ifnLocation:
		strFieldDef = "Location"; break;
	case ifnProviderName:
		strFieldDef = "MainPhysician"; break;
	case ifnEthnicity:
		strFieldDef = "Ethnicity"; break;
	case ifnLanguage:
		strFieldDef = "LanguageID"; break;
	case ifnRace:
		strFieldDef = "RaceID"; break;
	case ifnReferringPhysicianName:
		strFieldDef = "DefaultReferringPhyID"; break;
	case ifnPrimaryCarePhysicianName:
		strFieldDef = "PCP"; break;
	case ifnPatientCurrentStatus:
		strFieldDef = "CurrentStatus"; break;
	case ifnFirstContactDate:
		strFieldDef = "FirstContactDate"; break;
	case ifnReferralSourceName:
		strFieldDef = "ReferralID"; break;
	case ifnEmergencyContactFirstName:
		strFieldDef = "EmergFirst"; break;
	case ifnEmergencyContactLastName:
		strFieldDef = "EmergLast"; break;
	case ifnEmergencyContactRelation:
		strFieldDef = "EmergRelation"; break;
	case ifnEmergencyContactHomePhone:
		strFieldDef = "EmergHPhone"; break;
	case ifnEmergencyContactWorkPhone:
		strFieldDef = "EmergWPhone"; break;
	case ifnWarningMessage:
		strFieldDef = "WarningMessage"; break;
	case ifnNote:
		strFieldDef = "Note"; break;
	case ifnGen1Custom1:
		strFieldDef = "Gen1-Custom1"; break;
	case ifnGen1Custom2:
		strFieldDef = "Gen1-Custom2"; break;
	case ifnGen1Custom3:
		strFieldDef = "Gen1-Custom3"; break;
	case ifnGen1Custom4:
		strFieldDef = "Gen1-Custom4"; break;
	case ifnCustomText1:
		strFieldDef = "CustomText1"; break;
	case ifnCustomText2:
		strFieldDef = "CustomText2"; break;
	case ifnCustomText3:
		strFieldDef = "CustomText3"; break;
	case ifnCustomText4:
		strFieldDef = "CustomText4"; break;
	case ifnCustomText5:
		strFieldDef = "CustomText5"; break;
	case ifnCustomText6:
		strFieldDef = "CustomText6"; break;
	case ifnCustomText7:
		strFieldDef = "CustomText7"; break;
	case ifnCustomText8:
		strFieldDef = "CustomText8"; break;
	case ifnCustomText9:
		strFieldDef = "CustomText9"; break;
	case ifnCustomText10:
		strFieldDef = "CustomText10"; break;
	case ifnCustomText11:
		strFieldDef = "CustomText11"; break;
	case ifnCustomText12:
		strFieldDef = "CustomText12"; break;
	case ifnCustomNote:
		strFieldDef = "CustomNote"; break;
	case ifnCustomCheckbox1:
		strFieldDef = "CustomCheckbox1"; break;
	case ifnCustomCheckbox2:
		strFieldDef = "CustomCheckbox2"; break;
	case ifnCustomCheckbox3:
		strFieldDef = "CustomCheckbox3"; break;
	case ifnCustomCheckbox4:
		strFieldDef = "CustomCheckbox4"; break;
	case ifnCustomCheckbox5:
		strFieldDef = "CustomCheckbox5"; break;
	case ifnCustomCheckbox6:
		strFieldDef = "CustomCheckbox6"; break;
	case ifnCustomDate1:
		strFieldDef = "CustomDate1"; break;
	case ifnCustomDate2:
		strFieldDef = "CustomDate2"; break;
	case ifnCustomDate3:
		strFieldDef = "CustomDate3"; break;
	case ifnCustomDate4:
		strFieldDef = "CustomDate4"; break;
	case ifnMaritalStatus:
		strFieldDef = "MaritalStatus"; break;
	// (b.savon 2015-03-31 09:03) - PLID 65233 - Create fields for the Recalls object for the import utility.
	case ifnRecallDate:
		strFieldDef = "RecallDate"; break;
	case ifnRecallTemplateName:
		strFieldDef = "RecallTemplateID"; break;
	case ifnCustomPatientID: // (r.farnworth 2015-04-01 10:28) - PLID 65246
		strFieldDef = "Patient Mapping ID"; break;
	// (r.farnworth 2015-04-01 17:35) - PLID 65167 - Create fields for the insurance companies object for the import utility
	case ifnConversionID:
		strFieldDef = "ConversionID"; break;
	case ifnInsCoName:
		strFieldDef = "Name"; break;
	case ifnContactFirst:
		strFieldDef = "First"; break;
	case ifnContactLast:
		strFieldDef = "Last"; break;
	case ifnContactTitle:
		strFieldDef = "Title"; break;
	case ifnContactPhone:
		strFieldDef = "WorkPhone"; break;
	case ifnContactFax:
		strFieldDef = "Fax"; break;
	case ifnContactNote:
		strFieldDef = "Note"; break;
	case ifnHCFAPayerID:
		strFieldDef = "HCFAPayerID"; break;
	case ifnUBPayerID:
		strFieldDef = "UBPayerID"; break;
	case ifnEligibilityPayerID:
		strFieldDef = "EligPayerID"; break;

		//(s.dhole 4/8/2015 1:28 PM ) - PLID 65224  Data ields we required for Patient notes
	case ifnPatientNoteCategory:
		strFieldDef = "[Category]"; break;
	case ifnPatientNoteText:
		strFieldDef = "[Note]"; break;
	case ifnPatientNoteDateTime:
		strFieldDef = "[Date]"; break;
	case ifnPatientNotePriority:
		strFieldDef = "Priority"; break;
	// (b.savon 2015-04-06 09:15) - PLID 65216 - Create fields for the Appointment object for the import utility.
	case ifnAppointmentDate:
		strFieldDef = "Date"; break;
	case ifnAppointmentIsEvent:
		strFieldDef = "Appointment Event"; break;
	case ifnAppointmentStartTime:
		strFieldDef = "StartTime"; break;
	case ifnAppointmentEndTime:
		strFieldDef = "EndTime"; break;
	case ifnAppointmentDuration:
		strFieldDef = "Duration"; break;
	case ifnAppointmentType:
		strFieldDef = "AptTypeID"; break;
	case ifnAppointmentPurpose:
		strFieldDef = "AptPurposeID"; break;
	case ifnAppointmentNotes:
		strFieldDef = "Notes"; break;
	case ifnAppointmentIsConfirmed:
		strFieldDef = "Confirmed"; break;
	case ifnAppointmentIsCancelled:
		strFieldDef = "Status"; break;
	case ifnAppointmentIsNoShow:
		strFieldDef = "ShowState"; break;
		//(s.dhole 4/14/2015 10:23 AM ) - PLID  65191 Added Insured Party fields 
	case ifnInsuranceCompanyConversionID:
		strFieldDef = "ConversionID"; break;
	case ifnInsuranceCompanyName:
		strFieldDef = "Name"; break;
	case ifnInsuredEmployer:
		strFieldDef = "Employer"; break;
	case ifnInsuredInsuranceID:
		strFieldDef = "IDForInsurance"; break;
	case ifnInsuredGroupNo:
		strFieldDef = "PolicyGroupNum"; break;
	case ifnInsuredCopay:
		strFieldDef = "CopayMoney"; break;
	case ifnInsuredCopayPercent:
		strFieldDef = "CopayPercentage"; break;
	case ifnInsuredEffectiveDate:
		strFieldDef = "EffectiveDate"; break;
	case ifnInsuredInactiveDate:
		strFieldDef = "ExpireDate"; break;
	case ifnInsuredPartyRespTypeID:
		strFieldDef = "RespTypeID"; break;
	case ifnInsuredPartyRelation:
		strFieldDef = "RelationToPatient"; break;
	// (r.goldschmidt 2016-02-09 14:59) - PLID 68163 - Add fields for new import type, Races
	case ifnRacePreferredName:
		strFieldDef = "Name"; break;
	case ifnRaceCDCCode:
		strFieldDef = "CDCID"; break;
	default:
		ASSERT(FALSE);//need to implement this field number
		break;
	case ifnIgnore:
		//This should never happen!
		strFieldDef = "[Ignore]"; break;
	}
	return strFieldDef;
}

int GetMaxFieldLength(ImportFieldNumber ifnField)
{
	int nMaxLength = -1; //Invalid field

	switch (ifnField){
	case ifnLastName:					nMaxLength = 50; break;
	case ifnFirstName:					nMaxLength = 50; break;
	case ifnMiddleName:					nMaxLength = 50; break;
	case ifnAddress1:					nMaxLength = 75; break;
	case ifnAddress2:					nMaxLength = 75; break;
	case ifnCity:						nMaxLength = 50; break;
	case ifnState:						nMaxLength = 20; break;
	case ifnZip:						nMaxLength = 20; break;
	case ifnSocialSecurity:				nMaxLength = 11; break;
	case ifnHomePhone:					nMaxLength = 20; break;
	case ifnGender:						nMaxLength = 10; break;
	case ifnWorkPhone:					nMaxLength = 20; break;
	case ifnWorkExt:					nMaxLength = 10; break;
	case ifnCellPhone:					nMaxLength = 20; break;
	case ifnPager:						nMaxLength = 20; break;
	case ifnOtherPhone:					nMaxLength = 20; break;
	case ifnFax:						nMaxLength = 20; break;
	case ifnEmail:						nMaxLength = 50; break;
	case ifnTitle:						nMaxLength = 50; break;
	case ifnCompany:					nMaxLength = 50; break;
	case ifnAccount:					nMaxLength = 50; break;
	//Provider fields
	case ifnProviderNPI:				nMaxLength = 50; break;
	case ifnProviderFederalEmpNumber:	nMaxLength = 30; break;
	case ifnProviderWorkersCompNumber:	nMaxLength = 50; break;
	case ifnProviderMedicaidNumber:		nMaxLength = 50; break;
	case ifnProviderLicenseNumber:		nMaxLength = 50; break;
	case ifnProviderBCBSNumber:			nMaxLength = 50; break;
	case ifnProviderTaxonomyCode:		nMaxLength = 20; break;
	case ifnProviderUPIN:				nMaxLength = 50; break;
	case ifnProviderMedicare:			nMaxLength = 50; break;
	case ifnProviderOtherID:			nMaxLength = 50; break;
	case ifnProviderDEANumber:			nMaxLength = 30; break;
	//Referring Physician fields
	case ifnRefPhysNPI:					nMaxLength = 50; break;
	case ifnRefPhysFederalEmpNumber:	nMaxLength = 50; break;
	case ifnRefPhysWorkersCompNumber:	nMaxLength = 50; break;
	case ifnRefPhysMedicaidNumber:		nMaxLength = 50; break;
	case ifnRefPhysLicenseNumber:		nMaxLength = 50; break;
	case ifnRefPhysBCBSNumber:			nMaxLength = 100; break;
	case ifnRefPhysTaxonomyCode:		nMaxLength = 20; break;
	case ifnRefPhysUPIN:				nMaxLength = 100; break;
	case ifnRefPhysMedicare:			nMaxLength = 50; break;
	case ifnRefPhysOtherID:				nMaxLength = 50; break;
	case ifnRefPhysDEANumber:			nMaxLength = 50; break;
	case ifnRefPhysID:					nMaxLength = 50; break;
	//supplier fields
	case ifnSupplierPayMethod:			nMaxLength = 50; break;
	//user fields
	case  ifnUserUsername:				nMaxLength = 50; break;
	case ifnUserNationalEmpNum:			nMaxLength = 30; break;
	// (j.jones 2010-04-02 17:55) - PLID 16717 - supported service codes
	case ifnServiceName:				nMaxLength = 255; break;
	case ifnBarcode:					nMaxLength = 80; break;
	case ifnServiceCode:				nMaxLength = 50; break;
	case ifnServiceSubCode:				nMaxLength = 50; break;
	case ifnResourceName:				nMaxLength = 50; break; // (r.farnworth 2015-03-16 14:36) - PLID 65198
	// (r.farnworth 2015-03-19 12:27) - PLID 65239 - Create fields for the Products object for the import utility.
	case ifnProductName:				nMaxLength = 255; break;
	case ifnProductDescription:			nMaxLength = 50; break;
	case ifnProductBarcode:				nMaxLength = 50; break;
	// (r.farnworth 2015-07-06 10:51) - PLID 65525 - Increased MaxLength to 5
	case ifnProductBillable:			nMaxLength = 5; break;
	case ifnProductTaxable1:			nMaxLength = 5; break;
	case ifnProductTaxable2:			nMaxLength = 5; break;
	// (b.savon 2015-03-19 12:38) - PLID 65144 - Add new fields to patient object for the import utility
	case ifnLocation:					nMaxLength = 255; break;
	case ifnProviderName:				nMaxLength = 150; break;
	case ifnEthnicity:					nMaxLength = 255; break;
	case ifnLanguage:					nMaxLength = 255; break;
	case ifnRace:						nMaxLength = 255; break;
	case ifnReferringPhysicianName:		nMaxLength = 150; break;
	case ifnPrimaryCarePhysicianName:	nMaxLength = 150; break;
	case ifnPatientCurrentStatus:		nMaxLength = 16; break;
	/* Not Needed: ifnFirstContactDate (Datetime)*/
	case ifnReferralSourceName:			nMaxLength = 50; break;
	case ifnEmergencyContactFirstName:	nMaxLength = 50; break;
	case ifnEmergencyContactLastName:	nMaxLength = 50; break;
	case ifnEmergencyContactRelation:	nMaxLength = 50; break;
	case ifnEmergencyContactHomePhone:	nMaxLength = 20; break;
	case ifnEmergencyContactWorkPhone:	nMaxLength = 20; break;
	case ifnWarningMessage:				nMaxLength = 1024; break;
	case ifnNote:						nMaxLength = INT_MAX; break; //~2GB
	case ifnGen1Custom1:				nMaxLength = 255; break;
	case ifnGen1Custom2:				nMaxLength = 255; break;
	case ifnGen1Custom3:				nMaxLength = 255; break;
	case ifnGen1Custom4:				nMaxLength = 255; break;
	case ifnCustomText1:				nMaxLength = 255; break;
	case ifnCustomText2:				nMaxLength = 255; break;
	case ifnCustomText3:				nMaxLength = 255; break;
	case ifnCustomText4:				nMaxLength = 255; break;
	case ifnCustomText5:				nMaxLength = 255; break;
	case ifnCustomText6:				nMaxLength = 255; break;
	case ifnCustomText7:				nMaxLength = 255; break;
	case ifnCustomText8:				nMaxLength = 255; break;
	case ifnCustomText9:				nMaxLength = 255; break;
	case ifnCustomText10:				nMaxLength = 255; break;
	case ifnCustomText11:				nMaxLength = 255; break;
	case ifnCustomText12:				nMaxLength = 255; break;
	case ifnCustomNote:					nMaxLength = 255; break;
	// (r.farnworth 2015-07-06 10:51) - PLID 65525 - Increased MaxLength to 5
	case ifnCustomCheckbox1:			nMaxLength = 5; break; //We allow up to 'False'
	case ifnCustomCheckbox2:			nMaxLength = 5; break; //We allow up to 'False'
	case ifnCustomCheckbox3:			nMaxLength = 5; break; //We allow up to 'False'
	case ifnCustomCheckbox4:			nMaxLength = 5; break; //We allow up to 'False'
	case ifnCustomCheckbox5:			nMaxLength = 5; break; //We allow up to 'False'
	case ifnCustomCheckbox6:			nMaxLength = 5; break; //We allow up to 'False'
	/* Not Needed: ifnCustomDate1, ifnCustomDate2, ifnCustomDate3, ifnCustomDate4 (DateTime) */
	case ifnMaritalStatus:				nMaxLength = 7; break;
	// (b.savon 2015-03-31 09:57) - PLID 65233 - Create fields for the Recalls object for the import utility.
	case ifnRecallTemplateName:			nMaxLength = 50; break;
	/* Not Needed: ifnRecallDate (DateTime) */
	// (r.farnworth 2015-04-01 14:16) - PLID 65246
	case ifnCustomPatientID:			nMaxLength = 255; break;
	// (r.farnworth 2015-04-02 10:11) - PLID 65167 - Create fields for the insurance companies object for the import utility
	case ifnConversionID:				nMaxLength = 50; break;
	case ifnInsCoName:					nMaxLength = 255; break;
	case ifnContactFirst:				nMaxLength = 50; break;
	case ifnContactLast:				nMaxLength = 50; break;
	case ifnContactTitle:				nMaxLength = 50; break;
	case ifnContactPhone:				nMaxLength = 20; break;
	case ifnContactFax:					nMaxLength = 20; break;
	case ifnContactNote:				nMaxLength = INT_MAX; break; //~2GB
	case ifnHCFAPayerID:				nMaxLength = 50; break;
	case ifnUBPayerID:					nMaxLength = 50; break;
	case ifnEligibilityPayerID:			nMaxLength = 50; break;
	//(s.dhole 4/8/2015 1:28 PM ) - PLID 65224  Fields we required for Patient notes
	case ifnPatientNoteCategory:		nMaxLength = 50; break;
	case ifnPatientNoteText:            nMaxLength = INT_MAX; break;
		//no need to define ifnPatientNoteDateTime
	case ifnPatientNotePriority:		nMaxLength = 20; break;

	// (b.savon 2015-04-06 09:15) - PLID 65216 - Create fields for the Appointment object for the import utility.
	/* Not Needed: ifnAppointmentDate, ifnAppointmentStartTime, ifnAppointmentEndTime (Datetime)*/
		// (r.farnworth 2015-07-06 10:51) - PLID 65525 - Increased length to 5
	case ifnAppointmentIsEvent:			nMaxLength = 5; break;
	case ifnAppointmentDuration:		nMaxLength = 4; break; //Can't be more than 4 digits
	case ifnAppointmentType:			nMaxLength = 50; break;
	case ifnAppointmentPurpose:			nMaxLength = 100; break;
	case ifnAppointmentNotes:			nMaxLength = 3000; break;
	// (r.farnworth 2015-07-06 10:51) - PLID 65525 - Increased MaxLength to 5
	case ifnAppointmentIsConfirmed:		nMaxLength = 5; break;
	case ifnAppointmentIsCancelled:		nMaxLength = 5; break;
	case ifnAppointmentIsNoShow:		nMaxLength = 5; break;
	//(s.dhole 4/14/2015 9:52 AM ) - PLID 65191 Added Insured Party fields
	case ifnInsuranceCompanyConversionID:	nMaxLength = 50; break;
	case ifnInsuranceCompanyName:		nMaxLength = 255; break;
	case ifnInsuredEmployer:		nMaxLength = 150; break;
	case ifnInsuredInsuranceID:		nMaxLength = 50; break; 
	case ifnInsuredGroupNo:		nMaxLength = 100; break; //(s.dhole 5/6/2015 10:55 AM ) - PLID 65755
	case ifnInsuredCopay:		nMaxLength = 20; break;
	case ifnInsuredCopayPercent:		nMaxLength = 5; break;
	case ifnInsuredPartyRelation:			nMaxLength = 120; break;
	case ifnInsuredPartyRespTypeID: 	nMaxLength = 10; break;
	// (r.goldschmidt 2016-02-10 09:31) - PLID 68163 - added race fields
	case ifnRaceCDCCode:			nMaxLength = 255; break;
	case ifnRacePreferredName:		nMaxLength = 255; break;
	default:
		ASSERT(FALSE);
		//We didn't call this for a string field, or it isn't in this list
		break;
	case ifnIgnore:
		//This should never happen!
		nMaxLength = -1; break;
	}
	return nMaxLength;
}


CString FormatPhoneForSql(CString strPhone)
{
	// (j.dinatale 2011-01-18) - PLID 39609 - Clean up the string to be only digits
	CString strClean;
	for (int i = 0; i < strPhone.GetLength(); i++) {
		char c = strPhone[i];
		if (c >= '0' && c <= '9') {
			strClean += c;
		}
	}

	// (j.dinatale 2011-01-18) - PLID 39609 - if the cleaned up string is 10 digits long, then go ahead and format it like a US/Canadian number
	if(strClean.GetLength() == 10){
		strPhone = FormatPhone(strClean, "(###) ###-####");
		// (j.dinatale 2011-01-18) - PLID 39609 - since we now check for 10 digits, no need to replace #'s because we know its 10 long
		//strPhone.Replace("(###) ###-####", "");
		//strPhone.Replace("#", "");
	}

	return strPhone;
}

//(e.lally 2007-06-12) PLID 26273
//(e.lally 2007-06-12) PLID 26275 - Format valid boolean string values into standard strings for the interface
	//Given a new generic function name
// (r.farnworth 2015-04-29 10:32) - PLID 65525 - Ensure boolean values are validated before the preview screen silently changes them to false if they are invalid
CString GetBooleanYesNoTextFromString(CString strInput)
{
	if( strInput.CompareNoCase("T")==0 || 
		strInput.CompareNoCase("TRUE") == 0 ||
		strInput.CompareNoCase("Y") == 0 ||
		strInput.CompareNoCase("YES") == 0 ||
		strInput == "1"){
		return "Yes";
	}
	else if(strInput.CompareNoCase("F") == 0 ||
		strInput.CompareNoCase("FALSE") == 0 ||
		strInput.CompareNoCase("N") == 0 ||
		strInput.CompareNoCase("NO") == 0 ||
		strInput == "0"){
		return "No"; //unset
	}
	else {
		// (r.farnworth 2015-04-29 11:26) - PLID 65525 - DEVELOPERS: We should never hit this point. If we do, something failed in the validation.
		// Make sure the value is being properly validated in the ValidateBooleanValue function in ImportValidationUtils.cpp
		ASSERT(FALSE);
	}
	return "No";
}

//(e.lally 2007-06-12) PLID 26273
//(e.lally 2007-06-12) PLID 26275 - Get SQL values for boolean strings; same strings allowed as above
	//Given a new generic function name
// (r.farnworth 2015-04-29 10:32) - PLID 65525 - Ensure boolean values are validated before the preview screen silently changes them to false if they are invalid
long GetSqlValueFromBoolString(CString strInput)
{
	if( strInput.CompareNoCase("T")==0 || 
		strInput.CompareNoCase("TRUE") == 0 ||
		strInput.CompareNoCase("Y") == 0 ||
		strInput.CompareNoCase("YES") == 0 ||
		strInput == "1"){
		return 1;
	}
	else if ( strInput.CompareNoCase("F") == 0 ||
		strInput.CompareNoCase("FALSE") == 0 ||
		strInput.CompareNoCase("N") == 0 ||
		strInput.CompareNoCase("NO") == 0 ||
		strInput == "0"){
		return 0; //unset
	}
	else {
		// (r.farnworth 2015-04-29 11:26) - PLID 65525 - DEVELOPERS: We should never hit this point. If we do, something failed in the validation.
		// Make sure the value is being properly validated in the ValidateBooleanValue function in ImportValidationUtils.cpp
		ASSERT(FALSE);
	}
	return 0;
}

// (r.farnworth 2015-03-17 15:36) - PLID 65197
BOOL DoesColumnDataExist(NXDATALIST2Lib::IRowSettingsPtr pRow, const CString &strData, const short nCol) {
	while (pRow != NULL){
		CWaitCursor pWait;
		if (VarString(pRow->GetValue(nCol), "<None>") == strData)
			return TRUE;
		pRow = pRow->GetNextRow();
	}
	return FALSE;
}

// (b.savon 2015-03-24 11:15) - PLID 65250 - Migrate the CapsFix algorithm from DCS tools
void FixCaps(CString &strValue)
{
	strValue.TrimLeft();
	strValue.TrimRight();
	strValue.TrimRight(",");

	bool bAlreadyLower = false;
	for (int c = 0; c < strValue.GetLength() && !bAlreadyLower; c++) {
		if (::isalpha(strValue[c]) && ::islower(strValue[c])) {
			bAlreadyLower = true;
		}
	}

	if (!bAlreadyLower && !strValue.IsEmpty()) {
		bool bNextCapped = true;
		for (int i = 0; i < strValue.GetLength(); i++) {
			if (::isalpha(strValue[i])) {
				if (bNextCapped) {
					strValue.SetAt(i, ::toupper(strValue[i]));
				}
				else {
					strValue.SetAt(i, ::tolower(strValue[i]));
				}
			}

			if (strValue[i] == '\'') { // O'Malley
				bNextCapped = true;
			}
			else if (i > 0 && strValue[i - 1] == 'M' && strValue[i] == 'c') { //McCabe
				bNextCapped = true;
			}
			else if (strValue[i] == ' ' || strValue[i] == '.') { // Mc Cabe, M.D.
				bNextCapped = true;
			}
			else if (!::isalnum(strValue[i])) {
				bNextCapped = true;
			}
			else {
				bNextCapped = false;
			}
		}

		if (strValue.GetLength() > 3) {
			if (strValue.Right(2) == "Md"){
				strValue.Replace("Md", "MD");
			}
			else if (strValue.Right(2) == "Rn"){
				strValue.Replace("Rn", "RN");
			}
			else if (strValue.Right(2) == "Np"){
				strValue.Replace("Np", "NP");
			}
			else if (strValue.Right(2) == "Ph"){
				strValue.Replace("Ph", "PH");
			}
			else if (strValue.Right(2) == "Pc"){
				strValue.Replace("Pc", "PC");
			}
			else if (strValue.Right(2) == "Ii"){
				strValue.Replace("Ii", "II");
			}
			else if (strValue.Right(2) == "Iv"){
				strValue.Replace("Iv", "IV");
			}
			else if (strValue.Right(2) == "Vi"){
				strValue.Replace("Vi", "VI");
			}
		}

		if (strValue.GetLength() > 4 && strValue.Right(3) == "Llp") {
			strValue.Replace("Llp", "LLP");
		}
		if (strValue.GetLength() > 4 && strValue.Right(3) == "Llc") {
			strValue.Replace("Llc", "LLC");
		}
		if (strValue.GetLength() > 4 && strValue.Right(3) == "Phd") {
			strValue.Replace("Phd", "PhD");
		}
		if (strValue.GetLength() > 4 && strValue.Right(3) == "Dmd") {
			strValue.Replace("Dmd", "DMD");
		}
		if (strValue.GetLength() > 4 && strValue.Right(3) == "Dvm") {
			strValue.Replace("Dvm", "DVM");
		}
		if (strValue.GetLength() > 4 && strValue.Right(3) == "Dpm") {
			strValue.Replace("Dpm", "DPM");
		}
		if (strValue.GetLength() > 4 && strValue.Right(3) == "Dds") {
			strValue.Replace("Dds", "DDS");
		}
		if (strValue.GetLength() > 4 && strValue.Right(3) == "Iii") {
			strValue.Replace("Iii", "III");
		}
		if (strValue.GetLength() > 4 && strValue.Right(3) == "Vii") {
			strValue.Replace("Vii", "VII");
		}
	}
}

// (b.savon 2015-03-24 12:37) - PLID 65251 - Create a utility function that returns if the supplied field is eligible for caps fix algorithm
bool IsEligibleCapsFixField(const CString &strField)
{
	return
	(
		strField == GetImportFieldDataField(ifnFirstName) ||
		strField == GetImportFieldDataField(ifnMiddleName) ||
		strField == GetImportFieldDataField(ifnLastName) ||
		strField == GetImportFieldDataField(ifnAddress1) ||
		strField == GetImportFieldDataField(ifnAddress2) ||
		strField == GetImportFieldDataField(ifnCity) ||
		strField == GetImportFieldDataField(ifnCompany) ||
		strField == GetImportFieldDataField(ifnServiceName) ||
		strField == GetImportFieldDataField(ifnResourceName) ||
		strField == GetImportFieldDataField(ifnProductName) ||
		strField == GetImportFieldDataField(ifnEmergencyContactFirstName) ||
		strField == GetImportFieldDataField(ifnEmergencyContactLastName) ||
		strField == GetImportFieldDataField(ifnEmergencyContactRelation) 
	);
}
//(s.dhole 4/7/2015 8:36 AM ) - PLID 65229 Get note Prority
long GetNotePriorityFromString(const CString &strInput)
{
	if (strInput.CompareNoCase("Low") == 0 ||
		strInput.CompareNoCase("3") == 0 ){
		return 3;
	}
	else if (strInput.CompareNoCase("Medium") == 0 ||
		strInput == "2"){
		return 2;
	}
	else if (strInput.CompareNoCase("High") == 0 ||
		strInput == "1"){
		return 1;
	}
	else{
		return 0; //unset
	}
}
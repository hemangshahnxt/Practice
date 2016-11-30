#include "stdafx.h"
#include "PrescriptionUtilsAPI.h"
#include "MonographDlg.h" // (b.savon 2013-06-19 17:13) - PLID 56880
#include "LeafletDlg.h"
#include "FirstDataBankUtils.h"

using namespace ADODB;

// (j.jones 2012-11-19 16:59) - PLID 52818 - added PrescriptionUtils
// (j.jones 2013-03-27 16:43) - PLID 55920 - Renamed to PrescriptionUtilsAPI, this
// should only define functions that depend on the API / Accessor.
// I then moved many enums & functions to PrescriptionUtilsNonAPI.h
// so that any code that needs enums or other functions without also needing API accessor code
// will not need to also include NxAPI.h.

// (b.savon 2013-03-08 13:12) - PLID 55518 - Changed everywhere to use EMNSpawnSource, QueuePrescriptionPrescriberUserPtr, and new return types for Location and UserID

// (j.jones 2012-11-19 17:03) - PLID 52818 - Created SaveNewPrescription as
// one single modular function used to make new entries in PatientMedications.
// Its results object can optionally return the current queue, with filters applied.
// Returns the new prescription object.
// (j.fouts 2013-02-04 11:58) - PLID 53954 - Modfied this for use with GetQueueFilters()
// (a.wilson 2013-02-07 15:54) - PLID 55014 - modified this to handle denynewrx prescriptions.
// (s.dhole 2013-10-18 13:14) - PLID 59068 Added formulary insurance id
NexTech_Accessor::_UpdatePresQueueResultsPtr SaveNewPrescription(long nMedicationID, BOOL bRequeryQueue /*=FALSE*/,
																 Nx::SafeArray<IUnknown*> saryFilters /*=NULL*/,
																 long nPatientID /*= -1*/, long nProviderID /*= -1*/, long nLocationID /*= GetCurrentLocation()*/,
																 COleDateTime dtDate /*= COleDateTime::GetCurrentTime()*/,
																 long nEMNID /*= -1*/, BOOL bIsEMRTemplate /*= FALSE*/,
																 SourceActionInfo saiSpawnedFromAction /*= SourceActionInfo()*/,
																 long nPharmacyID /* = -1 */, long nDenyNewRxResponseID /* = -1 */,
																 long nFInsuranceDetailID /* = -1 */)
{
	//verify our parameters
	if(nMedicationID < 0) {
		ThrowNxException("CreateNewPrescription called with %li medication ID.", nMedicationID);
	}

	if(nLocationID < 0) {
		ThrowNxException("CreateNewPrescription called with %li location ID.", nLocationID);
	}

	if(dtDate.GetStatus() == COleDateTime::invalid) {
		ThrowNxException("CreateNewPrescription called with an invalid date.");
	}

	//nPatientID is required unless we're on an EMR template
	if(!bIsEMRTemplate && nPatientID == -1) {
		ThrowNxException("CreateNewPrescription called with %li patient ID.", nPatientID);
	}

	// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
	CArray<NexTech_Accessor::_QueuePrescriptionPtr,NexTech_Accessor::_QueuePrescriptionPtr> aryPrescriptions;
	NexTech_Accessor::_QueuePrescriptionPtr pPrescription(__uuidof(NexTech_Accessor::QueuePrescription));
	NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pExpects(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));
	NexTech_Accessor::_NexERxDrugPtr pMedications(__uuidof(NexTech_Accessor::NexERxDrug));

	pExpects->Action = NexTech_Accessor::UpdatePresQueueAction_Add;
	pExpects->RequeryQueue = bRequeryQueue;

	pExpects->filters = saryFilters;

	CString strMedicationID;
	strMedicationID.Format("%li", nMedicationID);
	pPrescription->Medication = pMedications;
	pPrescription->Medication->DrugListID = _bstr_t(strMedicationID);
	// (s.dhole 2013-10-18 13:12) - PLID 59068
	if (nFInsuranceDetailID>0){
		pPrescription->InsuranceDetailID =bstr_t(AsString(nFInsuranceDetailID)) ;
	}
	//if provided a default provider, use it
	NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pProvider(__uuidof(NexTech_Accessor::QueuePrescriptionPrescriberUser));
	if(nProviderID > 0){ 
		pProvider->personID = FormatBstr("%li", nProviderID);
	}else{
		pProvider->personID = _bstr_t("");
	}
	pPrescription->Provider = pProvider;

	// (j.fouts 2013-04-22 14:13) - PLID 54719 - PharmacyID is passed in a Pharmacy object
	NexTech_Accessor::_ERxQueuePharmacyPtr pPharmacy(__uuidof(NexTech_Accessor::ERxQueuePharmacy));
	pPrescription->Pharmacy = pPharmacy;

	//default pharmacy
	if (nPharmacyID > 0)
		pPrescription->Pharmacy->PharmacyID = FormatBstr("%li", nPharmacyID);
	//this prescription is from a deny and rewrite if > 0
	if (nDenyNewRxResponseID > 0)
		pPrescription->DenialResponseID = FormatBstr("%li", nDenyNewRxResponseID);

	NexTech_Accessor::_NullableDateTimePtr pNullableDate(__uuidof(NexTech_Accessor::NullableDateTime));
	pPrescription->PrescriptionDate = pNullableDate;
	pPrescription->PrescriptionDate->SetDateTime(dtDate);

	// (j.jones 2012-11-21 09:40) - PLID 53818 - pass in our template status
	NexTech_Accessor::_EMNSpawnSourcePtr pEMRPrescriptionSource(__uuidof(NexTech_Accessor::EMNSpawnSource));
	if(bIsEMRTemplate) {
		pEMRPrescriptionSource->IsEMRTemplate = true;

		//make sure we have an EMNID (which should be the template ID)
		if(nEMNID == -1) {
			ThrowNxException("SaveNewPrescription called on a template without a template ID provided.");
		}

		//Require template access. Unlike patient EMNs, having access is not optional.
		// (j.armen 2013-05-14 12:41) - PLID 56683 - EMN Template Access Refactoring
		if(!ReturnsRecordsParam(
			"SELECT EMNID\r\n"
			"FROM EMNTemplateAccessT\r\n"
			"WHERE UserLoginTokenID = {INT} AND EMNID = {INT}", GetAPIUserLoginTokenID(), nEMNID)) {
			//this function should never have been called on a template if there was no access
			ThrowNxException("SaveNewPrescription called on a template without having user access to the template.");
		}
	}
	else {
		pEMRPrescriptionSource->IsEMRTemplate = false;
	}
	
	// (j.jones 2012-11-14 11:20) - PLID 52819 - if provided with an EMN ID, use it, but only if
	// we have writeable access to it
	NexTech_Accessor::_NullableIntPtr pNullableEMNID(__uuidof(NexTech_Accessor::NullableInt));
	pEMRPrescriptionSource->emnID = pNullableEMNID;
	pEMRPrescriptionSource->emnID->SetNull();

	//initialize the spawning data to null
	NexTech_Accessor::_NullableIntPtr pNullableSourceActionID(__uuidof(NexTech_Accessor::NullableInt));
	pEMRPrescriptionSource->SourceActionID = pNullableSourceActionID;
	pEMRPrescriptionSource->SourceActionID->SetNull();
	NexTech_Accessor::_NullableIntPtr pNullableSourceDataGroupID(__uuidof(NexTech_Accessor::NullableInt));
	pEMRPrescriptionSource->SourceDataGroupID = pNullableSourceDataGroupID;
	pEMRPrescriptionSource->SourceDataGroupID->SetNull();
	NexTech_Accessor::_NullableIntPtr pNullableSourceDetailID(__uuidof(NexTech_Accessor::NullableInt));
	pEMRPrescriptionSource->SourceDetailID = pNullableSourceDetailID;
	pEMRPrescriptionSource->SourceDetailID->SetNull();
	NexTech_Accessor::_NullableIntPtr pNullableSourceDetailImageStampID(__uuidof(NexTech_Accessor::NullableInt));
	pEMRPrescriptionSource->SourceDetailImageStampID = pNullableSourceDetailImageStampID;
	pEMRPrescriptionSource->SourceDetailImageStampID->SetNull();

	if(nEMNID != -1) {
		long nStatus = -1;
		BOOL bHasAccess = FALSE;
		//For patient EMNs, if we don't have writeable access, we won't say anything,
		//we will just add the prescription normally and not link it to the EMN.
		//For templates, we would have already thrown an exception if they did not have access, as it is required.
		if(bIsEMRTemplate || CanEditEMN(nEMNID, nStatus, bHasAccess)) {
			//we do have access, so add this new prescription to the EMN			
			pEMRPrescriptionSource->emnID->SetInt(nEMNID);
		}
		//if we have any spawning actions, fill the variables
		if(saiSpawnedFromAction.nSourceActionID != -1) {
			pEMRPrescriptionSource->SourceActionID->SetInt(saiSpawnedFromAction.nSourceActionID);
		}
		if(saiSpawnedFromAction.GetDataGroupID() != -1) {
			pEMRPrescriptionSource->SourceDataGroupID->SetInt(saiSpawnedFromAction.GetDataGroupID());
		}
		if(saiSpawnedFromAction.GetSourceDetailID() != -1) {
			pEMRPrescriptionSource->SourceDetailID->SetInt(saiSpawnedFromAction.GetSourceDetailID());
		}
		if(saiSpawnedFromAction.GetDetailStampID() != -1) {
			pEMRPrescriptionSource->SourceDetailImageStampID->SetInt(saiSpawnedFromAction.GetDetailStampID());
		}
	}
	pPrescription->EMRPrescriptionSource = pEMRPrescriptionSource;

	aryPrescriptions.Add(pPrescription);
	Nx::SafeArray<IUnknown*> saryPrescriptions = Nx::SafeArray<IUnknown*>::From(aryPrescriptions);
	// (b.savon 2013-03-12 13:11) - PLID 55518 - Use new object structure
	pExpects->PrescriptionsToAdd = saryPrescriptions;

	// (j.jones 2013-11-25 10:21) - PLID 59772 - we do want drug interactions when adding new prescriptions
	// (b.savon 2014-01-03 08:18) - PLID 58984 - But only if we have the NexERx LIcense
	if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts){
		pExpects->DrugDrugInteracts = VARIANT_TRUE;
		// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
		pExpects->ExcludeMonographInformation = VARIANT_TRUE;
		pExpects->DrugAllergyInteracts = VARIANT_TRUE;
		pExpects->DrugDiagnosisInteracts = VARIANT_TRUE;
	}

	// (j.fouts 2013-04-24 16:55) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter
	// (s.dhole 2012-12-05 17:21) - PLID 54067 Dont Pass Surescripts licence
	NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), GetAPILoginToken(), _bstr_t(FormatString("%li", nPatientID)),
		pExpects);

	return pResults;
}

// (j.fouts 2012-11-20 11:56) - PLID 52906 - This maps the ID of the PrescriptionQueueStatusT to the enum value used by the
//		accessor. Because SOAP serializes the enum as a string it loses the associated value, so there is a mismatch between
//		the values in data and the values of the enum in the accessor. This function is used to rectify the mismatch.
NexTech_Accessor::PrescriptionStatus MapQueueStatusToAccessor(long nStatus)
{
	switch(nStatus)
	{
		case pqsIncomplete:
			return NexTech_Accessor::PrescriptionStatus_Incomplete;
		case pqsPrinted:
			return NexTech_Accessor::PrescriptionStatus_Printed;
		case pqsOnHold:
		// (j.fouts 2013-01-28 16:34) - PLID 53025 - Added on hold status
			return NexTech_Accessor::PrescriptionStatus_OnHold;
		case pqseTransmitAuthorized:
			return NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized;
		case pqseTransmitSuccess:
			return NexTech_Accessor::PrescriptionStatus_eTransmitSuccess;
		case pqseTransmitError:
			return NexTech_Accessor::PrescriptionStatus_eTransmitError;
		case pqseTransmitPending:
			return NexTech_Accessor::PrescriptionStatus_eTransmitPending;
		// (j.fouts 2013-01-15 10:02) - PLID 55800 - Added Legacy Status
		case pqsLegacy:
			return NexTech_Accessor::PrescriptionStatus_Legacy;
		case pqseFaxed:
			return NexTech_Accessor::PrescriptionStatus_eFaxed;
		// (b.savon 2013-09-04 10:04) - PLID 58212 - Add a 'Void' and 'Ready for Doctor Review' type for Prescriptions
		case pqseVoid:
			return NexTech_Accessor::PrescriptionStatus_Void;
		case pqseReadyForDoctorReview:
			return NexTech_Accessor::PrescriptionStatus_ReadyForDoctorReview;
		// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
		case pqseDispensedInHouse:
			return NexTech_Accessor::PrescriptionStatus_DispensedInHouse;
		default:
			return NexTech_Accessor::PrescriptionStatus_NoStatus;
	}
}

// (j.jones 2013-01-23 16:30) - PLID 53259 - added MapAccessorToQueueStatus,
// takes in a NexTech_Accessor::PrescriptionStatus value and returns a Practice PrescriptionQueueStatus enum
PrescriptionQueueStatus MapAccessorToQueueStatus(NexTech_Accessor::PrescriptionStatus eStatus)
{
	switch(eStatus) {
		case NexTech_Accessor::PrescriptionStatus_Incomplete:
			return pqsIncomplete;
		case NexTech_Accessor::PrescriptionStatus_Printed:
			return pqsPrinted;
		// (j.fouts 2013-01-28 16:34) - PLID 53025 - Added on hold status
		case NexTech_Accessor::PrescriptionStatus_OnHold:
			return pqsOnHold;
		case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
			return pqseTransmitAuthorized;
		case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
			return pqseTransmitSuccess;
		case NexTech_Accessor::PrescriptionStatus_eTransmitError:
			return pqseTransmitError;
		case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
			return pqseTransmitPending;
		case NexTech_Accessor::PrescriptionStatus_Legacy:
			return pqsLegacy;
		case NexTech_Accessor::PrescriptionStatus_eFaxed:
			return pqseFaxed;
		// (b.savon 2013-09-04 10:04) - PLID 58212 - Add a 'Void' and 'Ready for Doctor Review' type for Prescriptions
		case NexTech_Accessor::PrescriptionStatus_Void:
			return pqseVoid;
		case NexTech_Accessor::PrescriptionStatus_ReadyForDoctorReview:
			return pqseReadyForDoctorReview;
		// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
		case NexTech_Accessor::PrescriptionStatus_DispensedInHouse:
			return pqseDispensedInHouse;
		default:
			//this is not a valid status, check the caller and make sure
			//this function is being called correctly
			//(remove the assertion if we conclude this return value is permitted)
			ASSERT(FALSE);
			return pqsInvalid;
	}
};

// (r.gonet 02/28/2014) - PLID 60755 - Converts an FDBDiagnosisCodeSystem value to an FDBDiagnosisCodeSystem in the accessor namespace.
NexTech_Accessor::FDBDiagnosisCodeSystem MapFDBDiagnosisCodeSystemToAccessor(FDBDiagnosisCodeSystem eCodeSystem)
{
	switch(eCodeSystem) {
	case fdbcsInvalid:
		return NexTech_Accessor::FDBDiagnosisCodeSystem_Invalid;
	case fdbcsICD9:
		return NexTech_Accessor::FDBDiagnosisCodeSystem_ICD9;
	case fdbcsICD10:
		return NexTech_Accessor::FDBDiagnosisCodeSystem_ICD10;
	default:
		ASSERT(FALSE);
		return NexTech_Accessor::FDBDiagnosisCodeSystem_Invalid;
	}
}

// (r.gonet 02/28/2014) - PLID 60755 - Converts an FDBDiagnosisCodeSystem value in the accessor namespace to an FDBDiagnosisCodeSystem. 
FDBDiagnosisCodeSystem MapAccessorToFDBDiagnosisCodeSystem(NexTech_Accessor::FDBDiagnosisCodeSystem eCodeSystem)
{
	switch(eCodeSystem) {
	case NexTech_Accessor::FDBDiagnosisCodeSystem_Invalid:
		return fdbcsInvalid;
	case NexTech_Accessor::FDBDiagnosisCodeSystem_ICD9:
		return fdbcsICD9;
	case NexTech_Accessor::FDBDiagnosisCodeSystem_ICD10:
		return fdbcsICD10;
	default:
		ASSERT(FALSE);
		return fdbcsInvalid;
	}
}

// (j.fouts 2012-11-27 16:48) - PLID 51889 - For now these map 1 to 1, but because we require both the API side
//		aswell as the practice side to access data where the enumeration is stored as integers,
//		I am mapping them to garuntee that they are equivilent incase they change on either end
//		or new enumerations are added to DrugType
//This will map a DrugType from data to its API equivalent
NexTech_Accessor::DrugType MapDrugTypeToAccessor(long nType)
{
	switch(nType)
	{
		case dgtNDC:
			return NexTech_Accessor::DrugType_NDC;
		case dgtSupply:
			return NexTech_Accessor::DrugType_Supply;
		case dgtCompound:
			return NexTech_Accessor::DrugType_Compound;
		default:
			return NexTech_Accessor::DrugType_NDC;
	}
}

// (j.fouts 2012-11-27 16:48) - PLID 51889 - For now these map 1 to 1, but because we require both the API side
//		aswell as the practice side to access data where the enumeration is stored as integers,
//		I am mapping them to garuntee that they are equivilent incase they change on either end
//		or new enumerations are added to DrugType
//This will map a DrugType from the API to the database equivalent
long MapDrugTypeToData(NexTech_Accessor::DrugType drugType)
{
	switch(drugType)
	{
		case NexTech_Accessor::DrugType_NDC:
			return 0;
		case NexTech_Accessor::DrugType_Supply:
			return 1;
		case NexTech_Accessor::DrugType_Compound:
			return 2;
		default:
			return 0;
	}
}

// (b.savon 2013-03-18 16:09) - PLID 55477
EUserRoleTypes AccessorERxUserRoleToPracticeEnum(NexTech_Accessor::ERxUserRole userRole)
{
	switch(userRole)
	{
		case NexTech_Accessor::ERxUserRole_LicensedPrescriber:
			return urtLicensedPrescriber;
		case NexTech_Accessor::ERxUserRole_MidlevelPrescriber:
			return urtMidlevelPrescriber;
		case NexTech_Accessor::ERxUserRole_NurseStaff:
			return urtNurseStaff;
		case NexTech_Accessor::ERxUserRole_None:
		default:
			return urtNone;
	}
}

// (j.fouts 2012-12-27 16:50) - PLID 53160 - A helper function to get the text for a PrescriptionQueueStatus based on the status
CString QueueStatusTextFromID(NexTech_Accessor::PrescriptionStatus status)
{
	switch(status)
	{
	case NexTech_Accessor::PrescriptionStatus_Incomplete:
		return "Incomplete";
	case NexTech_Accessor::PrescriptionStatus_Printed:
		return "Printed";
	// (j.fouts 2013-01-28 16:34) - PLID 53025 - Added on hold status	
	case NexTech_Accessor::PrescriptionStatus_OnHold:
		return "On Hold";
	case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
		return "e-Transmit Authorized";
	case NexTech_Accessor::PrescriptionStatus_eTransmitError:
		return "e-Transmitted Error";
	case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
		return "e-Transmitted Pending";
	case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
		return "e-Transmitted Success";
	// (j.fouts 2013-01-15 10:02) - PLID 55800 - Added Legacy Status
	case NexTech_Accessor::PrescriptionStatus_Legacy:
		return "PreNexERx Prescription";
	case NexTech_Accessor::PrescriptionStatus_eFaxed:
		return "eFaxed";
	// (b.savon 2013-09-04 15:21) - PLID 58212 - Add a new 'Void' type for Prescriptions
	case NexTech_Accessor::PrescriptionStatus_Void:
		return "Void";
	// (b.eyers 2016-02-03) - PLID 67982 - changed client facing name  
	case NexTech_Accessor::PrescriptionStatus_ReadyForDoctorReview:
		return "Ready for Review";
	// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
	case NexTech_Accessor::PrescriptionStatus_DispensedInHouse:
		return "Dispensed In-House";
	default:
		return "";
	}
}

// (b.savon 2013-03-19 17:08) - PLID 55477 - Returns struct now
// (j.fouts 2013-03-12 10:13) - PLID 52973 - Moved the loading and saving logic out from the PrescriptionEditDlg
//Loads a full prescription from the API
PrescriptionInfo LoadFullPrescription(long nPrescriptionID, bool bIsTemplate /*=false*/)
{
	CString strPrescriptionID;
	strPrescriptionID.Format("%li", nPrescriptionID);

	// (b.savon 2013-03-18 17:58) - PLID 55477
	NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = LoadFullPrescription(strPrescriptionID, bIsTemplate);

	if( pResults->PrescriptionsLoaded == NULL ){
		ThrowNxException("Unable to Load Prescription!");
	}

	Nx::SafeArray<IUnknown*> saryPrescriptionsLoaded(pResults->PrescriptionsLoaded);
	if(saryPrescriptionsLoaded.GetCount() == 0)
	{
		//Prescription was not found
		ThrowNxException("Unable to Load Prescription!");
	}

	PrescriptionInfo rxInformation;
	NexTech_Accessor::_QueuePrescriptionPtr pPrescription = saryPrescriptionsLoaded[0];
	rxInformation.pPrescription = pPrescription;
	rxInformation.erxUserRole = pResults->UserRole;
	rxInformation.saryPrescribers = pResults->Prescriber;
	rxInformation.sarySupervisors = pResults->Supervisor;
	rxInformation.saryNurseStaff = pResults->NurseStaff;

	return rxInformation; 
}

// (b.savon 2013-03-19 17:08) - PLID 55477 - Returns Results now
//Loads a full prescription from the API
NexTech_Accessor::_UpdatePresQueueResultsPtr LoadFullPrescription(CString &strPrescriptionID, bool bIsTemplate /*=false*/)
{
	NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pExpects(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));

	pExpects->Action = NexTech_Accessor::UpdatePresQueueAction_Load;

	NexTech_Accessor::_QueuePrescriptionPtr pPrescription(__uuidof(NexTech_Accessor::QueuePrescription));
	pPrescription->PrescriptionID = _bstr_t(strPrescriptionID);

	pPrescription->EMRPrescriptionSource = NexTech_Accessor::_EMNSpawnSourcePtr(__uuidof(NexTech_Accessor::EMNSpawnSource));
	pPrescription->EMRPrescriptionSource->IsEMRTemplate = (VARIANT_BOOL)bIsTemplate;

	CArray<NexTech_Accessor::_QueuePrescriptionPtr,NexTech_Accessor::_QueuePrescriptionPtr> aryPrescriptions;
	aryPrescriptions.Add(pPrescription);
	Nx::SafeArray<IUnknown *> saryPrescriptions = Nx::SafeArray<IUnknown *>::From(aryPrescriptions);
	pExpects->PrescriptionsToLoad = saryPrescriptions;

	// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
	pExpects->ExcludeMonographInformation = VARIANT_TRUE;

	// (j.fouts 2013-04-24 16:55) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter
	NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), GetAPILoginToken(), 
		_bstr_t(""), pExpects);

	return pResults;
}

// (j.jones 2013-05-10 15:21) - PLID 55955 - given a drug interaction severity enum, return a struct
// with the "short" name we display, the priority (for sorting purposes), and the color we would use,
// so that both the drug interaction dialog and the severity filter configuration use the same information
// (j.fouts 2013-05-20 10:17) - PLID 56571 - The API now calculates the severity level
DrugInteractionDisplayFields GetDrugAllergyInteractionSeverityInfo(NexTech_Accessor::FDBAllergyInteractionSource eSource, NexTech_Accessor::DrugAllergySeverityLevel eSeverity)
{
	DrugInteractionDisplayFields eResult;
	//default the result to unknown & top priority, in the event that we failed to match a source
	eResult.strSeverityName = "Unknown Source";
	eResult.ePriority = soTopPriority;
	eResult.eColor = scHigh;

	// (j.fouts 2013-05-17 14:41) - PLID 56571 - We are just using these values for display names now there
	// is a separate enum to track severity
	switch(eSource)
	{
	case NexTech_Accessor::FDBAllergyInteractionSource_InactiveIngredient:
		//Past 10601 the API should never be returning this value
		ASSERT(FALSE);
		eResult.strSeverityName = "Inactive";
		break;
	case NexTech_Accessor::FDBAllergyInteractionSource_RelatedIngredient:
		eResult.strSeverityName = "Related";
		break;
	case NexTech_Accessor::FDBAllergyInteractionSource_DirectIngredient:
		eResult.strSeverityName = "Direct";
		break;
	case NexTech_Accessor::FDBAllergyInteractionSource_GroupIngredient:
		//Group has two severity levels, so we allow filtering on both
		if(eSeverity == NexTech_Accessor::DrugAllergySeverityLevel_Severe)
		{
			eResult.strSeverityName = "Group (Severe)";
		}
		else
		{
			eResult.strSeverityName = "Group (Moderate)";
		}
		break;
	case NexTech_Accessor::FDBAllergyInteractionSource_CrossSensitiveIngredient:
		eResult.strSeverityName = "Cross Sensitive";
		break;
	default:
		//Hitting this assertion means that a new enum was added to the accessor
		//but not supported in Practice!
		//Don't throw an exception, just use the defaults we filled earlier to
		//admit we don't know the severity level, assume it is top priority.
		ASSERT(FALSE);
		break;
	}

	// (j.fouts 2013-05-17 14:41) - PLID 56571 - Added an new enum to track severity
	switch(eSeverity)
	{
		case NexTech_Accessor::DrugAllergySeverityLevel_Low:
			eResult.ePriority = soLowPriority;
			eResult.eColor = scLow;
			break;
		case NexTech_Accessor::DrugAllergySeverityLevel_Moderate:
			eResult.ePriority = soMidPriority;
			eResult.eColor = scMid;
			break;
		case NexTech_Accessor::DrugAllergySeverityLevel_Severe:
			eResult.ePriority = soTopPriority;
			eResult.eColor = scHigh;
			break;
		default:
			//Hitting this assertion means that a new enum was added to the accessor
			//but not supported in Practice!
			//Don't throw an exception, just use the defaults we filled earlier to
			//admit we don't know the severity level, assume it is top priority.
			ASSERT(FALSE);
			break;
	}

	return eResult;
}

// (j.jones 2013-05-10 15:21) - PLID 55955 - given a drug interaction severity enum, return a struct
// with the "short" name we display, the priority (for sorting purposes), and the color we would use,
// so that both the drug interaction dialog and the severity filter configuration use the same information
DrugInteractionDisplayFields GetDrugDrugInteractionSeverityInfo(NexTech_Accessor::FDBDrugInteractionSeverity eSeverity)
{
	DrugInteractionDisplayFields eResult;
	//default the result to unknown & top priority, in the event that we failed to match a severity
	eResult.strSeverityName = "Unknown Severity";
	eResult.ePriority = soTopPriority;
	eResult.eColor = scHigh;

	switch (eSeverity) {
		case NexTech_Accessor::FDBDrugInteractionSeverity_ContraindicatedDrugCombination:
			eResult.strSeverityName = "Contraindicated";
			eResult.ePriority = soTopPriority;
			eResult.eColor = scHigh;
			break;
		case NexTech_Accessor::FDBDrugInteractionSeverity_SevereInteraction:
			eResult.strSeverityName = "Severe";
			eResult.ePriority = soTopPriority;
			eResult.eColor = scHigh;
			break;
		case NexTech_Accessor::FDBDrugInteractionSeverity_ModerateInteraction:
			eResult.strSeverityName = "Moderate";
			eResult.ePriority = soMidPriority;
			eResult.eColor = scMid;
			break;
		case NexTech_Accessor::FDBDrugInteractionSeverity_UndeterminedSeverity:
			eResult.strSeverityName = "Undetermined";
			eResult.ePriority = soLowPriority;
			eResult.eColor = scLow;
			break;
		default:
			//Hitting this assertion means that a new enum was added to the accessor
			//but not supported in Practice!
			//Don't throw an exception, just use the defaults we filled earlier to
			//admit we don't know the severity level, assume it is top priority.
			ASSERT(FALSE);
			break;
	}

	return eResult;
}

// (j.jones 2013-05-10 15:21) - PLID 55955 - given a drug interaction severity enum, return a struct
// with the "short" name we display, the priority (for sorting purposes), and the color we would use,
// so that both the drug interaction dialog and the severity filter configuration use the same information
DrugInteractionDisplayFields GetDrugDiagnosisInteractionSeverityInfo(NexTech_Accessor::FDBDiagnosisInteractionSeverity eSeverity)
{
	DrugInteractionDisplayFields eResult;
	//default the result to unknown & top priority, in the event that we failed to match a severity
	eResult.strSeverityName = "Unknown Severity";
	eResult.ePriority = soTopPriority;
	eResult.eColor = scHigh;

	switch (eSeverity) {
		// (r.gonet 02/28/2014) - PLID 60755 - Fixed spelling from Contradiction to Contraindication
		case NexTech_Accessor::FDBDiagnosisInteractionSeverity_AbsoluteContraindication:
			eResult.strSeverityName = "Absolute Contraindication";
			eResult.ePriority = soTopPriority;
			eResult.eColor = scHigh;
			break;
		case NexTech_Accessor::FDBDiagnosisInteractionSeverity_RelativeContraindication:
			eResult.strSeverityName = "Relative Contraindication";
			eResult.ePriority = soMidPriority;
			eResult.eColor = scMid;
			break;
		case NexTech_Accessor::FDBDiagnosisInteractionSeverity_ContraindicationWarning:
			eResult.strSeverityName = "Contraindication Warning";
			eResult.ePriority = soLowPriority;
			eResult.eColor = scLow;
			break;
		default:
			//Hitting this assertion means that a new enum was added to the accessor
			//but not supported in Practice!
			//Don't throw an exception, just use the defaults we filled earlier to
			//admit we don't know the severity level, assume it is top priority.
			ASSERT(FALSE);
			break;
	}

	return eResult;
}

// (b.savon 2013-06-19 17:09) - PLID 56880 - Show Monograph - Moved to utility
void ShowMonograph(long nFDBMedID, CWnd* pParent)
{
	CWaitCursor cwait;

	NexTech_Accessor::_FDBMedicationFilterPtr pFilter(__uuidof(NexTech_Accessor::FDBMedicationFilter));

	pFilter->FDBMedID = nFDBMedID;
	pFilter->IncludeMonograph = TRUE;

	// (j.jones 2016-01-21 08:52) - PLID 68020 - always filter out non-FDB drugs
	NexTech_Accessor::_NullableBoolPtr pIncludeFDBMedsOnly(__uuidof(NexTech_Accessor::NullableBool));
	pIncludeFDBMedsOnly->SetBool(VARIANT_TRUE);
	pFilter->IncludeFDBMedsOnly = pIncludeFDBMedsOnly;

	//Call the API do do all the hard work for us
	NexTech_Accessor::_FDBMedicationArrayPtr searchResults = GetAPI()->MedicationSearch(GetAPISubkey(), GetAPILoginToken(), pFilter);

	if(searchResults)
	{
		//Get the medications that were found
		Nx::SafeArray<IUnknown *> saryMedications(searchResults->FDBMedications);

		foreach(NexTech_Accessor::_FDBMedicationPtr pMedication, saryMedications)
		{
			if(pMedication)
			{
				if(pMedication->MonographData)
				{
					if(pMedication->MonographData->MonographExists)
					{
						//If we got this far then the medid has a monograph so display it
						CMonographDlg dlg(pParent);
						//Give the dlg the HTML string that it should display
						dlg.m_strHTML = CString((LPCTSTR)pMedication->MonographData->HtmlString);
						dlg.DoModal();
						//If we some how got more than one medication back lets just show the first one so return.
						return;
					}
				}
			}
		}
	}

	MessageBox(pParent->m_hWnd, "First DataBank does not have a monograph on file for this medication.", "NexTech Practice", MB_ICONINFORMATION);
}

// (b.savon 2013-06-19 17:09) - PLID 56880 - Show Leftlet - Moved to utility
void ShowLeaflet(long nFDBMedID, CWnd* pParent)
{
	CWaitCursor cwait;

	NexTech_Accessor::_FDBMedicationFilterPtr pFilter(__uuidof(NexTech_Accessor::FDBMedicationFilter));

	pFilter->FDBMedID = nFDBMedID;
	pFilter->IncludeLeaflet = TRUE;

	// (j.jones 2016-01-21 08:52) - PLID 68020 - always filter out non-FDB drugs
	NexTech_Accessor::_NullableBoolPtr pIncludeFDBMedsOnly(__uuidof(NexTech_Accessor::NullableBool));
	pIncludeFDBMedsOnly->SetBool(VARIANT_TRUE);
	pFilter->IncludeFDBMedsOnly = pIncludeFDBMedsOnly;

	//Call the API do do all the hard work for us
	NexTech_Accessor::_FDBMedicationArrayPtr searchResults = GetAPI()->MedicationSearch(GetAPISubkey(), GetAPILoginToken(), pFilter);

	if(searchResults)
	{
		//Get the medications that were found
		Nx::SafeArray<IUnknown *> saryMedications(searchResults->FDBMedications);

		foreach(NexTech_Accessor::_FDBMedicationPtr pMedication, saryMedications)
		{
			if(pMedication)
			{
				if(pMedication->LeafletData)
				{
					if(pMedication->LeafletData->LeafletExists)
					{
						//If we got this far then the medid has a leaflet so display it
						CLeafletDlg dlg(pParent);
						//Give the dlg the HTML string that it should display
						dlg.m_strHTML = CString((LPCTSTR)pMedication->LeafletData->HtmlString);
						dlg.DoModal();
						//If we some how got more than one medication back lets just show the first one so return.
						return;
					}
				}
			}
		}
	}

	MessageBox(pParent->m_hWnd, "First DataBank does not have a leaflet on file for this medication.", "NexTech Practice", MB_ICONINFORMATION);
}

// (r.farnworth 2013-08-14 10:56) - PLID 58001 - Moved the function here to be used by the Formulary Project
VARIANT_BOOL ImportMedication(long nFDBID, const CString& strMedName, long &nNewDrugListID)
{
	VARIANT_BOOL vbSuccess = VARIANT_FALSE;
	try{
		// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
		{
			//Also check that the database exists
			if(!FirstDataBank::EnsureDatabase(NULL, true)) 
			{
				//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
				return VARIANT_FALSE;
			}
		}

		CArray<NexTech_Accessor::_FDBMedicationImportInputPtr, NexTech_Accessor::_FDBMedicationImportInputPtr> aryMedications;
		NexTech_Accessor::_FDBMedicationImportInputPtr med(__uuidof(NexTech_Accessor::FDBMedicationImportInput));

		med->FirstDataBankID = nFDBID;
		med->MedicationName = AsBstr(strMedName);

		aryMedications.Add(med);

		//	Create our SAFEARRAY to be passed to the AllergyImport function in the API
		Nx::SafeArray<IUnknown *> saryMedications = Nx::SafeArray<IUnknown *>::From(aryMedications);

		CWaitCursor cwait;

		//	Call the API to import the meds and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
		NexTech_Accessor::_FDBMedicationImportResultsArrayPtr importResults = GetAPI()->MedicationImport(GetAPISubkey(), GetAPILoginToken(), saryMedications);

		//	If for some reason we get nothing back (although, this should never happen), tell the user and bail.
		if( importResults->FDBMedicationImportResults == NULL ){
			//MsgBox("There were no medications to import."); This shouldn't happen, if it did, there is some accessor error
			return VARIANT_FALSE;
		}

		Nx::SafeArray<IUnknown *> saryMedicationResults(importResults->FDBMedicationImportResults);

		// Hand back our new DrugListID
		foreach(NexTech_Accessor::_FDBMedicationImportOutputPtr pMedicationResult, saryMedicationResults)
		{
			nNewDrugListID = pMedicationResult->DrugListID;
			vbSuccess = pMedicationResult->Success;
		}

		 return vbSuccess;
	}NxCatchAll(__FUNCTION__);
	return vbSuccess;
}

// (s.dhole 2013-08-16 13:05) - PLID 58000  return copay value
//TES 8/28/2013 - PLID 57999 - Moved here from NexFormularyDlg
CString GetCopayInformation(const Nx::SafeArray<IUnknown *> &saryCopayInformation  )
{
	BOOL bIsDrugSpecific = FALSE ; 
	CString strCopay;
	foreach(NexTech_Accessor::_CopayInformationPtr  oCopayInformation, saryCopayInformation){
		CString strTier,strRange, strCopayRangeInfo;
		CString strFlatCopayAmount,strPercentageCopay;
		CString strCoPayLine; 
		switch (oCopayInformation->TypeOfPharmacy) {
			case NexTech_Accessor::PharmacyType_Retail:  
				{
					strCoPayLine = "Retail Pharmacy: ";
				}
				break;
			case NexTech_Accessor::PharmacyType_MailOrder:  
				{
					strCoPayLine = "Mail Order Pharmacy: ";
				}
				break;
			case NexTech_Accessor::PharmacyType_Specialty:  
				{
					strCoPayLine = "Specialty Pharmacy:  ";
				}
				break;
			case NexTech_Accessor::PharmacyType_LongTermCare:  
				{
					strCoPayLine = "Long Term Care Pharmacy:  ";
				}
				break;
			case NexTech_Accessor::PharmacyType_Any:  
				{
					strCoPayLine = " Any Pharmacy: ";
				}
				break;
			default:
			//no pahrmacy information
				strCoPayLine += " None;";
			break;
		}

		if (!bIsDrugSpecific){
			bIsDrugSpecific= AsBool(oCopayInformation->IsDrugSpecific);
		}
		// flat ammount
		if (!oCopayInformation->FlatCopayAmount->IsNull() && !AsString(oCopayInformation->FlatCopayAmount->GetValue()).IsEmpty()){
			strFlatCopayAmount = "$";
			strFlatCopayAmount += AsString(oCopayInformation->FlatCopayAmount->GetValue()); 
			strFlatCopayAmount += " Flat Copay, ";
		}
		// Percent
		if (!oCopayInformation->PercentageCopay->IsNull() && !AsString(oCopayInformation->PercentageCopay->GetValue()).IsEmpty()){
			strPercentageCopay = AsString(AsDouble(oCopayInformation->PercentageCopay->GetValue()) * 100.00 ); 
			strPercentageCopay += "% Copay, ";
		}

		//TODO Check  if priority on flat Copay or Term  , if it is exist
		if (oCopayInformation->FirstTerm == NexTech_Accessor::FirstCopayTerm_FlatCopay){
			strCoPayLine += strFlatCopayAmount; 
			strCoPayLine += strPercentageCopay;
		}
		else if (oCopayInformation->FirstTerm == NexTech_Accessor::FirstCopayTerm_PercentCopay){
			strCoPayLine += strPercentageCopay;
			strCoPayLine += strFlatCopayAmount; 
		}
		else{
			// We missing priority, willm load flat and %
			if (!strFlatCopayAmount.IsEmpty()){
				strCoPayLine += strFlatCopayAmount; 
			}
			if (!strPercentageCopay.IsEmpty()){
				strCoPayLine += strPercentageCopay;
			}
		}
		

		if (!oCopayInformation->DaysSupplyPerCopay->IsNull() && !AsString(oCopayInformation->DaysSupplyPerCopay->GetValue()).IsEmpty()){
			strCoPayLine += AsString( oCopayInformation->DaysSupplyPerCopay->GetValue()); 
			strCoPayLine += " Day Supply, ";
		}


		if (!oCopayInformation->CopayTier->IsNull()){
			strTier = AsString( oCopayInformation->CopayTier->GetValue()) ;
			//if CopayTier  is exist than only check MaximumCopayTier
			if (!oCopayInformation->MaximumCopayTier->IsNull() && !AsString(oCopayInformation->MaximumCopayTier->GetValue()).IsEmpty()){
				strTier += "/";
				strTier += AsString(oCopayInformation->MaximumCopayTier->GetValue());   
			}
			if (!strTier.IsEmpty()){
				strTier +=  " Tier, ";
			}
			else{
				// none
			}

			strCoPayLine += strTier;
		}

		// we should have both value else it is invalid
		if (!oCopayInformation->OutOfPocketRangeStart->IsNull() && !AsString(oCopayInformation->OutOfPocketRangeStart->GetValue()).IsEmpty() ){
			strRange = "Min $";
			strRange += AsString(oCopayInformation->OutOfPocketRangeStart->GetValue()); 
		}
		if (!oCopayInformation->OutOfPocketRangeEnd->IsNull() && !AsString(oCopayInformation->OutOfPocketRangeEnd->GetValue()).IsEmpty()){
			if (!strRange.IsEmpty()){
				strRange += " - ";
			 }
			strRange += "Max $";
			strRange += AsString(oCopayInformation->OutOfPocketRangeEnd->GetValue()); 
		}
		if (!strRange.IsEmpty()){
			strCoPayLine += strRange;
			strCoPayLine += " Out Of Pocket, ";
		}
	
		// if min is exist then only max is there
		if (!oCopayInformation->MinimumCopay->IsNull() && !AsString(oCopayInformation->MinimumCopay->GetValue()).IsEmpty()){
			strCopayRangeInfo = "Min $";
			strCopayRangeInfo += AsString(oCopayInformation->MinimumCopay->GetValue()); 
		}
		// (j.fouts 2013-10-01 11:38) - PLID 57690  - Corrected spelling
		if (!oCopayInformation->MaximumCopay->IsNull() && !AsString(oCopayInformation->MaximumCopay->GetValue()).IsEmpty()){
			if (!strCopayRangeInfo.IsEmpty()){
				strCopayRangeInfo += " - ";
			 }
			strCopayRangeInfo += "Max $";
			// (j.fouts 2013-10-01 11:38) - PLID 57690  - Corrected spelling
			strCopayRangeInfo += AsString(oCopayInformation->MaximumCopay->GetValue()); 
			
		}
		
		if (!strCopayRangeInfo.IsEmpty()){
			strCoPayLine += strCopayRangeInfo;
			strCoPayLine += " Copay Range, ";
		}

		if (strCoPayLine.GetLength()>3){
			strCoPayLine =strCoPayLine.Left(strCoPayLine.GetLength()-2); 
		}
		
		if (!strCoPayLine.IsEmpty()){
			if (!strCopay.IsEmpty()){
				strCopay += ";\r\n";
			}
			strCopay+=strCoPayLine; 
		}
	}
	// We show only drug specific info
	if (!strCopay.IsEmpty() )
	{
		if (bIsDrugSpecific) {
			return "Drug Specific\r\n" + strCopay;
		}
		else
		{
			return "Summary Level\r\n" + strCopay;
		}
	}
	return strCopay;
}

// (s.dhole 2013-09-16 09:05) - PLID 58357 Check if patient has any appointment  last +72/-72 hour
// we should get date to copare time diffrence
BOOL IsPatientEventExist(CWnd* pParent, long nPatitntID)
{
	// only allow 72 Hour validation
		if  (ReturnsRecordsParam(
		 " SELECT Top 1 1  \r\n"
		 "FROM PatientsT \r\n"
		 "INNER JOIN AppointmentsT ON PatientsT.PersonID = AppointmentsT.PatientID \r\n"
		 "WHERE AppointmentsT.Status <> 4 \r\n"
		 "AND AppointmentsT.PatientID ={INT} \r\n"
		 "AND AppointmentsT.StartTime BETWEEN DATEADD(HH,-72,GETDATE())  AND DATEADD(HH,72,GETDATE())  \r\n",nPatitntID))
		{
		 return TRUE;
		}
		else {
			
			if (IDNO == MessageBox(pParent->m_hWnd,"There is no appointment scheduled in the next three days for this patient. Formulary information is limited to use "
			"for a patient encounter. By continuing you agree that this is associated with a patient encounter.\r\n\r\nWould you like to continue?", "NexTech Practice", MB_ICONQUESTION|MB_YESNO)){ 
				return FALSE;
			}
			else{
				return TRUE;
			}
		}

}
// (r.farnworth 2013-09-24 15:41) - PLID 58386 - Will determine whether the patient has a provider, if that provider has an NPI, and then returns the provider's ID
// (r.farnworth 2013-10-18 14:17) - PLID 59095 - Need to pass in parent dialog to produce the truncation warning
long GetGen1ProviderID(long PatientID, CWnd* pParent)
{
	long ProviderID;
	CString ProviderNPI, ProviderName;


	_RecordsetPtr rs = CreateParamRecordset("SELECT ProvidersT.PersonID, ProvidersT.NPI, PersonT.FullName "
											"FROM PatientsT "
											"INNER JOIN ProvidersT ON MainPhysician = ProvidersT.PersonID "
											"INNER JOIN PersonT ON MainPhysician = PersonT.ID "
											"WHERE PatientsT.PersonID = {INT}", PatientID);
	if(!rs->eof) {
		FieldsPtr nFlds = rs->Fields;
		ProviderID = AdoFldLong(nFlds, "PersonID");
		ProviderNPI = AdoFldString(nFlds, "NPI");
		ProviderName = AdoFldString(nFlds, "FullName");
	} else {
		MsgBox(MB_OK|MB_ICONINFORMATION, 
					"There is currently no provider selected for this patient.\n"
					"This information is required to send an eligibility request.\n"
					"You can add this information on the General 1 tab of the Patient Module.");
			return -1;
	}

	if(ProviderNPI == "")
	{
		MsgBox(MB_OK|MB_ICONINFORMATION, 
					"Provider %s does not have an NPI.\n"
					"This information is required to send an eligibility request.", ProviderName);
			return -1;
	} 

	GenerateTruncationWarning(PatientID, ProviderID, pParent);
	
	return ProviderID;
}

// (r.farnworth 2013-10-24 16:58) - PLID 59095 - Need a new utility function for more lengths
void GenerateTruncationWarning(long PatientID, long ProviderID, CWnd* pParent)
{
	bool bDisplayWarning = false;
	CString ProviderNPI, ProviderFirst, ProviderLast, ProviderMiddle, ProviderTitle, ProviderAddress1, ProviderAddress2, ProviderCity, ProviderState, 
		ProviderZip, ProviderTaxonomyCode, PatientFirst, PatientLast, PatientMiddle, PatientTitle, PatientAddress1, PatientAddress2, PatientCity, PatientState,
		PatientZip, LocationEIN, LocationAddress1, LocationAddress2, LocationCity, LocationState, LocationZip;

	CString strTruncateWarning = "A provider, patient, and location are required to retrieve eligibility information.\r\n"
		"One of the above selected has a field that exceeds the character limit and will be truncated\r\n"
		"when retrieving eligibility information."
		"\r\n\r\n";

	_RecordsetPtr rs1 = CreateParamRecordset("SELECT ProvidersT.NPI AS 'ProvNPI', PersonT.First AS 'ProvFirst', PersonT.Last AS 'ProvLast', PersonT.Middle AS 'ProvMiddle', "
											"PersonT.Title AS 'ProvTitle', PersonT.Address1 AS 'ProvAddr1', PersonT.Address2 AS 'ProvAddr2', "
											"PersonT.City AS 'ProvCity', PersonT.State AS 'ProvState', PersonT.Zip AS 'ProvZip', ProvidersT.TaxonomyCode AS 'ProvTaxo' "
											"FROM ProvidersT "
											"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
											"WHERE ProvidersT.PersonID = {INT} "
											, ProviderID);

	_RecordsetPtr rs2 = CreateParamRecordset("SELECT PersonT.First AS 'PatFirst', PersonT.Last AS 'PatLast', PersonT.Middle AS 'PatMid', PersonT.Title AS 'PatTitle', "
											"PersonT.Address1 AS 'PatAddr1', PersonT.Address2 AS 'PatAddr2', "
											"PersonT.City AS 'PatCity', PersonT.State AS 'PatState', PersonT.Zip AS 'PatZip', "
											"LocationsT.EIN AS 'LocEIN', LocationsT.Address1 AS 'LocAddr1', LocationsT.Address2 AS 'LocAddr2', "
											"LocationsT.City AS 'LocCity', LocationsT.State AS 'LocState', LocationsT.Zip AS 'LocZip' "
											"FROM PatientsT "
											"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
											"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
											"WHERE PatientsT.PersonID = {INT} "
											, PatientID);


	if(!rs1->eof && !rs2->eof) {
		FieldsPtr nFldsProv = rs1->Fields;
		FieldsPtr nFldsPat = rs2->Fields;

		ProviderNPI = AdoFldString(nFldsProv, "ProvNPI", "");
		ProviderFirst = AdoFldString(nFldsProv, "ProvFirst", "");
		ProviderLast = AdoFldString(nFldsProv, "ProvLast", "");
		ProviderMiddle = AdoFldString(nFldsProv, "ProvMiddle", "");
		ProviderTitle = AdoFldString(nFldsProv, "ProvTitle", "");
		ProviderAddress1  = AdoFldString(nFldsProv, "ProvAddr1", "");
		ProviderAddress2  = AdoFldString(nFldsProv, "ProvAddr2", "");
		ProviderCity = AdoFldString(nFldsProv, "ProvCity", "");
		ProviderState  = AdoFldString(nFldsProv, "ProvState", "");
		ProviderZip  = AdoFldString(nFldsProv, "ProvZip", "");
		ProviderTaxonomyCode = AdoFldString(nFldsProv, "ProvTaxo", "");
		PatientFirst = AdoFldString(nFldsPat, "PatFirst", "");
		PatientLast = AdoFldString(nFldsPat, "PatLast", "");
		PatientMiddle = AdoFldString(nFldsPat, "PatMid", "");
		PatientTitle = AdoFldString(nFldsPat, "PatTitle", "");
		PatientAddress1 = AdoFldString(nFldsPat, "PatAddr1", "");
		PatientAddress2 = AdoFldString(nFldsPat, "PatAddr2", "");
		PatientCity = AdoFldString(nFldsPat, "PatCity", "");
		PatientState = AdoFldString(nFldsPat, "PatState", "");
		PatientZip = AdoFldString(nFldsPat, "PatZip", "");
		LocationEIN = AdoFldString(nFldsPat, "LocEin", "");
		LocationAddress1 = AdoFldString(nFldsPat, "LocAddr1", "");
		LocationAddress2 = AdoFldString(nFldsPat, "LocAddr2", "");
		LocationCity = AdoFldString(nFldsPat, "LocCity", "");
		LocationState = AdoFldString(nFldsPat, "LocState", "");
		LocationZip = AdoFldString(nFldsPat, "LocZip", "");
	}

	//// (r.farnworth 2013-10-18 14:19) - PLID 59095 - Check for the various length restrictions and warn the user of each individually
	if (ProviderNPI.GetLength() > 80)
	{
		strTruncateWarning += "The provider's NPI is more than 80 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderFirst.GetLength() > 25)
	{
		strTruncateWarning += "The provider's first name is more than 25 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderLast.GetLength() > 35)
	{
		strTruncateWarning += "The provider's last name is more than 35 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderMiddle.GetLength() > 25)
	{
		strTruncateWarning += "The provider's middle name is more than 25 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderTitle.GetLength() > 10)
	{
		strTruncateWarning += "The provider's title is more than 10 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderAddress1.GetLength() > 55)
	{
		strTruncateWarning += "The provider's address 1 is more than 55 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderAddress2.GetLength() > 55)
	{
		strTruncateWarning += "The provider's address 2 is more than 55 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderCity.GetLength() > 30)
	{
		strTruncateWarning += "The provider's city is more than 30 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderState.GetLength() > 2)
	{
		strTruncateWarning += "The provider's state is more than 2 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderZip.GetLength() > 15)
	{
		strTruncateWarning += "The provider's zip is more than 15 characters.\r\n";
		bDisplayWarning = true;
	}

	if (ProviderTaxonomyCode.GetLength() > 50)
	{
		strTruncateWarning += "The provider's taxonomy code is more than 50 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientFirst.GetLength() > 25)
	{
		strTruncateWarning += "The patient's first name is more than 25 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientLast.GetLength() > 60)
	{
		strTruncateWarning += "The patients's last name is more than 60 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientMiddle.GetLength() > 25)
	{
		strTruncateWarning += "The patient's middle name is more than 25 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientTitle.GetLength() > 10)
	{
		strTruncateWarning += "The patient's title is more than 10 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientAddress1.GetLength() > 55)
	{
		strTruncateWarning += "The patient's address 1 is more than 55 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientAddress2.GetLength() > 55)
	{
		strTruncateWarning += "The patient's address 2 is more than 55 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientCity.GetLength() > 30)
	{
		strTruncateWarning += "The patient's city is more than 30 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientState.GetLength() > 2)
	{
		strTruncateWarning += "The patient's state is more than 2 characters.\r\n";
		bDisplayWarning = true;
	}

	if (PatientZip.GetLength() > 15)
	{
		strTruncateWarning += "The patient's zip is more than 15 characters.\r\n";
		bDisplayWarning = true;
	}

	if (LocationEIN.GetLength() > 10)
	{
		strTruncateWarning += "The location's EIN is more than 10 characters.\r\n";
		bDisplayWarning = true;
	}

	if (LocationAddress1.GetLength() > 55)
	{
		strTruncateWarning += "The location's address 1 is more than 55 characters.\r\n";
		bDisplayWarning = true;
	}
	if (LocationAddress2.GetLength() > 55)
	{
		strTruncateWarning += "The location's address 2 is more than 55 characters.\r\n";
		bDisplayWarning = true;
	}

	if (LocationCity.GetLength() > 30)
	{
		strTruncateWarning += "The location's city is more than 30 characters.\r\n";
		bDisplayWarning = true;
	}

	if (LocationState.GetLength() > 2)
	{
		strTruncateWarning += "The location's state is more than 2 characters.\r\n";
		bDisplayWarning = true;
	}

	if (LocationZip.GetLength() > 15)
	{
		strTruncateWarning += "The location's zip is more than 15 characters.\r\n";
		bDisplayWarning = true;
	}


	if(bDisplayWarning)
	{
		DontShowMeAgain(pParent, strTruncateWarning, "SSEligibilityTruncateWarning", "Eligibility information will be truncated.");
	}
}


// (s.dhole 2013-10-08 10:57) - PLID  58923 moved from CNexFormularyDlg
CString GetFormularyInsuranceSQL()
{
	// Do not filter any case [r.gonet - Moved an old comment to top of the SQL. As far as I can tell, during development of formular support,
	// we used to filter on only Active coverages. However, at some point for some reason we decided to also allow selection of Inactive coverages,
	// so we removed the filter on SureScriptsEligibilityDetailT.Coverage.]
	// (r.gonet 2016-03-03 11:25) - PLID 68494 - Fixed an issue where an expression that was clearly meant to be part of the where clause
	// was being made part of the join. I had some more changes in here before but removed them. Left in this fix though since it was a
	// maintenance concern.
	return
		"( SELECT SureScriptsEligibilityDetailT.ID,SureScriptsEligibilityRequestT.PatientID, SureScriptsEligibilityDetailT.InsuranceCoName, SureScriptsEligibilityDetailT.PBMID, SureScriptsEligibilityDetailT.PBMName, SureScriptsEligibilityDetailT.Insurancelevel, \r\n"
		"SureScriptsEligibilityDetailT.PlanName, SureScriptsEligibilityDetailT.Coverage ,SureScriptsEligibilityDetailT.FormularyID,SureScriptsEligibilityDetailT.AlternativesID \r\n"
		",SureScriptsEligibilityDetailT.CoverageID,SureScriptsEligibilityDetailT.CopayID,SureScriptsEligibilityRequestT.SentDateUTC  \r\n"
		// (j.fouts 2013-09-12 12:11) - PLID 58358 - Added DemographicsHaveChanged
		",'Active' AS CoverageActive, SureScriptsEligibilityDetailT.DemographicsHaveChanged, \r\n"
		" SureScriptsEligibilityDetailT.IsFailure, CAST(CASE WHEN SureScriptsEligibilityDetailT.Coverage = 1 THEN 1 ELSE 0 END AS BIT) AS Active, \r\n"
		"(      SELECT STUFF((SELECT ',' + CHAR(13)+CHAR(10) + ' ' + (CASE WHEN  ServiceType =1 THEN 'Retail ' +  \r\n"
		"CASE WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=1 THEN '(Active)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=2 THEN '(Inactive)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=3 THEN '(Out Of Pocket)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=4 THEN '(Non Covered)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=5 THEN '(Could Not Process)' \r\n"
		"END   \r\n"
		"+ (CASE  WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NOT NULL  THEN ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101)  + ' - ' + 'End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']'   \r\n"
		"		WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NULL  THEN  ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101) + ']'    \r\n"
		"		WHEN [CoverageStartDate] IS NULL AND CoverageEndDate IS NOT NULL  THEN  ' :[End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']'     \r\n"
		"		ELSE + ''   \r\n"
		"  END)   \r\n"
		"WHEN  ServiceType =2 THEN 'Mail Order '  \r\n"
		"+ \r\n"
		"CASE WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=1 THEN '(Active)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=2 THEN '(Inactive)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=3 THEN '(Out Of Pocket)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=4 THEN '(Non Covered)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=5 THEN '(Could Not Process)' \r\n"
		"END   \r\n"
		"+ (CASE  WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NOT NULL  THEN ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101)  + ' - ' + 'End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']'    \r\n"
		"		WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NULL  THEN  ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101) + ']'     \r\n"
		"		WHEN [CoverageStartDate] IS NULL AND CoverageEndDate IS NOT NULL  THEN  ' :[End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']'     \r\n"
		"		ELSE + ''   \r\n"
		"  END)   \r\n"
		"WHEN  ServiceType =3 THEN 'Speciality '  \r\n"
		"+ \r\n"
		"CASE WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=1 THEN '(Active)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=2 THEN '(Inactive)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=3 THEN '(Out Of Pocket)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=4 THEN '(Non Covered)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=5 THEN '(Could Not Process)' \r\n"
		"END     \r\n"
		"+ (CASE  WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NOT NULL  THEN ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101)  + ' - ' + 'End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']'  \r\n"
		"		WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NULL  THEN  ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101) + ']'   \r\n"
		"		WHEN [CoverageStartDate] IS NULL AND CoverageEndDate IS NOT NULL  THEN  ' :[End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']'   \r\n"
		"		ELSE + ''   \r\n"
		"   END)   \r\n"
		"WHEN  ServiceType =4 THEN 'Long Term '  \r\n"
		"+ \r\n"
		"CASE WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=1 THEN '(Active)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=2 THEN '(Inactive)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=3 THEN '(Out Of Pocket)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=4 THEN '(Non Covered)' \r\n"
		"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=5 THEN '(Could Not Process)' \r\n"
		"END   \r\n"
		"+ (CASE  WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NOT NULL  THEN ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101)  + ' - ' + 'End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']'  \r\n"
		"		WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NULL  THEN  ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101) + ']'   \r\n"
		"		WHEN [CoverageStartDate] IS NULL AND CoverageEndDate IS NOT NULL  THEN  ' :[End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']'   \r\n"
		"		ELSE + ''   \r\n"
		"  END)   \r\n"
		"END)    \r\n"
		"  As PharmacyCov  \r\n"
		"FROM [SureScriptsEligibilityPharmacyCoverageT] WHERE SureScriptsEligibilityPharmacyCoverageT.ResponseDetailID =SureScriptsEligibilityDetailT.ID    FOR XML PATH(''), TYPE).value('/', 'NVARCHAR(MAX)'), 1, 3, '')) AS PharmacyCoverage \r\n"
		"FROM SureScriptsEligibilityResponseT INNER JOIN \r\n"
		"SureScriptsEligibilityRequestT ON SureScriptsEligibilityResponseT.RequestID = SureScriptsEligibilityRequestT.ID INNER JOIN \r\n"
		"SureScriptsEligibilityDetailT ON SureScriptsEligibilityResponseT.ID = SureScriptsEligibilityDetailT.ResponseID \r\n"
		//"WHERE SureScriptsEligibilityDetailT.Coverage = 1 \r\n"
		"WHERE (DATEDIFF( HH , SureScriptsEligibilityResponseT.ReceivedDateUTC ,GETUTCDATE()) <=  72   AND DATEDIFF( HH , SureScriptsEligibilityResponseT.ReceivedDateUTC,GETUTCDATE()) >=0) ) AS SureScriptsEligibilityDetailQ \r\n";


}

// (b.savon 2014-08-26 10:35) - PLID 63401 - Moved this logic into utilities
long CheckExistingFormularyData(CWnd* parent, long nPatientID)
{
	long nInsuranceID = -1;

	if (nPatientID == -1) {
		//this function should not have been called
		ASSERT(FALSE);
	}

	//Will check if there is successful formulary request we made during last 72 hours  
	if (FALSE == ReturnsRecordsParam("Select TOP 1 1 from SureScriptsEligibilityResponseT INNER JOIN "
		" SureScriptsEligibilityRequestT "
		" ON SureScriptsEligibilityResponseT.RequestID  = SureScriptsEligibilityRequestT.ID  "
		" WHERE SureScriptsEligibilityResponseT.IsFailure =0  "
		" AND (DATEDIFF( HH , SureScriptsEligibilityResponseT.ReceivedDateUTC ,GETUTCDATE()) <=  72   AND DATEDIFF( HH, SureScriptsEligibilityResponseT.ReceivedDateUTC,GETUTCDATE()) >=0)   "
		" AND SureScriptsEligibilityRequestT.PatientID = {INT}", nPatientID)){
		// Execute new request 
		//Make sure we pass Correct provider information
		long ProviderID = GetGen1ProviderID(nPatientID, parent);
		if (ProviderID <= 0){
			return nInsuranceID;
		}
		GetAPI()->RequestSureScriptsEEligibility(GetAPISubkey(), GetAPILoginToken(), AsBstr(AsString(nPatientID)), AsBstr(AsString(ProviderID)), AsBstr(AsString(GetCurrentLocationID())));
	}
	// (r.gonet 2016-03-03 13:45) - PLID 68494 - Exclude "CouldNotProcess" details, which are invalid coverages.
	_RecordsetPtr rs = CreateParamRecordset("Select TOP 1 SureScriptsEligibilityDetailT.ID AS ID from SureScriptsEligibilityResponseT INNER JOIN  "
		" SureScriptsEligibilityRequestT  "
		" ON SureScriptsEligibilityResponseT.RequestID  = SureScriptsEligibilityRequestT.ID   "
		" INNER JOIN "
		" SureScriptsEligibilityDetailT ON SureScriptsEligibilityResponseT.ID = SureScriptsEligibilityDetailT.ResponseID "
		" WHERE SureScriptsEligibilityResponseT.IsFailure =0   "
		" AND SureScriptsEligibilityDetailT.Coverage <> {CONST_INT} "
		" AND (DATEDIFF( HH , SureScriptsEligibilityResponseT.ReceivedDateUTC ,GETUTCDATE()) <=  72   AND DATEDIFF( HH, SureScriptsEligibilityResponseT.ReceivedDateUTC,GETUTCDATE()) >=0)    "
		" AND SureScriptsEligibilityRequestT.PatientID = {INT}", (long)EligibilityCoverageStatus::CouldNotProcess, nPatientID);
	if (!rs->eof) {
		nInsuranceID = AdoFldLong(rs, "ID", -1);
	}

	return nInsuranceID;
}

// (r.farnworth 2016-01-08 15:37) - PLID 58692 - Moved to Utils
// (a.walling 2009-04-09 15:49) - PLID 33949 - Load a default pharmacy from patient favorites
long GetDefaultPharmacyID(long nPatientID)
{
	long nPharmacyID = -1;
	long nDefaultPharmacyPreference = GetRemotePropertyInt("DefaultPharmacy", 0, 0, GetCurrentUserName(), true);

	switch (nDefaultPharmacyPreference) {
	case 2:
		// no pharmacy
		nPharmacyID = -1;
		break;
	case 1:
		// last favorite pharmacy
	{
		_RecordsetPtr prsPharmacy = CreateParamRecordset("SELECT TOP 1 PatientMedications.PharmacyID FROM PatientMedications INNER JOIN LocationsT ON PatientMedications.PharmacyID = LocationsT.ID INNER JOIN FavoritePharmaciesT ON LocationsT.ID = FavoritePharmaciesT.PharmacyID WHERE Active = 1 AND Deleted = 0 AND TypeID = 3 AND PatientMedications.PatientID = {INT} ORDER BY PrescriptionDate DESC, PatientMedications.ID DESC", nPatientID);
		if (!prsPharmacy->eof) {
			nPharmacyID = AdoFldLong(prsPharmacy, "PharmacyID", -1);
		}
		else {
			nPharmacyID = -1;
		}
	}
	break;
	case 0:
	default:
		// favourite pharmacy, or none if none are set.
	{
		_RecordsetPtr prsPharmacy = CreateParamRecordset("SELECT TOP 1 PharmacyID FROM FavoritePharmaciesT LEFT JOIN LocationsT ON PharmacyID = LocationsT.ID WHERE Active = 1 AND TypeID = 3 AND PatientID = {INT} ORDER BY OrderIndex ASC", nPatientID);
		if (!prsPharmacy->eof) {
			nPharmacyID = AdoFldLong(prsPharmacy, "PharmacyID", -1);
		}
		else {
			nPharmacyID = -1;
		}
	}
	break;
	}

	return nPharmacyID;
}
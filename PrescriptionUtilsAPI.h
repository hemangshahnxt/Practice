#pragma once

// (j.jones 2012-11-19 16:59) - PLID 52818 - added PrescriptionUtils
// (j.jones 2013-03-27 16:43) - PLID 55920 - Renamed to PrescriptionUtilsAPI, this
// should only define functions that depend on the API / Accessor.
// I then moved many enums & functions to PrescriptionUtilsNonAPI.h
// so that any code that needs enums or other functions without also needing API accessor code
// will not need to also include NxAPI.h.

#include "NxAPI.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:18) - PLID 55920 - some non-API enums are referenced here

// (b.savon 2013-03-19 17:08) - PLID 55477 - created
struct PrescriptionInfo{
	NexTech_Accessor::_QueuePrescriptionPtr pPrescription;
	Nx::SafeArray<IUnknown*> saryPrescribers;
	Nx::SafeArray<IUnknown*> sarySupervisors;
	Nx::SafeArray<IUnknown*> saryNurseStaff;
	NexTech_Accessor::ERxUserRole erxUserRole;
};

// (j.jones 2013-11-25 09:11) - PLID 59772 - added struct of interaction information
// to make passing this info. around a little easier
struct DrugInteractionInfo {
	Nx::SafeArray<IUnknown*> saryDrugDrugInteracts;
	Nx::SafeArray<IUnknown*> saryDrugAllergyInteracts;
	Nx::SafeArray<IUnknown*> saryDrugDiagnosisInteracts;
};

// (j.jones 2012-11-19 17:03) - PLID 52818 - Created SaveNewPrescription as
// one single modular function used to make new entries in PatientMedications.
// Its results object can optionally return the current queue, with filters applied.
// Returns the new prescription object.
// (j.fouts 2013-02-04 11:58) - PLID 53954 - Modfied this for use with GetQueueFilters()
// (s.dhole 2013-10-18 13:14) - PLID 59068 Added formulary insurance id
NexTech_Accessor::_UpdatePresQueueResultsPtr SaveNewPrescription(long nMedicationID, 
																 BOOL bRequeryQueue = FALSE,
																 Nx::SafeArray<IUnknown*> saryFilters = NULL,
																 long nPatientID = -1, 
																 long nProviderID = -1, 
																 long nLocationID = GetCurrentLocation(),
																 COleDateTime dtDate = COleDateTime::GetCurrentTime(),
																 long nEMNID = -1, 
																 BOOL bIsEMRTemplate = FALSE,
																 SourceActionInfo saiSpawnedFromAction = SourceActionInfo(),
																 long nPharmacyID = -1, 
																 long nDenyNewRxResponseID = -1,
																 long nFInsuranceDetailID =-1);

// (j.fouts 2012-11-20 11:56) - PLID 52906 - This maps the ID of the PrescriptionQueueStatusT to the enum value used by the
//		accessor. Because SOAP serializes the enum as a string it loses the associated value, so there is a mismatch between
//		the values in data and the values of the enum in the accessor. This function is used to rectify the mismatch.
NexTech_Accessor::PrescriptionStatus MapQueueStatusToAccessor(long nStatus);

// (j.jones 2013-01-23 16:30) - PLID 53259 - added MapAccessorToQueueStatus,
// takes in a NexTech_Accessor::PrescriptionStatus value and returns a Practice PrescriptionQueueStatus enum
PrescriptionQueueStatus MapAccessorToQueueStatus(NexTech_Accessor::PrescriptionStatus eStatus);

// (r.gonet 02/28/2014) - PLID 60755 - Converts an FDBDiagnosisCodeSystem value to an FDBDiagnosisCodeSystem in the accessor namespace.
NexTech_Accessor::FDBDiagnosisCodeSystem MapFDBDiagnosisCodeSystemToAccessor(FDBDiagnosisCodeSystem eCodeSystem);
// (r.gonet 02/28/2014) - PLID 60755 - Converts an FDBDiagnosisCodeSystem value in the accessor namespace to an FDBDiagnosisCodeSystem. 
FDBDiagnosisCodeSystem MapAccessorToFDBDiagnosisCodeSystem(NexTech_Accessor::FDBDiagnosisCodeSystem eCodeSystem);

// (j.fouts 2012-11-27 16:46) - PLID 51889 - This will map a DrugType from data to its API equivalent
NexTech_Accessor::DrugType MapDrugTypeToAccessor(long nType);
// (j.fouts 2012-11-27 16:46) - PLID 51889 - This will map a DrugType from the API to the database equivalent
long MapDrugTypeToData(NexTech_Accessor::DrugType drugType);

// (b.savon 2013-03-18 16:10) - PLID 55477
EUserRoleTypes AccessorERxUserRoleToPracticeEnum(NexTech_Accessor::ERxUserRole userRole);

// (j.fouts 2012-12-27 16:50) - PLID 53160 - A helper function to get the text for a PrescriptionQueueStatus based on the status
CString QueueStatusTextFromID(NexTech_Accessor::PrescriptionStatus status);

// (b.savon 2013-03-19 17:08) - PLID 55477 - Reworked to use API and different return types
// (j.fouts 2013-03-12 10:13) - PLID 52973 - Moved the loading and saving logic out from the PrescriptionEditDlg
//Loads a full prescription from the API
PrescriptionInfo LoadFullPrescription(long nPrescriptionIDbIsTemplate, bool bIsTemplate = false);
//Loads a full prescription from the API
NexTech_Accessor::_UpdatePresQueueResultsPtr LoadFullPrescription(CString &strPrescriptionID, bool bIsTemplate = false);

// (j.jones 2013-05-10 15:30) - PLID 55955 - moved the severity enums here to be used globally

//Severity Colors
// (j.jones 2012-11-13 15:34) - PLID 53734 - changed these colors so highest severity (Severe) is dark pink,
// medium (Serious) is a faint yellow, low (Moderate) is white
enum EDrugInteractionSeverityColor { 
	scHigh = 0xAEAEFF, 
	scMid = RGB(255, 253, 170), 
	scLow = 0xFFFFFF,
};

// (j.jones 2012-11-13 12:20) - PLID 53724 - This severity enum is defined by us, not FDB.
// It is only used for the purposes of sorting the most severe interactions to the top,
// so I renamed it to reflect that.
//Severity Order Levels
enum EDrugInteractionSeverityOrder { 
	soTopPriority = 0, 
	soMidPriority, 
	soLowPriority,
};

//Drug Interaction Types
enum EDrugInteractionType { 
	ditDrugDrug = 0,
	ditDrugAllergy, 
	ditDrugDiagnosis, 
};

// (j.jones 2013-05-10 15:21) - PLID 55955 - this struct allows a lookup by interaction severity
// so that the "short" name we display, the priority (for sorting purposes), and the color we would use
// are passed back to both the drug interaction dialog and the severity filter configuration dialog
struct DrugInteractionDisplayFields
{
	CString strSeverityName;
	EDrugInteractionSeverityOrder ePriority;
	EDrugInteractionSeverityColor eColor;
};

// (j.jones 2013-05-10 15:21) - PLID 55955 - given a drug interaction severity enum, return a struct
// with the "short" name we display, the priority (for sorting purposes), and the color we would use,
// so that both the drug interaction dialog and the severity filter configuration use the same information
// (j.fouts 2013-05-20 10:17) - PLID 56571 - The API now calcultes the severity
DrugInteractionDisplayFields GetDrugAllergyInteractionSeverityInfo(NexTech_Accessor::FDBAllergyInteractionSource eSource, NexTech_Accessor::DrugAllergySeverityLevel eSeverity);
DrugInteractionDisplayFields GetDrugDrugInteractionSeverityInfo(NexTech_Accessor::FDBDrugInteractionSeverity eSeverity);
DrugInteractionDisplayFields GetDrugDiagnosisInteractionSeverityInfo(NexTech_Accessor::FDBDiagnosisInteractionSeverity eSeverity);

// (b.savon 2013-06-19 17:10) - PLID 56880
void ShowMonograph(long nFDBMedID, CWnd* pParent);
void ShowLeaflet(long nFDBMedID, CWnd* pParent);

VARIANT_BOOL ImportMedication(long nFDBID, const CString& strMedName, long &nNewDrugListID); // (r.farnworth 2013-08-14 10:57) - PLID 58001

// (r.farnworth 2013-09-24 15:41) - PLID 58386 - Created
// (r.farnworth 2013-10-18 14:17) - PLID 59095 - Need to pass in parent dialog to produce the truncation warning
long GetGen1ProviderID(long PatientID, CWnd* pParent); 
void GenerateTruncationWarning(long PatientID, long ProviderID, CWnd* pParent); // (r.farnworth 2013-10-24 16:56) - PLID 59095

//TES 8/28/2013 - PLID 57999 - Moved here from NexFormularyDlg
CString GetCopayInformation(const Nx::SafeArray<IUnknown *> &saryCopayInformation);

// (s.dhole 2013-09-16 08:50) - PLID 58357
BOOL IsPatientEventExist(CWnd* pParent,long nPatitntID);
// (s.dhole 2013-10-08 10:56) - PLID 58923 
CString GetFormularyInsuranceSQL();
// (b.savon 2014-08-26 10:35) - PLID 63401 - Put this logic in utilities
long CheckExistingFormularyData(CWnd* parent, long nPatientID);
// (r.farnworth 2016-01-08 15:37) - PLID 58692 - Moved to Utils
long GetDefaultPharmacyID(long nPatientID);

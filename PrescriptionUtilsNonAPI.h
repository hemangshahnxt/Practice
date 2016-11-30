#pragma once

// (j.jones 2013-03-27 16:43) - PLID 55920 - Created to define enums & non-API functions,
// in a file separately from PrescriptionUtilsAPI.h, so that any code that needs the enums
// without also needing API accessor code will not need to also include NxAPI.h.

// (b.savon 2012-11-28 17:34) - PLID 51705
enum EPrescriptionResponseStatusColor {
	prscFailure = RGB(255,127,127),
	prscSuccess = RGB(128,255,128),
};

// (b.savon 2013-01-24 15:10) - PLID 54831 
enum EDoctorsListColor{
	dlcWriteSelected = RGB(187, 242, 197),
	dlcSupervisor = RGB(200, 210, 240),
	dlcConfigure = RGB(255, 222, 191),
};

// (b.savon 2012-11-30 08:36) - PLID 53773
enum EInteractionParent{
	eipMedications = 0,
	eipEMR = 1,
	eipQueue = 2,
	epiPrescriptionEdit = 3,	// (j.jones 2013-11-25 13:47) - PLID 59772
};

// (b.savon 2013-01-18 10:08) - PLID 54678 - Moved to utility
// (b.savon 2013-01-14 10:29) - PLID 54592
enum MedicationSearchListColumns{
	mlcMedicationID = 0,
	mlcMedicationName = 1,
	mlcFirstDataBankID = 2,
	mlcFDBOutOfDate = 3, //TES 5/9/2013 - PLID 56614
	mlcNexFormulary, // (b.savon 2013-07-16 17:49) - PLID 57377
};

// (b.savon 2013-01-30 12:09) - PLID 54927 - Add ConceptID, ConceptTypeID
// (b.savon 2013-01-21 09:51) - PLID 54704 - Moved to utility
// (b.savon 2013-01-14 16:11) - PLID 54613
enum AllergySearchListColumns{
	aslcAllergyID = 0,
	aslcAllergyName = 1,
	aslcStatus = 2,
	aslcConceptID,
	aslcConceptTypeID,
	aslcBackgroundColor,
};


enum ENurseStaffIdentifier{
	nsiID = 0,
	nsiName = 1,
};

// (j.jones 2013-01-23 16:30) - PLID 53259 - added Practice-side enums of queue statuses,
// these are the only valid values that can be saved in PatientMedications.QueueStatus
enum PrescriptionQueueStatus {
	pqsInvalid = -1,
	pqsIncomplete = 1,
	pqsPrinted = 4,
	// (j.fouts 2013-01-28 16:34) - PLID 53025 - Added on hold status
	pqsOnHold = 5,
	pqseTransmitAuthorized = 8,
	pqseTransmitSuccess = 9,
	pqseTransmitError = 10,
	pqseTransmitPending = 11,
	pqsLegacy = 12,
	pqseFaxed = 13,
	// (b.savon 2013-09-04 10:04) - PLID 58212 - Add a 'Void' and 'Ready for Doctor Review' type for Prescriptions
	pqseVoid = 14,
	pqseReadyForDoctorReview = 15,
	// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
	pqseDispensedInHouse = 16, 
};

// (r.gonet 2016-03-03 10:05) - PLID 47279 - Added an enum for SureScriptsEligibilityDetailT.Coverage and 
//  SureScriptsEligibilityPharmacyCoverageT.CoverageStatus
enum class EligibilityCoverageStatus {
	ActiveCoverage = 1,
	Inactive = 2,
	OutOfPocket = 3,
	NonCovered = 4,
	CouldNotProcess = 5,
};

// (r.gonet 02/28/2014) - PLID 60755 - Code system for diagnosis codes
enum FDBDiagnosisCodeSystem {
	fdbcsInvalid = -1,
	fdbcsICD9 = 0,
	fdbcsICD10 = 1,
};

// (b.savon 2013-03-19 17:08) - PLID 55477 - Moved this here
enum EUserRoleTypes{
	urtNone = -1,
	urtLicensedPrescriber = 0,
	urtMidlevelPrescriber,
	urtNurseStaff,
};

// (j.fouts 2013-04-30 14:15) - PLID 51889 - Added
enum DrugType {
	dgtNDC = 0,
	dgtSupply = 1,
	dgtCompound = 2,
};

// (j.fouts 2013-05-20 10:17) - PLID 56571 - The category no longer implies a Severity level
// Group interactions can be both severe or moderate so this enum allows us to filter accordinly
enum DrugAllergyFilerLevels {
	daflDirectSevere = 0,
	daflRelatedModerate,
	daflGroupSevere,
	daflGroupModerate,
	daflCrossSensitiveLow,
};

//(s.dhole 3/10/2015 10:56 AM ) - PLID 64561 enum for medication lookup serach dropdown
enum MedicationResultColumns {
	mrcMedicationID = 0,
	mrcMedicationName = 1,
	mrcFirstDataBankID = 2,
	mrcFDBOutOfDate = 3, 
	mrcBackgroundColor,		//background row color
	mrcNexFormulary,
};


// (b.savon 2013-01-07 12:47) - PLID 54459 - Moved this to the utilitys
#define ERX_IMPORTED_COLOR RGB(255, 225, 206)
//TES 5/9/2013 - PLID 56614 - A different highlight color for records which are imported, but are out of date
#define ERX_IMPORTED_OUTOFDATE_COLOR	RGB(250,50,50) //Same del
#define ERX_NO_RESULTS_COLOR RGB(193, 193, 193)

#define NO_RESULTS_ROW -100

// (b.savon 2013-01-14 15:32) - PLID 54592
#define MAX_MED_SEARCH_ROW_HEIGHT 18

#define HIDE_SEARCH_RESULTS_TIMER 100 //milliseconds
// (b.savon 2013-01-29 18:19) - PLID 54919
#define START_SEARCH_RESULTS 250 //milliseconds

// (b.savon 2013-01-14 16:07) - PLID 54613
#define MAX_ALLERGY_SEARCH_ROW_HEIGHT 18

// (b.savon 2013-01-24 14:42) - PLID 54782
#define QUICK_LIST_WRITE_SELECTED_ROW -1
#define QUICK_LIST_CONFIGURE_ROW -2
#define QUICK_LIST_SUPERVISOR_ROW -10

// (b.savon 2013-01-15 12:12) - PLID 54632 - Move this to a utility
CSqlFragment GetNexERxUserRoles(long nUserID);
void GetNurseStaffMidlevelIdentifiers(CString strPrescribers, CString &strSupervising, CString &strMidlevel, ENurseStaffIdentifier nsiIdentifier);

// (b.savon 2013-01-23 10:22) - PLID 54758 - Moved this to a utility
void SplitNames(CString strDelimetedNames, CArray<CString, LPCTSTR> &arr, CString strDelimeter);

// (j.jones 2013-01-23 16:30) - PLID 53259 - added IsERxStatus, returns true
//if the given QueueStatus is pqseTransmitAuthorized, pqseTransmitSuccess, pqseTransmitError, or pqseTransmitPending
BOOL IsERxStatus(PrescriptionQueueStatus eStatus);

// (j.jones 2013-01-23 16:30) - PLID 53259 - added GetERxStatusFilter, returns a SQL fragment of
// comma-delimited {CONST} values with the known ERx statuses
CSqlFragment GetERxStatusFilter();

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
// (j.jones 2012-10-17 10:03) - PLID 53179 - UpdateHasNoAllergiesStatus will update and audit
// changes to PatientsT.HasNoAllergy. This function assumes this status has changed, and doesn't re-verify that fact.
void UpdateHasNoAllergiesStatus(BOOL bHasNoAllergies, long nPatientID, CTableChecker* pTableChecker = NULL);

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
// (j.jones 2012-10-17 13:08) - PLID 51713 - UpdateHasNoMedsStatus will update and audit changes to PatientsT.HasNoMeds.
// This function assumes this status has changed, and doesn't re-verify that fact.
void UpdateHasNoMedsStatus(BOOL bHasNoMeds, long nPatientID, CTableChecker* pTableChecker = NULL);

// (j.fouts 2013-02-05 14:40) - PLID 54463 - Created a utility function for displaying the sig/drug name
// (j.fouts 2013-04-22 10:26) - PLID 56155 - This now just uses the defined sig
CString GenerateDrugSigDisplayName(const CString& strDrugName,
								   const CString& strSig);

// (j.fouts 2013-02-05 14:40) - PLID 54463 - Created a utility function for generating a sig
CString GenerateSig(const CString& DosageQuantity, 
					   const CString& strDosageUnitSingular,
					   const CString& strDosageUnitPlural,
					   const CString& strDosageRoute, 
					   const CString& strDosageFrequency);

// (s.dhole 2013-08-28 10:33) - PLID 58000 
#define COLOR_ON_FORMULARY RGB(0,100,0)
#define COLOR_NON_FORMULARY RGB(0,0,0)
#define COLOR_ON_FORMULARY_CELL RGB(144,238,144)
#define COLOR_NON_FORMULARY_CELL RGB(255,255,255)
//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
// (j.jones 2016-01-21 08:52) - PLID 68020 - added option to filter out non-FDB drugs
LPUNKNOWN BindMedicationSearchListCtrl(CWnd *pParent, UINT nID, ADODB::_ConnectionPtr pDataConn, bool bFormulary, bool bIncludeFDBMedsOnly);
//(s.dhole 3/10/2015 5:25 PM ) - PLID 64564
// (j.jones 2016-01-21 09:18) - PLID 68021 - added an option to include only FDB allergies
LPUNKNOWN BindAllergySearchListCtrl(CWnd *pParent, UINT nID, ADODB::_ConnectionPtr pDataConn, bool bIncludeFDBAllergiesOnly);

// (j.jones 2016-01-21 11:34) - PLID 67970 - Added a warning used whenever a user toggles on the free-text search options.
// bIsAllergySearch is false for med searches, true for allergy searches.
// Returns true if the user still wants to continue.
bool ConfirmFreeTextSearchWarning(CWnd *pParent, bool bIsAllergySearch);
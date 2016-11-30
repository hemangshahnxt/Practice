#ifndef EMN_Medication_h
#define EMN_Medication_h

#pragma once

#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here

// (j.jones 2012-11-30 14:03) - PLID 53966 - moved the EMNMedication class to its own file

//TES 3/26/2009 - PLID 33262 - Used for medications to track their associated diagnosis codes, and any information
// that they might need so we don't have to go to the database to find it.
struct Med_DiagCodeInfo {
	long nID;
	CString strCodeNumber;
	CString strCodeDesc;
	long nSortOrder;
};

class EMNMedication {
public:
	long nID;
	long nMedicationID;
	//TES 2/10/2009 - PLID 33034 - Changed Description to PatientExplanation
	CString strPatientExplanation;
	CString strEnglishDescription;	// (j.jones 2010-05-07 11:05) - PLID 36062
	CString strRefillsAllowed;
	//TES 2/10/2009 - PLID 33034 - Changed PillsPerBottle to Quantity
	CString strQuantity;
	CString strUnit;
	
	// (z.manning 2009-02-26 16:40) - PLID 33141 - Use the new source action info class to keep track
	// of the source action/detail info for spawned medications.
	SourceActionInfo sai;
	CString m_strDrugName;	//This field is not changeable from the EMR

	// (j.jones 2008-05-20 09:25) - PLID 30079 - added other fields that the prescription editor uses
	long nProviderID;
	long nLocationID;
	long nPharmacyID;
	long nPatientID;

	// (a.walling 2009-07-01 12:01) - PLID 34052 - Supervisor and Agent support
	// (b.savon 2013-01-16 16:11) - PLID 54656 - Removed AgentID
	long nSupervisorID;

	CString strPatientName;
	COleDateTime dtPrescriptionDate;

	// (j.gruber 2009-03-30 11:27) - PLID 33736 - added newcropGUID
	CString strNewCropGUID;
	BOOL bIsDiscontinued;

	//TES 2/12/2009 - PLID 33002, 30043 - Added Strength, Dosage Form
	//TES 3/31/2009 - PLID 33750 - Removed DosageFormID, these are now just "read-only" fields.
	CString strStrength;
	CString strDosageForm;
	// (d.thompson 2009-04-02) - PLID 33571 - Added strength unit, also read only
	CString strStrengthUnit;

	// (s.dhole 2013-03-07 12:10) - PLID 55509 ADDED StrengthUnitID , DosageFormID ,QuantityUnitID
	long nStrengthUnitID;
	long nDosageFormID;
	long nQuantityUnitID;
	//TES 2/17/2009 - PLID 33140 - Added more SureScripts fields
	long nDaysSupply;
	CString strNoteToPharmacist;
	BOOL bAllowSubstitutions;
	CString strPriorAuthorization;
	BOOL bPriorAuthIsSample;
	// (j.fouts 2013-04-23 14:55) - PLID 55101 - Added Dosage Unit, Quantity, Route, and Frequency
	long nDosageUnitID;
	long nDosageRouteID;
	CString strDosageQuantity;
	CString strDosageFrequency;

	//TES 3/25/2009 - PLID 33262 - Track associated diagnoses
	CArray<Med_DiagCodeInfo,Med_DiagCodeInfo&> aryDiagnoses;

	//TES 5/11/2009 - PLID 28519 - Added SampleExpirationDate
	COleDateTime dtSampleExpirationDate;

	// (j.jones 2008-07-22 10:13) - PLID 30792 - added an array to track problems
	// (c.haag 2009-05-16 12:03) - PLID 34310 - We now track problem links instead of problems.
	CArray<CEmrProblemLink*, CEmrProblemLink*> m_apEmrProblemLinks;

	// (a.walling 2009-04-22 11:17) - PLID 33948 - Should this medication be E-Prescribed?
	BOOL bEPrescribe;
	// (a.walling 2009-04-22 14:41) - PLID 33948 - Will not be automatically sent unless it was reviewed
	BOOL bEPrescribeReviewed;

	//TES 8/3/2009 - PLID 35008 - Added DEASchedule
	CString strDEASchedule;

	// (z.manning 2009-08-18 09:54) - PLID 35207 - Added a pointer to an EMN that can be set to
	// use as the EMN for problem links in the assignment operator.
	CEMN *pEmnOverride;

	// (j.jones 2012-10-29 16:06) - PLID 53259 - added the E-Rx status
	PrescriptionQueueStatus eQueueStatus;

	EMNMedication();
	~EMNMedication();

	// (j.jones 2008-07-22 10:17) - PLID 30792 - required after I added m_aryEmrProblems
	// (z.manning 2009-08-18 10:15) - PLID 35207 - Moved the body of this function to EMN.cpp
	void operator =(EMNMedication &cSource);

	// (j.jones 2008-07-22 10:59) - PLID 30792 - returns true if there are any undeleted problems on the medication
	BOOL HasProblems();

	// (j.jones 2008-07-22 10:59) - PLID 30792 - returns true if there are only undeleted, closed problems on the medication
	BOOL HasOnlyClosedProblems();

	// (j.jones 2008-07-23 11:06) - PLID 30792 - returns true if any problems are marked as modified,
	// including deleted items
	BOOL HasChangedProblems();

	// (a.walling 2009-04-22 14:08) - PLID 34044 - Includes unsaved as well as modified problems
	BOOL HasUnsavedProblemLinks();

	// (j.jones 2013-01-07 15:50) - PLID 52819 - All medication changes should be saving
	// immediately using the API. Therefore very few - and eventually, no - places should
	// be referencing nor setting the changed flag.
	BOOL HasChanged_Deprecated();	//returns the value of bChanged
	void SetUnchanged();			//sets bChanged to FALSE
	void SetChanged_Deprecated();	//sets bChanged to TRUE

	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent, BOOL bDeleted);

protected:
	// (j.jones 2013-01-07 15:48) - PLID 52819 - Made bChanged private, because with the API
	// requirement of saving medications immediately, this boolean is mostly (but not entirely)
	// outdated. Its public facing functions are now properly labeled as _Deprecated.
	BOOL bChanged;
};

#endif
#include "stdafx.h"
#include "EMNMedication.h"
#include "EMN.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2012-11-30 14:03) - PLID 53966 - moved the EMNMedication class to its own file,
// and in turn moved more functions to be defined in a .cpp, not a .h

EMNMedication::EMNMedication()
{
	nID = -1;
	nMedicationID = -1;
	bChanged = FALSE;
	nProviderID = -1;
	nLocationID = -1;
	nPharmacyID = -1;
	nPatientID = -1;
	dtPrescriptionDate.SetStatus(COleDateTime::invalid);
	nDaysSupply = -1;
	bAllowSubstitutions = TRUE;
	bPriorAuthIsSample = FALSE;
	strNewCropGUID = "";
	bIsDiscontinued = FALSE;
	bEPrescribe = FALSE;
	bEPrescribeReviewed = FALSE;
	dtSampleExpirationDate.SetStatus(COleDateTime::invalid);
	// (a.walling 2009-07-01 12:01) - PLID 34052 - Supervisor and Agent support
	// (b.savon 2013-01-16 16:11) - PLID 54656 - Removed AgentID
	nSupervisorID = -1;
	pEmnOverride = NULL;
	// (j.jones 2012-10-29 16:06) - PLID 53259 - added the E-Rx status
	eQueueStatus = pqsIncomplete;
	// (s.dhole 2013-03-07 12:10) - PLID 55509 ADDED StrengthUnitID , DosageFormID ,QuantityUnitID
	nStrengthUnitID = -1;
	nDosageFormID = -1;
	nQuantityUnitID = -1;
	// (j.fouts 2013-04-23 14:55) - PLID 55101 - Added Dosage Unit, Quantity, Route, and Frequency
	nDosageUnitID = -1;
	nDosageRouteID = -1;
	strDosageQuantity = "";
	strDosageFrequency = "";
}

// (j.jones 2008-07-22 11:41) - PLID 30792 - remove all problems
// (c.haag 2009-05-19 12:01) - PLID 34310 - Problem links
EMNMedication::~EMNMedication()
{
	for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++) {
		delete m_apEmrProblemLinks[i];
	}
	m_apEmrProblemLinks.RemoveAll();
}

// (j.jones 2008-07-22 10:59) - PLID 30792 - returns true if there are any undeleted problems on the medication
// (c.haag 2009-05-19 12:02) - PLID 34310 - Use new problem link structure
BOOL EMNMedication::HasProblems()
{
	try {

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL && !pProblem->m_bIsDeleted) {

				return TRUE;
			}
		}

	}NxCatchAll("Error in EMNMedication::HasProblems");

	return FALSE;
}

// (j.jones 2008-07-22 10:59) - PLID 30792 - returns true if there are only undeleted, closed problems on the medication
// (c.haag 2009-05-19 12:03) - PLID 34310 - Use new problem link structure
BOOL EMNMedication::HasOnlyClosedProblems()
{
	try {

		BOOL bHasProblems = FALSE;
		BOOL bHasOnlyClosed = TRUE;

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL && !pProblem->m_bIsDeleted) {

				bHasProblems = TRUE;
				
				if(pProblem->m_nStatusID != 2) {
					bHasOnlyClosed = FALSE;
				}
			}
		}

		if(bHasProblems && bHasOnlyClosed) {
			return TRUE;
		}
		else {
			return FALSE;
		}

	}NxCatchAll("Error in EMNMedication::HasOnlyClosedProblems");

	return FALSE;
}

// (j.jones 2008-07-23 11:06) - PLID 30792 - returns true if any problems are marked as modified,
// including deleted items
// (c.haag 2009-05-19 12:04) - PLID 34310 - Use new problem link structure
BOOL EMNMedication::HasChangedProblems()
{
	try {

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL) {

				if(pProblem->m_bIsModified) {
					return TRUE;
				}
			}
		}

	}NxCatchAll("Error in EMNMedication::HasChangedProblems");

	return FALSE;
}

// (a.walling 2009-04-22 14:08) - PLID 34044 - Includes unsaved as well as modified problems
// (c.haag 2009-05-19 12:04) - PLID 34310 - Use new problem link structure
// (z.manning 2009-05-22 11:05) - PLID 34297 - HasUnsavedProblems was no longer needed so I
// changed this to HasUnsavedProblemLinks
BOOL EMNMedication::HasUnsavedProblemLinks()
{
	try
	{
		for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++)
		{
			CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(i);
			if(pProblemLink != NULL) {
				if(pProblemLink->GetID() == -1) {
					return TRUE;
				}
			}
		}

	}NxCatchAll("Error in EMNMedication::HasUnsavedProblemLinks");

	return FALSE;
}

// (z.manning 2009-08-18 10:10) - PLID 35207 - Moved the body of this function here from the .h
void EMNMedication::operator =(EMNMedication &cSource)
{
	nID = cSource.nID;
	nMedicationID = cSource.nMedicationID;
	strPatientExplanation = cSource.strPatientExplanation;
	// (j.jones 2010-08-23 16:10) - PLID 36062 - copy the strEnglishDescription
	strEnglishDescription = cSource.strEnglishDescription;
	strRefillsAllowed = cSource.strRefillsAllowed;
	strQuantity = cSource.strQuantity;
	strUnit = cSource.strUnit;
	bChanged = cSource.bChanged;
	sai = cSource.sai;
	m_strDrugName = cSource.m_strDrugName;
	nProviderID = cSource.nProviderID;
	nLocationID = cSource.nLocationID;
	nPharmacyID = cSource.nPharmacyID;
	nPatientID = cSource.nPatientID;

	// (a.walling 2009-07-01 12:01) - PLID 34052 - Supervisor and Agent support
	// (b.savon 2013-01-16 16:11) - PLID 54656 - Removed AgentID
	nSupervisorID = cSource.nSupervisorID;

	strPatientName = cSource.strPatientName;
	dtPrescriptionDate = cSource.dtPrescriptionDate;	
	nDaysSupply = cSource.nDaysSupply;
	strNoteToPharmacist = cSource.strNoteToPharmacist;
	bAllowSubstitutions = cSource.bAllowSubstitutions;
	strPriorAuthorization = cSource.strPriorAuthorization;
	bPriorAuthIsSample = cSource.bPriorAuthIsSample;
	strNewCropGUID = cSource.strNewCropGUID;
	bIsDiscontinued = cSource.bIsDiscontinued;
	// (d.thompson 2009-04-02) - PLID 33571 - added strength unit
	strStrengthUnit = cSource.strStrengthUnit;
	//TES 4/2/2009 - PLID 33002, 30043 - Added Strength, Dosage Form
	strStrength = cSource.strStrength;
	strDosageForm = cSource.strDosageForm;
	//TES 5/14/2009 - PLID 28519 - Added SampleExpirationDate
	dtSampleExpirationDate = cSource.dtSampleExpirationDate;
	// (s.dhole 2013-03-07 12:10) - PLID 55509 ADDED StrengthUnitID , DosageFormID ,QuantityUnitID
	nStrengthUnitID = cSource.nStrengthUnitID;
	nDosageFormID = cSource.nDosageFormID;
	nQuantityUnitID = cSource.nQuantityUnitID;
	// (c.haag 2009-05-19 10:03) - PLID 34310 - We now copy problem links
	for(int i = 0; i < cSource.m_apEmrProblemLinks.GetSize(); i++) {
		// (z.manning 2009-08-18 10:19) - PLID 35207 - When copying problem links, check and see if
		// we have another EMN pointer to use to associate with the problem links and if so, use that.
		CEMR *pOwningEmr = NULL;
		if(pEmnOverride != NULL) {
			pOwningEmr = pEmnOverride->GetParentEMR();
		}
		else {
			pEmnOverride = cSource.m_apEmrProblemLinks[i]->GetEMN();
		}
		CEmrProblemLink *pNewLink = new CEmrProblemLink(cSource.m_apEmrProblemLinks[i], pOwningEmr);
		pNewLink->UpdatePointersWithMedication(pEmnOverride, this);
		m_apEmrProblemLinks.Add(pNewLink);
	}

	for(i = 0; i < cSource.aryDiagnoses.GetSize(); i++) {
		aryDiagnoses.Add(cSource.aryDiagnoses[i]);
	}

	bEPrescribe = cSource.bEPrescribe;
	bEPrescribeReviewed = cSource.bEPrescribeReviewed;
	// (j.fouts 2013-04-23 14:55) - PLID 55101 - Added Dosage Unit, Quantity, Route, and Frequency
	nDosageUnitID = cSource.nDosageUnitID;
	nDosageRouteID = cSource.nDosageRouteID;
	strDosageQuantity = cSource.strDosageQuantity;
	strDosageFrequency = cSource.strDosageFrequency;
}

// (j.jones 2013-01-07 15:50) - PLID 52819 - All medication changes should be saving
// immediately using the API. Therefore very few - and eventually, no - places should
// be referencing nor setting the changed flag.

//this function should theoretically not be needed anymore
BOOL EMNMedication::HasChanged_Deprecated()
{
	return bChanged;
}

//sets bChanged to FALSE
void EMNMedication::SetUnchanged() {
	
	bChanged = FALSE;
}

//this function should theoretically not be needed anymore
void EMNMedication::SetChanged_Deprecated() {

	bChanged = TRUE;
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
void EMNMedication::LogEmrObjectData(int nIndent, BOOL bDeleted)
{
	// Log this object
	::LogEmrObjectData(nIndent, nID, this, esotPrescription, (nID == -1), bChanged, bDeleted, m_strDrugName,
		"strQuantity = %s  strEnglishDescription = %s  strUnit = %s  sourceActionID = %d  sourceDetailID = %d  sourceDataGroupID = %d  sourceDetailImageStampID = %d"
		, strQuantity
		, strEnglishDescription
		, strUnit
		, sai.nSourceActionID
		, sai.GetSourceDetailID()
		, sai.GetDataGroupID()
		, sai.GetDetailStampID()
	);

	// Log problems and problem links
	for (auto l : m_apEmrProblemLinks)
	{
		if (nullptr != l)
		{
			CEmrProblem* p = l->GetProblem();
			if (nullptr != p)
			{
				p->LogEmrObjectData(nIndent + 1);
			}
			if (nullptr != l)
			{
				l->LogEmrObjectData(nIndent + 1);
			}
		}
	}
}
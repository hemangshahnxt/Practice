//EMR.h

#ifndef EMR_H
#define EMR_H

#pragma once

#include "EmrUtils.h"
#include "EmrTopicWnd.h"
#include "ReconcileMedicationsDlg.h"

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

// (j.jones 2013-05-08 10:05) - PLID 56596 - removed some more .h files, turned them into classes declared here
class CEMN;
class CEmrEditorDlg;
class CEMNDetail;
class CProgressParameter;
class CShowProgressFeedbackDlg;
class CEMRTopic;
class CEMR;
class EMNProcedure;
class CEMNPreloadedDetails;
class CEMRLoader;
class CActionAndSource;
class CDoomedEmrObject;
struct SkippedTopic;
struct NewCropBrowserResult;
class CEMRHotSpot;
class CWoundCareMetCondition; // (r.gonet 08/03/2012) - PLID 51949
typedef CArray<CEMN*, CEMN*> CEMNArray;
class CEMNDetailArray;
struct TableSpawnInfo;

// (c.haag 2007-04-24 11:01) - PLID 25881 - We now use the official EMN loader
// object rather than the old preloaded detail array object
class CEMNLoader;
// (c.haag 2007-08-06 10:35) - PLID 26954 - We now use CEMNSpawner to preload
// spawning-related data in bulk
class CEMNSpawner;
// (c.haag 2008-07-09 13:31) - PLID 30648
class EMNTodo;

//DRT 9/17/2007 - PLID 27384 - This is used for loading the recordsets for all CEMN objects in an EMR, and passing down
//	to the individual EMNs.
// (c.haag 2008-07-18 15:37) - PLID 30784 - Added prsProblemInfo
//TES 3/26/2009 - PLID 33262 - Added prsMedDiagCodesInfo
// (j.gruber 2009-05-07 17:19) - PLID 33688 - added prsOtherProviderInfo;
class LoadEMNRecordsets_Objects {
public:
	ADODB::_RecordsetPtr prsEMNInfo;
	ADODB::_RecordsetPtr prsProcInfo;
	ADODB::_RecordsetPtr prsDiagInfo;
	ADODB::_RecordsetPtr prsChargeInfo;
	ADODB::_RecordsetPtr prsWhichCodesInfo;
	ADODB::_RecordsetPtr prsMedicationInfo;
	ADODB::_RecordsetPtr prsMedDiagCodesInfo;
	ADODB::_RecordsetPtr prsProviderInfo;
	ADODB::_RecordsetPtr prsSecProviderInfo;
	ADODB::_RecordsetPtr prsTechnicianInfo;		// (d.lange 2011-03-23 11:25) - PLID 42136 - added for Assistant/Technician field
	ADODB::_RecordsetPtr prsOtherProviderInfo;
	ADODB::_RecordsetPtr prsDetailInfo;
	// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
	ADODB::_RecordsetPtr prsDetailListOrderInfo; // (z.manning 2011-04-06 15:03) - PLID 43140
	ADODB::_RecordsetPtr prsTopicInfo;
	ADODB::_RecordsetPtr prsProblemInfo;
	ADODB::_RecordsetPtr prsLabInfo; // (z.manning 2008-10-06 15:23) - PLID 21094
	ADODB::_RecordsetPtr prsDetailImageStampInfo; // (z.manning 2010-02-17 16:28) - PLID 37412
	ADODB::_RecordsetPtr prsTopicPositionInfo; //TES 4/15/2010 - PLID 24692
	ADODB::_RecordsetPtr prsCommonLists; // (c.haag 2011-03-18) - PLID 42895 - Loading Common Lists for EMR items
	ADODB::_RecordsetPtr prsCommonListItems; // (c.haag 2011-03-18) - PLID 42895 - Loading Common List Items for EMR items
	ADODB::_RecordsetPtr prsEmrCodingGroupData; // (z.manning 2011-07-07 12:42) - PLID 44469

	bool AllAreEOF() {
		if(!prsEMNInfo->eof)
			return false;
		if(!prsProcInfo->eof)
			return false;
		if(!prsDiagInfo->eof)
			return false;
		if(!prsChargeInfo->eof)
			return false;
		if(!prsWhichCodesInfo->eof)
			return false;
		if(!prsMedicationInfo->eof)
			return false;
		if(!prsMedDiagCodesInfo->eof)
			return false;
		if(!prsProviderInfo->eof)
			return false;
		if(!prsSecProviderInfo->eof)
			return false;
		// (d.lange 2011-03-23 11:26) - PLID 42136
		if(!prsTechnicianInfo->eof)
			return false;
		// (j.gruber 2009-05-08 09:23) - PLID 33688 - Other Providers
		if(!prsOtherProviderInfo->eof)
			return false;
		if(!prsDetailInfo->eof)
			return false;
		// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
		if(!prsDetailListOrderInfo->eof) // (z.manning 2011-04-06 15:11) - PLID 43140
			return false;
		if(!prsTopicInfo->eof)
			return false;
		if(!prsProblemInfo->eof)
			return false;
		if(!prsLabInfo->eof) // (z.manning 2008-10-07 08:39) - PLID 21094
			return false;
		if(!prsDetailImageStampInfo->eof) // (z.manning 2010-02-17 16:29) - PLID 37412
			return false;
		if(!prsTopicPositionInfo->eof) //TES 4/15/2010 - PLID 24692
			return false;
		if(!prsEmrCodingGroupData->eof) // (z.manning 2011-07-07 12:42) - PLID 44469
			return false;

		//if we got here, all are eof
		return true;
	}
};

// (c.haag 2008-06-17 10:11) - PLID 17842 - This class is used to collect lists of details
// and topics that were added as a result of processing EMR actions
class CProcessEmrActionsResult
{
public:
	CArray<CEMNDetail*,CEMNDetail*> m_apNewDetails;
	CArray<CEMRTopic*, CEMRTopic*> m_apNewTopics;
	// (j.jones 2013-01-09 09:58) - PLID 45446 - added array for new prescription IDs
	CArray<long, long> m_aryNewPrescriptionIDs;
};

// (j.jones 2013-05-08 10:10) - PLID 56596 - turned into a proper class, so that CEMNDetailArray
// can be forward declared and still compile properly
class EMRTopicLoadInfo {
public:
	//Input parameters, these tell the thread how to load.
	long nID;
	BOOL bLoadFromTemplate; //This is pulling from an EMRTemplateTopicsT record.
	BOOL bLoadToTemplate;  //This is loading into a template topic.
	BOOL bIsNewTopic;
	//DRT 9/25/2007 - PLID 27515 - These match up with the SourceActionID
	long nOverrideSourceActionSourceID;
	long nOverrideSourceActionSourceDataGroupID;
	// (z.manning, 01/23/2008) - PLID 28690
	long nOverrideSourceActionSourceHotSpotGroupID;
	long nOverrideSourceActionSourceTableDropdownGroupID; // (z.manning 2009-02-13 09:17) - PLID 33070
	// (z.manning 2009-03-05 11:08) - PLID 33338
	SourceActionInfo m_saiOverride;

	BOOL bLoadHiddenTopics;
	BOOL bLoadSubTopics; // (c.haag 2007-08-08 11:14) - PLID 27014 - TRUE if we need to load subtopics
						// if there are any
	long nPatientID; //This is needed for loading the state of the details.
	long nEMRGroupID;	// (j.jones 2008-09-22 15:52) - PLID 31408 - also needed to load detail states
	//The thread will post an NXM_TOPIC_LOAD_COMPLETE message to this window when it's done.
	HWND hWnd;

	
	//Output parameters.  These are filled by the thread.
	long m_nTemplateTopicID;
	long m_nTemplateID;
	long m_nTemplateTopicOrderIndex;
	//EmrTemplateTopicsT.SourceTemplateTopicID, if !m_bIsTemplate, this may still be valid, as the source template topic for the topic represented by EmrTopicsT.EmrTemplateTopicID.
	long m_nOriginalTemplateTopicID;
	long m_nOriginalTemplateID; // (a.walling 2007-03-21 09:49) - PLID 25301 - The original templateID of a spawned topic

	//TES 4/15/2010 - PLID 24692 - This is maintained at the EMN level now.
	//long m_nTopicOrderIndex;

	// (z.manning 2011-02-11 17:36) - PLID 42446 - Changed the array's type to CEMNDetailArray
	// (j.jones 2013-05-08 10:26) - PLID 56596 - changed this into a reference, in order to not require its .h file
	CEMNDetailArray &m_aryEMNDetails;
	BOOL m_bDetailsCopied; //This is set to TRUE by some other class, when it takes responsibility for cleaning up the detail pointers.
	CArray<CEMRTopic*, CEMRTopic*> m_aryEMRTopics;
	BOOL m_bTopicsCopied; //This is set to TRUE by some other class, when it takes responsibility for cleaning up the topic pointers.

	//DRT 9/25/2007 - PLID 27515 - These match up with the SourceActionID
	long m_nSourceActionSourceID;
	long m_nSourceActionSourceDataGroupID;
	// (z.manning, 01/23/2008) - PLID 28690 - Added a group ID for hot spot based actions.
	long m_nSourceActionSourceHotSpotGroupID;
	long m_nSourceActionSourceTableDropdownGroupID; // (z.manning 2009-02-13 09:17) - PLID 33070
	CString m_strSourceActionName;
	EmrActionObject m_SourceActionDestType;
	// (z.manning 2009-03-05 11:05) - PLID 33338 - Use the new source action info class
	SourceActionInfo m_sai;
	
	BOOL m_bShowIfEmpty;

	//Should this template topic be included on EMNs?
	BOOL m_bHideOnEMN;

	CString m_strName;

	EmrTopicCompletionStatus m_CompletionStatus;
	
	// (a.walling 2008-06-30 12:45) - PLID 29271 - Preview Pane flags
	DWORD m_nPreviewFlags;

	//After all those variables are filled, this variable will be set to TRUE.
	BOOL m_bCompletelyLoaded;


	// (c.haag 2007-02-26 17:27) - PLID 24949 - Optionally maintain an array of
	// preloaded details so that the thread does not have to run queries to get
	// detail data.
	CEMNLoader* m_pLoader;

	//LoadEMRTopic() will lock this, and unlock it; once it unlocks it, that means it does not intend to access this variable
	//ever again, so CEMRTopic, which owns it, can delete it.
	CMutex m_mtxCanDelete;

	// (j.jones 2013-05-08 10:28) - PLID 56596 - moved the constructor and destructor to the .cpp
	// (j.jones 2013-05-08 10:26) - PLID 56596 - needs to fill the m_aryEMNDetails reference
	EMRTopicLoadInfo();
	~EMRTopicLoadInfo();
};

struct PendingAction {
	EmrAction ea;
	// (z.manning 2009-02-23 12:53) - PLID 33141 - Replaced the source detail pointer with the source action
	// info class.
	SourceActionInfo sai;
	//Was this action part of the initial loading of the template?
	BOOL bIsInitialLoad;
	CEMN* pSourceEmn;
	//TES 2/16/2010 - PLID 37375 - Added the source HotSpot
	CEMRHotSpot *pSourceSpot;
	BOOL bProcessed; // (z.manning 2011-03-04 17:23) - PLID 42336
};
 
struct LoadAllTopicsInfo
{
	CArray<EMRTopicLoadInfo*,EMRTopicLoadInfo*> arTopics;
	CMutex mtxTopics;
	//This is passed on to subtopics, so that they can be loaded from our same thread.
	CEMR *pEmr;
};

class CEMRItemAdvMultiPopupDlg;
class CEMR
{
public:	
	// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
	CEMR(BOOL bIsTemplate = FALSE, ADODB::_ConnectionPtr pDefaultConnection = NULL);
	~CEMR();
	
	// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
	ADODB::_ConnectionPtr GetRemoteData();

private:
	// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
	ADODB::_ConnectionPtr m_pDefaultConnection;

	// (c.haag 2009-05-19 08:42) - PLID 34277 - Prior to this item, EMR objects were responsible
	// for managing their own EMR problems. Now that we allow one EMR problem to be linked with
	// multiple EMR objects, those objects now store "problem links" rather than problems. However,
	// there is still the task of managing the problems themselves -- making sure they are not deleted
	// until they are no longer referenced, for instance. There's also the matter of needing to save
	// EMR problems and propagating new ID's for them; which requires coordination between all EMR objects.
	//
	// To address this issue, we make the CEMR object, being the root level object, responsible for
	// maintaining the master problem collection in memory. No EMR object can allocate or destroy a
	// problem without going through this CEMR.
	//
	CArray<CEmrProblem*, CEmrProblem*> m_apEmrProblemHeap;
public:
	// (c.haag 2009-05-20 10:56) - PLID 34277 - These functions must be used when creating a new CEMR problem
	// object from within an EMR object (except in special cases where EMNs don't have EMRs, like when loading
	// a patient's problem warning from the pt. module)
	// (z.manning 2009-05-27 10:10) - PLID 34297 - Added patient ID
	// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
	// (s.tullis 2015-02-23 15:44) - PLID 64723 
	// (r.gonet 2015-03-09 19:24) - PLID 64723 - Added DoNotShowOnProblemPrompt.
	CEmrProblem* AllocateEmrProblem(long nID, long nPatientID, CString strDescription, COleDateTime dtEnteredDate, COleDateTime dtModifiedDate, COleDateTime dtOnsetDate,
			long nStatusID, long nDiagCodeID_ICD9, long nDiagCodeID_ICD10, long nChronicityID, BOOL bIsModified, long nCodeID, BOOL bDoNotShowOnCCDA,
			BOOL bDoNotShowOnProblemPrompt);
	CEmrProblem* AllocateEmrProblem(ADODB::FieldsPtr& f);
	// (c.haag 2009-07-09 11:08) - PLID 34829 - Overload for getting fields from an existing problem
	CEmrProblem* AllocateEmrProblem(CEmrProblem* pProblem);

	// (c.haag 2009-05-20 10:58) - PLID 34277 - Returns an EMR problem given an ID. This does NOT reference count.
	CEmrProblem* GetEmrProblem(long nID);

	// (j.jones 2009-05-21 15:58) - PLID 34325 - recurses through all children and returns all problem links within the entire EMR,
	// or just the problem links that reference pFilterProblem, if pFilterProblem is not NULL
	// Links that have been deleted are not returned by default, unless bIncludeDeletedLinks is TRUE
	void GetAllProblemLinks(CArray<CEmrProblemLink*, CEmrProblemLink*> &aryProblemLinks, CEmrProblem *pFilterProblem = NULL, BOOL bIncludeDeletedLinks = FALSE);

	// (z.manning 2016-04-12 09:27) - NX-100140 - Gets the IDs for any problems that have been deleted
	std::set<long> GetDeletedEmrProblemIDs();

	// (c.haag 2014-07-17) - PLID 54905 - This fragment of LoadFromID is now contained in a string that can be used in other places if necessary
	CString AppendLoadSubQuery_rsChargesWhichCodes() const;

	//DRT 7/27/2007 - PLID 26836 - Added nEMNIDToBeDisplayed parameter.  See implementation for details.
	// (a.walling 2008-06-09 17:26) - PLID 22049 - Added param to reload a single EMN/Template
	void LoadFromID(long nID, BOOL bIsTemplate, long &nEMNIDToBeDisplayed, long nEMNIDToReload = -1);
	
	//Used by EMNs that have been loaded independently.
	// (j.jones 2007-08-02 11:42) - PLID 26915 - added ability to pass in a connection
	void LoadFromEmn(CEMN *pEMN, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	//TES 11/22/2010 - PLID 41582 - Added nPicID (-1 for templates and the various places that create temporary EMRs)
	void CreateNew(long nPatientID, BOOL bIsTemplate, long nPicID = -1);

	// (j.jones 2007-01-10 15:26) - PLID 24027 - supported SourceDetailID
	//TES 2/26/2007 - PLID 24768 - Took out the bForceAddAsNew parameter, if we want to call SetNew() we need to do it
	// after the EMN is completely loaded.
	// (z.manning, 04/12/2007) - PLID 25600 - Removed category ID as an optional parameter.
	// (j.jones 2007-07-17 09:45) - PLID 26702 - added pSourceEMN, for when we create dummy EMNs
	// for popups, we can query the source EMN for key data
	// (z.manning 2009-03-04 15:04) - PLID 33338 - Use the new source action info class
	// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking
	CEMN* AddEMNFromTemplate(long nTemplateID, SourceActionInfo &sai, OPTIONAL IN CEMN *pSourceEMN, long nAppointmentID);

	CEMN* AddNewTemplateEMN(long nCollectionID);
	void AddEMN(CEMN* pEMN);

	// (a.walling 2010-08-17 13:51) - PLID 38006 - Add an EMN without notifying
	void AddEMN_Internal(CEMN* pEMN);

	// (j.jones 2013-07-01 17:48) - PLID 57271 - identical to AddEMN_Internal except
	// that it sorts based on the EMRLoadEMNsSortOrder preference
	void AddEMNSorted_Internal(CEMN* pEMN, long nEMRLoadEMNsSortOrder);
	
	// (a.walling 2010-04-02 11:56) - PLID 37923 - An EMN will notify the EMR when its initial load is complete
	void PostInitialEMNLoadComplete(CEMN* pEMN);

	long GetID()
	{
		return m_nEMRID;
	}

	// (a.walling 2012-03-23 15:27) - PLID 49187
	long GetSafeID()
	{
		if (!this) return -1;
		return GetID();
	}

	// (a.walling 2008-06-27 15:03) - PLID 30482
	void OverrideID(long nNewID);

	int GetEMNCount();
	CEMN* GetEMN(int nIndex);
	CEMN* GetEMNByID(long nID);
	CEMN* GetEMNByPointer(long nPointer);

	// (a.walling 2012-10-04 12:41) - PLID 52878 - Verify that the given emr object pointer is a child
	CEMN* VerifyPointer(CEMN* pEMN);
	CEMRTopic* VerifyPointer(CEMRTopic* pTopic);
	CEMNDetail* VerifyPointer(CEMNDetail* pDetail);

	// (a.walling 2012-03-22 17:37) - PLID 49141 - const access to the array
	// (a.walling 2012-07-09 12:35) - PLID 51441 - non-const since we are already far too deep to start dealing with const-correctness at this point
	CEMNArray& GetAllEMNs()
	{
		return m_arypEMNs;
	}

	long GetPatientID();
	COleDateTime GetPatientBirthDate();
	BOOL GetIsTemplate();

	// (a.walling 2012-05-18 17:19) - PLID 50546 - inlined
	const CString& GetDescription()
	{
		return m_strDescription;
	}

	void SetDescription(const CString& strDescription);

	long GetStatus()
	{
		return m_nStatus;
	}

	void SetStatus(long nStatus)
	{
		if(m_nStatus != nStatus) {
			m_bIsUnsaved = TRUE;
		}

		m_nStatus = nStatus;
	}

	// (a.walling 2008-07-07 12:58) - PLID 30513 - Sets flag to ignore readonly modified EMNs
	void SetIgnoreReadOnly(BOOL bIgnore);
	inline BOOL SafeGetIgnoreReadOnly() {
		if (this == NULL) return FALSE;
		return m_bIgnoreReadOnlyEMNs;
	};

	//used when an item is changed from the administrator,
	//for all instances of that item in the entire EMR
	void MarkItemChanged(long nEMRInfoID);

	//refreshes all details currently loaded across the entire EMR
	// (j.jones 2007-07-26 09:23) - PLID 24686 - this is a horrible idea that should never occur
	//void RefreshAllItems();
	// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RefreshContent into two functions,
	// accepting an InfoID or a MasterID
	void RefreshContentByInfoID(long nEMRInfoID, BOOL bSyncContentAndState = FALSE);
	// (c.haag 2008-06-12 22:16) - PLID 27831 - Added papMasterDetails so that the caller may
	// optionally get a list of all the details with a matching master ID
	void RefreshContentByInfoMasterID(long nEMRInfoMasterID, BOOL bSyncContentAndState = FALSE,
		CArray<CEMNDetail*,CEMNDetail*>* papMasterDetails = NULL);
	// (a.wetta 2007-04-09 13:30) - PLID 25532 - This function refreshes the content
	// all all EMR items of a certain type.
	// (a.walling 2008-12-19 09:21) - PLID 29800 - This was only used for images, and only to refresh the custom stamps, which was causing the content
	// to be reloaded. This is all unnecessary, and the custom stamps is entirely UI. So let's just do what we need to do, and refresh the custom stamps,
	// rather than flag as needed to reload content. This is all controlled by the new bRefreshCustomStampsOnly param. I could have renamed the function
	// entirely, but I can see how this might come in handy in the future.
	void RefreshContentByType(EmrInfoType eitItemType, BOOL bSyncContentAndState = FALSE, BOOL bRefreshCustomStampsOnly = FALSE);

	//if editing a locked item creates a new copy, and we have unsaved items
	//using the old info item, make them use the new info item
	void UpdateInfoID(long nOldEMRInfoID, long nNewEMRInfoID, EMRInfoChangedIDMap* pChangedIDMap);

	//TES 10/9/2006 - PLID 22932 - We need to track which objects are already in our save string.
	// (j.jones 2007-01-11 14:28) - PLID 24027 - tracked strPostSaveSql, for sql statements to occur after the main save
	// (c.haag 2007-06-20 12:41) - PLID 26397 - We now store saved objects in a map for fast lookups.
	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
	// such as new or deleted prescriptions, or new or deleted diagnosis codes
	Nx::Quantum::Batch GenerateSaveString(long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynAffectedDetailIDs, OUT BOOL &bDrugInteractionsChanged, BOOL bSaveRecordOnly);
	BOOL PropagateNewID(long nID, EmrSaveObjectType esotSaveType, long nObjectPtr, long &nAuditTransactionID, CEMN** pFoundOnEMN);
	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent, BOOL bForceDeletedFlagTrue);

	// (a.walling 2010-03-31 11:23) - PLID 38006 - Now we need to ensure any cross-EMN IDs are updated
	void PropagateCrossEMNIDs();

	// (a.walling 2007-10-18 16:33) - PLID 27664 - Added array to gather all topics affected in the PostSaveUpdate cascade.
	void PostSaveUpdate(CShowProgressFeedbackDlg* pProgressDlg, BOOL bSaveRecordOnly = FALSE, CArray<CEMRTopic*, CEMRTopic*> *parTopicsAffected = NULL);
	// (z.manning 2009-06-15 17:05) - PLID 34332 - Moved the post save update logic for problems to 
	// its own function
	void PostSaveUpdateProblems();

	// (c.haag 2007-01-25 10:25) - PLID 24396 - Post-save handling for patient medications
	//DRT 8/15/2007 - PLID 26527 - Added pCurrentMedsDetail as a pointer so we don't have to query data.
	// (j.jones 2012-10-03 15:47) - PLID 36220 - if current meds. change, we will update bDrugInteractionsChanged
	//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
	void PostSaveUpdateCurrentMedications(CShowProgressFeedbackDlg* pProgressDlg, CEMNDetail *pCurrentMedsDetail, OUT BOOL &bDrugInteractionsChanged, IN OUT CDWordArray &arNewCDSInterventions);

	// (c.haag 2007-04-06 09:03) - PLID 25525 - When the EMR has been saved, we need
	// to check if the patient Allergies as defined by the most recent EMN is different
	// from the patient allergy list. If they are different, we must give the user the 
	// opportunity to change the patient allergy list to be consistent with the EMN.
	// (c.haag 2007-08-15 18:04) - PLID 25525 - We now take in the official detail so we don't have to query data
	// (j.jones 2012-10-03 15:47) - PLID 36220 - if allergies change, we will update bDrugInteractionsChanged
	void PostSaveUpdateAllergies(CShowProgressFeedbackDlg* pProgressDlg, CEMNDetail *pCurrentAllergiesDetail, OUT BOOL &bDrugInteractionsChanged);
	
	// (c.haag 2012-10-17) - PLID 52863 - This must be called after PropagateNewID for all ID's so that we can assign ID values to EMRTodosT.
	void UpdateUnsavedTodos();

	// (z.manning 2010-03-11 14:56) - PLID 37571 - Will reassasign the source detail stamp pointer to the given
	// new pointer for any object in this EMN that could have potentially been spawned.
	// (a.walling 2010-03-31 14:46) - PLID 38009 - Pass in the source EMN; if we are not the source EMN, we do not need to recurse into topics
	void UpdateSourceDetailStampPointers(EmrDetailImageStamp *pDetailStampOld, EmrDetailImageStamp *pDetailStampNew, CEMN* pSourceEMN);

	// (c.haag 2010-08-05 09:39) - PLID 39984 - This function will go through every EMN in arEMNs and see if 
	// any of their source detail pointers are pointing to a detail in another EMR. Each one that is found will be 
	// updated to its local counterpart.
	void UpdateEMNSourceDetailsFromCopy(const CArray<CEMN*,CEMN*>& arEMNs);

	//DRT 7/13/2007 - PLID 26671 - Get full procedure object, not just ID
	void GetProcedures(CArray<EMNProcedure, EMNProcedure> &arProcedureIDs);

	// (c.haag 2008-08-05 11:37) - PLID 30799 - Added bIsUnspawning. We don't allow manual removal
	// of EMN's with problems if the user doesn't have permission to do so. However, if the EMN is
	// removed via unspawning, then we don't do the permissions check.
	BOOL RemoveEMN(CEMN *pEMN, BOOL bIsEMNUnspawning = FALSE);
	BOOL RemoveEMNByID(long nEMNID);

	// (a.walling 2008-06-09 17:40) - PLID 22049 - Remove the EMN from the list but do not perform any specific actions
	// returns the index of the found EMN, -1 if failed
	long RemoveEMNByPointerRaw(CEMN *pEMN);

	// (a.walling 2008-06-10 09:21) - PLID 22049 - Reorder an EMN in the array
	void ReorderEMN(CEMN* pEMN, long nDesiredIndex);

	// (a.walling 2011-10-20 14:23) - PLID 46075 - Clean up interaction with external interfaces
	void SetInterface(class CEmrTreeWnd* pEmrTreeWnd);
	class CEmrTreeWnd* GetInterface() const
	{
		return m_pInterface;
	}

	//Pass TRUE for bIsInitialLoad to indicate that this action is being processed by the loading of this template, and 
	//therefore should not cause anything to be flagged as unsaved.
	// (c.haag 2007-04-25 11:32) - PLID 25774 - We now optionally pass in the loader object for faster
	// initial load processing
	void ProcessEMRInfoActions(CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader* pLoader = NULL);
	void ProcessEMRDataActions(long nEMRDataID, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader* pLoader = NULL);
	// (j.jones 2013-01-09 16:44) - PLID 54541 - added an ability to process actions for more than one EMRDataID at a time,
	// provided they are from the same detail (such as committing a popped up multi-select list)
	void ProcessEMRDataActions(CArray<long, long> &aryEMRDataIDs, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader* pLoader = NULL);
	// (z.manning, 01/30/2007) - PLID 24468 - Function to proccess all actions for given procedures.
	void ProcessEmrProcedureActions(CArray<long,long> &aryProcedureIDs, CEMN* pSourceEmn, BOOL bIsInitialLoad);
	void ProcessEmrProcedureActions(CArray<EMNProcedure*,EMNProcedure*> &aryProcedures, CEMN* pSourceEmn, BOOL bIsInitialLoad);
	// (z.manning, 01/22/2008) - PLID 28690 - Process hot spot actions
	//TES 2/15/2010 - PLID 37375 - Changed to take a CEMRHotSpot rather than just an ID
	// (z.manning 2011-07-25 12:15) - PLID 44649 - Changed the hot spot param to a pointer
	void ProcessEmrImageHotSpotActions(CEMRHotSpot *pSpot, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader* pLoader = NULL);
	// (r.gonet 08/03/2012) - PLID 51949 - Process actions associated with a set of met conditions found in Wound Care Coding.
	void ProcessEmrWoundCalculatorActions(CArray<CWoundCareMetCondition,CWoundCareMetCondition> &aryWoundCareConditions, CEMN* pSourceEmn, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader *pLoader = NULL);
	// (z.manning 2009-02-16 12:06) - PLID 33072 - Process table dropdown item actions
	void ProcessEmrTableDropdownItemActions(TableSpawnInfo tdsi, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader *pLoader = NULL);
	void ProcessEmrTableDropdownItemActions(CArray<long,long> &arynDropdownIDs, CEMNDetail *pSourceDetail, TableElement SourceTableElement, BOOL bIsInitialLoad, CEMNLoader* pLoader = NULL);
	// (z.manning 2010-03-01 15:06) - PLID 37571 - Smart stamp spawning
	void ProcessEmrSmartStampImageActions(TableRow *pSourceTableRow, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader *pLoader = NULL);

	// (c.haag 2008-07-21 14:21) - PLID 30725 - Processes problem actions which correspond to an action
	// that created a new EMN.
	// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
	void ProcessEmrProblemActions(const EmrAction& ea, SourceActionInfo &sai, CEMN* pNewEMN);
	// (c.haag 2008-07-21 14:22) - PLID 30725 - This function will spawn a single problem onto an EMR object
	void ProcessEmrProblemAction(const EmrAction& ea, const EmrProblemAction& epa, SourceActionInfo &sai,
									   CEMN* pNewEMN);
	// (c.haag 2008-07-21 14:27) - PLID 30725 - This function will spawn a problem onto an EMN
	void ProcessEmrProblemAction_eaoMint(const EmrProblemAction& epa, CEMN* pDestEMN);


	//DRT 8/3/2007 - PLID 26937 - Removed, use CEMNUnspawner from now on
	//void RevokeEMRInfoActions(CEMNDetail *pSourceDetail);
	//void RevokeEMRDataActions(long nEMRDataID, CEMNDetail *pSourceDetail);
	// (z.manning, 01/31/2007) - PLID 24486 - Function to revoke all actions for given procedures.
	void RevokeEMRProcedureActions(CArray<long,long> &aryProcedureIDs, CEMN* pSourceEmn);

	// (j.jones 2007-04-12 16:16) - PLID 25618 - removes any pending actions we may have
	void DeletePendingEMRActions(EmrAction ea, CEMNDetail *pSourceDetail);

	// (z.manning 2011-03-04 16:27) - PLID 42682
	void ClearAllPendingActions();

	// (z.manning 2011-03-04 14:40) - PLID 42682
	void UpdatePendingActionSourceDetailStampID(const long nOldDetailStampID, const long nNewDetailStampID, CEMN *pEmn);
	void UpdatePendingActionSourceDetailPointers(EmrDetailImageStamp *pOldDetailStamp, EmrDetailImageStamp *pNewDetailStamp, CEMN *pEmn);

	// (c.haag 2008-08-01 16:27) - PLID 30897 - This function gathers a list of all EMN objects that will be removed
	// from an EMR as a result of one or more unspawning actions, and stores the EMN object pointer and action-pertinent 
	// information in adeoDoomedObjects. That array will later be used in the actual removing of those EMN's.
	// (a.walling 2014-07-14 16:32) - PLID 62812 - use MFCArray
	void GetEmrObjectsToRevoke_eaoMint(MFCArray<CActionAndSource> *paryActionsToRevoke, CArray<CDoomedEmrObject, CDoomedEmrObject&>& adeoDoomedObjects);

	// (z.manning, 04/23/2007) - PLID 24586 - Made this function public so that other classes can access it to
	// unspawn EMNs (action type = eaoMint).
	// (c.haag 2008-08-07 09:50) - PLID 30979 - This function has been deprecated
	//void RevokeEmrAction(EmrAction ea, CEMNDetail *pSourceDetail, CEMN* pSourceEmn = NULL);
	//DRT 8/7/2007 - PLID 27003 - Original source pulled out of RevokeEmrAction
	// (c.haag 2008-08-01 16:30) - PLID 30897 - This function no longer searches for objects to delete. Those
	// objects are now passed into this function in adeoDoomedObjects
	void RevokeEmrActions_eaoMint(const CArray<CDoomedEmrObject, CDoomedEmrObject&>& adeoDoomedObjects);

	//TES 1/24/2007 - PLID 24377 - This function finds any details on this EMR that are linked to the given detail, and 
	// applies those links to the detail as appropriate.
	// This function will only have an effect on multi-select lists, and will only check items, never uncheck them.
	void ApplyEmrLinks(CEMNDetail *pDetail);

	//Call this if you're doing something, like adding a topic, which might cause a bunch of actions to happen.
	//While spawning is locked, the EMR will keep track of all the items it should be spawning, and when spawning is unlocked,
	//it will go through and process them all in the order they came in.
	void LockSpawning();
	void UnlockSpawning();

	//When processing an action, don't let it be processed again as a result.
	void LockAction(const EmrAction &ea);
	void UnlockAction(const EmrAction &ea);
	BOOL IsActionLocked(const EmrAction &ea);

	// (j.jones 2007-03-13 17:21) - PLID 25193 - lets the caller know if we have any pending actions
	BOOL GetHasFinishedSpawning();

	// (z.manning 2008-12-12 14:06) - PLID 32427 - Returns true if spawning is unlocked, false if it's locked
	BOOL IsSpawningUnlocked();

	// (a.wetta 2007-01-31 10:03) - PLID 24369 - The following functions are used to ensure that an EMR detail's
	// pointer will not get deleted while it is still being used.  To prevent a detail pointer from getting deleted
	// a reference must be added to the pointer.  As long as it has that reference, it will not be deleted.  But, this
	// protection is only in place if the DeleteEMNDetail() function is used instead of the traditional delete 
	// keyword.  So, when protecting a detail pointer be sure to use DeleteEMNDetail() when deleting the pointer.
	void AddEMNDetailReference(CEMNDetail *pDetail);
	void RemoveEMNDetailReference(CEMNDetail *pDetail);
	BOOL IsEMNDetailReferenced(CEMNDetail *pDetail);
	void DeleteEMNDetail(CEMNDetail *pDetail);

	//We want to ignore actions iff we're loading an existing EMN.
	BOOL m_bIgnoreActions;

	//TES 5/20/2008 - PLID 27905 - This is sometimes called when procedures are about to be deleted; if so, pass in the EMN
	// they're being deleted from and this will only return true if the procedure is on some other EMN.
	BOOL IsProcedureInEMR(long nProcedureID, CEMN* pExcludingEmn = NULL);

	BOOL IsEMRUnsaved();

	// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
	// and returns TRUE if any Image detail on this EMR references it
	BOOL IsImageFileInUse(const CString strFileName);
	
	//Tell the EMR that it has been saved
	void SetSaved(BOOL bIsPostLoad = FALSE);
	// (j.jones 2008-07-22 16:05) - PLID 30789 - added SetUnsaved
	void SetUnsaved();

	// (j.jones 2012-01-31 16:21) - PLID 47878 - added a bIsPostLoad parameter
	void SetSavedNoPropagate(BOOL bIsPostLoad = FALSE);

	//Used to add a topic to our queue of asynchronously loaded topics.
	// (z.manning 2010-08-20 09:42) - PLID 40190 - Added connection pointer parameter
	void LoadTopic(ADODB::_Connection *lpCon, EMRTopicLoadInfo *pEtli);
	//A topic will pass itself in here when it needs to be loaded as soon as possible, this function won't return until it's loaded.
	void LoadTopicImmediate(EMRTopicLoadInfo *pTopic);

	// (m.hancock 2006-10-06 17:00) - PLID 22302 - Audit closing an unsaved EMN.
	void AuditCloseUnsaved();

	//TES 1/23/2007 - PLID 24377 - Used when processing EMR Links, this function returns TRUE if any one of the EmrDataT.IDs
	// in the array is selected on any detail in the EMR, otherwise it returns FALSE.
	//TES 2/6/2007 - PLID 24377 - arDetailsToIgnore is passed in when processing a batch of details, which we don't want
	// to link to each other.  The function will only return TRUE if one of the data IDs is checked on a detail that is NOT
	// in arDetailsToIgnore.
	BOOL IsAnyItemChecked(const CArray<long,long> &arDataIDs, const CArray<CEMNDetail*,CEMNDetail*> &arDetailsToIgnore);

	// (c.haag 2011-05-19) - PLID 43696 - Populates a map with all EmrDataID's that correspond to checked-off single-select
	// and multi-select list items. All details in mapDetailsToIgnore are ignored during the search.
	void GetAllCheckedItems(CMap<long,long,BOOL,BOOL>& mapDataIDs, const CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> &mapDetailsToIgnore);

	// (z.manning, 02/01/2007) - PLID 24524 - Functions to show/hide the action progress bar.
	void ShowActionProgressBar(UINT nShow);

	// (j.jones 2007-02-06 13:45) - PLID 24509 - allow to set/get StartEditingTime
	void TryStartTrackingTime();	
	void TryStopTrackingTime();
	COleDateTime GetStartEditingTime();

	// (j.jones 2012-01-31 15:13) - PLID 47878 - HasChangesMadeThisSession will return TRUE
	// if m_bChangesMadeThisSession is true for the EMR *OR* for any EMN on the EMR.
	BOOL HasChangesMadeThisSession();

	// (j.jones 2012-01-31 15:30) - PLID 47878 - added ability to set m_bChangesMadeThisSession from a caller
	void SetChangesMadeThisSession(BOOL bChangesMade);

	// (j.jones 2007-09-17 17:35) - PLID 27396 - update the server offset
	void UpdateServerTimeOffset(COleDateTimeSpan dtOffset);
	// (j.jones 2007-09-17 17:35) - PLID 27396 - return the server offset
	COleDateTimeSpan GetServerTimeOffset();

	// (j.jones 2007-07-20 14:55) - PLID 26742 - grab the cached value from the EMR
	long GetCurrentMedicationsInfoID(ADODB::_Connection *lpCon = NULL);
	long GetCurrentAllergiesInfoID(ADODB::_Connection *lpCon = NULL);
	// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
	long GetCurrentGenericTableInfoID(ADODB::_Connection *lpCon = NULL);

	// (j.jones 2007-07-23 10:54) - PLID 26742 - be able to update the cached value from an external source
	void SetCurrentMedicationsInfoID(long nNewInfoID);
	void SetCurrentAllergiesInfoID(long nNewInfoID);
	// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
	void SetCurrentGenericTableInfoID(long nNewInfoID);

	// (z.manning, 08/15/2007) - PLID 26809 - Added an accessor for the load topics info object.
	LoadAllTopicsInfo* GetLoadAllTopicsInfo();

	// (j.jones 2008-07-21 17:28) - PLID 30729 - add all of this EMR's problems
	// and all of the EMNs' problems to the passed-in array
	// (c.haag 2008-08-14 12:05) - PLID 30820 - Added bIncludeDeletedProblems
	void GetAllProblems(CArray<CEmrProblem*, CEmrProblem*> &aryProblems, BOOL bIncludeDeletedProblems = FALSE);

	// (j.jones 2008-07-22 10:11) - PLID 30789 - added problem class, and an array to track problems
	// (c.haag 2009-05-16 12:03) - PLID 34277 - We now track problem links instead of problems.
	CArray<CEmrProblemLink*, CEmrProblemLink*> m_apEmrProblemLinks;

	// (j.jones 2008-07-22 10:11) - PLID 30789 - returns true if there are any undeleted problems on the EMR
	BOOL HasProblems();
	// (j.jones 2008-07-22 10:11) - PLID 30789 - returns true if there are only undeleted, closed problems on the EMR
	BOOL HasOnlyClosedProblems();
	// (j.jones 2008-07-23 16:54) - PLID 30789 - returns true if any problems are marked as modified,
	// including deleted items
	BOOL HasChangedProblems();
	// (c.haag 2008-07-24 09:49) - PLID 30826 - Returns TRUE if there is at least one saved problem for this EMR or any of
	// its children. This does not check deleted EMR objects.
	BOOL DoesEmrOrChildrenHaveSavedProblems();
	// (z.manning 2009-05-21 12:11) - PLID 34297 - Returns true if there is at least one unsaved problem for this EMR.
	BOOL DoesEmrOrChildrenHaveUnsavedProblems();
	// (z.manning 2009-06-30 14:44) - PLID 34340
	BOOL DoesEmrOrChildrenHaveUnsavedProblemLinks();

	// (a.walling 2013-07-23 21:36) - PLID 57686 - CEMR::m_mapDropdownIDToDropdownGropupID (sic) no longer necessary; we already have this in Nx::Cache. 
	// (j.dinatale 2012-07-13 12:25) - PLID 51481 - need this for determining if save should be called
	bool HasValidInfo();

protected:
	// (c.haag 2010-01-14 12:14) - PLID 26036 - We now track all the times the "EMR clock" was paused and resumed
	// in SERVER time.
	CArray<COleDateTime,COleDateTime&> m_adtServerPauseStartTimes;
	CArray<COleDateTime,COleDateTime&> m_adtServerPauseEndTimes;

protected:
	// (c.haag 2010-01-25) - PLID 42222 - This map is populated after the NewCrop browser is dismissed after the
	// user elects to reconcile current medications with prescriptions. Any current medications they wish to create
	// will have a corresponding NewCrop GUID; we need to hang on to that until the save is done. The map key is
	// an EmrDataID, the map value is the NewCrop GUID. This map is emptied in PostSaveUpdateCurrentMedications.
	CMap<long,long,CString,LPCTSTR> m_mapNewEMRDataIDToNewCropGUID;

public:
	// (s.dhole 2013-06-21 16:26) - PLID   57219 Restructure routin
	void AddReconciledMedChange(CReconcileMedicationsDlg::MergeMedication *pChange); // (z.manning 2011-09-22 14:31) - PLID 45623

protected:

	long m_nEMRID;

	//TES 11/22/2010 - PLID 41582 - We need to store our PicID, so we can save it at the same time as we save the EMR (if it's new).
	long m_nPicID;

	// (a.walling 2011-10-20 14:23) - PLID 46075 - Clean up interaction with external interfaces
	class CEmrTreeWnd* m_pInterface;

	long m_nPatientID;
	COleDateTime m_dtPatientBirthDate;
	BOOL m_bIsTemplate;

	BOOL m_bIsUnsaved;

	//Are we responsible for deleting our child EMNs?
	BOOL m_bOwnChildren;

	CArray<CEMN*, CEMN*> m_arypEMNs;
	CArray<CEMN*, CEMN*> m_aryDeletedEMNs;
	CArray<CEMN*, CEMN*> m_aryPendingDeleteEMNs;

	CString m_strDescription;
	long m_nStatus;

	// (j.jones 2007-03-13 17:00) - PLID 25193 - track whether or not we are processing locked spawns
	long m_nUnlockSpawnsRefCount;

	// (j.jones 2006-08-18 14:25) - PLID 22096 - for Auditing purposes,
	// store the last saved status and description
	long m_nLastSavedStatus;
	CString m_strLastSavedDescription;

	int m_nSpawnLocks;
	CArray<PendingAction*,PendingAction*> m_arPendingActions;

	// (a.walling 2007-10-18 12:01) - PLID 25548 - this is the depth of recursive ProcessEmrActions calls.
	long m_nProcessEmrActionsDepth;
	// (a.walling 2007-10-18 12:01) - PLID 25548 - this is called at the end of every ProcessEmrActions function. If the depth is zero,
	// it sends a message to the treewnd that the actions are completely processed.
	void ProcessEmrActionsComplete();
	// (a.walling 2007-10-26 09:05) - PLID 25548 - Helper functions to increase and decrease the EmrActions depth
	// and call ProcessEmrActionsComplete when 0
	void IncreaseEmrActionsDepth();
	void DecreaseEmrActionsDepth();

	//TES 1/24/2007 - PLID 24377 - LockSpawning() will also lock the processing of EMR links; these details are ones
	// we need to apply links to once the spawning is unlocked.
	CArray<CEMNDetail*,CEMNDetail*> m_arDetailsToBeLinked;
	// (a.walling 2009-10-12 18:01) - PLID 36024 - Maintaining reference counts on details to be linked
	void ClearDetailsToBeLinked();

	//Processes the actions, and shows a progress bar.
	// (z.manning, 01/31/2007) - PLID 24468 - Not all actions have a source detail, so added a parameter for source EMN.
	// (c.haag 2008-06-17 10:12) - PLID 17842 - Added an optional parameter to allow the caller to track what was spawned
	//TES 2/15/2010 - PLID 37375 - Added an optional parameter for the source HotSpot
	// (z.manning 2010-02-24 17:16) - PLID 37532 - Replaced nSourceDataGroupID with ptrSourceTableRow
	void ProcessEmrActions(MFCArray<EmrAction> &arActions, CEMNDetail *pSourceDetail, TableRow *ptrSourceTableRow, BOOL bIsInitialLoad, CEMRHotSpot *pSourceSpot = NULL, CEMN* pSourceEmn = NULL,
		CProcessEmrActionsResult* pEmrActionsResult = NULL);
	// (z.manning 2009-02-24 09:32) - PLID 33138 - Changed pSourceDetail to source action info
	void RevokeEmrActions(MFCArray<EmrAction> &arActions, SourceActionInfo &sai, CEMN* pSourceEmn = NULL);
	// (c.haag 2008-06-17 10:12) - PLID 17842 - Added an optional parameter to allow the caller to track what was spawned
	// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
	void ProcessEmrAction(EmrAction ea, SourceActionInfo &sai, CArray<SkippedTopic,SkippedTopic&> &arSkippedTopics, BOOL bIsInitialLoad, CProgressParameter *pProgress = NULL, CEMN* pSourceEmn = NULL,
		CEMNSpawner* pEMNSpawner = NULL, CProcessEmrActionsResult* pEmrActionsResult = NULL);

	// (j.jones 2013-01-09 11:48) - PLID 45446 - opens medication reconciliation as a result of spawning multiple prescriptions
	void ReconcileCurrentMedicationsWithSpawnedPrescriptions(CEMN *pEMN, CArray<long, long> &aryNewPrescriptionIDs);

	CArray<long,long> m_arLockedActionIDs;

	CShowProgressFeedbackDlg *m_pActionProgressDlg;
	int m_nCurrentProgress, m_nProgressMax;

	// (j.jones 2012-08-22 09:08) - PLID 50486 - added an insured party ID
	// to use for new charges, defaults to Unknown (-3) for not calculated,
	// NoDefault (-2) for no default, PatientResp (-1) for patient,
	// and then the actual insured party ID if we set a default
	long m_nDefaultChargeInsuredPartyID;

	// (j.jones 2012-10-25 17:18) - PLID 53322 - Tracks that we tried to save allergies, and could have updated
	// the patient account. We may change nothing, and may not actually change the account. But we track that we
	// tried, such that we can prompt later to confirm that a blank allergy list was confirmed to be intentional.
	BOOL m_bHadSavedAllergies;

	// (j.jones 2012-10-29 08:56) - PLID 53324 - Tracks that we tried to save current medications, and could
	// have updated the patient account. We may change nothing, and may not actually change the account.
	// But we track that we tried, such that we can prompt later to confirm that a blank current medications
	// list was confirmed to be intentional.
	BOOL m_bHadSavedCurrentMeds;

public:

	// (j.jones 2012-08-22 09:08) - PLID 50486 - If m_nDefaultChargeInsuredPartyID is Unknown (-3)
	// then this will check the EMNChargeRespDefault preference and fill m_nDefaultChargeInsuredPartyID.
	// Once filled, this function will always return m_nDefaultChargeInsuredPartyID.
	long GetDefaultChargeInsuredPartyID();

	// (j.jones 2012-10-25 17:18) - PLID 53322 - returns m_bHadSavedAllergies
	BOOL GetHadSavedAllergies();

	// (j.jones 2012-10-29 08:55) - PLID 53324 - returns m_bHadSavedCurrentMeds
	BOOL GetHadSavedCurrentMeds();

protected:

	//This thread runs as long as we are in scope, its purpose is to pull topics out of a global array of topics that we will 
	//set up, which need to be loaded, load them, and send messages to the associated window that the topic was loaded.  Also,
	//if necessary, we need to be able to tell this thread to load a specific topic immediately.

	// (a.walling 2010-07-23 10:30) - PLID 39834 - Turning this into its own class for more robustness - now uses a message-only
	// window internally to prevent losing threadmessages due to unexpected modal loops
	class CLoadTopicsThread : public CWinThread {
	public:
		CLoadTopicsThread(CEMR* pEMR);
		virtual ~CLoadTopicsThread();
		
		LoadAllTopicsInfo m_LoadTopicsInfo;

		// (a.walling 2010-07-23 10:30) - PLID 39834 - Simple overload to post a message to the message-only window. Now using this
		// instead of PostThreadMessage since we can guarantee the windowproc will process the message.
		LRESULT PostMessage(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

	protected:
		static UINT ThreadProc(LPVOID pParam);

		virtual UINT RunThread();

		// (a.walling 2010-07-23 10:30) - PLID 39834 - Used by CLoadTopicsThread::PostMessage to ensure we don't post anything
		// before the window (and message pump) is ready.
		CEvent m_eventWindowReady;

		// (a.walling 2010-07-23 10:30) - PLID 39834 - Our message-only window which handles all the messages posted to us
		class CLoadTopicsMessageWindow : public CWnd {
		public:
			CLoadTopicsMessageWindow(LoadAllTopicsInfo& loadTopicsInfo);

			ADODB::_ConnectionPtr m_pConn;

		protected:
		
			LoadAllTopicsInfo& m_LoadTopicsInfo;

			//Keep track of whether we're actively loading topics.
			bool m_bLoadingTopics;

			LRESULT OnLoadTopics(WPARAM wParam, LPARAM lParam);
			LRESULT OnLoadNextTopic(WPARAM wParam, LPARAM lParam);
			LRESULT OnLoadTopicImmediate(WPARAM wParam, LPARAM lParam);
			LRESULT OnForciblyClose(WPARAM wParam, LPARAM lParam);
	
			DECLARE_MESSAGE_MAP();
		};

		CLoadTopicsMessageWindow m_wndMessageWindow;
	};

	//CWinThread *m_pLoadTopicsThread;
	CLoadTopicsThread* m_pLoadTopicsThread;

	//We pass this into m_pLoadTopicsThread.
	//LoadAllTopicsInfo *m_pLoadTopicsInfo;

	//TES 1/24/2007 - PLID 24377 - This is just like ApplyEmrLinks(), indeed ApplyEmrLinks() just calls this function.
	// The reason for this function, however, is to ensure that details which are added simultaneously don't link to 
	// each other, any items checked while processing this array will not cause other items in this array to be checked.
	void ApplyEmrLinksToBatch(const CArray<CEMNDetail*,CEMNDetail*> &arDetails);

	CArray<CEMNDetail*, CEMNDetail*> m_arypReferencedEMNDetails;
	CArray<CEMNDetail*, CEMNDetail*> m_arypPendingDetailsToDelete;
	BOOL IsEMNDetailPendingForDeletion(CEMNDetail *pDetail);
	// (a.walling 2009-10-12 18:07) - PLID 36024 - Maintaining reference counts on details pending deletion
	void RemoveEMNDetailFromPendingDeletion(CEMNDetail *pDetail);

	// (j.jones 2007-02-06 13:45) - PLID 24509 - track the StartEditingTime
	BOOL m_bIsTrackingTime;
	COleDateTime m_dtStartEditingTime;
	BOOL m_bChangesMadeThisSession;

	// (c.haag 2007-05-21 13:19) - PLID 26084 - This event is set in the EMR's
	// destructor
	HANDLE m_hevShuttingDown;

	// (j.jones 2007-07-20 14:44) - PLID 26742 - cached these IDs per EMR
	long m_nCurrentMedicationsInfoID;
	long m_nCurrentAllergiesInfoID;
	// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
	long m_nCurrentGenericTableInfoID;

	// (j.jones 2007-09-17 17:30) - PLID 27396 - store the offset between the workstation time and the server time
	COleDateTimeSpan m_dtServerTimeOffset;

	//TES 1/16/2008 - PLID 24157 - Finds any open multipopupdlg (there should only be one) and returns a pointer to it.
	// Returns NULL if none of our EMNs have an open multipopupdlg.
	CEMRItemAdvMultiPopupDlg* GetOpenMultiPopupDlg();

	// (a.walling 2008-07-07 12:59) - PLID 30513 - Flag whether to ignore readonly modified EMNs
	BOOL m_bIgnoreReadOnlyEMNs;
	BOOL m_bLastSavedIgnoreReadOnlyEMNs;

	// (a.walling 2013-07-23 21:36) - PLID 57686 - CEMR::m_mapDropdownIDToDropdownGropupID (sic) no longer necessary; we already have this in Nx::Cache. 

protected:
	// (c.haag 2008-06-26 12:46) - PLID 27549 - This constitutes the bulk of code that existed in CEMR::UnlockSpawning.
	// We pass in a reference to an array of pending actions, and optionally a pointer to a CEMNLoader object if we
	// happen to be spawning mint items such that the source detail itself is a spawned mint item; it's necessary for
	// proper topic positioning.
	void ProcessPendingActions(CArray<PendingAction*,PendingAction*>& arPendingActions,
									 CEMNLoader* pEMNSpawnedMintLoader = NULL);

public:
	// (c.haag 2008-06-26 12:11) - PLID 27549 - Called from ProcessEmrAction_eaoMintItems to search for
	// any spawned multi-select details that were checked by default, and themselves spawn mint items;
	// and then spawn those mint items using the proper topic hierarchy.
	void DetectAndSpawnChildMintItems(CEMNLoader* pEMNSpawnedMintLoader,
									CArray<CEMRTopic*,CEMRTopic*>& apNewTopics);

public:
	// (c.haag 2008-07-09 13:15) - PLID 30648 - Appends apTodos with this EMR's list of todos
	// created prior to their corresponding entities having been saved
	void GenerateCreatedTodosWhileUnsavedList(CArray<EMNTodo*,EMNTodo*>& apTodos);

	// (c.haag 2008-07-09 13:15) - PLID 30648 - Appends apTodos with this EMR's list of todos
	// deleted prior to their corresponding entities having been saved
	void GenerateDeletedTodosWhileUnsavedList(CArray<EMNTodo*,EMNTodo*>& apTodos);

// (c.haag 2008-07-23 12:16) - PLID 30820 - Populate apProblems with a list of all deleted problems for this object and
// all its children. If a child or related EMR object is deleted, all its problems are considered deleted as well.
	// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now, not problems
	void GetAllDeletedEmrProblemLinks(CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks, BOOL bIncludeThisObject);

public:
	// (c.haag 2009-06-08 09:23) - PLID 34398 - This function populates apDoomedProblems with a list
	// of all EMR problems that are either deleted, or are not associated with any undeleted problem links.
	void FindEmrProblemsToDelete(const CArray<CEmrProblem*,CEmrProblem*>& apProblems,								   
								   const CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinksToDelete,
								   CArray<CEmrProblem*,CEmrProblem*>& apDoomedProblems,
								   BOOL bProblemsInDataOnly);
	// (z.manning 2016-04-12 10:20) - NX-100140 - Added version that doesn't take in the current problems and links
	// since the EMR already has them all.
	void FindAllEmrProblemsToDelete(CArray<CEmrProblem*, CEmrProblem*>& apDoomedProblems, BOOL bProblemsInDataOnly);

public:
	HANDLE GetShutdownEvent() { return m_hevShuttingDown; }

public:
	// (c.haag 2010-01-14 11:28) - PLID 26036 - Called to "pause" the clock timer that tracks how long
	// someone has had the chart open
	void PauseClock(const COleDateTime& dtServerPauseTime);

	// (c.haag 2010-01-14 11:28) - PLID 26036 - Called to "resume" the clock timer that tracks how long
	// someone has had the chart open
	void ResumeClock(const COleDateTime& dtServerResumeTime);
};

#endif
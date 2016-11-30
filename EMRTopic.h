//EMRTopic.h

#ifndef EMRTOPIC_H
#define EMRTOPIC_H

#pragma once

#include "EmrUtils.h"

// (j.jones 2013-05-16 10:57) - PLID 56596 - removed a number of .h files

class CEMN;
class CEmrTopicWnd;
class EMRTopicLoadInfo;
class CEMR;
class CEMNPreloadedDetails;
class CEmrDebugDlg;
class EMNTodo;
class CEmrProblemLink;
class CEMRTopic;
class CEMNDetailArray;
class CEMNSpawner;
class CEmrTopicHeaderWnd;
typedef shared_ptr<CEmrTopicWnd> CEmrTopicWndPtr;
typedef shared_ptr<CEmrTopicHeaderWnd> CEmrTopicHeaderWndPtr;
typedef weak_ptr<CEmrTopicWnd> CEmrTopicWndRef;
typedef weak_ptr<CEmrTopicHeaderWnd> CEmrTopicHeaderWndRef;

// (j.jones 2013-05-08 09:27) - PLID 56596 - moved CTopicArray to EMRUtils.h

// (c.haag 2007-04-24 11:01) - PLID 25881 - We now use the official EMN loader
// object rather than the old preloaded detail array object
class CEMNLoader;

struct GlassesOrder;
struct ContactLensOrder;

// (z.manning 2009-03-05 09:05) - PLID 33338 - Moved this here from EmrUtils
struct SkippedTopic
{
	long nTemplateTopicID;
	long nParentTemplateTopicID;
	long nOrderIndex;
	// (z.manning 2009-03-05 09:03) - PLID 33338 - Use the new source action info class
	SourceActionInfo sai;
	//Was this topic skipped during the initial load of this template?
	BOOL bIsInitialLoad;
	//DRT 9/27/2007 - PLID 27515 - SourceAction extra info
	long nSourceActionSourceID;
	long nSourceActionSourceDataGroupID;
	// (z.manning, 01/23/2008) - PLID 28690 - Added hot spot group ID.
	long nSourceActionSourceHotSpotGroupID;
	// (z.manning 2009-02-13 09:27) - PLID 33070
	long nSourceActionSourceTableDropdownGroupID;
};

struct TopicPositionEntry;
	
class CEMRTopic
{
public:
	//TES 4/15/2010 - PLID 24692 - CEMRTopics must be passed in an entry in the master linked list of topic positions.
	//TES 5/9/2010 - PLID 24692 - Actually, in the case where a topic is being loaded by one if its children, it's OK for 
	// NULL to be passed in.
	CEMRTopic(CEMN *pParentEMN, TopicPositionEntry *tpe);
	CEMRTopic(CEMRTopic *pParentTopic, TopicPositionEntry *tpe);
	CEMRTopic(BOOL bIsTemplate, BOOL bOwnsParent, TopicPositionEntry *tpe);
	~CEMRTopic();
	
	// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
	ADODB::_ConnectionPtr GetRemoteData();

public:
#ifdef _DEBUG
	// (c.haag 2007-08-07 11:45) - PLID 26946 - This is used for debugging and developer testing with EMR
	// (c.haag 2007-09-17 10:35) - PLID 27401 - We now include the dialog itself
	// (c.haag 2007-09-17 13:07) - PLID 27408 - We now support suppressing address information
	void DebugReportMembers(CEmrDebugDlg* pDlg, BOOL bOnlyIncludeLegacyFields, BOOL bAllowAddresses);
#endif

public:
	void operator =(CEMRTopic &etSource);

	//TES 5/25/2006 - If you want this to load asynchronously, you must provide a hWnd that will receive the NXM_POST_LOAD message,
	//and call PostLoad on this topic.
	//Load an existing EmrTopicsT record.
	//TES 6/5/2006 - When calling from a thread, pass in a connection pointer.
	//TES 6/6/2006 - Also, pass in a CEMR pointer, so that the topic, even though it won't be attached to that EMR yet, 
	//can still tell it to LoadTopic().
	// (c.haag 2007-04-30 17:57) - PLID 25853 - Added LoadFromPreloadedTopic for loading from a preloaded topic object for faster loads
	// (c.haag 2007-02-27 10:12) - PLID 24949 - Added pLoader for cases where the details have already been loaded except for their states
	void LoadFromTopicID(long nEMRTopicID, HWND hWnd, OPTIONAL IN ADODB::_Connection *lpCon = NULL, OPTIONAL IN CEMR *pEmr = NULL, OPTIONAL IN CEMNLoader* pLoader = NULL);
	// (c.haag 2007-02-27 09:06) - PLID 24949 - Added pLoader for cases where the details have already been loaded except for their states
	// (z.manning 2010-08-20 09:47) - PLID 40190 - Added connection pointer
	void LoadFromTopicFields(ADODB::_Connection *lpCon, ADODB::Fields *lpfldsTopicInfo, HWND hWnd, OPTIONAL IN CEMR *pEmr = NULL, OPTIONAL IN CEMNLoader* pLoader = NULL);
	// (z.manning 2010-08-20 09:49) - PLID 40190 - Added connection pointer
	void LoadFromPreloadedTopic(ADODB::_Connection *lpCon, CEMNLoader* pLoader, long nTopicID, long nPatientID, HWND hWnd, OPTIONAL IN CEMR *pEmr = NULL);

	//Load an existing EmrTemplateTopicsT record (either onto a new EMN, or as part of loading the template).
	//bIsNewTopic means your loading a templatetopic on a new template, ignored for EMNs.
	//bLoadHiddenTopics means to load details and subtopics, even if they're not currently visible (not spawned yet).
	//TES 5/20/2006 - nPatientID should only be set by the Load thread.
	//TES 6/5/2006 - When calling from a thread, pass in a connection pointer.
	//TES 6/6/2006 - Also, pass in a CEMR pointer, so that the topic, even though it won't be attached to that EMR yet, 
	//can still tell it to LoadTopic().
	// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
	// (c.haag 2007-02-27 10:46) - PLID 24949 - Added pLoader for cases where the details have already been loaded except for their states
	//DRT 9/25/2007 - PLID 27515 - Added SourceActionSourceID and SourceActionSourceDataGroupID fields
	// (z.manning 2009-02-13 09:18) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-05 08:47) - PLID 33338 - Use the new source action info class
	void LoadFromTemplateTopicID(long nEmrTemplateTopicID, BOOL bIsNewTopic, HWND hWnd, SourceActionInfo &sai, BOOL bLoadHiddenTopics = FALSE, OPTIONAL IN long nPatientID = -1, OPTIONAL IN ADODB::_Connection *lpCon = NULL, OPTIONAL IN CEMR *pEmr = NULL, OPTIONAL IN CEMNLoader* pLoader = NULL, OPTIONAL IN long nSourceActionSourceID = -1, OPTIONAL IN long nSourceActionSourceDataGroupID = -1, OPTIONAL IN long nSourceActionSourceHotSpotGroupID = -1, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID = -1);
	// (c.haag 2007-02-27 10:46) - PLID 24949 - Added pLoader for cases where the details have already been loaded except for their states
	//DRT 9/25/2007 - PLID 27515 - Added SourceActionSourceID and SourceActionSourceDataGroupID fields
	// (z.manning 2009-02-13 11:18) - PLID 33070 - SourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-05 08:48) - PLID 33338 - Use the new source action info class
	// (z.manning 2010-08-20 09:51) - PLID 40190 - Added connection pointer
	void LoadFromTemplateTopicFields(ADODB::_Connection *lpCon, ADODB::Fields *lpfldsTopicInfo, BOOL bIsNewTopic, HWND hWnd, SourceActionInfo &sai, BOOL bLoadHiddenTopics = FALSE, OPTIONAL IN long nPatientID = -1, OPTIONAL IN CEMR *pEmr = NULL, OPTIONAL IN CEMNLoader* pLoader = NULL, OPTIONAL IN long nSourceActionSourceID = -1, OPTIONAL IN long nSourceActionSourceDataGroupID = -1, OPTIONAL IN long nSourceActionSourceHotSpotGroupID = -1, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID = -1);
	// (c.haag 2007-05-02 17:36) - PLID 25881 - Added LoadFromPreloadedTemplateTopic for loading from a preloaded template topic
	// (c.haag 2007-08-08 11:15) - PLID 27014 - Added bLoadSubtopics
	//DRT 9/25/2007 - PLID 27515 - Added nSourceActionSourceID and m_nSourceActionSourceDataGroupID
	// (z.manning 2009-02-13 11:19) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-04 15:00) - PLID 33338 - Use the new source action info class
	// (z.manning 2010-08-20 09:53) - PLID 40190 - Added connection pointer
	void LoadFromPreloadedTemplateTopic(ADODB::_Connection *lpCon, CEMNLoader* pLoader, long nEmrTemplateTopicID, BOOL bIsNewTopic, HWND hWnd, SourceActionInfo &sai, BOOL bLoadHiddenTopics = FALSE, OPTIONAL IN long nPatientID = -1, OPTIONAL IN CEMR *pEmr = NULL,
		OPTIONAL IN BOOL bLoadSubtopics = TRUE, long nSourceActionSourceID = -1, long nSourceActionSourceDataGroupID = -1, OPTIONAL IN long nSourceActionSourceHotSpotGroupID = -1, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID = -1);

	//TES 2/3/2006 - This is for details that are being loaded independently; load just the info relevant to the detail
	//(i.e., no other details or subtopics).
	// (j.jones 2007-08-02 11:42) - PLID 26915 - added ability to pass in a connection
	void LoadFromDetail(CEMNDetail *pDetail, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	//TES 2/3/2006 - This is called from LoadFromDetailID, to continue up the chain.
	// (j.jones 2007-08-02 11:42) - PLID 26915 - added ability to pass in a connection
	void LoadFromSubTopic(CEMRTopic *pSubTopic, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	
	//void ShowTopicDetails();
	//void HideTopicDetails();

	//Because of the way this loads, HasSubTopics() is MUCH faster than GetSubTopicCount(), so use this one if possible.
	BOOL HasSubTopics();
	//Similarly with HasDetails()
	BOOL HasDetails();

	CRect GetClientArea();

	long GetEMNDetailCount();
	long GetSubTopicCount();
	CEMRTopic* GetSubTopic(long nIndex);
	CEMRTopic* GetSubTopicByID(long nID);
	long GetDeletedSubTopicCount() const;
	CEMRTopic* GetDeletedSubTopic(long nIndex) const;

	//TES 4/15/2010 - PLID 24692 - The entry that defines our position (sort order)
	TopicPositionEntry* GetTopicPositionEntry() { return m_pTopicPositionEntry;}

	// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
	// (z.manning 2008-09-03 12:56) - PLID 31235 - GetSubTopicByTemplateTopicID was split into 4 functions
	// (a.walling 2010-04-05 13:28) - PLID 38060 - Also look for the spawned group table row
	CEMRTopic* GetSubTopicByTemplateTopicIDAndSpawnedGroupID(long nTemplateTopicID, long nSpawnedGroupID, CEMNDetail *pSourceDetail, SourceActionInfo* pSpawnedGroupSourceActionInfo );
	// (a.walling 2010-04-05 13:28) - PLID 38060 - Also look for the spawned group table row
	CEMRTopic* GetSubTopicByOriginalTemplateTopicIDAndSpawnedGroupID(long nTemplateTopicID, long nSpawnedGroupID, CEMNDetail *pSourceDetail, SourceActionInfo* pSpawnedGroupSourceActionInfo );
	// (a.walling 2010-04-05 11:07) - PLID 38060
	// (a.walling 2010-04-05 13:28) - PLID 38060 - Also look for the spawned group table row
	CEMRTopic* GetSubTopicByTemplateTopicIDAndSpawnedGroupIDOnly(long nTemplateTopicID, long nSpawnedGroupID, SourceActionInfo* pSpawnedGroupSourceActionInfo );
	CEMRTopic* GetSubTopicByOriginalTemplateTopicIDAndSpawnedGroupIDOnly(long nTemplateTopicID, long nSpawnedGroupID, SourceActionInfo* pSpawnedGroupSourceActionInfo );
	CEMRTopic* GetSubTopicByTemplateTopicIDOnly(long nTemplateTopicID);
	CEMRTopic* GetSubTopicByOriginalTemplateTopicIDOnly(long nTemplateTopicID);

	//Takes the subtopic out of our array, but doesn't destroy it.
	void DetachSubTopic(CEMRTopic *pTopic);
	//Takes the detail out of our array, but doesn't destroy it.
	void DetachDetail(CEMNDetail *pDetail, BOOL bReleaseRef); // (a.walling 2009-10-13 14:48) - PLID 36024 - Parameter to release ref
	//DRT 8/9/2007 - PLID 26876 - Use this version when calling from an external source
	void CEMRTopic::RemoveSubTopic_External(CEMRTopic *pTopic);

	//this delves deeper into the topic and finds all details in this topic or any subtopics
	long GetSubTopicDetailCount() const;	

	//this returns the top-level parent topic to this topic (ie. has no parent itself)
	CEMRTopic* GetTopMostParentTopic();
	//this checks to see if pTopicToFind is underneat this topic, and if so, gives us the immediate subtopic
	//that is a parent of pTopicToFind (note: this can return pTopicToFind if it is an immediate subtopic)
	CEMRTopic* GetSubTopicWithThisTopic(CEMRTopic *pTopicToFind);

	CEMNDetail* GetDetailByIndex(long nIndex);
	CEMNDetail* GetDetailByID(long nID);
	CEMNDetail* GetDetailByTemplateDetailID(long nID);
	// (j.jones 2010-02-12 14:19) - PLID 37318 - added GetSmartStampImageDetail_ByTemplateDetailID which is
	// a clone of GetDetailByTemplateDetailID, but only returns a detail where m_pSmartStampTableDetail is NULL
	CEMNDetail* GetSmartStampImageDetail_ByTemplateDetailID(long nID);

	// (z.manning 2008-10-09 16:05) - PLID 31628 - Gets all details with the given lab ID
	void GetDetailsByLabID(IN const long nLabID, OUT CArray<CEMNDetail*,CEMNDetail*> &m_arypDetails);

	// (a.walling 2007-04-09 12:51) - PLID 25549 - returns the CEMNDetail* that matched nDetailPtr
	CEMNDetail* GetDetailByPointer(long nDetailPtr);

	// (a.walling 2012-10-04 12:41) - PLID 52878 - Verify that the given emr object pointer is a child
	CEMRTopic* VerifyPointer(CEMRTopic* pTopic);
	CEMNDetail* VerifyPointer(CEMNDetail* pDetail);

	// (z.manning 2011-01-25 15:58) - PLID 42336
	void GetParentDetailsByDetailID(const long nChildDetailID, OUT CEMNDetailArray *paryParentDetails);
	void GetParentDetailsByTemplateDetailID(const long nChildTemplateDetailID, OUT CEMNDetailArray *paryParentDetails);

	// (z.manning 2011-01-28 12:26) - PLID 42336 - This function will look for any smart stamp table with the given
	// table info master ID as long as it is not already linked to an image with the given image info master ID.
	CEMNDetail* GetSmartStampTableDetailByInfoMasterID(const long nTableInfoMasterID, const long nImageInfoMasterID);

	// (z.manning 2011-03-02 11:16) - PLID 42638
	BOOL ContainsSmartStampTable();
	//TES 4/13/2012 - PLID 49482
	BOOL ContainsSmartStampImage();

	// (z.manning 2011-03-04 14:13) - PLID 42682
	void UpdateSourceDetailStampIDs(const long nOldDetailStampID, const long nNewDetailStampID);

	// (a.walling 2007-12-17 15:53) - PLID 28391 - returns count of 'visible' HTML details
	void GetVisibleHTMLDetails(long& nScreen, long& nPrint);
	void RefreshHTMLVisibility();

	//the "global index" is a calculated placement in the GetSubTopicDetailCount() logic
	//across all topics for an EMN
	CEMNDetail* GetDetailByGlobalIndex(long nIndexToFind, long &nCurCount);
	void RemoveDetail(CEMNDetail* pDetail);

	//DRT 7/30/2007 - PLID 26876 - Topic removal redesign.  See implementation for details.
	void RemoveThisTopicAndSubTopics();
	void RemoveDetail_DetailSpecific(CEMNDetail *pDetail);

	// (a.walling 2009-11-17 08:38) - PLID 36366 - Ignore details in pIgnoreTopic (they have probably already been added)
	void GenerateEMNDetailArray(CArray<CEMNDetail*,CEMNDetail*> *arypEMNDetails, CEMRTopic* pIgnoreTopic = NULL);
	void GenerateEMNDeletedDetailArray(CArray<CEMNDetail*,CEMNDetail*> *arypEMNDeletedDetails);

	long GetID() const;
	const CString& GetName() const
	{
		return m_strTopicName;
	}

	// (a.walling 2012-04-30 10:58) - PLID 46176 - Ensure something is displayed rather than nothing if the topic's label is blank
	CString GetDisplayName() const;

	//TES 1/11/2007 - PLID 24172 - This function may call PostLoad(), but we don't want PostLoad() to be called before
	// this topic has been told what its parent is AND the parent knows that this is one of its children.  So, places
	// which are doing both will pass FALSE into bAllowPostLoad, in which case it is their responsibility to make sure that
	// PostLoad() does in fact get called.
	void SetParentEMN(CEMN* pParentEMN, BOOL bAllowPostLoad = TRUE);
	CEMN* GetParentEMN() const;
	
	// (a.walling 2010-04-01 10:31) - PLID 38013 - Added function to get the parent EMR
	CEMR* GetParentEMR() const;

	//used when an item is changed from the administrator,
	//for all instances of that item in the topic and subtopics
	void MarkItemChanged(long nEMRInfoID);

	//refreshes all details in the topic and subtopics
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

	//invalidates merge buttons across the entire Topic, including subtopics
	void InvalidateAllDetailMergeButtons();

	void FindDetailsByMergeFieldName(const CString& strMergeFieldName, CArray<CEMNDetail*,CEMNDetail*>& arypEMNDetail);

	//TES 12/15/2005 - You can pass in a detail to ignore in the calculation (if it's about to be deleted).
	// (j.jones 2007-06-14 15:02) - PLID 26276 - added bCompleteIfNoDetails boolean, which means that
	// when bCompleteIfNoDetails is TRUE, no details on the topic means the topic is complete
	// (z.manning, 04/07/2008) - PLID 29495 - Removed the bCompleteIfNoDetails optional parameter as it is no longer relevant.
	// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Ability to recalculate completion status, not just 
	// return existing or calculated status.  Default behavior is unchanged.
	EmrTopicCompletionStatus GetCompletionStatus(CEMNDetail *pDetailToIgnore = NULL, BOOL bForceRecalculate = FALSE);

	//TES 10/9/2006 - PLID 22932 - We need to track which objects are already in our save string.
	// (j.jones 2007-01-11 14:28) - PLID 24027 - tracked strPostSaveSql, for sql statements to occur after the main save
	// (c.haag 2007-06-20 12:41) - PLID 26397 - We now store saved objects in a map for fast lookups.
	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
	// such as new or deleted prescriptions, or new or deleted diagnosis codes
	Nx::Quantum::Batch GenerateSaveString(long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynAffectedDetailIDs, OUT BOOL &bDrugInteractionsChanged, BOOL bIsTopLevelSave, BOOL bSaveRecordOnly);
	// (z.manning, 02/07/2007) - PLID 24599 - This overload of GenerateSaveString allows an array to be passed
	// in to keep track of deleted template topic IDs. When using this overload the calling function is completely
	// responsible for deleting any records from EmrTemplateTopicsT.
	// (c.haag 2007-06-20 12:41) - PLID 26397 - We now store saved objects in a map for fast lookups.
	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
	// such as new or deleted prescriptions, or new or deleted diagnosis codes
	Nx::Quantum::Batch GenerateSaveString(long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynAffectedDetailIDs, OUT BOOL &bDrugInteractionsChanged, CArray<long,long> &arynDeletedTemplateTopicIDs, BOOL bIsTopLevelSave, BOOL bSaveRecordOnly);
	BOOL PropagateNewID(long nID, EmrSaveObjectType esotSaveType, long nObjectPtr, long &nAuditTransactionID);
	// (a.walling 2007-10-18 16:34) - PLID 27664 - Added array to gather all topics affected in the PostSaveUpdate cascade.
	void PostSaveUpdate(BOOL bTopLevelUpdate = FALSE, BOOL bUpdateRecordOnly = FALSE, CArray<CEMRTopic*, CEMRTopic*> *parTopicsAffected = NULL);
	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent, BOOL bForceDeletedFlagTrue);

	// (z.manning, 02/07/2007) - PLID 24599 - Added a parameter to keep track of any deleted template topic IDs.
	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	// The calling function is responsible for deleting any records from EmrTemplateTopicsT with these IDs.
	Nx::Quantum::Batch GenerateDeleteString(long &nAuditTransactionID, CStringArray &arystrErrors, CArray<long,long> &arynDeletedTemplateTopicIDs, CDWordArray &arynAffectedDetailIDs);

	BOOL GetIsOnLockedAndSavedEMN();

	//TES 6/13/2006 - We need to pass in FALSE when we are calling this function as part of the topic loading.
	long GetTemplateTopicID(BOOL bEnsureVariable = TRUE);
	//TES 2/9/2006 - Whenever the TemplateTopicID is set, these two variables will be loaded from that TemplateTopic; the EMN
	//can then use them in order to calculate the placement of spawned topics.
	long GetTemplateID();
	long GetTemplateTopicOrderIndex();

	//TES 4/15/2010 - PLID 24692 - We don't maintain an order index any more.
	//long GetLastSavedTopicOrderIndex();
	long GetTopicOrderIndex();

	//TES 8/10/2010 - PLID 24692 - Gets the order index of the given topic within our list of topics (non-recursive).
	long GetChildOrderIndex(CEMRTopic *pChild);

	CString GetLastSavedParentTopicName() const
	{
		return m_strLastSavedParentTopicName;
	}

	// (c.haag 2007-05-22 12:01) - PLID 26095 - Added the option to do reference counting
	// (c.haag 2009-04-06 09:46) - PLID 33859 - Added bSyncSystemTableData (only used when popping up an item)
	void AddDetail(CEMNDetail *pDetail, BOOL bIsInitialLoad = FALSE, BOOL bAddRef = TRUE, BOOL bSyncSystemTableData = TRUE);
	//e.lally 3/20/2006 - This function adds the new detail, then loads its information, and finally updates the narratives.
		//It returns a pointer to the new detail that was added.
	//TES 12/6/2006 - PLID 23724 - This was using EmrInfoT.ID before, now it uses EmrInfoMasterT.ID
	// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
	//DRT 8/2/2007 - PLID 26919 - Added SourceActionSourceID
	// (c.haag 2007-08-06 15:30) - PLID 26992 - Added an optional parameter for a CEMNSpawner object to avoid redundant querying in LoadContent
	//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
	// (z.manning 2009-02-13 09:24) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-05 08:52) - PLID 33338 - Use the new source action info class
	// (j.jones 2010-02-22 17:13) - PLID 37318 - added pParentSmartStampImage, given only when this is called from CEMN::PostSmartStampImageAdded
	CEMNDetail* AddNewDetailFromEmrInfoMasterID(long nEmrInfoMasterID, BOOL bIsTemplate, SourceActionInfo &sai, BOOL bIsInitialLoad = FALSE, OPTIONAL IN long nSourceActionSourceID = -1, OPTIONAL IN CEMNSpawner* pEMNSpawner = NULL, long nSourceActionSourceDataGroupID = -1, OPTIONAL IN long nSourceActionSourceHotSpotGroupID = -1, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID = -1, CEMNDetail *pParentSmartStampImage = NULL);

	//TES 5/17/2006 - This function, used by the asynchronous loading thread, ONLY adds the detail to the array, does nothing else.
	// (c.haag 2007-05-22 12:01) - PLID 26095 - Added the option to do reference counting
	void AddDetail_Flat(CEMNDetail *pDetail, BOOL bAddRef = TRUE);

	//TES 3/18/2011 - PLID 42762 - Fills a GlassesOrder struct with any associated fields found in the EMN.  Will return TRUE if any values
	// were filled, otherwise FALSE.  strIgnoredData will be a user-friendly description of all the items that had data which could not
	// be included (because it was already there, if for example this EMN has two OD Spheres, the first will be added to goOrder, and 
	// the second will be desccribed in strIgnoredData).
	BOOL GetGlassesOrderData(IN OUT GlassesOrder &goOrder, IN OUT CString &strIgnoredData);

	//TES 4/11/2012 - PLID 49621 - Added, gathers any information on that could be loaded into a Contact Lens Order.  Same as GetGlassesOrderData
	BOOL GetContactLensOrderData(IN OUT ContactLensOrder &cloOrder, IN OUT CString &strIgnoredData);

	long GetSourceActionID();
	//DRT PLID 27515
	long GetSourceActionSourceID();
	long GetSourceActionSourceDataGroupID();
	// (z.manning, 01/23/2008) - PLID 28690
	long GetSourceActionSourceHotSpotGroupID();
	// (z.manning 2009-02-13 09:26) - PLID 33070
	long GetSourceActionSourceTableDropdownGroupID();

	// (j.jones 2007-07-31 09:09) - PLID 26882 - added an overload that takes in an action object
	//TES 3/18/2010 - PLID 37530 - Added overloads to ensure that either the source action name is passed in, or the index of the spawning stamp
	// is passed in (it's needed to calculate the source action name).
	void SetSourceActionID(const EmrAction &ea, const CString* pstrSourceActionName);
	void SetSourceActionID(const EmrAction &ea, long nSourceStampIndex);
protected:
	void SetSourceActionID(const EmrAction &ea, long nSourceStampIndex, const CString* pstrSourceActionName);
public:
	void SetSourceActionID(long nSourceActionID, long nSourceStampIndex, EmrActionObject* pSourceActionDestType = NULL);
	void SetSourceActionID(long nSourceActionID, const CString* pstrSourceActionName, EmrActionObject* pSourceActionDestType = NULL);
protected:
	void SetSourceActionID(long nSourceActionID, long nSourceStampIndex, const CString* pstrSourceActionName, EmrActionObject* pSourceActionDestType = NULL);
public:	
	// (a.walling 2010-04-01 12:17) - PLID 38013 - Returns the CEMNDetail that directly or indirectly spawned this topic
	// if bFurthest, we will keep traversing until we get to a non-spawned item or topic. Otherwise, it will return the first.
	CEMNDetail* FindSpawningItem(bool bFurthest, SourceActionInfo& sai);

	// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
	long GetSourceDetailID() const
	{
		return m_sai.nSourceDetailID;
	}

	void SetSourceDetailID(long nSourceDetailID)
	{
		m_sai.nSourceDetailID = nSourceDetailID;
	}

	CEMNDetail* GetSourceDetail()
	{
		return m_sai.pSourceDetail;
	}

	void SetSourceDetail(CEMNDetail *pSourceDetail);

	// (z.manning 2009-03-05 10:44) - PLID 33338
	void SetSourceDataGroupID(long nDataGroupID)
	{
		m_sai.SetDataGroupID(nDataGroupID);
	}

	long GetSourceDataGroupID()
	{
		return m_sai.GetDataGroupID();
	}

	//TES 3/17/2010 - PLID 37530 - This appears to be dead code
	// (z.manning 2010-03-03 15:15) - PLID 37532
	/*void SetSourceDetailStampID(const long nDetailStampID);
	void SetSourceDetailStampPointer(EmrDetailImageStamp *pDetailStamp);*/

	// (z.manning 2009-03-23 15:55) - PLID 33089 - Added an accessor for the source action info
	SourceActionInfo GetSourceActionInfo() const { return m_sai; }

	//TES 3/16/2010 - PLID 37530 - Added
	inline void SetSourceActionInfo(const SourceActionInfo &sai) { m_sai = sai; }

	EmrActionObject GetSourceActionDestType() const;

	void SetTemplateTopicID(long nTemplateTopicID);
	void SetTemplateID(long nTemplateID);
	void SetTemplateTopicOrderIndex(long nTemplateTopicOrderIndex);
	//TES 4/15/2010 - PLID 24692 - We don't maintain our own order index any more.
	//void SetTopicOrderIndex(long nTopicOrderIndex);
	void SetLastSavedTopicName(const CString &strLastSavedTopicName);
	//TES 4/15/2010 - PLID 24692 - We don't maintain our own order index any more.
	//void SetLastSavedTopicOrderIndex(long nTopicOrderIndex);
	
protected:
	void SetSourceActionDestType(EmrActionObject type);

	//Destroys and deletes the topic.
	//DRT 8/9/2007 - PLID 26876 - This should only be called internally to the class, so I moved it to protected.
	void RemoveSubTopic(CEMRTopic *pTopic);

public:
	// (c.haag 2007-05-30 10:53) - PLID 26175 - Call this function to force an asynchronous load to finish if
	// one is in progress
	void ForceLoadToFinish();
	
public:
	// (z.manning 2010-12-08 15:54) - PLID 41731 - This function is no longer const
	CString GetSourceActionName();

	//The SpawnedGroupID is -1, unless the SourceActionID is for an eaoMintItems action, in which case it's the SourceActionID
	long GetSpawnedGroupID();

	//TES 4/15/2010 - PLID 24692 - Added a parameter for the ID
	CEMRTopic* AddSubTopic(const CString &strName, long nTopicID);
	//TES 10/5/2009 - PLID 35755 - Added a parameter for whether this insertion should cause the topic order indexes
	// to be recalculated when saving.
	void InsertSubTopic(CEMRTopic *pInsert, CEMRTopic *pInsertBefore, BOOL bIsInitialLoad, BOOL bTopicArrayChanged);

	void SetName(const CString &strName);
	
	// (a.walling 2010-04-01 10:31) - PLID 38013 - Added function to get the top-level parent topic
	CEMRTopic* GetTopLevelParentTopic() const;

	CEMRTopic* GetParentTopic() const
	{
		return m_pParentTopic;
	}

	CEMRTopic* GetOriginalParentTopic() const
	{
		return m_pOriginalParentTopic;
	}

	//TES 1/11/2007 - PLID 24172 - This function may call PostLoad(), but we don't want PostLoad() to be called before
	// this topic has been told what its parent is AND the parent knows that this is one of its children.  So, places
	// which are doing both will pass FALSE into bAllowPostLoad, in which case it is their responsibility to make sure that
	// PostLoad() does in fact get called.
	void SetParentTopic(CEMRTopic *pParent, BOOL bIsOriginalParent = FALSE, BOOL bAllowPostLoad = TRUE);

	//TES 6/13/2006 - We need to pass in FALSE when we are calling this function as part of the topic loading.
	long GetOriginalTemplateTopicID(BOOL bEnsureVariable = TRUE);
	void SetOriginalTemplateTopicID(long nID);

	// (a.walling 2007-03-21 10:42) - PLID 25301 - Same as TES's 6/13 comment, pass FALSE when calling as part of topic loading
	long GetOriginalTemplateID(BOOL bEnsureVariable = TRUE);
	void SetOriginalTemplateID(long nID);

	BOOL IsTemplate() {return m_bIsTemplate;}

	//This function returns a number that says that this topic is, say, the 12th topic in the EMN.  It can be used to uniquely
	//identify this topic within the EMN, even if it hasn't been saved yet and therefore doesn't have an ID.
	int GetIndexAgainstEMN();

	//The InterfaceWnd is the window that this object will send messages to when things change.
	// (a.walling 2011-10-20 14:23) - PLID 46075 - Clean up interaction with external interfaces
	void SetTopicWnd(CEmrTopicWndPtr pEmrTopicWnd)
	{
		m_pTopicWnd = pEmrTopicWnd;
	}

	// (a.walling 2012-06-22 14:01) - PLID 51150 - GetInterfaceWnd returns the EMN's GetInterface now
	class CEmrTreeWnd* GetInterfaceWnd() const;
	
	CEmrTopicWndPtr GetTopicWnd() const
	{
		if (!this) return CEmrTopicWndPtr();
		return m_pTopicWnd.lock();
	}

	// (a.walling 2012-06-22 14:01) - PLID 51150 - returns NULL if !GetTopicWnd()
	CEmrTopicWnd* GetTopicWndRaw() const
	{
		if (!this) return NULL;

		if (CEmrTopicWndPtr pTopicWnd = GetTopicWnd()) {
			return pTopicWnd.get();
		}
		return NULL;
	}

	void ResetTopicWnd()
	{
		m_pTopicWnd.reset();
	}

	void SetTopicHeaderWnd(CEmrTopicHeaderWndPtr pTopicHeaderWnd)
	{
		m_pTopicHeaderWnd = pTopicHeaderWnd;
	}

	CEmrTopicHeaderWndPtr GetTopicHeaderWnd() const
	{
		if (!this) return CEmrTopicHeaderWndPtr();
		return m_pTopicHeaderWnd.lock();
	}

	void ResetTopicHeaderWnd()
	{
		m_pTopicHeaderWnd.reset();
	}

	//DRT 8/3/2007 - PLID 26937 - No longer needed, so .cpp for details
	//Revokes all actions for items on this topic, and selected checkboxes on this topic, and recurses to subtopics.
	//void RevokeAllActions();

	//Processes all actions for items on this topic, and selected checkboxes on this topic and recurses to subtopics.
	void ProcessAllActions(BOOL bIsInitialLoad);

	//TES 6/29/2006 - If you're calling this strictly to determine the appearance in the tree (i.e., whether the row is visible),
	//then pass FALSE, and this function will return immediately, and if the variable is still loading, the visibility of the row
	//will be updated when the load completes.  However, if you're calling for some other reason where you need to now for sure the
	//real value (like if you right-click on the row), then pass TRUE, and this will wait for the topic to finish loading before it
	//returns.
	BOOL ShowIfEmpty(BOOL bForceLoadFromData = TRUE);
	void SetShowIfEmpty(BOOL bShow);

	//A topic is empty if there are not visible details on it or any of its subtopics.
	//If you pass in pDetailToIgnore, will return whether the topic would be empty if that detail was removed.
	// (z.manning, 03/05/2007) - PLID 24529 - Added an optional parameter to determine whether or not blank
	// subtopics should count when determining if a topic is blank.
	// (a.walling 2012-07-09 12:35) - PLID 51441 - Added option to ignore a topic
	BOOL IsEmpty(CEMNDetail *pDetailToIgnore, BOOL bIgnoreBlankSubtopics, OPTIONAL CEMRTopic* pTopicToIgnore = NULL);

	//This function checks whether the topic needs to be saved.
	BOOL IsUnsaved(BOOL bCheckSubtopics);

	//Tells this topic that it has been saved.
	void SetSaved(BOOL bIsPostLoad = FALSE);
	// (j.jones 2007-01-31 14:25) - PLID 24515 - added bForceSave, so if you make a change
	// before PostLoad() finishes, it will not mark the topic as saved again
	void SetUnsaved(BOOL bForceSave = FALSE);

	void SetVisible(BOOL bVisible) {m_bVisible = bVisible;}
	BOOL GetVisible() const { return m_bVisible; }

	// (a.walling 2012-07-09 12:35) - PLID 51441 - IsDisplayed for total UI visibility
	// if not visible, false; else if template, true; else if not empty or ShowIfEmpty, true; else false
	bool IsDisplayed(OPTIONAL CEMNDetail *pDetailToIgnore = NULL, OPTIONAL BOOL bIgnoreBlankSubtopics = TRUE, OPTIONAL CEMRTopic* pTopicToIgnore = NULL);

	bool IsParentDisplayed(OPTIONAL CEMNDetail *pDetailToIgnore = NULL, OPTIONAL BOOL bIgnoreBlankSubtopics = TRUE);

	static CTopicArray& GetEmptyTopicSet()
	{
		static CTopicArray emptyTopicSet;
		return emptyTopicSet;
	}

	// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns NULL if no new topic can be found; never returns this
	CEMRTopic* GetNextTreeTopic();
	CEMRTopic* GetPrevTreeTopic();
	CEMRTopic* GetNextDisplayedTreeTopic(OPTIONAL CEMNDetail *pDetailToIgnore = NULL, OPTIONAL BOOL bIgnoreBlankSubtopics = TRUE, OPTIONAL CEMRTopic* pTopicToIgnore = NULL);
	CEMRTopic* GetPrevDisplayedTreeTopic(OPTIONAL CEMNDetail *pDetailToIgnore = NULL, OPTIONAL BOOL bIgnoreBlankSubtopics = TRUE, OPTIONAL CEMRTopic* pTopicToIgnore = NULL);

	// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns next/prev sibling topic, or NULL
	CEMRTopic* GetNextSiblingTopic();
	CEMRTopic* GetPrevSiblingTopic();

	// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns first/last child topic, or NULL
	CEMRTopic* GetFirstChildTopic();
	CEMRTopic* GetLastChildTopic();

	// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns the last, deepest child topic, recursively, or this
	CEMRTopic* GetDeepestLastTreeTopic();

	// (a.walling 2012-07-09 12:35) - PLID 51441 - Reference to topic array
	CTopicArray& GetSubtopics()
	{
		return m_arypEMRTopics;
	}

	CTopicArray& GetParentTopics();
	
	// (a.walling 2012-07-09 12:35) - PLID 51441 - Walks the tree until predicate is true or NULL
	template <typename Predicate>
	CEMRTopic* GetNextTreeTopicWhere(Predicate predicate)
	{
		CEMRTopic* pTopic = this;
		do {
			pTopic = pTopic->GetNextTreeTopic();
			if (pTopic && predicate(pTopic)) {
				return pTopic;
			}
		} while (pTopic);
		ASSERT(pTopic != this);
		return pTopic;
	}
	
	// (a.walling 2012-07-09 12:35) - PLID 51441 - Walks the tree until predicate is true or NULL
	template <typename Predicate>
	CEMRTopic* GetPrevTreeTopicWhere(Predicate predicate)
	{
		CEMRTopic* pTopic = this;
		do {
			pTopic = pTopic->GetPrevTreeTopic();
			if (pTopic && predicate(pTopic)) {
				return pTopic;
			}
		} while (pTopic);
		ASSERT(pTopic != this);
		return pTopic;
	}

	BOOL HideOnEMN() const
	{
		return m_bHideOnEMN;
	}

	void SetHideOnEMN(BOOL bHide);

	//Tells this topic that it is new, when it is saved it will INSERT rather than UPDATE.
	// (a.walling 2007-09-27 14:01) - PLID 25548 - option to retain the order index
	void SetNew(BOOL bRetainOrderIndex =FALSE);

	// (j.jones 2007-01-23 09:14) - PLID 24027 - reassigns source detail IDs/pointers due to an EMN copy
	//TES 5/20/2014 - PLID 52705 - I added bResetIDs, so this function knows whether or not it should set everything to -1
	void UpdateSourceDetailsFromCopy(bool bResetIDs);

	// (z.manning 2010-03-11 14:56) - PLID 37571 - Will reassasign the source detail stamp pointer to the given
	// new pointer for any object in this topic that could have potentially been spawned.
	void UpdateSourceDetailStampPointers(EmrDetailImageStamp *pDetailStampOld, EmrDetailImageStamp *pDetailStampNew);

	// (j.jones 2007-01-23 11:00) - PLID 24027 - update the source details such that their pointers are set
	// and if bClearEraseSourceDetailID is TRUE, then also clear their detail IDs
	void UpdateSourceDetailPointers(BOOL bClearEraseSourceDetailID);

	void ProcessOriginalParentTopicDeleted(CEMRTopic *pTopicBeingDeleted);

	//TES 4/4/2006 - This function (added by Josh on 2/20/2006) calculates whether there are any topics that need to be saved
	//before or after this function.  If there are, it returns TRUE.  When it returns, aryEMRTopicsToSave will be an array
	//of CEMRTopics, in order, which must be saved in that order any time this topic is saved. You can pass in 
	//aryTopicsAlreadySaved, that is an array of topics which have already been calculated, this will tell this function not
	//to calculate them.
	BOOL CalculateTopicSavingOrderNeeded(OUT CArray<CEMRTopic*,CEMRTopic*> &aryEMRTopicsToSave, IN CArray<CEMRTopic*,CEMRTopic*> &aryTopicsAlreadySaved = CArray<CEMRTopic*,CEMRTopic*>());

	//Determine whether or not a detail has been deleted from this topic.
	// (z.manning 2011-03-02 17:20) - PLID 42335 - Added a flag for whether or not to check pending deleted details
	BOOL IsDetailDeleted(CEMNDetail* pDetail, BOOL bCheckPendingDeleted = TRUE);

	//Used in preventing details from unspawning themselves.
	BOOL AllowUnspawn();

	//Quick function that says whether this topic is completely loaded.
	BOOL IsLoaded();

	//Details call this when their state changes, so that the topic can do whatever it needs to do (specifically, update the cached
	//completion status).
	void HandleDetailStateChange(CEMNDetail *pChangedDetail);

	//TES 1/23/2007 - PLID 24377 - Used when processing EMR Links, this function returns TRUE if any one of the EmrDataT.IDs
	// in the array is selected on any detail in this topic or any subtopics, otherwise it returns FALSE.
	//TES 2/6/2007 - PLID 24377 - arDetailsToIgnore is passed in when processing a batch of details, which we don't want
	// to link to each other.  The function will only return TRUE if one of the data IDs is checked on a detail that is NOT
	// in arDetailsToIgnore.
	BOOL IsAnyItemChecked(const CArray<long,long> &arDataIDs, const CArray<CEMNDetail*,CEMNDetail*> &arDetailsToIgnore);

	// (c.haag 2011-05-19) - PLID 43696 - Populates a map with all EmrDataID's that correspond to checked-off single-select
	// and multi-select list items. All details in mapDetailsToIgnore are ignored during the search.
	void GetAllCheckedItems(CMap<long,long,BOOL,BOOL>& mapDataIDs, const CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> &mapDetailsToIgnore);

public:

	// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items	// (j.jones 2008-07-21 17:28) - PLID 30729 - add all of this topic's problems
	// and its details' problems to the passed-in list
	// (c.haag 2008-08-14 12:05) - PLID 30820 - Added bIncludeDeletedProblems
	void GetAllProblems(CArray<CEmrProblem*, CEmrProblem*> &aryProblems, BOOL bIncludeDeletedProblems = FALSE);

	// (j.jones 2009-05-21 15:58) - PLID 34325 - recurses through children and returns all problem links within the topic,
	// or just the problem links that reference pFilterProblem, if pFilterProblem is not NULL
	// Links that have been deleted are not returned by default, unless bIncludeDeletedLinks is TRUE
	void GetAllProblemLinks(CArray<CEmrProblemLink*, CEmrProblemLink*> &aryProblemLinks, CEmrProblem *pFilterProblem = NULL, BOOL bIncludeDeletedLinks = FALSE);

	// (j.jones 2008-07-22 08:48) - PLID 30789 - added problem class, and an array to track problems
	// (c.haag 2009-05-16 12:03) - PLID 34311 - We now track problem links instead of problems.
	CArray<CEmrProblemLink*, CEmrProblemLink*> m_apEmrProblemLinks;

	// (j.jones 2008-07-22 08:48) - PLID 30789 - returns true if there are any undeleted problems on the topic
	BOOL HasProblems();
	// (j.jones 2008-07-22 08:48) - PLID 30789 - returns true if there are only undeleted, closed problems on the topic
	BOOL HasOnlyClosedProblems();
	// (j.jones 2008-07-23 15:19) - PLID 30789 - returns true if any problems are marked as modified,
	// including deleted items
	BOOL HasChangedProblems();
	// (c.haag 2008-07-24 09:49) - PLID 30826 - Returns TRUE if there is at least one saved problem for this topic or any of
	// its children. This does not check deleted EMR objects.
	BOOL DoesTopicOrChildrenHaveSavedProblems();

	// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Search this topic and all subtopics for details with a reconstructed status of -1.
	BOOL HasReconstructedDetails();

	//TES 6/4/2008 - PLID 30196 - Does this topic have anywhere on it or on any subtopics an (undeleted) system 
	// table of the specified type?
	BOOL HasSystemTable(EmrInfoSubType eistType);

	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	// Safely checks this topic and subtopics recursively to find if there are any visible details that are marked as required and aren't filled in.
	BOOL HasVisibleUnfilledRequiredDetails();

	BOOL GetHasMoved();
	void SetHasMoved(BOOL bMoved);

	// (a.walling 2007-04-05 16:40) - PLID 25454 - returns the HTML string for the entire topic
	CString GetHTML(); // (a.walling 2013-03-13 08:40) - PLID 55632 - Ignore unsaved is no longer applicable

	// (a.walling 2007-10-18 17:19) - PLID 27664 - Clears out the last saved HTML, so it will be saved again next time
	//void ClearLastSavedHTML(); // (a.walling 2013-03-13 08:40) - PLID 55632 - Ignore unsaved is no longer applicable

	// (a.walling 2007-04-25 12:28) - PLID 25549 - Fill the array with sorted details
	void GetSortedDetailArray(CArray<CEMNDetail*, CEMNDetail*> &arDetails);

	// (a.walling 2007-04-25 12:29) - PLID 25549 - fill the array with sorted topics
	// (a.walling 2007-10-18 15:35) - PLID 25549 - No longer necessary
	//void GetSortedTopicArray(CArray<CEMRTopic*, CEMRTopic*> &arTopics);

	CEMRTopic* GetSubTopicByPointer(long nSubTopicPtr);
	
	// (a.walling 2008-06-30 16:19) - PLID 30571
	inline DWORD GetPreviewFlags() {return m_nPreviewFlags;};

	// (a.walling 2008-07-01 09:24) - PLID 30570
	// (a.walling 2009-07-06 11:24) - PLID 34793 - Option to refresh parent
	void SetPreviewFlags(DWORD dwFlags, BOOL bRefreshParent = FALSE);

	// (a.walling 2008-06-30 12:34) - PLID 29271
	DWORD m_nPreviewFlags;

protected:
	void LoadTemplateInfo(long nTemplateTopicID);

	//Exactly one of these two should be NULL.
	CEMN* m_pParentEMN;
	CEMRTopic *m_pParentTopic;
	CEMRTopic *m_pOriginalParentTopic;
	//TES 2/3/2006 - Sometimes we are given a parent, sometimes we create it ourselves.  This tells us whether we "own" our parent.
	BOOL m_bOwnParent;
	//This tells us whether we are responsible for deleting our details/subtopics.
	BOOL m_bOwnChildren;

	//If m_bIsTemplate, EmrTemplateTopicsT.ID, otherwise EmrTopicsT.ID
	long m_nID;
	BOOL m_bIsTemplate;

	//These two variables are loaded synchronously, under the assumption that they will be needed immediately.
	CString m_strTopicName;
	BOOL m_bHasSubTopics;	//This is redundant, but we can load it synchronously, and let the actual subtopics be loaded
							//asynchronously.
	BOOL m_bHasDetails;		//Same for this.

	CMutex m_mtxLoadingTopics; //Since multiple threads may be trying to get at PostLoadTopics, we need this mutex.
								//You MUST lock this mutex while accessing m_bTopicsLoaded or calling PostLoadTopics().
	BOOL m_bTopicsLoaded;	//Make sure we only load the subtopics once.
	void PostLoadTopics();
	
	CMutex m_mtxLoadingDetails; //Since multiple threads may be trying to get at PostLoadDetails, we need this mutex.
								//You MUST lock this mutex while accessing m_bDetailsLoaded or calling PostLoadDetails().
	BOOL m_bDetailsLoaded; //Make sure we only load the details once.
	void PostLoadDetails();

	//This makes sure that the details are loaded properly.  If they're loaded already, will immediately return FALSE.  
	//Otherwise, it will load them, waiting on the LoadTopic thread if needed, then return TRUE.
	BOOL EnsureDetails();
	//This makes sure that the details are loaded properly.  If they're loaded already, will immediately return FALSE.  
	//Otherwise, it will load them, waiting on the LoadTopic thread if needed, then return TRUE.
	BOOL EnsureTopics();

	EMRTopicLoadInfo *m_pLoadInfo;
	/****************************************************************************************************************************
	/* TES 5/17/2006 - The following variables are all loaded asynchronously by m_pLoadingThread into m_pLoadInfo.  The Get
	/* functions for each of these will first check whether we have finished loading, if not, it will wait for it to finish, and
	/* make sure that PostLoad() (which copies values to our member variables) is called.  Then it returns the variable.
	/****************************************************************************************************************************/
	//If m_bIsTemplate, this is the same as m_nID, otherwise, EmrTopicsT.EmrTemplateTopicID
	long m_nTemplateTopicID;
	long m_nTemplateID;
	long m_nTemplateTopicOrderIndex;
	//EmrTemplateTopicsT.SourceTemplateTopicID, if !m_bIsTemplate, this may still be valid, as the source template topic for the topic represented by EmrTopicsT.EmrTemplateTopicID.
	long m_nOriginalTemplateTopicID;
	long m_nOriginalTemplateID; // (a.walling 2007-03-21 10:45) - PLID 25301 - simply the template of the original template topic

	//TES 4/15/2010 - PLID 24692 - We don't maintain our own order index any more.
	//long m_nTopicOrderIndex;
	TopicPositionEntry *m_pTopicPositionEntry;

	CArray<CEMNDetail*, CEMNDetail*> m_arypEMNDetails;
	CArray<CEMRTopic*, CEMRTopic*> m_arypEMRTopics;
	BOOL m_bSubTopicArrayChanged; //Set to TRUE whenever the order is changed, or any are added or removed.

	//DRT 8/3/2007 - PLID 26928 - We want to track details that are "pending" deletion.  This means they're going to be
	//	deleted very soon, but not quite at this instant.  This is mostly useful for unspawning on a template, when
	//	we need to know the status of a source detail, but we might not have physically removed that source detail yet.
	CArray<CEMNDetail*, CEMNDetail*> m_paryPendingDeletionDetails;

	CArray<CEMRTopic*, CEMRTopic*> m_arypOtherSavedTopics;

	// (z.manning 2009-03-05 08:54) - PLID 33338 - use the new source action info class
	SourceActionInfo m_sai;
	CString m_strSourceActionName;
	EmrActionObject m_SourceActionDestType;

	//DRT PLID 27515
	long m_nSourceActionSourceID;
	long m_nSourceActionSourceDataGroupID;
	// (z.manning, 01/23/2008) - PLID 28690
	long m_nSourceActionSourceHotSpotGroupID;
	long m_nSourceActionSourceTableDropdownGroupID; // (z.manning 2009-02-13 09:26) - PLID 33070

	BOOL m_bShowIfEmpty;

	//Should this template topic be included on EMNs?
	BOOL m_bHideOnEMN;

	EmrTopicCompletionStatus m_CompletionStatus;

	/****************************************************************************************************************************
	/* END asynchronously loaded member variables.
	/****************************************************************************************************************************/

public:
	//When the load thread is done, it will post a message to the window we gave it (CEmrTreeWnd), which will call this function.
	void PostLoad();

	// (a.walling 2007-10-17 14:41) - PLID 27017 - Sets the topic as loaded, probably because it was not loaded from data
	void SetLoaded();

	//TES 2/7/2007 - PLID 24652 - This function returns TRUE if the loading of this topic's data finished before the topic 
	// had been fully integrated into the structure, and therefore PostLoad() couldn't be called at the time, but must
	// be called as soon as possible.
	BOOL NeedPostLoad() {return m_bNeedPostLoad;}

	//DRT 8/3/2007 - PLID 26928 - See comments around m_paryPendingDeletionDetails
	void AddDetailToPendingDeletion(CEMNDetail *pDetail);
	void RemoveDetailFromPendingDeletion(CEMNDetail *pDetail);

public:
	// (c.haag 2007-05-08 10:55) - PLID 25928 - Returns TRUE if the post load is complete
	BOOL IsPostLoadComplete() const { return m_bPostLoadComplete; }

	// (c.haag 2007-05-08 10:55) - PLID 25928 - Functions that control whether we will reposition
	// topic details after the topic load is done
	BOOL GetNeedsToRepositionDetailsInPostLoad() const { return m_bNeedsToRepositionDetailsInPostLoad; }
	void SetNeedsToRepositionDetailsInPostLoad(BOOL bSet) { m_bNeedsToRepositionDetailsInPostLoad = bSet; }

public:
	// (c.haag 2007-05-30 10:14) - PLID 26175 - Returns the load info object
	inline EMRTopicLoadInfo* GetLoadInfo() { return m_pLoadInfo; }

	// (a.walling 2007-06-28 16:32) - PLID 26494 - moved to public scope so we can do this from the EMN.
	//Won't return until we're done loading.
	void EnsureLoaded();

public:
// (c.haag 2008-07-23 12:16) - PLID 30820 - Populate apProblems with a list of all deleted problems for this object and
// all its children. If a child or related EMR object is deleted, all its problems are considered deleted as well.
	// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now, not problems
	void GetAllDeletedEmrProblemLinks(CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks, BOOL bIncludeThisObject);

	// (z.manning 2009-06-23 14:53) - PLID 34692 - Function to go through all subtopics on the topic
	// and make sure that a topic with the given ID is not in any of the deleted topic arrays.
	void EnsureTopicNotDeleted(const long nTopicID);

	// (j.jones 2013-06-18 13:23) - PLID 47217 - added ability to remove all signature details from new topics
	void RemoveNewSignatureDetails();

protected:

	BOOL m_bPostLoadCalled; //We only want to call PostLoad() once.
	//TES 1/12/2007 - PLID 24172 - The variable m_bPostLoadCalled does what m_bPostLoadComplete used to do, m_bPostLoadComplete
	// now means what the name implies: that the PostLoad() function has been fully executed.
	BOOL m_bPostLoadComplete;

	//TES 2/7/2007 - PLID 24652 - This variable will be set to true if the loading of the data gets finished before the topic
	// has been integrated into the structure, and therefore PostLoad() couldn't be called then.
	BOOL m_bNeedPostLoad;

	BOOL m_bVisible; 

	CString m_strLastSavedTopicName;
	CString m_strLastSavedParentTopicName;
	//TES 4/15/2010 - PLID 24692 - We don't maintain our own order index any more.
	//long m_nLastSavedTopicOrderIndex;
	BOOL m_bHasMoved;
	BOOL m_bLastSavedShowIfEmpty;
	// (a.walling 2007-10-15 15:51) - PLID 27664 - Last saved HTML
	//CString m_strLastSavedHTML; // (a.walling 2013-03-13 08:40) - PLID 55632 - Ignore unsaved is no longer applicable

	// (c.haag 2007-05-08 10:55) - PLID 25928 - TRUE if we need to call RepositionDetailsInTopic
	// when the load is done
	BOOL m_bNeedsToRepositionDetailsInPostLoad;

	// (a.walling 2011-10-20 14:23) - PLID 46075 - Clean up interaction with external interfaces
	CEmrTopicWndRef m_pTopicWnd;

	CEmrTopicHeaderWndRef m_pTopicHeaderWnd;

	BOOL m_bUnsaved;
	// (j.jones 2007-01-31 14:25) - PLID 24515 - added bForceSave, so if you make a change
	// before PostLoad() finishes, it will not mark the topic as saved again
	BOOL m_bForceSave;

	CArray<CEMNDetail*, CEMNDetail*> m_aryDeletedDetails;
	CArray<CEMRTopic*, CEMRTopic*> m_aryDeletedTopics;

	//Used to prevent details from unspawning their own topic.
	BOOL m_bAllowUnspawn;
};

inline long CEMRTopic::GetEMNDetailCount()
{
	EnsureDetails();
	return m_arypEMNDetails.GetSize();
}

inline long CEMRTopic::GetSubTopicCount()
{
	EnsureTopics();
	return m_arypEMRTopics.GetSize();
}

inline CEMRTopic* CEMRTopic::GetSubTopic(long nIndex)
{
	EnsureTopics();
	return m_arypEMRTopics[nIndex];
}

inline long CEMRTopic::GetDeletedSubTopicCount() const
{
	return m_aryDeletedTopics.GetSize();
}

inline CEMRTopic* CEMRTopic::GetDeletedSubTopic(long nIndex) const
{
	return m_aryDeletedTopics[nIndex];
}

inline CEMNDetail* CEMRTopic::GetDetailByIndex(long nIndex) 
{
	if (nIndex >= 0 && nIndex < m_arypEMNDetails.GetSize()) {
		return (CEMNDetail*)(m_arypEMNDetails.GetAt(nIndex));
	}
	else {
		if(EnsureDetails()) { //EnsureDetails() only returns TRUE if it actually loaded them.
			//Maybe that detail hadn't been loaded until now.
			return GetDetailByIndex(nIndex);
		}
		return NULL;
	}
}

#endif
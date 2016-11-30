#include "stdafx.h"
#include "EMRTopic.h"
#include "EMNDetail.h"
#include "EMN.h"
#include "EmrTopicWnd.h"
#include "EmrTreeWnd.h"
#include "AuditTrail.h"
#include "GlobalUtils.h"
#include "EMRItemAdvDlg.h"
#include "EmrItemAdvImageDlg.h"
#include "EMRDebugDlg.h"
#include <NxAlgorithm.h>
#include "EMNDetailStructures.h"
#include "EMR.h"
#include "NxAutoQuantum.h"
#include <OpticalUtils.h>
using namespace ADODB;

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2013-05-01 10:33) - PLID 55632 - Removed all the last saved html (m_strLastSaveHTML, ClearLastSavedHTML etc)

// (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs

// (c.haag 2007-04-24 11:01) - PLID 25881 - We now use the official EMN loader
// object rather than the old preloaded detail array object
#include "EMNLoader.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (c.haag 2007-05-23 16:15) - PLID 26115 - This macro will throw an exception
// if not invoked from the main thread
// (j.jones 2008-10-30 16:43) - PLID 31869 - This was called ASSERT_IN_MAIN_THREAD but the purpose was
// so it couldn't be called in a worker thread that would access the main interface. However if this
// can be called from a known safe worker thread that does not interact with an EMR interface, such as 
// the Problem List threads, then that too is a safe thread.
#define ASSERT_IN_SAFE_THREAD(funcname) { \
	extern CPracticeApp theApp; \
	long nProblemListDlg_PopulateDetailValues_CurThreadID = -1; \
	long nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID = -1; \
	if(GetMainFrame()) { \
		nProblemListDlg_PopulateDetailValues_CurThreadID = GetMainFrame()->m_nProblemListDlg_PopulateDetailValues_CurThreadID; \
		nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID = GetMainFrame()->m_nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID; \
	} \
	if(theApp.m_nThreadID != GetCurrentThreadId() \
		&& (nProblemListDlg_PopulateDetailValues_CurThreadID == -1 || nProblemListDlg_PopulateDetailValues_CurThreadID != GetCurrentThreadId()) \
		&& (nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID == -1 || nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID != GetCurrentThreadId()) \
		) { \
		ASSERT(FALSE); \
		ThrowNxException("Attempted to call %s outside a safe thread!", funcname); \
	}\
}

//TES 4/15/2010 - PLID 24692 - Pass in this topic's position in the EMN's linked list.
CEMRTopic::CEMRTopic(CEMN *pParentEMN, TopicPositionEntry *tpe)
{
	m_nID = -1;
	m_bIsTemplate = pParentEMN->IsTemplate();
	m_nTemplateTopicID = -1;
	m_nTemplateID = -1;
	m_nTemplateTopicOrderIndex = -1;
	m_pParentEMN = pParentEMN;
	m_bOwnParent = FALSE;
	m_pParentTopic = NULL;
	m_pOriginalParentTopic = NULL;
	//DRT 9/25/2007 - PLID 27515
	m_nSourceActionSourceID = -1;
	m_nSourceActionSourceDataGroupID = -1;
	m_nSourceActionSourceHotSpotGroupID = -1;
	m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:43) - PLID 33070
	m_SourceActionDestType = eaoInvalid;
	m_nOriginalTemplateTopicID = -1;
	m_nOriginalTemplateID = -1; // (a.walling 2007-03-21 10:37) - PLID 25301 - initialize
	//If we're a new topic on an EMN, then we always want to be visible, otherwise, hide us when empty.
	m_bShowIfEmpty = m_bIsTemplate?FALSE:TRUE;
	m_bUnsaved = FALSE;
	m_bForceSave = FALSE;
	m_bVisible = TRUE;
	m_bHideOnEMN = FALSE;
	m_bOwnChildren = TRUE;
	m_bAllowUnspawn = TRUE;
	m_bHasSubTopics = FALSE;
	m_bHasDetails = FALSE;
	m_CompletionStatus = etcsInvalid;

	m_pLoadInfo = NULL;
	m_bDetailsLoaded = FALSE;
	m_bTopicsLoaded = FALSE;
	m_bPostLoadCalled = FALSE;
	m_bPostLoadComplete = FALSE;
	m_bSubTopicArrayChanged = FALSE;
	m_bHasMoved = FALSE;
	m_bLastSavedShowIfEmpty = m_bShowIfEmpty;
	m_bNeedPostLoad = FALSE;
	m_bNeedsToRepositionDetailsInPostLoad = FALSE;

	// (a.walling 2008-06-30 12:36) - PLID 29271
	m_nPreviewFlags = 0;

	//TES 5/3/2010 - PLID 24692 - In some cases, we now permit this to be NULL.
	m_pTopicPositionEntry = tpe;
}

//TES 4/15/2010 - PLID 24692 - Pass in this topic's position in the EMN's linked list.
CEMRTopic::CEMRTopic(CEMRTopic *pParentTopic, TopicPositionEntry *tpe)
{
	m_nID = -1;
	m_bIsTemplate = pParentTopic->IsTemplate();
	m_nTemplateTopicID = -1;
	m_nTemplateID = -1;
	m_nTemplateTopicOrderIndex = -1;
	m_pParentEMN = NULL;
	m_pParentTopic = pParentTopic;
	m_pOriginalParentTopic = m_pParentTopic;
	if(m_pOriginalParentTopic)
		m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
	m_bOwnParent = FALSE;
	//DRT 9/25/2007 - PLID 27515
	m_nSourceActionSourceID = -1;
	m_nSourceActionSourceDataGroupID = -1;
	m_nSourceActionSourceHotSpotGroupID = -1;
	m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:43) - PLID 33070
	m_SourceActionDestType = eaoInvalid;
	m_nOriginalTemplateTopicID = -1;
	m_nOriginalTemplateID = -1; // (a.walling 2007-03-21 10:37) - PLID 25301 - initialize
	//If we're a new topic on an EMN, then we always want to be visible, otherwise, hide us when empty.
	m_bShowIfEmpty = m_bIsTemplate?FALSE:TRUE;
	m_bUnsaved = FALSE;
	m_bForceSave = FALSE;
	m_bVisible = TRUE;
	m_bHideOnEMN = FALSE;
	m_bOwnChildren = TRUE;
	m_bAllowUnspawn = TRUE;
	m_bHasSubTopics = FALSE;
	m_bHasDetails = FALSE;
	m_CompletionStatus = etcsInvalid;

	m_pLoadInfo = NULL;
	m_bDetailsLoaded = FALSE;
	m_bTopicsLoaded = FALSE;
	m_bPostLoadCalled = FALSE;
	m_bPostLoadComplete = FALSE;
	m_bSubTopicArrayChanged = FALSE;
	m_bHasMoved = FALSE;
	m_bLastSavedShowIfEmpty = m_bShowIfEmpty;
	m_bNeedPostLoad = FALSE;
	m_bNeedsToRepositionDetailsInPostLoad = FALSE;

	// (a.walling 2008-06-30 12:36) - PLID 29271
	m_nPreviewFlags = 0;

	//TES 5/3/2010 - PLID 24692 - In some cases, we now permit this to be NULL.
	m_pTopicPositionEntry = tpe;
}

//TES 4/15/2010 - PLID 24692 - Pass in this topic's position in the EMN's linked list.
CEMRTopic::CEMRTopic(BOOL bIsTemplate, BOOL bOwnsParent, TopicPositionEntry *tpe)
{
	m_nID = -1;
	m_bIsTemplate = bIsTemplate;
	m_nTemplateTopicID = -1;
	m_nTemplateID = -1;
	m_nTemplateTopicOrderIndex = -1;
	m_pParentEMN = NULL;
	m_pParentTopic = NULL;
	m_pOriginalParentTopic = NULL;
	m_bOwnParent = bOwnsParent;
	//DRT 9/25/2007 - PLID 27515
	m_nSourceActionSourceID = -1;
	m_nSourceActionSourceDataGroupID = -1;
	m_nSourceActionSourceHotSpotGroupID = -1;
	m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:44) - PLID 33070
	m_SourceActionDestType = eaoInvalid;
	m_nOriginalTemplateTopicID = -1;
	m_nOriginalTemplateID = -1; // (a.walling 2007-03-21 10:37) - PLID 25301 - initialize
	//If we're a new topic on an EMN, then we always want to be visible, otherwise, hide us when empty.
	m_bShowIfEmpty = m_bIsTemplate?FALSE:TRUE;
	m_bUnsaved = FALSE;
	m_bForceSave = FALSE;
	m_bVisible = TRUE;
	m_bHideOnEMN = FALSE;
	m_bOwnChildren = TRUE;
	m_bAllowUnspawn = TRUE;
	m_bHasSubTopics = FALSE;
	m_bHasDetails = FALSE;
	m_CompletionStatus = etcsInvalid;

	m_pLoadInfo = NULL;
	m_bDetailsLoaded = FALSE;
	m_bTopicsLoaded = FALSE;
	m_bPostLoadCalled = FALSE;
	m_bPostLoadComplete = FALSE;
	m_bSubTopicArrayChanged = FALSE;
	m_bHasMoved = FALSE;
	m_bLastSavedShowIfEmpty = m_bShowIfEmpty;
	m_bNeedPostLoad = FALSE;
	m_bNeedsToRepositionDetailsInPostLoad = FALSE;

	// (a.walling 2008-06-30 12:36) - PLID 29271
	m_nPreviewFlags = 0;

	//TES 5/3/2010 - PLID 24692 - In some cases, we now permit this to be NULL.
	m_pTopicPositionEntry = tpe;
}

CEMRTopic::~CEMRTopic()
{
	//TES 6/22/2006 - Once we passed this into CEMR::Load(), it took ownership of it, it is no longer our job to clean it up.
	/*if(m_pLoadInfo) {
		//If we're already loaded, this should return immediately, otherwise, we can't delete this variable until the thread
		//is done with it in any case.
		m_pLoadInfo->m_mtxCanDelete.Lock();
		delete m_pLoadInfo;
		m_pLoadInfo = NULL;
	}*/

	if (GetTopicWnd()) {
		ASSERT(GetTopicWnd()->GetSafeHwnd());
		if (GetTopicWnd()->GetSafeHwnd()) {
			GetTopicWnd()->DestroyWindow();
		}
		m_pTopicWnd.reset();
	}

	// (a.walling 2008-11-17 17:20) - PLID 32030 - This topic is being destroyed, so ensure
	// no details are holding on to a reference to invalid memory
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail* pDetail = m_arypEMNDetails[i];

		if (pDetail->m_pParentTopic == this) {
			pDetail->m_pParentTopic = NULL;
		}
	}

	if(m_bOwnChildren) {
		for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
			// (c.haag 2007-05-22 12:14) - PLID 26095 - Release our details
			//m_arypEMNDetails[i]->Release();
			// (a.walling 2009-10-12 16:05) - PLID 36024
			m_arypEMNDetails[i]->__Release("~CEMRTopic own children");
			//delete (CEMNDetail*)m_arypEMNDetails.GetAt(i);
		}

		for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
			delete (CEMRTopic*)m_arypEMRTopics.GetAt(i);
		}
	}

	//DRT 8/7/2007 - PLID 26928 - Check for any pending deletions in the array -- it should be
	//	impossible that these is anything remaining.
	//DRT 10/17/2007 - PLID 27788 - There is at least 1 known case where this can pop up legitimately due to a bug
	//	not yet fixed.
	ASSERT(m_paryPendingDeletionDetails.GetSize() == 0);

	// (a.walling 2009-10-13 14:22) - PLID 36024
	for(int i = m_paryPendingDeletionDetails.GetSize() - 1; i >= 0; i--) {
		CEMNDetail *pPendingDeletionDetail = m_paryPendingDeletionDetails.GetAt(i);
		// (a.walling 2009-10-13 14:20) - PLID 36024
		pPendingDeletionDetail->__Release("~CEMRTopic dangling pending deletion detail");
	}
	m_paryPendingDeletionDetails.RemoveAll();


	for(int i = 0; i < m_aryDeletedTopics.GetSize(); i++) {
		delete (CEMRTopic*)m_aryDeletedTopics.GetAt(i);
	}

	for(i = 0; i < m_aryDeletedDetails.GetSize(); i++) {
		// (c.haag 2009-07-06 11:48) - PLID 34786 - As we do with non-deleted details, make sure we nullify the detail's topic
		CEMNDetail* pDetail = m_aryDeletedDetails[i];
		if (pDetail->m_pParentTopic == this) {
			pDetail->m_pParentTopic = NULL;
		}
		// (c.haag 2007-05-24 16:58) - PLID 26095 - Use Release() instead of delete
		//pDetail->Release();
		// (a.walling 2009-10-12 16:05) - PLID 36024
		pDetail->__Release("~CEMRTopic deleted details");
	}

	// (j.jones 2008-07-22 11:41) - PLID 30789 - remove all problems
	// (c.haag 2009-05-16 13:32) - PLID 34311 - Deleting problem link objects
	for(i = 0; i < m_apEmrProblemLinks.GetSize(); i++) {
		delete m_apEmrProblemLinks[i];
	}
	m_apEmrProblemLinks.RemoveAll();

	if(m_bOwnParent) {
		if(m_pParentEMN) {
			delete m_pParentEMN;
			m_pParentEMN = NULL;
		}
		if(m_pParentTopic) {

			if(m_pParentTopic == m_pOriginalParentTopic)
				m_pOriginalParentTopic = NULL;

			delete m_pParentTopic;
			m_pParentTopic = NULL;
		}
		if(m_pOriginalParentTopic) {
			//this case is probably impossible
			delete m_pOriginalParentTopic;
			m_pOriginalParentTopic = NULL;
		}
	}
}

// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
ADODB::_ConnectionPtr CEMRTopic::GetRemoteData()
{
	if (m_pParentEMN) {
		return m_pParentEMN->GetRemoteData();
	} else if (m_pParentTopic) {
		return m_pParentTopic->GetRemoteData();
	} else {
		return ::GetRemoteData();
	}
}

#ifdef _DEBUG
void CEMRTopic::DebugReportMembers(CEmrDebugDlg* pDlg, BOOL bOnlyInclude8300Fields, BOOL bAllowAddresses)
{
	// (c.haag 2007-08-07 11:45) - PLID 26946 - This is used for debugging and developer testing with EMR
	// (c.haag 2007-09-17 11:35) - PLID 27401 - We now do output through the dialog
	// (c.haag 2007-09-17 13:08) - PLID 27408 - We now support suppressing address information
	pDlg->Output(FormatString("Report for topic %d (%s):\n", m_nID, m_strTopicName));
	pDlg->Output(FormatString("=========================\n"));
	pDlg->Output(FormatString("m_nID = %d\n", m_nID));
	pDlg->Output(FormatString("m_bIsTemplate = %d\n", m_bIsTemplate));
	pDlg->Output(FormatString("m_nTemplateTopicID = %d\n", m_nTemplateTopicID));
	pDlg->Output(FormatString("m_nTemplateID = %d\n", m_nTemplateID));
	pDlg->Output(FormatString("m_nTemplateTopicOrderIndex = %d\n", m_nTemplateTopicOrderIndex));
	pDlg->Output(FormatString("m_bOwnParent = %d\n", m_bOwnParent));
	pDlg->Output(FormatString("m_nSourceActionID = %d\n", m_sai.nSourceActionID));
	pDlg->Output(FormatString("m_nSourceDetailID = %d\n", m_sai.nSourceDetailID));
	pDlg->Output(FormatString("m_nSourceDataGroupID = %d\n", m_sai.GetDataGroupID()));
	pDlg->Output(FormatString("m_SourceActionDestType = %d\n", m_SourceActionDestType));
	pDlg->Output(FormatString("m_nOriginalTemplateTopicID = %d\n", m_nOriginalTemplateTopicID));
	pDlg->Output(FormatString("m_nOriginalTemplateID = %d\n", m_nOriginalTemplateID));
	pDlg->Output(FormatString("m_bShowIfEmpty = %d\n", m_bShowIfEmpty));
	pDlg->Output(FormatString("m_bUnsaved = %d\n", m_bUnsaved));
	pDlg->Output(FormatString("m_bForceSave = %d\n", m_bForceSave));
	pDlg->Output(FormatString("m_bVisible = %d\n", m_bVisible));
	pDlg->Output(FormatString("m_bHideOnEMN = %d\n", m_bHideOnEMN));
	pDlg->Output(FormatString("m_bOwnChildren = %d\n", m_bOwnChildren));
	pDlg->Output(FormatString("m_bAllowUnspawn = %d\n", m_bAllowUnspawn));
	pDlg->Output(FormatString("m_bHasSubTopics = %d\n", m_bHasSubTopics));
	pDlg->Output(FormatString("m_bHasDetails = %d\n", m_bHasDetails));
	pDlg->Output(FormatString("m_CompletionStatus = %d\n", m_CompletionStatus));
	if (!bOnlyInclude8300Fields) {
		// Add new fields here
		// (c.haag 2007-09-17 13:08) - PLID 27408 - Different version if addresses are not allowed
		if (!bAllowAddresses) {
			pDlg->Output(FormatString("m_pLoadInfo ID=%s\n", m_pLoadInfo ? AsString(m_pLoadInfo->nID) : "(null)"));
		} else {
			pDlg->Output(FormatString("m_pLoadInfo = 0x%08x (ID=%s)\n", m_pLoadInfo, m_pLoadInfo ? AsString(m_pLoadInfo->nID) : "(null)")); // (a.walling 2007-08-08 13:13)
		}

		pDlg->Output(FormatString("SourceActionID = %d\n", m_sai.nSourceActionID));
		pDlg->Output(FormatString("SourceDetailID = %d\n", m_sai.nSourceDetailID));
		pDlg->Output(FormatString("SourceDataGroupID = %d\n", m_sai.GetDataGroupID()));
		pDlg->Output(FormatString("SourceDetailDetailImageStampID = %d\n", m_sai.GetDetailStampID()));

		//DRT 9/27/2007 - PLID 27515 - Extra source action info
		pDlg->Output(FormatString("m_nSourceActionSourceID = %d\n", m_nSourceActionSourceID));
		pDlg->Output(FormatString("m_nSourceActionSourceDataGroupID = %d\n", m_nSourceActionSourceDataGroupID));
		pDlg->Output(FormatString("m_nSourceActionSourceHotSpotGroupID = %d\n", m_nSourceActionSourceHotSpotGroupID));
		// (z.manning 2009-02-13 10:44) - PLID 33070
		pDlg->Output(FormatString("m_nSourceActionSourceTableDropdownGroupID = %d\n", m_nSourceActionSourceTableDropdownGroupID));
		//TES 3/17/2010 - PLID 37530
		pDlg->Output(FormatString("m_nSourceStampID = %d\n", m_sai.GetStampID()));
		pDlg->Output(FormatString("m_nSourceStampIndex = %d\n", m_sai.GetStampIndexInDetailByType()));
		pDlg->Output(FormatString("m_strSourceActionName = %s\n", m_strSourceActionName));

		//TES 4/15/2010 - PLID 24692 - Output the linked list of topics (starting from this topic).
		pDlg->Output(FormatString("m_pTopicPositionEntry = %s\n", m_pTopicPositionEntry->GetOutputText()));
	}
	pDlg->Output("\n");
}
#endif

long CEMRTopic::GetID() const
{
	return m_nID;
}

// (a.walling 2012-04-30 10:58) - PLID 46176 - Ensure something is displayed rather than nothing if the topic's label is blank
CString CEMRTopic::GetDisplayName() const
{
	if (!m_strTopicName.IsEmpty()) {
		return m_strTopicName;
	} else {
		return "(Untitled Topic)" ;
	}
}

CEMN* CEMRTopic::GetParentEMN() const
{
	if(m_pParentEMN) {
		return m_pParentEMN;
	}
	else {
		if(m_pParentTopic)
			return m_pParentTopic->GetParentEMN();
	}

	return NULL;
}

// (a.walling 2010-04-01 10:31) - PLID 38013 - Added function to get the parent EMR
CEMR* CEMRTopic::GetParentEMR() const
{
	return GetParentEMN() ? GetParentEMN()->GetParentEMR() : NULL;
}

// (z.manning 2010-08-20 09:49) - PLID 40190 - Added connection pointer
void CEMRTopic::LoadFromPreloadedTopic(ADODB::_Connection *lpCon, CEMNLoader* pLoader, long nTopicID, long nPatientID,
									   HWND hWnd, OPTIONAL IN CEMR *pEmr /*= NULL*/)
{
	//
	// (c.haag 2007-05-01 09:39) - PLID 25853 - This is effectively a fast overload of LoadFromTopicFields.
	// We populate a load info object with data, and pass it along to our parent EMR's call to LoadTopic.
	// This is only called for EMN topics, not template topics.
	//
	// I also heeded Bob's warning in LoadFromTopicFields, but decided to use exception handling for
	// consistency and to minimize non-data-accessing behavioral changes where possible.
	//
	try {
		if (NULL == pLoader) ThrowNxException("The loader object is invalid!");
		// (c.haag 2007-07-03 10:10) - PLID 26523 - Request access to topic information for
		// thread safety
		CHoldEMNLoaderMutex mh(pLoader->GetTopicsMutex());
		CEMNLoader::CPreloadedTopic* pTopic = pLoader->GetPreloadedTopicByID(nTopicID);
		if (NULL == pTopic) ThrowNxException("The topic object is invalid!");

		m_nID = pTopic->m_nID;
		m_bIsTemplate = FALSE;
		m_strTopicName = VarString(pTopic->m_vName);
		m_bHasSubTopics = (pTopic->m_anSubTopicIDs.GetSize() > 0) ? TRUE : FALSE;
		m_bHasDetails = pTopic->m_bHasDetails;
		
		// (a.walling 2008-06-30 14:20) - PLID 29271 - Preview Pane flags
		m_nPreviewFlags = VarLong(pTopic->m_vPreviewFlags, 0);

		CEMR *pParentEmr = pEmr ? pEmr : GetParentEMN()->GetParentEMR();

		m_pLoadInfo = new EMRTopicLoadInfo;
		m_pLoadInfo->nID = m_nID;
		m_pLoadInfo->bLoadFromTemplate = FALSE;
		m_pLoadInfo->bLoadToTemplate = FALSE;
		m_pLoadInfo->bIsNewTopic = FALSE;
		//DRT 9/25/2007 - PLID 27515
		m_pLoadInfo->nOverrideSourceActionSourceID = -1;
		m_pLoadInfo->nOverrideSourceActionSourceDataGroupID = -1;
		m_pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID = -1;
		m_pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:44) - PLID 33070	
		//DRT 9/25/2007 - PLID 27515
		m_pLoadInfo->m_nSourceActionSourceID = -1;
		m_pLoadInfo->m_nSourceActionSourceDataGroupID = -1;
		m_pLoadInfo->m_nSourceActionSourceHotSpotGroupID = -1;
		m_pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:45) - PLID 33070
		m_pLoadInfo->bLoadHiddenTopics = FALSE;
		// (a.walling 2008-06-30 15:01) - PLID 29271 - Preview Pane Flags
		m_pLoadInfo->m_nPreviewFlags = 0;
		m_pLoadInfo->nPatientID = nPatientID;
		// (j.jones 2008-09-22 15:54) - PLID 31408 - added nEMRGroupID
		long nEMRGroupID = -1;
		if(pParentEmr) {
			nEMRGroupID = pParentEmr->GetID();
		}
		m_pLoadInfo->nEMRGroupID = nEMRGroupID;
		m_pLoadInfo->hWnd = hWnd;
		m_pLoadInfo->m_pLoader = pLoader;
		pLoader->AddRef();
		// (c.haag 2007-07-03 10:10) - PLID 26523 - We're all done with the topic object
		mh.Release();

		pParentEmr->LoadTopic(lpCon, m_pLoadInfo);
		if(!hWnd) {
			//They didn't give us a window to post back to when we're finished, so CEMR::LoadTopic will have loaded us synchronously.
			//Make sure the load process is complete.
			if(GetParentEMN()) {
				//If we don't have a parent EMN, that means we're being loaded in a thread and haven't been attached to a parent yet.
				//We call PostLoad in the SetParent___() functions so as to handle that case.
				//TES 2/7/2007 - PLID 24652 - However, note the comment in the else branch.
				PostLoad();
			}
			else {
				//TES 2/7/2007 - PLID 24652 - Make sure that callers know that PostLoad() hasn't been called, and therefore
				// they need to call it as soon as this object has been fully integrated into the EMN.  Despite the comment
				// in the top branch of this logic, we can't rely on the SetParent___() functions, they sometimes are also
				// called before the object is fully integrated, and therefore can't call PostLoad() either.
				m_bNeedPostLoad = TRUE;
			}
		}
	}
	NxCatchAllThread("Error in CEMRTopic::LoadFromPreloadedTopic()");
}

// (z.manning 2010-08-20 09:47) - PLID 40190 - Added connection pointer
void CEMRTopic::LoadFromTopicFields(ADODB::_Connection *lpCon, Fields *lpfldsTopicInfo, HWND hWnd, OPTIONAL IN CEMR *pEmr /*= NULL*/,
									OPTIONAL IN CEMNLoader* pLoader /*= NULL*/)
{
	try {
		// (b.cardillo 2006-06-01 16:39) - PLID 20861 - We now pull everything from the Fields 
		// object that was given to us as a parameter instead of generating our own recordset.
		FieldsPtr pfldsTopicInfo(lpfldsTopicInfo);
				
		m_nID = AdoFldLong(pfldsTopicInfo->GetItem("ID"));
		m_bIsTemplate = FALSE;
		
		long nPatientID = -1;
		//We need to synchronously load the things that are immediately visible (name, and whether there are subtopics.
		m_strTopicName = AdoFldString(pfldsTopicInfo, "Name");
		m_bHasSubTopics = AdoFldLong(pfldsTopicInfo, "HasSubTopics") ? TRUE : FALSE;
		m_bHasDetails = AdoFldLong(pfldsTopicInfo, "HasDetails") ? TRUE : FALSE;
		nPatientID = AdoFldLong(pfldsTopicInfo, "PatientID");
		// (a.walling 2008-06-30 14:20) - PLID 29271 - Preview Pane flags
		m_nPreviewFlags = AdoFldLong(pfldsTopicInfo, "PreviewFlags", 0);
				
		m_pLoadInfo = new EMRTopicLoadInfo;
		m_pLoadInfo->nID = m_nID;
		m_pLoadInfo->bLoadFromTemplate = FALSE;
		m_pLoadInfo->bLoadToTemplate = FALSE;
		m_pLoadInfo->bIsNewTopic = FALSE;
		//DRT 9/25/2007 - PLID 27515
		m_pLoadInfo->nOverrideSourceActionSourceID = -1;
		m_pLoadInfo->nOverrideSourceActionSourceDataGroupID = -1;
		m_pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID = -1;
		m_pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:45) - PLID 33070	
		//DRT 9/25/2007 - PLID 27515
		m_pLoadInfo->m_nSourceActionSourceID = -1;
		m_pLoadInfo->m_nSourceActionSourceDataGroupID = -1;
		m_pLoadInfo->m_nSourceActionSourceHotSpotGroupID = -1;
		m_pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:46) - PLID 33070
		m_pLoadInfo->bLoadHiddenTopics = FALSE;
		// (a.walling 2008-06-30 15:01) - PLID 29271 - Preview Pane Flags
		m_pLoadInfo->m_nPreviewFlags = 0;
		m_pLoadInfo->nPatientID = nPatientID;
		
		// (j.jones 2008-09-22 15:54) - PLID 31408 - added nEMRGroupID
		long nEMRGroupID = -1;
		if(pEmr) {
			nEMRGroupID = pEmr->GetID();
		}
		m_pLoadInfo->nEMRGroupID = nEMRGroupID;

		m_pLoadInfo->hWnd = hWnd;
		// (c.haag 2007-02-27 09:05) - PLID 24949 - Add our preloaded detail array to the
		// load info object. Later in the topic loading thread, it will be processed in favor
		// of using recordsets.
		m_pLoadInfo->m_pLoader = pLoader;
		if (pLoader) pLoader->AddRef();

		//DRT 8/16/2007 - PLID 25582 - I believe that once we got to this case and GetParentEMN() was NULL.  Note that the comment
		//	below says that it is valid for us not to have a parent EMN here.  However, at current time, this function can only
		//	be called from 1 place, and that 1 place passes in a valid pEmr pointer, so this should be moot from here out.
		//Regardless, I'm improving the error catching here so that we can better tell what the error is if it happens again, 
		//	rather than getting a nameless neen error.
		//
		//If the pEmr is not NULL, then we don't care about the parent EMN.  If it is NULL, then GetParentEMN() had 
		//	best be legit.
		if(pEmr == NULL && GetParentEMN() == NULL) {
			//This will cause this topic to fail its load
			AfxThrowNxException("Attempted to load a topic which does not have a valid parent EMN.");
		}

		CEMR *pParentEmr = pEmr ? pEmr : GetParentEMN()->GetParentEMR();
		pParentEmr->LoadTopic(lpCon, m_pLoadInfo);
		if(!hWnd) {
			//They didn't give us a window to post back to when we're finished, so CEMR::LoadTopic will have loaded us synchronously.
			//Make sure the load process is complete.
			if(GetParentEMN()) {
				//If we don't have a parent EMN, that means we're being loaded in a thread and haven't been attached to a parent yet.
				//We call PostLoad in the SetParent___() functions so as to handle that case.
				//TES 2/7/2007 - PLID 24652 - However, note the comment in the else branch.
				PostLoad();
			}
			else {
				//TES 2/7/2007 - PLID 24652 - Make sure that callers know that PostLoad() hasn't been called, and therefore
				// they need to call it as soon as this object has been fully integrated into the EMN.  Despite the comment
				// in the top branch of this logic, we can't rely on the SetParent___() functions, they sometimes are also
				// called before the object is fully integrated, and therefore can't call PostLoad() either.
				m_bNeedPostLoad = TRUE;
			}
		}
	// (c.haag 2007-03-19 13:15) - PLID 24663 - Use thread-safe exception handling
	}NxCatchAllThread("Error in CEMRTopic::LoadFromTopicFields()");

	// (b.cardillo 2006-06-01 16:18) - I don't think we should be catching exceptions in this 
	// function or LoadFromEmnID() or any number of other similar functions throughout EMR, as 
	// they are clearly utility functions.  I let t.schneider know my thinking and I leave it 
	// to him to decide what to do.
}

// (c.haag 2007-02-27 10:12) - PLID 24949 - Added pLoader for cases where the details have already been loaded except for their states
void CEMRTopic::LoadFromTopicID(long nEMRTopicID, HWND hWnd, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/, OPTIONAL IN CEMR *pEmr /*= NULL*/,
								OPTIONAL IN CEMNLoader* pLoader /*= NULL*/)
{
	try {
		_ConnectionPtr pCon;
		if(lpCon) pCon = lpCon;
		else pCon = GetRemoteData();
		// Get the recordset and then call the standard implementation instead
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr rsTopic = CreateParamRecordset(pCon, "SELECT EmrTopicsT.ID, EmrTopicsT.Name, EmrMasterT.PatientID, "
			"CASE WHEN EXISTS (SELECT EmrTopicsT.ID FROM EmrTopicsT ChildTopic LEFT JOIN EmrDetailsT ON ChildTopic.ID = "
			"EmrDetailsT.EmrTopicID LEFT JOIN EmrTopicsT SubTopic ON ChildTopic.ID = SubTopic.EmrParentTopicID "
			"WHERE ChildTopic.EmrParentTopicID = EmrTopicsT.ID AND (EmrTopicsT.ShowIfEmpty = 1 OR EmrDetailsT.ID Is Not NULL "
			"OR SubTopic.ID Is Not Null)) THEN 1 ELSE 0 END AS HasSubTopics, "
			"CASE WHEN EXISTS (SELECT ID FROM EmrDetailsT WHERE EmrDetailsT.EmrTopicID = EmrTopicsT.ID) THEN 1 ELSE 0 END "
			"AS HasDetails, EmrTopicsT.ShowIfEmpty, "
			"SourceTemplateTopics.TemplateID AS OriginalTemplateID, " // (a.walling 2007-03-21 10:22) - PLID 25301 - Load original template id for spawned topics
			// (a.walling 2008-06-30 14:23) - PLID 29271 - Preview Pane flags
			"EmrTopicsT.PreviewFlags "
			"FROM EmrTopicsT INNER JOIN EmrMasterT ON EmrTopicsT.EmrID = EmrMasterT.ID "
			"LEFT JOIN EMRTemplateTopicToTopicLinkT ON EMRTopicsT.ID = EMRTemplateTopicToTopicLinkT.EmrTopicID "
			"LEFT JOIN EMRTemplateTopicsT ON EMRTemplateTopicToTopicLinkT.EmrTemplateTopicID = EmrTemplateTopicsT.ID "
			"LEFT JOIN EMRTemplateTopicsT AS SourceTemplateTopics ON EmrTemplateTopicsT.SourceTemplateTopicID = SourceTemplateTopics.ID "
			" WHERE EmrTopicsT.ID = {INT}", nEMRTopicID);
		ASSERT(!rsTopic->eof);
		LoadFromTopicFields(pCon, rsTopic->GetFields(), hWnd, pEmr, pLoader);
		rsTopic->Close();
	// (c.haag 2007-03-19 13:15) - PLID 24663 - Use thread-safe exception handling
	}NxCatchAllThread("Error in CEMRTopic::LoadFromTopicID()");

	// (b.cardillo 2006-06-01 16:18) - I don't think we should be catching exceptions in this 
	// function or LoadFromEmnID() or any number of other similar functions throughout EMR, as 
	// they are clearly utility functions.  I let t.schneider know my thinking and I leave it 
	// to him to decide what to do.
}

// (c.haag 2007-02-27 10:44) - PLID 24949 - Added pLoader for cases where the details have already been loaded except for their states
// (z.manning 2009-02-13 10:46) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
// (z.manning 2009-03-05 09:18) - PLID 33338 - Use the new source action info class
void CEMRTopic::LoadFromTemplateTopicID(long nEmrTemplateTopicID, BOOL bIsNewTopic, HWND hWnd, SourceActionInfo &sai, BOOL bLoadHiddenTopics /*= FALSE*/, OPTIONAL IN long nPatientID /*= -1*/, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/, OPTIONAL IN CEMR *pEmr /*= NULL*/,
										OPTIONAL IN CEMNLoader* pLoader /*= NULL*/, OPTIONAL IN long nSourceActionSourceID /*= -1*/, OPTIONAL IN long nSourceActionSourceDataGroupID /*= -1*/, OPTIONAL IN long nSourceActionSourceHotSpotGroupID /* = -1 */, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID /* = -1 */)
{
	try {
		_ConnectionPtr pCon;
		if(lpCon) pCon = lpCon;
		else pCon = GetRemoteData();

		if (nEmrTemplateTopicID <= 0) {
			ASSERT(FALSE);
			ThrowNxException("CEMRTopic::LoadFromTemplateTopicID was called with an invalid template topic ID!");
		}

		//NOTE: This needs to be kept in sync with CEMN::LoadFromTemplateID.
		//We need to synchronously load the things that are immediately visible (name, and whether there are subtopics.
		//DRT 9/25/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
		// (z.manning, 01/23/2008) - PLID 28690 - Added hot spot group ID
		// (z.manning 2009-02-13 10:46) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
		// (z.manning 2009-03-05 12:22) - PLID 33338 - SourceDataGroupID
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr rsTemplateTopic = CreateParamRecordset(pCon, "SELECT EmrTemplateTopicsT.ID, "
			//If we are a spawned template topic, use the source template topic's name.
			"CASE WHEN ({INT} <> -1 OR EmrTemplateTopicsT.SourceActionID <> -1) AND SourceEMRTemplateTopicsT.Name Is Not NULL "
			"THEN SourceEMRTemplateTopicsT.Name ELSE EmrTemplateTopicsT.Name END AS Name, "
			"CASE WHEN EXISTS (SELECT ID FROM EmrTemplateTopicsT ChildTopic WHERE ChildTopic.SourceActionID Is Null AND "
			"ChildTopic.EmrParentTemplateTopicID = EmrTemplateTopicsT.ID OR "
			"ChildTopic.EmrParentTemplateTopicID = EmrTemplateTopicsT.SourceTemplateTopicID) THEN 1 ELSE 0 END AS HasSubTopics, "
			"CASE WHEN EXISTS (SELECT ID FROM EmrTemplateDetailsT WHERE "
			"EmrTemplateDetailsT.EmrTemplateTopicID = EmrTemplateTopicsT.ID OR "
			"EmrTemplateDetailsT.EmrTemplateTopicID = EmrTemplateTopicsT.SourceTemplateTopicID) THEN 1 ELSE 0 END AS HasDetails, "
			"EmrTemplateTopicsT.ShowIfEmpty, EmrTemplateTopicsT.EmrParentTemplateTopicID, EmrTemplateTopicsT.SourceActionID, EmrTemplateTopicsT.SourceDetailID, EmrTemplateTopicsT.TemplateID, "
			"CASE WHEN SourceEMRTemplateTopicsT.TemplateID IS NULL THEN EmrTemplateTopicsT.TemplateID ELSE SourceEMRTemplateTopicsT.TemplateID END AS OriginalTemplateID, " // (a.walling 2007-03-21 10:25) - PLID 25301 - Load original template id for spawned template items
			"EmrActionsT.SourceID AS SourceActionSourceID, EMRDataT.EMRDataGroupID AS SourceActionSourceDataGroupID, EmrSpotGroupID, "
			// (a.walling 2008-06-30 14:23) - PLID 29271 - Preview Pane flags
			"EmrTemplateTopicsT.PreviewFlags, DropdownGroupID AS SourceActionSourceTableDropdownGroupID, "
			"EmrTemplateTopicsT.SourceDataGroupID, SourceType "
			"FROM EmrTemplateTopicsT LEFT JOIN EMRTemplateTopicsT AS SourceEMRTemplateTopicsT ON "
			"EMRTemplateTopicsT.SourceTemplateTopicID = SourceEMRTemplateTopicsT.ID "
			"LEFT JOIN EmrActionsT ON EmrTemplateTopicsT.SourceActionID = EmrActionsT.ID "
			// (j.jones 2010-09-22 09:04) - PLID 29039 - ensured we force a join only on data item actions
			"LEFT JOIN EMRDataT ON EMRActionsT.SourceID = EMRDataT.ID AND EmrActionsT.SourceType = {CONST} "
			"LEFT JOIN EmrImageHotSpotsT ON EmrActionsT.SourceID = EmrImageHotSpotsT.ID AND EmrActionsT.SourceType = {CONST} "
			"LEFT JOIN EmrTableDropdownInfoT ON EmrActionsT.SourceID = EmrTableDropdownInfoT.ID AND EmrActionsT.SourceType = {CONST} "
			"WHERE EmrTemplateTopicsT.ID = {INT}"
			, sai.nSourceActionID, eaoEmrDataItem, eaoEmrImageHotSpot, eaoEmrTableDropDownItem
			, nEmrTemplateTopicID);
		ASSERT(!rsTemplateTopic->eof);
		LoadFromTemplateTopicFields(pCon, rsTemplateTopic->Fields, bIsNewTopic, hWnd, sai, bLoadHiddenTopics, nPatientID, pEmr, pLoader, nSourceActionSourceID, nSourceActionSourceDataGroupID, nSourceActionSourceHotSpotGroupID, nSourceActionSourceTableDropdownGroupID);
		rsTemplateTopic->Close();
	// (c.haag 2007-03-19 13:15) - PLID 24663 - Use thread-safe exception handling
	}NxCatchAllThread("Error in CEMRTopic::LoadFromTemplateTopicID()");
}

// (c.haag 2007-02-27 10:44) - PLID 24949 - Added pLoader for cases where the details have already been loaded except for their states
// (z.manning 2009-02-13 10:49) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
// (z.manning 2009-03-05 09:19) - PLID 33338 - Use the new source action info class
// (z.manning 2010-08-20 09:51) - PLID 40190 - Added connection pointer
void CEMRTopic::LoadFromTemplateTopicFields(ADODB::_Connection *lpCon, ADODB::Fields *lpfldsTopicInfo, BOOL bIsNewTopic, HWND hWnd, SourceActionInfo &sai, BOOL bLoadHiddenTopics /*= FALSE*/, OPTIONAL IN long nPatientID /*= -1*/, OPTIONAL IN CEMR *pEmr /*= NULL*/,
											OPTIONAL IN CEMNLoader* pLoader /*= NULL*/, OPTIONAL IN long nSourceActionSourceID /*= -1*/, OPTIONAL IN long nSourceActionSourceDataGroupID /*= -1*/, OPTIONAL IN long nSourceActionSourceHotSpotGroupID /* = -1 */, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID /* = -1 */)
{
	try {
		//Three possibilities: 1.) loading a template topic onto its original template
		//2.) loading a template topic onto a different template.
		//3.) loading a template topic onto an EMN, based on the same template.
		//4.) loading a template topic onto an EMN, based on a different template.
		FieldsPtr pfldsTopicInfo(lpfldsTopicInfo);
		long nEmrTemplateTopicID = AdoFldLong(pfldsTopicInfo, "ID");
		if(m_bIsTemplate) {
			if(!bIsNewTopic) {
				//1.) loading a template topic onto its original template
				m_nID = nEmrTemplateTopicID;
				m_nTemplateTopicID = nEmrTemplateTopicID;
				//m_nOriginalTemplateTopicID will be pulled from data.
			}
			else {
				//2.) loading a template topic onto a different template.
				m_nID = -1;
				m_nTemplateTopicID = -1;
				m_nOriginalTemplateTopicID = nEmrTemplateTopicID;
				m_nOriginalTemplateID = AdoFldLong(pfldsTopicInfo, "OriginalTemplateID", -1); // (a.walling 2007-03-21 10:28) - PLID 25301
			}
		}
		else {
			if(!bIsNewTopic) {
				//3.) loading a template topic onto an EMN, based on the same template
				m_nID = -1;
				m_nTemplateTopicID = nEmrTemplateTopicID;
				//m_nOriginalTemplateTopicID will be pulled from data.
			}
			else {
				//4.) loading a template topic onto an EMN, based on a different template.
				m_nID = -1;
				m_nTemplateTopicID = nEmrTemplateTopicID;
				m_nOriginalTemplateTopicID = nEmrTemplateTopicID;
				m_nOriginalTemplateID = AdoFldLong(pfldsTopicInfo, "OriginalTemplateID", -1); // (a.walling 2007-03-21 10:28) - PLID 25301
			}
		}

		m_strTopicName = AdoFldString(pfldsTopicInfo, "Name");
		m_bHasSubTopics = AdoFldLong(pfldsTopicInfo, "HasSubTopics") ? TRUE : FALSE;
		m_bHasDetails = AdoFldLong(pfldsTopicInfo, "HasDetails") ? TRUE : FALSE;
		m_bShowIfEmpty = AdoFldBool(pfldsTopicInfo, "ShowIfEmpty");
		// (a.walling 2008-06-30 14:20) - PLID 29271 - Preview Pane flags
		m_nPreviewFlags = AdoFldLong(pfldsTopicInfo, "PreviewFlags", 0);
		
		m_pLoadInfo = new EMRTopicLoadInfo;
		m_pLoadInfo->nID = nEmrTemplateTopicID;
		m_pLoadInfo->m_nTemplateID = AdoFldLong(pfldsTopicInfo, "TemplateID", -1); // (a.walling 2007-03-21 17:24) - PLID 25301 - Set the template ID
		m_pLoadInfo->bLoadFromTemplate = TRUE;
		m_pLoadInfo->bLoadToTemplate = m_bIsTemplate;
		m_pLoadInfo->bIsNewTopic = bIsNewTopic;
		m_pLoadInfo->bLoadHiddenTopics = bLoadHiddenTopics;
		// (a.walling 2008-06-30 15:01) - PLID 29271 - Preview Pane Flags
		m_pLoadInfo->m_nPreviewFlags = 0;
		m_pLoadInfo->nPatientID = nPatientID;
		// (j.jones 2008-09-22 15:54) - PLID 31408 - added nEMRGroupID
		long nEMRGroupID = -1;
		if(pEmr) {
			nEMRGroupID = pEmr->GetID();
		}
		m_pLoadInfo->nEMRGroupID = nEMRGroupID;
		m_pLoadInfo->hWnd = hWnd;

		//DRT 9/25/2007 - PLID 27515
		m_pLoadInfo->m_nSourceActionSourceID = -1;
		m_pLoadInfo->m_nSourceActionSourceDataGroupID = -1;
		m_pLoadInfo->m_nSourceActionSourceHotSpotGroupID = -1;
		m_pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:49) - PLID 33070

		// (c.haag 2007-02-27 10:47) - PLID 24949 - Add our preloaded detail array to the
		// load info object. Later in the topic loading thread, it will be processed in favor
		// of using recordsets.
		m_pLoadInfo->m_pLoader = pLoader;
		if (pLoader) pLoader->AddRef();

		// (z.manning, 11/07/2006) - PLID 23359 - If we're loading hidden topics then we need to load its
		// SourceActionID from data because the one that's being passed through does not apply to hidden topics,
		// which are just topics that get spawned by other actions.
		long nTopicSourceActionID = AdoFldLong(pfldsTopicInfo, "SourceActionID", -1);
		if(bLoadHiddenTopics && nTopicSourceActionID != -1) {
			m_pLoadInfo->m_saiOverride.nSourceActionID = nTopicSourceActionID;
			m_pLoadInfo->m_saiOverride.SetDataGroupID(AdoFldLong(pfldsTopicInfo, "SourceDataGroupID", -1));
			m_pLoadInfo->m_saiOverride.eaoSourceType = (EmrActionObject)AdoFldLong(pfldsTopicInfo, "SourceType", eaoInvalid);
			//DRT 9/25/2007 - PLID 27515
			m_pLoadInfo->nOverrideSourceActionSourceID = AdoFldLong(pfldsTopicInfo, "SourceActionSourceID", -1);;
			m_pLoadInfo->nOverrideSourceActionSourceDataGroupID = AdoFldLong(pfldsTopicInfo, "SourceActionSourceDataGroupID", -1);
			m_pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID = AdoFldLong(pfldsTopicInfo, "EmrSpotGroupID", -1);
			// (z.manning 2009-02-13 10:50) - PLID 33070
			m_pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID = AdoFldLong(pfldsTopicInfo, "SourceActionSourceTableDropdownGroupID", -1);
		}
		else {
			m_pLoadInfo->m_saiOverride = sai;
			//DRT 9/25/2007 - PLID 27515
			m_pLoadInfo->nOverrideSourceActionSourceID = nSourceActionSourceID;
			m_pLoadInfo->nOverrideSourceActionSourceDataGroupID = nSourceActionSourceDataGroupID;
			m_pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID = nSourceActionSourceHotSpotGroupID;
			// (z.manning 2009-02-13 10:55) - PLID 33070
			m_pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID = nSourceActionSourceTableDropdownGroupID;
		}

		// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
		long nTopicSourceDetailID = AdoFldLong(pfldsTopicInfo, "SourceDetailID", -1);
		if(bLoadHiddenTopics && nTopicSourceDetailID != -1) {
			m_pLoadInfo->m_saiOverride.nSourceDetailID = nTopicSourceDetailID;
		}
		else {
			m_pLoadInfo->m_saiOverride.nSourceDetailID = sai.nSourceDetailID;
		}

		if(bLoadHiddenTopics)
			m_pLoadInfo->m_saiOverride.pSourceDetail = NULL;
		else
			m_pLoadInfo->m_saiOverride.pSourceDetail = sai.pSourceDetail;

		//TES 9/12/2006 - If we own our parent, we need to create it if it doesn't exist yet.
		CEMN *pParentEMN = GetParentEMN();
		if(!pParentEMN && m_bOwnParent) {
			long nParentTopicID = AdoFldLong(pfldsTopicInfo, "EmrParentTemplateTopicID",-1);
			if(nParentTopicID == -1) {
				m_pParentEMN = new CEMN(NULL);
				m_pParentEMN->LoadFromTopic(this);
			}
			else {
				//TES 4/15/2010 - PLID 24692 - If this is a template, there may already be an entry for this topic, otherwise, create a new one.
				//TES 5/11/2010 - PLID 24692 - For "bottom-up" loading, we don't maintain position entries.
				TopicPositionEntry *tpe = NULL;
				m_pParentTopic = new CEMRTopic(m_bIsTemplate, TRUE, tpe);
				m_pParentTopic->LoadFromSubTopic(this);
			}
		}

		CEMR *pParentEMR = pEmr ? pEmr : GetParentEMN()->GetParentEMR();
		pParentEMR->LoadTopic(lpCon, m_pLoadInfo);
		if(!hWnd) {
			//They didn't give us a window to post back to when we're finished, so CEMR::LoadTopic will have loaded us synchronously.
			//Make sure the load process is complete.
			if(GetParentEMN()) {
				//If we don't have a parent EMN, that means we're being loaded in a thread and haven't been attached to a parent yet.
				//We call PostLoad in the SetParent___() functions so as to handle that case.
				PostLoad();
			}
		}
	// (c.haag 2007-03-19 13:15) - PLID 24663 - Use thread-safe exception handling
	}NxCatchAllThread("Error in CEMRTopic::LoadFromTemplateTopicFields()");
}

// (c.haag 2007-08-08 11:13) - PLID 27014 - We can now specify whether to load subtopics
// (z.manning 2009-02-13 10:55) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
// (z.manning 2009-03-04 15:01) - PLID 33338 - Use the new source action info class
// (z.manning 2010-08-20 09:53) - PLID 40190 - Added connection pointer
void CEMRTopic::LoadFromPreloadedTemplateTopic(ADODB::_Connection *lpCon, CEMNLoader* pLoader, long nEmrTemplateTopicID, BOOL bIsNewTopic, HWND hWnd, SourceActionInfo &sai, 
											   BOOL bLoadHiddenTopics /*= FALSE*/, OPTIONAL IN long nPatientID /*= -1*/, OPTIONAL IN CEMR *pEmr /*= NULL*/,
											   OPTIONAL IN BOOL bLoadSubtopics /*= TRUE */, long nSourceActionSourceID /*= -1*/, long nSourceActionSourceDataGroupID /*= -1*/, OPTIONAL IN long nSourceActionSourceHotSpotGroupID /* = -1 */,
											   OPTIONAL IN long nSourceActionSourceTableDropdownGroupID /* = -1 */)
{
	//
	// (c.haag 2007-05-02 17:36) - PLID 25881 - Added LoadFromPreloadedTemplateTopic for loading from a preloaded template topic
	// This function is based off LoadFromTemplateTopicFields, but the fields pointer is replaced with the EMN loader object and
	// a template topic ID. There are no other differences between the two functions.
	//
	try {
		//Three possibilities: 1.) loading a template topic onto its original template
		//2.) loading a template topic onto a different template.
		//3.) loading a template topic onto an EMN, based on the same template.
		//4.) loading a template topic onto an EMN, based on a different template.
		if (NULL == pLoader) ThrowNxException("The loader object is invalid!");
		// (c.haag 2007-07-03 10:10) - PLID 26523 - Request access to topic information for
		// thread safety
		CHoldEMNLoaderMutex mh(pLoader->GetTopicsMutex());
		CEMNLoader::CPreloadedTemplateTopic* pTemplateTopic = pLoader->GetPreloadedTemplateTopicByID(nEmrTemplateTopicID);
		if (NULL == pTemplateTopic) ThrowNxException("The topic object is invalid!");

		if(m_bIsTemplate) {
			if(!bIsNewTopic) {
				//1.) loading a template topic onto its original template
				m_nID = nEmrTemplateTopicID;
				m_nTemplateTopicID = nEmrTemplateTopicID;
				//m_nOriginalTemplateTopicID will be pulled from data.
			}
			else {
				//2.) loading a template topic onto a different template.
				m_nID = -1;
				m_nTemplateTopicID = -1;
				m_nOriginalTemplateTopicID = nEmrTemplateTopicID;
				// (c.haag 2007-07-23 17:41) - PLID 26344 - This is assigned if this topic
				// was spawned onto its template by a list detail
				m_nOriginalTemplateID = VarLong(pTemplateTopic->m_vOriginalTemplateID, -1);
			}
		}
		else {
			if(!bIsNewTopic) {
				//3.) loading a template topic onto an EMN, based on the same template
				m_nID = -1;
				m_nTemplateTopicID = nEmrTemplateTopicID;
				//m_nOriginalTemplateTopicID will be pulled from data.
			}
			else {
				//4.) loading a template topic onto an EMN, based on a different template.
				m_nID = -1;
				m_nTemplateTopicID = nEmrTemplateTopicID;
				m_nOriginalTemplateTopicID = nEmrTemplateTopicID;
				// (c.haag 2007-07-23 17:41) - PLID 26344 - This is assigned if this topic
				// was spawned onto its template by a list detail
				m_nOriginalTemplateID = VarLong(pTemplateTopic->m_vOriginalTemplateID, -1);
			}
		}

		// (c.haag 2007-05-02 17:47) - PLID 25881 - Calculate the topic name. The query definition was:
		//
		//	CASE WHEN (%li <> -1 OR EmrTemplateTopicsT.SourceActionID <> -1) AND SourceEMRTemplateTopicsT.Name Is Not NULL
		//	THEN SourceEMRTemplateTopicsT.Name ELSE EmrTemplateTopicsT.Name END AS Name
		//
		//  where %li = nSourceActionID, which is an input parameter in both CEMN::LoadFromTemplateID
		//  and CEMRTopic::LoadFromTemplateTopicID. We will use it here as well.
		//
		//m_strTopicName = AdoFldString(pfldsTopicInfo, "Name");
		if ( (sai.nSourceActionID != -1 || VarLong(pTemplateTopic->m_vSourceActionID, -1) != -1) && (VT_NULL != pTemplateTopic->m_vOriginalTopicName.vt)) {
			// (c.haag 2007-07-23 17:41) - PLID 26344 - This is assigned if this topic
			// was spawned onto its template by a list detail
			m_strTopicName = VarString(pTemplateTopic->m_vOriginalTopicName, "");
		} else {
			m_strTopicName = VarString(pTemplateTopic->m_vName, "");
		}

		// (a.walling 2008-06-30 14:20) - PLID 29271 - Preview Pane flags
		m_nPreviewFlags = VarLong(pTemplateTopic->m_vPreviewFlags, 0);

		// (c.haag 2007-07-20 17:05) - PLID 25881 - m_bHasDetails cannot be true unless there is at least one
		// child topic with a NULL action ID or a matching source topic ID. Here is the SQL equivalent:
		//
		// CASE WHEN EXISTS (SELECT ID FROM EmrTemplateTopicsT ChildTopic WHERE ChildTopic.SourceActionID Is Null AND ChildTopic.EmrParentTemplateTopicID = EmrTemplateTopicsT.ID OR 
		// ChildTopic.EmrParentTemplateTopicID = EmrTemplateTopicsT.SourceTemplateTopicID) THEN 1 ELSE 0 END AS HasSubTopics
		//
		//m_bHasSubTopics = (pTemplateTopic->m_anSubTemplateTopicIDs.GetSize() > 0) ? TRUE : FALSE;

		// (c.haag 2007-07-24 17:41) - PLID 25881 - If there is a valid source action ID, we must rely on the
		// source topic to determine whether there are any children.
		CEMNLoader::CPreloadedTemplateTopic* pTopicOfInterest;
		if ( VarLong(pTemplateTopic->m_vSourceActionID, -1) != -1 ) {
			if (NULL == (pTopicOfInterest = pLoader->GetPreloadedTemplateTopicByID(VarLong(pTemplateTopic->m_vSourceTemplateTopicID,-1)))) {
				ThrowNxException("Could not calculate whether the topic %s has children!", m_strTopicName);
			}
		} else {
			pTopicOfInterest = pTemplateTopic;
		}

		m_bHasSubTopics = FALSE;
		const int nChildren = pTopicOfInterest->m_anSubTemplateTopicIDs.GetSize();
		for (int nChild = 0; nChild < nChildren && !m_bHasSubTopics; nChild++) {
			const long nChildTopicID = pTopicOfInterest->m_anSubTemplateTopicIDs[nChild];
			CEMNLoader::CPreloadedTemplateTopic* pChildTopic = pLoader->GetPreloadedTemplateTopicByID(nChildTopicID);
			if (NULL != pChildTopic) {
				if (VT_NULL == pChildTopic->m_vSourceActionID.vt ||
					(
						// (c.haag 2007-08-28 16:01) - PLID 25881: When SQL server compares two NULL's, the result is
						// always false. So, this conditional can only be true if we have valid values in both fields.
						pChildTopic->m_vSourceTemplateTopicID.vt != VT_NULL &&
						pTemplateTopic->m_vSourceTemplateTopicID.vt != VT_NULL &&
						VarLong(pChildTopic->m_vSourceTemplateTopicID) == VarLong(pTemplateTopic->m_vSourceTemplateTopicID)
					)
				) 
				{
					m_bHasSubTopics = TRUE;
				}
			}
		}

		m_bHasDetails = pTemplateTopic->m_bHasDetails;
		m_bShowIfEmpty = VarBool(pTemplateTopic->m_vShowIfEmpty);
		
		m_pLoadInfo = new EMRTopicLoadInfo;
		m_pLoadInfo->nID = nEmrTemplateTopicID;
		m_pLoadInfo->m_nTemplateID = pTemplateTopic->m_nTemplateID; //AdoFldLong(pfldsTopicInfo, "TemplateID", -1); // (a.walling 2007-03-21 17:24) - PLID 25301 - Set the template ID
		m_pLoadInfo->bLoadFromTemplate = TRUE;
		m_pLoadInfo->bLoadToTemplate = m_bIsTemplate;
		m_pLoadInfo->bIsNewTopic = bIsNewTopic;
		m_pLoadInfo->bLoadHiddenTopics = bLoadHiddenTopics;
		// (c.haag 2007-08-08 11:13) - PLID 27014 - Include bLoadSubtopics
		m_pLoadInfo->bLoadSubTopics = bLoadSubtopics;
		// (a.walling 2008-06-30 15:01) - PLID 29271 - Preview Pane Flags
		m_pLoadInfo->m_nPreviewFlags = 0;
		m_pLoadInfo->nPatientID = nPatientID;
		// (j.jones 2008-09-22 15:54) - PLID 31408 - added nEMRGroupID
		long nEMRGroupID = -1;
		if(pEmr) {
			nEMRGroupID = pEmr->GetID();
		}
		// (a.walling 2009-01-08 17:56) - PLID 32666 - We were not setting the loading info's EMR group id
		m_pLoadInfo->nEMRGroupID = nEMRGroupID;
		m_pLoadInfo->hWnd = hWnd;

		//DRT 9/25/2007 - PLID 27515
		m_pLoadInfo->m_nSourceActionSourceID = -1;
		m_pLoadInfo->m_nSourceActionSourceDataGroupID = -1;
		m_pLoadInfo->m_nSourceActionSourceHotSpotGroupID = -1;
		m_pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 10:56) - PLID 33070

		// (c.haag 2007-02-27 10:47) - PLID 24949 - Add our preloaded detail array to the
		// load info object. Later in the topic loading thread, it will be processed in favor
		// of using recordsets.
		m_pLoadInfo->m_pLoader = pLoader;
		if (pLoader) pLoader->AddRef();

		// (z.manning, 11/07/2006) - PLID 23359 - If we're loading hidden topics then we need to load its
		// SourceActionID from data because the one that's being passed through does not apply to hidden topics,
		// which are just topics that get spawned by other actions.
		long nTopicSourceActionID = VarLong(pTemplateTopic->m_vSourceActionID, -1); //AdoFldLong(pfldsTopicInfo, "SourceActionID", -1);
		if(bLoadHiddenTopics && nTopicSourceActionID != -1) {
			m_pLoadInfo->m_saiOverride.nSourceActionID = nTopicSourceActionID;
			m_pLoadInfo->m_saiOverride.eaoSourceType = (EmrActionObject)VarLong(pTemplateTopic->m_vSourceActionType, eaoInvalid);
			m_pLoadInfo->m_saiOverride.SetDataGroupID(VarLong(pTemplateTopic->m_vSourceDataGroupID, -1));
			//DRT 9/26/2007 - PLID 27515
			m_pLoadInfo->nOverrideSourceActionSourceID = VarLong(pTemplateTopic->m_vSourceActionSourceID, -1);
			m_pLoadInfo->nOverrideSourceActionSourceDataGroupID = VarLong(pTemplateTopic->m_vSourceActionSourceDataGroupID, -1);
			m_pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID = VarLong(pTemplateTopic->m_vSourceActionSourceHotSpotGroupID, -1);
			// (z.manning 2009-02-13 10:57) - PLID 33070
			m_pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID = VarLong(pTemplateTopic->m_vSourceActionSourceTableDropdownGroupID, -1);
		}
		else {
			m_pLoadInfo->m_saiOverride = sai;
			//DRT 9/26/2007 - PLID 27515
			m_pLoadInfo->nOverrideSourceActionSourceID = nSourceActionSourceID;
			m_pLoadInfo->nOverrideSourceActionSourceDataGroupID = nSourceActionSourceDataGroupID;
			m_pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID = nSourceActionSourceHotSpotGroupID;
			// (z.manning 2009-02-13 10:58) - PLID 33070
			m_pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID = nSourceActionSourceTableDropdownGroupID;
		}

		// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
		long nTopicSourceDetailID = VarLong(pTemplateTopic->m_vSourceDetailID, -1); //AdoFldLong(pfldsTopicInfo, "SourceDetailID", -1);
		if(bLoadHiddenTopics && nTopicSourceDetailID != -1) {
			m_pLoadInfo->m_saiOverride.nSourceDetailID = nTopicSourceDetailID;
		}
		else {
			m_pLoadInfo->m_saiOverride.nSourceDetailID = sai.nSourceDetailID;
		}

		if(bLoadHiddenTopics)
			m_pLoadInfo->m_saiOverride.pSourceDetail = NULL;
		else
			m_pLoadInfo->m_saiOverride.pSourceDetail = sai.pSourceDetail;

		//TES 9/12/2006 - If we own our parent, we need to create it if it doesn't exist yet.
		CEMN *pParentEMN = GetParentEMN();
		if(!pParentEMN && m_bOwnParent) {
			long nParentTopicID = VarLong(pTemplateTopic->m_vEMRParentTemplateTopicID, -1); //AdoFldLong(pfldsTopicInfo, "EmrParentTemplateTopicID",-1);
			// (c.haag 2007-07-03 10:10) - PLID 26523 - We're all done with the topic object
			mh.Release();
			if(nParentTopicID == -1) {
				m_pParentEMN = new CEMN(NULL);
				m_pParentEMN->LoadFromTopic(this);
			}
			else {
				//TES 4/15/2010 - PLID 24692 - If this is a template, there may already be an entry for this topic, otherwise, create a new one.
				//TES 5/3/2010 - PLID 24692 - For "bottom-up" loading, we don't maintain position entries.
				TopicPositionEntry *tpe = NULL;
				m_pParentTopic = new CEMRTopic(m_bIsTemplate, TRUE, tpe);
				m_pParentTopic->LoadFromSubTopic(this);
			}
		} else {
			// (c.haag 2007-07-03 10:10) - PLID 26523 - We're all done with the topic object
			mh.Release();
		}

		CEMR *pParentEMR = pEmr ? pEmr : GetParentEMN()->GetParentEMR();
		pParentEMR->LoadTopic(lpCon, m_pLoadInfo);
		if(!hWnd) {
			//They didn't give us a window to post back to when we're finished, so CEMR::LoadTopic will have loaded us synchronously.
			//Make sure the load process is complete.
			if(GetParentEMN()) {
				//If we don't have a parent EMN, that means we're being loaded in a thread and haven't been attached to a parent yet.
				//We call PostLoad in the SetParent___() functions so as to handle that case.
				PostLoad();
			}
		}
	// (c.haag 2007-03-19 13:15) - PLID 24663 - Use thread-safe exception handling
	}NxCatchAllThread("Error in CEMRTopic::LoadFromPreloadedTemplateTopic()");
}

/*void CEMRTopic::ShowTopicDetails()
{
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		((CEMNDetail*)m_arypEMNDetails.GetAt(i))->ShowDetailDlg();
	}
}

void CEMRTopic::HideTopicDetails()
{
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		((CEMNDetail*)m_arypEMNDetails.GetAt(i))->HideDetailDlg();
	}
}*/

// (c.haag 2007-05-30 10:55) - PLID 26175 - This macro has been here for a long time,
// but I made it call ForceLoadToFinish() now.
#define ENSURE_VARIABLE(strPrefix, strVariable)	ForceLoadToFinish();

CEMNDetail* CEMRTopic::GetDetailByID(long nID) {
	
	for(int i=0;i<m_arypEMNDetails.GetSize();i++) {
		if(m_arypEMNDetails.GetAt(i)->m_nEMRDetailID == nID)
			return m_arypEMNDetails.GetAt(i);
	}

	//if we're still here, recurse through the sub-topics
	EnsureTopics();	// (c.haag 2007-01-08 11:41) - PLID 23799 - Make sure subtopics exist first!
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		CEMNDetail* pCheck = m_arypEMRTopics.GetAt(i)->GetDetailByID(nID);
		if(pCheck) {
			//found it, return
			return pCheck;
		}
	}

	if(EnsureDetails()) {
		//Maybe that detail hadn't been loaded until now.
		return GetDetailByID(nID);
	}

	//nothing, return NULL
	return NULL;
}


CEMNDetail* CEMRTopic::GetDetailByTemplateDetailID(long nID)
{
	for(int i=0;i<m_arypEMNDetails.GetSize();i++) {
		if(m_arypEMNDetails.GetAt(i)->m_nEMRTemplateDetailID == nID)
			return m_arypEMNDetails.GetAt(i);
	}

	//if we're still here, recurse through the sub-topics
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		CEMNDetail* pCheck = m_arypEMRTopics.GetAt(i)->GetDetailByTemplateDetailID(nID);
		if(pCheck) {
			//found it, return
			return pCheck;
		}
	}

	if(EnsureDetails()) {
		//Maybe that detail hadn't been loaded yet.
		return GetDetailByTemplateDetailID(nID);
	}

	//nothing, return NULL
	return NULL;
}

// (z.manning 2011-01-25 15:58) - PLID 42336
void CEMRTopic::GetParentDetailsByDetailID(const long nChildDetailID, OUT CEMNDetailArray *paryParentDetails)
{
	EnsureDetails();
	for(int nDetailIndex = 0; nDetailIndex < m_arypEMNDetails.GetSize(); nDetailIndex++) {
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(nDetailIndex);
		if(pDetail->m_nChildEMRDetailID == nChildDetailID) {
			paryParentDetails->Add(pDetail);
		}
	}

	//if we're still here, recurse through the sub-topics
	EnsureTopics();	// (c.haag 2007-01-08 11:41) - PLID 23799 - Make sure subtopics exist first!
	for(int nTopicIndex = 0; nTopicIndex < m_arypEMRTopics.GetSize(); nTopicIndex++) {
		m_arypEMRTopics.GetAt(nTopicIndex)->GetParentDetailsByDetailID(nChildDetailID, paryParentDetails);
	}
}

// (z.manning 2011-01-25 15:58) - PLID 42336
void CEMRTopic::GetParentDetailsByTemplateDetailID(const long nChildTemplateDetailID, OUT CEMNDetailArray *paryParentDetails)
{
	EnsureDetails();
	for(int nDetailIndex = 0; nDetailIndex < m_arypEMNDetails.GetSize(); nDetailIndex++) {
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(nDetailIndex);
		if(pDetail->m_nChildEMRTemplateDetailID == nChildTemplateDetailID) {
			paryParentDetails->Add(pDetail);
		}
	}

	//if we're still here, recurse through the sub-topics
	EnsureTopics();	// (c.haag 2007-01-08 11:41) - PLID 23799 - Make sure subtopics exist first!
	for(int nTopicIndex = 0; nTopicIndex < m_arypEMRTopics.GetSize(); nTopicIndex++) {
		m_arypEMRTopics.GetAt(nTopicIndex)->GetParentDetailsByTemplateDetailID(nChildTemplateDetailID, paryParentDetails);
	}
}

// (z.manning 2011-01-28 12:26) - PLID 42336 - This function will look for any smart stamp table with the given
// table info master ID as long as it is not already linked to an image with the given image info master ID.
CEMNDetail* CEMRTopic::GetSmartStampTableDetailByInfoMasterID(const long nTableInfoMasterID, const long nImageInfoMasterID)
{
	EnsureDetails();
	for(int nDetailIndex = 0; nDetailIndex < m_arypEMNDetails.GetSize(); nDetailIndex++) {
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(nDetailIndex);
		if(pDetail->m_nEMRInfoMasterID == nTableInfoMasterID) {
			if(pDetail->GetSmartStampImageDetails()->GetDetailFromEmrInfoMasterID(nImageInfoMasterID) == NULL) {
				return pDetail;
			}
		}
	}

	//if we're still here, recurse through the sub-topics
	EnsureTopics();
	for(int nTopicIndex = 0; nTopicIndex < m_arypEMRTopics.GetSize(); nTopicIndex++) {
		CEMNDetail *pDetail = m_arypEMRTopics.GetAt(nTopicIndex)->GetSmartStampTableDetailByInfoMasterID(nTableInfoMasterID, nImageInfoMasterID);
		if(pDetail != NULL) {
			return pDetail;
		}
	}

	return NULL;
}

// (j.jones 2010-02-12 14:19) - PLID 37318 - added GetSmartStampImageDetail_ByTemplateDetailID which is
// a clone of GetDetailByTemplateDetailID, but only returns a detail where m_pSmartStampTableDetail is NULL
CEMNDetail* CEMRTopic::GetSmartStampImageDetail_ByTemplateDetailID(long nID)
{
	for(int i=0;i<m_arypEMNDetails.GetSize();i++)
	{
		// (z.manning 2010-07-21 09:29) - PLID 39591 - Only return an image if m_bSmartStampsEnabled is true
		if(m_arypEMNDetails.GetAt(i)->m_nEMRTemplateDetailID == nID
			&& m_arypEMNDetails.GetAt(i)->GetSmartStampTableDetail() == NULL
			&& m_arypEMNDetails.GetAt(i)->m_bSmartStampsEnabled)
		{
			return m_arypEMNDetails.GetAt(i);
		}
	}

	//if we're still here, recurse through the sub-topics
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		CEMNDetail* pCheck = m_arypEMRTopics.GetAt(i)->GetSmartStampImageDetail_ByTemplateDetailID(nID);
		if(pCheck) {
			//found it, return
			return pCheck;
		}
	}

	if(EnsureDetails()) {
		//Maybe that detail hadn't been loaded yet.
		return GetSmartStampImageDetail_ByTemplateDetailID(nID);
	}

	//nothing, return NULL
	return NULL;
}

//this delves deeper into the topic and recursively finds all details in this topic or any subtopics
long CEMRTopic::GetSubTopicDetailCount() const
{
	/*EnsureDetails();
	EnsureTopics();*/

	long nTotal = m_arypEMNDetails.GetSize();

	for(int i=0;i<m_arypEMRTopics.GetSize();i++) {
		nTotal += m_arypEMRTopics.GetAt(i)->GetSubTopicDetailCount();
	}

	return nTotal;
}

//the "global index" is a calculated placement in the GetSubTopicDetailCount() logic across all topics for an EMN
CEMNDetail* CEMRTopic::GetDetailByGlobalIndex(long nIndexToFind, long &nCurCount) 
{
	/*EnsureDetails();
	EnsureTopics();*/

	for(int i=0;i<m_arypEMNDetails.GetSize();i++) {
		if(nCurCount == nIndexToFind) {		
			return m_arypEMNDetails.GetAt(i);
		}
		nCurCount++;
	}

	//if we're still here, recurse through the sub-topics
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		CEMNDetail* pCheck = m_arypEMRTopics.GetAt(i)->GetDetailByGlobalIndex(nIndexToFind,nCurCount);
		if(pCheck) {
			//found it, return
			return pCheck;
		}
	}

	return NULL;
}

// (a.walling 2009-11-17 08:38) - PLID 36366 - Ignore details in pIgnoreTopic (they have probably already been added)
void CEMRTopic::GenerateEMNDetailArray(CArray<CEMNDetail*,CEMNDetail*> *arypEMNDetails, CEMRTopic* pIgnoreTopic)
{
	//used to generate a complete array of details from the EMN as a whole
	EnsureDetails();
	EnsureTopics();

	for(int i=0;i<m_arypEMNDetails.GetSize();i++) {
		arypEMNDetails->Add(m_arypEMNDetails.GetAt(i));
	}

	//recurse through the sub-topics
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		CEMRTopic* pTopic = m_arypEMRTopics.GetAt(i);
		if (pTopic != pIgnoreTopic) {
			pTopic->GenerateEMNDetailArray(arypEMNDetails);
		}
	}
}

void CEMRTopic::GenerateEMNDeletedDetailArray(CArray<CEMNDetail*,CEMNDetail*> *arypEMNDeletedDetails)
{
	//used to generate a complete array of deleted details from the EMN as a whole

	for(int i=0;i<m_aryDeletedDetails.GetSize();i++) {
		arypEMNDeletedDetails->Add(m_aryDeletedDetails.GetAt(i));
	}

	//recurse through the sub-topics
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->GenerateEMNDeletedDetailArray(arypEMNDeletedDetails);
	}

	for(i=0;i<m_aryDeletedTopics.GetSize();i++) {
		m_aryDeletedTopics.GetAt(i)->GenerateEMNDetailArray(arypEMNDeletedDetails);
		m_aryDeletedTopics.GetAt(i)->GenerateEMNDeletedDetailArray(arypEMNDeletedDetails);
	}
}

//used when an item is changed from the administrator,
//for all instances of that item in the topic and subtopics
void CEMRTopic::MarkItemChanged(long nEMRInfoID)
{
	EnsureDetails();
	EnsureTopics();
	//first our details
	for(int i=0; i<m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pInfo = m_arypEMNDetails.GetAt(i);
		if(pInfo->m_nEMRInfoID == nEMRInfoID)
			pInfo->SetNeedContentReload();
	}

	//now recurse through all subtopics
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->MarkItemChanged(nEMRInfoID);
	}
}

//refreshes all details in the topic and subtopics
// (j.jones 2007-07-26 09:23) - PLID 24686 - this is a horrible idea that should never occur
/*
void CEMRTopic::RefreshAllItems()
{
	CWaitCursor wc;

	// (j.jones 2004-12-08 11:15) -  we need to reload all items silently in the background
	//we cannot just trigger the "need content refresh" and then just assume the user will
	//view those tabs prior to saving.

	//first go through this topic's details, if there are any
	if(m_arypEMNDetails.GetSize() > 0) {

		//mark the items as needing refreshed
		for(int i=0; i<m_arypEMNDetails.GetSize(); i++) {
			m_arypEMNDetails.GetAt(i)->SetNeedContentReload();
		}
	}

	//now recurse through all subtopics
	for(int i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->RefreshAllItems();
	}
}
*/

// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RefreshContent into two functions,
// accepting an InfoID or a MasterID
// (c.haag 2008-06-12 22:16) - PLID 27831 - Added papMasterDetails so that the caller may
// optionally get a list of all the details with a matching master ID
void CEMRTopic::RefreshContentByInfoMasterID(long nEMRInfoMasterID, BOOL bSyncContentAndState /*= FALSE*/,
											 CArray<CEMNDetail*,CEMNDetail*>* papMasterDetails /*= NULL */)
{
	// (c.haag 2006-03-31 10:13) - All this function does is set a flag that we
	// need to reload content. For example, if you call RefreshContent on an invisible
	// topic, it will know to call LoadContent for its details when it is made visible.
	// If the topic is never made visible, there's no point in loading content anyway
	// because LoadContent just synchronizes a detail with pracdata.

	// (c.haag 2004-07-08 13:30) - PLID 13373 - We now force the caller to decide
	// if bSetRegionAndInvalidate is set in RepositionDetailsInTab(). The reason is
	// that setting it to true will reset the positions of some controls. In most
	// cases we never want to do this. In 7600, we will revisit this and see the
	// best way to handle content positioning and sizing.

	// (c.haag 2004-05-28 15:54) - PLID 12289 - The new way of doing this is to
	// set a dirty content flag for all the EMNDetail items, and have 
	// RepositionDetailsInTab examine the flag and deal with it accordingly.

	//update the m_bNeedContentRefresh flag for each detail on this topic level
	for(int i=0; i<m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pInfo = (CEMNDetail*)m_arypEMNDetails.GetAt(i);
		if (pInfo->m_nEMRInfoMasterID == nEMRInfoMasterID) {
			pInfo->SetNeedContentReload();

			// (c.haag 2006-03-31 11:23) - PLID 19387 - If bSyncContentAndState
			// is true, it means that CEmrItemAdvDlg::OpenItemEntryDlg called
			// RefreshContent, which implies the user edited the item. So, because
			// the user edited the item, we HAVE to validate the state so the user
			// can't save bad state data in lieu of changes made to the content.
			//
			// This will be removed pending a better, more permanent solution that
			// involves LoadContent itself checking if SyncContentAndState needs
			// to be called.
			//
			if (bSyncContentAndState) {
				pInfo->SetNeedSyncContentAndState();
			}

			// (c.haag 2008-06-12 21:29) - PLID 27831 - Add this detail to papMasterDetails.
			// If it's not NULL, it means a caller in the stack needs to know what details share
			// the master ID to do further updates to EMR.
			if (NULL != papMasterDetails) {
				papMasterDetails->Add(pInfo);
			}
		}
	}

	//recurse through each subtopic and call its RefreshContentByInfoMasterID function
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->RefreshContentByInfoMasterID(nEMRInfoMasterID, bSyncContentAndState,
			papMasterDetails);
	}
}

// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RefreshContent into two functions,
// accepting an InfoID or a MasterID
void CEMRTopic::RefreshContentByInfoID(long nEMRInfoID, BOOL bSyncContentAndState /*= FALSE*/)
{
	// (c.haag 2006-03-31 10:13) - All this function does is set a flag that we
	// need to reload content. For example, if you call RefreshContent on an invisible
	// topic, it will know to call LoadContent for its details when it is made visible.
	// If the topic is never made visible, there's no point in loading content anyway
	// because LoadContent just synchronizes a detail with pracdata.

	// (c.haag 2004-07-08 13:30) - PLID 13373 - We now force the caller to decide
	// if bSetRegionAndInvalidate is set in RepositionDetailsInTab(). The reason is
	// that setting it to true will reset the positions of some controls. In most
	// cases we never want to do this. In 7600, we will revisit this and see the
	// best way to handle content positioning and sizing.

	// (c.haag 2004-05-28 15:54) - PLID 12289 - The new way of doing this is to
	// set a dirty content flag for all the EMNDetail items, and have 
	// RepositionDetailsInTab examine the flag and deal with it accordingly.

	//update the m_bNeedContentRefresh flag for each detail on this topic level
	for(int i=0; i<m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pInfo = (CEMNDetail*)m_arypEMNDetails.GetAt(i);
		if (pInfo->m_nEMRInfoID == nEMRInfoID) {
			pInfo->SetNeedContentReload();

			// (c.haag 2006-03-31 11:23) - PLID 19387 - If bSyncContentAndState
			// is true, it means that CEmrItemAdvDlg::OpenItemEntryDlg called
			// RefreshContent, which implies the user edited the item. So, because
			// the user edited the item, we HAVE to validate the state so the user
			// can't save bad state data in lieu of changes made to the content.
			//
			// This will be removed pending a better, more permanent solution that
			// involves LoadContent itself checking if SyncContentAndState needs
			// to be called.
			//
			if (bSyncContentAndState) {
				pInfo->SetNeedSyncContentAndState();
			}
		}
	}

	//recurse through each subtopic and call its RefreshContentByInfoID function
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->RefreshContentByInfoID(nEMRInfoID);
	}
}

// (a.wetta 2007-04-09 13:30) - PLID 25532 - This function refreshes the content
// all all EMR items of a certain type.  This was copied from the original RefreshContent()
// (a.walling 2008-12-19 09:21) - PLID 29800 - This was only used for images, and only to refresh the custom stamps, which was causing the content
// to be reloaded. This is all unnecessary, and the custom stamps is entirely UI. So let's just do what we need to do, and refresh the custom stamps,
// rather than flag as needed to reload content. This is all controlled by the new bRefreshCustomStampsOnly param. I could have renamed the function
// entirely, but I can see how this might come in handy in the future.
void CEMRTopic::RefreshContentByType(EmrInfoType eitItemType, BOOL bSyncContentAndState, BOOL bRefreshCustomStampsOnly)
{
	// (c.haag 2006-03-31 10:13) - All this function does is set a flag that we
	// need to reload content. For example, if you call RefreshContent on an invisible
	// topic, it will know to call LoadContent for its details when it is made visible.
	// If the topic is never made visible, there's no point in loading content anyway
	// because LoadContent just synchronizes a detail with pracdata.

	// (c.haag 2004-07-08 13:30) - PLID 13373 - We now force the caller to decide
	// if bSetRegionAndInvalidate is set in RepositionDetailsInTab(). The reason is
	// that setting it to true will reset the positions of some controls. In most
	// cases we never want to do this. In 7600, we will revisit this and see the
	// best way to handle content positioning and sizing.

	// (c.haag 2004-05-28 15:54) - PLID 12289 - The new way of doing this is to
	// set a dirty content flag for all the EMNDetail items, and have 
	// RepositionDetailsInTab examine the flag and deal with it accordingly.

	//update the m_bNeedContentRefresh flag for each detail on this topic level
	for(int i=0; i<m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pInfo = (CEMNDetail*)m_arypEMNDetails.GetAt(i);
		if (pInfo->m_EMRInfoType == eitItemType) {
			if (bRefreshCustomStampsOnly) {
				_ASSERTE(pInfo->m_EMRInfoType == eitImage);

				if (pInfo->m_EMRInfoType == eitImage) {
					CEmrItemAdvDlg* pDlg = pInfo->m_pEmrItemAdvDlg;

					// (a.walling 2008-12-19 09:25) - PLID 29800 - if they don't have one, it will be refreshed when the content is loaded anyway.
					if (pDlg) {
						if (pDlg->IsKindOf(RUNTIME_CLASS(CEmrItemAdvImageDlg))) {
							CEmrItemAdvImageDlg* pImageDlg = (CEmrItemAdvImageDlg*)pDlg;

							// (z.manning 2011-10-25 15:01) - PLID 39401 - We are refreshing custom stamps so we need to
							// reload any stamp exclusions.
							pInfo->ReloadStampExclusions();

							// (j.jones 2010-02-10 11:11) - PLID 37224 - stamps are now tracked globally, just call the
							// EmrUtils function to generate the stamp info that is readable by the ink picture			
							// (z.manning 2011-10-25 13:04) - PLID 39401 - Pass in the detail
							CString strStamps = GenerateInkPictureStampInfo(pInfo);
							pImageDlg->SetCustomStamps(strStamps);
						} else {
							_ASSERTE(pDlg->IsKindOf(RUNTIME_CLASS(CEmrItemAdvImageDlg)));
						}
					}
				}
			} else {
				pInfo->SetNeedContentReload();

				// (c.haag 2006-03-31 11:23) - PLID 19387 - If bSyncContentAndState
				// is true, it means that CEmrItemAdvDlg::OpenItemEntryDlg called
				// RefreshContent, which implies the user edited the item. So, because
				// the user edited the item, we HAVE to validate the state so the user
				// can't save bad state data in lieu of changes made to the content.
				//
				// This will be removed pending a better, more permanent solution that
				// involves LoadContent itself checking if SyncContentAndState needs
				// to be called.
				//
				if (bSyncContentAndState) {
					pInfo->SetNeedSyncContentAndState();
				}
			}
		}
	}

	//recurse through each subtopic and call its RefreshContent function
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->RefreshContentByType(eitItemType, bSyncContentAndState, bRefreshCustomStampsOnly);
	}
}

//if editing a locked item creates a new copy, and we have unsaved items
//using the old info item, make them use the new info item
void CEMRTopic::UpdateInfoID(long nOldEMRInfoID, long nNewEMRInfoID, EMRInfoChangedIDMap* pChangedIDMap)
{
	//go through each detail and update its Info ID,
	//only update if the item is unsaved, OR if the item is on a template
	for(int i=0; i<m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pInfo = (CEMNDetail*)m_arypEMNDetails.GetAt(i);
		if((m_bIsTemplate || pInfo->m_nEMRDetailID == -1)
			&& pInfo->m_nEMRInfoID == nOldEMRInfoID) {

			pInfo->UpdateInfoID(nOldEMRInfoID, nNewEMRInfoID, pChangedIDMap);
		}
	}

	//recurse through each subtopic and call its UpdateInfoID function
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->UpdateInfoID(nOldEMRInfoID, nNewEMRInfoID, pChangedIDMap);
	}
}

void CEMRTopic::FindDetailsByMergeFieldName(const CString& strMergeFieldName, CArray<CEMNDetail*,CEMNDetail*>& arypEMNDetail)
{
	//TES 5/26/2006 - This is only called to find duplicates; if the duplicate hasn't been loaded yet, then this will be called
	//again once it is loaded.  So we don't need to Ensure anything.
	/*EnsureDetails();
	EnsureTopics();*/

	//first look at this topic's details
	for(int i=0; i<m_arypEMNDetails.GetSize(); i++) {

		CEMNDetail *pEMNDetail = m_arypEMNDetails.GetAt(i);
		if (strMergeFieldName == pEMNDetail->GetMergeFieldName(TRUE)) {
			//TES 10/28/2004 - If we're on a template, don't include items that are not visible.
			if(m_bIsTemplate || pEMNDetail->GetVisible()) {
				arypEMNDetail.Add(pEMNDetail);
			}
		}
	}

	//now recurse through subtopics
	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->FindDetailsByMergeFieldName(strMergeFieldName,arypEMNDetail);
	}
}

//invalidates merge buttons across the entire Topic, including subtopics
void CEMRTopic::InvalidateAllDetailMergeButtons()
{
	//first deal with this topic's details

	if(m_arypEMNDetails.GetSize() > 0) {

		// Because the position and visibilty of the merge icon button will affect
		// the arrangement of controls, and size of the detail dialog, we flag all
		// of them as needed repositioning. Also notice how we don't do anything that
		// will result in prompting the user that a detail has been modified, like
		// we do with refreshing content. That's by intent.
		for (long i=0; i < m_arypEMNDetails.GetSize(); i++)
		{
			CEMNDetail *pEMNDetail = m_arypEMNDetails.GetAt(i);
			if (pEMNDetail) {
				// (c.haag 2004-07-06 12:02) - This should not be set to TRUE because it
				// will reposition details inside tabs at seemingly random.
				//pEMNDetail->m_bNeedReposition = TRUE;
				pEMNDetail->m_bNeedRefreshMergeButton = TRUE;

			}
		}
	}

	//now recurse through all subtopics
	for(int i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->InvalidateAllDetailMergeButtons();
	}
}

// (j.jones 2007-06-14 15:02) - PLID 26276 - added bCompleteIfNoDetails boolean, which means that
// when bCompleteIfNoDetails is TRUE, no details on the topic means the topic is complete
// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Ability to recalculate completion status, not just 
// return existing or calculated status
EmrTopicCompletionStatus CEMRTopic::GetCompletionStatus(CEMNDetail *pDetailToIgnore /*= NULL*/, BOOL bForceRecalculate /*= FALSE*/)
{
	//if there are reconstructed details, return this status immediately
	if(HasReconstructedDetails()) {
		// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Save the new status if requested before returning
		if (bForceRecalculate) {
			m_CompletionStatus = etcsReconstructed;
		}
		return etcsReconstructed;
	}

	// (b.cardillo 2012-03-06 14:56) - PLID 48647 - If we're being asked to recalculate, then by 
	// definition we never want to use the existing value.
	if(!bForceRecalculate && ((!pDetailToIgnore && m_CompletionStatus != etcsInvalid) || m_CompletionStatus == etcsBlank)) {
		//If we weren't given one to ignore, our cached value is good.  And if our cached value is empty, then even if we're 
		//ignoring one, it wouldn't change anything.
		return m_CompletionStatus;
	}
	else {
		//We have to re-calculate to make sure we're ignoring pDetailToIgnore.
		//find out if the topic's details are all empty, all set, or partially set
		int nEmptyCount = 0;
		int nNonEmptyCount = 0;
		
		BOOL bNoDetails = TRUE;

		for (long i=0; i<m_arypEMNDetails.GetSize(); i++) {

			bNoDetails = FALSE;

			CEMNDetail *pEMNDetail = m_arypEMNDetails.GetAt(i);
			if(pEMNDetail->GetVisible() && pEMNDetail != pDetailToIgnore) {
				if (!pEMNDetail->IsStateSet()) {
					// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
					if (pEMNDetail->IsRequired()) {
						// If we have even one required detail that isn't filled, then the whole topic is in that state
						if (bForceRecalculate) {
							m_CompletionStatus = etcsRequired;
						}
						return etcsRequired;
					}
					// The current state is empty
					nEmptyCount++;
				} else {
					// The current state is non-empty
					nNonEmptyCount++;
				}
			}
		}

		EmrTopicCompletionStatus ans;
		if(bNoDetails) {
			ans = etcsBlank;
		}
		else if(nNonEmptyCount && !nEmptyCount) {
			ans = etcsComplete;
		}
		else if(nNonEmptyCount) {
			ans = etcsPartiallyComplete;
		}
		else {
			ans = etcsIncomplete;
		}
		
		// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Save the new status if requested before returning
		if (bForceRecalculate) {
			m_CompletionStatus = ans;
		}
		return ans;
	}
}

// (z.manning, 02/07/2007) - PLID 24599 - This is the orignal overload of GenerateSaveString for those places
// that wish to save a topic without worrying about having multiple topics deleted at once.
// (c.haag 2007-06-20 12:41) - PLID 26397 - We now store saved objects in a map for fast lookups.
// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
// such as new or deleted prescriptions, or new or deleted diagnosis codes
// (a.walling 2014-01-30 00:00) - PLID 60542 - Quantize
Nx::Quantum::Batch CEMRTopic::GenerateSaveString(long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynAffectedEMRDetailIDs, OUT BOOL &bDrugInteractionsChanged, BOOL bIsTopLevelSave, BOOL bSaveRecordOnly)
{
	Nx::Quantum::Batch strSaveString;

	CArray<long,long> arynDeletedTemplateTopicIDs;

	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	strSaveString += GenerateSaveString(nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedEMRDetailIDs, bDrugInteractionsChanged, arynDeletedTemplateTopicIDs, bIsTopLevelSave, bSaveRecordOnly);

	// (z.manning, 02/07/2007) - PLID 24599 - Since we are tracking the array of deleted template topic IDs
	// locally, we are responsible for deleting them here as well.
	// (a.walling 2014-01-30 00:00) - PLID 60542 - Quantize
	foreach (long id, arynDeletedTemplateTopicIDs) {
		AddStatementToSqlBatch(strSaveString, "DELETE FROM EmrTemplateTopicsT WHERE ID = %li", id);
	}

	return strSaveString;
}

//DRT 2/24/2006 - PLID 19465 - This function is ONLY for use in generating the string to save.  This function should NOT
//	be changing any member variables under the assumption that the save succeeded.  All code that needs to be updated after
//	the save succeeds should be placed in PostSaveUpdate()
// (j.jones 2007-01-11 14:28) - PLID 24027 - tracked strPostSaveSql, for sql statements to occur after the main save
// (z.manning, 02/07/2007) - PLID 24599 - This overload of GenerateSaveString allows an array to be passed
// in to keep track of deleted template topic IDs. When using this overload the calling function is completely
// responsible for deleting any records from EmrTemplateTopicsT.
// (c.haag 2007-06-20 12:41) - PLID 26397 - We now store saved objects in a map for fast lookups.
// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
// such as new or deleted prescriptions, or new or deleted diagnosis codes
// (a.walling 2014-01-30 00:00) - PLID 60542 - Quantize
Nx::Quantum::Batch CEMRTopic::GenerateSaveString(long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynAffectedEMRDetailIDs, OUT BOOL &bDrugInteractionsChanged, CArray<long,long> &arynDeletedTemplateTopicIDs, BOOL bIsTopLevelSave, BOOL bSaveRecordOnly)
{
	EnsureLoaded();

	// (a.walling 2008-05-23 09:33) - PLID 22049
	_ASSERTE(GetParentEMN() != NULL && GetParentEMN()->IsWritable());

	Nx::Quantum::Batch strSaveString;
	CString strCurParentTopicName = "";

	//TES 4/15/2010 - PLID 24692 - Topics don't maintain their own order any more, so we don't need to do this.
	// (z.manning 2010-03-24 16:26) - PLID 37867 - Before saving this topic's subtopics, make sure we first fix
	// the order index for all of them to ensure that new topics have the correct value too.
	/*if(m_bSubTopicArrayChanged) {
		for(int nSubTopicIndex = 0; nSubTopicIndex < m_arypEMRTopics.GetSize(); nSubTopicIndex++) {
			CEMRTopic *pSubTopic = m_arypEMRTopics.GetAt(nSubTopicIndex);
			pSubTopic->SetTopicOrderIndex(nSubTopicIndex);
		}
	}*/

	//if this is the initial save string, then we need to generate the parent records
	//bSaveRecordOnly will be true if a child is the top level save
	if(bIsTopLevelSave || bSaveRecordOnly) {
		//tell our parent to save its record, which will propagate upwards
		if(m_pParentTopic) {
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strSaveString += m_pParentTopic->GenerateSaveString(nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedEMRDetailIDs, bDrugInteractionsChanged, FALSE, TRUE);
		}
		else {
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strSaveString += GetParentEMN()->GenerateSaveString(GetParentEMN()->GetParentEMR()->GetID(), nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedEMRDetailIDs, bDrugInteractionsChanged, FALSE, TRUE);
		}
		//TES 10/9/2006 - This may have resulted in this topic already being saved.
		if (mapSavedObjects[this] != NULL) {
			//We have already had our save string generated, so there's no need to go any further.
			return strSaveString;
		}
		/*for(int i = 0; i < arSavedObjects.GetSize(); i++) {
			if(arSavedObjects[i] == this) {
				//We have already had our save string generated, so there's no need to go any further.
				return strSaveString;
			}
		}*/
	}

	// (j.jones 2006-08-24 10:37) - PLID 22183 - find out if this topic is on a locked EMN
	BOOL bIsLockedAndSaved = GetIsOnLockedAndSavedEMN();

	// (j.jones 2006-07-11 11:16) - PLID 21391 - while not common, it is possible to get into a state where
	// we've deleted a template topic while still having unsaved topics using the deleted template topic as a parent.
	// In these states, it is virtually impossible to detect and auto-update the unsaved variables, so we have to
	// include it in the batch save so to most efficiently validate our save

	// (a.walling 2009-07-02 15:57) - PLID 34789 - Deprecated -- use @nEMRTemplateTopicID
	//CString strTemplateTopicID = "NULL";
	//GetTemplateTopicID() won't return until we've finished loading, if by chance we haven't yet.
	// (j.jones 2007-07-27 17:38) - PLID 26841 - moved lower in the code to not occur unless necessary
	/*
	if(m_nTemplateTopicID != -1) {
		AddStatementToSqlBatch(strSaveString, "SET @nEMRTemplateTopicID = (SELECT CASE WHEN %li NOT IN (SELECT ID FROM EMRTemplateTopicsT) THEN NULL ELSE %li END)",m_nTemplateTopicID,m_nTemplateTopicID);
		strTemplateTopicID = "@nEMRTemplateTopicID";
	}
	*/

	CString strSourceActionID = "NULL";
	if(m_sai.nSourceActionID != -1) {
		strSourceActionID.Format("%li", m_sai.nSourceActionID);
	}
	//DRT 9/25/2007 - PLID 27515 - The SourceAction SourceID and SourceDataGroupID are not saved, just referenced

	// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
	CString strSourceDetailID = "NULL";
	if(m_sai.nSourceDetailID != -1) {
		strSourceDetailID.Format("%li", m_sai.nSourceDetailID);
	}

	// (z.manning 2009-03-05 09:25) - PLID 33338 - SourceDataGroupID
	CString strSourceDataGroupID = "NULL";
	if(m_sai.GetDataGroupID() != -1) {
		strSourceDataGroupID = AsString(m_sai.GetDataGroupID());
	}
	// (z.manning 2010-02-26 16:37) - PLID 37540
	CString strSourceDetailImageStampID = "NULL";
	if(m_sai.GetDetailStampID() != -1) {
		strSourceDetailImageStampID = AsString(m_sai.GetDetailStampID());
	}

	//TES 3/16/2010 - PLID 37530 - Need to store the (global) Stamp ID, and its index within this image (i.e., is this the 1st or 2nd AK).
	// (z.manning 2010-05-18 11:05) - PLID 38711 - I improved the below logic. Before we were only setting the
	// source stamp ID and index if we had a detail stamp pointer, but we also needed to handle the case where
	// we don't have a pointer.
	CString strSourceStampID = "NULL";
	if(m_sai.GetStampID() != -1) {
		strSourceStampID = AsString(m_sai.GetStampID());
	}
	else if(m_sai.GetDetailStampPointer() != NULL) {
		long nStampID = m_sai.GetDetailStampPointer()->nStampID;
		if(nStampID != -1) {
			strSourceStampID = AsString(nStampID);
		}
	}
	CString strSourceStampIndex = "NULL";
	long nStampIndex = m_sai.GetTableRow()->m_ID.nStampIndexInDetailByType;
	if(nStampIndex != -1) {
		strSourceStampIndex = AsString(nStampIndex);
	}

	CString strParentTopicID = "NULL";
	if(m_pParentTopic) {
		if(m_pParentTopic->GetID() == -1) {
			AddStatementToSqlBatch(strSaveString, "SET @nParentTopicID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotTopic, (long)m_pParentTopic);
			strParentTopicID = "@nParentTopicID";
		}
		else {
			strParentTopicID.Format("%li", m_pParentTopic->GetID());
		}
	}

	long nEMNID = GetParentEMN()->GetID();

	CString strOriginalTemplateTopicID = "NULL";
	if(m_nOriginalTemplateTopicID != -1) {
		strOriginalTemplateTopicID.Format("%li", m_nOriginalTemplateTopicID);
	}

	//DRT 9/28/2006 - PLID 22563 - This has been here for a while, but not documented.  We believe this was put in
	//	to account for some bad data in official content, but was never hit before being released.  Then I found
	//	bad data in 3 templates, but 2 of them were deleted.  This has also happened to 2 clients as well.
	//You should not be able to have a SourceTemplateTopicID in EMRTemplateTopicsT without a SourceActionID.  You
	//	can use this query to detect the problem in your data.  If you are certain the data was created after
	//	the 2006-07-31 (8.2) release, then there is a new bug generating this data which needs to be tracked down.
	//
	//select EMRTemplateT.ID, EMRTemplateT.Name AS TemplateName, 
	//EMRTemplateTopicst.ID, EMRTemplateTopicsT.Name AS TopicName
	//from emrtemplatetopicst 
	//left join emrtemplatet on emrtemplatetopicst.templateid = emrtemplatet.id
	//where sourceactionid is null and sourcetemplatetopicid is not null
	//
	if(m_bIsTemplate && m_nOriginalTemplateTopicID != -1 && m_sai.nSourceActionID == -1) {
		AfxThrowNxException("Tried to save topic with source topic but no source action!");
	}

	// (j.jones 2007-04-12 12:22) - PLID 25096 - moved the deletion code to be prior to the saving code,
	// which solves a problem with triggers if we deleted an item then created a duplicate item in one save

	//Handle deleted sub-topics.
	for(int i=0; i < m_aryDeletedTopics.GetSize(); i++) {
		// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
		strSaveString += m_aryDeletedTopics[i]->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynDeletedTemplateTopicIDs, arynAffectedEMRDetailIDs);
//		delete m_aryDeletedTopics.GetAt(i); // (a.walling 2007-02-06 15:01) - PLID 24602
		// these CEMNTopics are referred to in other GenerateSaveStrings, so this is causing a crash.
		// Instead I am moving these deletions to PostSaveUpdate() as per instructions in the function header.
	}
//	m_aryDeletedTopics.RemoveAll();

	//Handle deleted details.
	for(i=0;i < m_aryDeletedDetails.GetSize(); i++) {
		// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
		strSaveString += m_aryDeletedDetails[i]->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynAffectedEMRDetailIDs);
		// (a.walling 2007-07-19 14:19) - PLID 26757 - When I fixed 26402, I should have realized that this
		// DeleteEMNDetail function actually deletes the CEMNDetail and moved it to PostSaveUpdate appropriately.
		// Unfortunately I had overlooked it, so let us do that now.

		// GetParentEMN()->GetParentEMR()->DeleteEMNDetail(m_aryDeletedDetails.GetAt(i));
	}

	// (j.jones 2007-04-12 12:22) - PLID 25096 - moved to PostSaveUpdate()
	//m_aryDeletedDetails.RemoveAll();

	if(m_nID == -1) {
		//save new

		//TES 4/15/2010 - PLID 24692 - We'll just be pulling this from our TopicPositionEntry
		/*if(m_nTopicOrderIndex == -1) {

			//TES 6/22/2006 - The order index is now our parent's responsibility.  However, since the column doesn't allow NULL,
			//we might as well put in a very good guess as to what the actual value will be.

			//calculate the order index
			if(m_pParentTopic) {
				for(int i=0; i<m_pParentTopic->GetSubTopicCount(); i++) {
					if(m_pParentTopic->GetSubTopic(i) == this)
						m_nTopicOrderIndex = i;
				}
			}
			else {
				for(int i=0; i<GetParentEMN()->GetTopicCount(); i++) {
					if(GetParentEMN()->GetTopic(i) == this)
						m_nTopicOrderIndex = i;
				}
			}
		}*/

		

		if(m_bIsTemplate) {
			//save new template topic record
			if(nEMNID == -1) {
				AddStatementToSqlBatch(strSaveString, "SET @nEMRTemplateID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotEMN, (long)GetParentEMN());
			}
			else {
				AddStatementToSqlBatch(strSaveString, "SET @nEMRTemplateID = %li", nEMNID);
			}
			// (a.walling 2008-06-30 14:43) - PLID 29271 - Preview Pane flags
			// (z.manning 2009-03-05 09:37) - PLID 33338 - SourceActionGroupID
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID, SourceStampIndex
			//TES 4/15/2010 - PLID 24692 - Pull the order from m_pTopicPositionEntry
			AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRTemplateTopicsT (TemplateID, Name, OrderIndex, EMRParentTemplateTopicID, "
				"SourceActionID, SourceDetailID, SourceTemplateTopicID, ShowIfEmpty, HideOnEMN, PreviewFlags, SourceDataGroupID, SourceStampID, SourceStampIndex)"
				"VALUES (@nEMRTemplateID, '%s', %li, %s, %s, %s, %s, %i, %i, %li, %s, %s, %s)\r\n"
				"SET @nNewObjectID = SCOPE_IDENTITY()\r\n" // (c.haag 2007-06-11 15:36) - PLID 26277 - Use the scope identity function to get the ID of the newly created template topic record
				"SET @nEMRTemplateTopicID = @nNewObjectID" // (c.haag 2007-06-11 15:40) - PLID 26267 - Immediately set the EMR template topic ID. It should not change until a new template topic is added.
				, _Q(m_strTopicName), m_pTopicPositionEntry->GetSortOrder(), strParentTopicID, strSourceActionID, strSourceDetailID, strOriginalTemplateTopicID, m_bShowIfEmpty?1:0, m_bHideOnEMN?1:0, m_nPreviewFlags, strSourceDataGroupID, strSourceStampID, strSourceStampIndex);

			// (j.jones 2007-01-11 14:41) - PLID 24027 - if our source detail ID is -1, then
			// be sure to update SourceDetailID at the end of the save!
			if(m_sai.pSourceDetail && m_sai.nSourceDetailID == -1) {
				//find this topic's ID, find the source detail's ID, and update this topic's SourceDetailID
				AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRObjectIDToUpdate = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), -1)", esotTopic, (long)this);
				AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRSourceDetailID = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetail, (long)m_sai.pSourceDetail);
				AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRTemplateTopicsT SET SourceDetailID = @nEMRSourceDetailID WHERE ID = @nEMRObjectIDToUpdate");
			}

			// (c.haag 2007-06-11 15:38) - PLID 26277 - This is no longer necessary now that we use the scope identity function
			//AddStatementToSqlBatch(strSaveString, "SET @nNewObjectID = (SELECT COALESCE(MAX(ID), 0) FROM EMRTemplateTopicsT WITH(UPDLOCK, HOLDLOCK))");
			AddNewEMRObjectToSqlBatch(strSaveString, esotTopic, (long)this, mapSavedObjects);

			//only audit if the EMN is not new
			if(nEMNID != -1) {
				//auditing (using transactions)
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				AuditEvent(-1, "", nAuditTransactionID, aeiEMNTemplateTopicAdded, nEMNID, "", m_strTopicName, aepMedium, aetCreated);

				// (z.manning, 11/01/2006) - PLID 22396 - If they changed the show/hide when empty option, audit it.
				if(m_bShowIfEmpty != m_bLastSavedShowIfEmpty) {
					CString strNew, strOld = FormatString("(Topic: %s) ", m_strTopicName);
					if(m_bShowIfEmpty) {
						strOld += "Hide When Empty";
						strNew = "Show When Empty";
					}
					else {
						strOld += "Show When Empty";
						strNew = "Hide When Empty";
					}
					AuditEvent(-1, "", nAuditTransactionID, aeiEMNTemplateTopicToggleShowIfEmpty, nEMNID, strOld, strNew, aepMedium, aetChanged);
				}
			}
		}
		else {
			//new patient EMN topic record

			if(!bIsLockedAndSaved) {

				// (c.haag 2007-06-11 17:28) - PLID 26280 - This is no longer necessary. We assign nEMNID when the record is created in NewObjectsT.
				/*if(nEMNID == -1) {
					AddStatementToSqlBatch(strSaveString, "SET @nEMNID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotEMN, (long)GetParentEMN());
				}
				else {
					AddStatementToSqlBatch(strSaveString, "SET @nEMNID = %li", nEMNID);
				}*/
				CString strEMNID;
				if(nEMNID != -1) {
					strEMNID.Format("%li",nEMNID);
				}
				else {
					strEMNID = "@nEMNID";
				}
				// (a.walling 2008-06-30 14:44) - PLID 29271 - Preview Pane flags
				// (z.manning 2009-03-05 09:39) - PLID 33338 - SourceDataGroupID
				//TES 4/15/2010 - PLID 24692 - Pull the order from m_pTopicPositionEntry
				AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRTopicsT (EMRID, Name, OrderIndex, "
					"SourceActionID, SourceDetailID, EmrParentTopicID, ShowIfEmpty, PreviewFlags, SourceDataGroupID, SourceDetailImageStampID) "
					"VALUES (%s, '%s', %li, %s, %s, %s, %i, %li, %s, %s)\r\n"
					"SET @nNewObjectID = SCOPE_IDENTITY()\r\n" // (c.haag 2007-06-11 15:43) - PLID 26277 - Use the scope identity function to get the ID of the newly created topic record
					"SET @nEMRTopicID = @nNewObjectID"		// (c.haag 2007-06-11 15:43) - PLID 26267 - Immediately set the EMR topic ID. It should not change until a new topic is added.
					,strEMNID, _Q(m_strTopicName), m_pTopicPositionEntry->GetSortOrder(), strSourceActionID, strSourceDetailID, strParentTopicID, m_bShowIfEmpty?1:0, m_nPreviewFlags, strSourceDataGroupID, strSourceDetailImageStampID);

				// (j.jones 2007-01-11 14:41) - PLID 24027 - if our source detail ID is -1, then
				// be sure to update SourceDetailID at the end of the save!
				if(m_sai.pSourceDetail && m_sai.nSourceDetailID == -1) {
					//find this topic's ID, find the source detail's ID, and update this topic's SourceDetailID
					AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRObjectIDToUpdate = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), -1)", esotTopic, (long)this);
					AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRSourceDetailID = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetail, (long)m_sai.pSourceDetail);
					AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRTopicsT SET SourceDetailID = @nEMRSourceDetailID WHERE ID = @nEMRObjectIDToUpdate");
				}

				// (z.manning 2010-02-26 17:12) - PLID 37540 - Handle source detail stamp pointer
				if(m_sai.GetDetailStampPointer() != NULL && m_sai.GetDetailStampID() == -1) {
					AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRObjectIDToUpdate = COALESCE((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), -1)", esotTopic, (long)this);
					AddStatementToSqlBatch(strPostSaveSql, "SET @nSourceDetailImageStampID = COALESCE((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetailImageStamp, (long)m_sai.GetDetailStampPointer());
					AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRTopicsT SET SourceDetailImageStampID = @nSourceDetailImageStampID WHERE ID = @nEMRObjectIDToUpdate");
				}

				// (c.haag 2007-06-11 15:43) - PLID 26267 - This is no longer necessary now that we use the scope identity function
				//AddStatementToSqlBatch(strSaveString, "SET @nNewObjectID = (SELECT COALESCE(MAX(ID), 0) FROM EMRTopicsT WITH(UPDLOCK, HOLDLOCK))");
				AddNewEMRObjectToSqlBatch(strSaveString, esotTopic, (long)this, mapSavedObjects);

				// (j.jones 2007-07-27 17:39) - PLID 26841 - moved from higher in code, to only be set here when needed
				if(m_nTemplateTopicID != -1) {
					//AddStatementToSqlBatch(strSaveString, "SET @nEMRTemplateTopicID = (SELECT CASE WHEN %li NOT IN (SELECT ID FROM EMRTemplateTopicsT) THEN NULL ELSE %li END)",m_nTemplateTopicID,m_nTemplateTopicID);
					// (a.walling 2009-07-02 15:57) - PLID 34789 - Faster way to determine whether this is still a valid template topic id
					AddStatementToSqlBatch(strSaveString, "SET @nEMRTemplateTopicID = (SELECT ID FROM EMRTemplateTopicsT WHERE ID = %li)", m_nTemplateTopicID);
					//strTemplateTopicID = "@nEMRTemplateTopicID";
					
					// (a.walling 2009-07-02 15:57) - PLID 34789 - And only save if it is not null.
					AddStatementToSqlBatch(strSaveString, "IF @nEMRTemplateTopicID IS NOT NULL BEGIN "
						"INSERT INTO EmrTemplateTopicToTopicLinkT (EmrTopicID, EmrTemplateTopicID) "
						"VALUES (@nNewObjectID, @nEMRTemplateTopicID) END");
				}

				// (a.walling 2009-07-02 15:57) - PLID 34789 - Taken care of above
				/*
				if(strTemplateTopicID != "NULL") {
					AddStatementToSqlBatch(strSaveString, "INSERT INTO EmrTemplateTopicToTopicLinkT (EmrTopicID, EmrTemplateTopicID) "
						"VALUES (@nNewObjectID, %s)", strTemplateTopicID);
				}
				*/

				// (j.jones 2008-07-22 15:06) - PLID 30789 - save all our problems
				// (z.manning 2009-05-22 09:58) - PLID 34297 - Just save a link to the problem
				SaveProblemLinkArray(strSaveString, m_apEmrProblemLinks, "@nEMRTopicID", mapSavedObjects
					, nAuditTransactionID, GetParentEMN()->GetParentEMR()->GetPatientID(), GetExistingPatientName(GetParentEMN()->GetParentEMR()->GetPatientID()));

				//only audit if the EMN is not new
				if(nEMNID != -1) {
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(GetParentEMN()->GetParentEMR()->GetPatientID(), GetExistingPatientName(GetParentEMN()->GetParentEMR()->GetPatientID()), 
						nAuditTransactionID, aeiEMNTopicAdded, nEMNID, "", m_strTopicName, aepMedium, aetCreated);
				}
			}
			else {
				//we tried to change this on a locked EMN!
				CString str;
				str.Format("Attempted to create a new topic '%s' on a locked EMN.", m_strTopicName);
				arystrErrors.Add(str);
			}
		}		
	}
	else {

		if(m_bUnsaved || bSaveRecordOnly) {

			//update existing

			if(m_bIsTemplate) {
				//update existing template topic record

				// (a.walling 2008-06-30 14:44) - PLID 29271 - Preview Pane flags
				//TES 3/17/2010 - PLID 37530 - Added SourceStampID, SourceStampIndex
				AddStatementToSqlBatch(strSaveString, "UPDATE EMRTemplateTopicsT SET Name = '%s', "
					"EMRParentTemplateTopicID = %s, SourceActionID = %s, SourceDetailID = %s, SourceDataGroupID = %s, SourceTemplateTopicID = %s, ShowIfEmpty = %i, "
					"HideOnEMN = %i, PreviewFlags = %li, SourceStampID = %s, SourceStampIndex = %s "
					"WHERE ID = %li", _Q(m_strTopicName), strParentTopicID, strSourceActionID, strSourceDetailID, strSourceDataGroupID, strOriginalTemplateTopicID, m_bShowIfEmpty?1:0, m_bHideOnEMN?1:0, m_nPreviewFlags, strSourceStampID, strSourceStampIndex, m_nID);

				//only audit if the topic is not new
				if(m_nID != -1) {
					//auditing (using transactions)

					//Name
					if(m_strLastSavedTopicName != m_strTopicName) {
						if(nAuditTransactionID == -1)
							nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(-1, "", nAuditTransactionID, aeiEMNTemplateTopicRenamed, nEMNID, m_strLastSavedTopicName, m_strTopicName, aepMedium, aetChanged);
					}

					//OrderIndex is audited on the parent level

					// (j.jones 2006-01-23 13:52) - audit if the topic moved to be underneath another topic
					if(m_pParentTopic)
						strCurParentTopicName = m_pParentTopic->GetName();
					if(m_pParentTopic != m_pOriginalParentTopic) {
						CString strOld, strNew;
						if(m_strLastSavedParentTopicName == "")
							strOld.Format("%s was a top-level topic.", m_strTopicName);
						else
							strOld.Format("%s was below %s.", m_strTopicName, m_strLastSavedParentTopicName);
						if(strCurParentTopicName == "")
							strNew.Format("%s is now a top-level topic.", m_strTopicName);
						else
							strNew.Format("%s is now below %s.", m_strTopicName, strCurParentTopicName);
						if(nAuditTransactionID == -1)
							nAuditTransactionID = BeginAuditTransaction();
						AuditEvent(-1, "", nAuditTransactionID, aeiEMNTemplateTopicMoved, nEMNID, strOld, strNew, aepMedium, aetChanged);
					}
				}

				// Only audit if the EMN is not new.
				if(nEMNID != -1) {
					if(nAuditTransactionID == -1) {	nAuditTransactionID = BeginAuditTransaction(); }

					// (z.manning, 11/01/2006) - PLID 22396 - If they changed the show/hide when empty option, audit it.
					if(m_bShowIfEmpty != m_bLastSavedShowIfEmpty) {
						CString strNew, strOld = FormatString("(Topic: %s) ", m_strTopicName);
						if(m_bShowIfEmpty) {
							strOld += "Hide When Empty";
							strNew = "Show When Empty";
						}
						else {
							strOld += "Show When Empty";
							strNew = "Hide When Empty";
						}
						AuditEvent(-1, "", nAuditTransactionID, aeiEMNTemplateTopicToggleShowIfEmpty, nEMNID, strOld, strNew, aepMedium, aetChanged);
					}
				}
			}
			else {
				//update existing patient EMN topic record

				if(!bIsLockedAndSaved) {

					// (a.walling 2008-06-30 14:45) - PLID 29271 - Preview Pane flags
					AddStatementToSqlBatch(strSaveString, "UPDATE EMRTopicsT SET Name = '%s', "
						"EMRParentTopicID = %s, ShowIfEmpty = %i, PreviewFlags = %li WHERE ID = %li",
						_Q(m_strTopicName), strParentTopicID, m_bShowIfEmpty?1:0, m_nPreviewFlags, m_nID);

					// (j.jones 2008-07-22 15:06) - PLID 30789 - save all our problems
					// (z.manning 2009-05-22 09:59) - PLID 34297 - Just save problem links
					SaveProblemLinkArray(strSaveString, m_apEmrProblemLinks, AsString(m_nID), mapSavedObjects
						, nAuditTransactionID, GetParentEMN()->GetParentEMR()->GetPatientID(), GetExistingPatientName(GetParentEMN()->GetParentEMR()->GetPatientID()));
				}

				//only audit if the topic is not new
				//or, process through here if we tried to update a locked EMN
				if(m_nID != -1 || bIsLockedAndSaved) {
					//auditing (using transactions)
					//also processing error messages if updating a locked EMN
					
					//Name
					if(m_strLastSavedTopicName != m_strTopicName) {

						if(!bIsLockedAndSaved) {
							if(nAuditTransactionID == -1)
								nAuditTransactionID = BeginAuditTransaction();
							AuditEvent(GetParentEMN()->GetParentEMR()->GetPatientID(), GetExistingPatientName(GetParentEMN()->GetParentEMR()->GetPatientID()),
								nAuditTransactionID, aeiEMNTopicRenamed, nEMNID, m_strLastSavedTopicName, m_strTopicName, aepMedium, aetChanged);
						}
						else {
							//we tried to change this on a locked EMN!
							CString str;
							str.Format("Attempted to change the name of topic '%s' to '%s' on a locked EMN.", m_strLastSavedTopicName, m_strTopicName);
							arystrErrors.Add(str);
						}
					}
					
					//OrderIndex is audited on the parent level

					// (j.jones 2006-01-23 13:52) - audit if the topic moved to be underneath another topic				
					if(m_pParentTopic)
						strCurParentTopicName = m_pParentTopic->GetName();

					if(m_pParentTopic != m_pOriginalParentTopic) {

						CString strOld, strNew;
							if(m_strLastSavedParentTopicName == "")
								strOld.Format("%s was a top-level topic.", m_strTopicName);
							else
								strOld.Format("%s was below %s.", m_strTopicName, m_strLastSavedParentTopicName);
							if(strCurParentTopicName == "")
								strNew.Format("%s is now a top-level topic.", m_strTopicName);
							else
								strNew.Format("%s is now below %s.", m_strTopicName, strCurParentTopicName);

						if(!bIsLockedAndSaved) {							
							if(nAuditTransactionID == -1)
								nAuditTransactionID = BeginAuditTransaction();
							AuditEvent(GetParentEMN()->GetParentEMR()->GetPatientID(), GetExistingPatientName(GetParentEMN()->GetParentEMR()->GetPatientID()),
								nAuditTransactionID, aeiEMNTopicMoved, nEMNID, strOld, strNew, aepMedium, aetChanged);
						}
						else {
							//we tried to change this on a locked EMN!
							CString str;
							str.Format("Attempted to move the placement of topic '%s' on a locked EMN. %s. %s.", m_strTopicName, strOld, strNew);
							arystrErrors.Add(str);
						}
					}
				}
			}			
		}
	}

	//if we are saving just the record, then do not save its details
	if(!bSaveRecordOnly) {

		//now generate save statements for our details

		// (j.jones 2006-02-27 09:35) - PLID 19473 - needs to be removed/altered at some point in the future,
		// at the moment we load details from spawned topics and we will not re-save them
		BOOL bSkipSaveDetails = FALSE;
		if(m_nOriginalTemplateTopicID != -1 && m_sai.nSourceActionID != -1 && m_bIsTemplate) {
			// (j.jones 2006-02-24 17:36) - PLID 19473 - if a spawned, remembered topic,
			//load the details from the original topic, not anything saved in the remembered topic
			bSkipSaveDetails = TRUE;
		}

		if(!bSkipSaveDetails) {
			for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
				// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
				strSaveString += m_arypEMNDetails.GetAt(i)->GenerateSaveString(m_nID, nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedEMRDetailIDs, FALSE);
			}
		}
	}

	//if saving the record only, we have to save the "required" topics so far, so we can save just those,
	//but otherwise we are saving everything and need to add the remaining topics

	//now save all sub-topics

	// (j.jones 2006-02-21 09:02) - PLID 19341 - Detect whether or not any details
	// have switched topics. If so, determine the proper topic saving order, then
	// save other topics first if necessary

	CArray<CEMRTopic*,CEMRTopic*> aryEMRTopicsInOrder;

	if(GetParentEMN()->GetDetailsHaveMoved()) {		
		//this will calculate if we need to save topics in a given order,
		//and populates the array with that desired saving order
		for(int i=0;i<m_arypEMRTopics.GetSize();i++) {
			
			CEMRTopic *pTopic = m_arypEMRTopics.GetAt(i);
			CArray<CEMRTopic*,CEMRTopic*> aryEMRTopicsToSave;

			//this is going to return more topics than we need,
			//we only need to save our own subtopics in the appropriate order
			
			if(pTopic->CalculateTopicSavingOrderNeeded(aryEMRTopicsToSave)) {
				
				//we're only going to save those listed topics that are beneath our own topic
				
				for(int j=0; j<aryEMRTopicsToSave.GetSize(); j++) {

					//verify that the topic is underneath us
					CEMRTopic *pTopicToSave = aryEMRTopicsToSave.GetAt(j);
					CEMRTopic *pOurSubtopic = GetSubTopicWithThisTopic(pTopicToSave);

					//if pOurSubtopic is not NULL, it means we do need to save this subtopic in order
					if(pOurSubtopic) {

						//ensure we are only adding unique topics
						BOOL bFound = FALSE;					
						for(int k=0; k<aryEMRTopicsInOrder.GetSize() && !bFound; k++) {
							if(pOurSubtopic == aryEMRTopicsInOrder.GetAt(k))
								bFound = TRUE;
						}
						
						if(!bFound) {
							aryEMRTopicsInOrder.Add(pOurSubtopic);

							// (j.jones 2006-08-31 10:01) - PLID 22325 - remember that PostSaveUpdate()
							// needs to run for the other topics later
							m_arypOtherSavedTopics.Add(pOurSubtopic);
						}
					}
				}
			}
		}
	}

	//if saving the record only, we have to save the "required" topics so far, so we can save just those,
	//but otherwise we are saving everything and need to add the remaining topics
	if(!bSaveRecordOnly) {
		//aryEMRTopicsInOrder may or may not be populated at this point,
		//so add all the subtopics topics that aren't in aryEMRTopicsInOrder
		for(int i=0;i<m_arypEMRTopics.GetSize();i++) {
			
			CEMRTopic *pTopic = m_arypEMRTopics.GetAt(i);

			BOOL bFound = FALSE;		
			for(int j=0; j<aryEMRTopicsInOrder.GetSize() && !bFound; j++) {
				if(pTopic == aryEMRTopicsInOrder.GetAt(j))
					bFound = TRUE;
			}
			
			if(!bFound) {
				aryEMRTopicsInOrder.Add(pTopic);
			}
		}
	}

	//now save in order of aryEMRTopicsInOrder, which may or may not have been rearranged
	//from the initial topic order (this won't affect the OrderIndex)
	//this can also be empty or less than the total topic count if bSaveRecordOnly is TRUE
	for(i=0;i<aryEMRTopicsInOrder.GetSize();i++) {
		// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
		strSaveString += aryEMRTopicsInOrder.GetAt(i)->GenerateSaveString(nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedEMRDetailIDs, bDrugInteractionsChanged, FALSE, FALSE);
	}

	//TES 4/15/2010 - PLID 24692 - Topics don't maintain their own order any more, so we don't need to do this.
	//TES 6/22/2006 - Now set the order of the topics, if we need to.
	/*if(m_bSubTopicArrayChanged) {
		for(int i=0; i < m_arypEMRTopics.GetSize(); i++) {
			// (j.jones 2007-03-02 17:02) - PLID 25048 - don't try to update a nonexistent topic!
			long nTopicID = m_arypEMRTopics[i]->GetID();
			if(nTopicID != -1) {
				if(m_bIsTemplate) {
					// (c.haag 2007-06-11 09:19) - PLID 26268 - Make sure the statement is placed properly in the batch
					AddStatementToSqlBatch(strSaveString, "UPDATE EmrTemplateTopicsT SET OrderIndex = %li WHERE ID = %li", i, nTopicID);
					// (z.manning 2010-03-24 16:23) - PLID 37867 - We already updated topic sort order
					//m_arypEMRTopics[i]->SetTopicOrderIndex(i);
				}
				else {
					if(!bIsLockedAndSaved) {
						// (c.haag 2007-06-11 09:19) - PLID 26268 - Make sure the statement is placed properly in the batch
						AddStatementToSqlBatch(strSaveString, "UPDATE EmrTopicsT SET OrderIndex = %li WHERE ID = %li", i, nTopicID);
						// (z.manning 2010-03-24 16:23) - PLID 37867 - We already updated topic sort order
						//m_arypEMRTopics[i]->SetTopicOrderIndex(i);
					}
					else {
						//we tried to change this on a locked EMN!
						CString str;
						str.Format("Attempted to change the order index of topic '%s' - subtopic '%s' to %li on a locked EMN.", m_strTopicName, m_arypEMRTopics[i]->GetName(), i);
						arystrErrors.Add(str);
					}
				}


				// (j.jones 2006-10-16 11:56) - PLID 23084 - restored the OrderIndex audit ability

				//OrderIndex audit
				if(m_arypEMRTopics[i]->GetHasMoved() && m_arypEMRTopics[i]->GetLastSavedTopicOrderIndex() != m_arypEMRTopics[i]->GetTopicOrderIndex()) {
					CString strOldIndex, strNewIndex;
					CString strOldParent, strNewParent;

					strOldParent = m_arypEMRTopics[i]->GetLastSavedParentTopicName();
					if(strOldParent.IsEmpty())
						strOldParent = "Top-Level";

					if(m_arypEMRTopics[i]->GetParentTopic())
						strNewParent = m_arypEMRTopics[i]->GetParentTopic()->GetName();
					else
						strNewParent = "Top-Level";

					strOldIndex.Format("%s was Topic %li (%s)", m_arypEMRTopics[i]->GetName(), m_arypEMRTopics[i]->GetLastSavedTopicOrderIndex()+1, strOldParent);
					strNewIndex.Format("%s is now Topic %li (%s)", m_arypEMRTopics[i]->GetName(), m_arypEMRTopics[i]->GetTopicOrderIndex()+1, strNewParent);
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(-1, "", nAuditTransactionID, m_bIsTemplate ? aeiEMNTemplateTopicMoved : aeiEMNTopicMoved, nEMNID, strOldIndex, strNewIndex, aepMedium, aetChanged);
				}
			}
		}
	}*/

	//TES 5/3/2010 - PLID 24692 - Restore just the auditing part of the above code.
	for(int i=0; i < m_arypEMRTopics.GetSize(); i++) {
		long nTopicID = m_arypEMRTopics[i]->GetID();
		if(nTopicID != -1) {
			// (j.jones 2006-10-16 11:56) - PLID 23084 - restored the OrderIndex audit ability

			//OrderIndex audit
			TopicPositionEntry * tpe = m_arypEMRTopics[i]->GetTopicPositionEntry();
			//TES 5/20/2010 - PLID 24692 - Restored the original behavior, where this would only audit if it had been manually moved.
			if(m_arypEMRTopics[i]->GetHasMoved() && tpe->nLastSavedOrderIndex != tpe->GetSortOrder()) {
				CString strOldIndex, strNewIndex;
				CString strOldParent, strNewParent;

				strOldParent = m_arypEMRTopics[i]->GetLastSavedParentTopicName();
				if(strOldParent.IsEmpty())
					strOldParent = "Top-Level";

				if(m_arypEMRTopics[i]->GetParentTopic())
					strNewParent = m_arypEMRTopics[i]->GetParentTopic()->GetName();
				else
					strNewParent = "Top-Level";

				strOldIndex.Format("%s was Topic %li (%s)", m_arypEMRTopics[i]->GetName(), tpe->nLastSavedOrderIndex+1, strOldParent);
				strNewIndex.Format("%s is now Topic %li (%s)", m_arypEMRTopics[i]->GetName(), tpe->GetSortOrder()+1, strNewParent);
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				//TES 5/12/2010 - PLID 38470 - If this is an EMN, include the patient ID and name.
				AuditEvent(m_bIsTemplate ? -1 : GetParentEMR()->GetPatientID(), m_bIsTemplate ? "" : GetExistingPatientName(GetParentEMR()->GetPatientID()), nAuditTransactionID, m_bIsTemplate ? aeiEMNTemplateTopicMoved : aeiEMNTopicMoved, nEMNID, strOldIndex, strNewIndex, aepMedium, aetChanged);
			}
		}
	}

	// (j.jones 2006-08-29 09:00) - PLID 22250 - update the EMN/Template modified date
	if(IsUnsaved(TRUE) && nEMNID != -1) {
		if(m_bIsTemplate) {
			AddStatementToSqlBatch(strSaveString, "UPDATE EMRTemplateT SET ModifiedDate = GetDate(), ModifiedLogin = '%s' WHERE ID = %li", _Q(GetCurrentUserName()), nEMNID);
		}
		else if(!bIsLockedAndSaved) {
			AddStatementToSqlBatch(strSaveString, "UPDATE EMRMasterT SET ModifiedDate = GetDate() WHERE ID = %li", nEMNID);
		}
	}

	//We have now had our save string generated.
	mapSavedObjects[this] = this;

	return strSaveString;
}

//DRT 2/24/2006 - PLID 19465 - This is a notification function that lets us know a save has just taken place.  This function
//	should contain any code that updates members, etc based on the successful save, not GenerateSaveString()
// (a.walling 2007-10-19 09:48) - PLID 27664 - Added array to gather all topics affected in the PostSaveUpdate cascade.
void CEMRTopic::PostSaveUpdate(BOOL bTopLevelUpdate /*= FALSE*/, BOOL bUpdateRecordOnly /*= FALSE*/, CArray<CEMRTopic*, CEMRTopic*> *parTopicsAffected /*= NULL*/)
{
	int i;

	//if this is the initial update, then we need to generate the parent records
	//bUpdateRecordOnly will be true if a child is the top level save
	if(bTopLevelUpdate || bUpdateRecordOnly) {
		//tell our parent to save its record, which will propagate upwards
		if(m_pParentTopic) {
			// (a.walling 2007-10-18 16:38) - PLID 27664 - Pass the array to gather all affected topics
			m_pParentTopic->PostSaveUpdate(FALSE, TRUE, parTopicsAffected);
		}
		else {
			// (j.jones 2007-02-27 11:27) - PLID 24940 - we didn't pass in a value for the progress dialog!
			// (a.walling 2007-10-18 16:38) - PLID 27664 - Pass the array to gather all affected topics
			GetParentEMN()->PostSaveUpdate(NULL, FALSE, TRUE, parTopicsAffected);
		}
	}


	////////////////////////
	//Put all topic specific update code here

	// (a.walling 2007-10-15 15:54) - PLID 27664 - Store the last saved HTML
	if (GetParentEMN() && parTopicsAffected != NULL) {
		// (a.walling 2007-10-18 16:41) - PLID 27664 - As an optimization, we will simply add ourselves to the parTopicsAffected array
		// and then the HTML generation can occur afterwards.
		if (!IsTopicInArray(*parTopicsAffected, this)) {
			parTopicsAffected->Add(this);
		}
	}

	// (j.jones 2006-02-21 13:15) - now we can set the current parent topic
	m_pOriginalParentTopic = m_pParentTopic;

	m_strLastSavedTopicName = m_strTopicName;
//	m_nLastSavedTopicOrderIndex = m_nTopicOrderIndex;
	//TES 5/3/2010 - PLID 24692 - Tell our position entry to remember its order
	if(m_pTopicPositionEntry) {
		m_pTopicPositionEntry->SetLastSavedOrderIndex();
	}
	m_bLastSavedShowIfEmpty = m_bShowIfEmpty;
	if(m_pParentTopic)
		m_strLastSavedParentTopicName = m_pParentTopic->GetName();
	else
		m_strLastSavedParentTopicName = "";

	for(i=0;i<m_arypEMRTopics.GetSize();i++) {
		m_arypEMRTopics.GetAt(i)->SetHasMoved(FALSE);
	}

	// (c.haag 2008-08-15 13:00) - PLID 30820 - Remove deleted problems from memory
	// (z.manning 2009-05-22 11:56) - PLID 34332 - Update problem links
	for(int nProblemLinkIndex = 0; nProblemLinkIndex < m_apEmrProblemLinks.GetSize(); nProblemLinkIndex++)
	{
		CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(nProblemLinkIndex);
		if(pProblemLink != NULL && (pProblemLink->GetIsDeleted() || (pProblemLink->GetProblem() != NULL && pProblemLink->GetProblem()->m_bIsDeleted))) {
			m_apEmrProblemLinks.RemoveAt(nProblemLinkIndex--);
			delete pProblemLink;
		}
	}

	//Handle deleted sub-topics.
	// (a.walling 2007-02-06 15:01) - PLID 24602
	// These delete statements used to be in GenerateSaveString; however the objects are referred to in
	// other GenerateSaveStrings further down the call stack. This of course led to a crash. So here they
	// are, since our save was successful, and all strings have been generated.
	for(i=0; i < m_aryDeletedTopics.GetSize(); i++) {
		delete m_aryDeletedTopics.GetAt(i); 
	}
	m_aryDeletedTopics.RemoveAll();


	//End all topic specific update code
	////////////////////////

	// (j.jones 2007-04-12 12:22) - PLID 25096 - moved from GenerateSaveString
	// (a.walling 2007-07-19 14:20) - PLID 26757 - Actually delete the CEMNDetail objects marked for deletion
	// (Moved from GenerateSaveString as well)
	{
		for (int i = 0; i < m_aryDeletedDetails.GetSize(); i++) {
			// (c.haag 2009-06-30 08:43) - PLID 34750 - We now have the parent EMN be
			// responsible for deleting the detail instead of the parent EMR
			GetParentEMN()->DeleteEMNDetail(m_aryDeletedDetails.GetAt(i));
		}

		m_aryDeletedDetails.RemoveAll();
	}

	//////////////
	//Handle calling PostSaveUpdate on all details of this topic

	if(!bUpdateRecordOnly) {
		//now generate an update for all children details
		for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
			m_arypEMNDetails.GetAt(i)->PostSaveUpdate();
		}
	}


	/////////////
	//Handle calling PostSaveUpdate on all further subtopics

	if(!bUpdateRecordOnly) {
		for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
			// (a.walling 2007-10-18 16:39) - PLID 27664 - Pass the array to gather all affected topics
			m_arypEMRTopics.GetAt(i)->PostSaveUpdate(FALSE, FALSE, parTopicsAffected);
		}
	}
	else {
		// (j.jones 2006-08-31 10:02) - PLID 22325 - PostSaveUpdate does not
		// maintain the order of saving, so we need to reference
		// m_arypOtherSavedTopics in order to ensure we call PostSaveUpdate
		// for all saved topics
		for(int i=0;i<m_arypOtherSavedTopics.GetSize();i++) {
			// (a.walling 2007-10-18 16:39) - PLID 27664 - Pass the array to gather all affected topics
			m_arypOtherSavedTopics.GetAt(i)->PostSaveUpdate(FALSE, FALSE, parTopicsAffected);
		}		
	}
	//do not delete the pointers, just clear the array
	m_arypOtherSavedTopics.RemoveAll();
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
void CEMRTopic::LogEmrObjectData(int nIndent, BOOL bForceDeletedFlagTrue)
{
	BOOL bDeleted = bForceDeletedFlagTrue;

	// Log this object
	::LogEmrObjectData(nIndent, m_nID, this, esotTopic, (m_nID == -1), IsUnsaved(FALSE), bDeleted, m_strTopicName,
		"sourceActionID = %d  sourceDetailID = %d  sourceDataGroupID = %d  sourceDetailImageStampID = %d"
		, m_sai.nSourceActionID
		, m_sai.GetSourceDetailID()
		, m_sai.GetDataGroupID()
		, m_sai.GetDetailStampID()
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

	// esotDetail
	for (auto d : m_arypEMNDetails)
	{
		d->LogEmrObjectData(nIndent + 1, FALSE, FALSE);
	}
	for (auto d : m_paryPendingDeletionDetails)
	{
		d->LogEmrObjectData(nIndent + 1, TRUE, TRUE);
	}
	for (auto d : m_aryDeletedDetails)
	{
		d->LogEmrObjectData(nIndent + 1, TRUE, FALSE);
	}
	
	// esotTopic
	for (auto t : m_arypEMRTopics)
	{
		t->LogEmrObjectData(nIndent + 1, FALSE);
	}
	for (auto t : m_aryDeletedTopics)
	{
		t->LogEmrObjectData(nIndent + 1, TRUE);
	}
}

// (z.manning, 02/07/2007) - PLID 24599 - Added a parameter to keep track of any deleted template topic IDs.
// The calling function is responsible for deleting any records from EmrTemplateTopicsT with these IDs.
// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
// (a.walling 2014-01-30 00:00) - PLID 60542 - Quantize
Nx::Quantum::Batch CEMRTopic::GenerateDeleteString(long &nAuditTransactionID, CStringArray &arystrErrors, CArray<long,long> &arynDeletedTemplateTopicIDs, CDWordArray &arynAffectedEMRDetailIDs)
{
	EnsureLoaded();
	Nx::Quantum::Batch strDeleteString;
	SourceActionInfo saiBlank;

	// (j.jones 2006-08-24 10:37) - PLID 22183 - find out if this topic is on a locked EMN
	BOOL bIsLockedAndSaved = GetIsOnLockedAndSavedEMN();

	//Handle deleted details.

	BOOL bSkipDeleteDetails = FALSE;

	if(m_nOriginalTemplateTopicID != -1 && m_sai.nSourceActionID != -1 && m_bIsTemplate) {
		// (j.jones 2006-02-24 17:36) - PLID 19473 - if a spawned, remembered topic, then
		//we loaded the details from the original topic, so do not delete them
		bSkipDeleteDetails = TRUE;
	}

	if(!bSkipDeleteDetails)	{
		for(int i = m_aryDeletedDetails.GetSize() - 1; i >= 0; i--) {
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strDeleteString += m_aryDeletedDetails[i]->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynAffectedEMRDetailIDs);
			// (c.haag 2007-05-24 16:57) - PLID 26095 - Use Release() instead of delete
			// (j.jones 2007-10-04 09:59) - PLID 27642 - No! Don't delete it until PostSaveUpdate()!
			//m_aryDeletedDetails.GetAt(i)->Release();
		}
		// (c.haag 2007-10-02 12:41) - PLID 27602 - The following statement belongs inside
		// the !bSkipDeleteDetails conditional, not outside
		// (j.jones 2007-10-04 09:59) - PLID 27642 - No! Don't delete it until PostSaveUpdate()!
		//m_aryDeletedDetails.RemoveAll();
	}

	//TES 3/16/2006 - PLID 19496 - Track which topics we're deleting.
	CArray<long,long> arDeletedTopicIDs;
	//remember to delete all of its child topics that have been flagged for deletion
	if(!bSkipDeleteDetails)	{
		for(int i = m_aryDeletedTopics.GetSize() - 1; i >= 0; i--) {
			arDeletedTopicIDs.Add(m_aryDeletedTopics[i]->GetID());
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strDeleteString += m_aryDeletedTopics[i]->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynDeletedTemplateTopicIDs, arynAffectedEMRDetailIDs);
			// (j.jones 2007-10-04 09:59) - PLID 27642 - No! Don't delete it until PostSaveUpdate()!
			//delete m_aryDeletedTopics.GetAt(i);
		}
		// (c.haag 2007-10-02 12:41) - PLID 27602 - The following statement belongs inside
		// the !bSkipDeleteDetails conditional, not outside
		// (j.jones 2007-10-04 09:59) - PLID 27642 - No! Don't delete it until PostSaveUpdate()!
		//m_aryDeletedTopics.RemoveAll();
	}
	
	if(!bSkipDeleteDetails)	{
		//CString strChildIDs;
		for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
			arDeletedTopicIDs.Add(m_arypEMRTopics[i]->GetID());
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strDeleteString += m_arypEMRTopics[i]->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynDeletedTemplateTopicIDs, arynAffectedEMRDetailIDs);
			//strChildIDs += AsString(m_arypEMRTopics[i]->GetID()) + ",";
		}

		//delete all of the topic's details
		for(i = 0; i < m_arypEMNDetails.GetSize(); i++) {
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strDeleteString += m_arypEMNDetails[i]->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynAffectedEMRDetailIDs);
		}
	}

	if(m_bIsTemplate) {

		//don't need to delete it if the item doesn't exist,
		//but this does need to be after the previous code so we can clean out the child lists
		if(m_nID == -1)
			return strDeleteString;

		AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrTemplateTopicToTopicLinkT WHERE EmrTemplateTopicID = %li", m_nID);

		// (j.jones 2006-02-24 09:55) - PLID 19416 - any spawned child template topics need to be deleted
		//TES 3/16/2006 - Clarification: This query is actually getting all topics which are actually just the remembered location
		//of this topic, embedded in some other template.  Spawned child template topics are now taken care of just below.
		_RecordsetPtr rsSpawnedTopics = CreateRecordset("SELECT ID FROM EmrTemplateTopicsT WHERE SourceTemplateTopicID = %li AND SourceActionID IS NOT NULL", m_nID);
		while(!rsSpawnedTopics->eof) {
			//create and delete
			long nTemplateTopicID = AdoFldLong(rsSpawnedTopics,"ID");
			//TES 4/15/2010 - PLID 24692 - If we're on a template, we may already have a position entry, otherwise create a new one.
			//TES 5/11/2010 - PLID 24692 - We're about to delete this topic, no need to track its position.  Just give it a new entry,
			// and make sure to clean it up afterwards.
			TopicPositionEntry *tpe = new TopicPositionEntry;
			CEMRTopic *pTopic = new CEMRTopic(m_bIsTemplate, TRUE, tpe);
			//DRT 9/25/2007 - PLID 27515 - We don't override the nSourceActionID, so we won't override the other nSource... data
			pTopic->LoadFromTemplateTopicID(nTemplateTopicID, FALSE, NULL, saiBlank, FALSE, m_bIsTemplate ? -1 : GetParentEMN()->GetParentEMR()->GetPatientID(), NULL, m_bIsTemplate ? NULL : GetParentEMN()->GetParentEMR());
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strDeleteString += pTopic->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynDeletedTemplateTopicIDs, arynAffectedEMRDetailIDs);
			delete pTopic;
			tpe->FreeDescendants();
			delete tpe;
			rsSpawnedTopics->MoveNext();
		}
		rsSpawnedTopics->Close();


		//TES 3/16/2006 - PLID 19496 - If this topic has any child topics in data that we haven't loaded, it means that
		//they are spawned topics which haven't been spawned.  Therefore we should delete them, that way when they are spawned
		//they will be recreated in a default location, which is what we want to happen.
		CArray<CEMRTopic*,CEMRTopic*> arLoadedTopics;
		GetParentEMN()->GetAllTopics(arLoadedTopics);
		CString strLoadedTopics;
		for(int i = 0; i < arLoadedTopics.GetSize(); i++) strLoadedTopics += FormatString("%li,",arLoadedTopics[i]->GetID());
		strLoadedTopics.TrimRight(",");
		if(strLoadedTopics.IsEmpty()) strLoadedTopics = "-1";
		_RecordsetPtr rsChildTopics = CreateRecordset("SELECT ID FROM EmrTemplateTopicsT WHERE EmrParentTemplateTopicID = %li "
			"AND ID NOT IN (%s,%s)", m_nID, ArrayAsString(arDeletedTopicIDs), strLoadedTopics);
		while(!rsChildTopics->eof) {
			//create and delete
			long nTopicID = AdoFldLong(rsChildTopics, "ID");
			//TES 4/15/2010 - PLID 24692 - If we're on a template, we may already have a position entry, otherwise create a new one.
			//TES 5/11/2010 - PLID 24692 - We're about to delete this topic, no need to track its position.  Just give it a new entry,
			// and make sure to clean it up afterwards.
			TopicPositionEntry *tpe = new TopicPositionEntry;
			CEMRTopic *pTopic = new CEMRTopic(m_bIsTemplate, TRUE, tpe);
			//DRT 9/25/2007 - PLID 27515 - We don't override the nSourceActionID, so we won't override the other nSource... data
			pTopic->LoadFromTemplateTopicID(nTopicID, FALSE, NULL, saiBlank, FALSE, m_bIsTemplate ? -1 : GetParentEMN()->GetParentEMR()->GetPatientID(), NULL, m_bIsTemplate ? NULL : GetParentEMN()->GetParentEMR());
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strDeleteString += pTopic->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynDeletedTemplateTopicIDs, arynAffectedEMRDetailIDs);
			delete pTopic;
			tpe->FreeDescendants();
			delete tpe;
			rsChildTopics->MoveNext();
		}
		rsChildTopics->Close();

		// (j.jones 2006-03-15 17:01) - PLID 19473 - we don't store details anymore on "remembered" topic locations,
		// but we errantly used to, so check for those now as we did not do so earlier
		if(bSkipDeleteDetails) {
			//subtopics
			_RecordsetPtr rsBadData = CreateRecordset("SELECT ID FROM EMRTemplateTopicsT WHERE EMRParentTemplateTopicID = %li", m_nID);
			while(!rsBadData->eof) {
				//create and delete
				long nTopicID = AdoFldLong(rsBadData,"ID");
				//TES 4/15/2010 - PLID 24692 - If we're on a template, we may already have a position entry, otherwise create a new one.
				//TES 5/11/2010 - PLID 24692 - We're about to delete this topic, no need to track its position.  Just give it a new entry,
				// and make sure to clean it up afterwards.
				TopicPositionEntry *tpe = new TopicPositionEntry;
				CEMRTopic *pTopic = new CEMRTopic(m_bIsTemplate, TRUE, tpe);
				//DRT 9/25/2007 - PLID 27515 - We don't override the nSourceActionID, so we won't override the other nSource... data
				pTopic->LoadFromTemplateTopicID(nTopicID, FALSE, NULL, saiBlank, FALSE, m_bIsTemplate ? -1 : GetParentEMN()->GetParentEMR()->GetPatientID(), NULL, m_bIsTemplate ? NULL : GetParentEMN()->GetParentEMR());
				// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
				strDeleteString += pTopic->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynDeletedTemplateTopicIDs, arynAffectedEMRDetailIDs);
				delete pTopic;
				tpe->FreeDescendants();
				delete tpe;
				rsBadData->MoveNext();
			}
			rsBadData->Close();

			//details
			rsBadData = CreateRecordset("SELECT ID FROM EMRTemplateDetailsT WHERE EMRTemplateTopicID = %li", m_nID);
			while(!rsBadData->eof) {
				//create and delete
				// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
				CEMNDetail *pDetail = CEMNDetail::CreateDetail(NULL, "EMRTopic delete string baddata detail");
				pDetail->m_nEMRTemplateDetailID = AdoFldLong(rsBadData,"ID");
				pDetail->m_bIsTemplateDetail = TRUE;
				// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
				strDeleteString += pDetail->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynAffectedEMRDetailIDs);
				// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
				pDetail->__QuietRelease();
				//delete pDetail;
				rsBadData->MoveNext();
			}
			rsBadData->Close();
		}

		// (j.jones 2006-02-24 09:58) - if there are any linked topics that aren't spawned, unlink them now
		// (one can argue that this is bad data but currently Import Topics can set data like this, and it is harmless)
		AddStatementToSqlBatch(strDeleteString, "UPDATE EmrTemplateTopicsT SET SourceTemplateTopicID = NULL WHERE SourceTemplateTopicID = %li AND SourceActionID Is Null", m_nID);
		
		// (j.jones 2006-02-21 13:54) - I believe PLID 19341 made this obsolete
		/*
		//TES 2/10/2006 - Now, there may be EmrTemplateTopicsT records that have us as a parent, but are not currently our children.
		//This means that they are not currently spawned, but if they are, then they would be under us.  Since we're being deleted,
		//we need to delete them (which won't actually delete them per se, but will mean that if they are spawned, they will be
		//placed in a default location).
		if(strChildIDs.IsEmpty())
			strChildIDs = "-1";
		else
			strChildIDs.TrimRight(",");
		_RecordsetPtr rsOtherChildren = CreateRecordset("SELECT ID FROM EmrTemplateTopicsT WHERE EmrParentTemplateTopicID = %li AND ID NOT IN (%s)", m_nID, strChildIDs);
		while(!rsOtherChildren->eof) {
			//Create an object.
			CEMRTopic *pTopic = new CEMRTopic(m_bIsTemplate);
			pTopic->LoadFromTemplateTopicID(AdoFldLong(rsOtherChildren,"ID"), FALSE, NULL, -1, -1, FALSE, m_bIsTemplate ? -1 : GetParentEMN()->GetParentEMR()->GetPatientID());
			strDeleteString += pTopic->GenerateDeleteString(nAuditTransactionID);
			rsOtherChildren->MoveNext();
		}
		rsOtherChildren->Close();
		*/
		
		// (z.manning, 02/07/2007) - PLID 24599 - We don't want to delete the EmrTemplateTopicsT record yet
		// because it's possible due to moving details between topics that this can cause foreign key errors.
		// Instead, let's just add the template topic's ID to the array that's tracking them.
		arynDeletedTemplateTopicIDs.Add(m_nID);
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrTemplateTopicsT WHERE ID = %li", m_nID);

		//only audit if the EMN is not new
		if(GetParentEMN()->GetID() != -1) {
			//auditing (using transactions)
			if(nAuditTransactionID == -1)
				nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(-1, "", nAuditTransactionID, aeiEMNTemplateTopicRemoved, GetParentEMN()->GetID(), m_strTopicName, "<Deleted>", aepHigh, aetDeleted);
		}
	}
	else {

		//don't need to delete it if the item doesn't exist,
		//but this does need to be after the previous code so we can clean out the child lists
		if(m_nID == -1)
			return strDeleteString;

		if(bIsLockedAndSaved) {
			//this is locked! add a warning and return
			CString str;
			str.Format("Attempted to delete topic '%s' from a locked EMN.", m_strTopicName);
			arystrErrors.Add(str);
			return strDeleteString;
		}

		// (j.jones 2006-02-21 13:54) - I believe PLID 19341 made this obsolete		
		/*
		if(strChildIDs.IsEmpty())
			strChildIDs = "-1";
		else
			strChildIDs.TrimRight(",");
		_RecordsetPtr rsOtherChildren = CreateRecordset("SELECT ID FROM EmrTopicsT WHERE EmrParentTopicID = %li AND ID NOT IN (%s)", m_nID, strChildIDs);
		while(!rsOtherChildren->eof) {
			//Create an object.
			CEMRTopic *pTopic = new CEMRTopic(m_bIsTemplate);
			pTopic->LoadFromTopicID(AdoFldLong(rsOtherChildren,"ID"));
			strDeleteString += pTopic->GenerateDeleteString(nAuditTransactionID);
			rsOtherChildren->MoveNext();
		}
		rsOtherChildren->Close();
		*/

		// (j.jones 2006-04-26 09:37) - PLID 20064 - we now simply mark these records as being deleted
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrTopicsT WHERE ID = %li", m_nID);
		AddStatementToSqlBatch(strDeleteString, "UPDATE EmrTopicsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE ID = %li", _Q(GetCurrentUserName()), m_nID);
		// (c.haag 2008-07-28 09:37) - PLID 30853 - Delete all problems linked directly with the topic. Note that we don't do this for details
		// as well; this is because the details themselves are not marked as deleted.
		// (c.haag 2009-05-11 17:38) - PLID 28494 - Problem-regarding information now goes into its own table
		// (j.jones 2009-06-02 12:14) - PLID 34301 - only delete links right now, the parent should know to delete problems
		AddStatementToSqlBatch(strDeleteString, "DELETE FROM EMRProblemLinkT WHERE EMRRegardingType = %li AND EMRRegardingID = %li", eprtEmrTopic, m_nID);

		//only audit if the EMN is not new
		if(GetParentEMN()) {
			if(GetParentEMN()->GetID() != -1) {
				//auditing (using transactions)
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(GetParentEMN()->GetParentEMR()->GetPatientID(), GetExistingPatientName(GetParentEMN()->GetParentEMR()->GetPatientID()),
					nAuditTransactionID, aeiEMNTopicRemoved, GetParentEMN()->GetID(), m_strTopicName, "<Deleted>", aepHigh, aetDeleted);
			}
		}
	}
	return strDeleteString;
}

long CEMRTopic::GetTemplateTopicID(BOOL bEnsureVariable /*= TRUE*/)
{
	if(bEnsureVariable) ENSURE_VARIABLE(m_n, TemplateTopicID);
	return m_nTemplateTopicID;
}

long CEMRTopic::GetTemplateID()
{
	ENSURE_VARIABLE(m_n, TemplateID);
	return m_nTemplateID;
}

long CEMRTopic::GetTemplateTopicOrderIndex()
{
	ENSURE_VARIABLE(m_n, TemplateTopicOrderIndex);
	return m_nTemplateTopicOrderIndex;
}

// (c.haag 2007-05-22 12:01) - PLID 26095 - Added the option to do reference counting
// (c.haag 2009-04-06 09:46) - PLID 33859 - Added bSyncSystemTableData (only used when popping up an item)
void CEMRTopic::AddDetail(CEMNDetail *pDetail, BOOL bIsInitialLoad /*= FALSE*/, BOOL bAddRef /*= TRUE*/, BOOL bSyncSystemTableData /*= TRUE */)
{	
	EMRLOGINDENT(1,"CEMRTopic::AddDetail called for topic \"%s\" (ID=%d m_nTemplateTopicID=%d). pDetail = (0x%08x)", m_strTopicName, m_nID, m_nTemplateTopicID, pDetail); // (c.haag 2010-05-19 9:04) - PLID 38759

	pDetail->m_pParentTopic = this;

	//this function is used both to originally add a detail to a topic,
	//and when moving, so only update m_pOriginalParentTopic once
	if(pDetail->m_pOriginalParentTopic == NULL) {
		pDetail->m_pOriginalParentTopic = this;
		pDetail->m_strLastSavedParentTopicName = m_strTopicName;
	}

	//update problems
	// (c.haag 2009-05-16 13:35) - PLID 34311 - Use problem link structure
	for(int i=0; i<pDetail->m_apEmrProblemLinks.GetSize(); i++) {
		CEmrProblemLink *pLink = pDetail->m_apEmrProblemLinks.GetAt(i);
		pLink->UpdatePointersWithDetail(pDetail);
	}

	// (j.jones 2010-02-25 14:40) - PLID 37318 - ensure the SmartStamp links, if any, remain valid
	pDetail->ReconfirmSmartStampLinks();

	AddDetail_Flat(pDetail, bAddRef);
	
	// (a.walling 2011-01-26 14:06) - PLID 42058 - Unfortunately, adding a detail could completely change what a narrative displays
	// since we do not strongly link a detail to a narrative (rather we do so by name). So we have to go through and ensure the cache
	// has been invalidated.
	if (!bIsInitialLoad) {
		CEMN* pEMN = GetParentEMN();
		long nTotalDetails = GetParentEMN()->GetTotalDetailCount();
		for (int x = 0; x < nTotalDetails; x++) {
			CEMNDetail* pPossibleNarrative = pEMN->GetDetail(x);

			if (pPossibleNarrative->m_EMRInfoType == eitNarrative) {
				pPossibleNarrative->InvalidateLinkedDetailCache();
			}
		}
	}

	//Make sure our interface is up to date.
	CWnd *pWnd = GetParentEMN()->GetInterface();
	if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
		// (a.walling 2007-04-10 09:42) - PLID 25549 - send initial load
		pWnd->SendMessage(NXM_POST_ADD_EMR_ITEM, (WPARAM)pDetail, (LPARAM)bIsInitialLoad);
	}
	if(!bIsInitialLoad) {
		//If this is the initial load, then CEMN::PostLoad() will update narratives and merge conflicts efficiently.
		//TES 1/23/2008 - PLID 24157 - Renamed.
		GetParentEMN()->HandleDetailChange(pDetail);
		GetParentEMN()->UpdateMergeConflicts(pDetail->GetMergeFieldName(TRUE));

		m_bUnsaved = TRUE;
	}	

	if(pDetail->GetMergeNameConflict() && pDetail->GetMergeFieldOverride().IsEmpty()) {
		//TES 12/4/2006 - PLID 22304 - We just loaded a detail that conflicts, immediately try and override the name, so that
		// data isn't being modified at unexpected times.
		GetParentEMN()->TryToOverrideMergeField(pDetail);
	}

	if(pDetail->GetVisible()) {
		//If we thought we didn't have details before, we do now.
		m_bHasDetails = TRUE;
		
		//Also, our completion status may have changed.
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		if (m_CompletionStatus == etcsRequired) {
			// The topic state is already "required but not filled" due to some other detail, so our incoming state won't 
			// be able to affect it no matter what.
		}
		else if (pDetail->IsRequired() && !pDetail->IsStateSet()) {
			// Incoming detail is required to be filled, and its state is not set, so our entire topic, which wasn't in a 
			// "required but not filled" state before, now is.
			m_CompletionStatus = etcsRequired;
		}
		else if(m_CompletionStatus == etcsIncomplete || m_CompletionStatus == etcsInvalid) {
			//TES 1/24/2007 - PLID 24377 - If our status is etcsInvalid, that must mean that we haven't had any details added
			// yet, so we can treat it as equivalent to etcsIncomplete.
			if(pDetail->IsStateSet()) {
				//Do we have any other details?
				BOOL bOtherDetailFound = FALSE;
				for(int i = 0; i < m_arypEMNDetails.GetSize() && !bOtherDetailFound; i++) {
					if(m_arypEMNDetails[i] != pDetail && m_arypEMNDetails[i]->GetVisible()) bOtherDetailFound = TRUE;
				}
				if(bOtherDetailFound) {
					m_CompletionStatus = etcsPartiallyComplete;
				}
				else {
					m_CompletionStatus = etcsComplete;
				}
			}
		}
		else if(m_CompletionStatus == etcsComplete) {
			if(!pDetail->IsStateSet()) {
				m_CompletionStatus = etcsPartiallyComplete;
			}
		}
		// (z.manning, 04/04/2008) - PLID 29495 - Added status for topics with no details
		else if(m_CompletionStatus == etcsBlank) {
			if(pDetail->IsStateSet()) {
				m_CompletionStatus = etcsComplete;
			}
			else {
				m_CompletionStatus = etcsIncomplete;
			}
		}
		else {
			//m_CompletionStatus must be etcsPartiallyComplete, so it doesn't need to change.
		}
	}

	//TES 6/6/2008 - PLID 29416 - If this is the current medications detail, add in the official current medications.
	// (c.haag 2009-04-06 09:49) - PLID 33859 - Only attempt if bSyncSystemTableData is TRUE. If it's FALSE, it's probably
	// because we're popping up a detail.
	if (!bIsInitialLoad && pDetail->IsCurrentMedicationsTable() && bSyncSystemTableData) {
		// (c.haag 2007-02-08 12:17) - PLID 24376 - If we are pulling an official Current Medications detail,
		// then we must calculate the state from the EMN itself. Similar code is run in these three places:
		//
		// CEMNDetail::LoadFromInfoID()				- When adding a Current Medications detail to a topic manually
		// CEMNDetail::LoadFromTemplateDetailID()	- When adding a Current Medications detail from a template
		// CEMRTopic::PostLoadDetails()				- When loading a patient EMN from a template
		// LoadEmrTopic								- When loading detail states in a thread
		// CEMRTopic::AddDetail()					- The rest of the time (failsafe)
		//
		// When loading Current Medication info items into templates, we should not do any special calculating. It
		// should behave exactly as it did before.
		//
		if (!pDetail->m_bIsTemplateDetail) {

			// (j.jones 2007-07-24 09:27) - PLID 26742 - the medications info ID is cached in CEMR
			long nActiveCurrentMedicationsInfoID = -2;
			//do memory checks
			if(GetParentEMN()) {
				if(GetParentEMN()->GetParentEMR()) {
					nActiveCurrentMedicationsInfoID = GetParentEMN()->GetParentEMR()->GetCurrentMedicationsInfoID();
				}
			}

			if(nActiveCurrentMedicationsInfoID == -2) {
				//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
				//but why don't we have an EMR?
				ASSERT(FALSE);
				nActiveCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
			}

			// (j.jones 2009-10-05 11:33) - PLID 35735 - do not sync if the detail is not new
			bool bSyncWithCurrentMedications = (nActiveCurrentMedicationsInfoID == pDetail->m_nEMRInfoID && pDetail->GetID() == -1);
			if (bSyncWithCurrentMedications) {
				//TES 6/5/2008 - PLID 29416 - Rather than using the "official" current medications state (which never really
				// existed), just merge in the Medications tab data (which only affects one
				// column in the table).
				GetParentEMN()->ApplyOfficialCurrentMedications(pDetail, pDetail->m_nEMRInfoID);
				pDetail->RequestStateChange(pDetail->GetState());
			}
		}
	}
	//TES 6/6/2008 - PLID 29416 - If this is the allergies detail, add in the official allergies.
	// (c.haag 2009-04-06 09:49) - PLID 33859 - Only attempt if bSyncSystemTableData is TRUE. If it's FALSE, it's probably
	// because we're popping up a detail.
	else if (!bIsInitialLoad && pDetail->IsAllergiesTable() && bSyncSystemTableData) {
		// (c.haag 2007-02-08 12:17) - PLID 24376 - If we are pulling an official Allergies detail,

		if (!pDetail->m_bIsTemplateDetail) {
			// (j.jones 2007-07-24 09:27) - PLID 26742 - the Allergy Info ID is cached in CEMR
			long nActiveCurrentAllergiesInfoID = -2;
			//do memory checks
			if(GetParentEMN()) {
				if(GetParentEMN()->GetParentEMR()) {
					nActiveCurrentAllergiesInfoID = GetParentEMN()->GetParentEMR()->GetCurrentAllergiesInfoID();
				}
			}

			if(nActiveCurrentAllergiesInfoID == -2) {
				//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
				//but why don't we have an EMR?
				ASSERT(FALSE);
				nActiveCurrentAllergiesInfoID = GetActiveAllergiesInfoID();
			}

			// (j.jones 2009-10-05 11:33) - PLID 35735 - do not sync if the detail is not new
			bool bSyncWithAllergies = (nActiveCurrentAllergiesInfoID == pDetail->m_nEMRInfoID && pDetail->GetID() == -1);
			if (bSyncWithAllergies) {
				//TES 6/5/2008 - PLID 29416 - Rather than using the "official" allergies state (which never really
				// existed), just merge in the Medications tab data (which only affects one
				// column in the table).
				GetParentEMN()->ApplyOfficialAllergies(pDetail, pDetail->m_nEMRInfoID);
				pDetail->RequestStateChange(pDetail->GetState());
			}
		}
	}
	EMRLOGINDENT(-1,"CEMRTopic::AddDetail END"); // (c.haag 2010-05-19 9:04) - PLID 38759
}

// (c.haag 2007-08-06 15:30) - PLID 26992 - Added an optional parameter for a CEMNSpawner object to avoid redundant querying in LoadContent
// (z.manning 2009-02-13 10:58) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
// (z.manning 2009-03-05 09:42) - PLID 33338 - Use the new source action info class
// (j.jones 2010-02-22 17:13) - PLID 37318 - added pParentSmartStampImage, given only when this is called from CEMN::PostSmartStampImageAdded
CEMNDetail* CEMRTopic::AddNewDetailFromEmrInfoMasterID(long nInfoMasterID, BOOL bIsTemplate, SourceActionInfo &sai, BOOL bIsInitialLoad /*= FALSE*/, OPTIONAL IN long nSourceActionSourceID /*= -1*/, CEMNSpawner* pEMNSpawner /*= NULL */, long nSourceActionSourceDataGroupID /*= -1*/, OPTIONAL IN long nSourceActionSourceHotSpotGroupID /* = -1 */, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID /* = -1 */, CEMNDetail *pParentSmartStampImage /*= NULL*/)
{
	//Create the new detail
	// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
	CEMNDetail *pDetail = CEMNDetail::CreateDetail(this, "EMRTopic AddNewDetailFromEmrInfoMasterID");

	pDetail->m_pOriginalParentTopic = this;
	pDetail->m_strLastSavedParentTopicName = m_strTopicName;

	// (j.jones 2010-02-25 14:40) - PLID 37318 - ensure the SmartStamp links, if any, remain valid
	pDetail->ReconfirmSmartStampLinks();

	//In this case, we want to add the new detail to the topic, then load the data.
	m_arypEMNDetails.Add(pDetail);

	// (j.jones 2010-02-22 17:20) - PLID 37318 - if given a pParentSmartStampImage,
	// then this must be a SmartStamp table, so link the two
	if(pParentSmartStampImage) {
		pParentSmartStampImage->SetSmartStampTableDetail(pDetail);
		pParentSmartStampImage->SetUnsaved();
		pDetail->EnsureSmartStampImageDetail(pParentSmartStampImage);
	}

	//DRT 8/2/2007 - PLID 26919 - Added SourceActionSourceID
	//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
	//TES 10/8/2007 - PLID 27660 - Pass bIsInitialLoad into this function, since it can call ProcessEmrActions().
	// (z.manning 2009-02-13 10:59) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	pDetail->LoadFromInfoMasterID(nInfoMasterID, m_bIsTemplate, bIsInitialLoad, sai, nSourceActionSourceID, pEMNSpawner, nSourceActionSourceDataGroupID, nSourceActionSourceHotSpotGroupID, nSourceActionSourceTableDropdownGroupID);
	//Now that the detail has been added and loaded, we can update the narratives.
	//TES 1/23/2008 - PLID 24157 - Renamed.
	GetParentEMN()->HandleDetailChange(pDetail);
	GetParentEMN()->UpdateMergeConflicts(pDetail->GetMergeFieldName(TRUE));
	if(!bIsInitialLoad) {
		m_bUnsaved = TRUE;
		// (j.jones 2010-03-03 10:39) - PLID 37231 - ensure this is "new",
		// we may have remembered content from a prior item that needs
		// to be reset as new
		if((m_bIsTemplate && pDetail->m_nEMRTemplateDetailID == -1)
			|| (!m_bIsTemplate && pDetail->m_nEMRDetailID == -1)) {
			pDetail->SetNew();
		}
	}

	if(pDetail->GetVisible()) {
		//If we thought we didn't have details before, we do now.
		m_bHasDetails = TRUE;
		
		//Also, our completion status may have changed.
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		if (m_CompletionStatus == etcsRequired) {
			// The topic state is already "required but not filled" due to some other detail, so our incoming state won't 
			// be able to affect it no matter what.
		}
		else if (pDetail->IsRequired() && !pDetail->IsStateSet()) {
			// Incoming detail is required to be filled, and its state is not set, so our entire topic, which wasn't in a 
			// "required but not filled" state before, now is.
			m_CompletionStatus = etcsRequired;
		}
		else if(m_CompletionStatus == etcsIncomplete || m_CompletionStatus == etcsInvalid) {
			//TES 1/24/2007 - PLID 24377 - If our status is etcsInvalid, that must mean that we haven't had any details added
			// yet, so we can treat it as equivalent to etcsIncomplete.
			if(pDetail->IsStateSet()) {
				//Do we have any other details?
				BOOL bOtherDetailFound = FALSE;
				for(int i = 0; i < m_arypEMNDetails.GetSize() && !bOtherDetailFound; i++) {
					if(m_arypEMNDetails[i] != pDetail && m_arypEMNDetails[i]->GetVisible()) bOtherDetailFound = TRUE;
				}
				if(bOtherDetailFound) {
					m_CompletionStatus = etcsPartiallyComplete;
				}
				else {
					m_CompletionStatus = etcsComplete;
				}
			}
		}
		else if(m_CompletionStatus == etcsComplete) {
			if(!pDetail->IsStateSet()) {
				m_CompletionStatus = etcsPartiallyComplete;
			}
		}
		// (z.manning, 04/04/2008) - PLID 29495 - Added status for topics with no details
		else if(m_CompletionStatus == etcsBlank) {
			if(pDetail->IsStateSet()) {
				m_CompletionStatus = etcsComplete;
			}
			else {
				m_CompletionStatus = etcsIncomplete;
			}
		}
		else {
			//m_CompletionStatus must be etcsPartiallyComplete, so it doesn't need to change.
		}
	}

	//Return the pointer to the new detail we just added.
	return pDetail;
}

	// (c.haag 2007-05-22 12:01) - PLID 26095 - Added the option to do reference counting
void CEMRTopic::AddDetail_Flat(CEMNDetail *pDetail, BOOL bAddRef /*= TRUE*/)
{
	m_arypEMNDetails.Add(pDetail);
	// (c.haag 2007-05-22 12:02) - PLID 26095 - Add a reference to the detail if necessary
	if (bAddRef) {
		if (pDetail) {
			//pDetail->AddRef();
			// (a.walling 2009-10-12 16:05) - PLID 36024
			pDetail->__AddRef("CEMRTopic::AddDetail_Flat");
		} else {
			ASSERT(FALSE); // We should never get here
		}
	}
}

//DRT 7/30/2007 - PLID 26876 - Provide a way to mass remove everything.  Previous behavior was to call RemoveDetail for each 
//	detail on the topic, which is insanely slow.
//The design of this function is:
//	Run all the overhead that needs to be done first.  Lock narratives, ensure loading is done, etc.
//	Loop over all details and remove them.
//	Remove all subtopics.
//	Run all the overhead that needs to be done last.  Updating narratives, ensuring that member variables are update to the
//		latest changes, etc.
void CEMRTopic::RemoveThisTopicAndSubTopics()
{
	//This code is basically RemoveDetail(), except pulled out so that the things that only need done once are done at the beginning/end, 
	//	instead of happening repeatedly.

	//1)  Ensure that all the details on this topic are loaded.  This forces the preloader to finish loading this topic, if it hasn't yet.
	EnsureDetails();

	//2)  DRT 7/30/2007 - PLID 26876 - Do not allow this topic to be unspawned by one of the details on it.  I pulled this out of the detail removal loop, it only 
	//	needs done once.
	//TES 3/31/2006 - First, make sure that this doesn't result in unspawning ourself.
	m_bAllowUnspawn = FALSE;

	//3)  DRT 7/30/2007 - PLID 26876 - This need only happen once, before we loop over all details to be removed.
	//TES 1/30/2007 - PLID 24484 - We don't want to be updating all the narratives for every single action
	// we process, so lock the updating here, we'll unlock it once we're done.
	//TES 1/23/2008 - PLID 24157 - Renamed.
	GetParentEMN()->LockHandlingDetailChanges();


	//4)  Loop over all the details in this topic.  This code was pulled partially out of CEMN::RemoveTopic, and partially out of
	//	CEMRTopic::RemoveDetail
	for(int j = GetEMNDetailCount() - 1; j >= 0 ; j--) {
		CEMNDetail *pDetail = GetDetailByIndex(j);

		// (j.jones 2006-02-27 15:24) - massive unspawning may have already removed this item!
		if(!pDetail)
			continue;

		RemoveDetail_DetailSpecific(pDetail);

	}//end for loop of all details

	//5)  DRT 7/30/2007 - PLID 26876 - Before we finalize the stuff needed for our removal, remove all the subtopics.
	while(GetSubTopicCount()) {
		RemoveSubTopic(GetSubTopic(0));
	}

	//DRT 7/30/2007 - PLID 26876 - I pulled this out of the detail removal loop.  It only needs executed once, after
	//	all removing is done.
	// (a.walling 2007-04-12 08:56) - PLID 25599 - Ensure that m_bHasDetails is up to date.
	if (m_arypEMNDetails.GetSize() == 0)
		m_bHasDetails = FALSE;

	//DRT 7/30/2007 - PLID 26876 - Pulled out of the detail removal loop.  This need only happen once for the entire topic
	//	removal process.
	GetParentEMN()->InvalidateAllDetailMergeButtons();

	//DRT 7/30/2007 - PLID 26876 - We can be unspawned again now.  I pulled this out of the detail removal loop.
	m_bAllowUnspawn = TRUE;

	//DRT 7/30/2007 - PLID 26876 - The topic has changed and is now unsaved.  I pulled this out of the detail removal loop.
	m_bUnsaved = TRUE;

	//DRT 7/30/2007 - PLID 26876 - Pulled this out of the detail removal loop, it need happen only once after all details are removed.
	//TES 1/30/2007 - PLID 24484 - Now, update all our narratives.
	//TES 1/23/2008 - PLID 24157 - Renamed.
	GetParentEMN()->UnlockHandlingDetailChanges();

	//DRT 7/30/2007 - PLID 26876 - Set the completion status to empty, as we have no details
	// (z.manning, 04/04/2008) - PLID 29495 - We now have a status specifically for topics with no details
	m_CompletionStatus = etcsBlank;

	// (a.walling 2008-02-06 14:33) - PLID 28391 - Refresh HTML visibility state
	RefreshHTMLVisibility();
}

//DRT 7/30/2007 - PLID 26876 - This function is the "guts" of RemoveDetail.  Each of the things here must be done specifically
//	for each detail that is being removed.  This function does nothing that requires overhead, such as updating narratives, etc.
//	This function is designed so that it can be called repeatedly in a mass deletion, such as removing a topic entirely, or when
//	unspawning a number of details.
void CEMRTopic::RemoveDetail_DetailSpecific(CEMNDetail *pDetail)
{
	// (z.manning 2011-01-27 16:58) - PLID 42335 - Before we remove this detail, let's request a state change
	// if it's a smart stamp image because if the linked table still has other images then it will not be removed but
	// we need to update the table to reflect any necessary changes based on the removal of this image.
	if(pDetail->IsSmartStampImage()) {
		pDetail->HandleImageStateChange(::LoadEMRDetailStateBlank(eitImage), pDetail->GetState(), FALSE);
	}

	//Tell the interface we're about to remove each detail.
	//DRT 7/30/2007 - PLID 26876 - This used to be needed to be called by the interface before each call to RemoveDetail.  We're
	//	now calling it only in here, I removed it everyplace else.
	//DRT 8/2/2007 - Important change:  We don't want to send a message to the topic wnd (it's not even created until someone visits
	//	this topic), we want to send a message to the tree window.  We can safely access that via the EMNs interface wnd.
	CWnd* pTreeWnd = GetParentEMN()->GetInterface();
	if(pTreeWnd) {
		pTreeWnd->SendMessage(NXM_PRE_DELETE_EMR_ITEM, (WPARAM)pDetail);
	}

	//DRT 7/30/2007 - PLID 26876 - Previously this was part of RemoveDetail().  Now, we take just the important parts of RemoveDetail tha
	//	are not "overhead", so that this function may be called in batch.  Mostly it's so we don't need to update narratives once for
	//	each detail removed, we can just remove all details then update narratives.

	//Everything in brackets was taken out of RemoveDetail
	{
		// (a.wetta 2007-01-31 09:18) - PLID 24369 - Let's make sure that while we are in the process of 
		// removing the detail, the pointer to it doesn't get deleted by something, namely the 
		// CEMRTopic::GenerateSaveString() function which can sometimes be called during the RemoveDetail
		// function and mess things up.
		GetParentEMN()->GetParentEMR()->AddEMNDetailReference(pDetail);

		// (j.jones 2010-02-17 10:00) - PLID 37397 - if we're linked to an image or a table,
		// remove that image/table as well
		// (z.manning 2011-01-21 09:48) - PLID 42338 - Support multiple images per smart stamp table
		for(int nImageIndex = 0; nImageIndex < pDetail->GetSmartStampImageDetails()->GetSize(); nImageIndex++)
		{
			CEMNDetail *pImage = pDetail->GetSmartStampImageDetails()->GetAt(nImageIndex);
			//remove its reference to this table detail
			pImage->SetSmartStampTableDetail(NULL);

			//remove the image
			CEMRTopic *pTopic = pImage->m_pParentTopic;
			if(pTopic) {
				pTopic->RemoveDetail(pImage);
				//Make sure that if the detail is on our popped-up items dialog, that it gets taken off.
				pTopic->GetParentEMN()->RemoveDetailFromPopup(pImage);
				
				CWnd *pWnd = pTopic->GetParentEMN()->GetInterface();
				if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
					pWnd->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)pTopic);
				}
			}
		}

		//clear our reference to the image
		pDetail->ClearSmartStampImageDetails();

		if(pDetail->GetSmartStampTableDetail())
		{
			CEMNDetail *pTable = pDetail->GetSmartStampTableDetail();
			//remove its reference to this image detail
			pTable->ClearSmartStampImageDetailByEmrInfoMasterID(pDetail->m_nEMRInfoMasterID);
			
			//remove the table
			// (z.manning 2011-01-26 11:05) - PLID 42336 - Only remove the table if this was its only linked image
			if(pTable->GetSmartStampImageDetails()->GetCount() == 0)
			{
				CEMRTopic *pTopic = pTable->m_pParentTopic;
				if(pTopic) {
					pTopic->RemoveDetail(pTable);
					//Make sure that if the detail is on our popped-up items dialog, that it gets taken off.
					pTopic->GetParentEMN()->RemoveDetailFromPopup(pTable);
					
					CWnd *pWnd = pTopic->GetParentEMN()->GetInterface();
					if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
						pWnd->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)pTopic);
					}
				}
			}

			//clear our reference to the table
			pDetail->SetSmartStampTableDetail(NULL);
		}

		// (a.walling 2009-10-12 17:45) - PLID 36024
		pDetail->__AddRef("RemoveDetail_DetailSpecific");
		m_aryDeletedDetails.Add(pDetail);
		pDetail->HideDetailDlg();
		//We have to remove this detail from the topic's array of EMN details
		for(int nEMNDetailIdx = 0; nEMNDetailIdx < m_arypEMNDetails.GetSize(); nEMNDetailIdx++) {
			if(m_arypEMNDetails.GetAt(nEMNDetailIdx) == pDetail) {
				m_arypEMNDetails.RemoveAt(nEMNDetailIdx);
				
				pDetail->__Release("RemoveDetail_DetailSpecific releasing from topic details");

				//DRT 8/3/2007 - PLID 26928 - Just in case the user forgot, make sure this isn't in our "pending" array anymore.
				RemoveDetailFromPendingDeletion(pDetail);

				break;
			}
		}

		//Now, revoke any actions.
/*	DRT 8/3/2007 - PLID 26937 - This is no longer needed here.  Anywhere that wants to revoke any actions will
		use the CEMNUnspawner, which will recursively find all details to be unspawned.
		// (j.jones 2007-07-27 10:29) - PLID 26845 - check whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them
		if(pDetail->GetHasInfoActionsStatus() != ehiasHasNoInfoItems) {
			GetParentEMN()->GetParentEMR()->RevokeEMRInfoActions(pDetail);
		}

		// (z.manning, 3/2/2006, PLID 19099) - A spawned item in a template may exist in data
		// even if that list item that spawned is not currently checked, so we need to check all list items.
		if(m_bIsTemplate) {
			long nListElementCount = pDetail->GetListElementCount();
			for(int i = 0; i < nListElementCount; i++) {
				//DRT 7/31/2007 - PLID 26885 - the ListElement class has a member for "what kind of actions are spawned?", and
				//	one possibility is "nothing".  We should take that to heart before calling RevokeEMRActions, which will run
				//	from 1 - 5 queries depending on the type of action.  Also note.  On templates, spawned actions of charges, 
				//	diag codes, medications, procedures, and mints DO NOTHING.  So those never need revoked, as they never did
				//	anything in the first place.  Therefore, we need only check for case 1.
				ListElement le = pDetail->GetListElement(i);
				if(le.nActionsType == 1) {
					GetParentEMN()->GetParentEMR()->RevokeEMRDataActions(le.nID, pDetail);
				}
			}
		}
		else {
			CDWordArray arIDsToRevoke;
			pDetail->GetSelectedValues(arIDsToRevoke);
			for(int i = 0; i < arIDsToRevoke.GetSize(); i++) {
				GetParentEMN()->GetParentEMR()->RevokeEMRDataActions(arIDsToRevoke[i], pDetail);
			}
		}
*/
		//TES 1/23/2008 - PLID 24157 - Renamed.
		GetParentEMN()->HandleDetailChange(pDetail, TRUE);
		GetParentEMN()->UpdateMergeConflicts(pDetail->GetMergeFieldName(TRUE), pDetail);

		// (c.haag 2007-10-30 17:24) - PLID 27914 - Assert that this detail does not exist in any
		// cached linked detail arrays for any detail. If it does, the cache is invalidated so that
		// it is far less likely for it to be left with a dangling pointer if other code does not
		// update the cache like it should.
		//
		// The motivation for this code is that we need a final line of defense from linked cache details
		// having dangling pointers. Unfortunately, this is not a 100% guarantee because, later on, some
		// function may add pDetail back to the cache before it's deleted from memory.
		{
			CEMN* pEMN = GetParentEMN();
			if (NULL != pEMN && !pEMN->GetInDestructor()) {
				const long nTotalDetailCount = pEMN->GetTotalDetailCount();
				for (long i=0; i < nTotalDetailCount; i++) {
					CEMNDetail* p = pEMN->GetDetail(i);
					// Don't fire an assertion because this may actually be legitimate, and
					// a successive call to UpdateNarrativesAndLinkedTables where the lock
					// count is zero will properly rebuild the cache
					p->InvalidateLinkedDetailCacheIfExists(pDetail, FALSE);
				}
			}
		}

		// (a.wetta 2007-01-31 09:20) - PLID 24369 - Ok, we're done with the detail's pointer, it's safe for 
		// it to be deleted.
		GetParentEMN()->GetParentEMR()->RemoveEMNDetailReference(pDetail);

		// (c.haag 2009-07-02 09:08) - PLID 34760 - Ensure this detail does not exist in the multi-popup window
		GetParentEMN()->RemoveDetailFromPopup(pDetail);
	}
}

void CEMRTopic::RemoveDetail(CEMNDetail *pDetail)
{
	EnsureDetails();

	//We'll also need to update our cached completion status.
	BOOL bNonEmptyFound = FALSE;
	BOOL bEmptyFound = FALSE;
	BOOL bTargetFound = FALSE;
	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	BOOL bFoundRequiredUnfilled = FALSE;
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pDetailToCheck = m_arypEMNDetails[i];
		if(pDetailToCheck == pDetail) {

			//DRT 7/30/2007 - PLID 26876 - I separated the "guts" out of this function, for things that are not overhead when removing
			//	multiple details from the topic.
			//Overhead -- Pre-removal

			//TES 3/31/2006 - First, make sure that this doesn't result in unspawning ourself.
			m_bAllowUnspawn = FALSE;

			//TES 1/30/2007 - PLID 24484 - We don't want to be updating all the narratives for every single action
			// we process, so lock the updating here, we'll unlock it once we're done.
			//TES 1/23/2008 - PLID 24157 - Renamed.
			GetParentEMN()->LockHandlingDetailChanges();

			//DRT 7/30/2007 - PLID 26876 - Now do the "guts" portion, where we remove the detail.
			RemoveDetail_DetailSpecific(pDetail);

			//DRT 7/30/2007 - PLID 26876 - This is everything that must be done afterward to ensure the topic is cleaned up
			//Overhead - Post-removal

			// (a.walling 2007-04-12 08:56) - PLID 25599 - Ensure that m_bHasDetails is up to date.
			if (m_arypEMNDetails.GetSize() == 0)
				m_bHasDetails = FALSE;

			GetParentEMN()->InvalidateAllDetailMergeButtons();

			//We can be unspawned again now.
			m_bAllowUnspawn = TRUE;

			m_bUnsaved = TRUE;

			//TES 1/30/2007 - PLID 24484 - Now, update all our narratives.
			//TES 1/23/2008 - PLID 24157 - Renamed.
			GetParentEMN()->UnlockHandlingDetailChanges();

			//Don't return, we have to finish updating our completion status.
			//return;
			
			// (b.cardillo 2012-03-08 09:04) - PLID 48715 - Since we just removed a detail from the array we're 
			// looping through, we need to pre-decrement i (it's about to be incremented) so that we don't skip 
			// over the NEXT entry in the array.
			i--;

			bTargetFound = TRUE;
		}
		else if(m_arypEMNDetails[i]->GetVisible()) {
			if (m_arypEMNDetails[i]->IsStateSet()) {
				bNonEmptyFound = TRUE;
			} else {
				bEmptyFound = TRUE;
				// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
				if (m_arypEMNDetails[i]->IsRequired()) {
					bFoundRequiredUnfilled = TRUE;
				}
			}
			if (bTargetFound) {
				// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
				if (bFoundRequiredUnfilled) {
					// Nothing will supercede this status, so no need to keep looping
					// (b.cardillo 2012-03-08 11:14) - PLID 48717 - Break out of the loop instead of returning out 
					// of the whole function.  This ensures the RefreshHTMLVisibility() and any other future code 
					// outside of this loop will always get called.
					break;
				} else {
					// Keep looping
				}
			}
		}
	}

	// (z.manning, 04/04/2008) - PLID 29495 - Now have a status for topics with no details
	if(m_arypEMNDetails.GetSize() == 0) {
		m_CompletionStatus = etcsBlank;
	}
	else if (bFoundRequiredUnfilled) {
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		m_CompletionStatus = etcsRequired;
	}
	else if(bNonEmptyFound) {
		if(bEmptyFound) {
			m_CompletionStatus = etcsPartiallyComplete;
		}
		else {
			m_CompletionStatus = etcsComplete;
		}
	}
	else {
		m_CompletionStatus = etcsIncomplete;
	}

	// (a.walling 2008-02-06 14:33) - PLID 28391 - Refresh HTML visibility state
	RefreshHTMLVisibility();
}

long CEMRTopic::GetSourceActionID()
{
	// (c.haag 2007-05-23 16:17) - PLID 26115 - If we're in the middle of the initial load, we
	// cannot rely on this object's m_nSourceActionID value. We instead have to refer to the
	// preloader to get the value.
	//
	// In case you're wondering why: The preloader gathers all topic variables before the asynchronous
	// load even starts. Ultimately, the topic's source action ID will match that of the preloaded topic.
	// The reason we did not set it already is to be as consistent with the historic design as possible.
	//
	if (!m_bPostLoadCalled && NULL != m_pLoadInfo && NULL != m_pLoadInfo->m_pLoader) {
		// (c.haag 2007-07-03 10:10) - PLID 26523 - Request access to topic information for
		// thread safety
		CHoldEMNLoaderMutex mh(m_pLoadInfo->m_pLoader->GetTopicsMutex());
		if (m_pLoadInfo->bLoadFromTemplate) {
			CEMNLoader::CPreloadedTemplateTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTemplateTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionID, -1);
		} else {
			CEMNLoader::CPreloadedTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionID, -1);
		}
	}

	//TES 1/30/2007 - PLID 24455 - Make sure the variable has been loaded, don't return a bad value.
	ENSURE_VARIABLE(m_n, SourceActionID);
	return m_sai.nSourceActionID;
}

long CEMRTopic::GetSourceActionSourceID()
{
	//DRT 9/25/2007 - PLID 27515 - Basically a copy of GetSourceActionID, with a different variable


	// (c.haag 2007-05-23 16:17) - PLID 26115 - If we're in the middle of the initial load, we
	// cannot rely on this object's m_nSourceActionSourceID value. We instead have to refer to the
	// preloader to get the value.
	//
	// In case you're wondering why: The preloader gathers all topic variables before the asynchronous
	// load even starts. Ultimately, the topic's source action ID will match that of the preloaded topic.
	// The reason we did not set it already is to be as consistent with the historic design as possible.
	//
	if (!m_bPostLoadCalled && NULL != m_pLoadInfo && NULL != m_pLoadInfo->m_pLoader) {
		// (c.haag 2007-07-03 10:10) - PLID 26523 - Request access to topic information for
		// thread safety
		CHoldEMNLoaderMutex mh(m_pLoadInfo->m_pLoader->GetTopicsMutex());
		if (m_pLoadInfo->bLoadFromTemplate) {
			CEMNLoader::CPreloadedTemplateTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTemplateTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionSourceID, -1);
		} else {
			CEMNLoader::CPreloadedTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionSourceID, -1);
		}
	}

	//TES 1/30/2007 - PLID 24455 - Make sure the variable has been loaded, don't return a bad value.
	ENSURE_VARIABLE(m_n, SourceActionSourceID);
	return m_nSourceActionSourceID;
}

long CEMRTopic::GetSourceActionSourceDataGroupID()
{
	//DRT 9/25/2007 - PLID 27515 - Basically a copy of GetSourceActionID, with a different variable



	// (c.haag 2007-05-23 16:17) - PLID 26115 - If we're in the middle of the initial load, we
	// cannot rely on this object's m_nSourceActionSourceDataGroupID value. We instead have to refer to the
	// preloader to get the value.
	//
	// In case you're wondering why: The preloader gathers all topic variables before the asynchronous
	// load even starts. Ultimately, the topic's source action ID will match that of the preloaded topic.
	// The reason we did not set it already is to be as consistent with the historic design as possible.
	//
	if (!m_bPostLoadCalled && NULL != m_pLoadInfo && NULL != m_pLoadInfo->m_pLoader) {
		// (c.haag 2007-07-03 10:10) - PLID 26523 - Request access to topic information for
		// thread safety
		CHoldEMNLoaderMutex mh(m_pLoadInfo->m_pLoader->GetTopicsMutex());
		if (m_pLoadInfo->bLoadFromTemplate) {
			CEMNLoader::CPreloadedTemplateTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTemplateTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionSourceDataGroupID, -1);
		} else {
			CEMNLoader::CPreloadedTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionSourceDataGroupID, -1);
		}
	}

	//TES 1/30/2007 - PLID 24455 - Make sure the variable has been loaded, don't return a bad value.
	ENSURE_VARIABLE(m_n, SourceActionSourceDataGroupID);
	return m_nSourceActionSourceDataGroupID;
}

long CEMRTopic::GetSourceActionSourceHotSpotGroupID()
{
	// (z.manning, 01/23/2008) - PLID 28690 - Basically a copy of GetSourceActionID, with a different variable

	// (c.haag 2007-05-23 16:17) - PLID 26115 - If we're in the middle of the initial load, we
	// cannot rely on this object's m_nSourceActionSourceHotSpotGroupID value. We instead have to refer to the
	// preloader to get the value.
	//
	// In case you're wondering why: The preloader gathers all topic variables before the asynchronous
	// load even starts. Ultimately, the topic's source action ID will match that of the preloaded topic.
	// The reason we did not set it already is to be as consistent with the historic design as possible.
	//
	if (!m_bPostLoadCalled && NULL != m_pLoadInfo && NULL != m_pLoadInfo->m_pLoader) {
		// (c.haag 2007-07-03 10:10) - PLID 26523 - Request access to topic information for
		// thread safety
		CHoldEMNLoaderMutex mh(m_pLoadInfo->m_pLoader->GetTopicsMutex());
		if (m_pLoadInfo->bLoadFromTemplate) {
			CEMNLoader::CPreloadedTemplateTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTemplateTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionSourceHotSpotGroupID, -1);
		} else {
			CEMNLoader::CPreloadedTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionSourceHotSpotGroupID, -1);
		}
	}

	//TES 1/30/2007 - PLID 24455 - Make sure the variable has been loaded, don't return a bad value.
	ENSURE_VARIABLE(m_n, SourceActionSourceHotSpotGroupID);
	return m_nSourceActionSourceHotSpotGroupID;
}

long CEMRTopic::GetSourceActionSourceTableDropdownGroupID()
{
	// (z.manning 2009-02-13 10:59) - PLID 33070 - Basically a copy of GetSourceActionID, with a different variable

	// (c.haag 2007-05-23 16:17) - PLID 26115 - If we're in the middle of the initial load, we
	// cannot rely on this object's m_nSourceActionSourceTableDropdownGroupID value. We instead have to refer to the
	// preloader to get the value.
	//
	// In case you're wondering why: The preloader gathers all topic variables before the asynchronous
	// load even starts. Ultimately, the topic's source action ID will match that of the preloaded topic.
	// The reason we did not set it already is to be as consistent with the historic design as possible.
	//
	if (!m_bPostLoadCalled && NULL != m_pLoadInfo && NULL != m_pLoadInfo->m_pLoader) {
		// (c.haag 2007-07-03 10:10) - PLID 26523 - Request access to topic information for
		// thread safety
		CHoldEMNLoaderMutex mh(m_pLoadInfo->m_pLoader->GetTopicsMutex());
		if (m_pLoadInfo->bLoadFromTemplate) {
			CEMNLoader::CPreloadedTemplateTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTemplateTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionSourceTableDropdownGroupID, -1);
		} else {
			CEMNLoader::CPreloadedTopic* pPreloadedTopic = m_pLoadInfo->m_pLoader->GetPreloadedTopicByID(m_pLoadInfo->nID);
			if (NULL != pPreloadedTopic) return VarLong(pPreloadedTopic->m_vSourceActionSourceTableDropdownGroupID, -1);
		}
	}

	//TES 1/30/2007 - PLID 24455 - Make sure the variable has been loaded, don't return a bad value.
	ENSURE_VARIABLE(m_n, SourceActionSourceTableDropdownGroupID);
	return m_nSourceActionSourceTableDropdownGroupID;
}

//TES 3/18/2010 - PLID 37530 - Added overloads to ensure that either the source action name is passed in, or the index of the spawning stamp
// is passed in (it's needed to calculate the source action name).
void CEMRTopic::SetSourceActionID(const EmrAction &ea, const CString* pstrSourceActionName)
{
	SetSourceActionID(ea, -1, pstrSourceActionName);
}

void CEMRTopic::SetSourceActionID(const EmrAction &ea, long nSourceStampIndex)
{
	SetSourceActionID(ea, nSourceStampIndex, NULL);
}

// (j.jones 2007-07-31 09:09) - PLID 26882 - added an overload that takes in an action object,
// which helps save needless recordset calls
void CEMRTopic::SetSourceActionID(const EmrAction &ea, long nSourceStampIndex, const CString* pstrSourceActionName /* = NULL */)
{
	// (c.haag 2006-03-03 13:39) - PLID 19559 - We now track the source action
	// name for optimization reasons (namely, we don't have to call GetEmrAction
	// name for this specific topic anymore).
	//
	// (c.haag 2006-03-03 16:17) - PLID 19566 - The same applies with source
	// action destination types, too.
	if ((m_sai.nSourceActionID != ea.nID) ||
		(pstrSourceActionName && (*pstrSourceActionName != m_strSourceActionName)) ||
		(ea.eaoDestType != m_SourceActionDestType)
		){
		if (-1 != (m_sai.nSourceActionID = ea.nID)) {

			if(pstrSourceActionName) {
				m_strSourceActionName = *pstrSourceActionName;
			}
			else {

				// (j.jones 2007-07-31 16:03) - PLID 26898 - the EmrAction may have the strSourceName,
				// if so, we don't need to calculate it
				if(!ea.strSourceName.IsEmpty()) {
					m_strSourceActionName = ea.strSourceName;
				}
				else {
					//TES 3/18/2010 - PLID 37530 - Pass in the StampIndex
					m_strSourceActionName = GetEmrActionName(ea.nID, nSourceStampIndex, ea.eaoSourceType, ea.nSourceID);
				}
			}

			//DRT 9/27/2007 - PLID 27515 - If we're setting from an EmrAction object, set the Source Action's source ID too.  Then
			//	clear the group ID.  The unspawning only needs 1 of these, and since this is a "live" spawn, the first will always
			//	match up.
			m_nSourceActionSourceID = ea.nSourceID;
			m_nSourceActionSourceDataGroupID = -1;
			m_nSourceActionSourceHotSpotGroupID = -1;
			m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 11:01) - PLID 33070

			m_SourceActionDestType = ea.eaoDestType;
		} else {
			m_strSourceActionName.Empty();
			m_SourceActionDestType = eaoInvalid;
		}
	}
}

//TES 3/18/2010 - PLID 37530 - Added overloads to ensure that either the source action name is passed in, or the index of the spawning stamp
// is passed in (it's needed to calculate the source action name).
void CEMRTopic::SetSourceActionID(long nSourceActionID, const CString* pstrSourceActionName, EmrActionObject* pSourceActionDestType /* = NULL */)
{
	SetSourceActionID(nSourceActionID, -1, pstrSourceActionName, pSourceActionDestType);
}

void CEMRTopic::SetSourceActionID(long nSourceActionID, long nSourceStampIndex, EmrActionObject* pSourceActionDestType /* = NULL */)
{
	SetSourceActionID(nSourceActionID, nSourceStampIndex, NULL, pSourceActionDestType);
}

void CEMRTopic::SetSourceActionID(long nSourceActionID,
								  long nSourceStampIndex,
								  const CString* pstrSourceActionName,
								  EmrActionObject* pSourceActionDestType /* = NULL */)
{
	// (c.haag 2006-03-03 13:39) - PLID 19559 - We now track the source action
	// name for optimization reasons (namely, we don't have to call GetEmrAction
	// name for this specific topic anymore).
	//
	// (c.haag 2006-03-03 16:17) - PLID 19566 - The same applies with source
	// action destination types, too.
	if ((m_sai.nSourceActionID != nSourceActionID) ||
		(pstrSourceActionName && (*pstrSourceActionName != m_strSourceActionName)) ||
		(pSourceActionDestType && (*pSourceActionDestType != m_SourceActionDestType))
		){
		if (-1 != (m_sai.nSourceActionID = nSourceActionID)) {

			// (j.jones 2007-07-31 10:03) - PLID 26882 - Rather than call two recordsets, just call it once,
			// and pass appropriate information to GetEmrActionName. But only if we do not have both
			// passed in values

			EmrActionObject eaoSourceType = eaoInvalid;
			EmrActionObject eaoDestType = eaoInvalid;
			long nSourceID = -1;
			
			if(pstrSourceActionName == NULL || pSourceActionDestType == NULL) {

				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				_RecordsetPtr rsAction = CreateParamRecordset("SELECT SourceType, DestType, SourceID FROM EmrActionsT WHERE ID = {INT}", nSourceActionID);
				if(!rsAction->eof) {
					eaoSourceType = (EmrActionObject)AdoFldLong(rsAction, "SourceType");
					eaoDestType = (EmrActionObject)AdoFldLong(rsAction, "DestType");
					nSourceID = AdoFldLong(rsAction, "SourceID");
				}
				rsAction->Close();
			}

			//TES 3/18/2010 - PLID 37530 - Pass in the StampIndex
			m_strSourceActionName = ((pstrSourceActionName) ? (*pstrSourceActionName) : GetEmrActionName(nSourceActionID, nSourceStampIndex, eaoSourceType, nSourceID));
			m_SourceActionDestType = ((pSourceActionDestType) ? (*pSourceActionDestType) : eaoDestType);

			//DRT 9/27/2007 - PLID 27515 - If we're setting the SourceAction, set the Source Action's source ID too.  Then
			//	clear the group ID.  The unspawning only needs 1 of these, and since this is a "live" spawn, the first will always
			//	match up.
			m_nSourceActionSourceID = nSourceID;
			m_nSourceActionSourceDataGroupID = -1;
			m_nSourceActionSourceHotSpotGroupID = -1;
			m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 11:01) - PLID 33070
		} else {
			m_strSourceActionName.Empty();
			m_SourceActionDestType = eaoInvalid;
		}
	}
}

// (a.walling 2010-04-01 12:17) - PLID 38013 - Returns the CEMNDetail that directly or indirectly spawned this topic
// if bFurthest, we will keep traversing until we get to a non-spawned item or topic. Otherwise, it will return the first.
CEMNDetail* CEMRTopic::FindSpawningItem(bool bFurthest, SourceActionInfo& sai)
{
	// first determine if we are directly spawned from something
	if (!m_sai.IsBlank()) {
		// hey we are!
		CEMNDetail* pSpawningItem = m_sai.pSourceDetail;
		if (!pSpawningItem && m_sai.nSourceDetailID != -1) {
			pSpawningItem = GetParentEMN() ? GetParentEMN()->GetDetailByID(m_sai.nSourceDetailID) : NULL;
		}

		if (pSpawningItem) {

			// if we need to go to the farthest, then go.
			if (bFurthest) {
				SourceActionInfo saiFarthest;
				CEMNDetail* pFarthest = pSpawningItem->FindSpawningItem(bFurthest, saiFarthest);
				if (pFarthest) {
					sai = saiFarthest;
					return pFarthest;
				}
			}

			// at this point, we either did not find anything further, or we are not bothering to look.
			sai = m_sai;
			return pSpawningItem;
		}
	} else {
		// this topic is not spawned, so we'll just fall through and return NULL
	}

	// no luck
	return NULL;
}

void CEMRTopic::SetSourceDetail(CEMNDetail *pSourceDetail)
{
	m_sai.pSourceDetail = pSourceDetail;

	//also set the ID, if we have one
	if(pSourceDetail) {
		if(m_bIsTemplate && pSourceDetail->m_nEMRTemplateDetailID != -1) {
			m_sai.nSourceDetailID = pSourceDetail->m_nEMRTemplateDetailID;
		}
		else if(!m_bIsTemplate && pSourceDetail->m_nEMRDetailID != -1) {
			m_sai.nSourceDetailID = pSourceDetail->m_nEMRDetailID;
		}
	}
}

//TES 3/17/2010 - PLID 37530 - This appears to be dead code
/*// (z.manning 2010-03-03 15:15) - PLID 37532
void CEMRTopic::SetSourceDetailStampID(const long nDetailStampID)
{
	m_sai.SetDetailStampID(nDetailStampID);
}

// (z.manning 2010-03-03 15:15) - PLID 37532
void CEMRTopic::SetSourceDetailStampPointer(EmrDetailImageStamp *pDetailStamp)
{
	m_sai.SetDetailStampPointer(pDetailStamp);
}*/

CString CEMRTopic::GetSourceActionName()
{
	// (z.manning 2010-12-08 15:59) - PLID 41731 - We had previously only ever set the action name when initially loading
	// a topic. And until now, all action names were based on data at the admin level so that was fine. However, for table
	// dropdown spawning where the table row is a smart stamp row (meaning it's not in EmrDataT) some of the data we need
	// to name the table row is not at the admin level. Rather than do something drastic where we either force this into
	// the current method of loading the action name ahead of time or completely revamping this to always load on demand,
	// for now we just detect is this is a smart stamp row table dropdown spawning and if so, then we load on demand, 
	// otherwise we stick with the old method. I don't think it would be very difficult to always load this on the fly
	// so we may want to change to do that some time in the future.
	if(m_sai.IsSmartStampTableRowAction()) {
		CString strSourceActionName = m_sai.GenerateEmrActionName(m_pParentEMN);
		return strSourceActionName;
	}
	else {
		return m_strSourceActionName;
	}
}

EmrActionObject CEMRTopic::GetSourceActionDestType() const
{
	return m_SourceActionDestType;
}

void CEMRTopic::SetSourceActionDestType(EmrActionObject type)
{
	m_SourceActionDestType = type;
}

void CEMRTopic::ForceLoadToFinish()
{
	// (c.haag 2007-05-30 10:53) - PLID 26175 - Call this function to force an asynchronous load to finish if
	// one is in progress
	if(m_pLoadInfo) {
		GetParentEMN()->GetParentEMR()->LoadTopicImmediate(m_pLoadInfo);
		PostLoad();
	}
}

CEMRTopic* CEMRTopic::AddSubTopic(const CString &strName, long nTopicID)
{
	//TES 4/15/2010 - PLID 24692 - We may already have a position entry, otherwise create a new one.
	TopicPositionEntry *tpeNew = NULL;
	if(nTopicID != -1) {
		//TES 5/11/2010 - PLID 24692 - This is currently dead code (every place that calls this function passes in -1),
		// but this code should ever work if it ever gets revived.
		CEMN *pEMN = GetParentEMN();
		if(pEMN) {
			tpeNew = pEMN->GetTopicPositionEntryByID(nTopicID);
		}
		else {
			tpeNew = m_pTopicPositionEntry->GetEntryByID(nTopicID);
		}
	}
	if(tpeNew == NULL) {
		tpeNew = new TopicPositionEntry;
		tpeNew->nTopicID = nTopicID;
	}
	m_pTopicPositionEntry->pChild = AddTopicPositionEntryAtEnd(m_pTopicPositionEntry->pChild, tpeNew, m_pTopicPositionEntry);
	CEMRTopic *pNewTopic = new CEMRTopic(this, tpeNew);
	pNewTopic->SetName(strName);
	m_arypEMRTopics.Add(pNewTopic);
	m_bHasSubTopics = TRUE;
	m_bSubTopicArrayChanged = TRUE;
	return pNewTopic;
}

void CEMRTopic::SetName(const CString &strName)
{
	m_strTopicName = strName;
	m_bUnsaved = TRUE;
}

// (a.walling 2010-04-01 10:31) - PLID 38013 - Added function to get the top-level parent topic
CEMRTopic* CEMRTopic::GetTopLevelParentTopic() const
{
	CEMRTopic* pTopLevelParentTopic = NULL;
	CEMRTopic* pCurrentTopic = m_pParentTopic;

	while (pCurrentTopic) {
		pTopLevelParentTopic = pCurrentTopic;
		pCurrentTopic = pCurrentTopic->m_pParentTopic;
	}

	return pTopLevelParentTopic;
}

void CEMRTopic::SetParentTopic(CEMRTopic *pParent, BOOL bIsOriginalParent /*= FALSE*/, BOOL bAllowPostLoad /*= TRUE*/)
{
	m_pParentTopic = pParent;

	// (j.jones 2008-08-22 12:12) - PLID 30789 - if any problems are on this topic, update their pointers
	// (c.haag 2009-05-16 13:35) - PLID 34311 - We now use problem links
	for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
		CEmrProblemLink *pLink = m_apEmrProblemLinks.GetAt(i);
		if(pLink != NULL && pLink->GetType() == eprtEmrTopic) {
			pLink->UpdatePointersWithTopic(this);
		}
	}

	//TES 1/11/2007 - PLID 24172 - Don't call PostLoad() if we've been specifically told not to.
	if(m_pLoadInfo && m_pLoadInfo->m_bCompletelyLoaded && bAllowPostLoad) {
		//We may have been loaded synchronously without a parent, and are now having our parent set, meaning this is our first
		//chance to call PostLoad().  If it's already been called, it won't do anything.
		PostLoad();
	}
	if(bIsOriginalParent) {
		m_pOriginalParentTopic = pParent;
		if(m_pOriginalParentTopic)
			m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
	}
}

//TES 10/5/2009 - PLID 35755 - Added a parameter for whether this insertion should cause the topic order indexes
// to be recalculated when saving.
void CEMRTopic::InsertSubTopic(CEMRTopic *pInsert, CEMRTopic *pInsertBefore, BOOL bIsInitialLoad, BOOL bTopicArrayChanged)
{
	//TES 1/11/2007 - PLID 24172 - Make sure these functions don't call PostLoad(), if we're in the initial load.
	pInsert->SetParentEMN(GetParentEMN(), !bIsInitialLoad);
	pInsert->SetParentTopic(this, FALSE, !bIsInitialLoad);

	if(!bIsInitialLoad)
		pInsert->SetUnsaved();

	//TES 4/15/2010 - PLID 24692 - If the topic array didn't change, then we don't need to move our position entry, otherwise
	// ensure that it's in the correct location.
	//TES 4/29/2010 - PLID 24692 - Also, if we don't have this entry at all, make sure it gets added.
	if(bTopicArrayChanged || m_pTopicPositionEntry == NULL || !m_pTopicPositionEntry->HasEntry(pInsert->GetTopicPositionEntry())) {
		if(pInsertBefore) {
			m_pTopicPositionEntry->pChild = InsertTopicPositionEntry(m_pTopicPositionEntry->pChild, pInsert->GetTopicPositionEntry(), pInsertBefore->GetTopicPositionEntry());
			if(!m_pTopicPositionEntry->pChild->HasEntry(pInsert->GetTopicPositionEntry())) {
				//TES 5/11/2010 - PLID 24692 - We must have been asked to insert before something that isn't in the list.  So, add to the end,
				// since that's what the topic insertion logic below will do.
				m_pTopicPositionEntry->pChild = AddTopicPositionEntryAtEnd(m_pTopicPositionEntry->pChild, pInsert->GetTopicPositionEntry(), GetTopicPositionEntry());
			}
		}
		else {
			m_pTopicPositionEntry->pChild = AddTopicPositionEntryAtEnd(m_pTopicPositionEntry->pChild, pInsert->GetTopicPositionEntry(), GetTopicPositionEntry());
		}
	}
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		if(m_arypEMRTopics[i] == pInsertBefore) {
			m_arypEMRTopics.InsertAt(i, pInsert);
			if(!bIsInitialLoad) {
				//We're now unsaved.
				m_bUnsaved = TRUE;
				//TES 10/5/2009 - PLID 35755 - Only update this if our flag was set
				if(bTopicArrayChanged) {
					m_bSubTopicArrayChanged = TRUE;
				}
			}
			return;
		}
	}
	m_arypEMRTopics.Add(pInsert);

	m_bHasSubTopics = TRUE;

	if(!bIsInitialLoad) {
		//We're now unsaved.
		m_bUnsaved = TRUE;
		//TES 10/5/2009 - PLID 35755 - Only update this if our flag was set
		if(bTopicArrayChanged) {
			m_bSubTopicArrayChanged = TRUE;
		}
	}
}

void CEMRTopic::DetachSubTopic(CEMRTopic *pTopic)
{
	pTopic->m_pParentTopic = NULL;

	for(int i = m_arypEMRTopics.GetSize()-1; i >= 0; i--) {
		if(m_arypEMRTopics[i] == pTopic) {
			//TES 4/15/2010 - PLID 24692 - Detach it from our entry list.
			m_pTopicPositionEntry = DetachTopicPositionEntry(m_pTopicPositionEntry, m_arypEMRTopics[i]->GetTopicPositionEntry());
			m_arypEMRTopics.RemoveAt(i);
		}

		pTopic->SetUnsaved();
	}

	m_bHasSubTopics = m_arypEMRTopics.GetSize() > 0;
	m_bUnsaved = TRUE;
	m_bSubTopicArrayChanged = TRUE;
}

// (a.walling 2009-10-13 14:48) - PLID 36024 - Parameter to release ref
void CEMRTopic::DetachDetail(CEMNDetail *pDetail, BOOL bReleaseRef)
{
	EnsureDetails();
	
	pDetail->m_pParentTopic = NULL;

	//While we're at it, we need to update our completion status.
	BOOL bNonEmptyFound = FALSE;
	BOOL bEmptyFound = FALSE;
	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	BOOL bFoundRequiredUnfilled = FALSE;
	for(int i = m_arypEMNDetails.GetSize()-1; i >= 0; i--) {
		if(m_arypEMNDetails[i] == pDetail) {
			// (a.walling 2009-10-13 14:47) - PLID 36024 - Release if desired
			if (bReleaseRef) {
				pDetail->__Release("CEMRTopic::DetachDetail");
			}
			m_arypEMNDetails.RemoveAt(i);

			// (a.walling 2007-04-12 08:56) - PLID 25599 - Ensure that m_bHasDetails is up to date.
			if (m_arypEMNDetails.GetSize() == 0)
				m_bHasDetails = FALSE;
		}
		else if(m_arypEMNDetails[i]->GetVisible()) {
			if (m_arypEMNDetails[i]->IsStateSet()) {
				bNonEmptyFound = TRUE;
			} else {
				bEmptyFound = TRUE;
				// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
				if (m_arypEMNDetails[i]->IsRequired()) {
					bFoundRequiredUnfilled = TRUE;
				}
			}
		}
	}
	
	// (z.manning, 04/04/2008) - PLID 29495 - Now have a status for topics with no details
	if(m_arypEMNDetails.GetSize() == 0) {
		m_CompletionStatus = etcsBlank;
	}
	else if (bFoundRequiredUnfilled) {
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		m_CompletionStatus = etcsRequired;
	}
	else if(bNonEmptyFound) {
		if(bEmptyFound) {
			m_CompletionStatus = etcsPartiallyComplete;
		}
		else {
			m_CompletionStatus = etcsComplete;
		}
	}
	else {
		m_CompletionStatus = etcsIncomplete;
	}

	m_bUnsaved = TRUE;
}

long CEMRTopic::GetOriginalTemplateTopicID(BOOL bEnsureVariable /*= TRUE*/)
{
	if(bEnsureVariable) ENSURE_VARIABLE(m_n, OriginalTemplateTopicID);
	return m_nOriginalTemplateTopicID;
}

void CEMRTopic::SetOriginalTemplateTopicID(long nID)
{
	m_nOriginalTemplateTopicID = nID;
}

// (a.walling 2007-03-21 10:39) - PLID 25301 - Get the original template id, ensuring that it is loaded
long CEMRTopic::GetOriginalTemplateID(BOOL bEnsureVariable /*= TRUE*/)
{
	if(bEnsureVariable) ENSURE_VARIABLE(m_n, OriginalTemplateID);
	return m_nOriginalTemplateID;
}

// (a.walling 2007-03-21 10:39) - PLID 25301 - Set the original template id
void CEMRTopic::SetOriginalTemplateID(long nID)
{
	m_nOriginalTemplateID = nID;
}

//DRT 8/9/2007 - PLID 26876 - If wanting to remove a subtopic from an external source (another class), you should use this function.
void CEMRTopic::RemoveSubTopic_External(CEMRTopic *pTopic)
{
	BOOL bFound = FALSE;
	int i;

	// (c.haag 2007-06-12 17:05) - PLID 26288 - Check whether the topic exists in
	// our topic child array
	for (i=0; i < m_arypEMRTopics.GetSize() && !bFound; i++) {
		if (pTopic == m_arypEMRTopics[i]) {
			bFound = TRUE;
		}
	}
	if (!bFound) {
		// (c.haag 2007-06-12 17:05) - PLID 26288 - The legacy code never complained
		// if pTopic was not in m_arypEMRTopics, and this item won't change that.
		return;
	}
	//Do the actual removal.  This is the "high level" way of removing, which does all the overhead work, 
	//	then removes all the details, then does any cleanup.
	pTopic->RemoveThisTopicAndSubTopics();
	//TES 4/15/2010 - PLID 24692 - Detach and destroy the topic's position entry.
	TopicPositionEntry *tpeDoomed = pTopic->GetTopicPositionEntry();
	m_pTopicPositionEntry = DetachTopicPositionEntry(m_pTopicPositionEntry, tpeDoomed);
	tpeDoomed->FreeDescendants();
	delete tpeDoomed;

	//Need to mark this subtopic as deleted from our current topic and update the related variables
	m_aryDeletedTopics.Add(pTopic);
	m_bSubTopicArrayChanged = TRUE;
	m_bHasSubTopics = (m_arypEMRTopics.GetSize() > 0);

	// (c.haag 2007-06-12 17:06) - PLID 26288 - Ensure pTopic does not exist in m_arypEMRTopics.
	// Notice that we don't care what i was the last time we did this. That's because m_arypEMRTopics
	// may have changed since a previous call to pTopic->RemoveDetail.
	for (i=0, bFound = FALSE; i < m_arypEMRTopics.GetSize() && !bFound; i++) {
		if (pTopic == m_arypEMRTopics[i]) {
			m_arypEMRTopics.RemoveAt(i);
			bFound = TRUE;
		}
	}
}

//DRT 7/30/2007 - PLID 26876 - Modified so that it uses the new RemoveDetail_DetailSpecific() functionality.
//	This should never be called anywhere except ::RemoveThisTopicAndSubTopics, or anywhere that covers the 
//	"overhead" portions of detail removal.
void CEMRTopic::RemoveSubTopic(CEMRTopic *pTopic)
{
	// (c.haag 2007-06-12 16:59) - PLID 26288 - We used to do this:
	//
	// for(int i = m_arypEMRTopics.GetSize()-1; i >= 0; i--) {
	//		if(m_arypEMRTopics[i] == pTopic) {
	//			...
	//			m_arypEMRTopics.RemoveAt(i);
	//		}
	// }
	//
	// This is a poor design. We should only run through m_arypEMRTopics twice:
	// Once to confirm that the topic exists, and once more to remove pTopic.
	//
	BOOL bFound = FALSE;
	int i;

	// (c.haag 2007-06-12 17:05) - PLID 26288 - Check whether the topic exists in
	// our topic child array
	for (i=0; i < m_arypEMRTopics.GetSize() && !bFound; i++) {
		if (pTopic == m_arypEMRTopics[i]) {
			bFound = TRUE;
		}
	}
	if (!bFound) {
		// (c.haag 2007-06-12 17:05) - PLID 26288 - The legacy code never complained
		// if pTopic was not in m_arypEMRTopics, and this item won't change that.
		return;
	}
	
	//DRT 7/30/2007 - PLID 26876 - I made a function which does the "guts" of ::RemoveDetail, without
	//	things that don't need to be done en masse (narrative updating, etc).  All of that stuff is taken
	//	care of outside of this function in ::RemoveThisTopicAndSubTopics()
	//We do need to ensure that this topic has finished loading in the preloader.
	EnsureDetails();

	//We have to revoke all the actions for this topic's details.
	for(int j = pTopic->GetEMNDetailCount()-1; j >= 0; j--) {
		CEMNDetail *pDetail = pTopic->GetDetailByIndex(j);

		// (j.jones 2006-03-15 09:17) - massive unspawning may have already removed this item
		if(!pDetail)
			continue;

		//Just remove the detail specific stuff, don't worry about narratives, other topics, etc.
		pTopic->RemoveDetail_DetailSpecific(pDetail);
	}

	//And remove each of the subtopics.
	while(pTopic->GetSubTopicCount()) {
		pTopic->RemoveSubTopic(pTopic->GetSubTopic(0));
	}

	//TES 4/15/2010 - PLID 24692 - Detach and destry the topic's position entry.
	TopicPositionEntry *tpeDoomed = pTopic->GetTopicPositionEntry();
	m_pTopicPositionEntry = DetachTopicPositionEntry(m_pTopicPositionEntry, tpeDoomed);
	tpeDoomed->FreeDescendants();
	delete tpeDoomed;

	m_aryDeletedTopics.Add(pTopic);

	// (c.haag 2007-06-12 17:06) - PLID 26288 - Ensure pTopic does not exist in m_arypEMRTopics.
	// Notice that we don't care what i was the last time we did this. That's because m_arypEMRTopics
	// may have changed since a previous call to pTopic->RemoveDetail.
	for (i=0, bFound = FALSE; i < m_arypEMRTopics.GetSize() && !bFound; i++) {
		if (pTopic == m_arypEMRTopics[i]) {
			m_arypEMRTopics.RemoveAt(i);
			bFound = TRUE;
		}
	}

	m_bSubTopicArrayChanged = TRUE;


	m_bHasSubTopics = (m_arypEMRTopics.GetSize() > 0);

	// (a.walling 2008-02-06 14:33) - PLID 28391 - Refresh HTML visibility state
	RefreshHTMLVisibility();
}

int CEMRTopic::GetIndexAgainstEMN()
{
	return GetParentEMN()->GetTopicIndex(this);
}

// (a.walling 2012-06-22 14:01) - PLID 51150 - returns the EMN's GetInterface
CEmrTreeWnd* CEMRTopic::GetInterfaceWnd() const
{
	if (CEMN* pEMN = GetParentEMN()) {
		return pEMN->GetInterface();
	}
	return NULL;
}

/*	DRT 8/3/2007 - PLID 26937 - This is no longer needed here.  Anywhere that wants to revoke any actions will
		use the CEMNUnspawner, which will recursively find all details to be unspawned.
void CEMRTopic::RevokeAllActions()
{
	EnsureDetails();
	EnsureTopics();

	//We have to revoke all the actions for this topic's details.
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pDetail = m_arypEMNDetails[i];
		
		// (j.jones 2007-07-27 10:29) - PLID 26845 - check whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them
		if(pDetail->GetHasInfoActionsStatus() != ehiasHasNoInfoItems) {
			GetParentEMN()->GetParentEMR()->RevokeEMRInfoActions(pDetail);
		}

		CDWordArray arDoomedIDs;
		pDetail->GetSelectedValues(arDoomedIDs);
		for(int j = 0; j < arDoomedIDs.GetSize(); j++) GetParentEMN()->GetParentEMR()->RevokeEMRDataActions(arDoomedIDs[j], pDetail);
	}
	for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		m_arypEMRTopics[i]->RevokeAllActions();
	}
}
*/

void CEMRTopic::ProcessAllActions(BOOL bIsInitialLoad)
{
	EnsureDetails();
	EnsureTopics();

	//We have to process all the actions for this topic's details.
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pDetail = m_arypEMNDetails[i];
		
		// (j.jones 2007-07-18 13:21) - PLID 26730 - check whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them
		if(pDetail->GetHasInfoActionsStatus() != ehiasHasNoInfoItems) {
			GetParentEMN()->GetParentEMR()->ProcessEMRInfoActions(pDetail, bIsInitialLoad);
		}

		// (j.jones 2010-02-12 14:23) - PLID 37318 - check whether this is part of a SmartStamp
		// set of details, and ensure links (or create the table) accordingly
		GetParentEMN()->EnsureSmartStampLinks(pDetail);

		pDetail->CollectAndProcessActionsPostLoad(bIsInitialLoad);
	}
	for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		m_arypEMRTopics[i]->ProcessAllActions(bIsInitialLoad);
	}
}

// (z.manning 2008-09-03 12:56) - PLID 31235 - GetSubTopicByTemplateTopicID was split into 4 functions
// (a.walling 2010-04-05 13:28) - PLID 38060 - Also look for the spawned group table row
CEMRTopic* CEMRTopic::GetSubTopicByTemplateTopicIDAndSpawnedGroupID(long nTemplateTopicID, long nSpawnedGroupID, CEMNDetail *pSourceDetail, SourceActionInfo* pSpawnedGroupSourceActionInfo )
{
	EnsureTopics();

	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		//Is it one of our topics?

		// (j.jones 2007-01-12 12:31) - PLID 24027 - I simplified this into booleans.
		// bSourceDetailOK means that we have no source detail object, or that they match

		BOOL bTemplateTopicIDsMatch = (m_arypEMRTopics[i]->GetTemplateTopicID() == nTemplateTopicID);
		BOOL bSpawnedGroupIDsMatch = (m_arypEMRTopics[i]->GetSpawnedGroupID() == nSpawnedGroupID);
		// (a.walling 2010-04-05 13:22) - PLID 38060
		BOOL bSpawnedGroupSourceActionInfoMatches = pSpawnedGroupSourceActionInfo ? FALSE : TRUE;
		BOOL bSourceDetailOK = FALSE;

		//if we have an object, see that the pointer matches the topic source,
		//or that the source detail IDs match
		if(pSourceDetail) {
			
			if(m_arypEMRTopics[i]->GetSourceDetail()) {
				if(pSourceDetail == m_arypEMRTopics[i]->GetSourceDetail()) {
					bSourceDetailOK = TRUE;
				}
			}

			if(m_arypEMRTopics[i]->GetSourceDetailID() != -1) {
				if(m_arypEMRTopics[i]->GetSourceDetailID() == pSourceDetail->m_nEMRDetailID) {
					bSourceDetailOK = TRUE;
				}
			}
		}
		else {
			//no pointer, well that's ok, we just won't search by it
			bSourceDetailOK = TRUE;
		}

		// (a.walling 2010-04-05 13:23) - PLID 38060
		if (pSpawnedGroupSourceActionInfo) {
			SourceActionInfo saiCheck = m_arypEMRTopics[i]->GetSourceActionInfo();

			if (pSpawnedGroupSourceActionInfo->eaoSourceType == eaoEmrTableDropDownItem &&
				pSpawnedGroupSourceActionInfo->eaoSourceType == saiCheck.eaoSourceType &&
				pSpawnedGroupSourceActionInfo->TableRowMatches(saiCheck)) {

				bSpawnedGroupSourceActionInfoMatches = TRUE;
			}
		}

		if(bTemplateTopicIDsMatch && bSpawnedGroupIDsMatch && bSourceDetailOK && bSpawnedGroupSourceActionInfoMatches)
			return m_arypEMRTopics[i];

		//Nope, is it one of our topics' subtopics?
		CEMRTopic *pTmp = m_arypEMRTopics[i]->GetSubTopicByTemplateTopicIDAndSpawnedGroupID(nTemplateTopicID, nSpawnedGroupID, pSourceDetail, pSpawnedGroupSourceActionInfo);
		if(pTmp) {
			return pTmp;
		}
	}

	return NULL;
}

// (z.manning 2008-09-03 12:56) - PLID 31235 - GetSubTopicByTemplateTopicID was split into 4 functions
// (a.walling 2010-04-05 13:28) - PLID 38060 - Also look for the spawned group table row
CEMRTopic* CEMRTopic::GetSubTopicByOriginalTemplateTopicIDAndSpawnedGroupID(long nTemplateTopicID, long nSpawnedGroupID, CEMNDetail *pSourceDetail, SourceActionInfo* pSpawnedGroupSourceActionInfo )
{
	//None of our subtopics are directly from this template topic, but maybe one of them was originally?
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {

		// (j.jones 2007-01-12 12:31) - PLID 24027 - I simplified this into booleans.
		// bSourceDetailOK means that we have no source detail object, or that they match

		BOOL bOriginalTemplateTopicIDsMatch = (m_arypEMRTopics[i]->GetOriginalTemplateTopicID() == nTemplateTopicID);
		BOOL bSpawnedGroupIDsMatch = (m_arypEMRTopics[i]->GetSpawnedGroupID() == nSpawnedGroupID);
		// (a.walling 2010-04-05 13:22) - PLID 38060
		BOOL bSpawnedGroupSourceActionInfoMatches = pSpawnedGroupSourceActionInfo ? FALSE : TRUE;
		BOOL bSourceDetailOK = FALSE;

		//if we have an object, see that the pointer matches the topic source,
		//or that the source detail IDs match
		if(pSourceDetail) {
			
			if(m_arypEMRTopics[i]->GetSourceDetail()) {
				if(pSourceDetail == m_arypEMRTopics[i]->GetSourceDetail()) {
					bSourceDetailOK = TRUE;
				}
			}

			if(m_arypEMRTopics[i]->GetSourceDetailID() != -1) {
				if(m_arypEMRTopics[i]->GetSourceDetailID() == pSourceDetail->m_nEMRDetailID) {
					bSourceDetailOK = TRUE;
				}
			}
		}
		else {
			//no pointer, well that's ok, we just won't search by it
			bSourceDetailOK = TRUE;
		}
		
		// (a.walling 2010-04-05 13:23) - PLID 38060
		if (pSpawnedGroupSourceActionInfo) {
			SourceActionInfo saiCheck = m_arypEMRTopics[i]->GetSourceActionInfo();

			if (pSpawnedGroupSourceActionInfo->eaoSourceType == eaoEmrTableDropDownItem &&
				pSpawnedGroupSourceActionInfo->eaoSourceType == saiCheck.eaoSourceType &&
				pSpawnedGroupSourceActionInfo->TableRowMatches(saiCheck)) {

				bSpawnedGroupSourceActionInfoMatches = TRUE;
			}
		}

		if(bOriginalTemplateTopicIDsMatch && bSpawnedGroupIDsMatch && bSourceDetailOK && bSpawnedGroupSourceActionInfoMatches)
			return m_arypEMRTopics[i];

		//Nope, is it one of our topics' subtopics?
		CEMRTopic *pTmp = m_arypEMRTopics[i]->GetSubTopicByOriginalTemplateTopicIDAndSpawnedGroupID(nTemplateTopicID, nSpawnedGroupID, pSourceDetail, pSpawnedGroupSourceActionInfo);
		if(pTmp) {
			return pTmp;
		}
	}

	return NULL;
}

// (a.walling 2010-04-05 11:08) - PLID 38060
// (a.walling 2010-04-05 13:28) - PLID 38060 - Also look for the spawned group table row
CEMRTopic* CEMRTopic::GetSubTopicByTemplateTopicIDAndSpawnedGroupIDOnly(long nTemplateTopicID, long nSpawnedGroupID, SourceActionInfo* pSpawnedGroupSourceActionInfo )
{
	EnsureTopics();

	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		//Is it one of our topics?

		// (j.jones 2007-01-12 12:31) - PLID 24027 - I simplified this into booleans.
		// bSourceDetailOK means that we have no source detail object, or that they match

		BOOL bTemplateTopicIDsMatch = (m_arypEMRTopics[i]->GetTemplateTopicID() == nTemplateTopicID);
		BOOL bSpawnedGroupIDsMatch = (m_arypEMRTopics[i]->GetSpawnedGroupID() == nSpawnedGroupID);
		// (a.walling 2010-04-05 13:22) - PLID 38060
		BOOL bSpawnedGroupSourceActionInfoMatches = pSpawnedGroupSourceActionInfo ? FALSE : TRUE;

		// (a.walling 2010-04-05 13:23) - PLID 38060
		if (pSpawnedGroupSourceActionInfo) {
			SourceActionInfo saiCheck = m_arypEMRTopics[i]->GetSourceActionInfo();

			if (pSpawnedGroupSourceActionInfo->eaoSourceType == eaoEmrTableDropDownItem &&
				pSpawnedGroupSourceActionInfo->eaoSourceType == saiCheck.eaoSourceType &&
				pSpawnedGroupSourceActionInfo->TableRowMatches(saiCheck)) {

				bSpawnedGroupSourceActionInfoMatches = TRUE;
			}
		}

		if(bTemplateTopicIDsMatch && bSpawnedGroupIDsMatch && bSpawnedGroupSourceActionInfoMatches)
			return m_arypEMRTopics[i];

		//Nope, is it one of our topics' subtopics?
		CEMRTopic *pTmp = m_arypEMRTopics[i]->GetSubTopicByTemplateTopicIDAndSpawnedGroupIDOnly(nTemplateTopicID, nSpawnedGroupID, pSpawnedGroupSourceActionInfo);
		if(pTmp) {
			return pTmp;
		}
	}

	return NULL;
}

// (a.walling 2010-04-05 11:08) - PLID 38060
// (a.walling 2010-04-05 13:28) - PLID 38060 - Also look for the spawned group table row
CEMRTopic* CEMRTopic::GetSubTopicByOriginalTemplateTopicIDAndSpawnedGroupIDOnly(long nTemplateTopicID, long nSpawnedGroupID, SourceActionInfo* pSpawnedGroupSourceActionInfo )
{
	//None of our subtopics are directly from this template topic, but maybe one of them was originally?
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {

		// (j.jones 2007-01-12 12:31) - PLID 24027 - I simplified this into booleans.
		// bSourceDetailOK means that we have no source detail object, or that they match

		BOOL bOriginalTemplateTopicIDsMatch = (m_arypEMRTopics[i]->GetOriginalTemplateTopicID() == nTemplateTopicID);
		BOOL bSpawnedGroupIDsMatch = (m_arypEMRTopics[i]->GetSpawnedGroupID() == nSpawnedGroupID);
		// (a.walling 2010-04-05 13:22) - PLID 38060
		BOOL bSpawnedGroupSourceActionInfoMatches = pSpawnedGroupSourceActionInfo ? FALSE : TRUE;

		// (a.walling 2010-04-05 13:23) - PLID 38060
		if (pSpawnedGroupSourceActionInfo) {
			SourceActionInfo saiCheck = m_arypEMRTopics[i]->GetSourceActionInfo();

			if (pSpawnedGroupSourceActionInfo->eaoSourceType == eaoEmrTableDropDownItem &&
				pSpawnedGroupSourceActionInfo->eaoSourceType == saiCheck.eaoSourceType &&
				pSpawnedGroupSourceActionInfo->TableRowMatches(saiCheck)) {

				bSpawnedGroupSourceActionInfoMatches = TRUE;
			}
		}

		if(bOriginalTemplateTopicIDsMatch && bSpawnedGroupIDsMatch && bSpawnedGroupSourceActionInfoMatches)
			return m_arypEMRTopics[i];

		//Nope, is it one of our topics' subtopics?
		CEMRTopic *pTmp = m_arypEMRTopics[i]->GetSubTopicByOriginalTemplateTopicIDAndSpawnedGroupIDOnly(nTemplateTopicID, nSpawnedGroupID, pSpawnedGroupSourceActionInfo);
		if(pTmp) {
			return pTmp;
		}
	}

	return NULL;
}

// (z.manning 2008-09-03 12:56) - PLID 31235 - GetSubTopicByTemplateTopicID was split into 4 functions
CEMRTopic* CEMRTopic::GetSubTopicByTemplateTopicIDOnly(long nTemplateTopicID)
{
	// (z.manning, 08/13/2007) - PLID 27041 - The code here to search for template topic ID and original
	// template topic ID was not here even though the same logic is in CEMN::GetTopicByTemplateTopicID.
	// I added it here as well to be consistent.
	//Not found so far, try matching just nTemplateTopicID.
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		if(m_arypEMRTopics[i]->GetTemplateTopicID() == nTemplateTopicID) {
			return m_arypEMRTopics[i];
		}

		CEMRTopic *pTmp = m_arypEMRTopics[i]->GetSubTopicByTemplateTopicIDOnly(nTemplateTopicID);
		if(pTmp) {
			return pTmp;
		}
	}
	return NULL;
}

// (z.manning 2008-09-03 12:56) - PLID 31235 - GetSubTopicByTemplateTopicID was split into 4 functions
CEMRTopic* CEMRTopic::GetSubTopicByOriginalTemplateTopicIDOnly(long nTemplateTopicID)
{
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		if(m_arypEMRTopics[i]->GetOriginalTemplateTopicID() == nTemplateTopicID) {
			return m_arypEMRTopics[i];
		}

		CEMRTopic *pTmp = m_arypEMRTopics[i]->GetSubTopicByOriginalTemplateTopicIDOnly(nTemplateTopicID);
		if(pTmp) {
			return pTmp;
		}
	}
	return NULL;
}

void CEMRTopic::SetParentEMN(CEMN* pParentEMN, BOOL bAllowPostLoad /*= TRUE*/)
{
	m_pParentEMN = pParentEMN;
	// (z.manning, 09/05/2006) - PLID 22392 - We may have just set the parent EMN to NULL, and if so,
	// we have to assume that they are removing this topic, thus there is no reason to call PostLoad().
	// (Which needs a valid parent EMN to function properly anyway.)
	//TES 1/11/2007 - PLID 24172 - Don't call PostLoad() if we've been specifically told not to.
	if(m_pParentEMN && m_pLoadInfo && m_pLoadInfo->m_bCompletelyLoaded && bAllowPostLoad) {
		//We may have been loaded synchronously without a parent, and are now having our parent set, meaning this is our first
		//chance to call PostLoad().  If it's already been called, it won't do anything.
		PostLoad();
	}
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - Added option to ignore a topic
BOOL CEMRTopic::IsEmpty(OPTIONAL CEMNDetail *pDetailToIgnore /*= NULL */, OPTIONAL BOOL bIgnoreBlankSubtopics /* = TRUE */, OPTIONAL CEMRTopic* pTopicToIgnore /*= NULL*/)
{
	if(!pDetailToIgnore) {
		//We can just use our cached (and synchronously loaded) value.
		if(HasDetails()) {
			return FALSE;
		}
		if(HasSubTopics()) {
			// (z.manning, 03/05/2007) - PLID 24529 - Having subtopics used to mean not empty in all situations.
			// Now we have a parameter to tell us whether or not we want count blank subtopics when determing if
			// the parent topic is empty.
			if(bIgnoreBlankSubtopics) {
				for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
					if (m_arypEMRTopics[i] == pTopicToIgnore) {
						continue;
					}
					if(!m_arypEMRTopics[i]->IsEmpty(pDetailToIgnore, bIgnoreBlankSubtopics, pTopicToIgnore)) {
						return FALSE;
					}
				}
			}
			else {
				return FALSE;
			}
		}
		return TRUE;
	}
	else {
		//We'll have to make sure we're fully loaded.
		EnsureDetails();
		EnsureTopics();

		for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
			if(m_arypEMNDetails[i] != pDetailToIgnore && m_arypEMNDetails[i]->GetVisible()) return FALSE;
		}
		for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
			if (m_arypEMRTopics[i] == pTopicToIgnore) {
				continue;
			}
			if(!m_arypEMRTopics[i]->IsEmpty(pDetailToIgnore, bIgnoreBlankSubtopics, pTopicToIgnore)) return FALSE;
		}
		return TRUE;
	}
}

BOOL CEMRTopic::PropagateNewID(long nID, EmrSaveObjectType esotSaveType, long nObjectPtr, long &nAuditTransactionID)
{
	EnsureDetails();
	EnsureTopics();

	//test to see if the nObjectPtr and esotSaveType match this object,
	//if so, assign the ID and return TRUE,
	//otherwise, propagate into the children of this item

	//is it this object?
	if(esotSaveType == esotTopic && (CEMRTopic*)nObjectPtr == this) {
		//we have a winner!
		// (c.haag 2007-06-19 17:40) - PLID 26388 - Fail if an ID already exists
		if (m_nID != -1) {
			ASSERT(FALSE); ThrowNxException("Called CEMRTopic::PropagateNewID on a topic with an existing ID! (Current = %d nID = %d)", m_nID, nID);
		}
		m_nID = nID;
		//TES 4/15/2010 - PLID 24692 - Update our position entry.
		m_pTopicPositionEntry->nTopicID = nID;

		// (j.jones 2008-07-29 17:33) - PLID 30880 - if any problems are on this topic, update their regarding IDs
		// (c.haag 2009-05-16 13:35) - PLID 34311 - We now use problem links
		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblemLink *pLink = m_apEmrProblemLinks.GetAt(i);
			if(pLink != NULL && pLink->GetType() == eprtEmrTopic) {
				pLink->SetRegardingID(nID);
			}
		}

		return TRUE;
	}
	else {
		//otherwise check all subtopics and details
		BOOL bFound = FALSE;
		// (a.walling 2007-09-04 10:48) - PLID 27284 - PropagateNewID not only updates the ID of a detail, but also the
		// SourceDetailIDs of any item that references that detail. If we immediately stop the search as soon as the target
		// detail is found, we are effectively updating the SourceDetailID of any object preceding it in the search while
		// leaving those following in an invalid state (SourceActionID with no SourceDetailID!)
		// We don't seem to have any choice other than to scan the entire structure.
		for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
			if(m_arypEMRTopics.GetAt(i)->PropagateNewID(nID, esotSaveType, nObjectPtr, nAuditTransactionID))
				bFound = TRUE;
		}
		for(i = 0; i < m_arypEMNDetails.GetSize(); i++) {
			if(m_arypEMNDetails.GetAt(i)->PropagateNewID(nID, esotSaveType, nObjectPtr, nAuditTransactionID))
				bFound = TRUE;
		}

		if(esotSaveType == esotDetail) {

			// (j.jones 2007-01-12 14:35) - PLID 24027 - see if our SourceDetail pointer is this detail,
			// if so, update our SourceDetailID accordingly
			
			if(m_sai.nSourceDetailID == -1 && m_sai.pSourceDetail != NULL
				&& m_sai.pSourceDetail == (CEMNDetail*)nObjectPtr) {

				//this is our source detail! update the ID
				// (c.haag 2007-06-19 17:40) - PLID 26388 - Fail if an ID already exists
				if (m_sai.nSourceDetailID != -1) {
					ASSERT(FALSE); ThrowNxException("Called CEMRTopic::PropagateNewID on a topic with an existing source detail ID! (Current = %d nID = %d)", m_sai.nSourceDetailID, nID);
				}
				m_sai.nSourceDetailID = nID;
			}
		}

		if(esotSaveType == esotProblemLink)
		{
			// (z.manning 2009-05-22 15:14) - PLID 34332 - Need to handle problem links
			for(int nProblemLinkIndex = 0; nProblemLinkIndex < m_apEmrProblemLinks.GetSize(); nProblemLinkIndex++)
			{
				CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(nProblemLinkIndex);
				if(pProblemLink == (CEmrProblemLink*)nObjectPtr)
				{
					if(pProblemLink->GetID() > 0) {
						ThrowNxException("Called CEMRTopic::PropagateNewID on an existing problem link for a EMR topic! (Problem link ID: %li)", pProblemLink->GetID());
					}

					pProblemLink->SetID(nID);
					pProblemLink->Audit(aeiEMNProblemLinkCreated, nAuditTransactionID, GetExistingPatientName(GetParentEMN()->GetParentEMR()->GetPatientID()));
					bFound = TRUE;
				}
			}
		}

		if(esotSaveType == esotDetailImageStamp)
		{
			// (z.manning 2011-01-03 17:05) - PLID 41974 - We must also check the source action info for this detail stamp.
			if(m_sai.GetDetailStampID() == -1 && m_sai.GetDetailStampPointer() != NULL
				&& (long)m_sai.GetDetailStampPointer() == nObjectPtr)
			{
				m_sai.SetDetailStampID(nID);
			}
		}

		return bFound;
	}
}

BOOL CEMRTopic::IsUnsaved(BOOL bCheckSubtopics)
{
	//We're unsaved if m_bUnsaved is true, our ID is -1, any of our details are unsaved, or we have any deleted details or subtopics.
	if(m_bUnsaved || m_bSubTopicArrayChanged) {
		return TRUE;
	}

	//TES 7/5/06 - If we're a spawned topic on a template, then even if we're new, we are saved.
	if(m_nID == -1 && (!GetParentEMN()->IsTemplate() || m_sai.nSourceActionID == -1)) {
		return TRUE;
	}

	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		if(m_arypEMNDetails[i]->IsModified()) 
			return TRUE;
	}

	if(bCheckSubtopics) {
		for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
			if(m_arypEMRTopics[i]->IsUnsaved(TRUE)) 
				return TRUE;
		}
	}

	if(m_aryDeletedDetails.GetSize() || m_aryDeletedTopics.GetSize()) 
		return TRUE;

	return FALSE;
}

void CEMRTopic::SetSaved(BOOL bIsPostLoad)
{
	// (j.jones 2006-08-24 11:43) - PLID 22183 - if this is a post save, as opposed
	// to a post load, then don't mark as saved if it's on a locked EMN,
	// because it would not have been saved
	if(bIsPostLoad || !GetIsOnLockedAndSavedEMN()) {
		m_bUnsaved = FALSE;
		m_bForceSave = FALSE;
		m_bSubTopicArrayChanged = FALSE;
	}
	
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		m_arypEMNDetails[i]->SetSaved(bIsPostLoad);
	}

	for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		m_arypEMRTopics[i]->SetSaved(bIsPostLoad);
	}
}

// (j.jones 2007-01-31 14:25) - PLID 24515 - added bForceSave, so if you make a change
// before PostLoad() finishes, it will not mark the topic as saved again
void CEMRTopic::SetUnsaved(BOOL bForceSave /*= FALSE*/)
{
	m_bUnsaved = TRUE;

	//do not blindly assign bForceSave to m_bForceSave,
	//we do not want to set it to false if already true
	if(bForceSave)
		m_bForceSave = TRUE;
}


// (j.jones 2007-08-02 11:42) - PLID 26915 - added ability to pass in a connection
void CEMRTopic::LoadFromDetail(CEMNDetail *pDetail, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	m_bIsTemplate = pDetail->m_bIsTemplateDetail;
	
	_RecordsetPtr rsTopic;

	if(m_bIsTemplate) {
		//TES 2/17/2006 - Template Details are never loaded independently, if they ever are we'll need to handle it here.
		ASSERT(FALSE);
	}
	else {
		//Load just the information relevant to this detail.
		//DRT 9/26/2007 - PLID 27515 - Added SourceActionSourceID, SourceActionSourceDataGroupID
		// (z.manning, 01/23/2008) - PLID 28690 - Added SourceActionSourceHotSpotGroupID
		// (z.manning 2009-02-13 11:02) - PLID 33070 - SourceActionSourceTableDropdownGroupID
		// (z.manning 2009-03-05 10:57) - PLID 33338
		//TES 2/16/2010 - PLID 37298 - Added SourceActionName info for HotSpots
		// (z.manning 2010-02-25 10:38) - PLID 37532 - SourceDetailImageStampID
		//TES 3/18/2010 - PLID 37530 - Added SourceActionName info for Smart Stamps
		// (a.walling 2010-04-02 17:44) - PLID 38059
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		rsTopic = CreateParamRecordset(pCon, "SELECT EmrTopicsT.ID, EmrTopicsT.Name, EmrTemplateTopicToTopicLinkT.EMRTemplateTopicID, "
			"EmrTopicsT.SourceActionID, EMRActionsT.SourceID AS SourceActionSourceID, EMRDataQ.EMRDataGroupID AS SourceActionSourceDataGroupID, "
			"EmrTopicsT.SourceDetailID, EmrTemplateTopicsT.SourceTemplateTopicID, EmrTopicsT.ShowIfEmpty, EmrTopicsT.EmrParentTopicID, "
			"EmrTemplateTopicsT.TemplateID, EmrTemplateTopicsT.OrderIndex AS TemplateOrderIndex, EMRTopicsT.OrderIndex, "
			"CASE WHEN EmrActionsT.SourceType = {CONST} THEN EmrInfoQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrDataQ.Data WHEN EmrActionsT.SourceType = {CONST} THEN EmrProcedureQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrImageHotSpotQ.AnatomicLocation "
			// (a.walling 2010-04-02 17:42) - PLID 38059 - Gather the action name for table dropdown spawns
			" WHEN EmrActionsT.SourceType = {CONST} THEN EmrDetailImageStampsQ.StampText + ' - ' + convert(nvarchar(50),EmrDetailImageStampsQ.IndexByStamp) WHEN EmrActionsT.SourceType = 13 THEN EmrTableDropdownQ.DropdownData + ' - ' + EmrTableDropdownRowQ.Data + ' - ' + EmrTableDropdownQ.DropdownTableColumn ELSE '' END AS SourceActionName, "
			"EmrActionsT.DestType AS SourceActionDestType, "
			"SourceTemplateTopics.TemplateID AS OriginalTemplateID, EmrTemplateTopicsT.SourceTemplateTopicID, " // (a.walling 2007-03-21 10:22) - PLID 25301 - Load original template id for spawned topics
			"EmrImageHotSpotQ.EmrSpotGroupID, EmrTopicsT.SourceDataGroupID, SourceType, EmrTopicsT.SourceDetailImageStampID, "
			// (a.walling 2008-06-30 13:57) - PLID 29271 - Preview Pane flags
			"EMRTopicsT.PreviewFlags, EmrTableDropdownQ.DropdownGroupID AS SourceActionSourceTableDropdownGroupID "
			"FROM EMRTopicsT "
			"LEFT JOIN EmrActionsT ON EmrActionsT.ID = EmrTopicsT.SourceActionID "
			"LEFT JOIN EmrTemplateTopicToTopicLinkT ON EmrTopicsT.ID = EmrTemplateTopicToTopicLinkT.EmrTopicID "
			"LEFT JOIN EmrTemplateTopicsT ON EmrTemplateTopicToTopicLinkT.EmrTemplateTopicID = EmrTemplateTopicsT.ID "
			"LEFT JOIN EMRTemplateTopicsT AS SourceTemplateTopics ON EmrTemplateTopicsT.SourceTemplateTopicID = SourceTemplateTopics.ID " // (a.walling 2007-03-21 10:35) - PLID 25301
			// (a.walling 2010-04-06 08:16) - PLID 38061 - Filtered these on their appropriate source types
			"LEFT JOIN (SELECT ID, Name FROM EmrInfoT) AS EmrInfoQ ON EmrInfoQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 3  "
			"LEFT JOIN (SELECT ID, Data, EMRDataGroupID  FROM EmrDataT) AS EmrDataQ ON EmrDataQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 4  "
			"LEFT JOIN (SELECT ID, Name FROM ProcedureT) AS EmrProcedureQ ON EmrProcedureQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 5  "
			"LEFT JOIN (SELECT EmrImageHotSpotsT.ID, EmrImageHotSpotsT.EmrSpotGroupID, "
			// (z.manning 2010-04-30 16:30) - PLID 37553 - Pull anatomic location from a view
			"EmrHotSpotAnatomicLocationQ.AnatomicLocation "
			"FROM EmrImageHotSpotsT "
			"LEFT JOIN EmrHotSpotAnatomicLocationQ ON EmrImageHotSpotsT.ID = EmrHotSpotAnatomicLocationQ.EmrHotSpotID) AS EmrImageHotSpotQ ON EmrImageHotSpotQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST}  "
			// (a.walling 2010-04-02 17:44) - PLID 38059
			"LEFT JOIN (SELECT EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Data AS DropdownData, EMRDataTableColumnQ.Data AS DropdownTableColumn "
				"FROM EmrTableDropdownInfoT "
				"INNER JOIN EMRDataT EMRDataTableColumnQ ON EMRTableDropdownInfoT.EMRDataID = EMRDataTableColumnQ.ID "
				"AND EMRDataTableColumnQ.ListType IN (3,4) " // (a.walling 2013-02-28 17:35) - PLID 55391 - This eliminates thousands of seeks!
				") AS EmrTableDropdownQ ON EmrTableDropdownQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST} "
			// (a.walling 2010-04-02 17:44) - PLID 38059
			"LEFT JOIN EMRDetailsT SourceEMRDetailQ ON EMRTopicsT.SourceDetailID = SourceEMRDetailQ.ID "
			"LEFT JOIN EMRDataT EmrTableDropdownRowQ ON EMRTopicsT.SourceDataGroupID = EMRTableDropdownRowQ.EmrDataGroupID AND SourceEMRDetailQ.EMRInfoID = EmrTableDropdownRowQ.EMRInfoID AND EmrActionsT.SourceType = 13 "			
			"LEFT JOIN (SELECT EmrDetailImageStampsT.ID, EmrImageStampsT.StampText, "
			" (SELECT Count(*) FROM EmrDetailImageStampsT OtherStamps WHERE OtherStamps.EmrDetailID = EmrDetailImageStampsT.EmrDetailID "
			" AND OtherStamps.EmrImageStampID = EmrDetailImageStampsT.EmrImageStampID "
			" AND OtherStamps.OrderIndex < EmrDetailImageStampsT.OrderIndex) + 1 AS IndexByStamp "
			" FROM EmrDetailImageStampsT INNER JOIN EmrImageStampsT ON EmrDetailImageStampsT.EmrImageStampID = EmrImageStampsT.ID "
			") EmrDetailImageStampsQ ON EmrTopicsT.SourceDetailImageStampID = EmrDetailImageStampsQ.ID "
			"WHERE EmrTopicsT.ID = (SELECT EmrTopicID FROM EmrDetailsT WHERE ID = {INT})",
				eaoEmrItem, eaoEmrDataItem, eaoProcedure, eaoEmrImageHotSpot, eaoSmartStamp, eaoEmrImageHotSpot, eaoEmrTableDropDownItem,
				pDetail->m_nEMRDetailID);

		ASSERT(!rsTopic->eof);
		CString strSourceActionName = AdoFldString(rsTopic, "SourceActionName", "");
		EmrActionObject eaoDestType = (EmrActionObject)AdoFldLong(rsTopic, "SourceActionDestType", -1);
		m_nID = AdoFldLong(rsTopic,"ID");
		m_strTopicName = AdoFldString(rsTopic, "Name","");
		m_nTemplateTopicID = AdoFldLong(rsTopic, "EMRTemplateTopicID",-1);
		m_nTemplateID = AdoFldLong(rsTopic, "TemplateID", -1);
		m_nTemplateTopicOrderIndex = AdoFldLong(rsTopic, "TemplateOrderIndex", -1);
		SetSourceActionID(AdoFldLong(rsTopic, "SourceActionID", -1), &strSourceActionName, &eaoDestType);
		// (z.manning 2009-03-05 10:55) - PLID 33338
		SetSourceDataGroupID(AdoFldLong(rsTopic, "SourceDataGroupID", -1));
		// (z.manning 2010-02-25 10:39) - PLID 37532
		m_sai.SetDetailStampID(AdoFldLong(rsTopic, "SourceDetailImageStampID", -1));
		m_sai.eaoSourceType = (EmrActionObject)AdoFldLong(rsTopic, "SourceType", eaoInvalid);
		//DRT 9/25/2007 - PLID 27515
		m_nSourceActionSourceID = AdoFldLong(rsTopic, "SourceActionSourceID", -1);
		m_nSourceActionSourceDataGroupID = AdoFldLong(rsTopic, "SourceActionSourceDataGroupID", -1);
		m_nSourceActionSourceHotSpotGroupID = AdoFldLong(rsTopic, "EmrSpotGroupID", -1);
		m_nSourceActionSourceTableDropdownGroupID = AdoFldLong(rsTopic, "SourceActionSourceTableDropdownGroupID", -1);
		// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
		SetSourceDetailID(AdoFldLong(rsTopic, "SourceDetailID", -1));
		m_nOriginalTemplateTopicID = AdoFldLong(rsTopic, "SourceTemplateTopicID", -1);
		m_nOriginalTemplateID = AdoFldLong(rsTopic, "OriginalTemplateID", -1); // (a.walling 2007-03-21 10:44) - PLID 25301
		m_bShowIfEmpty = AdoFldBool(rsTopic, "ShowIfEmpty");
		m_bHideOnEMN = FALSE;

		// (a.walling 2008-06-30 13:57) - PLID 29271 - Preview Pane flags
		m_nPreviewFlags = AdoFldLong(rsTopic, "PreviewFlags", 0);

		//save this data for later
		m_strLastSavedTopicName = m_strTopicName;
		//TES 5/3/2010 - PLID 24692 - Tell our position entry to remember its order
		if(m_pTopicPositionEntry) {
			m_pTopicPositionEntry->SetLastSavedOrderIndex();
		}
		m_bLastSavedShowIfEmpty = m_bShowIfEmpty;
	}

	ASSERT(!m_arypEMNDetails.GetSize());

	// (j.jones 2010-02-25 14:40) - PLID 37318 - ensure the SmartStamp links, if any, remain valid
	pDetail->ReconfirmSmartStampLinks();

	m_arypEMNDetails.Add(pDetail);
	m_bOwnChildren = FALSE;

	//Now, continue up the chain (we should own our parent).
	ASSERT(m_bOwnParent);
	long nParentTopicID = AdoFldLong(rsTopic, "EmrParentTopicID", -1);
	if(nParentTopicID == -1) {
		//Our parent is an EMN.
		m_pParentEMN = new CEMN(NULL);
		// (j.jones 2007-08-02 12:09) - PLID 26915 - pass in our connection
		m_pParentEMN->LoadFromTopic(this, pCon);
	}
	else {
		//Our parent is another topic.
		//TES 4/15/2010 - PLID 24692 - If we have an EMN, we should already have an entry for this ID.
		//TES 5/3/2010 - PLID 24692 - For "bottom-up" loading, we don't maintain position entries.
		TopicPositionEntry *tpe = NULL;
		m_pParentTopic = new CEMRTopic(m_bIsTemplate, TRUE, tpe);
		// (j.jones 2007-08-02 12:09) - PLID 26915 - pass in our connection
		m_pParentTopic->LoadFromSubTopic(this, pCon);
		m_pOriginalParentTopic = m_pParentTopic;
		if(m_pOriginalParentTopic)
			m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
	}
}

// (j.jones 2007-08-02 11:42) - PLID 26915 - added ability to pass in a connection
void CEMRTopic::LoadFromSubTopic(CEMRTopic *pSubTopic, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	m_bIsTemplate = pSubTopic->IsTemplate();
	_RecordsetPtr rsTopic;
	if(m_bIsTemplate) {
		//DRT 9/26/2007 - PLID 27515 - Added SourceActionSourceID, SourceActionSourceDataGroupID
		// (z.manning, 01/23/2008) - PLID 28690 - Added hot spot group id
		// (z.manning 2009-02-13 11:04) - PLID 33070 - SourceActionSourceTableDropdownGroupID
		// (z.manning 2009-03-05 14:04) - PLID 33338 - SourceDataGroupID
		//TES 2/16/2010 - PLID 37298 - Added SourceActionName info for HotSpots
		//TES 3/18/2010 - PLID 37530 - Added SourceActionName info for Smart Stamps
		// (a.walling 2010-04-02 17:44) - PLID 38059
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		rsTopic = CreateParamRecordset(pCon, "SELECT EmrTemplateTopicsT.ID, EmrTemplateTopicsT.Name, EmrTemplateTopicsT.ID AS EMRTemplateTopicID, "
			"EmrTemplateTopicsT.SourceActionID, EMRActionsT.SourceID AS SourceActionSourceID, EMRDataQ.EMRDataGroupID AS SourceActionSourceDataGroupID, "
			"EmrTemplateTopicsT.SourceDetailID, EmrTemplateTopicsT.SourceTemplateTopicID, EmrTemplateTopicsT.ShowIfEmpty, "
			"EmrTemplateTopicsT.EmrParentTemplateTopicID AS ParentTopicID, "
			"EmrTemplateTopicsT.TemplateID, EmrTemplateTopicsT.OrderIndex AS TemplateOrderIndex, EmrTemplateTopicsT.OrderIndex, EmrTemplateTopicsT.HideOnEMN, "
			"CASE WHEN EmrActionsT.SourceType = {CONST} THEN EmrInfoQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrDataQ.Data WHEN EmrActionsT.SourceType = {CONST} THEN EmrProcedureQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrImageHotSpotsQ.AnatomicLocation "
			// (a.walling 2010-04-02 17:45) - PLID 38059 - Gather the action name for table dropdown spawns
			" WHEN EmrActionsT.SourceType = {CONST} THEN EmrImageStampsT.StampText + ' - ' + convert(nvarchar(50),EmrTemplateTopicsT.SourceStampIndex) WHEN EmrActionsT.SourceType = 13 THEN EmrTableDropdownQ.DropdownData + ' - ' + EmrTableDropdownRowQ.Data + ' - ' + EmrTableDropdownQ.DropdownTableColumn ELSE '' END AS SourceActionName, "
			"EmrActionsT.DestType AS SourceActionDestType, EmrTemplateTopicsT.SourceDataGroupID, SourceType, "
			"SourceTemplateTopics.TemplateID AS OriginalTemplateID, " // (a.walling 2007-03-21 10:22) - PLID 25301 - Load original template id for spawned topics
			// (a.walling 2008-06-30 13:58) - PLID 29271 - Preview Pane flags
			"EmrSpotGroupID, EmrTemplateTopicsT.PreviewFlags, EmrTableDropdownQ.DropdownGroupID AS SourceActionSourceTableDropdownGroupID, "
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
			"NULL AS SourceDetailImageStampID, EmrTemplateTopicsT.SourceStampID, EmrTemplateTopicsT.SourceStampIndex "
			"FROM EmrTemplateTopicsT "
			"LEFT JOIN EmrActionsT ON EmrActionsT.ID = EmrTemplateTopicsT.SourceActionID "
			// (a.walling 2010-04-06 08:16) - PLID 38061 - Filtered these on their appropriate source types
			"LEFT JOIN (SELECT ID, Name FROM EmrInfoT) AS EmrInfoQ ON EmrInfoQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 3  "
			"LEFT JOIN (SELECT ID, Data, EMRDataGroupID FROM EmrDataT) AS EmrDataQ ON EmrDataQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 4  "
			"LEFT JOIN (SELECT ID, Name FROM ProcedureT) AS EmrProcedureQ ON EmrProcedureQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 5  "
			"LEFT JOIN (SELECT EmrImageHotSpotsT.ID, EmrImageHotSpotsT.EmrSpotGroupID, "
			// (z.manning 2010-04-30 16:31) - PLID 37553 - Pull anatomic location from a view
			"EmrHotSpotAnatomicLocationQ.AnatomicLocation "
			"FROM EmrImageHotSpotsT "
			"LEFT JOIN EmrHotSpotAnatomicLocationQ ON EmrImageHotSpotsT.ID = EmrHotSpotAnatomicLocationQ.EmrHotSpotID) AS EmrImageHotSpotsQ ON EmrImageHotSpotsQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST} "
			// (a.walling 2010-04-02 17:48) - PLID 38059
			"LEFT JOIN (SELECT EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Data AS DropdownData, EMRDataTableColumnQ.Data AS DropdownTableColumn "
				"FROM EmrTableDropdownInfoT "
				"INNER JOIN EMRDataT EMRDataTableColumnQ ON EMRTableDropdownInfoT.EMRDataID = EMRDataTableColumnQ.ID "
				"AND EMRDataTableColumnQ.ListType IN (3,4) " // (a.walling 2013-02-28 17:35) - PLID 55391 - This eliminates thousands of seeks!
				") AS EmrTableDropdownQ ON EmrTableDropdownQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST} "
			// (a.walling 2010-04-02 17:48) - PLID 38059
			"LEFT JOIN EMRTemplateDetailsT SourceEMRDetailQ ON EMRTemplateTopicsT.SourceDetailID = SourceEMRDetailQ.ID "
			"LEFT JOIN EMRInfoMasterT SourceEMRInfoMasterQ ON SourceEMRDetailQ.EMRInfoMasterID = SourceEMRInfoMasterQ.ID "
			"LEFT JOIN EMRDataT EmrTableDropdownRowQ ON EMRTemplateTopicsT.SourceDataGroupID = EMRTableDropdownRowQ.EmrDataGroupID AND SourceEMRInfoMasterQ.ActiveEMRInfoID = EmrTableDropdownRowQ.EMRInfoID AND EmrActionsT.SourceType = 13 "			
			"LEFT JOIN EMRTemplateTopicsT AS SourceTemplateTopics ON EmrTemplateTopicsT.SourceTemplateTopicID = SourceTemplateTopics.ID "
			"LEFT JOIN EmrImageStampsT ON EmrTemplateTopicsT.SourceStampID = EmrImageStampsT.ID "
			"WHERE EmrTemplateTopicsT.ID = (SELECT EmrParentTemplateTopicID FROM EmrTemplateTopicsT WHERE ID = {INT})",
				eaoEmrItem, eaoEmrDataItem, eaoProcedure, eaoEmrImageHotSpot, eaoSmartStamp, eaoEmrImageHotSpot, eaoEmrTableDropDownItem,
				pSubTopic->GetID());
		ASSERT(!rsTopic->eof);
		m_bHideOnEMN = AdoFldBool(rsTopic, "HideOnEMN");
	}
	else {
		//Load just the information relevant to this detail.
		//DRT 9/26/2007 - PLID 27515 - Added SourceActionSourceID, SourceActionSourceDataGroupID
		// (z.manning, 01/23/2008) - PLID 28690 - Added SourceActionSourceHotSpotGroupID
		// (z.manning 2009-02-13 11:06) - PLID 33070 - SourceActionSourceTableDropdownGroupID
		// (z.manning 2009-03-05 10:57) - PLID 33338 - SourceDataGroupID
		//TES 2/16/2010 - PLID 37298 - Added SourceActionName info for HotSpots
		// (z.manning 2010-02-25 10:39) - PLID 37532 - SourceDetailImageStampID
		//TES 3/18/2010 - PLID 37530 - Added SourceActionName info for Smart Stamps
		// (a.walling 2010-04-02 17:49) - PLID 38059
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		rsTopic = CreateParamRecordset(pCon, "SELECT EmrTopicsT.ID, EmrTopicsT.Name, EmrTemplateTopicToTopicLinkT.EMRTemplateTopicID, "
			"EmrTopicsT.SourceActionID, EMRActionsT.SourceID AS SourceActionSourceID, EMRDataQ.EMRDataGroupID AS SourceActionSourceDataGroupID, "
			"EmrTopicsT.SourceDetailID, EmrTemplateTopicsT.SourceTemplateTopicID, EmrTopicsT.ShowIfEmpty, "
			"EmrTopicsT.EmrParentTopicID AS ParentTopicID, "
			"EmrTemplateTopicsT.TemplateID, EmrTemplateTopicsT.OrderIndex AS TemplateOrderIndex, "
			"EmrTopicsT.OrderIndex, EmrTopicsT.SourceDataGroupID, SourceType, "
			"CASE WHEN EmrActionsT.SourceType = {CONST} THEN EmrInfoQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrDataQ.Data WHEN EmrActionsT.SourceType = {CONST} THEN EmrProcedureQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrImageHotSpotQ.AnatomicLocation "
			// (a.walling 2010-04-02 17:49) - PLID 38059 - Gather the action name for table dropdown spawns
			" WHEN EmrActionsT.SourceType = {CONST} THEN EmrDetailImageStampsQ.StampText + ' - ' + convert(nvarchar(50),EmrDetailImageStampsQ.IndexByStamp) WHEN EmrActionsT.SourceType = 13 THEN EmrTableDropdownQ.DropdownData + ' - ' + EmrTableDropdownRowQ.Data + ' - ' + EmrTableDropdownQ.DropdownTableColumn ELSE '' END AS SourceActionName, "
			"EmrActionsT.DestType AS SourceActionDestType, "
			"SourceTemplateTopics.TemplateID AS OriginalTemplateID, " // (a.walling 2007-03-21 10:22) - PLID 25301 - Load original template id for spawned topics
			// (a.walling 2008-06-30 13:58) - PLID 29271 - Preview Pane flags
			"EmrImageHotSpotQ.EmrSpotGroupID, EMRTopicsT.PreviewFlags, EmrTableDropdownQ.DropdownGroupID AS SourceActionSourceTableDropdownGroupID, "
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
			"EmrTopicsT.SourceDetailImageStampID, NULL AS SourceStampID, NULL AS SourceStampIndex "
			"FROM EMRTopicsT "
			"LEFT JOIN EmrTemplateTopicToTopicLinkT ON EmrTopicsT.ID = EmrTemplateTopicToTopicLinkT.EmrTopicID "
			"LEFT JOIN EmrTemplateTopicsT ON EmrTemplateTopicToTopicLinkT.EmrTemplateTopicID = EmrTemplateTopicsT.ID "
			"LEFT JOIN EMRTemplateTopicsT AS SourceTemplateTopics ON EmrTemplateTopicsT.SourceTemplateTopicID = SourceTemplateTopics.ID " // (a.walling 2007-03-21 10:35) - PLID 25301
			"LEFT JOIN EmrActionsT ON EmrActionsT.ID = EmrTopicsT.SourceActionID "
			// (a.walling 2010-04-06 08:16) - PLID 38061 - Filtered these on their appropriate source types
			"LEFT JOIN (SELECT ID, Name FROM EmrInfoT) AS EmrInfoQ ON EmrInfoQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 3  "
			"LEFT JOIN (SELECT ID, Data, EMRDataGroupID FROM EmrDataT) AS EmrDataQ ON EmrDataQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 4  "
			"LEFT JOIN (SELECT ID, Name FROM ProcedureT) AS EmrProcedureQ ON EmrProcedureQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 5  "
			"LEFT JOIN (SELECT EmrImageHotSpotsT.ID, EmrImageHotSpotsT.EmrSpotGroupID, "
			// (z.manning 2010-04-30 16:32) - PLID 37553 - Pull anatomic location from a view
			"EmrHotSpotAnatomicLocationQ.AnatomicLocation "
			"FROM EmrImageHotSpotsT "
			"LEFT JOIN EmrHotSpotAnatomicLocationQ ON EmrImageHotSpotsT.ID = EmrHotSpotAnatomicLocationQ.EmrHotSpotID) AS EmrImageHotSpotQ ON EmrImageHotSpotQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST}  "
			// (a.walling 2010-04-02 17:50) - PLID 38059
			"LEFT JOIN (SELECT EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Data AS DropdownData, EMRDataTableColumnQ.Data AS DropdownTableColumn "
				"FROM EmrTableDropdownInfoT "
				"INNER JOIN EMRDataT EMRDataTableColumnQ ON EMRTableDropdownInfoT.EMRDataID = EMRDataTableColumnQ.ID "
				"AND EMRDataTableColumnQ.ListType IN (3,4) " // (a.walling 2013-02-28 17:35) - PLID 55391 - This eliminates thousands of seeks!
				") AS EmrTableDropdownQ ON EmrTableDropdownQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST} "
			// (a.walling 2010-04-02 17:50) - PLID 38059
			"LEFT JOIN EMRDetailsT SourceEMRDetailQ ON EMRTopicsT.SourceDetailID = SourceEMRDetailQ.ID "
			"LEFT JOIN EMRDataT EmrTableDropdownRowQ ON EMRTopicsT.SourceDataGroupID = EMRTableDropdownRowQ.EmrDataGroupID AND SourceEMRDetailQ.EMRInfoID = EmrTableDropdownRowQ.EMRInfoID AND EmrActionsT.SourceType = 13 "
			"LEFT JOIN (SELECT EmrDetailImageStampsT.ID, EmrImageStampsT.StampText, "
			" (SELECT Count(*) FROM EmrDetailImageStampsT OtherStamps WHERE OtherStamps.EmrDetailID = EmrDetailImageStampsT.EmrDetailID "
			" AND OtherStamps.EmrImageStampID = EmrDetailImageStampsT.EmrImageStampID "
			" AND OtherStamps.OrderIndex < EmrDetailImageStampsT.OrderIndex) + 1 AS IndexByStamp "
			" FROM EmrDetailImageStampsT INNER JOIN EmrImageStampsT ON EmrDetailImageStampsT.EmrImageStampID = EmrImageStampsT.ID "
			") EmrDetailImageStampsQ ON EmrTopicsT.SourceDetailImageStampID = EmrDetailImageStampsQ.ID "
			"WHERE EmrTopicsT.ID = (SELECT EmrParentTopicID FROM EmrTopicsT WHERE ID = {INT})",
				eaoEmrItem, eaoEmrDataItem, eaoProcedure, eaoEmrImageHotSpot, eaoSmartStamp, eaoEmrImageHotSpot, eaoEmrTableDropDownItem,
				pSubTopic->GetID());
		ASSERT(!rsTopic->eof);
		m_bHideOnEMN = FALSE;
	}
	CString strSourceActionName = AdoFldString(rsTopic, "SourceActionName", "");
	EmrActionObject eaoDestType = (EmrActionObject)AdoFldLong(rsTopic, "SourceActionDestType", -1);
	m_nID = AdoFldLong(rsTopic,"ID");
	m_strTopicName = AdoFldString(rsTopic, "Name","");
	m_nTemplateTopicID = AdoFldLong(rsTopic, "EMRTemplateTopicID",-1);
	m_nTemplateID = AdoFldLong(rsTopic, "TemplateID", -1);
	m_nTemplateTopicOrderIndex = AdoFldLong(rsTopic, "TemplateOrderIndex", -1);
	SetSourceActionID(AdoFldLong(rsTopic, "SourceActionID", -1), &strSourceActionName, &eaoDestType);
	// (z.manning 2010-02-25 10:40) - PLID 37532 - SourceDetailImageStampID
	m_sai.SetDetailStampID(AdoFldLong(rsTopic, "SourceDetailImageStampID", -1));
	//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
	m_sai.SetGlobalStampIDAndIndex(AdoFldLong(rsTopic, "SourceStampID", -1), AdoFldLong(rsTopic, "SourceStampIndex", -1));
	SetSourceDataGroupID(AdoFldLong(rsTopic, "SourceDataGroupID", -1));
	m_sai.eaoSourceType = (EmrActionObject)AdoFldLong(rsTopic, "SourceType", eaoInvalid);
	//DRT 9/25/2007 - PLID 27515
	m_nSourceActionSourceID = AdoFldLong(rsTopic, "SourceActionSourceID", -1);
	m_nSourceActionSourceDataGroupID = AdoFldLong(rsTopic, "SourceActionSourceDataGroupID", -1);
	m_nSourceActionSourceHotSpotGroupID = AdoFldLong(rsTopic, "EmrSpotGroupID", -1);
	// (z.manning 2009-02-13 11:08) - PLID 33070 - SourceActionSourceTableDropdownGroupID
	m_nSourceActionSourceTableDropdownGroupID = AdoFldLong(rsTopic, "SourceActionSourceTableDropdownGroupID", -1);
	// (j.jones 2007-01-11 10:32) - PLID 24027 - supported SourceDetailID
	SetSourceDetailID(AdoFldLong(rsTopic, "SourceDetailID", -1));
	m_nOriginalTemplateTopicID = AdoFldLong(rsTopic, "SourceTemplateTopicID", -1);
	m_nOriginalTemplateID = AdoFldLong(rsTopic, "OriginalTemplateID", -1); // (a.walling 2007-03-21 10:36) - PLID 25301
	m_bShowIfEmpty = AdoFldBool(rsTopic, "ShowIfEmpty");

	// (a.walling 2008-06-30 13:58) - PLID 29271 - Preview Pane flags
	m_nPreviewFlags = AdoFldLong(rsTopic, "PreviewFlags", 0);

	ASSERT(!m_arypEMRTopics.GetSize());
	m_arypEMRTopics.Add(pSubTopic);
	m_bOwnChildren = FALSE;

	//save this data for later
	m_strLastSavedTopicName = m_strTopicName;
	//TES 5/3/2010 - PLID 24692 - Tell our position entry to remember its order
	if(m_pTopicPositionEntry) {
		m_pTopicPositionEntry->SetLastSavedOrderIndex();
	}
	m_bLastSavedShowIfEmpty = m_bShowIfEmpty;

	//Now, continue up the chain (we should own our parent).
	ASSERT(m_bOwnParent);
	long nParentTopicID = AdoFldLong(rsTopic, "ParentTopicID", -1);
	if(nParentTopicID == -1) {
		//Our parent is an EMN.
		m_pParentEMN = new CEMN(NULL);
		// (j.jones 2007-08-02 12:09) - PLID 26915 - pass in our connection
		m_pParentEMN->LoadFromTopic(this, pCon);
	}
	else {
		//Our parent is another topic.
		//TES 4/15/2010 - PLID 24692 - If we have an EMN, we should already have a position entry for this topic.
		//TES 5/3/2010 - PLID 24692 - For "bottom-up" loading, we don't maintain position entries.
		TopicPositionEntry *tpe = NULL;
		m_pParentTopic = new CEMRTopic(m_bIsTemplate, TRUE, tpe);
		// (j.jones 2007-08-02 12:09) - PLID 26915 - pass in our connection
		m_pParentTopic->LoadFromSubTopic(this, pCon);
		m_pOriginalParentTopic = m_pParentTopic;
		if(m_pOriginalParentTopic)
			m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
	}
}

long CEMRTopic::GetSpawnedGroupID()
{
	if(GetSourceActionID() == -1) {
		return -1;
	}

	//This is only valid if we came from a template
	if(m_nOriginalTemplateTopicID == -1) {
		return -1;
	}

	if(m_SourceActionDestType == eaoInvalid) {
		//We need to look it up.
		SetSourceActionDestType((EmrActionObject)VarLong(GetTableField("EmrActionsT","DestType","ID",m_sai.nSourceActionID),eaoInvalid));
	}
	if(m_SourceActionDestType == eaoMintItems) {
		return m_sai.nSourceActionID;
	}
	else {
		return -1;
	}
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - IsDisplayed for total UI visibility
// if not visible, false; else if template, true; else if not empty or ShowIfEmpty, true; else false
bool CEMRTopic::IsDisplayed(CEMNDetail *pDetailToIgnore, BOOL bIgnoreBlankSubtopics, OPTIONAL CEMRTopic* pTopicToIgnore)
{
	if (!this) return false;

	if (!GetVisible()) {
		return false;
	}

	if (!IsTemplate() && !ShowIfEmpty(FALSE) && IsEmpty(pDetailToIgnore, bIgnoreBlankSubtopics, pTopicToIgnore)) {
		return false;
	}

	// checking the parent, do not check for empty subtopics since we already know this topic is displayed on its own
	return IsParentDisplayed(pDetailToIgnore, FALSE);
}

bool CEMRTopic::IsParentDisplayed(CEMNDetail *pDetailToIgnore, BOOL bIgnoreBlankSubtopics)
{
	if (!GetParentTopic()) {
		return true;
	}

	return GetParentTopic()->IsDisplayed(pDetailToIgnore, bIgnoreBlankSubtopics, this);
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns NULL if no new topic can be found; never returns this
CEMRTopic* CEMRTopic::GetNextTreeTopic()
{
	if (!this) return NULL;

	CEMRTopic* pTopic = NULL;	

	if (pTopic = GetFirstChildTopic()) {
		return pTopic;
	}

	if (pTopic = GetNextSiblingTopic()) {
		return pTopic;
	}

	if (CEMRTopic* pParentTopic = GetParentTopic()) {
		do {
			pTopic = pParentTopic->GetNextSiblingTopic();
			pParentTopic = pParentTopic->GetParentTopic();
		} while (!pTopic && pParentTopic);
	}

	return pTopic;
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns NULL if no new topic can be found; never returns this
CEMRTopic* CEMRTopic::GetPrevTreeTopic()
{
	if (!this) return NULL;

	CEMRTopic* pPrev = NULL;

	if (pPrev = GetPrevSiblingTopic()) {
		return pPrev->GetDeepestLastTreeTopic();
	}

	if (GetParentTopic()) {
		return GetParentTopic();
	}

	return NULL;
}

CEMRTopic* CEMRTopic::GetNextDisplayedTreeTopic(CEMNDetail *pDetailToIgnore, BOOL bIgnoreBlankSubtopics, CEMRTopic* pTopicToIgnore)
{
	return GetNextTreeTopicWhere(boost::bind(&CEMRTopic::IsDisplayed, _1, pDetailToIgnore, bIgnoreBlankSubtopics, pTopicToIgnore));
}

CEMRTopic* CEMRTopic::GetPrevDisplayedTreeTopic(CEMNDetail *pDetailToIgnore, BOOL bIgnoreBlankSubtopics, CEMRTopic* pTopicToIgnore)
{
	return GetPrevTreeTopicWhere(boost::bind(&CEMRTopic::IsDisplayed, _1, pDetailToIgnore, bIgnoreBlankSubtopics, pTopicToIgnore));
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns next sibling topic, or NULL
CEMRTopic* CEMRTopic::GetNextSiblingTopic()
{
	if (!this) return NULL;
	using namespace boost;
	CTopicArray& topics = GetParentTopics();

	range_iterator<CTopicArray>::type it = find(topics, this);
	if (it == end(topics) || ++it == end(topics)) {
		return NULL;
	}

	return *it;
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns prev sibling topic, or NULL
CEMRTopic* CEMRTopic::GetPrevSiblingTopic()
{	
	if (!this) return NULL;
	using namespace boost;
	CTopicArray& topics = GetParentTopics();

	range_iterator<CTopicArray>::type it = find(topics, this);
	if (it == end(topics) || it == begin(topics)) {
		return NULL;
	}

	return *--it;
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns first child topic, or NULL
CEMRTopic* CEMRTopic::GetFirstChildTopic()
{
	if (!this) return NULL;
	CTopicArray& subtopics = GetSubtopics();
	if (subtopics.IsEmpty()) return NULL;
	return subtopics[0];
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns last child topic, or NULL
CEMRTopic* CEMRTopic::GetLastChildTopic()
{
	if (!this) return NULL;
	CTopicArray& subtopics = GetSubtopics();
	if (subtopics.IsEmpty()) return NULL;
	return subtopics[subtopics.GetSize() - 1];
}

// (a.walling 2012-07-09 12:35) - PLID 51441 - Returns the last, deepest child topic, recursively, or this
CEMRTopic* CEMRTopic::GetDeepestLastTreeTopic()
{
	if (!this) return NULL;

	CEMRTopic* pDeepest = GetLastChildTopic();

	while (pDeepest) {
		if (CEMRTopic* pChildDeepest = pDeepest->GetLastChildTopic()) {
			pDeepest = pChildDeepest;
		} else {
			return pDeepest;
		}
	}

	return this;
}

CTopicArray& CEMRTopic::GetParentTopics()
{
	if (CEMRTopic* pParent = GetParentTopic()) {
		return pParent->GetSubtopics();
	} else if (CEMN* pEMN = GetParentEMN()) {
		return pEMN->GetSubtopics();
	} else {
		ASSERT(FALSE);
		return GetEmptyTopicSet();
	}
}

void CEMRTopic::SetHideOnEMN(BOOL bHide)
{
	m_bHideOnEMN = bHide;
	SetUnsaved();
}

// (a.walling 2007-09-27 14:01) - PLID 25548 - Option to retain the order index
void CEMRTopic::SetNew(BOOL bRetainOrderIndex /*=FALSE*/)
{
	// (a.walling 2007-06-28 12:10) - PLID 26494 - We need to ensure our variables are valid
	EnsureLoaded();
	
	m_nID = -1;

	//Tell our subtopics.
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		m_arypEMRTopics[i]->SetNew(bRetainOrderIndex);
	}
	//Tell our details.
	for(i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		m_arypEMNDetails[i]->SetNew();
	}

	for(i = 0; i < m_aryDeletedTopics.GetSize(); i++) {
		delete m_aryDeletedTopics[i];
	}
	m_aryDeletedTopics.RemoveAll();

	for(i = 0; i < m_aryDeletedDetails.GetSize(); i++) {
		// (c.haag 2007-05-24 16:58) - PLID 26095 - Use Release() instead of delete
		//m_aryDeletedDetails[i]->Release();
		// (a.walling 2009-10-12 16:05) - PLID 36024
		m_aryDeletedDetails[i]->__Release("CEMRTopic::SetNew deleted details");
	}
	m_aryDeletedDetails.RemoveAll();

	// (j.jones 2008-07-23 14:18) - PLID 30789 - set all problems as new,
	// and reconfirm their pointers are correct with this object
	// (z.manning 2009-05-22 10:01) - PLID 34297 - We just have problem links now
	for(i = 0; i < m_apEmrProblemLinks.GetSize(); i++)
	{
		CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(i);
		if(pProblemLink != NULL) {
			pProblemLink->SetID(-1);
			pProblemLink->SetID(-1);
			pProblemLink->UpdatePointersWithTopic(this);
		}
	}

	// (a.walling 2007-06-28 11:59) - PLID 26494 - If we have a source detail, we need to ensure we have
	// a pointer to the actual source object and then reset the ID to -1. That way the GenerateSaveString knows
	// that we need to update the sourcedetail to the ID of the new detail object.
	// (a.walling 2007-06-28 16:43) - PLID 26494 - Actually, we will do this in UpdateSourceDetailPointers, and
	// that way we can guarantee that all topics have been loaded.
	/*if (m_nSourceDetailID != -1 && m_pSourceDetail == NULL && GetParentEMN() != NULL) {
		VERIFY(m_pSourceDetail = m_bIsTemplate ? GetParentEMN()->GetDetailByTemplateDetailID(m_nSourceDetailID) : GetParentEMN()->GetDetailByID(m_nSourceDetailID));
		if (m_pSourceDetail) {
			// only clear this ID if we got the pointer.
			m_nSourceDetailID = -1;
		}
	}*/
}

void CEMRTopic::operator =(CEMRTopic &etSource)
{
	etSource.EnsureLoaded();
	//Do we already have a parent?
	if(!m_pParentEMN && !m_pParentTopic) {
		//Nope, use the source's parent.
		m_pParentEMN = etSource.m_pParentEMN;
		m_pParentTopic = etSource.m_pParentTopic;
		m_pOriginalParentTopic = etSource.m_pOriginalParentTopic;
		if(m_pOriginalParentTopic)
			m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
	}
	m_bOwnParent = FALSE;
	m_bOwnChildren = TRUE;

	long m_nID = etSource.m_nID;
	m_bIsTemplate = etSource.m_bIsTemplate;

	m_strTopicName = etSource.m_strTopicName;
	m_nTemplateTopicID = etSource.m_nTemplateTopicID;
	m_nTemplateID = etSource.m_nTemplateID;
	m_nTemplateTopicOrderIndex = etSource.m_nTemplateTopicOrderIndex;
	m_nOriginalTemplateTopicID = etSource.m_nOriginalTemplateTopicID;
	m_nOriginalTemplateID = etSource.m_nOriginalTemplateID; // (a.walling 2007-03-21 10:44) - PLID 25301
	
	m_strLastSavedTopicName = etSource.m_strTopicName;
	if(m_pTopicPositionEntry && etSource.m_pTopicPositionEntry) {
		//TES 5/4/2010 - PLID 24692 - Copy LastSavedOrderIndex
		m_pTopicPositionEntry->nLastSavedOrderIndex = etSource.m_pTopicPositionEntry->GetSortOrder();
	}
	m_bLastSavedShowIfEmpty = etSource.m_bShowIfEmpty;

	for(int i = 0; i < etSource.m_arypEMNDetails.GetSize(); i++) {
		// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
		CEMNDetail *pDetail = CEMNDetail::CreateDetail(this, "EMRTopic operator=");
		//Make sure our source detail has loaded its values.
		etSource.m_arypEMNDetails[i]->LoadContent();
		pDetail->CopyFrom(*(etSource.m_arypEMNDetails[i])); // (a.walling 2012-10-31 17:17) - PLID 53550 - CEMNDetail::opreator= leads to very confusing semantics

		// (j.jones 2010-02-25 14:40) - PLID 37318 - do not reconfirm SmartStamp links here,
		// instead the calling EMN needs to call ReconfirmSmartStampLinks_PostEMNCopy()
		//pDetail->ReconfirmSmartStampLinks();

		m_arypEMNDetails.Add(pDetail);
		//TES 1/23/2008 - PLID 24157 - Renamed.
		GetParentEMN()->HandleDetailChange(pDetail);
		GetParentEMN()->UpdateMergeConflicts(pDetail->GetMergeFieldName(TRUE));
	}
	for(i = 0; i < etSource.m_arypEMRTopics.GetSize(); i++) {
		TopicPositionEntry *tpe = new TopicPositionEntry;
		CEMRTopic *pTopic = new CEMRTopic(this, tpe);
		m_arypEMRTopics.Add(pTopic);
		*pTopic = *(etSource.m_arypEMRTopics[i]);
		//TES 4/20/2010 - PLID 24692 - Now make sure our topic position is in the right spot in the list.
		m_pTopicPositionEntry->pChild = AddTopicPositionEntryAtEnd(m_pTopicPositionEntry->pChild, tpe, m_pTopicPositionEntry);
	}
	
	//TES 3/18/2010 - PLID 37530 - Pass in the index of the spawning stamp, in order to properly calculate the name.
	SetSourceActionID(etSource.m_sai.nSourceActionID, etSource.m_sai.GetStampIndexInDetailByType(), &(etSource.m_SourceActionDestType));
	//DRT 9/25/2007 - PLID 27515
	m_nSourceActionSourceID = etSource.m_nSourceActionSourceID;
	m_nSourceActionSourceDataGroupID = etSource.m_nSourceActionSourceDataGroupID;
	m_nSourceActionSourceHotSpotGroupID = etSource.m_nSourceActionSourceHotSpotGroupID;
	m_nSourceActionSourceTableDropdownGroupID = etSource.m_nSourceActionSourceTableDropdownGroupID; // (z.manning 2009-02-13 11:08) - PLID 33070

	//TES 3/16/2010 - PLID 37530 - Just copy the m_sai struct, rather than copying each member individually.
	m_sai = etSource.m_sai;

	m_bShowIfEmpty = etSource.m_bShowIfEmpty;

	m_bVisible = etSource.m_bVisible;

	m_bHideOnEMN = etSource.m_bHideOnEMN;

	m_bHasSubTopics = etSource.m_bHasSubTopics;
	m_bHasDetails = etSource.m_bHasDetails;

	m_CompletionStatus = etSource.m_CompletionStatus;

	m_bDetailsLoaded = etSource.m_bDetailsLoaded;
	m_bTopicsLoaded = etSource.m_bTopicsLoaded;
	m_bPostLoadCalled = etSource.m_bPostLoadCalled;
	m_bPostLoadComplete = etSource.m_bPostLoadComplete;

	m_bHasMoved = etSource.m_bHasMoved;

	// (a.walling 2008-06-30 13:59) - PLID 29271 - Preview Pane flags
	m_nPreviewFlags = etSource.m_nPreviewFlags;

	// (j.jones 2008-07-23 14:18) - PLID 30789 - problems are now stored in an array of objects
	// (c.haag 2009-05-16 14:08) - PLID 34311 - Copy problem link objects
	for(i = 0; i < etSource.m_apEmrProblemLinks.GetSize(); i++) {
		// (c.haag 2009-07-09 10:49) - PLID 34829 - Now that the parent EMR is responsible for
		// problem allocation, we must try to ensure that the new link is bound to a problem for
		// this EMR. Determine the EMR that owns this topic, and pass it into the EMR problem
		// link ctor so it can do this. (If NULL, we won't try to enforce this)
		CEMR* pOwningEMR = (NULL == GetParentEMN()) ? NULL : GetParentEMN()->GetParentEMR();
		CEmrProblemLink *pNewLink = new CEmrProblemLink(etSource.m_apEmrProblemLinks[i], pOwningEMR);
		pNewLink->UpdatePointersWithTopic(this);
		m_apEmrProblemLinks.Add(pNewLink);
	}
}

// (a.walling 2007-10-18 17:27) - PLID 27664 - Moved IsTopicInArray to emrutils

//this will calculate if we need to save topics in a given order,
//and populates the array with that desired saving order
BOOL CEMRTopic::CalculateTopicSavingOrderNeeded(CArray<CEMRTopic*,CEMRTopic*> &aryEMRTopicsToSave, CArray<CEMRTopic*,CEMRTopic*> &aryTopicsAlreadySaved /*= CArray<CEMRTopic*,CEMRTopic*>()*/)
{
	//this function is necessary because if we drag a detail on an existing EMN
	//from Topic A to Topic B, where Topic B appears later in the EMN than Topic A,
	//then we delete Topic A, the save will try to remove Topic A while the detail
	//is still linked to it in data because Topic B has not been saved. Same for subtopics.

	//To solve this, we have to save other topics at the same time, in a given priority.
	//This function calculates that priority, relative to the current topic, and includes
	//the current topic in the array.

	// (j.jones 2010-06-28 14:13) - PLID 39114 - throw an exception if this is called on a NULL topic
	if(this == NULL) {
		ThrowNxException("CEMRTopic::CalculateTopicSavingOrderNeeded called with a NULL topic");
	}

	//TES 4/4/2006 - First, we're definitely saving this topic, so add it to the list for any topics we recurse to.
	aryTopicsAlreadySaved.Add(this);

	//First, see if this topic (or subtopics) is the original parent for any other detail or subtopic
	CArray<CEMRTopic*,CEMRTopic*> aryTopicsToSaveBefore;

	{
		//Check for 1. Deleted details that moved, 2. Deleted topics that moved, 3. Details that moved, 4. Topics that moved

		{
			//1a. get a list of all deleted details on the EMN
			CArray<CEMNDetail*,CEMNDetail*> aryTotalEMNDeletedDetails;
			GetParentEMN()->GenerateTotalEMNDeletedDetailArray(&aryTotalEMNDeletedDetails);

			//1b. for each deleted detail on the EMN
			for(int i=0;i<aryTotalEMNDeletedDetails.GetSize();i++) {
				CEMNDetail *pDetail = aryTotalEMNDeletedDetails.GetAt(i);
				//see if the original parent is this topic, but the current parent is not
				// (j.jones 2010-06-28 14:13) - PLID 39114 - if we have no current parent,
				// then we do not need to try and save it
				if(pDetail->m_pParentTopic != NULL && pDetail->m_pOriginalParentTopic == this && pDetail->m_pParentTopic != this) {

					//now we have to save the current parent, which means we have to run this function on that parent
					CArray<CEMRTopic*,CEMRTopic*> aryParentTopicsToSaveBefore;					
					if(!IsTopicInArray(aryTopicsAlreadySaved, pDetail->m_pParentTopic) &&						
						pDetail->m_pParentTopic->CalculateTopicSavingOrderNeeded(aryParentTopicsToSaveBefore, aryTopicsAlreadySaved)) {
					}
					aryTopicsToSaveBefore.Append(aryParentTopicsToSaveBefore);
				}
			}
		}

		{
			//2a. get a list of all deleted topics on the EMN
			CArray<CEMRTopic*,CEMRTopic*> aryTotalEMNDeletedTopics;
			GetParentEMN()->GetAllDeletedTopics(aryTotalEMNDeletedTopics);

			//2b. for each deleted topic on the EMN
			for(int i=0; i<aryTotalEMNDeletedTopics.GetSize(); i++) {
				CEMRTopic *pTopic = aryTotalEMNDeletedTopics.GetAt(i);

				//see if the original parent is this topic, but the current parent is not
				// (j.jones 2010-06-28 14:13) - PLID 39114 - if we have no current parent,
				// then we do not need to try and save it
				if(pTopic->m_pParentTopic != NULL && pTopic->m_pOriginalParentTopic == this && pTopic->m_pParentTopic != this) {

					//now we have to save the current parent, which means we have to run this function on that parent
					CArray<CEMRTopic*,CEMRTopic*> aryParentTopicsToSaveBefore;
					if(!IsTopicInArray(aryTopicsAlreadySaved, pTopic->m_pParentTopic) &&						
						pTopic->m_pParentTopic->CalculateTopicSavingOrderNeeded(aryParentTopicsToSaveBefore, aryTopicsAlreadySaved)) {
					}
					aryTopicsToSaveBefore.Append(aryParentTopicsToSaveBefore);
				}
			}
		}

		{
			//3a. get a list of all details on the EMN
			CArray<CEMNDetail*,CEMNDetail*> aryTotalEMNDetails;
			GetParentEMN()->GenerateTotalEMNDetailArray(&aryTotalEMNDetails);

			//3b. for each detail on the EMN
			for(int i=0;i<aryTotalEMNDetails.GetSize();i++) {
				CEMNDetail *pDetail = aryTotalEMNDetails.GetAt(i);
				//see if the original parent is this topic, but the current parent is not
				// (j.jones 2010-06-28 14:13) - PLID 39114 - if we have no current parent,
				// then we do not need to try and save it
				if(pDetail->m_pParentTopic != NULL && pDetail->m_pOriginalParentTopic == this && pDetail->m_pParentTopic != this) {

					//now we have to save the current parent, which means we have to run this function on that parent
					CArray<CEMRTopic*,CEMRTopic*> aryParentTopicsToSaveBefore;
					if(!IsTopicInArray(aryTopicsAlreadySaved, pDetail->m_pParentTopic) &&						
						pDetail->m_pParentTopic->CalculateTopicSavingOrderNeeded(aryParentTopicsToSaveBefore, aryTopicsAlreadySaved)) {
					}
					aryTopicsToSaveBefore.Append(aryParentTopicsToSaveBefore);
				}
			}
		}

		{
			//4a. get a list of all topics on the EMN
			CArray<CEMRTopic*,CEMRTopic*> aryTotalEMNTopics;
			GetParentEMN()->GetAllTopics(aryTotalEMNTopics);

			//4b. for each topic on the EMN
			for(int i=0; i<aryTotalEMNTopics.GetSize(); i++) {
				CEMRTopic *pTopic = aryTotalEMNTopics.GetAt(i);

				//see if the original parent is this topic, but the current parent is not
				// (j.jones 2010-06-28 14:13) - PLID 39114 - if we have no current parent,
				// then we do not need to try and save it
				if(pTopic->m_pParentTopic != NULL && pTopic->m_pOriginalParentTopic == this && pTopic->m_pParentTopic != this) {

					//now we have to save the current parent, which means we have to run this function on that parent
					CArray<CEMRTopic*,CEMRTopic*> aryParentTopicsToSaveBefore;
					if(!IsTopicInArray(aryTopicsAlreadySaved, pTopic->m_pParentTopic) &&
						pTopic->m_pParentTopic->CalculateTopicSavingOrderNeeded(aryParentTopicsToSaveBefore, aryTopicsAlreadySaved)) {
					}
					aryTopicsToSaveBefore.Append(aryParentTopicsToSaveBefore);
				}
			}
		}
	}

	//Second, see if this topic (or subtopics) has any details dragged from elsewhere.
	CArray<CEMRTopic*,CEMRTopic*> aryTopicsToSaveAfter;

	{
		//Check for 1. Deleted details that moved, 2. Deleted topics that moved, 3. Details that moved, 4. Topics that moved, , 5. Topics with details that are linked to details on our topic.

		{
			//1a. get a list of all deleted details underneath this topic or any of its subtopics
			CArray<CEMNDetail*,CEMNDetail*> aryTotalTopicDeletedDetails;
			GenerateEMNDeletedDetailArray(&aryTotalTopicDeletedDetails);
			
			//1b. for each detail underneath this topic or any of its subtopics
			for(int i=0;i<aryTotalTopicDeletedDetails.GetSize();i++) {
				CEMNDetail *pDetail = aryTotalTopicDeletedDetails.GetAt(i);

				//see if the parent is different
				// (j.jones 2010-06-28 14:13) - PLID 39114 - if we have no original parent,
				// then we do not need to try and save it
				if(pDetail->m_pOriginalParentTopic != NULL && pDetail->m_pOriginalParentTopic != pDetail->m_pParentTopic) {

					//it is, so now we have to save that parent, which means we have to run this function on that parent
					CArray<CEMRTopic*,CEMRTopic*> aryParentTopicsToSaveAfter;
					if(!IsTopicInArray(aryTopicsAlreadySaved, pDetail->m_pOriginalParentTopic) &&
						pDetail->m_pOriginalParentTopic->CalculateTopicSavingOrderNeeded(aryParentTopicsToSaveAfter, aryTopicsAlreadySaved)) {
					}
					aryTopicsToSaveAfter.Append(aryParentTopicsToSaveAfter);
				}
			}
		}

		{
			//2a. get a list of all deleted subtopics underneath this topic or any of its subtopics
			CArray<CEMRTopic*,CEMRTopic*> aryTotalTopicDeletedSubtopics;
			GetParentEMN()->AddDeletedTopicToList(this, aryTotalTopicDeletedSubtopics);
			
			//2b. for each deleted subtopic underneath this topic or any of its subtopics
			for(int i=0;i<aryTotalTopicDeletedSubtopics.GetSize();i++) {
				CEMRTopic *pTopic = aryTotalTopicDeletedSubtopics.GetAt(i);

				//see if the parent is different
				// (j.jones 2010-06-28 14:13) - PLID 39114 - if we have no original parent,
				// then we do not need to try and save it
				if(pTopic->m_pOriginalParentTopic != NULL && pTopic->m_pOriginalParentTopic != pTopic->m_pParentTopic) {

					//it is, so now we have to save that parent, which means we have to run this function on that parent
					CArray<CEMRTopic*,CEMRTopic*> aryParentTopicsToSaveAfter;
					if(!IsTopicInArray(aryTopicsAlreadySaved, pTopic->m_pOriginalParentTopic) &&
						pTopic->m_pOriginalParentTopic->CalculateTopicSavingOrderNeeded(aryParentTopicsToSaveAfter, aryTopicsAlreadySaved)) {
					}
					aryTopicsToSaveAfter.Append(aryParentTopicsToSaveAfter);
				}
			}
		}

		{
			//3a. get a list of all details underneath this topic or any of its subtopics
			CArray<CEMNDetail*,CEMNDetail*> aryTotalTopicDetails;
			GenerateEMNDetailArray(&aryTotalTopicDetails);
			
			//3b. for each detail underneath this topic or any of its subtopics
			for(int i=0;i<aryTotalTopicDetails.GetSize();i++) {
				CEMNDetail *pDetail = aryTotalTopicDetails.GetAt(i);

				//see if the parent is different
				// (j.jones 2010-06-28 14:13) - PLID 39114 - if we have no original parent,
				// then we do not need to try and save it
				if(pDetail->m_pOriginalParentTopic != NULL && pDetail->m_pOriginalParentTopic != pDetail->m_pParentTopic) {

					//it is, so now we have to save that parent, which means we have to run this function on that parent
					CArray<CEMRTopic*,CEMRTopic*> aryParentTopicsToSaveAfter;
					if(!IsTopicInArray(aryTopicsAlreadySaved, pDetail->m_pOriginalParentTopic) &&
						pDetail->m_pOriginalParentTopic->CalculateTopicSavingOrderNeeded(aryParentTopicsToSaveAfter, aryTopicsAlreadySaved)) {
					}
					aryTopicsToSaveAfter.Append(aryParentTopicsToSaveAfter);
				}
			}
		}

		{
			//4a. get a list of all subtopics underneath this topic or any of its subtopics
			CArray<CEMRTopic*,CEMRTopic*> aryTotalTopicSubtopics;
			for(int i=0; i<m_arypEMRTopics.GetSize(); i++)
				GetParentEMN()->AddTopicToList(m_arypEMRTopics.GetAt(i), aryTotalTopicSubtopics);
			
			//4b. for each subtopic underneath this topic or any of its subtopics
			for(i=0;i<aryTotalTopicSubtopics.GetSize();i++) {
				CEMRTopic *pTopic = aryTotalTopicSubtopics.GetAt(i);

				//see if the parent is different
				// (j.jones 2010-06-28 14:13) - PLID 39114 - if we have no original parent,
				// then we do not need to try and save it
				if(pTopic->m_pOriginalParentTopic != NULL && pTopic->m_pOriginalParentTopic != pTopic->m_pParentTopic) {
					//it is, so now we have to save that parent, which means we have to run this function on that parent
					CArray<CEMRTopic*,CEMRTopic*> aryParentTopicsToSaveAfter;

					if(!IsTopicInArray(aryTopicsAlreadySaved, pTopic->m_pOriginalParentTopic) &&
						pTopic->m_pOriginalParentTopic->CalculateTopicSavingOrderNeeded(aryParentTopicsToSaveAfter, aryTopicsAlreadySaved)) {
					}
					aryTopicsToSaveAfter.Append(aryParentTopicsToSaveAfter);
				}
			}
		}

		{
			//5. look for details linked to or from our details.
			long nCount = GetParentEMN()->GetTotalDetailCount();
			for(int i = 0; i < nCount; i++) {
				CEMNDetail *pDetail = GetParentEMN()->GetDetail(i);
				CArray<CEMNDetail*,CEMNDetail*> arLinked;
				// (c.haag 2007-08-09 16:51) - PLID 27038 - Pass in the parent EMN
				pDetail->GetLinkedDetails(arLinked, GetParentEMN());

				// (j.jones 2010-02-12 11:03) - PLID 37318 - if this is a SmartStamp image detail
				// that is linked to a table detail, add to the array for the purposes of saving
				if(pDetail->GetSmartStampTableDetail()) {
					CEMNDetail *pTable = pDetail->GetSmartStampTableDetail();
					// (j.jones 2010-06-29 09:22) - PLID 39114 - only add if we have a parent topic,
					// otherwise throw an exception because we can't save without also saving
					// the table we are linked to
					if(pTable->m_pParentTopic == NULL) {
						ThrowNxException("CEMRTopic::CalculateTopicSavingOrderNeeded failed, SmartStamp table found with no parent topic");
					}
					
					arLinked.Add(pTable);
				}

				if(arLinked.GetSize()) {
					//OK, this detail has a link.  Is it on one of our topics?
					bool bDetailOnThis = false;
					CEMRTopic *pParent = pDetail->m_pParentTopic;
					while(pParent && !bDetailOnThis) {
						if(pParent == this)
							bDetailOnThis = true;
						pParent = pParent->GetParentTopic();
					}
					//Now, go through each of the linked details, see if any of the links are between us and a different topic.
					for(int j = 0; j < arLinked.GetSize(); j++) {
						bool bLinkedOnThis = false;
						CEMNDetail *pLinkedDetail = arLinked[j];
						CEMRTopic *pParent = pLinkedDetail->m_pParentTopic;
						while(pParent && !bLinkedOnThis) {
							if(pParent == this)
								bLinkedOnThis = true;
							pParent = pParent->GetParentTopic();
						}
						if(bDetailOnThis) {
							if(!bLinkedOnThis) {
								// (j.jones 2010-06-28 14:13) - PLID 39114 - ensure we have a parent topic
								if(pLinkedDetail->m_pParentTopic == NULL) {
									ThrowNxException("CEMRTopic::CalculateTopicSavingOrderNeeded failed, linked detail found with no parent topic (1)");
								}
								if(!IsTopicInArray(aryTopicsAlreadySaved, pLinkedDetail->m_pParentTopic)) {
									//OK, this is our detail, linked to not our detail.
									pLinkedDetail->m_pParentTopic->CalculateTopicSavingOrderNeeded(aryTopicsToSaveAfter, aryTopicsAlreadySaved);
								}
								aryTopicsToSaveAfter.Add(pLinkedDetail->m_pParentTopic);
							}
						}
						else {
							if(bLinkedOnThis) {
								// (j.jones 2010-06-28 14:13) - PLID 39114 - ensure we have a parent topic
								if(pDetail->m_pParentTopic == NULL) {
									ThrowNxException("CEMRTopic::CalculateTopicSavingOrderNeeded failed, linked detail found with no parent topic (2)");
								}
								if(!IsTopicInArray(aryTopicsAlreadySaved, pDetail->m_pParentTopic)) {
									//OK, this is not our detail, linked to our detail.									
									pDetail->m_pParentTopic->CalculateTopicSavingOrderNeeded(aryTopicsToSaveAfter, aryTopicsAlreadySaved);
								}
								aryTopicsToSaveAfter.Add(pDetail->m_pParentTopic);
							}
						}
					}
				}
			}

		}

	}

	//Now organize the final array to include all the "topics to save before",
	//then this topic, then all "topics to save after". Filter out duplicates.

	{
		//process the "topics to save before"
		for(int i=0;i<aryTopicsToSaveBefore.GetSize();i++) {
			CEMRTopic *pTopic = aryTopicsToSaveBefore.GetAt(i);
			BOOL bFound = FALSE;
			for(int j=0; j<aryEMRTopicsToSave.GetSize() && !bFound; j++) {
				if(pTopic == aryEMRTopicsToSave.GetAt(j))
					bFound = TRUE;
			}
			if(!bFound && pTopic != this) {
				aryEMRTopicsToSave.Add(pTopic);
			}
		}

		//process this topic

		//only add if this topic is not deleted
		if(!GetParentEMN()->IsTopicMarkedDeleted(this))
			aryEMRTopicsToSave.Add(this);

		for(i=0;i<aryTopicsToSaveAfter.GetSize();i++) {
			CEMRTopic *pTopic = aryTopicsToSaveAfter.GetAt(i);
			BOOL bFound = FALSE;
			for(int j=0; j<aryEMRTopicsToSave.GetSize() && !bFound; j++) {
				if(pTopic == aryEMRTopicsToSave.GetAt(j))
					bFound = TRUE;
			}
			if(!bFound && pTopic != this)
				aryEMRTopicsToSave.Add(pTopic);
		}
	}

	//we will return false if there is only one item (the calling topic)
	//in the list, which means that no special save is needed
	if(aryEMRTopicsToSave.GetSize() > 1)
		return TRUE;

	return FALSE;
}

//this returns the top-level parent topic to this topic (ie. has no parent itself)
CEMRTopic* CEMRTopic::GetTopMostParentTopic()
{
	if(!m_pParentTopic)
		return this;
	else
		return m_pParentTopic->GetTopMostParentTopic();
}

//this checks to see if pTopicToFind is underneat this topic, and if so, gives us the immediate subtopic
//that is a parent of pTopicToFind (note: this can return pTopicToFind if it is an immediate subtopic)
CEMRTopic* CEMRTopic::GetSubTopicWithThisTopic(CEMRTopic *pTopicToFind)
{
	for(int i=0;i<m_arypEMRTopics.GetSize();i++) {
		CEMRTopic *pSubTopic = m_arypEMRTopics.GetAt(i);

		//is this the topic we are looking for?
		if(pSubTopic == pTopicToFind) {
			return pSubTopic;
		}
		
		//these aren't the droids you're looking for

		CEMRTopic *pTestSubTopics = GetSubTopicWithThisTopic(pSubTopic);
		if(pTestSubTopics) {
			//found it, so return it
			return pTestSubTopics;
		}
	}

	return NULL;
}

// (z.manning 2011-03-02 17:20) - PLID 42335 - Added a flag for whether or not to check pending deleted details
BOOL CEMRTopic::IsDetailDeleted(CEMNDetail* pDetail, BOOL bCheckPendingDeleted /* = TRUE */)
{
	//DRT 8/3/2007 - PLID 26928 - Check the pending array as well.  If the detail is in that array, 
	//	it is considered deleted.
	if(bCheckPendingDeleted)
	{
		long nPendingCount = m_paryPendingDeletionDetails.GetSize();
		for(int i = 0; i < nPendingCount; i++) {
			CEMNDetail *pCurrent = m_paryPendingDeletionDetails.GetAt(i);
			if(pCurrent == pDetail) {
				return TRUE;
			}
		}
	}

	int nDeletedItemCount = m_aryDeletedDetails.GetSize();
	for(int i = 0; i < nDeletedItemCount; i++) {
		if(pDetail == m_aryDeletedDetails.GetAt(i)) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CEMRTopic::AllowUnspawn()
{
	if(!m_bAllowUnspawn) return FALSE;

	EnsureDetails();
	EnsureTopics();

	//We can't unspawn if any of our details or subtopics can't.
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		if(!m_arypEMNDetails[i]->GetAllowUnspawn()) return FALSE;
	}
	for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		if(!m_arypEMRTopics[i]->AllowUnspawn()) return FALSE;
	}
	return TRUE;
}

void CEMRTopic::SetLastSavedTopicName(const CString &strLastSavedTopicName)
{
	m_strLastSavedTopicName = strLastSavedTopicName;
}

void CEMRTopic::SetTemplateTopicOrderIndex(long nTemplateTopicOrderIndex)
{
	m_nTemplateTopicOrderIndex = nTemplateTopicOrderIndex;
}

void CEMRTopic::SetTemplateID(long nTemplateID)
{
	m_nTemplateID = nTemplateID;
}

void CEMRTopic::SetTemplateTopicID(long nTemplateTopicID)
{
	m_nTemplateTopicID = nTemplateTopicID;
}

BOOL CEMRTopic::HasSubTopics()
{
	return m_bHasSubTopics;
}

BOOL CEMRTopic::HasDetails()
{
	return m_bHasDetails;
}

void CEMRTopic::EnsureLoaded()
{
	if(!m_pLoadInfo) {
		//We weren't loaded from data.
		return;
	}
	ENSURE_VARIABLE(m_n, TemplateTopicID);
	ENSURE_VARIABLE(m_n, TemplateID);
	ENSURE_VARIABLE(m_n, TemplateTopicOrderIndex);
	ENSURE_VARIABLE(m_n, OriginalTemplateTopicID);

	EnsureTopics();
	EnsureDetails();

	ENSURE_VARIABLE(m_n, SourceActionID);
	ENSURE_VARIABLE(m_n, SourceDetailID);
	ENSURE_VARIABLE(m_str, SourceActionName);
	ENSURE_VARIABLE(m_, SourceActionDestType);

	ENSURE_VARIABLE(m_b, ShowIfEmpty);

	ENSURE_VARIABLE(m_b, HideOnEMN);

	GetCompletionStatus();
}

BOOL CEMRTopic::EnsureDetails()
{
	if(!m_pLoadInfo) {
		//We weren't loaded from data.
		return FALSE;
	}
	m_mtxLoadingDetails.Lock();
	if(!m_bDetailsLoaded) {
		//We haven't loaded the details yet.  Force the thread to finish loading us, if it hasn't yet.
		GetParentEMN()->GetParentEMR()->LoadTopicImmediate(m_pLoadInfo);
				
		//This won't do anything if it's already been called somehow.
		PostLoadDetails();
		m_mtxLoadingDetails.Unlock();

		// (c.haag 2007-08-27 15:34) - PLID 25853 - We can't assume the validity of the EMN loader object
		// from this point onward.
		if (m_pLoadInfo) m_pLoadInfo->m_pLoader = NULL;

		//Return TRUE, indicating that we actually loaded them.
		return TRUE;					
	}
	m_mtxLoadingDetails.Unlock();
	//If we got here, m_bDetailsLoaded was TRUE, meaning that PostLoadDetails() had already been called.  
	//So, the details were loaded without us having to do anything, so return FALSE.
	return FALSE;
}


BOOL CEMRTopic::EnsureTopics()
{
	if(!m_pLoadInfo) {
		//We weren't loaded from data.
		return FALSE;
	}
	m_mtxLoadingTopics.Lock();
	if(!m_bTopicsLoaded) {
		//We haven't loaded the details yet.  Force the thread to finish loading us, if it hasn't yet.
		GetParentEMN()->GetParentEMR()->LoadTopicImmediate(m_pLoadInfo);
				
		CEMNLoader* pEMNLoader = m_pLoadInfo->m_pLoader;
		if (NULL != pEMNLoader) {
			// (c.haag 2007-08-28 11:15) - PLID 25239 - At this point, the CEMNLoader object has the
			// fully loaded versions of all the topic details, and the states of related details, in
			// itself. We now need to percolate those changes to the details of our parent EMN.
			pEMNLoader->FlushPendingEMNDetailWrites();
		}

		//This won't do anything if it's already been called somehow.
		PostLoadTopics();
		m_mtxLoadingTopics.Unlock();

		// (c.haag 2007-08-27 15:34) - PLID 25853 - We can't assume the validity of the EMN loader object
		// from this point onward.
		if (m_pLoadInfo) m_pLoadInfo->m_pLoader = NULL;

		//Return TRUE, indicating that we actually loaded them.
		return TRUE;					
	}
	m_mtxLoadingTopics.Unlock();
	//If we got here, m_bTopicsLoaded was TRUE, meaning that PostLoadTopics() had already been called.  
	//So, the subtopics were loaded without us having to do anything, so return FALSE.
	return FALSE;
}

void CEMRTopic::PostLoad()
{
	if(m_bPostLoadCalled) return;
	m_bPostLoadCalled = TRUE;

	EMRLOGINDENT(1,"CEMRTopic::PostLoad called for and processing topic \"%s\" (ID=%d m_pLoadInfo->m_nTemplateTopicID=%s)", m_strTopicName, m_nID, (m_pLoadInfo) ? AsString(m_pLoadInfo->m_nTemplateTopicID) : "(null)"); // (c.haag 2010-05-19 9:04) - PLID 38759

	// (c.haag 2007-05-23 16:35) - PLID 26115 - We must be called from the main thread. It goes
	// against the asynchronous load design, and attempting to modify interface window content from
	// a worker thread will definitely cause problems.
	// (j.jones 2008-10-30 17:04) - PLID 31869 - renamed to ASSERT_IN_SAFE_THREAD
	ASSERT_IN_SAFE_THREAD("CEMRTopic::PostLoad");

	//Copy all our variables.	
	m_nTemplateTopicID = m_pLoadInfo->m_nTemplateTopicID;
	m_nTemplateID = m_pLoadInfo->m_nTemplateID;
	m_nTemplateTopicOrderIndex = m_pLoadInfo->m_nTemplateTopicOrderIndex;
	m_nOriginalTemplateTopicID = m_pLoadInfo->m_nOriginalTemplateTopicID;
	m_nOriginalTemplateID = m_pLoadInfo->m_nOriginalTemplateID; // (a.walling 2007-03-21 10:44) - PLID 25301
	
	// (c.haag 2006-06-21 17:18) - PLID 21163 - Calling SetSourceActionID here is a bad idea.
	// All it does is assign m_nSourceActionID, m_strSourceActionName and m_SourceActionDestType;
	// and it does it by querying the data no less since the optional parameters are null! Lets
	// just set a = b and move along.
	//SetSourceActionID(m_pLoadInfo->m_nSourceActionID);
	m_sai = m_pLoadInfo->m_sai; // (z.manning 2009-03-05 12:36) - PLID 33338
	//DRT 9/25/2007 - PLID 27515
	m_nSourceActionSourceID = m_pLoadInfo->m_nSourceActionSourceID;
	m_nSourceActionSourceDataGroupID = m_pLoadInfo->m_nSourceActionSourceDataGroupID;
	m_nSourceActionSourceHotSpotGroupID = m_pLoadInfo->m_nSourceActionSourceHotSpotGroupID;
	m_nSourceActionSourceTableDropdownGroupID = m_pLoadInfo->m_nSourceActionSourceTableDropdownGroupID; // (z.manning 2009-02-13 11:09) - PLID 33070

	m_strSourceActionName = m_pLoadInfo->m_strSourceActionName;
	m_SourceActionDestType = m_pLoadInfo->m_SourceActionDestType;

	m_bShowIfEmpty = m_pLoadInfo->m_bShowIfEmpty;
	m_bLastSavedShowIfEmpty = m_pLoadInfo->m_bShowIfEmpty;

	//Should this template topic be included on EMNs?
	m_bHideOnEMN = m_pLoadInfo->m_bHideOnEMN;

	m_strTopicName = m_pLoadInfo->m_strName;
	m_strLastSavedTopicName = m_strTopicName;
	//TES 5/3/2010 - PLID 24692 - Tell our position entry to remember its order
	if(m_pTopicPositionEntry) {
		m_pTopicPositionEntry->SetLastSavedOrderIndex();
	}

	//TES 1/24/2007 - PLID 24377 - Our status was initialized to etcsInvalid; if it's changed from that, then we will
	// assume that whatever changed it is more up-to-date than what was being loaded, so we will ignore the value in 
	// m_pLoadInfo.
	if(m_CompletionStatus == etcsInvalid) {
		m_CompletionStatus = m_pLoadInfo->m_CompletionStatus;
	}

	m_mtxLoadingTopics.Lock();
	PostLoadTopics();
	m_mtxLoadingTopics.Unlock();

	m_mtxLoadingDetails.Lock();
	PostLoadDetails();
	m_mtxLoadingDetails.Unlock();

	//Also, we just loaded ourselves, so if we aren't new, then we're saved.
	//TES 7/5/06 - If we're a spawned topic on a template, then even if we're new, we are saved.
	if(!m_bForceSave && (m_nID != -1 || (GetParentEMN()->IsTemplate() && m_sai.nSourceActionID != -1))) {
		SetSaved(TRUE);
		CWnd *pWnd = GetParentEMN()->GetInterface();
		if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
			pWnd->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)this);
		}
	}

	// (c.haag 2007-05-30 09:57) - PLID 25853 - We can't assume the validity of the EMN loader object
	// from this point onward.
	m_pLoadInfo->m_pLoader = NULL;

	//TES 1/12/2006 - PLID 24172 - We are now completely loaded.  Note that, and tell our parent.
	m_bPostLoadComplete = TRUE;

	/*
	// (z.manning 2009-06-22 11:13) - PLID 34286 - This was causing all sorts of problems with
	// narratives because calling RepositionDetailsInTopicByInfoID can lead to calling
	// EnsureEmrItemAdvDlg which calls ReflectCurrentState. However, we may not have loaded
	// narrative fields yet, so it was setting the state without the correct field data, which
	// cause narratives to not show any data at all in the preview pane, among other issues.
	// This logic is now in CEMN::PostInitialLoad
	//
	// (c.haag 2007-05-08 10:54) - PLID 25928 - If we had the need to reposition details during
	// the load, but had to wait for the load to finish, then reposition the details now.
	if (GetNeedsToRepositionDetailsInPostLoad()) {
		m_bNeedsToRepositionDetailsInPostLoad = FALSE;
		// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
		(GetInterface())->RepositionDetailsInTopicByInfoID(-1, FALSE);
	}*/

	GetParentEMN()->PostTopicLoad(this);

	EMRLOGINDENT(-1,"CEMRTopic::PostLoad END"); // (c.haag 2010-05-19 9:04) - PLID 38759
}

void CEMRTopic::PostLoadTopics()
{
	if(m_bTopicsLoaded) return; //This function has already been called.
	m_bTopicsLoaded = TRUE;
	//We just need to attach them.
	for(int i = 0; i < m_pLoadInfo->m_aryEMRTopics.GetSize(); i++) {
		CEMRTopic *pTopic = m_pLoadInfo->m_aryEMRTopics[i];
		//TES 1/11/2007 - Make sure this function doesn't call PostLoad(), since this topic won't have been fully attached yet.
		pTopic->SetParentTopic(this, TRUE, FALSE);
		//TES 10/5/2009 - PLID 35755 - We don't want to re-calculate topic indexes
		InsertSubTopic(pTopic, NULL, TRUE, FALSE);
		//TES 2/7/2007 - PLID 24652 - Now, if this function needs PostLoad() called on it (which would mean thatit was loaded
		// synchronously, and therefore isn't getting any posted messages to tell it to call PostLoad()), then call it.
		if(pTopic->NeedPostLoad()) pTopic->PostLoad();
		// (z.manning, 03/05/2007) - PLID 25062 - Make sure we count empty subtopics if we're on a template.
		// (z.manning, 08/24/2007) - PLID 27147 - Actually, determing whether or not a topic has subtopics this
		// way is flawed on patient EMNs as well. The way we use m_bHasSubtopics is such that even if it has
		// only invisible and empty subtopics then it still needs to be true in case we later spawn something
		// onto one of those children. So, I took this code out and set m_bSubTopics to true as long is the 
		// the subtopic array is non-empty. Note: this won't cause empty topics to be visible when they shouldn't
		// be as that's handled by EmrTreeWnd.
		//if(m_bIsTemplate || !pTopic->IsEmpty()) {
		//	bFoundSubTopic = TRUE;
		//}
	}
	m_pLoadInfo->m_bTopicsCopied = TRUE;
	m_bHasSubTopics = m_pLoadInfo->m_aryEMRTopics.GetSize() > 0;

	// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
	//(e.lally 2012-04-05) PLID 49446 - We don't need to post this message so much on loading
	/*
	CWnd *pWnd = GetParentEMN()->GetInterface();
	if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
		pWnd->PostMessage(NXM_EMR_PROBLEM_CHANGED);
	}
	*/
}

void CEMRTopic::PostLoadDetails()
{
	if(m_bDetailsLoaded) return; //This function has already been called.
	m_bDetailsLoaded = TRUE; //This function doesn't need to be called again.

	EMRLOGINDENT(1,"CEMRTopic::PostLoadDetails called for and processing topic \"%s\" (ID=%d m_nTemplateTopicID=%d)", m_strTopicName, m_nID, m_nTemplateTopicID); // (c.haag 2010-05-19 9:04) - PLID 38759

	CEMNLoader* pEMNLoader = m_pLoadInfo->m_pLoader;
	if (NULL != pEMNLoader) {
		// (c.haag 2007-07-05 09:05) - PLID 25239 - At this point, the CEMNLoader object has the
		// fully loaded versions of all the topic details, and the states of related details, in
		// itself. We now need to percolate those changes to the details of our parent EMN.
		pEMNLoader->FlushPendingEMNDetailWrites();
	}

	//TES 1/23/2008 - PLID 24157 - Renamed.
	GetParentEMN()->LockHandlingDetailChanges();
	GetParentEMN()->GetParentEMR()->LockSpawning();
	BOOL bFoundDetail = FALSE;

	// (c.haag 2007-01-31 16:30) - PLID 24376 - This is used to populate the state for
	// Current Medications details later on in the loop. If none of the details use
	// the Current Medications details, then the variable is unused
	long nActiveCurrentMedicationsInfoID = -2;

	// (c.haag 2007-04-05 13:30) - PLID 25516 - This is used to populate the state for
	// Allergies details later in the loop.
	_variant_t varAllergiesDefaultState;
	BOOL bAllergiesDefaultStateLoaded = FALSE;
	long nActiveCurrentAllergiesInfoID = -2;

	// (z.manning 2011-02-11 17:34) - PLID 42446 - I won't get into a big rant here, but needless
	// to say that EMR loading code is complex.  Smart stamp tables load their content based on
	// its linked image's state. Thus, the smart stamp table must already be linked to an image
	// before its content is loaded. Most of the time, this is true but their seem to be a certain
	// sequence of loading events (e.g. when sometimes when spawning topics) were an immediate load
	// is forced and we end up getting here before smart stamp links have been established. So, in
	// the event that this happens
	m_pLoadInfo->m_aryEMNDetails.SortWithParentDetailsFirst();

	for(int nDetail = 0; nDetail < m_pLoadInfo->m_aryEMNDetails.GetSize(); nDetail++) {
		CEMNDetail *pDetail = m_pLoadInfo->m_aryEMNDetails[nDetail];
		//Tell the detail it's on us.
		pDetail->m_pParentTopic = this;
		//Add to our array.
		// (c.haag 2007-06-27 11:34) - PLID 26468 - We now exclusively own all the detail
		// objects passed from the loader, so there's no need to increment the reference
		// count of a detail
		AddDetail(pDetail, GetParentEMN()->IsLoading(), FALSE /* Don't add a reference to the detail */);
		if(pDetail->GetVisible()) bFoundDetail = TRUE;

		// (c.haag 2007-02-08 12:19) - PLID 24376 - Fill in the active Current Medications info ID if we haven't already.
		// We'll use it below.
		// (c.haag 2007-05-17 10:18) - PLID 26046 - Use GetStateVarType to get the detail state type
		//TES 6/23/2008 - PLID 29416 - Don't check for VT_EMPTY (not sure why we ever did).
		if (pDetail->IsCurrentMedicationsTable() /*&& VT_EMPTY == pDetail->GetStateVarType()*/) {
			// (c.haag 2007-07-24 12:46) - PLID 26742 - Should check for -2, not -1
			if (-2 == nActiveCurrentMedicationsInfoID) {
				
				// (j.jones 2007-07-24 09:27) - PLID 26742 - the medications info ID is cached in CEMR

				//do memory checks
				if(GetParentEMN()) {
					if(GetParentEMN()->GetParentEMR()) {
						nActiveCurrentMedicationsInfoID = GetParentEMN()->GetParentEMR()->GetCurrentMedicationsInfoID();
					}
				}

				if(nActiveCurrentMedicationsInfoID == -2) {
					//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
					//but why don't we have an EMR?
					ASSERT(FALSE);
					nActiveCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
				}
			}
		}
		// (c.haag 2007-04-05 13:31) - PLID 25516 - Fill in the active Allergies info ID if we haven't already.
		// We'll use it below
		//TES 6/23/2008 - PLID 29416 - Don't check for VT_EMPTY (not sure why we ever did)
		else if (pDetail->IsAllergiesTable() /*&& VT_EMPTY == pDetail->GetStateVarType()*/) {
			// (c.haag 2007-07-24 12:46) - PLID 26742 - Should check for -2, not -1
			if (-2 == nActiveCurrentAllergiesInfoID) {
				
				// (j.jones 2007-07-24 09:27) - PLID 26742 - the Allergy Info ID is cached in CEMR
				//do memory checks
				if(GetParentEMN()) {
					if(GetParentEMN()->GetParentEMR()) {
						nActiveCurrentAllergiesInfoID = GetParentEMN()->GetParentEMR()->GetCurrentAllergiesInfoID();
					}
				}

				if(nActiveCurrentAllergiesInfoID == -2) {
					//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
					//but why don't we have an EMR?
					ASSERT(FALSE);
					nActiveCurrentAllergiesInfoID = GetActiveAllergiesInfoID();
				}
			}
		}


		// (c.haag 2007-02-08 12:19) - PLID 24376 - If we are pulling an official Current Medications detail,
		// then we must calculate the state from the EMN itself. Similar code is run in these three places:
		//
		// CEMNDetail::LoadFromInfoID()				- When adding a Current Medications detail to a topic manually
		// CEMNDetail::LoadFromTemplateDetailID()	- When adding a Current Medications detail from a template
		// CEMRTopic::PostLoadDetails()				- When loading a patient EMN from a template
		// LoadEmrTopic								- When loading detail states in a thread
		// CEMRTopic::AddDetail()					- The rest of the time (failsafe)
		//
		// When loading Current Medication info items into templates, we should not do any special calculating. It
		// should behave exactly as it did before.
		//
		// (c.haag 2007-05-17 10:18) - PLID 26046 - Use GetStateVarType to get the detail state type
		// (c.haag 2007-08-03 13:13) - PLID 26929 - If this is a template Current Medications detail, we
		// must load the blank detail state for it. Otherwise, we will get assertions in the save string
		// generation code that the detail is empty (plus there should be no persistent empty details in
		// an EMN anyway)
		//TES 6/23/2008 - PLID 29416 - Check GetHasLoadedSystemInfo() rather than VT_EMPTY
		//TES 6/30/2008 - PLID 29416 - Also, we only want to sync if we're loading from a template.  If we're loading an
		// existing EMN, we want to leave it as is.
		bool bSyncWithCurrentMedications = (m_pLoadInfo->bLoadFromTemplate && pDetail->IsCurrentMedicationsTable() && !pDetail->GetHasLoadedSystemInfo() && nActiveCurrentMedicationsInfoID == pDetail->m_nEMRInfoID) ? true : false;
		if (bSyncWithCurrentMedications) {

			if (pDetail->m_bIsTemplateDetail) {
				// (c.haag 2009-02-13 10:30) - PLID 33073 - The state may already be blank in the first place. Don't
				// request a state change in that case as it would have no net effect other than to flag the detail
				// as modified right after the initial load (which is a bad thing). We need to be careful when calling
				// GetState(); it throws an exception if the state is empty (not that I can think of any good reason
				// why it would be here; but EMR is so complicated, best not assume)
				_variant_t vDetail;
				if (VT_EMPTY != pDetail->GetStateVarType()) {
					vDetail = pDetail->GetState();
				} else {
					vDetail.vt = VT_EMPTY;
				}
				_variant_t vBlank = LoadEMRDetailStateBlank( eitTable );
				if (VARCMP_EQ != VariantCompare(&vDetail, &vBlank)) {
					pDetail->RequestStateChange( vBlank );
				}
			} else {
				// Determine the Current Medications state if we haven't already
				//TES 6/5/2008 - PLID 29416 - Rather than using the "official" current medications state (which never really
				// existed), just merge in the Medications tab data (which only affects one
				// column in the table).
				GetParentEMN()->ApplyOfficialCurrentMedications(pDetail, pDetail->m_nEMRInfoID);
				pDetail->RequestStateChange(pDetail->GetState());
			}
		}
		// (c.haag 2007-04-05 13:31) - PLID 25516 - Repeat this test for allergies items
		// (c.haag 2007-08-03 13:12) - PLID 26929 - If this is a template Allergies detail, we
		// must load the blank detail state for it. Otherwise, we will get assertions in the save string
		// generation code that the detail is empty (plus there should be no persistent empty details in
		// an EMN anyway)
		//TES 6/23/2008 - PLID 29416 - Check GetHasLoadedSystemInfo() rather than VT_EMPTY
		//TES 6/30/2008 - PLID 29416 - Also, we only want to sync if we're loading from a template.  If we're loading an
		// existing EMN, we want to leave it as is.
		bool bSyncWithAllergies = (m_pLoadInfo->bLoadFromTemplate && pDetail->IsAllergiesTable() && !pDetail->GetHasLoadedSystemInfo() && nActiveCurrentAllergiesInfoID == pDetail->m_nEMRInfoID) ? true : false;
		if (bSyncWithAllergies) {

			if (pDetail->m_bIsTemplateDetail) {
				// (c.haag 2009-02-13 10:30) - PLID 33073 - The state may already be blank in the first place. Don't
				// request a state change in that case as it would have no net effect other than to flag the detail
				// as modified right after the initial load (which is a bad thing). We need to be careful when calling
				// GetState(); it throws an exception if the state is empty (not that I can think of any good reason
				// why it would be here; but EMR is so complicated, best not assume)
				_variant_t vDetail;
				if (VT_EMPTY != pDetail->GetStateVarType()) {
					vDetail = pDetail->GetState();
				} else {
					vDetail.vt = VT_EMPTY;
				}
				_variant_t vBlank = LoadEMRDetailStateBlank( eitTable );
				if (VARCMP_EQ != VariantCompare(&vDetail, &vBlank)) {
					pDetail->RequestStateChange( vBlank );
				}
			} else {
				// Determine the Allergies state if we haven't already
				//TES 6/5/2008 - PLID 29416 - Rather than using the "official" current medications state (which never really
				// existed), just merge in the Medications tab data (which only affects one
				// column in the table).
				
				GetParentEMN()->ApplyOfficialAllergies(pDetail, pDetail->m_nEMRInfoID);
				pDetail->RequestStateChange(pDetail->GetState());
			}
		}

		//TES 1/23/2007 - PLID 24377 - We need to apply links here, iff we're on a new non-template topic.
		if(m_nID == -1 && !m_bIsTemplate) {
			pDetail->ApplyEmrLinks();
		}

		// (j.jones 2010-02-12 14:23) - PLID 37318 - check whether this is part of a SmartStamp
		// set of details, and ensure links (or create the table) accordingly
		GetParentEMN()->EnsureSmartStampLinks(pDetail);

		//Process actions.
		//TES 5/24/2006 - If we were loading an existing topic, we don't want to process actions.  So only do this
		//if we're on a template, or new.
		//TES 8/7/2006 - PLID 21807 - Also, don't spawn actions if we are hidden.
		if((m_bIsTemplate || m_nID == -1) && pDetail->GetVisible()) {
			// (j.jones 2006-07-26 17:30) - PLID 20339 - changed the bIsInitialLoad to TRUE here and on
			// the below ProcessEMRDataActions, because even though this is the post load, the actions
			// are still part of that load
			// (j.jones 2007-07-18 13:21) - PLID 26730 - check whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them
			if(pDetail->GetHasInfoActionsStatus() != ehiasHasNoInfoItems) {
				GetParentEMN()->GetParentEMR()->ProcessEMRInfoActions(pDetail, GetParentEMN()->IsLoading(), m_pLoadInfo->m_pLoader);
			}

			// (z.manning 2011-11-16 16:03) - PLID 38130 - Moved this logic to a new function
			pDetail->CollectAndProcessActionsPostLoad(GetParentEMN()->IsLoading(), m_pLoadInfo->m_pLoader);
		}

	}

	// (j.jones 2010-03-03 14:40) - PLID 37231 - ensure new details have SetNew() called on them
	for(int i=0; i<m_arypEMNDetails.GetSize(); i++) {
		//we may have remembered content from a prior item that needs to be reset as new
		CEMNDetail *pDetail = (CEMNDetail*)m_arypEMNDetails.GetAt(i);
		if((m_bIsTemplate && pDetail->m_nEMRTemplateDetailID == -1)
			|| (!m_bIsTemplate && pDetail->m_nEMRDetailID == -1)) {
			pDetail->SetNew();
		}
	}

	m_pLoadInfo->m_bDetailsCopied = TRUE;
	m_bHasDetails = bFoundDetail;

	// (a.walling 2007-12-24 10:53) - PLID 28431 - Recalculate the topic's completion status now that all writes have been
	// performed for this topic via pEMNLoader->FlushPendingEMNDetailWrites();
	{
		BOOL bNonEmptyFound = FALSE;
		BOOL bEmptyFound = FALSE;
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		BOOL bFoundRequiredUnfilled = FALSE;
		for(int i = m_arypEMNDetails.GetSize()-1; i >= 0; i--) {
			if(m_arypEMNDetails[i]->GetVisible()) {
				if (m_arypEMNDetails[i]->IsStateSet()) {
					bNonEmptyFound = TRUE;
				} else {
					bEmptyFound = TRUE;
					// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
					if (m_arypEMNDetails[i]->IsRequired()) {
						bFoundRequiredUnfilled = TRUE;
					}
				}
			}
		}
		
		// (z.manning, 04/04/2008) - PLID 29495 - Now have a status for topics with no details
		if(m_arypEMNDetails.GetSize() == 0) {
			m_CompletionStatus = etcsBlank;
		}
		else if (bFoundRequiredUnfilled) {
			// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
			m_CompletionStatus = etcsRequired;
		}
		else if(bNonEmptyFound) {
			if(bEmptyFound) {
				m_CompletionStatus = etcsPartiallyComplete;
			}
			else {
				m_CompletionStatus = etcsComplete;
			}
		}
		else {
			m_CompletionStatus = etcsIncomplete;
		}
	}

	// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
	//(e.lally 2012-04-05) PLID 49446 - We don't need to post this message so much on loading
	/*
	CWnd *pWnd = GetParentEMN()->GetInterface();
	if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
		pWnd->PostMessage(NXM_EMR_PROBLEM_CHANGED);
	}
	*/

	//All done! Tell the EMR it can start spawning again.
	GetParentEMN()->GetParentEMR()->UnlockSpawning();
	//TES 1/23/2008 - PLID 24157 - Renamed.
	GetParentEMN()->UnlockHandlingDetailChanges();

	EMRLOGINDENT(-1,"CEMRTopic::PostLoadDetails END"); // (c.haag 2010-05-19 9:04) - PLID 38759
}


CEMRTopic* CEMRTopic::GetSubTopicByID(long nID)
{
	EnsureTopics();
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		if(m_arypEMRTopics[i]->GetID() == nID) return m_arypEMRTopics[i];
		CEMRTopic *pSubTopic = m_arypEMRTopics[i]->GetSubTopicByID(nID);
		if(pSubTopic) return pSubTopic;
	}
	return NULL;
}

BOOL CEMRTopic::ShowIfEmpty(BOOL bForceLoadFromData /*= TRUE*/)
{
	if(bForceLoadFromData) ENSURE_VARIABLE(m_b, ShowIfEmpty);
	return m_bShowIfEmpty;
}

void CEMRTopic::SetShowIfEmpty(BOOL bShow)
{
	m_bShowIfEmpty = bShow;
	SetUnsaved();
}

BOOL CEMRTopic::IsLoaded()
{
	//TES 1/12/2007 - PLID 24172 - If we haven't gotten all the way through PostLoad(), then we're not fully loaded
	if(!m_bPostLoadComplete) return FALSE;

	if(m_pLoadInfo && !m_pLoadInfo->m_bCompletelyLoaded ) return FALSE;

	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		if(!m_arypEMRTopics[i]->IsLoaded()) return FALSE;
	}
	return TRUE;
}

void CEMRTopic::HandleDetailStateChange(CEMNDetail *pChangedDetail)
{
	//The only thing we need to do is make sure our cached CompletionStatus is up-to-date.
	if(!pChangedDetail->GetVisible()) {
		//This has no affect on our completion status.
		return;
	}
	if(pChangedDetail->IsStateSet()) {
		if(m_CompletionStatus != etcsComplete) {
			// Are there any other non-completed or non-completed required details?
			BOOL bOtherEmptyFound = FALSE;
			// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
			BOOL bUnfilledRequiredFound = FALSE;
			for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
				CEMNDetail *pDetail = m_arypEMNDetails[i];
				if (pDetail->GetVisible() && !pDetail->IsStateSet()) {
					if (pDetail->IsRequired()) {
						bUnfilledRequiredFound = TRUE;
						break;
					} else {
						bOtherEmptyFound = TRUE;
					}
				}
			}
			// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
			if (bUnfilledRequiredFound) {
				m_CompletionStatus = etcsRequired;
			} else if (bOtherEmptyFound) {
				m_CompletionStatus = etcsPartiallyComplete;
			} else {
				m_CompletionStatus = etcsComplete;
			}
		}
	}
	else {
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		if (pChangedDetail->IsRequired()) {
			m_CompletionStatus = etcsRequired;
		} else if (m_CompletionStatus != etcsIncomplete && m_CompletionStatus != etcsRequired) {
			//Are there any other completed details?
			BOOL bOtherCompletedFound = FALSE;
			for(int i = 0; i < m_arypEMNDetails.GetSize() && !bOtherCompletedFound; i++) {
				if(m_arypEMNDetails[i]->GetVisible() && m_arypEMNDetails[i]->IsStateSet()) bOtherCompletedFound = TRUE;
			}
			if(bOtherCompletedFound) {
				m_CompletionStatus = etcsPartiallyComplete;
			}
			else {
				m_CompletionStatus = etcsIncomplete;
			}
		}
	}
}

// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

//TES 6/4/2008 - PLID 30196 - Does this topic have anywhere on it or on any subtopics an (undeleted) system 
// table of the specified type?
BOOL CEMRTopic::HasSystemTable(EmrInfoSubType eistType)
{
	//TES 6/5/2008 - PLID 30196 - Does this topic have any details of this type?
	long nDetails = GetEMNDetailCount();
	BOOL bHasTable = FALSE;
	long i; 
	for(i = 0; i < nDetails && !bHasTable; i++) {
		CEMNDetail* pDetail = GetDetailByIndex(i);
		if(pDetail->m_EMRInfoSubType == eistType) bHasTable = TRUE;
	}

	//TES 6/5/2008 - PLID 30196 - How about our subtopics?
	long nSubTopics = GetSubTopicCount();
	for(i = 0; i < nSubTopics && !bHasTable; i++) {
		CEMRTopic* pTopic = GetSubTopic(i);
		if(pTopic && pTopic->HasSystemTable(eistType)) {
			bHasTable = TRUE;
		}
	}
	return bHasTable;
}

// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
// Safely checks this topic and subtopics recursively to find if there are any visible details that are marked as required and aren't filled in.
BOOL CEMRTopic::HasVisibleUnfilledRequiredDetails()
{
	// Are we visible?
	if (this == NULL || !this->GetVisible()) {
		// No, so we can't contain anything visible so just return now
		return FALSE;
	}

	// Are any of our visible details required and not filled?
	for (long iDetail = 0, nDetailCount = this->GetEMNDetailCount(); iDetail < nDetailCount; iDetail++) {
		CEMNDetail *pDetail = this->GetDetailByIndex(iDetail);
		if (pDetail != NULL && pDetail->GetVisible() && pDetail->IsRequired() && !pDetail->IsStateSet()) {
			// We just found a detail that is visible, required, and unfilled.  So we're done.
			return TRUE;
		}
	}

	// We don't have any details, but maybe some descendent child of ours does
	for (long iTopic = 0, nTopicCount = this->GetSubTopicCount(); iTopic < nTopicCount; iTopic++) {
		if (this->GetSubTopic(iTopic)->HasVisibleUnfilledRequiredDetails()) {
			// Found one, we're done.
			return TRUE;
		}
	}

	// Made it through the whole search without finding one, so there isn't one.
	return FALSE;
}

BOOL CEMRTopic::GetIsOnLockedAndSavedEMN()
{
	BOOL bIsLockedAndSaved = FALSE;
	{
		CEMN *pEMN = GetParentEMN();
		if(pEMN) {
			bIsLockedAndSaved = pEMN->IsLockedAndSaved();
		}
		else {
			//no parent EMN? We have to find out for sure from data
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(!m_bIsTemplate && m_nID != -1
				&& ReturnsRecordsParam("SELECT ID FROM EMRTopicsT WHERE ID = {INT} AND EMRID IN (SELECT ID FROM EMRMasterT WHERE Status = 2)", m_nID)) {
				bIsLockedAndSaved = TRUE;
			}
		}
	}

	return bIsLockedAndSaved;
}


BOOL CEMRTopic::GetHasMoved()
{
	return m_bHasMoved;
}

void CEMRTopic::SetHasMoved(BOOL bMoved)
{
	m_bHasMoved = bMoved;
}

// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Search this topic and all subtopics for details with a reconstructed status of -1.
BOOL CEMRTopic::HasReconstructedDetails()
{
	// See if any detail on this topic is reconstructed and not yet verified by the user (i.e. review status -1)
	{
		for (long i=0, nCount=GetEMNDetailCount(); i<nCount; i++) {
			CEMNDetail *pDetail = GetDetailByIndex(i);
			if (pDetail && pDetail->m_nReviewState == -1) {
				// Ok, we have a reconstructed detail so break out!
				return TRUE;
			}
		}
	}

	// Now check if any of our subtopics has reconstructed details
	{
		for (long i=0, nCount=GetSubTopicCount(); i<nCount; i++) {
			CEMRTopic* pTopic = GetSubTopic(i);
			if (pTopic && pTopic->HasReconstructedDetails()) {
				// Ok, we have a subtopic with a reconstructed detail so break out!
				return TRUE;
			}
		}
	}

	// If we made it here, there are no reconstructed details
	return FALSE;
}

// (j.jones 2007-01-23 09:14) - PLID 24027 - reassigns source detail IDs/pointers due to an EMN copy
// Basic logic: for each spawnable item (topics, More Info, Details, {EMNs excluded}) that has a 
// source detail, we need to point it to the proper detail in the new copy of the EMN.
// We do this prior to wiping out the detail IDs for the new copy, so if we have the detail ID, 
// it's a matter of finding the detail with that ID, setting our pointer to it, and we're done.
// If all we have is a pointer though, it's a pointer to the old detail in the EMN we copied from.
// So the detail's operator = function keeps a pointer to each detail a new one was copied from,
// which then gets cleared out in SetNew later on. For the duration of this function though,
// this means that we can look at each detail on the new EMN, find the original detail's address,
// compare to our source detail pointer, and if they match, reassign the current detail's source
// detail pointer to the new detail that was linked to the original detail's address.
// In all cases make SourceDetailID be -1 since everything on the EMN is new and has no ID, or at
// least will soon have no ID because SetNew() is called right after this
//TES 5/20/2014 - PLID 52705 - The comment above this was incorrect, SetNew() is not always called right after this,
// because this function is now called when importing topics, not just when copying EMNs. 
// So, I added bResetIDs, so this function knows whether or not it should set everything to -1
void CEMRTopic::UpdateSourceDetailsFromCopy(bool bResetIDs)
{
	//update this topic
	
	//first try the detail ID
	if(m_sai.nSourceDetailID != -1) {
		CEMNDetail *pSourceDetail = m_bIsTemplate ? GetParentEMN()->GetDetailByTemplateDetailID(m_sai.nSourceDetailID) : GetParentEMN()->GetDetailByID(m_sai.nSourceDetailID);
		m_sai.pSourceDetail = pSourceDetail;
		//ensure our source detail ID is -1 now
		//TES 5/20/2014 - PLID 52705 - Only if bResetIDs is true
		if (bResetIDs) {
			m_sai.nSourceDetailID = -1;
		}
	}
	else if(m_sai.pSourceDetail) {
		//we have a pointer but no detail ID, so try to find the parent detail
		BOOL bFound = FALSE;
		long nDetails = GetParentEMN()->GetTotalDetailCount();
		for(int j=0; j<nDetails && !bFound; j++) {
			//we have to compare to every detail in the EMN
			CEMNDetail *pDetailToCompare = GetParentEMN()->GetDetail(j);
			if(pDetailToCompare) {
				//and we are temporarily storing a pointer to the copied-from topic,
				//which is what our sourcedetail currently links to
				CEMNDetail *pCopiedFromDetail = pDetailToCompare->GetCopiedFromDetail();
				if(pCopiedFromDetail == m_sai.pSourceDetail) {
					//good, we found our source detail, now reassign our pointer
					m_sai.pSourceDetail = pDetailToCompare;
					//ensure our source detail ID is -1 now
					//TES 5/20/2014 - PLID 52705 - Only if bResetIDs is true (although really we already know it is -1)
					if (bResetIDs) {
						m_sai.nSourceDetailID = -1;
					}
					bFound = TRUE;
				}
			}
		}
	}

	//update all subtopics
	for(int i = 0; i < GetSubTopicCount(); i++) {
		CEMRTopic *pTopic = m_arypEMRTopics.GetAt(i);
		//TES 5/20/2014 - PLID 52705 - Pass in bResetIDs
		pTopic->UpdateSourceDetailsFromCopy(bResetIDs);
	}

	//update all details
	for(i = 0; i < GetEMNDetailCount(); i++)
	{
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(i);
		//first try the detail ID
		if(pDetail->GetSourceDetailID() != -1) {
			CEMNDetail *pSourceDetail = m_bIsTemplate ? GetParentEMN()->GetDetailByTemplateDetailID(pDetail->GetSourceDetailID()) : GetParentEMN()->GetDetailByID(pDetail->GetSourceDetailID());
			pDetail->SetSourceDetail(pSourceDetail);
			//ensure our source detail ID is -1 now
			//TES 5/20/2014 - PLID 52705 - Only if bResetIDs is true
			if (bResetIDs) {
				pDetail->SetSourceDetailID(-1);
			}
		}
		else if(pDetail->GetSourceDetail())
		{
			//we have a pointer but no detail ID, so try to find the parent detail
			BOOL bFound = FALSE;
			long nDetails = GetParentEMN()->GetTotalDetailCount();
			for(int j=0; j<nDetails && !bFound; j++) {
				//we have to compare to every detail in the EMN
				CEMNDetail *pDetailToCompare = GetParentEMN()->GetDetail(j);
				if(pDetailToCompare) {
					//and we are temporarily storing a pointer to the copied-from detail,
					//which is what our sourcedetail currently links to
					CEMNDetail *pCopiedFromDetail = pDetailToCompare->GetCopiedFromDetail();
					if(pCopiedFromDetail == pDetail->GetSourceDetail()) {
						//good, we found our source detail, now reassign our pointer
						pDetail->SetSourceDetail(pDetailToCompare);
						//ensure our source detail ID is -1 now
						//TES 5/20/2014 - PLID 52705 - Only if bResetIDs is true (although really we already know it is -1)
						if (bResetIDs) {
							pDetail->SetSourceDetailID(-1);
						}
						bFound = TRUE;
					}
				}
			}
		}

		// (z.manning 2013-03-12 17:15) - PLID 55016 - Also go through any detail image stamps and update those pointers
		// based on their copies.
		for(int nDetailStampIndex = 0; nDetailStampIndex < pDetail->GetImageStampCount(); nDetailStampIndex++)
		{
			EmrDetailImageStamp *pDetailStamp = pDetail->GetImageStampByIndex(nDetailStampIndex);
			if(GetParentEMN() != NULL && pDetailStamp != NULL && pDetailStamp->GetCopiedFromDetailStamp() != NULL) {
				// (z.manning 2013-03-13 09:38) - PLID 55016 - Replace any instances of the original stamp with the current
				// stamp on this EMN.
				GetParentEMN()->UpdateSourceDetailStampPointers(pDetailStamp->GetCopiedFromDetailStamp(), pDetailStamp, GetParentEMN());
			}
		}
	}
}

// (z.manning 2010-03-11 14:56) - PLID 37571 - Will reassasign the source detail stamp pointer to the given
// new pointer for any object in this topic that could have potentially been spawned.
void CEMRTopic::UpdateSourceDetailStampPointers(EmrDetailImageStamp *pDetailStampOld, EmrDetailImageStamp *pDetailStampNew)
{
	//update this topic
	m_sai.UpdateDetailStampPointerIfMatch(pDetailStampOld, pDetailStampNew);

	//update all subtopics
	for(int i = 0; i < GetSubTopicCount(); i++) {
		CEMRTopic *pTopic = m_arypEMRTopics.GetAt(i);
		pTopic->UpdateSourceDetailStampPointers(pDetailStampOld, pDetailStampNew);
	}

	//update all details
	for(i = 0; i < GetEMNDetailCount(); i++) {
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(i);
		pDetail->UpdateSourceDetailStampPointers(pDetailStampOld, pDetailStampNew);
	}
}

// (j.jones 2007-01-23 11:00) - PLID 24027 - update the source details such that their pointers are set
// and if bClearEraseSourceDetailID is TRUE, then also clear their detail IDs
// Basic Logic: for every item that has a source detail ID (regardless of whether it has a pointer), find the source detail and ensure
// its pointer is set to that detail. Then based on the boolean, optionally set the SourceDetailID to -1.
// (Note: if the source detail itself is -1, this would happen anyways)
void CEMRTopic::UpdateSourceDetailPointers(BOOL bClearEraseSourceDetailID)
{
	//update this topic
	// (a.walling 2007-06-28 16:39) - PLID 26494 - Ensure everything is loaded on this topic
	// If we are a subtopic, we will be called from that topic's UpdateSourceDetailPointers
	// before we had a chance to process PostLoad (and therefore get all our variables).
	EnsureLoaded();
	
	if(m_sai.nSourceDetailID != -1) {
		CEMNDetail *pSourceDetail = m_bIsTemplate ? GetParentEMN()->GetDetailByTemplateDetailID(m_sai.nSourceDetailID) : GetParentEMN()->GetDetailByID(m_sai.nSourceDetailID);
		VERIFY(m_sai.pSourceDetail = pSourceDetail); // (a.walling 2007-06-29 16:48) - PLID 26494 - Assert if we did not get a valid detail.
		if(bClearEraseSourceDetailID)
			m_sai.nSourceDetailID = -1;
	}

	//update all subtopics
	for(int i = 0; i < GetSubTopicCount(); i++) {
		CEMRTopic *pTopic = m_arypEMRTopics.GetAt(i);
		pTopic->UpdateSourceDetailPointers(bClearEraseSourceDetailID);
	}

	//update all details
	for(i = 0; i < GetEMNDetailCount(); i++) {
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(i);
		if(pDetail->GetSourceDetailID() != -1) {
			CEMNDetail *pSourceDetail = m_bIsTemplate ? GetParentEMN()->GetDetailByTemplateDetailID(pDetail->GetSourceDetailID()) : GetParentEMN()->GetDetailByID(pDetail->GetSourceDetailID());
			pDetail->SetSourceDetail(pSourceDetail);
			if(bClearEraseSourceDetailID)
				pDetail->SetSourceDetailID(-1);
		}
	}
}

//TES 1/23/2007 - PLID 24377 - Used when processing EMR Links, this function returns TRUE if any one of the EmrDataT.IDs
// in the array is selected on any detail in this topic or any subtopics, otherwise it returns FALSE.
BOOL CEMRTopic::IsAnyItemChecked(const CArray<long,long> &arDataIDs, const CArray<CEMNDetail*,CEMNDetail*> &arDetailsToIgnore)
{
	//Check each of our details.
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pDetail = m_arypEMNDetails[i];
		//TES 2/6/2007 - PLID 24377 - Is this one of the details we're ignoring?
		bool bIgnore = false;
		for(int j = 0; j < arDetailsToIgnore.GetSize() && !bIgnore; j++) {
			if(pDetail == arDetailsToIgnore[j]) bIgnore = true;
		}
		if(!bIgnore) {
			//Check each element on this detail.
			for(int j = 0; j < pDetail->GetListElementCount(); j++) {
				ListElement le = pDetail->GetListElement(j);
				//Is this element selected?
				if(le.bIsSelected) {
					//This element is selected, does it match anything in the list?
					for(int k = 0; k < arDataIDs.GetSize(); k++) {
						if(le.nID == arDataIDs[k]) {
							//It does!  We have a match!
							return TRUE;
						}
					}
				}
			}
		}
	}

	//None of our details matched, lets check our subtopics.
	for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		if(m_arypEMRTopics[i]->IsAnyItemChecked(arDataIDs, arDetailsToIgnore)) return TRUE;
	}
	return FALSE;
}

// (c.haag 2011-05-19) - PLID 43696 - Populates a map with all EmrDataID's that correspond to checked-off single-select
// and multi-select list items. All details in mapDetailsToIgnore are ignored during the search.
void CEMRTopic::GetAllCheckedItems(CMap<long,long,BOOL,BOOL>& mapDataIDs, const CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> &mapDetailsToIgnore)
{
	//Check each of our details.
	for(int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pDetail = m_arypEMNDetails[i];
		BOOL bDummy;
		if ((pDetail->m_EMRInfoType == eitSingleList || pDetail->m_EMRInfoType == eitMultiList) // It's a list detail
			&& !mapDetailsToIgnore.Lookup(pDetail, bDummy) // We're not ignoring the detail
			&& pDetail->GetStateVarType() == VT_BSTR && VarString(pDetail->GetState()) != "") // The state has string content in it; meaning at least one item is checked
		{
			if (pDetail->GetNeedContentReload())
			{
				// If the detail needs a content reload, then we'd have to query for all the selections.
				// Lets just parse them out of the state instead.
				CDWordArray adwDataIDs;
				FillArrayFromSemiColonDelimitedIDList(adwDataIDs, VarString(pDetail->GetState()));
				for (int j=0; j < adwDataIDs.GetSize(); j++)
				{
					mapDataIDs.SetAt(adwDataIDs[j], TRUE);
				}
			} else {
				//Check each element on this detail.
				for(int j = 0; j < pDetail->GetListElementCount(); j++) {
					ListElement le = pDetail->GetListElement(j);
					//Is this element selected?
					if(le.bIsSelected) {
						// Yep!
						mapDataIDs.SetAt(le.nID, TRUE);
					}
				}
			}
		}
	}

	// Check our subtopics.
	for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		m_arypEMRTopics[i]->GetAllCheckedItems(mapDataIDs, mapDetailsToIgnore);
	}
}

// (a.walling 2007-04-05 16:34) - PLID 25454 - returns the HTML string for the entire topic
CString CEMRTopic::GetHTML()
{
	try {
		CString strHTML;

		strHTML.Preallocate(0x1000);

		CString str;
		long nTopicID = GetID();

		BOOL bUnsaved = (nTopicID == -1); // an unsaved topic will have an id of -1!
		if (bUnsaved) {
			nTopicID = reinterpret_cast<long>(this); // cast ourself (CEMRTopic*) to a long pointer.
		}

		CString strPointer = bUnsaved ? "PT" : "ID"; // PT for pointer, ID for ID

		CString strExtraClass;

		// hide empty topics
		if (!ShowIfEmpty(TRUE) && IsEmpty(NULL, TRUE)) {
			strExtraClass += " hide";
		}

		// (a.walling 2007-10-09 11:06) - PLID 25548 - Need to use the source action if there are other visible topics here
		CString strName = GetName();
		// find all the sibling topics
		long nSiblingCount = -1;
		if (m_pParentTopic) {
			nSiblingCount = m_pParentTopic->GetSubTopicCount();
		} else if (m_pParentEMN) {
			nSiblingCount = m_pParentEMN->GetTopicCount();
		} else {
			ASSERT(FALSE);
		}

		if (GetVisible()) {
			for (int i = 0; i < nSiblingCount; i++) {
				CEMRTopic* pSiblingTopic = NULL;

				pSiblingTopic = m_pParentTopic ? m_pParentTopic->GetSubTopic(i) : m_pParentEMN->GetTopic(i);

				if (pSiblingTopic != NULL && pSiblingTopic->GetVisible()) {
					// same name
					if (strName == pSiblingTopic->GetName()) {
						// different source actions?
						//TES 3/18/2010 - PLID 37530 - The source actions can be the same iff this is a Smart Stamp action.
						// (a.walling 2010-04-06 08:57) - PLID 38059 - This can also be true for table dropdowns. So let's just simply check for that sourcetype. We could
						// check even deeper for more information, but I think that's beyond the point, which is to distinguish sibling topics with the same names.
						if (GetSourceActionID() != pSiblingTopic->GetSourceActionID() || pSiblingTopic->GetSourceActionInfo().eaoSourceType == eaoSmartStamp || pSiblingTopic->GetSourceActionInfo().eaoSourceType == eaoEmrTableDropDownItem) {
							// named differently?
							if (GetSourceActionName() != pSiblingTopic->GetSourceActionName()) {
								// yep, we need to be renamed
								strName = GetName() + " (" + GetSourceActionName() + ")";
								break;
							}
						}
					}
				}
			}
		}


		// (a.walling 2007-12-17 15:58) - PLID 28391 - Prepend this after getting HTML to help speed up getting HTML visibility
		CString strHeader;
		//strHTML += str;

		CArray<CEMNDetail*, CEMNDetail*> arSortedDetails;
		GetSortedDetailArray(arSortedDetails);
		
		// (a.walling 2009-07-06 10:33) - PLID 34793
		CArray<CEMNDetail*, CEMNDetail*> arDetailsColumnOne;
		CArray<CEMNDetail*, CEMNDetail*> arDetailsColumnTwo;

		CString strDetailHTML;
		strDetailHTML.Preallocate(0x1000);

		{
			for (int ixDetail = 0; ixDetail < arSortedDetails.GetSize(); ixDetail++) {
				CEMNDetail* pDetail = arSortedDetails.GetAt(ixDetail);

				// (a.walling 2007-10-18 15:34) - PLID 27664 - Details never need to ignore unsaved, the last saved
				// state stored in the topic object's cached HTML string.
				if (pDetail) {
					// (a.walling 2008-10-23 09:52) - PLID 27552 - Set bCheckSpawnedOutput to TRUE so it will simply return
					// an empty string if the detail has no real output (such as if it is displaying under a different detail)
					if (pDetail->GetPreviewFlags() & epfColumnOne) {
						arDetailsColumnOne.Add(pDetail);
					} else if (pDetail->GetPreviewFlags() & epfColumnTwo) {
						arDetailsColumnTwo.Add(pDetail);
					} else {
						// (a.walling 2009-07-06 12:49) - PLID 34793 - If not grouping at beginning or ending, group adjacent
						if ((m_nPreviewFlags & (epfGroupBegin|epfGroupEnd)) == 0) {
							// (a.walling 2009-07-06 10:35) - PLID 34793 - If any details in columns, flush them out.
							if (!arDetailsColumnOne.IsEmpty() && !arDetailsColumnTwo.IsEmpty()) {
								strDetailHTML += "<table class='columntable'><tbody><tr>";
								
								{
									strDetailHTML += "<td>";
									for (int ixDetailColumnIndex = 0; ixDetailColumnIndex <arDetailsColumnOne.GetSize(); ixDetailColumnIndex++) {
										strDetailHTML += arDetailsColumnOne[ixDetailColumnIndex]->GetHTML();
									}
									strDetailHTML += "</td>";
								}
								
								{
									strDetailHTML += "<td>";
									for (int ixDetailColumnIndex = 0; ixDetailColumnIndex <arDetailsColumnTwo.GetSize(); ixDetailColumnIndex++) {
										strDetailHTML += arDetailsColumnTwo[ixDetailColumnIndex]->GetHTML();
									}
									strDetailHTML += "</td>";
								}
								
								strDetailHTML += "</tr></tbody></table>";

								arDetailsColumnOne.RemoveAll();
								arDetailsColumnTwo.RemoveAll();
							} else if (!arDetailsColumnOne.IsEmpty()) {
								for (int ixDetailColumnIndex = 0; ixDetailColumnIndex <arDetailsColumnOne.GetSize(); ixDetailColumnIndex++) {
									strDetailHTML += arDetailsColumnOne[ixDetailColumnIndex]->GetHTML();
								}

								arDetailsColumnOne.RemoveAll();
							} else if (!arDetailsColumnTwo.IsEmpty()) {
								for (int ixDetailColumnIndex = 0; ixDetailColumnIndex <arDetailsColumnTwo.GetSize(); ixDetailColumnIndex++) {
									strDetailHTML += arDetailsColumnTwo[ixDetailColumnIndex]->GetHTML();
								}

								arDetailsColumnTwo.RemoveAll();
							}
						}

						strDetailHTML += pDetail->GetHTML(TRUE);
					}
				}		
			}

			{
				CString strPostColumnsHTML;

				// (a.walling 2009-07-06 10:35) - PLID 34793 - If any details in columns, flush them out.
				if (!arDetailsColumnOne.IsEmpty() && !arDetailsColumnTwo.IsEmpty()) {
					strPostColumnsHTML += "<table class='columntable'><tbody><tr>";
					
					{
						strPostColumnsHTML += "<td>";
						for (int ixDetailColumnIndex = 0; ixDetailColumnIndex <arDetailsColumnOne.GetSize(); ixDetailColumnIndex++) {
							strPostColumnsHTML += arDetailsColumnOne[ixDetailColumnIndex]->GetHTML();
						}
						strPostColumnsHTML += "</td>";
					}
					
					{
						strPostColumnsHTML += "<td>";
						for (int ixDetailColumnIndex = 0; ixDetailColumnIndex <arDetailsColumnTwo.GetSize(); ixDetailColumnIndex++) {
							strPostColumnsHTML += arDetailsColumnTwo[ixDetailColumnIndex]->GetHTML();
						}
						strPostColumnsHTML += "</td>";
					}
					
					strPostColumnsHTML += "</tr></tbody></table>";

					arDetailsColumnOne.RemoveAll();
					arDetailsColumnTwo.RemoveAll();
				} else if (!arDetailsColumnOne.IsEmpty()) {
					for (int ixDetailColumnIndex = 0; ixDetailColumnIndex <arDetailsColumnOne.GetSize(); ixDetailColumnIndex++) {
						strPostColumnsHTML += arDetailsColumnOne[ixDetailColumnIndex]->GetHTML();
					}

					arDetailsColumnOne.RemoveAll();
				} else if (!arDetailsColumnTwo.IsEmpty()) {
					for (int ixDetailColumnIndex = 0; ixDetailColumnIndex <arDetailsColumnTwo.GetSize(); ixDetailColumnIndex++) {
						strPostColumnsHTML += arDetailsColumnTwo[ixDetailColumnIndex]->GetHTML();
					}

					arDetailsColumnTwo.RemoveAll();
				}

				// (a.walling 2009-07-06 12:51) - PLID 34793 - If we are grouping at the beginning, put all the columns in now.
				if (m_nPreviewFlags & epfGroupBegin) {
					strHTML += strPostColumnsHTML;
				} else {
					// if we are grouping at end, or we are grouping adjacent and these are the final ones, then we will just append
					strDetailHTML += strPostColumnsHTML;
				}
			}



			strHTML += strDetailHTML;
		}
		strHTML += "</div>"; //TopicDivDetailsID12345 div

		// (a.walling 2007-12-18 16:47) - PLID 28436 - Cosmetic changes to EMR.CSS - Added class declaration for the topics div
		str.Format("<div class='topicbucket' id='TopicDivTopics%s%li'>", strPointer, nTopicID);
		strHTML += str;

		// (a.walling 2007-09-27 15:58) - PLID 25549 - It turns out the topic array
		// is already sorted, unlike the detail array of course. Awesome!
		// CArray<CEMRTopic*, CEMRTopic*> arSortedTopics;
		// GetSortedTopicArray(arSortedTopics);

		// now process all subtopics
		// (a.walling 2009-07-06 10:33) - PLID 34793
		{
			CString strTopicHTML;
			strTopicHTML.Preallocate(0x1000);

			CArray<CEMRTopic*, CEMRTopic*> arTopicsColumnOne;
			CArray<CEMRTopic*, CEMRTopic*> arTopicsColumnTwo;

			for (int ixSubTopic = 0; ixSubTopic < m_arypEMRTopics.GetSize(); ixSubTopic++) {
				CEMRTopic* pSubTopic = m_arypEMRTopics.GetAt(ixSubTopic);

				if (pSubTopic) {
					if (pSubTopic->GetPreviewFlags() & epfColumnOne) {
						arTopicsColumnOne.Add(pSubTopic);
					} else if (pSubTopic->GetPreviewFlags() & epfColumnTwo) {
						arTopicsColumnTwo.Add(pSubTopic);
					} else {
						
						// (a.walling 2009-07-06 12:49) - PLID 34793 - If not grouping at beginning or ending, group adjacent
						if ((m_nPreviewFlags & (epfGroupBegin|epfGroupEnd)) == 0) {
							// (a.walling 2009-07-06 10:35) - PLID 34793 - If any topics in columns, flush them out.
							if (!arTopicsColumnOne.IsEmpty() && !arTopicsColumnTwo.IsEmpty()) {
								strTopicHTML += "<table class='columntable'><tbody><tr>";

								{
									strTopicHTML += "<td>";
									for (int ixTopicColumnIndex = 0; ixTopicColumnIndex <arTopicsColumnOne.GetSize(); ixTopicColumnIndex++) {
										strTopicHTML += arTopicsColumnOne[ixTopicColumnIndex]->GetHTML();
									}
									strTopicHTML += "</td>";
								}
								
								{
									strTopicHTML += "<td>";
									for (int ixTopicColumnIndex = 0; ixTopicColumnIndex <arTopicsColumnTwo.GetSize(); ixTopicColumnIndex++) {
										strTopicHTML += arTopicsColumnTwo[ixTopicColumnIndex]->GetHTML();
									}
									strTopicHTML += "</td>";
								}
								
								strTopicHTML += "</tr></tbody></table>";

								arTopicsColumnOne.RemoveAll();
								arTopicsColumnTwo.RemoveAll();
							} else if (!arTopicsColumnOne.IsEmpty()) {
								for (int ixTopicColumnIndex = 0; ixTopicColumnIndex <arTopicsColumnOne.GetSize(); ixTopicColumnIndex++) {
									strTopicHTML += arTopicsColumnOne[ixTopicColumnIndex]->GetHTML();
								}

								arTopicsColumnOne.RemoveAll();
							} else if (!arTopicsColumnTwo.IsEmpty()) {
								for (int ixTopicColumnIndex = 0; ixTopicColumnIndex <arTopicsColumnTwo.GetSize(); ixTopicColumnIndex++) {
									strTopicHTML += arTopicsColumnTwo[ixTopicColumnIndex]->GetHTML();
								}

								arTopicsColumnTwo.RemoveAll();
							}
						}

						strTopicHTML += pSubTopic->GetHTML();
					}
				}
			}

			
			{
				CString strPostColumnsHTML;

				strPostColumnsHTML.Preallocate(0x1000);

				// (a.walling 2009-07-06 10:35) - PLID 34793 - If any topics in columns, flush them out.
				if (!arTopicsColumnOne.IsEmpty() || !arTopicsColumnTwo.IsEmpty()) {
					strPostColumnsHTML += "<table class='columntable'><tbody><tr>";

					{
						strPostColumnsHTML += "<td>";
						for (int ixTopicColumnIndex = 0; ixTopicColumnIndex <arTopicsColumnOne.GetSize(); ixTopicColumnIndex++) {
							strPostColumnsHTML += arTopicsColumnOne[ixTopicColumnIndex]->GetHTML();
						}
						strPostColumnsHTML += "</td>";
					}
					
					{
						strPostColumnsHTML += "<td>";
						for (int ixTopicColumnIndex = 0; ixTopicColumnIndex <arTopicsColumnTwo.GetSize(); ixTopicColumnIndex++) {
							strPostColumnsHTML += arTopicsColumnTwo[ixTopicColumnIndex]->GetHTML();
						}
						strPostColumnsHTML += "</td>";
					}
					
					strPostColumnsHTML += "</tr></tbody></table>";

					arTopicsColumnOne.RemoveAll();
					arTopicsColumnTwo.RemoveAll();
				} else if (!arTopicsColumnOne.IsEmpty()) {
					for (int ixTopicColumnIndex = 0; ixTopicColumnIndex <arTopicsColumnOne.GetSize(); ixTopicColumnIndex++) {
						strPostColumnsHTML += arTopicsColumnOne[ixTopicColumnIndex]->GetHTML();
					}

					arTopicsColumnOne.RemoveAll();
				} else if (!arTopicsColumnTwo.IsEmpty()) {
					for (int ixTopicColumnIndex = 0; ixTopicColumnIndex <arTopicsColumnTwo.GetSize(); ixTopicColumnIndex++) {
						strPostColumnsHTML += arTopicsColumnTwo[ixTopicColumnIndex]->GetHTML();
					}

					arTopicsColumnTwo.RemoveAll();
				}

				// (a.walling 2009-07-06 12:51) - PLID 34793 - If we are grouping at the beginning, put all the columns in now.
				if (m_nPreviewFlags & epfGroupBegin) {
					strHTML += strPostColumnsHTML;
				} else {
					// if we are grouping at end, or we are grouping adjacent and these are the final ones, then we will just append
					strTopicHTML += strPostColumnsHTML;
				}
			}

			strHTML += strTopicHTML;
		}

		strHTML += "</div>" ; //TopicDivTopicsID12345 div

		// (a.walling 2007-12-21 09:17) - PLID 28436 - Cosmetic changes to EMR.CSS
		strHTML += "</div>" ; //WholeContentBucket div

		str.Format("</div>\r\n");
		strHTML += str;

		// (a.walling 2007-12-17 16:00) - PLID 28391 - Do we have any visible HTML details?
		long nHideOptions = GetRemotePropertyInt("EMRPreview_HideDetails", g_dhPreviewDisplayHideDefaults, 0, "<None>", true);
		if ( (nHideOptions & dhEmptyTopics) || (nHideOptions & dhEmptyTopicsPrint) ) {
			long nScreen = 0, nPrint = 0;
			GetVisibleHTMLDetails(nScreen, nPrint);

			if ( (nScreen == 0) && (nHideOptions & dhEmptyTopics) ) {
				strExtraClass += " hidescreen";
			}
			if ( (nPrint == 0) && (nHideOptions & dhEmptyTopicsPrint) ) {
				strExtraClass += " hideprint";
			}
		}

		// (a.walling 2008-07-01 10:01) - PLID 30571 - Hide if we should
		{
			CEMRTopic* pWatchTopic = this;
			while (pWatchTopic) {
				if (pWatchTopic->GetPreviewFlags() & epfHideItem) {
					if (strExtraClass.Find("hideprint") == -1) strExtraClass += " hideprint";
					break;
				}

				pWatchTopic = pWatchTopic->GetParentTopic();
			}
		}
		
		// (a.walling 2008-07-01 17:55) - PLID 30571 - Apparently they want the titles of topics to be hidden as well
		CString strTitleClass;
		// (a.walling 2008-08-28 12:31) - PLID 30571 - What was I thinking! This was a remnant of pre-checkin behaviour.
		// We want to set the title class regardless of what the topic's class may be!!
		//if (strExtraClass.Find("hideprint") == -1) {
		if (GetPreviewFlags() & epfHideTitle) {
			strTitleClass = "hideprint";
		}
		//}

		// (a.walling 2009-01-07 14:54) - PLID 31961 - floating elements

		// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
		// (a.walling 2009-07-06 10:43) - PLID 34793 - Although this is all handled by the parent object now
		/*
		if (m_nPreviewFlags & epfColumnOne) {
			strExtraClass += " floatleft";
		} else if (m_nPreviewFlags & epfColumnTwo) {
			strExtraClass += " floatright";
		}
		*/

		// (a.walling 2009-07-06 10:10) - PLID 34793 - Clearing is deprecated
		/*
		if ((m_nPreviewFlags & epfClearNone) == epfClearNone) {
			strExtraClass += " clearboth";
		} else {
			if (m_nPreviewFlags & epfClearRight) {
				strExtraClass += " clearright";
			} 		
			if (m_nPreviewFlags & epfClearLeft) {
				strExtraClass += " clearleft";
			}
		}
		*/

		// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
		if (m_nPreviewFlags & epfTextRight) {
			strExtraClass += " textright";
		}

		// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
		if (m_nPreviewFlags & epfPageBreakAfter) {
			strExtraClass += " ffafter";
		} else if (m_nPreviewFlags & epfPageBreakBefore) {
			strExtraClass += " ffbefore";
		}

		// (a.walling 2007-12-17 15:59) - PLID 28391 - Now calculate the header
		strHeader.Format(
			"<div class='topic%s' id='TopicDiv%s%li'>\r\n"
			"<span class='%s'><h2><a name='TopicAnchor%s%li' href='nexemr://topic%s/?%li'>%s:</a></h2></span>\r\n"
			"<div class='WholeContentBucket'>" // (a.walling 2007-12-21 09:17) - PLID 28436 - Cosmetic changes to EMR.CSS
			"<div class='detailbucket' id='TopicDivDetails%s%li'>", strExtraClass, strPointer, nTopicID, strTitleClass, strPointer, nTopicID, strPointer, nTopicID, ConvertToHTMLEmbeddable(strName), strPointer, nTopicID);

		strHTML.Insert(0, strHeader);

		return strHTML;
	} NxCatchAllThrow("Error generating topic HTML!");

	return "<p>Error</p>";
}

// (a.walling 2007-04-09 12:51) - PLID 25549 - returns the CEMNDetail* that matched nDetailPtr
CEMNDetail* CEMRTopic::GetDetailByPointer(long nDetailPtr)
{
	try {
		long nCurrentDetailPtr;

		// process this topic first
		for (int i = 0; i < GetEMNDetailCount(); i++) {
			CEMNDetail* pDetail = GetDetailByIndex(i);

			nCurrentDetailPtr = reinterpret_cast<long>(pDetail);
			
			if (nCurrentDetailPtr == nDetailPtr)
				return pDetail;
		}

		// then subtopics
		for (int ixSubTopic = 0; ixSubTopic < GetSubTopicCount(); ixSubTopic++) {
			CEMRTopic* pSubTopic = GetSubTopic(ixSubTopic);

			if (pSubTopic) {
				CEMNDetail* pFoundDetail = NULL;

				pFoundDetail = pSubTopic->GetDetailByPointer(nDetailPtr);

				if (pFoundDetail)
					return pFoundDetail;
			}
		}
	} NxCatchAll("Error in GetDetailByPointer");
	// not found!
	return NULL;
}

CEMRTopic* CEMRTopic::GetSubTopicByPointer(long nSubTopicPtr)
{
	try {
		// scan through all our subtopics and their subtopics
		for (int ixSubTopic = 0; ixSubTopic < GetSubTopicCount(); ixSubTopic++) {
			CEMRTopic* pCurrentSubTopic = GetSubTopic(ixSubTopic);
			long nCurrentSubTopicPtr = reinterpret_cast<long>(pCurrentSubTopic);
			
			if (nCurrentSubTopicPtr == nSubTopicPtr) {
				return pCurrentSubTopic;
			} else {
				if (pCurrentSubTopic) {
					// look through its subtopics
					CEMRTopic* pFoundTopic = pCurrentSubTopic->GetSubTopicByPointer(nSubTopicPtr);

					if (pFoundTopic)
						return pFoundTopic;
				}
			}
		}
	} NxCatchAll("Error in GetSubTopicByPointer");

	return NULL;
}

// (a.walling 2012-10-04 12:41) - PLID 52878 - Verify that the given emr object pointer is a child

CEMRTopic* CEMRTopic::VerifyPointer(CEMRTopic* pTopic)
{
	foreach (CEMRTopic* p, m_arypEMRTopics) {
		if (p == pTopic) {
			return pTopic;
		}
	}

	foreach (CEMRTopic* p, m_arypEMRTopics) {
		if (p && p->VerifyPointer(pTopic)) {
			return pTopic;
		}
	}

	return NULL;
}

CEMNDetail* CEMRTopic::VerifyPointer(CEMNDetail* pDetail)
{
	foreach (CEMNDetail* p, m_arypEMNDetails) {
		if (p == pDetail) {
			return pDetail;
		}
	}

	foreach (CEMRTopic* p, m_arypEMRTopics) {
		if (p && p->VerifyPointer(pDetail)) {
			return pDetail;
		}
	}

	return NULL;
}

// (a.walling 2007-04-25 12:28) - PLID 25549 - Fill the array with sorted details
void CEMRTopic::GetSortedDetailArray(CArray<CEMNDetail*, CEMNDetail*> &arDetails)
{
	try {
		EnsureDetails();
		arDetails.Copy(m_arypEMNDetails);
		SortDetailArray(arDetails);
	} NxCatchAll("Could not get sorted detail array");
}

// (a.walling 2007-04-25 12:29) - PLID 25549 - fill the array with sorted topics
// (a.walling 2007-10-18 15:35) - PLID 25549 - No longer necessary
/*
void CEMRTopic::GetSortedTopicArray(CArray<CEMRTopic*, CEMRTopic*> &arTopics)
{
	try {
		EnsureTopics();
		arTopics.Copy(m_arypEMRTopics);
		SortTopicArray(arTopics);
	} NxCatchAll("Could not get sorted topic array");
}
*/

//DRT 8/3/2007 - PLID 26928 - See comments around m_paryPendingDeletionDetails
void CEMRTopic::AddDetailToPendingDeletion(CEMNDetail *pDetail)
{
	//This allows a detail to be added more than once.  The removal will remove all instances
	//	of the detail from the array (it's faster that way).
	m_paryPendingDeletionDetails.Add(pDetail);
	// (a.walling 2009-10-13 14:20) - PLID 36024
	pDetail->__AddRef("AddDetailToPendingDeletion");
}

//DRT 8/3/2007 - PLID 26928 - See comments around m_paryPendingDeletionDetails
void CEMRTopic::RemoveDetailFromPendingDeletion(CEMNDetail *pDetail)
{
	//DRT 9/5/2007 - Reworked so that we remove all instances of the detail
	//	from the array, not just the first one.
	for(int i = m_paryPendingDeletionDetails.GetSize() - 1; i >= 0; i--) {
		CEMNDetail *pCurrent = m_paryPendingDeletionDetails.GetAt(i);
		if(pCurrent == pDetail) {
			// (a.walling 2009-10-13 14:20) - PLID 36024
			pCurrent->__Release("RemoveDetailFromPendingDeletion");
			m_paryPendingDeletionDetails.RemoveAt(i);
		}
	}

	//It's OK if this function was called and the detail doesn't exist
}

// (a.walling 2007-10-17 14:41) - PLID 27017 - Sets the topic as loaded, probably because it was not loaded from data
// Basically pretends as if PostLoad has been called.
void CEMRTopic::SetLoaded()
{
	// PostLoad post
	m_bPostLoadCalled = TRUE;
	m_bPostLoadComplete = TRUE;

	// PostLoadTopics post
	m_bTopicsLoaded = TRUE;
	m_bHasSubTopics = m_arypEMRTopics.GetSize() > 0;

	// PostLoadDetails post
	m_bDetailsLoaded = TRUE;

	BOOL bHasDetails = FALSE;
	for (int i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		if (m_arypEMNDetails[i]->GetVisible()) {
			bHasDetails = TRUE;
			break;
		}
	}

	m_bHasDetails = bHasDetails;
}

// (a.walling 2007-12-17 15:57) - PLID 28391
void CEMRTopic::GetVisibleHTMLDetails(long& nScreen, long& nPrint)
{
	for (int i = 0; i < GetEMNDetailCount(); i++) {
		CEMNDetail* pDetail = GetDetailByIndex(i);

		// (a.walling 2008-10-23 09:53) - PLID 27552 - We need to ignore details that are not visible within this topic
		BOOL bDisplayedInOtherTopic = FALSE;

		CEMNDetail* pParentDetail = NULL;
		if (pDetail->IsSubDetail()) {
			pParentDetail = pDetail->GetSubDetailParent();
		}

		if (pParentDetail) {
			// (a.walling 2008-10-23 09:54) - PLID 27552 - GetDetailByPointer works on this topic and all subtopics, so it is
			// a quick and easy way to find out if this detail will be displayed underneath ourselves or not.
			CEMNDetail* pParentDetailInThisTopicOrSubTopics = GetDetailByPointer(reinterpret_cast<long>(pParentDetail));
			if (pParentDetailInThisTopicOrSubTopics == NULL) {
				// the detail this item is being displayed under (as a sub-detail) is not a child of this topic (or subtopics)
				// so therefore it is not visible.

				bDisplayedInOtherTopic = TRUE;
			}
		}

		if (!bDisplayedInOtherTopic) {
			if (pDetail->GetHTMLVisible())
				nScreen++;
			if (pDetail->GetHTMLVisiblePrint())
				nPrint++;
		}
	}

	for (int s = 0; s < GetSubTopicCount(); s++) {
		CEMRTopic* pTopic = GetSubTopic(s);
		pTopic->GetVisibleHTMLDetails(nScreen, nPrint);
	}
}

// (a.walling 2007-12-17 16:40) - PLID 28391
void CEMRTopic::RefreshHTMLVisibility()
{
	// (a.walling 2008-02-06 14:44) - PLID 28391 - Ignore if on a template
	if (GetParentEMN() != NULL && !GetParentEMN()->IsTemplate()) {

		long nHideOptions = GetRemotePropertyInt("EMRPreview_HideDetails", g_dhPreviewDisplayHideDefaults, 0, "<None>", true);
		if ( (nHideOptions & dhEmptyTopics) || (nHideOptions & dhEmptyTopicsPrint) ) {
			long nScreen = 0, nPrint = 0;

			GetVisibleHTMLDetails(nScreen, nPrint);

			CWnd *pWnd = GetParentEMN()->GetInterface();
			if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
				pWnd->SendMessage(NXM_REFRESH_TOPIC_HTML_VISIBILITY, (WPARAM)this, (LPARAM)MAKELPARAM(nScreen, nPrint));
			}

			if (GetParentTopic())
				GetParentTopic()->RefreshHTMLVisibility();
		}
	}
}

// (a.walling 2008-07-01 09:24) - PLID 30570
// (a.walling 2009-07-06 11:24) - PLID 34793 - Option to refresh parent
void CEMRTopic::SetPreviewFlags(DWORD dwFlags, BOOL bRefreshParent)
{
	try {
		if (m_nPreviewFlags != dwFlags) {
			m_nPreviewFlags = dwFlags;

			SetUnsaved();

			if (GetParentEMN()) {
				CWnd *pWnd = GetParentEMN()->GetInterface();
				if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
					if (bRefreshParent && m_pParentTopic != NULL) {
						pWnd->SendMessage(NXM_UPDATE_EMR_PREVIEW, (WPARAM)TRUE, (LPARAM)m_pParentTopic);
					} else if (bRefreshParent) {
						GetParentEMN()->GenerateHTMLFile(TRUE, FALSE, FALSE);
					} else {
						pWnd->SendMessage(NXM_UPDATE_EMR_PREVIEW, (WPARAM)TRUE, (LPARAM)this);
					}
				}
			}

			RefreshHTMLVisibility();
		}
	} NxCatchAll("Error setting topic preview flags");
}

// (j.jones 2008-07-21 17:28) - PLID 30729 - add all of this topic's problems
// and its details' problems to the passed-in list
// (c.haag 2008-08-14 12:05) - PLID 30820 - Added bIncludeDeletedProblems
// (c.haag 2009-05-16 14:09) - PLID 34311 - Use the new problem link structure
void CEMRTopic::GetAllProblems(CArray<CEmrProblem*, CEmrProblem*> &aryProblems, BOOL bIncludeDeletedProblems /* = FALSE */)
{
	try {

		//add problems from this topic
		int i = 0;
		for(i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL && (!pProblem->m_bIsDeleted || bIncludeDeletedProblems)) {
				EnsureProblemInArray(aryProblems, pProblem);
			}
		}

		//add problems from subtopics
		for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
			CEMRTopic *pTopic = (CEMRTopic*)m_arypEMRTopics.GetAt(i);
			if(pTopic) {
				//get the topic's problems
				pTopic->GetAllProblems(aryProblems, bIncludeDeletedProblems);
			}
		}

		//add problems from deleted subtopics
		if(bIncludeDeletedProblems) {
			for(i = 0; i < m_aryDeletedTopics.GetSize(); i++) {
				CEMRTopic *pTopic = (CEMRTopic*)m_aryDeletedTopics.GetAt(i);
				if(pTopic) {
					//get the topic's problems
					pTopic->GetAllProblems(aryProblems, bIncludeDeletedProblems);
				}
			}
		}

		//add problems from the details
		for(i = 0; i < m_arypEMNDetails.GetSize(); i++) {
			CEMNDetail *pDetail = (CEMNDetail*)m_arypEMNDetails.GetAt(i);
			if(pDetail) {
				//find each problem
				for(int j=0; j<pDetail->m_apEmrProblemLinks.GetSize(); j++) {
					CEmrProblem *pProblem = pDetail->m_apEmrProblemLinks.GetAt(j)->GetProblem();
					if(pProblem != NULL && (!pProblem->m_bIsDeleted || bIncludeDeletedProblems)) {
						EnsureProblemInArray(aryProblems, pProblem);
					}
				}
			}
		}

		//add problems from the deleted details
		if(bIncludeDeletedProblems) {
			for(i = 0; i < m_aryDeletedDetails.GetSize(); i++) {
				CEMNDetail *pDetail = (CEMNDetail*)m_aryDeletedDetails.GetAt(i);
				if(pDetail) {
					//find each problem
					for(int j=0; j<pDetail->m_apEmrProblemLinks.GetSize(); j++) {
						CEmrProblem *pProblem = pDetail->m_apEmrProblemLinks.GetAt(j)->GetProblem();
						if(pProblem != NULL) {
							EnsureProblemInArray(aryProblems, pProblem);
						}
					}
				}
			}
		}

	}NxCatchAll("Error in CEMRTopic::GetAllProblems");
}

// (j.jones 2008-07-22 08:48) - PLID 30789 - returns true if there are any undeleted problems on the topic
// (c.haag 2009-05-16 14:15) - PLID 34311 - Use the new problem link structure
BOOL CEMRTopic::HasProblems()
{
	try {

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL && !pProblem->m_bIsDeleted && !m_apEmrProblemLinks.GetAt(i)->IsDeleted()) {

				return TRUE;
			}
		}

	}NxCatchAll("Error in CEMRTopic::HasProblems");

	return FALSE;
}

// (j.jones 2008-07-22 08:48) - PLID 30789 - returns true if there are only undeleted, closed problems on the topic
// (c.haag 2009-05-16 14:16) - PLID 34311 - Use the new problem link structure
BOOL CEMRTopic::HasOnlyClosedProblems()
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

	}NxCatchAll("Error in CEMRTopic::HasOnlyClosedProblems");

	return FALSE;
}

// (j.jones 2008-07-23 15:19) - PLID 30789 - returns true if any problems are marked as modified,
// including deleted items
// (c.haag 2009-05-16 14:16) - PLID 34311 - Use the new problem link structure
BOOL CEMRTopic::HasChangedProblems()
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

	}NxCatchAll("Error in CEMRTopic::HasChangedProblems");

	return FALSE;
}

// (c.haag 2008-07-23 12:16) - PLID 30820 - Populate apProblems with a list of all deleted problems for this object and
// all its children. If a child or related EMR object is deleted, all its problems are considered deleted as well.
// (c.haag 2009-05-16 14:16) - PLID 34311 - Use the new problem structure
// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now, not problems
void CEMRTopic::GetAllDeletedEmrProblemLinks(CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks, BOOL bIncludeThisObject)
{
	int i;

	// Consistent with GenerateSaveString: Ensure this topic is loaded
	EnsureLoaded();

	// Do nothing if this topic is locked
	if (GetIsOnLockedAndSavedEMN()) {
		return;
	}

	// Deleted details
	for (i=0; i < m_aryDeletedDetails.GetSize(); i++) {
		CEMNDetail* pDetail = m_aryDeletedDetails[i];
		for (int j=0; j < pDetail->m_apEmrProblemLinks.GetSize(); j++) {
			CEmrProblemLink *pLink = pDetail->m_apEmrProblemLinks[j];
			// Don't check the deleted flag of the problem link. It's possible for a problem's
			// deleted flag to be set even if only its bound EMR object is deleted.
			// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now
			// also we need to include -1 ID links that we created but then deleted,
			// so we can reconcile against the problems in memory
			if(pLink != NULL /*&& pLink->GetID() > 0*/) {
				EnsureProblemLinkInArray(apProblemLinks, pLink);
			}
		}
	}

	// Deleted sub-topics
	for (i=0; i < m_aryDeletedTopics.GetSize(); i++) {
		m_aryDeletedTopics[i]->GetAllDeletedEmrProblemLinks(apProblemLinks, TRUE);
	}

	// Sub-topics
	for (i=0; i < m_arypEMRTopics.GetSize(); i++) {
		m_arypEMRTopics[i]->GetAllDeletedEmrProblemLinks(apProblemLinks, FALSE);
	}

	CArray<CEmrProblemLink*,CEmrProblemLink*> apTmp;
	GetAllProblemLinks(apTmp, NULL, TRUE);
	// If the following is true, it means that this topic is going to be deleted, so include all
	// the existing problems for presently undeleted objects.
	if (bIncludeThisObject) {
		for (i=0; i < apTmp.GetSize(); i++) {
			CEmrProblemLink *pLink = apTmp[i];
			// Don't check the deleted flag of the problem link. It's possible for a problem's
			// deleted flag to be set even if only its bound EMR object is deleted.
			// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now
			// also we need to include -1 ID links that we created but then deleted,
			// so we can reconcile against the problems in memory
			if(pLink != NULL /*&& pLink->GetID() > 0*/) {
				EnsureProblemLinkInArray(apProblemLinks, pLink);
			}
		}
	}
	// If we get here, this topic itself is not going to be deleted. So, go through apTmp and just
	// check for deleted problems
	else {
		for (i=0; i < apTmp.GetSize(); i++) {
			CEmrProblemLink *pLink = apTmp[i];
			// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now
			// also we need to include -1 ID links that we created but then deleted,
			// so we can reconcile against the problems in memory
			if(pLink != NULL && pLink->GetIsDeleted() /*&& pLink->GetID() > 0*/) {
				EnsureProblemLinkInArray(apProblemLinks, pLink);
			}
		}
	}
}


// (c.haag 2008-07-24 09:49) - PLID 30826 - Returns TRUE if there is at least one saved problem for this topic or any of
// its children. This does not check deleted EMR objects.
BOOL CEMRTopic::DoesTopicOrChildrenHaveSavedProblems()
{
	CArray<CEmrProblem*,CEmrProblem*> apProblems;
	GetAllProblems(apProblems);
	const int nProblems = apProblems.GetSize();
	for (int i=0; i < nProblems; i++) {
		CEmrProblem* p = apProblems[i];
		if (p && !p->m_bIsDeleted && p->m_nID > 0) {
			return TRUE;
		}
	}
	return FALSE;
}

// (z.manning 2008-10-09 16:05) - PLID 31628 - Gets all details with the given lab ID
void CEMRTopic::GetDetailsByLabID(IN const long nLabID, OUT CArray<CEMNDetail*,CEMNDetail*> &m_arypDetails)
{	
	for(int nDetail = 0; nDetail < m_arypEMNDetails.GetSize(); nDetail++)
	{
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(nDetail);
		if(pDetail->GetLabID().vt == VT_I4 && VarLong(pDetail->GetLabID()) == nLabID) {
			m_arypDetails.Add(pDetail);
		}
	}

	EnsureTopics();
	for(int nSubTopic = 0; nSubTopic < m_arypEMRTopics.GetSize(); nSubTopic++)
	{
		CEMRTopic *pSubTopic = m_arypEMRTopics.GetAt(nSubTopic);
		pSubTopic->GetDetailsByLabID(nLabID, m_arypDetails);
	}
}

// (j.jones 2009-05-21 15:58) - PLID 34325 - recurses through children and returns all problem links within the topic,
// or just the problem links that reference pFilterProblem, if pFilterProblem is not NULL
// Links that have been deleted are not returned by default, unless bIncludeDeletedLinks is TRUE
void CEMRTopic::GetAllProblemLinks(CArray<CEmrProblemLink*, CEmrProblemLink*> &aryProblemLinks, CEmrProblem *pFilterProblem /*= NULL*/, BOOL bIncludeDeletedLinks /*= FALSE*/)
{
	//throw exceptions to the caller

	//add problem links from this topic
	int i = 0;
	for(i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
		CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(i);
		CEmrProblem *pProblem = pProblemLink->GetProblem();
		if(pProblem != NULL && (pFilterProblem == NULL || pFilterProblem == pProblem)
			&& (!pProblem->m_bIsDeleted || bIncludeDeletedLinks)) {

			//add if it doesn't already exist
			BOOL bFound = FALSE;
			for(int p=0; p<aryProblemLinks.GetSize() && !bFound; p++) {
				if(aryProblemLinks.GetAt(p) == pProblemLink) {
					bFound = TRUE;
				}
			}

			if(!bFound) {
				aryProblemLinks.Add(pProblemLink);
			}
		}
	}

	//add problem links from subtopics
	for(i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		CEMRTopic *pTopic = (CEMRTopic*)m_arypEMRTopics.GetAt(i);
		if(pTopic) {
			//get the topic's problem links
			pTopic->GetAllProblemLinks(aryProblemLinks, pFilterProblem, bIncludeDeletedLinks);
		}
	}

	//add problem links from deleted subtopics
	if(bIncludeDeletedLinks) {
		for(i = 0; i < m_aryDeletedTopics.GetSize(); i++) {
			CEMRTopic *pTopic = (CEMRTopic*)m_aryDeletedTopics.GetAt(i);
			if(pTopic) {
				//get the topic's problem links
				pTopic->GetAllProblemLinks(aryProblemLinks, pFilterProblem, bIncludeDeletedLinks);
			}
		}
	}

	//add problem links from the details
	for(i = 0; i < m_arypEMNDetails.GetSize(); i++) {
		CEMNDetail *pDetail = (CEMNDetail*)m_arypEMNDetails.GetAt(i);
		if(pDetail) {
			//find each problem
			for(int j=0; j<pDetail->m_apEmrProblemLinks.GetSize(); j++) {
				CEmrProblemLink *pProblemLink = pDetail->m_apEmrProblemLinks.GetAt(j);
				CEmrProblem *pProblem = pProblemLink->GetProblem();
				if(pProblem != NULL && (pFilterProblem == NULL || pFilterProblem == pProblem)
					&& (!pProblem->m_bIsDeleted || bIncludeDeletedLinks)) {

					//add if it doesn't already exist
					BOOL bFound = FALSE;
					for(int p=0; p<aryProblemLinks.GetSize() && !bFound; p++) {
						if(aryProblemLinks.GetAt(p) == pProblemLink) {
							bFound = TRUE;
						}
					}

					if(!bFound) {
						aryProblemLinks.Add(pProblemLink);
					}
				}
			}
		}
	}

	//add problem links from the deleted details
	if(bIncludeDeletedLinks) {
		for(i = 0; i < m_aryDeletedDetails.GetSize(); i++) {
			CEMNDetail *pDetail = (CEMNDetail*)m_aryDeletedDetails.GetAt(i);
			if(pDetail) {
				//find each problem
				for(int j=0; j<pDetail->m_apEmrProblemLinks.GetSize(); j++) {
					CEmrProblemLink *pProblemLink = pDetail->m_apEmrProblemLinks.GetAt(j);
					CEmrProblem *pProblem = pProblemLink->GetProblem();
					if(pProblem != NULL && (pFilterProblem == NULL || pFilterProblem == pProblem)) {

						//add if it doesn't already exist
						BOOL bFound = FALSE;
						for(int p=0; p<aryProblemLinks.GetSize() && !bFound; p++) {
							if(aryProblemLinks.GetAt(p) == pProblemLink) {
								bFound = TRUE;
							}
						}

						if(!bFound) {
							aryProblemLinks.Add(pProblemLink);
						}
					}
				}
			}
		}
	}
}

// (z.manning 2009-06-23 14:53) - PLID 34692 - Function to go through all subtopics on the topic
// and make sure that a topic with the given ID is not in any of the deleted topic arrays.
void CEMRTopic::EnsureTopicNotDeleted(const long nTopicID)
{
	for(int nTopicIndex = 0; nTopicIndex < m_arypEMRTopics.GetSize(); nTopicIndex++)
	{
		CEMRTopic *pTopic = m_arypEMRTopics.GetAt(nTopicIndex);
		if(pTopic != NULL) {
			pTopic->EnsureTopicNotDeleted(nTopicID);
		}
	}

	for(int nDeletedTopicIndex = m_aryDeletedTopics.GetSize() - 1; nDeletedTopicIndex >= 0; nDeletedTopicIndex--)
	{
		CEMRTopic *pDeletedTopic = m_aryDeletedTopics.GetAt(nDeletedTopicIndex);
		if(pDeletedTopic->GetID() == nTopicID) {
			m_aryDeletedTopics.RemoveAt(nDeletedTopicIndex);
		}
	}
}

long CEMRTopic::GetTopicOrderIndex()
{
	if(m_pTopicPositionEntry) {
		//TES 4/15/2010 - PLID 24692 - Just pull from our position entry.
		return m_pTopicPositionEntry->GetSortOrder();
	}
	else {
		//TES 8/10/2010 - PLID 24692 - We don't have a topic-position entry; that means we've been loaded bottom-up, and the order index
		// won't be changing, but we still need to return something.  Just ask our parent what our order index is.
		if(m_pParentTopic) {
			return m_pParentTopic->GetChildOrderIndex(this);
		}
		else if(m_pParentEMN) {
			return m_pParentEMN->GetChildOrderIndex(this);
		}
		else {
			//TES 8/10/2010 - PLID 24692 - It shouldn't be possible to not have a parent.
			ASSERT(FALSE);
			return -1;
		}
	}
}

//TES 8/10/2010 - PLID 24692 - Gets the order index of the given topic within our list of topics (non-recursive).
long CEMRTopic::GetChildOrderIndex(CEMRTopic *pChild)
{
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		if(m_arypEMRTopics[i] == pChild) {
			return i;
		}
	}
	return -1;
}

// (z.manning 2011-03-02 11:16) - PLID 42638
BOOL CEMRTopic::ContainsSmartStampTable()
{
	for(int nDetail = 0; nDetail < m_arypEMNDetails.GetSize(); nDetail++)
	{
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(nDetail);
		if(pDetail->IsSmartStampTable()) {
			return TRUE;
		}
	}

	for(int nTopic = 0; nTopic < m_arypEMRTopics.GetSize(); nTopic++)
	{
		CEMRTopic *pTopic = m_arypEMRTopics.GetAt(nTopic);
		if(pTopic->ContainsSmartStampTable()) {
			return TRUE;
		}
	}

	return FALSE;
}

//TES 4/13/2012 - PLID 49482
BOOL CEMRTopic::ContainsSmartStampImage()
{
	for(int nDetail = 0; nDetail < m_arypEMNDetails.GetSize(); nDetail++)
	{
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(nDetail);
		if(pDetail->IsSmartStampImage()) {
			return TRUE;
		}
	}

	for(int nTopic = 0; nTopic < m_arypEMRTopics.GetSize(); nTopic++)
	{
		CEMRTopic *pTopic = m_arypEMRTopics.GetAt(nTopic);
		if(pTopic->ContainsSmartStampImage()) {
			return TRUE;
		}
	}

	return FALSE;
}

// (z.manning 2011-03-04 14:13) - PLID 42682
void CEMRTopic::UpdateSourceDetailStampIDs(const long nOldDetailStampID, const long nNewDetailStampID)
{
	if(m_sai.GetDetailStampID() == nOldDetailStampID) {
		m_sai.SetDetailStampID(nNewDetailStampID);
	}

	for(int nDetail = 0; nDetail < m_arypEMNDetails.GetSize(); nDetail++) {
		CEMNDetail *pDetail = m_arypEMNDetails.GetAt(nDetail);
		SourceActionInfo sai = pDetail->GetSourceActionInfo();
		if(sai.GetDetailStampID() == nOldDetailStampID) {
			sai.SetDetailStampID(nNewDetailStampID);
			pDetail->SetSourceActionInfo(sai);
		}
	}

	for(int nTopic = 0; nTopic < m_arypEMRTopics.GetSize(); nTopic++) {
		CEMRTopic *pTopic = m_arypEMRTopics.GetAt(nTopic);
		pTopic->UpdateSourceDetailStampIDs(nOldDetailStampID, nNewDetailStampID);
	}
}
//TES 3/18/2011 - PLID 42762 - Used in the function below for handling all the different RxNumber fields.
//TES 4/11/2012 - PLID 49621 - Added the prefix argument, so that this could be used for Contact Lens orders as well.
#define HANDLE_TEXT_PRESCRIPTION_FIELD(prefix, Field, strValue, pnf)	case gorn##Field##: {\
													if(!IsValidPrescriptionNumber(strValue, pnf)) { \
														if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";\
														strIgnoredElements += pTr->strName + " - " + pTc->strName;\
													} \
													if(gol == golOD) { \
													if(!goOrder.##prefix##OD.str##Field##.IsEmpty()) {\
															if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";\
															strIgnoredElements += pTr->strName + " - " + pTc->strName;\
														}\
														else {\
															goOrder.##prefix##OD.str##Field## = strValue;\
														}\
													}\
													else {\
														ASSERT(gol == golOS);\
														if(!goOrder.##prefix##OS.str##Field##.IsEmpty()) {\
															if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";\
															strIgnoredElements += pTr->strName + " - " + pTc->strName;\
														}\
														else {\
															goOrder.##prefix##OS.str##Field## = strValue;\
														}\
													}\
												}\
												break;

BOOL CEMRTopic::GetGlassesOrderData(IN OUT GlassesOrder &goOrder, IN OUT CString &strIgnoredData)
{
	//TES 3/18/2011 - PLID 42762 - Go through all our details, find any with Glasses Order data
	BOOL bDataAdded = FALSE;
	for(int nDetail = 0; nDetail < m_arypEMNDetails.GetSize(); nDetail++) {
		CEMNDetail *pDetail = m_arypEMNDetails[nDetail];
		if(pDetail->m_bHasGlassesOrderData) {
			pDetail->LoadContent();
			CString strDetailName = pDetail->GetLabelText();
			CString strIgnoredElements;
			//TES 3/18/2011 - PLID 42762 - Now we need to determine what's selected on this item.  First, what type is it?
			if(pDetail->m_EMRInfoType == eitSingleList || pDetail->m_EMRInfoType == eitMultiList) {
				//TES 3/18/2011 - PLID 42762 - OK, make sure that this detail is associated with a lens.
				GlassesOrderLens gol = pDetail->m_golLens;
				if(gol != golInvalid) {
					//TES 3/18/2011 - PLID 42762 - Now go through each list item that is selected, and associated with Glasses Order data
					for(int nListElement = 0; nListElement < pDetail->GetListElementCount(); nListElement++) {
						ListElement le = pDetail->GetListElement(nListElement);
						if(le.bIsSelected && le.godtGlassesOrderDataType != godtInvalid && le.nGlassesOrderDataID != -1) {
							//TES 3/18/2011 - PLID 42762 - Got something!  Can we add it?
							bDataAdded = TRUE;
							switch(le.godtGlassesOrderDataType) {
								case godtDesign:
									if(gol == golOD) {
										if(goOrder.golOD.nDesignID != -1) {
											//TES 3/18/2011 - PLID 42762 - We've already got this field, so this element will be ignored
											if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";
											strIgnoredElements += le.strName;
										}
										else {
											goOrder.golOD.nDesignID = le.nGlassesOrderDataID;
										}
									}
									else {
										ASSERT(gol == golOS);
										if(goOrder.golOS.nDesignID != -1) {
											//TES 3/18/2011 - PLID 42762 - We've already got this field, so this element will be ignored
											if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";
											strIgnoredElements += le.strName;
										}
										else {
											goOrder.golOS.nDesignID = le.nGlassesOrderDataID;
										}
									}
									break;
								case godtMaterial:
									if(gol == golOD) {
										if(goOrder.golOD.nMaterialID != -1) {
											//TES 3/18/2011 - PLID 42762 - We've already got this field, so this element will be ignored
											if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";
											strIgnoredElements += le.strName;
										}
										else {
											goOrder.golOD.nMaterialID = le.nGlassesOrderDataID;
										}
									}
									else {
										ASSERT(gol == golOS);
										if(goOrder.golOS.nMaterialID != -1) {
											//TES 3/18/2011 - PLID 42762 - We've already got this field, so this element will be ignored
											if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";
											strIgnoredElements += le.strName;
										}
										else {
											goOrder.golOS.nMaterialID = le.nGlassesOrderDataID;
										}
									}
									break;
								case godtTreatment:
									//TES 3/18/2011 - PLID 42762 - We can have as many treatments as we want.
									if(gol == golOD) {
										goOrder.golOD.arTreatmentIDs.Add(le.nGlassesOrderDataID);
									}
									else {
										ASSERT(gol == golOS);
										goOrder.golOS.arTreatmentIDs.Add(le.nGlassesOrderDataID);
									}
									break;
								default:
									//TES 3/18/2011 - PLID 42762 - No other types are valid for list-type details.
									ASSERT(FALSE);
									break;
							}
						}
					}
				}
			}
			else if(pDetail->m_EMRInfoType == eitTable) {
				//TES 3/18/2011 - PLID 42762 - We're going to be going through all the selected dropdown items, so go ahead and tell this
				// detail to pull all the associated GlassesOrderDataIDs from the database if it hasn't already.
				pDetail->LoadAllSelectedDropdowns();
				for(int nRow = 0; nRow < pDetail->GetRowCount(); nRow++) {
					//TES 3/18/2011 - PLID 42762 - Is this row tied to a lens?
					TableRow *pTr = pDetail->GetRowPtr(nRow);
					if(pTr->m_GlassesOrderDataType != godtInvalid) {
						ASSERT(pTr->m_GlassesOrderDataType == godtLens);
						GlassesOrderLens gol = (GlassesOrderLens)pTr->m_nGlassesOrderDataID;
						if(gol != golInvalid) {
							//TES 3/18/2011 - PLID 42762 - OK, this row has a lens, now go through each of the columns.
							for(int nCol = 0; nCol < pDetail->GetColumnCount(); nCol++) {
								TableColumn *pTc = pDetail->GetColumnPtr(nCol);
								if(pTc->nType == LIST_TYPE_TEXT) {
									//TES 3/18/2011 - PLID 42762 - OK, text types have to be RxNumbers (if anything).
									if(pTc->m_GlassesOrderDataType != godtInvalid) {
										ASSERT(pTc->m_GlassesOrderDataType == godtRxNumber);
										TableElement *pTe = pDetail->GetTableElementByRowColPtr(pTr, pTc);
										//TES 3/18/2011 - PLID 42762 - Does this field have anything?
										//TES 3/21/2011 - PLID 42762 - If GetTableElement() returns NULL, it means the element is blank,
										// so we don't need to check it.
										if(pTe && !pTe->m_strValue.IsEmpty()) {
											//TES 3/18/2011 - PLID 42762 - Yes, so we're definitely adding something.
											bDataAdded = TRUE;
											//TES 3/18/2011 - PLID 42762 - Now copy the data to the appropriate field in the GlassesOrder struct
											GlassesOrderRxNumber gorn = (GlassesOrderRxNumber)pTc->m_nGlassesOrderDataID;
											switch(gorn) {
												//TES 4/11/2012 - PLID 49621 - Pass in gol
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Sphere, pTe->m_strValue, pnfSignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Cylinder, pTe->m_strValue, pnfSignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Axis, pTe->m_strValue, pnfInt);
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Addition, pTe->m_strValue, pnfUnsignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Prism, pTe->m_strValue, pnfUnsignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Base, pTe->m_strValue, pnfBase);
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, DistPD, pTe->m_strValue, pnfNaturalFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, NearPD, pTe->m_strValue, pnfNaturalFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Height, pTe->m_strValue, pnfNaturalFloat);
											}
										}
									}
								}
								else if(pTc->nType == LIST_TYPE_DROPDOWN) {
									//TES 3/18/2011 - PLID 42762 - Get all the GlassesOrderDataIDs for the items selected in this cell.
									CArray<long,long> arGlassesOrderDataIDs;
									pDetail->GetSelectedGlassesOrderDataIDs(pTr, pTc, arGlassesOrderDataIDs);
									//TES 3/18/2011 - PLID 42762 - Did we get anything?
									if(arGlassesOrderDataIDs.GetSize()) {
										//TES 3/18/2011 - PLID 42762 - We did, so add to the struct, based on the column's godt
										switch(pTc->m_GlassesOrderDataType) {
											case godtDesign:
												bDataAdded = TRUE;
												for(int i = 0; i < arGlassesOrderDataIDs.GetSize(); i++) {
													if(gol == golOD) {
														if(goOrder.golOD.nDesignID != -1) {
															//TES 3/18/2011 - PLID 42762 - We've already got this field, so this element will be ignored
															if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";
															strIgnoredElements += pTr->strName + " - " + pTc->strName;
														}
														else {
															goOrder.golOD.nDesignID = arGlassesOrderDataIDs[i];
														}
													}
													else {
														ASSERT(gol == golOS);
														if(goOrder.golOS.nDesignID != -1) {
															//TES 3/18/2011 - PLID 42762 - We've already got this field, so this element will be ignored
															if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";
															strIgnoredElements += pTr->strName + " - " + pTc->strName;
														}
														else {
															goOrder.golOS.nDesignID = arGlassesOrderDataIDs[i];
														}
													}
												}
												break;
											case godtMaterial:
												bDataAdded = TRUE;
												for(int i = 0; i < arGlassesOrderDataIDs.GetSize(); i++) {
													if(gol == golOD) {
														if(goOrder.golOD.nMaterialID != -1) {
															//TES 3/18/2011 - PLID 42762 - We've already got this field, so this element will be ignored
															if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";
															strIgnoredElements += pTr->strName + " - " + pTc->strName;
														}
														else {
															goOrder.golOD.nMaterialID = arGlassesOrderDataIDs[i];
														}
													}
													else {
														ASSERT(gol == golOS);
														if(goOrder.golOS.nMaterialID != -1) {
															//TES 3/18/2011 - PLID 42762 - We've already got this field, so this element will be ignored
															if(!strIgnoredElements.IsEmpty()) strIgnoredElements += ", ";
															strIgnoredElements += pTr->strName + " - " + pTc->strName;
														}
														else {
															goOrder.golOS.nMaterialID = arGlassesOrderDataIDs[i];
														}
													}
												}
												break;
											case godtTreatment:
												bDataAdded = TRUE;
												//TES 3/18/2011 - PLID 42762 - We can add as many treatments as we like
												for(int i = 0; i < arGlassesOrderDataIDs.GetSize(); i++) {
													if(gol == golOD) {
														goOrder.golOD.arTreatmentIDs.Add(arGlassesOrderDataIDs[i]);
													}
													else {
														ASSERT(gol == golOS);
														goOrder.golOS.arTreatmentIDs.Add(arGlassesOrderDataIDs[i]);
													}
												}
												break;
											case godtRxNumber:
												{
													//TES 3/18/2011 - PLID 42762 - So, for this, we'll just be treating the dropdown items
													// as text, essentially, so get the output for the cell, that's what will go on the order.
													bDataAdded = TRUE;
													TableElement *pTe = pDetail->GetTableElementByRowColPtr(pTr, pTc);
													//TES 3/21/2011 - PLID 42762 - If GetTableElement() returns NULL, it means the element is blank,
													// so we don't need to check it.
													if(pTe) {
														CStringArray saTmp;
														// (a.walling 2011-05-31 11:56) - PLID 42448
														CString strRxNumber = pTe->GetValueAsOutput(pDetail, false, saTmp);
														ASSERT(saTmp.IsEmpty());
														//TES 3/18/2011 - PLID 42762 - Now get the specific field for this column, and update
														// the GlassesOrder struct appropriately.
														GlassesOrderRxNumber gorn = (GlassesOrderRxNumber)pTc->m_nGlassesOrderDataID;
														switch(gorn) {
															//TES 4/11/2012 - PLID 49621 - Pass in gol
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Sphere, strRxNumber, pnfSignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Cylinder, strRxNumber, pnfSignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Axis, strRxNumber, pnfInt);
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Addition, strRxNumber, pnfUnsignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Prism, strRxNumber, pnfUnsignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Base, strRxNumber, pnfBase);
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, DistPD, strRxNumber, pnfNaturalFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, NearPD, strRxNumber, pnfNaturalFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(gol, Height, strRxNumber, pnfNaturalFloat);
														}
													}
												}
												break;
										}
									}
								}
								else {
									//TES 3/18/2011 - PLID 42762 - Text and Dropdown are the only column types that have Glasses Order data, 
									// so we're done
								}
							}
						}
					}
				}
				
			}
			else {
				//TES 3/18/2011 - PLID 42762 - Lists and Tables are the only detail types that have Glasses Order data, so we're done
			}

			//TES 3/18/2011 - PLID 42762 - We've finished this detail, add any ignored data to the output.
			if(!strIgnoredElements.IsEmpty()) {
				strIgnoredData += strDetailName + ": " + strIgnoredElements + "\r\n";
			}
		}		
	}

	//TES 3/18/2011 - PLID 42762 - Now give our subtopics a chance
	for(int nTopic = 0; nTopic < m_arypEMRTopics.GetSize(); nTopic++) {
		CEMRTopic *pSubTopic = m_arypEMRTopics[nTopic];
		if(pSubTopic->GetGlassesOrderData(goOrder, strIgnoredData)) {
			bDataAdded = TRUE;
		}
	}

	return bDataAdded;
}

//TES 4/11/2012 - PLID 49621 - Added, gathers any information on that could be loaded into a Contact Lens Order.  Same as GetGlassesOrderData
BOOL CEMRTopic::GetContactLensOrderData(IN OUT ContactLensOrder &goOrder, IN OUT CString &strIgnoredData)
{
	//TES 4/11/2012 - PLID 49621 - Go through all our details, find any with Contact Lens data
	BOOL bDataAdded = FALSE;
	for(int nDetail = 0; nDetail < m_arypEMNDetails.GetSize(); nDetail++) {
		CEMNDetail *pDetail = m_arypEMNDetails[nDetail];
		if(pDetail->m_bHasContactLensData) {
			pDetail->LoadContent();
			CString strDetailName = pDetail->GetLabelText();
			CString strIgnoredElements;
			//TES 4/11/2012 - PLID 49621 - Only tables support contact lens data at present.
			ASSERT(pDetail->m_EMRInfoType == eitTable);
			if(pDetail->m_EMRInfoType == eitTable) {
				//TES 4/11/2012 - PLID 49621 - We're going to be going through all the selected dropdown items, so go ahead and tell this
				// detail to pull all the associated GlassesOrderDataIDs from the database if it hasn't already.
				pDetail->LoadAllSelectedDropdowns();
				for(int nRow = 0; nRow < pDetail->GetRowCount(); nRow++) {
					//TES 4/11/2012 - PLID 49621 - Is this row tied to a lens?
					TableRow *pTr = pDetail->GetRowPtr(nRow);
					if(pTr->m_GlassesOrderDataType != godtInvalid) {
						ASSERT(pTr->m_GlassesOrderDataType == godtLens);
						GlassesOrderLens gol = (GlassesOrderLens)pTr->m_nGlassesOrderDataID;
						if(gol != golInvalid) {
							//TES 4/11/2012 - PLID 49621 - OK, this row has a lens, now go through each of the columns.
							for(int nCol = 0; nCol < pDetail->GetColumnCount(); nCol++) {
								TableColumn *pTc = pDetail->GetColumnPtr(nCol);
								if(pTc->nType == LIST_TYPE_TEXT) {
									//TES 4/11/2012 - PLID 49621 - OK, text types have to be RxNumbers (if anything).
									if(pTc->m_GlassesOrderDataType != godtInvalid) {
										ASSERT(pTc->m_GlassesOrderDataType == godtRxNumber);
										TableElement *pTe = pDetail->GetTableElementByRowColPtr(pTr, pTc);
										//TES 4/11/2012 - PLID 49621 - Does this field have anything?
										//TES 4/11/2012 - PLID 49621 - If GetTableElement() returns NULL, it means the element is blank,
										// so we don't need to check it.
										if(pTe && !pTe->m_strValue.IsEmpty()) {
											//TES 4/11/2012 - PLID 49621 - Yes, so we're definitely adding something.
											bDataAdded = TRUE;
											//TES 4/11/2012 - PLID 49621 - Now copy the data to the appropriate field in the GlassesOrder struct
											GlassesOrderRxNumber gorn = (GlassesOrderRxNumber)pTc->m_nGlassesOrderDataID;
											switch(gorn) {
												//TES 4/11/2012 - PLID 49621 - Check each of the contact lens prescription numbers, passing
												// in clol to let it know this is for a Contact Lens Order
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Sphere, pTe->m_strValue, pnfSignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Cylinder, pTe->m_strValue, pnfSignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Axis, pTe->m_strValue, pnfInt);
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Addition, pTe->m_strValue, pnfUnsignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, BC, pTe->m_strValue, pnfUnsignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Diameter, pTe->m_strValue, pnfUnsignedFloat);
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Color, pTe->m_strValue, pnfGeneralText);
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Quantity, pTe->m_strValue, pnfInt);
												// (j.dinatale 2013-03-20 14:22) - PLID 55766 - Need the new EMN brand/manufacturer field to show up on the previous rx dialog.
												HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Manufacturer, pTe->m_strValue, pnfGeneralText);
											}
										}
									}
								}
								else if(pTc->nType == LIST_TYPE_DROPDOWN) {
									//TES 4/11/2012 - PLID 49621 - Get all the GlassesOrderDataIDs for the items selected in this cell.
									CArray<long,long> arGlassesOrderDataIDs;
									pDetail->GetSelectedGlassesOrderDataIDs(pTr, pTc, arGlassesOrderDataIDs);
									//TES 4/11/2012 - PLID 49621 - Did we get anything?
									if(arGlassesOrderDataIDs.GetSize()) {
										//TES 4/11/2012 - PLID 49621 - We did, so add to the struct, based on the column's godt (which should
										// always be RxNumber for Contact Lens details)
										//TES 6/27/2012 - PLID 49621 - Took out this ASSERT, the type may be godtInvalid, and anyway the 
										// switch statement properly handles the data type by ignoring anything unexpected.
										//ASSERT(pTc->m_GlassesOrderDataType == godtRxNumber);
										switch(pTc->m_GlassesOrderDataType) {
											case godtRxNumber:
												{
													//TES 4/11/2012 - PLID 49621 - So, for this, we'll just be treating the dropdown items
													// as text, essentially, so get the output for the cell, that's what will go on the order.
													bDataAdded = TRUE;
													TableElement *pTe = pDetail->GetTableElementByRowColPtr(pTr, pTc);
													//TES 4/11/2012 - PLID 49621 - If GetTableElement() returns NULL, it means the element is blank,
													// so we don't need to check it.
													if(pTe) {
														CStringArray saTmp;
														// (a.walling 2011-05-31 11:56) - PLID 42448
														CString strRxNumber = pTe->GetValueAsOutput(pDetail, false, saTmp);
														ASSERT(saTmp.IsEmpty());
														//TES 4/11/2012 - PLID 49621 - Now get the specific field for this column, and update
														// the GlassesOrder struct appropriately.
														GlassesOrderRxNumber gorn = (GlassesOrderRxNumber)pTc->m_nGlassesOrderDataID;
														switch(gorn) {
															//TES 4/11/2012 - PLID 49621 - Check each of the contact lens prescription numbers, passing
															// in clol to let it know this is for a Contact Lens Order
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Sphere, strRxNumber, pnfSignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Cylinder, strRxNumber, pnfSignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Axis, strRxNumber, pnfInt);
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Addition, strRxNumber, pnfUnsignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, BC, strRxNumber, pnfUnsignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Diameter, strRxNumber, pnfUnsignedFloat);
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Color, strRxNumber, pnfGeneralText);
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Quantity, strRxNumber, pnfInt);
															// (j.dinatale 2013-03-20 14:23) - PLID 55766 - Need the new EMN brand/manufacturer field to show up on the previous rx dialog.
															HANDLE_TEXT_PRESCRIPTION_FIELD(clol, Manufacturer, strRxNumber, pnfGeneralText);
														}
													}
												}
												break;
										}
									}
								}
								else {
									//TES 4/11/2012 - PLID 49621 - Text and Dropdown are the only column types that have Contact Lens Order data, 
									// so we're done
								}
							}
						}
					}
				}
				
			}
			else {
				//TES 4/11/2012 - PLID 49621 -Tables are the only detail types that have Contact Lens Order data, so we're done
			}

			//TES 4/11/2012 - PLID 49621 -We've finished this detail, add any ignored data to the output.
			if(!strIgnoredElements.IsEmpty()) {
				strIgnoredData += strDetailName + ": " + strIgnoredElements + "\r\n";
			}
		}		
	}

	//TES 4/11/2012 - PLID 49621 - Now give our subtopics a chance
	for(int nTopic = 0; nTopic < m_arypEMRTopics.GetSize(); nTopic++) {
		CEMRTopic *pSubTopic = m_arypEMRTopics[nTopic];
		if(pSubTopic->GetContactLensOrderData(goOrder, strIgnoredData)) {
			bDataAdded = TRUE;
		}
	}

	return bDataAdded;
}

CRect CEMRTopic::GetClientArea()
{	
	// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
	if (GetTopicWnd()) {
		CRect rc;
		GetTopicWnd()->GetClientRect(&rc);
		return rc;
	}

	CRect rc(0, 0, 1, 1);

	EnsureDetails();

	for (long i = 0; i < m_arypEMNDetails.GetCount(); ++i) {
		CEMNDetail* pDetail = m_arypEMNDetails[i];
		if (pDetail->GetVisible()) {
			rc |= m_arypEMNDetails[i]->GetClientAreaDirect();
		}
	}

	ASSERT(rc.top == 0 && rc.left == 0);

	return rc;
}

// (j.jones 2013-06-18 13:23) - PLID 47217 - added ability to remove all signature details from new topics
void CEMRTopic::RemoveNewSignatureDetails()
{
	//this should never be called on a saved topic
	if(m_nID != -1) {
		//silently return, but this is a coding error, as this function
		//is only meant to be called on new topics
		ASSERT(FALSE);
		return;
	}

	//Tell our subtopics.
	for(int i = 0; i < m_arypEMRTopics.GetSize(); i++) {
		m_arypEMRTopics[i]->RemoveNewSignatureDetails();
	}
	
	//for each detail, if the detail is a signature, remove it
	for(int i = m_arypEMNDetails.GetSize()-1; i >= 0; i--) {
		CEMNDetail *pDetail = m_arypEMNDetails[i];
		if(pDetail->IsSignatureDetail()) {
			//the detail should be new
			if(pDetail->GetID() == -1) {
				RemoveDetail(pDetail);
			}
			else {
				//should never be called on saved details,
				//but also this shouldn't be called on saved EMNs
				//in the first place
				ASSERT(FALSE);
			}
		}
	}
}